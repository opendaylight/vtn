/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_MODTYPES_H
#define	_PFC_MODTYPES_H

/*
 * Definitions of data types for PFC module management.
 */

#include <pfc/base.h>
#include <pfc/conf.h>
#include <pfc/ipc.h>

PFC_C_BEGIN_DECL

/*
 * Module type.
 */
typedef uint8_t		pfc_modtype_t;

#define	PFC_MODTYPE_C		PFC_CONST_U(0)		/* written in C */
#define	PFC_MODTYPE_CXX		PFC_CONST_U(1)		/* written in C++ */
#define	PFC_MODTYPE_MAX		PFC_MODTYPE_CXX

/*
 * Prototype of module initializer and finalizer.
 * PFC_TRUE must be returned on success, PFC_FALSE on failure.
 */
typedef pfc_bool_t	(*pfc_modfunc_t)(void);

/*
 * Prototype of IPC service handler provided by the PFC module.
 *
 * @srv:	Pointer to IPC service session instance. Argument can be obtain
 *		from this instance, and arbitrary data can be sent to the
 *		client via this instance.
 * @service:	Service identifier specified by the client.
 *
 * Calling/Exit State:
 *	If PFC_IPCRESP_FATAL (-1) is returned by the IPC service handler,
 *	the IPC framework considers as fatal error. In this case, any data
 *	added to the IPC service session is discarded.
 *
 *	A value other than PFC_IPCRESP_FATAL is treated as response of the
 *	IPC service, and it is sent to the client.
 */
typedef pfc_ipcresp_t	(*pfc_modipc_t)(pfc_ipcsrv_t *srv,
					pfc_ipcid_t service);

/*
 * pfc_modattr_t defines the basic attributes of the module.
 * Each module has one pfc_modattr_t, and it will be registered to PFC system
 * using DSO's init function.
 */
typedef struct {
	uint8_t			pma_sysversion;	/* module system version */
	uint8_t			pma_type;	/* module type */
	uint8_t			pma_version;	/* module version */
	uint8_t			pma_pad1;	/* pad for future expansion */
	uint32_t		pma_nipcs;	/* number of IPC services */
	const char		*pma_name;	/* module name */
	const pfc_cfdef_t	*pma_cfdef;	/* config file definition */
	pfc_modipc_t		pma_ipchdlr;	/* IPC service handler (C) */
	void			*pma_pad2;	/* pad for future expansion */
	void			*pma_pad3;	/* pad for future expansion */
} pfc_modattr_t;

/*
 * Prototype of module instance factory function.
 */
typedef void	*(*pfc_modfac_t)(const pfc_modattr_t *mattr);

/*
 * Descriptor of the PFC module.
 */
typedef uint16_t	pfc_module_t;

/*
 * Invalid module handle.
 */
#define	PFC_MODULE_INVALID	((pfc_module_t)-1)

/*
 * Prototype of module bootstrap function.
 */
typedef int	(*pfc_modboot_t)(void);

/*
 * Prototype of module unload hook function.
 * Module unload hook takes a boolean argument.
 * PFC_TRUE is passed if the module is about to be unloaded in the PFC daemon
 * shutdown sequence.
 * PFC_FALSE is passed if the module is about to be unloaded by user request.
 */
typedef void	(*pfc_modhook_t)(pfc_bool_t);

PFC_C_END_DECL

#endif	/* !_PFC_MODTYPES_H */
