/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * conf_parser.c - PFC configuration file parser.
 */

#include <stdarg.h>
#include <string.h>
#include <pfc/debug.h>
#include <pfc/atomic.h>
#include "conf_impl.h"

/*
 * Global read/write lock for configuration file parser.
 */
pfc_rwlock_t	conf_parser_lock PFC_ATTR_HIDDEN = PFC_RWLOCK_INITIALIZER;

/*
 * Parser state used by conf_parser_params().
 */
typedef enum {
	CONF_PSTATE_NAME,
	CONF_PSTATE_NAME_END,
	CONF_PSTATE_VALUE,
	CONF_PSTATE_VALUE_END,
} conf_param_state_t;

/*
 * Set temporary refptr into the parser context so that it could be released
 * on error.
 */
#define	CONF_PARSER_REFPTR_SET(parser, refptr)			\
	do {							\
		PFC_ASSERT((parser)->cfp_refptr == NULL);	\
		(parser)->cfp_refptr = (refptr);		\
	} while (0)

/*
 * Clear temporary refptr set by CONF_PARSER_REFPTR_SET().
 */
#define	CONF_PARSER_REFPTR_CLEAR(parser)			\
	do {							\
		(parser)->cfp_refptr = NULL;			\
	} while (0)

/* Default error handler. */
static pfc_conf_err_t	conf_errfunc;

/*
 * Operations to parse parameter value.
 */
typedef struct {
	/*
	 * Parse the specified token as parameter value.
	 * Value is stored into `param'.
	 * Any resource in the specified token must be released by this
	 * function.
	 */
	void		(*parse)(conf_parser_t *PFC_RESTRICT parser,
				 conf_param_t *PFC_RESTRICT param,
				 conf_token_t *PFC_RESTRICT token);

	/*
	 * Create array parameter and return pointer to it.
	 * This function convert elements in parser->cfp_array into
	 * simple array. This function must release all conf_param_t struct
	 * in parser->cfp_array.
	 *
	 * Note that this function is never called if no element exists in
	 * parser->cfp_array. So the caller must guarantees that it never
	 * passes zero to `size'.
	 */
	pfc_ptr_t	(*to_array)(conf_parser_t *parser, uint32_t size);
} value_parser_t;

/*
 * Configuration handle associated with the PFC system configuration file.
 */
pfc_conf_t	pfcd_conf PFC_ATTR_HIDDEN = PFC_CONF_INVALID;

/*
 * Global read/write lock to protect the PFC system configuration file handle.
 */
static pfc_rwlock_t	sysconf_lock = PFC_RWLOCK_INITIALIZER;

#define	SYSCONF_RDLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&sysconf_lock), 0)
#define	SYSCONF_WRLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&sysconf_lock), 0)
#define	SYSCONF_UNLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&sysconf_lock), 0)

#define	SYSCONF_TRYWRLOCK()	pfc_rwlock_trywrlock(&sysconf_lock)

/*
 * Internal prototypes.
 */
static int	conf_do_open(pfc_conf_t *PFC_RESTRICT confp,
			     pfc_refptr_t *PFC_RESTRICT rpath,
			     const pfc_cfdef_t *PFC_RESTRICT defs,
			     pfc_conf_err_t errfunc, uint32_t flags);
static int	conf_do_open2(pfc_conf_t *PFC_RESTRICT confp,
			      pfc_refptr_t *PFC_RESTRICT primary,
			      pfc_refptr_t *PFC_RESTRICT secondary,
			      const pfc_cfdef_t *PFC_RESTRICT defs,
			      pfc_conf_err_t errfunc, uint32_t flags);
static int	conf_open(pfc_conf_t *PFC_RESTRICT confp,
			  const char *PFC_RESTRICT path,
			  const pfc_cfdef_t *PFC_RESTRICT defs,
			  pfc_conf_err_t errfunc);
static int	conf_refopen(pfc_conf_t *PFC_RESTRICT confp,
			     pfc_refptr_t *PFC_RESTRICT rpath,
			     const pfc_cfdef_t *PFC_RESTRICT defs,
			     pfc_conf_err_t errfunc);
static int	conf_parser_create(conf_parser_t **PFC_RESTRICT parserp,
				   pfc_refptr_t *PFC_RESTRICT rpath,
				   const pfc_cfdef_t *PFC_RESTRICT defs,
				   pfc_conf_err_t errfunc, uint32_t flags);
static void	conf_parser_destroy(conf_parser_t *parser, int err);
static int	conf_parser_file_create(conf_file_t **PFC_RESTRICT filep,
					pfc_refptr_t *PFC_RESTRICT rpath,
					const pfc_cfdef_t *PFC_RESTRICT defs);
static void	conf_parser_file_destroy(conf_file_t *cfp, pfc_bool_t do_free);
static int	conf_parser_do_block(conf_parser_t *PFC_RESTRICT parser,
				     conf_blkmap_t *PFC_RESTRICT blkmap,
				     pfc_refptr_t *PFC_RESTRICT name,
				     const pfc_cfdef_block_t *PFC_RESTRICT bdp);
static void	conf_parser_array_dtor(pfc_listm_t array);

static int	conf_parser_check_mandatory(conf_parser_t *PFC_RESTRICT parser,
					    conf_file_t *PFC_RESTRICT cfp,
					    const pfc_cfdef_block_t
					    *PFC_RESTRICT bdp);
static void	conf_parser_do_parse(conf_parser_t *parser, uint32_t flags);
static void	conf_parser_block(conf_parser_t *PFC_RESTRICT parser,
				  pfc_refptr_t *PFC_RESTRICT name,
				  const pfc_cfdef_block_t *PFC_RESTRICT bdp);
static void	conf_parser_map(conf_parser_t *PFC_RESTRICT parser,
				pfc_refptr_t *PFC_RESTRICT name,
				const pfc_cfdef_block_t *PFC_RESTRICT bdp);
static void	conf_parser_params(conf_parser_t *PFC_RESTRICT parser,
				   conf_pmap_t *PFC_RESTRICT pmap,
				   const pfc_cfdef_block_t *PFC_RESTRICT bdp);
static void	conf_parser_value(conf_parser_t *PFC_RESTRICT parser,
				  conf_pmap_t *PFC_RESTRICT pmap,
				  const pfc_cfdef_param_t *PFC_RESTRICT pdp,
				  pfc_refptr_t *PFC_RESTRICT pname,
				  conf_token_t *PFC_RESTRICT token);

static void	conf_parser_parse_byte(conf_parser_t *PFC_RESTRICT parser,
				       conf_param_t *PFC_RESTRICT param,
				       conf_token_t *PFC_RESTRICT token);
static void	conf_parser_parse_string(conf_parser_t *PFC_RESTRICT parser,
					 conf_param_t *PFC_RESTRICT param,
					 conf_token_t *PFC_RESTRICT token);
static void	conf_parser_parse_bool(conf_parser_t *PFC_RESTRICT parser,
				       conf_param_t *PFC_RESTRICT param,
				       conf_token_t *PFC_RESTRICT token);
static void	conf_parser_parse_int32(conf_parser_t *PFC_RESTRICT parser,
					conf_param_t *PFC_RESTRICT param,
					conf_token_t *PFC_RESTRICT token);
static void	conf_parser_parse_uint32(conf_parser_t *PFC_RESTRICT parser,
					 conf_param_t *PFC_RESTRICT param,
					 conf_token_t *PFC_RESTRICT token);
static void	conf_parser_parse_int64(conf_parser_t *PFC_RESTRICT parser,
					conf_param_t *PFC_RESTRICT param,
					conf_token_t *PFC_RESTRICT token);
static void	conf_parser_parse_uint64(conf_parser_t *PFC_RESTRICT parser,
					 conf_param_t *PFC_RESTRICT param,
					 conf_token_t *PFC_RESTRICT token);

static pfc_ptr_t	conf_parser_to_byte_array(conf_parser_t *parser,
						  uint32_t size);
static pfc_ptr_t	conf_parser_to_string_array(conf_parser_t *parser,
						    uint32_t size);
static pfc_ptr_t	conf_parser_to_bool_array(conf_parser_t *parser,
						  uint32_t size);
static pfc_ptr_t	conf_parser_to_int32_array(conf_parser_t *parser,
						   uint32_t size);
static pfc_ptr_t	conf_parser_to_uint32_array(conf_parser_t *parser,
						    uint32_t size);
static pfc_ptr_t	conf_parser_to_int64_array(conf_parser_t *parser,
						   uint32_t size);
static pfc_ptr_t	conf_parser_to_uint64_array(conf_parser_t *parser,
						    uint32_t size);

static conf_param_t	*conf_parser_param_alloc(conf_parser_t *PFC_RESTRICT
						 parser,
						 const pfc_cfdef_param_t
						 *PFC_RESTRICT pdp);
static conf_pnode_t	*conf_parser_pnode_alloc(conf_parser_t *PFC_RESTRICT
						 parser,
						 const pfc_cfdef_param_t
						 *PFC_RESTRICT pdp);
static void	conf_parser_param_init(conf_param_t *PFC_RESTRICT param,
				       const pfc_cfdef_param_t *PFC_RESTRICT
				       pdp);

static void	conf_parser_token_error(conf_parser_t *PFC_RESTRICT parser,
					conf_token_t *PFC_RESTRICT token)
	PFC_FATTR_NORETURN;
static void	conf_parser_error_nojmp(pfc_conf_err_t errfunc,
					const char *PFC_RESTRICT fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3);
static void	conf_parser_verror_nojmp(pfc_conf_err_t errfunc,
					 const char *PFC_RESTRICT fmt,
					 va_list ap)
	PFC_FATTR_PRINTFLIKE(2, 0);

/*
 * Return value of conf_parser_check_mandatory().
 */
#define	CFP_MISSING_BLOCK		(-1)	/* missing mandatory block */
#define	CFP_MISSING_MAP			(-2)	/* missing mandatory map */

/*
 * Parser operation array.
 * pfc_cftype_t is used as array index.
 */

#define	VALUE_PARSER_DECL(type)						\
	{								\
		.parse		= conf_parser_parse_##type,		\
		.to_array	= conf_parser_to_##type##_array,	\
	}

static const value_parser_t	value_parsers[] = {
	VALUE_PARSER_DECL(byte),		/* PFC_CFTYPE_BYTE */
	VALUE_PARSER_DECL(string),		/* PFC_CFTYPE_STRING */
	VALUE_PARSER_DECL(bool),		/* PFC_CFTYPE_BOOL */
	VALUE_PARSER_DECL(int32),		/* PFC_CFTYPE_INT32 */
	VALUE_PARSER_DECL(uint32),		/* PFC_CFTYPE_UINT32 */
	VALUE_PARSER_DECL(int64),		/* PFC_CFTYPE_INT64 */
	VALUE_PARSER_DECL(uint64),		/* PFC_CFTYPE_UINT64 */
#ifdef	PFC_LP64
	/* On LP64 system, LONG and ULONG are treated as 64-bit integer. */
	VALUE_PARSER_DECL(int64),		/* PFC_CFTYPE_LONG */
	VALUE_PARSER_DECL(uint64),		/* PFC_CFTYPE_ULONG */
#else	/* !PFC_LP64 */
	/* On ILP32 system, LONG and ULONG are treated as 32-bit integer. */
	VALUE_PARSER_DECL(int32),		/* PFC_CFTYPE_LONG */
	VALUE_PARSER_DECL(uint32),		/* PFC_CFTYPE_ULONG */
#endif	/* PFC_LP64 */
};

#define	PFC_CFTYPE_ASSERT(type)						\
	PFC_ASSERT((uint32_t)(type) <					\
		   (uint32_t)PFC_ARRAY_CAPACITY(value_parsers))

/*
 * Release resources grabbed by the given token.
 * This function must be used on parse error.
 */
static inline void
conf_parser_token_release(conf_parser_t *PFC_RESTRICT parser,
			  conf_token_t *PFC_RESTRICT token)
{
	conf_token_type_t	type = token->cft_type;

	if (type == TOKEN_STRING || type == TOKEN_SYMBOL) {
		pfc_refptr_t	*rstr = token->cft_value.string;

		CONF_PARSER_REFPTR_SET(parser, rstr);
	}
}

/*
 * static inline int64_t
 * conf_parser_verify_signed(conf_parser_t *PFC_RESTRICT parser,
 *			     const pfc_cfdef_param_t *PFC_RESTRICT pdp,
 *			     conf_token_t *PFC_RESTRICT token)
 *	Ensure that the parameter value satisfies the constraints for
 *	signed integer parameter.
 *
 * Calling/Exit State:
 *	A signed 64-bit value in the specified token is returned.
 *	This function never returns if the value violates the constraints.
 */
static inline int64_t
conf_parser_verify_signed(conf_parser_t *PFC_RESTRICT parser,
			  const pfc_cfdef_param_t *PFC_RESTRICT pdp,
			  conf_token_t *PFC_RESTRICT token)
{
	int64_t	value;

	/* Ensure the token is integer token. */
	if (PFC_EXPECT_FALSE(token->cft_type != TOKEN_INT)) {
		conf_parser_token_error(parser, token);
		/* NOTREACHED */
	}

	/* If '-' is not specified, this number must be treated as positive. */
	value = (int64_t)token->cft_value.integer;
	if (PFC_EXPECT_FALSE(!token->cft_negative &&
			     CONF_INT64_IS_NEGATIVE(value))) {
		parser->cfp_errno = ERANGE;
		conf_parser_file_error(parser, "Integer overflow: 0x%"
				       PFC_PFMT_x64, value);
		/* NOTREACHED */
	}

	/* Check lower limit. */
	if (PFC_EXPECT_FALSE(value < (int64_t)pdp->cfdp_min)) {
		conf_parser_file_error(parser, "%s: %" PFC_PFMT_d64
				       ": Value must be more than or equal %"
				       PFC_PFMT_d64 ".",
				       pdp->cfdp_name, value,
				       (int64_t)pdp->cfdp_min);
		/* NOTREACHED */
	}

	/* Check upper limit. */
	if (PFC_EXPECT_FALSE(value > (int64_t)pdp->cfdp_max)) {
		conf_parser_file_error(parser, "%s: %" PFC_PFMT_d64
				       ": Value must be less than or equal %"
				       PFC_PFMT_d64 ".",
				       pdp->cfdp_name, value,
				       (int64_t)pdp->cfdp_max);
		/* NOTREACHED */
	}

	return value;
}

/*
 * static inline uint64_t
 * conf_parser_verify_unsigned(conf_parser_t *PFC_RESTRICT parser,
 *			       const pfc_cfdef_param_t *PFC_RESTRICT pdp,
 *			       conf_token_t *PFC_RESTRICT token)
 *	Ensure that the parameter value satisfies the constraints for
 *	unsigned integer parameter.
 *
 * Calling/Exit State:
 *	An unsigned 64-bit value in the specified token is returned.
 *	This function never returns if the value violates the constraints.
 */
static inline uint64_t
conf_parser_verify_unsigned(conf_parser_t *PFC_RESTRICT parser,
			    const pfc_cfdef_param_t *PFC_RESTRICT pdp,
			    conf_token_t *PFC_RESTRICT token)
{
	uint64_t	value;

	/* Ensure the token is integer token. */
	if (PFC_EXPECT_FALSE(token->cft_type != TOKEN_INT)) {
		conf_parser_token_error(parser, token);
		/* NOTREACHED */
	}

	/* Unsigned value must not be negative. */
	value = token->cft_value.integer;
	if (PFC_EXPECT_FALSE(token->cft_negative)) {
		conf_parser_file_error(parser,
				       "%s: Value must not be negative: %"
				       PFC_PFMT_d64,
				       pdp->cfdp_name, (int64_t)value);
		/* NOTREACHED */
	}

	/* Check lower limit. */
	if (PFC_EXPECT_FALSE(value < pdp->cfdp_min)) {
		conf_parser_file_error(parser, "%s: %" PFC_PFMT_u64
				       ": Value must be more than or equal %"
				       PFC_PFMT_u64 ".",
				       pdp->cfdp_name, value, pdp->cfdp_min);
		/* NOTREACHED */
	}

	/* Check upper limit. */
	if (PFC_EXPECT_FALSE(value > pdp->cfdp_max)) {
		conf_parser_file_error(parser, "%s: %" PFC_PFMT_u64
				       ": Value must be less than or equal %"
				       PFC_PFMT_u64 ".",
				       pdp->cfdp_name, value, pdp->cfdp_max);
		/* NOTREACHED */
	}

	return value;
}

/*
 * static inline pfc_conf_t PFC_FATTR_ALWAYS_INLINE
 * conf_sysconf(void)
 *	Return the configuration file handle associated with the PFC system
 *	configuration file.
 */
static inline pfc_conf_t PFC_FATTR_ALWAYS_INLINE
conf_sysconf(void)
{
	pfc_conf_t	sysconf;

	SYSCONF_RDLOCK();
	sysconf = pfcd_conf;
	SYSCONF_UNLOCK();

	return sysconf;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_conf_libfini(void)
 *	Do global finalization for PFC configuration file parser.
 */
void PFC_ATTR_HIDDEN
pfc_conf_libfini(void)
{
	pfc_conf_t	sysconf;

	/* Try to invalidate system configuration file. */
	if (SYSCONF_TRYWRLOCK() != 0) {
		return;
	}

	sysconf = pfcd_conf;
	pfcd_conf = PFC_CONF_INVALID;
	SYSCONF_UNLOCK();

	/* Try to free up system configuration file handle. */
	if (CONF_PARSER_TRYWRLOCK() != 0) {
		return;
	}

	if (sysconf != PFC_CONF_INVALID) {
		conf_file_t	*cfp = CONF_FILE_PTR(sysconf);

		conf_parser_file_destroy(cfp, PFC_TRUE);
	}

	/* Make all configuration blocks invisible. */
	conf_hdlmap_clear();

	CONF_PARSER_UNLOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_conf_fork_prepare(void)
 *	fork(2) handler which will be called just before fork(2) on parent
 *	process.
 */
void PFC_ATTR_HIDDEN
pfc_conf_fork_prepare(void)
{
	SYSCONF_WRLOCK();
	CONF_PARSER_WRLOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_conf_fork_parent(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on parent process.
 */
void PFC_ATTR_HIDDEN
pfc_conf_fork_parent(void)
{
	CONF_PARSER_UNLOCK();
	SYSCONF_UNLOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_conf_fork_child(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on parent process.
 */
void PFC_ATTR_HIDDEN
pfc_conf_fork_child(void)
{
	PFC_ASSERT_INT(pfc_rwlock_init(&sysconf_lock), 0);
	PFC_ASSERT_INT(pfc_rwlock_init(&conf_parser_lock), 0);
}

/*
 * int
 * pfc_conf_open(pfc_conf_t *PFC_RESTRICT confp, const char *PFC_RESTRICT path,
 *		 const pfc_cfdef_t *PFC_RESTRICT defs)
 *	Open the specified configuration file.
 *	The syntax of the configuration file is determined by the specified
 *	pfc_cfdef_t struct.
 *
 * Calling/Exit State:
 *	Upon successful completion, configuration file handle is set to
 *	`*confp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_conf_open(pfc_conf_t *PFC_RESTRICT confp, const char *PFC_RESTRICT path,
	      const pfc_cfdef_t *PFC_RESTRICT defs)
{
	/* Use default error handler. */
	return conf_open(confp, path, defs, conf_errfunc);
}

/*
 * int
 * pfc_conf_open_ex(pfc_conf_t *PFC_RESTRICT confp,
 *		    const char *PFC_RESTRICT path,
 *		    const pfc_cfdef_t *PFC_RESTRICT defs,
 *		    pfc_conf_err_t errfunc)
 *	Open the specified configuration file.
 *	The syntax of the configuration file is determined by the specified
 *	pfc_cfdef_t struct.
 *
 *	`errfunc' is called with specifying error message when the parser
 *	detects parse error. If NULL is specified to `errfunc', default error
 *	handler is used.
 *
 * Calling/Exit State:
 *	Upon successful completion, configuration file handle is set to
 *	`*confp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_conf_open_ex(pfc_conf_t *PFC_RESTRICT confp, const char *PFC_RESTRICT path,
		 const pfc_cfdef_t *PFC_RESTRICT defs, pfc_conf_err_t errfunc)
{
	if (errfunc == NULL) {
		errfunc = conf_errfunc;
	}

	return conf_open(confp, path, defs, errfunc);
}

/*
 * int
 * pfc_conf_refopen(pfc_conf_t *PFC_RESTRICT confp,
 *		    pfc_refptr_t *PFC_RESTRICT rpath,
 *		    const pfc_cfdef_t *PFC_RESTRICT defs)
 *	Open the specified configuration file.
 *	Configuration file path must be specified by a string refptr object.
 *	The syntax of the configuration file is determined by the specified
 *	pfc_cfdef_t struct.
 *
 * Calling/Exit State:
 *	Upon successful completion, configuration file handle is set to
 *	`*confp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_conf_refopen(pfc_conf_t *PFC_RESTRICT confp,
		 pfc_refptr_t *PFC_RESTRICT rpath,
		 const pfc_cfdef_t *PFC_RESTRICT defs)
{
	/* Use default error handler. */
	return conf_refopen(confp, rpath, defs, conf_errfunc);
}

/*
 * int
 * pfc_conf_refopen_ex(pfc_conf_t *PFC_RESTRICT confp,
 *		       pfc_refptr_t *PFC_RESTRICT rpath,
 *		       const pfc_cfdef_t *PFC_RESTRICT defs,
 *		       pfc_conf_err_t errfunc)
 *	Open the specified configuration file.
 *	Configuration file path must be specified by a string refptr object.
 *	The syntax of the configuration file is determined by the specified
 *	pfc_cfdef_t struct.
 *
 *	`errfunc' is called with specifying error message when the parser
 *	detects parse error. If NULL is specified to `errfunc', default error
 *	handler is used.
 *
 * Calling/Exit State:
 *	Upon successful completion, configuration file handle is set to
 *	`*confp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_conf_refopen_ex(pfc_conf_t *PFC_RESTRICT confp,
		    pfc_refptr_t *PFC_RESTRICT rpath,
		    const pfc_cfdef_t *PFC_RESTRICT defs,
		    pfc_conf_err_t errfunc)
{
	if (errfunc == NULL) {
		errfunc = conf_errfunc;
	}

	return conf_refopen(confp, rpath, defs, errfunc);
}

/*
 * int
 * pfc_conf_refopen2(pfc_conf_t *PFC_RESTRICT confp,
 *		     pfc_refptr_t *PFC_RESTRICT primary,
 *		     pfc_refptr_t *PFC_RESTRICT secondary,
 *		     const pfc_cfdef_t *PFC_RESTRICT defs)
 *	Open the specified configuration file.
 *
 *	This function takes two configuration file path by refptr string:
 *
 *	- If `primary' exists but `secondary' does not, only the file specified
 *	  by `primary' is loaded.
 *	- If `secondary' exists but `primary' does not, only the file specified
 *	  by `secondary' is loaded.
 *	- If both files exist, both files are loaded and merges configurations.
 *	  Configurations in the file specified by `primary' always precedes
 *	  the file specified by `secondary'.
 *	- If neither file exists, an empty configuration file handle is
 *	  created. So all pfc_conf_get_XXX() APIs will return default value.
 *	  In this case, mandatory parameter check is skipped.
 *
 * Calling/Exit State:
 *	Upon successful completion, configuration file handle is set to
 *	`*confp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_conf_refopen2(pfc_conf_t *PFC_RESTRICT confp,
		  pfc_refptr_t *PFC_RESTRICT primary,
		  pfc_refptr_t *PFC_RESTRICT secondary,
		  const pfc_cfdef_t *PFC_RESTRICT defs)
{
	int	err;

	/*
	 * This API doesn't allow to open PFC system configuration file.
	 */
	if (PFC_EXPECT_FALSE(defs == &pfcd_conf_defs)) {
		err = EPERM;
		goto error;
	}

	if (PFC_EXPECT_FALSE(primary == NULL || secondary == NULL)) {
		err = EINVAL;
		goto error;
	}

	/* Load existing primary and secondary file. */
	err = conf_do_open2(confp, primary, secondary, defs, conf_errfunc,
			    CONF_PF_EXPORT);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return 0;
	}

error:
	/*
	 * Set PFC_CONF_INVALID as handle to be returned, so that the caller
	 * can obtain default parameter value even on error.
	 */
	if (confp != NULL) {
		*confp = PFC_CONF_INVALID;
	}

	return err;
}

/*
 * int
 * pfc_conf_reload(pfc_conf_t conf)
 *	Reload configuration file.
 *
 *	If the secondary configuration file is configured, both primary and
 *	secondary files are reloaded.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *	On error, previous contents of configuration file handle are retained.
 *
 * Remarks:
 *	- If pfc_conf_reload() returns zero, all parameter block handles
 *	  returned by pfc_conf_get_block() and pfc_conf_get_map() are
 *	  discarded.
 *	- ENOENT error on file open is always ignored. If neither primary nor
 *	  secondary file exists, the configuration file handle becomes empty.
 *	  So all pfc_conf_get_XXX() APIs will return default value.
 */
int
pfc_conf_reload(pfc_conf_t conf)
{
	conf_file_t	*cfp, *newcfp;
	pfc_conf_t	newconf;
	pfc_list_t	*elem;
	pfc_refptr_t	*secondary;
	pfc_conf_err_t	errfunc = conf_errfunc;
	int		err;

	if (PFC_EXPECT_FALSE(conf == PFC_CONF_INVALID)) {
		return EINVAL;
	}

	cfp = CONF_FILE_PTR(conf);
	if (PFC_EXPECT_FALSE(cfp->cf_defs == &pfcd_conf_defs)) {
		/*
		 * This API doesn't allow to reload PFC system configuration
		 * file.
		 */
		return EPERM;
	}

	/*
	 * Create another configuration file handle without exporting new
	 * parameter blocks. They should be exported in atomic section.
	 */
	if ((secondary = cfp->cf_secondary) == NULL) {
		err = conf_do_open(&newconf, cfp->cf_path, cfp->cf_defs,
				   errfunc, CONF_PF_NO_ENOENT);
	}
	else {
		err = conf_do_open2(&newconf, cfp->cf_path, secondary,
				    cfp->cf_defs, errfunc, CONF_PF_NO_ENOENT);
	}
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	newcfp = CONF_FILE_PTR(newconf);
	CONF_PARSER_WRLOCK();

	/* Make all new parameter blocks visible. */
	PFC_LIST_FOREACH(&newcfp->cf_blklist, elem) {
		conf_block_t	*bp = CONF_LIST2BLOCK(elem);

		err = conf_hdlmap_put(bp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			conf_parser_file_destroy(newcfp, PFC_TRUE);
			CONF_PARSER_UNLOCK();
			conf_parser_error_nojmp(errfunc,
						"Failed to register block "
						"handle: %s",
						strerror(err));

			return err;
		}
	}

	/* Release all contents of file handle except file handle itself. */
	conf_parser_file_destroy(cfp, PFC_FALSE);

	/* Copy file handle. */
	conf_blkmap_copy(&cfp->cf_blocks, &newcfp->cf_blocks);
	conf_imap_copy(&cfp->cf_maps, &newcfp->cf_maps);
	pfc_list_move_all(&newcfp->cf_blklist, &cfp->cf_blklist);

	/* Copy file flags. */
	cfp->cf_flags = newcfp->cf_flags;

	PFC_ASSERT(cfp->cf_path == newcfp->cf_path);
	PFC_ASSERT(cfp->cf_secondary == newcfp->cf_secondary);
	PFC_ASSERT(cfp->cf_defs == newcfp->cf_defs);

	CONF_PARSER_UNLOCK();

	free(newcfp);

	return 0;
}

/*
 * void
 * pfc_conf_close(pfc_conf_t conf)
 *	Close the configuration file handle.
 *
 * Remarks:
 *	All resources grabbed by the specified handle, including a string
 *	parameter value, are released.
 *
 *	This function is not thread safe. So the caller must guarantee that
 *	no other thread uses the specified handle.
 */
void
pfc_conf_close(pfc_conf_t conf)
{
	if (PFC_EXPECT_TRUE(conf != PFC_CONF_INVALID)) {
		conf_file_t	*cfp = CONF_FILE_PTR(conf);

		/* PFC system configuration file must not be closed. */
		if (PFC_EXPECT_TRUE(cfp->cf_defs != &pfcd_conf_defs)) {
			CONF_PARSER_WRLOCK();
			conf_parser_file_destroy(cfp, PFC_TRUE);
			CONF_PARSER_UNLOCK();
		}
	}
}

/*
 * void
 * pfc_conf_error(const char *fmt, ...)
 *	Print error message using the default error message handler.
 *	Error message must be specified in printf(3) format.
 */
void
pfc_conf_error(const char *fmt, ...)
{
	pfc_conf_err_t	errfunc = conf_errfunc;
	va_list		ap;

	va_start(ap, fmt);
	conf_parser_verror_nojmp(errfunc, fmt, ap);
	va_end(ap);
}

/*
 * void
 * pfc_conf_verror(const char *fmt, va_list ap)
 *	Print error message using the default error message handler.
 *	Error message must be specified in vprintf(3) format.
 */
void
pfc_conf_verror(const char *fmt, va_list ap)
{
	pfc_conf_err_t	errfunc = conf_errfunc;

	conf_parser_verror_nojmp(errfunc, fmt, ap);
}

/*
 * const char *
 * pfc_conf_get_path(pfc_conf_t conf)
 *	Return path to configuration file associated with the configuration
 *	file handle specified by `conf'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to string which keeps
 *	configuration file path is returned.
 *
 *	NULL is returned on failure.
 *
 * Remarks:
 *	- The caller must ensure that the configuration file handle specified
 *	  by `conf' is never closed while this function is called.
 *
 *	- Returned string will be invalidated when the configuration file
 *	  handle specified by `conf' is closed.
 */
const char *
pfc_conf_get_path(pfc_conf_t conf)
{
	conf_file_t	*cfp;

	if (PFC_EXPECT_FALSE(conf == PFC_CONF_INVALID)) {
		return NULL;
	}

	cfp = CONF_FILE_PTR(conf);

	return pfc_refptr_string_value(cfp->cf_path);
}

/*
 * const char *
 * pfc_conf_get_secondary(pfc_conf_t conf)
 *	Return path to the secondary configuration file associated with the
 *	configuration file handle specified by `conf'.
 *
 * Calling/Exit State:
 *	If the secondary file is configured, a non-NULL pointer to string
 *	which keeps path to the secondary configuration file is returned.
 *
 *	NULL is returned if invalid configuration file handle is specified
 *	to `conf', or no secondary file is configured.
 *
 * Remarks:
 *	- The caller must ensure that the configuration file handle specified
 *	  by `conf' is never closed while this function is called.
 *
 *	- Returned string will be invalidated when the configuration file
 *	  handle specified by `conf' is closed.
 */
const char *
pfc_conf_get_secondary(pfc_conf_t conf)
{
	conf_file_t	*cfp;
	pfc_refptr_t	*secondary;

	if (PFC_EXPECT_FALSE(conf == PFC_CONF_INVALID)) {
		return NULL;
	}

	cfp = CONF_FILE_PTR(conf);
	secondary = cfp->cf_secondary;
	if (secondary == NULL) {
		return NULL;
	}

	return pfc_refptr_string_value(secondary);
}

/*
 * pfc_bool_t
 * pfc_conf_is_primary_loaded(pfc_conf_t conf)
 *	Determine whether the primary configuration associated with the given
 *	handle is loaded.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the primary file is loaded.
 *	PFC_FALSE is returned if an invalid handle is specified, or the
 *	primary file is not loaded because it does not exist.
 *
 * Remarks:
 *	The caller must ensure that the configuration file handle specified
 *	by `conf' is never closed while this function is called.
 */
pfc_bool_t
pfc_conf_is_primary_loaded(pfc_conf_t conf)
{
	if (PFC_EXPECT_TRUE(conf != PFC_CONF_INVALID)) {
		conf_file_t	*cfp = CONF_FILE_PTR(conf);

		return (cfp->cf_flags & CONF_FF_PRIMARY)
			? PFC_TRUE : PFC_FALSE;
	}

	return PFC_FALSE;
}

/*
 * pfc_bool_t
 * pfc_conf_is_secondary_loaded(pfc_conf_t conf)
 *	Determine whether the secondary configuration associated with the
 *	given handle is loaded.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the secondary file is loaded.
 *	PFC_FALSE is returned if an invalid handle is specified, or the
 *	secondary file is not loaded because it does not exist.
 *
 * Remarks:
 *	The caller must ensure that the configuration file handle specified
 *	by `conf' is never closed while this function is called.
 */
pfc_bool_t
pfc_conf_is_secondary_loaded(pfc_conf_t conf)
{
	if (PFC_EXPECT_TRUE(conf != PFC_CONF_INVALID)) {
		conf_file_t	*cfp = CONF_FILE_PTR(conf);

		return (cfp->cf_flags & CONF_FF_SECONDARY)
			? PFC_TRUE : PFC_FALSE;
	}

	return PFC_FALSE;
}

/*
 * void
 * pfc_conf_set_errfunc(pfc_conf_err_t func)
 *	Set default error handler for the configuration file parser.
 *	The specified function will be called on parser error.
 */
void
pfc_conf_set_errfunc(pfc_conf_err_t func)
{
	pfc_ptr_t	*addr = (pfc_ptr_t *)&conf_errfunc;
	pfc_ptr_t	cur, old;

	do {
		cur = *addr;
		old = pfc_atomic_cas_ptr(addr, (pfc_ptr_t)func, cur);
	} while (old != cur);
}

/*
 * int
 * pfc_sysconf_init(pfc_refptr_t *rpath)
 *	Initialize the PFC system configuration file API.
 *	`rpath' must be a string refptr object which contains PFC system
 *	configuration file path.
 *
 * Remarks:
 *	This function must be called only once at the bootstrap of PFC daemon.
 *	The daemon must be terminated if this function returns non-zero value.
 */
int
pfc_sysconf_init(pfc_refptr_t *rpath)
{
	int	err;

	SYSCONF_WRLOCK();

	if (PFC_EXPECT_FALSE(pfcd_conf != PFC_CONF_INVALID)) {
		SYSCONF_UNLOCK();

		return EEXIST;
	}

	/* Open configuration file. */
	err = conf_do_open(&pfcd_conf, rpath, &pfcd_conf_defs, conf_errfunc,
			   CONF_PF_EXPORT);
	SYSCONF_UNLOCK();

	return err;
}

/*
 * pfc_conf_t
 * pfc_sysconf_open(void)
 *	Open a configuration file handle associated with the PFC system
 *	configuration file.
 */
pfc_conf_t
pfc_sysconf_open(void)
{
	return conf_sysconf();
}

/*
 * const char *
 * pfc_sysconf_get_path(void)
 *	Return path to PFC system configuration file.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to string which keeps
 *	configuration file path is returned.
 *
 *	NULL is returned if pfc_sysconf_init() is not yet called.
 */
const char *
pfc_sysconf_get_path(void)
{
	return pfc_conf_get_path(conf_sysconf());
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_parser_error(conf_parser_t *PFC_RESTRICT parser,
 *		     const char *PFC_RESTRICT fmt, ...)
 *	Call parser error handler.
 *	This function takes an error message in vprintf(3) format, and passes
 *	it to the error handler.
 *
 * Calling/Exit State:
 *	This function never returns.
 */
void PFC_ATTR_HIDDEN
conf_parser_error(conf_parser_t *PFC_RESTRICT parser,
		  const char *PFC_RESTRICT fmt, ...)
{
	pfc_conf_err_t	errfunc = parser->cfp_errfunc;
	va_list		ap;

	va_start(ap, fmt);
	conf_parser_verror_nojmp(errfunc, fmt, ap);
	va_end(ap);

	/* Jump to the saved parser context. */
	longjmp(parser->cfp_env, 1);
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_parser_file_error(conf_parser_t *PFC_RESTRICT parser,
 *			  const char *PFC_RESTRICT fmt, ...)
 *	Call parser error handler.
 *	This function appends configuration file path and current line number
 *	to the specified error message.
 *
 * Calling/Exit State:
 *	This function never returns.
 */
void PFC_ATTR_HIDDEN
conf_parser_file_error(conf_parser_t *PFC_RESTRICT parser,
		       const char *PFC_RESTRICT fmt, ...)
{
	int	n;
	va_list	ap;
	char	*buf = (char *)parser->cfp_buffer;
	size_t	remains = CONF_PARSER_BUFSIZE;

	/* Append file path and current line number. */
	n = snprintf(buf, remains, "%s:%u: ",
		     parser->cfp_path, parser->cfp_line);
	PFC_ASSERT(n > 0);
	if (PFC_EXPECT_TRUE((size_t)n < remains)) {
		buf += n;
		remains -= n;

		/* Append error message. */
		va_start(ap, fmt);
		(void)vsnprintf(buf, remains, fmt, ap);
		va_end(ap);
	}
	*(buf + remains - 1) = '\0';

	conf_parser_error(parser, "%s", parser->cfp_buffer);
	/* NOTREACHED */
}

/*
 * static int
 * conf_do_open(pfc_conf_t *PFC_RESTRICT confp,
 *		pfc_refptr_t *PFC_RESTRICT rpath,
 *		const pfc_cfdef_t *PFC_RESTRICT defs, pfc_conf_err_t errfunc,
 *		uint32_t flags)
 *	Open the specified configuration file.
 *
 *	Zero or more of the following flag bits can be specified to `flags':
 *
 *	CONF_PF_EXPORT
 *	    Parsed configuration blocks are exported to the global block
 *	    handle map.
 *	CONF_PF_NO_ENOENT
 *	    Ignore ENOENT error on file open.
 *	CONF_PF_NO_MANDATORY
 *	    Don't check whether all blocks which contains mandatory parameter
 *	    are defined.
 */
static int
conf_do_open(pfc_conf_t *PFC_RESTRICT confp,
	     pfc_refptr_t *PFC_RESTRICT rpath,
	     const pfc_cfdef_t *PFC_RESTRICT defs, pfc_conf_err_t errfunc,
	     uint32_t flags)
{
	conf_parser_t	*parser = NULL;
	int	err;

	PFC_ASSERT(rpath != NULL);
	if (PFC_EXPECT_FALSE(confp == NULL || defs == NULL)) {
		/* This should not happen. */
		conf_parser_error_nojmp(errfunc,
					"conf_open: Invalid parameter.");

		return EINVAL;
	}

	/* Create a configuration file parser context. */
	err = conf_parser_create(&parser, rpath, defs, errfunc, flags);
	if (PFC_EXPECT_FALSE(err != 0)) {
		conf_parser_error_nojmp(errfunc,
					"%s: Failed to create parser context"
					": %s",
					pfc_refptr_string_value(rpath),
					strerror(err));

		return err;
	}
	PFC_ASSERT(parser != NULL);

	/*
	 * Save current context.
	 * The configuration parser will jump back here on error.
	 */
	if (setjmp(parser->cfp_env) != 0) {
		err = parser->cfp_errno;
		if (err == 0) {
			/* This should be a syntax error in the file. */
			err = EINVAL;
		}
		conf_parser_destroy(parser, err);

		return err;
	}

	/* Parse configuration file. */
	conf_parser_do_parse(parser, flags);

	/* Destroy parser context except file handle. */
	*confp = (pfc_conf_t)parser->cfp_value;
	conf_parser_destroy(parser, 0);

	return 0;
}

/*
 * static int
 * conf_do_open2(pfc_conf_t *PFC_RESTRICT confp,
 *		 pfc_refptr_t *PFC_RESTRICT primary,
 *		 pfc_refptr_t *PFC_RESTRICT secondary,
 *		 const pfc_cfdef_t *PFC_RESTRICT defs, pfc_conf_err_t errfunc,
 *		 uint32_t flags)
 *	Open the primary and secondary configuration file.
 *
 * Remarks:
 *	This function always ignores ENOENT error on file open.
 */
static int
conf_do_open2(pfc_conf_t *PFC_RESTRICT confp,
	      pfc_refptr_t *PFC_RESTRICT primary,
	      pfc_refptr_t *PFC_RESTRICT secondary,
	      const pfc_cfdef_t *PFC_RESTRICT defs, pfc_conf_err_t errfunc,
	      uint32_t flags)
{
	const pfc_cfdef_block_t	*bdp;
	pfc_conf_t	conf_s;
	pfc_list_t	*elem, *next;
	conf_file_t	*cfp, *cfp_s;
	uint32_t	nblocks;
	int		err;

	/* Load primary file if it exists. */
	err = conf_do_open(confp, primary, defs, errfunc,
			   CONF_PF_NO_ENOENT | CONF_PF_NO_MANDATORY);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	cfp = CONF_FILE_PTR(*confp);

	/* Load secondary file if it exists. */
	err = conf_do_open(&conf_s, secondary, defs, errfunc,
			   CONF_PF_NO_ENOENT | CONF_PF_NO_MANDATORY);
	CONF_PARSER_WRLOCK();

	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}
	cfp_s = CONF_FILE_PTR(conf_s);

	/* Merge parameters in secondary to primary. */
	conf_blkmap_merge(&cfp->cf_blocks, &cfp_s->cf_blocks);
	err = conf_imap_merge(&cfp->cf_maps, &cfp_s->cf_maps);
	if (PFC_EXPECT_FALSE(err != 0)) {
		conf_parser_file_destroy(cfp_s, PFC_TRUE);
		conf_parser_error_nojmp(errfunc, "Failed to merge map: %s",
					strerror(err));
		goto error;
	}

	PFC_LIST_FOREACH_SAFE(&cfp_s->cf_blklist, elem, next) {
		pfc_list_remove(elem);
		pfc_list_push_tail(&cfp->cf_blklist, elem);
	}

	/* Install secondary file path to the file handle. */
	cfp->cf_secondary = secondary;
	pfc_refptr_get(secondary);

	/* Set secondary file flag if secondary file exists. */
	if (cfp_s->cf_flags & CONF_FF_PRIMARY) {
		cfp->cf_flags |= CONF_FF_SECONDARY;
	}

	/* Secondary file handle is no longer used. */
	conf_parser_file_destroy(cfp_s, PFC_TRUE);

	/*
	 * Ensure all parameter blocks which contains at least one mandatory
	 * parameter are defined. If neither primary nor secondary file exists,
	 * this test needs to be skipped.
	 */
	nblocks = (cfp->cf_flags & CONF_FF_FILEMASK) ? defs->cfd_nblocks : 0;

	for (bdp = defs->cfd_block; bdp < defs->cfd_block + nblocks; bdp++) {
		int		result;

		result = conf_parser_check_mandatory(NULL, cfp, bdp);
		if (PFC_EXPECT_FALSE(result != 0)) {
			const char	*errfile, *type;

			if (cfp->cf_flags & CONF_FF_PRIMARY) {
				errfile = pfc_refptr_string_value(primary);
			}
			else {
				PFC_ASSERT(cfp->cf_flags & CONF_FF_SECONDARY);
				errfile = pfc_refptr_string_value(secondary);
			}

			type = (result == CFP_MISSING_MAP) ? "map" : "block";
			conf_parser_error_nojmp(errfunc,
						"%s: Missing mandatory %s "
						"\"%s\".", errfile, type,
						bdp->cfdb_name);
			err = EINVAL;
			goto error;
		}
	}

	if (flags & CONF_PF_EXPORT) {
		/* Export all block handles. */
		PFC_LIST_FOREACH(&cfp->cf_blklist, elem) {
			conf_block_t	*bp = CONF_LIST2BLOCK(elem);

			err = conf_hdlmap_put(bp);
			if (PFC_EXPECT_FALSE(err != 0)) {
				conf_parser_error_nojmp(errfunc,
							"Failed to register "
							"block handle: %s",
							strerror(err));
				goto error;
			}
		}
	}

	CONF_PARSER_UNLOCK();

	return 0;

error:
	conf_parser_file_destroy(cfp, PFC_TRUE);
	CONF_PARSER_UNLOCK();

	return err;
}

/*
 * static int
 * conf_open(pfc_conf_t *PFC_RESTRICT confp, const char *PFC_RESTRICT path,
 *	     const pfc_cfdef_t *PFC_RESTRICT defs, pfc_conf_err_t errfunc)
 *	Open the specified configuration file.
 *	This function implements common part of pfc_conf_open() and
 *	pfc_conf_open_ex().
 *
 * Calling/Exit State:
 *	Upon successful completion, configuration file handle is set to
 *	`*confp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
conf_open(pfc_conf_t *PFC_RESTRICT confp, const char *PFC_RESTRICT path,
	  const pfc_cfdef_t *PFC_RESTRICT defs, pfc_conf_err_t errfunc)
{
	pfc_refptr_t	*rpath;
	int	err;

	/*
	 * This API doesn't allow to open PFC system configuration file.
	 */
	if (PFC_EXPECT_FALSE(defs == &pfcd_conf_defs)) {
		err = EPERM;
		goto error;
	}

	if (PFC_EXPECT_FALSE(path == NULL)) {
		err = EINVAL;
		goto error;
	}

	rpath = pfc_refptr_string_create(path);
	if (PFC_EXPECT_FALSE(rpath == NULL)) {
		err = ENOMEM;
		goto error;
	}

	err = conf_do_open(confp, rpath, defs, errfunc, CONF_PF_EXPORT);
	pfc_refptr_put(rpath);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return 0;
	}

error:
	/*
	 * Set PFC_CONF_INVALID as handle to be returned, so that the caller
	 * can obtain default parameter value even on error.
	 */
	if (confp != NULL) {
		*confp = PFC_CONF_INVALID;
	}

	return err;
}

/*
 * static int
 * conf_refopen(pfc_conf_t *PFC_RESTRICT confp,
 *	 	pfc_refptr_t *PFC_RESTRICT rpath,
 *		const pfc_cfdef_t *PFC_RESTRICT defs, pfc_conf_err_t errfunc)
 *	Open the specified configuration file.
 *	This function implements common part of pfc_conf_refopen() and
 *	pfc_conf_refopen_ex().
 *
 * Calling/Exit State:
 *	Upon successful completion, configuration file handle is set to
 *	`*confp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
conf_refopen(pfc_conf_t *PFC_RESTRICT confp, pfc_refptr_t *PFC_RESTRICT rpath,
	     const pfc_cfdef_t *PFC_RESTRICT defs, pfc_conf_err_t errfunc)
{
	int	err;

	/*
	 * This API doesn't allow to open PFC system configuration file.
	 */
	if (PFC_EXPECT_FALSE(defs == &pfcd_conf_defs)) {
		err = EPERM;
		goto error;
	}

	if (PFC_EXPECT_FALSE(rpath == NULL)) {
		err = EINVAL;
		goto error;
	}

	err = conf_do_open(confp, rpath, defs, errfunc, CONF_PF_EXPORT);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return 0;
	}

error:
	/*
	 * Set PFC_CONF_INVALID as handle to be returned, so that the caller
	 * can obtain default parameter value even on error.
	 */
	if (confp != NULL) {
		*confp = PFC_CONF_INVALID;
	}

	return err;

}

/*
 * static int
 * conf_parser_create(conf_parser_t **PFC_RESTRICT parserp,
 *		      pfc_refptr_t *PFC_RESTRICT rpath,
 *		      const pfc_cfdef_t *PFC_RESTRICT defs,
 *		      pfc_conf_err_t errfunc, uint32_t flags)
 *	Create a configuration file parser context.
 */
static int
conf_parser_create(conf_parser_t **PFC_RESTRICT parserp,
		   pfc_refptr_t *PFC_RESTRICT rpath,
		   const pfc_cfdef_t *PFC_RESTRICT defs,
		   pfc_conf_err_t errfunc, uint32_t flags)
{
	const pfc_cfdef_block_t	*bdp;
	conf_parser_t	*parser;
	int	err;

	/* Allocate a parser context. */
	parser = (conf_parser_t *)malloc(sizeof(*parser));
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		return ENOMEM;
	}

	/* Create a configuration file handle. */
	err = conf_parser_file_create(&parser->cfp_value, rpath, defs);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/*
	 * Prepare containers used by the parser.
	 * Any internal lock is not needed because these containers are private
	 * for the parser.
	 */
	conf_ctmap_init(&parser->cfp_blocks);
	conf_ctmap_init(&parser->cfp_maps);
	conf_ctmap_init(&parser->cfp_params);
	conf_ctmap_init(&parser->cfp_mandatory);

	/* Copy block and map definitions into internal map. */
	for (bdp = defs->cfd_block;
	     bdp < defs->cfd_block + defs->cfd_nblocks; bdp++) {
		conf_ctmap_t	*ctmap;

		if (bdp->cfdb_flags & PFC_CFBF_MAP) {
			ctmap = &parser->cfp_maps;
		}
		else {
			ctmap = &parser->cfp_blocks;
		}

		err = conf_ctmap_put(ctmap, bdp->cfdb_name, (pfc_cptr_t)bdp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error_file;
		}
	}

	/* Create vector which keeps array elements. */
	err = PFC_VECTOR_CREATE(&parser->cfp_array);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_file;
	}

	parser->cfp_path = pfc_refptr_string_value(rpath);
	parser->cfp_errno = 0;
	parser->cfp_errfunc = errfunc;
	parser->cfp_file = NULL;
	parser->cfp_refptr = NULL;
	parser->cfp_flags = flags;

	*parserp = parser;

	return 0;

error_file:
	conf_ctmap_clear(&parser->cfp_mandatory);
	conf_ctmap_clear(&parser->cfp_params);
	conf_ctmap_clear(&parser->cfp_maps);
	conf_ctmap_clear(&parser->cfp_blocks);

	CONF_PARSER_WRLOCK();
	conf_parser_file_destroy(parser->cfp_value, PFC_TRUE);
	CONF_PARSER_UNLOCK();

error:
	free(parser);

	return err;
}

/*
 * static void
 * conf_parser_destroy(conf_parser_t *parser, int err)
 *	Destroy the parser context.
 *	If `err' is not zero, configuration file handle in the parser context
 *	is destroyed.
 */
static void
conf_parser_destroy(conf_parser_t *parser, int err)
{
	pfc_listm_t	array = parser->cfp_array;

	/* Close lexer. */
	conf_lexer_close(parser);

	/* Destroy internal maps. */
	conf_ctmap_clear(&parser->cfp_mandatory);
	conf_ctmap_clear(&parser->cfp_params);
	conf_ctmap_clear(&parser->cfp_maps);
	conf_ctmap_clear(&parser->cfp_blocks);

	/* Clean up temporary array elements. */
	conf_parser_array_dtor(array);

	/* Release temporary reference pointer. */
	if (parser->cfp_refptr != NULL) {
		pfc_refptr_put(parser->cfp_refptr);
	}

	if (PFC_EXPECT_FALSE(err != 0)) {
		/* Destroy configuration file handle. */
		CONF_PARSER_WRLOCK();
		conf_parser_file_destroy(parser->cfp_value, PFC_TRUE);
		CONF_PARSER_UNLOCK();
	}

	free(parser);
}

/*
 * static int
 * conf_parser_file_create(conf_file_t **PFC_RESTRICT filep,
 *			   pfc_refptr_t *PFC_RESTRICT rpath,
 *			   const pfc_cfdef_t *PFC_RESTRICT defs)
 *	Create a configuration file handle.
 */
static int
conf_parser_file_create(conf_file_t **PFC_RESTRICT filep,
			pfc_refptr_t *PFC_RESTRICT rpath,
			const pfc_cfdef_t *PFC_RESTRICT defs)
{
	conf_file_t	*cfp;

	/* Allocate a configuration file handle. */
	cfp = (conf_file_t *)malloc(sizeof(*cfp));
	if (PFC_EXPECT_FALSE(cfp == NULL)) {
		return ENOMEM;
	}

	/*
	 * Initialize internal maps to keep pairs of block name and parameters.
	 * Any internal lock is not needed because these maps are immutable
	 * for public.
	 */
	conf_blkmap_init(&cfp->cf_blocks);
	conf_imap_init(&cfp->cf_maps);

	pfc_refptr_get(rpath);
	cfp->cf_path = rpath;
	cfp->cf_secondary = NULL;
	cfp->cf_defs = defs;
	cfp->cf_flags = 0;
	pfc_list_init(&cfp->cf_blklist);

	*filep = cfp;

	return 0;
}

/*
 * static void
 * conf_parser_file_destroy(conf_file_t *cfp, pfc_bool_t do_free)
 *	Destroy the specified configuration file handle.
 *	If `do_free' is PFC_FALSE, conf_file_t itself is retained.
 *
 * Remarks:
 *	The caller must call this function with holding parser lock in
 *	writer mode.
 */
static void
conf_parser_file_destroy(conf_file_t *cfp, pfc_bool_t do_free)
{
	pfc_list_t	*elem;

	/*
	 * Unregister block handles.
	 * All configuration block instances in the global handle map are
	 * also registered in cfp->cf_blocks and cfp->cf_maps, and they are
	 * freed by the call of conf_blkmap_clear() and conf_imap_clear().
	 * So conf_hdlmap_remove() must be called before them.
	 */
	PFC_LIST_FOREACH(&cfp->cf_blklist, elem) {
		conf_block_t	*bp = CONF_LIST2BLOCK(elem);

		conf_hdlmap_remove(bp);
	}

	conf_blkmap_clear(&cfp->cf_blocks);
	conf_imap_clear(&cfp->cf_maps);

	pfc_refptr_put(cfp->cf_path);
	if (cfp->cf_secondary != NULL) {
		pfc_refptr_put(cfp->cf_secondary);
	}

	if (do_free) {
		free(cfp);
	}
}

/*
 * static conf_block_t *
 * conf_parser_do_block(conf_parser_t *PFC_RESTRICT parser,
 *			conf_blkmap_t *PFC_RESTRICT blkmap,
 *			pfc_refptr_t *PFC_RESTRICT name)
 *	Parse one parameter block.
 *	This function allocates a new block handle, and register it into
 *	the specified block map.
 *	`name' must be a pointer to pfc_refptr_t which keeps block name or
 *	map key.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function decrements the reference counter of `name' on successful
 *	return.
 */
static int
conf_parser_do_block(conf_parser_t *PFC_RESTRICT parser,
		     conf_blkmap_t *PFC_RESTRICT blkmap,
		     pfc_refptr_t *PFC_RESTRICT name,
		     const pfc_cfdef_block_t *PFC_RESTRICT bdp)
{
	conf_file_t	*cfp = parser->cfp_value;
	conf_block_t	*bp;
	int		err;

	/* Allocate a block handle instance, and register it into the map. */
	err = conf_blkmap_alloc(blkmap, name, &bp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		parser->cfp_errno = err;

		return err;
	}

	/* Link this handle to the block handle list. */
	pfc_list_push_tail(&cfp->cf_blklist, &bp->cb_list);

	/* Parse parameters. */
	conf_parser_params(parser, &bp->cb_params, bdp);

	return 0;
}

/*
 * static void
 * conf_parser_array_dtor(pfc_listm_t array)
 *	Free up resources kept by parser->cfp_array and list itself.
 */
static void
conf_parser_array_dtor(pfc_listm_t array)
{
	int		i, size;

	size = pfc_listm_get_size(array);
	for (i = 0; i < size; i++) {
		conf_param_t	*param;

		PFC_ASSERT_INT(pfc_listm_getat(array, i,
					       (pfc_cptr_t *)&param), 0);
		PFC_ASSERT(param != NULL);
		PFC_ASSERT(param->cp_def != NULL);

		/*
		 * parser->cfp_array must contains only scalar value.
		 * So we must NOT treat this parameter as array value.
		 */
		if (param->cp_def->cfdp_type == PFC_CFTYPE_STRING) {
			pfc_refptr_t	*str = CONF_VALUE_STRING(param);

			if (str != NULL) {
				pfc_refptr_put(str);
			}
		}
		free(param);
	}
	pfc_listm_destroy(array);
}

/*
 * static int
 * conf_parser_check_mandatory(conf_parser_t *PFC_RESTRICT parser,
 *			       conf_file_t *PFC_RESTRICT cfp,
 *			       const pfc_cfdef_block_t *PFC_RESTRICT bdp)
 *	Validate parameter block which contains mandatory parameter.
 *	If `bdp' represents a parameter block which contains mandatory
 *	parameter, at least one parameter block must be defined.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	If a non-NULL value is specified to `parse', a parser exception is
 *	caused on error.
 *
 *	If NULL is specified to `parse', CFP_MISSING_BLOCK is returned if
 *	at least one block with mandatory parameter is missing.
 *	CFP_MISSING_MAP is returned if at least one map block with mandatory
 *	parameter is missing.
 */
static int
conf_parser_check_mandatory(conf_parser_t *PFC_RESTRICT parser,
			    conf_file_t *PFC_RESTRICT cfp,
			    const pfc_cfdef_block_t *PFC_RESTRICT bdp)
{
	const char	*name, *label;
	int		ret = 0;

	if ((bdp->cfdb_flags & PFC_CFBF_MANDATORY) == 0) {
		return 0;
	}

	name = bdp->cfdb_name;
	if (bdp->cfdb_flags & PFC_CFBF_MAP) {
		conf_blkmap_t	*blkmap;

		blkmap = conf_imap_getmap(&cfp->cf_maps, name);
		if (PFC_EXPECT_TRUE(blkmap != NULL)) {
			return 0;
		}
		label = "map";
		ret = CFP_MISSING_MAP;
	}
	else {
		conf_block_t	*bp;

		bp = conf_blkmap_get(&cfp->cf_blocks, name);
		if (PFC_EXPECT_TRUE(bp != NULL)) {
			return 0;
		}
		label = "block";
		ret = CFP_MISSING_BLOCK;
	}

	if (parser != NULL) {
		conf_parser_file_error(parser, "Missing mandatory %s \"%s\".",
				       label, name);
		/* NOTREACHED */
	}

	return ret;
}

/*
 * static void
 * conf_parser_do_parse(conf_parser_t *parser, uint32_t flags)
 *	Parse the configuration file.
 */
static void
conf_parser_do_parse(conf_parser_t *parser, uint32_t flags)
{
	conf_token_t	token;
	conf_file_t	*cfp = parser->cfp_value;
	pfc_list_t	*elem;
	int		err;
	const pfc_cfdef_t	*defs = cfp->cf_defs;

	/* Open lexical analyzer. */
	conf_lexer_open(parser);

	while (conf_lexer_next(parser, &token, PFC_FALSE)) {
		pfc_refptr_t	*rsym;
		const char	*ssym;
		conf_ctvalue_t	ctvalue;

		if (PFC_EXPECT_FALSE(token.cft_type != TOKEN_SYMBOL)) {
			conf_parser_token_error(parser, &token);
			/* NOTREACHED */
		}

		rsym = token.cft_value.string;
		ssym = pfc_refptr_string_value(rsym);

		/* Determine configuration block type. */
		err = conf_ctmap_get(&parser->cfp_blocks, ssym, &ctvalue);
		if (err == 0) {
			/* Parse block. */
			conf_parser_block(parser, rsym, ctvalue.ccv_block);
			continue;
		}

		err = conf_ctmap_get(&parser->cfp_maps, ssym, &ctvalue);
		if (PFC_EXPECT_FALSE(err != 0)) {
			conf_parser_token_error(parser, &token);
			/* NOTREACHED */
		}

		/* Parse map. */
		conf_parser_map(parser, rsym, ctvalue.ccv_block);
	}

	/*
	 * Ensure that all blocks which contain at least one mandatory
	 * parameter are defined.
	 */
	if ((flags & CONF_PF_NO_MANDATORY) == 0) {
		const pfc_cfdef_block_t	*bdp;
		conf_file_t	*cfp = parser->cfp_value;

		for (bdp = defs->cfd_block;
		     bdp < defs->cfd_block + defs->cfd_nblocks; bdp++) {
			(void)conf_parser_check_mandatory(parser, cfp, bdp);
		}
	}

	if ((parser->cfp_flags & CONF_PF_EXPORT) == 0) {
		/* The caller does not want to export parameter blocks. */
		return;
	}

	/* Register valid block handles to conf_blocks. */
	CONF_PARSER_WRLOCK();
	PFC_LIST_FOREACH(&cfp->cf_blklist, elem) {
		conf_block_t	*bp = CONF_LIST2BLOCK(elem);

		err = conf_hdlmap_put(bp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			CONF_PARSER_UNLOCK();
			parser->cfp_errno = err;
			conf_parser_error(parser,
					  "Failed to register block handle: "
					  "%s", strerror(err));
			/* NOTREACHED */
		}
	}
	CONF_PARSER_UNLOCK();
}

/*
 * static void
 * conf_parser_block(conf_parser_t *PFC_RESTRICT parser,
 *		     pfc_refptr_t *PFC_RESTRICT name,
 *		     const pfc_cfdef_block_t *PFC_RESTRICT bdp)
 *	Parse one parameter block.
 *
 * Remarks:
 *	This function always decrements the reference counter of `name'
 */
static void
conf_parser_block(conf_parser_t *PFC_RESTRICT parser,
		  pfc_refptr_t *PFC_RESTRICT name,
		  const pfc_cfdef_block_t *PFC_RESTRICT bdp)
{
	conf_file_t	*cfp = parser->cfp_value;
	int		err;

	/* conf_parser_do_block() will do actual work. */
	err = conf_parser_do_block(parser, &cfp->cf_blocks, name, bdp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* cfp_errno must be set by conf_parser_do_block(). */
		CONF_PARSER_REFPTR_SET(parser, name);
		if (err == EEXIST) {
			conf_parser_file_error
				(parser, "Block \"%s\" is already defined.",
				 pfc_refptr_string_value(name));
		}
		else {
			conf_parser_error(parser,
					  "Failed to register parameter block"
					  ": %s", strerror(err));
		}
		/* NOTREACHED */
	}
}

/*
 * static void
 * conf_parser_map(conf_parser_t *PFC_RESTRICT parser,
 *		   pfc_refptr_t *PFC_RESTRICT name,
 *		   const pfc_cfdef_block_t *PFC_RESTRICT bdp)
 *	Parse one parameter map.
 *
 * Remarks:
 *	This function always decrements the reference counter of `name'
 */
static void
conf_parser_map(conf_parser_t *PFC_RESTRICT parser,
		pfc_refptr_t *PFC_RESTRICT name,
		const pfc_cfdef_block_t *PFC_RESTRICT bdp)
{
	conf_token_t	token;
	conf_file_t	*cfp = parser->cfp_value;
	conf_imap_t	*maps = &cfp->cf_maps;
	conf_blkmap_t	*blkmap;
	pfc_refptr_t	*key;
	int	err;

	/* Parse key of this map entry. */
	conf_lexer_next(parser, &token, PFC_TRUE);
	if (PFC_EXPECT_FALSE(token.cft_type != TOKEN_STRING &&
			     token.cft_type != TOKEN_SYMBOL)) {
		conf_parser_token_error(parser, &token);
		/* NOTREACHED */
	}
	key = token.cft_value.string;

	/*
	 * Prepare internal block map for the parameter map specified by
	 * `name'. Reference counter of `name' will be decremented by
	 * conf_imap_alloc().
	 */
	err = conf_imap_alloc(maps, name, key, &blkmap);
	if (PFC_EXPECT_FALSE(err != 0)) {
		parser->cfp_errno = err;
		conf_parser_error(parser, "Failed to register internal map "
				  "for parameter map: %s", strerror(err));
		/* NOTREACHED */
	}

	/* conf_parser_do_block() will do the rest of work. */
	err = conf_parser_do_block(parser, blkmap, key, bdp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* cfp_errno must be set by conf_parser_do_block(). */
		CONF_PARSER_REFPTR_SET(parser, key);
		if (err == EEXIST) {
			conf_parser_file_error
				(parser, "Map key \"%s\" is already defined.",
				 pfc_refptr_string_value(key));
		}
		else {
			conf_parser_error(parser,
					  "Failed to register map entry: %s",
					  strerror(err));
		}
		/* NOTREACHED */
	}
}

/*
 * static void
 * conf_parser_params(conf_parser_t *PFC_RESTRICT parser,
 *		      conf_pmap_t *PFC_RESTRICT pmap,
 *		      const pfc_cfdef_block_t *PFC_RESTRICT bdp)
 *	Parse all parameters in the block.
 */
static void
conf_parser_params(conf_parser_t *PFC_RESTRICT parser,
		   conf_pmap_t *PFC_RESTRICT pmap,
		   const pfc_cfdef_block_t *PFC_RESTRICT bdp)
{
	const pfc_cfdef_param_t	*pdp;
	conf_param_state_t	state;
	conf_token_t	token;
	conf_ctmap_t	*pdefs = &parser->cfp_params;
	conf_ctmap_t	*mandatory = &parser->cfp_mandatory;
	pfc_refptr_t	*pname;
	const char	*string;
	int	err;

	/* Clear previous configurations. */
	conf_ctmap_clear(pdefs);
	conf_ctmap_clear(mandatory);

	/* Copy parameter configurations. */
	for (pdp = bdp->cfdb_params;
	     pdp < bdp->cfdb_params + bdp->cfdb_nparams; pdp++) {
		string = pdp->cfdp_name;
		err = conf_ctmap_put(pdefs, string, (pfc_cptr_t)pdp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			parser->cfp_errno = err;
			conf_parser_error(parser,
					  "Failed to setup parameter: %s",
					  strerror(err));
			/* NOTREACHED */
		}
		if (pdp->cfdp_flags & PFC_CFPF_MANDATORY) {
			err = conf_ctmap_put(mandatory, string, NULL);
			if (PFC_EXPECT_FALSE(err != 0)) {
				parser->cfp_errno = err;
				conf_parser_error(parser,
						  "Failed to setup mandatory "
						  "parameter: %s",
						  strerror(err));
				/* NOTREACHED */
			}
		}
	}

	/* Next token must be '{'. */
	conf_lexer_next(parser, &token, PFC_TRUE);
	if (PFC_EXPECT_FALSE(token.cft_type != TOKEN_LBRACE)) {
		conf_parser_token_error(parser, &token);
		/* NOTREACHED */
	}

	state = CONF_PSTATE_NAME;
	pname = NULL;

	while (1) {
		conf_token_type_t	type;

		/* Read next token. */
		conf_lexer_next(parser, &token, PFC_TRUE);
		type = token.cft_type;

		if (state == CONF_PSTATE_NAME) {
			conf_ctvalue_t	ctvalue;

			if (type == TOKEN_RBRACE) {
				/* Block is closed. */
				break;
			}
			if (PFC_EXPECT_FALSE(type != TOKEN_SYMBOL)) {
				conf_parser_token_error(parser, &token);
				/* NOTREACHED */
			}

			pname = token.cft_value.string;
			string = pfc_refptr_string_value(pname);
			state = CONF_PSTATE_NAME_END;

			/* Obtain definition for this parameter. */
			err = conf_ctmap_get(pdefs, string, &ctvalue);
			if (PFC_EXPECT_FALSE(err != 0)) {
				PFC_ASSERT(err == ENOENT);
				parser->cfp_errno = EINVAL;
				CONF_PARSER_REFPTR_SET(parser, pname);
				conf_parser_file_error
					(parser,
					 "Unknown parameter name: %s", string);
				/* NOTREACHED */
			}
			pdp = ctvalue.ccv_param;
		}
		else if (state == CONF_PSTATE_NAME_END) {
			if (PFC_EXPECT_FALSE(type != TOKEN_EQUAL)) {
				pfc_refptr_put(pname);
				conf_parser_token_error(parser, &token);
				/* NOTREACHED */
			}
			state = CONF_PSTATE_VALUE;
		}
		else if (state == CONF_PSTATE_VALUE) {
			/*
			 * Parse parameter value.
			 * Note that conf_parser_value() always decrements
			 * references to pname.
			 */
			conf_parser_value(parser, pmap, pdp, pname, &token);
			state = CONF_PSTATE_VALUE_END;
		}
		else {
			PFC_ASSERT(state == CONF_PSTATE_VALUE_END);

			if (PFC_EXPECT_FALSE(type != TOKEN_SEMI)) {
				conf_parser_token_error(parser, &token);
				/* NOTREACHED */
			}
			state = CONF_PSTATE_NAME;
		}
	}

	/* Ensure that all mandatory parameters are defined. */
	string = conf_ctmap_rootkey(mandatory);
	if (PFC_EXPECT_FALSE(string != NULL)) {
		conf_parser_file_error(parser, "Missing mandatory parameter "
				       "\"%s\".", string);
		/* NOTREACHED */
	}
}

/*
 * static void
 * conf_parser_value(conf_parser_t *PFC_RESTRICT parser,
 *		     conf_pmap_t *PFC_RESTRICT pmap,
 *		     const pfc_cfdef_param_t *PFC_RESTRICT pdp,
 *		     pfc_refptr_t *PFC_RESTRICT pname,
 *		     conf_token_t *PFC_RESTRICT token)
 *	Parse parameter value.
 *
 * Remarks:
 *	This function always decrements the reference counter of `pname'
 */
static void
conf_parser_value(conf_parser_t *PFC_RESTRICT parser,
		  conf_pmap_t *PFC_RESTRICT pmap,
		  const pfc_cfdef_param_t *PFC_RESTRICT pdp,
		  pfc_refptr_t *PFC_RESTRICT pname,
		  conf_token_t *PFC_RESTRICT token)
{
	const value_parser_t	*vpp;
	pfc_cftype_t	ptype = pdp->cfdp_type;
	pfc_listm_t	array;
	conf_pnode_t	*pnp;
	conf_param_t	*param;
	int32_t		i, nelems, maxelems;
	int	err;

	/* Allocate parameter node instance. */
	CONF_PARSER_REFPTR_SET(parser, pname);
	pnp = conf_parser_pnode_alloc(parser, pdp);

	/* Register parameter instance. */
	err = conf_pmap_put(pmap, pnp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/*
		 * free() can be used to release conf_pnode_t because
		 * parameter value is not yet parsed.
		 */
		free(pnp);
		parser->cfp_errno = err;
		if (err == EEXIST) {
			conf_parser_file_error(parser, "Parameter \"%s\" is "
					       "already defined.",
					       pfc_refptr_string_value(pname));
		}
		else {
			conf_parser_error(parser,
					  "Failed to register parameter: %s",
					  strerror(err));
		}
		/* NOTREACHED */
	}

	if (pdp->cfdp_flags & PFC_CFPF_MANDATORY) {
		conf_ctmap_t	*mandatory = &parser->cfp_mandatory;
		const char	*key = pfc_refptr_string_value(pname);

		/* Remove defined mandatory parameter from cfp_mandatory. */
		err = conf_ctmap_remove(mandatory, key);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* This should not happen. */
			parser->cfp_errno = err;
			conf_parser_error(parser, "Failed to remove "
					  "mandatory parameter: %s: %s",
					  key, strerror(err));
			/* NOTREACHED */
		}
	}

	/* Decrement reference counter of this parameter name. */
	CONF_PARSER_REFPTR_CLEAR(parser);
	pfc_refptr_put(pname);

	PFC_CFTYPE_ASSERT(ptype);
	vpp = &value_parsers[ptype];
	param = &pnp->cpn_param;

	if (!PFC_CFDEF_PARAM_IS_ARRAY(pdp)) {
		/* Parse scalar value. */
		vpp->parse(parser, param, token);

		return;
	}

	/* Array value must start with left square bracket. */
	if (PFC_EXPECT_FALSE(token->cft_type != TOKEN_SQLEFT)) {
		conf_parser_token_error(parser, token);
		/* NOTREACHED */
	}

	array = parser->cfp_array;
	nelems = pdp->cfdp_nelems;
	if (nelems != PFC_CFPARAM_NELEMS_VARLEN) {
		int	cap = pfc_vector_get_capacity(array);

		/* Expand list as needed. */
		PFC_ASSERT((uint32_t)nelems <= PFC_CF_MAX_ARRAY_SIZE);
		PFC_ASSERT(cap >= 0);
		if (cap < nelems) {
			err = pfc_vector_set_capacity(array, nelems);
			if (PFC_EXPECT_FALSE(err != 0)) {
				parser->cfp_errno = err;
				conf_parser_error(parser, "Failed to expand "
						  "array: %s", strerror(err));
				/* NOTREACHED */
			}
		}
		maxelems = nelems;
	}
	else {
		maxelems = PFC_CF_MAX_ARRAY_SIZE;
	}

	i = 0;
	while (1) {
		conf_token_type_t	type;
		conf_param_t	*ep;

		conf_lexer_next(parser, token, PFC_TRUE);
		type = token->cft_type;
		if (type == TOKEN_SQRIGHT) {
			break;
		}

		/* Set token to be released on unexpected error. */
		conf_parser_token_release(parser, token);

		if (PFC_EXPECT_FALSE(i >= maxelems)) {
			if (nelems != PFC_CFPARAM_NELEMS_VARLEN) {
				goto bad_nelems;
			}
			conf_parser_file_error(parser,
					       "%s: Too many array elements.",
					       pdp->cfdp_name);
			/* NOTREACHED */
		}

		/* Allocate temporary parameter instance for this element. */
		ep = conf_parser_param_alloc(parser, pdp);
		err = pfc_listm_push_tail(array, (pfc_cptr_t)ep);
		if (PFC_EXPECT_FALSE(err != 0)) {
			free(ep);
			parser->cfp_errno = err;
			conf_parser_error(parser, "Failed to set array element"
					  ": %s", strerror(err));
			/* NOTREACHED */
		}

		/*
		 * Parse an array element.
		 * Note that token resource set by previous
		 * conf_parser_token_release() call must be cleared.
		 * The token will be released by parse operation.
		 */
		CONF_PARSER_REFPTR_CLEAR(parser);
		i++;
		vpp->parse(parser, ep, token);

		/*
		 * Read one more token.
		 * Note that we allow verbose comma at the tail of array.
		 * For instance, both [1, 2, 3] and [1, 2, 3,] are valid
		 * integer array.
		 */
		conf_lexer_next(parser, token, PFC_TRUE);
		type = token->cft_type;
		if (type == TOKEN_SQRIGHT) {
			break;
		}
		if (PFC_EXPECT_FALSE(type != TOKEN_COMMA)) {
			conf_parser_token_error(parser, token);
			/* NOTREACHED */
		}
	}

	PFC_ASSERT(pfc_listm_get_size(array) == i);
	if (PFC_EXPECT_FALSE(nelems != PFC_CFPARAM_NELEMS_VARLEN &&
			     i != nelems)) {
		goto bad_nelems;
	}

	/* Set array element as parameter value. */
	param->cp_nelems = i;
	if (i == 0) {
		/* Empty array. */
		param->cp_value.a = NULL;
	}
	else {
		/*
		 * Convert array elements into simple array.
		 * All conf_param_t instances in the temporary vector are
		 * released by to_array operation.
		 */
		param->cp_value.a = vpp->to_array(parser, i);
	}
	pfc_listm_clear(array);

	return;

bad_nelems:
	conf_parser_file_error(parser, "%s: Number of elements must be %d",
			       pdp->cfdp_name, nelems);
	/* NOTREACHED */
}

/*
 * static void
 * conf_parser_parse_byte(conf_parser_t *PFC_RESTRICT parser,
 *			  conf_param_t *PFC_RESTRICT param,
 *			  conf_token_t *PFC_RESTRICT token)
 *	Parse the given token as BYTE parameter value.
 */
static void
conf_parser_parse_byte(conf_parser_t *PFC_RESTRICT parser,
		       conf_param_t *PFC_RESTRICT param,
		       conf_token_t *PFC_RESTRICT token)
{
	const pfc_cfdef_param_t	*pdp = param->cp_def;

	CONF_VALUE_BYTE(param) =
		(uint8_t)conf_parser_verify_unsigned(parser, pdp, token);
}

/*
 * static void
 * conf_parser_parse_string(conf_parser_t *PFC_RESTRICT parser,
 *			    conf_param_t *PFC_RESTRICT param,
 *			    conf_token_t *PFC_RESTRICT token)
 *	Parse the given token as STRING parameter value.
 */
static void
conf_parser_parse_string(conf_parser_t *PFC_RESTRICT parser,
			 conf_param_t *PFC_RESTRICT param,
			 conf_token_t *PFC_RESTRICT token)
{
	const pfc_cfdef_param_t	*pdp = param->cp_def;
	pfc_refptr_t	*string;
	size_t		len;

	if (PFC_EXPECT_FALSE(token->cft_type != TOKEN_STRING &&
			     token->cft_type != TOKEN_SYMBOL)) {
		conf_parser_token_error(parser, token);
		/* NOTREACHED */
	}

	string = token->cft_value.string;

	/* Check lower limit of string length. */
	len = pfc_refptr_string_length(string);
	if (PFC_EXPECT_FALSE(len < pdp->cfdp_min)) {
		CONF_PARSER_REFPTR_SET(parser, string);
		conf_parser_file_error(parser, "%s: %u: String length must be "
				       "more than or equal %u",
				       pdp->cfdp_name, (int32_t)len,
				       (int32_t)pdp->cfdp_min);
		/* NOTREACHED */
	}

	/* Check upper limit of string length. */
	if (PFC_EXPECT_FALSE(len > pdp->cfdp_max)) {
		CONF_PARSER_REFPTR_SET(parser, string);
		conf_parser_file_error(parser, "%s: %u: String length must be "
				       "less than or equal %u",
				       pdp->cfdp_name, (int32_t)len,
				       (int32_t)pdp->cfdp_max);
		/* NOTREACHED */
	}

	CONF_VALUE_STRING(param) = string;
}

/*
 * static void
 * conf_parser_parse_bool(conf_parser_t *PFC_RESTRICT parser,
 *			  conf_param_t *PFC_RESTRICT param,
 *			  conf_token_t *PFC_RESTRICT token)
 *	Parse the given token as BOOL parameter value.
 */
static void
conf_parser_parse_bool(conf_parser_t *PFC_RESTRICT parser,
		       conf_param_t *PFC_RESTRICT param,
		       conf_token_t *PFC_RESTRICT token)
{
	pfc_refptr_t	*string;
	const char	*bstr;

	if (PFC_EXPECT_FALSE(token->cft_type != TOKEN_SYMBOL)) {
		conf_parser_token_error(parser, token);
		/* NOTREACHED */
	}

	string = token->cft_value.string;
	bstr = pfc_refptr_string_value(string);
	if (strcasecmp(bstr, "true") == 0) {
		CONF_VALUE_BOOL(param) = PFC_TRUE;
	}
	else if (strcasecmp(bstr, "false") == 0) {
		CONF_VALUE_BOOL(param) = PFC_FALSE;
	}
	else {
		CONF_PARSER_REFPTR_SET(parser, string);
		conf_parser_file_error(parser,
				       "Unexpected boolean value: %s", bstr);
		/* NOTREACHED */
	}

	pfc_refptr_put(string);
}

/*
 * static void
 * conf_parser_parse_int32(conf_parser_t *PFC_RESTRICT parser,
 *			   conf_param_t *PFC_RESTRICT param,
 *			   conf_token_t *PFC_RESTRICT token)
 *	Parse the given token as INT32 parameter value.
 */
static void
conf_parser_parse_int32(conf_parser_t *PFC_RESTRICT parser,
		       conf_param_t *PFC_RESTRICT param,
		       conf_token_t *PFC_RESTRICT token)
{
	const pfc_cfdef_param_t	*pdp = param->cp_def;

	CONF_VALUE_INT32(param) =
		(int32_t)conf_parser_verify_signed(parser, pdp, token);
}

/*
 * static void
 * conf_parser_parse_uint32(conf_parser_t *PFC_RESTRICT parser,
 *			    conf_param_t *PFC_RESTRICT param,
 *			    conf_token_t *PFC_RESTRICT token)
 *	Parse the given token as UINT32 parameter value.
 */
static void
conf_parser_parse_uint32(conf_parser_t *PFC_RESTRICT parser,
			 conf_param_t *PFC_RESTRICT param,
			 conf_token_t *PFC_RESTRICT token)
{
	const pfc_cfdef_param_t	*pdp = param->cp_def;

	CONF_VALUE_UINT32(param) =
		(uint32_t)conf_parser_verify_unsigned(parser, pdp, token);
}

/*
 * static void
 * conf_parser_parse_int64(conf_parser_t *PFC_RESTRICT parser,
 *			   conf_param_t *PFC_RESTRICT param,
 *			   conf_token_t *PFC_RESTRICT token)
 *	Parse the given token as INT64 parameter value.
 */
static void
conf_parser_parse_int64(conf_parser_t *PFC_RESTRICT parser,
		       conf_param_t *PFC_RESTRICT param,
		       conf_token_t *PFC_RESTRICT token)
{
	const pfc_cfdef_param_t	*pdp = param->cp_def;

	CONF_VALUE_INT64(param) =
		conf_parser_verify_signed(parser, pdp, token);
}

/*
 * static void
 * conf_parser_parse_uint64(conf_parser_t *PFC_RESTRICT parser,
 *			    conf_param_t *PFC_RESTRICT param,
 *			    conf_token_t *PFC_RESTRICT token)
 *	Parse the given token as UINT64 parameter value.
 */
static void
conf_parser_parse_uint64(conf_parser_t *PFC_RESTRICT parser,
			 conf_param_t *PFC_RESTRICT param,
			 conf_token_t *PFC_RESTRICT token)
{
	const pfc_cfdef_param_t	*pdp = param->cp_def;

	CONF_VALUE_UINT64(param) =
		conf_parser_verify_unsigned(parser, pdp, token);
}

/*
 * static pfc_ptr_t
 * conf_parser_to_byte_array(conf_parser_t *parser, uint32_t size)
 *	Create BYTE array that keeps all elements in parser->cfp_array.
 */
static pfc_ptr_t
conf_parser_to_byte_array(conf_parser_t *parser, uint32_t size)
{
	pfc_listm_t	array = parser->cfp_array;
	uint32_t	i;
	uint8_t		*ptr, *p;

	ptr = (uint8_t *)malloc(sizeof(*ptr) * size);
	if (PFC_EXPECT_FALSE(ptr == NULL)) {
		parser->cfp_errno = ENOMEM;
		conf_parser_error(parser, "Failed to allocate byte array.");
		/* NOTREACHED */
	}

	for (p = ptr, i = 0; i < size; p++, i++) {
		conf_param_t	*param;

		PFC_ASSERT_INT(pfc_listm_getat(array, i,
					       (pfc_cptr_t *)&param), 0);
		*p = CONF_VALUE_BYTE(param);
		free(param);
	}

	return (pfc_ptr_t)ptr;
}

/*
 * static pfc_ptr_t
 * conf_parser_to_string_array(conf_parser_t *parser, uint32_t size)
 *	Create STRING array that keeps all elements in parser->cfp_array.
 */
static pfc_ptr_t
conf_parser_to_string_array(conf_parser_t *parser, uint32_t size)
{
	pfc_listm_t	array = parser->cfp_array;
	uint32_t	i;
	pfc_refptr_t	**ptr, **pp;

	ptr = (pfc_refptr_t **)malloc(sizeof(*ptr) * size);
	if (PFC_EXPECT_FALSE(ptr == NULL)) {
		parser->cfp_errno = ENOMEM;
		conf_parser_error(parser, "Failed to allocate string array.");
		/* NOTREACHED */
	}

	for (pp = ptr, i = 0; i < size; pp++, i++) {
		conf_param_t	*param;

		PFC_ASSERT_INT(pfc_listm_getat(array, i,
					       (pfc_cptr_t *)&param), 0);
		*pp = CONF_VALUE_STRING(param);
		free(param);
	}

	return (pfc_ptr_t)ptr;
}

/*
 * static pfc_ptr_t
 * conf_parser_to_bool_array(conf_parser_t *parser, uint32_t size)
 *	Create BOOL array that keeps all elements in parser->cfp_array.
 */
static pfc_ptr_t
conf_parser_to_bool_array(conf_parser_t *parser, uint32_t size)
{
	pfc_listm_t	array = parser->cfp_array;
	uint32_t	i;
	pfc_bool_t	*ptr, *p;

	ptr = (pfc_bool_t *)malloc(sizeof(*ptr) * size);
	if (PFC_EXPECT_FALSE(ptr == NULL)) {
		parser->cfp_errno = ENOMEM;
		conf_parser_error(parser, "Failed to allocate bool array.");
		/* NOTREACHED */
	}

	for (p = ptr, i = 0; i < size; p++, i++) {
		conf_param_t	*param;

		PFC_ASSERT_INT(pfc_listm_getat(array, i,
					       (pfc_cptr_t *)&param), 0);
		*p = CONF_VALUE_BOOL(param);
		free(param);
	}

	return (pfc_ptr_t)ptr;
}

/*
 * static pfc_ptr_t
 * conf_parser_to_int32_array(conf_parser_t *parser, uint32_t size)
 *	Create INT32 array that keeps all elements in parser->cfp_array.
 */
static pfc_ptr_t
conf_parser_to_int32_array(conf_parser_t *parser, uint32_t size)
{
	pfc_listm_t	array = parser->cfp_array;
	uint32_t	i;
	int32_t		*ptr, *p;

	ptr = (int32_t *)malloc(sizeof(*ptr) * size);
	if (PFC_EXPECT_FALSE(ptr == NULL)) {
		parser->cfp_errno = ENOMEM;
		conf_parser_error(parser, "Failed to allocate int32 array.");
		/* NOTREACHED */
	}

	for (p = ptr, i = 0; i < size; p++, i++) {
		conf_param_t	*param;

		PFC_ASSERT_INT(pfc_listm_getat(array, i,
					       (pfc_cptr_t *)&param), 0);
		*p = CONF_VALUE_INT32(param);
		free(param);
	}

	return (pfc_ptr_t)ptr;
}

/*
 * static pfc_ptr_t
 * conf_parser_to_uint32_array(conf_parser_t *parser, uint32_t size)
 *	Create UINT32 array that keeps all elements in parser->cfp_array.
 */
static pfc_ptr_t
conf_parser_to_uint32_array(conf_parser_t *parser, uint32_t size)
{
	pfc_listm_t	array = parser->cfp_array;
	uint32_t	i;
	uint32_t	*ptr, *p;

	ptr = (uint32_t *)malloc(sizeof(*ptr) * size);
	if (PFC_EXPECT_FALSE(ptr == NULL)) {
		parser->cfp_errno = ENOMEM;
		conf_parser_error(parser, "Failed to allocate uint32 array.");
		/* NOTREACHED */
	}

	for (p = ptr, i = 0; i < size; p++, i++) {
		conf_param_t	*param;

		PFC_ASSERT_INT(pfc_listm_getat(array, i,
					       (pfc_cptr_t *)&param), 0);
		*p = CONF_VALUE_UINT32(param);
		free(param);
	}

	return (pfc_ptr_t)ptr;
}

/*
 * static pfc_ptr_t
 * conf_parser_to_int64_array(conf_parser_t *parser, uint32_t size)
 *	Create INT64 array that keeps all elements in parser->cfp_array.
 */
static pfc_ptr_t
conf_parser_to_int64_array(conf_parser_t *parser, uint32_t size)
{
	pfc_listm_t	array = parser->cfp_array;
	uint32_t	i;
	int64_t		*ptr, *p;

	ptr = (int64_t *)malloc(sizeof(*ptr) * size);
	if (PFC_EXPECT_FALSE(ptr == NULL)) {
		parser->cfp_errno = ENOMEM;
		conf_parser_error(parser, "Failed to allocate int64 array.");
		/* NOTREACHED */
	}

	for (p = ptr, i = 0; i < size; p++, i++) {
		conf_param_t	*param;

		PFC_ASSERT_INT(pfc_listm_getat(array, i,
					       (pfc_cptr_t *)&param), 0);
		*p = CONF_VALUE_INT64(param);
		free(param);
	}

	return (pfc_ptr_t)ptr;
}

/*
 * static pfc_ptr_t
 * conf_parser_to_uint64_array(conf_parser_t *parser, uint32_t size)
 *	Create UINT64 array that keeps all elements in parser->cfp_array.
 */
static pfc_ptr_t
conf_parser_to_uint64_array(conf_parser_t *parser, uint32_t size)
{
	pfc_listm_t	array = parser->cfp_array;
	uint32_t	i;
	uint64_t	*ptr, *p;

	ptr = (uint64_t *)malloc(sizeof(*ptr) * size);
	if (PFC_EXPECT_FALSE(ptr == NULL)) {
		parser->cfp_errno = ENOMEM;
		conf_parser_error(parser, "Failed to allocate uint64 array.");
		/* NOTREACHED */
	}

	for (p = ptr, i = 0; i < size; p++, i++) {
		conf_param_t	*param;

		PFC_ASSERT_INT(pfc_listm_getat(array, i,
					       (pfc_cptr_t *)&param), 0);
		*p = CONF_VALUE_UINT64(param);
		free(param);
	}

	return (pfc_ptr_t)ptr;
}

/*
 * static conf_param_t *
 * conf_parser_param_alloc(conf_parser_t *PFC_RESTRICT parser,
 *			   const pfc_cfdef_param_t *PFC_RESTRICT pdp)
 *	Allocate a new parameter instance.
 *
 * Calling/Exit State:
 *	This function always returns a valid non-NULL pointer.
 *	An exception is caused on error.
 */
static conf_param_t *
conf_parser_param_alloc(conf_parser_t *PFC_RESTRICT parser,
			const pfc_cfdef_param_t *PFC_RESTRICT pdp)
{
	conf_param_t	*param;

	param = (conf_param_t *)malloc(sizeof(*param));
	if (PFC_EXPECT_FALSE(param == NULL)) {
		parser->cfp_errno = ENOMEM;
		conf_parser_error(parser,
				  "Failed to allocate memory for parameter.");
		/* NOTREACHED */
	}

	conf_parser_param_init(param, pdp);

	return param;
}

/*
 * static conf_pnode_t *
 * conf_parser_pnode_alloc(conf_parser_t *PFC_RESTRICT parser,
 *			   const pfc_cfdef_param_t *PFC_RESTRICT pdp)
 *	Allocate a new parameter node instance.
 *
 * Calling/Exit State:
 *	This function always returns a valid pointer.
 *	An exception is caused on error.
 */
static conf_pnode_t *
conf_parser_pnode_alloc(conf_parser_t *PFC_RESTRICT parser,
			const pfc_cfdef_param_t *PFC_RESTRICT pdp)
{
	conf_pnode_t	*pnp;

	pnp = (conf_pnode_t *)malloc(sizeof(*pnp));
	if (PFC_EXPECT_FALSE(pnp == NULL)) {
		parser->cfp_errno = ENOMEM;
		conf_parser_error(parser,
				  "Failed to allocate memory for parameter "
				  "node.");
		/* NOTREACHED */
	}

	conf_parser_param_init(&pnp->cpn_param, pdp);

	return pnp;
}

/*
 * static void
 * conf_parser_param_init(conf_param_t *PFC_RESTRICT param,
 *			  const pfc_cfdef_param_t *PFC_RESTRICT pdp)
 *	Initialize conf_param_t instance.
 */
static void
conf_parser_param_init(conf_param_t *PFC_RESTRICT param,
		       const pfc_cfdef_param_t *PFC_RESTRICT pdp)
{
	param->cp_def = pdp;
	param->cp_nelems = 0;
	CONF_VALUE_UINT64(param) = 0;
}

/*
 * static void
 * conf_parser_token_error(conf_parser_t *PFC_RESTRICT parser,
 *			   conf_token_t *PFC_RESTRICT token)
 *	Raise an error which notifies unexpected token is detected.
 */
static void
conf_parser_token_error(conf_parser_t *PFC_RESTRICT parser,
			conf_token_t *PFC_RESTRICT token)
{
	conf_token_type_t	type = token->cft_type;

	if (type == TOKEN_INT) {
		conf_parser_file_error(parser, "Unexpected INT token: %"
				       PFC_PFMT_u64, token->cft_value.integer);
	}
	else if (type == TOKEN_STRING || type == TOKEN_SYMBOL) {
		pfc_refptr_t	*rstr = token->cft_value.string;
		const char	*label = (type == TOKEN_STRING)
			? "STRING" : "SYMBOL";

		CONF_PARSER_REFPTR_SET(parser, rstr);
		conf_parser_file_error(parser, "Unexpected %s token: %s",
				       label, pfc_refptr_string_value(rstr));
	}
	else {
		conf_parser_file_error(parser, "Unexpected token: %c",
				       token->cft_value.character);
	}

	/* NOTREACHED */
}

/*
 * static void
 * conf_parser_error_nojmp(pfc_conf_err_t errfunc, const char *fmt, ...)
 *	Call parser error handler without longjmp().
 */
static void
conf_parser_error_nojmp(pfc_conf_err_t errfunc, const char *PFC_RESTRICT fmt,
			...)
{
	va_list	ap;

	va_start(ap, fmt);
	conf_parser_verror_nojmp(errfunc, fmt, ap);
	va_end(ap);
}

/*
 * static void
 * conf_parser_verror_nojmp(pfc_conf_err_t errfunc,
 *			    const char *PFC_RESTRICT fmt, va_list ap)
 *	Call parser error handler without longjmp().
 *	Error message must be specified in vprintf(3) format.
 */
static void
conf_parser_verror_nojmp(pfc_conf_err_t errfunc, const char *PFC_RESTRICT fmt,
			 va_list ap)
{
	if (errfunc != NULL) {
		(*errfunc)(fmt, ap);
	}
	else {
		/* Print error message to the standard error output. */
		vfprintf(stderr, fmt, ap);
		fputc('\n', stderr);
	}
}
