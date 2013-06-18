/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_PFCD_CONF_H
#define	_PFC_PFCD_CONF_H

/*
 * Constants used by pfcd.cfdef.
 *
 * Remarks:
 *	This file is included by libpfc_util/pfcd.cfdef.
 *	Never define C or C++ language specific symbols.
 */

#include <pfc/base.h>

/*
 * psdump block.
 */
#define	PFCD_CONF_PSDUMP_INTERVAL_MIN		PFC_CONST_U(10)
#define	PFCD_CONF_PSDUMP_INTERVAL_MAX		PFC_CONST_U(86400)
#define	PFCD_CONF_PSDUMP_SIZE_MIN		PFC_CONST_U(0)
#define	PFCD_CONF_PSDUMP_SIZE_MAX		PFC_CONST_U(0x10000000)
#define	PFCD_CONF_PSDUMP_ROTATE_MIN		PFC_CONST_U(0)
#define	PFCD_CONF_PSDUMP_ROTATE_MAX		PFC_CONST_U(100)

/*
 * modmap block.
 */
#define	PFCD_CONF_MODMAP_SIZE_MIN		PFC_CONST_U(0x10000)
#define	PFCD_CONF_MODMAP_SIZE_MAX		PFC_CONST_U(0x10000000)
#define	PFCD_CONF_MODMAP_ROTATE_MIN		PFC_CONST_U(0)
#define	PFCD_CONF_MODMAP_ROTATE_MAX		PFC_CONST_U(100)

/*
 * lldp block.
 */
#define	PFCD_CONF_LLDP_ETHTYPE_MIN		PFC_CONST_U(0x05dd)
#define	PFCD_CONF_LLDP_ETHTYPE_MAX		PFC_CONST_U(0xffff)

/*
 * ipc_event block.
 */
#define	PFCD_CONF_IPC_EVENT_IDLE_TIMEOUT_MIN	PFC_CONST_U(10)
#define	PFCD_CONF_IPC_EVENT_IDLE_TIMEOUT_MAX	PFC_CONST_U(86400000)
#define	PFCD_CONF_IPC_EVENT_MAXTHREADS_MIN	PFC_CONST_U(1)
#define	PFCD_CONF_IPC_EVENT_MAXTHREADS_MAX	PFC_CONST_U(1024)
#define	PFCD_CONF_IPC_EVENT_CONN_INTERVAL_MIN	PFC_CONST_U(1000)
#define	PFCD_CONF_IPC_EVENT_CONN_INTERVAL_MAX	PFC_CONST_U(86400000)
#define	PFCD_CONF_IPC_EVENT_KEEP_INTERVAL_MIN	PFC_CONST_U(1000)
#define	PFCD_CONF_IPC_EVENT_KEEP_INTERVAL_MAX	PFC_CONST_U(86400000)
#define	PFCD_CONF_IPC_EVENT_KEEP_TIMEOUT_MIN	PFC_CONST_U(1000)
#define	PFCD_CONF_IPC_EVENT_KEEP_TIMEOUT_MAX	PFC_CONST_U(3600000)
#define	PFCD_CONF_IPC_EVENT_TIMEOUT_MIN		PFC_CONST_U(1000)
#define	PFCD_CONF_IPC_EVENT_TIMEOUT_MAX		PFC_CONST_U(3600000)

#endif	/* !_PFC_PFCD_CONF_H */
