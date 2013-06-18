/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * main.c - Main routine of pfc_modcache command.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/stat.h>
#include <pfc/path.h>
#include <cmdopt.h>
#include <cmdutil.h>
#include <conf_impl.h>
#include "modcache_impl.h"

extern const char	copyright[];

/*
 * Program name.
 */
static const char	progname_default[] = "pfc_modcache";
static const char	*progname = progname_default;

/*
 * The name of the target daemon.
 */
static const char	daemon_name_default[] = "pfcd";
static const char	*daemon_name = daemon_name_default;

/*
 * Debugging level.
 */
static int	modcache_debug;

/*
 * Path to module directory.
 */
const char	*module_dir = NULL;

/*
 * Path to module cache directory.
 */
const char	*cache_dir = NULL;

/*
 * Determine we should wait for the module directory lock to be released.
 */
pfc_bool_t	module_lock_wait = PFC_FALSE;

/*
 * Umask for pfc_modcache.
 */
#define	MODCACHE_UMASK		(S_IWGRP | S_IWOTH)

/*
 * Help message.
 */
#define	HELP_MESSAGE_1						\
	"Module cache management tool for" PFC_LBRK_SPACE_STR
#define	HELP_MESSAGE_2							\
	".\nIf no option is specified, the contents of module cache "	\
	"is displayed."

/*
 * Command line options.
 */
static const char	*copt_desc[] = {
	"Specify path to configuration file.\n(default: ",
	PFC_PFCD_CONF_PATH,
	")",
	NULL,
};

#define	OPTCHAR_CONF		'C'
#define	OPTCHAR_MODULEDIR	'M'
#define	OPTCHAR_CACHEDIR	'c'
#define	OPTCHAR_UPDATE		'u'
#define	OPTCHAR_WAITLOCK	'w'
#define	OPTCHAR_DEBUG		'd'
#define	OPTCHAR_VERSION		'\v'

static const pfc_cmdopt_def_t	option_spec[] = {
	{OPTCHAR_CONF, "conf-file", PFC_CMDOPT_TYPE_STRING,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_ARRAYDESC,
	 (const char *)copt_desc, "FILE"},
	{OPTCHAR_MODULEDIR, "module-dir", PFC_CMDOPT_TYPE_STRING,
	 PFC_CMDOPT_DEF_ONCE,
	 "Specify path to module directory.", "DIR"},
	{OPTCHAR_CACHEDIR, "cache-dir", PFC_CMDOPT_TYPE_STRING,
	 PFC_CMDOPT_DEF_ONCE,
	 "Specify path to module cache directory.", "DIR"},
	{OPTCHAR_UPDATE, "update", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Update module cache.", NULL},
	{OPTCHAR_WAITLOCK, "wait-lock", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE,
	 "Wait for the module lock to be released.", NULL},
	{OPTCHAR_DEBUG, "debug", PFC_CMDOPT_TYPE_NONE, 0,
	 "Run with debugging message.", NULL},
	{OPTCHAR_VERSION, "version", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_LONGONLY,
	 "Print version information and exit.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Internal prototypes.
 */
static void	dump_version(void);
static void	sysconf_init(const char *conffile);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * copt_desc_update(const char *conffile)
 *	Set default configuration file path into description about "-C" option.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
copt_desc_update(const char *conffile)
{
	copt_desc[1] = conffile;
}

/*
 * int
 * main(int argc, char **argv)
 *	Start routine of pfc_modcache command.
 */
int
main(int argc, char **argv)
{
	pfc_cmdopt_t	*parser;
	pfc_bool_t	update = PFC_FALSE;
	const char	*conffile;
	pfc_refptr_t	*defconf, *dname;
	int		argidx;
	char		c;

	/* Use C locale. */
	(void)setlocale(LC_ALL, "C");

	/* Set timezone. */
	tzset();

	/* Initialize umask. */
	(void)umask(MODCACHE_UMASK);

	if (PFC_EXPECT_TRUE(argc > 0)) {
		/* Initialize program name. */
		dname = progname_init(*argv, &progname);
		if (PFC_EXPECT_FALSE(dname == NULL)) {
			fatal("Failed to create daemon name.");
			/* NOTREACHED */
		}

		daemon_name = pfc_refptr_string_value(dname);
	}
	else {
		dname = NULL;
	}

	/* Construct default configuration file path. */
	defconf = pfc_refptr_sprintf(PFC_SYSCONFDIR "/%s.conf", daemon_name);
	if (PFC_EXPECT_FALSE(defconf == NULL)) {
		fatal("Failed to create default configuration file path.");
		/* NOTREACHED */
	}

	conffile = pfc_refptr_string_value(defconf);
	copt_desc_update(conffile);

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(progname, argc, argv, option_spec, NULL, 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		fatal("Failed to create option parser.");
		/* NOTREACHED */
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case OPTCHAR_CONF:
			conffile = pfc_cmdopt_arg_string(parser);
			if (PFC_EXPECT_FALSE(*conffile == '\0')) {
				fatal("Configuration file path is empty.");
				/* NOTREACHED */
			}
			break;

		case OPTCHAR_MODULEDIR:
			module_dir = pfc_cmdopt_arg_string(parser);
			if (PFC_EXPECT_FALSE(*module_dir == '\0')) {
				fatal("Module directory path is empty.");
				/* NOTREACHED */
			}
			break;

		case OPTCHAR_CACHEDIR:
			cache_dir = pfc_cmdopt_arg_string(parser);
			if (PFC_EXPECT_FALSE(*cache_dir == '\0')) {
				fatal("Module cache directory path is empty.");
				/* NOTREACHED */
			}
			break;

		case OPTCHAR_DEBUG:
			/* Run as debug mode. */
			modcache_debug++;
			break;

		case OPTCHAR_UPDATE:
			/* Cache update mode. */
			update = PFC_TRUE;
			break;

		case OPTCHAR_WAITLOCK:
			module_lock_wait = PFC_TRUE;
			break;

		case OPTCHAR_VERSION:
			dump_version();
			/* NOTREACHED */

		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stdout);
			exit(PFC_EX_OK);
			/* NOTREACHED */

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help_list(parser, stdout, HELP_MESSAGE_1,
					     daemon_name, HELP_MESSAGE_2,
					     NULL);
			exit(PFC_EX_OK);
			/* NOTREACHED */

		case PFC_CMDOPT_ERROR:
			exit(PFC_EX_FATAL);
			/* NOTREACHED */

		default:
			fatal("Failed to parse command line options.");
			/* NOTREACHED */
		}
	}

	if ((argidx = pfc_cmdopt_validate(parser)) == -1) {
		fatal("Invalid command line options.");
		/* NOTREACHED */
	}
	pfc_cmdopt_destroy(parser);

	/* Import system configuration. */
	sysconf_init(conffile);
	pfc_refptr_put(defconf);

	/* Ensure that the module directory is safe. */
	path_verify(module_dir);
	debug_printf(1, "module_dir = %s", module_dir);
	debug_printf(1, "cache_dir = %s", cache_dir);

	if (update) {
		/* Update module cache. */
		cache_update();
	}
	else {
		/* Dump the contents of the cache. */
		cache_dump();
	}

	if (PFC_EXPECT_TRUE(dname != NULL)) {
		daemon_name = daemon_name_default;
		pfc_refptr_put(dname);
	}

	return PFC_EX_OK;
}

/*
 * void
 * fatal(const char *fmt, ...)
 *	Print error message to the standard error output and die.
 */
void
fatal(const char *fmt, ...)
{
	va_list	ap;

	fprintf(stderr, "*** %s: ERROR: ", progname);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);

	exit(PFC_EX_FATAL);
	/* NOTREACHED */
}

/*
 * void
 * warning(const char *fmt, ...)
 *	Print warning message to the standard error output.
 */
void
warning(const char *fmt, ...)
{
	va_list	ap;

	fprintf(stderr, "*** %s: WARNING: ", progname);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

/*
 * void
 * debug_printf(int level, const char *fmt, ...)
 *	Print debugging message if current debugging level is higher than or
 *	equal to the specified level.
 */
void
debug_printf(int level, const char *fmt, ...)
{
	PFC_ASSERT(level > 0);

	if (level <= modcache_debug) {
		va_list	ap;
		int	i;

		for (i = 0; i < level; i++) {
			fputc('*', stdout);
		}
		fputc(' ', stdout);
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
		fputc('\n', stdout);
	}
}

/*
 * pfc_flock_t
 * module_dir_lock(pfc_bool_t write_mode)
 *	Acquire module cache directory lock.
 *	If `write_mode' is true, module_dir_lock() tries to acquire exclusive
 *	directory lock. Otherwise it tries to acquire shared lock.
 */
pfc_flock_t
module_dir_lock(pfc_bool_t write_mode)
{
	pfc_flock_t	lk;
	int	err, oflags;
	pid_t	owner, *ownerp;

	if (write_mode) {
		oflags = O_RDWR | O_CREAT;
	}
	else {
		oflags = O_RDONLY | O_CREAT;
	}

	err = pfc_flock_opendir(&lk, cache_dir, oflags, MODCH_LOCK_PERM);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == EACCES) {
			not_allowed();
		}
		else {
			fatal("Failed to open module directory lock handle: "
			      "%s", strerror(err));
		}
		/* NOTREACHED */
	}

	ownerp = (module_lock_wait) ? NULL : &owner;
	if (write_mode) {
		err = pfc_flock_wrlock(lk, ownerp);
	}
	else {
		err = pfc_flock_rdlock(lk, ownerp);
	}
	if (PFC_EXPECT_FALSE(err == EAGAIN)) {
		fatal("Another process is holding module directory lock: "
		      "pid = %d", owner);
		/* NOTREACHED */
	}
	else if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to acquire module directory lock: %s",
		      strerror(err));
		/* NOTREACHED */
	}

	return lk;
}

/*
 * void
 * module_dir_unlock(pfc_flock_t lock)
 *	Release module directory lock held by the call of module_dir_lock().
 */
void
module_dir_unlock(pfc_flock_t lock)
{
	PFC_ASSERT_INT(pfc_flock_closedir(lock), 0);
}

/*
 * static void
 * dump_version(void)
 *	Dump system version and exit.
 */
static void
dump_version(void)
{
	printf("%s (%s) %s%s\n\n%s\n",
	       progname, PFC_PRODUCT_NAME,
	       PFC_VERSION_STRING, PFC_BUILD_TYPE_SUFFIX, copyright);

	exit(PFC_EX_OK);
	/* NOTREACHED */
}

/*
 * static void
 * sysconf_init(const char *conffile)
 *	Import system configuration.
 *
 * Remarks:
 *	This function changes the user and group ID of the process if they
 *	are specified in the configuration file.
 */
static void
sysconf_init(const char *conffile)
{
	pfc_refptr_t	*rconf;
	pfc_cfblk_t	opts;
	int		err;

	/* Ensure that the configuration file is safe. */
	path_verify(conffile);

	/* Create refptr string that keeps configuration file path. */
	rconf = pfc_refptr_string_create(conffile);
	if (PFC_EXPECT_FALSE(rconf == NULL)) {
		fatal("Failed to duplicate configuration file path.");
		/* NOTREACHED */
	}

	/* Load PFC system configuration file. */
	err = pfc_sysconf_init(rconf);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to load system configuration file.");
		/* NOTREACHED */
	}
	pfc_refptr_put(rconf);

	/* Derive module directory from configuration file. */
	opts = pfc_sysconf_get_block("options");

	if (module_dir == NULL) {
		/* Derive module directory from the configuration file. */
		module_dir = pfc_conf_get_string(opts, "module_dir",
						 PFC_MODULEDIR);
	}

	if (cache_dir == NULL) {
		/*
		 * Derive module cache directory from the configuration
		 * file.
		 */
		cache_dir = pfc_conf_get_string(opts, "modcache_dir",
						module_dir);
	}

	/*
	 * Switch user and group to the same as the daemon specified by
	 * the configuration file.
	 * pfccmd_switchuser() never returns on error because it will
	 * call fatal().
	 */
	PFC_ASSERT_INT(pfccmd_switchuser(opts, PFC_TRUE, fatal), 0);
	if (modcache_debug >= 1) {
		debug_printf(1, "uid=%d, gid=%d", getuid(), getgid());
	}
}

/*
 * void
 * path_verify(const char *path)
 *	Ensure that the specified file path can be used safely.
 *
 * Calling/Exit State:
 *	Program exits on error.
 */
void
path_verify(const char *path)
{
	char	*cpath;
	int	err;

	/*
	 * We need to copy the specified path because pfc_is_safepath()
	 * always breaks the string.
	 */
	cpath = strdup(path);
	if (PFC_EXPECT_FALSE(cpath == NULL)) {
		fatal("Failed to duplicate file path.");
		/* NOTREACHED */
	}

	err = pfc_is_safepath(cpath);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ENOENT) {
			fatal("%s: File does not exist.", path);
		}
		else {
			fatal("%s: File is unsafe.: %s: %s",
			      path, cpath, strerror(err));
		}
		/* NOTREACHED */
	}
	free(cpath);
}

/*
 * void
 * not_allowed(void)
 *	Print an error message that indicates the current user is not allowed
 *	to run this program.
 */
void
not_allowed(void)
{
	fatal("You are not allowed to use this program.");
	/* NOTREACHED */
}
