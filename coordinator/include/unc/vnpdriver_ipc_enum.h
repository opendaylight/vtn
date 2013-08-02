/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef IPC_VNPDRV_IPC_ENUM_H_
#define IPC_VNPDRV_IPC_ENUM_H_

#include "unc/base.h"

/* enum for vnpdrv_val_vtunnel structure */
typedef enum {
  VNPDRV_IDX_LABEL_VTNL = 0
} vnpdrv_val_vtunnel_index_t;

/* enum for vnpdrv_val_vtunnel_if structure */
typedef enum {
  VNPDRV_IDX_LABEL_VTNL_IF = 0,
  VNPDRV_IDX_VLAN_ID_VTNL_IF
} vnpdrv_val_vtunnel_if_index_t;

#endif
