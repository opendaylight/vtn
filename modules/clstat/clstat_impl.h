/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_MODULE_CLSTAT_IMPL_H
#define	_UNC_MODULE_CLSTAT_IMPL_H

/*
 * Internal definitions for clstat module.
 */

#include <unc/clstat_types.h>
#include <pfc/module.h>
#include <pfc/clock.h>
#include <pfc/conf.h>
#include <pfc/debug.h>
#include <pfc/synch.h>
#include <pfc/thread.h>
#include <pfc/log.h>
#include "clstat_launcher.h"

/*
 * Default values for module parameters.
 */
#define	CLSTAT_CONF_DEF_init_timeout	PFC_CONST_U(5000)	/* 5 seconds */
#define	CLSTAT_CONF_DEF_ack_timeout	PFC_CONST_U(5000)	/* 5 seconds */

/*
 * Helper macros to fetch module parameter.
 */
#define	CLSTAT_CONF_GET(blk, type, name)				\
	pfc_conf_get_##type((blk), #name, CLSTAT_CONF_DEF_##name)

/*
 * Context to manage IPC event handlers.
 */
typedef struct {
	pfc_ipcevhdlr_t	cic_hdlr_clevent;	/* CLEVENT handler */
	pfc_ipcevhdlr_t	cic_hdlr_chstate;	/* channel state handler */
	pfc_ipcevhdlr_t	cic_hdlr_stop;		/* SYS_STOP handler */
	clst_ipclist_t	cic_sessions;		/* list of client sessions */
	pfc_timespec_t	cic_ack_timeout;	/* CLEVACK service timeout */
	pfc_timespec_t	cic_limit;		/* deadline of CLEVENT */

	/* Realtime clock at the last cluster state transition. */
	pfc_timespec_t	cic_trans_start;
	pfc_timespec_t	cic_trans_end;

	pfc_thread_t	cic_thread;		/* event control thread */
	pfc_thread_t	cic_stopthread;		/* SYS_STOP timer thread */
	uint32_t	cic_flags;		/* flags */
	uint8_t		cic_type;		/* cluster event type */
	pfc_bool_t	cic_sysact;		/* SYSACT flag */
} clst_ipcctx_t;

#define	CLST_IPC_LOCK(ctx)	CLST_IPCLIST_LOCK(&(ctx)->cic_sessions)
#define	CLST_IPC_UNLOCK(ctx)	CLST_IPCLIST_UNLOCK(&(ctx)->cic_sessions)

#define	CLST_IPC_BROADCAST(ctx)	CLST_IPCLIST_BROADCAST(&(ctx)->cic_sessions)
#define	CLST_IPC_TIMEDWAIT_ABS(ctx, abstime)				\
	CLST_IPCLIST_TIMEDWAIT_ABS(&(ctx)->cic_sessions, abstime)

/*
 * Determine whether clst_ipc_fini() is already called or not.
 */
#define	CLST_IPC_IS_FINI(ctx)	((ctx)->cic_sessions.cil_disabled)

/*
 * Flags for cic_flags.
 */
#define	CLIPCF_UP	PFC_CONST_U(0x1)	/* channel is up */
#define	CLIPCF_DOWN	PFC_CONST_U(0x2)	/* channel is down */
#define	CLIPCF_FATAL	PFC_CONST_U(0x4)	/* fatal error */
#define	CLIPCF_STOP	PFC_CONST_U(0x8)	/* channel listener stopped */

#define	CLIPCF_CANCELED		(CLIPCF_DOWN | CLIPCF_FATAL | CLIPCF_STOP)

#define	CLST_IPC_SETFLAG(ctx, flag)			\
	do {						\
		(ctx)->cic_flags |= CLIPCF_##flag;	\
		CLST_IPC_BROADCAST(ctx);		\
	} while (0)

extern pid_t		uncd_pid;
extern clst_ipcctx_t	ipc_ctx;

/*
 * Prototypes.
 */
extern int	clst_event_init(void);
extern int	clst_event_fini(const pfc_timespec_t *abstime);

extern int	clst_ipc_init(void);
extern int	clst_ipc_fini(const pfc_timespec_t *abstime);

/*
 * Determine whether the clstat module is running on the UNC daemon.
 */
#define	CLST_ON_UNCD()		(uncd_pid == 0)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * clst_clock_remains(pfc_timespec_t *UNC_RESTRICT remains,
 *		      const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Determine difference between the given absolute system time (monotonic)
 *	and the current time.
 *
 *	This function is used to convert absolute timeout to relative timeout.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
clst_clock_remains(pfc_timespec_t *UNC_RESTRICT remains,
		   const pfc_timespec_t *UNC_RESTRICT abstime)
{
	int	err;

	err = pfc_clock_isexpired(remains, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* Return 1 millisecond. */
		PFC_ASSERT(err == ETIMEDOUT);
		remains->tv_sec = 0;
		remains->tv_nsec = PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC;
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * clst_ipc_stop_listener(clst_ipcctx_t *ctx)
 *	Stop listening on IPC channel state associated with parent UNC daemon.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
clst_ipc_stop_listener(clst_ipcctx_t *ctx)
{
	CLST_IPC_LOCK(ctx);
	CLST_IPC_SETFLAG(ctx, STOP);
	CLST_IPC_UNLOCK(ctx);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * clst_ipc_mark_trans_start(clst_ipcctx_t *ctx)
 *	Record start time of cluster state transition.
 *
 * Remarks:
 *	This function must be called with holding the IPC event context lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
clst_ipc_mark_trans_start(clst_ipcctx_t *ctx)
{
	pfc_timespec_t	*endtime = &ctx->cic_trans_end;

	PFC_ASSERT_INT(pfc_clock_get_realtime(&ctx->cic_trans_start), 0);

	/* Clear transition end time. */
	endtime->tv_sec = 0;
	endtime->tv_nsec = 0;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * clst_ipc_mark_trans_end(clst_ipcctx_t *ctx)
 *	Record end time of cluster state transition.
 *
 * Remarks:
 *	This function must be called with holding the IPC event context lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
clst_ipc_mark_trans_end(clst_ipcctx_t *ctx)
{
	pfc_timespec_t	*endtime = &ctx->cic_trans_end;

	if (endtime->tv_sec == 0 && endtime->tv_nsec == 0) {
		PFC_ASSERT_INT(pfc_clock_get_realtime(endtime), 0);
	}
}

#endif	/* !_UNC_MODULE_CLSTAT_IMPL_H */
