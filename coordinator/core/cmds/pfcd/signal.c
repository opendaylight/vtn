/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * signal.c - Signal management.
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pfc/atomic.h>
#include "pfcd.h"

/*
 * Signals to be reset at bootstrap.
 */
static const int	pfcd_reset_signals[] = {
	SIGCHLD,
	SIGCONT,
};

/*
 * Signals which mean fatal error.
 * Note that SIGKILL and SIGSTOP are reserved by the system.
 */
static const int	pfcd_fatal_signals[] = {
	SIGKILL,
	SIGSTOP,
	SIGILL,
	SIGABRT,
	SIGFPE,
	SIGSEGV,
	SIGBUS,
	SIGSYS,
	SIGTRAP,
	SIGXCPU,
	SIGXFSZ,
};

/*
 * Signals to be ignored.
 */
static const int	pfcd_ign_signals[] = {
	SIGQUIT,
	SIGPIPE,
	SIGALRM,
	SIGTTIN,
	SIGTTOU,
	SIGTSTP,
};

/*
 * Signals to be reset on child process.
 */
static const int	pfcd_cldrst_signals[] = {
	SIGALRM,
};

/*
 * Signals to be ignored on child process.
 */
static const int	pfcd_cldign_signals[] = {
	SIGHUP,
};

/*
 * Shutdown signals.
 */
static const int	pfcd_down_signals[] = {
	SIGINT,
	SIGTERM,
};

/*
 * Signals to be delivered as event.
 */
typedef struct {
	const int		se_signal;		/* signal number */
	volatile uint64_t	se_count;		/* received count */
	uint64_t		se_delivered;		/* delivered signals */
} pfcd_sigevent_t;

#define	SIGEVENT_DECL(sig)	{ (sig), 0, 0 }

static pfcd_sigevent_t	pfcd_event_signals[] = {
	SIGEVENT_DECL(SIGHUP),
	SIGEVENT_DECL(SIGUSR1),
	SIGEVENT_DECL(SIGUSR2),
};

/*
 * Flag which determines whether shutdown signal was received or not.
 */
volatile uint32_t	pfcd_shutdown_received;

/*
 * Unexpected event signal received by the last call of signal_event_handler().
 */
static int	bogus_event_signal = -1;

/*
 * Internal prototype.
 */
static void	signal_shutdown_handler(int sig);
static void	signal_event_handler(int sig);

/*
 * void
 * signal_bootstrap(void)
 *	Bootstrap code of signal management.
 *	This function contains signal initializations which should be done
 *	before creating daemon process.
 */
void
signal_bootstrap(void)
{
	struct sigaction	sact;
	const int	*sp;
	sigset_t	mask;
	int		err;

	/* Unmask all signals. */
	sigemptyset(&mask);
	err = pthread_sigmask(SIG_SETMASK, &mask, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to unmask all set signals: %s", strerror(err));
		/* NOTREACHED */
	}

	/* Reset signals in pfcd_reset_signals array. */
	sact.sa_handler = SIG_DFL;
	sact.sa_flags = 0;
	sigemptyset(&sact.sa_mask);
	for (sp = pfcd_reset_signals; sp < PFC_ARRAY_LIMIT(pfcd_reset_signals);
	     sp++) {
		if (PFC_EXPECT_FALSE(sigaction(*sp, &sact, NULL) != 0)) {
			fatal("Failed to reset signal %d: %s",
			      *sp, strerror(errno));
			/* NOTREACHED */
		}
	}

	/* Reset fatal signals but SIGKILL and SIGSTOP. */
	for (sp = pfcd_fatal_signals; sp < PFC_ARRAY_LIMIT(pfcd_fatal_signals);
	     sp++) {
		const int	sig = *sp;

		if (sig == SIGKILL || sig == SIGSTOP) {
			continue;
		}
		if (PFC_EXPECT_FALSE(sigaction(sig, &sact, NULL) != 0)) {
			fatal("Failed to reset signal %d: %s",
			      sig, strerror(errno));
			/* NOTREACHED */
		}
	}
}

/*
 * void
 * signal_init(void)
 *	Initialize signal configuration for PFC daemon process.
 *	This function configures signal mask which are inherited to
 *	all threads.
 */
void
signal_init(void)
{
	struct sigaction	sact;
	static pfcd_sigevent_t	*sev;
	sigset_t	mask;
	const int	*sp;
	int	err;

	/* By default, all signals are masked on all threads. */
	sigfillset(&mask);

	/* But fatal signals must not be masked. */
	for (sp = pfcd_fatal_signals; sp < PFC_ARRAY_LIMIT(pfcd_fatal_signals);
	     sp++) {
		sigdelset(&mask, *sp);
	}

	/* And unmask ignored signals. */
	for (sp = pfcd_ign_signals; sp < PFC_ARRAY_LIMIT(pfcd_ign_signals);
	     sp++) {
		sigdelset(&mask, *sp);
	}

	/* Apply default signal mask. */
	err = pthread_sigmask(SIG_SETMASK, &mask, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to set signal mask: %s", strerror(err));
		/* NOTREACHED */
	}

	/* Ignore unused signals. */
	sact.sa_handler = SIG_IGN;
	sact.sa_flags = 0;
	sigemptyset(&sact.sa_mask);
	for (sp = pfcd_ign_signals; sp < PFC_ARRAY_LIMIT(pfcd_ign_signals);
	     sp++) {
		if (PFC_EXPECT_FALSE(sigaction(*sp, &sact, NULL) != 0)) {
			pfc_log_fatal("Failed to ignore signal %d: %s",
				      *sp, strerror(errno));
			/* NOTREACHED */
		}
	}

	/* Register shutdown signal handler. */
	sact.sa_handler = signal_shutdown_handler;
	sact.sa_flags = 0;
	sact.sa_mask = mask;
	for (sp = pfcd_down_signals; sp < PFC_ARRAY_LIMIT(pfcd_down_signals);
	     sp++) {
		if (PFC_EXPECT_FALSE(sigaction(*sp, &sact, NULL) != 0)) {
			pfc_log_fatal("Failed to register shutdown signal(%u)"
				      ": %s", *sp, strerror(errno));
			/* NOTREACHED */
		}
	}

	/* Register event signals. */
	sact.sa_handler = signal_event_handler;
	sact.sa_flags = 0;
	sact.sa_mask = mask;
	for (sev = pfcd_event_signals;
	     sev < PFC_ARRAY_LIMIT(pfcd_event_signals); sev++) {
		if (PFC_EXPECT_FALSE(sigaction(sev->se_signal, &sact,
					       NULL) != 0)) {
			pfc_log_fatal("Failed to register event signal(%u): %s",
				      sev->se_signal, strerror(errno));
			/* NOTREACHED */
		}
	}
}

/*
 * pfc_bool_t
 * signal_dispatch(void)
 *	Dispatch received signals to appropriate handlers.
 *
 * Calling/Exit State:
 *	Return PFC_TRUE if shutdown signal was received. Otherwise PFC_FALSE.
 *
 * Remarks:
 *	This function must be called on pfcd's main thread except for
 *	bootstrap.
 */
pfc_bool_t
signal_dispatch(void)
{
	static pfcd_sigevent_t	*sev;
	int	badsig;

	PFC_ASSERT(pfcd_getstate() != PFCD_STATE_RUNNING ||
		   pthread_equal(pthread_self(), pfcd_main_thread));

	/* Log bogus event signal. */
	badsig = pfc_atomic_swap_uint32((uint32_t *)&bogus_event_signal, -1);
	if (PFC_EXPECT_FALSE(badsig != -1)) {
		pfc_log_warn("Unexpected event signal: %d", badsig);
	}

	if (pfcd_shutdown_received) {
		return PFC_TRUE;
	}

	if (PFC_EXPECT_FALSE(pfcd_getstate() != PFCD_STATE_RUNNING)) {
		return PFC_FALSE;
	}

	/*
	 * Deliver received external signals.
	 * We don't need to take care of signal delivery counter overflow
	 * because it is 64 bits long.
	 */
	for (sev = pfcd_event_signals;
	     sev < PFC_ARRAY_LIMIT(pfcd_event_signals); sev++) {
		for (; sev->se_delivered < sev->se_count;
		     sev->se_delivered++) {
			if (!event_post_signal(sev->se_signal)) {
				/* Try next signal. */
				break;
			}
		}
	}

	return PFC_FALSE;
}

/*
 * void
 * signal_delete_shutdown(sigset_t *mask)
 *	Delete shutdown signals from the specified signal mask.
 */
void
signal_delete_shutdown(sigset_t *mask)
{
	const int	*sp;

	for (sp = pfcd_down_signals; sp < PFC_ARRAY_LIMIT(pfcd_down_signals);
	     sp++) {
		sigdelset(mask, *sp);
	}
}

/*
 * void
 * signal_child_init(void)
 *	Initialize signal for child process.
 *	This function is called on a child process created via pfc_extcmd_t.
 */
void
signal_child_init(void)
{
	struct sigaction	sact;
	const int	*sp;
	sigset_t	mask;

	/* Reset signals in pfcd_cldrst_signals array. */
	sact.sa_handler = SIG_DFL;
	sact.sa_flags = 0;
	sigemptyset(&sact.sa_mask);
	for (sp = pfcd_cldrst_signals;
	     sp < PFC_ARRAY_LIMIT(pfcd_cldrst_signals); sp++) {
		PFC_ASSERT_INT(sigaction(*sp, &sact, NULL), 0);
	}

	/* Ignore signals in pfcd_cldign_signals array. */
	sact.sa_handler = SIG_IGN;
	for (sp = pfcd_cldign_signals;
	     sp < PFC_ARRAY_LIMIT(pfcd_cldign_signals); sp++) {
		PFC_ASSERT_INT(sigaction(*sp, &sact, NULL), 0);
	}

	/* Unmask all signals. */
	sigemptyset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);
}

#ifdef	PFC_VERBOSE_DEBUG

/*
 * void
 * signal_mask_assert(void)
 *	Verify signal mask for the calling thread.
 */
void
signal_mask_assert(void)
{
	static pfcd_sigevent_t	*sev;
	sigset_t	mask;
	const int	*sp;

	/* Obtain current signal mask. */
	PFC_ASSERT_INT(pthread_sigmask(SIG_SETMASK, NULL, &mask), 0);

	/* Fatal signals must not be masked. */
	for (sp = pfcd_fatal_signals; sp < PFC_ARRAY_LIMIT(pfcd_fatal_signals);
	     sp++) {
		PFC_ASSERT_INT(sigismember(&mask, *sp), 0);
	}

	/* Ignored signals must not be masked. */
	for (sp = pfcd_ign_signals; sp < PFC_ARRAY_LIMIT(pfcd_ign_signals);
	     sp++) {
		PFC_ASSERT_INT(sigismember(&mask, *sp), 0);
	}

	/* Shutdown signals must be masked. */
	for (sp = pfcd_down_signals; sp < PFC_ARRAY_LIMIT(pfcd_down_signals);
	     sp++) {
		PFC_ASSERT_INT(sigismember(&mask, *sp), 1);
	}

	/* Event signals must be masked. */
	for (sev = pfcd_event_signals;
	     sev < PFC_ARRAY_LIMIT(pfcd_event_signals); sev++) {
		PFC_ASSERT_INT(sigismember(&mask, sev->se_signal), 1);
	}
}

#endif	/* PFC_VERBOSE_DEBUG */

/*
 * static void
 * signal_shutdown_handler(int sig)
 *	Signal handler for shutdown signal.
 */
static void
signal_shutdown_handler(int sig)
{
	uint32_t	*ptr = (uint32_t *)&pfcd_shutdown_received;

	/*
	 * Update shutdown flag.
	 * We should use atomic operation to insert memory fence.
	 */
	(void)pfc_atomic_cas_acq_uint32(ptr, 1, 0);
}

/*
 * static void
 * signal_event_handler(int sig)
 *	Signal handler for event signals.
 */
static void
signal_event_handler(int sig)
{
	static pfcd_sigevent_t	*sev, *endsev;

	endsev = PFC_ARRAY_LIMIT(pfcd_event_signals);
	for (sev = pfcd_event_signals; sev < endsev; sev++) {
		if (sev->se_signal == sig) {
			break;
		}
	}

	if (PFC_EXPECT_FALSE(sev >= endsev)) {
		/* This should not happen. */
		(void)pfc_atomic_swap_uint32((uint32_t *)&bogus_event_signal,
					     sig);

		return;
	}

	sev->se_count++;
}
