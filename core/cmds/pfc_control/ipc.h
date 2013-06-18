/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_CONTROL_IPC_H
#define	_PFC_CONTROL_IPC_H

/*
 * Common utilities for subcommands which use IPC framework.
 */

#include <pfc/ipc_pfcd.h>
#include <pfc/ipc_client.h>

/*
 * Prototypes.
 */
extern void	ipc_init(const char *chaddr);
extern int	ipc_sess_create_pfcd(pfc_ipcsess_t **PFC_RESTRICT sessp,
				     pfc_ipcid_t service);
extern void	ipc_invocation_error(int err);

#endif	/* !_PFC_CONTROL_IPC_H */
