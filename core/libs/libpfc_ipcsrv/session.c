/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * session.c - Establish a new session on an IPC channel.
 */

#include <unistd.h>
#include <signal.h>
#include <pfc/socket.h>
#include <pfc/util.h>
#include <pfc/modconst.h>
#include <pfc/atomic.h>
#include <iostream_impl.h>
#include <tid_impl.h>
#include "ipcsrv_impl.h"

/*
 * Timeout value, in seconds, for IPC channel finalization.
 */
#define	IPCSRV_SHUTDOWN_TIMEOUT		PFC_CONST_U(10)

/*
 * Maximum number of events to be received by one epoll_wait() call.
 */
#define	IPCSRV_EPOLL_MAXEVENTS		8

/*
 * Destructor of callback argument.
 */
typedef struct {
	void		(*iscbd_dtor)(pfc_ptr_t arg);
	pfc_ptr_t	iscbd_arg;
} ipc_srvcbdtor_t;

/*
 * Destructors of all callbacks in the IPC server session.
 */
typedef struct {
	ipc_srvcbdtor_t	iscbds_dtor[IPCSRV_NCBTYPES];
} ipc_srvcbdtorset_t;

/*
 * Internal prototypes.
 */
static int	ipcsrv_poll(ipc_channel_t *chp);
static int	ipcsrv_sess_create(ipc_channel_t *PFC_RESTRICT chp, int sock,
				   const pfc_hostaddr_t *PFC_RESTRICT haddr,
				   ipc_coption_t *opts,
				   pfc_ipcsrv_t **PFC_RESTRICT srvp);
static int	ipcsrv_sess_alloc(ipc_channel_t *PFC_RESTRICT chp, int sock,
				  const pfc_hostaddr_t *PFC_RESTRICT haddr,
				  ipc_coption_t *opts,
				  pfc_ipcsrv_t **PFC_RESTRICT srvp);
static void	ipcsrv_sess_close(pfc_ipcsrv_t *srv, pfc_bool_t do_log);
static void	ipcsrv_sess_destroy(pfc_ipcsrv_t *srv);
static void	*ipcsrv_sess_main(void *arg);
static int	ipcsrv_sess_start(pfc_ipcsrv_t *srv);
static int	ipcsrv_sess_dispatch(pfc_ipcsrv_t *srv);
static int	ipcsrv_sess_ping(pfc_ipcsrv_t *PFC_RESTRICT srv,
				 pfc_timespec_t *PFC_RESTRICT abstime);
static int	ipcsrv_sess_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv,
				   pfc_timespec_t *PFC_RESTRICT abstime);
static int	ipcsrv_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv,
			      ipc_svresp_t *PFC_RESTRICT resp,
			      uint32_t *PFC_RESTRICT flagsp);
static int	ipcsrv_send_error(pfc_ipcsrv_t *PFC_RESTRICT srv,
				  ipc_svresp_t *PFC_RESTRICT resp,
				  const pfc_timespec_t *PFC_RESTRICT abstime);

static int	ipcsrv_settimeout(pfc_ipcsrv_t *PFC_RESTRICT srv,
				  uint32_t flags,
				  pfc_timespec_t **PFC_RESTRICT abstimep,
				  pfc_timespec_t *PFC_RESTRICT tsp,
				  const pfc_timespec_t *PFC_RESTRICT called);
static int	ipcsrv_epoll_client(pfc_ephdlr_t *ephp, uint32_t events,
				    void *arg);
static int	ipcsrv_log_econnreset(pfc_ipcsrv_t *srv, pfc_bool_t active,
				      int sock)
	PFC_FATTR_NOINLINE;

static pfc_iostream_t	ipcsrv_sync_listener(pfc_ipcsrv_t *srv);
static void		ipcsrv_sync_listener_l(pfc_ipcsrv_t *srv);

static int	ipcsrv_callback_start(ipc_srvcb_t *cbp);
static void	*ipcsrv_callback_thread(void *arg);
static void	ipcsrv_callback_clear(pfc_ipcsrv_t *PFC_RESTRICT srv,
				      ipc_srvcbdtorset_t *PFC_RESTRICT dset);
static void	ipcsrv_callback_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv,
				       ipc_srvcb_t *PFC_RESTRICT cbp);
static void	ipcsrv_callback_destroy(ipc_srvcb_t *PFC_RESTRICT cbp,
					ipc_srvcbdtor_t *PFC_RESTRICT dtorp);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipcsrv_callback_wait(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			ipc_srvcb_t *PFC_RESTRICT cbp)
 *	Wait for completion of the IPC server callback specified by `cbp'.
 *
 * Remarks:
 *	This function must be called with holding the server session lock.
 *	Note that the lock may be released for a while.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipcsrv_callback_wait(pfc_ipcsrv_t *PFC_RESTRICT srv,
		     ipc_srvcb_t *PFC_RESTRICT cbp)
{
	while (cbp->iscb_active) {
		IPCSRV_CB_WAIT(srv, cbp);
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipcsrv_cbdtor_init(ipc_srvcbdtor_t *dtorp)
 *	Zeroed out the callback destructor.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipcsrv_cbdtor_init(ipc_srvcbdtor_t *dtorp)
{
	dtorp->iscbd_dtor = NULL;
	dtorp->iscbd_arg = NULL;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipcsrv_cbdtor_copy(ipc_srvcbdtor_t *PFC_RESTRICT dtorp,
 *		      const ipc_srvcb_t *PFC_RESTRICT cbp)
 *	Copy callback argument and destructor to the buffer pointed by
 *	`dtorp'.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipcsrv_cbdtor_copy(ipc_srvcbdtor_t *PFC_RESTRICT dtorp,
		   const ipc_srvcb_t *PFC_RESTRICT cbp)
{
	dtorp->iscbd_dtor = cbp->iscb_callback.isc_argdtor;
	dtorp->iscbd_arg = cbp->iscb_arg;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipcsrv_cbdtor_invoke(ipc_srvcbdtor_t *dtorp)
 *	Invoke callback argument destructor preserved in `dtor'.
 *
 *	It is strongly recommended to call this function without holding
 *	any lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipcsrv_cbdtor_invoke(ipc_srvcbdtor_t *dtorp)
{
	if (dtorp->iscbd_dtor != NULL) {
		dtorp->iscbd_dtor(dtorp->iscbd_arg);
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipcsrv_cbdtorset_invoke(ipc_srvcbdtorset_t *dset)
 *	Invoke all callback destructors in the specified destructor set.
 *
 *	It is strongly recommended to call this function without holding
 *	any lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipcsrv_cbdtorset_invoke(ipc_srvcbdtorset_t *dset)
{
	ipc_srvcbdtor_t	*dtorp;

	for (dtorp = dset->iscbds_dtor;
	     dtorp < PFC_ARRAY_LIMIT(dset->iscbds_dtor); dtorp++) {
		ipcsrv_cbdtor_invoke(dtorp);
	}
}

/*
 * IPC protocol command operation.
 */
typedef struct {
	/*
	 * int
	 * icops_exec(pfc_ipcsrv_t *PFC_RESTRICT srv,
	 *	      pfc_timespec_t *PFC_RESTRICT abstime)
	 *	Execute an IPC protocol command on the given IPC server
	 *	session.
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, zero is returned.
	 *	Otherwise error number which indicates the cause of error is
	 *	returned.
	 */
	int	(*icops_exec)(pfc_ipcsrv_t *PFC_RESTRICT srv,
			      pfc_timespec_t *PFC_RESTRICT abstime);
} ipc_cmdops_t;

typedef const ipc_cmdops_t	ipc_ccmdops_t;

/*
 * IPC protocol command handlers.
 */
#define	IPCSRV_CMD_DECL(cmd)					\
	{							\
		.icops_exec	= ipcsrv_sess_cmd_##cmd,	\
	}

#define	ipcsrv_sess_cmd_PING		ipcsrv_sess_ping
#define	ipcsrv_sess_cmd_INVOKE		ipcsrv_sess_invoke
#define	ipcsrv_sess_cmd_EVENT		pfc_ipcsrv_event_session

static ipc_ccmdops_t	ipc_proto_commands[] = {
	IPCSRV_CMD_DECL(PING),
	IPCSRV_CMD_DECL(INVOKE),
	IPCSRV_CMD_DECL(EVENT),
};

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipcsrv_ipcmsg_assert(ipc_msg_t *msg, uint8_t bflags)
 *	Ensure that the IPC message instance is initialized correctly.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipcsrv_ipcmsg_assert(ipc_msg_t *msg, uint8_t bflags)
{
	pfc_ipcmsg_assert(msg, bflags);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipcsrv_ipcstream_assert(ipc_stream_t *stp)
 *	Ensure that the IPC stream is initialized correctly.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipcsrv_ipcstream_assert(ipc_stream_t *stp)
{
	pfc_ipcstream_assert(stp, 0);
}

/*
 * int
 * pfc_ipcsrv_main(void)
 *	Main loop of the IPC server.
 *
 *	This function listens the listener socket of the IPC channel, and
 *	processes requests from IPC clients.
 *
 *	Note that this function never returns unless an error is detected.
 *	So the caller must call this function on a thread dedicated to
 *	the IPC server work.
 *
 * Calling/Exit State:
 *	ECANCELED is returned if the IPC channel is already closed.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Note that this function always returns non-zero error number.
 *
 * Remarks:
 *	- All signals to the calling thread is blocked.
 *
 *	- Any EINTR error on event polling is simply ignored.
 *
 *	- Once ECANCELED is returned, further call of pfc_ipcsrv_main() will
 *	  return ECANCELED immediately.
 */
int
pfc_ipcsrv_main(void)
{
	ipc_channel_t	*chp = &ipc_channel;
	sigset_t	mask;
	int		err;

	err = pfc_ipcsrv_channel_hold(chp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Block all signals. */
	sigfillset(&mask);
	err = pthread_sigmask(SIG_BLOCK, &mask, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		IPCSRV_LOG_ERROR("Unable to mask signals: %s", strerror(err));
	}
	else {
		/*
		 * Wait for events on file descriptors, and dispatch received
		 * events to event handlers.
		 */
		err = ipcsrv_poll(chp);
		if (err == ECANCELED) {
			pfc_ipcsrv_shutdown(chp);
		}
	}

	pfc_ipcsrv_channel_release(chp);

	return err;
}

/*
 * int
 * pfc_ipcsrv_fini(void)
 *	Finalize the IPC channel service.
 *
 *	If shutdown FD is specified to the call of pfc_ipcsrv_init(),
 *	the caller must ensure that the shutdown event is already sent via
 *	the shutdown FD.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcsrv_fini(void)
{
	ipc_channel_t	*chp = &ipc_channel;
	pfc_timespec_t	abstime;
	int		fd, err;

	IPCCH_LOCK(chp);

	err = pfc_ipcsrv_channel_check(chp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* The IPC channel is in bad state. */
		goto out;
	}

	/* Send shutdown event to the listener thread. */
	fd = chp->ich_killfd;
	if (fd != -1) {
		chp->ich_killfd = -1;
		PFC_IPCSRV_CLOSE(fd);
	}

	err = pfc_ipcsrv_gettime(&abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto out;
	}

	abstime.tv_sec += IPCSRV_SHUTDOWN_TIMEOUT;
	chp->ich_flags |= IPCCHF_SHUTDOWN;

	while (chp->ich_nsessions != 0) {
		if (PFC_EXPECT_FALSE(err != 0)) {
			uint32_t	nsessions = chp->ich_nsessions;

			PFC_ASSERT(err == ETIMEDOUT);
			IPCSRV_LOG_ERROR("%u IPC server session%s did not "
					 "stop within %u seconds.", nsessions,
					 (nsessions > 1) ? "s"
					 : ipcsrv_str_empty,
					 IPCSRV_SHUTDOWN_TIMEOUT);
			break;
		}
		err = IPCCH_TIMEDWAIT_ABS(chp, &abstime);
	}

out:
	PFC_ASSERT(chp->ich_nsessions != 0 ||
		   (pfc_list_is_empty(&chp->ich_sessions) &&
		    pfc_list_is_empty(&chp->ich_evqueues)));
	IPCCH_UNLOCK(chp);

	if (err == 0) {
		pfc_ipcsrv_shutdown(chp);
		pfc_ipcsrv_channel_release(chp);
	}

	return err;
}

/*
 * int
 * pfc_ipcsrv_getcladdr(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			pfc_ipccladdr_t *PFC_RESTRICT claddr)
 *	Get the IPC client address associated with the IPC server session
 *	specified by `srv'.
 *
 *	`srv' must be a pointer to IPC server session passed to IPC service
 *	handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, the IPC client address associated with
 *	`srv' is set to the buffer pointed by `claddr', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The IPC client address identifies the IPC client process, not peer
 *	address of the IPC server session socket.
 */
int
pfc_ipcsrv_getcladdr(pfc_ipcsrv_t *PFC_RESTRICT srv,
		     pfc_ipccladdr_t *PFC_RESTRICT claddr)
{
	if (PFC_EXPECT_FALSE(srv == NULL || IPCSRV_IS_EVENT(srv) ||
			     claddr == NULL)) {
		return EINVAL;
	}

	claddr->cla_pid = IPCSRV_PID(srv);
	claddr->cla_hostaddr = srv->isv_addr;

	return 0;
}

/*
 * int
 * pfc_ipcsrv_setcallback(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			  pfc_ipcsrvcb_type_t type,
 *			  const pfc_ipcsrvcb_t *PFC_RESTRICT callback,
 *			  pfc_ptr_t arg)
 *	Set IPC server session callback to the server session specified by
 *	`srv'.
 *
 *      `type' is a callback type defined by pfc_ipcsrvcb_type_t.
 *	`callback' is a callback to be registered.
 *
 *	`arg' is an arbitrary pointer to be passed to callback function.
 *	Destructor function of `arg' can be specified by isc_argdtor field
 *	in `callback'. If it is not NULL, it is called when the callback
 *	is unregistered.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called by IPC service handler.
 *
 *	- Installed callback is invalidated when IPC service handler
 *	  is returned.
 *
 *	- Destructor function set in `callback' is called on error return.
 */
int
pfc_ipcsrv_setcallback(pfc_ipcsrv_t *PFC_RESTRICT srv,
		       pfc_ipcsrvcb_type_t type,
		       const pfc_ipcsrvcb_t *PFC_RESTRICT callback,
		       pfc_ptr_t arg)
{
	ipc_srvcb_t	*cbp, **cbpp;
	void		(*dtor)(pfc_ptr_t arg);
	int		err;

	/* At first, determine destructor for cleanup on error. */
	if (PFC_EXPECT_FALSE(callback == NULL)) {
		return EINVAL;
	}
	else {
		dtor = callback->isc_argdtor;
	}

	if (PFC_EXPECT_FALSE(srv == NULL || IPCSRV_IS_EVENT(srv) ||
			     !IPCSRV_CBTYPE_IS_VALID(type) ||
			     callback->isc_callback == NULL)) {
		err = EINVAL;
		goto error;
	}

	IPCSRV_LOCK(srv);

	PFC_ASSERT(srv->isv_flags & IPCSRVF_ACTIVE);

	if (IPCSRV_IS_SHUTDOWN(srv)) {
		/* IPC server is already terminated. */
		err = ECANCELED;
		goto error_unlock;
	}

	if (srv->isv_stream == NULL) {
		/* Already reset. */
		err = ECONNRESET;
		goto error_unlock;
	}

	cbpp = &srv->isv_callback[type];
	if (PFC_EXPECT_FALSE(*cbpp != NULL)) {
		/* Another callback is already registered. */
		err = EEXIST;
		goto error_unlock;
	}

	/* Allocate a new callback data. */
	cbp = (ipc_srvcb_t *)malloc(sizeof(*cbp));
	if (PFC_EXPECT_FALSE(cbp == NULL)) {
		err = ENOMEM;
		goto error_unlock;
	}

	err = pfc_cond_init(&cbp->iscb_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error_free;
	}

	/* Install new callback. */
	cbp->iscb_sess = srv;
	cbp->iscb_callback = *callback;
	cbp->iscb_arg = arg;
	cbp->iscb_type = type;
	cbp->iscb_active = PFC_FALSE;
	*cbpp = cbp;

	IPCSRV_UNLOCK(srv);

	return 0;

error_free:
	free(cbp);

error_unlock:
	IPCSRV_UNLOCK(srv);

error:
	if (dtor) {
		/* Call destructor. */
		(*dtor)(arg);
	}

	return err;
}

/*
 * void
 * pfc_ipcsrv_unsetcallback(pfc_ipcsrv_t *srv, pfc_ipcsrvcb_type_t type)
 *	Unset IPC server session callback specified by `srv' and `type'.
 *
 *	If the specified callback is being called, the calling thread is
 *	blocked until it returns.
 */
void
pfc_ipcsrv_unsetcallback(pfc_ipcsrv_t *srv, pfc_ipcsrvcb_type_t type)
{
	ipc_srvcb_t	*cbp, **cbpp;
	ipc_srvcbdtor_t	dtor;

	if (PFC_EXPECT_FALSE(srv == NULL || IPCSRV_IS_EVENT(srv) ||
			     !IPCSRV_CBTYPE_IS_VALID(type))) {
		/* Nothing to do. */
		return;
	}

	IPCSRV_LOCK(srv);

	cbpp = &srv->isv_callback[type];
	cbp = *cbpp;
	if (cbp == NULL) {
		/* The callback is not installed. */
		IPCSRV_UNLOCK(srv);

		return;
	}

	/* Uninstall the callback. */
	*cbpp = NULL;

	/* Destroy the callback. */
	PFC_ASSERT(cbp->iscb_sess == srv);
	ipcsrv_callback_destroy(cbp, &dtor);

	IPCSRV_UNLOCK(srv);

	/* Call destructor of callback argument. */
	ipcsrv_cbdtor_invoke(&dtor);
}

/*
 * void
 * pfc_ipcsrv_clearcallbacks(pfc_ipcsrv_t *srv)
 *	Unset all IPC server session callbacks in the IPC server session
 *	specified by `srv'.
 *
 *	The calling thread is blocked until all callbacks being called are
 *	returned.
 */
void
pfc_ipcsrv_clearcallbacks(pfc_ipcsrv_t *srv)
{
	ipc_srvcbdtorset_t	dset;

	if (PFC_EXPECT_FALSE(srv == NULL || IPCSRV_IS_EVENT(srv))) {
		/* Nothing to do. */
		return;
	}

	IPCSRV_LOCK(srv);
	ipcsrv_callback_clear(srv, &dset);
	IPCSRV_UNLOCK(srv);

	/* Call destructor of callbacks. */
	ipcsrv_cbdtorset_invoke(&dset);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcsrv_sess_init(ipc_channel_t *PFC_RESTRICT chp,
 *			pfc_ipcsrv_t *PFC_RESTRICT srv, int sock,
 *			const pfc_hostaddr_t *haddr, ipc_coption_t *opts)
 *	Initialize the given IPC server session.
 *
 *	If -1 is specified to `sock', the session specified by `srv' is
 *	initialized as pseudo session for sending an IPC event. In this case,
 *	both `haddr' and `opts' are ignored.
 *
 *	Otherwise, `sock' must be a valid socket descriptor associated with
 *	the client connection.
 *	  - `haddr' must contain client host address.
 *	  - If `opts' is not NULL, socket options in `opts' are applied
 *	    to the specified socket.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	If a valid socket is specified to `sock', it will be always closed
 *	on error.
 */
int PFC_ATTR_HIDDEN
pfc_ipcsrv_sess_init(ipc_channel_t *PFC_RESTRICT chp,
		     pfc_ipcsrv_t *PFC_RESTRICT srv, int sock,
		     const pfc_hostaddr_t *haddr, ipc_coption_t *opts)
{
	pfc_ucred_t	*credp;
	ipc_stream_t	*stp = &srv->isv_output;
	ipc_srvcb_t	**cbpp;
	int		err;

	err = PFC_MUTEX_INIT(&srv->isv_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error;
	}

	err = pfc_cond_init(&srv->isv_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error;
	}

	pfc_ipcmsg_init(&srv->isv_args, 0);
	srv->isv_name[0] = '\0';
	srv->isv_service = 0;
	srv->isv_channel = chp;
	srv->isv_tid = PFC_TID_INVALID;
	srv->isv_magic = IPC_PROTO_MAGIC;
	srv->isv_flags = 0;
	srv->isv_procname = NULL;
	pfc_epoll_handler_init(&srv->isv_epoll, ipcsrv_epoll_client);

	for (cbpp = srv->isv_callback;
	     cbpp < PFC_ARRAY_LIMIT(srv->isv_callback); cbpp++) {
		*cbpp = NULL;
	}

	credp = &srv->isv_cred;
	credp->pid = (pid_t)-1;
	credp->uid = (uid_t)-1;
	credp->gid = (gid_t)-1;

	if (sock == -1) {
		/* This session is a pseudo session for sending an event. */
		PFC_ASSERT_INT(pfc_hostaddr_init_local(&srv->isv_addr), 0);
		pfc_ipcstream_init(stp, IPC_STRF_EVENT);
		srv->isv_stream = NULL;
		srv->isv_event = 1;
		srv->isv_start = 0;
	}
	else {
		/* Initialize common context of IPC session. */
		err = pfc_ipc_sess_init(&srv->isv_session, haddr);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* This should never happen. */
			IPCSRV_LOG_ERROR("Failed to initialize IPC session: %s",
					 strerror(err));
			goto error;
		}

		srv->isv_event = 0;

		/* Create session stream. */
		err = pfc_ipc_iostream_create(&srv->isv_session, sock,
					      chp->ich_shutfd, opts);
		sock = -1;
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPCSRV_LOG_ERROR("Failed to create session stream: %s",
					 strerror(err));
			goto error;
		}

		/* Record session start time. */
		srv->isv_start = time(NULL);

		pfc_ipcstream_init(stp, 0);
	}

	return 0;

error:
	if (sock != -1) {
		PFC_IPCSRV_CLOSE(sock);
	}
	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcsrv_sess_cleanup(pfc_ipcsrv_t *srv)
 *	Clean up any resources held by the server session pointed by `srv'.
 */
void PFC_ATTR_HIDDEN
pfc_ipcsrv_sess_cleanup(pfc_ipcsrv_t *srv)
{
	pfc_iostream_t	stream = srv->isv_stream;
	pfc_refptr_t	*rpname = srv->isv_procname;

#ifdef	PFC_VERBOSE_DEBUG
	ipc_srvcb_t	**cbpp;

	for (cbpp = srv->isv_callback;
	     cbpp < PFC_ARRAY_LIMIT(srv->isv_callback); cbpp++) {
		PFC_ASSERT_PRINTF(*cbpp == NULL,
				  "Callback remains at %u: srv=%p",
				  (uint32_t)(cbpp - srv->isv_callback), srv);
	}
#endif	/* PFC_VERBOSE_DEBUG */

	if (stream != NULL) {
		PFC_ASSERT_INT(pfc_iostream_destroy(stream), 0);
	}
	if (rpname != NULL) {
		pfc_refptr_put(rpname);
	}

#ifdef	PFC_VERBOSE_DEBUG
	PFC_ASSERT_INT(pfc_mutex_destroy(&srv->isv_mutex), 0);
	PFC_ASSERT_INT(pfc_cond_destroy(&srv->isv_cond), 0);
#endif	/* PFC_VERBOSE_DEBUG */
	pfc_ipcmsg_destroy(&srv->isv_args);
	pfc_ipcstream_destroy(&srv->isv_output);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcsrv_epoll_shutdown(pfc_ephdlr_t *ephp, uint32_t events,
 *			     void *arg PFC_ATTR_UNUSED)
 *	Handle an epoll event on the shutdown file descriptor for the IPC
 *	channel.
 *
 * Calling/Exit State:
 *	ECANCELED is returned if a read event is detected on the shutdown
 *	file descriptor.
 *	Otherwise zero is returned.
 */
int PFC_ATTR_HIDDEN
pfc_ipcsrv_epoll_shutdown(pfc_ephdlr_t *ephp, uint32_t events,
			  void *arg PFC_ATTR_UNUSED)
{
	ipc_channel_t	*chp;
	pfc_list_t	*elem;

	if (PFC_EXPECT_FALSE(events == 0)) {
		return 0;
	}

	chp = IPCCH_SHUT_EP2PTR(ephp);
	PFC_ASSERT(chp == &ipc_channel);

	IPCCH_LOCK(chp);

	PFC_LIST_FOREACH(&chp->ich_sessions, elem) {
		pfc_ipcsrv_t	*srv = IPCSRV_LIST2PTR(elem);

		IPCSRV_LOCK(srv);

		/* Set shutdown flag. */
		srv->isv_flags |= IPCSRVF_SHUTDOWN;

		/*
		 * The listener thread no longer detects any client connection
		 * reset. So we must wake up all server session threads
		 * waiting for completion of connection watching.
		 */
		if (srv->isv_flags & IPCSRVF_WATCH) {
			srv->isv_flags &= ~IPCSRVF_WATCH;
		}

		IPCSRV_SIGNAL(srv);
		IPCSRV_UNLOCK(srv);
	}

	IPCCH_UNLOCK(chp);

	return ECANCELED;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcsrv_epoll_listener(pfc_ephdlr_t *ephp, uint32_t events,
 *			     void *arg PFC_ATTR_UNUSED)
 *	Handle an epoll event on the listener socket for the IPC channel.
 *
 *	This function creates a new thread for an IPC server session, and
 *	starts the session on the thread.
 *
 * Calling/Exit State:
 *	Zero is always returned.
 */
int PFC_ATTR_HIDDEN
pfc_ipcsrv_epoll_listener(pfc_ephdlr_t *ephp, uint32_t events,
			  void *arg PFC_ATTR_UNUSED)
{
	ipc_channel_t	*chp = IPCCH_LIS_EP2PTR(ephp);
	ipc_option_t	*opts;
	pfc_ipcsrv_t	*srv ;
	pfc_hostaddr_t	haddr;
	int		err, sock;

	PFC_ASSERT(chp == &ipc_channel);

	/* Accept connection request on the IPC channel. */
	sock = pfc_sock_accept(chp->ich_listener, NULL, NULL,
			       PFC_SOCK_CLOEXEC_NB);
	if (PFC_EXPECT_FALSE(sock == -1)) {
		err = errno;
		if (!PFC_IS_EWOULDBLOCK(err)) {
			IPCSRV_LOG_ERROR("accept() on IPC server failed: %s",
					 strerror(err));
		}

		return 0;
	}

	/* Create a new IPC server session. */
	opts = &chp->ich_option;
	PFC_ASSERT_INT(pfc_hostaddr_init_local(&haddr), 0);
	if (PFC_EXPECT_TRUE(ipcsrv_sess_create(chp, sock, &haddr, opts, &srv)
			    == 0)) {
		/* Create a new thread for this session. */
		err = chp->ich_ops.isvops_thread_create(ipcsrv_sess_main, srv);
		if (PFC_EXPECT_TRUE(err == 0)) {
			return 0;
		}

		IPCSRV_LOG_ERROR("Unable to create a new IPC server session "
				 "thread: %s.", strerror(err));
		ipcsrv_sess_close(srv, PFC_FALSE);
	}

	IPCSRV_LOG_ERROR("A request from IPC client was dropped: sock=%d",
			 sock);

	return 0;
}

/*
 * static int
 * ipcsrv_poll(ipc_channel_t *chp)
 *	Wait for events on file descriptors associated with the IPC channel,
 *	and invoke event handlers associated with received events.
 *
 *	- A new IPC server session is created if a read event is detected on
 *	  the listener socket, a new IPC server
 *
 *	- ECANCELED is returned if a read event is detected on the shutdown
 *	  notification file descriptor.
 *
 *	- A server session is disconnected if a read hang-up event is detected
 *	  on a server session socket.
 *
 * Calling/Exit State:
 *	ECANCELED is returned if the IPC channel was closed.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Note that this function never returns zero.
 */
static int
ipcsrv_poll(ipc_channel_t *chp)
{
	epoll_event_t	events[IPCSRV_EPOLL_MAXEVENTS];
	int		err, epfd = chp->ich_epfd;

	for (;;) {
		int	nfds;

		nfds = epoll_wait(epfd, events, PFC_ARRAY_CAPACITY(events), -1);
		if (PFC_EXPECT_FALSE(nfds < 0)) {
			err = errno;
			if (PFC_EXPECT_FALSE(err == EINTR)) {
				/* Ignore interrupt. */
				continue;
			}

			IPCSRV_LOG_ERROR("epoll_wait(2) on IPC server failed: "
					 "%s", strerror(err));
			if (PFC_EXPECT_FALSE(err != 0)) {
				err = EIO;
			}
			break;
		}

		PFC_ASSERT((size_t)nfds <= PFC_ARRAY_CAPACITY(events));

		/* Dispatch events to epoll event handlers. */
		err = pfc_epoll_dispatch(events, nfds, chp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}
	}

	PFC_ASSERT(err != 0);

	return err;
}

/*
 * static int
 * ipcsrv_sess_create(ipc_channel_t *PFC_RESTRICT chp, int sock,
 *		      const pfc_hostaddr_t *PFC_RESTRICT haddr,
 *		      ipc_coption_t *opts,
 *		      pfc_ipcsrv_t **PFC_RESTRICT srvp)
 *	Create a new IPC session instance associated with the given socket.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to IPC session instance
 *	is set to `*srvp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The socket specified by `sock' will be always closed on error.
 */
static int
ipcsrv_sess_create(ipc_channel_t *PFC_RESTRICT chp, int sock,
		   const pfc_hostaddr_t *PFC_RESTRICT haddr,
		   ipc_coption_t *opts, pfc_ipcsrv_t **PFC_RESTRICT srvp)
{
	pfc_ipcsrv_t	*srv;
	int		err;

	err = ipcsrv_sess_alloc(chp, sock, haddr, opts, &srv);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	IPCCH_LOCK(chp);

	/* Hold IPC channel. */
	err = pfc_ipcsrv_channel_hold_l(chp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* Not yet initialized, or already finalized. */
		IPCCH_UNLOCK(chp);
		ipcsrv_sess_destroy(srv);

		return err;
	}

	/* Update counters to make a new session visible. */
	chp->ich_nclients++;
	chp->ich_nsessions++;

	/* Check limitation of the number of clients and session threads. */
	if (PFC_EXPECT_FALSE(chp->ich_nclients > chp->ich_max_clients)) {
		IPCSRV_LOG_ERROR("Too many connections: %u",
				 chp->ich_nclients);
		srv->isv_magic = IPC_PROTO_MAGIC_TOOMANY;
	}
	else if (PFC_EXPECT_FALSE(chp->ich_nsessions >
				  chp->ich_max_sessions)) {
		IPCSRV_LOG_ERROR("Too many sessions: %u", chp->ich_nsessions);
		srv->isv_magic = IPC_PROTO_MAGIC_TOOMANY;
	}

	IPCSRV_ACTIVE_ADD(chp, srv);
	IPCCH_UNLOCK(chp);

	*srvp = srv;

	return 0;
}

/*
 * static int
 * ipcsrv_sess_alloc(ipc_channel_t *PFC_RESTRICT chp, int sock,
 *		     const pfc_hostaddr_t *PFC_RESTRICT haddr,
 *		     ipc_coption_t *opts, pfc_ipcsrv_t **PFC_RESTRICT srvp)
 *	Create a new IPC session instance associated with the given socket.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to IPC session instance
 *	is set to `*srvp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The socket specified by `sock' will be always closed on error.
 */
static int
ipcsrv_sess_alloc(ipc_channel_t *PFC_RESTRICT chp, int sock,
		  const pfc_hostaddr_t *PFC_RESTRICT haddr,
		  ipc_coption_t *opts, pfc_ipcsrv_t **PFC_RESTRICT srvp)
{
	pfc_ipcsrv_t	*srv;
	int		err;

	/* Allocate a new session instance. */
	srv = (pfc_ipcsrv_t *)malloc(sizeof(*srv));
	if (PFC_EXPECT_FALSE(srv == NULL)) {
		IPCSRV_LOG_ERROR("No memory for a new IPC server session.");
		PFC_IPCSRV_CLOSE(sock);
		err = ENOMEM;
		goto error;
	}

	/* Initialize the server session instance. */
	err = pfc_ipcsrv_sess_init(chp, srv, sock, haddr, opts);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Remarks: ipc_sess_t will be initialized later. */
		*srvp = srv;

		return 0;
	}

	free(srv);

error:
	/* Suppress warning blamed by old gcc. */
	*srvp = NULL;

	return err;
}

/*
 * static void
 * ipcsrv_sess_close(pfc_ipcsrv_t *srv, pfc_bool_t do_log)
 *	Close the IPC session specified by `srv'.
 *
 *	Once the server session is linked to the IPC channel, this function
 *	must be called to close the session.
 *
 *	If `do_log' is PFC_TRUE, this function records a log message that
 *	indicates an IPC session was closed.
 */
static void
ipcsrv_sess_close(pfc_ipcsrv_t *srv, pfc_bool_t do_log)
{
	ipc_channel_t	*chp = srv->isv_channel;
	uint32_t	*counter;
	int		sock;

	PFC_ASSERT(chp == &ipc_channel);

	IPCCH_LOCK(chp);
	IPCSRV_LOCK(srv);

	/* Decrement session counter. */
	PFC_ASSERT(chp->ich_nsessions > 0);
	chp->ich_nsessions--;
	counter = &chp->ich_nsessions;

	/*
	 * Decrement client connection counter if connection is still
	 * active.
	 */
	if (srv->isv_stream != NULL) {
		sock = pfc_iostream_getfd(srv->isv_stream);
		PFC_ASSERT(chp->ich_nclients > 0);
		chp->ich_nclients--;
	}
	else {
		sock = -1;
	}

	IPCSRV_UNLOCK(srv);

	/* Unlink the session from the IPC channel. */
	IPCSRV_ACTIVE_REMOVE(srv);

	/* Wake up a thread blocked in pfc_ipcsrv_fini(). */
	if ((chp->ich_flags & IPCCHF_SHUTDOWN) && *counter == 0) {
		IPCCH_SIGNAL(chp);
	}

	if (do_log) {
		IPCSRV_LOG_INFO("Session has been destroyed: srv=%p, sock=%d",
				srv, sock);
	}

	pfc_ipcsrv_channel_release_l(chp);
	ipcsrv_sess_destroy(srv);
}

/*
 * static void
 * ipcsrv_sess_destroy(pfc_ipcsrv_t *srv)
 *	Destroy the given IPC server session instance.
 */
static void
ipcsrv_sess_destroy(pfc_ipcsrv_t *srv)
{
	pfc_ipcsrv_sess_cleanup(srv);
	free(srv);
}

/*
 * static void *
 * ipcsrv_sess_main(void *arg)
 *	Start routine of IPC server session thread.
 *
 * Calling/Exit State:
 *	NULL is always returned.
 */
static void *
ipcsrv_sess_main(void *arg)
{
	pfc_ipcsrv_t	*srv = (pfc_ipcsrv_t *)arg;
	int		err;

#ifdef	PFC_VERBOSE_DEBUG
	/* The IPC channel must be held by pfc_ipcsrv_main(). */
	IPCCH_LOCK(srv->isv_channel);
	PFC_ASSERT(srv->isv_channel->ich_refcnt > 1);
	IPCCH_UNLOCK(srv->isv_channel);
#endif	/* PFC_VERBOSE_DEBUG */

	/* Establish a new IPC session. */
	err = ipcsrv_sess_start(srv);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Dispatch IPC protocol commands. */
		while (ipcsrv_sess_dispatch(srv) == 0) {}
	}

	ipcsrv_sess_close(srv, PFC_TRUE);

	return NULL;
}

/*
 * static int
 * ipcsrv_sess_start(pfc_ipcsrv_t *srv)
 *	Establish the IPC server session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_sess_start(pfc_ipcsrv_t *srv)
{
	ipc_sess_t	*sess = &srv->isv_session;
	ipc_hshake_t	hshake;
	pfc_iostream_t	stream = sess->iss_stream;
	pfc_timespec_t	abstime;
	pfc_ucred_t	*credp;
	uint8_t		magic;
	size_t		sz;
	int		err, failed, sock;

	/* Read handshake message. */
	err = pfc_ipcsrv_gettime(&abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	abstime.tv_sec += IPCSRV_HSHAKE_TIMEOUT;
	sz = sizeof(hshake);

	/* Read handshake message and client credentials at the same time. */
	PFC_ASSERT(!IPC_NEED_BSWAP(sess->iss_flags));
	credp = &srv->isv_cred;
	err = __pfc_iostream_recvcred_abs(stream, &hshake, &sz, credp,
					  &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_hshake;
	}

	if (PFC_EXPECT_FALSE(sz != sizeof(hshake))) {
		return ipcsrv_log_econnreset(srv, PFC_FALSE, -1);
	}

	if (PFC_EXPECT_FALSE(credp->uid == (uid_t)-1)) {
		IPCSRV_LOG_ERROR("Client did not send credentials.");

		return EPERM;
	}
		
	if (PFC_EXPECT_FALSE(hshake.ih_magic != IPC_PROTO_MAGIC)) {
		IPCSRV_LOG_ERROR("Bad IPC magic: 0x%02x", hshake.ih_magic);

		return EPROTO;
	}

	if (PFC_EXPECT_FALSE(!IPC_ORDER_IS_VALID(hshake.ih_order) ||
			     !IPC_ORDER_IS_VALID(hshake.ih_forder))) {
		IPCSRV_LOG_ERROR("Unexpected IPC byte order: %u,%u",
				 hshake.ih_order, hshake.ih_forder);

		return EPROTO;
	}

	IPC_SESS_PROTO_INIT(sess, &hshake);

	magic = srv->isv_magic;
	if (PFC_EXPECT_FALSE(magic != IPC_PROTO_MAGIC)) {
		/* Too many clients or session threads. */
		PFC_ASSERT(magic == IPC_PROTO_MAGIC_TOOMANY);
		failed = ENOSPC;
	}
	else {
		failed = 0;
	}

	IPC_HSHAKE_MAGIC_INIT(&hshake, magic);

	/* Send response of handshake. */
	err = pfc_ipc_write(stream, &hshake, sizeof(hshake), PFC_TRUE,
			    &abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("Failed to send IPC handshake message: %s",
				 strerror(err));

		return err;
	}

	if (PFC_EXPECT_FALSE(failed != 0)) {
		return failed;
	}

	/*
	 * Set thread ID after reading credentials.
	 */
	IPCSRV_LOCK(srv);
	srv->isv_tid = pfc_gettid();
	sock = pfc_iostream_getfd(stream);
	IPCSRV_UNLOCK(srv);

	IPCSRV_LOG_INFO("New connection: srv=%p, sock=%d, client=%u, "
			"uid=%d, gid=%d ver=%u, bflags=0x%x",
			srv, sock, credp->pid, credp->uid, credp->gid,
			sess->iss_version, sess->iss_flags);

	return 0;

error_hshake:
	IPCSRV_LOG_ERROR("Failed to read IPC handshake: %s", strerror(err));

	return err;
}

/*
 * static int
 * ipcsrv_sess_dispatch(pfc_ipcsrv_t *srv)
 *	Receive a command on the IPC service session, and dispatch the command.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	If this function returns non-zero value, the IPC session will be
 *	closed immediately.
 */
static int
ipcsrv_sess_dispatch(pfc_ipcsrv_t *srv)
{
	ipc_channel_t	*chp = srv->isv_channel;
	ipc_sess_t	*sess = &srv->isv_session;
	ipc_ccmdops_t	*ops;
	pfc_iostream_t	stream = sess->iss_stream;
	pfc_timespec_t	*abstime, tsbuf;
	uint8_t		cmd;
	size_t		sz;
	int		err;

	/* Read an IPC service protocol command without timeout. */
	sz = sizeof(cmd);
	err = pfc_iostream_read_abs(stream, &cmd, &sz, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err != ECANCELED) {
			IPCSRV_LOG_ERROR("Failed to read an IPC protocol "
					 "command: %s", strerror(err));
		}

		return err;
	}

	if (PFC_EXPECT_FALSE(sz != sizeof(cmd))) {
		if (PFC_EXPECT_TRUE(sz == 0)) {
			return ECONNRESET;
		}

		return ipcsrv_log_econnreset(srv, PFC_FALSE, -1);
	}

	ops = &ipc_proto_commands[cmd];
	if (PFC_EXPECT_FALSE(ops >= PFC_ARRAY_LIMIT(ipc_proto_commands))) {
		IPCSRV_LOG_ERROR("Unknown IPC command: 0x%x", cmd);

		return EPROTO;
	}

	/* Initialize session timeout. */
	err = pfc_ipcsrv_timeout_init(chp, &abstime, &tsbuf);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Execute IPC protocol command. */
	return ops->icops_exec(srv, abstime);
}

/*
 * static int
 * ipcsrv_sess_ping(pfc_ipcsrv_t *PFC_RESTRICT srv, pfc_timespec_t *abstime)
 *	Process a ping request on the given IPC server session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	If this function returns non-zero value, the IPC session will be
 *	closed immediately.
 */
static int
ipcsrv_sess_ping(pfc_ipcsrv_t *PFC_RESTRICT srv, pfc_timespec_t *abstime)
{
	pfc_iostream_t	stream = srv->isv_stream;
	uint32_t	value;
	int		err;

	/* Read an unsigned 32-bit value. */
	err = pfc_ipc_read(stream, &value, sizeof(value), abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("Failed to read PING argument: %s",
				 strerror(err));

		return err;
	}

	/* Echo back argument. */
	err = pfc_ipc_write(stream, &value, sizeof(value), PFC_TRUE, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("Failed to send PING response: %s",
				 strerror(err));
	}
	else {
		IPCSRV_LOG_VERBOSE("PING succeeded: value=0x%08x", value);
	}

	return err;
}

/*
 * static int
 * ipcsrv_sess_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv, pfc_timespec_t *abstime)
 *	Read an IPC service request from the given session, and invoke it.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	If this function returns non-zero value, the IPC session will be
 *	closed immediately.
 */
static int
ipcsrv_sess_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv, pfc_timespec_t *abstime)
{
	ipc_sess_t	*sess = &srv->isv_session;
	ipc_stream_t	*stp = &srv->isv_output;
	ipc_msg_t	*msg = &srv->isv_args;
	pfc_iostream_t	stream = sess->iss_stream;
	pfc_timespec_t	called, tsbuf;
	uint32_t	namelen, flags = 0;
	ipc_svreq_t	request;
	char		*svname;
	int		err;
	ipc_svresp_t	resp;
	pfc_bool_t	resperr = PFC_FALSE;

#ifdef	PFC_VERBOSE_DEBUG
	IPCSRV_LOCK(srv);
	PFC_ASSERT(srv->isv_flags == 0);
	IPCSRV_UNLOCK(srv);
#endif	/* PFC_VERBOSE_DEBUG */

	/* Read service request header. */
	err = pfc_ipc_read(stream, &request, sizeof(request), abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("Failed to receive request header: %s",
				 strerror(err));

		return err;
	}

	namelen = request.isrq_namelen;
	srv->isv_service = request.isrq_service;
	IPC_SESS_BSWAP_INT(sess, namelen);
	IPC_SESS_BSWAP_INT(sess, srv->isv_service);

	if (PFC_EXPECT_FALSE(!IPC_SERVICE_NAMELEN_IS_VALID(namelen))) {
		IPCSRV_LOG_ERROR("Invalid service name length: %u", namelen);

		return EINVAL;
	}

	/* Read service name. */
	svname = srv->isv_name;
	err = pfc_ipc_read(stream, svname, namelen, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("Failed to read service name: %s",
				 strerror(err));

		return err;
	}

	*(svname + namelen) = '\0';

	IPCSRV_LOCK(srv);
	srv->isv_flags |= IPCSRVF_ACTIVE;
	IPCSRV_UNLOCK(srv);

	/* Set byte swap flags to the IPC message instance. */
	pfc_ipcmsg_setbflags(msg, sess->iss_flags);

	/* Verify the state of the IPC message and the IPC stream. */
	ipcsrv_ipcmsg_assert(msg, sess->iss_flags);
	ipcsrv_ipcstream_assert(stp);

	/*
	 * Receive an IPC message which represents arguments for IPC
	 * service.
	 */
	err = pfc_ipcmsg_recv(sess, msg, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("Failed to receive arguments: %s",
				 strerror(err));
		goto out;
	}

	/* Record the time when the IPC service handler is called. */
	err = pfc_ipcsrv_gettime(&called);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Call IPC service handler. */
	err = ipcsrv_invoke(srv, &resp, &flags);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	if (PFC_EXPECT_FALSE(resp.isrs_response != 0)) {
		PFC_ASSERT(resp.isrs_response == ENOSYS);
		IPCSRV_LOG_ERROR("%s/%u: Unknown IPC service.",
				 svname, srv->isv_service);

		/* Send error response. */
		err = ipcsrv_send_error(srv, &resp, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			resperr = PFC_TRUE;
		}
		goto out;
	}

	if (PFC_EXPECT_FALSE(pfc_ipcstream_is_broken(stp))) {
		IPCSRV_LOG_ERROR("%s/%u: Output stream is broken.",
				 svname, srv->isv_service);
		err = EIO;
		resperr = PFC_TRUE;
		goto out;
	}

	IPCSRV_LOG_DEBUG("service=%s/%u: result=%d", svname, srv->isv_service,
			 resp.isrs_result);

	/* Update session timeout if the IPC service handler set timeout. */
	err = ipcsrv_settimeout(srv, flags, &abstime, &tsbuf, &called);
	if (PFC_EXPECT_FALSE(err != 0)) {
		resperr = PFC_TRUE;
		goto out;
	}

	/* Check to see whether the client connection is reset. */
	stream = ipcsrv_sync_listener(srv);
	if (PFC_EXPECT_FALSE(stream == NULL)) {
		err = ECONNRESET;
		resperr = PFC_TRUE;
		goto out;
	}

	/* Send response code and results of the IPC service. */
	err = pfc_ipc_write(stream, &resp, sizeof(resp), PFC_FALSE, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("%s/%u: Failed to send response: %s",
				 svname, srv->isv_service, strerror(err));
		resperr = PFC_TRUE;
		goto out;
	}
	if (PFC_EXPECT_FALSE(resp.isrs_result == PFC_IPCRESP_FATAL)) {
		IPCSRV_LOG_WARN("%s/%u: Service handler returned fatal error "
				"code.", svname, srv->isv_service);

		/* Flush output stream. */
		err = pfc_iostream_flush_abs(stream, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPCSRV_LOG_ERROR("%s/%u: Failed to flush stream: %s",
					 svname, srv->isv_service,
					 strerror(err));
			resperr = PFC_TRUE;
		}
	}
	else {
		/* Send IPC message constructed by the IPC service handler. */
		err = pfc_ipcstream_send(sess, stp, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPCSRV_LOG_ERROR("%s/%u: "
					 "Failed to send IPC message: %s",
					 svname, srv->isv_service,
					 strerror(err));
			resperr = PFC_TRUE;
		}
	}

out:
	if (PFC_EXPECT_FALSE(resperr)) {
		/* Increment response error counter. */
		pfc_ipcsrv_inc_resperror(srv);
	}

	/* Ensure that the listener thread no longer watches this session. */
	IPCSRV_LOCK(srv);
	ipcsrv_sync_listener_l(srv);
	srv->isv_flags = 0;
	IPCSRV_UNLOCK(srv);

	/* Clean up session. */
	pfc_ipcmsg_reset(msg);
	pfc_ipcstream_reset(stp, 0);
	srv->isv_name[0] = '\0';
	srv->isv_service = 0;

	return err;
}

/*
 * static int
 * ipcsrv_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *		 ipc_svresp_t *PFC_RESTRICT resp,
 *		 uint32_t *PFC_RESTRICT flags)
 *	Call IPC service handler on the given server session.
 *	This is an internal function of ipc_sess_invoke().
 *
 * Calling/Exit State:
 *	Upon successful completion, response data to be sent to the client
 *	is set to the buffer pointed by `resp', and the current server session
 *	flag value is set to the buffer pointed by `flagsp', and then
 *	zero is returned.
 *
 *	ECONNRESET is returned if the connection is reset by IPC client.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv, ipc_svresp_t *PFC_RESTRICT resp,
	      uint32_t *PFC_RESTRICT flagsp)
{
	ipc_srvcbdtorset_t	dset;
	ipc_channel_t	*chp = srv->isv_channel;
	int		fd, err, epfd = chp->ich_epfd;

	/*
	 * Set connection watching flag.
	 * This flag indicates the listener thread is watching this session.
	 */
	IPCSRV_LOCK(srv);
	srv->isv_flags |= IPCSRVF_WATCH;
	IPCSRV_UNLOCK(srv);

	/*
	 * Register the connection socket to the epoll instance in order to
	 * detect connection reset during IPC service handler call.
	 */
	fd = pfc_iostream_getfd(srv->isv_stream);
	err = pfc_epoll_ctl(epfd, EPOLL_CTL_ADD, fd, IPC_EPOLLEV_RESET,
			    &srv->isv_epoll);
	if (PFC_EXPECT_FALSE(err != 0)) {
		err = errno;
		IPCSRV_LOG_ERROR("%s/%u: Failed to register client connection "
				 "to the epoll instance: %s", srv->isv_name,
				 srv->isv_service, strerror(err));
		if (PFC_EXPECT_FALSE(err == 0)) {
			err = EIO;
		}

		IPCSRV_LOCK(srv);
		srv->isv_flags &= ~IPCSRVF_WATCH;
		IPCSRV_UNLOCK(srv);

		return err;
	}

	/* Call IPC service handler. */
	resp->isrs_response = pfc_ipcsrv_invoke(srv, &resp->isrs_result);

	/*
	 * The listener thread may be going to close the connection of
	 * this server session. We must ensure that the listener thread
	 * no longer watches this session before the end of the session,
	 * or the listener thread may call connection reset event handler
	 * with specifying obsolete server session instance.
	 *
	 * So we append EPOLLOUT to the client socket events to watch.
	 * It should cause an EPOLLOUT event immediately, and then the
	 * connection reset handler removes the socket from the epoll instance.
	 */
	IPCSRV_LOCK(srv);
	if (!IPCSRV_IS_RESET(srv)) {
		err = pfc_epoll_ctl(epfd, EPOLL_CTL_MOD, fd,
				    IPC_EPOLLEV_RESET | EPOLLOUT,
				    &srv->isv_epoll);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* This should never happen. */
			err = errno;
			IPCSRV_LOG_ERROR("%s/%u: Failed to change client "
					 "connection event: %s", srv->isv_name,
					 srv->isv_service, strerror(err));

			/* Ignore this error. */
			srv->isv_flags &= ~IPCSRVF_WATCH;
		}

		*flagsp = srv->isv_flags;
	}

	/* Unregister all callbacks. */
	ipcsrv_callback_clear(srv, &dset);
	IPCSRV_UNLOCK(srv);

	/* Call destructor of callbacks. */
	ipcsrv_cbdtorset_invoke(&dset);

	return 0;
}

/*
 * static int
 * ipcsrv_send_error(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *		     ipc_svresp_t *PFC_RESTRICT resp,
 *		     const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Send error response to the client.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_send_error(pfc_ipcsrv_t *PFC_RESTRICT srv,
		  ipc_svresp_t *PFC_RESTRICT resp,
		  const pfc_timespec_t *PFC_RESTRICT abstime)
{
	pfc_iostream_t	stream;
	int		err;

	stream = ipcsrv_sync_listener(srv);
	if (PFC_EXPECT_FALSE(stream == NULL)) {
		return ECONNRESET;
	}

	resp->isrs_result = PFC_IPCRESP_FATAL;
	err = pfc_ipc_write(stream, resp, sizeof(*resp), PFC_TRUE, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("%s/%u: Failed to send error response: %s",
				 srv->isv_name, srv->isv_service,
				 strerror(err));
	}

	return err;
}

/*
 * static int
 * ipcsrv_settimeout(pfc_ipcsrv_t *PFC_RESTRICT srv, uint32_t flags,
 *		     pfc_timespec_t **PFC_RESTRICT abstimep,
 *		     pfc_timespec_t *PFC_RESTRICT tsp,
 *		     const pfc_timespec_t *PFC_RESTRICT called)
 *	Set server session timeout specified by the IPC service handler.
 *
 *	`flags' must be the server session flags set by the IPC service
 *	handler. Note that this function never uses srv->isv_flags.
 *
 *	`abstimep' must point a pointer to a pointer which points
 *	current absolute timeout value. Note that a pointer pointed by
 *	`abstimep' or pfc_timespec_t value pointed by a pointer in `*abstimep'
 *	will be updated if the IPC service handler changes the timeout.
 *
 *	`tsp' is must be a pointer to pfc_timespec_t to store new timeout.
 *	It will be used only if `abstimep' points NULL.
 *
 *	`called' must point the time when the IPC service handler is called.
 *
 * Calling/Exit State:
 *	Upon successful completion, an absolute system time which represents
 *	the server session timeout is set to `*abstime', and zero is returned.
 *
 *	ETIMEDOUT is returned if the server session is already timed out.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_settimeout(pfc_ipcsrv_t *PFC_RESTRICT srv, uint32_t flags,
		  pfc_timespec_t **PFC_RESTRICT abstimep,
		  pfc_timespec_t *PFC_RESTRICT tsp,
		  const pfc_timespec_t *PFC_RESTRICT called)
{
	pfc_timespec_t	*abstime, cur;
	int		err, ret;

	PFC_ASSERT(abstimep != NULL);
	PFC_ASSERT(tsp != NULL);
	PFC_ASSERT(called != NULL);

	if ((flags & IPCSRVF_HASTIMEOUT) == 0) {
		/* The IPC service handler did not set the timeout value. */
		abstime = *abstimep;
		if (abstime == NULL) {
			/* Infinite timeout. */
			return 0;
		}
	}
	else if (flags & IPCSRVF_NOTIMEOUT) {
		/* Disable session timeout. */
		*abstimep = NULL;

		return 0;
	}
	else {
		/* Calculate new timeout. */
		abstime = *abstimep;
		if (abstime == NULL) {
			*abstimep = tsp;
			abstime = tsp;
		}
		*abstime = *called;
		pfc_timespec_add(abstime, &srv->isv_timeout);
		IPCSRV_LOG_VERBOSE("%s/%u: New session timeout: %lu.%lu",
				   srv->isv_name, srv->isv_service,
				   srv->isv_timeout.tv_sec,
				   srv->isv_timeout.tv_nsec);
	}

	err = pfc_ipcsrv_gettime(&cur);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	PFC_ASSERT(abstime != NULL);
	ret = pfc_clock_compare(&cur, abstime);
	if (PFC_EXPECT_FALSE(ret >= 0)) {
		/* The session is already timed out. */
		IPCSRV_LOG_ERROR("%s/%u: Server session timed out.",
				 srv->isv_name, srv->isv_service);

		return ETIMEDOUT;
	}

	return 0;
}

/*
 * static int
 * ipcsrv_epoll_client(pfc_ephdlr_t *ephp, uint32_t events,
 *		       void *arg PFC_ATTR_UNUSED)
 *	Handle an event on the client connection socket for the IPC server
 *	session.
 *
 *	This function is used to detect connection reset during IPC service
 *	handler call.
 *
 * Calling/Exit State:
 *	Zero is always returned.
 */
static int
ipcsrv_epoll_client(pfc_ephdlr_t *ephp, uint32_t events,
		    void *arg PFC_ATTR_UNUSED)
{
	pfc_ipcsrv_t	*srv = IPCSRV_EP2PTR(ephp);
	ipc_channel_t	*chp = srv->isv_channel;
	ipc_srvcb_t	*cbp = NULL;
	pfc_iostream_t	stream;
	pfc_bool_t	do_close;
	int		fd, err, cberr = 0;

	PFC_ASSERT(chp == &ipc_channel);

	if (PFC_EXPECT_FALSE(events & IPC_EPOLLEV_TEST_RESET)) {
		/* The connection was reset by IPC client. */
		do_close = PFC_TRUE;
	}
	else if (events & EPOLLOUT) {
		/*
		 * IPC service handler returned.
		 * Only thing to do here is to remove the connection socket
		 * from the epoll instance.
		 */
		do_close = PFC_FALSE;
	}
	else {
		return 0;
	}

	if (PFC_EXPECT_FALSE(do_close)) {
		/*
		 * The IPC channel lock is required to adjust client
		 * connection counter.
		 */
		IPCCH_LOCK(chp);
	}
	IPCSRV_LOCK(srv);

	stream = srv->isv_stream;
	PFC_ASSERT(stream != NULL);
	PFC_ASSERT(srv->isv_flags & IPCSRVF_ACTIVE);

	/*
	 * Unregister the connection socket from the epoll instance.
	 *
	 * Although closing the connection socket implies this, we should
	 * unregister explicitly because unregistration from the epoll
	 * instance is performed when the last reference to the file is closed.
	 * The socket file descriptor may be duplicated in a short period
	 * by fork(2).
	 */
	fd = pfc_iostream_getfd(stream);
	err = pfc_epoll_ctl(chp->ich_epfd, EPOLL_CTL_DEL, fd, 0, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		err = errno;
		IPCSRV_LOG_ERROR("%s/%u: Failed to remove client connection "
				 "from the epoll instance: %s", srv->isv_name,
				 srv->isv_service, strerror(err));
		/* FALLTROUGH */
	}

	if (PFC_EXPECT_FALSE(do_close)) {
		int	sock = pfc_iostream_getfd(stream);

		if (PFC_EXPECT_FALSE(events & EPOLLERR)) {
			IPCSRV_LOG_ERROR("%s/%u: "
					 "Reset connection due to error: "
					 "srv=%p, sock=%d",
					 srv->isv_name, srv->isv_service,
					 srv, sock);
		}
		else {
			(void)ipcsrv_log_econnreset(srv, PFC_TRUE, sock);
		}

		/* Destroy server-side connection stream. */
		PFC_ASSERT_INT(pfc_iostream_destroy(stream), 0);
		srv->isv_stream = NULL;

		/* Adjust client connection counter. */
		PFC_ASSERT(chp->ich_nclients > 0);
		chp->ich_nclients--;

		cbp = srv->isv_callback[PFC_IPCSRVCB_CONNRESET];
		if (cbp != NULL) {
			/* Invoke reset callback. */
			PFC_ASSERT(cbp->iscb_sess == srv);
			cberr = ipcsrv_callback_start(cbp);
		}
	}

	/* Wake up the server session thread. */
	srv->isv_flags &= ~IPCSRVF_WATCH;
	IPCSRV_SIGNAL(srv);

	IPCSRV_UNLOCK(srv);
	if (PFC_EXPECT_FALSE(do_close)) {
		IPCCH_UNLOCK(chp);
	}

	if (PFC_EXPECT_FALSE(cberr != 0)) {
		/* Invoke reset callback on the listener thread. */
		IPCSRV_LOG_WARN("Failed to create thread for reset callback: "
				"%s", strerror(cberr));
		PFC_ASSERT(cbp != NULL);
		ipcsrv_callback_invoke(srv, cbp);
	}

	return 0;
}

/*
 * static int
 * ipcsrv_log_econnreset(pfc_ipcsrv_t *srv, pfc_bool_t active, int sock)
 *	Record error log which indicates ECONNRESET error.
 *
 *	If PFC_TRUE is specified to `active', this function logs IPC service
 *	name and ID, and socket file descriptor specified by `sock'.
 *
 *	`sock' is simply ignored if `active' is PFC_FALSE.
 *
 * Calling/Exit State:
 *	This function always returns ECONNRESET.
 *
 * Remarks:
 *	The caller must call this function with holding the server session
 *	lock if it passes PFC_TRUE to `active'.
 */
static int
ipcsrv_log_econnreset(pfc_ipcsrv_t *srv, pfc_bool_t active, int sock)
{
	const char	*msg = "Connection reset by peer";

	if (active) {
		IPCSRV_LOG_ERROR("%s/%u: %s: srv=%p, sock=%d", srv->isv_name,
				 srv->isv_service, msg, srv, sock);
	}
	else {
		IPCSRV_LOG_ERROR("%s.", msg);
	}

	return ECONNRESET;
}

/*
 * static pfc_iostream_t
 * ipcsrv_sync_listener(pfc_ipcsrv_t *srv)
 *	Wait for the listener thread to terminate client connection watching.
 *
 * Calling/Exit State:
 *	A non-NULL pointer to the connection stream is returned if the client
 *	connection is still active.
 *	NULL is returned if the connection is reset by IPC client.
 */
static pfc_iostream_t
ipcsrv_sync_listener(pfc_ipcsrv_t *srv)
{
	pfc_iostream_t	stream;

	IPCSRV_LOCK(srv);
	ipcsrv_sync_listener_l(srv);
	stream = srv->isv_stream;
	IPCSRV_UNLOCK(srv);

	return stream;
}

/*
 * static void
 * ipcsrv_sync_listener_l(pfc_ipcsrv_t *srv)
 *	Wait for the listener thread to terminate client connection watching.
 *
 * Remarks:
 *	This function must be called with holding the server session lock.
 */
static void
ipcsrv_sync_listener_l(pfc_ipcsrv_t *srv)
{
	PFC_ASSERT(srv->isv_flags & IPCSRVF_ACTIVE);
	while (srv->isv_flags & IPCSRVF_WATCH) {
		IPCSRV_WAIT(srv);
	}
}

/*
 * static int
 * ipcsrv_callback_start(ipc_srvcb_t *cbp)
 *	Create a temporary thread to invoke the callback function specified
 *	by `cbp'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the server session lock.
 *
 *	- This function always set PFC_TRUE to chp->iscb_active even on error.
 */
static int
ipcsrv_callback_start(ipc_srvcb_t *cbp)
{
	pfc_ipcsrv_t	*srv = cbp->iscb_sess;
	ipc_channel_t	*chp = srv->isv_channel;
	int		err;

	PFC_ASSERT(chp == &ipc_channel);

	/* Turn the active flag on. */
	PFC_ASSERT(!cbp->iscb_active);
	cbp->iscb_active = PFC_TRUE;

	/* Create a temporary thread to invoke callback. */
	err = chp->ich_ops.isvops_thread_create(ipcsrv_callback_thread, cbp);

	return err;
}

/*
 * static void *
 * ipcsrv_callback_thread(void *arg)
 *	Start routine of a temporary thread which invokes the IPC server
 *	callback.
 */
static void *
ipcsrv_callback_thread(void *arg)
{
	ipc_srvcb_t	*cbp = (ipc_srvcb_t *)arg;
	pfc_ipcsrv_t	*srv = cbp->iscb_sess;

	ipcsrv_callback_invoke(srv, cbp);

	return NULL;
}

/*
 * static void
 * ipcsrv_callback_clear(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			 ipc_srvcbdtorset_t *PFC_RESTRICT dset)
 *	Unregister all IPC server session callbacks.
 *
 *	All arguments and destructors are copied into `dset'.
 *
 * Remarks:
 *	- This function must be called with holding the IPC server session
 *	  lock. Note that it may be released for a while.
 *
 *	- The calling thread is blocked until all callbacks being called are
 *	  returned.
 */
static void
ipcsrv_callback_clear(pfc_ipcsrv_t *PFC_RESTRICT srv,
		      ipc_srvcbdtorset_t *PFC_RESTRICT dset)
{
	ipc_srvcb_t	**cbpp;
	ipc_srvcbdtor_t	*dtorp;

	for (cbpp = srv->isv_callback, dtorp = dset->iscbds_dtor;
	     cbpp < PFC_ARRAY_LIMIT(srv->isv_callback); cbpp++, dtorp++) {
		ipc_srvcb_t	*cbp = *cbpp;

		PFC_ASSERT(cbp == NULL || cbp->iscb_sess == srv);
		*cbpp = NULL;
		ipcsrv_callback_destroy(cbp, dtorp);
	}
}

/*
 * static void
 * ipcsrv_callback_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv,
 *			  ipc_srvcb_t *PFC_RESTRICT cbp)
 *	Invoke the callback function specified by `cbp'.
 */
static void
ipcsrv_callback_invoke(pfc_ipcsrv_t *PFC_RESTRICT srv,
		       ipc_srvcb_t *PFC_RESTRICT cbp)
{
	pfc_ipcsrvcb_t	*callback = &cbp->iscb_callback;

	PFC_ASSERT(cbp->iscb_sess == srv);
	PFC_ASSERT(callback->isc_callback != NULL);
	PFC_ASSERT(IPCSRV_CBTYPE_IS_VALID(cbp->iscb_type));

	/* Invoke callback function. */
	callback->isc_callback(srv, cbp->iscb_type, cbp->iscb_arg);

	IPCSRV_LOCK(srv);

	/* Notify the completion of the callback. */
	PFC_ASSERT(cbp->iscb_active);
	cbp->iscb_active = PFC_FALSE;
	IPCSRV_CB_BROADCAST(cbp);

	IPCSRV_UNLOCK(srv);
}

/*
 * static void
 * ipcsrv_callback_destroy(ipc_srvcb_t *PFC_RESTRICT cbp,
 *			   ipc_srvcbdtor_t *PFC_RESTRICT dtorp)
 *	Destroy the IPC server callback specified by `cbp'.
 *
 *	If the callback is being called, the calling thread is blocked until
 *	it returns.
 *
 *	If the callback has argument destructor, destructor and callback
 *	argument are copied to the buffer pointed by `dtorp'.
 *
 *	If `cbp' is NULL, this function only zeroes out the buffer pointed
 *	by `dtorp'.
 *
 * Remarks:
 *	This function must be called with holding the server session lock.
 */
static void
ipcsrv_callback_destroy(ipc_srvcb_t *PFC_RESTRICT cbp,
			ipc_srvcbdtor_t *PFC_RESTRICT dtorp)
{
	if (cbp == NULL) {
		ipcsrv_cbdtor_init(dtorp);
	}
	else {
		/* Ensure that the callback is not being called. */
		ipcsrv_callback_wait(cbp->iscb_sess, cbp);

		/* Copy argument and destructor. */
		ipcsrv_cbdtor_copy(dtorp, cbp);

		free(cbp);
	}
}
