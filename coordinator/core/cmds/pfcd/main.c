/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * main.c - Main routine of PFC daemon.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pfc/refptr.h>
#include <pfc/synch.h>
#include <pfc/debug.h>
#include <pfc/clock.h>
#include <pfc/thread.h>
#include <pfc/exstatus.h>
#include <pfc/atomic.h>
#include <pfc/extcmd.h>
#include <conf_impl.h>
#include <property_impl.h>
#include <cmdopt.h>
#include "pfcd.h"
#include "ipc.h"

#define	LABEL_WIDTH	8

/*
 * Program name.
 */
static const char	pfcd_progname_default[] = "pfcd";
static pfc_refptr_t	*pfcd_rprogname;
const char		*pfcd_progname = pfcd_progname_default;

/*
 * Daemon mode.
 */
pfcd_mode_t		pfcd_mode = PFCD_MODE_DAEMON;

/*
 * Daemon state.
 */
static pfcd_state_t	pfcd_state = PFCD_STATE_BOOTING;

/*
 * Daemon state lock.
 */
static pfc_rwlock_t	pfcd_state_lock = PFC_RWLOCK_INITIALIZER;

#define	PFCD_STATE_RDLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&pfcd_state_lock), 0)
#define	PFCD_STATE_WRLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&pfcd_state_lock), 0)
#define	PFCD_STATE_UNLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&pfcd_state_lock), 0)

/*
 * Thread ID for the main thread.
 */
pthread_t	pfcd_main_thread;

/*
 * Debug level.
 */
uint32_t	pfcd_debug;

/*
 * Process ID of PFC daemon.
 */
pid_t		pfcd_pid;

/*
 * PID file handle.
 */
static pfc_pidf_t	pfcd_pidfile;

/*
 * Global option block handle.
 */
pfc_cfblk_t		pfcd_options;

/*
 * True if stdin/stdout redirection is disabled.
 */
static pfc_bool_t	pfcd_no_redirect = PFC_FALSE;

/*
 * Determine whether PID file was installed or not.
 */
static pfc_bool_t	pfcd_has_pidfile = PFC_FALSE;

/*
 * Exit status of PFC daemon.
 */
static int		pfcd_exit_status = PFC_EX_OK;

/*
 * Only one ticket to call exit(3).
 */
static pfc_bool_t	pfcd_exit_ticket = PFC_FALSE;

/*
 * How long, in seconds, we should wait for completion of concurrent call
 * of exit(3).
 */
#define	PFCD_EXIT_TIMEOUT		10U

/*
 * How long, in seconds, we should wait for signal from the daemon.
 */
#define	PFCD_DAEMONIZE_TIMEOUT		10U

/*
 * Umask for pfcd.
 */
#define	PFCD_UMASK	(S_IWGRP | S_IWOTH)

/*
 * Help message.
 */
#define	PFCD_HELP_MESSAGE						\
	".\n\n"								\
	"All arguments are considered as module name to be loaded. "	\
	"If modules to be loaded are also defined in the "		\
	"configuration file, all modules defined in the configuration "	\
	"file and modules specified by arguments are loaded."

/*
 * Brief description of arguments.
 */
#define	PFCD_ARG_FORMAT		"[module ...]"

/*
 * Command line options.
 */
static const char	*pfcd_copt_desc[] = {
	"Specify path to configuration file.\n(default: ",
	PFC_PFCD_CONF_PATH,
	")",
	NULL,
};

#define	OPTCHAR_CONF		'C'
#define	OPTCHAR_ALL		'a'
#define	OPTCHAR_CHECK		'c'
#define	OPTCHAR_DEBUG		'd'
#define	OPTCHAR_NODAEMON	'D'
#define	OPTCHAR_NOREDIRECT	'R'
#define	OPTCHAR_PROPERTY	'P'
#define	OPTCHAR_NOLOADMOD	'N'
#define	OPTCHAR_VERSION		'\v'

static const pfc_cmdopt_def_t	pfcd_option_spec[] = {
	{OPTCHAR_CONF, "conf-file", PFC_CMDOPT_TYPE_STRING,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_ARRAYDESC,
	 (const char *)pfcd_copt_desc, "FILE"},
	{OPTCHAR_ALL, "all", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Load all modules at the daemon boot.", NULL},
	{OPTCHAR_CHECK, "check", PFC_CMDOPT_TYPE_NONE, 0,
	 "Validate configuration file and exit.\n"
	 "Configuration files for all installed modules are also validated. "
	 "In that case both \"modules.load_modules\" in the configuration "
	 "file and command line arguments are ignored.", NULL},
	{OPTCHAR_DEBUG, "debug", PFC_CMDOPT_TYPE_NONE, 0,
	 "Run in foreground with sending verbose message to stderr.", NULL},
	{OPTCHAR_NODAEMON, "no-daemon", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE,
	 "Don't daemonize for monitoring.", NULL},
	{OPTCHAR_NOREDIRECT, "no-redirect", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE,
	 "Don't redirect stdin and stdout to /dev/null.\n"
	 "This option also disables stderr logging.", NULL},
	{OPTCHAR_PROPERTY, "property", PFC_CMDOPT_TYPE_STRING, 0,
	 "Set PFC daemon system property.", "KEY=VALUE"},
	{OPTCHAR_NOLOADMOD, "no-load-modules", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE,
	 "Ignore \"modules.load_modules\" option in the configuration file.",
	 NULL},
	{OPTCHAR_VERSION, "version", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_LONGONLY,
	 "Print version information and exit.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Internal prototypes.
 *
 * Remarks:
 *	Functions which is called from main() should be prevented from
 *	inline optimization in order to reduce main thread's stack.
 */
static void	dump_version(void);
static void	pfcd_set_error(void);
static void	pfcd_exit(void) PFC_FATTR_NOINLINE PFC_FATTR_NORETURN;
static void	pfcd_do_exit(int status) PFC_FATTR_NOINLINE PFC_FATTR_NORETURN;
static void	pfcd_mode_error(void) PFC_FATTR_NORETURN;
static void	fatal_log_handler(void);
static void	fatal_log_die(void) PFC_FATTR_NORETURN;
static void	conf_error(const char *fmt, va_list ap);
static void	setup(int argc, char **argv) PFC_FATTR_NOINLINE;
static void	pfcd_progname_init(const char *arg0);
static void	sysconf_init(const char *conffile);
static void	log_earlyinit(FILE *logfp, pfc_log_level_t lvl);
static void	log_init(void);
static void	daemonize(void);
static void	verify_daemon(pid_t pid, pfc_pidf_t pf, sigset_t *mask);
static void	open_pidfile(pfc_pidf_t *pfp);
static void	check_pidfile(pfc_pidf_t pf);
static void	install_pidfile(pfc_pidf_t pf);
static void	daemon_foreground_init(void);
static void	daemon_common_init(void);
static void	cleanup(void);
static void	pfcd_boot(void) PFC_FATTR_NOINLINE;
static void	pfcd_shutdown(void) PFC_FATTR_NOINLINE PFC_FATTR_NORETURN;
static void	pfcd_shutdown_common(void) PFC_FATTR_NORETURN;
static void	prop_add(const char *arg);

/*
 * Path to directory which contains static attributes file.
 */
#define	DAEMON_ATTR_DIR		PFC_DATADIR "/daemon"

/*
 * Definition of static attributes file.
 */
extern pfc_cfdef_t	pfcd_attr_defs;

/*
 * Configuration file handle associated with static attributes.
 */
static pfc_conf_t	attr_conf = PFC_CONF_INVALID;
static pfc_cfblk_t	attr_block = PFC_CFBLK_INVALID;

static void		daemon_attr_init(void);
static void		daemon_attr_fini(void);
static const char	*daemon_attr_getdesc(void);
static const char	*daemon_attr_getident(void);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfcd_copt_desc_update(const char *conffile)
 *	Set default configuration file path into description about "-C" option.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfcd_copt_desc_update(const char *conffile)
{
	pfcd_copt_desc[1] = conffile;
}

/*
 * int
 * main(int argc, char **argv)
 *	Start routine of PFC daemon.
 */
int
main(int argc, char **argv)
{
	setup(argc, argv);

	/* Boot PFC service. */
	pfcd_boot();

	/* Main thread becomes a daemon control thread. */
	ctrl_main();

	/* Start PFC shutdown sequence. */
	pfcd_shutdown();
	/* NOTREACHED */
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

	fprintf(stderr, "*** %s: ERROR: ", pfcd_progname);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);

	pfcd_do_exit(PFC_EX_FATAL);
	/* NOTREACHED */
}

/*
 * void
 * fatal_status(int status, const char *fmt, ...)
 *	Print error message to the standard error output, and exit with
 *	the specified status.
 */
void
fatal_status(int status, const char *fmt, ...)
{
	va_list	ap;

	fprintf(stderr, "*** %s: ERROR: ", pfcd_progname);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);

	pfcd_do_exit(status);
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

	fprintf(stderr, "*** %s: WARNING: ", pfcd_progname);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

/*
 * void
 * die(const char *fmt, ...)
 *	Record fatal error log message, and die immediately.
 */
void
die(const char *fmt, ...)
{
	va_list	ap;

	PFCD_STATE_WRLOCK();
	pfcd_set_error();
	pfcd_state = PFCD_STATE_DEAD;
	PFCD_STATE_UNLOCK();

	va_start(ap, fmt);
	pfc_log_fatal_v(fmt, ap);
	va_end(ap);

	/* This should not happen. */
	abort();
	/* NOTREACHED */
}

#ifdef	PFC_VERBOSE_DEBUG

#ifdef	__GNUC__

/*
 * void
 * verbose(const char *fmt, ...)
 *	Print verbose message.
 */
void
verbose(const char *fmt, ...)
{
	va_list	ap;

	if (pfcd_debug > 1) {
		fprintf(stderr, "*** %s: VERBOSE: ", pfcd_progname);
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fputc('\n', stderr);
	}
}

#else	/* !__GNUC__ */

/*
 * void
 * verbose_v(const char *fmt, va_list ap)
 *	Print verbose message using va_list.
 */
void
verbose_v(const char *fmt, va_list ap)
{
	if (pfcd_debug > 1) {
		fprintf(stderr, "*** %s: VERBOSE: ", pfcd_progname);
		vfprintf(stderr, fmt, ap);
		fputc('\n', stderr);
	}
}

#endif	/* __GNUC__ */

#endif	/* PFC_VERBOSE_DEBUG */

/*
 * pfcd_state_t
 * pfcd_getstate(void)
 *	Return current state of PFC daemon.
 */
pfcd_state_t
pfcd_getstate(void)
{
	pfcd_state_t	state;

	PFCD_STATE_RDLOCK();
	state = pfcd_state;
	PFCD_STATE_UNLOCK();

	return state;
}

/*
 * void
 * pfcd_setstate(pfcd_state_t state)
 *	Update PFC daemon state.
 */
void
pfcd_setstate(pfcd_state_t state)
{
	PFCD_STATE_WRLOCK();
	pfcd_state = state;
	PFCD_STATE_UNLOCK();
}

/*
 * int
 * pfcd_getexstatus(void)
 *	Return the current exit status of the PFC daemon that is passed to
 *	succeeding call of exit(3).
 */
int
pfcd_getexstatus(void)
{
	int	status;

	PFCD_STATE_RDLOCK();
	status = pfcd_exit_status;
	PFCD_STATE_UNLOCK();

	return status;
}

/*
 * static void
 * dump_version(void)
 *	Dump system version and exit.
 */
static void
dump_version(void)
{
	printf("%s (%s) %s%s\n  %*s: %s\n",
	       pfcd_progname, PFC_PRODUCT_NAME,
	       PFC_VERSION_STRING, PFC_BUILD_TYPE_SUFFIX,
	       LABEL_WIDTH, "Compiled", build_stamp);

#ifdef	PFC_BUILD_ID
	printf("  %*s: %s\n", LABEL_WIDTH, "Build ID", PFC_BUILD_ID);
#endif	/* PFC_BUILD_ID */
#ifdef	PFC_SCM_REVISION
	printf("  %*s: %s\n", LABEL_WIDTH, "SCM Rev", PFC_SCM_REVISION);
#endif	/* PFC_SCM_REVISION */

	printf("\n%s\n", copyright);

	pfcd_do_exit(PFC_EX_OK);
	/* NOTREACHED */
}

/*
 * static void
 * pfcd_set_error(void)
 *	Change exit state of pfcd to error state.
 *
 * Remarks:
 *	This function must be called with holding global state lock in
 *	writer mode.
 */
static void
pfcd_set_error(void)
{
	int	status = pfcd_exit_status;

	if (status == PFC_EX_OK) {
		if (pfcd_state < PFCD_STATE_MODLOAD) {
			status = PFC_EX_FATAL;
		}
		else {
			status = PFC_EX_TEMPFAIL;
		}
		pfcd_exit_status = status;
	}
}

/*
 * static void
 * pfcd_exit(void)
 *	Exit PFC daemon with the exit status in pfcd_exit_status.
 */
static void
pfcd_exit(void)
{
	int	status;

	PFCD_STATE_RDLOCK();
	status = pfcd_exit_status;
	PFCD_STATE_UNLOCK();

	if (status == PFC_EX_OK) {
		pfc_log_info("Quit %s: pid=%u", pfcd_progname, pfcd_pid);
	}
	else {
		pfc_log_error("Quit %s with error status: %d: pid=%u",
			      pfcd_progname, status, pfcd_pid);
	}

	pfcd_do_exit(status);
	/* NOTREACHED */
}

/*
 * static void
 * pfcd_do_exit(int status)
 *	Call exit(3).
 *	The purpose of this function is to block concurrent call of exit(3).
 */
static void
pfcd_do_exit(int status)
{
	pfc_bool_t	ticket;
	struct timespec	timeout, remain;

	/* Kill all children by force. */
	child_killall();

	/* Acquire ticket to call exit(3). */
	ticket = pfc_atomic_swap_uint8(&pfcd_exit_ticket, PFC_TRUE);
	if (PFC_EXPECT_TRUE(!ticket)) {
		/* This is the first call. */

		if (PFC_EXPECT_FALSE(status != PFC_EX_OK)) {
			/*
			 * On abnormal exit, exit(3) should not be called
			 * in order to avoid unexpected call of C++ object
			 * destructor.
			 */

			/* Clean up the PFC daemon resources. */
			cleanup();

			/* Flush all stdio output streams. */
			fflush(NULL);

			_exit(status);
			/* NOTREACHED */
		}

		exit(status);
		/* NOTREACHED */
	}

	/*
	 * exit(3) is already called by another thread. We must wait for
	 * completion of exit(3) call.
	 * Error log should be printed to the standard error output because
	 * the PFC log may be already disabled.
	 */
	fprintf(stderr, "*** %s: ERROR: Concurrent call of exit(): "
		"pid=%u, status=%d\n", pfcd_progname, pfcd_pid, status);

	timeout.tv_sec = PFCD_EXIT_TIMEOUT;
	timeout.tv_nsec = 0;
	while (nanosleep(&timeout, &remain) != 0) {
		int	err = errno;

		if (PFC_EXPECT_FALSE(err != EINTR)) {
			/* This should never happen. */
			break;
		}
		timeout = remain;
	}

	fprintf(stderr, "*** %s: ERROR: exit() call seems stuck: pid=%u\n",
		pfcd_progname, pfcd_pid);
	abort();
	/* NOTREACHED */
}

/*
 * static void
 * pfcd_mode_error(void)
 *	Display mode error message and die.
 */
static void
pfcd_mode_error(void)
{
	fatal("-D and -d options are mutually exclusive.");
	/* NOTREACHED */
}

/*
 * static void
 * fatal_log_handler(void)
 *	Fatal error handler which is called from pfc_log_fatal().
 */
static void
fatal_log_handler(void)
{
	int	err;

	/*
	 * At first, change fatal log handler to fatal_log_die().
	 * So pfc_log_fatal() call in this function never causes the recursive
	 * call of fatal_log_handler().
	 */
	pfc_log_set_fatal_handler(fatal_log_die);

	PFCD_STATE_WRLOCK();

	if (pfcd_state == PFCD_STATE_FATAL || pfcd_state == PFCD_STATE_DEAD ||
	    pfcd_state < PFCD_STATE_MODLOAD) {
		/* We must die here. */
		PFCD_STATE_UNLOCK();
		fatal_log_die();
		/* NOTREACHED */
	}

	pfcd_exit_status = PFC_EX_TEMPFAIL;

	if (PFC_EXPECT_FALSE(pfcd_state == PFCD_STATE_SHUTDOWN)) {
		PFCD_STATE_UNLOCK();
		pfc_log_fatal("Fatal error during shutdown. Force to quit.");
		pfcd_exit();
		/* NOTREACHED */
	}

	pfcd_state = PFCD_STATE_FATAL;
	PFCD_STATE_UNLOCK();

	/* Start shutdown sequence. */
	err = pthread_kill(pfcd_main_thread, SIGTERM);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to send shutdown signal: %s",
			      strerror(err));
		pfcd_exit();
		/* NOTREACHED */
	}
}

/*
 * static void
 * fatal_log_die(void)
 *	Fatal error handler which is called from pfc_log_fatal().
 *
 *	fatal_log_die() is set as fatal log handler in the first call of
 *	pfc_log_fatal(). This function simply exits the program.
 */
static void
fatal_log_die(void)
{
	PFCD_STATE_WRLOCK();
	pfcd_set_error();
	PFCD_STATE_UNLOCK();

	pfcd_exit();
	/* NOTREACHED */
}

/*
 * static void
 * conf_error(const char *fmt, va_list ap)
 *	Error message handler for configuration file parser.
 */
static void
conf_error(const char *fmt, va_list ap)
{
	/* Use PFC log. */
	pfc_log_error_v(fmt, ap);
}

/*
 * static void
 * setup(int argc, char **argv)
 *	Do early setup for PFC daemon.
 */
static void
setup(int argc, char **argv)
{
	pfc_cmdopt_t	*parser;
	const char	*conffile;
	pfc_refptr_t	*defconf;
	pfc_bool_t	checkonly = PFC_FALSE;
	pfc_log_level_t	lvl = PFC_LOGLVL_NONE;
	uint32_t	modflags = 0;
	FILE	*logfp = stderr;
	int	argidx;
	char	c;

	/* Use C locale. */
	(void)setlocale(LC_ALL, "C");

	/* Set timezone. */
	tzset();

	/* Initialize umask. */
	(void)umask(PFCD_UMASK);

	/* Preserve thread ID of the main thread. */
	pfcd_main_thread = pthread_self();

	if (argc > 0) {
		/* Initialize program name. */
		pfcd_progname_init(*argv);
	}

	/* Initialize static attributes for the daemon. */
	daemon_attr_init();

	/* Construct default configuration file path. */
	defconf = pfc_refptr_sprintf(PFC_SYSCONFDIR "/%s.conf",
				     pfcd_progname);
	if (PFC_EXPECT_FALSE(defconf == NULL)) {
		fatal("Failed to create default configuration file path.");
		/* NOTREACHED */
	}

	conffile = pfc_refptr_string_value(defconf);
	pfcd_copt_desc_update(conffile);

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(pfcd_progname, argc, argv, pfcd_option_spec,
				 PFCD_ARG_FORMAT, 0);
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

		case OPTCHAR_ALL:
			modflags |= PFCD_MODF_ALL;
			break;

		case OPTCHAR_CHECK:
			checkonly = PFC_TRUE;
			break;

		case OPTCHAR_DEBUG:
			/* Run as debug mode. */
			if (PFC_EXPECT_FALSE(pfcd_mode ==
					     PFCD_MODE_FGDAEMON)) {
				pfcd_mode_error();
				/* NOTREACHED */
			}

			lvl = (lvl == PFC_LOGLVL_NONE)
				? PFC_LOGLVL_DEBUG : PFC_LOGLVL_VERBOSE;

			pfcd_mode = PFCD_MODE_DEBUG;
			pfcd_debug++;
			break;

		case OPTCHAR_NODAEMON:
			/* Foreground daemon mode. */
			if (PFC_EXPECT_FALSE(pfcd_mode == PFCD_MODE_DEBUG)) {
				pfcd_mode_error();
				/* NOTREACHED */
			}

			/*
			 * Foreground daemon must be invoked by a process
			 * which has no control tty.
			 */
			if (PFC_EXPECT_FALSE(tcgetpgrp(0) != -1)) {
				fatal("Foreground daemon can't be invoked "
				      "from TTY.");
				/* NOTREACHED */
			}

			pfcd_mode = PFCD_MODE_FGDAEMON;
			break;

		case OPTCHAR_NOREDIRECT:
			/* No redirection mode. */
			pfcd_no_redirect = PFC_TRUE;
			break;

		case OPTCHAR_PROPERTY:
			/* Append system property. */
			prop_add(pfc_cmdopt_arg_string(parser));
			break;

		case OPTCHAR_NOLOADMOD:
			/*
			 * Ignore modules.load_modules in the configuration
			 * file.
			 */
			modflags |= PFCD_MODF_NOCONF;
			break;

		case OPTCHAR_VERSION:
			dump_version();
			/* NOTREACHED */

		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stdout);
			pfcd_do_exit(PFC_EX_OK);
			/* NOTREACHED */

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help_list(parser, stdout,
					     daemon_attr_getdesc(),
					     PFCD_HELP_MESSAGE, NULL);
			pfcd_do_exit(PFC_EX_OK);
			/* NOTREACHED */

		case PFC_CMDOPT_ERROR:
			pfcd_do_exit(PFC_EX_FATAL);
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

	/* Close all file descriptors greater than or equal to 3. */
	pfc_closefrom(3);

	argc -= argidx;
	argv += argidx;

	/* Do early signal initialization. */
	signal_bootstrap();

	/* Load PFC system configuration file. */
	sysconf_init(conffile);
	pfc_refptr_put(defconf);

	/*
	 * Switch user and group as per pfcd.conf.
	 * pfccmd_switchuser() never returns on error because it will call
	 * fatal().
	 */
	PFC_ASSERT_INT(pfccmd_switchuser(pfcd_options, PFC_TRUE, fatal), 0);
	verbose("uid=%d, gid=%d", getuid(), getgid());

	if (checkonly && pfcd_debug == 0) {
		/* Drop all PFC logs. */
		logfp = fopen("/dev/null", "r");
		if (PFC_EXPECT_FALSE(logfp == NULL)) {
			fatal("Failed to open /dev/null: %s", strerror(errno));
			/* NOTREACHED */
		}
	}

	/*
	 * Initialize the logging system to dump early logs to the standard
	 * error output.
	 */
	log_earlyinit(logfp, lvl);

	/* Set up PFC system file path. */
	path_init();

	if (checkonly) {
		/* Verify module configuration files. */
		module_check_conf();
		pfcd_do_exit(PFC_EX_OK);
		/* NOTREACHED */
	}

	/* Enable external command execution logging. */
	pfc_extcmd_enable_log(PFC_TRUE);

	/* Initialize PFC modules. */
	module_init(argc, argv, modflags);

	/* Daemonize unless debug option is specified. */
	if (pfcd_debug == 0) {
		daemonize();
	}
	else {
		/* Do initialization specific on foreground mode. */
		daemon_foreground_init();
	}

	pfc_log_info("Start %s: %s%s (%s): pid=%u",
		     pfcd_progname, PFC_VERSION_STRING, PFC_BUILD_TYPE_SUFFIX,
		     build_stamp, pfcd_pid);
}

/*
 * static void
 * pfcd_progname_init(const char *arg0)
 *	Initialize pfcd_progname by argv[0].
 */
static void
pfcd_progname_init(const char *arg0)
{
	const char	*name;

	name = strrchr(arg0, '/');
	if (name == NULL) {
		name = arg0;
	}
	else {
		name++;
	}

	if (PFC_EXPECT_TRUE(*name != '\0')) {
		pfc_refptr_t	*rname;

		rname = pfc_refptr_string_create(name);
		if (PFC_EXPECT_FALSE(rname == NULL)) {
			fatal("Failed to initialize program name.");
			/* NOTREACHED */
		}

		pfcd_progname = pfc_refptr_string_value(rname);
		pfcd_rprogname = rname;
	}
}

/*
 * static void
 * sysconf_init(const char *conffile)
 *	Load PFC system configuration file.
 */
static void
sysconf_init(const char *conffile)
{
	pfc_refptr_t	*rconf;
	int	err;

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

	/* Obtain block handle for global options. */
	pfcd_options = pfc_sysconf_get_block("options");
}

/*
 * static void
 * log_earlyinit(FILE *logfp, pfc_log_level_t lvl)
 *	Initialize the logging system to dump early logs to the standard
 *	error output.
 */
static void
log_earlyinit(FILE *logfp, pfc_log_level_t lvl)
{
	pfc_log_conf_t	logconf;
	pfc_logconf_early(&logconf, pfcd_options, pfcd_progname, logfp,
			  lvl, fatal_log_handler);
	pfc_log_sysinit(&logconf);
}

/*
 * static void
 * log_init(void)
 *	Initialize the message logging for the daemon process.
 */
static void
log_init(void)
{
	size_t		dlen = strlen(pfcd_work_dir);
	pfc_log_conf_t	logconf;

	pfc_logconf_init(&logconf, pfcd_options, daemon_attr_getident(),
			 fatal_log_handler);
	pfc_logconf_setpath(&logconf, pfcd_work_dir, dlen,
			    PFC_PFCD_LOG_NAME, PFC_PFCD_LOG_NAMELEN,
			    PFC_MSGLOG_NAME, PFC_MSGLOG_NAMELEN);
	pfc_logconf_setlvlpath(&logconf, pfcd_work_dir, dlen,
			       PFC_LOGLVL_NAME, PFC_LOGLVL_NAMELEN);
	pfc_log_sysinit(&logconf);
}

/*
 * static void
 * daemonize(void)
 *	Spawn one child process, and let it run as a daemon process.
 */
static void
daemonize(void)
{
	pfc_pidf_t	pf;
	int		err;

	/* Open PID file. */
	open_pidfile(&pf);

	if (pfcd_mode == PFCD_MODE_DAEMON) {
		sigset_t	mask;
		pid_t	pid;

		/*
		 * We need to mask signals to receive signal from the
		 * daemon safely.
		 */
		sigemptyset(&mask);
		sigaddset(&mask, SIGCHLD);
		sigaddset(&mask, SIGUSR1);
		err = pthread_sigmask(SIG_BLOCK, &mask, NULL);
		if (PFC_EXPECT_FALSE(err != 0)) {
			fatal("Failed to set signal mask: %s", strerror(err));
			/* NOTREACHED */
		}

		/* Spawn a child process. */
		pid = fork();
		if (PFC_EXPECT_FALSE(pid == (pid_t)-1)) {
			fatal("fork() failed: %s", strerror(errno));
			/* NOTREACHED */
		}

		if (pid != 0) {
			verify_daemon(pid, pf, &mask);

			/* Rest of work will be done by a daemon. */
			pfcd_do_exit(PFC_EX_OK);
			/* NOTREACHED */
		}
	}

	/* At first, we should initialize the logging system. */
	log_init();

	/* Static attributes are no longer used. */
	daemon_attr_fini();

	/* Install my process ID to the PID file. */
	install_pidfile(pf);

	if (pfcd_mode == PFCD_MODE_DAEMON) {
		/* Create a new session. */
		if (PFC_EXPECT_FALSE(setsid() == -1)) {
			pfc_log_fatal("setsid() failed: %s", strerror(errno));
			/* NOTREACHED */
		}
	}

	if (!pfcd_no_redirect) {
		int	nullfd;

		/* Bind /dev/null to the standard input and output. */
		nullfd = open("/dev/null", O_RDWR);
		if (PFC_EXPECT_FALSE(nullfd == -1)) {
			pfc_log_fatal("open(/dev/null) failed: %s",
				      strerror(errno));
			/* NOTREACHED */
		}

		if (PFC_EXPECT_FALSE(nullfd != 0 && dup2(nullfd, 0) == -1)) {
			pfc_log_fatal("dup2(stdin) failed: %s",
				      strerror(errno));
			/* NOTREACHED */
		}
		if (PFC_EXPECT_FALSE(nullfd != 1 && dup2(nullfd, 1) == -1)) {
			pfc_log_fatal("dup2(stdout) failed: %s",
				      strerror(errno));
			/* NOTREACHED */
		}

		/* Store outputs to the standard error output to a file. */
		stderr_init(nullfd);
	}

	/* Do common initialization. */
	daemon_common_init();

	if (pfcd_mode == PFCD_MODE_DAEMON) {
		/* Send SIGUSR1 to the parent. */
		kill(getppid(), SIGUSR1);
	}
}

/*
 * static void
 * verify_daemon(pid_t pid, pfc_pidf_t pf, sigset_t *mask)
 *	Ensure that the daemon process has been started successfully.
 *	`pid' must be a process ID for the daemon process, and
 *	`pf' must be a PID file handle. `mask' is a signal set for which
 *	we should wait. It must contains SIGCHLD and SIGUSR1.
 */
static void
verify_daemon(pid_t pid, pfc_pidf_t pf, sigset_t *mask)
{
	struct timespec	ts;
	pfc_timespec_t	expire;
	siginfo_t	sinfo;
	int	sig, err, status;

	err = pfc_clock_gettime(&expire);
	if (PFC_EXPECT_FALSE(err != 0)) {
		kill(pid, SIGKILL);
		fatal("Failed to get current time: %s", strerror(err));
		/* NOTREACHED */
	}

	expire.tv_sec += PFCD_DAEMONIZE_TIMEOUT;
	ts.tv_sec = PFCD_DAEMONIZE_TIMEOUT;
	ts.tv_nsec = 0;

	/* Wait for signal from the daemon. */
	while (1) {
		pfc_timespec_t	remains;

		sig = sigtimedwait(mask, &sinfo, &ts);
		if (PFC_EXPECT_FALSE(sig == SIGCHLD)) {
			int	code = sinfo.si_code;

			if (code == CLD_EXITED || code == CLD_KILLED ||
			    code == CLD_DUMPED) {
				break;
			}
		}
		else if (PFC_EXPECT_TRUE(sig > 0)) {
			break;
		}

		err = errno;
		PFC_ASSERT(err != EINVAL);
		if (err == EAGAIN) {
			kill(pid, SIGKILL);
			fatal("No response from %s.", pfcd_progname);
			/* NOTREACHED */
		}

		err = pfc_clock_isexpired(&remains, &expire);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* One more try. */
			ts.tv_sec = 0;
			ts.tv_nsec = 1;
		}
		else {
			ts.tv_sec = remains.tv_sec;
			ts.tv_nsec = remains.tv_nsec;
		}
	}

	if (sig == SIGUSR1) {
		/* Daemon seems fine. */
		pfcd_do_exit(PFC_EX_OK);
		/* NOTREACHED */
	}

	/* Check whether another daemon is running. */
	check_pidfile(pf);

	PFC_ASSERT(sig == SIGCHLD);
	PFC_ASSERT(sinfo.si_pid == pid);
	PFC_ASSERT_INT(waitpid(pid, &status, WNOHANG), pid);

	if (WIFEXITED(status)) {
		fatal("%s exited unexpectedly: %d", pfcd_progname,
		      WEXITSTATUS(status));
		/* NOTREACHED */
	}
	if (WIFSIGNALED(status)) {
		fatal("%s was killed by signal: %d", pfcd_progname,
		      WTERMSIG(status));
		/* NOTREACHED */
	}

	kill(pid, SIGKILL);
	fatal("Unexpected SIGCHLD has been received: 0x%x", status);
	/* NOTREACHED */
}

/*
 * static void
 * open_pidfile(pfc_pidf_t *pfp)
 *	Open PID file, and check whether another daemon is running or not.
 */
static void
open_pidfile(pfc_pidf_t *pfp)
{
	int		err;

	/* Open PID file. */
	err = pidfile_open(pfp, pfcd_pid_file_path);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("%s: Failed to open PID file: %s", pfcd_pid_file_path,
		      strerror(err));
		/* NOTREACHED */
	}
	pfcd_pidfile = *pfp;

	/* Check whether another daemon is running. */
	check_pidfile(*pfp);
}

/*
 * static void
 * check_pidfile(pfc_pidf_t pf)
 *	Check whether another process is holding the PID file lock.
 *	If so, quit with error message.
 */
static void
check_pidfile(pfc_pidf_t pf)
{
	pid_t	owner;
	int	err;

	err = pidfile_getowner(pf, &owner);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to check PID file owner: %s", strerror(err));
		/* NOTREACHED */
	}

	if (owner != 0) {
		fatal_status(PFC_EX_BUSY, "Another %s is running. pid = %d",
			     pfcd_progname, owner);
		/* NOTREACHED */
	}
}

/*
 * static void
 * install_pidfile(pfc_pidf_t pf)
 *	Install my process ID to the PID file.
 */
static void
install_pidfile(pfc_pidf_t pf)
{
	pid_t	owner;
	int	err;

	err = pidfile_install(pf, &owner);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (owner != 0) {
			pfc_log_fatal("Another %s is running. pid = %d",
				      pfcd_progname, owner);
		}
		else {
			pfc_log_fatal("Failed to install PID: %s",
				      strerror(err));
		}
		/* NOTREACHED */
	}

	pfcd_has_pidfile = PFC_TRUE;
}

/*
 * static void
 * daemon_foreground_init(void)
 *	Do initialization specific on the foreground mode.
 */
static void
daemon_foreground_init(void)
{
	pfc_pidf_t	pf;

	/* Open PID file. */
	open_pidfile(&pf);

	/* Install my PID. */
	install_pidfile(pf);

	/* Do common initialization. */
	daemon_common_init();
}

/*
 * static void
 * daemon_common_init(void)
 *	Do initialization which should be done irrespective of daemon mode.
 */
static void
daemon_common_init(void)
{
	/* Load per-module logging level in pfcd.conf. */
	pfc_log_modlevel_init(pfcd_options);

	/* Initialize process ID. */
	pfcd_pid = getpid();

	/*
	 * Register clean up handler.
	 *
	 * Remarks:
	 *	PID file will be removed by clean up handler.
	 *	Although PFC daemon may abort before it comes here,
	 *	it's harmless because we can detect PID file's owner correctly
	 *	using file record lock.
	 */
	if (PFC_EXPECT_FALSE(atexit(cleanup) != 0)) {
		pfc_log_fatal("Failed to register clean up handler.");
		/* NOTREACHED */
	}

	/* Change working directory. */
	if (PFC_EXPECT_FALSE(chdir(pfcd_work_dir) == -1)) {
		pfc_log_fatal("Failed to change working directory: %s: %s",
			      pfcd_work_dir, strerror(errno));
		/* NOTREACHED */
	}

	/*
	 * Set error message handler for configuration file parser.
	 * This is required to dump parse error message to the logging system.
	 */
	pfc_conf_set_errfunc(conf_error);

	/* Configure resource limit. */
	resource_init();

	/* Create a control port. */
	ctrl_init();

	/* Initialize PFC events. */
	event_init();

	/* Initialize signals. */
	signal_init();

	/* Invoke libpfc bootstrap code. */
	libpfc_init();
	libpfc_setprogname(pfcd_rprogname);

	/* Initialize child process management. */
	child_init();
}

/*
 * static void
 * cleanup(void)
 *	Clean up handler for PFC daemon.
 */
static void
cleanup(void)
{
	pfc_refptr_t	*rname;
	pfc_bool_t	has_pidfile =
		pfc_atomic_swap_uint8(&pfcd_has_pidfile, PFC_FALSE);

	if (has_pidfile) {
		pfc_log_verbose("Unlink PID file: %s", pfcd_pid_file_path);
		pidfile_unlink(pfcd_pid_file_path);
	}

	ctrl_cleanup();

	/* Shutdown the logging system. */
	pfc_log_fini();

	/* Reset program name. */
	rname = (pfc_refptr_t *)pfc_atomic_swap_ptr((void **)&pfcd_rprogname,
						    NULL);
	if (rname != NULL) {
		pfcd_progname = pfcd_progname_default;
		pfc_refptr_put(rname);
	}
}

/*
 * static void
 * pfcd_boot(void)
 *	Boot up all services in the PFC daemon.
 */
static void
pfcd_boot(void)
{
	int	err;

	pfcd_setstate(PFCD_STATE_INIT);
	pfc_log_verbose("Start %s boot sequence.", pfcd_progname);

	/*
	 * Initialize IPC server library.
	 * This must be done before PFC network library initialization.
	 */
	ipc_init();

	/* Start IPC service. */
	ipc_start();

	/* Load mandatory modules. */
	err = module_boot();
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Module boot failed. Start shutdown: %s",
			      strerror(err));

		PFCD_STATE_WRLOCK();
		if (PFC_EXPECT_TRUE(pfcd_state != PFCD_STATE_FATAL)) {
			pfcd_state = PFCD_STATE_SHUTDOWN;
			pfcd_exit_status = PFC_EX_TEMPFAIL;
		}
		PFCD_STATE_UNLOCK();

		pfcd_shutdown_common();
		/* NOTREACHED */
	}

	/*
	 * Remarks: From here, pfc_log_fatal() will return.
	 */

	/* Raise a PFC service start event. */
	pfcd_setstate(PFCD_STATE_RUNNING);
	err = event_post_global(PFC_EVTYPE_SYS_START);
	if (PFC_EXPECT_FALSE(err != 0 && err != ETIMEDOUT)) {
		pfc_log_fatal("Failed to post system start event.");

		return;
	}

	/* Send service-ready notification via IPC client session. */
	ipc_notify_ready();

	pfc_log_info("%s service has been started.", pfcd_progname);
}

/*
 * static void
 * pfcd_shutdown(void)
 *	Start shutdown sequence.
 */
static void
pfcd_shutdown(void)
{
	pfc_log_info("Start shutdown sequence.");

	PFCD_STATE_WRLOCK();
	if (PFC_EXPECT_TRUE(pfcd_state != PFCD_STATE_FATAL)) {
		pfcd_state = PFCD_STATE_SHUTDOWN;
	}
	PFCD_STATE_UNLOCK();

	/* Raise a PFC service shutdown event. */
	(void)event_post_global(PFC_EVTYPE_SYS_STOP);
	(void)ipc_post_event(PFCD_IPCEVENT_SYS_STOP);

	/* Call common shutdown routine. */
	pfcd_shutdown_common();
	/* NOTREACHED */
}

/*
 * static void
 * pfcd_shutdown_common(void)
 *	Common PFC shutdown routine.
 */
static void
pfcd_shutdown_common(void)
{
	/* Unload all modules. */
	module_shutdown();

	/*
	 * Start libpfc shutdown sequence.
	 * Note that this will terminate below IPC server immediately.
	 */
	libpfc_shutdown_start();

	/* Start shutdown sequence of the IPC subsystem. */
	ipc_shutdown();

	/* Finalize IPC service. */
	ipc_fini();

	/* Finalize child process management. */
	child_fini();

	/* Call libpfc's destructor. */
	libpfc_fini();

	pfcd_exit();
	/* NOTREACHED */
}

/*
 * void
 * prop_add(const char *arg)
 *	Append a system property specified by the command line argument.
 *
 *	`arg' must be a string which contains key and value joined with '='.
 *
 * Calling/Exit State:
 *	fatal() will be called on any error.
 */
void
prop_add(const char *arg)
{
	char	*str = strdup(arg);
	char	*value;
	int	err;

	if (PFC_EXPECT_FALSE(str == NULL)) {
		fatal("Failed to copy property key and value.");
		/* NOTREACHED */
	}

	value = strchr(str, '=');
	if (PFC_EXPECT_FALSE(value == NULL)) {
		fatal("Illegal system property format: %s", arg);
		/* NOTREACHED */
	}

	*value = '\0';
	value++;

	err = pfc_prop_set(str, value);
	free(str);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to set system property: %s: %s", arg,
		      strerror(err));
		/* NOTREACHED */
	}
}

/*
 * static void
 * daemon_attr_init(void)
 *	Initialize static daemon attributes.
 */
static void
daemon_attr_init(void)
{
	pfc_refptr_t	*rpath;
	int		err;

	rpath = pfc_refptr_sprintf(DAEMON_ATTR_DIR "/%s.attr", pfcd_progname);
	if (PFC_EXPECT_FALSE(rpath == NULL)) {
		fatal("Failed to create path to static attributes file.");
		/* NOTREACHED */
	}

	err = pfc_conf_refopen(&attr_conf, rpath, &pfcd_attr_defs);
	pfc_refptr_put(rpath);

	if (PFC_EXPECT_TRUE(err == 0)) {
		attr_block = pfc_conf_get_block(attr_conf, "attributes");
	}
}

/*
 * static void
 * daemon_attr_fini(void)
 *	Finalize static daemon attributes.
 */
static void
daemon_attr_fini(void)
{
	pfc_conf_t	conf;

	conf = (pfc_conf_t)pfc_atomic_swap_ptr((void **)&attr_conf,
					       PFC_CONF_INVALID);
	if (conf != PFC_CONF_INVALID) {
		pfc_conf_close(conf);
		attr_block = PFC_CFBLK_INVALID;
	}
}

/*
 * static const char *
 * daemon_attr_getdesc(void)
 *	Derive a brief description about the daemon from static attributes.
 */
static const char *
daemon_attr_getdesc(void)
{
	return pfc_conf_get_string(attr_block, "description", "PFC daemon");
}

/*
 * static const char *
 * daemon_attr_getident(void)
 *	Derive a syslog identifier from static attributes.
 */
static const char *
daemon_attr_getident(void)
{
	return pfc_conf_get_string(attr_block, "log_ident", pfcd_progname);
}
