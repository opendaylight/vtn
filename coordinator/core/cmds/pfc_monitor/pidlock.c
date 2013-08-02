/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * pidlock.c - Acquire read lock of the PID file with specifying timeout.
 */

#include "pfc_monitor.h"

/*
 * Atomic flags for SIGALRM.
 */
static volatile sig_atomic_t	alarm_received;

/*
 * Atomic flags which means the main context is blocked in pidlock_acquire().
 */
volatile sig_atomic_t		pidlock_waiting;

/*
 * Internal prototypes.
 */
static pfc_bool_t	pidlock_dummy_callback(pidlock_t *plp, int err);
static void		alarm_handler(int sig);

/*
 * int
 * pidlock_init(sigset_t *mask)
 *	Initialize resources for pidlock_acquire().
 *
 *	`mask' is a signal mask to be set for this process.
 *	Note that SIGALRM is always added to the mask.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pidlock_init(sigset_t *mask)
{
	struct sigaction	sact;
	int	err;

	/* Append SIGALRM to the signal mask. */
	sigaddset(mask, SIGALRM);

	/* Install SIGALRM handler. */
	sact.sa_handler = alarm_handler;
	sact.sa_flags = 0;
	sact.sa_mask = *mask;

	if (PFC_EXPECT_FALSE(sigaction(SIGALRM, &sact, NULL) != 0)) {
		err = errno;
		error("Failed to set SIGALRM handler: %s", strerror(err));

		return err;
	}

	/* Apply signal mask to the process. */
	if (PFC_EXPECT_FALSE(sigprocmask(SIG_SETMASK, mask, NULL) != 0)) {
		err = errno;
		error("Failed to initialize signal mask: %s", strerror(err));

		return err;
	}

	return 0;
}

/*
 * int
 * pidlock_acquire(pidlock_t *plp, uint32_t timeout)
 *	Acquire file record lock for the specified PID file.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must call this function with blocking SIGALRM.
 */
int
pidlock_acquire(pidlock_t *plp, uint32_t timeout)
{
	struct itimerval	it;
	struct timespec	ts;
	int		err;
	pfc_pidf_t	pf = plp->pl_handle;
	sigset_t	*mask = &plp->pl_mask, alrm;
	pfc_bool_t	(*callback)(pidlock_t *, int) = plp->pl_callback;

	if (callback == NULL) {
		callback = pidlock_dummy_callback;
	}

	/* Program alarm clock. */
	alarm_received = 0;
	MONITOR_ITIMER_SET(&it, timeout, 0);
	PFC_ASSERT_INT(setitimer(ITIMER_REAL, &it, NULL), 0);

	/*
	 * Acquire read lock for the PID file.
	 * The target process holds the write lock for the PID file all the
	 * while it is running, so pfc_flock_rdlock() will block the calling
	 * thread unless the target process quits.
	 */
	for (;;) {
		sigset_t	old;

		/* Unmask signals. */
		pidlock_waiting = 1;
		PFC_ASSERT_INT(sigprocmask(SIG_SETMASK, mask, &old), 0);

		/* Try to acquire read lock for the PID file. */
		err = pfc_flock_rdlock((int)pf, NULL);

		/* Restore signal mask. */
		PFC_ASSERT_INT(sigprocmask(SIG_SETMASK, &old, NULL), 0);
		pidlock_waiting = 0;

		/* Call lock wait callback. */
		if ((*callback)(plp, err)) {
			err = ECANCELED;
			break;
		}

		if (err != EINTR) {
			break;
		}
		if (alarm_received) {
			err = ETIMEDOUT;
			break;
		}
	}

	/* Stop alarm. */
	MONITOR_ITIMER_SET(&it, 0, 0);
	PFC_ASSERT_INT(setitimer(ITIMER_REAL, &it, NULL), 0);

	/* Reap pending SIGALRM. */
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	sigemptyset(&alrm);
	sigaddset(&alrm, SIGALRM);
	(void)sigtimedwait(&alrm, NULL, &ts);
	PFC_ASSERT(sigpending(&alrm) == 0);
	PFC_ASSERT(sigismember(&alrm, SIGALRM) == 0);

	return err;
}

/*
 * static pfc_bool_t
 * pidlock_dummy_callback(pidlock_t *plp, int err)
 *	Dummy lock wait callback which always returns PFC_FALSE.
 */
static pfc_bool_t
pidlock_dummy_callback(pidlock_t *plp PFC_ATTR_UNUSED, int err PFC_ATTR_UNUSED)
{
	return PFC_FALSE;
}

/*
 * static void
 * alarm_handler(int sig)
 *	SIGALRM handler.
 */
static void
alarm_handler(int sig)
{
	if (pidlock_waiting) {
		/*
		 * The main context is waiting for SIGALRM in
		 * pfc_flock_rdlock(). pfc_flock_rdlock() can't change signal
		 * mask atomically, so SIGALRM signal may not cause EINTR error.
		 * So we need to program one more short alarm to ensure
		 * pfc_flock_rdlock() is interrupted.
		 */
		short_alarm();
	}
	alarm_received = 1;
}
