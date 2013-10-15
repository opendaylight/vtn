/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the
 * terms of the Eclipse Public License v1.0 which
 * accompanies this
 * distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef ODCDRIVER_COMMON_DEFS_H_
#define ODCDRIVER_COMMON_DEFS_H_

#include <string>
#define TCLIB_MODULE_NAME "tclib"

namespace unc {
namespace odcdriver {

const int ODC_PORT = 8080;
const std::string USER_NAME   = "admin";
const std::string PASS_WORD   = "admin";
const std::string DOM_NAME    = "(DEFAULT)";
const std::string READ_VTN_URL = "/controller/nb/v2/vtn/default/vtns";
const int CONNECTION_TIME_OUT = 30;
const int REQUEST_TIME_OUT = 30;
const uint32_t PING_INTERVAL = 10;
const uint32_t PING_RETRY_COUNT = 5;

typedef enum {
  ODC_DRV_SUCCESS = 0,
  ODC_DRV_FAILURE
} odc_drv_resp_code_t;

typedef enum {
  RESP_BAD_REQUEST = 400,
  RESP_UN_AUTHORISED = 401,
  RESP_NOT_FOUND = 404,
  RESP_NOT_ACCEPTABLE = 406,
  RESP_CONFLICT = 409,
  RESP_UN_SUPPORTED_MEDIA_TYPE = 415,
  RESP_INTERNAL_SERVER_ERROR = 500,
  RESP_SERVICE_UNAVAILABLE = 503,
  RESP_CREATED = 201,
  RESP_OK = 200,
  RESP_UNKNOWN = 1
} ServerResponseCode;
}  // namespace odcdriver
}  // namespace unc
#endif  // ODCDRIVER_COMMON_DEFS_H_
