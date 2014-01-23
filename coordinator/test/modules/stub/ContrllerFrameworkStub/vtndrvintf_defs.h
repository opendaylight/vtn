/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _VTNDRVINTF_DEFS_H_
#define _VTNDRVINTF_DEFS_H_

typedef enum {
  VTN_DRV_RET_SUCCESS = 0,
  VTN_DRV_RET_FAILURE
} VtnDrvRetEnum;

namespace unc {
namespace driver {
typedef enum {
  VTN_SWITCH_CREATE = 0,
  VTN_SWITCH_UPDATE,
  VTN_SWITCH_DELETE,
  VTN_LP_CREATE,
  VTN_LP_UPDATE,
  VTN_LP_DELETE,
  VTN_PORT_CREATE,
  VTN_PORT_UPDATE,
  VTN_PORT_DELETE,
  VTN_LINK_CREATE,
  VTN_LINK_UPDATE,
  VTN_LINK_DELETE
} oper_type;
}  // namespace driver
}  // namespace unc
#endif  // _VTNDRVINTF_DEFS_H_

