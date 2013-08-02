/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmd_stop.c - "stop" subcommand.
 *
 * "stop" subcommand stops the PFC daemon using its PID file. Control socket
 * is not used.
 */

#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <cmdopt.h>
#include "pfc_control.h"

/*
 * Help message.
 */
#define	HELP_MESSAGE_1			"Stop" PFC_LBRK_SPACE_STR
#define	HELP_MESSAGE_2				\
	".\nIf the daemon does not stop within the specified timeout, "	\
	"SIGABRT is sent to the daemon. If the daemon is not killed "	\
	"by SIGABRT, SIGKILL is sent to the daemon."

/*
 * Command line options.
 */
static const pfc_cmdopt_def_t	option_spec[] = {
	{'n', "nowait", PFC_CMDOPT_TYPE_NONE, 0,
	 "Don't wait for the daemon to stop.", NULL},
	{'t', "timeout", PFC_CMDOPT_TYPE_UINT32, PFC_CMDOPT_DEF_ONCE ,
	 "How long, in seconds, we should wait for the daemon to stop.\n"
	 "(default: 30 seconds)", str_timeout},
	{'T', "signal-timeout", PFC_CMDOPT_TYPE_UINT32, PFC_CMDOPT_DEF_ONCE ,
	 "How long, in seconds, we should wait for the daemon to stop "
	 "after sending emergency signal.\n(default: 10 seconds)", str_timeout},
	{'q', "quiet", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Don't blame if the daemon is not running.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Maximum, minimum, and default value of daemon wait timeout.
 */
#define	WAIT_TIMEOUT_MAX		86400U
#define	WAIT_TIMEOUT_MIN		1U
#define	WAIT_TIMEOUT_DEFAULT		30U

/*
 * Default value of daemon wait timeout after emergency signal.
 */
#define	WAIT_SIG_TIMEOUT_DEFAULT	10U

/*
 * A flag which determines whether we are waiting for the daemon to stop.
 */
static volatile sig_atomic_t	daemon_waiting;

/*
 * A flag which determines SIGALRM has been raised.
 */
static volatile sig_atomic_t	wait_alarmed;

/*
 * Internal prototypes.
 */
static int	stop_verify_timeout(uint32_t timeout);
static int	stop_daemon(pfc_pidf_t pf, uint32_t timeout,
			    uint32_t sig_timeout);
static int	stop_signal(pfc_pidf_t pf, pid_t pid, uint32_t timeout,
			    int sig);
static int	stop_daemon_wait(pfc_pidf_t pf, uint32_t timeout);
static void	stop_alarm(int sig);
static void	quit_not_running(void) PFC_FATTR_NORETURN;

/*
 * ctrlcmd_ret_t
 * cmd_ctor_stop(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
 *		 char **PFC_RESTRICT argv)
 *	Constructor of "stop" subcommand.
 */
ctrlcmd_ret_t
cmd_ctor_stop(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
	      char **PFC_RESTRICT argv)
{
	pfc_cmdopt_t	*parser;
	ctrlcmd_ret_t	ret = CMDRET_COMPLETE;
	pfc_pidf_t	pf;
	pfc_bool_t	do_wait = PFC_TRUE;
	uint32_t	timeout = WAIT_TIMEOUT_DEFAULT;
	uint32_t	sig_timeout = WAIT_SIG_TIMEOUT_DEFAULT;
	int		argidx, err;
	char		c;

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(CTRLCMD_FULLNAME(spec), argc, argv,
				 option_spec, NULL, 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		error("Failed to create option parser.");

		return CMDRET_FAIL;
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case 'n':
			do_wait = PFC_FALSE;
			break;

		case 't':
			timeout = pfc_cmdopt_arg_uint32(parser);
			err = stop_verify_timeout(timeout);
			if (PFC_EXPECT_FALSE(err != 0)) {
				ret = CMDRET_FAIL;
				goto out;
			}
			break;

		case 'T':
			sig_timeout = pfc_cmdopt_arg_uint32(parser);
			err = stop_verify_timeout(sig_timeout);
			if (PFC_EXPECT_FALSE(err != 0)) {
				ret = CMDRET_FAIL;
				goto out;
			}
			break;

		case 'q':
			not_running_hook = quit_not_running;
			break;

		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stdout);
			goto out;

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help_list(parser, stdout, HELP_MESSAGE_1,
					     daemon_name, HELP_MESSAGE_2,
					     NULL);
			goto out;

		case PFC_CMDOPT_ERROR:
			ret = CMDRET_FAIL;
			goto out;

		default:
			error("Failed to parse command line options.");
			ret = CMDRET_FAIL;
			goto out;
		}
	}

	if ((argidx = pfc_cmdopt_validate(parser)) == -1) {
		error("Invalid command line options.");
		ret = CMDRET_FAIL;
		goto out;
	}

	argc -= argidx;
	argv += argidx;
	if (argc != 0) {
		pfc_cmdopt_usage(parser, stderr);
		ret = CMDRET_FAIL;
		goto out;
	}

	/* Open PID file in read-only mode. */
	err = open_pidfile(&pf);
	if (PFC_EXPECT_FALSE(err != 0)) {
		ret = CMDRET_FAIL;
		goto out;
	}

	/* Stop the daemon. */
	if (do_wait == PFC_FALSE) {
		timeout = 0;
	}
	err = stop_daemon(pf, timeout, sig_timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		ret = CMDRET_FAIL;
	}
	pidfile_close(pf);

out:
	pfc_cmdopt_destroy(parser);

	return ret;
}

/*
 * static int
 * stop_verify_timeout(uint32_t timeout)
 *	Ensure that the specified timeout value is valid.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise EINVAL is returned.
 */
static int
stop_verify_timeout(uint32_t timeout)
{
	int	err = 0;

	if (PFC_EXPECT_FALSE(timeout > WAIT_TIMEOUT_MAX)) {
		error("Timeout value must be less than or equal %u.",
		      WAIT_TIMEOUT_MAX);
		err = EINVAL;
	}
	else if (PFC_EXPECT_FALSE(timeout < WAIT_TIMEOUT_MIN)) {
		error("Timeout value must be greater than or equal %u.",
		      WAIT_TIMEOUT_MIN);
		err = EINVAL;
	}

	return err;
}

/*
 * static int
 * stop_daemon(pfc_pidf_t pf, uint32_t timeout, uint32_t sig_timeout)
 *	Stop the daemon.
 *	If `timeout' is not zero, stop_daemon() waits for the daemon to stop.
 */
static int
stop_daemon(pfc_pidf_t pf, uint32_t timeout, uint32_t sig_timeout)
{
	struct sigaction	sact, oact;
	pid_t	pid;
	int	err;

	/* Determine the process id of the pfc daemon. */
	err = get_daemon_pid(pf, &pid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Install SIGALRM handler. */
	sact.sa_handler = stop_alarm;
	sact.sa_flags = 0;
	sigfillset(&sact.sa_mask);

	if (PFC_EXPECT_FALSE(sigaction(SIGALRM, &sact, &oact) != 0)) {
		err = errno;
		error("Failed to set SIGALRM handler: %s", strerror(err));

		return err;
	}

	/* Send SIGTERM to the daemon. */
	debug_printf(1, "Send SIGTERM to %d.", pid);
	err = stop_signal(pf, pid, timeout, SIGTERM);
	if (err == 0 || err == EPERM || err == ESRCH) {
		goto out;
	}

	/* Send SIGABRT to the daemon and wait. */
	warning("Sending SIGABRT to the daemon.");
	err = stop_signal(pf, pid, sig_timeout, SIGABRT);
	if (err == 0 || err == EPERM || err == ESRCH) {
		goto out;
	}

	/* Send SIGKILL to the daemon and wait. */
	warning("Sending SIGKILL to the daemon.");
	err = stop_signal(pf, pid, sig_timeout, SIGKILL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("%s is still running: pid = %u", daemon_name, pid);
	}

out:
	/* Restore SIGALRM handler. */
	if (PFC_EXPECT_FALSE(sigaction(SIGALRM, &oact, NULL) != 0)) {
		warning("Failed to restore SIGALRM handler: %s",
			strerror(errno));
	}

	return err;
}

/*
 * static int
 * stop_signal(pfc_pidf_t pf, pid_t pid, uint32_t timeout, int sig)
 *	Send signal to the daemon.
 *	If `timeout' is not zero, stop_signal() waits for the daemon to stop.
 */
static int
stop_signal(pfc_pidf_t pf, pid_t pid, uint32_t timeout, int sig)
{
	sigset_t	mask, old;
	int	err;

	/* Send signal to the daemon. */
	if (PFC_EXPECT_FALSE(kill(pid, sig) != 0)) {
		if ((err = errno) == ESRCH &&
		    not_running_hook == quit_not_running) {
			/* The PFC daemon has already exited. */
			return 0;
		}

		if (err == EPERM) {
			not_allowed();
		}
		else {
			error("Failed to send signal(%d) to the daemon: %s",
			      sig, strerror(err));
		}

		return err;
	}

	if (timeout == 0) {
		return 0;
	}

	/* Mask SIGALRM. */
	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);
	PFC_ASSERT_INT(sigprocmask(SIG_SETMASK, &mask, &old), 0);

	/* Wait for the daemon to stop. */
	err = stop_daemon_wait(pf, timeout);

	/* Restore signal mask. */
	PFC_ASSERT_INT(sigprocmask(SIG_SETMASK, &old, NULL), 0);

	return err;
}

/*
 * static int
 * stop_daemon_wait(pfc_pidf_t pf, uint32_t timeout)
 *	Wait for the PFC daemon to stop.
 */
static int
stop_daemon_wait(pfc_pidf_t pf, uint32_t timeout)
{
	struct itimerval	it;
	struct timespec	ts;
	sigset_t	empty;
	int		err;

	debug_printf(1, "Wait for the daemon to stop: timeout = %u", timeout);

	/* Program alarm clock. */
	wait_alarmed = 0;
	CTRL_ITIMER_SET(&it, timeout, 0);
	PFC_ASSERT_INT(setitimer(ITIMER_REAL, &it, NULL), 0);

	sigemptyset(&empty);

	/*
	 * Acquire read lock for the PID file.
	 * The PFC daemon holds the write lock for the PID file all the while
	 * it is running, so pfc_flock_rdlock() will block the calling thread
	 * unless the daemon quits.
	 */
	while (1) {
		sigset_t	old;

		/* Unmask SIGALRM. */
		daemon_waiting = 1;
		PFC_ASSERT_INT(sigprocmask(SIG_SETMASK, &empty, &old), 0);

		/* Try to acquire read lock for the PID file. */
		err = pfc_flock_rdlock((int)pf, NULL);

		/* Restore signal mask. */
		PFC_ASSERT_INT(sigprocmask(SIG_SETMASK, &old, NULL), 0);
		daemon_waiting = 0;

		if (PFC_EXPECT_TRUE(err == 0)) {
			debug_printf(1, "Completed.");
			break;
		}

		if (err != EINTR) {
			error("Failed to acquire PID file lock: %s",
			      strerror(err));
			break;
		}

		if (wait_alarmed) {
			PFC_ASSERT(timeout != 0);
			debug_printf(1, "Timeout.");
			err = ETIMEDOUT;
			break;
		}
	}

	/* Stop alarm. */
	CTRL_ITIMER_SET(&it, 0, 0);
	PFC_ASSERT_INT(setitimer(ITIMER_REAL, &it, NULL), 0);

	/* Reap pending SIGALRM. */
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	sigaddset(&empty, SIGALRM);
	(void)sigtimedwait(&empty, NULL, &ts);
	PFC_ASSERT(sigpending(&empty) == 0);
	PFC_ASSERT(sigismember(&empty, SIGALRM) == 0);

	return err;
}

/*
 * static void
 * stop_alarm(int sig)
 *	SIGALRM handler to detect timeout.
 */
static void
stop_alarm(int sig)
{
	if (daemon_waiting) {
		struct itimerval	it;

		/*
		 * fcntl() can't change signal mask atomically, so SIGALRM
		 * signal may not cause EINTR error. Program one more short
		 * alarm after 1 millisecond to ensure fcntl() is interrupted.
		 */
		CTRL_ITIMER_SET(&it, 0, 1000);
		PFC_ASSERT_INT(setitimer(ITIMER_REAL, &it, NULL), 0);
	}

	wait_alarmed = 1;
}

/*
 * static void
 * quit_not_running(void)
 *	Quit the program immediately.
 *	This function is called if -q option is specified, and this program
 *	detects that the PFC daemon is not running.
 */
static void
quit_not_running(void)
{
	exit(PFC_EX_OK);
	/* NOTREACHED */
}
