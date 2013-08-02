/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * main.c - Main routine of pfc_monitor command.
 */

#include <locale.h>
#include <syslog.h>
#include <pfc/path.h>
#include <cmdopt.h>
#include <cmdutil.h>
#include <conf_impl.h>
#include <pfc/log.h>
#include "pfc_monitor.h"

extern const char	copyright[];

/*
 * Program name.
 */
static const char	progname_default[] = "pfc_monitor";
static const char	*progname = progname_default;

/*
 * The name of the target daemon.
 */
const char		daemon_name_default[] = "pfcd";
const char		*daemon_name = daemon_name_default;
pfc_refptr_t		*daemon_rname;

/*
 * Debugging level.
 */
static int		monitor_debug;

/*
 * Default path to monitor configuration file.
 */
#define	MONITOR_CONF_PATH	PFC_SYSCONFDIR "/pfc_monitor.conf"

/*
 * Default path to PID file.
 */
#define	MONITOR_PID_PATH	PFC_RUNDIR_PATH "/pfc_monitor.pid"

/*
 * Default timeout to stop monitor.
 */
#define	KILL_TIMEOUT		10U		/* 10 seconds */
#define	KILL_TIMEOUT_MIN	1U
#define	KILL_TIMEOUT_MAX	3600U

/*
 * Parameters for log file rotation.
 */
#define	MONITOR_LOG_SIZE	1000000U
#define	MONITOR_LOG_ROTATE	1U

/*
 * Monitor context.
 */
monitor_ctx_t	monitor_ctx;

/*
 * Help message.
 */
#define	HELP_MESSAGE_1							\
	"Daemon monitor for" PFC_LBRK_SPACE_STR
#define	HELP_MESSAGE_2							\
	".\nThis program runs as foreground process until"		\
	PFC_LBRK_SPACE_STR
#define	HELP_MESSAGE_3							\
	PFC_LBRK_SPACE_STR "exits."

/*
 * String literals.
 */
static const char	str_file[] = "FILE";
static const char	str_timeout[] = "TIMEOUT";

/*
 * Command line options.
 */
static const char	*copt_desc[] = {
	"Specify path to configuration file.\n(default: ",
	PFC_PFCD_CONF_PATH,
	")",
	NULL,
};

static const char	*mcopt_desc[] = {
	"Specify path to monitor configuration file.\n(default: ",
	MONITOR_CONF_PATH,
	")",
	NULL,
};

static const char	*popt_desc[] = {
	"Specify path to PID file.\n(default: ",
	MONITOR_PID_PATH,
	")",
	NULL,
};

static const pfc_cmdopt_def_t	option_spec[] = {
	{'C', "conf-file", PFC_CMDOPT_TYPE_STRING,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_ARRAYDESC,
	 (const char *)copt_desc, str_file},
	{'c', "monitor-conf", PFC_CMDOPT_TYPE_STRING,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_ARRAYDESC,
	 (const char *)mcopt_desc, str_file},
	{'P', "pidfile", PFC_CMDOPT_TYPE_STRING,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_ARRAYDESC,
	 (const char*)popt_desc, str_file},
	{'s', "syslog", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Send error messages to the syslog daemon or the system log file "
	 "except early error.", NULL},
	{'k', "kill", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Kill monitor program if it is running, and exit.", NULL},
	{'K', "kill-timeout", PFC_CMDOPT_TYPE_UINT32, PFC_CMDOPT_DEF_ONCE,
	 "How long, in seconds, we should wait for the monitor to stop. "
	 "This option implies \"-k\" option.\n"
	 "(default: 10 seconds)", str_timeout},
	{'d', "debug", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_HIDDEN,
	 "Run with debugging message.", NULL},
	{'\v', "version", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_LONGONLY,
	 "Print version information and exit.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Internal prototypes.
 */
static void	setup(int argc, char **argv) PFC_FATTR_NOINLINE;
static void	fatal(const char *fmt, ...) PFC_FATTR_PRINTFLIKE(1, 2)
	PFC_FATTR_NORETURN;
static void	sysconf_init(monitor_ctx_t *PFC_RESTRICT mp,
			     const char *PFC_RESTRICT path);
static void	syslog_init(void);
static void	fatal_die(void) PFC_FATTR_NORETURN;
static void	dump_version(void);
static void	stop_monitor(const char *pidpath, uint32_t timeout);
static int	kill_monitor(pfc_pidf_t pf, pid_t pid, int sig,
			     uint32_t timeout);

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
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * mcopt_desc_update(const char *conffile)
 *	Set default monitor configuration file path into description about
 *	"-c" option.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
mcopt_desc_update(const char *conffile)
{
	mcopt_desc[1] = conffile;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * popt_desc_update(const char *conffile)
 *	Set default PID file path into description about "-P" option.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
popt_desc_update(const char *pidfile)
{
	popt_desc[1] = pidfile;
}

/*
 * int
 * main(int argc, char **argv)
 *	Start routine of pfc_modcache command.
 */
int
main(int argc, char **argv)
{
	/* Set up monitor process environment. */
	setup(argc, argv);

	/* Initialize monitor context. */
	monitor_init();

	for (;;) {
		/* Send ping to the daemon. */
		monitor_ping();

		/* Wait until the interval time has passed. */
		monitor_sleep();
	}

	/* The program must not reach here. */
	return MON_EX_FATAL;
}

/*
 * void
 * error(const char *fmt, va_list ap)
 *	Record error message.
 */
void
error(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	pfc_log_error_v(fmt, ap);
	va_end(ap);
}

/*
 * void
 * error_die(int status, const char *fmt, va_list ap)
 *	Record error message and die with specified exit status.
 */
void
error_die(int status, const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	verror_die(status, fmt, ap);
	/* NOTREACHED */
}

/*
 * void
 * verror_die(int status, const char *fmt, va_list ap)
 *	Record error message specified by vprintf(3) format and die with
 *	specified exit status.
 */
void
verror_die(int status, const char *fmt, va_list ap)
{
	pfc_log_error_v(fmt, ap);
	exit(status);
	/* NOTREACHED */
}

/*
 * void
 * log_warn(const char *fmt, ...)
 *	Record warning message.
 */
void
log_warn(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	log_vwarn(fmt, ap);
	va_end(ap);
}

/*
 * void
 * log_vwarn(const char *fmt, va_list ap)
 *	Record warning message specified by vprintf(3) format.
 */
void
log_vwarn(const char *fmt, va_list ap)
{
	pfc_log_warn_v(fmt, ap);
}

/*
 * void
 * log_info(const char *fmt, ...)
 *	Record informational message.
 */
void
log_info(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	pfc_log_info_v(fmt, ap);
	va_end(ap);
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
	va_list	ap;

	va_start(ap, fmt);
	debug_vprintf(level, fmt, ap);
	va_end(ap);
}

/*
 * void
 * debug_vprintf(int level, const char *fmt, va_list ap)
 *	Print debugging message specified by vprintf(3) format if current
 *	debugging level is higher than or equal to the specified level.
 */
void
debug_vprintf(int level, const char *fmt, va_list ap)
{
	PFC_ASSERT(level > 0);

	if (level <= monitor_debug) {
		int	i;

		for (i = 0; i < level; i++) {
			putc('*', stderr);
		}
		putc(' ', stderr);
		vfprintf(stderr, fmt, ap);
		putc('\n', stderr);
	}
}

/*
 * static void
 * setup(int argc, char **argv)
 *	Setup monitor environment.
 */
static void
setup(int argc, char **argv)
{
	pfc_cmdopt_t	*parser;
	pfc_refptr_t	*defconf, *dname, *defmconf, *defpfile;
	const char	*conffile;
	const char	*moncf;
	const char	*pidpath;
	monitor_ctx_t	*mp = &monitor_ctx;
	pfc_log_level_t	lvl = PFC_LOGLVL_NONE;
	int		argidx, do_kill = 0;
	uint32_t	ktout = KILL_TIMEOUT;
	char		c;

	/* Use C locale. */
	(void)setlocale(LC_ALL, "C");

	/* Set timezone. */
	tzset();

	if (PFC_EXPECT_TRUE(argc > 0)) {
		/* Initialize program name. */
		dname = progname_init(*argv, &progname);
		if (PFC_EXPECT_FALSE(dname == NULL)) {
			fatal("Failed to create daemon name.");
			/* NOTREACHED */
		}

		daemon_name = pfc_refptr_string_value(dname);
		daemon_rname = dname;
	}
	else {
		dname = NULL;
	}

	/* Construct default file paths. */
	defconf = pfc_refptr_sprintf(PFC_SYSCONFDIR "/%s.conf", daemon_name);
	if (PFC_EXPECT_FALSE(defconf == NULL)) {
		fatal("Failed to create default configuration file path.");
		/* NOTREACHED */
	}

	defmconf = pfc_refptr_sprintf(PFC_SYSCONFDIR "/%s.conf", progname);
	if (PFC_EXPECT_FALSE(defmconf == NULL)) {
		fatal("Failed to create default monitor configuration file "
		      "path.");
		/* NOTREACHED */
	}

	defpfile = pfc_refptr_sprintf(PFC_RUNDIR_PATH "/%s.pid", progname);
	if (PFC_EXPECT_FALSE(defpfile == NULL)) {
		fatal("Failed to create default PID file path.");
		/* NOTREACHED */
	}

	conffile = pfc_refptr_string_value(defconf);
	moncf = pfc_refptr_string_value(defmconf);
	pidpath = pfc_refptr_string_value(defpfile);
	copt_desc_update(conffile);
	mcopt_desc_update(moncf);
	popt_desc_update(pidpath);

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(progname, argc, argv, option_spec, NULL, 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		fatal("Failed to create option parser.");
		/* NOTREACHED */
	}

	mp->mc_syslog = PFC_FALSE;

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case 'C':
			conffile = pfc_cmdopt_arg_string(parser);
			if (PFC_EXPECT_FALSE(*conffile == '\0')) {
				fatal("Configuration file path is empty.");
				/* NOTREACHED */
			}
			break;

		case 'c':
			moncf = pfc_cmdopt_arg_string(parser);
			if (PFC_EXPECT_FALSE(*moncf == '\0')) {
				fatal("Monitor configuration file path is "
				      "empty.");
				/* NOTREACHED */
			}
			break;

		case 'P':
			pidpath = pfc_cmdopt_arg_string(parser);
			if (PFC_EXPECT_FALSE(*pidpath == '\0')) {
				fatal("Monitor PID file path is empty.");
				/* NOTREACHED */
			}
			break;

		case 's':
			mp->mc_syslog = PFC_TRUE;
			break;

		case 'd':
			/* Run as debug mode. */
			monitor_debug++;
			lvl = (lvl == PFC_LOGLVL_NONE)
				? PFC_LOGLVL_DEBUG : PFC_LOGLVL_VERBOSE;
			break;

		case 'K':
			ktout = pfc_cmdopt_arg_uint32(parser);
			if (PFC_EXPECT_FALSE(ktout < KILL_TIMEOUT_MIN)) {
				fatal("kill-timeout must be greater than or "
				      "equal %u.", KILL_TIMEOUT_MIN);
				/* NOTREACHED */
			}
			else if (PFC_EXPECT_FALSE(ktout > KILL_TIMEOUT_MAX)) {
				fatal("kill-timeout must be less than or "
				      "equal %u.", KILL_TIMEOUT_MAX);
				/* NOTREACHED */
			}
			/* FALLTHROUGH */

		case 'k':
			do_kill = 1;
			break;

		case '\v':
			dump_version();
			/* NOTREACHED */

		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stdout);
			exit(MON_EX_OK);
			/* NOTREACHED */

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help_list(parser, stdout, HELP_MESSAGE_1,
					     daemon_name, HELP_MESSAGE_2,
					     daemon_name, HELP_MESSAGE_3,
					     NULL);
			exit(MON_EX_OK);
			/* NOTREACHED */

		case PFC_CMDOPT_ERROR:
			exit(MON_EX_FATAL);
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

	argc -= argidx;
	argv += argidx;
	if (PFC_EXPECT_FALSE(argc != 0)) {
		pfc_cmdopt_usage(parser, stderr);
		exit(MON_EX_FATAL);
		/* NOTREACHED */
	}

	pfc_cmdopt_destroy(parser);

	/* Close all inherited file descriptors. */
	pfc_closefrom(3);

	/*
	 * Initialize the logging system to dump early logs to the standard
	 * error output.
	 */
	pfc_log_init(progname, stderr, lvl, fatal_die);

	if (do_kill) {
		/* Kill the monitor process. */
		stop_monitor(pidpath, ktout);
		exit(MON_EX_OK);
		/* NOTREACHED */
	}

	mp->mc_conf = PFC_CONF_INVALID;
	mp->mc_cfpath = pfc_refptr_string_create(moncf);
	pfc_refptr_put(defmconf);
	if (PFC_EXPECT_FALSE(mp->mc_cfpath == NULL)) {
		fatal("Failed to create monitor configuration file path.");
		/* NOTREACHED */
	}

	mp->mc_mon_pidfile = pfc_refptr_string_create(pidpath);
	pfc_refptr_put(defpfile);
	if (PFC_EXPECT_FALSE(mp->mc_mon_pidfile == NULL)) {
		fatal("Failed to create PID file path.");
		/* NOTREACHED */
	}

	/* Load system configuration file. */
	sysconf_init(mp, conffile);
	pfc_refptr_put(defconf);

	/* Initialize syslog if non-debug mode. */
	if (monitor_debug) {
		mp->mc_syslog = PFC_FALSE;
	}
	else if (mp->mc_syslog) {
		syslog_init();
	}
}

/*
 * static void
 * fatal(const char *fmt, ...)
 *	Print error message for early errors.
 *	This function prints error message to the standard error output and
 *	die.
 */
static void PFC_FATTR_PRINTFLIKE(1, 2) PFC_FATTR_NORETURN
fatal(const char *fmt, ...)
{
	va_list	ap;

	fprintf(stderr, "*** %s ERROR: ", progname);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	putc('\n', stderr);

	exit(MON_EX_FATAL);
	/* NOTREACHED */
}

/*
 * static void
 * sysconf_init(monitor_ctx_t *PFC_RESTRICT mp, const char *PFC_RESTRICT path)
 *	Import system configuration.
 *
 * Remarks:
 *	This function changes the user and group ID of the process if they
 *	are specified in the configuration file.
 */
static void
sysconf_init(monitor_ctx_t *PFC_RESTRICT mp, const char *PFC_RESTRICT path)
{
	pfc_refptr_t	*rconf;
	pfc_cfblk_t	options;
	int		err;

	rconf = pfc_refptr_string_create(path);
	if (PFC_EXPECT_FALSE(rconf == NULL)) {
		fatal("Failed to duplicate configuration file path.");
		/* NOTREACHED */
	}

	/* Load PFC system configuration file. */
	err = pfc_sysconf_init(rconf);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to load PFC system configuration file: %s",
		      strerror(err));
		/* NOTREACHED */
	}
	pfc_refptr_put(rconf);

	options = pfc_sysconf_get_block("options");
	monitor_ctx.mc_workdir = pfc_conf_get_string(options, "work_dir",
						     PFC_PFCD_WORKDIR_PATH);
	monitor_ctx.mc_pidfile = pidfile_default_path();

	/*
	 * Switch user and group to the same as the daemon specified by
	 * the configuration file.
	 * pfccmd_switchuser() never returns on error because it will
	 * call fatal().
	 */
	PFC_ASSERT_INT(pfccmd_switchuser(options, PFC_TRUE, fatal), 0);
	if (monitor_debug > 0) {
		debug_printf(1, "uid=%d, gid=%d", getuid(), getgid());
	}
}

/*
 * static void
 * syslog_init(void)
 *	Initialize syslog.
 */
static void
syslog_init(void)
{
	size_t		dlen = strlen(monitor_ctx.mc_workdir);
	pfc_cfblk_t	options = pfc_sysconf_get_block("options");
	pfc_log_conf_t	logconf;
	pfc_refptr_t	*rident;
	const char	*ident;

	/* Create log identifier. */
	rident = pfc_refptr_sprintf(PRODUCT_ACRONYM ".%s_monitor",
				    daemon_name);
	if (PFC_EXPECT_FALSE(rident == NULL)) {
		fatal("Failed to create log identifier.");
		/* NOTREACHED */
	}

	ident = pfc_refptr_string_value(rident);

	/* Initialize log system. */
	pfc_logconf_init(&logconf, options, ident, fatal_die);
	pfc_refptr_put(rident);
	pfc_logconf_setpath(&logconf, monitor_ctx.mc_workdir, dlen,
			    PFC_PFCD_MONITOR_NAME, PFC_PFCD_MONITOR_NAMELEN,
			    PFC_MONLOG_NAME, PFC_MONLOG_NAMELEN);
	pfc_logconf_setrotate(&logconf, MONITOR_LOG_ROTATE, MONITOR_LOG_SIZE);
	pfc_log_sysinit(&logconf);
}

/*
 * static void
 * fatal_die(void)
 *	Fatal error handler which is called from pfc_log_fatal().
 *	fatal_dir() terminates monitor program immediately.
 */
static void
fatal_die(void)
{
	exit(MON_EX_FATAL);
	/* NOTREACHED */
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

	exit(MON_EX_OK);
	/* NOTREACHED */
}

/*
 * static void
 * stop_monitor(const char *pidpath, uint32_t timeout)
 *	Stop monitor program which is currently running.
 */
static void
stop_monitor(const char *pidpath, uint32_t timeout)
{
	pfc_pidf_t	pf;
	sigset_t	mask;
	pid_t		pid;
	int		err;

	/* Initialize resources for PID file lock and signal mask. */
	sigemptyset(&mask);
	if (PFC_EXPECT_FALSE(pidlock_init(&mask) != 0)) {
		exit(MON_EX_FATAL);
		/* NOTREACHED */
	}

	err = pidfile_open_rdonly(&pf, pidpath);
	if (err == ENOENT) {
		/* Monitor is not running. */
		debug_printf(1, "PID file does not exist: %s", pidpath);

		return;
	}
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("%s: Failed to open PID file: %s", pidpath,
		      strerror(err));
		/* NOTREACHED */
	}

	/* Determine PID of monitor program. */
	err = pidfile_getowner(pf, &pid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("%s: Failed to check PID file owner: %s", pidpath,
		      strerror(err));
		/* NOTREACHED */
	}
	if (pid == 0) {
		debug_printf(1, "No one holds the PID file: %s", pidpath);
		goto out;
	}

	debug_printf(1, "Sending SIGTERM to monitor(%u).", pid);
	err = kill_monitor(pf, pid, SIGTERM, timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		goto out;
	}

	log_warn("Sending SIGKILL to monitor(%u).", pid);
	err = kill_monitor(pf, pid, SIGKILL, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error_die(MON_EX_FATAL,
			  "The " PRODUCT_ACRONYM
			  " daemon monitor is still running: pid = %u", pid);
		/* NOTREACHED */
	}

out:
	pidfile_close(pf);
}

/*
 * static int
 * kill_monitor(pfc_pidf_t pf, pid_t pid, int sig, uint32_t timeout)
 *	Kill a process specified by `pid', by sending a signal specified by
 *	`sig'.
 */
static int
kill_monitor(pfc_pidf_t pf, pid_t pid, int sig, uint32_t timeout)
{
	pidlock_t	plk;
	int		err;

	plk.pl_handle = pf;
	plk.pl_data = NULL;
	plk.pl_callback = NULL;
	sigemptyset(&plk.pl_mask);

	if (PFC_EXPECT_FALSE(kill(pid, sig) != 0)) {
		if ((err = errno) == ESRCH) {
			debug_printf(1, "kill(%u) has returned ESRCH.", pid);

			return 0;
		}

		fatal("Failed to send signal(%d) to the monitor(%u): %s",
		      sig, pid, strerror(err));
		/* NOTREACHED */
	}

	/* Try to acquire read lock for the PID file. */
	err = pidlock_acquire(&plk, timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return 0;
	}
	else if (err == ETIMEDOUT) {
		debug_printf(1, "Timed out.");
	}
	else {
		error("Failed to acquire PID file lock: %s", strerror(err));
	}

	return err;
}
