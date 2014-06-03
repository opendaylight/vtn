/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LOG_H
#define	_PFC_LOG_H

/*
 * Common PFC logging interface.
 */

#include <stdio.h>
#include <stdarg.h>
#include <pfc/base.h>
#include <pfc/conf.h>

/*
 * PFC_LOG_IDENT is used as module name to be passed to logging function.
 * If this file is included from the PFC module source, the module name is
 * used as value for PFC_LOG_IDENT.
 * If this file is not included from the PFC module source and PFC_LOG_IDENT
 * is not defined, NULL is used for its value.
 */
#ifdef	PFC_MODULE_BUILD

/* Import PFC_MODULE_THIS_NAME in pfc/module.h. */
#include <pfc/module.h>

/*
 * Ignore PFC_LOG_IDENT on module build.
 * We must use module name as log identifier.
 */
#undef	PFC_LOG_IDENT
#define	__PFC_LOG_IDENT_DEFINED		1

#else	/* !PFC_MODULE_BUILD */

#define	PFC_MODULE_THIS_NAME		NULL
#ifdef	PFC_LOG_IDENT
#define	__PFC_LOG_IDENT_DEFINED		1
#endif	/* PFC_LOG_IDENT */

#endif	/* PFC_MODULE_BUILD */

#ifndef	PFC_LOG_IDENT
#define	PFC_LOG_IDENT			PFC_MODULE_THIS_NAME
#endif	/* !PFC_LOG_IDENT */

#ifndef	__PFC_LOG_IDENT_DEFINED
#define	__PFC_LOG_IDENT_DEFINED		0
#endif	/* !__PFC_LOG_IDENT_DEFINED */

PFC_C_BEGIN_DECL

/*
 * Logging facility.
 */
typedef enum {
	PFC_LOGFACL_DAEMON	= 0,
	PFC_LOGFACL_LOCAL0,
	PFC_LOGFACL_LOCAL1,
	PFC_LOGFACL_LOCAL2,
	PFC_LOGFACL_LOCAL3,
	PFC_LOGFACL_LOCAL4,
	PFC_LOGFACL_LOCAL5,
	PFC_LOGFACL_LOCAL6,
	PFC_LOGFACL_LOCAL7,
} pfc_log_facl_t;

/*
 * Logging level.
 */
typedef enum {
	PFC_LOGLVL_NONE		= -1,
	PFC_LOGLVL_FATAL	= 0,
	PFC_LOGLVL_ERROR,
	PFC_LOGLVL_WARN,
	PFC_LOGLVL_NOTICE,
	PFC_LOGLVL_INFO,
	PFC_LOGLVL_DEBUG,
	PFC_LOGLVL_TRACE,
	PFC_LOGLVL_VERBOSE
} pfc_log_level_t;

/*
 * Current global log level.
 *
 * Remarks:
 *	Don't touch __pfc_log_current_level directly.
 *	Use pfc_log_current_level.
 */
extern const volatile pfc_log_level_t *const	__pfc_log_current_level;

/*
 * Public read-only pointer variable which determines per-module logging level
 * is configured or not.
 *
 * Remarks:
 *	Never touch __pfc_log_modlevel_defined directly.
 *	Use pfc_log_current_level instead.
 */
extern const void *volatile const *const	__pfc_log_modlevel_defined;

#define	__PFC_LOG_MODLEVEL_IS_DEFINED()	(*__pfc_log_modlevel_defined != NULL)

/*
 * Per-module logging configuration.
 */
typedef struct {
	const char	*plm_name;		/* module name */
	pfc_log_level_t	plm_level;		/* logging level */
} pfc_log_modconf_t;

/*
 * Prototype of fatal error handler.
 */
typedef void	(*pfc_log_fatal_t)(void);

/*
 * Logging level configuration.
 */
typedef struct {
	pfc_log_level_t	plvc_level;		/* logging level */
	pfc_log_level_t	plvc_deflevel;		/* default logging level */
} pfc_loglvlcf_t;

/*
 * Logging configuration for initialization.
 */
typedef struct {
	unsigned int	plc_type;		/* logging type */
	pfc_cfblk_t	plc_cfblk;		/* configuration file block */
	const char	*plc_ident;		/* identifier */
	pfc_log_fatal_t	plc_handler;		/* fatal error handler */
	pfc_loglvlcf_t	plc_level;		/* logging level */
	FILE		*plc_output;		/* output stream for early log*/
	pfc_log_facl_t	plc_facility;		/* config: facility */
	uint32_t	plc_rcount;		/* config: rotation count */
	size_t		plc_rsize;		/* config: rotation size */
	const char	*plc_logdir;		/* directory of log file */
	const char	*plc_logpath;		/* path to log file */
	const char	*plc_lvlpath;		/* path to log level file */
} pfc_log_conf_t;

/*
 * Prototypes for PFC daemon.
 */
extern void	pfc_log_init(const char *PFC_RESTRICT ident,
			     FILE *PFC_RESTRICT out, pfc_log_level_t level,
			     pfc_log_fatal_t handler);
extern void	pfc_log_sysinit(pfc_log_conf_t *cfp);
extern void	pfc_log_fini(void);

extern pfc_bool_t	pfc_log_isfatal(void);

extern void	pfc_log_set_fatal_handler(pfc_log_fatal_t handler);
extern int	pfc_log_set_level(pfc_log_level_t level);

extern void	pfc_log_modlevel_init(pfc_cfblk_t block);
extern int	pfc_log_modlevel_set(const char *ident, pfc_log_level_t level);
extern void	pfc_log_modlevel_reset(const char *ident);
extern int	pfc_log_modlevel_copy(pfc_log_modconf_t **levelsp);
extern void	pfc_log_modlevel_free(pfc_log_modconf_t *levels,
				      uint32_t count);

extern void	pfc_logconf_init(pfc_log_conf_t *PFC_RESTRICT cfp,
				 pfc_cfblk_t block,
				 const char *PFC_RESTRICT ident,
				 pfc_log_fatal_t handler);
extern void	pfc_logconf_early(pfc_log_conf_t *PFC_RESTRICT cfp,
				  pfc_cfblk_t block,
				  const char *PFC_RESTRICT ident,
				  FILE *PFC_RESTRICT out, pfc_log_level_t level,
				  pfc_log_fatal_t handler);
extern void	pfc_logconf_setpath(pfc_log_conf_t *PFC_RESTRICT cfp,
				   const char *PFC_RESTRICT base, size_t blen,
				   const char *PFC_RESTRICT dname, size_t dlen,
				   const char *PFC_RESTRICT fname, size_t flen);
extern void	pfc_logconf_setlvlpath(pfc_log_conf_t *PFC_RESTRICT cfp,
				   const char *PFC_RESTRICT base, size_t blen,
				   const char *PFC_RESTRICT fname, size_t flen);
extern void	pfc_logconf_setrotate(pfc_log_conf_t *PFC_RESTRICT cfp,
				      uint32_t rcount, size_t rsize);
extern void	pfc_logconf_setlevel(pfc_log_conf_t *cfp, pfc_log_level_t lvl);
extern void	pfc_logconf_setdeflevel(pfc_log_conf_t *cfp,
					pfc_log_level_t lvl);
extern void	pfc_logconf_setsyslog(pfc_log_conf_t *PFC_RESTRICT cfp,
				      pfc_bool_t sysonly);
extern void	pfc_logconf_setfacility(pfc_log_conf_t *cfp,
					pfc_log_facl_t facility);

/*
 * Function which returns current log level for the specified module.
 * Never call this directly.
 */
extern pfc_log_level_t	__pfc_log_current_modlevel(const char *modname);

/*
 * Common logging function using va_list.
 * Never call this directly!
 */
extern void	__pfc_log_common_v(pfc_log_level_t level,
				   const char *PFC_RESTRICT modname,
				   const char *PFC_RESTRICT format, va_list ap)
	PFC_FATTR_PRINTFLIKE(3, 0);

/*
 * Current log level.
 */
#define	pfc_log_current_level						\
	((__PFC_LOG_IDENT_DEFINED == 0 || !__PFC_LOG_MODLEVEL_IS_DEFINED()) \
	 ? *__pfc_log_current_level					\
	 : __pfc_log_current_modlevel(PFC_LOG_IDENT))

#define	__PFC_LOG_COMMON_V(level, format, ap)				\
	do {								\
		if (PFC_EXPECT_TRUE((level) <= pfc_log_current_level)) { \
			__pfc_log_common_v(level, PFC_LOG_IDENT,	\
					   format, ap);			\
		}							\
	} while (0)

#define	__PFC_LOG_COMMON_DBG_V(level, format, ap)			\
	do {								\
		if (PFC_EXPECT_FALSE((level) <= pfc_log_current_level)) { \
			__pfc_log_common_v(level, PFC_LOG_IDENT,	\
					   format, ap);			\
		}							\
	} while (0)

/*
 * Public interface of the PFC logging system using va_list.
 */
#define	pfc_log_fatal_v(format, ap)					\
	__PFC_LOG_COMMON_V(PFC_LOGLVL_FATAL, format, ap)
#define	pfc_log_error_v(format, ap)					\
	__PFC_LOG_COMMON_V(PFC_LOGLVL_ERROR, format, ap)
#define	pfc_log_warn_v(format, ap)					\
	__PFC_LOG_COMMON_V(PFC_LOGLVL_WARN, format, ap)
#define	pfc_log_notice_v(format, ap)					\
	__PFC_LOG_COMMON_V(PFC_LOGLVL_NOTICE, format, ap)
#define	pfc_log_info_v(format, ap)					\
	__PFC_LOG_COMMON_V(PFC_LOGLVL_INFO, format, ap)
#define	pfc_log_debug_v(format, ap)					\
	__PFC_LOG_COMMON_DBG_V(PFC_LOGLVL_DEBUG, format, ap)
#define	pfc_log_trace_v(format, ap)					\
	__PFC_LOG_COMMON_DBG_V(PFC_LOGLVL_TRACE, format, ap)

/*
 * pfc_log_verbose_v() is enabled only if PFC_VERBOSE_DEBUG is defined.
 */
#ifdef	PFC_VERBOSE_DEBUG
#define	pfc_log_verbose_v(format, ap)					\
	__PFC_LOG_COMMON_DBG_V(PFC_LOGLVL_VERBOSE, format, ap)
#else	/* !PFC_VERBOSE_DEBUG */
#define	pfc_log_verbose_v(format, ap)	((void)0)
#endif	/* PFC_VERBOSE_DEBUG */

#ifdef	__GNUC__
#define	__PFC_LOG_GNUC		1
#else	/* !__GNUC__ */
#undef	__PFC_LOG_GNUC
#endif	/* __GNUC__ */

/*
 * Common logging function.
 * Never call this directly!
 */
extern void	__pfc_log_common(pfc_log_level_t level, const char *modname,
				 const char *format, ...)
	PFC_FATTR_PRINTFLIKE(3, 4);

#ifdef	__PFC_LOG_GNUC

#define	__PFC_LOG_COMMON(level, format, ...)				\
	do {								\
		if (PFC_EXPECT_TRUE((level) <= pfc_log_current_level)) { \
			__pfc_log_common(level, PFC_LOG_IDENT,		\
					 format, ##__VA_ARGS__);	\
		}							\
	} while (0)

#define	__PFC_LOG_COMMON_DBG(level, format, ...)			\
	do {								\
		if (PFC_EXPECT_FALSE((level) <= pfc_log_current_level)) { \
			__pfc_log_common(level, PFC_LOG_IDENT,		\
					 format, ##__VA_ARGS__);	\
		}							\
	} while (0)

/*
 * Logging interfaces for all PFC components.
 * Use GCC styled macro with variable number of arguments.
 */
#define	pfc_log_fatal(format, ...)					\
	__PFC_LOG_COMMON(PFC_LOGLVL_FATAL, format, ##__VA_ARGS__)
#define	pfc_log_error(format, ...)					\
	__PFC_LOG_COMMON(PFC_LOGLVL_ERROR, format, ##__VA_ARGS__)
#define	pfc_log_warn(format, ...)					\
	__PFC_LOG_COMMON(PFC_LOGLVL_WARN, format, ##__VA_ARGS__)
#define	pfc_log_notice(format, ...)					\
	__PFC_LOG_COMMON(PFC_LOGLVL_NOTICE, format, ##__VA_ARGS__)
#define	pfc_log_info(format, ...)					\
	__PFC_LOG_COMMON(PFC_LOGLVL_INFO, format, ##__VA_ARGS__)
#define	pfc_log_debug(format, ...)					\
	__PFC_LOG_COMMON_DBG(PFC_LOGLVL_DEBUG, format, ##__VA_ARGS__)
#define	pfc_log_trace(format, ...)					\
	__PFC_LOG_COMMON_DBG(PFC_LOGLVL_TRACE, format, ##__VA_ARGS__)

/*
 * pfc_log_verbose() is enabled only if PFC_VERBOSE_DEBUG is defined.
 */
#ifdef	PFC_VERBOSE_DEBUG
#define	pfc_log_verbose(format, ...)					\
	__PFC_LOG_COMMON_DBG(PFC_LOGLVL_VERBOSE, format, ##__VA_ARGS__)
#else	/* !PFC_VERBOSE_DEBUG */
#define	pfc_log_verbose(format, ...)	((void)0)
#endif	/* PFC_VERBOSE_DEBUG */

#else	/* !__PFC_LOG_GNUC */

/*
 * Define logging interfaces which takes printf(3) like arguments
 * as inline function.
 */
static inline void PFC_FATTR_PRINTFLIKE(1, 2)
pfc_log_fatal(const char *format, ...)
{
	if (PFC_EXPECT_TRUE(pfc_log_current_level >= PFC_LOGLVL_FATAL)) {
		va_list	ap;

		va_start(ap, format);
		__pfc_log_common_v(PFC_LOGLVL_FATAL, PFC_LOG_IDENT,
				   format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
pfc_log_error(const char *format, ...)
{
	if (PFC_EXPECT_TRUE(pfc_log_current_level >= PFC_LOGLVL_ERROR)) {
		va_list	ap;

		va_start(ap, format);
		__pfc_log_common_v(PFC_LOGLVL_ERROR, PFC_LOG_IDENT,
				   format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
pfc_log_warn(const char *format, ...)
{
	if (PFC_EXPECT_TRUE(pfc_log_current_level >= PFC_LOGLVL_WARN)) {
		va_list	ap;

		va_start(ap, format);
		__pfc_log_common_v(PFC_LOGLVL_WARN, PFC_LOG_IDENT,
				   format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
pfc_log_notice(const char *format, ...)
{
	if (PFC_EXPECT_TRUE(pfc_log_current_level >= PFC_LOGLVL_NOTICE)) {
		va_list	ap;

		va_start(ap, format);
		__pfc_log_common_v(PFC_LOGLVL_NOTICE, PFC_LOG_IDENT,
				   format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
pfc_log_info(const char *format, ...)
{
	if (PFC_EXPECT_TRUE(pfc_log_current_level >= PFC_LOGLVL_INFO)) {
		va_list	ap;

		va_start(ap, format);
		__pfc_log_common_v(PFC_LOGLVL_INFO, PFC_LOG_IDENT,
				   format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
pfc_log_debug(const char *format, ...)
{
	if (PFC_EXPECT_FALSE(pfc_log_current_level >= PFC_LOGLVL_DEBUG)) {
		va_list	ap;

		va_start(ap, format);
		__pfc_log_common_v(PFC_LOGLVL_DEBUG, PFC_LOG_IDENT,
				   format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
pfc_log_trace(const char *format, ...)
{
	if (PFC_EXPECT_FALSE(pfc_log_current_level >= PFC_LOGLVL_TRACE)) {
		va_list	ap;

		va_start(ap, format);
		__pfc_log_common_v(PFC_LOGLVL_TRACE, PFC_LOG_IDENT,
				   format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
pfc_log_verbose(const char *format, ...)
{
#ifdef	PFC_VERBOSE_DEBUG
	if (PFC_EXPECT_FALSE(pfc_log_current_level >= PFC_LOGLVL_VERBOSE)) {
		va_list	ap;

		va_start(ap, format);
		__pfc_log_common_v(PFC_LOGLVL_VERBOSE, PFC_LOG_IDENT,
				   format, ap);
		va_end(ap);
	}
#endif	/* PFC_VERBOSE_DEBUG */
}

#endif	/* __PFC_LOG_GNUC */

/*
 * Common macros to declare burst logging block for normal PFC log.
 * Never use these macros directly.
 */

#ifdef	PFC_VERBOSE_DEBUG
#define	__PFC_LOG_BURST_LEVEL_TEST_DBG(level)			\
	(PFC_EXPECT_FALSE((level) <= pfc_log_current_level))
#else	/* !PFC_VERBOSE_DEBUG */
#define	__PFC_LOG_BURST_LEVEL_TEST_DBG(level)			\
	(PFC_EXPECT_FALSE((level) != PFC_LOGLVL_VERBOSE &&	\
			  (level) <= pfc_log_current_level))
#endif	/* PFC_VERBOSE_DEBUG */

#ifdef	__PFC_LOG_GNUC

#define	__PFC_LOG_BURST_PUSH(level)		((void)0)
#define	__PFC_LOG_BURST_POP(level)		((void)0)

#else	/* !__PFC_LOG_GNUC */

extern pfc_log_level_t	__pfc_log_burst_set(pfc_log_level_t level);

#define	__PFC_LOG_BURST_PUSH(level)				\
	pfc_log_level_t	__oldlvl = __pfc_log_burst_set(level)
#define	__PFC_LOG_BURST_POP()					\
	(void)__pfc_log_burst_set(__oldlvl)

#endif	/* __PFC_LOG_GNUC */

#define	__PFC_LOG_BURST_BEGIN(level)					\
	do {								\
		const pfc_log_level_t	__lvl = (level);		\
		if (PFC_EXPECT_TRUE(__lvl <= pfc_log_current_level)) {	\
			__PFC_LOG_BURST_PUSH(__lvl);

#define	__PFC_LOG_BURST_BEGIN_DBG(level)			\
	do {							\
		const pfc_log_level_t	__lvl = (level);	\
		if (__PFC_LOG_BURST_LEVEL_TEST_DBG(__lvl)) {	\
			__PFC_LOG_BURST_PUSH(__lvl);

#define	__PFC_LOG_BURST_END()					\
			__PFC_LOG_BURST_POP();			\
		}						\
	} while (0)

/*
 * Public APIs to record multiple logs at a time.
 *
 * void	pfc_log_burst_error_begin(void)
 * void	pfc_log_burst_warn_begin(void)
 * void	pfc_log_burst_notice_begin(void)
 * void	pfc_log_burst_info_begin(void)
 * void	pfc_log_burst_debug_begin(void)
 * void	pfc_log_burst_trace_begin(void)
 * void	pfc_log_burst_verbose_begin(void)
 *	Declare beginning of burst logging block for the normal PFC log.
 *
 * void	pfc_log_burst_end(void)
 *	Enclose burst logging block for the normal PFC log.
 *
 * void pfc_log_burst_write(const char *format, ...)
 *	Record the specified log message in the normal PFC log.
 *
 * pfc_log_burst_XXX_begin() and pfc_log_burst_end() must be paired within the
 * same function, and at the same lexical nesting level.
 * pfc_log_burst_write() must be called from the burst logging block.
 * Note that the burst logging block is valid only in the same function with
 * pfc_log_burst_XXX_begin().
 *
 * Logging level for log messages recorded by the call of pfc_log_burst_write()
 * is determined by pfc_log_burst_XXX_begin(). For instance, the call of
 * pfc_log_burst_write() in the burst logging block started by
 * pfc_log_burst_notice_begin() will record notice log.
 *
 * Remarks:
 *	- Checking the current logging level is done by the call of
 *	  pfc_log_burst_XXX_begin(). If the current logging level is lower
 *	  than the specified level, any code in the burst logging block will
 *	  be invalidated. For instance, any code between the call of
 *	  pfc_log_burst_debug_begin() and pfc_log_burst_end() will be
 *	  invalidated unless the current logging level is configured as
 *	  "debug" or "verbose".
 *
 *	- If PFC_VERBOSE_DEBUG is not defined, any code between the call of
 *	  pfc_log_burst_verbose_begin() and pfc_log_burst_end() will be
 *	  invalidated.
 */
#define	pfc_log_burst_error_begin()				\
	__PFC_LOG_BURST_BEGIN(PFC_LOGLVL_ERROR)
#define	pfc_log_burst_warn_begin()				\
	__PFC_LOG_BURST_BEGIN(PFC_LOGLVL_WARN)
#define	pfc_log_burst_notice_begin()				\
	__PFC_LOG_BURST_BEGIN(PFC_LOGLVL_NOTICE)
#define	pfc_log_burst_info_begin()				\
	__PFC_LOG_BURST_BEGIN(PFC_LOGLVL_INFO)
#define	pfc_log_burst_debug_begin()				\
	__PFC_LOG_BURST_BEGIN_DBG(PFC_LOGLVL_DEBUG)
#define	pfc_log_burst_trace_begin()				\
	__PFC_LOG_BURST_BEGIN_DBG(PFC_LOGLVL_TRACE)
#define	pfc_log_burst_verbose_begin()				\
	__PFC_LOG_BURST_BEGIN_DBG(PFC_LOGLVL_VERBOSE)

#define	pfc_log_burst_end()		__PFC_LOG_BURST_END()

#ifdef	__PFC_LOG_GNUC

#define	pfc_log_burst_write(format, ...)				\
	__pfc_log_common(__lvl, PFC_LOG_IDENT, format, ##__VA_ARGS__)

#else	/* !__PFC_LOG_GNUC */

extern void	__pfc_log_burst_write_v(const char *modname,
					const char *format, va_list ap)
	PFC_FATTR_PRINTFLIKE(2, 0);

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
pfc_log_burst_write(const char *format, ...)
{
	va_list	ap;

	va_start(ap, format);
	__pfc_log_burst_write_v(PFC_LOG_IDENT, format, ap);
	va_end(ap);
}

#endif	/* __PFC_LOG_GNUC */

PFC_C_END_DECL

#endif	/* !_PFC_LOG_H */
