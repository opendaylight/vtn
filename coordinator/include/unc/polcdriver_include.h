/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   POLC Driver common definition.
 * @file    polcdriver_include.h
 * @author  HCL
 * @date    2014
 * @version SYSTEM:UNC 1.0, MODULE:POLCDriver
 *
 */

#ifndef _POLCDRIVER_INCLUDE_H_
#define _POLCDRIVER_INCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "unc_events.h"

#define POLCDRIVER_CHANNEL_NAME         "drvpolcd"    /* Channel Name for POLC Driver */
#define POLCDRIVER_SERVICE_NAME         "polcdriver"  /* Service Name for normal IPC operation like request/response for POLC Driver */
#define POLCDRIVER_EVENT_SERVICE_NAME   "polcdriver"  /* Event Service Name for POLC Driver */

typedef enum {
  POLCDRV_SVID_LOGICAL = 0,              /* Logical Service ID  */
  POLCDRV_SVID_PHYSICAL                  /* Physical Service ID */
} PolcdrvSvid;

#ifdef __cplusplus
}  /* End of extern "C" */
#endif  /* __cplusplus */

#endif
