/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * clstat.c - clstat module definitions.
 */

#include <unistd.h>
#include <unc/lnc_types.h>
#include <pfc/strtoint.h>
#include <pfc/property.h>
#include "clstat_impl.h"

/*
 * Timeout of module finalization.
 */
#define	CLSTAT_FINI_TIMEOUT		PFC_CONST_U(10)	/* 10 seconds */

/*
 * Process ID of the UNC daemon which is the parent of this process.
 */
pid_t	uncd_pid PFC_ATTR_HIDDEN;

/*
 * static pfc_bool_t
 * clstat_init(void)
 *	Initialize clstat module.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned on successful return.
 *	Otherwise PFC_FALSE is returned.
 */
static pfc_bool_t
clstat_init(void)
{
	const char	*strpid;
	int		err;

	/*
	 * Determine whether this process is launched by the UNC daemon
	 * or not.
	 */
	strpid = pfc_prop_get(LNC_PROP_PID);
	if (strpid != NULL) {
		pid_t		ppid;
		uint32_t	u32;

		/* Determine process ID of the UNC daemon. */
		err = pfc_strtou32(strpid, &u32);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("Invalid PID of the UNC daemon: %s ",
				      strpid);

			return PFC_FALSE;
		}

		uncd_pid = (pid_t)u32;

		ppid = getppid();
		if (PFC_EXPECT_FALSE(ppid != uncd_pid)) {
			pfc_log_error("Unexpected PID of the UNC daemon: %u: "
				      ": parent=%u", uncd_pid, ppid);

			return PFC_FALSE;
		}
	}
	else {
		uncd_pid = (pid_t)0;
	}

	/* Initialize internal event source. */
	err = clst_event_init();
	if (PFC_EXPECT_FALSE(err != 0)) {
		return PFC_FALSE;
	}

	/* Initialize IPC event handler. */
	err = clst_ipc_init();
	if (PFC_EXPECT_FALSE(err != 0)) {
		PFC_ASSERT_INT(clst_event_fini(NULL), 0);

		return PFC_FALSE;
	}

	return PFC_TRUE;
}

/*
 * static pfc_bool_t
 * clstat_fini(void)
 *	Finalize clstat module.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned on successful return.
 *	Otherwise PFC_FALSE is returned.
 */
static pfc_bool_t
clstat_fini(void)
{
	pfc_timespec_t	limit;
	int		err;

	PFC_ASSERT_INT(pfc_clock_gettime(&limit), 0);
	limit.tv_sec += CLSTAT_FINI_TIMEOUT;

	if (!CLST_ON_UNCD()) {
		/* Finalize IPC event handling. */
		err = clst_ipc_fini(&limit);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return PFC_FALSE;
		}
	}

	/* Finalize internal event source. */
	err = clst_event_fini(&limit);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return PFC_FALSE;
	}

	return PFC_TRUE;
}

/* Declare C module. */
PFC_MODULE_DECL(clstat_init, clstat_fini);
