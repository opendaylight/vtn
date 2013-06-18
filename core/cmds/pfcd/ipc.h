/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFCD_IPC_H
#define	_PFCD_IPC_H

/*
 * Common definitions for IPC service handlers.
 */

#include <pfc/ipc_pfcd.h>
#include <ipcsrv_impl.h>

/*
 * Prototype for IPC service handlers in pfcd.
 */
typedef pfc_ipcresp_t		(*pfcd_ipc_t)(pfc_ipcsrv_t *srv);
typedef const pfcd_ipc_t	pfcd_cipc_t;

/*
 * Prototypes.
 */
extern void	ipc_init(void);
extern void	ipc_start(void);
extern void	ipc_notify_ready(void);
extern int	ipc_post_event(pfc_ipcevtype_t type);
extern void	ipc_shutdown(void);
extern void	ipc_fini(void);

#endif	/* !_PFCD_IPC_H */
