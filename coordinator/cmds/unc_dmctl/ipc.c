/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * ipc.c - IPC framework utilities.
 */

#include <pfc/conf.h>
#include <cmdutil.h>
#include "unc_dmctl.h"

/*
 * Default IPC channel name of UNC daemon.
 */
#define	DEFAULT_CHANNEL		"uncd@LOCAL"

/*
 * True if user and group was switched.
 */
static pfc_bool_t	cred_switched = PFC_FALSE;

/*
 * Internal prototypes.
 */
static int	ipc_invoke_error(int err);
/*
 * int
 * ipc_init(void)
 *	Initialize IPC client used to contact with UNC daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
ipc_init(void)
{
	int	err;

	if (ipc_channel == NULL) {
		pfc_cfblk_t	blk;

		/* Determine default IPC channel name. */
		blk = pfc_sysconf_get_block("ipc_server");
		ipc_channel = pfc_conf_get_string(blk, "channel_name",
						  DEFAULT_CHANNEL);
		if (PFC_EXPECT_FALSE(*ipc_channel == '\0')) {
			error("IPC server in UNC daemon is disabled.");

			return ECANCELED;
		}
	}

	/* Set default IPC channel name. */
	err = pfc_ipcclnt_setdefault(ipc_channel);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to set default IPC channel name: channel=%s: %s",
		      ipc_channel, strerror(err));
		/* FALLTHROUGH */
	}

	return err;
}

/*
 * void
 * ipc_fini(void)
 *	Finalize IPC client.
 */
void
ipc_fini(void)
{
	pfc_ipcclnt_cancel(PFC_TRUE);
}

/*
 * int
 * ipc_getsess(pfc_ipcsess_t **UNC_RESTRICT sessp, pfc_ipcid_t service,
 *	       const pfc_timespec_t *UNC_RESTRICT timeout)
 *	Create a new IPC client session.
 *
 *	IPC service name is fixed to "launcher".
 *	Default session timeout is used if NULL is specified to `timeout'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to IPC client session
 *	is set to the buffer pointed by `sessp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
ipc_getsess(pfc_ipcsess_t **UNC_RESTRICT sessp, pfc_ipcid_t service,
	    const pfc_timespec_t *UNC_RESTRICT timeout)
{
	pfc_ipcsess_t	*sess;
	pfc_timespec_t	ts;
	int		err;

	if (!cred_switched) {
		pfc_cfblk_t	opts;

		/* Switch user and group as per pfcd.conf. */
		opts = pfc_sysconf_get_block("options");
		err = pfccmd_switchuser(opts, PFC_FALSE, error);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		if (debug_level > 0) {
			debug_printf(1, "uid=%d, gid=%d", getuid(), getgid());
		}
		cred_switched = PFC_TRUE;
	}

	err = pfc_ipcclnt_sess_create(&sess, LNC_IPC_SERVICE, service);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to create an IPC client session: %s",
		      strerror(err));

		return err;
	}

	if (timeout == NULL) {
		/* Use default timeout. */
		ts.tv_sec = ipc_timeout;
		ts.tv_nsec = 0;
		timeout = &ts;
	}

	debug_printf(1, "IPC service=%s/%u, timeout=%lu.%lu",
		     LNC_IPC_SERVICE, service,
		     timeout->tv_sec, timeout->tv_nsec);

	err = pfc_ipcclnt_sess_settimeout(sess, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to set IPC session timeout: %s", strerror(err));
		PFC_ASSERT_INT(pfc_ipcclnt_sess_destroy(sess), 0);

		return err;
	}

	*sessp = sess;

	return 0;
}

/*
 * int
 * ipc_invoke(pfc_ipcsess_t *UNC_RESTRICT sess,
 *	       pfc_ipcresp_t *UNC_RESTRICT respp)
 *	Invoke IPC service handler in UNC daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, DMCTL_EX_OK is returned.
 *	Otherwise exit status of the program is returned.
 */
int
ipc_invoke(pfc_ipcsess_t *UNC_RESTRICT sess, pfc_ipcresp_t *UNC_RESTRICT respp)
{
	int	err = pfc_ipcclnt_sess_invoke(sess, respp);

	if (PFC_EXPECT_FALSE(err != 0)) {
		return ipc_invoke_error(err);
	}

	return DMCTL_EX_OK;
}

/*
 * void
 * ipc_gethostaddr(pfc_hostaddr_t *haddr)
 *	Set host address of the target IPC channel address to the buffer
 *	pointed by `haddr'.
 *
 * Remarks:
 *	This function must be called after the call of ipc_init().
 */
void
ipc_gethostaddr(pfc_hostaddr_t *haddr)
{
	const char	*p;

	if (PFC_EXPECT_FALSE(ipc_channel == NULL)) {
		/* This should never happen. */
		goto local;
	}

	/* Fetch address part in the IPC channel address. */
	p = strchr(ipc_channel, '@');
	if (p == NULL) {
		goto local;
	}

	p++;
	if (*p == '\0') {
		goto local;
	}

	/*
	 * This should never fail because ipc_channel is already set as
	 * the default IPC channel address.
	 */
	PFC_ASSERT_INT(pfc_hostaddr_fromstring(haddr, p), 0);

	return;

local:
	/* Set local address to haddr. */
	PFC_ASSERT_INT(pfc_hostaddr_init_local(haddr), 0);
}

/*
 * static int
 * ipc_invoke_error(int err)
 *	Print an error message which indicates IPC service invocation error.
 *
 * Calling/Exit State:
 *	Exit status of the program is returned.
 */
static int
ipc_invoke_error(int err)
{
	int	status = DMCTL_EX_FATAL;

	if (err == ECONNREFUSED) {
		error("UNC daemon is not running.");
		status = DMCTL_EX_NOTRUNNING;
	}
	else if (err == ENOSPC) {
		error("Too many clients are conneted to UNC daemon.");
		status = DMCTL_EX_TEMPFAIL;
	}
	else if (err == EPERM || err == EACCES) {
		error("%s", str_err_EPERM);
		status = DMCTL_EX_AUTH;
	}
	else if (err == ECONNRESET || err == EPIPE) {
		error("IPC client sesion was reset by UNC daemon.");
		status = DMCTL_EX_RESET;
	}
	else if (err == ETIMEDOUT) {
		error("IPC service timed out.");
		status = DMCTL_EX_TIMEDOUT;
	}
	else {
		error("Failed to send service request to UNC daemon: %s",
		      strerror(err));
	}

	return status;
}
