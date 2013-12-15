/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef _UT_STUB_H_
#define _UT_STUB_H_

/*
 * Include stub header files.
 */

#include "stub/include/core_include/pfc/module.h"
#include "stub/include/core_include/pfc/ipc_client.h"

#ifdef  __cplusplus
#include "stub/include/cxx/pfcxx/ipc_server.hh"
#include "stub/include/cxx/pfcxx/ipc_client.hh"
#include "stub/include/cxx/pfcxx/module.hh"
#include "stub/tclib_module/tclib_module.hh"
#include "stub/restjsonutil/rest_util.hh"
#include "stub/restjsonutil/json_build_parse.hh"
#include "stub/restjsonutil/json_type_util.hh"
#include "stub/restjsonutil/rest_common_defs.hh"
#include "stub/vtndrvintf/vtn_drv_module.hh"
#endif  /*cplusplus */

#endif  // _UT_STUB_H_
