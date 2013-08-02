/*
 * Copyright (c) 2012-2013 NEC Corporation
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
/**relative path of ODBCM_CONF_FILE*/
#define ODBCM_CONF_FILE_PATH            "/"
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

//  UPPL tables column names
//  controller_table columns
#define CTR_NAME            "controller_name"
#define CTR_TYPE            "type"
#define CTR_VERSION         "version"
#define CTR_DESCRIPTION     "description"
#define CTR_IP_ADDRESS      "ip_address"
#define CTR_USER_NAME       "user_name"
#define CTR_PASSWORD        "password"
#define CTR_ENABLE_AUDIT    "enable_audit"
#define CTR_ACTUAL_VERSION  "actual_version"
#define CTR_OPER_STATUS     "oper_status"
#define CTR_VALID           "valid"
#define CTR_CS_ROW_STATUS  "cs_row_status"
#define CTR_CS_ATTR         "cs_attr"

// ctr_domain_table columns

#define DOMAIN_NAME          "domain_name"
#define DOMAIN_TYPE          "type"
#define DOMAIN_DESCRIPTION   "description"
#define DOMAIN_OP_STATUS     "oper_status"
#define DOMAIN_VALID         "valid"
#define DOMAIN_CS_ROW_STATUS "cs_row_status"
#define DOMAIN_CS_ATTR       "cs_attr"

//  logicalport_table columns
#define LP_PORT_ID            "port_id"
#define LP_DESCRIPTION        "description"
#define LP_PORT_TYPE          "port_type"
#define LP_SWITCH_ID          "switch_id"
#define LP_PHYSICAL_PORT_ID   "physical_port_id"
#define LP_OPER_DOWN_CRITERIA "oper_down_criteria"
#define LP_OPER_STATUS         "oper_status"
#define LP_CTR_VALID           "valid"

//  logical_memberport_table columns
#define LMP_SWITCH_ID "switch_id"
#define LMP_PHYSICAL_PORT_ID "physical_port_id"
#define LMP_LP_PORT_ID "port_id"


//  switch table columns
#define SWITCH_ID           "switch_id"
#define SWITCH_DESCRIPTION  "description"
#define SWITCH_MODEL        "model"
#define SWITCH_IP_ADDRESS   "ip_address"
#define SWITCH_IPV6_ADDRESS "ipv6_address"
#define SWITCH_ADMIN_STATUS "admin_status"
#define SWITCH_DOMAIN_NAME  "domain_name"
#define SWITCH_MANUFACTURER "manufacturer"
#define SWITCH_HARDWARE     "hardware"
#define SWITCH_SOFTWARE     "software"
#define SWITCH_ALARM_STATUS  "alarms_status"
#define SWITCH_OPER_STATUS  "oper_status"
#define SWITCH_VALID        "valid"

// port_table column
//  #define PORT_CTR_NAME "controller_name"
#define PORT_ID             "port_id"
#define PORT_NUMBER         "port_number"
#define PORT_DESCRIPTION    "description"
#define PORT_ADMIN_STATUS   "admin_status"
#define PORT_DIRECTION      "direction"
#define PORT_TRUNK_ALL_VLAN "trunk_allowed_vlan"
#define PORT_OPER_STATUS    "oper_status"
#define PORT_MAC_ADDRESS    "mac_address"
#define PORT_DUPLEX         "duplex"
#define PORT_SPEED          "speed"
#define PORT_ALARM_STATUS   "alarms_status"
#define PORT_LOGIC_PORT_ID  "logical_port_id"
#define PORT_VALID          "valid"

// link_table columns
//  #define LINK_CTR_NAME       "controller_name"
#define LINK_SWITCH_ID1     "switch_id1"
#define LINK_PORT_ID1       "port_id1"
#define LINK_SWITCH_ID2     "switch_id2"
#define LINK_PORT_ID2       "port_id2"
#define LINK_DESCRIPTION    "description"
#define LINK_OPER_STATUS    "oper_status"
#define LINK_VALID          "valid"

// boundary_table columns
#define BDRY_ID             "boundary_id"
#define BDRY_DESCRIPTION    "description"
#define BDRY_CTR_NAME1      "controller_name1"
#define BDRY_DM_NAME1       "domain_name1"
#define BDRY_PORT_ID1       "logical_port_id1"
#define BDRY_CTR_NAME2      "controller_name2"
#define BDRY_DM_NAME2       "domain_name2"
#define BDRY_PORT_ID2       "logical_port_id2"
#define BDRY_OPER_STATUS    "oper_status"
#define BDRY_VALID          "valid"
#define BDRY_ROW_STATUS     "cs_row_status"
#define BDRY_ATTR           "cs_attr"

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
  }

/**Macro to fill the DBTableSchema Object instance with given input values.
 * In the case of static access. (memory might be allocated earlier, here reuse
 * the same memory*/
#define ODBCM_FILL_ATTRIBUTE_INFO(attr_var, \
    attr_name, attr_value, attr_length, attr_type, attr_vector) \
    { \
  attr_var.table_attribute_name = attr_name; \
  attr_var.p_table_attribute_value = reinterpret_cast<void*>(&attr_value); \
  attr_var.table_attribute_length = attr_length;\
  attr_var.request_attribute_type = attr_type; \
  attr_vector.push_back(attr_var); \
    }

#define ODBCM_FILL_ATTRIBUTE_INFOS(attr_var, \
    attr_name, attr_value, attr_length, attr_type, attr_vector) \
    { \
  attr_var.table_attribute_name = attr_name; \
  attr_var.p_table_attribute_value = reinterpret_cast<void*>(attr_value); \
  attr_var.table_attribute_length = attr_length;\
  attr_var.request_attribute_type = attr_type; \
  attr_vector.push_back(attr_var); \
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
  std::string         table_attribute_name;
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
