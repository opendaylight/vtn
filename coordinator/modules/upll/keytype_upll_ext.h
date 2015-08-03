/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef SRC_MODULES_UPLL_KEYTYPE_UPLL_EXT_H_
#define SRC_MODULES_UPLL_KEYTYPE_UPLL_EXT_H_

#include <unc/base.h>
#include <unc/keytype.h>

UNC_C_BEGIN_DECL

// Extending unc_keytype_datatype_t.
typedef enum {
  UPLL_DT_INVALID   = UNC_DT_INVALID,
  UPLL_DT_STATE     = UNC_DT_STATE,      // Entity database
  UPLL_DT_CANDIDATE = UNC_DT_CANDIDATE,  // Candidate configuration
  UPLL_DT_RUNNING   = UNC_DT_RUNNING,    // Running configuration
  UPLL_DT_STARTUP   = UNC_DT_STARTUP,    // Startup configuration
  UPLL_DT_IMPORT    = UNC_DT_IMPORT,     // Import configuration
  UPLL_DT_AUDIT     = (UNC_DT_STATE + UNC_DT_CANDIDATE +
                       UNC_DT_RUNNING + UNC_DT_STARTUP + UNC_DT_IMPORT),
  UPLL_DT_CANDIDATE_DEL,
  UPLL_DT_SYSTEM
} upll_keytype_datatype_t;

UNC_C_END_DECL

#endif  // SRC_MODULES_UPLL_KEYTYPE_UPLL_EXT_H_
