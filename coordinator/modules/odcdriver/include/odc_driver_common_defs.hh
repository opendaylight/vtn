/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef ODCDRIVER_COMMON_DEFS_H_
#define ODCDRIVER_COMMON_DEFS_H_

#include <stdint.h>
#include <string>

#define TCLIB_MODULE_NAME "tclib"
#define DRVAPI_AUDIT_RETRY      3
#define DRVAPI_AUDIT_WAIT_COUNT 1000000
#define DRVAPI_CTL_ERROR_CODE   503

namespace unc {
namespace odcdriver {

const std::string BASE_URL              = "/controller/nb/v2/vtn";
const std::string VERSION               = "/version";
const std::string CONTAINER_NAME        = "/default";
const std::string VTNS                  = "/vtns";

const std::string DEFAULT_USER_NAME     = "admin";
const std::string DEFAULT_PASSWORD      = "admin";
const std::string DOM_NAME              = "(DEFAULT)";
const std::string VTN_URL               = "/controller/nb/v2/vtn/default/vtns";

// Configuration block to read from odcdriver.conf
const std::string DRV_CONF_BLK          = "param";

const std::string CONF_USER_NAME        = "user_name";
const std::string CONF_PASSWORD         = "password";
const std::string CONF_ODC_PORT         = "odc_port";
const std::string CONF_CONNECT_TIME_OUT = "connect_time_out";
const std::string CONF_REQ_TIME_OUT     = "request_time_out";
const std::string CONF_PING_INTERVAL    = "odcdrv_ping_interval";

const std::string NODE_TYPE_OF          = "OF-";
const std::string NODE_TYPE_ANY         = "ANY";
const std::string PERIOD                = ".";
const std::string HYPHEN                = "-";
const std::string COLON                 = ":";
const std::string SW_PREFIX             = "SW-";
const std::string PP_PREFIX             = "PP-";
const std::string UNTAGGED_VLANID       = "65535";

const uint32_t DEFAULT_ODC_PORT         = 8080;
const uint32_t DEFAULT_CONNECT_TIME_OUT = 30;
const uint32_t DEFAULT_REQ_TIME_OUT     = 30;
const uint32_t PING_INTERVAL            = 10;
const uint32_t PING_RETRY_COUNT         = 5;
const uint32_t DEFAULT_IDLE_TIME_OUT    = 0;
const uint32_t DEFAULT_HARD_TIME_OUT    = 300;
const uint32_t DEFAULT_AGE_INTERVAL     = 600;
const uint32_t PING_DEFAULT_INTERVAL    = 30;

const uint32_t VTN_PARSE_FLAG           = 0;
const uint32_t VBR_PARSE_FLAG           = 1;
const uint32_t SWID_PARSE_FLAG          = 2;

typedef enum {
  ODC_DRV_SUCCESS = 0,
  ODC_DRV_FAILURE
} odc_drv_resp_code_t;

typedef enum {
  HTTP_400_BAD_REQUEST                = 400,
  HTTP_401_UN_AUTHORISED              = 401,
  HTTP_404_NOT_FOUND                  = 404,
  HTTP_406_NOT_ACCEPTABLE             = 406,
  HTTP_409_CONFLICT                   = 409,
  HTTP_415_UN_SUPPORTED_MEDIA_TYPE    = 415,
  HTTP_500_INTERNAL_SERVER_ERROR      = 500,
  HTTP_503_SERVICE_UNAVAILABLE        = 503,
  HTTP_204_NO_CONTENT                 = 204,
  HTTP_201_RESP_CREATED               = 201,
  HTTP_200_RESP_OK                    = 200
} ServerResponseCode;
}  //  namespace odcdriver
}  //  namespace unc
#endif  // ODCDRIVER_COMMON_DEFS_H_
