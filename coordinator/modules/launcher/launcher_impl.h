/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_MODULE_LAUNCHER_IMPL_H
#define	_UNC_MODULE_LAUNCHER_IMPL_H

/*
 * Internal definitions for launcher module.
 */

#include <launcher_api.h>
#include <clstat_launcher.h>
#include <unc/liblauncher.h>
#include <pfc/module.h>
#include <pfc/atomic.h>
#include <pfc/synch.h>
#include <pfc/rbtree.h>
#include <pfc/refptr.h>
#include <pfc/taskq.h>
#include <pfc/clock.h>
#include <pfc/list.h>
#include <pfc/extcmd.h>
#include <pfc/log.h>
#include <pfc/util.h>
#include <pfc/debug.h>

/*
 * Event source name of launcher event.
 */
#define	LNC_EVENT_SOURCE		PFC_MODULE_THIS_NAME

/*
 * Default values for module parameters.
 */
#define	LNC_CONFDEF_conf_dir		LIBLNC_CONFDIR
#define	LNC_CONFDEF_work_dir		NULL
#define	LNC_CONFDEF_event_timeout	PFC_CONST_U(5000)	/* 5 seconds */
#define	LNC_CONFDEF_clact_timeout	PFC_CONST_U(5000)	/* 5 seconds */
#define	LNC_CONFDEF_startup_timeout	PFC_CONST_U(5000)	/* 5 seconds */

/*
 * Helper macros to fetch module parameter.
 */
#define	LNC_CONF_GET(blk, type, name)				\
	pfc_conf_get_##type((blk), #name, LNC_CONFDEF_##name)

/*
 * Timeout of module finalization.
 */
#define	LNC_FINI_TIMEOUT		PFC_CONST_U(10)	/* 10 seconds */

/*
 * Number of internal event handlers.
 */
#define	LNC_NR_EVENT_HANDLERS		PFC_CONST_U(2)

/*
 * Global launcher context.
 */
typedef struct {
	const char	*lc_confdir;		/* daemon conf directory */
	pfc_refptr_t	*lc_workdir;		/* working directory */
	const char	*lc_channel;		/* IPC channel name */
	pfc_timespec_t	lc_event_timeout;	/* TERMINATED event timeout */
	pfc_timespec_t	lc_clact_timeout;	/* CLSTATE service timeout */
	pfc_timespec_t	lc_startup_timeout;	/* notification timeout */

	/* Realtime clock at the last cluster state transition. */
	pfc_timespec_t	lc_clevent_start;
	pfc_timespec_t	lc_clevent_end;

	pfc_mutex_t	lc_mutex;		/* mutex */
	pfc_cond_t	lc_cond;		/* condition variable */
	clst_ipclist_t	lc_sessions;		/* list of IPC client session */
	lnc_clstate_t	lc_clstate;		/* current cluster node state */
	pfc_taskq_t	lc_taskq;		/* taskq for async jobs */
	uint32_t	lc_ndaemons;		/* number of daemons */
	uint32_t	lc_ncallbacks;		/* number of active callbacks */
	uint32_t	lc_flags;		/* flags */
	uint32_t	lc_nipcs;		/* number of active IPC srv */

	/* Internal event handlers. */
	pfc_evhandler_t	lc_event_handler[LNC_NR_EVENT_HANDLERS];

	/* A string representation of self node address. */
	char		lc_selfaddr[PFC_HOSTADDR_STRSIZE];
} lnc_ctx_t;

#define	LNC_WORKDIR(ctx)	pfc_refptr_string_value((ctx)->lc_workdir)

#define	LNC_LOCK(ctx)		pfc_mutex_lock(&(ctx)->lc_mutex)
#define	LNC_UNLOCK(ctx)		pfc_mutex_unlock(&(ctx)->lc_mutex)

#define	LNC_BROADCAST(ctx)	pfc_cond_broadcast(&(ctx)->lc_cond)
#define	LNC_TIMEDWAIT_ABS(ctx, abstime)					\
	pfc_cond_timedwait_abs(&(ctx)->lc_cond, &(ctx)->lc_mutex, (abstime))

#define	LNC_EVHDLR_START(ctx)	((ctx)->lc_event_handler[0])
#define	LNC_EVHDLR_STOP(ctx)	((ctx)->lc_event_handler[1])

/*
 * Concurrency of task queue.
 */
#define	LNC_TASKQ_NTHREADS	PFC_CONST_U(4)

/*
 * Flags for lc_flags.
 */
#define	LNC_CF_INIT		PFC_CONST_U(0x1)	/* initialized */
#define	LNC_CF_FINI		PFC_CONST_U(0x2)	/* finalized */
#define	LNC_CF_SYSACT		PFC_CONST_U(0x4)	/* cluster system is
							   active */
#define	LNC_CF_STARTED		PFC_CONST_U(0x8)	/* SYS_START event was
							   received */
#define	LNC_CF_STASK_POSTED	PFC_CONST_U(0x10)	/* daemon stop task
							   was posted */
#define	LNC_CF_STASK_DONE	PFC_CONST_U(0x20)	/* daemon stop task
							   completed */

#define	LNC_IS_INIT(ctx)		((ctx)->lc_flags & LNC_CF_INIT)
#define	LNC_IS_FINI(ctx)		((ctx)->lc_flags & LNC_CF_FINI)
#define	LNC_IS_SYSACT(ctx)		((ctx)->lc_flags & LNC_CF_SYSACT)
#define	LNC_IS_STOP_TASK_POSTED(ctx)	((ctx)->lc_flags & LNC_CF_STASK_POSTED)
#define	LNC_IS_STOP_TASK_DONE(ctx)	((ctx)->lc_flags & LNC_CF_STASK_DONE)

#define	LNC_GETSYSACT(ctx)	((LNC_IS_SYSACT(ctx)) ? PFC_TRUE : PFC_FALSE)

#define	LNC_SETFLAG_L(ctx, flag)		\
	do {					\
		(ctx)->lc_flags |= (flag);	\
		LNC_BROADCAST(ctx);		\
	} while (0)

#define	LNC_SETFLAG(ctx, flag)			\
	do {					\
		LNC_LOCK(ctx);			\
		LNC_SETFLAG_L(ctx, flag);	\
		LNC_UNLOCK(ctx);		\
	} while (0)

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * lnc_is_finalized(lnc_ctx_t *ctx)
 *	Return PFC_TRUE only if the launcher module is already finalized.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
lnc_is_finalized(lnc_ctx_t *ctx)
{
	pfc_bool_t	ret;

	LNC_LOCK(ctx);
	ret = (LNC_IS_FINI(ctx)) ? PFC_TRUE : PFC_FALSE;
	LNC_UNLOCK(ctx);

	return ret;
}

struct lnc_proc;
typedef struct lnc_proc		lnc_proc_t;

/*
 * Standard error logging configuration.
 */
typedef struct {
	const char	*lse_fname;		/* log file name */
	int		lse_rotate;		/* file rotation limit */
} lnc_stderr_t;

/*
 * Daemon process instance.
 */
typedef struct {
	pfc_refptr_t	*ld_name;		/* configuration name */
	pfc_timespec_t	ld_start_time;		/* daemon start time */
	lnc_proc_t	*ld_command;		/* daemon command */
	lnc_proc_t	*ld_stop_cmd;		/* command to stop daemon */
	pfc_refptr_t	*ld_logdir;		/* stderr logging directory */
	pfc_refptr_t	*ld_channel;		/* IPC channel name */
	pfc_list_t	ld_typelist;		/* link for list per type */
	pfc_rbnode_t	ld_node;		/* Red-Black tree node */
	pfc_bool_t	ld_uncd;		/* UNC daemon or not */
	pfc_bool_t	ld_start_wait;		/* wait for start up signal */
	lnc_proctype_t	ld_type;		/* process type */
	int		ld_stop_sig;		/* signal to stop daemon */
	uint32_t	ld_start_timeout;	/* start timeout */
	uint32_t	ld_stop_timeout;	/* stop timeout */
} lnc_daemon_t;

#define	LNC_DAEMON_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), lnc_daemon_t, ld_node)
#define	LNC_DAEMON_LIST2PTR(list)				\
	PFC_CAST_CONTAINER((list), lnc_daemon_t, ld_typelist)

#define	LNC_DAEMON_NAME(ldp)	pfc_refptr_string_value((ldp)->ld_name)
#define	LNC_DAEMON_LOGDIR(ldp)	pfc_refptr_string_value((ldp)->ld_logdir)

/*
 * Process instance associated with the daemon.
 */
struct lnc_proc {
	const char	*lp_name;	/* command name */
	lnc_daemon_t	*lp_daemon;	/* daemon instance */
	lnc_stderr_t	*lp_stderr;	/* stderr logging conf */
	pfc_rbnode_t	lp_node;	/* Red-Black tree node */
	pfc_mutex_t	lp_mutex;	/* mutex */
	pfc_cond_t	lp_cond;	/* condition variable */
	uint32_t	lp_flags;	/* flags */
	pid_t		lp_pid;		/* process ID */
	int		lp_status;	/* exit status */

	union {
		pfc_extcmd_t	env;	/* execution environment */
		lnc_cmdmap_t	*conf;	/* command configuration */
	} lp_cmd;
};

#define	LNC_PROC_LOCK(lprp)	pfc_mutex_lock(&(lprp)->lp_mutex)
#define	LNC_PROC_UNLOCK(lprp)	pfc_mutex_unlock(&(lprp)->lp_mutex)

#define	LNC_PROC_BROADCAST(lprp)	pfc_cond_broadcast(&(lprp)->lp_cond)
#define	LNC_PROC_TIMEDWAIT_ABS(lprp, abstime)				\
	pfc_cond_timedwait_abs(&(lprp)->lp_cond, &(lprp)->lp_mutex, (abstime))

#define	LNC_PROC_ENV(lprp)	((lprp)->lp_cmd.env)
#define	LNC_PROC_CONF(lprp)	((lprp)->lp_cmd.conf)

#define	LNC_PROC_KEY(pid)	((pfc_cptr_t)(uintptr_t)(pid))
#define	LNC_PROC_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), lnc_proc_t, lp_node)

#define	LNC_PROC_RESET_STATUS(lprp)		\
	do {					\
		(lprp)->lp_pid = 0;		\
		(lprp)->lp_status = 0;		\
	} while (0)

/*
 * Flags for lp_flags.
 */
#define	LNC_PF_START_WAIT	PFC_CONST_U(0x1)	/* waiting for start */
#define	LNC_PF_FINI		PFC_CONST_U(0x2)	/* finalized */
#define	LNC_PF_CLEVENT		PFC_CONST_U(0x4)	/* sending CLEVENT */
#define	LNC_PF_CLEVOK		PFC_CONST_U(0x8)	/* CLEVENT succeeded */

struct lnc_ordnode;
typedef struct lnc_ordnode	lnc_ordnode_t;

/*
 * Prototype of function which raises a cluster state change event.
 * `type' specifies cluster event type defined as CLSTAT_EVTYPE_XXX.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is
 *	returned.
 */
typedef int	(*lnc_clevfunc_t)(lnc_ordnode_t *UNC_RESTRICT onp,
				  uint8_t type, pfc_bool_t sysact,
				  const pfc_timespec_t *UNC_RESTRICT abstime);

/*
 * Red-Black tree node associated with a daemon, which is indexed by UINT32
 * order value.
 */
struct lnc_ordnode {
	uint32_t	lo_order;		/* order value */
	pfc_refptr_t	*lo_name;		/* name of the daemon */
	lnc_daemon_t	*lo_daemon;		/* daemon instance */
	pfc_rbnode_t	lo_node;		/* Red-Black tree node */
	lnc_clevfunc_t	lo_clevent;		/* cluster event function */
};

#define	LNC_ORDNODE_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), lnc_ordnode_t, lo_node)
#define	LNC_ORDNODE_KEY(order)		((pfc_cptr_t)(uintptr_t)(order))
#define	LNC_ORDNODE_NAME(onp)		pfc_refptr_string_value((onp)->lo_name)

/*
 * List of daemon process information.
 */
struct lnc_dmlist {
	lnc_dminfo_t	*dml_info;		/* daemon information */
	uint32_t	dml_count;		/* number of array elements */
};

/*
 * Internal list of daemons.
 */
typedef struct {
	uint32_t	idml_count;		/* number of daemons */
	pfc_list_t	idml_list;		/* head of lnc_daemon_t list */
} lnc_idmlist_t;

/*
 * Global variables.
 */
extern lnc_ctx_t	launcher_ctx;
extern pfc_rbtree_t	daemon_tree;
extern pfc_rbtree_t	daemon_order_tree[LNC_ORDER_NTYPES];

/*
 * Prototypes.
 */
extern void	lnc_log_fatal(const char *fmt, ...) PFC_FATTR_PRINTFLIKE(1, 2);
extern void	lnc_log_fatal_l(const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(1, 2);
extern int	lnc_wait(lnc_ctx_t *UNC_RESTRICT ctx,
			 const pfc_timespec_t *UNC_RESTRICT abstime);
extern int	lnc_wait_flags(lnc_ctx_t *UNC_RESTRICT ctx,
			       const uint32_t flags,
			       const pfc_timespec_t *UNC_RESTRICT abstime);
extern void	lnc_finalize(lnc_ctx_t *ctx);

extern int		lnc_daemon_boot(void);
extern int		lnc_daemon_init(void);
extern void		lnc_daemon_destroy(void);
extern void		lnc_daemon_start(void *arg);
extern int		lnc_daemon_stop(uint32_t maxorder,
					const pfc_timespec_t *abstime);
extern void		lnc_daemon_kill(void);
extern pfc_bool_t	lnc_daemon_haschild(void);
extern int		lnc_daemon_notify(pid_t pid, const char *channel);
extern int		lnc_daemon_clevent_ack(pid_t pid, pfc_bool_t result);
extern void		lnc_daemon_fini(void);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * lnc_daemon_setstring(pfc_refptr_t *UNC_RESTRICT rchannel,
 *			 char *UNC_RESTRICT buf, size_t bufsize)
 *	Set a string in refptr string object to the given character array.
 *
 * Calling/Exit State:
 *	String length of `rstr' is returned.
 */
static inline size_t PFC_FATTR_ALWAYS_INLINE
lnc_daemon_setstring(pfc_refptr_t *UNC_RESTRICT rstr,
		     char *UNC_RESTRICT buf, size_t bufsize)
{
	size_t	len;

	PFC_ASSERT(bufsize != 0);

	if (rstr == NULL) {
		*buf = '\0';
		len = 0;
	}
	else {
		len = pfc_strlcpy_assert(buf, pfc_refptr_string_value(rstr),
					 bufsize);
	}

	return len;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * lnc_daemon_setdminfo(lnc_dminfo_t *UNC_RESTRICT dip,
 *			const char *UNC_RESTRICT name,
 *			pfc_refptr_t *UNC_RESTRICT rchannel,
 *			lnc_proctype_t type, pid_t pid)
 *	Fill lnc_dminfo_t fields.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
lnc_daemon_setdminfo(lnc_dminfo_t *UNC_RESTRICT dip,
		     const char *UNC_RESTRICT name,
		     pfc_refptr_t *UNC_RESTRICT rchannel, lnc_proctype_t type,
		     pid_t pid)
{
	pfc_strlcpy_assert(dip->dmi_name, name, sizeof(dip->dmi_name));

	/* Set an empty string if no IPC channel is provided. */
	(void)lnc_daemon_setstring(rchannel, dip->dmi_channel,
				   sizeof(dip->dmi_channel));

	dip->dmi_type = type;
	dip->dmi_pid = pid;
}

/*
 * static inline pfc_rbtree_t PFC_FATTR_ALWAYS_INLINE *
 * lnc_daemon_order_gettree(lnc_ordtype_t type, int index)
 *	Determine the Red-Black tree which keeps daemon order.
 *
 *	The target tree is determined by the order type `type' and
 *	type index `index'. `index' is always ignored if `type' is
 *	either LNC_ORDTYPE_START or LNC_ORDTYPE_STOP.
 */
static inline pfc_rbtree_t PFC_FATTR_ALWAYS_INLINE *
lnc_daemon_order_gettree(lnc_ordtype_t type, int index)
{
	uint32_t	idx = (uint32_t)type;

	if (LNC_ORDTYPE_HASINDEX(type)) {
		PFC_ASSERT(index >= 0);
		idx += index;
	}

	PFC_ASSERT(idx < PFC_ARRAY_CAPACITY(daemon_order_tree));

	return &daemon_order_tree[idx];
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * lnc_daemon_stop_low(void)
 *	Stop all daemons which are assigned "stop_order" value less than
 *	LNC_ORDER_UNCD.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
lnc_daemon_stop_low(void)
{
	int	err;

	err = lnc_daemon_stop(LNC_ORDER_UNCD - 1, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to stop low priority daemons: %s",
			      strerror(err));
		/* FALLTHROUGH */
	}
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * lnc_ipc_init(lnc_ctx_t *ctx)
 *	Initialize IPC services provided by the launcher module.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
lnc_ipc_init(lnc_ctx_t *ctx)
{
	ctx->lc_nipcs = 0;

	return 0;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * lnc_clock_remains(pfc_timespec_t *UNC_RESTRICT remains,
 *		     const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Determine difference between the given absolute system time (monotonic)
 *	and the current time.
 *
 *	This function is used to convert absolute timeout to relative timeout.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
lnc_clock_remains(pfc_timespec_t *UNC_RESTRICT remains,
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

extern int	lnc_ipc_fini(lnc_ctx_t *UNC_RESTRICT ctx,
			     const pfc_timespec_t *UNC_RESTRICT abstime);

extern pfc_ipcresp_t	lnc_ipc_service(pfc_ipcsrv_t *srv,
					pfc_ipcid_t service);

extern int	lnc_event_init(lnc_ctx_t *ctx);
extern int	lnc_event_fini(lnc_ctx_t *UNC_RESTRICT ctx,
			       const pfc_timespec_t *UNC_RESTRICT abstime);
extern int	lnc_event_post(pfc_evtype_t type);

extern int	lnc_cluster_boot(lnc_ctx_t *ctx, pfc_cfblk_t options);
extern int	lnc_cluster_event(lnc_ctx_t *UNC_RESTRICT ctx, uint8_t type,
				  pfc_bool_t sysact,
				  const pfc_timespec_t *UNC_RESTRICT abstime);
extern int	lnc_cluster_notify(lnc_ordnode_t *UNC_RESTRICT onp,
				   uint8_t type, pfc_bool_t sysact,
				   const pfc_timespec_t *UNC_RESTRICT abstime);
extern int	lnc_cluster_notify_uncd(lnc_ordnode_t *UNC_RESTRICT onp,
					uint8_t type, pfc_bool_t sysact,
					const pfc_timespec_t *UNC_RESTRICT
					abstime);

#endif	/* !_UNC_MODULE_LAUNCHER_IMPL_H */
