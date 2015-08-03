/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_IPCCLNT_IPCCLNT_EVENT_H
#define	_PFC_LIBPFC_IPCCLNT_IPCCLNT_EVENT_H

/*
 * Common definitions for IPC event feature.
 */

#include <pthread.h>
#include <pfc/ipc_client.h>
#include <pfc/refptr.h>
#include <pfc/rbtree.h>
#include <pfc/clock.h>
#include <pfc/synch.h>
#include <pfc/epoll.h>
#include <pfc/debug.h>
#include <ipc_impl.h>
#include "ipcclnt_impl.h"

PFC_C_BEGIN_DECL

/*
 * Pseudo IPC service name.
 */
#define	IPC_SVNAME_CHSTATE	"<chstate>"	/* state change event */

/*
 * Global state of the IPC event subsystem.
 */
typedef enum {
	IPC_EVSTATE_INITIAL	= 0,	/* initial state */
	IPC_EVSTATE_RUNNING,		/* running */
	IPC_EVSTATE_SHUTDOWN,		/* shut down */
	IPC_EVSTATE_FINI,		/* finalized */
	IPC_EVSTATE_FORK,		/* disabled by fork(2) */
} ipc_evstate_t;

/*
 * IPC server's address.
 */
typedef struct {
	pfc_hostaddr_t	he_addr;	/* host address */
	pfc_rbnode_t	he_node;	/* Red-Black tree node */
} ipc_hostent_t;

#define	IPC_HOSTENT_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_hostent_t, he_node)

/*
 * Set of IPC server's address.
 *
 * Remarks:
 *	hs_chref must not be serialized by hs_lock but the client lock.
 */
typedef struct {
	pfc_refptr_t	*hs_name;	/* name of this address set */
	pfc_rwlock_t	hs_lock;	/* read-write lock */
	pfc_rbnode_t	hs_node;	/* Red-Black tree node */
	pfc_rbtree_t	hs_host;	/* set of ipc_hostent_t */
	pfc_rbtree_t	hs_chref;	/* set of ipc_chref_t */
	uint32_t	hs_refcnt;	/* reference counter */
} ipc_hostset_t;

#define	IPC_HOSTSET_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_hostset_t, hs_node)

#define	IPC_HOSTSET_RDLOCK(hset)				\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&(hset)->hs_lock), 0)
#define	IPC_HOSTSET_WRLOCK(hset)				\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&(hset)->hs_lock), 0)
#define	IPC_HOSTSET_UNLOCK(hset)				\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&(hset)->hs_lock), 0)

#define	IPC_HOSTSET_NAME(hset)	pfc_refptr_string_value((hset)->hs_name)

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * ipc_hostset_contains_l(ipc_hostset_t *PFC_RESTRICT hset,
 *			  const pfc_hostaddr_t *PFC_RESTRICT addr)
 *	Check whether the given host address is contained in the host set.
 *
 * Calling/Exit State:
 *	Zero is returned if `addr' is found in the host set.
 *	Otherwise ENOENT is returned.
 *
 * Remarks:
 *	This function must be called with holding the host set lock in reader
 *	or writer mode.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
ipc_hostset_contains_l(ipc_hostset_t *PFC_RESTRICT hset,
		     const pfc_hostaddr_t *PFC_RESTRICT addr)
{
	pfc_rbnode_t	*node;
	int		err;

	node = pfc_rbtree_get(&hset->hs_host, (pfc_cptr_t)addr);
	err = (node == NULL) ? ENOENT : 0;

	return err;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * ipc_hostset_contains(ipc_hostset_t *PFC_RESTRICT hset,
 *			const pfc_hostaddr_t *PFC_RESTRICT addr)
 *	Check whether the given host address is contained in the host set.
 *
 * Calling/Exit State:
 *	Zero is returned if `addr' is found in the host set.
 *	Otherwise ENOENT is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
ipc_hostset_contains(ipc_hostset_t *PFC_RESTRICT hset,
		     const pfc_hostaddr_t *PFC_RESTRICT addr)
{
	int	err;

	IPC_HOSTSET_RDLOCK(hset);
	err = ipc_hostset_contains_l(hset, addr);
	IPC_HOSTSET_UNLOCK(hset);

	return err;
}

/*
 * Linkage between IPC channel and host set.
 */
typedef struct {
	ipc_clchan_t	*cr_chan;	/* IPC channel */
	ipc_hostset_t	*cr_hostset;	/* IPC host set */
	pfc_rbnode_t	cr_chnode;	/* rbtree node for channel index */
	pfc_rbnode_t	cr_hsnode;	/* rbtree node for host set index */
	uint32_t	cr_refcnt;	/* reference counter */
} ipc_chref_t;

#define	IPC_CHREF_CHAN_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_chref_t, cr_chnode)
#define	IPC_CHREF_HOSTSET_NODE2PTR(node)			\
	PFC_CAST_CONTAINER((node), ipc_chref_t, cr_hsnode)

struct ipc_etimeout;
typedef struct ipc_etimeout	ipc_etimeout_t;

/*
 * Prototype of timeout handler.
 */
typedef void	(*ipc_etmfunc_t)(ipc_etimeout_t *etmp);

/*
 * Timeout node.
 */
struct ipc_etimeout {
	uint64_t	etmo_time;	/* time to run handler (msec) */
	pfc_rbnode_t	etmo_node;	/* Red-Black tree node */
	ipc_etmfunc_t	etmo_func;	/* timeout handler */
};

/* Set true if timeout node is associated with event listener session. */
#define	etmo_elsess	etmo_node.rbn_pad[0]

#define	IPC_ETIMEOUT_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_etimeout_t, etmo_node)

/*
 * Initialize ipc_etimeout_t.
 */
#define	IPC_ETIMEOUT_INIT(etmp, func, is_elsess)	\
	do {						\
		(etmp)->etmo_time = 0;			\
		(etmp)->etmo_func = (func);		\
		(etmp)->etmo_elsess = (is_elsess);	\
	} while (0)

/*
 * Determine whether the given timeout node exists in the timeout tree.
 * This macro must be used with holding the IPC event listener context lock.
 */
#define	IPC_ETIMEOUT_IS_ACTIVE(etmp)	((etmp)->etmo_time != 0)

/*
 * Invalidate timeout node.
 * This macro must be used with holding the IPC event listener context lock.
 */
#define	IPC_ETIMEOUT_INVALIDATE(etmp)		\
	do {					\
		(etmp)->etmo_time = 0;		\
	} while (0)

/*
 * Event listener session instance.
 *
 * Remarks:
 *	- els_tqlist must be serialized by the event task queue lock.
 *	- els_eplist must be serialized by the event listener context lock.
 *	- els_tmplink must be serialized by the IPC channel lock.
 */
struct ipc_elsess {
	ipc_clchan_t	*els_chan;	/* IPC channel */
	pfc_sockaddr_t	els_sockaddr;	/* socket address */
	ipc_evset_t	els_target;	/* target event set */
	ipc_evset_t	els_newtarget;	/* new target event set */
	pfc_rbnode_t	els_node;	/* Red-Black tree node */
	pfc_list_t	els_tqlist;	/* link for task queue */
	pfc_list_t	els_eplist;	/* link for sessions in epoll device */
	ipc_elsess_t	*els_tmplink;	/* link for temporary use */
	pfc_mutex_t	els_mutex;	/* mutex */
	pfc_cond_t	els_cond;	/* condition variable */
	pfc_ephdlr_t	els_epoll;	/* epoll handler */
	ipc_etimeout_t	els_timeout;	/* timeout node */
	ipc_sess_t	els_sess;	/* IPC session */
	int		els_pipe[2];	/* control pipe */
	uint32_t	els_refcnt;	/* reference counter */
	uint32_t	els_nlinks;	/* link counter from handlers */
	uint32_t	els_nwilds;	/* number of wildcard handlers */
};

#define	els_stream		els_sess.iss_stream
#define	els_flags		els_sess.iss_resv1
#define	els_prevdown		els_sess.iss_resv2[0]
#define	els_addr		els_sess.iss_addr

#define	IPC_ELSESS_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_elsess_t, els_node)
#define	IPC_ELSESS_TQLIST2PTR(list)				\
	PFC_CAST_CONTAINER((list), ipc_elsess_t, els_tqlist)
#define	IPC_ELSESS_EPLIST2PTR(list)				\
	PFC_CAST_CONTAINER((list), ipc_elsess_t, els_eplist)
#define	IPC_ELSESS_EP2PTR(ephp)					\
	PFC_CAST_CONTAINER((ephp), ipc_elsess_t, els_epoll)
#define	IPC_ELSESS_ETM2PTR(etmp)				\
	PFC_CAST_CONTAINER((etmp), ipc_elsess_t, els_timeout)

#define	IPC_ELSESS_RDPIPE(elsp)		((elsp)->els_pipe[0])
#define	IPC_ELSESS_WRPIPE(elsp)		((elsp)->els_pipe[1])

#define	IPC_ELSESS_LOCK(elsp)		pfc_mutex_lock(&(elsp)->els_mutex)
#define	IPC_ELSESS_UNLOCK(elsp)		pfc_mutex_unlock(&(elsp)->els_mutex)

#define	IPC_ELSESS_WAIT(elsp)					\
	pfc_cond_wait(&(elsp)->els_cond, &(elsp)->els_mutex)
#define	IPC_ELSESS_SIGNAL(elsp)		pfc_cond_signal(&(elsp)->els_cond)

#define	IPC_ELSESS_IS_SHUTDOWN(elsp)			\
	PFC_EXPECT_FALSE(IPC_ELSESS_WRPIPE(elsp) == -1)
#define	IPC_ELSESS_ON_TASKQ(elsp)			\
	(!pfc_list_is_empty(&(elsp)->els_tqlist))

/*
 * Flags for els_flags.
 */
#define	IPC_ELSF_DOWN_MASK	PFC_CONST_U(0x000f)	/* down code mask */
#define	IPC_ELSF_BUSY		PFC_CONST_U(0x0010)	/* session is busy */
#define	IPC_ELSF_EPOLL		PFC_CONST_U(0x0020)	/* in epoll instance */
#define	IPC_ELSF_REPOST		PFC_CONST_U(0x0040)	/* repost task */
#define	IPC_ELSF_RECV		PFC_CONST_U(0x0080)	/* receive an event */
#define	IPC_ELSF_TARGET_ALL	PFC_CONST_U(0x0100)	/* send all targets */
#define	IPC_ELSF_UPDATE		PFC_CONST_U(0x0200)	/* need to update */
#define	IPC_ELSF_CONNECTING	PFC_CONST_U(0x0400)	/* now connecting */

/*
 * Flags not to be cleared on session reset.
 */
#define	IPC_ELSF_RETAIN_FLAGS	IPC_ELSF_EPOLL

/*
 * Pseudo connection down code which means no error.
 */
#define	IPC_CHDOWN_NONE		PFC_CONST_U(0)

/*
 * Get/Set connection down code to els_flags.
 *
 * Remarks:
 *	Although data type of the connection down code is defined as uint32_t,
 *	IPC event subsystem currently treats it as uint8_t.
 */
#define	IPC_ELSESS_GET_DOWNCODE(elsp)			\
	((elsp)->els_flags & IPC_ELSF_DOWN_MASK)

#define	IPC_ELSESS_SET_DOWNCODE(elsp, code)				\
	do {								\
		PFC_ASSERT(((code) & (~IPC_ELSF_DOWN_MASK)) == 0);	\
		if (IPC_ELSESS_GET_DOWNCODE(elsp) == IPC_CHDOWN_NONE) {	\
			(elsp)->els_flags |= (code);			\
		}							\
	} while (0)

/*
 * IPC event handler's attributes.
 */
typedef struct {
	ipc_hostset_t	*eva_hostset;	/* set of server host addresses */
	pfc_ptr_t	eva_arg;	/* argument to be passed to handler */
	pfc_ipcevdtor_t	eva_argdtor;	/* argument destructor */
	ipc_evset_t	eva_target;	/* set of targeted events */
	uint32_t	eva_priority;	/* priority */
	uint32_t	eva_flags;	/* flags */
} ipc_evattr_t;

PFC_TYPE_SIZE_ASSERT(ipc_evattr_t, __PFC_IPCEVATTR_SIZE);

#define	IPC_EVATTR_PTR(attr)	((ipc_evattr_t *)(attr))

/*
 * Flags for eva_flags.
 */
#define	IPC_EVATTRF_LOG		PFC_CONST_U(0x1)	/* delivery logging */
#define	IPC_EVATTRF_CXX		PFC_CONST_U(0x2)	/* C++ handler */

/*
 * Prototype of private destructor for pfc_ipcevent_t.
 */
typedef void	(*ipc_eventdtor_t)(pfc_ptr_t arg);

/*
 * IPC event object.
 */
struct __pfc_ipcevent {
	ipc_evproto_t	ie_proto;	/* common event data */
	pfc_refptr_t	*ie_chan;	/* IPC channel name */
	pfc_hostaddr_t	ie_addr;	/* address of IPC server */
	pfc_ipcsess_t	ie_sess;	/* dummy session */
	ipc_eventdtor_t	ie_dtor;	/* destructor */
	pfc_ptr_t	ie_data;	/* private data */
	uint32_t	ie_refcnt;	/* reference counter */
};

/*
 * Accessor of IPC event object.
 */
#define	IPC_EVENT_SERIAL(event)		((event)->ie_proto.iev_serial)
#define	IPC_EVENT_TYPE(event)		((event)->ie_proto.iev_type)
#define	IPC_EVENT_SERVICE(event)				\
	pfc_refptr_string_value((event)->ie_sess.icss_name)
#define	IPC_EVENT_SERVICE_NAMELEN(event) ((event)->ie_proto.iev_namelen)

/*
 * IPC event handler.
 */
typedef struct {
	pfc_ipcevhdlr_t	evh_id;		/* event handler's ID */
	uint32_t	evh_priority;	/* priority */
	uint32_t	evh_refcnt;	/* reference counter */
	uint32_t	evh_flags;	/* flags */
	pfc_refptr_t	*evh_name;	/* name of this handler */
	ipc_clchan_t	*evh_chan;	/* IPC channel */
	pfc_list_t	evh_list;	/* IPC channel link */
	pfc_ipcevfunc_t	evh_func;	/* event handler */
	pfc_ptr_t	evh_arg;	/* argument for event handler */
	pfc_ipcevdtor_t	evh_argdtor;	/* argument destructor */
	ipc_hostset_t	*evh_hostset;	/* target host set */
	ipc_evset_t	evh_target;	/* target event set */
	pfc_rbnode_t	evh_node;	/* Red-Black tree node for ID */
	pfc_mutex_t	evh_mutex;	/* mutex for this handler */
	pfc_cond_t	evh_cond;	/* condition variable */
	uint64_t	evh_delivered;	/* number of delivered events */
} ipc_evhdlr_t;

#define	IPC_EVHDLR_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_evhdlr_t, evh_node)
#define	IPC_EVHDLR_KEY(id)	((pfc_cptr_t)(uintptr_t)(id))

#define	IPC_EVHDLR_LIST2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_evhdlr_t, evh_list)

#define	IPC_EVHDLR_LOCK(ehp)	pfc_mutex_lock(&(ehp)->evh_mutex)
#define	IPC_EVHDLR_UNLOCK(ehp)	pfc_mutex_unlock(&(ehp)->evh_mutex)

#define	IPC_EVHDLR_BROADCAST(ehp)	pfc_cond_signal(&(ehp)->evh_cond)
#define	IPC_EVHDLR_TIMEDWAIT_ABS(ehp, abstime)				\
	pfc_cond_timedwait_abs(&(ehp)->evh_cond, &(ehp)->evh_mutex, (abstime))

/*
 * Flags for evh_flags.
 */
#define	IPC_EVHDF_LOG		PFC_CONST_U(0x1)	/* delivery logging */
#define	IPC_EVHDF_REMOVED	PFC_CONST_U(0x2)	/* already removed */
#define	IPC_EVHDF_ACTIVE	PFC_CONST_U(0x4)	/* being called */

#define	IPC_EVHDLR_IS_REMOVED(ehp)				\
	PFC_EXPECT_FALSE((ehp)->evh_flags & IPC_EVHDF_REMOVED)
#define	IPC_EVHDLR_NEEDS_LOG(ehp)					\
	(ipcclnt_log_enabled && ((ehp)->evh_flags & IPC_EVHDF_LOG))

/*
 * IPC event queue entry.
 */
struct ipc_clevqent;
typedef struct ipc_clevqent	ipc_clevqent_t;

struct ipc_clevqent {
	pfc_ipcevent_t	*ceq_event;	/* event object */
	ipc_evhdlr_t	*ceq_handler;	/* event handler */
	ipc_clevqent_t	*ceq_next;	/* next entry */
};

/*
 * IPC event dispatch queue.
 */
typedef struct {
	pfc_mutex_t	ed_mutex;	/* mutex */
	pfc_cond_t	ed_cond;	/* condition variable */
	ipc_clevqent_t	*ed_queue;	/* head of event queue */
	ipc_clevqent_t	**ed_qnext;	/* new queue entry link */
	uint32_t	ed_flags;	/* flags */
	int		ed_ctorerr;	/* return value of constructor */
} ipc_evdisp_t;

#define	IPC_EVDISP_LOCK(disp)		pfc_mutex_lock(&(disp)->ed_mutex)
#define	IPC_EVDISP_TRYLOCK(disp)	pfc_mutex_trylock(&(disp)->ed_mutex)
#define	IPC_EVDISP_UNLOCK(disp)		pfc_mutex_unlock(&(disp)->ed_mutex)

#define	IPC_EVDISP_WAIT(disp)					\
	pfc_cond_wait(&(disp)->ed_cond, &(disp)->ed_mutex)
#define	IPC_EVDISP_TIMEDWAIT_ABS(disp, abstime)				\
	pfc_cond_timedwait_abs(&(disp)->ed_cond, &(disp)->ed_mutex, (abstime))
#define	IPC_EVDISP_SIGNAL(disp)		pfc_cond_signal(&(disp)->ed_cond)

/*
 * Flags for ed_flags.
 */
#define	IPC_EVDISPF_SHUTDOWN	PFC_CONST_U(0x1)	/* shutdown */
#define	IPC_EVDISPF_DONE	PFC_CONST_U(0x2)	/* end of service */
#define	IPC_EVDISPF_INIT	PFC_CONST_U(0x4)	/* initialized */

#define	IPC_EVDISP_IS_SHUTDOWN(disp)					\
	PFC_EXPECT_FALSE((disp)->ed_flags & IPC_EVDISPF_SHUTDOWN)
#define	IPC_EVDISP_IS_DONE(disp)	((disp)->ed_flags & IPC_EVDISPF_DONE)
#define	IPC_EVDISP_IS_INITIALIZED(disp)	((disp)->ed_flags & IPC_EVDISPF_INIT)

/*
 * Task queue for event session work.
 */
typedef struct {
	uint32_t	etq_maxthreads;		/* max number of threads */
	uint32_t	etq_nthreads;		/* current number of threads */
	uint32_t	etq_nwaiting;		/* number of waiting threads */
	uint32_t	etq_flags;		/* flags */
	pfc_list_t	etq_queue;		/* queue head */
	pfc_mutex_t	etq_mutex;		/* mutex */
	pfc_cond_t	etq_cond;		/* condition variable */
	ipc_etimeout_t	etq_timeout;		/* timeout node */
} ipc_evtaskq_t;

#define	IPC_EVTASKQ_ETM2PTR(etmp)				\
	PFC_CAST_CONTAINER((etmp), ipc_evtaskq_t, etq_timeout)

#define	IPC_EVTASKQ_LOCK(tqp)		pfc_mutex_lock(&(tqp)->etq_mutex)
#define	IPC_EVTASKQ_UNLOCK(tqp)		pfc_mutex_unlock(&(tqp)->etq_mutex)

#define	IPC_EVTASKQ_TIMEDWAIT(tqp, timeout)				\
	pfc_cond_timedwait(&(tqp)->etq_cond, &(tqp)->etq_mutex, (timeout))
#define	IPC_EVTASKQ_TIMEDWAIT_ABS(tqp, abstime)				\
	pfc_cond_timedwait_abs(&(tqp)->etq_cond, &(tqp)->etq_mutex, (abstime))
#define	IPC_EVTASKQ_SIGNAL(tqp)		pfc_cond_signal(&(tqp)->etq_cond)
#define	IPC_EVTASKQ_BROADCAST(tqp)	pfc_cond_broadcast(&(tqp)->etq_cond)

/*
 * Flags for etq_flags.
 */
#define	IPC_EVTQF_SHUTDOWN	PFC_CONST_U(0x1)	/* shutdown */

#define	IPC_EVTASKQ_IS_SHUTDOWN(tqp)				\
	PFC_EXPECT_FALSE((tqp)->etq_flags & IPC_EVTQF_SHUTDOWN)

/*
 * IPC event listener context.
 */
typedef struct {
	pthread_t		ev_thread;	/* listener thread */
	pfc_mutex_t		ev_mutex;	/* mutex */
	pfc_rbtree_t		ev_timeout;	/* timeout tree */
	pfc_ephdlr_t		ev_ephdlr;	/* epoll handler for ev_pipe */
	epoll_event_t		*ev_epevents;	/* epoll event buffer */
	pfc_list_t		ev_epsess;	/* sessions in epoll device */
	uint32_t		ev_flags;	/* flags */
	int			ev_epoll;	/* epoll instance FD */
	int			ev_pipe[2];	/* pipe FDs for internal use */
} ipc_evctx_t;

#define	IPC_EVCTX_RDPIPE(ectx)		((ectx)->ev_pipe[0])
#define	IPC_EVCTX_WRPIPE(ectx)		((ectx)->ev_pipe[1])

#define	IPC_EVCTX_LOCK(ectx)		pfc_mutex_lock(&(ectx)->ev_mutex)
#define	IPC_EVCTX_UNLOCK(ectx)		pfc_mutex_unlock(&(ectx)->ev_mutex)

/*
 * Flags for ev_flags.
 */
#define	IPC_EVCTXF_DONE		PFC_CONST_U(0x1)	/* end of service */

#define	IPC_EVCTX_IS_DONE(ectx)					\
	PFC_EXPECT_FALSE((ectx)->ev_flags & IPC_EVCTXF_DONE)

/*
 * Operations for the IPC event subsystem.
 * This is used as an interface for other language bindings.
 */
typedef struct {
	/*
	 * int
	 * eop_disp_ctor(void)
	 *	Constructor of the IPC event dispatcher thread.
	 *
	 *	This function is called on the IPC event dispatcher thread
	 *	when it has been started.
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, zero must be returned.
	 *	Otherwise error number defined in errno.h must be returned.
	 *
	 * Remarks:
	 *	pfc_ipcclnt_event_init() will wait for completion of this
	 *	function. If this function returns a non-zero value, it is
	 *	returned by pfc_ipcclnt_event_init().
	 */
	int	(*eop_disp_ctor)(void);

	/*
	 * void
	 * eop_disp_dtor(void)
	 *	Destructor of the IPC event dispatcher thread.
	 *
	 *	This function is called on the IPC event dispatcher thread
	 *	just before it exits.
	 *
	 * Remarks:
	 *	- This function is never called if eop_disp_ctor() failed.
	 *
	 *	- IPC event subsystem operation is always uninstalled before
	 *	  eop_disp_dtor() is called.
	 */
	void	(*eop_disp_dtor)(void);
} ipc_evsysops_t;

typedef const ipc_evsysops_t	ipc_cevsysops_t;

#ifdef	_PFC_LIBPFC_IPCCLNT_BUILD

extern ipc_evstate_t	ipc_event_state;
extern ipc_evattr_t	ipc_evattr_default;
extern ipc_hostset_t	ipc_hostset_local;
extern pfc_refptr_t	*ipc_svname_chstate;

/*
 * Internal prototypes.
 */
extern int	pfc_ipcclnt_event_state_error(ipc_evstate_t state);
extern void	pfc_ipcclnt_event_fork_prepare(void);
extern void	pfc_ipcclnt_event_fork_parent(void);
extern void	pfc_ipcclnt_event_fork_child(void);

extern void	pfc_ipcclnt_evchan_init(ipc_clchan_t *chp);
extern int	pfc_ipcclnt_evchan_addhost(ipc_clchan_t *PFC_RESTRICT chp,
					   ipc_hostset_t *PFC_RESTRICT hset,
					   const pfc_hostaddr_t *PFC_RESTRICT
					   addr);
extern void	pfc_ipcclnt_evchan_removehost(ipc_clchan_t *PFC_RESTRICT chp,
					      ipc_hostset_t *PFC_RESTRICT hset,
					      const pfc_hostaddr_t *PFC_RESTRICT
					      addr);

extern int	pfc_ipcclnt_hostset_init(void);
extern void	pfc_ipcclnt_hostset_fini(void);
extern int	pfc_ipcclnt_hostset_lookup(const char *name,
					   ipc_hostset_t **hsetp);
extern void	pfc_ipcclnt_hostset_destroy_impl(ipc_hostset_t *hset);

extern void	pfc_ipcevent_attr_sysinit(void);

extern int	pfc_ipcevent_create(pfc_ipcevent_t **PFC_RESTRICT eventp,
				    ipc_elsess_t *PFC_RESTRICT elsp,
				    const char *PFC_RESTRICT svname);
extern int	pfc_ipcevent_create_chstate(pfc_ipcevent_t **PFC_RESTRICT
					    eventp,
					    ipc_elsess_t *PFC_RESTRICT elsp,
					    pfc_ipcevtype_t type);
extern void	pfc_ipcevent_destroy(pfc_ipcevent_t *event);

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * ipc_event_is_running(void)
 *	Ensure that the IPC event subsystem is running.
 *
 * Calling/Exit State:
 *	Zero is returned if the IPC event subsystem is running.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in reader
 *	or writer mode.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
ipc_event_is_running(void)
{
	ipc_evstate_t	state = ipc_event_state;

	if (PFC_EXPECT_FALSE(state != IPC_EVSTATE_RUNNING)) {
		return pfc_ipcclnt_event_state_error(state);
	}

	return 0;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * ipc_event_is_available(void)
 *	Ensure that the IPC event subsystem is still available.
 *
 * Calling/Exit State:
 *	Zero is returned if the IPC event subsystem is available.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in reader
 *	or writer mode.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
ipc_event_is_available(void)
{
	ipc_evstate_t	state = ipc_event_state;

	if (PFC_EXPECT_FALSE(state != IPC_EVSTATE_RUNNING &&
			     state != IPC_EVSTATE_SHUTDOWN)) {
		return pfc_ipcclnt_event_state_error(state);
	}

	return 0;
}

/*
 * static inline const char PFC_FATTR_ALWAYS_INLINE *
 * ipc_evhdlr_name(ipc_evhdlr_t *ehp)
 *	Return a pointer to the name of the specified event handler.
 */
static inline const char PFC_FATTR_ALWAYS_INLINE *
ipc_evhdlr_name(ipc_evhdlr_t *ehp)
{
	pfc_refptr_t	*rname = ehp->evh_name;

	if (rname == NULL) {
		return ipc_str_null;
	}

	return pfc_refptr_string_value(rname);
}

/*
 * Hold/Release IPC event object.
 */
#define	IPC_EVENT_HOLD(event)	pfc_atomic_inc_uint32(&(event)->ie_refcnt)
#define	IPC_EVENT_RELEASE(event)					\
	do {								\
		uint32_t	__refcnt =				\
			pfc_atomic_dec_uint32_old(&(event)->ie_refcnt);	\
									\
		if (__refcnt == 1) {					\
			pfc_ipcevent_destroy(event);			\
		}							\
		else {							\
			PFC_ASSERT(__refcnt != 0);			\
		}							\
	} while (0)

/*
 * Hold/Release IPC host set.
 * IPC_HOSTSET_HOLD() must be used with holding the client lock in reader or
 * writer mode.
 */
#define	IPC_HOSTSET_HOLD(hset)						\
	do {								\
		PFC_ASSERT((hset)->hs_refcnt != 0);			\
		pfc_atomic_inc_uint32(&(hset)->hs_refcnt);		\
	} while (0)

#define	IPC_HOSTSET_RELEASE(hset)					\
	do {								\
		uint32_t	__refcnt =				\
			pfc_atomic_dec_uint32_old(&(hset)->hs_refcnt);	\
									\
		if (__refcnt == 1) {					\
			pfc_ipcclnt_hostset_destroy_impl(hset);		\
		}							\
		else {							\
			PFC_ASSERT(__refcnt != 0);			\
		}							\
	} while (0)

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * ipc_elsess_conn_equals(ipc_elsess_t *elsp, ipc_conn_t *cnp)
 *	Determine whether specified IPC event listener session and
 *	IPC connection target the same IPC server or not.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
ipc_elsess_conn_equals(ipc_elsess_t *elsp, ipc_conn_t *cnp)
{
	return (elsp->els_chan == cnp->icn_chan &&
		pfc_hostaddr_compare(&elsp->els_addr, &cnp->icn_addr) == 0);
}

#endif	/* _PFC_LIBPFC_IPCCLNT_BUILD */

/*
 * Prototypes.
 */
extern int	pfc_ipcclnt_evsysops_install(ipc_cevsysops_t *ops);
extern void	pfc_ipcclnt_evsysops_uninstall(ipc_cevsysops_t *ops);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcevent_destroy_impl(pfc_ipcevent_t *event)
 *	Destroy the specified IPC event object.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcevent_destroy_impl(pfc_ipcevent_t *event)
{
	pfc_ipcsess_t	*sess = &event->ie_sess;

	PFC_ASSERT(event->ie_refcnt == 0);
	PFC_ASSERT(IPC_CLSESS_ISEVENT(sess));
	PFC_ASSERT(sess->icss_conn == NULL);
	PFC_ASSERT(sess->icss_canceller == NULL);
#ifdef	PFC_VERBOSE_DEBUG
	PFC_ASSERT_INT(pfc_mutex_destroy(&sess->icss_mutex), 0);
	event->ie_refcnt = (uint32_t)-1;
#endif	/* PFC_VERBOSE_DEBUG */

	pfc_ipcstream_destroy(&sess->icss_output);
	pfc_ipcmsg_destroy(&sess->icss_msg);
	pfc_refptr_put(sess->icss_name);

	pfc_refptr_put(event->ie_chan);
	free(event);
}

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_IPCCLNT_IPCCLNT_EVENT_H */
