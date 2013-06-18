/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
* @file   unc_base.h
* @brief  Driver common definition.
* @par History
*  - 2013/01/01 Create.
*  - 2013/03/11 Fix for ticket #35.
*/

#ifndef _UNC_BASE_H_
#define _UNC_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */
#if 0
#include "unc_events.h"
#include "pfcdriver_ipc_enum.h"
#endif
/**
* @brief  PFC Driver API response code returned
*/

typedef enum {
  DRVAPI_RESPONSE_SUCCESS = 0,              /* successful               */
  DRVAPI_RESPONSE_FAILURE,                  /* failure                  */
  DRVAPI_RESPONSE_CTRLAPI_FAILURE,          /* Driver api failed       */
  DRVAPI_RESPONSE_NOT_RUNNING,              /* not running              */
  DRVAPI_RESPONSE_INVALID_REQUEST_FORMAT,   /* invalid request format   */
  DRVAPI_RESPONSE_INVALID_SESSION_ID,       /* invalid session id       */
  DRVAPI_RESPONSE_INVALID_CONFIG_ID,        /* invalid config id        */
  DRVAPI_RESPONSE_INVALID_OPERATION,        /* invalid operation        */
  DRVAPI_RESPONSE_INVALID_OPTION1,          /* invalid option1          */
  DRVAPI_RESPONSE_INVALID_OPTION2,          /* invalid option2          */
  DRVAPI_RESPONSE_INVALID_DATATYPE,         /* invalid data type        */
  DRVAPI_RESPONSE_INVALID_KEYTYPE,          /* invalid key type         */
  DRVAPI_RESPONSE_MISSING_KEY_STRUCT,       /* missing key structure    */
  DRVAPI_RESPONSE_MISSING_VAL_STRUCT,       /* missing value structure  */
  DRVAPI_RESPONSE_CONTROLLER_DISCONNECTED,  /* controller disconnected  */
  DRVAPI_RESPONSE_NOT_SENT_TO_CONTROLLER,   /* not sent to controller   */
  DRVAPI_RESPONSE_NO_SUCH_INSTANCE,         /* READ has no result       */
  DRVAPI_RESPONSE_INVALID_TRANSACTION       /* invalid transaction      */
} drv_resp_code_t;

#ifdef __cplusplus
}  /* End of extern "C" */
#endif  /* __cplusplus */

#endif
