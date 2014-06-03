/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * log_syslog.c - PFC logging system using syslog(3).
 */

#include <syslog.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sysexits.h>
#include <sys/mman.h>
#include <pfc/log.h>
#include <pfc/conf.h>
#include <pfc/clock.h>
#include <pfc/util.h>
#include <pfc/synch.h>
#include <pfc/atomic.h>
#include <pfc/debug.h>
#include <pfc/refptr.h>
#include <pfc/rbtree.h>
#include "tid_impl.h"
#include "log_impl.h"

#ifndef	PFC_HAVE_VSYSLOG
#error	vsyslog(3) is required.
#endif	/* !PFC_HAVE_VSYSLOG */

/*
 * Maximum length of log message.
 */
#define	PFC_LOG_MAX_SIZE	256U

/*
 * Access permission bits for log file.
 */
#define	LOG_FILE_PERM		(S_IRUSR | S_IWUSR)

/*
 * Access permission bits for log directory.
 */
#define LOG_DIR_PERM		(S_IRWXU)

/*
 * Access permission bits for loglevel file.
 */
#define	LOG_LEVELFILE_PERM	(S_IRUSR | S_IWUSR)

/*
 * Internal log buffer.
 */
static char	log_buffer[PFC_LOG_MAX_SIZE + 1];

/*
 * Mutex for the PFC logging system.
 */
static pfc_mutex_t	log_mutex = PFC_MUTEX_INITIALIZER;

#define	PFC_LOG_LOCK()		pfc_mutex_lock(&log_mutex)
#define	PFC_LOG_UNLOCK()	pfc_mutex_unlock(&log_mutex)

/*
 * Prototype for actual logging function.
 */
typedef void	(*log_func_t)(int pri, const char *format, va_list ap);

/*
 * Prototype for function to shutdown PFC logging.
 */
typedef void	(*log_close_t)(void);

/*
 * Attributes associated with the log level.
 */
typedef struct {
	const char	*l_name;	/* string representation of the level */
	int		l_priority;	/* syslog(3) priority. */
} log_lvl_t;

typedef const log_lvl_t		log_clvl_t;

#define	PFC_LOG_LVL_DECL(name, pri)		\
	{					\
		.l_name		= (name),	\
		.l_priority	= (pri),	\
	}

/*
 * This table keeps pairs of PFC log level and its attributes.
 */
static const log_lvl_t	log_level_attr[] = {
	PFC_LOG_LVL_DECL("FATAL", LOG_EMERG),		/* PFC_LOGLVL_FATAL */
	PFC_LOG_LVL_DECL("ERROR", LOG_ERR),		/* PFC_LOGLVL_ERROR */
	PFC_LOG_LVL_DECL("WARNING", LOG_WARNING),	/* PFC_LOGLVL_WARN */
	PFC_LOG_LVL_DECL("NOTICE", LOG_NOTICE),		/* PFC_LOGLVL_NOTICE */
	PFC_LOG_LVL_DECL("INFO", LOG_INFO),		/* PFC_LOGLVL_INFO */
	PFC_LOG_LVL_DECL("DEBUG", LOG_DEBUG),		/* PFC_LOGLVL_DEBUG */
	PFC_LOG_LVL_DECL("TRACE", LOG_DEBUG),		/* PFC_LOGLVL_TRACE */
	PFC_LOG_LVL_DECL("VERBOSE", LOG_DEBUG),		/* PFC_LOGLVL_VERBOSE */
};

/*
 * Supported log facilities.
 */
typedef struct {
	const char	*f_name;	/* symbolic name of facility */
	int		f_facility;	/* syslog(3) facility */
} log_facl_t;

#define	PFC_LOG_FACL_DECL(name, facl)		\
	{					\
		.f_name		= (name),	\
		.f_facility	= (facl),	\
	}

#define	PFC_LOG_FACL_LOCAL_DECL(id)			\
	PFC_LOG_FACL_DECL("LOCAL" #id, LOG_LOCAL##id)

/*
 * Index of log_facility_attr index must equal pfc_log_facl_t value.
 */
static const log_facl_t log_facility_attr[] = {
	PFC_LOG_FACL_DECL("DAEMON", LOG_DAEMON),
	PFC_LOG_FACL_LOCAL_DECL(0),
	PFC_LOG_FACL_LOCAL_DECL(1),
	PFC_LOG_FACL_LOCAL_DECL(2),
	PFC_LOG_FACL_LOCAL_DECL(3),
	PFC_LOG_FACL_LOCAL_DECL(4),
	PFC_LOG_FACL_LOCAL_DECL(5),
	PFC_LOG_FACL_LOCAL_DECL(6),
	PFC_LOG_FACL_LOCAL_DECL(7),
};

/*
 * Logging level.
 */
static pfc_log_level_t	log_level = LOG_LEVEL_DEFAULT;

/*
 * Public read-only variable to obtain current log level.
 */
const volatile pfc_log_level_t *const	__pfc_log_current_level =
	(const volatile pfc_log_level_t *const)&log_level;

/*
 * Logging level per module.
 */
typedef struct {
	const char	*lml_name;	/* module name */
	pfc_log_level_t	lml_level;	/* logging level for this module */
	pfc_rbnode_t	lml_node;	/* Red-Black tree node */
} log_modlevel_t;

#define	LOG_NODE2MODLEVEL(node)					\
	PFC_CAST_CONTAINER((node), log_modlevel_t, lml_node)

typedef struct {
	pfc_rbtree_t	lmc_tree;	/* Red-Black tree */
	uint32_t	lmc_count;	/* number of configurations */
} log_modconf_t;

/*
 * File stream for log destination.
 */
static FILE	*log_output;

/*
 * Separator of log_modlevel parameter value.
 */
#define	LOG_MODLEVEL_SEP	':'

/*
 * Configuration parameter name which keeps string array of per-module logging
 * level.
 */
static const char	conf_log_modlevel[] = "log_modlevel";

/*
 * File path to save log levels.
 */
static const char	*log_lvlpath;

/*
 * Format of loglevel file.
 */
typedef struct {
	char		lvf_level;	/* log level */
} PFC_ATTR_PACKED log_lvlfile_t;

PFC_TYPE_SIZE_ASSERT(log_lvlfile_t, 1);

/*
 * Logging attributes per type.
 */
typedef struct {
	const char	*la_name;		/* name of logging type */
	const char	*la_lvlcfname;		/* level parameter name */
	pfc_log_level_t	*la_levelp;		/* pointer to current level */
	pfc_log_level_t	*la_initlevelp;		/* pointer to initial level */
	uint32_t	la_lvloff;		/* offset to log_lvlfile_t */
} log_attr_t;

typedef const log_attr_t	log_cattr_t;

/*
 * Context to access loglevel file.
 */
typedef struct {
	log_lvlfile_t	*lvfc_level;	/* contents of loglevel file */
	pfc_bool_t	lvfc_reset;	/* true if the file was reset */

	/* Buffer for error message. */
	char		lvfc_errmsg[PFC_LOG_MAX_SIZE + 1];
} log_lvlfctx_t;

/*
 * Character in loglevel file which means no level is saved.
 */
#define	LOG_LVLCHAR_NONE		'n'

/*
 * Update loglevel character in log_lvlfile_t pointed by `lfp'.
 * If PFC_LOGLVL_NONE is specified to `level', the level will be reset.
 */
#define	LOG_LVLCHAR_UPDATE(lattr, lfp, level)				\
	do {								\
		char	*__ptr = (char *)(lfp) + (lattr)->la_lvloff;	\
									\
		PFC_ASSERT((lattr)->la_lvloff < sizeof(log_lvlfile_t));	\
		PFC_ASSERT((lfp) != (log_lvlfile_t *)MAP_FAILED);	\
									\
		if ((level) == PFC_LOGLVL_NONE) {			\
			*__ptr = LOG_LVLCHAR_NONE;			\
		}							\
		else {							\
			PFC_ASSERT_INT(log_level_is_valid(lattr, level), 0); \
			*__ptr = (level) + '0';				\
		}							\
	} while (0)

/*
 * Convert loglevel file character into actual logging level.
 * Note that the LOG_LVLCHAR_NONE must not be specified to 'c'.
 */
#define	LOG_CHAR2LVL(c)			((c) - '0')

/*
 * Determine whether fatal log message is recorded or not.
 */
static pfc_bool_t	log_isfatal = PFC_FALSE;

/*
 * Internal prototypes.
 */
static void	pfc_log_init_impl(const pfc_log_conf_t *PFC_RESTRICT cfp);
static int	pfc_log_setup_directory(const char *path, int *errloc);
static void	pfc_log_common_impl(pfc_log_level_t level, const char *modname,
				    const char *format, va_list ap);
static void	pfc_log_file(int pri, const char *format, va_list ap);
static void	pfc_log_file_rotate(void);
static void	pfc_log_file_close(void);
static const char	*pfc_log_create_format(const log_lvl_t *lvl,
					       const char *modname,
					       const char *format);
static const char	 *pfc_log_timestamp(char *buffer, size_t bufsize);

static int	log_file_open(FILE **fpp, int *errloc);
static int	log_file_create(FILE **fpp, int *errloc);
static int	log_do_rotation(const char *path, int *errloc);
static int	log_file_rotation_init(const char *path,
				       uint32_t rcount, size_t rsize);
static void	log_file_rotation_fini(void);

static pfc_log_level_t	log_level_name_is_valid(log_cattr_t *lattr,
						const char *name);

static int	log_level_is_valid(log_cattr_t *lattr, pfc_log_level_t level);
static void	log_level_init(log_cattr_t *lattr, pfc_cfblk_t block,
			       const pfc_loglvlcf_t *lvcf,
			       pfc_log_level_t newlvl, const char **invnamep);
static void	log_level_init_default(log_cattr_t *lattr, pfc_cfblk_t block,
				       const pfc_loglvlcf_t *lvcf,
				       const char **invnamep);
static int	log_level_set(log_cattr_t *lattr, pfc_log_level_t level);
static void	log_lvlfctx_init(log_lvlfctx_t *fctx);
static void	log_lvlfctx_fini(log_lvlfctx_t *fctx);
static int	log_lvlfile_open(const char *path, log_lvlfctx_t *fctx,
				 pfc_bool_t do_create);
static int	log_lvlfile_create(const char *path, log_lvlfctx_t *fctx);

static void	log_load_loglevel(log_lvlfctx_t *fctx, pfc_log_level_t *lvlp);
static int	log_load_loglevel_impl(log_lvlfctx_t *fctx, log_cattr_t *lattr,
				       const char *path, char lc,
				       pfc_log_level_t *lvlp);
static int	log_save_loglevel(log_cattr_t *lattr, pfc_log_level_t level,
				  log_lvlfctx_t *fctx);

static void	log_modlevel_init(pfc_cfblk_t block, int index);
static int	log_modlevel_set(const char *modname, pfc_log_level_t level);
static void	log_modlevel_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);

static pfc_cptr_t	log_modlevel_getkey(pfc_rbnode_t *node);
static pfc_log_level_t	log_current_level(const char *modname);

/*
 * Per-module logging configuration.
 */
static log_modconf_t	log_modules = {
	.lmc_tree	= PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp,
						 log_modlevel_getkey),
	.lmc_count	= 0,
};

/*
 * Public read-only pointer variable which determines per-module logging level
 * is configured or not.
 */
const void *volatile const *const	__pfc_log_modlevel_defined =
	(const void *volatile const *const)&log_modules.lmc_tree.rb_root;

/* This macro must be redefined here in order to avoid type punning. */
#undef	__PFC_LOG_MODLEVEL_IS_DEFINED
#define	__PFC_LOG_MODLEVEL_IS_DEFINED()		\
	(log_modules.lmc_tree.rb_root != NULL)

/*
 * Logging function.
 */
static log_func_t	log_function = pfc_log_file;

/*
 * Shutdown function.
 */
static log_close_t	log_close = pfc_log_file_close;

/*
 * Fatal error handler.
 */
static pfc_log_fatal_t	log_fatal_handler;

/*
 * System log file and rotation information.
 */
typedef struct log_rotation {
	pfc_refptr_t	*r_path;	/* Path name for log destination */
	char		*r_oldpath;	/* Path name buffer for rotation */
	char		*r_newpath;
	size_t		r_pathsize;	/* Size of path name buffer */
	uint32_t	r_count;	/* Rotation count */
	size_t		r_size;		/* Rotation size */
} log_rotation_t;

static log_rotation_t	*log_rotate;

/*
 * Initial logging level.
 */
static pfc_log_level_t	log_initlevel = LOG_LEVEL_DEFAULT;

/*
 * Declare logging attributes per type.
 */
#define	LOG_ATTR_DECL(name, prefix, field)				\
	{								\
		.la_name	= (name),				\
		.la_lvlcfname	= #prefix "_level",			\
		.la_levelp	= &(prefix##_level),			\
		.la_initlevelp	= &(prefix##_initlevel),		\
		.la_lvloff	= offsetof(log_lvlfile_t, field),	\
	}

static log_cattr_t	log_attributes[] = {
	/* For trace log */
	LOG_ATTR_DECL("trace log", log, lvf_level),
};

#define	LOG_ATTR_TRACE		(&log_attributes[0])

/*
 * Current dumped size.
 */
static size_t		log_dump_size;

/*
 * Determine whether syslog is opened or not.
 */
static pfc_bool_t	log_syslog_open = PFC_FALSE;

/*
 * Macros to handle per-module logging levels.
 * These requires to be used with holding the log lock.
 */
#define	LOG_MODLEVEL_PUT(lmp)						\
	do {								\
		PFC_ASSERT_INT(pfc_rbtree_put(&log_modules.lmc_tree,	\
					      &(lmp)->lml_node), 0);	\
		log_modules.lmc_count++;				\
	} while (0)

#define	LOG_MODLEVEL_GET(modname)				\
	pfc_rbtree_get(&log_modules.lmc_tree, (modname))

#define	LOG_MODLEVEL_REMOVE(modname, removed)				\
	do {								\
		(removed) = pfc_rbtree_remove(&log_modules.lmc_tree,	\
					      (modname));		\
		if ((removed) != NULL) {				\
			PFC_ASSERT(log_modules.lmc_count > 0);		\
			log_modules.lmc_count--;			\
		}							\
	} while (0)

#define	LOG_MODLEVEL_CLEAR()						\
	do {								\
		pfc_rbtree_clear(&log_modules.lmc_tree,			\
				 log_modlevel_dtor, NULL);		\
		log_modules.lmc_count = 0;				\
	} while (0)

#define	LOG_MODLEVEL_NEXT(node)				\
	pfc_rbtree_next(&log_modules.lmc_tree, (node))

/*
 * static inline const log_facl_t PFC_FATTR_ALWAYS_INLINE *
 * log_facility_get(pfc_log_facl_t facility)
 *	Get attributes of logging facility associated with the internal
 *	logging facility specified by `facility'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to log_facl_t associated
 *	with `facility' is returned.
 *	NULL is returned if `facility' is invalid.
 */
static inline const log_facl_t PFC_FATTR_ALWAYS_INLINE *
log_facility_get(pfc_log_facl_t facility)
{
	if (PFC_EXPECT_FALSE((uint32_t)facility >=
			     PFC_ARRAY_CAPACITY(log_facility_attr))) {
		return NULL;
	}

	return &log_facility_attr[facility];
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_log_libinit(void)
 *	Early initialization of PFC logging architecture.
 */
void PFC_ATTR_HIDDEN
pfc_log_libinit(void)
{
	log_output = stderr;
}

/*
 * void
 * pfc_log_init(const char *PFC_RESTRICT ident, FILE *PFC_RESTRICT out,
 *		pfc_log_level_t level, pfc_log_fatal_t handler)
 *	Initialize the PFC logging system.
 *	This function is called once by the PFC daemon.
 *
 *	`out' is the file stream to dump logs.
 *	`level' determines the logging level. If PFC_LOGLVL_NONE is specified,
 *	the logging level is determined by the system.
 */
void
pfc_log_init(const char *PFC_RESTRICT ident, FILE *PFC_RESTRICT out,
	     pfc_log_level_t level, pfc_log_fatal_t handler)
{
	pfc_log_conf_t	logconf;

	pfc_logconf_early(&logconf, PFC_CFBLK_INVALID,
			  ident, out, level, handler);
	pfc_log_sysinit(&logconf);
}

/*
 * void
 * pfc_log_sysinit(pfc_log_conf_t *PFC_RESTRICT cfp)
 *	Initialize the PFC logging system.
 *	This function is called once by the PFC daemon.
 *
 *	`plc_type' in `cfp' determines the log destination.
 *	- LOG_TYPE_STREAM
 *		All logs are dumped to the file stream specified by the caller.
 *		This type is used for dumping early logs to the stderr.
 *	- LOG_TYPE_SYSLOG
 *		All logs are sent to syslog.
 *	- LOG_TYPE_FILE
 *		pfc_log_xxx() is dumped to the specified log file,
 */
void
pfc_log_sysinit(pfc_log_conf_t *PFC_RESTRICT cfp)
{
	FILE	*fp = NULL;
	int	err, errloc = 0;

	PFC_LOG_LOCK();

	if (PFC_EXPECT_FALSE(log_syslog_open)) {
		/*
		 * syslog() is already opened.
		 * We can't change log configuration.
		 */
		PFC_LOG_UNLOCK();
		goto out;
	}

	if (cfp->plc_type == LOG_TYPE_FILE) {
		PFC_ASSERT(cfp->plc_logdir != NULL);
		PFC_ASSERT(cfp->plc_logpath != NULL);

		/* Setup logging directory. */
		err = pfc_log_setup_directory(cfp->plc_logdir, &errloc);
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_LOG_UNLOCK();
			fprintf(stderr, "%s: Failed to setup log: %d: %s\n",
				cfp->plc_logdir, errloc, strerror(err));
			exit(EX_CANTCREAT);
			/* NOTREACHED */
		}

		err = log_file_rotation_init(cfp->plc_logpath,
					     cfp->plc_rcount, cfp->plc_rsize);
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_LOG_UNLOCK();
			fputs("Failed to initialize log rotation.\n", stderr);
			exit(EX_TEMPFAIL);
			/* NOTREACHED */
		}

		/* Open log file. */
		err = log_file_open(&fp, &errloc);
		if (PFC_EXPECT_FALSE(err != 0)) {
			log_file_rotation_fini();
			PFC_LOG_UNLOCK();
			fprintf(stderr, "%s: Failed to open log file: %d: "
				"%s\n", cfp->plc_logpath, errloc,
				strerror(err));
			exit(EX_CANTCREAT);
			/* NOTREACHED */
		}
		cfp->plc_output = fp;
	}

	/* Set log level file path. */
	if (log_lvlpath != NULL) {
		free((void *)log_lvlpath);
	}

	log_lvlpath = cfp->plc_lvlpath;
	cfp->plc_lvlpath = NULL;

	pfc_log_init_impl(cfp);

out:
	pfc_logconf_destroy(cfp);
}

/*
 * static void
 * pfc_log_init_impl(const pfc_log_conf_t *PFC_RESTRICT cfp)
 *	Initialize the PFC logging system.
 *
 * Remarks:
 *	This function must be called with holding the PFC log system lock.
 *	It is always released on return.
 */
static void
pfc_log_init_impl(const pfc_log_conf_t *PFC_RESTRICT cfp)
{
	pfc_log_level_t	level = cfp->plc_level.plvc_level;
	pfc_log_level_t	new_level = PFC_LOGLVL_NONE;
	pfc_cfblk_t	block = cfp->plc_cfblk;
	log_lvlfctx_t	fctx;
	const char	*inv_level;

	PFC_ASSERT(level >= PFC_LOGLVL_NONE &&
		   level < (int)PFC_ARRAY_CAPACITY(log_level_attr));
	PFC_ASSERT(!log_syslog_open);

	/* Determine logging level. */
	log_lvlfctx_init(&fctx);
	if (level == PFC_LOGLVL_NONE) {
		/* Use log levels defined in the loglevel file. */
		log_load_loglevel(&fctx, &new_level);
	}

	log_level_init(LOG_ATTR_TRACE, block, &cfp->plc_level, new_level,
		       &inv_level);

	log_output = cfp->plc_output;
	log_fatal_handler = cfp->plc_handler;

	if (cfp->plc_type == LOG_TYPE_SYSLOG ||
	    cfp->plc_type == LOG_TYPE_FILE) {
		const log_facl_t	*fp;

		log_syslog_open = PFC_TRUE;

		/* Determine logging facility. */
		fp = log_facility_get(cfp->plc_facility);
		PFC_ASSERT(fp != NULL);

		/* Open connection to syslog. */
		openlog(cfp->plc_ident, LOG_PID | LOG_NDELAY, fp->f_facility);

		/* Use vsyslog(3) to record logs. */
		if (cfp->plc_type == LOG_TYPE_SYSLOG) {
			log_function = vsyslog;
			log_close = closelog;
		}
	}

	PFC_LOG_UNLOCK();
	log_lvlfctx_fini(&fctx);

	if (PFC_EXPECT_FALSE(inv_level != NULL)) {
		pfc_log_warn("Invalid logging level: %s", inv_level);
	}
}

/*
 * void
 * pfc_log_fini(void)
 *	Shutdown the PFC logging system.
 *	This function is called by the PFC daemon when it is going to quit.
 *
 *	Once this function is called, configuration related to logging output
 *	is reset to initial state.
 */
void
pfc_log_fini(void)
{
	PFC_LOG_LOCK();

	/* Close logging output files. */
	(*log_close)();
	if (log_close != closelog) {
		closelog();
	}

	/* Finalize log file rotation. */
	log_file_rotation_fini();

	/* Clear per-module logging configuration. */
	LOG_MODLEVEL_CLEAR();

	/* Reset log level file path. */
	if (log_lvlpath != NULL) {
		free((void *)log_lvlpath);
		log_lvlpath = NULL;
	}

	/* Reset logging output configuration. */
	log_output = stderr;
	log_function = pfc_log_file;
	log_close = pfc_log_file_close;
	log_fatal_handler = NULL;
	log_rotate = NULL;
	log_dump_size = 0;
	log_syslog_open = PFC_FALSE;

	PFC_LOG_UNLOCK();
}

/*
 * static int
 * pfc_log_setup_directory(const char *path, int *errloc)
 *	Set up log directory.
 *	The caller must specify path to the log directory.
 */
static int
pfc_log_setup_directory(const char *path, int *errloc)
{
	struct stat	sbuf;
	int		err;

	if (lstat(path, &sbuf) == 0) {
		if (S_ISDIR(sbuf.st_mode)) {
			/*
			 * Ensure that the directory has safe permission.
			 */
			if (chmod(path, LOG_DIR_PERM) != 0) {
				*errloc = LOG_ERRLOC_DIR_CHMOD;
				return errno;
			}

			return 0;
		}

		/* Remove existing file. */
		(void)unlink(path);
	}
	else if (errno != ENOENT) {
		*errloc = LOG_ERRLOC_DIR_LSTAT;
		return errno;
	}

	/* Create directory. */
	err = pfc_mkdir(path, LOG_DIR_PERM);
	if (PFC_EXPECT_FALSE(err != 0)) {
		*errloc = LOG_ERRLOC_DIR_MKDIR;
		return err;
	}

	/*
	 * Change permission bits of the directory explicitly because some
	 * bits may be dropped by umask.
	 */
	err = chmod(path, LOG_DIR_PERM);
	if (PFC_EXPECT_FALSE(err != 0)) {
		err = errno;
		*errloc = LOG_ERRLOC_DIR_CHMOD;
		return err;
	}

	return 0;
}

/*
 * pfc_bool_t
 * pfc_log_isfatal(void)
 *	Determine whether at least one fatal log message is recorded or not.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if at least one fatal log message is recorded.
 *	Otherwise PFC_FALSE is returned.
 */
pfc_bool_t
pfc_log_isfatal(void)
{
	return log_isfatal;
}

/*
 * void
 * pfc_log_set_fatal_handler(pfc_log_fatal_t handler)
 *	Install fatal error log handler.
 */
void
pfc_log_set_fatal_handler(pfc_log_fatal_t handler)
{
	PFC_LOG_LOCK();
	log_fatal_handler = handler;
	PFC_LOG_UNLOCK();
}

/*
 * void
 * __pfc_log_common(pfc_log_level_t level, const char *modname,
 *		    const char *format, ...)
 *	Common routine to send a log to the PFC logging system.
 */
void
__pfc_log_common(pfc_log_level_t level, const char *modname,
		 const char *format, ...)
{
	va_list	ap;

	va_start(ap, format);
	pfc_log_common_impl(level, modname, format, ap);
	va_end(ap);
}

/*
 * int
 * pfc_log_set_level(pfc_log_level_t level)
 *	Change trace log level.
 *
 *	If `level' is PFC_LOGLVL_NONE, the logging level is reset to initial
 *	value.
 *
 * Calling/Exit State:
 *	Zero is returned if the logging level is changed successfully.
 *	1 is returned if the logging level is not changed.
 *	Otherwise negative error number defined in errno.h is returned.
 */
int
pfc_log_set_level(pfc_log_level_t level)
{
	return log_level_set(LOG_ATTR_TRACE, level);
}

/*
 * void
 * pfc_log_modlevel_init(pfc_cfblk_t block)
 *	Import per-module logging level from the configuration block handle
 *	specified by `block'.
 *
 *	This function obtains string array parameter named "log_modlevel"
 *	from `block'. Each string must consist of module name and level
 *	separated by colon, just like "modname:debug".
 */
void
pfc_log_modlevel_init(pfc_cfblk_t block)
{
	int	size, index;

	size = pfc_conf_array_size(block, conf_log_modlevel);

	for (index = 0; index < size; index++) {
		log_modlevel_init(block, index);
	}
}

/*
 * int
 * pfc_log_modlevel_set(const char *ident, pfc_log_level_t level)
 *	Change per-module logging level.
 *
 *	`ident' must be a non-NULL pointer to string which represents log
 *	identifier, which is defined by PFC_LOG_IDENT.
 *
 * Calling/Exit State:
 *	Zero is returned if the logging level is changed successfully.
 *	1 is returned if the logging level is not changed.
 *	Otherwise negative error number defined in errno.h is returned.
 */
int
pfc_log_modlevel_set(const char *ident, pfc_log_level_t level)
{
	PFC_ASSERT(ident != NULL);

	if (PFC_EXPECT_FALSE((uint32_t)level >=
			     PFC_ARRAY_CAPACITY(log_level_attr))) {
		return -EINVAL;
	}

	return log_modlevel_set(ident, level);
}

/*
 * void
 * pfc_log_modlevel_reset(const char *modname)
 *	Reset logging level for the module specified by `modname'.
 *
 *	`modname' must be a pointer to string which represents module name.
 *	Once this function is called, global logging level will be used for
 *	the specified module.
 *
 *	If NULL is passed to `modname', all per-module logging levels are
 *	reset.
 */
void
pfc_log_modlevel_reset(const char *modname)
{
	if (modname == NULL) {
		/* Clear all per-module configurations. */
		PFC_LOG_LOCK();
		LOG_MODLEVEL_CLEAR();
		PFC_LOG_UNLOCK();

		pfc_log_info("Per-module logging level has been reset.");
	}
	else {
		pfc_rbnode_t	*node;

		/* Remove logging level for the specified module. */
		PFC_LOG_LOCK();
		LOG_MODLEVEL_REMOVE(modname, node);
		PFC_LOG_UNLOCK();

		if (node != NULL) {
			log_modlevel_dtor(node, NULL);
			pfc_log_info("Logging level for \"%s\" has been reset.",
				     modname);
		}
	}
}

/*
 * int
 * pfc_log_modlevel_copy(pfc_log_modconf_t **levelsp)
 *	Create a complete copy of per-module logging level configuration.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to pfc_log_modconf_t array is
 *	set to `*levelsp', and number of pfc_log_modconf_t array elements is
 *	returned. Note that NULL is set to `*levelsp' if per-module logging
 *	level is not configured.
 *
 *	-1 is returned on failure.
 *
 * Remarks:
 *	Returned pfc_log_modconf_t array must be freed by the call of
 *	pfc_log_modlevel_free().
 */
int
pfc_log_modlevel_copy(pfc_log_modconf_t **levelsp)
{
	pfc_log_modconf_t	*plmp, *p, *array;
	pfc_rbnode_t		*node;
	uint32_t		count;

	PFC_LOG_LOCK();

	count = log_modules.lmc_count;
	if (count == 0) {
		array = NULL;
		goto out;
	}

	/* Allocate pfc_log_modconf_t array. */
	array = (pfc_log_modconf_t *)malloc(sizeof(pfc_log_modconf_t) * count);
	plmp = array;
	if (PFC_EXPECT_FALSE(array == NULL)) {
		goto error;
	}

	node = NULL;
	while ((node = LOG_MODLEVEL_NEXT(node)) != NULL) {
		log_modlevel_t	*lmp = LOG_NODE2MODLEVEL(node);
		const char	*name;

		/* Copy module name and level. */
		name = strdup(lmp->lml_name);
		if (PFC_EXPECT_FALSE(name == NULL)) {
			goto error;
		}

		plmp->plm_name = name;
		plmp->plm_level = lmp->lml_level;
		plmp++;
	}
	PFC_ASSERT(plmp == array + count);

out:
	PFC_LOG_UNLOCK();

	*levelsp = array;

	return count;

error:
	PFC_LOG_UNLOCK();

	for (p = array; p < plmp; p++) {
		free((void *)p->plm_name);
	}
	free(array);

	return -1;
}

/*
 * void
 * pfc_log_modlevel_free(pfc_log_modconf_t *levels, uint32_t count)
 *	Free up resources allocated by the call of pfc_log_modlevel_copy().
 */
void
pfc_log_modlevel_free(pfc_log_modconf_t *levels, uint32_t count)
{
	pfc_log_modconf_t	*plmp;

	if (count == 0) {
		return;
	}

	for (plmp = levels; plmp < levels + count; plmp++) {
		free((void *)plmp->plm_name);
	}
	free(levels);
}

/*
 * void
 * __pfc_log_common_v(pfc_log_level_t level, const char *PFC_RESTRICT modname,
 *		      const char *PFC_RESTRICT format, va_list ap)
 *	Common routine to send a log to the PFC logging system.
 */
void
__pfc_log_common_v(pfc_log_level_t level, const char *PFC_RESTRICT modname,
		   const char *PFC_RESTRICT format, va_list ap)
{
	pfc_log_common_impl(level, modname, format, ap);
}

/*
 * pfc_log_level_t
 * __pfc_log_current_modlevel(const char *modname)
 *	Return current log level associated with the specified module.
 *
 *	`modname' must be a pointer to string which represents module name.
 *
 * Remarks:
 *	This function assumes that NULL is never passed to `modname'.
 */
pfc_log_level_t
__pfc_log_current_modlevel(const char *modname)
{
	pfc_log_level_t	level;

	PFC_ASSERT(modname != NULL);

	PFC_LOG_LOCK();
	level = log_current_level(modname);
	PFC_LOG_UNLOCK();

	return level;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_log_level_is_valid(pfc_log_level_t level)
 *	Determine whether the specified log level is valid or not.
 *
 * Calling/Exit State:
 *	Zero is returned if the logging level specified by `level' is valid.
 *	Otherwise EINVAL is returned.
 */
int PFC_ATTR_HIDDEN
pfc_log_level_is_valid(pfc_log_level_t level)
{
	log_cattr_t	*lattr = LOG_ATTR_TRACE;

	return log_level_is_valid(lattr, level);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_log_facility_is_valid(pfc_log_facl_t facility)
 *	Determine whether the specified logging facility is valid or not.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified logging facility is valid.
 *	Otherwise EINVAL is returned.
 */
int PFC_ATTR_HIDDEN
pfc_log_facility_is_valid(pfc_log_facl_t facility)
{
	const log_facl_t	*fp = log_facility_get(facility);

	return (PFC_EXPECT_TRUE(fp != NULL)) ? 0 : EINVAL;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_log_facility_get(const char *PFC_RESTRICT name,
 *			pfc_log_facl_t *PFC_RESTRICT fap)
 *	Convert symbolic name of the logging facility into internal logging
 *	facility value.
 *
 * Calling/Exit State:
 *	Upon successful completion, logging facility value associated with
 *	the facility name `name' is set to the buffer pointed by `fap', and
 *	zero is returned.
 *	Otherwise EINVAL is returned.
 */
int PFC_ATTR_HIDDEN
pfc_log_facility_get(const char *PFC_RESTRICT name,
		     pfc_log_facl_t *PFC_RESTRICT fap)
{
	if (PFC_EXPECT_TRUE(name != NULL)) {
		const log_facl_t	*fp;

		for (fp = log_facility_attr;
		     fp < PFC_ARRAY_LIMIT(log_facility_attr); fp++) {
			if (strcasecmp(name, fp->f_name) == 0) {
				*fap = (pfc_log_facl_t)
					(fp - log_facility_attr);

				return 0;
			}
		}
	}

	return EINVAL;
}

/*
 * static void
 * pfc_log_common_impl(pfc_log_level_t level, const char *modname,
 *		       const char *format, va_list ap)
 *	Record the specified log message.
 *
 * Remarks:
 *	This function always records the specified log message using the level
 *	specified by `level'. The caller is responsible for checking the
 *	current log level.
 */
static void
pfc_log_common_impl(pfc_log_level_t level, const char *modname,
		    const char *format, va_list ap)
{
	const char	*fmt;
	const log_lvl_t	*lvl;
	pfc_log_fatal_t	handler;

	PFC_ASSERT((uint32_t)level < PFC_ARRAY_CAPACITY(log_level_attr));

	lvl = &log_level_attr[level];

	PFC_LOG_LOCK();
	fmt = pfc_log_create_format(lvl, modname, format);
	(*log_function)(lvl->l_priority, fmt, ap);

	if (level == PFC_LOGLVL_FATAL) {
		handler = log_fatal_handler;
		(void)pfc_atomic_swap_uint8(&log_isfatal, PFC_TRUE);
	}
	else {
		handler = NULL;
	}

	PFC_LOG_UNLOCK();

	if (handler != NULL) {
		/* Call fatal error handler. */
		(*handler)();
	}
}

/*
 * static void
 * pfc_log_file(int pri, const char *format, va_list ap)
 *	Record a log to the system log file.
 *
 * Remarks:
 *	This function must be called with holding the PFC log system lock.
 */
static void
pfc_log_file(int pri, const char *format, va_list ap)
{
	FILE	*out = log_output;

	if (out != NULL) {
		char	datebuf[PFC_TIME_STRING_LENGTH];
		const char	*datep;
		size_t	sz = 0;
		int	ret;

		/* Create log timestamp. */
		datep = pfc_log_timestamp(datebuf, sizeof(datebuf));
		ret = fprintf(out, "%s: ", datep);
		if (ret > 0) {
			sz = ret;
		}

#ifdef	PFC_TIDLOG_ENABLED
		/* Print system thread ID for the calling thread. */
		ret = fprintf(out, "[%u]: ", pfc_gettid());
		if (ret > 0) {
			sz += ret;
		}
#endif	/* PFC_TIDLOG_ENABLED */

		/* Print the specified log. */
		ret = vfprintf(out, format, ap);
		if (ret > 0) {
			sz += ret;
		}
		ret = fputc('\n', out);
		if (ret != EOF) {
			sz++;
		}
		fflush(out);

		log_dump_size += sz;
		if (log_rotate != NULL && log_dump_size >= log_rotate->r_size) {
			pfc_log_file_rotate();
			log_dump_size = 0;
		}
	}
}

/*
 * static void
 * pfc_log_file_rotate(void)
 *	Rotate the system log file and replace the file stream.
 *
 * Remarks:
 *	This function must be called with holding the PFC log system lock.
 */
static void
pfc_log_file_rotate(void)
{
	FILE	*newfp;
	int	err, errloc;

	PFC_ASSERT(log_output != NULL);
	PFC_ASSERT(log_output != stdout && log_output != stderr);

	err = log_file_create(&newfp, &errloc);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* Failed to rotate. */
		log_file_rotation_fini();
		log_rotate = NULL;
		fprintf(stderr, "Failed to rotate message log: %d: %s\n",
			errloc, strerror(err));
		return;
	}

	fclose(log_output);
	log_output = newfp;
}

/*
 * static void
 * pfc_log_file_close(void)
 *	Shutdown the logging to the file.
 *
 * Remarks:
 *	This function must be called with holding the PFC log system lock.
 */
static void
pfc_log_file_close(void)
{
	FILE	*file;

	file = log_output;
	if (file != NULL) {
		if (file != stdout && file != stderr) {
			fclose(file);
		}
		log_output = NULL;
	}
}

/*
 * static const char *
 * pfc_log_create_format(const log_lvl_t *lvl, const char *modname,
 *			 const char *format)
 *	Create format string of the log message.
 *
 * Remarks:
 *	This function must be called with holding the PFC log system lock.
 */
static const char *
pfc_log_create_format(const log_lvl_t *lvl, const char *modname,
		      const char *format)
{
	char	*buffer = log_buffer;

	if (modname != NULL) {
		snprintf(buffer, sizeof(log_buffer), "%s: %s: %s",
			 lvl->l_name, modname, format);
	}
	else {
		snprintf(buffer, sizeof(log_buffer), "%s: %s",
			 lvl->l_name, format);
	}

	/* Ensure the log message is terminated. */
	*(buffer + PFC_LOG_MAX_SIZE) = '\0';

	return (const char *)buffer;
}

/*
 * static const char *
 * pfc_log_timestamp(char *buffer, size_t bufsize)
 *	Create a string which represents current date and time.
 */
static const char *
pfc_log_timestamp(char *buffer, size_t bufsize)
{
	int	err;

	/* Create log timestamp. */
	err = pfc_time_mclock_ctime(buffer, bufsize);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return (const char *)buffer;
	}

	return "<unknown-date>";
}

/*
 * static int
 * log_file_open(FILE **fpp, int *errloc)
 *	Open an existing log file.
 *	If not exist, invoke log_file_create() to create a new log file.
 */
static int
log_file_open(FILE **fpp, int *errloc)
{
	int		fd, err;
	FILE		*fp;
	struct stat	lsbuf, sbuf;
	const char	*path = pfc_refptr_string_value(log_rotate->r_path);

	/* Obtain status of existing log file. */
	if (lstat(path, &lsbuf) != 0) {
		err = errno;
		if (PFC_EXPECT_FALSE(err != ENOENT)) {
			*errloc = LOG_ERRLOC_LSTAT;

			return err;
		}
		goto out;
	}

	if (PFC_EXPECT_FALSE(!S_ISREG(lsbuf.st_mode))) {
		/* Clean bogus file and create new one. */
		(void)pfc_rmpath(path);
		goto out;
	}

	/* Open an existing logfile */
	fd = pfc_open_cloexec(path, O_APPEND | O_WRONLY, 0);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		err = errno;
		if (PFC_EXPECT_FALSE(err != ENOENT)) {
			*errloc = LOG_ERRLOC_APPEND;

			return err;
		}
		goto out;
	}

	if (PFC_EXPECT_FALSE(fstat(fd, &sbuf) != 0)) {
		err = errno;
		*errloc = LOG_ERRLOC_FSTAT;
		PFC_ASSERT_INT(close(fd), 0);

		return err;
	}

	if (PFC_EXPECT_FALSE(sbuf.st_dev != lsbuf.st_dev ||
			     sbuf.st_ino != lsbuf.st_ino)) {
		/*
		 * The log file has been replaced between lstat(2) and
		 * open(2) call. We don't allow this situation.
		 */
		*errloc = LOG_ERRLOC_BUSY;
		PFC_ASSERT_INT(close(fd), 0);

		return EBUSY;
	}

	/*
	 * Check whether the existing file is safe or not.
	 * If the file is unsafe, rotate the logfile and create a new logfile.
	 *  - The logfile must be a regular file.
	 *  - Owner and group must be same as daemon's effective uid and gid.
	 *  - File permission must be 0600.
	 *  - The file is smaller than rotation limit.
	 */
	PFC_ASSERT(S_ISREG(sbuf.st_mode));
	if (PFC_EXPECT_TRUE((size_t)sbuf.st_size < log_rotate->r_size &&
			    sbuf.st_uid == geteuid() &&
			    sbuf.st_gid == getegid() &&
			    (sbuf.st_mode & 07777) == LOG_FILE_PERM)) {
		/* Use existing logfile. */
		log_dump_size = sbuf.st_size;

		fp = fdopen(fd, "w");
		if (PFC_EXPECT_FALSE(fp == NULL)) {
			err = errno;
			*errloc = LOG_ERRLOC_FDOPEN;
			close(fd);

			return err;
		}

		*fpp = fp;

		return 0;
	}

	PFC_ASSERT_INT(close(fd), 0);

out:
	/* rotate and create a new logfile. */
	return log_file_create(fpp, errloc);
}

/*
 * static int
 * log_file_create(FILE *fpp, int *errloc)
 *	Rotate old message log files and create a new log file.
 *
 * Remarks:
 *	This function is called with holding the PFC log system lock
 *	by PFC logging system. Don't invoke PFC logging system.
 */
static int
log_file_create(FILE **fpp, int *errloc)
{
	int	fd, err;
	FILE	*fp;
	const char	*path = pfc_refptr_string_value(log_rotate->r_path);

	/* Create a new logfile. */
	if (log_rotate->r_count == 0) {
		/* No need to preserve old log. */
		fd = pfc_open_cloexec(path, O_CREAT | O_WRONLY | O_TRUNC,
				      LOG_FILE_PERM);
		if (PFC_EXPECT_FALSE(fd == -1)) {
			*errloc = LOG_ERRLOC_TRUNCATE;
			return errno;
		}
	}
	else {
		fd = pfc_open_cloexec(path, O_CREAT | O_WRONLY | O_EXCL,
				      LOG_FILE_PERM);
		if (PFC_EXPECT_FALSE(fd == -1)) {
			if (errno != EEXIST) {
				*errloc = LOG_ERRLOC_CREATE;
				return errno;
			}

			/* Rotate old logs. */
			err = log_do_rotation(path, errloc);
			if (PFC_EXPECT_FALSE(err != 0)) {
				return err;
			}
			fd = pfc_open_cloexec(path, O_CREAT | O_WRONLY | O_EXCL,
					      LOG_FILE_PERM);
			if (PFC_EXPECT_FALSE(fd == -1)) {
				*errloc = LOG_ERRLOC_CREATE_ROTATE;
				return errno;
			}
		}
	}

	fp = fdopen(fd, "w");
	if (PFC_EXPECT_FALSE(fp == NULL)) {
		err = errno;
		*errloc = LOG_ERRLOC_FDOPEN;
		close(fd);
		return err;
        }
	*fpp = fp;
	return 0;
}

/*
 * static void
 * log_do_rotation(const char *path, int *errloc)
 *	Rotate old message log files.
 */
static int
log_do_rotation(const char *path, int *errloc)
{
	uint32_t	i;
	char		*opath = log_rotate->r_oldpath;
	char		*npath = log_rotate->r_newpath;
	size_t		psize = log_rotate->r_pathsize;

	/* Rotate old logs. */
	for (i = log_rotate->r_count; i > 1; i--) {
		snprintf(opath, psize, "%s.%u", path, i - 1);
		snprintf(npath, psize, "%s.%u", path, i);
		if (PFC_EXPECT_FALSE(rename(opath, npath) != 0)) {
			if (errno != ENOENT) {
				*errloc = LOG_ERRLOC_ROTATION;
				return errno;
			}
		}
	}

	/* Preserve old log. */
	snprintf(npath, psize, "%s.1", path);
	if (PFC_EXPECT_FALSE(rename(path, npath) != 0)) {
		*errloc = LOG_ERRLOC_ROTATION_LAST;
		return errno;
	}

	return 0;
}

/*
 * static unsigned int
 * log_digit(uint32_t num)
 *	Return digit number of the specified num.
 */
static inline unsigned int
log_digit(uint32_t num)
{
	unsigned int	digit;

	for (digit = 1; digit <= 10; digit++) {
		num /= 10;
		if (num == 0) {
			break;
		}
	}
	PFC_ASSERT(num == 0);
	return digit;
}

/*
 * static int
 * log_file_rotation_init(const char *path)
 *	Initialize the log rotation information.
 *
 * Remarks:
 *	This function must be called with holding the PFC log system lock.
 */
static int
log_file_rotation_init(const char *path, uint32_t rcount, size_t rsize)
{
	size_t		psize;

	if (PFC_EXPECT_FALSE(log_rotate != NULL)) {
		log_file_rotation_fini();
	}

	log_rotate = (log_rotation_t *)malloc(sizeof(log_rotation_t));
	if (PFC_EXPECT_FALSE(log_rotate == NULL)) {
		return ENOMEM;
	}

	/* Determine rotation count and size. */
	log_rotate->r_count = rcount;
	log_rotate->r_size = rsize;

	log_rotate->r_path = pfc_refptr_string_create(path);
	if (PFC_EXPECT_FALSE(log_rotate->r_path == NULL)) {
		goto error;
	}

	psize = pfc_refptr_string_length(log_rotate->r_path) +
		log_digit(log_rotate->r_count) + 2;
	log_rotate->r_oldpath = (char *)malloc(psize);
	if (PFC_EXPECT_FALSE(log_rotate->r_oldpath == NULL)) {
		goto error;
	}
	log_rotate->r_newpath = (char *)malloc(psize);
	if (PFC_EXPECT_FALSE(log_rotate->r_newpath == NULL)) {
		goto error;
	}

	log_rotate->r_pathsize = psize;

	return 0;

error:
	log_file_rotation_fini();
	return ENOMEM;
}

/*
 * static void
 * log_file_rotation_fini(void)
 *	Free the log rotation information.
 *
 * Remarks:
 *	This function must be called with holding the PFC log system lock.
 */
static void
log_file_rotation_fini(void)
{
	log_rotation_t	*rotate;

	rotate = pfc_atomic_swap_ptr((pfc_ptr_t *)&log_rotate, NULL);
	if (rotate == NULL) {
		return;
	}

	if (rotate->r_path != NULL) {
		pfc_refptr_put(rotate->r_path);
	}
	if (rotate->r_newpath != NULL) {
		free(rotate->r_newpath);
	}
	if (rotate->r_oldpath != NULL) {
		free(rotate->r_oldpath);
	}
	free(rotate);
}

/*
 * static pfc_log_level_t
 * log_level_name_is_valid(log_cattr_t *lattr, const char *name)
 *	Determine whether the string representation of the log level specified
 *	by `name' is valid or not.
 *
 * Calling/Exit State:
 *	PFC_LOGLVL_NONE is returned if the specified log level name is invalid.
 *	Otherwise valid log level value is returned.
 */
static pfc_log_level_t
log_level_name_is_valid(log_cattr_t *lattr, const char *name)
{
	log_clvl_t	*lvl;

	/* Parse string representation of logging level. */
	for (lvl = log_level_attr; lvl < PFC_ARRAY_LIMIT(log_level_attr);
	     lvl++) {
		if (strcasecmp(name, lvl->l_name) != 0) {
			continue;
		}

		return (pfc_log_level_t)(lvl - log_level_attr);
	}

	return PFC_LOGLVL_NONE;
}

/*
 * static int
 * log_level_is_valid(log_cattr_t *lattr, pfc_log_level_t level)
 *	Determine whether the specified log level is valid or not.
 *
 * Calling/Exit State:
 *	Zero is returned if the logging level specified by `level' is valid.
 *	Otherwise EINVAL is returned.
 */
static int
log_level_is_valid(log_cattr_t *lattr, pfc_log_level_t level)
{
	if (PFC_EXPECT_FALSE((uint32_t)level >=
			     PFC_ARRAY_CAPACITY(log_level_attr))) {
		return EINVAL;
	}

	return 0;
}

/*
 * static void
 * log_level_init(log_cattr_t *lattr, pfc_cfblk_t block,
 *		  const pfc_loglvlcf_t *lvcf, pfc_log_level_t newlvl,
 *		  const char **invnamep)
 *	Initialize logging level.
 *
 *	`newlvl' must be the logging level loaded from the loglevel file
 *	or PFC_LOGLVL_NONE.
 *
 *	This function tries to derive the level from the configuration
 *	parameter block specified by `block'. If an invalid logging level
 *	name is derived, it is set to the buffer pointed by `invnamep'.
 */
static void
log_level_init(log_cattr_t *lattr, pfc_cfblk_t block,
	       const pfc_loglvlcf_t *lvcf, pfc_log_level_t newlvl,
	       const char **invnamep)
{
	pfc_log_level_t	level = lvcf->plvc_level;

	/* Determine default log level. */
	log_level_init_default(lattr, block, lvcf, invnamep);

	/* Use the logging level in lvcf->plvc_level if it is valid. */
	if (log_level_is_valid(lattr, level) != 0) {
		if (newlvl != PFC_LOGLVL_NONE) {
			/* Use the logging level derived from loglevel file. */
			level = newlvl;
		}
		else {
			/* Use default log level. */
			level = *(lattr->la_initlevelp);
		}

		PFC_ASSERT_INT(log_level_is_valid(lattr, level), 0);
	}

	/* Initialize logging level. */
	*(lattr->la_levelp) = level;
}

/*
 * static void
 * log_level_init_default(log_cattr_t *lattr, pfc_cfblk_t block,
 *			  const pfc_loglvlcf_t *lvcf, const char **invnamep)
 *	Initialize default logging level.
 *
 *	This function tries to derive the level from the configuration
 *	parameter block specified by `block'. If an invalid logging level
 *	name is derived, it is set to the buffer pointed by `invnamep'.
 */
static void
log_level_init_default(log_cattr_t *lattr, pfc_cfblk_t block,
		       const pfc_loglvlcf_t *lvcf, const char **invnamep)
{
	pfc_log_level_t	level = lvcf->plvc_deflevel;
	const char	*lvlname;

	*invnamep = NULL;

	/* Try to derive the logging level from the configuration file. */
	lvlname = pfc_conf_get_string(block, lattr->la_lvlcfname, NULL);
	if (lvlname != NULL) {
		pfc_log_level_t	l;

		l = log_level_name_is_valid(lattr, lvlname);
		if (PFC_EXPECT_TRUE(l != PFC_LOGLVL_NONE)) {
			/* Prefer log level in configuration file. */
			level = l;
		}
		else {
			/*
			 * Invalid logging level is defined in the
			 * configuration file.
			 */
			*invnamep = lvlname;
		}
	}

	PFC_ASSERT_INT(log_level_is_valid(lattr, level), 0);
	(*lattr->la_initlevelp) = level;
}

/*
 * static int
 * log_level_set(log_cattr_t *lattr, pfc_log_level_t level)
 *	Change log level for the log type specified by `lattr'.
 *
 *	If `level' is PFC_LOGLVL_NONE, the logging level is reset to initial
 *	value.
 *
 * Calling/Exit State:
 *	Zero is returned if the logging level is changed successfully.
 *	1 is returned if the logging level is not changed.
 *
 *	Otherwise negative error number defined in errno.h is returned.
 */
static int
log_level_set(log_cattr_t *lattr, pfc_log_level_t level)
{
	log_clvl_t	*lvl;
	pfc_log_level_t	svlevel = level, *levelp;
	log_lvlfctx_t	fctx;
	int		err, ret;

	if (level == PFC_LOGLVL_NONE) {
		/* Reset logging level. */
		level = *(lattr->la_initlevelp);
	}
	else {
		/* Ensure that the specified level is valid. */
		err = log_level_is_valid(lattr, level);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return -err;
		}
	}

	log_lvlfctx_init(&fctx);
	levelp = lattr->la_levelp;
	lvl = &log_level_attr[level];

	PFC_LOG_LOCK();

	ret = (level == *levelp) ? 1 : 0;

	/*
	 * Logging level in the loglevel file must be updated if the level
	 * is going to be reset or changed.
	 */
	if (svlevel == PFC_LOGLVL_NONE || ret == 0) {
		int  err = log_save_loglevel(lattr, svlevel, &fctx);

		if (PFC_EXPECT_FALSE(err != 0)) {
			ret = -err;
		}
	}

	if (ret == 0) {
		(void)pfc_atomic_swap_uint32((uint32_t *)levelp, level);
	}

	PFC_LOG_UNLOCK();

	log_lvlfctx_fini(&fctx);

	if (ret == 0) {
		pfc_log_info("Logging level for %s has been changed to %s.",
			     lattr->la_name, lvl->l_name);
	}
	else if (PFC_EXPECT_FALSE(ret < 0)) {
		pfc_log_error("Failed to save %s level: %s",
			      lattr->la_name, strerror(-ret));
	}

	return ret;
}

/*
 * static void
 * log_lvlfctx_init(log_lvlfctx_t *fctx)
 *	Initialize loglevel file context.
 */
static void
log_lvlfctx_init(log_lvlfctx_t *fctx)
{
	fctx->lvfc_level = (log_lvlfile_t *)MAP_FAILED;
	fctx->lvfc_reset = PFC_FALSE;
	fctx->lvfc_errmsg[0] = '\0';
}

/*
 * static void
 * log_lvlfctx_fini(log_lvlfctx_t *fctx)
 *	Finalize loglevel file context.
 */
static void
log_lvlfctx_fini(log_lvlfctx_t *fctx)
{
	void	*lfp = (void *)fctx->lvfc_level;

	if (PFC_EXPECT_TRUE(lfp != MAP_FAILED)) {
		PFC_ASSERT_INT(munmap(lfp, sizeof(log_lvlfile_t)), 0);
	}

	if (PFC_EXPECT_FALSE(fctx->lvfc_reset)) {
		pfc_log_warn("Bogus loglevel file was reset.");
	}
	if (PFC_EXPECT_FALSE(fctx->lvfc_errmsg[0] != '\0')) {
		pfc_log_error("%s", fctx->lvfc_errmsg);
	}
}

/*
 * static int
 * log_lvlfile_open(const char *path, log_lvlfctx_t *fctx,
 *		     pfc_bool_t do_create)
 *	Open the loglevel file specified by `path'.
 *
 *	If PFC_TRUE is specified to `do_create', this function tries to create
 *	a new loglevel file if it does not exist, or is broken.
 */
static int
log_lvlfile_open(const char *path, log_lvlfctx_t *fctx, pfc_bool_t do_create)
{
	struct stat	sbuf, fsbuf;
	int		err, fd = -1;
	log_lvlfile_t	*lfp;

	/* Get status of existing loglevel file. */
	if (lstat(path, &sbuf) != 0) {
		err = errno;
		if (PFC_EXPECT_TRUE(err == ENOENT)) {
			if (do_create) {
				/* Create a new loglevel file. */
				return log_lvlfile_create(path, fctx);
			}
		}
		else {
			snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
				 "Failed to get status of loglevel file"
				 ": %s: %s", path, strerror(err));
		}

		return err;
	}

	/* Loglevel file must be a regular file. */
	if (PFC_EXPECT_FALSE(!S_ISREG(sbuf.st_mode))) {
		snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
			 "Loglevel file is not a regular file: %s", path);
		if (do_create) {
			goto clean_bogus;
		}

		return EBADFD;
	}

	/* Open existing loglevel file. */
	fd = pfc_open_cloexec(path, O_RDWR);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		err = errno;
		if (PFC_EXPECT_TRUE(err == ENOENT)) {
			if (do_create) {
				return log_lvlfile_create(path, fctx);
			}
		}
		else {
			snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
				 "Failed to open loglevel file: %s: %s",
				 path, strerror(err));
		}

		return err;
	}

	/* Get status of opened file. */
	if (PFC_EXPECT_FALSE(fstat(fd, &fsbuf) != 0)) {
		/* This should never happen. */
		err = errno;
		snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
			 "Failed to get status of loglevel file: %s: %s",
			 path, strerror(err));
		goto error;
	}

	/* Ensure that the target file is not changed. */
	if (PFC_EXPECT_FALSE(sbuf.st_dev != fsbuf.st_dev ||
			     sbuf.st_ino != fsbuf.st_ino ||
			     sbuf.st_size != fsbuf.st_size ||
			     !S_ISREG(fsbuf.st_mode))) {
		err = EBUSY;
		snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
			 "Loglevel file was changed: %s", path);
		if (do_create) {
			goto clean_bogus;
		}
		goto error;
	}

	/* Verify log file size. */
	if (PFC_EXPECT_FALSE(fsbuf.st_size != sizeof(*lfp))) {
		err = ENODATA;
		snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
			 "Invalid loglevel file size: %s: %lu",
			 path, fsbuf.st_size);
		if (do_create) {
			goto clean_bogus;
		}
		goto error;
	}

	/* Map the loglevel file into virtual memory space. */
	lfp = (log_lvlfile_t *)mmap(NULL, sizeof(*lfp), PROT_READ | PROT_WRITE,
				    MAP_SHARED, fd, 0);
	if (PFC_EXPECT_TRUE(lfp != (log_lvlfile_t *)MAP_FAILED)) {
		err = 0;
		fctx->lvfc_level = lfp;
	}
	else {
		err = errno;
		snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
			 "Failed to map loglevel file: %s: %s",
			 path, strerror(err));
	}

	PFC_ASSERT_INT(close(fd), 0);

	return err;

clean_bogus:
	/* Unlink bogus loglevel file, and create a new one. */
	if (fd != -1) {
		PFC_ASSERT_INT(close(fd), 0);
	}

	err = pfc_rmpath(path);
	if (PFC_EXPECT_TRUE(err == 0 || err == ENOENT)) {
		fctx->lvfc_reset = PFC_TRUE;
		err = log_lvlfile_create(path, fctx);
	}
	else {
		snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
			 "Failed to unlink bogus loglevel file: %s: %s",
			 path, strerror(err));
	}

	return err;

error:
	if (fd != -1) {
		PFC_ASSERT_INT(close(fd), 0);
	}

	return err;
}

/*
 * static int
 * log_lvlfile_create(const char *path, log_lvlfctx_t *fctx)
 *	Create a new loglevel file.
 *
 * Calling/Exit State:
 *	Upon successful completion, contents of the loglevel file is set
 *	to fctx->lvfc_level, and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	In this case, an error message is set to fctx->lvfc_errmsg.
 */
static int
log_lvlfile_create(const char *path, log_lvlfctx_t *fctx)
{
	int		fd, err;
	log_lvlfile_t	*lfp;

	/* Create a new loglevel file. */
	fd = pfc_open_cloexec(path, O_RDWR | O_CREAT | O_EXCL,
			      LOG_LEVELFILE_PERM);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		err = errno;
		snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
			 "Failed to create loglevel file: %s: %s",
			 path, strerror(err));

		return err;
	}

	/* Preserve room for log_lvlfile_t. */
	if (PFC_EXPECT_FALSE(ftruncate(fd, sizeof(*lfp)) != 0)) {
		err = errno;
		snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
			 "Failed to truncate loglevel file: %s: %s",
			 path, strerror(err));
		goto out;
	}

	/* Map the loglevel file into virtual memory space. */
	lfp = (log_lvlfile_t *)mmap(NULL, sizeof(*lfp), PROT_READ | PROT_WRITE,
				    MAP_SHARED, fd, 0);
	if (PFC_EXPECT_FALSE(lfp == (log_lvlfile_t *)MAP_FAILED)) {
		err = errno;
		snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
			 "Failed to map loglevel file: %s: %s",
			 path, strerror(err));
		goto out;
	}

	/* Set default level. */
	lfp->lvf_level = LOG_LVLCHAR_NONE;

	fctx->lvfc_level = lfp;
	err = 0;

out:
	PFC_ASSERT_INT(close(fd), 0);

	return err;
}

/*
 * static void
 * log_load_loglevel(log_lvlfctx_t *fctx, pfc_log_level_t *lvlp)
 *	Load loglevel from loglevel file.
 *	If the file is not found or loglevel in the file indicates not-changed,
 *	set negative value to specified level.
 *
 *	An error message is stored to fctx->lvfc_errmsg if an error was
 *	detected.
 *
 * Remarks:
 *	- This function must be called with holding the PFC log system lock.
 *
 *	- The caller must initialize `fctx' using LOG_LVLFCTX_INIT().
 */
static void
log_load_loglevel(log_lvlfctx_t *fctx, pfc_log_level_t *lvlp)
{
	const char	*path = log_lvlpath;
	int		err;
	log_lvlfile_t	*lfp;

	PFC_ASSERT(fctx->lvfc_errmsg[0] == '\0');
	if (path == NULL) {
		return;
	}

	/* Open loglevel file. */
	err = log_lvlfile_open(path, fctx, PFC_FALSE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (PFC_EXPECT_FALSE(err == ENODATA || err == EBADFD)) {
			goto broken;
		}

		return;
	}

	/* Set trace log level. */
	lfp = fctx->lvfc_level;
	PFC_ASSERT(lfp != (log_lvlfile_t *)MAP_FAILED);
	err = log_load_loglevel_impl(fctx, LOG_ATTR_TRACE, path,
				     lfp->lvf_level, lvlp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto broken;
	}

	return;

broken:
	/* Unlink broken loglevel file. */
	err = pfc_rmpath(path);
	if (PFC_EXPECT_TRUE(err == 0)) {
		fctx->lvfc_reset = PFC_TRUE;
	}

	/* Don't believe in levels in broken file. */
	*lvlp = PFC_LOGLVL_NONE;
}

/*
 * static int
 * log_load_loglevel_impl(log_lvlfctx_t *fctx, log_cattr_t *lattr,
 *			  char lc, const char *path, pfc_log_level_t *lvlp)
 *	Load loglevel in the loglevel file.
 *
 *	`lc' must be a character saved in the loglevel file, which is
 *	associated with the log level for the log type `lattr'.
 *
 * Calling/Exit State:
 *	If the log level character specified by `lc' is valid, converted
 *	log level is set to the buffer pointed by `lvlp', and zero is returned.
 *
 *	If `lc' equals LOG_LVLCHAR_NONE, zero is returned without changing
 *	the buffer pointed by `lvlp'.
 *
 *	Otherwise EINVAL is returned.
 */
static int
log_load_loglevel_impl(log_lvlfctx_t *fctx, log_cattr_t *lattr,
		       const char *path, char lc, pfc_log_level_t *lvlp)
{
	int	err;

	if (lc == LOG_LVLCHAR_NONE) {
		err = 0;
	}
	else {
		pfc_log_level_t	level = LOG_CHAR2LVL(lc);

		err = log_level_is_valid(lattr, level);
		if (PFC_EXPECT_FALSE(err != 0)) {
			snprintf(fctx->lvfc_errmsg, sizeof(fctx->lvfc_errmsg),
				 "Broken %s level in the loglevel file: %s: "
				 "0x%x", lattr->la_name, path, (uint8_t)lc);
		}
		else {
			*lvlp = level;
		}
	}

	return err;
}

/*
 * static int
 * log_save_loglevel(log_cattr_t *lattr, pfc_log_level_t level,
 *		     log_lvlfctx_t *fctx)
 *	Save log levels to the loglevel file.
 *
 *	`lattr' is a pointer to log attribute which determines the log level
 *	type to be saved.
 *
 *	`level' is the log level to be saved.
 *	PFC_LOGLVL_NONE means that the logging level should be reset.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the PFC log system lock.
 *
 *	- The caller must initialize `fctx' using LOG_LVLFCTX_INIT().
 */
static int
log_save_loglevel(log_cattr_t *lattr, pfc_log_level_t level,
		  log_lvlfctx_t *fctx)
{
	const char	*path = log_lvlpath;
	int		err;

	if (path == NULL) {
		err = 0;
	}
	else {
		/*
		 * Open loglevel file.
		 * A new loglevel file should be created if it does not exist.
		 */
		err = log_lvlfile_open(path, fctx, PFC_TRUE);
		if (PFC_EXPECT_TRUE(err == 0)) {
			log_lvlfile_t	*lfp = fctx->lvfc_level;

			/* Update logging level. */
			LOG_LVLCHAR_UPDATE(lattr, lfp, level);
		}
	}

	return err;
}

/*
 * static void
 * log_modlevel_init(pfc_cfblk_t block, int index)
 *	Set per-module logging level from "log_modlevel" parameter value.
 *	`index' must be an array index of "log_modlevel" parameter in the
 *	configuration block associated with `block'.
 */
static void
log_modlevel_init(pfc_cfblk_t block, int index)
{
	const char	*str, *lvlname;
	const log_lvl_t	*lvl;
	char		*buf, *sep;

	str = pfc_conf_array_stringat(block, conf_log_modlevel, index, NULL);
	if (PFC_EXPECT_FALSE(str == NULL)) {
		return;
	}

	buf = strdup(str);
	if (PFC_EXPECT_FALSE(buf == NULL)) {
		pfc_log_warn("Failed to copy string at log_modlevel[%d].",
			     index);

		return;
	}

	/* Search for a separator in this string. */
	sep = strchr(buf, LOG_MODLEVEL_SEP);
	if (PFC_EXPECT_FALSE(sep == NULL || sep == buf)) {
		pfc_log_warn("Ignore invalid value at log_modlevel[%d]: %s",
			     index, str);
		goto out;
	}

	*sep = '\0';
	lvlname = sep + 1;

	/* Determine log level. */
	for (lvl = log_level_attr; lvl < PFC_ARRAY_LIMIT(log_level_attr);
	     lvl++) {
		if (strcasecmp(lvlname, lvl->l_name) == 0) {
			pfc_log_level_t	level;
			int		err;

			level = (pfc_log_level_t)(lvl - log_level_attr);
			err = log_modlevel_set(buf, level);
			if (PFC_EXPECT_FALSE(err < 0)) {
				err = -err;
				pfc_log_warn("Failed to set logging level for "
					     "\"%s\": %s", buf, strerror(err));
			}
			goto out;
		}
	}

	pfc_log_warn("Ignore invalid logging level at log_modlevel[%d]: %s",
		     index, str);

out:
	free(buf);
}

/*
 * static int
 * log_modlevel_set(const char *modname, pfc_log_level_t level)
 *	Set logging level for the module specified by `modname'.
 *
 *	`modname' must be a pointer to string which represents module name.
 *
 * Calling/Exit State:
 *	Zero is returned if the logging level is changed successfully.
 *	1 is returned if the logging level is not changed.
 *	Otherwise negative error number defined in errno.h is returned.
 *
 * Remarks:
 *	The caller must ensure that `modname' is not NULL, and `level' keeps
 *	valid logging level.
 */
static int
log_modlevel_set(const char *modname, pfc_log_level_t level)
{
	log_modlevel_t	*lmp;
	pfc_rbnode_t	*node;
	int		ret = 0;

	PFC_LOG_LOCK();

	/* Search for the current logging level for the specified module. */
	node = LOG_MODLEVEL_GET(modname);
	if (node == NULL) {
		const char	*name;

		if (PFC_EXPECT_FALSE(log_modules.lmc_count >= INT32_MAX)) {
			/* Too many levels are configured. */
			ret = -EMFILE;
			goto out;
		}

		/* Insert new log level for the module. */
		name = strdup(modname);
		if (PFC_EXPECT_FALSE(name == NULL)) {
			ret = -ENOMEM;
			goto out;
		}

		lmp = (log_modlevel_t *)malloc(sizeof(*lmp));
		if (PFC_EXPECT_FALSE(lmp == NULL)) {
			free((void *)name);
			ret = -ENOMEM;
			goto out;
		}

		lmp->lml_name = name;
		lmp->lml_level = level;
		LOG_MODLEVEL_PUT(lmp);
	}
	else {
		lmp = LOG_NODE2MODLEVEL(node);
		if (lmp->lml_level == level) {
			ret = 1;
		}
		else {
			lmp->lml_level = level;
		}
	}

out:
	PFC_LOG_UNLOCK();

	if (ret == 0) {
		const log_lvl_t	*lvl = &log_level_attr[level];

		pfc_log_info("Logging level for \"%s\" has been changed to "
			     "%s.", modname, lvl->l_name);
	}

	return ret;
}

/*
 * static pfc_cptr_t
 * log_modlevel_getkey(pfc_rbnode_t *node)
 *	Return pointer to Red-Black tree key in the specified node.
 *	`node' must be a pointer to lml_node in log_modlevel_t.
 */
static pfc_cptr_t
log_modlevel_getkey(pfc_rbnode_t *node)
{
	log_modlevel_t	*lmp = LOG_NODE2MODLEVEL(node);

	return (pfc_cptr_t)lmp->lml_name;
}

/*
 * static void
 * log_modlevel_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destructor for log_modlevel_t.
 *	`node' must be a pointer to lml_node in log_modlevel_t.
 */
static void
log_modlevel_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	log_modlevel_t	*lmp = LOG_NODE2MODLEVEL(node);

	free((void *)lmp->lml_name);
	free(lmp);
}

/*
 * static pfc_log_level_t
 * log_current_level(const char *modname)
 *	Return current log level associated with the specified module.
 *
 *	`modname' must be a pointer to string which represents module name.
 *
 * Remarks:
 *	- This function must be called with holding the PFC log system lock.
 *
 *	- The caller must ensure that `modname' is not NULL.
 */
static pfc_log_level_t
log_current_level(const char *modname)
{
	pfc_rbnode_t	*node;

	if ((node = LOG_MODLEVEL_GET(modname)) == NULL) {
		/* Use global logging level. */
		return log_level;
	}

	return LOG_NODE2MODLEVEL(node)->lml_level;
}

#ifndef	__PFC_LOG_GNUC

/*
 * Below codes are validated only if non-gcc is used as C compiler.
 */

/*
 * Logging level for the current burst logging block.
 */
static __thread pfc_log_level_t	burst_log_level;
static __thread pfc_log_level_t	burst_syslog_level;

/*
 * pfc_log_level_t
 * __pfc_log_burst_set(pfc_log_level_t level)
 *	Set logging level for the burst logging block.
 */
pfc_log_level_t
__pfc_log_burst_set(pfc_log_level_t level)
{
	pfc_log_level_t	ret = burst_log_level;

	burst_log_level = level;

	return ret;
}

/*
 * void
 * __pfc_log_burst_write_v(const char *modname, const char *format, va_list ap)
 *	Record a log message with specifying logging level defined by
 *	the burst logging block.
 */
void
__pfc_log_burst_write_v(const char *modname, const char *format, va_list ap)
{
	pfc_log_common_impl(burst_log_level, modname, format, ap, PFC_FALSE);
}

#endif	/* !__PFC_LOG_GNUC */
