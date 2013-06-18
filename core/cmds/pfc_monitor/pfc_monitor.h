/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_MONITOR_PFC_MONITOR_H
#define	_PFC_MONITOR_PFC_MONITOR_H

/*
 * Common definitions for pfc_monitor command.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <pfc/refptr.h>
#include <pfc/debug.h>
#include <pfc/conf.h>
#include <pfc/util.h>
#include <cmdutil.h>
#include <ctrl_proto.h>

/*
 * Acronym of product name.
 */
#define	PRODUCT_ACRONYM		"UNC"

/*
 * Maximum size of error message.
 */
#define	MON_ERRMSG_LEN		256

/*
 * Exit status of pfc_monitor.
 */
#define	MON_EX_OK		0	/* No error */
#define	MON_EX_STOPPED		1	/* The daemon has been stopped. */
#define	MON_EX_STALL		2	/* The daemon did not respond. */
#define	MON_EX_FATAL		3	/* Fatal error */

/*
 * Context for PID file lock.
 */
struct pidlock;
typedef struct pidlock	pidlock_t;

struct pidlock {
	pfc_pidf_t	pl_handle;		/* PID file handle */
	void		*pl_data;		/* private data */
	sigset_t	pl_mask;		/* signal mask for lock wait */

	/*
	 * Callback which will be called in lock loop.
	 * If non-NULL function pointer is set, it will be called just after
	 * pfc_flock_rdlock() is returned. A value returned by
	 * pfc_flock_rdlock() is passed to `err'.
	 *
	 * pidlock_acquire() quits the lock loop if the callback handler
	 * returns PFC_TRUE.
	 */
	pfc_bool_t	(*pl_callback)(pidlock_t *plp, int err);
};

/*
 * Monitor context.
 */
typedef struct {
	uint32_t	mc_interval;		/* ping interval */
	uint32_t	mc_timeout;		/* I/O timeout */
	pfc_refptr_t	*mc_cfpath;		/* path to pfc_monitor.conf */
	pfc_conf_t	mc_conf;		/* configuration file handle */
	const char	*mc_workdir;		/* working directory of pfcd */
	const char	*mc_pidfile;		/* PID file of pfcd */
	pfc_refptr_t	*mc_mon_pidfile;	/* PID file of the monitor */
	pidlock_t	mc_pidlock;		/* PID lock context */
	pid_t		mc_pid;			/* PID of pfcd */
	pfc_bool_t	mc_syslog;		/* use syslog if true */

	/* Error message */
	char		mc_error[MON_ERRMSG_LEN];
} monitor_ctx_t;

extern monitor_ctx_t		monitor_ctx;
extern volatile sig_atomic_t	pidlock_waiting;
extern const char		daemon_name_default[];
extern const char		*daemon_name;
extern pfc_refptr_t		*daemon_rname;

/*
 * Set interval timer.
 * We use one shot timer only, so it_interval is always set to zero.
 */
#define	MONITOR_ITIMER_SET(it, sec, usec)		\
	do {						\
		(it)->it_interval.tv_sec = 0;		\
		(it)->it_interval.tv_usec = 0;		\
		(it)->it_value.tv_sec = (sec);		\
		(it)->it_value.tv_usec = (usec);	\
	} while (0)

/*
 * Prototypes.
 */
extern void	error(const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(1, 2);
extern void	error_die(int status, const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3) PFC_FATTR_NORETURN;
extern void	verror_die(int status, const char *fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(2, 0) PFC_FATTR_NORETURN;
extern void	log_warn(const char *fmt, ...) PFC_FATTR_PRINTFLIKE(1, 2);
extern void	log_vwarn(const char *fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(1, 0);
extern void	log_info(const char *fmt, ...) PFC_FATTR_PRINTFLIKE(1, 2);
extern void	debug_printf(int level, const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3);
extern void	debug_vprintf(int level, const char *fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(2, 0);

extern void	monitor_init(void);
extern void	monitor_ping(void);
extern void	monitor_sleep(void);

extern int	pidlock_init(sigset_t *mask);
extern int	pidlock_acquire(pidlock_t *plp, uint32_t timeout);

/*
 * static inline void
 * short_alarm(void)
 *	Program short term alarm timer.
 *
 *	This function is used to force the main context to get EINTR error.
 */
static inline void
short_alarm(void)
{
	struct itimerval	it;

	/* Program 1 millisecond alarm. */
	MONITOR_ITIMER_SET(&it, 0, 1000);
	PFC_ASSERT_INT(setitimer(ITIMER_REAL, &it, NULL), 0);
}

#endif	/* !_PFC_MONITOR_PFC_MONITOR_H */
