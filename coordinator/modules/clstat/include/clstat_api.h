/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_MODULE_CLSTAT_INC_CLSTAT_API_H
#define	_UNC_MODULE_CLSTAT_INC_CLSTAT_API_H

/*
 * Definitions for APIs provided by the clstat module.
 */

#include <unc/clstat_types.h>
#include <pfc/module.h>
#include <pfc/event.h>
#include <pfc/clock.h>

UNC_C_BEGIN_DECL

/*
 * Prototypes.
 */
extern const char	*clstat_event_getsource(void);
extern pfc_bool_t	clstat_event_isactive(pfc_event_t event);
extern int		clstat_event_getdeadline(pfc_event_t event,
						 pfc_timespec_t *tsp);

UNC_C_END_DECL

#endif	/* !_UNC_MODULE_CLSTAT_INC_CLSTAT_API_H */
