/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   VNP Driver common definition.
 * @file    vnpdriver_include.h
 * @author  HCL
 * @date    2013
 * @version SYSTEM:UNC 1.0, MODULE:VNPDriver
 *
 */

#ifndef _VNPDRIVER_INCLUDE_H_
#define _VNPDRIVER_INCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "unc_events.h"

#define VNPDRIVER_CHANNEL_NAME         "drvvnpd"    /* Channel Name for VNP Driver */
#define VNPDRIVER_SERVICE_NAME         "vnpdriver"  /* Service Name for normal IPC operation like request/response for VNP Driver */
#define VNPDRIVER_EVENT_SERVICE_NAME   "vnpdriver"  /* Event Service Name for VNP Driver */

typedef enum {
  VNPDRV_SVID_LOGICAL = 0,              /* Logical Service ID  */
  VNPDRV_SVID_PHYSICAL                  /* Physical Service ID */
} VnpdrvSvid;

#ifdef __cplusplus
}  /* End of extern "C" */
#endif  /* __cplusplus */

#endif
