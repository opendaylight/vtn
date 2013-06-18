/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_LNC_IPC_H
#define	_UNC_LNC_IPC_H

/*
 * Definitions for IPC services provided by UNC daemon launcher.
 */

#include <unc/base.h>
#include <unc/lnc_types.h>

UNC_C_BEGIN_DECL

/*
 * IPC service name provided by the launcher module.
 */
#define	LNC_IPC_SERVICE		"launcher"

/*
 * The number of IPC services provided by the launcher daemon.
 */
#define	LNC_IPC_NSERVICES	PFC_CONST_U(7)

/*
 * IPC services provided by the launcher module.
 */

/*
 * LNC_IPC_SVID_NOTIFY - Notify that the service is ready.
 *
 * Input:
 *	If the daemon is implemented as IPC server, the name of the IPC channel
 *	provided by the daemon must be set to the additional data array at
 *	index 0 as STRING data.
 *
 * Output:
 *	Upon successful completion, zero is returned.
 *	Otherwise PFC_IPCRESP_FATAL is returend.
 *	This service does not set any additional data.
 */
#define	LNC_IPC_SVID_NOTIFY		PFC_CONST_U(0)

/*
 * LNC_IPC_SVID_CLSTATE - Get current cluster node state.
 *
 * Input:
 *	This IPC service does not require additional data.
 *
 * Output:
 *	Upon successful completion, a value of lnc_clstate_t is returned.
 *
 *	This service sets one UINT8 additional data which represents status of
 *	cluster system. PFC_TRUE (1) is set if the cluster system is active.
 *	Otherwise PFC_FALSE (0) is set.
 *
 *	PFC_IPCRESP_FATAL is returned on error.
 */
#define	LNC_IPC_SVID_CLSTATE		PFC_CONST_U(1)

/*
 * LNC_IPC_SVID_CLEVACK - Send acknowledgement of LNC_IPCEVENT_CLEVENT.
 *
 * Input:
 *	This IPC service requires one UINT8 additional data which represents
 *	the result of cluster state transition.
 *	PFC_TRUE (1) must be set if the cluster state has been changed
 *	successfully. Otherwise PFC_FALSE (0) must be set.
 *
 * Output:
 *	Upon successful completion, zero is returned.
 *	Otherwise PFC_IPCRESP_FATAL is returned.
 *
 *	This IPC service set no additional data.
 */
#define	LNC_IPC_SVID_CLEVACK		PFC_CONST_U(2)

/*
 * Remarks:
 *	Below are non-public services.
 */

/*
 * LNC_IPC_SVID_STATUS - Get daemon status.
 *
 * Input:
 *	This IPC service takes variable-sized STRING aditional data.
 *	Each STRING data is considered as the daemon name to be queried.
 *	If no additional data is sent, this service returns status of all
 *	daemons.
 *
 * Output:
 *	Upon successful completion, LNC_IPC_STATUS_OK is returned.
 *	In this case, one or more lncipc_dmstat_t, which contains daemon
 *	statis, are set to additional data.
 *
 *	On error, LNC_IPC_STATUS_ERROR is returned. In this case, an error
 *	message STRING data is set to additional data at index zero.
 *
 *	PFC_IPCRESP_FATAL is returned if the server could not set
 *	lncipc_dmstat_t to additional data.
 */
#define	LNC_IPC_SVID_STATUS		PFC_CONST_U(3)

#define	LNC_IPC_STATUS_OK		(0)
#define	LNC_IPC_STATUS_ERROR		(1)

/*
 * LNC_IPC_SVID_VSTATUS - Get verbose daemon status.
 *
 * Input:
 *	This IPC service takes variable-sized STRING aditional data.
 *	Each STRING data is considered as the daemon name to be queried.
 *	If no additional data is sent, this service returns status of all
 *	daemons.
 *
 * Output:
 *	Upon successful completion, LNC_IPC_STATUS_OK is returned.
 *	In this case, variable-sized data per a daemon is set to additional
 *	data array:
 *
 *	data[N]: lncipc_dmstat_t
 *	    Daemon information
 *	data[N + 1]: STRING
 *	    Configuration file path
 *	data[N + 2]: STRING
 *	    Brief description about the daemon	
 *	data[N + 3]: STRING
 *	    Contents of stderr log file.
 *	    NULL is set if stderr log file is empty, or it is disabled by
 *	    configuration.
 *	data[N + 4]: UINT64
 *	    Process start time (tv_sec)
 *	data[N + 5]: UINT64
 *	    Process start time (tv_nsec)
 *	data[N + 6]: UINT32 (argc)
 *	    The number of process arguments (including argv[0])
 *	data[N + 7] .. data[N + 7 + (argc - 1)]: UINT32
 *	    Process name and arguments.
 *
 *	On error, LNC_IPC_STATUS_ERROR is returned. In this case, an error
 *	message STRING data is set to additional data at index zero.
 *
 *	PFC_IPCRESP_FATAL is returned if the server could not set
 *	lncipc_dmstat_t to additional data.
 */
#define	LNC_IPC_SVID_VSTATUS		PFC_CONST_U(4)

/*
 * LNC_IPC_SVID_CLEVENT - Raise a cluster state change event.
 *
 * Input:
 *	This IPC service requires at least one UINT8 additional data which
 *	represents type of cluster state event. Cluster event type is defined
 *	as CLSTAT_EVTYPE_XXX in unc/clstat_types.h.
 *
 *	One more UINT32 additional data can be added. It is considered as
 *	timeout in seconds. Default timeout is 30 seconds.
 *
 * Output:
 *	An response code for CLEVENT service (LNC_IPC_CLEVENT_XXX) is returned.
 *	This service does not set any additional data.
 */
#define	LNC_IPC_SVID_CLEVENT		PFC_CONST_U(5)
#define	LNC_IPC_CLEVENT_OK		(0)	/* succeeded */
#define	LNC_IPC_CLEVENT_BUSY		(1)	/* already in transition */
#define	LNC_IPC_CLEVENT_UNCHANGED	(2)	/* state is unchanged */
#define	LNC_IPC_CLEVENT_INVALID		(3)	/* argument is invalid */
#define	LNC_IPC_CLEVENT_ERRSTATE	(4)	/* cluster is in error state */
#define	LNC_IPC_CLEVENT_TC_BUSY		(5)	/* TC lock is busy */
#define	LNC_IPC_CLEVENT_TC_FAILED	(6)	/* failed to acquire TC lock */
#define	LNC_IPC_CLEVENT_FAILED		(7)	/* failed due to error */

/*
 * LNC_IPC_SVID_LNCSTAT - Get status information about the launcher daemon.
 *
 * Input:
 *	This IPC service does not require additional data.
 *
 * Output:
 *	This IPC service returns a bitwise OR-ed flag value, without setting
 *	any additional data.
 */
#define	LNC_IPC_SVID_LNCSTAT		PFC_CONST_U(6)

/*
 * Return value of LNC_IPC_SVID_LNCSTAT service.
 */

/* All daemons have already been launched. */
#define	LNC_IPC_LNCSTAT_INIT		PFC_CONST_U(0x1)

/*
 * The launcher module will be finalized soon, or has already been
 * finalized.
 */
#define	LNC_IPC_LNCSTAT_FINI		PFC_CONST_U(0x2)

/*
 * The cluster system is active.
 */
#define	LNC_IPC_LNCSTAT_SYSACT		PFC_CONST_U(0x4)

/*
 * The UNC daemon service has been started.
 */
#define	LNC_IPC_LNCSTAT_STARTED		PFC_CONST_U(0x8)

#if	LNC_IPC_NSERVICES != (LNC_IPC_SVID_LNCSTAT + 1)
#error	LNC_IPC_NSERVICES value is inconsistent.
#endif	/* LNC_IPC_NSERVICES != (LNC_IPC_SVID_LNCSTAT + 1) */

/*
 * IPC event types generated by the launcher module.
 */

/*
 * LNC_IPCEVENT_CLEVENT - Notify the start of cluster state transition.
 *
 * This event notifies the start of cluster state transition.
 * If a child process launched by the launcher module receives this event,
 * it must issue LNC_IPC_SVID_CLEVACK IPC service after completion of
 * cluster state transition.
 *
 * This event contains the following additional data.
 *
 * Index 0: UINT8
 *	PFC event type which represents new cluster state.
 *	CLSTAT_EVTYPE_ACT is set when the cluster node becomes active.
 *
 * Index 1: UINT8
 *	The state of cluster system.
 *	PFC_TRUE (1) is set if the cluster system is already active.
 *	PFC_FALSE (0) is set if the cluster system is not yet active.
 *
 * Index 2, 3: UINT64
 *	pfc_timespec_t value (monotinic clock) which represents deadline of
 *	cluster state transition. UINT64 value at index 2 is tv_sec value,
 *	and index 3 is tv_nsec value.
 */
#define	LNC_IPCEVENT_CLEVENT		PFC_CONST_U(0)

UNC_C_END_DECL

#endif	/* !_UNC_LNC_IPC_H */
