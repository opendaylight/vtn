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

namespace unc {
namespace odcdriver {

const int ODC_PORT = 8080;
const std::string USER_NAME = "admin";
const std::string PASS_WORD = "admin";

typedef enum {
  ODC_DRV_SUCCESS = 0,
  ODC_DRV_FAILURE
} odc_drv_resp_code_t;

typedef enum {
  HTTP_METHOD_POST = 0,
  HTTP_METHOD_PUT,
  HTTP_METHOD_DELETE,
  HTTP_METHOD_READ
} HttpMethod;

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
}
}
#endif  // ODCDRIVER_COMMON_DEFS_H_
