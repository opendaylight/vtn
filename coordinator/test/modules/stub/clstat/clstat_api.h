/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UNC_MODULE_CLSTAT_INC_CLSTAT_API_H
#define _UNC_MODULE_CLSTAT_INC_CLSTAT_API_H

/*
 * Definitions for APIs provided by the clstat module.
 */

#include <unc/clstat_types.h>
#include <pfc/module.h>
#include <pfc/event.h>
#include <pfc/clock.h>

UNC_C_BEGIN_DECL

static inline const char *
clstat_event_getsource(void) {
  return "event";
}

static inline pfc_bool_t
clstat_event_isactive(pfc_event_t event) {
  return PFC_TRUE;
}

extern inline int
clstat_event_getdeadline(pfc_event_t event, pfc_timespec_t *tsp) {
  tsp->tv_sec = 0;
  tsp->tv_nsec = 0;
  return 0;
}

UNC_C_END_DECL

#endif  /* !_UNC_MODULE_CLSTAT_INC_CLSTAT_API_H */
