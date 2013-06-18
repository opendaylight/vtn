/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_MODEVENT_IMPL_H
#define	_PFC_LIBPFC_MODEVENT_IMPL_H

/*
 * Internal definitions for module-specific event feature.
 */

#include <pfc/modevent.h>

/*
 * Prototypes.
 */
extern int	pfc_modevent_post_ex(const char *target, pfc_evtype_t type);

#endif	/* !_PFC_LIBPFC_MODEVENT_IMPL_H */
