/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_MODDEFS_H
#define	_PFC_MODDEFS_H

/*
 * PFC module-specific definitions.
 */

#include <pfc/base.h>
#include <pfc/modtypes.h>
#include <pfc/modconst.h>

PFC_C_BEGIN_DECL

/*
 * PFC_MODULE_BUILD is defined by the build system only for PFC module build.
 */
#ifdef	PFC_MODULE_BUILD

#ifdef	MODULE_VERSION
#if	MODULE_VERSION < 0 || MODULE_VERSION > 255
#error	Invalid module version is defined.
#endif	/* MODULE_VERSION < 0 || MODULE_VERSION > 255 */
#else	/* !MODULE_VERSION */
#define	MODULE_VERSION		0	/* default module version */
#endif	/* MODULE_VERSION */

/*
 * Module descriptor of the module.
 */
extern pfc_module_t	__pfc_this_module_id;

/*
 * My module name.
 */
extern const char	*__pfc_this_module_name;

#ifdef	MODULE_CFDEF_NAME

/*
 * Configuration file definition for my module.
 */
extern const pfc_cfdef_t	MODULE_CFDEF_NAME;

#define	PFC_MODULE_THIS_CFDEF	(&MODULE_CFDEF_NAME)

#else	/* !MODULE_CFDEF_NAME */

#define	PFC_MODULE_THIS_CFDEF	((const pfc_cfdef_t *)NULL)

#endif	/* MODULE_CFDEF_NAME */

/* Public macro to refer my module properties. */
#define	PFC_MODULE_THIS_ID	__pfc_this_module_id
#define	PFC_MODULE_THIS_NAME	__pfc_this_module_name

/*
 * Declare static module attributes.
 * This macro must be written at the end of variable definitions.
 */
#ifdef	MODULE_CFDEF_FUNC

/*
 * Determine definition of the module configuration file dynamically.
 * MODULE_CFDEF_FUNC must be a function macro which returns a pointer to
 * pfc_cfdef_t without argument.
 */
#define	__PFC_MODULE_ATTR_DECL(var, type, ipchdlr, nipcs)		\
	static pfc_modattr_t	var = {					\
		PFC_MODULE_SYSTEM_VERSION,	/* pma_sysversion */	\
		(type),				/* pma_type */		\
		MODULE_VERSION,			/* pma_version */       \
		0,				/* pma_pad1 */		\
		(nipcs),			/* pma_nipcs */		\
		MODULE_NAME,			/* pma_name */          \
		NULL,				/* pma_cfdef */         \
		(ipchdlr),			/* pma_ipchdlr */	\
		NULL,				/* pma_pad2 */          \
		NULL,				/* pma_pad3 */          \
	};								\
									\
	var.pma_cfdef = MODULE_CFDEF_FUNC()

#else	/* !MODULE_CFDEF_FUNC */

#define	__PFC_MODULE_ATTR_DECL(var, type, ipchdlr, nipcs)		\
	static const pfc_modattr_t	var = {				\
		PFC_MODULE_SYSTEM_VERSION,	/* pma_sysversion */	\
		(type),				/* pma_type */		\
		MODULE_VERSION,			/* pma_version */       \
		0,				/* pma_pad1 */		\
		(nipcs),			/* pma_nipcs */		\
		MODULE_NAME,			/* pma_name */          \
		PFC_MODULE_THIS_CFDEF,		/* pma_cfdef */         \
		(ipchdlr),			/* pma_ipchdlr */	\
		NULL,				/* pma_pad2 */          \
		NULL,				/* pma_pad3 */          \
 	}

#endif	/* MODULE_CFDEF_FUNC */

#ifndef	__cplusplus

/*
 * Declare C module.
 * `ipchdlr' is a pointer to IPC service handler.
 * `nipcs' is the number of IPC services provided by this module.
 * Zero means no IPC service is provided.
 */
#define	PFC_MODULE_IPC_DECL(init, fini, ipchdlr, nipcs)			\
	static void PFC_FATTR_INIT					\
	__pfc_module_init(void)						\
	{								\
		__PFC_MODULE_ATTR_DECL(mattr, PFC_MODTYPE_C, ipchdlr,	\
				       nipcs);				\
									\
		__pfc_this_module_name = mattr.pma_name;		\
		__pfc_this_module_id =					\
			pfc_module_c_register(&mattr, (init), (fini));	\
	}								\
									\
	pfc_module_t	__pfc_this_module_id PFC_ATTR_HIDDEN =		\
		PFC_MODULE_INVALID;					\
	const char	*__pfc_this_module_name PFC_ATTR_HIDDEN

/*
 * Declare C module without IPC service.
 */
#define	PFC_MODULE_DECL(init, fini)			\
	PFC_MODULE_IPC_DECL(init, fini, NULL, 0)

#endif	/* !__cplusplus */

/*
 * Register bootstrap function of the module.
 * Prototype of the given function must be the same as pfc_modboot_t.
 * It must return zero on successful return.
 * An error number defined by errno.h must be returned on failure.
 */
#define	PFC_MODULE_BOOTSTRAP(func)				\
	static void PFC_FATTR_INIT				\
	__pfc_module_bootstrap_register(void)			\
	{							\
		__pfc_module_bootstrap(MODULE_NAME, func);	\
	}

#else	/* !PFC_MODULE_BUILD */

#define	PFC_MODULE_THIS_ID	PFC_MODULE_INVALID
#define	PFC_MODULE_THIS_NAME	NULL

#endif	/* PFC_MODULE_BUILD */

PFC_C_END_DECL

#endif	/* !_PFC_MODDEFS_H */
