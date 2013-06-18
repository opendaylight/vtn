/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * session.c - IPC client session management.
 */

#include "ipcclnt_impl.h"

/*
 * Check whether flag bits passed to pfc_ipcclnt_sess_create4() and
 * pfc_ipcclnt_sess_altcreate5() are valid.
 */
#define	IPCSESS_CFLAGS_IS_VALID(flags)					\
	PFC_EXPECT_TRUE(((flags) & ~IPC_CLSESS_CF_VALID_FLAGS) == 0)

/*
 * Internal prototypes.
 */
static int	ipcsess_create(pfc_ipcsess_t **PFC_RESTRICT sessp,
			       const char *PFC_RESTRICT name,
			       pfc_ipcid_t service, uint32_t flags);
static int	ipcsess_altcreate(pfc_ipcsess_t **PFC_RESTRICT sessp,
				  pfc_ipcconn_t conn,
				  const char *PFC_RESTRICT name,
				  pfc_ipcid_t service, uint32_t flags);
static int	ipcsess_create_impl(pfc_ipcsess_t **PFC_RESTRICT sessp,
				    ipc_conn_t *PFC_RESTRICT cnp,
				    const char *PFC_RESTRICT name,
				    pfc_ipcid_t service, uint32_t flags);
static int	ipcsess_check_disabled(int err);

/*
 * int
 * pfc_ipcclnt_sess_create(pfc_ipcsess_t **PFC_RESTRICT sessp,
 *			   const char *PFC_RESTRICT name, pfc_ipcid_t service)
 *	Create a new IPC client session on the default IPC connection.
 *
 *	This function creates a new IPC client session to issue IPC service
 *	to the PFC daemon.
 *
 *	`name' must be a non-NULL string which represents IPC service name
 *	on the default IPC channel.
 *	`service' is a service identifier used to distinguish IPC service.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new IPC client handle is set to `*sessp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *	NULL is set to `*sessp' on error return unless `sessp' is NULL.
 */
int
pfc_ipcclnt_sess_create(pfc_ipcsess_t **PFC_RESTRICT sessp,
			const char *PFC_RESTRICT name, pfc_ipcid_t service)
{
	return ipcsess_create(sessp, name, service, 0);
}

/*
 * int
 * pfc_ipcclnt_sess_create4(pfc_ipcsess_t **PFC_RESTRICT sessp,
 *			    const char *PFC_RESTRICT name, pfc_ipcid_t service,
 *			    uint32_t flags)
 *	Create a new IPC client session on the default IPC connection.
 *
 *	This function is similar to pfc_ipcclnt_sess_create(), but it takes
 *	one additional argument `flags'. It is either zero or the bitwise OR
 *	of one or more of the following flags:
 *
 *	PFC_IPCSSF_CANCELABLE
 *	    Enable session-specific cancellation.
 *	    The IPC session created with this flag can be canceled by the
 *	    call of pfc_ipcclnt_sess_cancel(), without any effect to other
 *	    IPC sessions.
 *
 *	PFC_IPCSSF_NOGLOBCANCEL
 *	    Disable global cancellation by pfc_ipcclnt_cancel(PFC_FALSE).
 *	    The IPC session created with this flag will never be canceled
 *	    by the call of pfc_ipcclnt_cancel(PFC_FALSE).
 *	    Note that the call of pfc_ipcclnt_cancel(PFC_TRUE) still cancels
 *	    all IPC sessions, including sessions with this flag.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new IPC client handle is set to `*sessp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *	NULL is set to `*sessp' on error return unless `sessp' is NULL.
 */
int
pfc_ipcclnt_sess_create4(pfc_ipcsess_t **PFC_RESTRICT sessp,
			 const char *PFC_RESTRICT name, pfc_ipcid_t service,
			 uint32_t flags)
{
	int	err;

	if (PFC_EXPECT_TRUE(IPCSESS_CFLAGS_IS_VALID(flags))) {
		err = ipcsess_create(sessp, name, service, flags);
	}
	else {
		if (PFC_EXPECT_TRUE(sessp != NULL)) {
			*sessp = NULL;
		}
		err = EINVAL;
	}

	return err;
}

/*
 * int
 * pfc_ipcclnt_sess_reset(pfc_ipcsess_t *PFC_RESTRICT sess,
 *			  const char *PFC_RESTRICT name, pfc_ipcid_t service)
 *	Reset the session state.
 *
 *	`name' and `service' is a pair of IPC service name and ID for new
 *	IPC service request. If you want to issue a new IPC service request
 *	on existing client session, the session must be reset by this function.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Response of the previous IPC request from the server is always
 *	discarded.
 */
int
pfc_ipcclnt_sess_reset(pfc_ipcsess_t *PFC_RESTRICT sess,
		       const char *PFC_RESTRICT name, pfc_ipcid_t service)
{
	ipc_conn_t	*cnp;
	pfc_refptr_t	*rname;
	ipc_sstate_t	state;
	int		err;

	if (PFC_EXPECT_FALSE(sess == NULL)) {
		return EINVAL;
	}

	if (PFC_EXPECT_FALSE(IPC_CLSESS_ISEVENT(sess))) {
		/* Event session is always frozen. */
		return ESHUTDOWN;
	}

	/* Verify service name. */
	err = pfc_ipc_check_service_name(name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	rname = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(rname == NULL)) {
		return ENOMEM;
	}

	cnp = sess->icss_conn;
	IPC_CONN_LOCK(cnp);
	IPC_CLSESS_LOCK(sess);

	if (PFC_EXPECT_FALSE(IPC_CLSESS_ISBUSY(sess))) {
		/* This session is busy. */
		err = EBUSY;
		goto error;
	}

	if (IPC_CLSESS_ISFROZEN(sess)) {
		/* This session is frozen. */
		err = ESHUTDOWN;
		goto error;
	}

	state = sess->icss_state;
	if (PFC_EXPECT_FALSE(state != IPC_SSTATE_READY &&
			     state != IPC_SSTATE_RESULT)) {
		err = pfc_ipcclnt_sess_state_error(state);
		goto error;
	}

	/* Ensure the connection is still valid. */
	if (PFC_EXPECT_FALSE(cnp->icn_flags & IPCONNF_CLOSED)) {
		err = ESHUTDOWN;
		goto error;
	}

	/* Initialize IPC message and output stream. */
	pfc_ipcclnt_sess_resetmsg(sess);

	/* Reset the session state. */
	pfc_refptr_put(sess->icss_name);
	sess->icss_name = rname;
	sess->icss_service = service;
	sess->icss_state = IPC_SSTATE_READY;

	IPC_CLSESS_UNLOCK(sess);
	IPC_CONN_UNLOCK(cnp);

	return 0;

error:
	IPC_CLSESS_UNLOCK(sess);
	IPC_CONN_UNLOCK(cnp);
	pfc_refptr_put(rname);

	return err;
}

/*
 * int
 * pfc_ipcclnt_sess_settimeout(pfc_ipcsess_t *PFC_RESTRICT sess,
 *			       const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Set IPC client session timeout by pfc_timespec_t.
 *	Specifying NULL to `timeout' means that an infinite timeout should
 *	be used on the IPC client session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_sess_settimeout(pfc_ipcsess_t *PFC_RESTRICT sess,
			    const pfc_timespec_t *PFC_RESTRICT timeout)
{
	ipc_sstate_t	state;
	int		err;

	if (PFC_EXPECT_FALSE(sess == NULL)) {
		return EINVAL;
	}

	IPC_CLSESS_LOCK(sess);

	state = sess->icss_state;
	if (IPC_CLSESS_ISFROZEN(sess)) {
		/* This session is frozen. */
		err = ESHUTDOWN;
	}
	else if (PFC_EXPECT_FALSE(state != IPC_SSTATE_READY &&
				  state != IPC_SSTATE_RESULT)) {
		err = pfc_ipcclnt_sess_state_error(state);
	}
	else if (timeout == NULL) {
		err = 0;
		sess->icss_flags |=
			(IPC_CLSSF_NOTIMEOUT | IPC_CLSSF_HASTIMEOUT);
	}
	else if (PFC_EXPECT_FALSE(!PFC_CLOCK_IS_VALID(timeout))) {
		err = EINVAL;
	}
	else {
		err = 0;
		sess->icss_flags |= IPC_CLSSF_HASTIMEOUT;
		sess->icss_flags &= ~IPC_CLSSF_NOTIMEOUT;
		sess->icss_timeout = *timeout;
	}

	IPC_CLSESS_UNLOCK(sess);

	return err;
}

/*
 * int
 * pfc_ipcclnt_sess_invoke(pfc_ipcsess_t *PFC_RESTRICT sess,
 *			   pfc_ipcresp_t *PFC_RESTRICT respp)
 *	Issue a request of the IPC service.
 *
 *	All PDUs previously added by the call of pfc_ipcclnt_output_XXX() are
 *	sent to the IPC server. On successful return, pfc_ipcclnt_getres_XXX()
 *	will be available to obtain additional response from the IPC server.
 *
 * Calling/Exit State:
 *	Upon successful completion, a response code sent by the IPC server
 *	is set to `*respp', and zero is returned.
 *
 *	ENOSYS is returned if the IPC server does not implement the IPC
 *	service specified by a pair of the service name and ID, passed to
 *	the call of pfc_ipcclnt_sess_create() or pfc_ipcclnt_sess_reset().
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	PDUs added to the IPC output stream will be discarded on return,
 *	unless the session is active on another thread.
 */
int
pfc_ipcclnt_sess_invoke(pfc_ipcsess_t *PFC_RESTRICT sess,
			pfc_ipcresp_t *PFC_RESTRICT respp)
{
	ipc_conn_t	*cnp;
	ipc_sstate_t	state;
	ipc_canceller_t	*clp;
	pfc_timespec_t	*abstime, tsbuf;
	int		err;

	if (PFC_EXPECT_FALSE(sess == NULL)) {
		return EINVAL;
	}

	if (PFC_EXPECT_FALSE(IPC_CLSESS_ISEVENT(sess))) {
		/* Event session is always frozen. */
		return ESHUTDOWN;
	}

	clp = NULL;
	cnp = sess->icss_conn;

	if (PFC_EXPECT_FALSE(respp == NULL)) {
		err = EINVAL;
		goto error;
	}

	/* Obtain session canceller. */
	err = pfc_ipcclnt_canceller_get(&clp, sess);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	IPC_CONN_LOCK(cnp);
	IPC_CLSESS_LOCK(sess);

	if (IPC_CLSESS_ISFROZEN(sess)) {
		/* This session is frozen. */
		err = ESHUTDOWN;
		goto error_locked;
	}

	state = sess->icss_state;
	if (PFC_EXPECT_FALSE(state != IPC_SSTATE_READY)) {
		err = pfc_ipcclnt_sess_state_error(state);
		goto error_locked;
	}

	/* Set up session timeout. */
	if (sess->icss_flags & IPC_CLSSF_HASTIMEOUT) {
		if (sess->icss_flags & IPC_CLSSF_NOTIMEOUT) {
			abstime = NULL;
		}
		else {
			abstime = &tsbuf;
			err = pfc_clock_abstime(abstime, &sess->icss_timeout);
			if (PFC_EXPECT_FALSE(err != 0)) {
				/* This should never happen. */
				goto error_locked;
			}
		}
	}
	else {
		ipc_clchan_t	*chp = cnp->icn_chan;
		uint32_t	timeout = chp->ichc_timeout;

		/* Use default timeout defined by channel configuration. */
		if (timeout == 0) {
			abstime = NULL;
		}
		else {
			abstime = &tsbuf;
			err = pfc_clock_gettime(abstime);
			if (PFC_EXPECT_FALSE(err != 0)) {
				/* This should never happen. */
				goto error_locked;
			}

			abstime->tv_sec += timeout;
		}
	}

	if (IPC_CLSESS_NEED_SPECCANCEL(sess) &&
	    PFC_EXPECT_FALSE(sess->icss_canceller == NULL)) {
		/* This session was canceled by another thread. */
		err = ECANCELED;
		goto error_locked;
	}

	/*
	 * Set up IPC connection, and issue an IPC service request.
	 * Note that pfc_ipcclnt_conn_invoke() always frees up IPC output
	 * stream, and releases the connection and session locks.
	 */
	err = pfc_ipcclnt_conn_invoke(cnp, sess, clp, respp, abstime);
	if (PFC_EXPECT_FALSE(err == ECANCELED)) {
		/* Check to see whether the library is permanently disabled. */
		err = ipcsess_check_disabled(err);
	}

	return err;

error:
	IPC_CONN_LOCK(cnp);
	IPC_CLSESS_LOCK(sess);

error_locked:
	/*
	 * Free up IPC output stream on error.
	 * If the session state is not BUSY, we can safely destroy the output
	 * stream because only READY and BUSY state can have additional data.
	 */
	if (sess->icss_state != IPC_SSTATE_BUSY) {
		pfc_ipcstream_reset(&sess->icss_output, 0);
	}

	IPC_CLSESS_UNLOCK(sess);
	IPC_CONN_UNLOCK(cnp);

	if (clp != NULL) {
		IPC_CANCELLER_RELEASE(clp);
	}

	if (PFC_EXPECT_FALSE(err == ECANCELED)) {
		/* Check to see whether the library is permanently disabled. */
		err = ipcsess_check_disabled(err);
	}

	return err;
}

/*
 * int
 * pfc_ipcclnt_sess_cancel(pfc_ipcsess_t *sess, pfc_bool_t discard)
 *	Cancel ongoing IPC invocation request on the given IPC session.
 *
 *	After the call of this function, all ongoing call of
 *	pfc_ipcclnt_sess_invoke() with the given session will get ECANCELED
 *	error.
 *
 *	If PFC_TRUE is passed to `discard', the state of the client session
 *	will be changed to DISCARD. That is, further IPC service request on
 *	the given session will get ESHUTDOWN error.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EPERM is returned if the IPC session specified by `sess' is not
 *	created with PFC_IPCSSF_CANCELABLE or PFC_IPCSSF_NOGLOBCANCEL flag.
 */
int
pfc_ipcclnt_sess_cancel(pfc_ipcsess_t *sess, pfc_bool_t discard)
{
	ipc_sessclr_t	*sclp;

	if (PFC_EXPECT_FALSE(sess == NULL)) {
		return EINVAL;
	}

	/*
	 * We can check creation flags without holding the session lock
	 * because it is immutable.
	 */
	if (PFC_EXPECT_FALSE(!IPC_CLSESS_NEED_SPECCANCEL(sess))) {
		return EPERM;
	}

	IPC_CLIENT_RDLOCK();
	IPC_CLSESS_LOCK(sess);

	/* Make current session canceller invisible. */
	sclp = sess->icss_canceller;
	sess->icss_canceller = NULL;

	if (discard) {
		/* Freeze the client session. */
		pfc_ipcclnt_sess_freeze(sess);
	}

	IPC_CLSESS_UNLOCK(sess);

	if (sclp != NULL) {
		ipc_conn_t	*cnp = sess->icss_conn;

		/* Notify cancellation to the given session. */
		pfc_ipcclnt_canceller_notify(&sclp->iscl_canceller);

		/*
		 * Cancel busy wait on the connection associated with the
		 * session.
		 */
		IPC_CONN_LOCK(cnp);
		IPC_CONN_BROADCAST(cnp);
		IPC_CONN_UNLOCK(cnp);
	}

	IPC_CLIENT_UNLOCK();

	return 0;
}

/*
 * int
 * pfc_ipcclnt_sess_destroy(pfc_ipcsess_t *sess)
 *	Destroy the given IPC client session.
 *	Note that the caller must ensure that the given session is no longer
 *	used.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function returns zero immediately if `sess' is a pseudo IPC
 *	client session in an IPC event.
 */
int
pfc_ipcclnt_sess_destroy(pfc_ipcsess_t *sess)
{
	ipc_conn_t	*cnp;
	ipc_canceller_t	*clp;
	ipc_sessclr_t	*sclp;

	if (PFC_EXPECT_FALSE(sess == NULL)) {
		return EINVAL;
	}

	if (PFC_EXPECT_FALSE(IPC_CLSESS_ISEVENT(sess))) {
		/* Nothing to do here. */
		return 0;
	}

	cnp = sess->icss_conn;
	IPC_CONN_LOCK(cnp);
	IPC_CLSESS_LOCK(sess);

	if (PFC_EXPECT_FALSE(IPC_CLSESS_ISBUSY(sess))) {
		IPC_CLSESS_UNLOCK(sess);
		IPC_CONN_UNLOCK(cnp);

		return EBUSY;
	}

	/*
	 * Make the session-specific canceller for this session invisible to
	 * cached connection.
	 */
	if (IPC_CLSESS_NEED_SPECCANCEL(sess) &&
	    (clp = cnp->icn_canceller) != NULL &&
	    IPC_CANCELLER_GETSESS(clp) == sess) {
		pfc_iostream_t	stream = cnp->icn_stream;

		PFC_ASSERT(stream != NULL);
		cnp->icn_canceller = NULL;
		PFC_ASSERT_INT(pfc_iostream_setcanceller(stream, -1), 0);
		IPC_CANCELLER_RELEASE(clp);
	}

	/* Unlink this session from the connection handle. */
	pfc_list_remove(&sess->icss_list);

	if (cnp->icn_flags & IPCONNF_CLOSED) {
		IPC_CONN_BROADCAST(cnp);
	}

	sclp = sess->icss_canceller;
	sess->icss_canceller = NULL;

	IPC_CLSESS_UNLOCK(sess);
	IPC_CONN_UNLOCK(cnp);

	/* Release the connection handle. */
	IPC_CONN_RELEASE(cnp);

	if (sclp != NULL) {
		/*
		 * Release the session-specific canceller.
		 * This must be the last reference.
		 */
		clp = &sclp->iscl_canceller;
		PFC_ASSERT(clp->icl_refcnt == 1);
		IPC_CANCELLER_RELEASE(clp);
	}

	PFC_ASSERT_INT(pfc_mutex_destroy(&sess->icss_mutex), 0);
	pfc_refptr_put(sess->icss_name);

	pfc_ipcmsg_destroy(&sess->icss_msg);
	pfc_ipcstream_destroy(&sess->icss_output);

	free(sess);

	return 0;
}

/*
 * void
 * __pfc_ipcclnt_sess_destroy(pfc_ipcsess_t *sess)
 *	Destroy the given IPC client session.
 *	Note that the caller must ensure that the given session is no longer
 *	used.
 *
 * Calling/Exit State:
 *	On debug build, any error causes program abort.
 *	On release build, any error is ignored.
 */
#ifdef	PFC_VERBOSE_DEBUG
void
__pfc_ipcclnt_sess_destroy(pfc_ipcsess_t *sess)
{
	PFC_ASSERT_INT(pfc_ipcclnt_sess_destroy(sess), 0);
}
#else	/* !PFC_VERBOSE_DEBUG */
void	__pfc_ipcclnt_sess_destroy(pfc_ipcsess_t *sess)
	PFC_ATTR_WEAK_ALIAS(pfc_ipcclnt_sess_destroy);
#endif	/* PFC_VERBOSE_DEBUG */

/*
 * int
 * pfc_ipcclnt_sess_altcreate(pfc_ipcsess_t **PFC_RESTRICT sessp,
 *			      pfc_ipcconn_t conn,
 *			      const char *PFC_RESTRICT name,
 *			      pfc_ipcid_t service)
 *	Create a new IPC client session on the IPC connection specified by
 *	`conn'.
 *
 *	`conn' must be a connection handle created by pfc_ipcclnt_altopen().
 *	`name' must be a non-NULL string which represents IPC service name
 *	on the default IPC channel. `service' is a service identifier used to
 *	distinguish IPC service.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new IPC client handle is set to `*sessp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_sess_altcreate(pfc_ipcsess_t **PFC_RESTRICT sessp,
			   pfc_ipcconn_t conn, const char *PFC_RESTRICT name,
			   pfc_ipcid_t service)
{
	return ipcsess_altcreate(sessp, conn, name, service, 0);
}

/*
 * int
 * pfc_ipcclnt_sess_altcreate5(pfc_ipcsess_t **PFC_RESTRICT sessp,
 *			       pfc_ipcconn_t conn,
 *			       const char *PFC_RESTRICT name,
 *			       pfc_ipcid_t service, uint32_t flags)
 *	Create a new IPC client session on the IPC connection specified by
 *	`conn'.
 *
 *	This function is similar to pfc_ipcclnt_sess_altcreate(), but it takes
 *	one additional argument `flags'. It is either zero or the bitwise OR
 *	of one or more of the following flags:
 *
 *	PFC_IPCSSF_CANCELABLE
 *	    Enable session-specific cancellation.
 *	    The IPC session created with this flag can be canceled by the
 *	    call of pfc_ipcclnt_sess_cancel(), without any effect to other
 *	    IPC sessions.
 *
 *	PFC_IPCSSF_NOGLOBCANCEL
 *	    Disable global cancellation by pfc_ipcclnt_cancel(PFC_FALSE).
 *	    The IPC session created with this flag will never be canceled
 *	    by the call of pfc_ipcclnt_cancel(PFC_FALSE).
 *	    Note that the call of pfc_ipcclnt_cancel(PFC_TRUE) still cancels
 *	    all IPC sessions, including sessions with this flag.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new IPC client handle is set to `*sessp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipcclnt_sess_altcreate5(pfc_ipcsess_t **PFC_RESTRICT sessp,
			    pfc_ipcconn_t conn, const char *PFC_RESTRICT name,
			    pfc_ipcid_t service, uint32_t flags)
{
	int	err;

	if (PFC_EXPECT_TRUE(IPCSESS_CFLAGS_IS_VALID(flags))) {
		err = ipcsess_altcreate(sessp, conn, name, service, flags);
	}
	else {
		if (PFC_EXPECT_TRUE(sessp != NULL)) {
			*sessp = NULL;
		}
		err = EINVAL;
	}

	return err;
}

/*
 * int
 * pfc_ipcclnt_forward(pfc_ipcsess_t *PFC_RESTRICT dsess,
 *		       pfc_ipcsess_t *PFC_RESTRICT ssess, uint32_t begin,
 *		       uint32_t end)
 *	Forward additional data received from the IPC server to another IPC
 *	client session's output stream.
 *
 *	This function can be used to forward additional data received from
 *	the IPC server to another IPC service handler.
 *
 *	`dsess' is the IPC client session to append additional data. Its state
 *	must be READY.
 *	`ssess' is the IPC client session which keeps additional data received
 *	from the IPC server. Its state must be RESULT.
 *
 *	`begin' and `end' specifies additional data in `ssess' to be forwarded.
 *	`begin' is the inclusive beginning index, and `end' is the exclusive
 *	ending index of additional data. For instance, if `begin' is 3 and
 *	`end' is 10, additional data in `ssess' at the index from 3 to 9 are
 *	appended to the IPC output stream in `dsess'. Needless to say,
 *	`end' must be greater than `begin'.
 *
 *	If the value specified to `end' is greater than the number of
 *	additional data in `ssess', it is treated as if the number of
 *	additional data in `ssess' is specified to `end'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function returns zero if the same value is specified to `begin'
 *	  and `end', even if it is not valid additional data index.
 *
 *	- Errors except for EINVAL may be unrecoverable. If this function
 *	  returns non-zero value except for EINVAL, the IPC client session
 *	  specified by `dsess' should be destroyed.
 */
int
pfc_ipcclnt_forward(pfc_ipcsess_t *PFC_RESTRICT dsess,
		    pfc_ipcsess_t *PFC_RESTRICT ssess, uint32_t begin,
		    uint32_t end)
{
	ipc_sstate_t	state;
	int		err;

	if (PFC_EXPECT_FALSE(dsess == NULL || ssess == NULL)) {
		return EINVAL;
	}

	/*
	 * Pin additional data in `ssess'.
	 * Note that this must be done without holding the client session
	 * lock for `dsess', or deadlock may happen.
	 */
	err = pfc_ipcclnt_sess_pinmsg(ssess);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Check the state of the destination client session. */
	IPC_CLSESS_LOCK(dsess);

	state = dsess->icss_state;
	if (PFC_EXPECT_TRUE(state == IPC_SSTATE_READY)) {
		/* Copy additional data in the specified range. */
		err = pfc_ipcstream_copymsg(&dsess->icss_output,
					    &ssess->icss_msg, begin, end);
	}
	else {
		err = pfc_ipcclnt_sess_state_error(state);
	}

	IPC_CLSESS_UNLOCK(dsess);
	pfc_ipcclnt_sess_unpinmsg(ssess);

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipcclnt_sess_state_error(ipc_sstate_t state)
 *	Return error code which indicates the session state is invalid.
 */
int PFC_ATTR_HIDDEN
pfc_ipcclnt_sess_state_error(ipc_sstate_t state)
{
	if (state == IPC_SSTATE_BUSY) {
		return EBUSY;
	}

	if (state == IPC_SSTATE_DISCARD) {
		return ESHUTDOWN;
	}

	return EBADFD;
}

/*
 * static int
 * ipcsess_create(pfc_ipcsess_t **PFC_RESTRICT sessp,
 *		  const char *PFC_RESTRICT name, pfc_ipcid_t service,
 *		  uint32_t flags)
 *	Create a new IPC client session on the default IPC connection.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new IPC client handle is set to `*sessp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *	NULL is set to `*sessp' on error return unless `sessp' is NULL.
 *
 * Remarks:
 *	`flags' must be verified by the caller in advance.
 */
static int
ipcsess_create(pfc_ipcsess_t **PFC_RESTRICT sessp,
	       const char *PFC_RESTRICT name, pfc_ipcid_t service,
	       uint32_t flags)
{
	ipc_conn_t	*cnp;
	int		err;

	/* Get the default connection handle. */
	err = pfc_ipcclnt_conn_default(&cnp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (PFC_EXPECT_TRUE(sessp != NULL)) {
			*sessp = NULL;
		}

		return err;
	}

	/* Create a new session on the default handle. */
	return ipcsess_create_impl(sessp, cnp, name, service, flags);
}

/*
 * static int
 * ipcsess_altcreate(pfc_ipcsess_t **PFC_RESTRICT sessp,
 *		     pfc_ipcconn_t conn, const char *PFC_RESTRICT name,
 *		     pfc_ipcid_t service, uint32_t flags)
 *	Create a new IPC client session on the IPC connection specified by
 *	`conn'
 *
 * Calling/Exit State:
 *	Upon successful completion, a new IPC client handle is set to `*sessp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	`flags' must be verified by the caller in advance.
 */
static int
ipcsess_altcreate(pfc_ipcsess_t **PFC_RESTRICT sessp, pfc_ipcconn_t conn,
		  const char *PFC_RESTRICT name, pfc_ipcid_t service,
		  uint32_t flags)
{
	ipc_conn_t	*cnp;

	/* Get the connection handle. */
	cnp = pfc_ipcclnt_getconn(conn);
	if (PFC_EXPECT_FALSE(cnp == NULL)) {
		if (PFC_EXPECT_TRUE(sessp != NULL)) {
			*sessp = NULL;
		}

		return EBADF;
	}

	/* Create a new session on the specified connection. */
	return ipcsess_create_impl(sessp, cnp, name, service, flags);
}

/*
 * static int
 * ipcsess_create_impl(pfc_ipcsess_t **PFC_RESTRICT sessp,
 *		       ipc_conn_t *PFC_RESTRICT cnp,
 *		       const char *PFC_RESTRICT name, pfc_ipcid_t service,
 *		       uint32_t flags)
 *	Create a new client session on the given IPC connection.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to client session is
 *	stored to `*sessp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function releases the connection handle on error.
 *
 *	- `flags' must be verified by the caller in advance.
 */
static int
ipcsess_create_impl(pfc_ipcsess_t **PFC_RESTRICT sessp,
		    ipc_conn_t *PFC_RESTRICT cnp,
		    const char *PFC_RESTRICT name, pfc_ipcid_t service,
		    uint32_t flags)
{
	pfc_ipcsess_t	*sess;
	int		err;

	if (PFC_EXPECT_FALSE(sessp == NULL)) {
		err = EINVAL;
		goto error;
	}

	/* Verify service name. */
	err = pfc_ipc_check_service_name(name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Allocate a new session instance. */
	sess = (pfc_ipcsess_t *)malloc(sizeof(*sess));
	if (PFC_EXPECT_FALSE(sess == NULL)) {
		err = ENOMEM;
		goto error;
	}

	sess->icss_name = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(sess->icss_name == NULL)) {
		err = ENOMEM;
		goto error_sess;
	}

	err = PFC_MUTEX_INIT(&sess->icss_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error_sess;
	}

	if (flags & PFC_IPCSSF_NOGLOBCANCEL) {
		/* PFC_IPCSSF_NOGLOBCANCEL implies PFC_IPCSSF_CANCELABLE. */
		flags |= PFC_IPCSSF_CANCELABLE;
	}

	sess->icss_state = IPC_SSTATE_READY;
	sess->icss_service = service;
	sess->icss_flags = 0;
	sess->icss_cflags = flags;
	sess->icss_busy = 0;
	sess->icss_canceller = NULL;

	/* Ensure that the connection handle is valid. */
	IPC_CONN_LOCK(cnp);
	if (PFC_EXPECT_FALSE(cnp->icn_flags & IPCONNF_CLOSED)) {
		IPC_CONN_UNLOCK(cnp);
		err = ESHUTDOWN;
		goto error_sess;
	}

	pfc_ipcmsg_init(&sess->icss_msg, 0);
	pfc_ipcstream_init(&sess->icss_output, 0);

	/* Link session instance to the connection handle. */
	pfc_list_push_tail(&cnp->icn_sessions, &sess->icss_list);
	sess->icss_conn = cnp;
	*sessp = sess;

	IPC_CONN_UNLOCK(cnp);

	return 0;

error_sess:
	if (sess->icss_name != NULL) {
		pfc_refptr_put(sess->icss_name);
	}
	free(sess);

error:
	/* Release connection handle. */
	IPC_CONN_RELEASE(cnp);

	if (PFC_EXPECT_TRUE(sessp != NULL)) {
		*sessp = NULL;
	}

	return err;
}

/*
 * static int
 * ipcsess_check_disabled(int err)
 *	Check to see whether the IPC client library is permanently disabled.
 *
 * Calling/Exit State:
 *	ECONNABORTED is returned if the IPC client library is permanently
 *	disabled.
 *	Otherwise error number specified by `err' is returned.
 */
static int
ipcsess_check_disabled(int err)
{
	IPC_CLIENT_RDLOCK();
	if (PFC_EXPECT_FALSE(ipc_disabled)) {
		err = ECONNABORTED;
	}
	IPC_CLIENT_UNLOCK();

	return err;
}
