/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __HANDLER_HH__
#define __HANDLER_HH__

#include <pfc/debug.h>
#include <unc/keytype.h>

namespace unc {
namespace driver {
#define INPUT_KEY_STRUCT_INDEX 10
#define INPUT_VAL_STRUCT_INDEX 11
typedef struct {
  uint32_t session_id;  // Client session ID.
  uint32_t config_id;  // Configuration ID.
  uint32_t operation;  // Operation.
  char domain_id[32];  // Domain Id.
  uint32_t max_rep_count;  // Max Repetion Count.
  uint32_t option1;  // Option1.
  uint32_t option2;  // Option2.
  uint32_t data_type;  // Data type.
} odl_drv_common_header_t;

typedef struct {
  odl_drv_common_header_t header;
  unc_key_type_t key_type;  // Key type.
  char controller_name[32];  // Controller Name.
  uint8_t version[32];  // Version.
} odl_drv_request_header_t;

typedef struct {
  odl_drv_common_header_t header;
  char controller_name[32];  // Controller Name.
  uint32_t result;  // Result code.
  uint32_t key_type;  // Key type.
} odl_drv_response_header_t;

typedef enum {
  IPC_SESSION_ID_INDEX = 0,  /* IPC session_id */
  IPC_CONFIG_ID_INDEX,      /*  IPC config_id=1 */
  IPC_CONTROLLER_ID_INDEX, /* IPC Controller_id=2 */
  IPC_DOMAIN_ID_INDEX,    /* IPC Doamin_id=3 */
  IPC_OPERATION_INDEX,   /* IPC operation index=4*/
  IPC_MAX_REP_COUNT_INDEX,
  IPC_OPTION1_INDEX,    /* option1=5 */
  IPC_OPTION2_INDEX,   /* option2=6 */
  IPC_DATA_TYPE_INDEX, /* datatype=7 */
  IPC_KEY_TYPE_INDEX,  /* keytype=8 */
} request_header_t;
}  // namespace driver
}  // namespace unc

#endif

