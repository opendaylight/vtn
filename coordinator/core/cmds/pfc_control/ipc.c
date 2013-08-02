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

#include <pfc/ipc_pfcd.h>
#include <pfc/conf.h>
#include "pfc_control.h"
#include "ipc.h"

/*
 * IPC channel name of the PFC daemon.
 */
static const char	*pfcd_channel;

/*
 * IPC channel address specified by command line option.
 */
static const char	*pfcd_channel_addr;

/*
 * True if user and group was switched.
 */
static pfc_bool_t	cred_switched = PFC_FALSE;

/*
 * void
 * ipc_init(const char *chaddr)
 *	Initialize IPC client which connects to the PFC daemon.
 */
void
ipc_init(const char *chaddr)
{
	pfc_cfblk_t	cfblk;
	int		err;

	if (chaddr == NULL) {
		/* Determine IPC channel name of the PFC daemon. */
		cfblk = pfc_sysconf_get_block("ipc_server");
		chaddr = pfc_conf_get_string(cfblk, "channel_name",
					     daemon_name);
		if (PFC_EXPECT_FALSE(*chaddr == '\0')) {
			/* IPC server is disabled. */
			return;
		}
	}
	else {
		pfcd_channel_addr = chaddr;
	}

	/* Change IPC channel address of the default connection. */
	err = pfc_ipcclnt_setdefault(chaddr);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to set IPC channel address to \"%s\": %s",
		      chaddr, strerror(err));
		/* NOTREACHED */
	}

	debug_printf(1, "IPC channel = %s", chaddr);
	pfcd_channel = chaddr;
}

/*
 * int
 * ipc_sess_create_pfcd(pfc_ipcsess_t **PFC_RESTRICT sessp, pfc_ipcid_t service)
 *	Create an IPC client session which connects to the PFC daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to IPC client session is set
 *	to the buffer pointed by `sessp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function changes user and group ID of the calling process if
 *	user and group is defined in pfcd.conf.
 */
int
ipc_sess_create_pfcd(pfc_ipcsess_t **PFC_RESTRICT sessp, pfc_ipcid_t service)
{
	pfc_ipcsess_t	*sess;
	pfc_timespec_t	ts;
	int		err;

	if (PFC_EXPECT_FALSE(pfcd_channel == NULL)) {
		error("IPC service on %s is disabled.", daemon_name);

		return ENOSYS;
	}

	if (!cred_switched) {
		/*
		 * Switch user and group as per pfcd.conf.
		 * pfccmd_switchuser() never returns on error because it will
		 * call fatal().
		 */
		PFC_ASSERT_INT(pfccmd_switchuser(pfcd_options, PFC_FALSE,
						 fatal), 0);
		if (ctrl_debug > 0) {
			debug_printf(1, "uid=%d, gid=%d", getuid(), getgid());
		}
		cred_switched = PFC_TRUE;
	}

	debug_printf(1, "IPC service = %u, timeout = %u",
		     service, ctrl_timeout);

	err = pfc_ipcclnt_sess_create(&sess, PFCD_IPC_SERVICE, service);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to create IPC client session: %s",
		      strerror(err));

		return err;
	}

	ts.tv_sec = ctrl_timeout;
	ts.tv_nsec = 0;
	err = pfc_ipcclnt_sess_settimeout(sess, &ts);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to set IPC session timeout: %s", strerror(err));
		PFC_ASSERT_INT(pfc_ipcclnt_sess_destroy(sess), 0);

		return err;
	}

	*sessp = sess;

	return 0;
}

/*
 * void
 * ipc_invocation_error(int err)
 *	Print an error message that represents the cause of IPC service
 *	invocation error.
 *
 *	`err' must be an error number returned by the call of
 *	pfc_ipcsess_invoke().
 */
void
ipc_invocation_error(int err)
{
	if (err == ECONNREFUSED) {
		not_running_ipc(pfcd_channel_addr);
	}
	else if (err == EPERM || err == EACCES) {
		not_allowed();
	}
	else {
		const char	*msg = "Failed to invoke IPC service";
		const char	*chaddr = pfcd_channel_addr;
		const char	*errmsg = strerror(err);

		if (chaddr == NULL) {
			error("%s: %s", msg, errmsg);
		}
		else {
			error("%s at \"%s\": %s", msg, chaddr, errmsg);
		}
	}
}
