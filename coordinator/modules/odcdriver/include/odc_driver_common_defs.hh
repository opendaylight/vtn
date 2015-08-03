/*
 * Copyright (c) 2013-2015 NEC Corporation
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
const std::string BASE_SW_URL           = "/controller/nb/v2/switchmanager";
const std::string BASE_TOPO_URL         = "/controller/nb/v2/topology";
const std::string BASE_PORT_URL         = "/controller/nb/v2/statistics";
const std::string VERSION               = "/version";
const std::string CONTAINER_NAME        = "/default";
const std::string VTNS                  = "/vtns";
const std::string FLOWS                 = "/flows";
const std::string DETAIL                = "/detail";
const std::string NODES                 = "/nodes";
const std::string NODE                  = "/node";
const std::string NODE_OF               = "/OF";
const std::string PORT                  = "/port";

const std::string DEFAULT_USER_NAME     = "admin";
const std::string DEFAULT_PASSWORD      = "admin";
const std::string DOM_NAME              = "(DEFAULT)";
const std::string VTN_URL               = "/controller/nb/v2/vtn/default/vtns";
const std::string RESTCONF_BASE         = "/restconf/operational";
const std::string VTN_TOPO_URL          = "/vtn-topology:vtn-topology";
const std::string VTN_SW_NODES          = "/vtn-inventory:vtn-nodes";
const std::string VTN_PORT              = "/vtn-inventory:vtn-node";
const std::string ODL_PORT              = "/opendaylight-inventory:nodes";

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
const std::string SLASH                 = "/";
const std::string PIPE_SEPARATOR        = "|";
const std::string SW_PREFIX             = "SW-";
const std::string PP_PREFIX             = "PP-";
const std::string PP_OF_PREFIX          = "PP-OF:";
const std::string UNTAGGED_VLANID       = "65535";
const std::string DEFAULT_IP            = "0.0.0.0";
const std::string SWITCH_BASE           = "openflow:";
const std::string SWITCH_HEX            = "0000000000000000";

const uint32_t DEFAULT_ODC_PORT         = 8282;
const uint32_t DEFAULT_CONNECT_TIME_OUT = 30;
const uint32_t DEFAULT_REQ_TIME_OUT     = 30;
const uint32_t PING_INTERVAL            = 10;
const uint32_t PING_RETRY_COUNT         = 5;
const uint32_t DEFAULT_AGE_INTERVAL     = 600;
const uint32_t PING_DEFAULT_INTERVAL    = 30;

const uint32_t VTN_PARSE_FLAG           = 0;
const uint32_t VBR_PARSE_FLAG           = 1;
const uint32_t SWID_PARSE_FLAG          = 2;
const uint ADMIN_DOWN                   = 0;
const uint ADMIN_UP                     = 1;
const uint EDGE_UP                      = 1;

const std::string switchmodel           = "OFS";

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

typedef enum {
  VAL_SWITCH_ATTR = 0,
  VAL_OPER_STAT_ATTR,
  VAL_MAN_ATTR,
  VAL_HARD_ATTR,
  VAL_SOFT_ATTR,
  VAL_ALARAM_INFO
} SwitchValStFlags;

typedef enum {
  VAL_DESCRIPTION = 0,
  VAL_MODEL,
  VAL_SECURE_CHANNEL_IPV4ADDR_ATTR,
  VAL_SECURE_CHANNEL_IPV6ADDR_ATTR,
  VAL_ADMIN_STATUS,
  VAL_DOMAINID_ATTR
} SwitchValFlags;

typedef enum {
  VAL_LOGICAL_PORT_VALID = 0,
  VAL_OPERSTATUS
} LogicalPortValStFlag;

typedef enum {
  VAL_LOGICAL_PORT_DESCRIPTION = 0,
  VAL_LOGICAL_PORT_TYPE,
  VAL_LOGICAL_PORT_SWITCHID,
  VAL_LOGICAL_PORT_PHYPORTID,
  VAL_OPERSTATUS_CRITERIA,
} LogicalPortValFlag;

typedef enum {
  VAL_PORT_STRUCT_ATTR = 0,
} PortValStFlag;

typedef enum {
  VAL_PORT_EVENT_ATTR1 = 0,
  VAL_PORT_EVENT_ATTR2,
  VAL_PORT_EVENT_ATTR3,
  VAL_PORT_EVENT_ATTR4,
  VAL_PORT_EVENT_ATTR5,
  VAL_PORT_EVENT_ATTR6,
  VAL_PORT_EVENT_ATTR7,
  VAL_PORT_EVENT_ATTR8
} PortValFlags;

typedef enum {
  VAL_LINK_STRUCT_ATTR = 0,
  VAL_OPERSTATUS_ATTR
} LinkValStFlags;

typedef enum {
  VAL_DESCRPTION_ATTR = 0
} LinkValFlag;

typedef enum {
  CTRLINFO_NO_CHANGE = 0, /* No changes */
  CTRLINFO_IP_ADDED, /* IP Address added   */
  CTRLINFO_IP_REMOVED, /* IP Address removed */
  CTRLINFO_IP_CHANGED, /* IP Address changed */
  CTRLINFO_OTHER_UPDATE
      /* Other changes      */
} ctrl_info_update_type_t;

}  //  namespace odcdriver
}  //  namespace unc
#endif  // ODCDRIVER_COMMON_DEFS_H_
