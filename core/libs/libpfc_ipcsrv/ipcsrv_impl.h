/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_IPCSRV_IPCSRV_IMPL_H
#define	_PFC_LIBPFC_IPCSRV_IPCSRV_IMPL_H

/*
 * Internal definitions for the PFC IPC server management.
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pfc/ipc_server.h>
#include <pfc/debug.h>
#include <pfc/refptr.h>
#include <pfc/synch.h>
#include <pfc/rbtree.h>
#include <pfc/list.h>
#include <pfc/log.h>
#include <pfc/socket.h>
#include <ipc_impl.h>

PFC_C_BEGIN_DECL

/*
 * Handshake timeout.
 */
#define	IPCSRV_HSHAKE_TIMEOUT		PFC_CONST_U(10)	/* 10 seconds */

/*
 * Registered IPC service handler.
 */
typedef struct {
	pfc_refptr_t	*ihd_name;		/* service name */
	pfc_ipchdlr_t	ihd_handler;		/* service handler */
	pfc_ptr_t	ihd_arg;		/* arbitrary data */
	pfc_ipcsvdtor_t	ihd_dtor;		/* destructor */
	uint32_t	ihd_nservices;		/* number of services */
	uint32_t	ihd_refcnt;		/* reference counter */
	pfc_rbnode_t	ihd_node;		/* Red-Black tree node */
	pfc_bool_t	ihd_waiting;		/* waiting for completion */

	/*
	 * Mutex and condition variable.
	 * ihd_mutex may be acquired with holding the handler tree lock
	 * (ich_hlock) in ipc_channel_t.
	 * ihd_cond is used to wait for completion of the handler.
	 */
	pfc_mutex_t	ihd_mutex;
	pfc_cond_t	ihd_cond;

	/*
	 * Statistics.
	 */
	uint64_t	ihd_succeeded;		/* number of success */
	uint64_t	ihd_failed;		/* number of failure */
	uint64_t	ihd_invalid;		/* number of invalid ID */
	uint64_t	ihd_resperror;		/* number of response error */
} ipc_handler_t;

#define	IPC_HANDLER_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_handler_t, ihd_node)

#define	IPC_HANDLER_LOCK(hdp)		pfc_mutex_lock(&(hdp)->ihd_mutex)
#define	IPC_HANDLER_UNLOCK(hdp)		pfc_mutex_unlock(&(hdp)->ihd_mutex)

#define	IPC_HANDLER_SIGNAL(hdp)		pfc_cond_signal(&(hdp)->ihd_cond)
#define	IPC_HANDLER_TIMEDWAIT_ABS(hdp, abstime)				\
	pfc_cond_timedwait_abs(&(hdp)->ihd_cond, &(hdp)->ihd_mutex, (abstime))

#define	IPC_HANDLER_NAME(hdp)	pfc_refptr_string_value((hdp)->ihd_name)

/*
 * Shorthands for data types.
 */
typedef const pfc_ipcsrvops_t	ipc_srvops_t;

typedef struct ipc_channel	ipc_channel_t;

/*
 * Prototype of IPC channel hook function.
 */
typedef void	(*ipc_chhook_t)(ipc_channel_t *chp);

/*
 * IPC service channel instance.
 *
 * ich_handlers must be serialized by ich_hlock. Other fields must be
 * serialized by ich_mutex.
 */
struct ipc_channel {
	pfc_refptr_t	*ich_name;		/* channel name */
	pfc_refptr_t	*ich_sockpath;		/* path to socket file */
	ipc_option_t	ich_option;		/* global IPC server option */
	pfc_mutex_t	ich_mutex;		/* mutex */
	pfc_cond_t	ich_cond;		/* condition variable */
	pfc_rwlock_t	ich_hlock;		/* lock for service handlers */
	pfc_rbtree_t	ich_handlers;		/* service handlers */
	pfc_list_t	ich_sessions;		/* list of active sessions */
	pfc_list_t	ich_evqueues;		/* list of event queues */
	pfc_ipcsrvops_t	ich_ops;		/* server operations */
	pfc_ephdlr_t	ich_ep_listener;	/* listener event handler */
	pfc_ephdlr_t	ich_ep_shutdown;	/* shutdown event handler */
	ipc_chhook_t	ich_dtor;		/* additional destructor */
	int		ich_epfd;		/* epoll(7) FD */
	int		ich_listener;		/* listener socket */
	int		ich_shutfd;		/* shutdown notification FD */
	int		ich_killfd;		/* FD to kill IPC server */
	uint32_t	ich_refcnt;		/* reference counter */
	uint32_t	ich_flags;		/* flags */
	uint32_t	ich_nclients;		/* number of clients */
	uint32_t	ich_nsessions;		/* number of sessions */
	uint32_t	ich_max_clients;	/* max number of clients */
	uint32_t	ich_max_sessions;	/* max number of sessions */
	uint32_t	ich_timeout;		/* session timeout (sec) */
	pfc_ipcevid_t	ich_evserial_next;	/* serial ID for IPC event */
};

#define	IPCCH_NAME(chp)		pfc_refptr_string_value((chp)->ich_name)

#define	IPCCH_SHUT_EP2PTR(ephp)						\
	PFC_CAST_CONTAINER((ephp), ipc_channel_t, ich_ep_shutdown)
#define	IPCCH_LIS_EP2PTR(ephp)						\
	PFC_CAST_CONTAINER((ephp), ipc_channel_t, ich_ep_listener)

/*
 * Remarks:
 *	IPC channel lock must be acquired before IPC server session lock.
 */
#define	IPCCH_LOCK(chp)		pfc_mutex_lock(&(chp)->ich_mutex)
#define	IPCCH_TRYLOCK(chp)	pfc_mutex_trylock(&(chp)->ich_mutex)
#define	IPCCH_UNLOCK(chp)	pfc_mutex_unlock(&(chp)->ich_mutex)

#define	IPCCH_HANDLER_RDLOCK(chp)				\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&(chp)->ich_hlock), 0)
#define	IPCCH_HANDLER_WRLOCK(chp)				\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&(chp)->ich_hlock), 0)
#define	IPCCH_HANDLER_UNLOCK(chp)				\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&(chp)->ich_hlock), 0)

#define	IPCCH_SIGNAL(chp)	pfc_cond_signal(&(chp)->ich_cond)
#define	IPCCH_TIMEDWAIT_ABS(chp, abstime)				\
	pfc_cond_timedwait_abs(&(chp)->ich_cond, &(chp)->ich_mutex, (abstime))

/*
 * Flags for ich_flags.
 */
#define	IPCCHF_SHUTDOWN		PFC_CONST_U(0x1)	/* shutdown */
#define	IPCCHF_SELFPIPE		PFC_CONST_U(0x2)	/* use self-pipe */

/*
 * IPC client information.
 */
typedef struct {
	pfc_refptr_t	*ici_procname;	/* client process name (may be NULL) */
	time_t		ici_start;	/* session start time */
	pfc_ucred_t	ici_cred;	/* client credentials */
	uint32_t	ici_tid;	/* kernel thread ID */
} ipc_clinfo_t;

struct ipc_evsess;
typedef struct ipc_evsess	ipc_evsess_t;

/*
 * IPC event queue entry.
 */
struct ipc_evqent;
typedef struct ipc_evqent	ipc_evqent_t;

struct ipc_evqent {
	ipc_evsess_t	*ieqe_sess;	/* event session */
	ipc_evqent_t	*ieqe_next;	/* next link */
};

/*
 * Send queue of IPC events.
 *
 * Remarks:
 *	All fields must be synchronized by the mutex in ieq_sess.
 */
typedef struct {
	pfc_ipcsrv_t	*ieq_sess;	/* session associated with client */
	ipc_evqent_t	*ieq_queue;	/* head of the queue */
	ipc_evqent_t	**ieq_qnext;	/* pointer to link a new event */
	pfc_ephdlr_t	ieq_epoll;	/* epoll read event handler */
	ipc_evset_t	ieq_evset;	/* set of targeted events */
	pfc_list_t	ieq_list;	/* event queue link */
} ipc_evqueue_t;

#define	IPC_EVQUEUE_EP2PTR(ephp)				\
	PFC_CAST_CONTAINER((ephp), ipc_evqueue_t, ieq_epoll)
#define	IPC_EVQUEUE_LIST2PTR(list)				\
	PFC_CAST_CONTAINER((list), ipc_evqueue_t, ieq_list)

/*
 * IPC server callback data.
 */
typedef struct {
	pfc_ipcsrv_t		*iscb_sess;	/* server session */
	pfc_cond_t		iscb_cond;	/* condition variable */
	pfc_ipcsrvcb_t		iscb_callback;	/* callback function */
	pfc_ptr_t		iscb_arg;	/* an arbitrary argument */
	pfc_ipcsrvcb_type_t	iscb_type;	/* callback type */
	pfc_bool_t		iscb_active;	/* callback is active */
} ipc_srvcb_t;

#define	IPCSRV_CB_WAIT(srv, cbp)				\
	pfc_cond_wait(&(cbp)->iscb_cond, &(srv)->isv_mutex)
#define	IPCSRV_CB_BROADCAST(cbp)	pfc_cond_broadcast(&(cbp)->iscb_cond)

/*
 * The number of IPC server session callback types.
 */
#define	IPCSRV_NCBTYPES		(__PFC_IPCSRVCB_MAX + 1)

/*
 * Determine whether the specified callback type is valid or not.
 */
#define	IPCSRV_CBTYPE_IS_VALID(type)			\
	((uint32_t)(type) <= __PFC_IPCSRVCB_MAX)

/*
 * IPC server session.
 */
struct __pfc_ipcsrv {
	ipc_sess_t	isv_session;	/* common session instance */
	ipc_channel_t	*isv_channel;	/* IPC channel */
	pfc_mutex_t	isv_mutex;	/* mutex */
	pfc_cond_t	isv_cond;	/* condition variable */
	ipc_msg_t	isv_args;	/* arguments */
	ipc_stream_t	isv_output;	/* output stream */
	pfc_timespec_t	isv_timeout;	/* server session timeout */
	pfc_list_t	isv_list;	/* link for active session list */
	pfc_ephdlr_t	isv_epoll;	/* connection reset event handler */
	ipc_clinfo_t	isv_client;	/* client information */

	/* IPC server session callbacks. */
	ipc_srvcb_t	*isv_callback[IPCSRV_NCBTYPES];

	/* Active IPC service ID and name. */
	pfc_ipcid_t	isv_service;
	char		isv_name[IPC_SERVICE_NAMELEN_MAX + 1];
};

#define	isv_stream	isv_session.iss_stream
#define	isv_addr	isv_session.iss_addr
#define	isv_flags	isv_session.iss_resv1
#define	isv_magic	isv_session.iss_resv2[0]
#define	isv_event	isv_session.iss_resv2[1]
#define	isv_procname	isv_client.ici_procname
#define	isv_start	isv_client.ici_start
#define	isv_cred	isv_client.ici_cred
#define	isv_tid		isv_client.ici_tid

#define	IPCSRV_LIST2PTR(list)					\
	PFC_CAST_CONTAINER((list), pfc_ipcsrv_t, isv_list)
#define	IPCSRV_EP2PTR(ephp)					\
	PFC_CAST_CONTAINER((ephp), pfc_ipcsrv_t, isv_epoll)

#define	IPCSRV_ACTIVE_ADD(chp, srv)					\
	pfc_list_push_tail(&(chp)->ich_sessions, &(srv)->isv_list)
#define	IPCSRV_ACTIVE_REMOVE(srv)	pfc_list_remove(&(srv)->isv_list)

#define	IPCSRV_LOCK(srv)	pfc_mutex_lock(&(srv)->isv_mutex)
#define	IPCSRV_UNLOCK(srv)	pfc_mutex_unlock(&(srv)->isv_mutex)

#define	IPCSRV_SIGNAL(srv)	pfc_cond_signal(&(srv)->isv_cond)
#define	IPCSRV_WAIT(srv)					\
	pfc_cond_wait(&(srv)->isv_cond, &(srv)->isv_mutex)
#define	IPCSRV_TIMEDWAIT_ABS(srv, abstime)				\
	pfc_cond_timedwait_abs(&(srv)->isv_cond, &(srv)->isv_mutex, (abstime))

#define	IPCSRV_PID(srv)		(srv)->isv_cred.pid

/*
 * Flags for isv_flags.
 */
#define	IPCSRVF_HASTIMEOUT	PFC_CONST_U(0x0001)	/* timeout is set */
#define	IPCSRVF_NOTIMEOUT	PFC_CONST_U(0x0002)	/* no timeout */
#define	IPCSRVF_ACTIVE		PFC_CONST_U(0x0004)	/* session is active */
#define	IPCSRVF_WATCH		PFC_CONST_U(0x0008)	/* connection watch */
#define	IPCSRVF_SHUTDOWN	PFC_CONST_U(0x0010)	/* server shutdown */
#define	IPCSRVF_EVLISTENER	PFC_CONST_U(0x0020)	/* event listener */
#define	IPCSRVF_EVQ_INPUT	PFC_CONST_U(0x0040)	/* input on eventQ */
#define	IPCSRVF_EVQ_SHUTDOWN	PFC_CONST_U(0x0080)	/* eventQ shutdown */

/*
 * True if the specified server session is a pseudo session used to send
 * an IPC event.
 */
#define	IPCSRV_IS_EVENT(srv)	((srv)->isv_event)

/*
 * True if the IPC server is already shut down.
 */
#define	IPCSRV_IS_SHUTDOWN(srv)					\
	PFC_EXPECT_FALSE((srv)->isv_flags & IPCSRVF_SHUTDOWN)

/*
 * True if the event queue on the specified server session is already shut
 * down.
 */
#define	IPCSRV_IS_EVQ_SHUTDOWN(srv)					\
	PFC_EXPECT_FALSE((srv)->isv_flags & IPCSRVF_EVQ_SHUTDOWN)

/*
 * Determine whether the connection is already reset or not.
 * This macro must be used with holding the server session lock.
 */
#define	IPCSRV_IS_RESET(srv)						\
	PFC_EXPECT_FALSE(!IPCSRV_IS_EVENT(srv) && (srv)->isv_stream == NULL)

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * ipcsrv_is_reset(pfc_ipcsrv_t *srv)
 *	Check whether the connection associated with the given server session
 *	is already reset or not.
 *
 * Calling/Exit State:
 *	Zero is returned if the given server session still has active
 *	connection.
 *	Otherwise ECONNRESET is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
ipcsrv_is_reset(pfc_ipcsrv_t *srv)
{
	int	err;

	IPCSRV_LOCK(srv);
	err = (IPCSRV_IS_RESET(srv)) ? ECONNRESET : 0;
	IPCSRV_UNLOCK(srv);

	return err;
}

/*
 * Entity of IPC event delivery descriptor.
 */
typedef struct {
	pfc_ipcevdesc_t	ied_desc;	/* event delivery descriptor */
	uint32_t	ied_flags;	/* flags */
	ipc_evsess_t	*ied_sess;	/* event session */
	pfc_rbnode_t	ied_node;	/* Red-Black tree node */
} ipc_evdesc_t;

#define	IPC_EVDESC_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_evdesc_t, ied_node)
#define	IPC_EVDESC_KEY(desc)		((pfc_cptr_t)(uintptr_t)(desc))

/*
 * Flags for ied_flags.
 */
#define	IPC_EVDF_COMPLETE	PFC_CONST_U(0x1)	/* completed */
#define	IPC_EVDF_DESTROY	PFC_CONST_U(0x2)	/* destroyed */
#define	IPC_EVDF_SHUTDOWN	PFC_CONST_U(0x4)	/* server shutdown */
#define	IPC_EVDF_INVISIBLE	PFC_CONST_U(0x8)	/* not on desc tree */
#define	IPC_EVDF_WAITING	PFC_CONST_U(0x10)	/* already waiting */

#define	IPC_EVDF_WAKEUP							\
	(IPC_EVDF_COMPLETE | IPC_EVDF_DESTROY | IPC_EVDF_SHUTDOWN)

/*
 * Pseudo server session used to send an IPC event.
 */
struct ipc_evsess {
	ipc_evproto_t	ies_event;	/* common event data */
	pfc_ipcsrv_t	ies_session;	/* dummy server session */
	ipc_evdesc_t	*ies_desc;	/* event delivery descriptor */
	uint32_t	ies_refcnt;	/* reference counter */
};

#define	IPC_EVSESS_SRV2SESS(srv)				\
	PFC_CAST_CONTAINER((srv), ipc_evsess_t, ies_session)

#ifdef	_PFC_LIBPFC_IPCSRV_BUILD

extern ipc_channel_t	ipc_channel;
extern const char	ipcsrv_str_unknown[];
extern const char	ipcsrv_str_empty[];

/*
 * Internal logging functions.
 */

extern volatile pfc_bool_t	ipcsrv_log_enabled;

#ifdef	__PFC_LOG_GNUC

#define	IPCSRV_LOG_ERROR(format, ...)				\
	if (ipcsrv_log_enabled) {				\
		pfc_log_error((format), ##__VA_ARGS__);		\
	}

#define	IPCSRV_LOG_WARN(format, ...)				\
	if (ipcsrv_log_enabled) {				\
		pfc_log_warn((format), ##__VA_ARGS__);		\
	}

#define	IPCSRV_LOG_INFO(format, ...)				\
	if (ipcsrv_log_enabled) {				\
		pfc_log_info((format), ##__VA_ARGS__);		\
	}

#define	IPCSRV_LOG_NOTICE(format, ...)				\
	if (ipcsrv_log_enabled) {				\
		pfc_log_notice((format), ##__VA_ARGS__);	\
	}

#define	IPCSRV_LOG_DEBUG(format, ...)				\
	if (ipcsrv_log_enabled) {				\
		pfc_log_debug((format), ##__VA_ARGS__);		\
	}

#ifdef	PFC_VERBOSE_DEBUG
#define	IPCSRV_LOG_VERBOSE(format, ...)				\
	if (ipcsrv_log_enabled) {				\
		pfc_log_verbose((format), ##__VA_ARGS__);	\
	}
#else	/* !PFC_VERBOSE_DEBUG */
#define	IPCSRV_LOG_VERBOSE(format, ...)		((void)0)
#endif	/* PFC_VERBOSE_DEBUG */

#else	/* !__PFC_LOG_GNUC */

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCSRV_LOG_ERROR(const char *format, ...)
{
	if (ipcsrv_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_error_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCSRV_LOG_WARN(const char *format, ...)
{
	if (ipcsrv_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_warn_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCSRV_LOG_INFO(const char *format, ...)
{
	if (ipcsrv_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_info_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCSRV_LOG_NOTICE(const char *format, ...)
{
	if (ipcsrv_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_notice_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCSRV_LOG_DEBUG(const char *format, ...)
{
	if (ipcsrv_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_debug_v(format, ap);
		va_end(ap);
	}
}

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IPCSRV_LOG_VERBOSE(const char *format, ...)
{
#ifdef	PFC_VERBOSE_DEBUG
	if (ipcsrv_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_verbose_v(format, ap);
		va_end(ap);
	}
#endif	/* PFC_VERBOSE_DEBUG */
}

#endif	/* __PFC_LOG_GNUC */

/*
 * Internal prototypes.
 */
extern void	pfc_ipcsrv_shutdown(ipc_channel_t *chp);
extern void	pfc_ipcsrv_channel_cleanup(void);

extern int	pfc_ipcsrv_sess_init(ipc_channel_t *PFC_RESTRICT chp,
				     pfc_ipcsrv_t *PFC_RESTRICT srv, int sock,
				     const pfc_hostaddr_t *haddr,
				     ipc_coption_t *opts);
extern void	pfc_ipcsrv_sess_cleanup(pfc_ipcsrv_t *srv);
extern int	pfc_ipcsrv_epoll_shutdown(pfc_ephdlr_t *ephp, uint32_t events,
					  void *arg);
extern int	pfc_ipcsrv_epoll_listener(pfc_ephdlr_t *ephp, uint32_t events,
					  void *arg);

extern int	pfc_ipcsrv_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv,
				  pfc_ipcresp_t *PFC_RESTRICT resultp);
extern void	pfc_ipcsrv_remove_all(void);
extern void	pfc_ipcsrv_inc_resperror(pfc_ipcsrv_t *srv);

extern void	pfc_ipcsrv_evdesc_init(void);
extern void	pfc_ipcsrv_evdesc_shutdown(void);

extern int	pfc_ipcsrv_event_session(pfc_ipcsrv_t *PFC_RESTRICT srv,
					 pfc_timespec_t *PFC_RESTRICT abstime);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcsrv_setflags_l(pfc_ipcsrv_t *srv, uint32_t flags)
 *	Set server session flags to the given server session.
 *
 * Remarks:
 *	This function must be called with holding the server session lock
 *	associated with the event queue `evq'.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcsrv_setflags_l(pfc_ipcsrv_t *srv, uint32_t flags)
{
	srv->isv_flags |= flags;
	IPCSRV_SIGNAL(srv);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcsrv_evqueue_setflags(ipc_evqueue_t *evq, uint32_t flags)
 *	Set server session flags to the session associated with the given
 *	event queue.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcsrv_evqueue_setflags(ipc_evqueue_t *evq, uint32_t flags)
{
	pfc_ipcsrv_t	*srv = evq->ieq_sess;

	IPCSRV_LOCK(srv);
	pfc_ipcsrv_setflags_l(srv, flags);
	IPCSRV_UNLOCK(srv);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcsrv_gettime(pfc_timespec_t *tsp)
 *	Obtain current monotonic system time.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_ipcsrv_gettime(pfc_timespec_t *tsp)
{
	int	err = pfc_clock_gettime(tsp);

	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCSRV_LOG_ERROR("Failed to obtain system time: %s",
				 strerror(err));
	}

	return err;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcsrv_timeout_init(ipc_channel_t *chp, pfc_timespec_t **abstimep,
 *			   pfc_timespec_t *tsbuf)
 *	Initialize I/O timeout on the specified IPC channel.
 *
 *	If an infinite timeout is configured, NULL is set to the buffer pointed
 *	by `abstimep'.
 *	Otherwise, the system absolute expiry time of I/O is set to the buffer
 *	pointed by `tsbuf', and its address is set to the buffer pointed by
 *	`abstimep'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_ipcsrv_timeout_init(ipc_channel_t *chp, pfc_timespec_t **abstimep,
			pfc_timespec_t *tsbuf)
{
	uint32_t	timeout = chp->ich_timeout;

	if (timeout == 0) {
		*abstimep = NULL;
	}
	else {
		int	err = pfc_ipcsrv_gettime(tsbuf);

		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		tsbuf->tv_sec += timeout;
		*abstimep = tsbuf;
	}

	return 0;
}

/*
 * Close the given file descriptor.
 */
#define	PFC_IPCSRV_CLOSE(fd)						\
	do {								\
		if (PFC_EXPECT_FALSE(close(fd) != 0)) {			\
			IPCSRV_LOG_ERROR("%s:%u: "			\
					 "Failed to close FD(%d): %s",	\
					 __FILE__, __LINE__, fd,	\
					 strerror(errno));		\
		}							\
	} while (0)

#endif	/* _PFC_LIBPFC_IPCSRV_BUILD */

/*
 * Prototypes.
 */
extern int	pfc_ipcsrv_sysinit(const char *channel, int shutfd,
				   const pfc_ipcsrvops_t *ops);
extern void	pfc_ipcsrv_destroy(ipc_channel_t *chp);

extern int	pfc_ipcsrv_add_handler_impl(pfc_refptr_t *name,
					    uint32_t nservices,
					    pfc_ipchdlr_t handler,
					    pfc_ptr_t arg,
					    pfc_ipcsvdtor_t dtor);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcsrv_channel_inc(ipc_channel_t *chp)
 *	Increment reference counter of the IPC channel specified by `chp'
 *	without error check.
 *
 * Remarks:
 *	The caller must call this function with holding the IPC channel lock,
 *	and must ensure that the IPC channel is already held.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcsrv_channel_ref(ipc_channel_t *chp)
{
	PFC_ASSERT(chp->ich_refcnt != 0);
	chp->ich_refcnt++;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcsrv_channel_check(ipc_channel_t *chp)
 *	Ensure that the specified IPC channel is valid.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified IPC channel is valid.
 *	ECANCELED is returned if the IPC channel instance is already
 *	destroyed.
 *	ENODEV is returned if the IPC channel is not initialized yet.
 *
 * Remarks:
 *	This function must be called with holding the channel lock.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_ipcsrv_channel_check(ipc_channel_t *chp)
{
	if (PFC_EXPECT_FALSE(chp->ich_flags & IPCCHF_SHUTDOWN)) {
		/* The IPC channel is already destroyed. */
		return ECANCELED;
	}

	if (PFC_EXPECT_FALSE(chp->ich_refcnt == 0)) {
		/* The IPC channel is not initialized yet. */
		return ENODEV;
	}

	return 0;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcsrv_channel_hold_l(ipc_channel_t *chp)
 *	Hold the given IPC channel instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ECANCELED is returned if the given channel instance is already
 *	destroyed.
 *
 * Remarks:
 *	This function must be called with holding the channel lock.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_ipcsrv_channel_hold_l(ipc_channel_t *chp)
{
	int	err;

	err = pfc_ipcsrv_channel_check(chp);
	if (PFC_EXPECT_TRUE(err == 0)) {
		pfc_ipcsrv_channel_ref(chp);
	}

	return err;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcsrv_channel_hold(ipc_channel_t *chp)
 *	Hold the given IPC channel instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ECANCELED is returned if the given channel instance is already
 *	destroyed.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_ipcsrv_channel_hold(ipc_channel_t *chp)
{
	int	err;

	IPCCH_LOCK(chp);
	err = pfc_ipcsrv_channel_hold_l(chp);
	IPCCH_UNLOCK(chp);

	return err;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcsrv_channel_release_l(ipc_channel_t *chp)
 *	Release the given IPC channel instance.
 *
 * Remarks:
 *	This function must be called with holding the channel lock.
 *	The lock is always released on return.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcsrv_channel_release_l(ipc_channel_t *chp)
{
	uint32_t	refcnt = chp->ich_refcnt;

	if (refcnt == 1) {
		pfc_ipcsrv_destroy(chp);
	}
	else {
		PFC_ASSERT(refcnt != 0);
		chp->ich_refcnt = refcnt - 1;
		IPCCH_UNLOCK(chp);
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_ipcsrv_channel_release(ipc_channel_t *chp)
 *	Release the given IPC channel instance.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_ipcsrv_channel_release(ipc_channel_t *chp)
{
	IPCCH_LOCK(chp);
	pfc_ipcsrv_channel_release_l(chp);
}

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_IPCSRV_IPCSRV_IMPL_H */
