/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_JNI_JNI_IMPL_H
#define	_PFC_LIBPFC_JNI_JNI_IMPL_H

/*
 * Internal definitions for PFC-Core JNI helper library.
 */

#include <pfc/base.h>

/*
 * Prototype of logging configuration hook.
 */
typedef void	(*pjni_loghook_t)(pfc_bool_t enabled);

/*
 * Prototypes.
 */
extern void	pjni_log_enable(pfc_bool_t enable);
extern int	pjni_log_hook_install(pjni_loghook_t hook);
extern void	pjni_log_hook_uninstall(pjni_loghook_t hook);

/*
 * Internal prototypes.
 */
#ifdef	_PFC_LIBPFC_JNI_BUILD
extern void	pjni_log_error(const char *format, ...)
	PFC_FATTR_PRINTFLIKE(1, 2);
#endif	/* _PFC_LIBPFC_JNI_BUILD */

#endif	/* !_PFC_LIBPFC_JNI_JNI_IMPL_H */
