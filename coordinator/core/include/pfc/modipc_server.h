/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_MODIPC_SERVER_H
#define	_PFC_MODIPC_SERVER_H

/*
 * PFC module-specific definitions of IPC server APIs.
 */

#include <pfc/moddefs.h>
#include <pfc/ipc_server.h>

PFC_C_BEGIN_DECL

#ifdef	PFC_MODULE_BUILD

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_module_ipcevent_create(pfc_ipcsrv_t **srvp, pfc_ipcevtype_t type)
 *	Create a new IPC event.
 *
 *	`type' must is an IPC event type to be assigned to a new IPC event.
 *	The module name is used as the IPC service name of the event.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to IPC service session
 *	for sending an IPC event is set to the buffer pointed by `srvp', and
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_module_ipcevent_create(pfc_ipcsrv_t **srvp, pfc_ipcevtype_t type)
{
	extern int	pfc_ipcsrv_event_create(pfc_ipcsrv_t **PFC_RESTRICT s,
						const char *PFC_RESTRICT name,
						pfc_ipcevtype_t type);

	return pfc_ipcsrv_event_create(srvp, PFC_MODULE_THIS_NAME, type);
}

#endif	/* PFC_MODULE_BUILD */

PFC_C_END_DECL

#endif	/* !_PFC_MODIPC_SERVER_H */
