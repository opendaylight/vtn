/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * server.c - APIs for IPC service handler.
 */

#include <ipcsrv_impl.h>
#include "ipcclnt_impl.h"

/*
 * int
 * pfc_ipcclnt_forward_fromsrv(pfc_ipcsess_t *PFC_RESTRICT dsess,
 *			       pfc_ipcsrv_t *PFC_RESTRICT ssrv, uint32_t begin,
 *			       uint32_t end)
 *	Forward additional data received from the IPC client to another IPC
 *	client session's output stream.
 *
 *	This function can be used to forward additional data received from
 *	the IPC client to another IP service handler.
 *
 *	`dsess' is the IPC client session to append additional data. Its state
 *	must be READY.
 *	`ssrv' is the IPC server session which keeps additional data received
 *	from the IPC client.
 *
 *	`begin' and `end' specifies additional data in `ssrv' to be forwarded.
 *	`begin' is the inclusive beginning index, and `end' is the exclusive
 *	ending index of additional data. For instance, if `begin' is 3 and
 *	`end' is 10, additional data in `ssrv' at the index from 3 to 9 are
 *	appended to the IPC output stream in `dsess'. Needless to say,
 *	`end' must be greater than `begin'.
 *
 *	If the value specified to `end' is greater than the number of
 *	additional data in `ssrv', it is treated as if the number of
 *	additional data in `ssrv' is specified to `end'.
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
pfc_ipcclnt_forward_fromsrv(pfc_ipcsess_t *PFC_RESTRICT dsess,
			    pfc_ipcsrv_t *PFC_RESTRICT ssrv, uint32_t begin,
			    uint32_t end)
{
	ipc_sstate_t	state;
	int		err;

	if (PFC_EXPECT_FALSE(dsess == NULL || ssrv == NULL)) {
		return EINVAL;
	}

	/* Ensure that the IPC server session is still valid. */
	err = ipcsrv_is_reset(ssrv);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Check the state of the destination client session. */
	IPC_CLSESS_LOCK(dsess);

	state = dsess->icss_state;
	if (PFC_EXPECT_TRUE(state == IPC_SSTATE_READY)) {
		/* Copy additional data in the specified range. */
		err = pfc_ipcstream_copymsg(&dsess->icss_output,
					    &ssrv->isv_args, begin, end);
	}
	else {
		err = pfc_ipcclnt_sess_state_error(state);
	}

	IPC_CLSESS_UNLOCK(dsess);

	return err;
}

/*
 * int
 * pfc_ipcclnt_forward_tosrv(pfc_ipcsrv_t *PFC_RESTRICT dsrv,
 *			     pfc_ipcsess_t *PFC_RESTRICT ssess, uint32_t begin,
 *			     uint32_t end)
 *	Forward additional data received from the IPC server to another IPC
 *	server session's output stream.
 *
 *	This function can be used to forward additional data received from
 *	the IPC server to another IP client as response of IPC service handler.
 *
 *	`dsrv' is the IPC server session to append additional data.
 *	`ssess' is the IPC client session which keeps additional data received
 *	from the IPC server. Its state must be RESULT.
 *
 *	`begin' and `end' specifies additional data in `ssess' to be forwarded.
 *	`begin' is the inclusive beginning index, and `end' is the exclusive
 *	ending index of additional data. For instance, if `begin' is 3 and
 *	`end' is 10, additional data in `ssess' at the index from 3 to 9 are
 *	appended to the IPC output stream in `dsrv'. Needless to say,
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
 *	  returns non-zero value except for EINVAL, the IPC service handler
 *	  associated with `dsrv' should return a fatal error.
 */
int
pfc_ipcclnt_forward_tosrv(pfc_ipcsrv_t *PFC_RESTRICT dsrv,
			  pfc_ipcsess_t *PFC_RESTRICT ssess, uint32_t begin,
			  uint32_t end)
{
	int	err;

	if (PFC_EXPECT_FALSE(dsrv == NULL || ssess == NULL)) {
		return EINVAL;
	}

	/*
	 * Pin additional data in `ssess'.
	 * Note that this must be done without holding the server session
	 * lock for `dsrv', or deadlock may happen.
	 */
	err = pfc_ipcclnt_sess_pinmsg(ssess);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Ensure that the IPC server session is still valid. */
	IPCSRV_LOCK(dsrv);

	if (IPCSRV_IS_RESET(dsrv)) {
		err = ECONNRESET;
	}
	else {
		/* Copy additional data in the specified range. */
		err = pfc_ipcstream_copymsg(&dsrv->isv_output,
					    &ssess->icss_msg, begin, end);
	}

	IPCSRV_UNLOCK(dsrv);
	pfc_ipcclnt_sess_unpinmsg(ssess);

	return err;
}
