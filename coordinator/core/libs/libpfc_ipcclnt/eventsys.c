/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * eventsys.c - IPC event delivery system.
 */

#include <signal.h>
#include <pthread.h>
#include <pfc/util.h>
#include <pfc/pfcd_conf.h>
#include "ipcclnt_event.h"

/*
 * Timeout value, in seconds, for the IPC event subsystem finalization.
 */
#define	IPC_EVENT_SHUTDOWN_TIMEOUT	PFC_CONST_U(10)

/*
 * How long, in seconds, pfc_ipcevent_remove_handler() should wait for
 * completion of the call of event handler.
 */
#define	IPC_EVENT_HANDLER_WAIT_TIMEOUT	PFC_CONST_U(5)

/*
 * Default value and valid range of evopt_idle_timeout.
 */
#define	IPC_evopt_idle_timeout_DEF	PFC_CONST_U(1000)
#define	IPC_evopt_idle_timeout_MIN	PFCD_CONF_IPC_EVENT_IDLE_TIMEOUT_MIN
#define	IPC_evopt_idle_timeout_MAX	PFCD_CONF_IPC_EVENT_IDLE_TIMEOUT_MAX

/*
 * Default value and valid range of evopt_maxthreads.
 */
#define	IPC_evopt_maxthreads_DEF	PFC_CONST_U(32)
#define	IPC_evopt_maxthreads_MIN	PFCD_CONF_IPC_EVENT_MAXTHREADS_MIN
#define	IPC_evopt_maxthreads_MAX	PFCD_CONF_IPC_EVENT_MAXTHREADS_MAX

/*
 * Default value and valid range of evopt_conn_interval.
 */
#define	IPC_evopt_conn_interval_DEF	PFC_CONST_U(60000)	/* 1 minute */
#define	IPC_evopt_conn_interval_MIN	PFCD_CONF_IPC_EVENT_CONN_INTERVAL_MIN
#define	IPC_evopt_conn_interval_MAX	PFCD_CONF_IPC_EVENT_CONN_INTERVAL_MAX

/*
 * Default value and valid range of evopt_timeout.
 */
#define	IPC_evopt_timeout_DEF		PFC_CONST_U(10000)	/* 10 seconds */
#define	IPC_evopt_timeout_MIN		PFCD_CONF_IPC_EVENT_TIMEOUT_MIN
#define	IPC_evopt_timeout_MAX		PFCD_CONF_IPC_EVENT_TIMEOUT_MAX

#define	IPC_EVOPT_DEFAULT(name)	IPC_##name##_DEF
#define	IPC_EVOPT_MIN(name)		IPC_##name##_MIN
#define	IPC_EVOPT_MAX(name)		IPC_##name##_MAX
#define	IPC_EVOPT_DECL(name)		.name	= IPC_EVOPT_DEFAULT(name)

#define	IPC_EVOPT_IS_VALID(value, name)				\
	PFC_EXPECT_TRUE((value) >= IPC_EVOPT_MIN(name) &&	\
			(value) <= IPC_EVOPT_MAX(name))

#define	IPC_EVOPT_CHECK(opts, dstopts, type, name, label)		\
	do {								\
		type	__v = (opts)->name;				\
									\
		if (__v == 0) {						\
			/* Use default value. */			\
			__v = IPC_EVOPT_DEFAULT(name);			\
		}							\
		else if (!IPC_EVOPT_IS_VALID(__v, name)) {		\
			/* Invalid value is specified. */		\
			IPCCLNT_LOG_ERROR("Invalid option: %s = 0x%x",	\
					  #name, __v);			\
			goto label;					\
		}							\
									\
		(dstopts)->name = __v;					\
	} while (0)

#ifdef	PFC_VERBOSE_DEBUG

/*
 * Assertion to detect unexpected pthread_exit() call in event handler.
 */

static void
ipc_evdisp_handler_cleanup(void *arg)
{
	ipc_evhdlr_t	*ehp = (ipc_evhdlr_t *)arg;

	fprintf(stderr, "Unexpected exit in event handler: id=%u, name=%s, "
		"func=%p\n", ehp->evh_id, ipc_evhdlr_name(ehp),
		ehp->evh_func);
	abort();
	/* NOTREACHED */
}

#define	IPC_EVDISP_CALL_ASSERT_PUSH(ehp)		\
	pthread_cleanup_push(ipc_evdisp_handler_cleanup, (ehp))
#define	IPC_EVDISP_CALL_ASSERT_POP()	\
	pthread_cleanup_pop(0)

/*
 * Ensure that the given event listener session is in the epoll device.
 *
 * Remarks:
 *	- This macro must be used with holding the event listener session
 *	  lock.
 */
#define	IPC_ELSESS_EPOLL_ASSERT(elsp, ectx)				\
	do {								\
		IPC_EVCTX_LOCK(ectx);					\
		PFC_ASSERT((elsp)->els_flags & IPC_ELSF_EPOLL);		\
		PFC_ASSERT(!pfc_list_is_empty(&(elsp)->els_eplist));	\
		IPC_EVCTX_UNLOCK(ectx);					\
	} while (0)

/*
 * Ensure that the given event listener session is not in the epoll device.
 *
 * Remarks:
 *	- This macro must be used with holding the event listener session
 *	  lock.
 */
#define	IPC_ELSESS_NOT_EPOLL_ASSERT(elsp, ectx)				\
	do {								\
		IPC_EVCTX_LOCK(ectx);					\
		PFC_ASSERT(((elsp)->els_flags & IPC_ELSF_EPOLL) == 0);	\
		PFC_ASSERT(pfc_list_is_empty(&(elsp)->els_eplist));	\
		IPC_EVCTX_UNLOCK(ectx);					\
	} while (0)

/*
 * Ensure that the event listener thread is already gone.
 */
#define	IPC_EVCTX_DONE_ASSERT(ectx)			\
	do {						\
		IPC_EVCTX_LOCK(ectx);			\
		PFC_ASSERT(IPC_EVCTX_IS_DONE(ectx));	\
		IPC_EVCTX_UNLOCK(ectx);			\
	} while (0)

#else	/* !PFC_VERBOSE_DEBUG */
#define	IPC_EVDISP_CALL_ASSERT_PUSH(ehp)	((void)0)
#define	IPC_EVDISP_CALL_ASSERT_POP()		((void)0)
#define	IPC_ELSESS_EPOLL_ASSERT(elsp, ectx)	((void)0)
#define	IPC_ELSESS_NOT_EPOLL_ASSERT(elsp, ectx)	((void)0)
#define	IPC_EVCTX_DONE_ASSERT(ectx)		((void)0)
#endif	/* PFC_VERBOSE_DEBUG */

/*
 * Stack size of internal threads.
 */
#define	IPC_EVENT_DISP_STKSIZE		PFC_CONST_U(0x80000)	/* 512K */
#define	IPC_EVENT_LIS_STKSIZE		PFC_CONST_U(0x10000)	/* 64K */

/*
 * Fallback timeout, in milliseconds, for event listener thread.
 */
#define	IPC_EVCTX_FALLBACK_TIMEOUT	5000		/* 5 seconds */

/*
 * Fallback timeout, in milliseconds, for event task queue thread.
 */
#define	IPC_EVTASKQ_FALLBACK_TIMEOUT	600000		/* 10 minutes */

/*
 * Global state of the IPC event subsystem.
 * This variable must be accessed with holding the client lock.
 */
ipc_evstate_t	ipc_event_state PFC_ATTR_HIDDEN = IPC_EVSTATE_INITIAL;

/*
 * IPC event handler's ID for next allocation.
 */
static pfc_ipcevhdlr_t	ipc_evhdlr_id_next = PFC_IPCEVHDLR_INVALID + 1;

/*
 * Pseudo IPC service name which represents IPC channel state change event.
 */
pfc_refptr_t	*ipc_svname_chstate PFC_ATTR_HIDDEN;

/*
 * IPC event subsystem operations.
 */
static ipc_cevsysops_t	*ipc_evsys_ops = NULL;

/*
 * Determine whether the auto-cancellation is enabled or not.
 */
static pfc_bool_t	ipc_auto_cancel = PFC_FALSE;

/*
 * Internal prototypes.
 */
static void	ipc_event_shutdown(void);

static void		*ipc_evdisp_main(void *arg);
static int		ipc_evdisp_ctor(void);
static void		ipc_evdisp_dtor(void);
static uint32_t		ipc_evdisp_post(ipc_elsess_t *PFC_RESTRICT elsp,
					pfc_ipcevent_t *PFC_RESTRICT event);
static int		ipc_evdisp_post_event(pfc_ipcevent_t *PFC_RESTRICT
					      event,
					      ipc_evhdlr_t *PFC_RESTRICT ehp);
static ipc_clevqent_t	*ipc_evdisp_dequeue(ipc_evdisp_t *disp);
static void		ipc_evdisp_discard(ipc_clevqent_t *ent);
static void		ipc_evdisp_dispatch(ipc_clevqent_t *ent);
static void		ipc_evdisp_log(pfc_ipcevent_t *PFC_RESTRICT event,
				       ipc_evhdlr_t *PFC_RESTRICT ehp,
				       const char *PFC_RESTRICT message);

static void		*ipc_evtaskq_main(void *arg);
static ipc_elsess_t	*ipc_evtaskq_dequeue(ipc_evtaskq_t *PFC_RESTRICT tqp,
					     ctimespec_t *PFC_RESTRICT timeout);
static void		ipc_evtaskq_post(ipc_elsess_t *elsp);
static void		ipc_evtaskq_post_update(ipc_elsess_t *elsp);
static void		ipc_evtaskq_start(ipc_evtaskq_t *tqp);
static void		ipc_evtaskq_post_fail(ipc_etimeout_t *etmp);

static void	*ipc_evctx_main(void *arg);
static int	ipc_evctx_epoll(pfc_ephdlr_t *ephp, uint32_t events,
				void *arg);
static void	ipc_evctx_signal_raise(ipc_evctx_t *ectx);
static int	ipc_evctx_signal_clear(ipc_evctx_t *ectx);
static int	ipc_evctx_timeout_l(ipc_evctx_t *PFC_RESTRICT ectx,
				    ipc_etimeout_t *PFC_RESTRICT etmp,
				    ipc_etmfunc_t func, uint64_t msec);
static void	ipc_evctx_untimeout(ipc_etimeout_t *etmp);
static int	ipc_evctx_run_timeout(ipc_evctx_t *ectx);

static void	ipc_etimeout_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);

static void	ipc_evchan_dtor(ipc_clchan_t *chp, pfc_ptr_t arg);
static void	ipc_evchan_shutdown(ipc_clchan_t *chp, pfc_ptr_t arg);
static void	ipc_evchan_fork_prepare(ipc_clchan_t *chp, pfc_ptr_t arg);
static void	ipc_evchan_fork_parent(ipc_clchan_t *chp, pfc_ptr_t arg);
static void	ipc_evchan_fork_child(ipc_clchan_t *chp, pfc_ptr_t arg);

static int	ipc_chref_create(ipc_clchan_t *PFC_RESTRICT chp,
				 ipc_hostset_t *PFC_RESTRICT hset);
static void	ipc_chref_destroy(ipc_clchan_t *PFC_RESTRICT chp,
				  ipc_hostset_t *PFC_RESTRICT hset);
static void	ipc_chref_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);

static int	ipc_evhdlr_idalloc(ipc_evhdlr_t *ehp);
static void	ipc_evhdlr_add(ipc_clchan_t *PFC_RESTRICT chp,
			       ipc_evhdlr_t *PFC_RESTRICT ehp);
static void	ipc_evhdlr_destroy(ipc_evhdlr_t *ehp);
static int	ipc_evhdlr_wait(ipc_evhdlr_t *ehp);
static void	ipc_evhdlr_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);

static int	ipc_elsess_setup(ipc_clchan_t *PFC_RESTRICT chp,
				 ipc_evhdlr_t *PFC_RESTRICT ehp,
				 const pfc_hostaddr_t *PFC_RESTRICT haddr,
				 ipc_elsess_t **PFC_RESTRICT elspp);
static int	ipc_elsess_link(ipc_elsess_t *PFC_RESTRICT elsp,
				ipc_evhdlr_t *PFC_RESTRICT ehp);
static void	ipc_elsess_unlink(ipc_clchan_t *PFC_RESTRICT chp,
				  ipc_evhdlr_t *PFC_RESTRICT ehp,
				  const pfc_hostaddr_t *PFC_RESTRICT haddr);
static void	ipc_elsess_unlink_impl(ipc_clchan_t *PFC_RESTRICT chp,
				       ipc_elsess_t *PFC_RESTRICT elsp,
				       uint32_t nlinks, uint32_t nwilds);
static void	ipc_elsess_destroy(ipc_elsess_t *elsp);
static void	ipc_elsess_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);
static void	ipc_elsess_setdowncode(ipc_elsess_t *elsp, int err);
static void	ipc_elsess_update(ipc_elsess_t *elsp);
static int	ipc_elsess_connect(ipc_elsess_t *elsp);
static void	ipc_elsess_disconnect(ipc_elsess_t *elsp);
static void	ipc_elsess_post_upevent(ipc_elsess_t *elsp);
static void	ipc_elsess_post_downevent(ipc_elsess_t *elsp,
					  uint32_t downcode);
static int	ipc_elsess_post_notification(ipc_elsess_t *PFC_RESTRICT elsp,
					     ipc_evhdlr_t *PFC_RESTRICT ehp,
					     uint32_t state);
static int	ipc_elsess_addtarget(ipc_elsess_t *elsp);
static int	ipc_elsess_maketarget(ipc_elsess_t *PFC_RESTRICT elsp,
				      ipc_evset_t *PFC_RESTRICT target,
				      pfc_bool_t *PFC_RESTRICT do_reset);
static int	ipc_elsess_removetarget(ipc_elsess_t *PFC_RESTRICT elsp,
					pfc_iostream_t PFC_RESTRICT stream,
					const char *PFC_RESTRICT name,
					size_t namelen, pfc_ipcevtype_t type);
static int	ipc_elsess_fixtarget(ipc_elsess_t *PFC_RESTRICT elsp,
				     ipc_clchan_t *PFC_RESTRICT chp);
static int	ipc_elsess_sendevmask(ipc_elsess_t *PFC_RESTRICT elsp,
				      pfc_iostream_t PFC_RESTRICT stream,
				      const char *PFC_RESTRICT name,
				      size_t namelen,
				      pfc_ipcevmask_t *PFC_RESTRICT mask,
				      ctimespec_t *PFC_RESTRICT abstime);
static int	ipc_elsess_receive(ipc_elsess_t *elsp);
static void	ipc_elsess_shutdown(ipc_elsess_t *elsp,
				    pfc_bool_t do_untimeout);
static void	ipc_elsess_timeout(ipc_elsess_t *elsp, ipc_etmfunc_t func,
				   uint64_t msec);
static void	ipc_elsess_reconnect(ipc_etimeout_t *etmp);
static int	ipc_elsess_epoll(pfc_ephdlr_t *ephp, uint32_t events,
				 void *arg);
static void	ipc_elsess_epoll_delete(ipc_elsess_t *elsp);
static void	ipc_elsess_epoll_disable_input(ipc_elsess_t *elsp);
static int	ipc_elsess_lookup(const char *PFC_RESTRICT channel,
				  const pfc_hostaddr_t *PFC_RESTRICT haddr,
				  ipc_clchan_t **PFC_RESTRICT chpp,
				  ipc_elsess_t **PFC_RESTRICT elspp);
static int	ipc_elsess_getsockerror(ipc_elsess_t *elsp, int sock);

static pfc_cptr_t	ipc_evhdlr_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	ipc_elsess_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	ipc_chref_host_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	ipc_etimeout_getkey(pfc_rbnode_t *node);

/*
 * Hold/Release event handler instance.
 *
 * IPC_EVHDLR_HOLD_L() and IPC_EVHDLR_RELEASE_L() need to be used with holding
 * the event handler lock. Note that IPC_EVHDLR_RELEASE_L() always releases
 * the event handler lock.
 */
#define	IPC_EVHDLR_HOLD_L(ehp)				\
	do {						\
		PFC_ASSERT((ehp)->evh_refcnt > 0);	\
		(ehp)->evh_refcnt++;			\
	} while (0)

#define	IPC_EVHDLR_HOLD(ehp)				\
	do {						\
		IPC_EVHDLR_LOCK(ehp);			\
		IPC_EVHDLR_HOLD_L(ehp);			\
		IPC_EVHDLR_UNLOCK(ehp);			\
	} while (0)

#define	IPC_EVHDLR_RELEASE_L(ehp)					\
	do {								\
		PFC_ASSERT((ehp)->evh_refcnt > 0);			\
		(ehp)->evh_refcnt--;					\
		if ((ehp)->evh_refcnt == 0) {				\
			ipc_evhdlr_destroy(ehp);			\
		}							\
		else {							\
			if (IPC_EVHDLR_IS_REMOVED(ehp) &&		\
			    (ehp)->evh_refcnt == 1) {			\
				/*					\
				 * This is the last reference.		\
				 * So all threads blocked on this handler \
				 * should be woken up.			\
				 */					\
				IPC_EVHDLR_BROADCAST(ehp);		\
			}						\
			IPC_EVHDLR_UNLOCK(ehp);				\
		}							\
	} while (0)

#define	IPC_EVHDLR_RELEASE(ehp)			\
	do {					\
		IPC_EVHDLR_LOCK(ehp);		\
		IPC_EVHDLR_RELEASE_L(ehp);	\
	} while (0)

/*
 * Red-Black tree which keeps all event handlers.
 */
static pfc_rbtree_t	ipc_event_handlers =
	IPC_RBTREE_INITIALIZER(pfc_rbtree_uint32_compare, ipc_evhdlr_getkey);

/*
 * Global event dispatch queue.
 */
static ipc_evdisp_t	ipc_event_dispq = {
	.ed_mutex	= PFC_MUTEX_INITIALIZER,
	.ed_flags	= 0,
};

/*
 * Event session task queue.
 */
static ipc_evtaskq_t	ipc_event_taskq = {
	.etq_mutex	= PFC_MUTEX_INITIALIZER,
	.etq_queue	= PFC_LIST_INITIALIZER(ipc_event_taskq.etq_queue),
};

/*
 * Global event listener context.
 */
static ipc_evctx_t	ipc_event_ctx = {
	.ev_thread	= PFC_PTHREAD_INVALID_ID,
	.ev_mutex	= PFC_MUTEX_INITIALIZER,
	.ev_timeout	= PFC_RBTREE_INITIALIZER(pfc_rbtree_uint64_compare,
						 ipc_etimeout_getkey),
	.ev_epsess	= PFC_LIST_INITIALIZER(ipc_event_ctx.ev_epsess),
	.ev_flags	= 0,
	.ev_epoll	= -1,
	.ev_pipe	= {-1, -1},
};

/*
 * Global event attributes.
 */
static pfc_ipcevopts_t	ipc_event_opts = {
	IPC_EVOPT_DECL(evopt_idle_timeout),
	IPC_EVOPT_DECL(evopt_maxthreads),
	IPC_EVOPT_DECL(evopt_conn_interval),
	IPC_EVOPT_DECL(evopt_timeout),
	.evopt_thread_create	= pfc_ipc_thread_create,
};

/*
 * Hold the event listener session.
 * This macro needs to be used with holding the event listener session lock.
 */
#define	IPC_ELSESS_HOLD(elsp)					\
	do {							\
		PFC_ASSERT((elsp)->els_refcnt > 0);		\
		(elsp)->els_refcnt++;				\
	} while (0)

/*
 * Release the event listener session if `do_release' is true.
 * If `do_release' is false, this macro only releases the session lock.
 *
 * This macro needs to be used with holding the event listener session lock.
 * Note that this macro always releases the event listener session lock.
 */
#define	IPC_ELSESS_RELEASE_COND(elsp, do_release)			\
	do {								\
		PFC_ASSERT((elsp)->els_refcnt > 0);			\
		if ((do_release) && --(elsp)->els_refcnt == 0) {	\
			ipc_elsess_destroy(elsp);			\
		}							\
		else {							\
			IPC_ELSESS_UNLOCK(elsp);			\
		}							\
	} while (0)

/*
 * Release the event listener session unconditionally.
 * This macro needs to be used with holding the event listener session lock.
 * Note that this macro always releases the event listener session lock.
 */
#define	IPC_ELSESS_RELEASE(elsp)		\
	IPC_ELSESS_RELEASE_COND(elsp, PFC_TRUE)

/*
 * Decrement reference counter of the event listener session.
 * This macro never destroys the given session, and never releases the
 * listener session lock.
 */
#define	IPC_ELSESS_RELEASE_RETAIN(elsp)			\
	do {						\
		PFC_ASSERT((elsp)->els_refcnt >= 1);	\
		(elsp)->els_refcnt--;			\
	} while (0)

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * ipc_event_gettime(pfc_timespec_t *tsp)
 *	Get current system time.
 *
 * Calling/Exit State:
 *	Upon successful completion, current time is set to the buffer pointed
 *	by `tsp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
ipc_event_gettime(pfc_timespec_t *tsp)
{
	int		err;

	err = pfc_clock_gettime(tsp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCCLNT_LOG_ERROR("Failed to get current time: %s",
				  strerror(err));
	}

	return err;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * ipc_event_gettimeout_msec(pfc_timespec_t *abstime, uint32_t msec)
 *	Determine absolute I/O timeout from the specified milliseconds.
 *
 * Calling/Exit State:
 *	Upon successful completion, an upper limit of I/O time from now is set
 *	to the buffer pointed by `abstime', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
ipc_event_gettimeout_msec(pfc_timespec_t *abstime, uint32_t msec)
{
	pfc_timespec_t	timeout;
	int		err;

	pfc_clock_msec2time(&timeout, msec);
	err = pfc_clock_abstime(abstime, &timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCCLNT_LOG_ERROR("Failed to determine I/O timeout: %s",
				  strerror(err));
	}

	return err;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * ipc_event_gettimeout(pfc_timespec_t *abstime)
 *	Determine I/O timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, an upper limit of I/O time from now is set
 *	to the buffer pointed by `abstime', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
ipc_event_gettimeout(pfc_timespec_t *abstime)
{
	uint32_t	msec = ipc_event_opts.evopt_timeout;

	return ipc_event_gettimeout_msec(abstime, msec);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * ipc_evctx_gettime(uint64_t *tp)
 *	Get current system time in milliseconds.
 *
 * Calling/Exit State:
 *	Upon successful completion, current time in milliseconds is set to
 *	the buffer pointed by `tp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
ipc_evctx_gettime(uint64_t *tp)
{
	pfc_timespec_t	ts;
	int		err;

	err = ipc_event_gettime(&ts);
	if (PFC_EXPECT_TRUE(err == 0)) {
		*tp = pfc_clock_time2msec(&ts);
	}

	return err;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * ipc_evctx_timeout(ipc_etimeout_t *etmp, ipc_etmfunc_t func, uint64_t msec)
 *	Register timeout handler to the event listener context.
 *	The timeout handler will be invoked after `msec' milliseconds.
 *
 *	`func' is a pointer to timeout handler. If NULL is specified,
 *	the handler previously set in `etmp' is used.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
ipc_evctx_timeout(ipc_etimeout_t *etmp, ipc_etmfunc_t func, uint64_t msec)
{
	ipc_evctx_t	*ectx = &ipc_event_ctx;
	int		err;

	IPC_EVCTX_LOCK(ectx);
	err = ipc_evctx_timeout_l(ectx, etmp, func, msec);
	IPC_EVCTX_UNLOCK(ectx);

	return err;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipc_event_close(int *fdp)
 *	Close the file descriptor in the specified buffer, and initialize it
 *	with -1.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipc_event_close(int *fdp)
{
	int	fd = *fdp;

	if (fd != -1) {
		PFC_ASSERT_INT(close(fd), 0);
		*fdp = -1;
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipc_elsess_cancel_clients(ipc_elsess_t *elsp)
 *	Cancel ongoing IPC invocation requests on client sessions which
 *	connects to the same IPC server as the specified listener session.
 *
 * Remarks:
 *	The caller must call this function without holding any lock because
 *	it may acquire the global client lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipc_elsess_cancel_clients(ipc_elsess_t *elsp)
{
	/* Nothing to do if the IPC client library is already disabled. */
	if (!ipc_disabled) {
		ipc_clnotify_t	clnotify;

		IPC_CLNOTIFY_INIT(&clnotify, elsp, PFC_FALSE);

		IPC_CLIENT_RDLOCK();
		pfc_ipcclnt_conn_iterate(pfc_ipcclnt_canceller_conn_notify,
					 &clnotify);
		IPC_CLIENT_UNLOCK();
	}
}

/*
 * int
 * pfc_ipcclnt_event_init(const pfc_ipcevopts_t *opts)
 *	Initialize the IPC event subsystem.
 *
 *	If you want to register IPC event listener, this function must be
 *	called before registration of IPC event listener.
 *
 *	`opts' is a pointer to pfc_ipcevopts_t which determines behavior
 *	of the IPC event subsystem. Specifying NULL means that IPC event uses
 *	default	value.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_event_init(const pfc_ipcevopts_t *opts)
{
	ipc_evctx_t	*ectx = &ipc_event_ctx;
	ipc_evdisp_t	*disp = &ipc_event_dispq;
	ipc_evtaskq_t	*tqp = &ipc_event_taskq;
	ipc_evstate_t	state;
	pfc_ipcevopts_t	*dstopts = &ipc_event_opts;
	pfc_refptr_t	*rstr = NULL;
	pthread_t	thr;
	pthread_attr_t	tattr;
	sigset_t	sset, saved, *setp = NULL;
	size_t		sz;
	int		err;

	IPC_CLIENT_WRLOCK();

	state = ipc_event_state;
	if (PFC_EXPECT_FALSE(state != IPC_EVSTATE_INITIAL)) {
		err = pfc_ipcclnt_event_state_error(state);
		goto error;
	}

	/* Check specified options. */
	if (opts != NULL) {
		err = EINVAL;
		IPC_EVOPT_CHECK(opts, dstopts, uint32_t, evopt_idle_timeout,
				error);
		IPC_EVOPT_CHECK(opts, dstopts, uint32_t, evopt_maxthreads,
				error);
		IPC_EVOPT_CHECK(opts, dstopts, uint32_t, evopt_conn_interval,
				error);
		IPC_EVOPT_CHECK(opts, dstopts, uint32_t, evopt_timeout,
				error);
		if (opts->evopt_thread_create != NULL) {
			dstopts->evopt_thread_create =
				opts->evopt_thread_create;
		}
	}

	/* Initialize static local host set. */
	err = pfc_ipcclnt_hostset_init();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/*
	 * Create refptr string which represents IPC channel state change
	 * event.
	 */
	if (PFC_EXPECT_TRUE(ipc_svname_chstate == NULL)) {
		rstr = pfc_refptr_string_create(ipc_str_svname_chstate);
		if (PFC_EXPECT_FALSE(rstr == NULL)) {
			IPCCLNT_LOG_ERROR("Failed to create channel state "
					  "event name.");
			goto error;
		}
		ipc_svname_chstate = rstr;
	}

	/* Initialize event dispatch queue. */
	err = pfc_cond_init(&disp->ed_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error;
	}

	disp->ed_flags = 0;
	disp->ed_queue = NULL;
	disp->ed_qnext = &disp->ed_queue;

	/* Initialize event session task queue. */
	err = pfc_cond_init(&tqp->etq_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error;
	}

	tqp->etq_maxthreads = dstopts->evopt_maxthreads;
	tqp->etq_nthreads = 0;
	tqp->etq_nwaiting = 0;
	tqp->etq_flags = 0;
	IPC_ETIMEOUT_INIT(&tqp->etq_timeout, ipc_evtaskq_post_fail, PFC_FALSE);

	/* Allocate epoll event buffer. */
	sz = sizeof(epoll_event_t) * dstopts->evopt_maxthreads;
	ectx->ev_epevents = (epoll_event_t *)malloc(sz);
	if (PFC_EXPECT_FALSE(ectx->ev_epevents == NULL)) {
		IPCCLNT_LOG_ERROR("Failed to allocate epoll event buffer.");
		err = ENOMEM;
		goto error;
	}

	/* Create epoll instance. */
	ectx->ev_epoll = pfc_epoll_create();
	if (PFC_EXPECT_FALSE(ectx->ev_epoll == -1)) {
		err = errno;
		IPCCLNT_LOG_ERROR("Failed to create epoll instance: %s",
				  strerror(err));
		goto error_epevents;
	}

	/* Create control pipe. */
	err = pfc_pipe_open(ectx->ev_pipe, PFC_PIPE_CLOEXEC_NB);
	if (PFC_EXPECT_FALSE(err != 0)) {
		err = errno;
		IPCCLNT_LOG_ERROR("Failed to create pipe: %s", strerror(err));
		goto error_epoll;
	}

	/* Register control pipe to the epoll instance. */
	pfc_epoll_handler_init(&ectx->ev_ephdlr, ipc_evctx_epoll);
	err = pfc_epoll_ctl(ectx->ev_epoll, EPOLL_CTL_ADD,
			    IPC_EVCTX_RDPIPE(ectx), EPOLLIN, &ectx->ev_ephdlr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		err = errno;
		IPCCLNT_LOG_ERROR("Failed to register control pipe to the "
				  "epoll instance: %s", strerror(err));
		goto error_pipe;
	}

	/* Mask all signals. */
	sigfillset(&sset);
	PFC_ASSERT_INT(pthread_sigmask(SIG_BLOCK, &sset, &saved), 0);
	setp = &saved;

	/* Create event dispatch thread. */
	PFC_ASSERT_INT(pthread_attr_init(&tattr), 0);
	PFC_ASSERT_INT(pthread_attr_setdetachstate
		       (&tattr, PTHREAD_CREATE_DETACHED), 0);
	PFC_ASSERT_INT(pthread_attr_setstacksize(&tattr,
						 IPC_EVENT_DISP_STKSIZE), 0);
	err = pthread_create(&thr, &tattr, ipc_evdisp_main, disp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		PFC_ASSERT_INT(pthread_attr_destroy(&tattr), 0);
		IPCCLNT_LOG_ERROR("Failed to create event dispatch thread: %s",
				  strerror(err));
		goto error_pipe;
	}

	/*
	 * Wait for completion of initialization of the event dispatch thread.
	 * Although we block the calling thread with holding the client lock,
	 * It's harmless because it should never costs too much time.
	 */
	IPC_EVDISP_LOCK(disp);
	while (!IPC_EVDISP_IS_INITIALIZED(disp)) {
		IPC_EVDISP_WAIT(disp);
	}
	IPC_EVDISP_UNLOCK(disp);

	err = disp->ed_ctorerr;
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_dispatch;
	}

	/* Create event listener thread. */
	PFC_ASSERT_INT(pthread_attr_setdetachstate
		       (&tattr, PTHREAD_CREATE_JOINABLE), 0);
	PFC_ASSERT_INT(pthread_attr_setstacksize(&tattr,
						 IPC_EVENT_LIS_STKSIZE), 0);
	err = pthread_create(&ectx->ev_thread, &tattr, ipc_evctx_main,
			     ectx);
	PFC_ASSERT_INT(pthread_attr_destroy(&tattr), 0);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_dispatch;
	}

	/* Change event state to "RUNNING". */
	ipc_event_state = IPC_EVSTATE_RUNNING;
	IPC_CLIENT_UNLOCK();
	PFC_ASSERT_INT(pthread_sigmask(SIG_SETMASK, setp, NULL), 0);

	IPCCLNT_LOG_INFO("The IPC event subsystem has been initialized.");
	IPCCLNT_LOG_DEBUG("idle_timeout=%u, maxthreads=%u, conn_interval=%u,"
			  " timeout=%u",
			  dstopts->evopt_idle_timeout,
			  dstopts->evopt_maxthreads,
			  dstopts->evopt_conn_interval,
			  dstopts->evopt_timeout);

	return 0;

error_dispatch:
	/* Terminate event dispatch thread. */
	IPC_EVDISP_LOCK(disp);
	disp->ed_flags |= IPC_EVDISPF_SHUTDOWN;
	IPC_EVDISP_SIGNAL(disp);

	while (!IPC_EVDISP_IS_DONE(disp)) {
		IPC_EVDISP_WAIT(disp);
	}

	disp->ed_flags = 0;
	IPC_EVDISP_UNLOCK(disp);

error_pipe:
	ipc_event_close(&ectx->ev_pipe[0]);
	ipc_event_close(&ectx->ev_pipe[1]);

error_epoll:
	ipc_event_close(&ectx->ev_epoll);

error_epevents:
	free(ectx->ev_epevents);

error:
	if (rstr != NULL) {
		ipc_svname_chstate = NULL;
		pfc_refptr_put(rstr);
	}

	IPC_CLIENT_UNLOCK();

	if (setp != NULL) {
		PFC_ASSERT_INT(pthread_sigmask(SIG_SETMASK, setp, NULL), 0);
	}

	if (PFC_EXPECT_FALSE(err == 0)) {
		/* This should never happen. */
		err = EIO;
	}

	return err;
}

/*
 * int
 * pfc_ipcclnt_event_shutdown(void)
 *	Shut down the event listening.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_event_shutdown(void)
{
	int	err;

	IPC_CLIENT_WRLOCK();

	err = ipc_event_is_running();
	if (PFC_EXPECT_TRUE(err == 0)) {
		ipc_event_shutdown();
	}

	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * int
 * pfc_ipcclnt_event_fini(void)
 *	Finalize the IPC event subsystem.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_event_fini(void)
{
	pfc_rbtree_t	handlers;
	pfc_timespec_t	abstime;
	ipc_evctx_t	*ectx = &ipc_event_ctx;
	ipc_evdisp_t	*disp = &ipc_event_dispq;
	ipc_evtaskq_t	*tqp = &ipc_event_taskq;
	ipc_evstate_t	state;
	pthread_t	thr;
	int		err, wait_err = 0;

	err = ipc_event_gettime(&abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		return err;
	}

	abstime.tv_sec += IPC_EVENT_SHUTDOWN_TIMEOUT;

	IPC_CLIENT_WRLOCK();

	state = ipc_event_state;
	if (state == IPC_EVSTATE_RUNNING) {
		ipc_event_shutdown();
	}
	else if (PFC_EXPECT_FALSE(state != IPC_EVSTATE_SHUTDOWN)) {
		IPC_CLIENT_UNLOCK();

		return pfc_ipcclnt_event_state_error(state);
	}

	ipc_event_state = IPC_EVSTATE_FINI;

	/*
	 * Copy current event handler tree, and clear it.
	 * This code is safe because event handler is no longer added or
	 * removed.
	 */
	handlers = ipc_event_handlers;
	pfc_rbtree_init(&ipc_event_handlers, pfc_rbtree_uint32_compare,
			ipc_evhdlr_getkey);

	/* Join event listener thread. */
	thr = ectx->ev_thread;
	ectx->ev_thread = PFC_PTHREAD_INVALID_ID;
	IPC_CLIENT_UNLOCK();

	if (PFC_EXPECT_TRUE(thr != PFC_PTHREAD_INVALID_ID)) {
		err = pthread_join(thr, NULL);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPCCLNT_LOG_ERROR("Failed to join event listener "
					  "thread: %s", strerror(err));

			return err;
		}
	}

	/* Notify shut down to the event dispatch thread. */
	IPC_EVDISP_LOCK(disp);
	disp->ed_flags |= IPC_EVDISPF_SHUTDOWN;
	IPC_EVDISP_SIGNAL(disp);
	IPC_EVDISP_UNLOCK(disp);

	/* Wait for completion of all event tasks. */
	IPC_EVTASKQ_LOCK(tqp);
	PFC_ASSERT(IPC_EVTASKQ_IS_SHUTDOWN(tqp));
	while (tqp->etq_nthreads != 0) {
		if (PFC_EXPECT_FALSE(wait_err != 0)) {
			uint32_t	n = tqp->etq_nthreads;

			IPC_EVTASKQ_UNLOCK(tqp);
			IPCCLNT_LOG_ERROR("%u event task thread%s did not "
					  "stop within %u seconds.",
					  n, (n > 1) ? "s" : "",
					  IPC_EVENT_SHUTDOWN_TIMEOUT);

			return wait_err;
		}
		wait_err = IPC_EVTASKQ_TIMEDWAIT_ABS(tqp, &abstime);
	}
	IPC_EVTASKQ_UNLOCK(tqp);

	/* Wait for completion of the event dispatch thread. */
	wait_err = 0;
	IPC_EVDISP_LOCK(disp);
	while (!IPC_EVDISP_IS_DONE(disp)) {
		if (PFC_EXPECT_FALSE(wait_err != 0)) {
			IPC_EVDISP_UNLOCK(disp);
			IPCCLNT_LOG_ERROR("Event dispatch thread did not stop "
					  "within %u seconds.",
					  IPC_EVENT_SHUTDOWN_TIMEOUT);

			return wait_err;
		}
		wait_err = IPC_EVDISP_TIMEDWAIT_ABS(disp, &abstime);
	}
	IPC_EVDISP_UNLOCK(disp);

	/*
	 * Free up resources held by event listener.
	 * Event listener context lock does not need to be held because
	 * the event listener thread has already gone.
	 */
	ipc_event_close(&ectx->ev_epoll);
	ipc_event_close(&ectx->ev_pipe[0]);
	ipc_event_close(&ectx->ev_pipe[1]);
	free(ectx->ev_epevents);
	ectx->ev_epevents = NULL;

	/* Clean up event listeners. */
	pfc_rbtree_clear(&handlers, ipc_evhdlr_dtor, NULL);

	IPC_CLIENT_WRLOCK();

	/* Clean up all links from IPC channel to IPC host set. */
	pfc_ipcclnt_chan_iterate(ipc_evchan_dtor, NULL);

	/* Clean up IPC host set. */
	pfc_ipcclnt_hostset_fini();

	if (ipc_svname_chstate != NULL) {
		pfc_refptr_put(ipc_svname_chstate);
		ipc_svname_chstate = NULL;
	}

	IPC_CLIENT_UNLOCK();

	IPCCLNT_LOG_INFO("The IPC event subsystem has been finalized.");

	return 0;
}

/*
 * void
 * pfc_ipcclnt_event_setautocancel(pfc_bool_t value)
 *	Enable or disable auto-cancellation of IPC client sessions.
 *
 *	Auto-cancellation is enabled if PFC_TRUE is specified to `value'.
 *	When an event listener session is disconnected unexpectedly,
 *	the IPC event subsystem check whether auto-cancellation is enabled
 *	or not. If enabled, it calls pfc_ipcclnt_sess_cancel() with specifying
 *	PFC_FALSE to `discard' argument for each IPC client session associated
 *	with the same IPC server as the listener session.
 */
void
pfc_ipcclnt_event_setautocancel(pfc_bool_t value)
{
	(void)pfc_atomic_swap_uint8((uint8_t *)&ipc_auto_cancel, value);
}

/*
 * pfc_bool_t
 * pfc_ipcclnt_event_getautocancel(void)
 *	Determine whether auto-cancellation of IPC client session is enabled
 *	or not.
 */
pfc_bool_t
pfc_ipcclnt_event_getautocancel(void)
{
	return ipc_auto_cancel;
}

#define	ADDHDLR_TARGET		PFC_CONST_U(0x1)
#define	ADDHDLR_LOCK		PFC_CONST_U(0x2)
#define	ADDHDLR_CHLOCK		PFC_CONST_U(0x4)
#define	ADDHDLR_CHREF		PFC_CONST_U(0x8)
#define	ADDHDLR_ID		PFC_CONST_U(0x10)

/*
 * int
 * pfc_ipcevent_add_handler(pfc_ipcevhdlr_t *PFC_RESTRICT idp,
 *			    const char *PFC_RESTRICT channel,
 *			    pfc_ipcevfunc_t func,
 *			    const pfc_ipcevattr_t *PFC_RESTRICT attr,
 *			    const char *PFC_RESTRICT name)
 *	Add an IPC event handler which receives events generated by the IPC
 *	channel specified by `channel'.
 *
 *	`channel' is a pointer to IPC channel name, not IPC channel address.
 *	The IPC server's address part in `channel' is simply ignored.
 *      If NULL is specified, the channel name of the current default
 *      connection is used.
 *
 *	`attr' is a pointer to event attributes object which determines
 *	behavior of event handler. If NULL is specified to `attr', an event
 *	handler is added with default attributes.
 *
 *	`name' is a user-defined name of event handler.
 *	Currently, it is used only for event delivery logging.
 *	NULL means an anonymous handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, an identifier of the added event handler
 *	is set to the buffer pointed by `idp', and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	Note that argument destructor is called on error if it is set in
 *	`attr'.
 */
int
pfc_ipcevent_add_handler(pfc_ipcevhdlr_t *PFC_RESTRICT idp,
			 const char *PFC_RESTRICT channel,
			 pfc_ipcevfunc_t func,
			 const pfc_ipcevattr_t *PFC_RESTRICT attr,
			 const char *PFC_RESTRICT name)
{
	pfc_refptr_t	*rname;
	pfc_rbnode_t	*node;
	pfc_ipcevdtor_t	dtor;
	ipc_clchan_t	*chp = NULL;
	ipc_evhdlr_t	*ehp;
	ipc_hostset_t	*hset;
	ipc_hostent_t	*hep_error = NULL;
	ipc_evattr_t	*eattr = IPC_EVATTR_PTR(attr);
	ipc_elsess_t	*elsp, *sessions;
	uint32_t	errstate = 0;
	int		err;

	/* At first, obtain argument destructor for cleanup on error. */
	if (eattr == NULL) {
		/* Use default event attributes. */
		eattr = &ipc_evattr_default;
	}
	dtor = eattr->eva_argdtor;

	if (PFC_EXPECT_FALSE(idp == NULL || func == NULL)) {
		err = EINVAL;
		goto error;
	}

	/* Create a copy of handler's name. */
	if (name == NULL) {
		rname = NULL;
	}
	else {
		rname = pfc_refptr_string_create(name);
		if (PFC_EXPECT_FALSE(rname == NULL)) {
			return ENOMEM;
		}
	}

	/* Allocate a new event handler instance. */
	ehp = (ipc_evhdlr_t *)malloc(sizeof(*ehp));
	if (PFC_EXPECT_FALSE(ehp == NULL)) {
		IPCCLNT_LOG_ERROR("Failed to allocate event handler.");
		err = ENOMEM;
		goto error_name;
	}

	err = PFC_MUTEX_INIT(&ehp->evh_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCCLNT_LOG_ERROR("Failed to initialize handler mutex: %s",
				  strerror(err));
		goto error_free;
	}

	err = pfc_cond_init(&ehp->evh_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCCLNT_LOG_ERROR("Failed to initialize handler condvar: %s",
				  strerror(err));
		goto error_free;
	}

	/* Initialize event handler instance. */
	ehp->evh_priority = eattr->eva_priority;
	ehp->evh_refcnt = 1;
	ehp->evh_flags = (eattr->eva_flags & IPC_EVATTRF_LOG)
		? IPC_EVHDF_LOG : 0;
	ehp->evh_name = rname;
	ehp->evh_func = func;
	ehp->evh_arg = eattr->eva_arg;
	ehp->evh_argdtor = dtor;
	ehp->evh_delivered = 0;

	/* Copy event target set. */
	err = pfc_ipc_evset_copy(&ehp->evh_target, &eattr->eva_target);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to copy target event set: %s",
				  ipc_evhdlr_name(ehp), strerror(err));
		goto error_free;
	}

	IPC_CLIENT_WRLOCK();
	errstate |= (ADDHDLR_TARGET | ADDHDLR_LOCK);

	/* Check state of the event subsystem. */
	err = ipc_event_is_running();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_free;
	}

	/*
	 * Ensure that `idp' points PFC_IPCEVHDLR_INVALID if the specified
	 * handler is written in C++ language.
	 */
	if ((eattr->eva_flags & IPC_EVATTRF_CXX) &&
	    PFC_EXPECT_FALSE(*idp != PFC_IPCEVHDLR_INVALID)) {
		err = EEXIST;
		dtor = NULL;	/* Never call destructor. */
		goto error_free;
	}

	/*
	 * Fetch IPC channel configuration.
	 * IPC server's address part of the IPC channel address is simply
	 * ignored.
	 */
	err = pfc_ipcclnt_getchan(channel, &chp, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to obtain IPC channel: "
				  "channel=%s, %s",
				  ipc_evhdlr_name(ehp), channel,
				  strerror(err));
		goto error_free;
	}
	ehp->evh_chan = chp;

	/* Determine target hosts. */
	if ((hset = eattr->eva_hostset) == NULL) {
		/* Use default host set which contains local host only. */
		hset = &ipc_hostset_local;
	}
#ifdef	PFC_VERBOSE_DEBUG
	else {
		ipc_hostset_t	*h;
		const char	*hsname = IPC_HOSTSET_NAME(hset);

		PFC_ASSERT_INT(pfc_ipcclnt_hostset_lookup(hsname, &h), 0);
		PFC_ASSERT(h == hset);
	}
#endif	/* PFC_VERBOSE_DEBUG */
	PFC_ASSERT(hset->hs_refcnt != 0);
	ehp->evh_hostset = hset;

	IPC_CLCHAN_WRLOCK(chp);
	errstate |= ADDHDLR_CHLOCK;

	/* Create linkage between the IPC channel and the IPC host set. */
	err = ipc_chref_create(chp, hset);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_free;
	}
	errstate |= ADDHDLR_CHREF;

	/* Assign an identifier to this event handler. */
	err = ipc_evhdlr_idalloc(ehp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to assign handler's ID: %s",
				  ipc_evhdlr_name(ehp), strerror(err));
		goto error_free;
	}
	errstate |= ADDHDLR_ID;

	/* Link event handler to the IPC channel. */
	ipc_evhdlr_add(chp, ehp);

	/* Create IPC event sessions for target hosts. */
	sessions = NULL;
	node = NULL;
	IPC_HOSTSET_RDLOCK(hset);
	while ((node = pfc_rbtree_next(&hset->hs_host, node)) != NULL) {
		ipc_hostent_t	*hep = IPC_HOSTENT_NODE2PTR(node);

		err = ipc_elsess_setup(chp, ehp, &hep->he_addr, &elsp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			hep_error = hep;
			goto error_elsess;
		}

		elsp->els_tmplink = sessions;
		sessions = elsp;
	}
	IPC_HOSTSET_UNLOCK(hset);

	/* Hold the host set. */
	IPC_HOSTSET_HOLD(hset);

	/* Update session state in background. */
	for (elsp = sessions; elsp != NULL; elsp = elsp->els_tmplink) {
		ipc_evtaskq_post_update(elsp);
	}

	*idp = ehp->evh_id;

	IPC_CLCHAN_UNLOCK(chp);
	IPC_CLIENT_UNLOCK();

	return 0;

error_elsess:
	if (hep_error != NULL) {
		node = NULL;
		while ((node = pfc_rbtree_next(&hset->hs_host, node)) != NULL) {
			ipc_hostent_t	*hep = IPC_HOSTENT_NODE2PTR(node);

			if (hep == hep_error) {
				break;
			}

			ipc_elsess_unlink(chp, ehp, &hep->he_addr);
		}
		IPC_HOSTSET_UNLOCK(hset);
	}

error_free:
	if (errstate & ADDHDLR_ID) {
		pfc_list_remove(&ehp->evh_list);
		pfc_rbtree_remove_node(&ipc_event_handlers, &ehp->evh_node);
	}
	if (errstate & ADDHDLR_CHREF) {
		ipc_chref_destroy(chp, hset);
	}
	if (errstate & ADDHDLR_CHLOCK) {
		IPC_CLCHAN_UNLOCK(chp);
	}
	if (errstate & ADDHDLR_LOCK) {
		IPC_CLIENT_UNLOCK();
	}
	if (errstate & ADDHDLR_TARGET) {
		pfc_ipc_evset_destroy(&ehp->evh_target);
	}
	free(ehp);

	if (chp != NULL) {
		IPC_CLCHAN_RELEASE(chp);
	}

error_name:
	if (rname != NULL) {
		pfc_refptr_put(rname);
	}

error:
	if (dtor != NULL) {
		/* Call argument destructor. */
		(*dtor)(eattr->eva_arg);
	}

	return err;
}

/*
 * int
 * pfc_ipcevent_remove_handler(pfc_ipcevhdlr_t id)
 *	Remove the IPC event handler associated with the specified ID.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_remove_handler(pfc_ipcevhdlr_t id)
{
	pfc_rbnode_t	*node;
	ipc_evhdlr_t	*ehp;
	ipc_clchan_t	*chp;
	ipc_hostset_t	*hset;
	int		err;

	IPC_CLIENT_WRLOCK();

	/* Check state of the event subsystem. */
	err = ipc_event_is_available();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Remove the handler from handler's ID tree. */
	node = pfc_rbtree_remove(&ipc_event_handlers, IPC_EVHDLR_KEY(id));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		err = ENOENT;
		goto error;
	}

	ehp = IPC_EVHDLR_NODE2PTR(node);

	IPCCLNT_LOG_DEBUG("Removing IPC event handler: id=%u, name=%s",
			  id, ipc_evhdlr_name(ehp));

	chp = ehp->evh_chan;
	IPC_CLCHAN_WRLOCK(chp);

	/* Unlink the handler from the IPC channel. */
	pfc_list_remove(&ehp->evh_list);

	/* Set removed flag to the handler. */
	IPC_EVHDLR_LOCK(ehp);
	ehp->evh_flags |= IPC_EVHDF_REMOVED;
	IPC_EVHDLR_UNLOCK(ehp);

	/* Destroy linkage between the IPC channel and the IPC host set. */
	hset = ehp->evh_hostset;
	ipc_chref_destroy(chp, hset);

	/* Unlink this handler from listener sessions. */
	IPC_HOSTSET_RDLOCK(hset);
	node = NULL;
	while ((node = pfc_rbtree_next(&hset->hs_host, node)) != NULL) {
		ipc_hostent_t	*hep = IPC_HOSTENT_NODE2PTR(node);

		ipc_elsess_unlink(chp, ehp, &hep->he_addr);
	}
	IPC_HOSTSET_UNLOCK(hset);

	IPC_CLCHAN_UNLOCK(chp);
	IPC_CLIENT_UNLOCK();

	/* Ensure that this handler is not being called. */
	IPC_EVHDLR_LOCK(ehp);
	err = ipc_evhdlr_wait(ehp);

	/* Release the handler. */
	IPC_EVHDLR_RELEASE_L(ehp);

	return err;

error:
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * int
 * pfc_ipcevent_isconnected(const char *PFC_RESTRICT channel,
 *			    const pfc_hostaddr_t *PFC_RESTRICT addr)
 *	Determine whether the event listener session associated with the IPC
 *	server specified by `channel' and `addr' is established or not.
 *
 *	`channel' is an IPC channel name. The default channel name is used
 *	if it is NULL.
 *	`addr' is the host address of the IPC server. Local host address is
 *	used if it is NULL.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified event listener session is
 *	established.
 *
 *	ENOENT is returned if no event handler is registered to the specified
 *	IPC server.
 *	ECONNRESET is returned if the event listener session specified by
 *	`channel' and `addr' is not established.
 *	EINPROGRESS is returned if the IPC client library is trying to
 *	establish the event listener session.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_isconnected(const char *PFC_RESTRICT channel,
			 const pfc_hostaddr_t *PFC_RESTRICT addr)
{
	ipc_clchan_t	*chp;
	ipc_elsess_t	*elsp;
	pfc_hostaddr_t	laddr;
	int		err;

	if (addr == NULL) {
		PFC_ASSERT_INT(pfc_hostaddr_init_local(&laddr), 0);
		addr = &laddr;
	}

	IPC_CLIENT_RDLOCK();

	/*
	 * Search for the event listener session associated with the given
	 * host address.
	 */
	err = ipc_elsess_lookup(channel, addr, &chp, &elsp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	IPC_ELSESS_LOCK(elsp);

	/* Determine the state of the event listener session. */
	if (PFC_EXPECT_FALSE(elsp->els_flags & IPC_ELSF_CONNECTING)) {
		if (elsp->els_prevdown == IPC_CHDOWN_NONE) {
			err = EINPROGRESS;
		}
		else {
			/*
			 * This listener session is being reconnected.
			 * In this case we should treat this session is
			 * DOWN because PFC_IPCCHSTATE_DOWN event will never
			 * be posted even on error.
			 */
			err = ECONNRESET;
		}
	}
	else if (PFC_EXPECT_TRUE(elsp->els_stream != NULL)) {
		err = 0;
	}
	else {
		err = ECONNRESET;
	}

	IPC_ELSESS_UNLOCK(elsp);
	IPC_CLCHAN_UNLOCK(chp);

out:
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * int
 * pfc_ipcevent_notifystate(pfc_ipcevhdlr_t id)
 *	Post an IPC channel state notification event to the specified IPC
 *	event handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_notifystate(pfc_ipcevhdlr_t id)
{
	ipc_clchan_t	*chp;
	ipc_hostset_t	*hset;
	ipc_evhdlr_t	*ehp;
	pfc_rbnode_t	*node, *hnode;
	int		err;

	IPC_CLIENT_RDLOCK();

	err = ipc_event_is_running();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Determine event handler. */
	node = pfc_rbtree_get(&ipc_event_handlers, IPC_EVHDLR_KEY(id));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		err = ENOENT;
		goto out;
	}

	ehp = IPC_EVHDLR_NODE2PTR(node);
	chp = ehp->evh_chan;
	hset = ehp->evh_hostset;

	/*
	 * Ensure that the specified handler targets channel state notification
	 * event.
	 */
	if (PFC_EXPECT_FALSE(!pfc_ipc_evset_contains(&ehp->evh_target,
						     ipc_str_svname_chstate,
						     PFC_IPCCHSTATE_NOTIFY))) {
		err = EINVAL;
		goto out;
	}

	/*
	 * Iterate event listener sessions associated with the specified
	 * handler.
	 */
	IPC_CLCHAN_RDLOCK(chp);
	IPC_HOSTSET_RDLOCK(hset);

	hnode = NULL;
	while ((hnode = pfc_rbtree_next(&hset->hs_host, hnode)) != NULL) {
		ipc_hostent_t	*hep = IPC_HOSTENT_NODE2PTR(hnode);
		ipc_elsess_t	*elsp;

		node = pfc_rbtree_get(&chp->ichc_elsess,
				      (pfc_cptr_t)&hep->he_addr);
		PFC_ASSERT(node != NULL);
		elsp = IPC_ELSESS_NODE2PTR(node);

		IPC_ELSESS_LOCK(elsp);
		if (!IPC_ELSESS_IS_SHUTDOWN(elsp)) {
			uint32_t	state;
			int		e;

			/* Determine the state of the event listener session. */
			state = (elsp->els_stream != NULL &&
				 (elsp->els_flags & IPC_ELSF_CONNECTING) == 0)
				? PFC_IPCCHNOTIFY_UP
				: PFC_IPCCHNOTIFY_DOWN;

			/* Post a state notification event. */
			e = ipc_elsess_post_notification(elsp, ehp, state);
			if (PFC_EXPECT_FALSE(e != 0 && err == 0)) {
				err = e;
			}
		}
		IPC_ELSESS_UNLOCK(elsp);
	}

	IPC_HOSTSET_UNLOCK(hset);
	IPC_CLCHAN_UNLOCK(chp);

out:
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * int
 * pfc_ipcevent_reconnect(const char *PFC_RESTRICT channel,
 *			  const pfc_hostaddr_t *PFC_RESTRICT addr)
 *	Force to reconnect existing IPC event listener session associated with
 *	the IPC channel name `channel' and the host address `addr'.
 *
 *	If the IPC event listener session specified by `channel' and `addr'
 *	is not established, this method tries to reconnect it in background.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcevent_reconnect(const char *PFC_RESTRICT channel,
		       const pfc_hostaddr_t *PFC_RESTRICT addr)
{
	ipc_clchan_t	*chp;
	ipc_elsess_t	*elsp;
	int		err;

	if (PFC_EXPECT_FALSE(channel == NULL || addr == NULL)) {
		return EINVAL;
	}

	IPC_CLIENT_RDLOCK();

	/*
	 * Search for the event listener session associated with the given
	 * host address.
	 */
	err = ipc_elsess_lookup(channel, addr, &chp, &elsp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	IPC_ELSESS_LOCK(elsp);
	IPC_ELSESS_HOLD(elsp);

	if (PFC_EXPECT_FALSE(IPC_ELSESS_IS_SHUTDOWN(elsp))) {
		/* This session is going to be shut down. */
		err = ENOENT;
		goto out_unlock;
	}

	/*
	 * Update the session state if the listener session is not established,
	 * or it is now connecting.
	 */
	if (elsp->els_stream == NULL ||
	    (elsp->els_flags & IPC_ELSF_CONNECTING)) {
		/* Unregister timeout for this session. */
		ipc_evctx_untimeout(&elsp->els_timeout);
		PFC_ASSERT(elsp->els_refcnt > 0);

		/* Update the session state on an event task queue thread. */
		ipc_evtaskq_post(elsp);
	}

out_unlock:
	IPC_ELSESS_RELEASE(elsp);
	IPC_CLCHAN_UNLOCK(chp);

out:
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_event_state_error(ipc_evstate_t state)
 *	Return error number which indicates bad event state.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_event_state_error(ipc_evstate_t state)
{
	int	err;

	if (state == IPC_EVSTATE_INITIAL) {
		/* Not initialized. */
		err = ENXIO;
	}
	else if (state == IPC_EVSTATE_RUNNING) {
		/* Already running. */
		err = EBUSY;
	}
	else if (state == IPC_EVSTATE_FORK) {
		/* IPC event is disabled on a child process. */
		err = EPERM;
	}
	else {
		/* Already finalized. */
		PFC_ASSERT(state == IPC_EVSTATE_FINI ||
			   state == IPC_EVSTATE_SHUTDOWN);
		err = ECANCELED;
	}

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_event_fork_prepare(void)
 *	fork(2) handler which will be called just before fork(2) on parent
 *	process.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	writer mode.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_event_fork_prepare(void)
{
	pfc_ipcclnt_chan_iterate(ipc_evchan_fork_prepare, NULL);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_event_fork_parent(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on parent process.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	writer mode.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_event_fork_parent(void)
{
	pfc_ipcclnt_chan_iterate(ipc_evchan_fork_parent, NULL);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_event_fork_child(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on child process.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_event_fork_child(void)
{
	pfc_ipcclnt_chan_iterate(ipc_evchan_fork_child, NULL);

	if (ipc_event_state != IPC_EVSTATE_INITIAL) {
		/* Disable the IPC event subsystem. */
		ipc_event_state = IPC_EVSTATE_FORK;
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_evchan_init(ipc_clchan_t *chp)
 *	Initialize event related fields in the IPC channel.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_evchan_init(ipc_clchan_t *chp)
{
	pfc_rbtree_init(&chp->ichc_elsess, (pfc_rbcomp_t)pfc_hostaddr_compare,
			ipc_elsess_getkey);
	pfc_rbtree_init(&chp->ichc_evhostset, pfc_rbtree_ulong_compare,
			ipc_chref_host_getkey);
	pfc_list_init(&chp->ichc_evhandler);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_evchan_addhost(ipc_clchan_t *PFC_RESTRICT chp,
 *			      ipc_hostset_t *PFC_RESTRICT hset,
 *			      const pfc_hostaddr_t *PFC_RESTRICT addr)
 *	Set up the event listener session for the host address specified
 *	by `addr'.
 *
 *	This function is called when a new host address is added to the
 *	host set specified by `hset'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_evchan_addhost(ipc_clchan_t *PFC_RESTRICT chp,
			   ipc_hostset_t *PFC_RESTRICT hset,
			   const pfc_hostaddr_t *PFC_RESTRICT addr)
{
	pfc_list_t	*elem;
	ipc_elsess_t	*elsp = NULL;
	uint32_t	nlinks = 0, nwilds = 0;
	int		err;

	IPC_CLCHAN_WRLOCK(chp);

	PFC_LIST_FOREACH(&chp->ichc_evhandler, elem) {
		ipc_evhdlr_t	*ehp = IPC_EVHDLR_LIST2PTR(elem);
		ipc_elsess_t	*e;

		if (ehp->evh_hostset != hset) {
			continue;
		}

		/* Set up listener session. */
		err = ipc_elsess_setup(chp, ehp, addr, &e);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}

		PFC_ASSERT(elsp == NULL || e == elsp);
		elsp = e;
		nlinks++;

		if (pfc_ipc_evset_isempty(&ehp->evh_target)) {
			nwilds++;
		}
	}

	/* Update session state if needed. */
	PFC_ASSERT(elsp != NULL);
	ipc_evtaskq_post_update(elsp);

	IPC_CLCHAN_UNLOCK(chp);

	return 0;

error:
	/* Revert changes. */
	if (elsp != NULL) {
		PFC_ASSERT(nlinks != 0);
		IPC_ELSESS_LOCK(elsp);
		ipc_elsess_unlink_impl(chp, elsp, nlinks, nwilds);
	}

	IPC_CLCHAN_UNLOCK(chp);

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_evchan_removehost(ipc_clchan_t *PFC_RESTRICT chp,
 *				 ipc_hostset_t *PFC_RESTRICT hset,
 *				 const pfc_hostaddr_t *PFC_RESTRICT addr)
 *	Unlink the event listener session associated with the host address
 *	specified by `addr'.
 *
 *	This function is called when a host address is removed from the
 *	host set specified by `hset'.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_evchan_removehost(ipc_clchan_t *PFC_RESTRICT chp,
			      ipc_hostset_t *PFC_RESTRICT hset,
			      const pfc_hostaddr_t *PFC_RESTRICT addr)
{
	pfc_rbnode_t	*node;
	pfc_list_t	*elem;
	ipc_elsess_t	*elsp;
	uint32_t	nlinks = 0;

	IPC_CLCHAN_WRLOCK(chp);

	node = pfc_rbtree_get(&chp->ichc_elsess, (pfc_cptr_t)addr);
	PFC_ASSERT(node != NULL);

	elsp = IPC_ELSESS_NODE2PTR(node);
	IPC_ELSESS_LOCK(elsp);

	/* Determine link counter to subtract. */
	PFC_LIST_FOREACH(&chp->ichc_evhandler, elem) {
		ipc_evhdlr_t	*ehp = IPC_EVHDLR_LIST2PTR(elem);

		if (ehp->evh_hostset != hset) {
			continue;
		}

		nlinks++;
	}

	ipc_elsess_unlink_impl(chp, elsp, nlinks, 0);
	IPC_CLCHAN_UNLOCK(chp);
}

/*
 * int
 * pfc_ipcclnt_evsysops_install(ipc_cevsysops_t *ops)
 *	Install operations for the IPC event subsystem.
 *
 *	This function must be called before the call of
 *	pfc_ipcclnt_event_init().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EEXIST is returned if another operation is already installed.
 *	EBUSY is returned if the IPC event subsystem is already initialized.
 *	EPERM is returned if the IPC event subsystem is disabled by fork(2).
 *	ECANCELED is returned if the IPC event subsystem is already finalized.
 */
int
pfc_ipcclnt_evsysops_install(ipc_cevsysops_t *ops)
{
	ipc_evstate_t	state;
	int		err;

	PFC_ASSERT(ops != NULL);

	IPC_CLIENT_WRLOCK();

	state = ipc_event_state;
	if (PFC_EXPECT_FALSE(state != IPC_EVSTATE_INITIAL)) {
		err = pfc_ipcclnt_event_state_error(state);
	}
	else if (PFC_EXPECT_TRUE(ipc_evsys_ops == NULL)) {
		ipc_evsys_ops = ops;
		err = 0;
	}
	else {
		err = EEXIST;
	}

	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * void
 * pfc_ipcclnt_evsysops_uninstall(ipc_cevsysops_t *ops)
 *	Uninstall the IPC event subsystem operations.
 */
void
pfc_ipcclnt_evsysops_uninstall(ipc_cevsysops_t *ops)
{
	IPC_CLIENT_WRLOCK();

	if (PFC_EXPECT_TRUE(ipc_evsys_ops == ops)) {
		ipc_evsys_ops = NULL;
	}

	IPC_CLIENT_UNLOCK();
}

/*
 * static void
 * ipc_event_shutdown(void)
 *	Shut down the event listening.
 *
 * Remarks:
 *	- This function must be called with holding the client lock in writer
 *	  mode.
 *
 *	- The caller must ensure that the event subsystem is running.
 */
static void
ipc_event_shutdown(void)
{
	ipc_evctx_t	*ectx = &ipc_event_ctx;
	ipc_evtaskq_t	*tqp = &ipc_event_taskq;

	IPCCLNT_LOG_INFO("Shutting down the IPC event subsystem.");

	PFC_ASSERT(ipc_event_state == IPC_EVSTATE_RUNNING);
	ipc_event_state = IPC_EVSTATE_SHUTDOWN;

	/* Send shut down signal to the listener thread. */
	IPC_EVCTX_LOCK(ectx);
	ipc_event_close(&IPC_EVCTX_WRPIPE(ectx));
	IPC_EVCTX_UNLOCK(ectx);

	/* Shut down the event task queue. */
	IPC_EVTASKQ_LOCK(tqp);
	tqp->etq_flags |= IPC_EVTQF_SHUTDOWN;
	IPC_EVTASKQ_BROADCAST(tqp);
	IPC_EVTASKQ_UNLOCK(tqp);
}

/*
 * static void *
 * ipc_evdisp_main(void *arg)
 *	Start routine of IPC event dispatch thread.
 */
static void *
ipc_evdisp_main(void *arg)
{
	ipc_evdisp_t	*disp = (ipc_evdisp_t *)arg;
	ipc_clevqent_t	*ent;
	int		err;

	PFC_ASSERT(disp == &ipc_event_dispq);

	IPCCLNT_LOG_DEBUG("Event dispatch thread has been started.");

	/* Call constructor. */
	err = ipc_evdisp_ctor();

	/* Wake up a thread blocked in pfc_ipcclnt_event_init(). */
	IPC_EVDISP_LOCK(disp);
	disp->ed_ctorerr = err;
	disp->ed_flags |= IPC_EVDISPF_INIT;
	IPC_EVDISP_SIGNAL(disp);
	IPC_EVDISP_UNLOCK(disp);

	if (PFC_EXPECT_TRUE(err == 0)) {
		while ((ent = ipc_evdisp_dequeue(disp)) != NULL) {
			ipc_evdisp_dispatch(ent);
		}

		/* Call destructor. */
		ipc_evdisp_dtor();
	}

	IPC_EVDISP_LOCK(disp);
	disp->ed_flags |= IPC_EVDISPF_DONE;
	IPC_EVDISP_SIGNAL(disp);
	IPCCLNT_LOG_DEBUG("Event dispatch thread has been terminated.");
	IPC_EVDISP_UNLOCK(disp);

	return NULL;
}

/*
 * static int
 * ipc_evdisp_ctor(void)
 *	Call constructor of the IPC event subsystem if installed.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_evdisp_ctor(void)
{
	ipc_cevsysops_t	*ops;
	int		err;

	/*
	 * Call constructor if the event subsystem operation is installed.
	 * Note that we can access ipc_evsys_ops without holding the client
	 * lock because it is held by pfc_ipcclnt_event_init().
	 */
	ops = ipc_evsys_ops;
	if (ops == NULL) {
		return 0;
	}

	IPCCLNT_LOG_VERBOSE("Calling eop_disp_ctor(): %p", ops->eop_disp_ctor);
	err = ops->eop_disp_ctor();

	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("eop_disp_ctor() returned an error: %d",
				  err);
		/* FALLTHROUGH */
	}

	return err;
}

/*
 * static void
 * ipc_evdisp_dtor(void)
 *	Call destructor of the IPC event subsystem if installed.
 */
static void
ipc_evdisp_dtor(void)
{
	ipc_cevsysops_t	*ops;

	/* Uninstall IPC event subsystem operation. */
	IPC_CLIENT_WRLOCK();
	ops = ipc_evsys_ops;
	ipc_evsys_ops = NULL;
	IPC_CLIENT_UNLOCK();

	if (ops != NULL) {
		IPCCLNT_LOG_VERBOSE("Calling eop_disp_dtor(): %p",
				    ops->eop_disp_dtor);
		ops->eop_disp_dtor();
	}
}

/*
 * static uint32_t
 * ipc_evdisp_post(ipc_elsess_t *PFC_RESTRICT elsp,
 *		   pfc_ipcevent_t *PFC_RESTRICT event)
 *	Post an IPC event to the event dispatch queue.
 *
 * Calling/Exit State:
 *	The number of event handlers which requires the specified event
 *	is returned.
 *
 * Remarks:
 *	This function must be called with holding the IPC channel lock in
 *	reader or writer mode.
 */
static uint32_t
ipc_evdisp_post(ipc_elsess_t *PFC_RESTRICT elsp,
		pfc_ipcevent_t *PFC_RESTRICT event)
{
	pfc_list_t	*elem;
	pfc_ipcevtype_t	type = IPC_EVENT_TYPE(event);
	const char	*service = IPC_EVENT_SERVICE(event);
	ipc_clchan_t	*chp = elsp->els_chan;
	uint32_t	count = 0;

	/* Scan all event handlers associated with this IPC channel. */
	PFC_LIST_FOREACH(&chp->ichc_evhandler, elem) {
		ipc_evhdlr_t	*ehp = IPC_EVHDLR_LIST2PTR(elem);
		ipc_evset_t	*target = &ehp->evh_target;

		/* Test event target set. */
		if (!pfc_ipc_evset_contains(target, service, type)) {
			continue;
		}

		/* Test host address of the IPC server. */
		if (ipc_hostset_contains(ehp->evh_hostset, &event->ie_addr)
		    != 0) {
			continue;
		}

		/* Post the event to this handler. */
		(void)ipc_evdisp_post_event(event, ehp);
		count++;
	}

	return count;
}

/*
 * static int
 * ipc_evdisp_post_event(pfc_ipcevent_t *PFC_RESTRICT event,
 *			 ipc_evhdlr_t *PFC_RESTRICT ehp)
 *	Post an IPC event to the specified event handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_evdisp_post_event(pfc_ipcevent_t *PFC_RESTRICT event,
		      ipc_evhdlr_t *PFC_RESTRICT ehp)
{
	ipc_evdisp_t	*disp = &ipc_event_dispq;
	ipc_clevqent_t	*ent;

	/* Allocate an IPC event queue entry. */
	ent = (ipc_clevqent_t *)malloc(sizeof(*ent));
	if (PFC_EXPECT_FALSE(ent == NULL)) {
		IPCCLNT_LOG_ERROR("Failed to allocate event queue entry: "
				  "event=%s/%u, handler=%s",
				  IPC_EVENT_SERVICE(event),
				  IPC_EVENT_TYPE(event), ipc_evhdlr_name(ehp));

		return ENOMEM;
	}

	ent->ceq_event = event;
	ent->ceq_handler = ehp;
	IPC_EVENT_HOLD(event);
	IPC_EVHDLR_HOLD(ehp);

	IPC_EVDISP_LOCK(disp);

	/* Ensure that the event dispatch queue is active. */
	if (IPC_EVDISP_IS_SHUTDOWN(disp)) {
		IPC_EVDISP_UNLOCK(disp);
		IPC_EVENT_RELEASE(event);
		IPC_EVHDLR_RELEASE(ehp);
		free(ent);

		return ECANCELED;
	}

	/* Link queue entry to the tail of the dispatch queue. */
	*(disp->ed_qnext) = ent;
	disp->ed_qnext = &ent->ceq_next;
	ent->ceq_next = NULL;

	IPC_EVDISP_SIGNAL(disp);
	IPC_EVDISP_UNLOCK(disp);

	return 0;
}

/*
 * static ipc_clevqent_t *
 * ipc_evdisp_dequeue(ipc_evdisp_t *disp)
 *	Dequeue one event entry from the event dispatch queue.
 *
 * Calling/Exit State:
 *	A non-NULL pointer to an event entry is returned on success.
 *	NULL is returned if the event dispatch thread should be terminated.
 */
static ipc_clevqent_t *
ipc_evdisp_dequeue(ipc_evdisp_t *disp)
{
	ipc_clevqent_t	*ent;

	IPC_EVDISP_LOCK(disp);

	for (;;) {
		ent = disp->ed_queue;

		if (IPC_EVDISP_IS_SHUTDOWN(disp)) {
			/* Make the queue empty. */
			disp->ed_queue = NULL;
			disp->ed_qnext = &disp->ed_queue;
			IPC_EVDISP_UNLOCK(disp);

			/* Discard all events. */
			ipc_evdisp_discard(ent);

			return NULL;
		}

		if (ent != NULL) {
			/* Dequeue this event entry. */
			disp->ed_queue = ent->ceq_next;
			if (disp->ed_queue == NULL) {
				disp->ed_qnext = &disp->ed_queue;
			}
			break;
		}

		/* Wait for an event entry. */
		IPC_EVDISP_WAIT(disp);
	}

	IPC_EVDISP_UNLOCK(disp);

	return ent;
}

/*
 * static void
 * ipc_evdisp_discard(ipc_clevqent_t *ent)
 *	Discard all event entries on the specified entry list.
 */
static void
ipc_evdisp_discard(ipc_clevqent_t *ent)
{
	ipc_clevqent_t	*next;

	for (; ent != NULL; ent = next) {
		pfc_ipcevent_t	*event = ent->ceq_event;
		ipc_evhdlr_t	*ehp = ent->ceq_handler;

		next = ent->ceq_next;
		free(ent);
		IPC_EVENT_RELEASE(event);
		IPC_EVHDLR_RELEASE(ehp);
	}
}

/*
 * static void
 * ipc_evdisp_dispatch(ipc_clevqent_t *ent)
 *	Dispatch the specified event entry to the event handler.
 *
 * Remarks:
 *	This function always frees the given event entry.
 */
static void
ipc_evdisp_dispatch(ipc_clevqent_t *ent)
{
	pfc_ipcevent_t	*event = ent->ceq_event;
	ipc_evhdlr_t	*ehp = ent->ceq_handler;
	pfc_bool_t	removed, need_log;

	/* Event queue entry is no longer used. */
	free(ent);

	/* Event type test must be already done. */
	PFC_ASSERT(pfc_ipc_evset_contains(&ehp->evh_target,
					  IPC_EVENT_SERVICE(event),
					  IPC_EVENT_TYPE(event)));

	IPC_EVHDLR_LOCK(ehp);

	/* Ensure that this handler still exists. */
	if (IPC_EVHDLR_IS_REMOVED(ehp)) {
		removed = PFC_TRUE;
	}
	else {
		removed = PFC_FALSE;
		ehp->evh_delivered++;
		ehp->evh_flags |= IPC_EVHDF_ACTIVE;
	}

	/* Determine whether delivery log is needed or not. */
	need_log = (IPC_EVHDLR_NEEDS_LOG(ehp)) ? PFC_TRUE : PFC_FALSE;

	IPC_EVHDLR_UNLOCK(ehp);

	if (PFC_EXPECT_TRUE(!removed)) {
		if (need_log) {
			ipc_evdisp_log(event, ehp, "Calling");
		}

		IPC_EVDISP_CALL_ASSERT_PUSH(ehp);
		ehp->evh_func(event, ehp->evh_arg);
		IPC_EVDISP_CALL_ASSERT_POP();

		if (need_log) {
			ipc_evdisp_log(event, ehp, "Returned");
		}
	}

	/* Unlink reference to the event object. */
	IPC_EVENT_RELEASE(event);

	/* Unlink reference to the event handler instance. */
	IPC_EVHDLR_LOCK(ehp);
	ehp->evh_flags &= ~IPC_EVHDF_ACTIVE;
	IPC_EVHDLR_RELEASE_L(ehp);
}

/*
 * static void
 * ipc_evdisp_log(pfc_ipcevent_t *PFC_RESTRICT event,
 *		  ipc_evhdlr_t *PFC_RESTRICT ehp,
 *		  const char *PFC_RESTRICT message)
 *	Record event delivery log.
 */
static void
ipc_evdisp_log(pfc_ipcevent_t *PFC_RESTRICT event,
	       ipc_evhdlr_t *PFC_RESTRICT ehp,
	       const char *PFC_RESTRICT message)
{
	ipc_clchan_t	*chp = ehp->evh_chan;
	char		addr[PFC_HOSTADDR_STRSIZE];
	int		err;

	pfc_log_burst_info_begin();
	err = pfc_hostaddr_tostring(&event->ie_addr, addr, sizeof(addr),
				    PFC_HA2STR_TYPE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		(void)pfc_strlcpy(addr, "???", sizeof(addr));
	}

	pfc_log_burst_write("%s: "
			    "handler[id=%u name=\"%s\" pri=%u] "
			    "event[id=%u chan=%s@%s service=%s/%u]",
			    message,
			    ehp->evh_id, ipc_evhdlr_name(ehp),
			    ehp->evh_priority,
			    IPC_EVENT_SERIAL(event),
			    IPC_CLCHAN_NAME(chp), addr,
			    IPC_EVENT_SERVICE(event), IPC_EVENT_TYPE(event));
	pfc_log_burst_end();
}

/*
 * static void *
 * ipc_evtaskq_main(void *arg)
 *	Main routine of event task thread.
 */
static void *
ipc_evtaskq_main(void *arg)
{
	ipc_evtaskq_t	*tqp = (ipc_evtaskq_t *)arg;
	ipc_elsess_t	*elsp;
	pfc_timespec_t	timeout;

	PFC_ASSERT(tqp == &ipc_event_taskq);

	pfc_clock_msec2time(&timeout, ipc_event_opts.evopt_idle_timeout);

	IPC_EVTASKQ_LOCK(tqp);

	while ((elsp = ipc_evtaskq_dequeue(tqp, &timeout)) != NULL) {
		/* Dispatch event listener session task. */
		IPC_EVTASKQ_UNLOCK(tqp);
		ipc_elsess_update(elsp);
		IPC_EVTASKQ_LOCK(tqp);
	}

	tqp->etq_nthreads--;
	if (IPC_EVTASKQ_IS_SHUTDOWN(tqp) && tqp->etq_nthreads == 0) {
		IPC_EVTASKQ_BROADCAST(tqp);
	}

	IPC_EVTASKQ_UNLOCK(tqp);

	return NULL;
}

/*
 * static ipc_elsess_t *
 * ipc_evtaskq_dequeue(ipc_evtaskq_t *PFC_RESTRICT tqp,
 *		       ctimespec_t *PFC_RESTRICT timeout)
 *	Dequeue one event listener task from the event task queue.
 *
 * Calling/Exit State:
 *	A non-NULL pointer to an event listener session is returned on success.
 *	NULL is returned if the event task thread should be terminated.
 *
 * Remarks:
 *	This function must be called with holding the task queue lock.
 */
static ipc_elsess_t *
ipc_evtaskq_dequeue(ipc_evtaskq_t *PFC_RESTRICT tqp,
		    ctimespec_t *PFC_RESTRICT timeout)
{
	pfc_list_t	*elem, *queue = &tqp->etq_queue;
	int		wait_err = 0;

	/*
	 * Try to dequeue one task.
	 * Note that all tasks on the task queue should be executed even if
	 * the shutdown flag is set.
	 */
	while ((elem = pfc_list_pop(queue)) == NULL) {
		if (IPC_EVTASKQ_IS_SHUTDOWN(tqp)) {
			return NULL;
		}

		if (wait_err != 0) {
			PFC_ASSERT(wait_err == ETIMEDOUT);
			return NULL;
		}

		/* Wait for a new task. */
		tqp->etq_nwaiting++;
		wait_err = IPC_EVTASKQ_TIMEDWAIT(tqp, timeout);
		tqp->etq_nwaiting--;
	}

	pfc_list_init(elem);

	return IPC_ELSESS_TQLIST2PTR(elem);
}

/*
 * static void
 * ipc_evtaskq_post(ipc_elsess_t *elsp)
 *	Post an event listener session task to the task queue.
 *
 * Remarks:
 *	This function must be called with holding the event listener session
 *	lock.
 */
static void
ipc_evtaskq_post(ipc_elsess_t *elsp)
{
	ipc_evtaskq_t	*tqp = &ipc_event_taskq;

	IPC_EVTASKQ_LOCK(tqp);

	if (!IPC_EVTASKQ_IS_SHUTDOWN(tqp) && !IPC_ELSESS_ON_TASKQ(elsp)) {
		/* Increment reference from the task queue to this session. */
		IPC_ELSESS_HOLD(elsp);

		/* Enqueue the task. */
		pfc_list_push_tail(&tqp->etq_queue, &elsp->els_tqlist);

		/* Start task queue thread. */
		ipc_evtaskq_start(tqp);
	}

	IPC_EVTASKQ_UNLOCK(tqp);
}

/*
 * static void
 * ipc_evtaskq_post_update(ipc_elsess_t *elsp)
 *	Post an event listener session task to the task queue if handler
 *	configuration was changed.
 */
static void
ipc_evtaskq_post_update(ipc_elsess_t *elsp)
{
	IPC_ELSESS_LOCK(elsp);

	if (elsp->els_flags & IPC_ELSF_UPDATE) {
		elsp->els_flags &= ~IPC_ELSF_UPDATE;
		ipc_evtaskq_post(elsp);
	}

	IPC_ELSESS_UNLOCK(elsp);
}

/*
 * static void
 * ipc_evtaskq_start(ipc_evtaskq_t *tqp)
 *	Start task queue processing.
 *
 * Remarks:
 *	This function must be called with holding the task queue lock.
 *	Note that this function may release the lock for a while.
 */
static void
ipc_evtaskq_start(ipc_evtaskq_t *tqp)
{
	int	err;

	/*
	 * If at least one free task thread exists, or no more task thread
	 * is available, send a signal to one task thread.
	 */
	if (tqp->etq_nwaiting != 0 ||
	    tqp->etq_nthreads >= tqp->etq_maxthreads) {
		IPC_EVTASKQ_SIGNAL(tqp);

		return;
	}

	/* Create a new task thread. */
	err = ipc_event_opts.evopt_thread_create(ipc_evtaskq_main, tqp);
	if (PFC_EXPECT_TRUE(err == 0)) {
		tqp->etq_nthreads++;

		return;
	}

	IPCCLNT_LOG_ERROR("Failed to create task thread: %s", strerror(err));

	/* Try again after IPC_EVTASKQ_FALLBACK_TIMEOUT milliseconds. */
	(void)ipc_evctx_timeout(&tqp->etq_timeout, NULL,
				IPC_EVTASKQ_FALLBACK_TIMEOUT);
}

/*
 * static void
 * ipc_evtaskq_post_fail(ipc_etimeout_t *etmp)
 *	Timeout handler for the event listener session task queue.
 *
 *	This function tries to add one thread to the task queue if needed.
 */
static void
ipc_evtaskq_post_fail(ipc_etimeout_t *etmp)
{
	ipc_evtaskq_t	*tqp = IPC_EVTASKQ_ETM2PTR(etmp);

	PFC_ASSERT(tqp == &ipc_event_taskq);

	IPC_EVTASKQ_LOCK(tqp);
	if (tqp->etq_nthreads == 0 && !pfc_list_is_empty(&tqp->etq_queue)) {
		/* Try to start task queue processing. */
		ipc_evtaskq_start(tqp);
	}
	IPC_EVTASKQ_UNLOCK(tqp);
}

/*
 * static void *
 * ipc_evctx_main(void *arg)
 *	Start routine of IPC event listener thread.
 */
static void *
ipc_evctx_main(void *arg)
{
	ipc_evctx_t	*ectx = (ipc_evctx_t *)arg;
	epoll_event_t	*events = ectx->ev_epevents;
	pfc_rbtree_t	tree;
	pfc_list_t	epsess, *elem, *next;
	int		epfd = ectx->ev_epoll, timeout = -1;
	uint32_t	nevents = ipc_event_opts.evopt_maxthreads;

	PFC_ASSERT(ectx == &ipc_event_ctx);

	IPCCLNT_LOG_DEBUG("Event listener thread has been started.");

	for (;;) {
		int	nfds, err;

		/* Wait for evens. */
		nfds = epoll_wait(epfd, events, nevents, timeout);
		if (PFC_EXPECT_FALSE(nfds < 0)) {
			err = errno;
			if (PFC_EXPECT_FALSE(err != EINTR)) {
				struct timespec	ts;

				/* This should never happen. */
				IPCCLNT_LOG_ERROR("epoll_wait(2) failed: %s",
						  strerror(err));

				/* Insert short delay. */
				ts.tv_sec = 1;
				ts.tv_nsec = 0;
				nanosleep(&ts, NULL);
			}
		}
		else {
			/* Dispatch events to epoll event handlers. */
			err = pfc_epoll_dispatch(events, nfds, ectx);
			if (PFC_EXPECT_FALSE(err != 0)) {
				break;
			}
		}

		/* Run expired timeout handlers. */
		timeout = ipc_evctx_run_timeout(ectx);
	}

	IPC_EVCTX_LOCK(ectx);

	ectx->ev_flags |= IPC_EVCTXF_DONE;

	/* Clear timeouts. */
	tree = ectx->ev_timeout;
	pfc_rbtree_init(&ectx->ev_timeout, pfc_rbtree_uint64_compare,
			ipc_etimeout_getkey);

	/* Move all epoll'ed listener sessions to local list. */
	pfc_list_move_all(&ectx->ev_epsess, &epsess);
	pfc_list_init(&ectx->ev_epsess);

	IPC_EVCTX_UNLOCK(ectx);

	/* Cancel all timeouts. */
	pfc_rbtree_clear(&tree, ipc_etimeout_dtor, NULL);

	/* Cancel all event listener sessions. */
	IPC_CLIENT_RDLOCK();
	pfc_ipcclnt_chan_iterate(ipc_evchan_shutdown, NULL);
	IPC_CLIENT_UNLOCK();

	/*
	 * Wake up all threads waiting for the socket to be removed from
	 * the epoll instance.
	 */
	PFC_LIST_FOREACH_SAFE(&epsess, elem, next) {
		ipc_elsess_t	*elsp = IPC_ELSESS_EPLIST2PTR(elem);

		IPC_ELSESS_LOCK(elsp);
		PFC_ASSERT(elsp->els_flags & IPC_ELSF_EPOLL);
		elsp->els_flags &= ~IPC_ELSF_EPOLL;
		IPC_ELSESS_SIGNAL(elsp);
#ifdef	PFC_VERBOSE_DEBUG
		pfc_list_remove(elem);
		pfc_list_init(elem);
#endif	/* PFC_VERBOSE_DEBUG */
		IPC_ELSESS_RELEASE(elsp);
	}

	IPCCLNT_LOG_DEBUG("Event listener thread has been terminated.");

	return NULL;
}

/*
 * static int
 * ipc_evctx_epoll(pfc_ephdlr_t *ephp, uint32_t events, void *arg)
 *	Handle an epoll event on the event context control pipe.
 *
 * Calling/Exit State:
 *	ECANCELED is returned if the control pipe is closed.
 *	Otherwise zero is returned.
 */
static int
ipc_evctx_epoll(pfc_ephdlr_t *ephp, uint32_t events, void *arg)
{
	ipc_evctx_t	*ectx = (ipc_evctx_t *)arg;
	int		err;

	PFC_ASSERT(ectx == &ipc_event_ctx);
	PFC_ASSERT(ectx == PFC_CAST_CONTAINER(ephp, ipc_evctx_t, ev_ephdlr));

	if (PFC_EXPECT_FALSE(events & EPOLLHUP)) {
		return ECANCELED;
	}

	/* Make the control pipe empty. */
	err = ipc_evctx_signal_clear(ectx);
	if (PFC_EXPECT_FALSE(err != 0 && !PFC_IS_EWOULDBLOCK(err))) {
		if (PFC_EXPECT_FALSE(err != ECANCELED)) {
			/* This is unrecoverable error. */
			IPCCLNT_LOG_FATAL("Unable to read event context "
					  "control pipe: %s",
					  strerror(err));
		}

		return err;
	}

	return 0;
}

/*
 * static void
 * ipc_evctx_signal_raise(ipc_evctx_t *ectx)
 *	Raise the event listener context signal.
 *
 * Remarks:
 *	This function must be called with holding the event listener context
 *	lock.
 */
static void
ipc_evctx_signal_raise(ipc_evctx_t *ectx)
{
	ssize_t	sz;
	char	c;
	int	fd;

	fd = IPC_EVCTX_WRPIPE(ectx);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		return;
	}

	c = 0;
	sz = write(fd, &c, sizeof(c));
	if (PFC_EXPECT_FALSE(sz == (ssize_t)-1)) {
		int	err = errno;

		if (!PFC_IS_EWOULDBLOCK(err)) {
			/* This should never happen. */
			IPCCLNT_LOG_ERROR("Unable to write event context "
					  "control pipe: %s", strerror(err));
		}
	}
}

/*
 * static int
 * ipc_evctx_signal_clear(ipc_evctx_t *ectx)
 *	Clear the event listener context signal.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ECANCELED is returned if EOF is detected.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_evctx_signal_clear(ipc_evctx_t *ectx)
{
	ssize_t	sz;
	char	buf[16];
	int	err, fd = IPC_EVCTX_RDPIPE(ectx);

	for (;;) {
		sz = read(fd, buf, sizeof(buf));
		if (sz > 0) {
			continue;
		}
		if (sz == 0) {
			return ECANCELED;
		}

		PFC_ASSERT(sz == (ssize_t)-1);
		if ((err = errno) != EINTR) {
			break;
		}
	}

	return err;
}

/*
 * static int
 * ipc_evctx_timeout_l(ipc_evctx_t *PFC_RESTRICT ectx,
 *		       ipc_etimeout_t *PFC_RESTRICT etmp, ipc_etmfunc_t func,
 *		       uint64_t msec)
 *	Register timeout handler to the event listener context.
 *	The timeout handler will be invoked after `msec' milliseconds.
 *
 *	`func' is a pointer to timeout handler. If NULL is specified,
 *	the handler previously set in `etmp' is used.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the event listener context
 *	lock.
 */
static int
ipc_evctx_timeout_l(ipc_evctx_t *PFC_RESTRICT ectx,
		    ipc_etimeout_t *PFC_RESTRICT etmp, ipc_etmfunc_t func,
		    uint64_t msec)
{
	uint64_t	cur;
	int		err;

	if (PFC_EXPECT_FALSE(IPC_EVCTX_IS_DONE(ectx))) {
		return ECANCELED;
	}

	/* Determine current time in milliseconds. */
	cur = 0;
	err = ipc_evctx_gettime(&cur);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		return err;
	}

	if (PFC_EXPECT_FALSE(IPC_ETIMEOUT_IS_ACTIVE(etmp))) {
		return EEXIST;
	}

	if (func != NULL) {
		/* Update the timeout handler. */
		etmp->etmo_func = func;
	}
	else {
		PFC_ASSERT(etmp->etmo_func != NULL);
	}

	/* Register the timeout node to the listener context. */
	etmp->etmo_time = cur + msec;
	while (pfc_rbtree_put(&ectx->ev_timeout, &etmp->etmo_node) != 0) {
		etmp->etmo_time++;
	}

	/*
	 * Send signal to the listener thread in order to let it reconfigure
	 * timeout.
	 */
	ipc_evctx_signal_raise(ectx);

	return 0;
}

/*
 * static void
 * ipc_evctx_untimeout(ipc_etimeout_t *etmp)
 *	Unregister timeout handler from the event listener context.
 *
 * Remarks:
 *	- Unlike ipc_evctx_timeout(), this function never send a signal to the
 *	  listener thread. Although the listener thread may be woken up by
 *	  phantom timeout, it never causes any problem.
 *
 *	- The caller must call this function with holding the event listener
 *	  session lock if the session timeout field (els_timeout) is specified
 *	  to `etmp'. In that case, the reference counter of the listener
 *	  session is always decremented. Note that this function never calls
 *	  ipc_elsess_destroy() even if all references to the listener session
 *	  was unlinked.
 */
static void
ipc_evctx_untimeout(ipc_etimeout_t *etmp)
{
	ipc_evctx_t	*ectx = &ipc_event_ctx;

	IPC_EVCTX_LOCK(ectx);

	if (IPC_ETIMEOUT_IS_ACTIVE(etmp) && !IPC_EVCTX_IS_DONE(ectx)) {
		pfc_rbtree_remove_node(&ectx->ev_timeout, &etmp->etmo_node);
		IPC_ETIMEOUT_INVALIDATE(etmp);

		if (etmp->etmo_elsess) {
			ipc_elsess_t	*elsp = IPC_ELSESS_ETM2PTR(etmp);

			/* Release the listener session. */
			IPC_ELSESS_RELEASE_RETAIN(elsp);
		}
	}

	IPC_EVCTX_UNLOCK(ectx);
}

/*
 * static int
 * ipc_evctx_run_timeout(ipc_evctx_t *ectx)
 *	Run all expired timeout handlers.
 *
 * Calling/Exit State:
 *	A timeout value, in milliseconds, to be passed to epoll_wait(2)
 *	on the event listener thread is returned.
 */
static int
ipc_evctx_run_timeout(ipc_evctx_t *ectx)
{
	pfc_rbtree_t	*tree = &ectx->ev_timeout;
	pfc_rbnode_t	*node;
	uint64_t	cur = 0;
	int		err, ret = -1;

	/* Determine current time. */
	err = ipc_evctx_gettime(&cur);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		return IPC_EVCTX_FALLBACK_TIMEOUT;
	}

	IPC_EVCTX_LOCK(ectx);

	/* Search for a timeout node which has the lowest value. */
	while ((node = pfc_rbtree_next(tree, NULL)) != NULL) {
		ipc_etimeout_t	*etmp = IPC_ETIMEOUT_NODE2PTR(node);

		if (etmp->etmo_time > cur) {
			uint64_t	diff;

			/*
			 * Determine the timeout value to be passed to
			 * epoll_wait(2) call.
			 */
			diff = etmp->etmo_time - cur;
			if (PFC_EXPECT_TRUE(diff < (uint64_t)INT32_MAX)) {
				ret = (int)diff;
			}
			else {
				/* This should never happen. */
				ret = IPC_EVCTX_FALLBACK_TIMEOUT;
			}
			break;
		}

		/* Unlink this node from the timeout tree. */
		pfc_rbtree_remove_node(tree, node);
		IPC_ETIMEOUT_INVALIDATE(etmp);

		/* Invoke timeout handler. */
		IPC_EVCTX_UNLOCK(ectx);
		etmp->etmo_func(etmp);
		IPC_EVCTX_LOCK(ectx);
	}

	IPC_EVCTX_UNLOCK(ectx);

	return ret;
}

/*
 * static void
 * ipc_etimeout_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destructor of timeout node.
 *
 * Remarks:
 *	This function must be called without holding the event listener
 *	context lock.
 */
static void
ipc_etimeout_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	ipc_etimeout_t	*etmp = IPC_ETIMEOUT_NODE2PTR(node);

	IPC_ETIMEOUT_INVALIDATE(etmp);

	if (etmp->etmo_elsess) {
		ipc_elsess_t	*elsp = IPC_ELSESS_ETM2PTR(etmp);

		/* Release the listener session. */
		IPC_ELSESS_LOCK(elsp);
		IPC_ELSESS_RELEASE(elsp);
	}
}

/*
 * static void
 * ipc_evchan_dtor(ipc_clchan_t *chp, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	Destructor of event listener on the specified IPC channel.
 *
 * Remarks:
 *	- This function must be called with holding the client lock in writer
 *	  mode.
 *
 *	- This function is used to free up any IPC event resources associated
 *	  with the specified IPC channel.
 */
static void
ipc_evchan_dtor(ipc_clchan_t *chp, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	IPC_CLCHAN_WRLOCK(chp);

	/* Destroy all links to host set. */
	pfc_rbtree_clear(&chp->ichc_evhostset, ipc_chref_dtor, NULL);

	/* Initialize event handler list. */
	pfc_list_init(&chp->ichc_evhandler);

	/* Destroy all listener sessions. */
	pfc_rbtree_clear(&chp->ichc_elsess, ipc_elsess_dtor, NULL);

	IPC_CLCHAN_UNLOCK(chp);
}

/*
 * static void
 * ipc_evchan_shutdown(ipc_clchan_t *chp, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	Shut down all event listener sessions on the specified IPC channel.
 *
 * Remarks:
 *	This function must be called with holding the client lock in reader
 *	or writer mode.
 */
static void
ipc_evchan_shutdown(ipc_clchan_t *chp, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	pfc_rbtree_t	*tree = &chp->ichc_elsess;
	pfc_rbnode_t	*node;

	IPC_CLCHAN_RDLOCK(chp);

	node = NULL;
	while ((node = pfc_rbtree_next(tree, node)) != NULL) {
		ipc_elsess_t	*elsp = IPC_ELSESS_NODE2PTR(node);

		/* Shut down the session. */
		IPC_ELSESS_LOCK(elsp);
		ipc_elsess_shutdown(elsp, PFC_FALSE);
		PFC_ASSERT(elsp->els_refcnt > 0);
		IPC_ELSESS_UNLOCK(elsp);
	}

	IPC_CLCHAN_UNLOCK(chp);
}

/*
 * static void
 * ipc_evchan_fork_prepare(ipc_clchan_t *chp, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	fork(2) prepare handler for the specified IPC channel.
 */
static void
ipc_evchan_fork_prepare(ipc_clchan_t *chp, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	IPC_CLCHAN_WRLOCK(chp);
}

/*
 * static void
 * ipc_evchan_fork_parent(ipc_clchan_t *chp, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	fork(2) parent handler for the specified IPC channel.
 *
 * Remarks:
 *	This function must be called with holding the IPC channel lock in
 *	writer mode.
 */
static void
ipc_evchan_fork_parent(ipc_clchan_t *chp, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	IPC_CLCHAN_UNLOCK(chp);
}

/*
 * static void
 * ipc_evchan_fork_child(ipc_clchan_t *chp, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	fork(2) child handler for the specified IPC channel.
 */
static void
ipc_evchan_fork_child(ipc_clchan_t *chp, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	PFC_ASSERT_INT(pfc_rwlock_init(&chp->ichc_lock), 0);
}

/*
 * static int
 * ipc_chref_create(ipc_clchan_t *PFC_RESTRICT chp,
 *		    ipc_hostset_t *PFC_RESTRICT hset)
 *	Create linkage between the IPC channel and the IPC host set.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode, and the channel lock in writer mode.
 */
static int
ipc_chref_create(ipc_clchan_t *PFC_RESTRICT chp,
		 ipc_hostset_t *PFC_RESTRICT hset)
{
	pfc_rbtree_t	*tree = &chp->ichc_evhostset;
	pfc_rbnode_t	*node;
	ipc_chref_t	*crp;

	node = pfc_rbtree_get(tree, (pfc_cptr_t)hset);
	if (node != NULL) {
		/* Increment reference counter of this link. */
		crp = IPC_CHREF_HOSTSET_NODE2PTR(node);
		PFC_ASSERT(crp->cr_refcnt != 0);
		PFC_ASSERT(crp->cr_chan == chp);
		crp->cr_refcnt++;

		return 0;
	}

	/* Create a new linkage. */
	crp = (ipc_chref_t *)malloc(sizeof(*crp));
	if (PFC_EXPECT_FALSE(crp == NULL)) {
		IPCCLNT_LOG_ERROR("Failed to create IPC channel linkage: "
				  "%s,%s",
				  IPC_CLCHAN_NAME(chp),
				  IPC_HOSTSET_NAME(hset));

		return ENOMEM;
	}

	IPC_CLCHAN_HOLD(chp);
	crp->cr_chan = chp;
	crp->cr_hostset = hset;
	crp->cr_refcnt = 1;
	PFC_ASSERT_INT(pfc_rbtree_put(tree, &crp->cr_hsnode), 0);
	PFC_ASSERT_INT(pfc_rbtree_put(&hset->hs_chref, &crp->cr_chnode), 0);
	IPC_HOSTSET_HOLD(hset);

	return 0;
}

/*
 * static void
 * ipc_chref_destroy(ipc_clchan_t *PFC_RESTRICT chp,
 *		     ipc_hostset_t *PFC_RESTRICT hset)
 *	Destroy the linkage between the IPC channel and the IPC host set.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode, and the channel lock in writer mode.
 */
static void
ipc_chref_destroy(ipc_clchan_t *PFC_RESTRICT chp,
		  ipc_hostset_t *PFC_RESTRICT hset)
{
	pfc_rbtree_t	*tree = &chp->ichc_evhostset;
	pfc_rbnode_t	*node;
	uint32_t	refcnt;
	ipc_chref_t	*crp;

	PFC_ASSERT(chp->ichc_refcnt > 1);

	node = pfc_rbtree_get(tree, (pfc_cptr_t)hset);
	PFC_ASSERT(node != NULL);

	crp = IPC_CHREF_HOSTSET_NODE2PTR(node);
	refcnt = crp->cr_refcnt;
	PFC_ASSERT(refcnt != 0);
	PFC_ASSERT(crp->cr_chan == chp);

	if (refcnt == 1) {
		/* This is the last link. */
		pfc_rbtree_remove_node(tree, node);
		pfc_rbtree_remove_node(&hset->hs_chref, &crp->cr_chnode);
		IPC_HOSTSET_RELEASE(hset);
		IPC_CLCHAN_RELEASE(chp);
		free(crp);
	}
	else {
		crp->cr_refcnt = refcnt - 1;
	}
}

/*
 * static void
 * ipc_chref_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destructor of Red-Black tree node associated with ipc_chref_t.
 */
static void
ipc_chref_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	ipc_chref_t	*crp = IPC_CHREF_HOSTSET_NODE2PTR(node);
	ipc_clchan_t	*chp = crp->cr_chan;
	ipc_hostset_t	*hset = crp->cr_hostset;

	pfc_rbtree_remove_node(&hset->hs_chref, &crp->cr_chnode);
	IPC_HOSTSET_RELEASE(hset);
	IPC_CLCHAN_RELEASE(chp);
	free(crp);
}

/*
 * static int
 * ipc_evhdlr_idalloc(ipc_evhdlr_t *ehp)
 *	Allocate a new IPC event handler ID for the specified handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new ID is set to ehp->evh_id, and zero
 *	is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
static int
ipc_evhdlr_idalloc(ipc_evhdlr_t *ehp)
{
	pfc_rbtree_t	*tree = &ipc_event_handlers;
	pfc_ipcevhdlr_t	start, newid;
	int		err = 0;

	start = newid = ipc_evhdlr_id_next;
	PFC_ASSERT(newid != PFC_IPCEVHDLR_INVALID);

	for (;;) {
		pfc_ipcevhdlr_t	id;

		ehp->evh_id = newid;
		err = pfc_rbtree_put(tree, &ehp->evh_node);

		/* Prepare ID for next allocation. */
		id = newid + 1;
		if (PFC_EXPECT_FALSE(id == PFC_IPCEVHDLR_INVALID)) {
			id++;
		}
		if (PFC_EXPECT_TRUE(err == 0)) {
			ipc_evhdlr_id_next = id;
			break;
		}

		/* Try next ID. */
		newid = id;
		if (PFC_EXPECT_FALSE(newid == start)) {
			/* No ID is available. */
			err = ENFILE;
			break;
		}
	}

	return err;
}

/*
 * static void
 * ipc_evhdlr_add(ipc_clchan_t *PFC_RESTRICT chp,
 *		  ipc_evhdlr_t *PFC_RESTRICT ehp)
 *	Bind the specified event handler to the IPC channel specified by `chp'.
 *
 * Remarks:
 *	This function must be called with holding the IPC channel lock in
 *	writer mode.
 */
static void
ipc_evhdlr_add(ipc_clchan_t *PFC_RESTRICT chp, ipc_evhdlr_t *PFC_RESTRICT ehp)
{
	pfc_list_t	*elem, *link = &ehp->evh_list;
	pfc_list_t	*head = &chp->ichc_evhandler;
	uint32_t	pri = ehp->evh_priority;
	ipc_evhdlr_t	*h;

	if (pfc_list_is_empty(head)) {
		pfc_list_push(head, link);

		return;
	}

	/* Event handlers must be sorted in ascending priority order. */
	h = IPC_EVHDLR_LIST2PTR(head->pl_prev);
	if (h->evh_priority <= pri) {
		/* A new handler has the greatest priority. */
		pfc_list_push_tail(head, link);

		return;
	}

#ifdef	PFC_VERBOSE_DEBUG
	pfc_list_init(link);
#endif	/* PFC_VERBOSE_DEBUG */

	PFC_LIST_FOREACH(head, elem) {
		h = IPC_EVHDLR_LIST2PTR(elem);
		if (h->evh_priority >= pri) {
			/* Link the handler just before this element. */
			pfc_list_push_tail(elem, link);
			break;
		}
	}

	/* The handler must be linked by above loop. */
	PFC_ASSERT(!pfc_list_is_empty(link));
}

/*
 * static void
 * ipc_evhdlr_destroy(ipc_evhdlr_t *ehp)
 *	Destroy the specified event handler instance.
 *
 * Remarks:
 *	This function must be called with holding the event handler lock.
 */
static void
ipc_evhdlr_destroy(ipc_evhdlr_t *ehp)
{
	ipc_clchan_t	*chp = ehp->evh_chan;
	pfc_refptr_t	*rname = ehp->evh_name;

	PFC_ASSERT(IPC_EVHDLR_IS_REMOVED(ehp));
	PFC_ASSERT(ehp->evh_refcnt == 0);
	IPC_EVHDLR_UNLOCK(ehp);

#ifdef	PFC_VERBOSE_DEBUG
	PFC_ASSERT_INT(pfc_cond_destroy(&ehp->evh_cond), 0);
	PFC_ASSERT_INT(pfc_mutex_destroy(&ehp->evh_mutex), 0);
#endif	/* PFC_VERBOSE_DEBUG */

	if (rname != NULL) {
		pfc_refptr_put(rname);
	}

	IPC_HOSTSET_RELEASE(ehp->evh_hostset);
	IPC_CLCHAN_RELEASE(chp);

	/* Call argument destructor. */
	if (ehp->evh_argdtor != NULL) {
		ehp->evh_argdtor(ehp->evh_arg);
	}

	pfc_ipc_evset_destroy(&ehp->evh_target);
	free(ehp);
}

/*
 * static int
 * ipc_evhdlr_wait(ipc_evhdlr_t *ehp)
 *	Wait for completion of the call of the specified event handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the event handler lock.
 *	Note that it may be released for a while.
 */
static int
ipc_evhdlr_wait(ipc_evhdlr_t *ehp)
{
	pfc_timespec_t	abstime;
	int		err;

	PFC_ASSERT(ehp->evh_refcnt != 0);
	PFC_ASSERT(IPC_EVHDLR_IS_REMOVED(ehp));

	if (PFC_EXPECT_TRUE(ehp->evh_refcnt == 1)) {
		return 0;
	}

	err = ipc_event_gettime(&abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		return err;
	}

	abstime.tv_sec += IPC_EVENT_HANDLER_WAIT_TIMEOUT;
	do {
		err = IPC_EVHDLR_TIMEDWAIT_ABS(ehp, &abstime);
		if (PFC_EXPECT_TRUE(ehp->evh_refcnt == 1)) {
			return 0;
		}
		PFC_ASSERT(ehp->evh_refcnt != 0);
	} while (err == 0);

	PFC_ASSERT(err == ETIMEDOUT);
	IPCCLNT_LOG_ERROR("%s: Event handler did not complete within %u "
			  "seconds.", ipc_evhdlr_name(ehp),
			  IPC_EVENT_HANDLER_WAIT_TIMEOUT);

	return err;
}

/*
 * static void
 * ipc_evhdlr_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destructor of Red-Black tree node associated with ipc_evhdlr_t.
 *
 *	This function is used to clean up all event handlers on finalization.
 */
static void
ipc_evhdlr_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	ipc_evhdlr_t	*ehp = IPC_EVHDLR_NODE2PTR(node);

	IPC_EVHDLR_LOCK(ehp);
	PFC_ASSERT(ehp->evh_refcnt == 1);

	IPCCLNT_LOG_DEBUG("Clean up IPC event handler: id=%u, name=%s",
			  ehp->evh_id, ipc_evhdlr_name(ehp));

	ehp->evh_flags |= IPC_EVHDF_REMOVED;
	IPC_EVHDLR_RELEASE_L(ehp);
}

/*
 * static int
 * ipc_elsess_setup(ipc_clchan_t *PFC_RESTRICT chp,
 *		    ipc_evhdlr_t *PFC_RESTRICT ehp,
 *		    const pfc_hostaddr_t *PFC_RESTRICT addr,
 *		    ipc_elsess_t **PFC_RESTRICT elspp)
 *	Set up an event listener session for the IPC server specified by
 *	`chp' and `hep'.
 *
 * Calling/Exit State:
 *	Upon successful completion, an event listener session instance for
 *	the IPC server specified by `chp' and `hep' is set to the buffer
 *	pointed by `elspp', and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode, and the channel lock in writer mode.
 */
static int
ipc_elsess_setup(ipc_clchan_t *PFC_RESTRICT chp,
		 ipc_evhdlr_t *PFC_RESTRICT ehp,
		 const pfc_hostaddr_t *PFC_RESTRICT haddr,
		 ipc_elsess_t **PFC_RESTRICT elspp)
{
	pfc_rbtree_t	*tree = &chp->ichc_elsess;
	pfc_rbnode_t	*node;
	ipc_elsess_t	*elsp;
	int		err;

	/*
	 * Check to see whether an event session for the specified IPC server
	 * exists or not.
	 */
	node = pfc_rbtree_get(tree, (pfc_cptr_t)haddr);
	if (node != NULL) {
		/* Increment link counter. */
		elsp = IPC_ELSESS_NODE2PTR(node);
		err = ipc_elsess_link(elsp, ehp);
		if (PFC_EXPECT_TRUE(err == 0)) {
			*elspp = elsp;
		}

		return err;
	}

	/* Create a new event listener session. */
	elsp = (ipc_elsess_t *)malloc(sizeof(*elsp));
	if (PFC_EXPECT_FALSE(elsp == NULL)) {
		IPCCLNT_LOG_ERROR("Failed to allocate listener session.");

		return ENOMEM;
	}

	err = PFC_MUTEX_INIT(&elsp->els_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCCLNT_LOG_ERROR("Failed to initialize listener mutex: %s",
				  strerror(err));
		goto error;
	}

	err = pfc_cond_init(&elsp->els_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCCLNT_LOG_ERROR("Failed to initialize listener condvar: %s",
				  strerror(err));
		goto error;
	}

	/* Create socket address. */
	err = pfc_ipcclnt_sockaddr_init(&elsp->els_sockaddr, chp,
					&elsp->els_sess, haddr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Initialize target event set. */
	pfc_ipc_evset_init(&elsp->els_target);
	err = pfc_ipc_evset_copy(&elsp->els_newtarget, &ehp->evh_target);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("Failed to copy target event set to the "
				  "session: %s", strerror(err));
		goto error_sockaddr;
	}

	/* Create control pipe. */
	err = pfc_pipe_open(elsp->els_pipe, PFC_PIPE_CLOEXEC_NB);
	if (PFC_EXPECT_FALSE(err != 0)) {
		err = errno;
		IPCCLNT_LOG_ERROR("Failed to create listener control pipe: %s",
				  strerror(err));
		goto error_target;
	}

	IPC_CLCHAN_HOLD(chp);
	elsp->els_chan = chp;
	elsp->els_refcnt = 1;
	elsp->els_nlinks = 1;
	elsp->els_nwilds = (pfc_ipc_evset_isempty(&ehp->evh_target))
		? 1 : 0;
	elsp->els_flags = (IPC_ELSF_UPDATE | IPC_ELSF_CONNECTING);
	elsp->els_prevdown = IPC_CHDOWN_NONE;
	pfc_list_init(&elsp->els_tqlist);
	pfc_epoll_handler_init(&elsp->els_epoll, ipc_elsess_epoll);
	IPC_ETIMEOUT_INIT(&elsp->els_timeout, NULL, PFC_TRUE);

#ifdef	PFC_VERBOSE_DEBUG
	pfc_list_init(&elsp->els_eplist);
#endif	/* PFC_VERBOSE_DEBUG */

	PFC_ASSERT_INT(pfc_rbtree_put(tree, &elsp->els_node), 0);

	IPCCLNT_LOG_VERBOSE("%s: Event listener has been created: %p",
			    IPC_CLCHAN_NAME(elsp->els_chan), elsp);

	*elspp = elsp;

	return 0;

error_target:
	pfc_ipc_evset_destroy(&elsp->els_newtarget);

error_sockaddr:
	pfc_sockaddr_destroy(&elsp->els_sockaddr);

error:
	free(elsp);

	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}

/*
 * static int
 * ipc_elsess_link(ipc_elsess_t *PFC_RESTRICT elsp,
 *		   ipc_evhdlr_t *PFC_RESTRICT ehp)
 *	Link the event handler specified by `ehp' to the event listener
 *	session specified by `ehp'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode, and the channel lock in writer mode.
 */
static int
ipc_elsess_link(ipc_elsess_t *PFC_RESTRICT elsp,
		ipc_evhdlr_t *PFC_RESTRICT ehp)
{
	pfc_bool_t	empty = pfc_ipc_evset_isempty(&ehp->evh_target);
	int		err;

	IPC_ELSESS_LOCK(elsp);

	if (empty || elsp->els_nwilds != 0) {
		/*
		 * Don't set target event set because at least one event
		 * handler on this session requires all events.
		 */
		pfc_ipc_evset_destroy(&elsp->els_newtarget);
		if (empty) {
			elsp->els_nwilds++;
		}

		if (!pfc_ipc_evset_isempty(&elsp->els_target)) {
			/* Target event set needs to be reset. */
			elsp->els_flags |= IPC_ELSF_UPDATE;
		}

		/* Increment link counter. */
		PFC_ASSERT(elsp->els_nlinks > 0);
		elsp->els_nlinks++;

		err = 0;
	}
	else {
		/* Merge new target event set. */
		err = pfc_ipc_evset_mergenew(&elsp->els_newtarget,
					     &elsp->els_target,
					     &ehp->evh_target);
		if (PFC_EXPECT_TRUE(err == 0)) {
			/* Increment link counter. */
			PFC_ASSERT(elsp->els_nlinks > 0);
			elsp->els_nlinks++;
			if (!pfc_ipc_evset_isempty(&elsp->els_newtarget)) {
				elsp->els_flags |= IPC_ELSF_UPDATE;
			}
		}
		else {
			/*
			 * Remarks:
			 *	Target event set may have been copied to the
			 *	listener session partially. But it's harmless
			 *	because bogus target event will be removed
			 *	on the fly.
			 */
			IPCCLNT_LOG_ERROR("Failed to merge target event set "
					  "to the session: %s", strerror(err));
		}
	}

	IPC_ELSESS_UNLOCK(elsp);

	return err;
}

/*
 * static void
 * ipc_elsess_unlink(ipc_clchan_t *PFC_RESTRICT chp,
 *		     ipc_evhdlr_t *PFC_RESTRICT ehp,
 *		     const pfc_hostaddr_t *PFC_RESTRICT haddr)
 *	Unlink the event handler from the event listener session associated
 *	with the IPC channel `chp' and the IPC server's	address `hep'.
 *
 *	`ehp' must points event handler instance to be removed.
 *
 *	If the last link to the session is removed, the session will be
 *	destroyed.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode, and the channel lock in writer mode.
 */
static void
ipc_elsess_unlink(ipc_clchan_t *PFC_RESTRICT chp,
		  ipc_evhdlr_t *PFC_RESTRICT ehp,
		  const pfc_hostaddr_t *PFC_RESTRICT haddr)
{
	pfc_rbnode_t	*node;
	ipc_elsess_t	*elsp;
	uint32_t	nwilds;

	node = pfc_rbtree_get(&chp->ichc_elsess, (pfc_cptr_t)haddr);
	PFC_ASSERT(node != NULL);

	nwilds = (pfc_ipc_evset_isempty(&ehp->evh_target)) ? 1 : 0;

	/* Decrement link counter of the session. */
	elsp = IPC_ELSESS_NODE2PTR(node);
	IPC_ELSESS_LOCK(elsp);
	ipc_elsess_unlink_impl(chp, elsp, 1, nwilds);
}

/*
 * static void
 * ipc_elsess_unlink_impl(ipc_clchan_t *PFC_RESTRICT chp,
 *			  ipc_elsess_t *PFC_RESTRICT elsp, uint32_t nlinks,
 *			  uint32_t nwilds)
 *	Decrement link counter of the specified session.
 *
 *	`nlinks' must be the number of link counter to subtract.
 *	`nwilds' must be the number of wildcard handlers to subtract.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode, and the channel lock in writer mode, and the listener session
 *	lock. Note that the listener session lock is always released on return.
 */
static void
ipc_elsess_unlink_impl(ipc_clchan_t *PFC_RESTRICT chp,
		       ipc_elsess_t *PFC_RESTRICT elsp, uint32_t nlinks,
		       uint32_t nwilds)
{
	pfc_bool_t	do_release = PFC_FALSE;

	/* Decrement counters. */
	PFC_ASSERT(elsp->els_nlinks >= nlinks);
	PFC_ASSERT(elsp->els_nwilds >= nwilds);
	elsp->els_nlinks -= nlinks;
	elsp->els_nwilds -= nwilds;

	if (elsp->els_nlinks == 0) {
		/* Unlink this session from the IPC channel. */
		pfc_rbtree_remove_node(&chp->ichc_elsess, &elsp->els_node);

		/* Shut down the session. */
		ipc_elsess_shutdown(elsp, PFC_TRUE);

		/* Release this session. */
		do_release = PFC_TRUE;
	}

	IPC_ELSESS_RELEASE_COND(elsp, do_release);
}

/*
 * static void
 * ipc_elsess_destroy(ipc_elsess_t *elsp)
 *	Destroy the specified event listener session.
 *
 * Remarks:
 *	- This function must be called with holding the event listener
 *	  session lock.
 *	- The specified session must be unlinked from the IPC channel in
 *	  advance.
 */
static void
ipc_elsess_destroy(ipc_elsess_t *elsp)
{
	ipc_clchan_t	*chp = elsp->els_chan;

	PFC_ASSERT(elsp->els_nlinks == 0);
	IPC_ELSESS_NOT_EPOLL_ASSERT(elsp, &ipc_event_ctx);

	if (elsp->els_stream != NULL) {
		PFC_ASSERT_INT(pfc_iostream_destroy(elsp->els_stream), 0);
	}
	ipc_event_close(&elsp->els_pipe[0]);
	ipc_event_close(&elsp->els_pipe[1]);

	pfc_ipc_evset_destroy(&elsp->els_target);
	pfc_ipc_evset_destroy(&elsp->els_newtarget);
	pfc_sockaddr_destroy(&elsp->els_sockaddr);

	IPCCLNT_LOG_VERBOSE("%s: Event listener has been destroyed: %p",
			    IPC_CLCHAN_NAME(elsp->els_chan), elsp);

	IPC_ELSESS_UNLOCK(elsp);

#ifdef	PFC_VERBOSE_DEBUG
	IPC_EVCTX_LOCK(&ipc_event_ctx);
	PFC_ASSERT(!IPC_ETIMEOUT_IS_ACTIVE(&elsp->els_timeout));
	IPC_EVCTX_UNLOCK(&ipc_event_ctx);

	PFC_ASSERT_INT(pfc_cond_destroy(&elsp->els_cond), 0);
	PFC_ASSERT_INT(pfc_mutex_destroy(&elsp->els_mutex), 0);
#endif	/* PFC_VERBOSE_DEBUG */

	free(elsp);

	IPC_CLCHAN_RELEASE(chp);
}

/*
 * static void
 * ipc_elsess_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destructor of the event listener session.
 *
 * Calling/Exit State:
 *	- This function must be called with holding the client lock in writer
 *	  mode.
 *
 *	- This function is used to destroy event listener session by force.
 */
static void
ipc_elsess_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	ipc_elsess_t	*elsp = IPC_ELSESS_NODE2PTR(node);

	IPC_ELSESS_LOCK(elsp);
	IPC_ELSESS_NOT_EPOLL_ASSERT(elsp, &ipc_event_ctx);
	PFC_ASSERT(elsp->els_refcnt == 1);
	elsp->els_nlinks = 0;
	elsp->els_flags = 0;
	IPC_ELSESS_RELEASE(elsp);
}

/*
 * static void
 * ipc_elsess_setdowncode(ipc_elsess_t *elsp, int err)
 *	Set the connection down code to the specified event listener session.
 *
 *	`err' is an error number which indicates the cause of error.
 *
 * Remarks:
 *	This function must be called with holding the event listener session
 *	lock. Note that it may be released for a while.
 */
static void
ipc_elsess_setdowncode(ipc_elsess_t *elsp, int err)
{
	uint32_t	downcode;

	if (err == ECONNREFUSED) {
		downcode = PFC_IPCCHDOWN_REFUSED;
	}
	else if (err == ECONNRESET) {
		downcode = PFC_IPCCHDOWN_RESET;
	}
	else if (err == EPIPE) {
		downcode = PFC_IPCCHDOWN_HANGUP;
	}
	else if (err == ETIMEDOUT) {
		downcode = PFC_IPCCHDOWN_TIMEDOUT;
	}
	else {
		downcode = PFC_IPCCHDOWN_ERROR;
	}

	IPC_ELSESS_SET_DOWNCODE(elsp, downcode);
}

/*
 * static void
 * ipc_elsess_update(ipc_elsess_t *elsp)
 *	Update state of the event listener session.
 *
 * Remarks:
 *	The caller must increment references to the specified session in
 *	advance.
 */
static void
ipc_elsess_update(ipc_elsess_t *elsp)
{
	int	err;

	IPC_ELSESS_LOCK(elsp);
	PFC_ASSERT(elsp->els_refcnt > 0);

	if (PFC_EXPECT_FALSE(elsp->els_flags & IPC_ELSF_BUSY)) {
		/* This function is being called on another thread. */
		elsp->els_flags |= IPC_ELSF_REPOST;
		goto out;
	}

	elsp->els_flags |= IPC_ELSF_BUSY;

	if (IPC_ELSESS_IS_SHUTDOWN(elsp) ||
	    IPC_ELSESS_GET_DOWNCODE(elsp) != 0) {
		/* The session should be shut down by request. */
		goto error;
	}

	/* Establish a connection to the server. */
	err = ipc_elsess_connect(elsp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err < 0) {
			goto clear_busy;
		}
		goto error;
	}

	/* Send target event set to the server. */
	err = ipc_elsess_addtarget(elsp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Receive an IPC event from the server. */
	err = ipc_elsess_receive(elsp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	if (IPC_ELSESS_IS_SHUTDOWN(elsp)) {
		goto error;
	}

clear_busy:
	elsp->els_flags &= ~IPC_ELSF_BUSY;
	if (elsp->els_flags & IPC_ELSF_REPOST) {
		/* Need to repost session task. */
		elsp->els_flags &= ~IPC_ELSF_REPOST;
		ipc_evtaskq_post(elsp);
	}

out:
	IPC_ELSESS_RELEASE(elsp);

	return;

error:
	/* Disconnect the session. */
	ipc_elsess_disconnect(elsp);

	IPC_ELSESS_RELEASE(elsp);
}

/*
 * static int
 * ipc_elsess_connect(ipc_elsess_t *elsp)
 *	Establish event listener session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	-1 is returned if reconnection timeout is registered.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the event listener session
 *	  lock. Note that it may be released for a while.
 *
 *	- This function returns zero if the session is already established.
 *
 *	- The caller must ensure that the given session is not shut down.
 */
static int
ipc_elsess_connect(ipc_elsess_t *elsp)
{
	pfc_timespec_t	abstime;
	pfc_iostream_t	stream;
	pfc_sockaddr_t	*sap = &elsp->els_sockaddr;
	ipc_evctx_t	*ectx = &ipc_event_ctx;
	ipc_sess_t	*sess = &elsp->els_sess;
	ipc_clchan_t	*chp = elsp->els_chan;
	uint8_t		cmd = IPC_COMMAND_EVENT, response;
	uint32_t	prevdown, downcode = 0;
	pfc_bool_t	locked = PFC_TRUE;
	int		err, sock, canceller;

	PFC_ASSERT(!IPC_ELSESS_IS_SHUTDOWN(elsp));

	if (PFC_EXPECT_FALSE(elsp->els_stream != NULL)) {
		/* Already connected. */
		IPC_ELSESS_EPOLL_ASSERT(elsp, ectx);

		return 0;
	}

	PFC_ASSERT(IPC_ELSESS_GET_DOWNCODE(elsp) == 0);

	IPC_EVCTX_LOCK(ectx);
	if (IPC_ETIMEOUT_IS_ACTIVE(&elsp->els_timeout)) {
		/*
		 * Reconnection timer is already posted by the previous call
		 * of ipc_elsess_disconnect().
		 */
		PFC_ASSERT(elsp->els_timeout.etmo_func ==
			   ipc_elsess_reconnect);
		IPC_EVCTX_UNLOCK(ectx);

		return -1;
	}
	IPC_EVCTX_UNLOCK(ectx);

	/* Determine I/O timeout. */
	err = ipc_event_gettimeout(&abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error;
	}

	/* Create a new socket. */
	sock = pfc_sockaddr_open(sap, PFC_SOCK_CLOEXEC_NB);
	if (PFC_EXPECT_FALSE(sock == -1)) {
		err = errno;
		IPCCLNT_LOG_ERROR("%s: Failed to create an event socket: %s",
				  IPC_CLCHAN_NAME(chp), strerror(err));
		goto error;
	}

	/* Create a new session stream without read-ahead cache. */
	canceller = IPC_ELSESS_RDPIPE(elsp);
	err = pfc_ipc_iostream_create(sess, sock, canceller, &ipcclnt_option);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}
	stream = sess->iss_stream;
	prevdown = elsp->els_prevdown;
	elsp->els_flags |= IPC_ELSF_CONNECTING;

	IPC_ELSESS_UNLOCK(elsp);
	locked = PFC_FALSE;

	/* Establish connection between the client and the IPC server. */
	err = pfc_ipcclnt_connect(sess, chp, sap, canceller, &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (prevdown == IPC_CHDOWN_NONE && err != ECANCELED) {
			IPCCLNT_LOG_ERROR("%s: Failed to connect to the "
					  "server: %s",
					  IPC_CLCHAN_NAME(chp),
					  strerror(err));
		}

		if (err == EPERM) {
			downcode = PFC_IPCCHDOWN_AUTHFAIL;
		}
		else if (err == ENOSPC) {
			downcode = PFC_IPCCHDOWN_TOOMANY;
		}
		goto error_stream;
	}

	/* Send EVENT command. */
	err = pfc_ipc_write(stream, &cmd, sizeof(cmd), PFC_TRUE, &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to send EVENT command: %s",
				  IPC_CLCHAN_NAME(chp), strerror(err));
		goto error_stream;
	}

	/* Wait for response from the server. */
	err = pfc_ipc_read(stream, &response, sizeof(response), &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to receive EVENT response: %s",
				  IPC_CLCHAN_NAME(chp), strerror(err));
		goto error_stream;
	}

	IPC_ELSESS_LOCK(elsp);
	IPC_EVCTX_LOCK(ectx);
	locked = PFC_TRUE;

	if (PFC_EXPECT_FALSE(IPC_EVCTX_IS_DONE(ectx) ||
			     IPC_ELSESS_IS_SHUTDOWN(elsp))) {
		/*
		 * The event listener thread is already gone, or this session
		 * is shut down by another thread.
		 */
		IPC_EVCTX_UNLOCK(ectx);
		err = ECANCELED;
		goto error_stream;
	}

	/* Register the session socket to the epoll instance. */
	err = pfc_epoll_ctl(ectx->ev_epoll, EPOLL_CTL_ADD, sock,
			    EPOLLIN | IPC_EPOLLEV_ONESHOT, &elsp->els_epoll);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_EVCTX_UNLOCK(ectx);
		err = errno;
		IPCCLNT_LOG_ERROR("%s: Failed to add session to the epoll "
				  "instance: %s",
				  IPC_CLCHAN_NAME(chp), strerror(err));
		goto error_stream;
	}

	IPCCLNT_LOG_INFO("%s: Event listener session has been established.",
			 IPC_CLCHAN_NAME(chp));

	/* Need to send all target event set to the IPC server. */
	elsp->els_flags |= (IPC_ELSF_EPOLL | IPC_ELSF_TARGET_ALL);

	/*
	 * Listener session needs to be linked to the event listener context
	 * so that pfc_ipcclnt_event_fini() can wake up all threads waiting
	 * for IPC_ELSF_EPOLL flag to be cleared.
	 */
	PFC_ASSERT(pfc_list_is_empty(&elsp->els_eplist));
	pfc_list_push_tail(&ectx->ev_epsess, &elsp->els_eplist);

	/* Add reference from the epoll instance to the session. */
	IPC_ELSESS_HOLD(elsp);
	IPC_EVCTX_UNLOCK(ectx);

	/* Clear the previous connection down code. */
	elsp->els_prevdown = IPC_CHDOWN_NONE;

	/* Clear the connecting flag. */
	elsp->els_flags &= ~IPC_ELSF_CONNECTING;

	/* Post an IPC channel up event. */
	IPC_ELSESS_UNLOCK(elsp);
	ipc_elsess_post_upevent(elsp);
	IPC_ELSESS_LOCK(elsp);

	return 0;

error_stream:
	PFC_ASSERT_INT(pfc_iostream_destroy(stream), 0);

error:
	if (!locked) {
		IPC_ELSESS_LOCK(elsp);
	}

	if (downcode == 0) {
		ipc_elsess_setdowncode(elsp, err);
	}
	else {
		IPC_ELSESS_SET_DOWNCODE(elsp, downcode);
	}
	elsp->els_stream = NULL;
	elsp->els_flags &= ~IPC_ELSF_CONNECTING;

	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}

/*
 * static void
 * ipc_elsess_disconnect(ipc_elsess_t *elsp)
 *	Disconnect the event listener session.
 *
 * Remarks:
 *	- The caller must remove the session socket from the epoll instance
 *	  in advance.
 *
 *	- This function must be called with holding the event listener session
 *	  lock. Note that it may be released for a while.
 */
static void
ipc_elsess_disconnect(ipc_elsess_t *elsp)
{
	pfc_iostream_t	stream;
	uint32_t	downcode, repost;

	/* Unregister timeout for this session. */
	ipc_evctx_untimeout(&elsp->els_timeout);
	PFC_ASSERT(elsp->els_refcnt > 0);

	stream = elsp->els_stream;
	downcode = IPC_ELSESS_GET_DOWNCODE(elsp);
	if (stream != NULL) {
		/* Remove the session socket from the epoll instance. */
		ipc_elsess_epoll_delete(elsp);

		elsp->els_stream = NULL;
		PFC_ASSERT_INT(pfc_iostream_destroy(stream), 0);

		IPCCLNT_LOG_INFO("%s: Event listener session has been "
				 "disconnected: code=%u",
				 IPC_CLCHAN_NAME(elsp->els_chan), downcode);
	}

	if (!IPC_ELSESS_IS_SHUTDOWN(elsp)) {
		pfc_bool_t	need_event, locked = PFC_TRUE;

		PFC_ASSERT(downcode != IPC_CHDOWN_NONE);
		if (elsp->els_prevdown == IPC_CHDOWN_NONE) {
			elsp->els_prevdown = (uint8_t)downcode;
			need_event = PFC_TRUE;
		}
		else {
			need_event = PFC_FALSE;
		}

		if (ipc_auto_cancel) {
			/*
			 * Cancel ongoing IPC invocation request on client
			 * sessions which connects to the same IPC server as
			 * the listener session.
			 */
			IPC_ELSESS_UNLOCK(elsp);
			locked = PFC_FALSE;
			ipc_elsess_cancel_clients(elsp);
		}

		if (need_event) {
			/* Post an IPC channel down event. */
			if (locked) {
				IPC_ELSESS_UNLOCK(elsp);
				locked = PFC_FALSE;
			}
			ipc_elsess_post_downevent(elsp, downcode);
		}

		if (!locked) {
			IPC_ELSESS_LOCK(elsp);
		}

		/* Try to connect to the server later. */
		ipc_elsess_timeout(elsp, ipc_elsess_reconnect,
				   ipc_event_opts.evopt_conn_interval);
	}

	/* Reset session flags. */
	repost = elsp->els_flags & IPC_ELSF_REPOST;
	IPC_SESS_RESET_FLAGS(&elsp->els_sess);
	elsp->els_flags &= IPC_ELSF_RETAIN_FLAGS;

	if (repost) {
		/* Update the session state again. */
		ipc_evtaskq_post(elsp);
	}
}

/*
 * static void
 * ipc_elsess_post_upevent(ipc_elsess_t *elsp)
 *	Post an IPC channel state change event which indicates the IPC channel
 *	is up.
 *
 * Remarks:
 *	This function must be called without holding any lock.
 */
static void
ipc_elsess_post_upevent(ipc_elsess_t *elsp)
{
	pfc_ipcevent_t	*event;
	int		err;

	/* Create a channel up event. */
	err = pfc_ipcevent_create_chstate(&event, elsp, PFC_IPCCHSTATE_UP);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to create channel up event: %s",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));
	}
	else {
		ipc_clchan_t	*chp = elsp->els_chan;

		/* Post an event to the event dispatch queue. */
		IPC_CLCHAN_RDLOCK(chp);
		(void)ipc_evdisp_post(elsp, event);
		IPC_CLCHAN_UNLOCK(chp);
		IPC_EVENT_RELEASE(event);
	}
}

/*
 * static void
 * ipc_elsess_post_downevent(ipc_elsess_t *elsp, uint32_t downcode)
 *	Post an IPC channel state change event which indicates the IPC channel
 *	is down.
 *
 * Remarks:
 *	This function must be called without holding any lock.
 */
static void
ipc_elsess_post_downevent(ipc_elsess_t *elsp, uint32_t downcode)
{
	pfc_ipcevent_t	*event;
	pfc_ipcsess_t	*sess;
	ipc_clchan_t	*chp = elsp->els_chan;
	int		err;

	PFC_ASSERT(downcode != IPC_CHDOWN_NONE);

	/* Create a channel down event. */
	err = pfc_ipcevent_create_chstate(&event, elsp, PFC_IPCCHSTATE_DOWN);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to create channel down event"
				  "(code=%u): %s",
				  IPC_CLCHAN_NAME(chp), downcode,
				  strerror(err));

		return;
	}

	/* Set down code as additional data. */
	sess = &event->ie_sess;
	err = pfc_ipcmsg_init_uint32(&sess->icss_msg, downcode);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to set channel down code"
				  "(code=%u): %s",
				  IPC_CLCHAN_NAME(chp), downcode,
				  strerror(err));
	}
	else {
		/* Post an event to the event dispatch queue. */
		IPC_CLCHAN_RDLOCK(chp);
		(void)ipc_evdisp_post(elsp, event);
		IPC_CLCHAN_UNLOCK(chp);
	}

	IPC_EVENT_RELEASE(event);
}

/*
 * static int
 * ipc_elsess_post_notification(ipc_elsess_t *PFC_RESTRICT elsp,
 *				ipc_evhdlr_t *PFC_RESTRICT ehp, uint32_t state)
 *	Post an IPC channel state notification event to the IPC event handler
 *	specified by `ehp'.
 *
 *	`state' must be a channel state value to be set in an IPC event.
 *
 * Remarks:
 *	This function must be called with holding the following lock in order:
 *	  - The client lock (reader or writer)
 *	  - IPC channel lock associated with `elsp' (reader or writer)
 *	  - The host set lock associated with `ehp' (reader or writer)
 *	  - The event listener session lock associated with `elsp'
 */
static int
ipc_elsess_post_notification(ipc_elsess_t *PFC_RESTRICT elsp,
			     ipc_evhdlr_t *PFC_RESTRICT ehp, uint32_t state)
{
	pfc_ipcevent_t	*event;
	pfc_ipcsess_t	*sess;
	ipc_clchan_t	*chp = elsp->els_chan;
	int		err;

	/* Create a channel state notification event. */
	PFC_ASSERT(chp == ehp->evh_chan);
	err = pfc_ipcevent_create_chstate(&event, elsp, PFC_IPCCHSTATE_NOTIFY);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to create channel state "
				  "notification event(%u): %s",
				  IPC_CLCHAN_NAME(chp), state, strerror(err));

		return err;
	}

	/* Set state code as additional data. */
	sess = &event->ie_sess;
	err = pfc_ipcmsg_init_uint32(&sess->icss_msg, state);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to set channel notification "
				  "state(%u): %s",
				  IPC_CLCHAN_NAME(chp), state, strerror(err));
	}
	else {
		/* Post an event to the specified handler. */
		err = ipc_evdisp_post_event(event, ehp);
	}

	IPC_EVENT_RELEASE(event);

	return err;
}

/*
 * static int
 * ipc_elsess_addtarget(ipc_elsess_t *elsp)
 *	Send target event set to the IPC server.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the event listener session
 *	lock. Note that it may be released for a while.
 */
static int
ipc_elsess_addtarget(ipc_elsess_t *elsp)
{
	pfc_iostream_t	stream = elsp->els_stream;
	pfc_timespec_t	abstime;
	pfc_rbtree_t	*tree;
	pfc_rbnode_t	*node;
	pfc_bool_t	do_reset, empty;
	ipc_evset_t	target;
	uint8_t		cmd;
	int		err;

	PFC_ASSERT(stream != NULL);

	if (IPC_ELSESS_IS_SHUTDOWN(elsp)) {
		/* Already shut down. */
		return ECANCELED;
	}

	/* Create target event set to be sent to the IPC server. */
	err = ipc_elsess_maketarget(elsp, &target, &do_reset);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	tree = &target.est_tree;
	empty = pfc_rbtree_isempty(tree);
	if (!do_reset && empty) {
		/* Nothing to do. */
		return 0;
	}

	IPC_ELSESS_UNLOCK(elsp);

	/* Determine I/O timeout. */
	err = ipc_event_gettimeout(&abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error_target;
	}

	if (do_reset) {
		/* Send MASK_RESET subcommand. */
		cmd = IPC_EVENTCMD_MASK_RESET;
		err = pfc_ipc_write(stream, &cmd, sizeof(cmd), empty,
				    &abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPCCLNT_LOG_ERROR("%s: Failed to send MASK_RESET "
					  "subcommand: %s",
					  IPC_CLCHAN_NAME(elsp->els_chan),
					  strerror(err));
			goto error_target;
		}

		if (empty) {
			IPC_ELSESS_LOCK(elsp);

			return 0;
		}
	}

	/* Send MASK_ADD subcommand. */
	cmd = IPC_EVENTCMD_MASK_ADD;
	err = pfc_ipc_write(stream, &cmd, sizeof(cmd), PFC_FALSE, &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to send MASK_ADD subcommand: %s",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));
		goto error_target;
	}

	/* Send pairs of IPC service name and event mask. */
	node = NULL;
	while ((node = pfc_rbtree_next(tree, node)) != NULL) {
		ipc_evmask_t	*emp = IPC_EVMASK_NODE2PTR(node);
		pfc_refptr_t	*rname = emp->iem_name;
		const char	*name;
		size_t		namelen;

		/*
		 * Remarks:
		 *	Although pseudo service name of IPC channel state
		 *	change event does not need to be sent to the server,
		 *	we do send it so that the IPC server can filter out
		 *	unwanted events.
		 */
		name = pfc_refptr_string_value(rname);
		namelen = pfc_refptr_string_length(rname);
		err = ipc_elsess_sendevmask(elsp, stream, name, namelen,
					    &emp->iem_mask, &abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPCCLNT_LOG_ERROR("%s: MASK_ADD: "
					  "Failed to send target: %s",
					  IPC_CLCHAN_NAME(elsp->els_chan),
					  strerror(err));
			goto error_target;
		}

		IPCCLNT_LOG_VERBOSE("%s: MASK_ADD: target: "
				    "name=%s, mask=0x%" IPC_PFMT_EVMASK,
				    IPC_CLCHAN_NAME(elsp->els_chan),
				    name, emp->iem_mask);
	}

	/* Send EOF mark. */
	cmd = 0;
	err = pfc_ipc_write(stream, &cmd, sizeof(cmd), PFC_TRUE, &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: MASK_ADD: "
				  "Failed to send EOF mark: %s",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));
		goto error_target;
	}

	pfc_ipc_evset_destroy(&target);
	IPC_ELSESS_LOCK(elsp);

	return 0;

error_target:
	pfc_ipc_evset_destroy(&target);
	IPC_ELSESS_LOCK(elsp);

error:
	ipc_elsess_setdowncode(elsp, err);

	return err;
}

/*
 * static int
 * ipc_elsess_maketarget(ipc_elsess_t *PFC_RESTRICT elsp,
 *			 ipc_evset_t *PFC_RESTRICT target,
 *			 pfc_bool_t *PFC_RESTRICT do_reset)
 *	Make target event set to be sent to the IPC server.
 *
 *	This function creates a local copy of target event set, and set it to
 *	the the buffer pointed by `target'. The caller is responsible for
 *	destroying the target event set in `target'.
 *
 *	If the target event set needs to be reset, PFC_TRUE is set to the
 *	buffer pointed by `do_reset'. Otherwise PFC_FALSE is set.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the event listener session
 *	lock.
 */
static int
ipc_elsess_maketarget(ipc_elsess_t *PFC_RESTRICT elsp,
		      ipc_evset_t *PFC_RESTRICT target,
		      pfc_bool_t *PFC_RESTRICT do_reset)
{
	int	err;

	*do_reset = PFC_FALSE;

	if (elsp->els_nwilds != 0) {
		/* This session requires all events. */
		if (!pfc_ipc_evset_isempty(&elsp->els_target)) {
			/* Need to reset target event set. */
			*do_reset = PFC_TRUE;
		}

		pfc_ipc_evset_destroy(&elsp->els_target);
		pfc_ipc_evset_destroy(&elsp->els_newtarget);
		pfc_ipc_evset_init(target);

		return 0;
	}

	/* Merge new target event set to current set. */
	err = pfc_ipc_evset_merge(&elsp->els_target, &elsp->els_newtarget);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to merge target event set: "
				  "%s", IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));

		return err;
	}

	/* Create local copy of target event set to be sent. */
	if (elsp->els_flags & IPC_ELSF_TARGET_ALL) {
		/* All event target set need to be sent. */
		err = pfc_ipc_evset_copy(target, &elsp->els_target);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPCCLNT_LOG_ERROR("%s: Failed to copy target event "
					  "set: %s",
					  IPC_CLCHAN_NAME(elsp->els_chan),
					  strerror(err));

			return err;
		}

		pfc_ipc_evset_destroy(&elsp->els_newtarget);
		elsp->els_flags &= ~IPC_ELSF_TARGET_ALL;
	}
	else {
		/* Move all target set nodes to `target'. */
		*target = elsp->els_newtarget;
		pfc_ipc_evset_init(&elsp->els_newtarget);
	}

	return 0;
}

/*
 * static int
 * ipc_elsess_removetarget(ipc_elsess_t *PFC_RESTRICT elsp,
 *			   pfc_iostream_t PFC_RESTRICT stream,
 *			   const char *PFC_RESTRICT name, size_t namelen,
 *			   pfc_ipcevtype_t type)
 *	Remove the specified event type from the target.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the IPC channel lock
 *	associated with `elsp' in reader or writer mode. It is always released
 *	on return.
 */
static int
ipc_elsess_removetarget(ipc_elsess_t *PFC_RESTRICT elsp,
			pfc_iostream_t PFC_RESTRICT stream,
			const char *PFC_RESTRICT name, size_t namelen,
			pfc_ipcevtype_t type)
{
	pfc_timespec_t	abstime;
	pfc_ipcevmask_t	mask;
	ipc_clchan_t	*chp = elsp->els_chan;
	int		err;
	uint8_t		cmd;

	PFC_ASSERT(PFC_IPC_EVTYPE_IS_VALID(type));

	IPC_ELSESS_LOCK(elsp);

	if (pfc_ipc_evset_contains(&elsp->els_newtarget, name, type)) {
		/* This event is subscribed by new event handler. */
		IPC_ELSESS_UNLOCK(elsp);
		IPC_CLCHAN_UNLOCK(chp);

		return 0;
	}

	if (!pfc_ipc_evset_contains(&elsp->els_target, name, type)) {
		/* Need to fix up the target event set. */
		err = ipc_elsess_fixtarget(elsp, chp);
		IPC_ELSESS_UNLOCK(elsp);

		return err;
	}

	mask = PFC_IPC_EVENT_MASK_BIT(type);
	pfc_ipc_evset_remove(&elsp->els_target, name, &mask);
	IPC_ELSESS_UNLOCK(elsp);
	IPC_CLCHAN_UNLOCK(chp);

	/* Determine I/O timeout. */
	err = ipc_event_gettimeout(&abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		return err;
	}

	/* Send MASK_DEL subcommand. */
	cmd = IPC_EVENTCMD_MASK_DEL;
	err = pfc_ipc_write(stream, &cmd, sizeof(cmd), PFC_FALSE, &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to send MASK_DEL subcommand: "
				  "%s", IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));

		return err;
	}

	/* Send pairs of IPC service name and event mask. */
	err = ipc_elsess_sendevmask(elsp, stream, name, namelen, &mask,
				    &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: MASK_DEL: "
				  "Failed to send target: %s",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));

		return err;
	}

	IPCCLNT_LOG_VERBOSE("%s: MASK_DEL: target: name=%s, mask=0x%"
			    IPC_PFMT_EVMASK, IPC_CLCHAN_NAME(elsp->els_chan),
			    name, mask);

	/* Send EOF mark. */
	cmd = 0;
	err = pfc_ipc_write(stream, &cmd, sizeof(cmd), PFC_TRUE, &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: MASK_DEL: "
				  "Failed to send EOF mark: %s",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));
	}

	return err;
}

/*
 * static int
 * ipc_elsess_fixtarget(ipc_elsess_t *PFC_RESTRICT elsp,
 *			ipc_clchan_t *PFC_RESTRICT chp)
 *	Fix up target event set for this session by scanning all event
 *	handlers.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the IPC channel lock in
 *	  reader or writer mode, and the event listener session lock in order.
 *	  Note that the IPC channel lock is always released on return.
 *
 *	- The caller must ensure that no target event set is configured
 *	  to the specified listener session.
 */
static int
ipc_elsess_fixtarget(ipc_elsess_t *PFC_RESTRICT elsp,
		     ipc_clchan_t *PFC_RESTRICT chp)
{
	pfc_list_t	*elem;
	int		err;

	PFC_ASSERT(elsp->els_nwilds == 0);
	pfc_ipc_evset_destroy(&elsp->els_target);
	pfc_ipc_evset_destroy(&elsp->els_newtarget);

	/* Scan all event handlers and merge targets. */
	PFC_LIST_FOREACH(&chp->ichc_evhandler, elem) {
		ipc_evhdlr_t	*ehp = IPC_EVHDLR_LIST2PTR(elem);

		err = pfc_ipc_evset_merge(&elsp->els_target, &ehp->evh_target);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPCCLNT_LOG_WARN("%s: Failed to merge target "
					 "event set for fixup: %s",
					 IPC_CLCHAN_NAME(elsp->els_chan),
					 strerror(err));
			goto scan_error;
		}
	}

	elsp->els_flags |= IPC_ELSF_TARGET_ALL;
	IPC_CLCHAN_UNLOCK(chp);

	/* Send target event set to the server. */
	return ipc_elsess_addtarget(elsp);

scan_error:
	/*
	 * Remarks:
	 *	We can ignore target merge error because currently no target
	 *	event set is configured to the IPC server. So it will send
	 *	all events to this session.
	 */
	pfc_ipc_evset_destroy(&elsp->els_target);
	IPC_CLCHAN_UNLOCK(chp);

	return 0;
}

/*
 * static int
 * ipc_elsess_sendevmask(ipc_elsess_t *PFC_RESTRICT elsp,
 *			 pfc_iostream_t PFC_RESTRICT stream,
 *			 const char *PFC_RESTRICT name, size_t namelen,
 *			 pfc_ipcevmask_t *PFC_RESTRICT mask,
 *			 ctimespec_t *PFC_RESTRICT abstime)
 *	Write a pair of IPC service name and event mask to the given stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_elsess_sendevmask(ipc_elsess_t *PFC_RESTRICT elsp,
		      pfc_iostream_t PFC_RESTRICT stream,
		      const char *PFC_RESTRICT name, size_t namelen,
		      pfc_ipcevmask_t *PFC_RESTRICT mask,
		      ctimespec_t *PFC_RESTRICT abstime)
{
	uint8_t	sz;
	int	err;

	/* Send length of IPC service name. */
	PFC_ASSERT(namelen <= IPC_SERVICE_NAMELEN_MAX);
	sz = (uint8_t)namelen;
	err = pfc_ipc_write(stream, &sz, sizeof(sz), PFC_FALSE, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: "
				  "Failed to send length of service name: %s ",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));

		return err;
	}

	/* Send IPC service name. */
	err = pfc_ipc_write(stream, name, sz, PFC_FALSE, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to send service name: %s ",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));

		return err;
	}

	/* Send event mask. */
	err = pfc_ipc_write(stream, mask, sizeof(*mask), PFC_FALSE, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to send event mask: %s ",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));

		return err;
	}

	return 0;
}

/*
 * static int
 * ipc_elsess_receive(ipc_elsess_t *elsp)
 *	Receive an IPC event from the IPC server.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the event listener session
 *	lock. Note that it may be released for a while.
 */
static int
ipc_elsess_receive(ipc_elsess_t *elsp)
{
	ipc_sess_t	*sess = &elsp->els_sess;
	ipc_evproto_t	evproto;
	ipc_msg_t	msg;
	ipc_clchan_t	*chp = elsp->els_chan;
	pfc_iostream_t	stream = sess->iss_stream;
	pfc_timespec_t	abstime;
	pfc_ipcevent_t	*event;
	uint8_t		type, namelen;
	uint32_t	count;
	char		name[IPC_SERVICE_NAMELEN_MAX + 1];
	int		err;

	PFC_ASSERT(stream != NULL);

	if (IPC_ELSESS_IS_SHUTDOWN(elsp)) {
		/* Already shut down. */
		return ECANCELED;
	}

	if ((elsp->els_flags & IPC_ELSF_RECV) == 0) {
		/* No event is arrived. */
		return 0;
	}

	IPC_ELSESS_UNLOCK(elsp);

	/* Initialize dummy IPC message instance. */
	pfc_ipcmsg_init(&msg, sess->iss_flags);

	/* Determine I/O timeout. */
	err = ipc_event_gettimeout(&abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error_lock;
	}

	/* Read type of event. */
	err = pfc_ipc_read(stream, &type, sizeof(type), &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to receive event type: %s",
				  IPC_CLCHAN_NAME(chp), strerror(err));
		goto error_lock;
	}

	if (PFC_EXPECT_FALSE(type != IPC_EVENTTYPE_EVENT)) {
		IPCCLNT_LOG_ERROR("%s: Unexpected event type: 0x%x",
				  IPC_CLCHAN_NAME(chp), type);
		err = EPROTO;
		goto error_lock;
	}

	/* Receive common event data. */
	err = pfc_ipc_read(stream, &evproto, sizeof(evproto), &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to receive common event data"
				  ": %s", IPC_CLCHAN_NAME(chp), strerror(err));
		goto error_lock;
	}

	namelen = evproto.iev_namelen;
	if (PFC_EXPECT_FALSE(namelen >= sizeof(name))) {
		IPCCLNT_LOG_ERROR("%s: Unexpected length of service name: %u",
				  IPC_CLCHAN_NAME(chp), namelen);
		goto error_lock;
	}

	/* Receive IPC service name. */
	err = pfc_ipc_read(stream, name, namelen, &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to receive service name: %s",
				  IPC_CLCHAN_NAME(chp), strerror(err));
		goto error_lock;
	}
	name[namelen] = '\0';

	/* Receive additional data. */
	err = pfc_ipcmsg_recv(sess, &msg, &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to receive IPC message: %s",
				  IPC_CLCHAN_NAME(chp), strerror(err));
		goto error_lock;
	}

	/* Create a new IPC event object. */
	err = pfc_ipcevent_create(&event, elsp, name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCCLNT_LOG_ERROR("%s: Failed to create IPC event: %s",
				  IPC_CLCHAN_NAME(chp), strerror(err));
		pfc_ipcmsg_destroy(&msg);
		goto error_lock;
	}

	/*
	 * Copy received data to the event object.
	 * Note that buffer for additional data is delegated to the event
	 * object. So `msg' must not be passed to pfc_ipcmsg_destroy().
	 */
	event->ie_proto = evproto;
	event->ie_sess.icss_msg = msg;

	/* Dispatch IPC event to handlers. */
	IPC_CLCHAN_RDLOCK(chp);
	count = ipc_evdisp_post(elsp, event);
	IPC_EVENT_RELEASE(event);

	if (PFC_EXPECT_FALSE(count == 0)) {
		/* No one wants this event. */
		err = ipc_elsess_removetarget(elsp, stream, name, namelen,
					      evproto.iev_type);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error_lock;
		}
	}
	else {
		IPC_CLCHAN_UNLOCK(chp);
	}

	IPC_ELSESS_LOCK(elsp);
	elsp->els_flags &= ~IPC_ELSF_RECV;

	if (PFC_EXPECT_TRUE(elsp->els_flags & IPC_ELSF_EPOLL)) {
		int	fd;

		/* Enable EPOLLIN event. */
		fd = pfc_iostream_getfd(stream);
		err = pfc_epoll_ctl(ipc_event_ctx.ev_epoll, EPOLL_CTL_MOD, fd,
				    EPOLLIN | IPC_EPOLLEV_ONESHOT,
				    &elsp->els_epoll);
		if  (PFC_EXPECT_FALSE(err != 0)) {
			err = errno;
			IPCCLNT_LOG_ERROR("%s: "
					  "Failed to enable input event: %s",
					  IPC_CLCHAN_NAME(chp), strerror(err));
			goto error;
		}
	}
	else {
		/* This can happen if the event system is finalized. */
		IPC_EVCTX_DONE_ASSERT(&ipc_event_ctx);

		return ECANCELED;
	}

	return 0;

error_lock:
	IPC_ELSESS_LOCK(elsp);

error:
	ipc_elsess_setdowncode(elsp, err);

	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}

/*
 * static void
 * ipc_elsess_shutdown(ipc_elsess_t *elsp, pfc_bool_t do_untimeout)
 *	Shut down the event listener session.
 *
 *	If `do_untimeout' is PFC_TRUE, this function tries to unregister
 *	the session timeout.
 *
 * Remarks:
 *	- This function must be called with holding the event listener session
 *	  lock.
 *
 *	- If PFC_TRUE is specified to `do_untimeout', this function may
 *	  decrement the reference counter of the listener session. But it never
 *	  destroys the listener session.
 */
static void
ipc_elsess_shutdown(ipc_elsess_t *elsp, pfc_bool_t do_untimeout)
{
	/* Notify shutdown to the session. */
	ipc_event_close(&IPC_ELSESS_WRPIPE(elsp));

	if (elsp->els_stream != NULL) {
		/* Update the session state. */
		ipc_evtaskq_post(elsp);
	}

	if (do_untimeout) {
		/* Unregister timeout for this session. */
		ipc_evctx_untimeout(&elsp->els_timeout);
	}
}

/*
 * static void
 * ipc_elsess_timeout(ipc_elsess_t *elsp, ipc_etmfunc_t func, uint64_t msec)
 *	Register timeout handler for the specified event listener session.
 *
 * Remarks:
 *	This function must be called with holding the event listener session
 *	lock.
 */
static void
ipc_elsess_timeout(ipc_elsess_t *elsp, ipc_etmfunc_t func, uint64_t msec)
{
	ipc_evctx_t	*ectx = &ipc_event_ctx;
	int		err;

	PFC_ASSERT(elsp->els_timeout.etmo_elsess);

	if (IPC_ELSESS_IS_SHUTDOWN(elsp)) {
		return;
	}

	IPC_EVCTX_LOCK(ectx);
	err = ipc_evctx_timeout_l(ectx, &elsp->els_timeout, func, msec);
	if (PFC_EXPECT_TRUE(err == 0)) {
		IPC_ELSESS_HOLD(elsp);
	}
	IPC_EVCTX_UNLOCK(ectx);
}

/*
 * static void
 * ipc_elsess_reconnect(ipc_etimeout_t *etmp)
 *	The listener session timeout handler which tries to reconnect to the
 *	IPC server.
 */
static void
ipc_elsess_reconnect(ipc_etimeout_t *etmp)
{
	ipc_elsess_t	*elsp = IPC_ELSESS_ETM2PTR(etmp);

	IPC_ELSESS_LOCK(elsp);

	if (!IPC_ELSESS_IS_SHUTDOWN(elsp) &&
	    (elsp->els_flags & IPC_ELSF_BUSY) == 0 &&
	    elsp->els_stream == NULL) {
		ipc_evtaskq_post(elsp);
	}

	IPC_ELSESS_RELEASE(elsp);
}

/*
 * static int
 * ipc_elsess_epoll(pfc_ephdlr_t *ephp, uint32_t events, void *arg)
 *	Event poll event handler for the event listener session.
 *
 * Calling/Exit State:
 *	Zero is always returned.
 */
static int
ipc_elsess_epoll(pfc_ephdlr_t *ephp, uint32_t events, void *arg)
{
	ipc_elsess_t	*elsp = IPC_ELSESS_EP2PTR(ephp);
	ipc_evctx_t	*ectx = (ipc_evctx_t *)arg;
	pfc_bool_t	do_release, do_untimeout = PFC_FALSE;

	PFC_ASSERT(ectx == &ipc_event_ctx);

	IPC_ELSESS_LOCK(elsp);
	IPC_ELSESS_EPOLL_ASSERT(elsp, ectx);

	if (PFC_EXPECT_FALSE(events & (EPOLLOUT | EPOLLHUP | EPOLLERR))) {
		int	err, fd;

		/* Remove the session socket from the epoll instance. */
		IPC_EVCTX_LOCK(ectx);
		PFC_ASSERT(!IPC_EVCTX_IS_DONE(ectx));
		fd = pfc_iostream_getfd(elsp->els_stream);
		err = pfc_epoll_ctl(ectx->ev_epoll, EPOLL_CTL_DEL, fd, 0,
				    NULL);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* This should never happen. */
			err = errno;
			IPCCLNT_LOG_ERROR("%s: Failed to delete session "
					  "from the epoll instance: %s",
					  IPC_CLCHAN_NAME(elsp->els_chan),
					  strerror(err));
			/* FALLTHROUGH */
		}

		elsp->els_flags &= ~IPC_ELSF_EPOLL;
		pfc_list_remove(&elsp->els_eplist);
		IPC_ELSESS_SIGNAL(elsp);
#ifdef	PFC_VERBOSE_DEBUG
		pfc_list_init(&elsp->els_eplist);
#endif	/* PFC_VERBOSE_DEBUG */
		IPC_EVCTX_UNLOCK(ectx);

		if (events & (EPOLLHUP | EPOLLERR)) {
			int	soerr;

			/* Determine socket level error. */
			soerr = ipc_elsess_getsockerror(elsp, fd);
			if (soerr == 0) {
				/* Session was reset by peer. */
				soerr = ECONNRESET;
			}

			/* Need to update the state of this session. */
			ipc_elsess_setdowncode(elsp, soerr);
			ipc_evtaskq_post(elsp);
		}

		/* Unregister timeout for this session. */
		do_release = PFC_TRUE;
		do_untimeout = PFC_TRUE;
	}
	else {
		if (events & EPOLLIN) {
			/* Disable input event. */
			ipc_elsess_epoll_disable_input(elsp);

			/* Need to receive an IPC event from the server. */
			elsp->els_flags |= IPC_ELSF_RECV;
			ipc_evtaskq_post(elsp);
			do_untimeout = PFC_TRUE;
		}
		do_release = PFC_FALSE;
	}

	if (do_untimeout) {
		/* Unregister timeout for this session. */
		ipc_evctx_untimeout(&elsp->els_timeout);
		PFC_ASSERT(elsp->els_refcnt > 0);
	}

	IPC_ELSESS_RELEASE_COND(elsp, do_release);

	return 0;
}

/*
 * static void
 * ipc_elsess_epoll_delete(ipc_elsess_t *elsp)
 *	Delete the socket associated with the specified session from the
 *	epoll instance.
 *
 * Remarks:
 *	This function must be called with holding the event listener session
 *	lock. Note that it may be released for a while.
 */
static void
ipc_elsess_epoll_delete(ipc_elsess_t *elsp)
{
	ipc_evctx_t	*ectx;
	int		err, fd;

	if (PFC_EXPECT_FALSE((elsp->els_flags & IPC_ELSF_EPOLL) == 0)) {
		/* The session socket does not exist in the epoll instance. */
		IPC_ELSESS_NOT_EPOLL_ASSERT(elsp, &ipc_event_ctx);

		return;
	}

	/*
	 * Set EPOLLOUT to the session socket events to wake up the listener
	 * thread blocked by the epoll_wait(2) call.
	 */
	ectx = &ipc_event_ctx;
	IPC_EVCTX_LOCK(ectx);
	if (PFC_EXPECT_FALSE(IPC_EVCTX_IS_DONE(ectx))) {
		/*
		 * No need to delete the socket from the epoll instance
		 * because epoll_wait(2) is never called any more.
		 * EPOLL flag in this session will be cleared by the event
		 * listener thread.
		 */
		IPC_EVCTX_UNLOCK(ectx);

		return;
	}

	fd = pfc_iostream_getfd(elsp->els_stream);
	err = pfc_epoll_ctl(ectx->ev_epoll, EPOLL_CTL_MOD, fd, EPOLLOUT,
			    &elsp->els_epoll);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		err = errno;
		IPCCLNT_LOG_ERROR("%s: Failed to set EPOLLOUT event to the "
				  "epoll instance: %s",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));
		/* FALLTHROUGH */
	}
	IPC_EVCTX_UNLOCK(ectx);

	/* Wait for the listener thread to delete this session. */
	do {
		IPC_ELSESS_WAIT(elsp);
	} while (elsp->els_flags & IPC_ELSF_EPOLL);
}

/*
 * static void
 * ipc_elsess_epoll_disable_input(ipc_elsess_t *elsp)
 *	Disable input event on the listener session.
 *
 * Remarks:
 *	This function must be called with holding the event listener session
 *	lock.
 */
static void
ipc_elsess_epoll_disable_input(ipc_elsess_t *elsp)
{
#ifndef	PFC_HAVE_EPOLLONESHOT
	ipc_evctx_t	*ectx = &ipc_event_ctx;
	int		err, fd = pfc_iostream_getfd(elsp->els_stream);

	err = pfc_epoll_ctl(ectx->ev_epoll, EPOLL_CTL_MOD, fd, EPOLLHUP,
			    &elsp->els_epoll);
	if (PFC_EXPECT_FALSE(err != 0)) {
		err = errno;
		IPCCLNT_LOG_ERROR("%s: Failed to disable input event: %s",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(err));
		/* FALLTHROUGH */
	}
#endif	/* !PFC_HAVE_EPOLLONESHOT */
}

/*
 * static int
 * ipc_elsess_lookup(const char *PFC_RESTRICT channel,
 *		     const pfc_hostaddr_t *PFC_RESTRICT haddr,
 *		     ipc_clchan_t **PFC_RESTRICT chpp,
 *		     ipc_elsess_t **PFC_RESTRICT elspp)
 *	Search for the IPC event listener session associated with the given
 *	IPC channel name and the host address.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to the IPC channel
 *	associated with `channel' is set to the buffer pointed by `chpp',
 *	and the IPC event listener session is set to the buffer pointed by
 *	`elspp', and zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the IPC client lock in
 *	  reader or writer mode.
 *
 *	-  Note that this function returns with holding the IPC channel lock
 *	   in reader mode on successful return.
 */
static int
ipc_elsess_lookup(const char *PFC_RESTRICT channel,
		  const pfc_hostaddr_t *PFC_RESTRICT haddr,
		  ipc_clchan_t **PFC_RESTRICT chpp,
		  ipc_elsess_t **PFC_RESTRICT elspp)
{
	ipc_clchan_t	*chp;
	pfc_rbnode_t	*node;
	int		err;

	/* Ensure that the IPC event subsystem is running. */
	err = ipc_event_is_running();
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Determine the IPC channel. */
	chp = pfc_ipcclnt_chan_lookup(channel, PFC_FALSE);
	if (PFC_EXPECT_FALSE(chp == NULL)) {
		return ENOENT;
	}

	/*
	 * Search for the event listener session associated with the given
	 * host address.
	 */
	IPC_CLCHAN_RDLOCK(chp);

	node = pfc_rbtree_get(&chp->ichc_elsess, (pfc_cptr_t)haddr);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		IPC_CLCHAN_UNLOCK(chp);

		return ENOENT;
	}

	*elspp = IPC_ELSESS_NODE2PTR(node);
	*chpp = chp;

	return 0;
}

/*
 * static int
 * ipc_elsess_getsockerror(ipc_elsess_t *elsp, int sock)
 *	Get and clear the socket error pending on the given socket.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pending socket error number is
 *	returned.
 *	On failure, an error number which indicates the cause of error is
 *	returned.
 */
static int
ipc_elsess_getsockerror(ipc_elsess_t *elsp, int sock)
{
	int		soerr, ret;
	socklen_t	optlen = sizeof(soerr);

	ret = getsockopt(sock, SOL_SOCKET, SO_ERROR, &soerr, &optlen);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		soerr = errno;
		IPCCLNT_LOG_ERROR("%s: Failed to get socket error: %s",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(soerr));
		if (PFC_EXPECT_FALSE(soerr == 0)) {
			soerr = EIO;
		}
	}
	else if (soerr != 0) {
		IPCCLNT_LOG_ERROR("%s: Detected socket error: %s",
				  IPC_CLCHAN_NAME(elsp->els_chan),
				  strerror(soerr));
	}

	return soerr;
}


/*
 * static pfc_cptr_t
 * ipc_evhdlr_getkey(pfc_rbnode_t *node)
 *	Return the key of the IPC event handler instance.
 *	`node' must be a pointer to evh_node in ipc_evhdlr_t.
 */
static pfc_cptr_t
ipc_evhdlr_getkey(pfc_rbnode_t *node)
{
	ipc_evhdlr_t	*ehp = IPC_EVHDLR_NODE2PTR(node);

	return IPC_EVHDLR_KEY(ehp->evh_id);
}

/*
 * static pfc_cptr_t
 * ipc_elsess_getkey(pfc_rbnode_t *node)
 *	Return the key of the IPC event listener session.
 *	`node' must be a pointer to els_node in ipc_elsess_t.
 */
static pfc_cptr_t
ipc_elsess_getkey(pfc_rbnode_t *node)
{
	ipc_elsess_t	*elsp = IPC_ELSESS_NODE2PTR(node);

	return (pfc_cptr_t)&elsp->els_addr;
}

/*
 * static pfc_cptr_t
 * ipc_chref_host_getkey(pfc_rbnode_t *node)
 *	Return the key of IPC host set reference.
 *	`node' must be a pointer to cr_hsnode in ipc_chref_t.
 */
static pfc_cptr_t
ipc_chref_host_getkey(pfc_rbnode_t *node)
{
	ipc_chref_t	*crp = IPC_CHREF_HOSTSET_NODE2PTR(node);

	return (pfc_cptr_t)crp->cr_hostset;
}

/*
 * static pfc_cptr_t
 * ipc_etimeout_getkey(pfc_rbnode_t *node)
 *	Return the key of ipc_etimeout_t.
 *	`node' must be a pointer to et_node in ipc_etimeout_t.
 */
static pfc_cptr_t
ipc_etimeout_getkey(pfc_rbnode_t *node)
{
	ipc_etimeout_t	*etp = IPC_ETIMEOUT_NODE2PTR(node);

	return PFC_RBTREE_KEY64(etp->etmo_time);
}

#ifdef	PFC_VERBOSE_DEBUG

/*
 * Below functions are provided only for debugging.
 */

/*
 * const pfc_ipcevopts_t *
 * pfc_ipcevent_getopts(void)
 *	Return a pointer to global event attributes.
 */
const pfc_ipcevopts_t *
pfc_ipcevent_getopts(void)
{
	return &ipc_event_opts;
}

/*
 * uint32_t
 * pfc_ipcevent_getnumhandlers(void)
 *	Return the number of event handlers.
 */
uint32_t
pfc_ipcevent_getnumhandlers(void)
{
	uint32_t	count = 0;
	pfc_rbnode_t	*node = NULL;

	IPC_CLIENT_RDLOCK();

	while ((node = pfc_rbtree_next(&ipc_event_handlers, node)) != NULL) {
		count++;
	}

	IPC_CLIENT_UNLOCK();

	return count;
}

#endif	/* PFC_VERBOSE_DEBUG */
