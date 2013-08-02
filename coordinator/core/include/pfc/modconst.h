/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_MODCONST_H
#define	_PFC_MODCONST_H

/*
 * Constants and macros related to PFC module management.
 */

#include <string.h>
#include <pfc/base.h>

/*
 * Current version of PFC module management system.
 */
#define	PFC_MODULE_SYSTEM_VERSION	3

/*
 * Maximum number of modules.
 */
#define	PFC_MODULE_MAX			PFC_CONST_U(255)

/*
 * Maximum length of module name.
 */
#define	PFC_MODULE_NAME_MAX		PFC_CONST_U(31)

/*
 * String length enough to store module related file name.
 */
#define	PFC_MODULE_FNAME_MAX		(PFC_MODULE_NAME_MAX + 8)

/*
 * Module name prefix which represents the system internal component.
 */
#define	PFC_MODULE_CORE_PREFIX		"core."
#define	PFC_MODULE_CORE_PREFIX_LEN	PFC_CONST_U(5)

#if	defined(_PFC_CORE_LIBS_BUILD) && defined(PFC_LOG_IDENT)

/*
 * Pseudo module name which represents the system internal component.
 */
#define	PFC_MODULE_CORE_THIS		PFC_MODULE_CORE_PREFIX PFC_LOG_IDENT

#endif	/* !defined(_PFC_CORE_LIBS_BUILD) && defined(PFC_LOG_IDENT) */

/*
 * Determine whether the module name is pseudo module name which represents
 * the PFC core component. A non NULL string must be specified to `name'.
 */
#define	PFC_MODULE_IS_CORE(name)					\
	(strncmp((name), PFC_MODULE_CORE_PREFIX,			\
		 PFC_MODULE_CORE_PREFIX_LEN) == 0)

#endif	/* !_PFC_MODCONST_H */
