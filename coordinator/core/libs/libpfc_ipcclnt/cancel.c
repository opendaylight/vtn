/*
 * Copyright (c) 2011-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cancel.c - Session canceller.
 */

#include <pfc/util.h>
#include "ipcclnt_impl.h"
#include "ipcclnt_event.h"

/*
 * If this flag is turned on, the IPC client library is disabled.
 */
pfc_bool_t	ipc_disabled PFC_ATTR_HIDDEN = PFC_FALSE;

/*
 * Current canceller.
 */
static ipc_canceller_t	*ipc_current_canceller;

/*
 * List of active cancellers.
 */
static pfc_list_t	ipc_cancellers = PFC_LIST_INITIALIZER(ipc_cancellers);

/*
 * Mutex for ipc_cancellers.
 */
static pfc_mutex_t	ipc_canceller_mutex = PFC_MUTEX_INITIALIZER;

#define	IPC_CANCELLER_LOCK()	pfc_mutex_lock(&ipc_canceller_mutex)
#define	IPC_CANCELLER_TRYLOCK()	pfc_mutex_trylock(&ipc_canceller_mutex)
#define	IPC_CANCELLER_UNLOCK()	pfc_mutex_unlock(&ipc_canceller_mutex)

/*
 * Internal prototypes.
 */
static int	canceller_init(ipc_canceller_t *clp);
static void	canceller_destroy(ipc_canceller_t *clp);
static void	canceller_cleanup(void);
static int	canceller_getforsess(ipc_canceller_t **PFC_RESTRICT clpp,
				     pfc_ipcsess_t *PFC_RESTRICT sess);
static void	canceller_sess_notify(pfc_ipcsess_t *sess, pfc_bool_t discard,
				      pfc_bool_t do_log);

static int		canceller_testcancel(ipc_canceller_t *clp);
static int		canceller_sess_testcancel(ipc_canceller_t *clp);
static pfc_ipcsess_t	*canceller_sess_getsess(ipc_canceller_t *clp);

/*
 * Operations for global canceller.
 */
static ipc_clops_t	canceller_global_ops = {
	.clops_testcancel	= canceller_testcancel,
};

/*
 * Operations for session-specific canceller.
 */
static ipc_clops_t	canceller_sess_ops = {
	.clops_testcancel	= canceller_sess_testcancel,
	.clops_getsess		= canceller_sess_getsess,
};

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * canceller_close(int fd)
 *	Close the given file descriptor.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
canceller_close(int fd)
{
	if (fd != -1) {
		(void)close(fd);
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * canceller_destroy_l(ipc_canceller_t *clp)
 *	Unlink the given canceller instance from the global list,
 *	and destroy it.
 *
 * Remarks:
 *	This function must be called with holding the canceller lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
canceller_destroy_l(ipc_canceller_t *clp)
{
	pfc_list_remove(&clp->icl_list);
	canceller_destroy(clp);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * canceller_conn_notify_l(ipc_conn_t *PFC_RESTRICT cnp,
 *			   const ipc_clnotify_t *PFC_RESTRICT clnp)
 *	Send global cancellation notification to all sessions associated with
 *	the given connection.
 *
 *	See comments on pfc_ipcclnt_canceller_conn_notify() and ipc_clnotify_t.
 *
 * Remarks:
 *	This function must be called with holding the connection lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
canceller_conn_notify_l(ipc_conn_t *PFC_RESTRICT cnp,
			const ipc_clnotify_t *PFC_RESTRICT clnp)
{
	pfc_list_t	*elem;
	pfc_bool_t	force;
	pfc_bool_t	discard = clnp->icln_discard;
	ipc_elsess_t	*elsp = clnp->icln_elsess;

	/*
	 * All sessions must be canceled if the IPC client library is already
	 * disabled.
	 */
	force = ipc_disabled;

	if (elsp == IPC_CLNOTIFY_FORCE) {
		/* Cancel all sessions by force. */
		force = PFC_TRUE;
	}
	else if (elsp != NULL) {
		/*
		 * If an event listener session is specified, cancel sessions
		 * associated with connections which connect to the same IPC
		 * server as the specified event listener session.
		 */
		if (ipc_elsess_conn_equals(elsp, cnp)) {
			force = PFC_TRUE;
		}
		else {
			return;
		}
	}

	/*
	 * Send cancellation notification to all sessions which accept
	 * global cancellation.
	 */
	PFC_LIST_FOREACH(&cnp->icn_sessions, elem) {
		pfc_ipcsess_t	*sess = IPC_CLSESS_LIST2PTR(elem);

		/*
		 * Check to see if this session should be canceled.
		 * Note that we can check session creation flag without
		 * holding the session lock because it is immutable.
		 */
		if (force || IPC_CLSESS_ACCEPT_GLOBCANCEL(sess)) {
			canceller_sess_notify(sess, discard, force);
		}
	}

	/* Cancel busy wait on this connection. */
	IPC_CONN_BROADCAST(cnp);
}

/*
 * void
 * pfc_ipcclnt_cancel(pfc_bool_t permanent)
 *	Cancel all ongoing IPC client sessions.
 *
 *	After the call of this function, all ongoing call of
 *	pfc_ipcclnt_sess_invoke() will get ECANCELED error.
 *
 *	If PFC_TRUE is passed to `permanent', the IPC client library is
 *	permanently disabled. If PFC_FALSE is specified, only ongoing sessions
 *	will be canceled.
 */
void
pfc_ipcclnt_cancel(pfc_bool_t permanent)
{
	ipc_canceller_t	*clp;

	IPC_CLIENT_WRLOCK();

	if (permanent) {
		/* Disable the IPC client library. */
		ipc_disabled = PFC_TRUE;
	}

	/* Reset the current canceller. */
	clp = ipc_current_canceller;
	ipc_current_canceller = NULL;

	if (clp != NULL) {
		/* Notify cancellation to all IPC sessions. */
		pfc_ipcclnt_canceller_notify(clp);
	}

	/*
	 * Send notification to all session-specific cancellers which accept
	 * global cancellation, and cancel busy wait on all active connections.
	 */
	pfc_ipcclnt_conn_iterate(pfc_ipcclnt_canceller_conn_notify, NULL);

	IPC_CLIENT_UNLOCK();
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_canceller_get(ipc_canceller_t **PFC_RESTRICT clpp,
 *			     pfc_ipcsess_t *PFC_RESTRICT sess)
 *	Get IPC session canceller for the given IPC session.
 *
 *	If the IPC session specified by `sess' is allowed to session-specific
 *	cancellation, this function obtains the session canceller instance
 *	specific to the session. Otherwise this function obtains the global
 *	session canceller instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to canceller instance
 *	is set to `*clpp', and zero is returned.
 *
 *	ECONNABORTED is returned if the IPC client library is disabled.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function holds the canceller on successful return.
 *	The caller must release it by IPC_CANCEL_RELEASE().
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_canceller_get(ipc_canceller_t **PFC_RESTRICT clpp,
			  pfc_ipcsess_t *PFC_RESTRICT sess)
{
	ipc_canceller_t	*clp;
	int		err = 0;

	IPC_CLIENT_RDLOCK();

	if (PFC_EXPECT_FALSE(ipc_disabled)) {
		err = ECONNABORTED;
		goto out;
	}

	if (IPC_CLSESS_NEED_SPECCANCEL(sess)) {
		/* Get session-specific canceller instance. */
		return canceller_getforsess(clpp, sess);
	}

	/* Check whether we can share existing canceller. */
	clp = ipc_current_canceller;
	if (PFC_EXPECT_TRUE(clp != NULL)) {
		IPC_CANCELLER_HOLD(clp);
		*clpp = clp;
		goto out;
	}

	/* We need to create a new canceller. */
	IPC_CLIENT_UNLOCK();
	IPC_CLIENT_WRLOCK();
	IPC_CANCELLER_LOCK();

	/* We need to check state again. */
	PFC_MEMORY_RELOAD();
	if (PFC_EXPECT_FALSE(ipc_disabled)) {
		err = ECONNABORTED;
		goto out_lock;
	}

	clp = ipc_current_canceller;
	if (clp != NULL) {
		IPC_CANCELLER_HOLD(clp);
		*clpp = clp;
		goto out_lock;
	}

	/* Create a new canceller instance. */
	clp = (ipc_canceller_t *)malloc(sizeof(*clp));
	if (PFC_EXPECT_FALSE(clp == NULL)) {
		err = ENOMEM;
		goto out_lock;
	}

	err = canceller_init(clp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		free(clp);
		goto out_lock;
	}

	/* Initialize this canceller as the global canceller. */
	clp->icl_ops = &canceller_global_ops;
	ipc_current_canceller = clp;
	pfc_list_push_tail(&ipc_cancellers, &clp->icl_list);

	*clpp = clp;

out_lock:
	IPC_CANCELLER_UNLOCK();
out:
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_canceller_destroy(ipc_canceller_t *clp)
 *	Destroy the given IPC session canceller.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_canceller_destroy(ipc_canceller_t *clp)
{
	IPC_CANCELLER_LOCK();
	canceller_destroy_l(clp);
	IPC_CANCELLER_UNLOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_canceller_destroy_fini(ipc_canceller_t *clp)
 *	Destroy the given IPC session canceller.
 *	This function will be called via library destructor.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_canceller_destroy_fini(ipc_canceller_t *clp)
{
	int	lock_err = IPC_CANCELLER_TRYLOCK();

	if (lock_err == 0) {
		canceller_destroy_l(clp);
		IPC_CANCELLER_UNLOCK();
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_canceller_sess_destroy(ipc_sessclr_t *sclp)
 *	Destroy the given session-specific canceller.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_canceller_sess_destroy(ipc_sessclr_t *sclp)
{
	canceller_destroy(&sclp->iscl_canceller);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_canceller_cleanup(void)
 *	Clean up all session cancellers.
 *	This function will be called via library destructor.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_canceller_cleanup(void)
{
	int	lock_err;

	lock_err = IPC_CLIENT_TRYWRLOCK();
	if (lock_err == 0) {
		canceller_cleanup();
		IPC_CLIENT_UNLOCK();
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_canceller_fork_prepare(void)
 *	fork(2) handler which will be called just before fork(2) on parent
 *	process.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	writer mode.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_canceller_fork_prepare(void)
{
	/* Acquire the canceller lock. */
	IPC_CANCELLER_LOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_canceller_fork_parent(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on parent process.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	writer mode.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_canceller_fork_parent(void)
{
	/* Release the canceller lock. */
	IPC_CANCELLER_UNLOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_canceller_fork_child(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on child process.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_canceller_fork_child(void)
{
	pfc_list_t	*head = &ipc_cancellers, *elem, *next;

	/* Initialize the canceller lock. */
	PFC_ASSERT_INT(PFC_MUTEX_INIT(&ipc_canceller_mutex), 0);

	/* Discard all cancellers. */
	PFC_LIST_FOREACH_SAFE(head, elem, next) {
		canceller_destroy(IPC_CANCELLER_LIST2PTR(elem));
	}

	pfc_list_init(head);
	ipc_current_canceller = NULL;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcclnt_canceller_conn_notify(ipc_conn_t *cnp, pfc_ptr_t arg)
 *	Send global cancellation notification to all sessions associated with
 *	the given connection, and wake up all threads blocked on the given
 *	connection.
 *
 *	`arg' must be NULL or a pointer to ipc_clnotify_t.
 *
 *	icln_elsess
 *	    If IPC_CLNOTIFY_FORCE is specified, all cancelable client sessions
 *	    associated with the given connection will be canceled.
 *
 *	    If a non-NULL value except IPC_CLNOTIFY_FORCE is specified,
 *	    all cancelable client sessions associated with the given connection
 *	    will be canceled only if the target server of the specified IPC
 *	    event listener session equals to the server with which the given
 *	    connection is associated.
 *
 *	    Note that above 2 cases will always ignore PFC_IPCSSF_NOGLOBCANCEL
 *	    flag in client sessions.
 *
 *	    If NULL is specified, this function performs global cancellation.
 *	    So client sessions with PFC_IPCSSF_NOGLOBCANCEL flag will never
 *	    be canceled.
 *
 *	icln_discard
 *	    If PFC_TRUE is specified, the state of the client session will be
 *	    changed to DISCARD only if the session is canceled.
 *	    Note that this field is treated as PFC_FALSE if icln_elsess is
 *	    not NULL.
 *	    
 *	If NULL is specified to `arg', it will be treated as if NULL is
 *	specified to icln_elsess and PFC_FALSE is specified to icln_discard.
 *
 * Remarks:
 *	This function must be called with holding the client lock in
 *	reader or writer mode.
 */
void PFC_ATTR_HIDDEN
pfc_ipcclnt_canceller_conn_notify(ipc_conn_t *cnp, pfc_ptr_t arg)
{
	ipc_clnotify_t	clnotify, *clnp = (ipc_clnotify_t *)arg;

	if (clnp == NULL) {
		/* Perform global cancellation. */
		clnp = &clnotify;
		IPC_CLNOTIFY_INIT_GLOBAL(clnp, PFC_FALSE);
	}

	IPC_CONN_LOCK(cnp);
	canceller_conn_notify_l(cnp, clnp);
	IPC_CONN_UNLOCK(cnp);
}

/*
 * static int
 * canceller_init(ipc_canceller_t *clp)
 *	Initialize the given canceller instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function initializes only icl_pipe and icl_refcnt.
 *	Other fields must be initialized by the caller.
 */
static int
canceller_init(ipc_canceller_t *clp)
{
	int	err;

	/* Prepare a pipe which notifies cancel request. */
	err = pfc_pipe_open(clp->icl_pipe, PFC_PIPE_CLOEXEC_NB);
	if (PFC_EXPECT_FALSE(err != 0)) {
		err = errno;
		if (PFC_EXPECT_FALSE(err == 0)) {
			/* This should never happen. */
			err = EIO;
		}

		return err;
	}

	/*
	 * Initialize the reference counter with 2. One is for reference from
	 * ipc_current_canceller or session instance, and another is for
	 * the caller.
	 */
	clp->icl_refcnt = 2;

	return 0;
}

/*
 * static void
 * canceller_destroy(ipc_canceller_t *clp)
 *	Destroy the given session canceller.
 *
 * Remarks:
 *	The caller must unlink the given canceller from ipc_cancellers
 *	in advance.
 */
static void
canceller_destroy(ipc_canceller_t *clp)
{
	canceller_close(clp->icl_pipe[0]);
	canceller_close(clp->icl_pipe[1]);

	/*
	 * This free() call is valid even if clp points iscl_canceller
	 * in ipc_sessclr_t.
	 */
	PFC_ASSERT(offsetof(ipc_sessclr_t, iscl_canceller) == 0);
	free(clp);
}

/*
 * static void
 * canceller_cleanup(void)
 *	Clean up all session cancellers.
 *
 * Remarks:
 *	This function must be called with holding the client lock in writer
 *	mode.
 */
static void
canceller_cleanup(void)
{
	ipc_canceller_t	*clp;
	int		lock_err;

	lock_err = IPC_CANCELLER_TRYLOCK();
	if (lock_err != 0) {
		return;
	}

	clp = ipc_current_canceller;
	ipc_current_canceller = NULL;

	if (clp != NULL) {
		uint32_t	ref =
			pfc_atomic_dec_uint32_old(&clp->icl_refcnt);

		if (ref == 1) {
			canceller_destroy_l(clp);
		}
	}

	IPC_CANCELLER_UNLOCK();
}

/*
 * static int
 * canceller_getforsess(ipc_canceller_t **PFC_RESTRICT clpp,
 *			pfc_ipcsess_t *PFC_RESTRICT sess)
 *	Get IPC session canceller instance specific to the given IPC session.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to canceller instance
 *	is set to `*clpp', and zero is returned.
 *
 * Remarks:
 *	This function must be called with holding the client lock in reader
 *	or writer mode.
 */
static int
canceller_getforsess(ipc_canceller_t **PFC_RESTRICT clpp,
		     pfc_ipcsess_t *PFC_RESTRICT sess)
{
	ipc_sessclr_t	*sclp;
	ipc_canceller_t	*clp;
	int		err = 0;

	PFC_ASSERT(IPC_CLSESS_NEED_SPECCANCEL(sess));

	IPC_CLSESS_LOCK(sess);

	/*
	 * Check whether the canceller for the given session is already
	 * created or not.
	 */
	sclp = sess->icss_canceller;
	if (sclp != NULL) {
		/* The canceller is already prepared for this session. */
		PFC_ASSERT(sclp->iscl_sess == sess);
		clp = &sclp->iscl_canceller;
		IPC_CANCELLER_HOLD(clp);
	}
	else {
		/* A new canceller needs to be created for this session. */
		sclp = (ipc_sessclr_t *)malloc(sizeof(*sclp));
		if (PFC_EXPECT_FALSE(sclp == NULL)) {
			err = ENOMEM;
			goto out;
		}

		clp = &sclp->iscl_canceller;
		err = canceller_init(clp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			free(sclp);
			goto out;
		}

		/* Initialize this canceller as session-specific canceller. */
		clp->icl_ops = &canceller_sess_ops;
		sclp->iscl_sess = sess;
		sess->icss_canceller = sclp;

		/*
		 * Initialize clp->icl_list as the list head so that
		 * canceller_destroy_l() can pass this canceller to
		 * pfc_list_remove() unconditionally.
		 */
		pfc_list_init(&clp->icl_list);
	}

	*clpp = clp;

out:
	IPC_CLSESS_UNLOCK(sess);
	IPC_CLIENT_UNLOCK();

	return err;
}

/*
 * static void
 * canceller_sess_notify(pfc_ipcsess_t *sess, pfc_bool_t discard,
 *			 pfc_bool_t do_log)
 *	Send global cancellation notification to the given session-specific
 *	canceller instance.
 *
 *	If PFC_TRUE is specified to `discard' and the given client session has
 *	PFC_IPCSSF_CANCELABLE flag, the state of the given client session will
 *	be changed to DISCARD. In this case further IPC service request on
 *	the given session will get ESHUTDOWN error.
 *	
 *	If PFC_TRUE is specified to `do_log', this funciton records an
 *	informational log record which indicates the specified session has
 *	been canceled.
 *
 * Remarks:
 *	This function must be called with holding the client lock in reader
 *	or writer mode, and the connection lock associated with the given
 *	session.
 */
static void
canceller_sess_notify(pfc_ipcsess_t *sess, pfc_bool_t discard,
		      pfc_bool_t do_log)
{
	ipc_sessclr_t	*sclp;

	IPC_CLSESS_LOCK(sess);

	sclp = sess->icss_canceller;
	if (sclp == NULL) {
		IPC_CLSESS_UNLOCK(sess);
		return;
	}

	/* Make this session-specific canceller invisible to the session. */
	PFC_ASSERT(IPC_CLSESS_NEED_SPECCANCEL(sess));
	sess->icss_canceller = NULL;

	if (discard) {
		/* Freeze the client session. */
		pfc_ipcclnt_sess_freeze(sess);
	}

	IPC_CLSESS_UNLOCK(sess);

	/* Notify cancellation to this session. */
	PFC_ASSERT(sclp->iscl_sess == sess);

	if (do_log) {
		ipc_conn_t	*cnp = sess->icss_conn;

		IPCCLNT_LOG_INFO("Client session has been %s: sess=%p, "
				 "server=%s, service=%s/%u",
				 (discard) ? "discarded" : "canceled",
				 sess, IPC_CLCHAN_NAME(cnp->icn_chan),
				 pfc_refptr_string_value(sess->icss_name),
				 sess->icss_service);
	}

	pfc_ipcclnt_canceller_notify(&sclp->iscl_canceller);
}

/*
 * static int
 * canceller_testcancel(ipc_canceller_t *clp)
 *	Determine whether the given global canceller is already canceled
 *	or not.
 *
 * Calling/Exit State:
 *	ECANCELED is returned if the given canceller is already canceled.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	This function is used to terminate busy wait on the IPC connection.
 *	So it may be called with holding the connection lock associated with
 *	the given canceller.
 */
static int
canceller_testcancel(ipc_canceller_t *clp)
{
	int	err;

	err = (PFC_EXPECT_TRUE(IPC_CANCELLER_ISACTIVE(clp))) ? 0 : ECANCELED;

	return err;
}

/*
 * static int
 * canceller_sess_testcancel(ipc_canceller_t *clp)
 *	Determine whether the given session-specific canceller is already
 *	canceled or not.
 *
 * Calling/Exit State:
 *	ECANCELED is returned if the given canceller is already canceled.
 *	Otherwise zero is returned.
 *
 * Remarks:
 *	- This function is used to terminate busy wait on the IPC connection.
 *	  So it may be called with holding the connection lock associated with
 *	  the given canceller.
 *
 *	- `clp' must be a pointer to iscl_canceller field in ipc_sessclr_t.
 */
static int
canceller_sess_testcancel(ipc_canceller_t *clp)
{
	ipc_sessclr_t	*sclp = IPC_CLP2SCLP(clp);
	pfc_ipcsess_t	*sess = sclp->iscl_sess;
	int		err;

	/*
	 * This function is called while the IPC session is busy.
	 * So we can access the IPC session instance safely because busy
	 * session can not be destroyed.
	 */
	IPC_CLSESS_LOCK(sess);
	PFC_ASSERT(IPC_CLSESS_ISBUSY(sess));
	err = (PFC_EXPECT_TRUE(sess->icss_canceller == sclp)) ? 0 : ECANCELED;
	IPC_CLSESS_UNLOCK(sess);

	return err;
}

/*
 * static pfc_ipcsess_t *
 * canceller_sess_getsess(ipc_canceller_t *clp)
 *	Return a pointer to the IPC client session associated with the given
 *	canceller. `clp' must be a session-specific canceller.
 *
 * Calling/Exit State:
 *	A non-NULL pointer to the IPC client session associated with the
 *	given canceller is returned.
 */
static pfc_ipcsess_t *
canceller_sess_getsess(ipc_canceller_t *clp)
{
	ipc_sessclr_t	*sclp = IPC_CLP2SCLP(clp);

	PFC_ASSERT(sclp->iscl_sess != NULL);

	return sclp->iscl_sess;
}
