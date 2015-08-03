/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   ODBC Manager
 * @file    odbcm_common.hh
 *
 */
#ifndef _ODBCM_COMMON_HH_
#define _ODBCM_COMMON_HH_

#include <sqlext.h>
#include <sqltypes.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unc/keytype.h>
#include <pfc/log.h>
#include <sql.h>
#include <map>
#include <vector>
#include <utility>
#include <string>
#include "physical_common_def.hh"

namespace unc {
namespace uppl {

/**number of table present in each DB*/
/**startup database UNC_DT_STARTUP table count*/
#define ODBCM_MAX_STARTUP_TABLES        3
/**candidate database UNC_DT_CANDIDATE table count*/
#define ODBCM_MAX_CANDIDATE_TABLES      3
/**running database UNC_DT_RUNNING table count*/
#define ODBCM_MAX_RUNNING_TABLES        3
/**state database UNC_DT_STATE table count*/
#define ODBCM_MAX_STATE_TABLES          5
/**import database UNC_DT_IMPORT table count*/
#define ODBCM_MAX_IMPORT_TABLES         6
/**Total number of configuration tables **/
#define ODBCM_MAX_UPPL_TABLES           3
/**Total number of running and state tables which are stored together in
 * running database
 */
#define ODBCM_MAX_RUNNING_STATE_TABLES  8
/**odbc manager's connection configuration file, which consists
 * 1. server name/ip
 * 2. name of driver
 * 3. data source name (dsn)
 * 4. connection type
 * 5. port number on which the dbms server is running
 * 6. time out
 * 7. connection time out
 * 8. user login timeout
 * 9. database username
 * 10.database user's password
 */
#define ODBCM_CONF_FILE                 "odbcm.conf"

/*CTR_PORT_INVALID_VALUE will be returned when controller_table
 * port attribute is null*/
#define CTR_PORT_INVALID_VALUE          65536

/**null string assign*/
#define ODBCM_NULL_STRING ""

//  UPPL table names
#define UPPL_CTR_TABLE                  "controller_table"
#define UPPL_CTR_DOMAIN_TABLE           "ctr_domain_table"
#define UPPL_LOGICALPORT_TABLE          "logicalport_table"
#define UPPL_LOGICAL_MEMBER_PORT_TABLE  "logical_member_port_table"
#define UPPL_SWITCH_TABLE               "switch_table"
#define UPPL_PORT_TABLE                 "port_table"
#define UPPL_LINK_TABLE                 "link_table"
#define UPPL_BOUNDARY_TABLE             "boundary_table"


/**controller table Column names enumeration*/
typedef enum {
  CTR_NAME = 0,
  CTR_TYPE,
  CTR_VERSION,
  CTR_DESCRIPTION,
  CTR_IP_ADDRESS,
  CTR_USER_NAME,
  CTR_PASSWORD,
  CTR_ENABLE_AUDIT,
  CTR_ACTUAL_VERSION,
  CTR_OPER_STATUS,
  CTR_PORT,
  CTR_PORT_READ,
  CTR_VALID,
  CTR_CS_ROW_STATUS,
  CTR_CS_ATTR,
  CTR_COMMIT_NUMBER,
  CTR_COMMIT_DATE,
  CTR_COMMIT_APPLICATION,
  CTR_VALID_COMMIT_VERSION,
  CTR_ACTUAL_CONTROLLERID,
  CTR_VALID_ACTUAL_CONTROLLERID,
  DOMAIN_NAME,
  DOMAIN_TYPE,
  DOMAIN_DESCRIPTION,
  DOMAIN_OP_STATUS,
  DOMAIN_VALID,
  DOMAIN_CS_ROW_STATUS,
  DOMAIN_CS_ATTR,
  LP_PORT_ID,
  LP_DESCRIPTION,
  LP_PORT_TYPE,
  LP_SWITCH_ID,
  LP_PHYSICAL_PORT_ID,
  LP_OPER_DOWN_CRITERIA,
  LP_OPER_STATUS,
  LP_CTR_VALID,
  LMP_SWITCH_ID,
  LMP_PHYSICAL_PORT_ID,
  LMP_LP_PORT_ID,
  SWITCH_ID,
  SWITCH_DESCRIPTION,
  SWITCH_MODEL,
  SWITCH_IP_ADDRESS,
  SWITCH_IPV6_ADDRESS,
  SWITCH_ADMIN_STATUS,
  SWITCH_DOMAIN_NAME,
  SWITCH_MANUFACTURER,
  SWITCH_HARDWARE,
  SWITCH_SOFTWARE,
  SWITCH_ALARM_STATUS,
  SWITCH_OPER_STATUS,
  SWITCH_VALID,
  PORT_ID,
  PORT_NUMBER,
  PORT_DESCRIPTION,
  PORT_ADMIN_STATUS,
  PORT_DIRECTION,
  PORT_TRUNK_ALL_VLAN,
  PORT_OPER_STATUS,
  PORT_MAC_ADDRESS,
  PORT_DUPLEX,
  PORT_SPEED,
  PORT_ALARM_STATUS,
  PORT_LOGIC_PORT_ID,
  PORT_VALID,
  PORT_CONNECTED_SWITCH_ID,
  PORT_CONNECTED_PORT_ID,
  PORT_CONNECTED_CONTROLLER_ID,
  PORT_CONNECTEDNEIGHBOR_VALID,
  LINK_SWITCH_ID1,
  LINK_PORT_ID1,
  LINK_SWITCH_ID2,
  LINK_PORT_ID2,
  LINK_DESCRIPTION,
  LINK_OPER_STATUS,
  LINK_VALID,
  BDRY_ID,
  BDRY_DESCRIPTION,
  BDRY_CTR_NAME1,
  BDRY_DM_NAME1,
  BDRY_PORT_ID1,
  BDRY_CTR_NAME2,
  BDRY_DM_NAME2,
  BDRY_PORT_ID2,
  BDRY_OPER_STATUS,
  BDRY_VALID,
  BDRY_ROW_STATUS,
  BDRY_ATTR,
  UNKNOWN_COLUMN
}ODBCMTableColumns;

/*
* odbcm tables column name struct
*/
typedef struct {
  const unc::uppl::ODBCMTableColumns column_id;
  const std::string column_string;
}OdbcmColumnName;

//  UPPL tables column names
//  controller_table columns
#define CTR_NAME_STR            "controller_name"
#define CTR_TYPE_STR            "type"
#define CTR_VERSION_STR         "version"
#define CTR_DESCRIPTION_STR     "description"
#define CTR_IP_ADDRESS_STR      "ip_address"
#define CTR_USER_NAME_STR       "user_name"
#define CTR_PASSWORD_STR        "password"
#define CTR_ENABLE_AUDIT_STR    "enable_audit"
#define CTR_PORT_STR            "port"
#define CTR_PORT_STR_COALESCE   "COALESCE(port, 65536)"
#define CTR_COMMIT_NUMBER_STR    "commit_number"
#define CTR_COMMIT_DATE_STR    "commit_date"
#define CTR_COMMIT_APPLICATION_STR    "commit_application"
#define CTR_ACTUAL_VERSION_STR  "actual_version"
#define CTR_ACTUAL_CONTROLLERID_STR   "actual_controllerid"
#define CTR_VALID_ACTUAL_CONTROLLERID_STR   "actual_ctrid_valid"
#define CTR_OPER_STATUS_STR     "oper_status"
#define CTR_VALID_STR           "valid"
#define CTR_CS_ROW_STATUS_STR   "cs_row_status"
#define CTR_CS_ATTR_STR         "cs_attr"
#define CTR_VALID_COMMIT_VERSION_STR    "valid_commit_version"

// ctr_domain_table columns
#define DOMAIN_NAME_STR          "domain_name"
#define DOMAIN_TYPE_STR          "type"
#define DOMAIN_DESCRIPTION_STR   "description"
#define DOMAIN_OP_STATUS_STR     "oper_status"
#define DOMAIN_VALID_STR         "valid"
#define DOMAIN_CS_ROW_STATUS_STR "cs_row_status"
#define DOMAIN_CS_ATTR_STR       "cs_attr"

//  logicalport_table columns
#define LP_PORT_ID_STR            "port_id"
#define LP_DESCRIPTION_STR        "description"
#define LP_PORT_TYPE_STR          "port_type"
#define LP_SWITCH_ID_STR          "switch_id"
#define LP_PHYSICAL_PORT_ID_STR   "physical_port_id"
#define LP_OPER_DOWN_CRITERIA_STR "oper_down_criteria"
#define LP_OPER_STATUS_STR        "oper_status"
#define LP_CTR_VALID_STR          "valid"

//  logical_memberport_table columns
#define LMP_SWITCH_ID_STR        "switch_id"
#define LMP_PHYSICAL_PORT_ID_STR "physical_port_id"
#define LMP_LP_PORT_ID_STR       "port_id"

//  switch table columns
#define SWITCH_ID_STR           "switch_id"
#define SWITCH_DESCRIPTION_STR  "description"
#define SWITCH_MODEL_STR        "model"
#define SWITCH_IP_ADDRESS_STR   "ip_address"
#define SWITCH_IPV6_ADDRESS_STR "ipv6_address"
#define SWITCH_ADMIN_STATUS_STR "admin_status"
#define SWITCH_DOMAIN_NAME_STR  "domain_name"
#define SWITCH_MANUFACTURER_STR "manufacturer"
#define SWITCH_HARDWARE_STR     "hardware"
#define SWITCH_SOFTWARE_STR     "software"
#define SWITCH_ALARM_STATUS_STR "alarms_status"
#define SWITCH_OPER_STATUS_STR  "oper_status"
#define SWITCH_VALID_STR        "valid"

//  port table columns
#define PORT_ID_STR             "port_id"
#define PORT_NUMBER_STR         "port_number"
#define PORT_DESCRIPTION_STR    "description"
#define PORT_ADMIN_STATUS_STR   "admin_status"
#define PORT_DIRECTION_STR      "direction"
#define PORT_TRUNK_ALL_VLAN_STR "trunk_allowed_vlan"
#define PORT_OPER_STATUS_STR    "oper_status"
#define PORT_MAC_ADDRESS_STR    "mac_address"
#define PORT_DUPLEX_STR         "duplex"
#define PORT_SPEED_STR          "speed"
#define PORT_ALARM_STATUS_STR   "alarms_status"
#define PORT_LOGIC_PORT_ID_STR  "logical_port_id"
#define PORT_CONNECTED_CONTROLLER_ID_STR   "connected_controller"
#define PORT_CONNECTED_SWITCH_ID_STR   "connected_switch"
#define PORT_CONNECTED_PORT_ID_STR   "connected_port"
#define PORT_VALID_STR          "valid"
#define PORT_CONNECTEDNEIGHBOR_VALID_STR          "connectedneighbor_valid"

// link_table columns
#define LINK_SWITCH_ID1_STR     "switch_id1"
#define LINK_PORT_ID1_STR       "port_id1"
#define LINK_SWITCH_ID2_STR     "switch_id2"
#define LINK_PORT_ID2_STR       "port_id2"
#define LINK_DESCRIPTION_STR    "description"
#define LINK_OPER_STATUS_STR    "oper_status"
#define LINK_VALID_STR          "valid"

// boundary_table columns
#define BDRY_ID_STR             "boundary_id"
#define BDRY_DESCRIPTION_STR    "description"
#define BDRY_CTR_NAME1_STR      "controller_name1"
#define BDRY_DM_NAME1_STR       "domain_name1"
#define BDRY_PORT_ID1_STR       "logical_port_id1"
#define BDRY_CTR_NAME2_STR      "controller_name2"
#define BDRY_DM_NAME2_STR       "domain_name2"
#define BDRY_PORT_ID2_STR       "logical_port_id2"
#define BDRY_OPER_STATUS_STR    "oper_status"
#define BDRY_VALID_STR          "valid"
#define BDRY_ROW_STATUS_STR     "cs_row_status"
#define BDRY_ATTR_STR           "cs_attr"
#define VLAN_ID_STR             "vlan_id"

#define CTR_DATAFLOW_FLOWID_STR "flow_id"

#define PHY_FINI_READ_LOCK() \
  if (PhysicalLayer::phyFiniFlag == 1) { \
    pfc_log_info("PhysicalLayer Fini is invoked already ..!!"); \
    return ODBCM_RC_GENERAL_ERROR; \
  } \
  ScopedReadWriteLock dbFiniLock(PhysicalLayer::get_phy_fini_db_lock(), \
      PFC_FALSE); \
  if (PhysicalLayer::phyFiniFlag == 1) { \
    pfc_log_info("PhysicalLayer:: Fini is invoked already ..!!"); \
    return ODBCM_RC_GENERAL_ERROR; \
  } \


/**Macro to fill the DBTableSchema Object instance with given input values.
 * In the case of static access. (memory might be allocated earlier, here reuse
 * the same memory*/
#define ODBCM_FILL_ATTRIBUTE_INFO(attr_var, \
    attr_name, attr_value, attr_length, attr_type, attr_vector) \
    { \
  (attr_var).table_attribute_name = (attr_name); \
  (attr_var).p_table_attribute_value = reinterpret_cast<void*>(&(attr_value)); \
  (attr_var).table_attribute_length = (attr_length);\
  (attr_var).request_attribute_type = (attr_type); \
  (attr_vector).push_back(attr_var); \
    }

#define ODBCM_FILL_ATTRIBUTE_INFOS(attr_var, \
    attr_name, attr_value, attr_length, attr_type, attr_vector) \
    { \
  (attr_var).table_attribute_name = (attr_name); \
  (attr_var).p_table_attribute_value = reinterpret_cast<void*>(attr_value); \
  (attr_var).table_attribute_length = (attr_length);\
  (attr_var).request_attribute_type = (attr_type); \
  (attr_vector).push_back(attr_var); \
    }

/**return type of query factory methods*/
typedef std::string SQLQUERY;
/* To receive all data type values from ITC
 * and the same template will be filled up during the values fetch from table*/
template <typename T>
struct ColumnAttrValue {
  T value;
};
/**this structure will hold the table name, column names, datatype and values
 * template address.*/
struct TableAttrSchema {
  ODBCMTableColumns table_attribute_name;
  //  pointer to struct TableAttrValue
  void*               p_table_attribute_value;
  unsigned int        table_attribute_length;
  AttributeDataType   request_attribute_type;
};
/**enum of arithmetic operators */
typedef enum {
  UNKNOWN_OPERATOR = -1,
  EQUAL,
  NOT_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESSER,
  LESSER_EQUAL,
  MULTIPLE_QUERY
}ODBCMOperator;

/* Enum to store the table_id */
typedef enum {
  UNKNOWN_TABLE = 0,
  CTR_TABLE,
  CTR_DOMAIN_TABLE,
  LOGICALPORT_TABLE,
  LOGICAL_MEMBERPORT_TABLE,
  SWITCH_TABLE,
  PORT_TABLE,
  LINK_TABLE,
  BOUNDARY_TABLE,
  IS_ROW_EXISTS  // keep it last always
}ODBCMTable;
/**During commit all configuration operation, various states will be handled
 * internally, those are listed in this enum*/
typedef enum {
  CHANGE_ROWSTATUS1,
  CHANGE_ROWSTATUS2,
  COPY_STATE_INFO, /*from running to candidate*/
  TRUNCATE_RUNNING,
  COPY_CANDIDATE_TO_RUNNING,
  COMMIT_END
} ODBCMCommitStates;

}  // namespace uppl
}  // namespace unc

#endif /* _ODBCM_COMMON_HH_*/
