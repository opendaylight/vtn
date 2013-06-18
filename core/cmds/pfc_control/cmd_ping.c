/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmd_ping.c - "ping" subcommand.
 *
 * "ping" subcommand send a NOP command to the daemon as a ping message.
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <cmdopt.h>
#include "pfc_control.h"

/*
 * Help message.
 */
#define	HELP_MESSAGE_1					\
	"Send a ping message to" PFC_LBRK_SPACE_STR
#define	HELP_MESSAGE_2							\
	PFC_LBRK_SPACE_STR "in order to determine whether the daemon "	\
	"is alive or not.\n"						\
	"Ping timeout can be specified by \"-T\" global option."	\

/*
 * Command line options.
 */
static const pfc_cmdopt_def_t	option_spec[] = {
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Atomic flags to receive SIGALRM.
 */
static volatile sig_atomic_t	alarm_received;

/*
 * Preserved SIGALRM handler.
 */
static struct sigaction		alarm_action;

/*
 * Preserved signal mask.
 */
static sigset_t		sigmask_saved;

/*
 * Signal mask applied during I/O wait.
 */
static sigset_t		*sigmask_iowait;

/*
 * Internal prototypes.
 */
static void		alarm_handler(int sig);
static pfc_bool_t	ping_intr_callback(pfc_ptr_t arg);

/*
 * ctrlcmd_ret_t
 * cmd_ctor_ping(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
 *		 char **PFC_RESTRICT argv)
 *	Constructor of "ping" subcommand.
 */
ctrlcmd_ret_t
cmd_ctor_ping(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
	      char **PFC_RESTRICT argv)
{
	pfc_cmdopt_t	*parser;
	ctrlcmd_ret_t	ret = CMDRET_CONT;
	int		argidx;
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
		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stdout);
			ret = CMDRET_COMPLETE;
			goto out;

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help_list(parser, stdout, HELP_MESSAGE_1,
					     daemon_name, HELP_MESSAGE_2,
					     NULL);
			ret = CMDRET_COMPLETE;
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

out:
	pfc_cmdopt_destroy(parser);

	return ret;
}

/*
 * void
 * cmd_dtor_ping(const ctrlcmd_spec_t *spec)
 *	Destructor of "ping" subcommand.
 */
void
cmd_dtor_ping(const ctrlcmd_spec_t *spec)
{
	struct itimerval	it;
	struct timespec	ts;
	sigset_t	alrm;

	if (sigmask_iowait == NULL) {
		return;
	}

	debug_printf(1, "Cancel alarm timer.");

	/* Cancel alarm timer. */
	CTRL_ITIMER_SET(&it, 0, 0);
	PFC_ASSERT_INT(setitimer(ITIMER_REAL, &it, NULL), 0);

	/* Reap pending SIGALRM. */
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	sigemptyset(&alrm);
	sigaddset(&alrm, SIGALRM);
	(void)sigtimedwait(&alrm, NULL, &ts);
	PFC_ASSERT(sigpending(&alrm) == 0);
	PFC_ASSERT(sigismember(&alrm, SIGALRM) == 0);

	/* Restore signal mask and handler. */
	PFC_ASSERT_INT(sigaction(SIGALRM, &alarm_action, NULL), 0);
	PFC_ASSERT_INT(sigprocmask(SIG_SETMASK, &sigmask_saved, NULL), 0);

	free(sigmask_iowait);
	sigmask_iowait = NULL;
}

/*
 * int
 * cmd_iowait_ping(const ctrlcmd_spec_t *PFC_RESTRICT spec,
 *		   pfc_iowait_t *PFC_RESTRICT iowait)
 *	Define behavior of I/O wait on the control protocol session.
 */
int
cmd_iowait_ping(const ctrlcmd_spec_t *PFC_RESTRICT spec,
		pfc_iowait_t *PFC_RESTRICT iowait)
{
	struct sigaction	sact;
	struct itimerval	it;
	sigset_t	mask;
	int		err;

	sigmask_iowait = (sigset_t *)malloc(sizeof(sigset_t));
	if (PFC_EXPECT_FALSE(sigmask_iowait == NULL)) {
		error("Failed to allocate memory for signal mask.");

		return ENOMEM;
	}

	sigemptyset(sigmask_iowait);
	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);

	/* Install SIGALRM handler. */
	sact.sa_handler = alarm_handler;
	sact.sa_flags = 0;
	sact.sa_mask = mask;
	if (PFC_EXPECT_FALSE(sigaction(SIGALRM, &sact, &alarm_action) != 0)) {
		err = errno;
		free(sigmask_iowait);
		sigmask_iowait = NULL;
		error("Failed to set SIGALRM handler: %s", strerror(err));

		return err;
	}

	/* Mask SIGALRM. */
	PFC_ASSERT_INT(sigprocmask(SIG_BLOCK, &mask, &sigmask_saved), 0);

	/* Program alarm timer. */
	debug_printf(1, "ping timeout = %u", ctrl_timeout);
	CTRL_ITIMER_SET(&it, ctrl_timeout, 0);
	PFC_ASSERT_INT(setitimer(ITIMER_REAL, &it, NULL), 0);

	iowait->iw_intrfunc = ping_intr_callback;
	iowait->iw_intrarg = NULL;
	iowait->iw_sigmask = sigmask_iowait;

	return 0;
}

/*
 * static void
 * alarm_handler(int sig)
 *	SIGALRM handler.
 */
static void
alarm_handler(int sig)
{
	alarm_received = 1;
}

/*
 * static pfc_bool_t
 * ping_intr_callback(pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	EINTR callback for control protocol session.
 */
static pfc_bool_t
ping_intr_callback(pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	pfc_bool_t	ret = PFC_FALSE;

	if (alarm_received) {
		/* Timeout. */
		error("%s did not respond within %u second%s.",
		      daemon_name, ctrl_timeout, ENGLISH_PLUAL(ctrl_timeout));
		ret = PFC_TRUE;
		ctrl_err_status = PFC_EX_TEMPFAIL;
	}

	return ret;
}
