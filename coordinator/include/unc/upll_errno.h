/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_ERRNO_H_
#define UNC_UPLL_ERRNO_H_

#include "base.h"

UNC_C_BEGIN_DECL

/* Error numbers returned by UPLL Service */
typedef enum {
  UPLL_RC_SUCCESS = 0,                   /* Request successfully processed */
  UPLL_RC_ERR_GENERIC,                   /* Generic error */
  UPLL_RC_ERR_BAD_REQUEST,               /* The request message format is bad */
  UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID,  /* The given session does not have the
                                            config lock */
  UPLL_RC_ERR_NO_SUCH_OPERATION,         /* Not a valid operation */
  UPLL_RC_ERR_INVALID_OPTION1,           /* Not a valid option1 */
  UPLL_RC_ERR_INVALID_OPTION2,           /* Not a valid option2 */
  UPLL_RC_ERR_CFG_SYNTAX,                /* Syntax check failed */
  UPLL_RC_ERR_CFG_SEMANTIC,              /* Semantic check failed */
  UPLL_RC_ERR_RESOURCE_DISCONNECTED,     /* Resource (DBMS, Physical, Driver) is
                                            diconnected */
  UPLL_RC_ERR_DB_ACCESS,                 /* DBMS access (read / write /
                                            transacation) failure */
  UPLL_RC_ERR_NO_SUCH_INSTANCE,          /* Instance specified by key does not
                                            exist */
  UPLL_RC_ERR_NO_SUCH_NAME,              /* The specified keytype is unknown */
  UPLL_RC_ERR_NO_SUCH_DATATYPE,          /* The specified datatype is unknown */
  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,    /* The operation not supported by
                                            controller */
  UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY,  /* The operation not supported by
                                            standby UPLL */
  UPLL_RC_ERR_PARENT_DOES_NOT_EXIST,     /* For creating the given keytype
                                            instance, its parent does not
                                            exist */
  UPLL_RC_ERR_INSTANCE_EXISTS,           /* The given keytype instance cannot be
                                            created because it already exists */
  UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT,   /* Not allowed for this datatype */
  UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT,   /* Not allowed for this KT */
  UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME,  /* Not allowed for at this time */
  UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT,    /* The given operation exceeds the
                                            resource limit */
  UPLL_RC_ERR_MERGE_CONFLICT,            /* Merge failed as there is a merge
                                            conflict */
  UPLL_RC_ERR_CANDIDATE_IS_DIRTY,        /* The operation could not be performed
                                            because there are uncommited changes
                                            in the candidate configuration */
  UPLL_RC_ERR_SHUTTING_DOWN              /* UPLL daemon is shutting down and
                                            cannot process the request */
} upll_rc_t;

UNC_C_END_DECL

#endif  /* UNC_UPLL_ERRNO_H_ */
