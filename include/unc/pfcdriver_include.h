/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
* @file   pfcdriver_include.h
* @brief  PFC Driver common definition.
* @par History
*  - 2013/01/01 Create.
*  - 2013/03/11 Fix for ticket #35.
*/

#ifndef _PFCDRIVER_INCLUDE_H_
#define _PFCDRIVER_INCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "unc_events.h"
#include "pfcdriver_ipc_enum.h"

#define PFCDRIVER_CHANNEL_NAME         "drvpfcd"    /* Channel Name for PFC Driver */
#define PFCDRIVER_SERVICE_NAME         "pfcdriver"  /* Service Name for normal IPC operation like request/response for PFC Driver */
#define PFCDRIVER_EVENT_SERVICE_NAME   "pfcdriver"  /* Event Service Name for PFC Driver */

typedef enum {
  PFCDRIVER_SVID_LOGICAL = 0,              /* Logical Service ID  */
  PFCDRIVER_SVID_PHYSICAL                  /* Physical Service ID */
} pfcdrv_svid_t;

#ifdef __cplusplus
}  /* End of extern "C" */
#endif  /* __cplusplus */

#endif
