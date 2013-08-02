/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_CONF_IMPL_H
#define	_PFC_LIBPFC_UTIL_CONF_IMPL_H

/*
 * Internal definitions for configuration file parser.
 */

#include <stdio.h>
#include <setjmp.h>
#include <pfc/conf.h>
#include <pfc/conf_parser.h>
#include <pfc/rbtree.h>
#include <pfc/listmodel.h>
#include <pfc/refptr.h>
#include <pfc/synch.h>
#include <pfc/debug.h>
#include <pfc/list.h>

PFC_C_BEGIN_DECL

/*
 * Depth of character stack.
 */
#define	CONF_CHAR_STACK_DEPTH	PFC_CONST_U(16)

/*
 * Temporary buffer size.
 */
#define	CONF_PARSER_BUFSIZE	(PFC_CF_MAX_STRLEN + 1)

/*
 * Sign bit for 64-bit integer.
 */
#define	CONF_INT64_SIGN		(PFC_CONST_ULL(1) << 63)

/*
 * Determine whether the given 64-bit value is negative or not.
 */
#define	CONF_INT64_IS_NEGATIVE(value)	((uint64_t)(value) & CONF_INT64_SIGN)

/*
 * Token type.
 */
typedef enum {
	TOKEN_SEMI,			/* semicolon (;) */
	TOKEN_LBRACE,			/* left brace ({) */
	TOKEN_RBRACE,			/* right brace (}) */
	TOKEN_SQLEFT,			/* left square bracket ([) */
	TOKEN_SQRIGHT,			/* right square bracket (]) */
	TOKEN_EQUAL,			/* equal (=) */
	TOKEN_COMMA,			/* comma (,) */
	TOKEN_SYMBOL,			/* symbol */
	TOKEN_INT,			/* 64-bit integer */
	TOKEN_STRING,			/* quoted string */
} conf_token_type_t;

/*
 * Lexer token.
 */
typedef struct {
	conf_token_type_t	cft_type;	/* token type */

	/*
	 * PFC_TRUE if the token represents negative integer.
	 */
	pfc_bool_t		cft_negative;

	/* token value */
	union {
		pfc_refptr_t	*string;	/* STRING, SYMBOL */
		uint64_t	integer;	/* INT */
		uint8_t		character;	/* others */
	} cft_value;
} conf_token_t;

/*
 * Type definitions of maps for internal use.
 * Currently all of them are implemented by Red-Black tree.
 */
typedef pfc_rbtree_t		conf_hdlmap_t;
typedef pfc_rbtree_t		conf_blkmap_t;
typedef pfc_rbtree_t		conf_pmap_t;
typedef pfc_rbtree_t		conf_ctmap_t;
typedef pfc_rbtree_t		conf_imap_t;

/*
 * Parameter block handle.
 */
struct conf_block;
typedef struct conf_block	conf_block_t;

struct conf_block {
	pfc_refptr_t		*cb_name;	/* block name or map key */
	pfc_cfblk_t		cb_handle;	/* parameter block handle */
	pfc_rbnode_t		cb_blknode;	/* block map node */
	pfc_rbnode_t		cb_hdlnode;	/* block handle map node */
	conf_pmap_t		cb_params;	/* parameter map */
	pfc_list_t		cb_list;	/* next and previous link */
};

#define	CONF_BLK_PTR(block)	((conf_block_t *)(block))

#define	CONF_BLKNODE2BLOCK(node)				\
	PFC_CAST_CONTAINER(node, conf_block_t, cb_blknode)
#define	CONF_HDLNODE2BLOCK(node)				\
	PFC_CAST_CONTAINER(node, conf_block_t, cb_hdlnode)

#define	CONF_LIST2BLOCK(list)				\
	PFC_CAST_CONTAINER(list, conf_block_t, cb_list)

/*
 * Configuration file handle.
 */
typedef struct {
	conf_blkmap_t		cf_blocks;	/* parsed values in block */
	conf_imap_t		cf_maps;	/* parsed values in map */
	uint32_t		cf_flags;	/* flags */
	const pfc_cfdef_t	*cf_defs;	/* definitions */
	pfc_refptr_t		*cf_path;	/* configuration file path */
	pfc_refptr_t		*cf_secondary;	/* secondary conf file path */
	pfc_list_t		cf_blklist;	/* all block handle list */
} conf_file_t;

#define	CONF_FILE_PTR(conf)	((conf_file_t *)(conf))

/*
 * Flags for cf_flags.
 */
#define	CONF_FF_PRIMARY		PFC_CONST_U(0x1)	/* primary exists */
#define	CONF_FF_SECONDARY	PFC_CONST_U(0x2)	/* secondary exists */
#define	CONF_FF_FILEMASK	(CONF_FF_PRIMARY |CONF_FF_SECONDARY)

/*
 * Parameter instance.
 */
typedef struct {
	const pfc_cfdef_param_t	*cp_def;	/* parameter definition */
	uint32_t		cp_nelems;	/* number of array elements */

	/* Parameter value */
	union {
		pfc_ptr_t	a;
		pfc_refptr_t	*s;
		pfc_bool_t	b;
		uint8_t		byte;
		int32_t		i32;
		uint32_t	u32;
		int64_t		i64;
		uint64_t	u64;
	} cp_value;
} conf_param_t;

#define	CONF_VALUE_BYTE(param)		((param)->cp_value.byte)
#define	CONF_VALUE_STRING(param)	((param)->cp_value.s)
#define	CONF_VALUE_BOOL(param)		((param)->cp_value.b)
#define	CONF_VALUE_INT32(param)		((param)->cp_value.i32)
#define	CONF_VALUE_UINT32(param)	((param)->cp_value.u32)
#define	CONF_VALUE_INT64(param)		((param)->cp_value.i64)
#define	CONF_VALUE_UINT64(param)	((param)->cp_value.u64)

#define	CONF_VALUE_ARRAY(param, type)	((type *)((param)->cp_value.a))
#define	CONF_VALUE_BYTE_ARRAY(param)	CONF_VALUE_ARRAY((param), uint8_t)
#define	CONF_VALUE_STRING_ARRAY(param)			\
	CONF_VALUE_ARRAY((param), pfc_refptr_t *)
#define	CONF_VALUE_BOOL_ARRAY(param)	CONF_VALUE_ARRAY((param), pfc_bool_t)
#define	CONF_VALUE_INT32_ARRAY(param)	CONF_VALUE_ARRAY((param), int32_t)
#define	CONF_VALUE_UINT32_ARRAY(param)	CONF_VALUE_ARRAY((param), uint32_t)
#define	CONF_VALUE_INT64_ARRAY(param)	CONF_VALUE_ARRAY((param), int64_t)
#define	CONF_VALUE_UINT64_ARRAY(param)	CONF_VALUE_ARRAY((param), uint64_t)
#define	CONF_VALUE_LONG_ARRAY(param)	CONF_VALUE_ARRAY((param), pfc_long_t)
#define	CONF_VALUE_ULONG_ARRAY(param)	CONF_VALUE_ARRAY((param), pfc_ulong_t)

#ifdef	PFC_LP64

/* On LP64 system, LONG and ULONG are treated as 64-bit integer. */
#define	CONF_VALUE_LONG(param)		CONF_VALUE_INT64(param)
#define	CONF_VALUE_ULONG(param)		CONF_VALUE_UINT64(param)

#else	/* !PFC_LP64 */

/* On ILP32 system, LONG and ULONG are treated as 32-bit integer. */
#define	CONF_VALUE_LONG(param)		CONF_VALUE_INT32(param)
#define	CONF_VALUE_ULONG(param)		CONF_VALUE_UINT32(param)

#endif	/* PFC_LP64 */

#define	CONF_VALUE_IS_ARRAY(param)			\
	PFC_CFDEF_PARAM_IS_ARRAY((param)->cp_def)

#define	CONF_VALUE_TYPE(param)		((param)->cp_def->cfdp_type)
#define	CONF_VALUE_IS_BYTE(param)			\
	(CONF_VALUE_TYPE(param) == PFC_CFTYPE_BYTE)
#define	CONF_VALUE_IS_STRING(param)			\
	(CONF_VALUE_TYPE(param) == PFC_CFTYPE_STRING)
#define	CONF_VALUE_IS_BOOL(param)			\
	(CONF_VALUE_TYPE(param) == PFC_CFTYPE_BOOL)
#define	CONF_VALUE_IS_INT32(param)			\
	(CONF_VALUE_TYPE(param) == PFC_CFTYPE_INT32)
#define	CONF_VALUE_IS_UINT32(param)			\
	(CONF_VALUE_TYPE(param) == PFC_CFTYPE_UINT32)
#define	CONF_VALUE_IS_INT64(param)			\
	(CONF_VALUE_TYPE(param) == PFC_CFTYPE_INT64)
#define	CONF_VALUE_IS_UINT64(param)			\
	(CONF_VALUE_TYPE(param) == PFC_CFTYPE_UINT64)
#define	CONF_VALUE_IS_LONG(param)			\
	(CONF_VALUE_TYPE(param) == PFC_CFTYPE_LONG)
#define	CONF_VALUE_IS_ULONG(param)			\
	(CONF_VALUE_TYPE(param) == PFC_CFTYPE_ULONG)

/*
 * Intermediate map entry which keeps a pair of string refptr and block map.
 */
typedef struct {
	pfc_refptr_t	*cie_name;		/* key string refptr */
	pfc_rbnode_t	cie_node;		/* Red-Black tree node */
	conf_blkmap_t	cie_blkmap;		/* parameter block map */
	pfc_listm_t	cie_keys;		/* map keys in cie_blkmap */
} conf_imapent_t;

#define	CONF_NODE2IMAPENT(node)					\
	PFC_CAST_CONTAINER(node, conf_imapent_t, cie_node)

/*
 * Container map entry which keeps pair of string and anonymous pointer.
 * Note that no destructor for key and value is provided.
 */
typedef struct {
	const char	*cc_key;		/* string key */
	pfc_cptr_t	cc_value;		/* value */
	pfc_rbnode_t	cc_node;		/* Red-Black tree node */
} conf_ctent_t;

#define	CONF_NODE2CTENT(node)			\
	PFC_CAST_CONTAINER(node, conf_ctent_t, cc_node)

/*
 * Value for container map.
 * This type is provided to avoid unreasonable warning blamed by gcc-4.4.5.
 */
typedef union {
	pfc_cptr_t			ccv_value;
	const pfc_cfdef_block_t		*ccv_block;
	const pfc_cfdef_param_t		*ccv_param;
} conf_ctvalue_t;

/*
 * Node of parameter map.
 */
typedef struct {
	conf_param_t	cpn_param;		/* parameter */
	pfc_rbnode_t	cpn_node;		/* parameter map node */
} conf_pnode_t;

#define	CONF_NODE2PNODE(node)					\
	PFC_CAST_CONTAINER(node, conf_pnode_t, cpn_node)

/*
 * Parser context.
 */
typedef struct {
	conf_file_t		*cfp_value;	/* parsed values */
	conf_ctmap_t		cfp_blocks;	/* block definitions */
	conf_ctmap_t		cfp_maps;	/* map definitions */
	conf_ctmap_t		cfp_params;	/* parameter definitions */
	conf_ctmap_t		cfp_mandatory;	/* mandatory params */
	pfc_conf_err_t		cfp_errfunc;	/* error handler */
	jmp_buf			cfp_env;	/* context for exception */

	const char		*cfp_path;	/* configuration file path */
	uint32_t		cfp_line;	/* current line number */
	uint32_t		cfp_flags;	/* parser flags */
	FILE			*cfp_file;	/* file handle */

	/* temporary refptr which should be released on error */
	pfc_refptr_t		*cfp_refptr;

	/* list which keeps array elements temporarily. */
	pfc_listm_t		cfp_array;

	/* error number which indicates the cause of parse error */
	int			cfp_errno;

	/* current character stack depth */
	uint8_t			cfp_chstack_depth;

	/* character stack used by lexer */
	uint8_t			cfp_chstack[CONF_CHAR_STACK_DEPTH];

	/* temporary buffer */
	uint8_t			cfp_buffer[CONF_PARSER_BUFSIZE];
} conf_parser_t;

/*
 * Parser flags.
 */
#define	CONF_PF_EXPORT		PFC_CONST_U(0x1)	/* export blocks */
#define	CONF_PF_NO_ENOENT	PFC_CONST_U(0x2)	/* ignore ENOENT */
#define	CONF_PF_NO_MANDATORY	PFC_CONST_U(0x4)	/* skip mandatory */

#ifdef	_PFC_LIBPFC_UTIL_BUILD

/*
 * pfc_cfdef_t which determines the syntax of PFC system configuration file.
 */
extern pfc_cfdef_t	pfcd_conf_defs;

/*
 * Global read/write lock for configuration file parser.
 */
extern pfc_rwlock_t	conf_parser_lock;

#define	CONF_PARSER_RDLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&conf_parser_lock), 0)
#define	CONF_PARSER_WRLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&conf_parser_lock), 0)
#define	CONF_PARSER_UNLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&conf_parser_lock), 0)

#define	CONF_PARSER_TRYWRLOCK()	pfc_rwlock_trywrlock(&conf_parser_lock)

/*
 * Global block handle map.
 * Note that any block handle map access requires parser lock.
 */
extern conf_hdlmap_t	conf_blocks;

/*
 * Configuration handle associated with the PFC system configuration file.
 */
extern pfc_conf_t	pfcd_conf;

/*
 * Prototypes.
 */
extern void	pfc_conf_libfini(void);
extern void 	pfc_conf_fork_prepare(void);
extern void 	pfc_conf_fork_parent(void);
extern void 	pfc_conf_fork_child(void);

extern void	conf_parser_error(conf_parser_t *PFC_RESTRICT parser,
				  const char *PFC_RESTRICT fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3) PFC_FATTR_NORETURN;
extern void	conf_parser_file_error(conf_parser_t *PFC_RESTRICT parser,
				       const char *PFC_RESTRICT fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3) PFC_FATTR_NORETURN;

extern void		conf_lexer_open(conf_parser_t *parser);
extern void		conf_lexer_close(conf_parser_t *parser);
extern pfc_bool_t	conf_lexer_next(conf_parser_t *PFC_RESTRICT parser,
					conf_token_t *PFC_RESTRICT token,
					pfc_bool_t needed);

/* Internal APIs for conf_hdlmap_t. */
extern int	conf_hdlmap_put(conf_block_t *bp);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * conf_hdlmap_remove(conf_block_t *bp)
 *	Remove the specified configuration block handle from the global
 *	handle map. The call of this function invalidates bp->cb_handle.
 *
 * Remarks:
 *	- The caller must call this function with holding parser lock in
 *	  writer mode.
 *	- This function never frees removed conf_block_t instance.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
conf_hdlmap_remove(conf_block_t *bp)
{
	if (PFC_EXPECT_TRUE(bp->cb_handle != PFC_CFBLK_INVALID)) {
		/* Remove this handle from the handle map. */
		pfc_rbtree_remove_node(&conf_blocks, &bp->cb_hdlnode);
	}
}

/*
 * static inline conf_block_t PFC_FATTR_ALWAYS_INLINE *
 * conf_hdlmap_get(pfc_cfblk_t block)
 *	Return a pointer to configuration block instance associated with
 *	the specified handle ID.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to conf_block_t instance
 *	is returned. NULL is returned if not found.
 *
 * Remarks:
 *	This function must be called with holding parser lock in reader or
 *	writer mode.
 */
static inline conf_block_t PFC_FATTR_ALWAYS_INLINE *
conf_hdlmap_get(pfc_cfblk_t block)
{
	pfc_rbnode_t	*node;

	node = pfc_rbtree_get(&conf_blocks, (pfc_cptr_t)(uintptr_t)block);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		return NULL;
	}

	return CONF_HDLNODE2BLOCK(node);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * conf_hdlmap_clear(void)
 *	Invalidate all active handle IDs.
 *
 * Remarks:
 *	- The caller must call this function with holding parser lock in
 *	  writer mode.
 *	- This function only clears the global handle map, so all conf_block_t
 *	  instances are not freed.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
conf_hdlmap_clear(void)
{
	pfc_rbtree_clear(&conf_blocks, NULL, NULL);
}

/* Internal APIs for conf_blkmap_t. */
extern void	conf_blkmap_init(conf_blkmap_t *blkmap);
extern int	conf_blkmap_alloc(conf_blkmap_t *PFC_RESTRICT blkmap,
				  pfc_refptr_t *PFC_RESTRICT name,
				  conf_block_t *PFC_RESTRICT *bpp);
extern void	conf_blkmap_clear(conf_blkmap_t *blkmap);
extern void	conf_blkmap_merge(conf_blkmap_t *PFC_RESTRICT blkmap,
				  conf_blkmap_t *PFC_RESTRICT merged);

/*
 * static inline conf_block_t PFC_FATTR_ALWAYS_INLINE *
 * conf_blkmap_get(conf_blkmap_t *PFC_RESTRICT blkmap,
 *		   const char *PFC_RESTRICT name)
 *	Return a pointer to parameter block instance associated with the
 *	specified name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to conf_block_t
 *	instance is returned.
 *	NULL is returned if parameter block is not found.
 */
static inline conf_block_t PFC_FATTR_ALWAYS_INLINE *
conf_blkmap_get(conf_blkmap_t *PFC_RESTRICT blkmap,
		const char *PFC_RESTRICT name)
{
	pfc_rbnode_t	*node = pfc_rbtree_get(blkmap, name);

	if (PFC_EXPECT_TRUE(node != NULL)) {
		return CONF_BLKNODE2BLOCK(node);
	}

	return NULL;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * conf_blkmap_copy(conf_blkmap_t *dst, conf_blkmap_t *src)
 *	Copy parameter block map from `src' to `dst'.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
conf_blkmap_copy(conf_blkmap_t *dst, conf_blkmap_t *src)
{
	dst->rb_root = src->rb_root;
}

/* Internal APIs for conf_imap_t. */
extern void	conf_imap_init(conf_imap_t *imap);
extern int	conf_imap_alloc(conf_imap_t *PFC_RESTRICT imap,
				pfc_refptr_t *rname, pfc_refptr_t *key,
				conf_blkmap_t *PFC_RESTRICT *blkmapp);
extern void	conf_imap_clear(conf_imap_t *imap);
extern int	conf_imap_merge(conf_imap_t *PFC_RESTRICT imap,
				conf_imap_t *PFC_RESTRICT merged);

/*
 * static inline conf_blkmap_t PFC_FATTR_ALWAYS_INLINE *
 * conf_imap_getmap(conf_imap_t *PFC_RESTRICT imap,
 *		    const char *PFC_RESTRICT name)
 *	Return a pointer to conf_blkmap_t instance associated with the
 *	specified name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to conf_blkmap_t
 *	instance is returned.
 *	NULL is returned if parameter map is not found.
 */
static inline conf_blkmap_t PFC_FATTR_ALWAYS_INLINE *
conf_imap_getmap(conf_imap_t *PFC_RESTRICT imap, const char *PFC_RESTRICT name)
{
	pfc_rbnode_t	*node = pfc_rbtree_get(imap, name);

	if (PFC_EXPECT_TRUE(node != NULL)) {
		conf_imapent_t	*imp = CONF_NODE2IMAPENT(node);

		return &imp->cie_blkmap;
	}

	return NULL;
}

/*
 * static inline conf_imapent_t PFC_FATTR_ALWAYS_INLINE *
 * conf_imap_get(conf_imap_t *PFC_RESTRICT imap, const char *PFC_RESTRICT name)
 *	Return a pointer to intermediate map entry associated with the
 *	specified name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to conf_imapent_t
 *	instance is returned.
 *	NULL is returned if parameter map is not found.
 */
static inline conf_imapent_t PFC_FATTR_ALWAYS_INLINE *
conf_imap_get(conf_imap_t *PFC_RESTRICT imap, const char *PFC_RESTRICT name)
{
	pfc_rbnode_t	*node = pfc_rbtree_get(imap, name);

	if (PFC_EXPECT_TRUE(node != NULL)) {
		return CONF_NODE2IMAPENT(node);
	}

	return NULL;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * conf_imap_copy(conf_imap_t *dst, conf_imap_t *src)
 *	Copy intermediate map from `src' to `dst'.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
conf_imap_copy(conf_imap_t *dst, conf_imap_t *src)
{
	dst->rb_root = src->rb_root;
}

/* Internal APIs for conf_ctmap_t. */
extern void	conf_ctmap_init(conf_ctmap_t *ctmap);
extern int	conf_ctmap_put(conf_ctmap_t *PFC_RESTRICT ctmap,
			       const char *PFC_RESTRICT key,
			       pfc_cptr_t PFC_RESTRICT value);
extern int	conf_ctmap_remove(conf_ctmap_t *PFC_RESTRICT ctmap,
				  const char *PFC_RESTRICT key);
extern void	conf_ctmap_clear(conf_ctmap_t *ctmap);

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * conf_ctmap_get(conf_ctmap_t *PFC_RESTRICT ctmap,
 *		  const char *PFC_RESTRICT key,
 *		  conf_ctvalue_t *PFC_RESTRICT valuep)
 *	Find a value associated with the given string in the specified
 *	container map.
 *
 * Calling/Exit State:
 *	Upon successful completion, a value associated with the given key is
 *	set to `*valuep', and zero is returned.
 *	ENOENT is returned if the given key is not found.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
conf_ctmap_get(conf_ctmap_t *PFC_RESTRICT ctmap, const char *PFC_RESTRICT key,
	       conf_ctvalue_t *PFC_RESTRICT valuep)
{
	pfc_rbnode_t	*node;
	conf_ctent_t	*ccp;

	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	node = pfc_rbtree_get(ctmap, (pfc_cptr_t)key);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		return ENOENT;
	}

	ccp = CONF_NODE2CTENT(node);
	valuep->ccv_value = ccp->cc_value;

	return 0;
}

/*
 * static inline const char PFC_FATTR_ALWAYS_INLINE *
 * conf_ctmap_rootkey(conf_ctmap_t *ctmap)
 *	If the container map is not empty, return key at the root tree node.
 *	NULL is returned if the container is empty.
 */
static inline const char PFC_FATTR_ALWAYS_INLINE *
conf_ctmap_rootkey(conf_ctmap_t *ctmap)
{
	pfc_rbnode_t	*node = ctmap->rb_root;

	if (node != NULL) {
		conf_ctent_t	*ccp = CONF_NODE2CTENT(node);

		return ccp->cc_key;
	}

	return NULL;
}

/* Internal APIs for conf_pmap_t. */
extern void	conf_pmap_init(conf_pmap_t *pmap);
extern void	conf_pmap_clear(conf_pmap_t *pmap);

/*
 * static inline conf_param_t PFC_FATTR_ALWAYS_INLINE *
 * conf_pmap_get(conf_pmap_t *PFC_RESTRICT pmap, const char *PFC_RESTRICT name)
 *	Return a pointer to parameter value associated with the specified name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to conf_param_t
 *	instance is returned.
 *	NULL is returned if parameter node is not found.
 */
static inline conf_param_t PFC_FATTR_ALWAYS_INLINE *
conf_pmap_get(conf_pmap_t *PFC_RESTRICT pmap, const char *PFC_RESTRICT name)
{
	pfc_rbnode_t	*node = pfc_rbtree_get(pmap, name);

	if (PFC_EXPECT_TRUE(node != NULL)) {
		conf_pnode_t	*pnp = CONF_NODE2PNODE(node);

		return &pnp->cpn_param;
	}

	return NULL;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * conf_pmap_put(conf_pmap_t *PFC_RESTRICT pmap, conf_pnode_t *PFC_RESTRICT pnp)
 *	Register the specified parameter node into parameter map.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
conf_pmap_put(conf_pmap_t *PFC_RESTRICT pmap, conf_pnode_t *PFC_RESTRICT pnp)
{
	return pfc_rbtree_put(pmap, &pnp->cpn_node);
}

#endif	/* _PFC_LIBPFC_UTIL_BUILD */

extern void	pfc_conf_set_errfunc(pfc_conf_err_t func);
extern int	pfc_sysconf_init(pfc_refptr_t *rpath);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_UTIL_CONF_IMPL_H */
