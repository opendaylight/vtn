/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef VTN_COORDINATOR_MODULES_VTNDRVINTF_INCLUDE_AUDIT_KEYTYPES_H_
#define VTN_COORDINATOR_MODULES_VTNDRVINTF_INCLUDE_AUDIT_KEYTYPES_H_

#include <unc/keytype.h>

#define AUDIT_KT_SIZE 4

struct audit_key_type {
  unc_key_type_t key_type;
  unc_key_type_t parent_key_type;
};

audit_key_type audit_key[AUDIT_KT_SIZE] = {
                               {UNC_KT_VTN, UNC_KT_ROOT},
                               {UNC_KT_VBRIDGE, UNC_KT_VTN},
                               {UNC_KT_VBR_IF, UNC_KT_VBRIDGE},
                               {UNC_KT_VBR_VLANMAP, UNC_KT_VBRIDGE}
                             };
#endif  // VTN_COORDINATOR_MODULES_VTNDRVINTF_INCLUDE_AUDIT_KEYTYPES_H_
