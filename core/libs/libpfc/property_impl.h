/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_PROPERTY_IMPL_H
#define	_PFC_LIBPFC_PROPERTY_IMPL_H

/*
 * Internal definitions for PFC daemon system property APIs.
 */

#include <stdio.h>
#include <pfc/property.h>

PFC_C_BEGIN_DECL

/*
 * Prototypes.
 */
extern int	pfc_prop_set(const char *key, const char *value);

#ifdef	_PFC_LIBPFC_BUILD

/*
 * Internal prototypes.
 */
extern void	pfc_prop_fini(void);

#endif	/* _PFC_LIBPFC_BUILD */

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_EVENT_IMPL_H */
