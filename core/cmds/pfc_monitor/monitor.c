/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * monitor.c - Monitor the PFC daemon.
 */

#include <pfc/conf_parser.h>
#include <pfc/log.h>
#include <conf_impl.h>
#include <ctrl_client.h>
#include "pfc_monitor.h"

extern const pfc_cfdef_t	monitor_conf_defs;

/*
 * Default configuration parameters.
 */
#define	DEF_INTERVAL		60U
#define	DEF_TIMEOUT		5U

/*
 * String literals to access monitor configuration.
 */
static const char	mon_cf_block[] = "monitor";
static const char	mon_cf_interval[] = "interval";
static const char	mon_cf_timeout[] = "timeout";

/*
 * Atomic flags for SIGINT/SIGTERM.
 */
static volatile sig_atomic_t	terminated;

/*
 * Atomic flags for SIGUSR1.
 */
static volatile sig_atomic_t	conf_reload;

/*
 * Internal prototypes.
 */
static void	not_running(monitor_ctx_t *mp) PFC_FATTR_NORETURN;
static void	not_respond(monitor_ctx_t *mp) PFC_FATTR_NORETURN;
static void	monitor_pidfile_init(monitor_ctx_t *mp);
static void	monitor_cleanup(void);
static void	monitor_exit(void) PFC_FATTR_NORETURN;
static void	interrupt(int sig);
static void	trigger_reload_conf(int sig);
static void	open_pidfile(monitor_ctx_t *mp);
static void	monitor_error_handler(const char *fmt, va_list ap);
static void	monitor_die(monitor_ctx_t *mp, int status, const char *fmt,
			    ...) PFC_FATTR_PRINTFLIKE(3, 4) PFC_FATTR_NORETURN;

static pfc_bool_t	monitor_lock_callback(pidlock_t *plp, int err);
static pfc_bool_t	monitor_intr_callback(pfc_ptr_t arg);

static int	monitor_conf_load(void);
static void	monitor_conf_reload(void);
static void	monitor_conf_warn(monitor_ctx_t *mp, const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3);

/*
 * Signal handlers.
 */
typedef struct {
	int		ms_signal;			/* signal number */
	const char	*ms_name;			/* symbolic name */
	void		(*ms_handler)(int sig);		/* signal handler */
} monitor_sig_t;

#define	MONITOR_SIG_DECL(sig, handler)	{(sig), #sig, (handler)}

static const monitor_sig_t	monitor_signals[] = {
	MONITOR_SIG_DECL(SIGINT, interrupt),
	MONITOR_SIG_DECL(SIGTERM, interrupt),
	MONITOR_SIG_DECL(SIGUSR1, trigger_reload_conf),
};

/*
 * Determine whether error message is stored in the monitor context.
 */
#define	MONITOR_HAS_ERRMSG(mp)		((mp)->mc_error[0] != '\0')

/*
 * Reset error message in the monitor context.
 */
#define	MONITOR_ERR_RESET(mp)			\
	do {					\
		(mp)->mc_error[0] = '\0';	\
	} while (0)

/*
 * void
 * monitor_init(void)
 *	Initialize monitor context.
 */
void
monitor_init(void)
{
	monitor_ctx_t		*mp = &monitor_ctx;
	const monitor_sig_t	*msp;
	struct sigaction	sact;
	pidlock_t	*plp = &mp->mc_pidlock;
	sigset_t	mask;
	int		err;

	/* Initialize PID file for the monitor process. */
	monitor_pidfile_init(mp);

	/* Clear error message. */
	MONITOR_ERR_RESET(mp);

	/* Ignore SIGPIPE. */
	sact.sa_handler = SIG_IGN;
	sact.sa_flags = 0;
	sigemptyset(&sact.sa_mask);
	if (PFC_EXPECT_FALSE(sigaction(SIGPIPE, &sact, NULL) != 0)) {
		error_die(MON_EX_FATAL, "Failed to ignore SIGPIPE: %s",
			  strerror(errno));
		/* NOTREACHED */
	}

	/* Create signal mask which masks signals used by this program. */
	sigemptyset(&mask);
	for (msp = monitor_signals; msp < PFC_ARRAY_LIMIT(monitor_signals);
	     msp++) {
		sigaddset(&mask, msp->ms_signal);
	}

	/*
	 * Initialize resources for PID file lock, and apply signal mask.
	 * Note that pidlock_init() adds SIGALRM to the mask.
	 */
	if (PFC_EXPECT_FALSE(pidlock_init(&mask) != 0)) {
		exit(MON_EX_FATAL);
		/* NOTREACHED */
	}

	/* Install signal handlers. */
	sact.sa_flags = 0;
	sact.sa_mask = mask;

	for (msp = monitor_signals; msp < PFC_ARRAY_LIMIT(monitor_signals);
	     msp++) {
		sact.sa_handler = msp->ms_handler;
		if (PFC_EXPECT_FALSE(sigaction(msp->ms_signal, &sact,
					       NULL) != 0)) {
			error_die(MON_EX_FATAL, "Failed to set %s handler: %s",
				  msp->ms_name, strerror(errno));
			/* NOTREACHED */
		}
	}

	/* Initialize control protocol client library. */
	err = pfc_ctrl_client_init(mp->mc_workdir, monitor_error_handler);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (MONITOR_HAS_ERRMSG(mp)) {
			monitor_die(mp, MON_EX_FATAL,
				    "Failed to initialize control client: %s",
				    strerror(err));
		}
		/* NOTREACHED */
	}

	/* Open PID file, and check privilege. */
	open_pidfile(mp);

	/*
	 * Initialize pidlock context.
	 * pl_handle must be set by open_pidfile().
	 */
	plp->pl_data = mp;
	plp->pl_callback = monitor_lock_callback;
	sigemptyset(&plp->pl_mask);

	pfc_ctrl_debug(debug_vprintf);

	/* Unmask all signals, but signals used by this program. */
	if (PFC_EXPECT_FALSE(sigprocmask(SIG_SETMASK, &mask, NULL) != 0)) {
		error_die(MON_EX_FATAL, "Failed to initialize signal mask: %s",
			  strerror(errno));
		/* NOTREACHED */
	}

	/* Install error handler to the configuration file parser. */
	pfc_conf_set_errfunc(monitor_error_handler);

	/* Load monitor configuration. */
	(void)monitor_conf_load();

	log_info("The " PRODUCT_ACRONYM " daemon monitor has been started: "
		 "%s=%u, interval=%u, timeout=%u",
		 daemon_name, mp->mc_pid, mp->mc_interval, mp->mc_timeout);
}

/*
 * void
 * monitor_ping(void)
 *	Send ping message to the PFC daemon via control protocol session.
 */
void
monitor_ping(void)
{
	monitor_ctx_t	*mp = &monitor_ctx;
	cproto_sess_t	session;
	pfc_timespec_t	ts, *timeout;
	pfc_iowait_t	iowait;
	sigset_t	mask;
	int		err;

	if (mp->mc_timeout == 0) {
		/* Use default timeout defined in pfcd.conf. */
		timeout = NULL;
	}
	else{
		ts.tv_sec = mp->mc_timeout;
		ts.tv_nsec = 0;
		timeout = &ts;
	}

	debug_printf(1, "Creating control protocol session.");

	/*
	 * Determine I/O wait behavior.
	 * monitor_intr_callback() terminates I/O if SIGINT or SIGTERM is
	 * received.
	 */
	iowait.iw_intrfunc = monitor_intr_callback;
	iowait.iw_intrarg = NULL;
	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);
	iowait.iw_sigmask = &mask;

	/* Create control protocol session. */
	err = pfc_ctrl_client_create(&session, timeout, &iowait);
	if (PFC_EXPECT_FALSE(err == ENOENT)) {
		not_running(mp);
		/* NOTREACHED */
	}
	else if (PFC_EXPECT_FALSE(err == ETIMEDOUT)) {
		not_respond(mp);
		/* NOTRECHED */
	}
	else if (PFC_EXPECT_FALSE(err != 0)) {
		monitor_die(mp, MON_EX_FATAL,
			    "Failed to create control protocol session: %s",
			    strerror(err));
		/* NOTREACHED */
	}

	/* Send NOP command to the daemon. */
	debug_printf(1, "Sending a ping message to the daemon.");
	err = pfc_ctrl_client_execute(&session, CTRL_CMDTYPE_NOP, NULL, NULL);
	if (PFC_EXPECT_FALSE(err < 0)) {
		err = -err;
		if (PFC_EXPECT_FALSE(err == ETIMEDOUT)) {
			not_respond(mp);
			/* NOTREACHED */
		}
		monitor_die(mp, MON_EX_FATAL,
			    "Failed to send ping to the " PRODUCT_ACRONYM
			    " daemon: %s", strerror(err));
		/* NOTREACHED */
	}
	else if (PFC_EXPECT_FALSE(err != 0)) {
		monitor_die(mp, MON_EX_FATAL,
			    "The daemon returns unexpected results.");
		/* NOTREACHED */
	}

	/* Destroy the session. */
	pfc_ctrl_client_destroy(&session);
}

/*
 * void
 * monitor_sleep(void)
 *	Block monitor context until next time to ping.
 *	If it detects the PFC daemon has been exited, the monitor program also
 *	exits.
 */
void
monitor_sleep(void)
{
	monitor_ctx_t	*mp = &monitor_ctx;
	int		err;

	debug_printf(1, "Blocking the monitor context: interval=%u",
		     mp->mc_interval);

	if (conf_reload) {
		/* Reload monitor configuration. */
		monitor_conf_reload();
		conf_reload = 0;
	}

	/* Try to acquire read lock for the PID file. */
	err = pidlock_acquire(&mp->mc_pidlock, mp->mc_interval);
	if (err == 0) {
		/*
		 * We has acquired the read lock successfully.
		 * This means the daemon has been exited.
		 */
		not_running(mp);
		/* NOTREACHED */
	}

	PFC_ASSERT(err == ETIMEDOUT || err == ECANCELED);
}

/*
 * static void
 * not_running(monitor_ctx_t *mp)
 *	Common error handler which is called if the PFC daemon is not running.
 */
static void
not_running(monitor_ctx_t *mp)
{
	pid_t	pid = mp->mc_pid;

	if (pid == 0) {
		error_die(MON_EX_STOPPED, PRODUCT_ACRONYM
			  " daemon is not running.");
		/* NOTREACHED */
	}

	error_die(MON_EX_STOPPED, PRODUCT_ACRONYM
		  " daemon is not running: %s=%u", daemon_name, pid);
	/* NOTREACHED */
}

/*
 * static void
 * not_respond(monitor_ctx_t *mp)
 *	Common error handler which is called if the PFC daemon seems to be
 *	stalled.
 */
static void
not_respond(monitor_ctx_t *mp)
{
	MONITOR_ERR_RESET(mp);
	monitor_die(mp, MON_EX_STALL,
		    PRODUCT_ACRONYM "daemon did not respond: %s=%u",
		    daemon_name, mp->mc_pid);
	/* NOTREACHED */
}

/*
 * static void
 * monitor_pidfile_init(monitor_ctx_t *mp)
 *	Create PID file for the monitor process.
 */
static void
monitor_pidfile_init(monitor_ctx_t *mp)
{
	pfc_pidf_t	mpf;
	const char	*pidfile = pfc_refptr_string_value(mp->mc_mon_pidfile);
	pid_t		mpid;
	int		err;

	/* Open PID file for the monitor process. */
	err = pidfile_open(&mpf, pidfile);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error_die(MON_EX_FATAL, "%s: Failed to open PID file: %s",
			  pidfile, strerror(err));
		/* NOTREACHED */
	}

	/* Install my PID to the monitor PID file. */
	err = pidfile_install(mpf, &mpid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (mpid != 0) {
			error_die(MON_EX_FATAL,
				  "Another %s monitor is runing: pid = %d",
				  daemon_name, mpid);
		}
		else {
			error_die(MON_EX_FATAL,
				  "Failed to install monitor PID: %s",
				  strerror(err));
		}
		/* NOTREACHED */
	}

	/* Install PID cleanup handler. */
	if (PFC_EXPECT_FALSE(atexit(monitor_cleanup) != 0)) {
		unlink(pidfile);
		error_die(MON_EX_FATAL, "Failed to register clean up handler.");
		/* NOTREACHED */
	}
}

/*
 * static void
 * monitor_cleanup(void)
 *	Clean up handler for the PFC daemon monitor.
 */
static void
monitor_cleanup(void)
{
	monitor_ctx_t	*mp = &monitor_ctx;
	pfc_refptr_t	*rstr = mp->mc_mon_pidfile;
	const char	*pidfile = pfc_refptr_string_value(rstr);

	/* Close monitor configuration file. */
	pfc_conf_close(mp->mc_conf);
	pfc_refptr_put(mp->mc_cfpath);

	/* Unlink PID file. */
	unlink(pidfile);
	pfc_refptr_put(rstr);

	/* Shutdown the logging system. */
	pfc_log_fini();

	rstr = daemon_rname;
	if (rstr != NULL) {
		daemon_name = daemon_name_default;
		daemon_rname = NULL;
		pfc_refptr_put(rstr);
	}
}

/*
 * static void
 * monitor_exit(void)
 *	Quit monitor by signal.
 */
static void
monitor_exit(void)
{
	monitor_ctx_t	*mp = &monitor_ctx;

	log_info("The " PRODUCT_ACRONYM
		 " daemon monitor has been terminated: %s=%u",
		 daemon_name, mp->mc_pid);
	exit(MON_EX_OK);
	/* NOTREACHED */
}

/*
 * static void
 * interrupt(int sig)
 *	SIGINT/SIGTERM handler, which terminates the monitor program.
 */
static void
interrupt(int sig)
{
	if (pidlock_waiting) {
		/*
		 * We need to program short term alarm in order to interrupt
		 * pfc_flock_rdlock(). See comments in alarm_handler().
		 */
		short_alarm();
	}
	terminated = 1;
}

/*
 * static void
 * trigger_reload_conf(int sig)
 *	SIGUSR1 handler, which triggers reloading monitor configuration.
 */
static void
trigger_reload_conf(int sig)
{
	if (pidlock_waiting) {
		/*
		 * We need to program short term alarm in order to interrupt
		 * pfc_flock_rdlock(). See comments in alarm_handler().
		 */
		short_alarm();
	}
	conf_reload = 1;
}

/*
 * static void
 * open_pidfile(monitor_ctx_t *mp)
 *	Open PFC daemon's PID file.
 */
static void
open_pidfile(monitor_ctx_t *mp)
{
	pidlock_t	*plp = &mp->mc_pidlock;
	pfc_pidf_t	pf;
	pid_t		pid;
	int		err;

	err = pidfile_open_rdonly(&pf, mp->mc_pidfile);
	if (PFC_EXPECT_FALSE(err == ENOENT)) {
		not_running(mp);
		/* NOTREACHED */
	}
	else if (PFC_EXPECT_FALSE(err != 0)) {
		error_die(MON_EX_FATAL, "%s: open failed: %s",
			  mp->mc_pidfile, strerror(err));
		/* NOTREACHED */
	}
	plp->pl_handle = pf;

	/* Obtain the PFC daemon's PID. */
	err = pidfile_getowner(pf, &pid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error_die(MON_EX_FATAL,
			  "Failed to determine PID for the daemon: %s",
			  strerror(err));
		/* NOTREACHED */
	}
	if (pid == 0) {
		not_running(mp);
		/* NOTREACHED */
	}
	mp->mc_pid = pid;

	/* Ensure that the current user can monitor the PFC daemon. */
	err = pfc_ctrl_check_permission();
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ENOENT) {
			not_running(mp);
		}
		else if (err == EACCES) {
			error_die(MON_EX_FATAL,
				  "You are not allowed to use this program.");
		}
		else {
			error_die(MON_EX_FATAL,
				  "Unable to check permission: %s",
				  strerror(err));
		}
		/* NOTREACHED */
	}
}

/*
 * static void
 * monitor_error_handler(const char *fmt, va_list ap)
 *	Error message handler for control protocol session and configuration
 *	file parser. This function preserves the specified error message
 *	in the monitor context.
 */
static void
monitor_error_handler(const char *fmt, va_list ap)
{
	monitor_ctx_t	*mp = &monitor_ctx;

	vsnprintf(mp->mc_error, sizeof(mp->mc_error), fmt, ap);
}

/*
 * static void
 * monitor_die(monitor_ctx_t *mp, int status, const char *fmt, ...)
 *	If an error message is stored in the monitor context, record it and
 *	die. If not, record specified error message and die.
 */
static void
monitor_die(monitor_ctx_t *mp, int status, const char *fmt, ...)
{
	pidlock_t	*plp = &mp->mc_pidlock;
	pid_t		pid;
	int		err;

	/* Ensure that the daemon is still running. */
	err = pidfile_getowner(plp->pl_handle, &pid);
	if (err == 0 && pid == 0) {
		/* The daemon is not running. */
		not_running(mp);
		/* NOTREACHED */
	}

	if (!MONITOR_HAS_ERRMSG(mp)) {
		va_list	ap;

		va_start(ap, fmt);
		verror_die(status, fmt, ap);
	}
	else {
		error_die(status, "%s", mp->mc_error);
	}
	/* NOTREACHED */
}

/*
 * static pfc_bool_t
 * monitor_lock_callback(pidlock_t *plp, int err)
 *	Callback which will be called from lock wait loop in pidlock_acquire().
 */
static pfc_bool_t
monitor_lock_callback(pidlock_t *plp, int err)
{
	pfc_bool_t	ret = PFC_FALSE;

	/* Quit monitor if the monitor has been interrupted. */
	if (terminated) {
		monitor_exit();
		/* NOTREACHED */
	}

	if (conf_reload) {
		/* Reload monitor configuration. */
		monitor_conf_reload();
		conf_reload = 0;

		/* Cancel lock wait. */
		ret = PFC_TRUE;
	}

	if (err != EINTR) {
		monitor_ctx_t	*mp = (monitor_ctx_t *)plp->pl_data;

		MONITOR_ERR_RESET(mp);
		monitor_die(mp, MON_EX_FATAL,
			    "Failed to acquire PID file lock: %s",
			    strerror(err));
		/* NOTREACHED */
	}

	return ret;
}

/*
 * static pfc_bool_t
 * monitor_intr_callback(pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	EINTR callback for control session.
 */
static pfc_bool_t
monitor_intr_callback(pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	/* Quit monitor if the monitor has been interrupted. */
	if (terminated) {
		monitor_exit();
		/* NOTREACHED */
	}

	return PFC_FALSE;
}

/*
 * static int
 * monitor_conf_load(void)
 *	Load monitor configuration file, and update parameters.
 *	If the file is already loaded, it will be reloaded.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Even this function returns non-zero value, it is not fatal error.
 *	This function guarantees that the context contains valid configuration.
 */
static int
monitor_conf_load(void)
{
	monitor_ctx_t	*mp = &monitor_ctx;
	pfc_conf_t	conf = mp->mc_conf;
	pfc_cfblk_t	cf;
	int		err;

	if (conf == PFC_CONF_INVALID) {
		/* Load monitor configuration file. */
		err = pfc_conf_refopen(&conf, mp->mc_cfpath,
				       &monitor_conf_defs);
		if (PFC_EXPECT_TRUE(err == 0)) {
			mp->mc_conf = conf;
		}
		else {
			if (PFC_EXPECT_FALSE(err != ENOENT)) {
				monitor_conf_warn(mp, "Failed to load monitor "
						  "configuration file.");
			}
			mp->mc_interval = DEF_INTERVAL;
			mp->mc_timeout = DEF_TIMEOUT;

			return err;
		}
	}
	else {
		/* Reload configuration file. */
		err = pfc_conf_reload(conf);
		if (PFC_EXPECT_FALSE(err != 0)) {
			if (PFC_EXPECT_FALSE(err != ENOENT)) {
				monitor_conf_warn(mp,
						  "Failed to reload monitor "
						  "configuration file.");
			}

			return err;
		}
	}

	cf = pfc_conf_get_block(conf, mon_cf_block);
	mp->mc_interval = pfc_conf_get_uint32(cf, mon_cf_interval,
					      DEF_INTERVAL);
	mp->mc_timeout = pfc_conf_get_uint32(cf, mon_cf_timeout, DEF_TIMEOUT);

	return 0;
}

/*
 * static void
 * monitor_conf_reload(void)
 *	Reload monitor configuration file.
 */
static void
monitor_conf_reload(void)
{
	monitor_ctx_t	*mp = &monitor_ctx;

	if (monitor_conf_load() == 0) {
		log_info("Configuration has been reloaded: interval=%u, "
			 "timeout=%u", mp->mc_interval, mp->mc_timeout);
	}
}

/*
 * static void
 * monitor_conf_warn(monitor_ctx_t *mp, const char *fmt, ...)
 *	Record warning message related to the monitor configuration file.
 */
static void
monitor_conf_warn(monitor_ctx_t *mp, const char *fmt, ...)
{
	va_list	ap;

	if (MONITOR_HAS_ERRMSG(mp)) {
		log_warn("%s", mp->mc_error);
	}

	va_start(ap, fmt);
	log_vwarn(fmt, ap);
	va_end(ap);
}
