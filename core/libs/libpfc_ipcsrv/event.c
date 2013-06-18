/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * event.c - Generate IPC event.
 */

#include <string.h>
#include <poll.h>
#include <pfc/atomic.h>
#include <pfc/util.h>
#include <tid_impl.h>
#include <iostream_impl.h>
#include "ipcsrv_impl.h"

/*
 * Event poll event bits to detect incoming data from the IPC client.
 */
#define	IPC_EVENT_EPOLLEV_INPUT					\
	(EPOLLIN | IPC_EPOLLEV_ONESHOT | IPC_EPOLLEV_RESET)

/*
 * Event poll event bits to test whether the session was reset.
 */
#define	IPC_EVENT_EPOLLEV_TEST_RESET	(IPC_EPOLLEV_TEST_RESET | EPOLLOUT)

/*
 * List of pairs of IPC service name and event type mask.
 */
struct ipc_tgtlist;
typedef struct ipc_tgtlist	ipc_tgtlist_t;

struct ipc_tgtlist{
	/* IPC service name. */
	char		itl_name[IPC_SERVICE_NAMELEN_MAX + 1];

	/* IPC event type mask. */
	pfc_ipcevmask_t	itl_mask;

	/* Link to the next element.*/
	ipc_tgtlist_t	*itl_next;
};

/*
 * IPC event delivery descriptor manager.
 */
typedef struct {
	pfc_rbtree_t	iedm_tree;	/* descriptor tree */
	pfc_rwlock_t	iedm_lock;	/* read-write lock */
	uint32_t	iedm_next;	/* descriptor for next allocation */
	uint32_t	iedm_flags;	/* flags */
} ipc_evdescmgr_t;

#define	IPC_EVDMGR_RDLOCK(dmgr)					\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&(dmgr)->iedm_lock), 0)
#define	IPC_EVDMGR_WRLOCK(dmgr)					\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&(dmgr)->iedm_lock), 0)
#define	IPC_EVDMGR_UNLOCK(dmgr)					\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&(dmgr)->iedm_lock), 0)

/*
 * Flags for iedm_flags.
 */
#define	IPC_EVDMGRF_INIT	PFC_CONST_U(0x1)	/* initialized */
#define	IPC_EVDMGRF_SHUTDOWN	PFC_CONST_U(0x2)	/* shutdown */

/*
 * Internal prototypes.
 */
static void	ipcsrv_evsess_destroy(ipc_evsess_t *esess);
static void	ipcsrv_evsess_enqueue(ipc_evqueue_t *PFC_RESTRICT evq,
				      ipc_evsess_t *PFC_RESTRICT esess);
static int	ipcsrv_epoll_event(pfc_ephdlr_t *ephp, uint32_t events,
				   void *arg);
static int	ipcsrv_event_postto(pfc_ipcsrv_t *PFC_RESTRICT srv,
				    const pfc_ipccladdr_t *PFC_RESTRICT
				    claddr);
static int	ipcsrv_event_main(pfc_ipcsrv_t *PFC_RESTRICT srv,
				  ipc_evqueue_t *PFC_RESTRICT evq);
static int	ipcsrv_event_recv(pfc_ipcsrv_t *PFC_RESTRICT srv,
				  ipc_evqueue_t *PFC_RESTRICT evq);
static int	ipcsrv_event_recvmask(pfc_ipcsrv_t *PFC_RESTRICT srv,
				      ipc_tgtlist_t **PFC_RESTRICT headp,
				      ctimespec_t *PFC_RESTRICT abstime);
static void	ipcsrv_event_epoll_remove(pfc_ipcsrv_t *PFC_RESTRICT srv,
					  ipc_evqueue_t *PFC_RESTRICT evq);
static int	ipcsrv_event_send(pfc_ipcsrv_t *PFC_RESTRICT srv,
				  ipc_evsess_t *PFC_RESTRICT esess);
static void	ipcsrv_event_timeout_init(pfc_ipcsrv_t *PFC_RESTRICT evsrv,
					  ctimespec_t *PFC_RESTRICT cur,
					  pfc_timespec_t **PFC_RESTRICT absp);

static int	ipcsrv_evdesc_create(ipc_evdescmgr_t *PFC_RESTRICT dmgr,
				     ipc_evsess_t *PFC_RESTRICT esess);
static int	ipcsrv_evdesc_wait(pfc_ipcevdesc_t desc,
				   const pfc_timespec_t *PFC_RESTRICT abstime);
static void	ipcsrv_evdesc_cancel(ipc_evsess_t *esess);
static void	ipcsrv_evdesc_cancel_l(ipc_evdescmgr_t *PFC_RESTRICT dmgr,
				       ipc_evdesc_t *PFC_RESTRICT edp,
				       ipc_evsess_t *PFC_RESTRICT esess,
				       uint32_t cflags);
static void	ipcsrv_evdesc_destroy(ipc_evdescmgr_t *PFC_RESTRICT dmgr,
				      ipc_evdesc_t *PFC_RESTRICT edp,
				      ipc_evsess_t *PFC_RESTRICT esess);
static void	ipcsrv_evdesc_shutdown(pfc_rbnode_t *node, pfc_ptr_t arg);

static pfc_cptr_t	ipcsrv_evdesc_getkey(pfc_rbnode_t *node);

static int	ipcsrv_evcmd_mask_add(pfc_ipcsrv_t *PFC_RESTRICT srv,
				      ipc_evqueue_t *PFC_RESTRICT evq,
				      ctimespec_t *PFC_RESTRICT abstime);
static int	ipcsrv_evcmd_mask_del(pfc_ipcsrv_t *PFC_RESTRICT srv,
				      ipc_evqueue_t *PFC_RESTRICT evq,
				      ctimespec_t *PFC_RESTRICT abstime);
static int	ipcsrv_evcmd_mask_reset(pfc_ipcsrv_t *PFC_RESTRICT srv,
					ipc_evqueue_t *PFC_RESTRICT evq,
					ctimespec_t *PFC_RESTRICT abstime);

/*
 * Operation for IPC event subcommand.
 */
typedef struct {
	/*
	 * int
	 * evcmd_exec(pfc_ipcsrv_t *PFC_RESTRICT srv,
	 *	      ipc_evqueue_t *PFC_RESTRICT evq,
	 *	      ctimespec_t *PFC_RESTRICT abstime)
	 *	Execute IPC event subcommand.
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, zero is returned.
	 *	Otherwise error number which indicates the cause of error is
	 *	returned.
	 */
	int	(*evcmd_exec)(pfc_ipcsrv_t *PFC_RESTRICT srv,
			      ipc_evqueue_t *PFC_RESTRICT evq,
			      ctimespec_t *PFC_RESTRICT abstime);
} ipc_evcmdops_t;

typedef const ipc_evcmdops_t	ipc_cevcmdops_t;

#define	IPC_EVCMDOPS_DECL(cmd)				\
	{						\
		.evcmd_exec	= ipcsrv_evcmd_##cmd,	\
	}

/*
 * IPC event subcommands.
 * Array index must be a command identifier.
 */
static ipc_cevcmdops_t	ipc_event_commands[] = {
	IPC_EVCMDOPS_DECL(mask_add),		/* IPC_EVENTCMD_MASK_ADD */
	IPC_EVCMDOPS_DECL(mask_del),		/* IPC_EVENTCMD_MASK_DEL */
	IPC_EVCMDOPS_DECL(mask_reset),		/* IPC_EVENTCMD_MASK_RESET */
};

/*
 * Initial value of IPC event delivery descriptor.
 */
#define	IPC_EVDESC_INITIAL		(PFC_IPCEVDESC_INVALID + 1)

/*
 * Instance of IPC event delivery manager.
 */
static ipc_evdescmgr_t	ipc_evdesc_mgr = {
	.iedm_tree	= PFC_RBTREE_INITIALIZER(pfc_rbtree_uint32_compare,
						 ipcsrv_evdesc_getkey),
	.iedm_lock	= PFC_RWLOCK_INITIALIZER,
	.iedm_next	= IPC_EVDESC_INITIAL,
	.iedm_flags	= 0,
};

/*
 * Hold/Release IPC event session.
 *
 * Remarks:
 *	IPC_EVSESS_RELEASE() must be used without holding IPC event session
 *	lock associated with `esess'. ipcsrv_evsess_destroy() may acquire it.
 */
#define	IPC_EVSESS_HOLD(esess)					\
	do {							\
		PFC_ASSERT((esess)->ies_refcnt != 0);		\
		pfc_atomic_inc_uint32(&(esess)->ies_refcnt);	\
	} while (0)

#define	IPC_EVSESS_RELEASE(esess)					\
	do {								\
		uint32_t	__cnt =					\
			pfc_atomic_dec_uint32_old(&(esess)->ies_refcnt); \
									\
		if (__cnt == 1) {					\
			ipcsrv_evsess_destroy(esess);			\
		}							\
		else {							\
			PFC_ASSERT(__cnt != 0);				\
		}							\
	} while (0)

/*
 * static inline int
 * ipc_event_epoll_enable_input(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *				ipc_evqueue_t *PFC_RESTRICT evq)
 *	Enable input event on the session socket.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the server session lock.
 *
 *	- This function must be called on the server session thread.
 */
static inline int
ipc_event_epoll_enable_input(pfc_ipcsrv_t *PFC_RESTRICT srv,
			     ipc_evqueue_t *PFC_RESTRICT evq)
{
	ipc_channel_t	*chp = srv->isv_channel;
	struct pollfd	pfd;
	int		err, fd, nfds;

	/* Check whether more incoming data exists or not. */
	fd = pfc_iostream_getfd(srv->isv_stream);
	pfd.fd = fd;
	pfd.events = POLLIN | IPC_POLL_RESET;
	pfd.revents = 0;
	nfds = poll(&pfd, 1, 0);
	if (nfds == 1) {
		if (PFC_EXPECT_FALSE(pfd.revents & IPC_POLL_TEST_RESET)) {
			/* The event session was reset by the client. */
			return ECANCELED;
		}

		if (pfd.revents & POLLIN) {
			srv->isv_flags |= IPCSRVF_EVQ_INPUT;

			return 0;
		}
	}
	else if (PFC_EXPECT_FALSE(nfds == -1 && (err = errno) != EINTR)) {
		/* This should never happen. */
		IPCSRV_LOG_ERROR("%u: poll() on event session failed: %s",
				 IPCSRV_PID(srv), strerror(err));

		return err;
	}
	else {
		PFC_ASSERT(nfds == 0 || (nfds == -1 && errno == EINTR));
	}

	/* Update epoll instance to watch read events. */
	err = pfc_epoll_ctl(chp->ich_epfd, EPOLL_CTL_MOD, fd,
			    IPC_EVENT_EPOLLEV_INPUT, &evq->ieq_epoll);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		err = errno;
		IPCSRV_LOG_ERROR("%u: Failed to activate input event on "
				 "the event session: %s", IPCSRV_PID(srv),
				 strerror(err));
	}

	return err;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipc_event_epoll_disable_input(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *				 ipc_evqueue_t *PFC_RESTRICT evq)
 *	Disable input event on the session socket.
 *
 * Remarks:
 *	This function must be called with holding the server session lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipc_event_epoll_disable_input(pfc_ipcsrv_t *PFC_RESTRICT srv,
			      ipc_evqueue_t *PFC_RESTRICT evq)
{
#ifndef	PFC_HAVE_EPOLLONESHOT
	ipc_channel_t	*chp = srv->isv_channel;
	int		ret, fd = pfc_iostream_getfd(srv->isv_stream);

	ret = pfc_epoll_ctl(chp->ich_epfd, EPOLL_CTL_MOD, fd,
			    IPCSRV_EPOLLEV_RESET, &evq->ieq_epoll);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		/* This should never happen. */
		IPCSRV_LOG_ERROR("%u: Failed to disable input event on the "
				 "event session: %s", IPCSRV_PID(srv),
				 strerror(errno));
	}
#endif	/* !PFC_HAVE_EPOLLONESHOT */
}

/*
 * int
 * pfc_ipcsrv_event_create(pfc_ipcsrv_t **PFC_RESTRICT srvp,
 *			   const char *PFC_RESTRICT name,
 *			   pfc_ipcevtype_t type)
 *	Create a new IPC event.
 *
 *	`name' must be an IPC service name, and `type' is an IPC event type
 *	to be assigned to a new event.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to IPC service session
 *	for sending an IPC event is set to the buffer pointed by `srvp', and
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcsrv_event_create(pfc_ipcsrv_t **PFC_RESTRICT srvp,
			const char *PFC_RESTRICT name,
			pfc_ipcevtype_t type)
{
	ipc_channel_t	*chp = &ipc_channel;
	ipc_evproto_t	*iep;
	ipc_evsess_t	*esess;
	pfc_ipcsrv_t	*srv;
	size_t		len;
	int		err;

	/* Ensure that the specified arguments are valid. */
	if (PFC_EXPECT_FALSE(srvp == NULL)) {
		return EINVAL;
	}

	if (!PFC_IPC_EVTYPE_IS_VALID(type)) {
		err = EINVAL;
		goto error;
	}

	err = pfc_ipc_check_service_name(name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Allocate a new IPC event session. */
	esess = (ipc_evsess_t *)malloc(sizeof(*esess));
	if (PFC_EXPECT_FALSE(esess == NULL)) {
		err = ENOMEM;
		goto error;
	}

	/* Initialize pseudo server session. */
	srv = &esess->ies_session;
	err = pfc_ipcsrv_sess_init(chp, srv, -1, NULL, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		free(esess);
		goto error;
	}

	len = pfc_strlcpy_assert(srv->isv_name, name, sizeof(srv->isv_name));
	esess->ies_refcnt = 1;
	esess->ies_desc = NULL;

	iep = &esess->ies_event;
	iep->iev_type = type;
	iep->iev_namelen = (uint8_t)len;
	iep->iev_resv = 0;

	/*
	 * Serial ID and timestamp and will be updated by
	 * pfc_ipcsrv_event_post().
	 */
	iep->iev_serial = 0;
	iep->iev_time_sec = 0;
	iep->iev_time_nsec= 0;

	*srvp = srv;

	return 0;

error:
	*srvp = NULL;

	return err;
}

/*
 * int
 * pfc_ipcsrv_event_destroy(pfc_ipcsrv_t *srv)
 *	Destroy IPC event session specified by `srv'.
 *
 *	This function is provided to destroy IPC event object that is not
 *	passed to pfc_ipcsrv_event_post().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcsrv_event_destroy(pfc_ipcsrv_t *srv)
{
	ipc_evsess_t	*esess;

	if (PFC_EXPECT_FALSE(srv == NULL || !IPCSRV_IS_EVENT(srv))) {
		return EINVAL;
	}

	esess = IPC_EVSESS_SRV2SESS(srv);

	/*
	 * Cancel wait on the event delivery descriptor associated with
	 * this event.
	 */
	ipcsrv_evdesc_cancel(esess);

	/* Release the event. */
	IPC_EVSESS_RELEASE(esess);

	return 0;
}

/*
 * int
 * pfc_ipcsrv_event_post(pfc_ipcsrv_t *srv)
 *	Post an IPC event to all registered event listeners.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function implies pfc_ipcsrv_event_destroy().
 *	That is, the specified IPC event session is always destroyed unless
 *	the specified session is not an event session.
 */
int
pfc_ipcsrv_event_post(pfc_ipcsrv_t *srv)
{
	return ipcsrv_event_postto(srv, NULL);
}

/*
 * int
 * pfc_ipcsrv_event_postto(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			   const pfc_ipccladdr_t *PFC_RESTRICT claddr)
 *	Post an IPC event to the IPC client specified by `claddr'.
 *
 *	If `claddr' is NULL, the specified IPC event is broadcasted to all
 *	IPC event listeners.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function implies pfc_ipcsrv_event_destroy().
 *	That is, the specified IPC event session is always destroyed unless
 *	the specified session is not an event session.
 */
int
pfc_ipcsrv_event_postto(pfc_ipcsrv_t *PFC_RESTRICT srv,
			const pfc_ipccladdr_t *PFC_RESTRICT claddr)
{
	return ipcsrv_event_postto(srv, claddr);
}

/*
 * int
 * pfc_ipcsrv_evdesc_create(pfc_ipcevdesc_t *PFC_RESTRICT descp,
 *			    pfc_ipcsrv_t *PFC_RESTRICT srv)
 *	Create an IPC event delivery descriptor associated with the given event
 *	if not yet created.
 *
 *	IPC event delivery descriptor can be used to wait for completion of
 *	delivery of the given IPC event. If an IPC event delivery descriptor
 *	is already assigned to the given event, this function returns it.
 *
 * Calling/Exit State:
 *	Upon successful completion, an IPC event delivery descriptor is set
 *	to the buffer pointed by `descp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Once IPC event delivery descriptor is created, it must be destroyed
 *	by pfc_ipcsrv_evdesc_wait(), pfc_ipcsrv_evdesc_wait_abs(), or
 *	pfc_ipcsrv_evdesc_destroy().
 */
int
pfc_ipcsrv_evdesc_create(pfc_ipcevdesc_t *PFC_RESTRICT descp,
			 pfc_ipcsrv_t *PFC_RESTRICT srv)
{
	ipc_evdescmgr_t	*dmgr = &ipc_evdesc_mgr;
	ipc_evdesc_t	*edp;
	ipc_evsess_t	*esess;
	uint32_t	flags;
	int		err;

	if (PFC_EXPECT_FALSE(descp == NULL || srv == NULL ||
			     !IPCSRV_IS_EVENT(srv))) {
		err = EINVAL;
		goto error;
	}

	esess = IPC_EVSESS_SRV2SESS(srv);

	IPC_EVDMGR_WRLOCK(dmgr);
	IPCSRV_LOCK(srv);

	flags = dmgr->iedm_flags;
	if (PFC_EXPECT_FALSE((flags & IPC_EVDMGRF_INIT) == 0)) {
		/* Not yet initialized. */
		err = ENODEV;
		goto error_unlock;
	}

	if (PFC_EXPECT_FALSE(flags & IPC_EVDMGRF_SHUTDOWN)) {
		/* IPC server is unavailable. */
		err = ECANCELED;
		goto error_unlock;
	}

	edp = esess->ies_desc;
	if (PFC_EXPECT_FALSE(edp != NULL)) {
		/* Delivery descriptor is already assigned. */
		err = EBUSY;
		goto error_unlock;
	}

	/* Create a new delivery descriptor. */
	err = ipcsrv_evdesc_create(dmgr, esess);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_unlock;
	}

	edp = esess->ies_desc;
	PFC_ASSERT(edp != NULL);

	*descp = edp->ied_desc;

	IPCSRV_UNLOCK(srv);
	IPC_EVDMGR_UNLOCK(dmgr);

	return 0;

error_unlock:
	IPCSRV_UNLOCK(srv);
	IPC_EVDMGR_UNLOCK(dmgr);

error:
	if (descp != NULL) {
		*descp = PFC_IPCEVDESC_INVALID;
	}

	return err;
}

/*
 * int
 * pfc_ipcsrv_evdesc_destroy(pfc_ipcevdesc_t desc)
 *	Destroy the given IPC event delivery descriptor.
 *
 *	After the call of this function, Concurrent call of
 *	pfc_ipcsrv_evdesc_wait() or pfc_ipcsrv_evdesc_wait_abs() will return
 *	ENXIO error.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcsrv_evdesc_destroy(pfc_ipcevdesc_t desc)
{
	ipc_evdescmgr_t	*dmgr = &ipc_evdesc_mgr;
	ipc_evdesc_t	*edp;
	ipc_evsess_t	*esess;
	pfc_ipcsrv_t	*srv;
	pfc_rbnode_t	*node;

	IPC_EVDMGR_WRLOCK(dmgr);

	/* Remove the given descriptor from the descriptor tree. */
	node = pfc_rbtree_remove(&dmgr->iedm_tree, IPC_EVDESC_KEY(desc));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		IPC_EVDMGR_UNLOCK(dmgr);

		return ENOENT;
	}

	edp = IPC_EVDESC_NODE2PTR(node);
	esess = edp->ied_sess;
	srv = &esess->ies_session;
	IPCSRV_LOCK(srv);
	edp->ied_flags |= IPC_EVDF_INVISIBLE;

	ipcsrv_evdesc_cancel_l(dmgr, edp, esess, IPC_EVDF_DESTROY);

	return 0;
}

/*
 * int
 * pfc_ipcsrv_evdesc_wait(pfc_ipcevdesc_t desc, const pfc_timespec_t *timeout)
 *	Wait for completion of event delivery associated with the given
 *	IPC event delivery descriptor.
 *
 *	The timeout expires when the time interval specified by `timeout'
 *	passes, as measured by the system monotonic clock. Specifying NULL
 *	to `timeout' means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The event delivery descriptor specified by `desc' is destroyed
 *	except for EBUSY error.
 */
int
pfc_ipcsrv_evdesc_wait(pfc_ipcevdesc_t desc, const pfc_timespec_t *timeout)
{
	pfc_timespec_t	*abstime, ts;
	int		err;

	if (timeout == NULL) {
		abstime = NULL;
	}
	else {
		abstime = &ts;
		err = pfc_clock_abstime(abstime, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* This should never happen. */
			return err;
		}
	}

	return ipcsrv_evdesc_wait(desc, abstime);
}

/*
 * int
 * pfc_ipcsrv_evdesc_wait_abs(pfc_ipcevdesc_t desc,
 *			      const pfc_timespec_t *abstime)
 *	Wait for completion of event delivery associated with the given
 *	IPC event delivery descriptor.
 *
 *	The timeout expires when the absolute time specified by `abstime'
 *	passes, as measured by the system monotonic clock. Specifying NULL
 *	to `abstime' means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The event delivery descriptor specified by `desc' is destroyed
 *	except for EBUSY error.
 */
int
pfc_ipcsrv_evdesc_wait_abs(pfc_ipcevdesc_t desc,
			   const pfc_timespec_t *abstime)
{
	return ipcsrv_evdesc_wait(desc, abstime);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcsrv_evdesc_init(void)
 *	Initialize IPC event delivery descriptor functionality.
 */
void PFC_ATTR_HIDDEN
pfc_ipcsrv_evdesc_init(void)
{
	ipc_evdescmgr_t	*dmgr = &ipc_evdesc_mgr;

	IPC_EVDMGR_WRLOCK(dmgr);

	PFC_ASSERT(dmgr->iedm_flags == 0);
	dmgr->iedm_flags = IPC_EVDMGRF_INIT;

	IPC_EVDMGR_UNLOCK(dmgr);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcsrv_evdesc_shutdown(void)
 *	Invalidate all event delivery descriptors.
 */
void PFC_ATTR_HIDDEN
pfc_ipcsrv_evdesc_shutdown(void)
{
	ipc_evdescmgr_t	*dmgr = &ipc_evdesc_mgr;
	pfc_rbtree_t	tree;

	IPC_EVDMGR_WRLOCK(dmgr);

	/* Copy descriptor tree, and clear it. */
	tree = dmgr->iedm_tree;
	pfc_rbtree_init(&dmgr->iedm_tree, pfc_rbtree_uint32_compare,
			ipcsrv_evdesc_getkey);

	/* Set shutdown flag. */
	dmgr->iedm_flags |= IPC_EVDMGRF_SHUTDOWN;

	IPC_EVDMGR_UNLOCK(dmgr);

	/* Shutdown all descriptors. */
	pfc_rbtree_clear(&tree, ipcsrv_evdesc_shutdown, dmgr);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcsrv_event_session(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			    pfc_timespec_t *PFC_RESTRICT abstime)
 *	Start IPC event listener session.
 *	`srv' must be a pointer to IPC server session associated with the
 *	IPC client.
 *
 * Calling/Exit State:
 *	This function always returns EINVAL in order to let the session
 *	thread quit.
 */
int PFC_ATTR_HIDDEN
pfc_ipcsrv_event_session(pfc_ipcsrv_t *PFC_RESTRICT srv,
			 pfc_timespec_t *PFC_RESTRICT abstime)
{
	ipc_channel_t	*chp = srv->isv_channel;
	ipc_evqueue_t	*evq;
	ipc_evqent_t	*qent, *qnext;
	pfc_iostream_t	stream;
	uint8_t		response;
	int		ret, err, fd;

	PFC_ASSERT(!IPCSRV_IS_EVENT(srv));

	/* Allocate event queue. */
	evq = (ipc_evqueue_t *)malloc(sizeof(*evq));
	if (PFC_EXPECT_FALSE(evq == NULL)) {
		IPCSRV_LOG_ERROR("%u: Failed to allocate event queue.",
				 IPCSRV_PID(srv));
		goto out;
	}

	evq->ieq_sess = srv;
	evq->ieq_queue = NULL;
	evq->ieq_qnext = &evq->ieq_queue;
	pfc_epoll_handler_init(&evq->ieq_epoll, ipcsrv_epoll_event);
	pfc_ipc_evset_init(&evq->ieq_evset);

	IPCCH_LOCK(chp);

	/* Ensure that IPC server is still running. */
	if (PFC_EXPECT_FALSE(chp->ich_flags & IPCCHF_SHUTDOWN)) {
		IPCCH_UNLOCK(chp);
		goto cleanup;
	}

	/*
	 * Destroy the input buffer.
	 * Read-ahead cache must be disabled on the EVENT listener session.
	 * Although __pfc_iostream_resize() fails if unread data exists in
	 * the input buffer, it should never happen because the IPC client
	 * should be waiting for response from the server.
	 */
	stream = srv->isv_stream;
	err = __pfc_iostream_resize(stream, 0, PFC_IOSSZ_RETAIN);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("%u: Failed to disable read-ahead cache "
				 "for EVENT session: %s", IPCSRV_PID(srv),
				 strerror(err));
		IPCCH_UNLOCK(chp);
		goto cleanup;
	}

	/*
	 * Register session socket to the epoll instance in order to
	 * watch incoming data from the client.
	 */
	fd = pfc_iostream_getfd(stream);
	ret = pfc_epoll_ctl(chp->ich_epfd, EPOLL_CTL_ADD, fd,
			    IPC_EVENT_EPOLLEV_INPUT, &evq->ieq_epoll);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		err = errno;
		IPCCH_UNLOCK(chp);
		IPCSRV_LOG_ERROR("%u: Failed to register event socket to "
				 "the epoll instance: %s", IPCSRV_PID(srv),
				 strerror(err));
		goto cleanup;
	}

	/* Make this event queue visible. */
	pfc_list_push_tail(&chp->ich_evqueues, &evq->ieq_list);
	IPCCH_UNLOCK(chp);

	/* Send response to the client. */
	response = IPC_PROTO_TRUE;
	err = pfc_ipc_write(stream, &response, sizeof(response), PFC_TRUE,
			    abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("%u: Failed to send EVENT response: %s",
				 IPCSRV_PID(srv), strerror(err));
		goto unregister;
	}

	/* Start event listener session. */
	IPCSRV_LOG_INFO("%u: Start event listener session.", IPCSRV_PID(srv));

	IPCSRV_LOCK(srv);
	srv->isv_flags |= IPCSRVF_EVLISTENER;
	while (ipcsrv_event_main(srv, evq) == 0) {}
	IPCSRV_UNLOCK(srv);

	IPCSRV_LOG_INFO("%u: Event listener session has been terminated.",
			IPCSRV_PID(srv));

unregister:
	/* Make this event queue invisible. */
	IPCCH_LOCK(chp);
	pfc_list_remove(&evq->ieq_list);
	IPCCH_UNLOCK(chp);

	/*
	 * Ensure that the listener thread removed this session socket
	 * from the epoll instance.
	 */
	ipcsrv_event_epoll_remove(srv, evq);

	/*
	 * Unlink all events posted to this queue.
	 * The session lock does not need to be held because this event queue
	 * is no longer visible to other threads.
	 */
	for (qent = evq->ieq_queue; qent != NULL; qent = qnext) {
		qnext = qent->ieqe_next;
		IPC_EVSESS_RELEASE(qent->ieqe_sess);
		free(qent);
	}

cleanup:
	pfc_ipc_evset_destroy(&evq->ieq_evset);

	free(evq);

out:
	return EINVAL;
}

/*
 * static void
 * ipcsrv_evsess_destroy(ipc_evsess_t *esess)
 *	Destroy the specified IPC event session.
 */
static void
ipcsrv_evsess_destroy(ipc_evsess_t *esess)
{
	pfc_ipcsrv_t	*srv = &esess->ies_session;
	ipc_evdesc_t	*edp;

	PFC_ASSERT(esess->ies_refcnt == 0);

	IPCSRV_LOCK(srv);

	edp = esess->ies_desc;
	if (edp != NULL) {
		uint32_t	flags = edp->ied_flags;

		/* Wake up a thread blocked on this event delivery. */
		edp->ied_flags = (flags | IPC_EVDF_COMPLETE);
		IPCSRV_SIGNAL(srv);
		IPCSRV_UNLOCK(srv);

		/* Rest of the work will be done by ipcsrv_evdesc_destroy(). */
	}
	else {
		IPCSRV_UNLOCK(srv);
		pfc_ipcsrv_sess_cleanup(srv);
		free(esess);
	}
}

/*
 * static void
 * ipcsrv_evsess_enqueue(ipc_evqueue_t *PFC_RESTRICT evq,
 *		         ipc_evsess_t *PFC_RESTRICT esess)
 *	Enqueue an IPC event to the event queue specified by `evq'.
 *
 * Remarks:
 *	- This function must be called with holding the session lock associated
 *	  with the event queue specified by `evq'.
 *
 *	- This function enqueues the event without event mask test.
 *	  Event mask test must be done by the caller.
 */
static void
ipcsrv_evsess_enqueue(ipc_evqueue_t *PFC_RESTRICT evq,
		      ipc_evsess_t *PFC_RESTRICT esess)
{
	pfc_ipcsrv_t	*srv = evq->ieq_sess;
	ipc_evqent_t	*qent;

	if (IPCSRV_IS_EVQ_SHUTDOWN(srv)) {
		/* This event listener is going to quit. */
		return;
	}

	/* Test event mask of this event listener. */
	if (!pfc_ipc_evset_contains(&evq->ieq_evset,
				    esess->ies_session.isv_name,
				    esess->ies_event.iev_type)) {
		/* The event listener doesn't want this type of event. */
		return;
	}

	/* Allocate a new queue entry. */
	qent = (ipc_evqent_t *)malloc(sizeof(*qent));
	if (PFC_EXPECT_FALSE(qent == NULL)) {
		IPCSRV_LOG_ERROR("%u: Unable to enqueue IPC event: %u/%s/%u",
				 IPCSRV_PID(srv),
				 esess->ies_event.iev_serial,
				 esess->ies_session.isv_name,
				 esess->ies_event.iev_type);

		return;
	}

	qent->ieqe_next = NULL;
	qent->ieqe_sess = esess;
	IPC_EVSESS_HOLD(esess);

	/* Enqueue the event. */
	*(evq->ieq_qnext) = qent;
	evq->ieq_qnext = &qent->ieqe_next;
	IPCSRV_SIGNAL(srv);
}

/*
 * static int
 * ipcsrv_epoll_event(pfc_ephdlr_t *ephp, uint32_t events, void *arg)
 *	Handle epoll read event on the IPC event session.
 *
 * Calling/Exit State:
 *	Zero is always returned.
 */
static int
ipcsrv_epoll_event(pfc_ephdlr_t *ephp, uint32_t events, void *arg)
{
	ipc_evqueue_t	*evq = IPC_EVQUEUE_EP2PTR(ephp);
	pfc_ipcsrv_t	*srv = evq->ieq_sess;

	IPCSRV_LOCK(srv);

	if (PFC_EXPECT_FALSE(events & IPC_EVENT_EPOLLEV_TEST_RESET)) {
		ipc_channel_t	*chp = srv->isv_channel;
		int		ret, fd;

		/*
		 * The connection was reset by IPC client, or the event
		 * listener session is going to quit. So the session socket
		 * must be removed from the epoll instance.
		 */
		fd = pfc_iostream_getfd(srv->isv_stream);
		ret = pfc_epoll_ctl(chp->ich_epfd, EPOLL_CTL_DEL, fd, 0, NULL);
		if (PFC_EXPECT_FALSE(ret != 0)) {
			/* This should never happen. */
			IPCSRV_LOG_ERROR("%u: Failed to remove event socket "
					 "from the epoll instance: %s",
					 IPCSRV_PID(srv), strerror(errno));
			/* FALLTHROUGH */
		}
		pfc_ipcsrv_setflags_l(srv, IPCSRVF_EVQ_SHUTDOWN);
	}
	else if (PFC_EXPECT_TRUE(events & EPOLLIN)) {
		/* Disable input event on the session socket. */
		ipc_event_epoll_disable_input(srv, evq);

		/* Wake up session thread to update event mask. */
		pfc_ipcsrv_setflags_l(srv, IPCSRVF_EVQ_INPUT);
	}

	IPCSRV_UNLOCK(srv);

	return 0;
}

/*
 * static int
 * ipcsrv_event_postto(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *		       const pfc_ipccladdr_t *PFC_RESTRICT claddr)
 *	Post an IPC event to the IPC client specified by `claddr'.
 *
 *	If `claddr' is NULL, the specified IPC event is broadcasted to all
 *	IPC event listeners.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function implies pfc_ipcsrv_event_destroy().
 *	That is, the specified IPC event session is always destroyed unless
 *	the specified session is not an event session.
 */
static int
ipcsrv_event_postto(pfc_ipcsrv_t *PFC_RESTRICT srv,
		    const pfc_ipccladdr_t *PFC_RESTRICT claddr)
{
	ipc_evsess_t	*esess;
	ipc_stream_t	*stp;
	ipc_channel_t	*chp;
	ipc_evproto_t	*iep;
	pfc_timespec_t	cur;
	pfc_ipcevid_t	serial;
	pfc_list_t	*elem;
	int		err;

	if (PFC_EXPECT_FALSE(srv == NULL || !IPCSRV_IS_EVENT(srv))) {
		return EINVAL;
	}

	esess = IPC_EVSESS_SRV2SESS(srv);

	if (claddr != NULL) {
		int	type;

		/* Ensure that the host address is initialized. */
		type = pfc_hostaddr_gettype(&claddr->cla_hostaddr);
		if (PFC_EXPECT_FALSE(type == AF_UNSPEC)) {
			err = EINVAL;
			goto error;
		}
	}

	stp = &srv->isv_output;
	if (PFC_EXPECT_FALSE(pfc_ipcstream_is_broken(stp))) {
		/* Output stream is broken. */
		err = EPERM;
		goto error;
	}

	/* Update creation time of the event. */
	err = pfc_clock_get_realtime(&cur);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCSRV_LOG_ERROR("Failed to obtain system realtime clock: %s",
				 strerror(err));
		goto error;
	}

	iep = &esess->ies_event;
	iep->iev_time_sec = cur.tv_sec;
	iep->iev_time_nsec = cur.tv_nsec;

	IPCSRV_LOG_DEBUG("Posting IPC event: %u/%s/%u", iep->iev_serial,
			 esess->ies_session.isv_name, iep->iev_type);

	/* Iterate all event listeners. */
	chp = srv->isv_channel;
	IPCCH_LOCK(chp);

	/* Ensure that the IPC server is available. */
	err = pfc_ipcsrv_channel_check(chp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_unlock;
	}

	if (PFC_EXPECT_FALSE(chp->ich_listener == -1)) {
		err = ECANCELED;
		goto error_unlock;
	}

	/* Assign serial ID to this event. */
	serial = chp->ich_evserial_next;
	PFC_ASSERT(serial != PFC_IPC_EVSERIAL_INVALID);
	iep->iev_serial = serial;

	serial++;
	if (PFC_EXPECT_FALSE(serial == PFC_IPC_EVSERIAL_INVALID)) {
		serial = IPC_EVENT_SERIAL_INITIAL;
	}
	chp->ich_evserial_next = serial;

	PFC_LIST_FOREACH(&chp->ich_evqueues, elem) {
		ipc_evqueue_t	*evq = IPC_EVQUEUE_LIST2PTR(elem);
		pfc_ipcsrv_t	*qsrv = evq->ieq_sess;

		/* Test client address. */
		if (claddr != NULL &&
		    (IPCSRV_PID(qsrv) != claddr->cla_pid ||
		     pfc_hostaddr_compare(&claddr->cla_hostaddr,
					  &qsrv->isv_addr) != 0)) {
			continue;
		}

		/*
		 * Post the event to this event listener if the event type
		 * matches the event mask sent by the IPC client.
		 */
		IPCSRV_LOCK(qsrv);
		ipcsrv_evsess_enqueue(evq, esess);
		IPCSRV_UNLOCK(qsrv);
	}

	IPCCH_UNLOCK(chp);
	IPC_EVSESS_RELEASE(esess);

	return 0;

error_unlock:
	IPCCH_UNLOCK(chp);

error:
	/*
	 * Cancel wait on the event delivery descriptor associated with
	 * this event.
	 */
	ipcsrv_evdesc_cancel(esess);
	IPC_EVSESS_RELEASE(esess);

	return err;
}

/*
 * static int
 * ipcsrv_event_main(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *		     ipc_evqueue_t *PFC_RESTRICT evq)
 *	Main routine of IPC event listener session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the server session lock.
 *	Note that this function may release the lock for a while.
 */
static int
ipcsrv_event_main(pfc_ipcsrv_t *PFC_RESTRICT srv,
		  ipc_evqueue_t *PFC_RESTRICT evq)
{
	ipc_evqent_t	*qent;
	int		err;

	if (IPCSRV_IS_EVQ_SHUTDOWN(srv)) {
		/* The listener thread removed this session socket. */
		return ECANCELED;
	}

	if (srv->isv_flags & IPCSRVF_EVQ_INPUT) {
		/* Update event mask as required by the IPC client. */
		srv->isv_flags &= ~IPCSRVF_EVQ_INPUT;
		IPCSRV_UNLOCK(srv);
		err = ipcsrv_event_recv(srv, evq);
		IPCSRV_LOCK(srv);

		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		/* Restart read event watching on the listener thread. */
		return ipc_event_epoll_enable_input(srv, evq);
	}

	if ((qent = evq->ieq_queue) != NULL) {
		ipc_evsess_t	*esess;

		/* Dequeue one event from the queue. */
		evq->ieq_queue = qent->ieqe_next;
		if (evq->ieq_queue == NULL) {
			evq->ieq_qnext = &evq->ieq_queue;
		}
		IPCSRV_UNLOCK(srv);

		esess = qent->ieqe_sess;
		free(qent);

		/* Send this IPC event to the IPC client. */
		err = ipcsrv_event_send(srv, esess);
		IPC_EVSESS_RELEASE(esess);

		IPCSRV_LOCK(srv);

		return err;
	}

	/* Block the server session thread. */
	IPCSRV_WAIT(srv);

	return 0;
}

/*
 * static int
 * ipcsrv_event_recv(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *		     ipc_evqueue_t *PFC_RESTRICT evq)
 *	Receive set of IPC event masks from the IPC client, and update set of
 *	event masks for the specified event queue.
 *
 *	The caller must guarantee that incoming data exists on the server
 *	session socket.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	ECONNRESET is returned if the connection was reset.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_event_recv(pfc_ipcsrv_t *PFC_RESTRICT srv,
		  ipc_evqueue_t *PFC_RESTRICT evq)
{
	ipc_channel_t	*chp = srv->isv_channel;
	ipc_cevcmdops_t	*ops;
	pfc_timespec_t	*abstime, tsbuf;
	pfc_iostream_t	stream = srv->isv_stream;
	uint8_t		cmd;
	int		err;

	/* Initialize session timeout. */
	err = pfc_ipcsrv_timeout_init(chp, &abstime, &tsbuf);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Receive subcommand. */
	err = pfc_ipc_read(stream, &cmd, sizeof(cmd), abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("%u: Failed to read EVENT subcommand: %s",
				 IPCSRV_PID(srv), strerror(err));

		return err;
	}

	if (PFC_EXPECT_FALSE(cmd >= PFC_ARRAY_CAPACITY(ipc_event_commands))) {
		IPCSRV_LOG_ERROR("%u: Unexpected EVENT subcommand: 0x%x",
				 IPCSRV_PID(srv), cmd);

		return EPROTO;
	}

	/* Execute subcommand. */
	ops = &ipc_event_commands[cmd];

	return ops->evcmd_exec(srv, evq, abstime);
}

/*
 * static int
 * ipcsrv_event_recvmask(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			 ipc_tgtlist_t **PFC_RESTRICT headp,
 *			 ctimespec_t *PFC_RESTRICT abstime)
 *	Receive a pair of IPC service name and event mask from the IPC client.
 *
 *	`headp' must be a pointer to the head of ipc_tgtlist_t list.
 *	Received IPC service name and event mask are set into ipc_tgtlist_t
 *	instance, and it is linked to the list specified by `headp'.
 *	The list of ipc_tgtlist_t must be terminated by NULL. So the caller
 *	must set NULL to the buffer pointed by `headp' on the first call.
 *
 *	The caller must guarantee that incoming data exists on the server
 *	session socket.
 *
 * Calling/Exit State:
 *	Upon successful completion, an ipc_tgtlist_t instance is linked to
 *	the list pointed by `headp', and zero is returned.
 *	-1 is returned if no more event mask exists to be updated.
 *
 *	ECONNRESET is returned if the connection was reset.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	ipc_tgtlist_t instance linked to the list pointed by `headp' must be
 *	released by the caller.
 */
static int
ipcsrv_event_recvmask(pfc_ipcsrv_t *PFC_RESTRICT srv,
		      ipc_tgtlist_t **PFC_RESTRICT headp,
		      ctimespec_t *PFC_RESTRICT abstime)
{
	ipc_sess_t	*sess = &srv->isv_session;
	ipc_tgtlist_t	*tgt;
	pfc_iostream_t	stream = sess->iss_stream;
	uint8_t		namelen;
	int		err;

	/* Receive length of the IPC service name. */
	err = pfc_ipc_read(stream, &namelen, sizeof(namelen), abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err != ECONNRESET) {
			IPCSRV_LOG_ERROR("%u: Failed to read service name "
					 "length for event mask: %s",
					 IPCSRV_PID(srv), strerror(err));
		}

		return err;
	}

	if (namelen == 0) {
		/* This is EOF mark. */
		return -1;
	}

	if (PFC_EXPECT_FALSE(namelen >= sizeof(tgt->itl_name))) {
		IPCSRV_LOG_ERROR("%u: "
				 "Too long service name for event mask: %u",
				 IPCSRV_PID(srv), namelen);

		return EINVAL;
	}

	/* Allocate a new ipc_tgtlist_t entry. */
	tgt = (ipc_tgtlist_t *)malloc(sizeof(*tgt));
	if (PFC_EXPECT_FALSE(tgt == NULL)) {
		IPCSRV_LOG_ERROR("%u: "
				 "Failed to allocate target event entry.",
				 IPCSRV_PID(srv));

		return ENOMEM;
	}

	/* Receive IPC service name. */
	err = pfc_ipc_read(stream, tgt->itl_name, namelen, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("%u: Failed to read service name for event"
				 " mask: %s", IPCSRV_PID(srv), strerror(err));
		goto error;
	}
	tgt->itl_name[namelen] = '\0';

	/* Receive event mask. */
	err = pfc_ipc_read(stream, &tgt->itl_mask, sizeof(tgt->itl_mask),
			   abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("%u: Failed to read event mask: %s",
				 IPCSRV_PID(srv), strerror(err));
		goto error;
	}
	IPC_SESS_BSWAP_INT(sess, tgt->itl_mask);

	IPCSRV_LOG_VERBOSE("%u: Received mask: %s, 0x%" IPC_PFMT_EVMASK,
			   IPCSRV_PID(srv), tgt->itl_name, tgt->itl_mask);
	tgt->itl_next = *headp;
	*headp = tgt;

	return 0;

error:
	free(tgt);

	return err;
}

/*
 * static void
 * ipcsrv_event_epoll_remove(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			     ipc_evqueue_t *PFC_RESTRICT evq)
 *	Remove the event listener socket from the epoll instance.
 */
static void
ipcsrv_event_epoll_remove(pfc_ipcsrv_t *PFC_RESTRICT srv,
			  ipc_evqueue_t *PFC_RESTRICT evq)
{
	ipc_channel_t	*chp = srv->isv_channel;
	int		ret, fd;

	IPCSRV_LOCK(srv);

	if (IPCSRV_IS_EVQ_SHUTDOWN(srv)) {
		/* Already removed. */
		goto out;
	}

	/*
	 * Set EPOLLOUT event bit, and let the listener thread remove this
	 * event listener session socket.
	 */
	fd = pfc_iostream_getfd(srv->isv_stream);
	ret = pfc_epoll_ctl(chp->ich_epfd, EPOLL_CTL_MOD, fd, EPOLLOUT,
			    &evq->ieq_epoll);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		int	err = errno;

		/* This should never happen. */
		IPCSRV_LOG_ERROR("%u: Failed to set EPOLLOUT event to the "
				 "event listener socket: %s",
				 IPCSRV_PID(srv), strerror(err));

		/* Ignore this error. */
		goto out;
	}

	/*
	 * Wait for the listener thread to remove the session socket from
	 * the epoll instance.
	 */
	IPCSRV_LOG_DEBUG("%u: Waiting for the listener thread to remove "
			 "event session.", IPCSRV_PID(srv));
	do {
		IPCSRV_WAIT(srv);
	} while (!IPCSRV_IS_EVQ_SHUTDOWN(srv));

out:
	IPCSRV_UNLOCK(srv);
}

/*
 * static int
 * ipcsrv_event_send(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *		     ipc_evsess_t *PFC_RESTRICT esess)
 *	Send an IPC event to the IPC client.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_event_send(pfc_ipcsrv_t *PFC_RESTRICT srv,
		  ipc_evsess_t *PFC_RESTRICT esess)
{
	pfc_ipcsrv_t	*evsrv = &esess->ies_session;
	ipc_evproto_t	*iep = &esess->ies_event;
	pfc_timespec_t	*abstime, cur;
	ipc_stream_t	*evstp = &evsrv->isv_output;
	ipc_sess_t	*sess = &srv->isv_session;
	pfc_iostream_t	stream = sess->iss_stream;
	uint8_t		type = IPC_EVENTTYPE_EVENT;
	int		err;

	PFC_ASSERT(srv->isv_channel == &ipc_channel);
	PFC_ASSERT(srv->isv_channel == evsrv->isv_channel);
	PFC_ASSERT(IPCSRV_IS_EVENT(evsrv));

	/* Obtain current system time. */
	err = pfc_ipcsrv_gettime(&cur);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		return err;
	}

	/* Determine session timeout. */
	ipcsrv_event_timeout_init(evsrv, &cur, &abstime);

	/* Send type of event data. */
	err = pfc_ipc_write(stream, &type, sizeof(type), PFC_FALSE, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("%u: Failed to send IPC event data type: "
				 "%s", IPCSRV_PID(srv), strerror(err));

		return err;
	}

	/* Send common event data. */
	err = pfc_ipc_write(stream, iep, sizeof(*iep), PFC_FALSE, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("%u: Failed to send IPC event data: %s",
				 IPCSRV_PID(srv), strerror(err));

		return err;
	}

	/* Send IPC service name associated with the event. */
	err = pfc_ipc_write(stream, evsrv->isv_name, iep->iev_namelen,
			    PFC_FALSE, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("%u: Failed to send IPC service name: %s",
				 IPCSRV_PID(srv), strerror(err));

		return err;
	}

	/* Send additional data. */
	PFC_ASSERT(!pfc_ipcstream_is_broken(evstp));
	PFC_ASSERT(evstp->is_flags & IPC_STRF_EVENT);
	err = pfc_ipcstream_send(sess, evstp, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("%u: Failed to send IPC event message: %s",
				 IPCSRV_PID(srv), strerror(err));
	}

	return err;
}

/*
 * static void
 * ipcsrv_event_timeout_init(pfc_ipcsrv_t *PFC_RESTRICT evsrv,
 *			     ctimespec_t *PFC_RESTRICT cur,
 *			     pfc_timespec_t **PFC_RESTRICT absp)
 *	Determine I/O timeout to send an IPC event.
 *	`evsrv' must be a pointer to pseudo IPC server session in
 *	ipc_evsess_t.
 *
 *	`cur' must be a pointer to pfc_timespec_t which contains current
 *	system time.
 *
 *	If an infinite timeout is configured, NULL is set to the buffer pointed
 *	by `absp'.
 *	Otherwise, the system absolute expiry time of I/O is set
 *	`evsrv->isv_timeout', and its address is set to the buffer pointed by
 *	`absp'.
 */
static void
ipcsrv_event_timeout_init(pfc_ipcsrv_t *PFC_RESTRICT evsrv,
			  ctimespec_t *PFC_RESTRICT cur,
			  pfc_timespec_t **PFC_RESTRICT absp)
{
	ipc_channel_t	*chp = evsrv->isv_channel;
	uint32_t	flags = evsrv->isv_flags;
	pfc_timespec_t	*tp = &evsrv->isv_timeout;

	if ((flags & IPCSRVF_HASTIMEOUT) == 0) {
		/* Use default timeout. */
		tp->tv_sec = chp->ich_timeout;
		tp->tv_nsec = 0;
	}
	else if (flags & IPCSRVF_NOTIMEOUT) {
		/* Use an infinite timeout. */
		*absp = NULL;

		return;
	}

	/* Convert timeout into absolute expiry time. */
	pfc_timespec_add(tp, cur);
	*absp = tp;
}

/*
 * static int
 * ipcsrv_evdesc_create(ipc_evdescmgr_t *PFC_RESTRICT dmgr,
 *			ipc_evsess_t *PFC_RESTRICT esess)
 *	Create a new IPC event delivery descriptor associated with the given
 *	event session.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new IPC event delivery descriptor is
 *	set to esess->ies_desc, and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the descriptor manager lock
 *	in writer mode, and the event session lock associated with `esess'.
 */
static int
ipcsrv_evdesc_create(ipc_evdescmgr_t *PFC_RESTRICT dmgr,
		     ipc_evsess_t *PFC_RESTRICT esess)
{
	ipc_evdesc_t	*edp;
	pfc_ipcevdesc_t	desc, start;
	pfc_rbtree_t	*tree = &dmgr->iedm_tree;
	int		err;

	PFC_ASSERT(esess->ies_desc == NULL);

	edp = (ipc_evdesc_t *)malloc(sizeof(*edp));
	if (PFC_EXPECT_FALSE(edp == NULL)) {
		return ENOMEM;
	}

	edp->ied_flags = 0;
	edp->ied_sess = esess;

	/* Assign a new descriptor. */
	start = desc = dmgr->iedm_next;
	PFC_ASSERT(desc != PFC_IPCEVDESC_INVALID);

	for (;;) {
		pfc_ipcevdesc_t	d;

		edp->ied_desc = desc;
		err = pfc_rbtree_put(tree, &edp->ied_node);

		/* Prepare descriptor for next allocation. */
		d = desc + 1;
		if (PFC_EXPECT_FALSE(d == PFC_IPCEVDESC_INVALID)) {
			d = IPC_EVDESC_INITIAL;
		}

		if (PFC_EXPECT_TRUE(err == 0)) {
			dmgr->iedm_next = d;
			esess->ies_desc = edp;

			return 0;
		}

		/* Try next ID. */
		desc = d;
		if (PFC_EXPECT_FALSE(desc == start)) {
			/* No ID is available. */
			break;
		}
	}

	return ENFILE;
}

/*
 * static int
 * ipcsrv_evdesc_wait(pfc_ipcevdesc_t desc,
 *		      const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Wait for completion of event delivery associated with the given
 *	IPC event delivery descriptor.
 *
 *	The timeout expires when the absolute time specified by `abstime'
 *	passes, as measured by the system monotonic clock. Specifying NULL
 *	to `abstime' means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The event delivery descriptor specified by `desc' is destroyed
 *	except for ENOENT and EBUSY error.
 */
static int
ipcsrv_evdesc_wait(pfc_ipcevdesc_t desc,
		   const pfc_timespec_t *PFC_RESTRICT abstime)
{
	ipc_evdescmgr_t	*dmgr = &ipc_evdesc_mgr;
	ipc_evdesc_t	*edp;
	ipc_evsess_t	*esess;
	pfc_ipcsrv_t	*srv;
	pfc_rbnode_t	*node;
	uint32_t	flags;
	int		err;

	IPC_EVDMGR_RDLOCK(dmgr);

	/* Obtain IPC event delivery descriptor. */
	node = pfc_rbtree_get(&dmgr->iedm_tree, IPC_EVDESC_KEY(desc));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		IPC_EVDMGR_UNLOCK(dmgr);

		return ENOENT;
	}

	edp = IPC_EVDESC_NODE2PTR(node);
	PFC_ASSERT(edp->ied_desc == desc);
	esess = edp->ied_sess;
	srv = &esess->ies_session;

	IPCSRV_LOCK(srv);

	flags = edp->ied_flags;
	if (PFC_EXPECT_FALSE(flags & IPC_EVDF_WAITING)) {
		/* Another thread is already waiting. */
		IPCSRV_UNLOCK(srv);
		IPC_EVDMGR_UNLOCK(dmgr);

		return EBUSY;
	}

	flags |= IPC_EVDF_WAITING;
	edp->ied_flags = flags;
	IPC_EVDMGR_UNLOCK(dmgr);

	err = 0;
	for (;;) {
		if (PFC_EXPECT_FALSE(flags & IPC_EVDF_SHUTDOWN)) {
			/* IPC server is unavailable. */
			err = ECANCELED;
			break;
		}
		if (PFC_EXPECT_FALSE(flags & IPC_EVDF_DESTROY)) {
			/*
			 * Descriptor was destroyed by
			 * pfc_ipcsrv_evdesc_destroy().
			 */
			err = ENXIO;
			break;
		}
		if (flags & IPC_EVDF_COMPLETE) {
			err = 0;
			break;
		}

		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ETIMEDOUT);
			break;
		}

		err = IPCSRV_TIMEDWAIT_ABS(srv, abstime);
		flags = edp->ied_flags;
	}

	IPCSRV_UNLOCK(srv);
	IPC_EVDMGR_WRLOCK(dmgr);
	IPCSRV_LOCK(srv);

	/* Destroy the event delivery descriptor. */
	edp->ied_flags &= ~IPC_EVDF_WAITING;
	ipcsrv_evdesc_destroy(dmgr, edp, esess);

	return err;
}

/*
 * static void
 * ipcsrv_evdesc_cancel(ipc_evsess_t *esess)
 *	Cancel wait on the event delivery descriptor associated with the
 *	given event.
 *	The descriptor is destroyed if no thread waits on it.
 */
static void
ipcsrv_evdesc_cancel(ipc_evsess_t *esess)
{
	ipc_evdescmgr_t	*dmgr = &ipc_evdesc_mgr;
	ipc_evdesc_t	*edp;
	pfc_ipcsrv_t	*srv = &esess->ies_session;

	IPC_EVDMGR_WRLOCK(dmgr);
	IPCSRV_LOCK(srv);
	if ((edp = esess->ies_desc) != NULL) {
		ipcsrv_evdesc_cancel_l(dmgr, edp, esess, IPC_EVDF_DESTROY);
	}
	else {
		IPCSRV_UNLOCK(srv);
		IPC_EVDMGR_UNLOCK(dmgr);
	}
}

/*
 * static void
 * ipcsrv_evdesc_cancel_l(ipc_evdescmgr_t *PFC_RESTRICT dmgr,
 * 			  ipc_evdesc_t *PFC_RESTRICT edp,
 *			  ipc_evsess_t *PFC_RESTRICT esess, uint32_t cflags)
 *	Cancel wait on the given event delivery descriptor.
 *	The descriptor is destroyed if no thread waits on it.
 *
 *	`cflags' is a flag to be set to edp->ied_flags.
 *
 * Remarks:
 *	This function must be called with holding the descriptor manager lock
 *	in writer mode, and the event session lock associated with `esess'.
 *	Both locks are always released on return.
 */
static void
ipcsrv_evdesc_cancel_l(ipc_evdescmgr_t *PFC_RESTRICT dmgr,
		       ipc_evdesc_t *PFC_RESTRICT edp,
		       ipc_evsess_t *PFC_RESTRICT esess, uint32_t cflags)
{
	pfc_ipcsrv_t	*srv = &esess->ies_session;
	uint32_t	flags = edp->ied_flags;

	PFC_ASSERT(edp->ied_sess == esess);
	PFC_ASSERT(esess->ies_desc == edp);
	PFC_ASSERT(cflags == IPC_EVDF_DESTROY || cflags == IPC_EVDF_SHUTDOWN);

	if (flags & IPC_EVDF_WAITING) {
		/* Cancel wait on this descriptor. */
		edp->ied_flags = (flags | cflags);
		IPCSRV_SIGNAL(srv);
		IPCSRV_UNLOCK(srv);
		IPC_EVDMGR_UNLOCK(dmgr);
	}
	else {
		/* Destroy the descriptor. */
		ipcsrv_evdesc_destroy(dmgr, edp, esess);
	}
}

/*
 * static void
 * ipcsrv_evdesc_destroy(ipc_evdescmgr_t *PFC_RESTRICT dmgr,
 *			 ipc_evdesc_t *PFC_RESTRICT edp)
 *	Destroy the given IPC event delivery descriptor.
 *
 * Remarks:
 *	This function must be called with holding the descriptor manager lock
 *	in writer mode, and the event session lock associated with `esess'.
 *	Both locks are always released on return.
 */
static void
ipcsrv_evdesc_destroy(ipc_evdescmgr_t *PFC_RESTRICT dmgr,
		      ipc_evdesc_t *PFC_RESTRICT edp,
		      ipc_evsess_t *PFC_RESTRICT esess)
{
	pfc_ipcsrv_t	*srv = &esess->ies_session;
	uint32_t	flags = edp->ied_flags;

	PFC_ASSERT(edp->ied_sess == esess);
	PFC_ASSERT(esess->ies_desc == edp);

	if (PFC_EXPECT_TRUE((dmgr->iedm_flags & IPC_EVDMGRF_SHUTDOWN) == 0) &&
	    (flags & IPC_EVDF_INVISIBLE) == 0) {
		/* Unlink descriptor from the descriptor tree. */
		pfc_rbtree_remove_node(&dmgr->iedm_tree, &edp->ied_node);
		flags |= IPC_EVDF_INVISIBLE;
		edp->ied_flags = flags;
	}

	/* Uninstall descriptor from the event session. */
	esess->ies_desc = NULL;

	PFC_ASSERT((flags & IPC_EVDF_WAITING) == 0);
	IPCSRV_UNLOCK(srv);
	IPC_EVDMGR_UNLOCK(dmgr);

	free(edp);

	if (flags & IPC_EVDF_COMPLETE) {
		/*
		 * Destructor of event session is already called.
		 * So this event session must be destroyed here.
		 */
		PFC_ASSERT(esess->ies_refcnt == 0);
		pfc_ipcsrv_sess_cleanup(srv);
		free(esess);
	}
}

/*
 * static void
 * ipcsrv_evdesc_shutdown(pfc_rbnode_t *node, pfc_ptr_t arg)
 *	Shutdown the given event delivery descriptor.
 */
static void
ipcsrv_evdesc_shutdown(pfc_rbnode_t *node, pfc_ptr_t arg)
{
	ipc_evdescmgr_t	*dmgr = (ipc_evdescmgr_t *)arg;
	ipc_evdesc_t	*edp = IPC_EVDESC_NODE2PTR(node);
	ipc_evsess_t	*esess = edp->ied_sess;
	pfc_ipcsrv_t	*srv = &esess->ies_session;

	PFC_ASSERT(dmgr == &ipc_evdesc_mgr);

	IPC_EVDMGR_WRLOCK(dmgr);
	IPCSRV_LOCK(srv);
	ipcsrv_evdesc_cancel_l(dmgr, edp, esess, IPC_EVDF_SHUTDOWN);
}

/*
 * static pfc_cptr_t
 * ipcsrv_evdesc_getkey(pfc_rbnode_t *node)
 *	Return the key of the given event delivery descriptor.
 *	`node' must be a pointer to ied_node in ipc_evdesc_t.
 */
static pfc_cptr_t
ipcsrv_evdesc_getkey(pfc_rbnode_t *node)
{
	ipc_evdesc_t	*edp = IPC_EVDESC_NODE2PTR(node);

	return IPC_EVDESC_KEY(edp->ied_desc);
}

/*
 * static int
 * ipcsrv_evcmd_mask_add(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			 ipc_evqueue_t *PFC_RESTRICT evq,
 *			 ctimespec_t *PFC_RESTRICT abstime)
 *	Execute IPC_EVENTCMD_MASK_ADD event subcommand.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_evcmd_mask_add(pfc_ipcsrv_t *PFC_RESTRICT srv,
		      ipc_evqueue_t *PFC_RESTRICT evq,
		      ctimespec_t *PFC_RESTRICT abstime)
{
	ipc_tgtlist_t	*tgt = NULL, *next;
	int		err;

	/* Receive pairs of IPC service name and event mask. */
	while ((err = ipcsrv_event_recvmask(srv, &tgt, abstime)) == 0) {}
	if (PFC_EXPECT_FALSE(err > 0)) {
		return err;
	}

	IPCSRV_LOCK(srv);

	/*
	 * Apply received event mask to the event set of this listener
	 * session.
	 */
	for (; tgt != NULL; tgt = next) {
		next = tgt->itl_next;

		err = pfc_ipc_evset_add(&evq->ieq_evset, tgt->itl_name,
					&tgt->itl_mask);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPCSRV_UNLOCK(srv);
			IPCSRV_LOG_ERROR("%u: Failed to append event mask: %s",
					 IPCSRV_PID(srv), strerror(err));
			free(tgt);
			for (tgt = next; tgt != NULL; tgt = next) {
				free(tgt);
			}

			return err;
		}
		else {
			IPCSRV_LOG_VERBOSE("%u: event: MASK_ADD: name=%s, "
					   "mask=0x%" IPC_PFMT_EVMASK,
					   IPCSRV_PID(srv),
					   tgt->itl_name, tgt->itl_mask);
			free(tgt);
		}
	}

	IPCSRV_UNLOCK(srv);

	return 0;
}

/*
 * static int
 * ipcsrv_evcmd_mask_del(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			 ipc_evqueue_t *PFC_RESTRICT evq,
 *			 ctimespec_t *PFC_RESTRICT abstime)
 *	Execute IPC_EVENTCMD_MASK_DEL event subcommand.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_evcmd_mask_del(pfc_ipcsrv_t *PFC_RESTRICT srv,
		      ipc_evqueue_t *PFC_RESTRICT evq,
		      ctimespec_t *PFC_RESTRICT abstime)
{
	ipc_tgtlist_t	*tgt = NULL, *next;
	int		err;

	/* Receive pairs of IPC service name and event mask. */
	while ((err = ipcsrv_event_recvmask(srv, &tgt, abstime)) == 0) {}
	if (PFC_EXPECT_FALSE(err > 0)) {
		return err;
	}

	IPCSRV_LOCK(srv);

	/*
	 * Remove received events from the event set of this listener
	 * session.
	 */
	for (; tgt != NULL; tgt = next) {
		next = tgt->itl_next;

		pfc_ipc_evset_remove(&evq->ieq_evset, tgt->itl_name,
				     &tgt->itl_mask);
		IPCSRV_LOG_VERBOSE("%u: event: MASK_DEL: name=%s, mask=0x%"
				   IPC_PFMT_EVMASK, IPCSRV_PID(srv),
				   tgt->itl_name, tgt->itl_mask);
		free(tgt);
	}

	IPCSRV_UNLOCK(srv);

	return 0;
}

/*
 * static int
 * ipcsrv_evcmd_mask_reset(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			   ipc_evqueue_t *PFC_RESTRICT evq,
 *			   ctimespec_t *PFC_RESTRICT abstime PFC_ATTR_UNUSED)
 *	Execute IPC_EVENTCMD_MASK_RESET event subcommand.
 *
 * Calling/Exit State:
 *	Zero is always returned.
 */
static int
ipcsrv_evcmd_mask_reset(pfc_ipcsrv_t *PFC_RESTRICT srv,
			ipc_evqueue_t *PFC_RESTRICT evq,
			ctimespec_t *PFC_RESTRICT abstime PFC_ATTR_UNUSED)
{
	/* Reset target event set. */
	IPCSRV_LOCK(srv);
	pfc_ipc_evset_destroy(&evq->ieq_evset);
	IPCSRV_LOG_VERBOSE("%u: event: MASK_RESET", IPCSRV_PID(srv));
	IPCSRV_UNLOCK(srv);

	return 0;
}
