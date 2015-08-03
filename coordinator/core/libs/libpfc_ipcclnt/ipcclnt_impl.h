/*
 * Copyright (c) 2011-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_IPCCLNT_IPCCLNT_IMPL_H
#define	_PFC_LIBPFC_IPCCLNT_IPCCLNT_IMPL_H

/*
 * Internal definitions for the PFC IPC client management.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/un.h>
#include <pfc/ipc_client.h>
#include <pfc/refptr.h>
#include <pfc/socket.h>
#include <pfc/synch.h>
#include <pfc/rbtree.h>
#include <pfc/list.h>
#include <pfc/atomic.h>
#include <pfc/debug.h>
#include <pfc/hostaddr.h>
#include <pfc/log.h>
#include <ipc_impl.h>

PFC_C_BEGIN_DECL

struct ipc_chmon;
typedef struct ipc_chmon	ipc_chmon_t;

struct ipc_evchan;
typedef struct ipc_evchan	ipc_evchan_t;

/*
 * Forward declaration of IPC event listener session structure.
 */
struct ipc_elsess;
typedef struct ipc_elsess	ipc_elsess_t;

/*
 * Separator of IPC channel name and address part in the IPC channel address.
 */
#define	IPC_CHANNEL_ADDR_SEP	'@'

/*
 * Red-Black tree initializer with cast.
 */
#define	IPC_RBTREE_INITIALIZER(comp, keyfunc)			\
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)(comp), (keyfunc))

/*
 * IPC channel information for client.
 *
 * Remarks:
 *	- Access to the following fields must be serialized by ichc_lock.
 *	  + ichc_elsess
 *	  + ichc_evhostset
 *	  + ichc_evhandler
 */
typedef struct {
	pfc_refptr_t	*ichc_name;	/* channel name */
	pfc_refptr_t	*ichc_path;	/* UNIX domain socket path */
	pfc_rbnode_t	ichc_node;	/* Red-Black tree node */
	pfc_rwlock_t	ichc_lock;	/* read-write lock */
	pfc_rbtree_t	ichc_elsess;	/* event listener session tree */
	pfc_rbtree_t	ichc_evhostset;	/* host set for event listener */
	pfc_list_t	ichc_evhandler;	/* event handlers */
	uint32_t	ichc_refcnt;	/* reference counter */
	uint32_t	ichc_timeout;	/* client session timeout in seconds */
} ipc_clchan_t;

#define	IPC_CLCHAN_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_clchan_t, ichc_node)
#define	IPC_CLCHAN_NAME(chp)				\
	pfc_refptr_string_value((chp)->ichc_name)

#define	IPC_CLCHAN_RDLOCK(chp)					\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&(chp)->ichc_lock), 0)
#define	IPC_CLCHAN_WRLOCK(chp)					\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&(chp)->ichc_lock), 0)
#define	IPC_CLCHAN_UNLOCK(chp)					\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&(chp)->ichc_lock), 0)

#define	IPC_CLCHAN_TRYWRLOCK(chp)	pfc_rwlock_trywrlock(&(chp)->ichc_lock)

#define	IPC_CLCHAN_HOLD(chp)	pfc_atomic_inc_uint32(&(chp)->ichc_refcnt)
#define	IPC_CLCHAN_RELEASE(chp)						\
	do {								\
		uint32_t	__ref =					\
			pfc_atomic_dec_uint32_old(&(chp)->ichc_refcnt);	\
									\
		if (__ref == 1) {					\
			pfc_ipcclnt_chan_destroy(chp);			\
		}							\
		else {							\
			PFC_ASSERT(__ref > 0);				\
		}							\
	} while (0)

/*
 * IPC channel iterator.
 */
typedef void	(*ipc_clchiter_t)(ipc_clchan_t *chp, pfc_ptr_t arg);

/*
 * Canceller operations.
 */
typedef struct ipc_canceller	ipc_canceller_t;

struct ipc_clops;
typedef const struct ipc_clops	ipc_clops_t;

struct ipc_clops {
	/*
	 * int
	 * clops_testcancel(ipc_canceller_t *clp)
	 *	Determine whether the cancellation is already notified to the
	 *	given canceller or not.
	 *
	 * Calling/Exit State:
	 *	ECANCELED is returned if the cancellation is already notified
	 *	to the given canceller. Otherwise zero is returned.
	 *
	 * Remarks:
	 *	This function is used to terminate busy wait on the IPC
	 *	connection. So it may be called with holding the connection
	 *	lock associated with the given canceller.
	 */
	int		(*clops_testcancel)(ipc_canceller_t *clp);

	/*
	 * pfc_ipcsess_t *
	 * clops_getsess(ipc_canceller_t *clp)
	 *	Return a pointer to the IPC client session associated with
	 *	the given canceller.
	 *
	 * Calling/Exit State:
	 *	A non-NULL pointer to the IPC client session associated with
	 *	the given canceller is returned.
	 *
	 * Remarks:
	 *	NULL is set to this field if the canceller is not
	 *	session-specific canceller.
	 */
	pfc_ipcsess_t	*(*clops_getsess)(ipc_canceller_t *clp);
};

/*
 * File descriptor for cancellation.
 */
struct ipc_canceller {
	int		icl_pipe[2];	/* pipe for cancellation */
	uint32_t	icl_refcnt;	/* reference counter */
	pfc_list_t	icl_list;	/* link for canceller list */
	ipc_clops_t	*icl_ops;	/* canceller operations. */
};

#define	icl_watch_fd	icl_pipe[0]
#define	icl_notify_fd	icl_pipe[1]

#define	IPC_CANCELLER_LIST2PTR(list)				\
	PFC_CAST_CONTAINER((list), ipc_canceller_t, icl_list)

#define	IPC_CANCELLER_HOLD(clp)	pfc_atomic_inc_uint32(&(clp)->icl_refcnt)
#define	IPC_CANCELLER_RELEASE(clp)					\
	do {								\
		uint32_t	__ref =					\
			pfc_atomic_dec_uint32_old(&(clp)->icl_refcnt);	\
									\
		if (__ref == 1) {					\
			pfc_ipcclnt_canceller_destroy(clp);		\
		}							\
		else {							\
			PFC_ASSERT(__ref > 0);				\
		}							\
	} while (0)

#define	IPC_CANCELLER_RELEASE_FINI(clp)					\
	do {								\
		uint32_t	__ref =					\
			pfc_atomic_dec_uint32_old(&(clp)->icl_refcnt);	\
									\
		if (__ref == 1) {					\
			pfc_ipcclnt_canceller_destroy_fini(clp);	\
		}							\
		else {							\
			PFC_ASSERT(__ref > 0);				\
		}							\
	} while (0)

#define	IPC_CANCELLER_ISACTIVE(clp)	((clp)->icl_notify_fd != -1)

#define	IPC_CANCELLER_GETSESS(clp)					\
	(((clp)->icl_ops->clops_getsess == NULL) ? NULL			\
	 : (clp)->icl_ops->clops_getsess(clp))

/*
 * IPC session specific canceller instance.
 *
 * Remarks:
 *	iscl_canceller must be the first field.
 */
typedef struct {
	ipc_canceller_t		iscl_canceller;		/* common data */
	pfc_ipcsess_t		*iscl_sess;		/* IPC session */
} ipc_sessclr_t;

#define	IPC_CLP2SCLP(clp)						\
	PFC_CAST_CONTAINER((clp), ipc_sessclr_t, iscl_canceller)

/*
 * IPC channel address.
 */
typedef struct {
	/* IPC channel name. */
	char		ica_name[IPC_CHANNEL_NAMELEN_MAX + 1];

	/* IPC server's host address. */
	pfc_hostaddr_t	ica_host;
} ipc_chaddr_t;

struct ipc_conn;
typedef struct ipc_conn		ipc_conn_t;

/*
 * Alternative connection pool instance.
 */
typedef struct {
	pfc_ipccpool_t	icp_id;		/* pool identifier */
	uint32_t	icp_size;	/* number of connections in the pool */
	uint32_t	icp_capacity;	/* capacity of the pool */
	pfc_mutex_t	icp_mutex;	/* mutex */
	pfc_rbtree_t	icp_pool;	/* connection pool */
	pfc_list_t	icp_lrulist;	/* LRU list of cached connections */
	pfc_list_t	icp_uncached;	/* list of uncached connections */
	pfc_rbnode_t	icp_node;	/* Red-Black tree node */
} ipc_cpool_t;

#define	IPC_CPOOL_KEY(id)	((pfc_cptr_t)(uintptr_t)(id))
#define	IPC_CPOOL_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_cpool_t, icp_node)

/*
 * The connection pool lock is used to create exclusive lock section while
 * reader lock of the client lock is held. The libpfc_ipcclnt assumes that
 * it is always held with holding the client lock.
 */
#define	IPC_CPOOL_LOCK(cplp)		pfc_mutex_lock(&(cplp)->icp_mutex)
#define	IPC_CPOOL_UNLOCK(cplp)		pfc_mutex_unlock(&(cplp)->icp_mutex)

/*
 * Connection pool iterator.
 */
typedef void	(*ipc_cpooliter_t)(ipc_cpool_t *cplp, pfc_ptr_t arg);

/*
 * Alternative connection pool entry.
 *
 * Remarks:
 *	- icpe_flags must be protected by either of the following ways:
 *	  + The client lock in writer mode.
 *	  + The client lock in reader mode, and the connection pool lock.
 */
typedef struct {
	ipc_chaddr_t	icpe_addr;	/* IPC channel address */
	uint32_t	icpe_refcnt;	/* reference counter */
	uint32_t	icpe_flags;	/* flags */
	ipc_conn_t	*icpe_conn;	/* connection handle */
	pfc_rbnode_t	icpe_node;	/* Red-Black tree node */
} ipc_cpoolent_t;

#define	IPC_CPOOLENT_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_cpoolent_t, icpe_node)

/*
 * Flags for icpe_flags.
 */
#define	IPC_CPENTF_AGED		PFC_CONST_U(0x1)	/* aged flag */

/*
 * Valid flags for pfc_ipcclnt_cpool_close().
 */
#define	IPC_CPOOL_CLOSE_FLAGS		PFC_IPCPLF_C_FORCE

/*
 * IPC client connection handle.
 *
 * Remarks:
 *	- icn_pool and icn_poolent must be protected by the client lock.
 *	- icn_plink must be protected by either of the following ways:
 *	  + The client lock in writer mode.
 *	  + The client lock in reader mode, and the connection pool lock
 *	    associated with icn_pool.
 */
struct ipc_conn {
	pfc_ipcconn_t	icn_id;		/* connection identifier */
	uint32_t	icn_refcnt;	/* reference counter */
	union {
		pfc_rbnode_t	node;	/* Red-Black tree node */
		pfc_list_t	list;	/* link for dead connections */
	} icn_link;
	pfc_sockaddr_t	icn_sockaddr;	/* socket address */
	ipc_clchan_t	*icn_chan;	/* IPC channel */
	pfc_ipcsess_t	*icn_current;	/* currently ongoing session */
	ipc_canceller_t	*icn_canceller;	/* session canceller */
	ipc_cpool_t	*icn_pool;	/* connection pool */
	ipc_cpoolent_t	*icn_poolent;	/* connection pool entry */
	pfc_list_t	icn_plink;	/* link for connection pool */
	pfc_list_t	icn_sessions;	/* active sessions */
	pfc_mutex_t	icn_mutex;	/* mutex */
	pfc_cond_t	icn_cond;	/* condition variable */
	ipc_sess_t	icn_sess;	/* client session handle */
};

#define	icn_stream	icn_sess.iss_stream
#define	icn_addr	icn_sess.iss_addr
#define	icn_textaddr	icn_sess.iss_textaddr
#define	icn_flags	icn_sess.iss_resv1
#define	icn_node	icn_link.node
#define	icn_list	icn_link.list

#define	IPC_CONN_KEY(id)	((pfc_cptr_t)(uintptr_t)(id))
#define	IPC_CONN_NODE2PTR(node)					\
	PFC_CAST_CONTAINER((node), ipc_conn_t, icn_node)
#define	IPC_CONN_LIST2PTR(list)					\
	PFC_CAST_CONTAINER((list), ipc_conn_t, icn_list)
#define	IPC_CONN_PLINK2PTR(list)				\
	PFC_CAST_CONTAINER((list), ipc_conn_t, icn_plink)

#define	IPC_CONN_LOCK(cnp)	pfc_mutex_lock(&(cnp)->icn_mutex)
#define	IPC_CONN_TRYLOCK(cnp)	pfc_mutex_trylock(&(cnp)->icn_mutex)
#define	IPC_CONN_UNLOCK(cnp)	pfc_mutex_unlock(&(cnp)->icn_mutex)

#define	IPC_CONN_SIGNAL(cnp)	pfc_cond_signal(&(cnp)->icn_cond)
#define	IPC_CONN_BROADCAST(cnp)	pfc_cond_broadcast(&(cnp)->icn_cond)

#define	IPC_CONN_TIMEDWAIT_ABS(cnp, abstime)				\
	pfc_cond_timedwait_abs(&(cnp)->icn_cond, &(cnp)->icn_mutex, (abstime))

#define	IPC_CONN_HOLD(cnp)	pfc_atomic_inc_uint32(&(cnp)->icn_refcnt)

/*
 * Remarks:
 *	IPC_CONN_RELEASE() must be used without holding the client lock
 *	because pfc_ipcclnt_conn_destroy() may acquire it.
 */
#define	IPC_CONN_RELEASE(cnp)						\
	do {								\
		uint32_t	__ref =					\
			pfc_atomic_dec_uint32_old(&(cnp)->icn_refcnt);	\
									\
		if (__ref == 1) {					\
			pfc_ipcclnt_conn_destroy(cnp);			\
		}							\
		else {							\
			PFC_ASSERT(__ref > 0);				\
		}							\
	} while (0)

/*
 * Flags for icn_flags.
 */
#define	IPCONNF_CLOSED		PFC_CONST_U(0x0001)	/* already closed */
#define	IPCONNF_DEAD		PFC_CONST_U(0x0002)	/* on dead list */

/*
 * IPC connection iterator.
 */
typedef void	(*ipc_conniter_t)(ipc_conn_t *cnp, pfc_ptr_t arg);

/*
 * Client session state.
 */
typedef enum {
	IPC_SSTATE_READY	= 0,	/* Ready to issue service request. */
	IPC_SSTATE_BUSY,		/* Session is busy. */
	IPC_SSTATE_RESULT,		/* Session keeps command result. */
	IPC_SSTATE_DISCARD,		/* Session must be discarded. */
} ipc_sstate_t;

/*
 * IPC client session.
 */
struct __pfc_ipcsess {
	ipc_conn_t	*icss_conn;		/* connection handle */
	pfc_mutex_t	icss_mutex;		/* mutex */
	pfc_refptr_t	*icss_name;		/* IPC service name */
	pfc_ipcid_t	icss_service;		/* IPC service ID */
	uint32_t	icss_flags;		/* flags */
	uint32_t	icss_cflags;		/* creation flags (immutable) */
	ipc_sstate_t	icss_state;		/* session state */
	uint32_t	icss_busy;		/* busy counter */
	pfc_timespec_t	icss_timeout;		/* session timeout */
	pfc_list_t	icss_list;		/* session list */
	ipc_msg_t	icss_msg;		/* received IPC message */
	ipc_stream_t	icss_output;		/* output stream */
	ipc_sessclr_t	*icss_canceller;	/* session-specific canceller */
};

#define	IPC_CLSESS_LIST2PTR(list)					\
	PFC_CAST_CONTAINER((list), pfc_ipcsess_t, icss_list)

#define	IPC_CLSESS_LOCK(sess)		pfc_mutex_lock(&(sess)->icss_mutex)
#define	IPC_CLSESS_TRYLOCK(sess)	pfc_mutex_trylock(&(sess)->icss_mutex)
#define	IPC_CLSESS_UNLOCK(sess)		pfc_mutex_unlock(&(sess)->icss_mutex)

/*
 * Handle busy counter of the client session.
 * These macros need to be used with holding the session lock.
 */
#define	IPC_CLSESS_SET_BUSY(sess)		\
	do {					\
		(sess)->icss_busy++;		\
	} while (0)

#define	IPC_CLSESS_CLEAR_BUSY(sess)			\
	do {						\
		PFC_ASSERT((sess)->icss_busy > 0);	\
		(sess)->icss_busy--;			\
	} while (0)

#define	IPC_CLSESS_ISBUSY(sess)		((sess)->icss_busy != 0)

/*
 * Flags for icss_flags.
 */
#define	IPC_CLSSF_NOTIMEOUT	PFC_CONST_U(0x1)	/* no timeout */
#define	IPC_CLSSF_HASTIMEOUT	PFC_CONST_U(0x2)	/* has timeout */
#define	IPC_CLSSF_FROZEN	PFC_CONST_U(0x4)	/* frozen session */

#define	IPC_CLSESS_ISFROZEN(sess)				\
	PFC_EXPECT_FALSE((sess)->icss_flags & IPC_CLSSF_FROZEN)

/*
 * Internal session creation flags.
 */
#define	IPC_CLSESS_CF_EVENT	PFC_CONST_U(0x80000000)	/* event session */

#define	IPC_CLSESS_ISEVENT(sess)			\
	((sess)->icss_cflags & IPC_CLSESS_CF_EVENT)

/*
 * Valid session creation flags.
 */
#define	IPC_CLSESS_CF_CANCEL_FLAGS				\
	(PFC_IPCSSF_CANCELABLE | PFC_IPCSSF_NOGLOBCANCEL)
#define	IPC_CLSESS_CF_VALID_FLAGS	IPC_CLSESS_CF_CANCEL_FLAGS

/*
 * Determine whether the given IPC session requires session-specific
 * canceller instance.
 */
#define	IPC_CLSESS_NEED_SPECCANCEL(sess)			\
	((sess)->icss_cflags & PFC_IPCSSF_CANCELABLE)

/*
 * Determine whether the given IPC session requires session-specific
 * canceller instance which accepts global cancellation request.
 */
#define	IPC_CLSESS_ACCEPT_GLOBCANCEL(sess)			\
	(((sess)->icss_cflags & IPC_CLSESS_CF_CANCEL_FLAGS) ==	\
	 PFC_IPCSSF_CANCELABLE)

/*
 * A set of parameters used for cancellation of client sessions associated
 * with a connection.
 */
typedef struct {
	/*
	 * A pointer to IPC event listener session used for auto-cancellation.
	 * If a non-NULL value is specified, IPC client sessions will be
	 * canceled only if the target server matches the specified event
	 * listener session.
	 */
	ipc_elsess_t	*icln_elsess;

	/*
	 * If PFC_TRUE is specified, IPC client sessions are discarded just
	 * after cancellation.
	 */
	pfc_bool_t	icln_discard;
} ipc_clnotify_t;

/*
 * A pointer to pseudo event listener session that indicates all client
 * sessions should be canceled.
 */
#define	IPC_CLNOTIFY_FORCE	((ipc_elsess_t *)(uintptr_t)-1)

/*
 * Macros to initialize ipc_clnotify_t.
 */

#define	IPC_CLNOTIFY_INIT(clnp, elsess, discard)	\
	do {						\
		(clnp)->icln_elsess = (elsess);		\
		(clnp)->icln_discard = (discard);	\
	} while (0)

/* Indicate global cancellation. */
#define	IPC_CLNOTIFY_INIT_GLOBAL(clnp, discard)	\
	IPC_CLNOTIFY_INIT(clnp, NULL, discard)

/* Indicate that all sessions should be canceled by force. */
#define	IPC_CLNOTIFY_INIT_FORCE(clnp, discard)			\
	IPC_CLNOTIFY_INIT(clnp, IPC_CLNOTIFY_FORCE, discard)

#ifdef	_PFC_LIBPFC_IPCCLNT_BUILD

extern const char	ipc_str_empty[];
extern const char	ipc_str_null[];
extern const char	ipc_str_any[];
extern const char	ipc_str_svname_chstate[];
extern const char	ipc_str_chaddr[];
extern const char	ipc_str_hostset[];
extern pfc_refptr_t	*ipcclnt_procname;
extern ipc_option_t	ipcclnt_option;

/*
 * Global client lock.
 *
 * Lock hierarchy is described below:
 *
 *   1.  Global client lock. (IPC_CLIENT_{RD,WR}LOCK())
 *   2.  IPC channel lock. (IPC_CLCHAN_{RD,WR}LOCK())
 *   3.  Host set lock. (IPC_HOSTSET_{RD,WR}LOCK())
 *   4.  Event listener session lock. (IPC_ELSESS_LOCK())
 *   5.  Event handler lock. (IPC_EVHDLR_LOCK())
 *   6.  Event task queue lock. (IPC_EVTASKQ_LOCK())
 *   7.  Event listener context lock. (IPC_EVCTX_LOCK())
 *   8.  Event dispatch queue lock. (IPC_EVDISP_LOCK())
 *   9.  Connection pool lock. (IPC_CPOOL_LOCK())
 *   10. Connection handle lock. (IPC_CONN_LOCK())
 *   11. Client session lock. (IPC_CLSESS_LOCK())
 *   12. Canceller lock. (IPC_CANCELLER_LOCK())
 *   13. Global option lock. (IPC_CLOPTION_{RD,WR}LOCK())
 */
extern pfc_rwlock_t	ipc_client_lock;

#define	IPC_CLIENT_RDLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&ipc_client_lock), 0)
#define	IPC_CLIENT_WRLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&ipc_client_lock), 0)
#define	IPC_CLIENT_UNLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&ipc_client_lock), 0)

#define	IPC_CLIENT_TRYWRLOCK()	pfc_rwlock_trywrlock(&ipc_client_lock)

/*
 * Library disabled flag.
 * This must be protected by the client lock.
 */
extern pfc_bool_t	ipc_disabled;

/*
 * Internal prototypes.
 */
extern void	pfc_ipcclnt_channel_init(void);
extern int	pfc_ipcclnt_getchan(const char *PFC_RESTRICT name,
				    ipc_clchan_t **PFC_RESTRICT chpp,
				    pfc_hostaddr_t *PFC_RESTRICT haddr);
extern int	pfc_ipcclnt_getchanbyname(const char *PFC_RESTRICT name,
					  ipc_clchan_t **PFC_RESTRICT chpp);
extern int	pfc_ipcclnt_chaddr_parse(const char *PFC_RESTRICT addr,
					 ipc_chaddr_t *PFC_RESTRICT cap,
					 pfc_bool_t use_hostaddr);
extern int	pfc_ipcclnt_chaddr_compare(const ipc_chaddr_t *cap1,
					   const ipc_chaddr_t *cap2);

extern ipc_clchan_t	*pfc_ipcclnt_chan_lookup(const char *name,
						 pfc_bool_t do_hold);

extern void	pfc_ipcclnt_chan_destroy(ipc_clchan_t *chp);
extern void	pfc_ipcclnt_chan_iterate(ipc_clchiter_t iter, pfc_ptr_t arg);
extern void	pfc_ipcclnt_chan_cleanup(void);

extern int	pfc_ipcclnt_canceller_get(ipc_canceller_t **PFC_RESTRICT clpp,
					  pfc_ipcsess_t *PFC_RESTRICT sess);
extern void	pfc_ipcclnt_canceller_destroy(ipc_canceller_t *clp);
extern void	pfc_ipcclnt_canceller_destroy_fini(ipc_canceller_t *clp);
extern void	pfc_ipcclnt_canceller_sess_destroy(ipc_sessclr_t *sclp);
extern void	pfc_ipcclnt_canceller_cleanup(void);
extern void	pfc_ipcclnt_canceller_fork_prepare(void);
extern void	pfc_ipcclnt_canceller_fork_parent(void);
extern void	pfc_ipcclnt_canceller_fork_child(void);
extern void	pfc_ipcclnt_canceller_conn_notify(ipc_conn_t *cnp,
						  pfc_ptr_t arg);

extern ipc_conn_t	*pfc_ipcclnt_getconn(pfc_ipcconn_t conn);

extern int	pfc_ipcclnt_conn_default(ipc_conn_t **cnpp);
extern void	pfc_ipcclnt_conn_destroy(ipc_conn_t *cnp);
extern int	pfc_ipcclnt_conn_invoke(ipc_conn_t *PFC_RESTRICT cnp,
					pfc_ipcsess_t *PFC_RESTRICT sess,
					ipc_canceller_t *PFC_RESTRICT clp,
					pfc_ipcresp_t *PFC_RESTRICT respp,
					ctimespec_t *PFC_RESTRICT abstime);
extern int	pfc_ipcclnt_conn_setdefault(const char *PFC_RESTRICT name,
					    char *PFC_RESTRICT newname,
					    pfc_hostaddr_t *PFC_RESTRICT haddr);
extern void	pfc_ipcclnt_conn_iterate(ipc_conniter_t iter, pfc_ptr_t arg);
extern void	pfc_ipcclnt_conn_cleanup(void);
extern void	pfc_ipcclnt_conn_fork_prepare(void);
extern void	pfc_ipcclnt_conn_fork_parent(void);
extern void	pfc_ipcclnt_conn_fork_child(void);

extern int	pfc_ipcclnt_connect(ipc_sess_t *PFC_RESTRICT isp,
				    ipc_clchan_t *PFC_RESTRICT chp,
				    pfc_sockaddr_t *PFC_RESTRICT saddr,
				    int canceller,
				    ctimespec_t *PFC_RESTRICT abstime);
extern int	pfc_ipcclnt_sockaddr_init(pfc_sockaddr_t *PFC_RESTRICT saddr,
					  ipc_clchan_t *PFC_RESTRICT chp,
					  ipc_sess_t *PFC_RESTRICT sess,
					  const pfc_hostaddr_t *PFC_RESTRICT
					  haddr);
extern int	pfc_ipcclnt_ping(pfc_iostream_t PFC_RESTRICT stream,
				 ctimespec_t *PFC_RESTRICT timeout);

extern int	pfc_ipcclnt_sess_state_error(ipc_sstate_t state);

/*
 * static inline uint32_t PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcclnt_conn_getsesscount(ipc_conn_t *cnp)
 *	Return the number of IPC client sessions associated with the given
 *	IPC connection.
 *
 * Calling/Exit State:
 *	The number of IPC client sessions is returned.
 *
 * Remarks:
 *	This function must be called with holding the connection lock.
 */
static inline uint32_t PFC_FATTR_ALWAYS_INLINE
pfc_ipcclnt_conn_getsesscount(ipc_conn_t *cnp)
{
	pfc_list_t	*elem;
	uint32_t	count = 0;

	PFC_LIST_FOREACH(&cnp->icn_sessions, elem) {
		count++;
	}

	return count;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcclnt_sess_resetmsg(pfc_ipcsess_t *sess)
 *	Reset both IPC message and output stream instance to the initial state.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcclnt_sess_resetmsg(pfc_ipcsess_t *sess)
{
	pfc_ipcmsg_reset(&sess->icss_msg);
	pfc_ipcstream_reset(&sess->icss_output, 0);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcclnt_sess_pinmsg(pfc_ipcsess_t *sess)
 *	Pin additional data received from the IPC server in the specified
 *	session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_ipcclnt_sess_pinmsg(pfc_ipcsess_t *sess)
{
	ipc_sstate_t	state;
	int		err;

	IPC_CLSESS_LOCK(sess);

	/* Check the state of this session. */
	state = sess->icss_state;
	if (PFC_EXPECT_FALSE(state != IPC_SSTATE_RESULT)) {
		/* This session does not keep additional data. */
		err = pfc_ipcclnt_sess_state_error(state);
	}
	else {
		/* Bump up busy counter to protect this session. */
		IPC_CLSESS_SET_BUSY(sess);
		err = 0;
	}

	IPC_CLSESS_UNLOCK(sess);

	return err;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcclnt_sess_unpinmsg(pfc_ipcsess_t *sess)
 *	Unpin additional data pinned by the call of pfc_ipcclnt_sess_pinmsg().
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcclnt_sess_unpinmsg(pfc_ipcsess_t *sess)
{
	IPC_CLSESS_LOCK(sess);
	IPC_CLSESS_CLEAR_BUSY(sess);
	IPC_CLSESS_UNLOCK(sess);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcclnt_sess_freeze(pfc_ipcsess_t *sess)
 *	Freeze the given IPC client session.
 *
 *	If the state of the client session is READY, the state is changed to
 *	DISCARD.
 *	Otherwise FROZEN flag is set to the client session so that no further
 *	IPC service request is allowed.
 *
 * Remarks:
 *	This function must be called with holding the client session lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcclnt_sess_freeze(pfc_ipcsess_t *sess)
{
	if (sess->icss_state == IPC_SSTATE_READY) {
		/* Change state to DISCARD. */
		sess->icss_state = IPC_SSTATE_DISCARD;
	}
	else {
		/* Freeze this session. */
		sess->icss_flags |= IPC_CLSSF_FROZEN;
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcclnt_canceller_notify(ipc_canceller_t *clp)
 *	Send cancellation notification to the given canceller instance.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcclnt_canceller_notify(ipc_canceller_t *clp)
{
	uint32_t	*fdp = (uint32_t *)&clp->icl_notify_fd;
	int		fd;

	/* Close file descriptor for notification. */
	fd = (int)pfc_atomic_swap_uint32(fdp, (uint32_t)-1);
	if (fd != -1) {
		(void)close(fd);
	}

	/* Release the canceller. */
	IPC_CANCELLER_RELEASE(clp);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcclnt_ipcmsg_assert(ipc_msg_t *msg, uint8_t bflags)
 *	Ensure that the IPC message instance is initialized correctly.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcclnt_ipcmsg_assert(ipc_msg_t *msg, uint8_t bflags)
{
	pfc_ipcmsg_assert(msg, bflags);
}

/*
 * Internal logging functions.
 */
extern volatile pfc_bool_t	ipcclnt_log_enabled;

#ifdef	__PFC_LOG_GNUC

#define	IPCCLNT_LOG_FATAL(format, ...)				\
	if (ipcclnt_log_enabled) {				\
		pfc_log_fatal((format), ##__VA_ARGS__);		\
	}

#define	IPCCLNT_LOG_ERROR(format, ...)				\
	if (ipcclnt_log_enabled) {				\
		pfc_log_error((format), ##__VA_ARGS__);		\
	}

#define	IPCCLNT_LOG_WARN(format, ...)				\
	if (ipcclnt_log_enabled) {				\
		pfc_log_warn((format), ##__VA_ARGS__);		\
	}

#define	IPCCLNT_LOG_INFO(format, ...)				\
	if (ipcclnt_log_enabled) {				\
		pfc_log_info((format), ##__VA_ARGS__);		\
	}

#define	IPCCLNT_LOG_NOTICE(format, ...)				\
	if (ipcclnt_log_enabled) {				\
		pfc_log_notice((format), ##__VA_ARGS__);	\
	}

#define	IPCCLNT_LOG_DEBUG(format, ...)				\
	if (ipcclnt_log_enabled) {				\
		pfc_log_debug((format), ##__VA_ARGS__);		\
	}

#ifdef	PFC_VERBOSE_DEBUG
#define	IPCCLNT_LOG_VERBOSE(format, ...)			\
	if (ipcclnt_log_enabled) {				\
		pfc_log_verbose((format), ##__VA_ARGS__);	\
	}
#else	/* !PFC_VERBOSE_DEBUG */
#define	IPCCLNT_LOG_VERBOSE(format, ...)		((void)0)
#endif	/* PFC_VERBOSE_DEBUG */

#else	/* !__PFC_LOG_GNUC */

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCCLNT_LOG_FATAL(const char *format, ...)
{
	if (ipcclnt_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_fatal_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCCLNT_LOG_ERROR(const char *format, ...)
{
	if (ipcclnt_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_error_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCCLNT_LOG_WARN(const char *format, ...)
{
	if (ipcclnt_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_warn_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCCLNT_LOG_INFO(const char *format, ...)
{
	if (ipcclnt_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_info_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCCLNT_LOG_NOTICE(const char *format, ...)
{
	if (ipcclnt_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_notice_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCCLNT_LOG_DEBUG(const char *format, ...)
{
	if (ipcclnt_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_debug_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCCLNT_LOG_VERBOSE(const char *format, ...)
{
#ifdef	PFC_VERBOSE_DEBUG
	if (ipcclnt_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_verbose_v(format, ap);
		va_end(ap);
	}
#endif	/* PFC_VERBOSE_DEBUG */
}

#endif	/* __PFC_LOG_GNUC */

#endif	/* _PFC_LIBPFC_IPCCLNT_BUILD */

/*
 * Prototypes.
 */
extern void	pfc_ipcclnt_libfini(void);
extern int	pfc_ipcclnt_getrescount2(pfc_ipcsess_t *PFC_RESTRICT sess,
					 uint32_t *PFC_RESTRICT countp);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_IPCCLNT_IPCCLNT_IMPL_H */
