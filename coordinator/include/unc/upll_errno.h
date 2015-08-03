/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_ERRNO_H_
#define UNC_UPLL_ERRNO_H_

#include "unc/unc_base.h"
#include "unc/base.h"

UNC_C_BEGIN_DECL

/* Error numbers returned by UPLL Service */
typedef enum {
  /* Request successfully processed */
  UPLL_RC_SUCCESS = UNC_RC_SUCCESS,
  /* Generic error */
  UPLL_RC_ERR_GENERIC = UNC_UPLL_RC_ERR_GENERIC,
  /* The request message format is bad */
  UPLL_RC_ERR_BAD_REQUEST = UNC_UPLL_RC_ERR_BAD_REQUEST,
  /* The given session does not have the config lock */
  UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID =
              UNC_UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID,
  /* Not a valid operation */
  UPLL_RC_ERR_NO_SUCH_OPERATION = UNC_UPLL_RC_ERR_NO_SUCH_OPERATION,
  /* Not a valid option1 */
  UPLL_RC_ERR_INVALID_OPTION1 = UNC_UPLL_RC_ERR_INVALID_OPTION1,
  /* Not a valid option2 */
  UPLL_RC_ERR_INVALID_OPTION2 = UNC_UPLL_RC_ERR_INVALID_OPTION2,
  /* Syntax check failed */
  UPLL_RC_ERR_CFG_SYNTAX = UNC_UPLL_RC_ERR_CFG_SYNTAX,
  /* Semantic check failed */
  UPLL_RC_ERR_CFG_SEMANTIC = UNC_UPLL_RC_ERR_CFG_SEMANTIC,
  /* Resource (DBMS, Physical, Driver) is disconnected */
  UPLL_RC_ERR_RESOURCE_DISCONNECTED = UNC_UPLL_RC_ERR_RESOURCE_DISCONNECTED,
  /* DBMS access (read / write / transacation) failure */
  UPLL_RC_ERR_DB_ACCESS = UNC_UPLL_RC_ERR_DB_ACCESS,
  /* Instance specified by key does not exist */
  UPLL_RC_ERR_NO_SUCH_INSTANCE = UNC_UPLL_RC_ERR_NO_SUCH_INSTANCE,
  /* The specified keytype is unknown */
  UPLL_RC_ERR_NO_SUCH_NAME = UNC_UPLL_RC_ERR_NO_SUCH_NAME,
  /* The specified datatype is unknown */
  UPLL_RC_ERR_NO_SUCH_DATATYPE = UNC_UPLL_RC_ERR_NO_SUCH_DATATYPE,
  /* The operation not supported by controller */
  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR = UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
  /* The operation not supported by standby UPLL */
  UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY =
              UNC_UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY,
  /* For creating the given keytype instance, its parent does not exist */
  UPLL_RC_ERR_PARENT_DOES_NOT_EXIST = UNC_UPLL_RC_ERR_PARENT_DOES_NOT_EXIST,
  /* The given keytype instance cannot be created because it already exists */
  UPLL_RC_ERR_INSTANCE_EXISTS = UNC_UPLL_RC_ERR_INSTANCE_EXISTS,
  /* Not allowed for this datatype */
  UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT = UNC_UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,
  /* Not allowed for this KT */
  UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT = UNC_UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT,
  /* Not allowed for at this time */
  UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME =
              UNC_UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME,
  /* The given operation exceeds the resource limit */
  UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT = UNC_UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT,
  /* Merge failed as there is a merge conflict */
  UPLL_RC_ERR_MERGE_CONFLICT = UNC_UPLL_RC_ERR_MERGE_CONFLICT,
  /* The operation could not be performed because there are uncommited changes
   * in the candidate configuration */
  UPLL_RC_ERR_CANDIDATE_IS_DIRTY = UNC_UPLL_RC_ERR_CANDIDATE_IS_DIRTY,
  /* UPLL daemon is shutting down and cannot process the request */
  UPLL_RC_ERR_SHUTTING_DOWN = UNC_UPLL_RC_ERR_SHUTTING_DOWN,
  /* Controller disconnected error */
  UPLL_RC_ERR_CTR_DISCONNECTED = UNC_RC_CTR_DISCONNECTED,
  /* Driver not present */
  UPLL_RC_ERR_DRIVER_NOT_PRESENT = UNC_RC_ERR_DRIVER_NOT_PRESENT,
  /* Audit is cancelled */
  UPLL_RC_ERR_AUDIT_CANCELLED = UNC_RC_REQUEST_CANCELLED,
  /* Unified-network/boundary configuration is incomplete */
  UPLL_RC_ERR_EXPAND = UNC_UPLL_RC_ERR_EXPAND,
  /* Specified logical port and VLAN ID is already in use */
  UPLL_RC_ERR_VLAN_IN_USE = UNC_UPLL_RC_ERR_VLAN_IN_USE,
  /* Mismatch in VLAN type */
  UPLL_RC_ERR_VLAN_TYPE_MISMATCH = UNC_UPLL_RC_ERR_VLAN_TYPE_MISMATCH,
  /* User configured name conflicts with internally created name */
  UPLL_RC_ERR_NAME_CONFLICT = UNC_UPLL_RC_ERR_NAME_CONFLICT,
  /* Specified domain is invalid */
  UPLL_RC_ERR_INVALID_DOMAIN = UNC_UPLL_RC_ERR_INVALID_DOMAIN,
  /* Specified destination address is invalid */
  UPLL_RC_ERR_INVALID_DEST_ADDR = UNC_UPLL_RC_ERR_INVALID_DEST_ADDR,
  /* Specified next-hop address is invalid */
  UPLL_RC_ERR_INVALID_NEXTHOP_ADDR = UNC_UPLL_RC_ERR_INVALID_NEXTHOP_ADDR,
  /* Specified network-group name does not exist */
  UPLL_RC_ERR_INVALID_NWMONGRP = UNC_UPLL_RC_ERR_INVALID_NWMONGRP
} upll_rc_t;

UNC_C_END_DECL

#endif  /* UNC_UPLL_ERRNO_H_ */
