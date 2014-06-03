/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * ipc_client.c - IPC client utilities.
 */

#include <pfc/log.h>
#include "clstat_launcher.h"

/*
 * int
 * clstat_sess_create(clst_ipclist_t *UNC_RESTRICT ilp,
 *		      clst_ipcsess_t **UNC_RESTRICT csessp,
 *		      const char *UNC_RESTRICT channel,
 *		      const char *UNC_RESTRICT name, pfc_ipcid_t service)
 *	Create a new IPC client session, and link it to the given session
 *	list.
 *
 *	This function creates a session-cancellable session.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to clst_ipcsess_t is set to the
 *	buffer pointed by `csessp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
clstat_sess_create(clst_ipclist_t *UNC_RESTRICT ilp,
		   clst_ipcsess_t **UNC_RESTRICT csessp,
		   const char *UNC_RESTRICT channel,
		   const char *UNC_RESTRICT name, pfc_ipcid_t service)
{
	clst_ipcsess_t	*csess;
	pfc_ipcconn_t	conn;
	int		err;

	csess = (clst_ipcsess_t *)malloc(sizeof(*csess));
	if (PFC_EXPECT_FALSE(csess == NULL)) {
		pfc_log_error("Failed to allocate clst_ipcsess_t.");

		return ENOMEM;
	}

	err = pfc_ipcclnt_altopen(channel, &conn);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to create alternative connection: "
			      "channel=%s: %s", channel, strerror(err));
		goto error;
	}

	err = pfc_ipcclnt_sess_altcreate5(&csess->cis_sess, conn, name,
					  service, PFC_IPCSSF_CANCELABLE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to create IPC client session: "
			      "channel=%s, service=%s/%u: %s",
			      channel, name, service, strerror(err));
		goto error_conn;
	}

	csess->cis_conn = conn;

	CLST_IPCLIST_LOCK(ilp);

	if (PFC_EXPECT_FALSE(ilp->cil_disabled)) {
		/* Already finalized. */
		pfc_log_error("IPC client session is no longer available.");
		err = ECANCELED;
		CLST_IPCLIST_UNLOCK(ilp);
		PFC_ASSERT_INT(pfc_ipcclnt_sess_destroy(csess->cis_sess), 0);
		goto error_conn;
	}

	pfc_list_push_tail(&ilp->cil_sessions, &csess->cis_list);
	CLST_IPCLIST_UNLOCK(ilp);

	*csessp = csess;

	return 0;

error_conn:
	PFC_ASSERT_INT(pfc_ipcclnt_altclose(conn), 0);

error:
	free(csess);

	return err;
}

/*
 * void
 * clstat_sess_destroy(clst_ipclist_t *UNC_RESTRICT ilp,
 *		       clst_ipcsess_t *UNC_RESTRICT csess)
 *	Destroy the given IPC client session.
 *
 * Remarks:
 *	The caller must ensure that the given session is no longer used.
 */
void
clstat_sess_destroy(clst_ipclist_t *UNC_RESTRICT ilp,
		    clst_ipcsess_t *UNC_RESTRICT csess)
{
	int	err;

	/* At first, make the specified session invisible to other threads. */
	CLST_IPCLIST_LOCK(ilp);
	pfc_list_remove(&csess->cis_list);
	CLST_IPCLIST_UNLOCK(ilp);

	err = pfc_ipcclnt_sess_destroy(csess->cis_sess);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to destroy IPC client session: %s",
			      strerror(err));
		/* FALLTHROUGH */
	}

	err = pfc_ipcclnt_altclose(csess->cis_conn);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to close IPC connection: %s",
			      strerror(err));
		/* FALLTHROUGH */
	}

	/* Wake up threads waiting for completion of finalization. */
	CLST_IPCLIST_LOCK(ilp);
	if (ilp->cil_disabled && pfc_list_is_empty(&ilp->cil_sessions)) {
		CLST_IPCLIST_BROADCAST(ilp);
	}
	CLST_IPCLIST_UNLOCK(ilp);

	free(csess);
}

/*
 * int
 * clstat_ipclist_fini(clst_ipclist_t *UNC_RESTRICT ilp,
 *		       const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Finalize the given IPC client session list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
clstat_ipclist_fini(clst_ipclist_t *UNC_RESTRICT ilp,
		    const pfc_timespec_t *UNC_RESTRICT abstime)
{
	pfc_list_t	*elem;
	int		err;

	CLST_IPCLIST_LOCK(ilp);

	if (PFC_EXPECT_FALSE(ilp->cil_disabled)) {
		/* Already disabled. */
		err = ECANCELED;
		goto out;
	}

	ilp->cil_disabled = PFC_TRUE;

	/* Cancel all IPC service requests. */
	PFC_LIST_FOREACH(&ilp->cil_sessions, elem) {
		clst_ipcsess_t	*csess = CLST_IPCSESS_LIST2PTR(elem);
		pfc_ipcsess_t	*sess = csess->cis_sess;

		PFC_ASSERT_INT(pfc_ipcclnt_sess_cancel(sess, PFC_TRUE), 0);
	}

	/* Wait for all sessions to be destroyed. */
	err = 0;
	while (!pfc_list_is_empty(&ilp->cil_sessions)) {
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ETIMEDOUT);
			pfc_log_error("At least one IPC service request did "
				      "not complete.");
			break;
		}
		err = CLST_IPCLIST_TIMEDWAIT_ABS(ilp, abstime);
	}

out:
	CLST_IPCLIST_UNLOCK(ilp);

	return err;
}
