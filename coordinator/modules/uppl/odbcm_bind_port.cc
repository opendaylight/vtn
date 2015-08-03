/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *  @brief   ODBC Manager
 *  @file    odbcm_bind_port.cc
 */

#include <stdio.h>
#include <sstream>
#include "odbcm_db_varbind.hh"
#include "odbcm_common.hh"
#include "odbcm_utils.hh"
#include "odbcm_db_tableschema.hh"
#include "odbcm_mgr.hh"

using unc::uppl::DBVarbind;
SQLLEN *p_switch_id1_len = NULL;
SQLLEN *p_switch_id2_len = NULL;
SQLLEN *p_logicalport_id1_len = NULL;
SQLLEN *p_logicalport_id2_len = NULL;
SQLLEN *p_ipv6_len = NULL;
SQLLEN *p_alarms_status_len = NULL;
SQLLEN *p_mac_len = NULL;
SQLLEN *p_speed_len = NULL;
SQLLEN *p_connected_switch_id_len = NULL;
/**
 * @Description : Function to bind input parameter of port_table
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 *                r_hstmt     - statement handler which carries the SQL Query
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::bind_port_table_input(
    std::vector<TableAttrSchema> &column_attr/*DBTableSchema->rowlist_entry*/,
    HSTMT &r_hstmt/**statement handler which carries the SQL Query*/) {
  SQLRETURN odbc_rc = SQL_SUCCESS;  // odbc API's return code initialize with 0
  SQLUSMALLINT col_no  = 0;  // column number
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator i;
  /**Flag to decide printing logs */
  uint8_t log_flag = 1;
  /**Loop for iterate all the elements in the vector, the TableAttrSchema
   * table_attribute_name value will be compared and corresponding
   * structure member will be binded here*/
  for (i = column_attr.begin(); i != column_attr.end(); ++i) {
    if ((*i).p_table_attribute_value == NULL) {
      pfc_log_debug("ODBCM::DBVarbind::bind value structure is empty ");
      continue;
    }
    switch ((*i).table_attribute_name) {
      case CTR_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
      /**binding structure buffer member for controller name input,
       * column size is ODBCM_SIZE_32,
       * Data type CHAR[32],
       * and buffer size will passed as length of value */
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              ODBCM_SIZE_32/*column size in DB table*/,
              0/**decimal point */,
              p_port_table->szcontroller_name/*buffer to carry values*/,
              sizeof(p_port_table->szcontroller_name)-1/**buffer length*/,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
      /**binding structure buffer member for switch_id1 input,
       * column size is ODBCM_SIZE_256,
       * Data type CHAR[256], this char data will be converted into
       * binary before store into database table. switch_id may have non
       * printable characters as well, To allow non printable character
       * from 0-255, the binary is chose*/
          *p_switch_id1_len = strlen((const char*)p_port_table->szswitch_id);
          odbc_rc = BindInputParameter_SQL_BINARY(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              ODBCM_SIZE_256/*column size in DB table*/,
              0/**decimal point */,
              p_port_table->szswitch_id/*buffer to carry values*/,
              sizeof(p_port_table->szswitch_id)-1/**buffer length*/,
              p_switch_id1_len/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              ODBCM_SIZE_32,
              0,
              p_port_table->szport_id,
              sizeof(p_port_table->szport_id)-1,
              NULL);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_NUMBER:
        if ((*i).request_attribute_type == DATATYPE_UINT32) {
          odbc_rc = BindInputParameter_SQL_BIGINT(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLBIGINT*>(&p_port_table->sport_number),
              0,
              NULL);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              ODBCM_SIZE_128,
              0,
              p_port_table->szdescription,
              sizeof(p_port_table->szdescription)-1,
              NULL);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_ADMIN_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLSMALLINT*>(&p_port_table->sadmins_status),
              0,
              NULL);
      /**set flag value 0 to print column binding details */
         log_flag = 0;
         }
         break;
      case PORT_DIRECTION:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLSMALLINT*>(&p_port_table->sdirection),
              0,
              NULL);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_TRUNK_ALL_VLAN:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_INTEGER(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLINTEGER*>(&p_port_table->strunk_allowed_vlan),
              0,
              NULL);
      /**set flag value 0 to print column binding details */
         log_flag = 0;
       }
       break;
      case PORT_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLSMALLINT*>(&p_port_table->soper_status),
              0,
              NULL);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_MAC_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_6) {
           *p_mac_len = ODBCM_SIZE_6;
          odbc_rc = BindInputParameter_SQL_BINARY(
              r_hstmt,
              ++col_no,
              ODBCM_SIZE_6,
              0,
              p_port_table->smac_address,
              sizeof(p_port_table->smac_address)-1,
              p_mac_len);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_DUPLEX:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLSMALLINT*>(&p_port_table->sduplex),
              0,
              NULL);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_SPEED:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
          *p_speed_len = sizeof(SQLLEN);
          odbc_rc = BindInputParameter_SQL_BINARY(
              r_hstmt,
              ++col_no,
              0,
              0,
              &p_port_table->sspeed,
              sizeof(p_port_table->sspeed),
              p_speed_len);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
    break;
      case PORT_ALARM_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
          *p_alarms_status_len = sizeof(SQLLEN);
          odbc_rc = BindInputParameter_SQL_BINARY(
              r_hstmt,
              ++col_no,
              0,
              0,
              &p_port_table->salarms_status,
              sizeof(p_port_table->salarms_status),
              p_alarms_status_len);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_LOGIC_PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
          *p_logicalport_id1_len =
          strlen((const char*)p_port_table->slogical_port_id);
          odbc_rc = BindInputParameter_SQL_BINARY(
              r_hstmt,
              ++col_no,
              0,
              0,
              p_port_table->slogical_port_id,
              sizeof(p_port_table->slogical_port_id)-1,
              p_logicalport_id1_len);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_11) {
          odbc_rc = BindInputParameter_SQL_CHAR(
              r_hstmt,
              ++col_no,
              ODBCM_SIZE_11,
              0,
              p_port_table->svalid,
              sizeof(p_port_table->svalid)-1,
              NULL);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_CONNECTED_CONTROLLER_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
      /**binding structure buffer member for controller name input,
       * column size is ODBCM_SIZE_32,
       * Data type CHAR[32],
       * and buffer size will passed as length of value */
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              ODBCM_SIZE_32/*column size in DB table*/,
              0/**decimal point */,
              p_port_table->szconnected_controller_id/*buffer to carry values*/,
           sizeof(p_port_table->szconnected_controller_id)-1/**buffer length*/,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_CONNECTED_SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
      /**binding structure buffer member for switch_id1 input,
       * column size is ODBCM_SIZE_256,
       * Data type CHAR[256], this char data will be converted into
       * binary before store into database table. switch_id may have non
       * printable characters as well, To allow non printable character
       * from 0-255, the binary is chose*/
          *p_connected_switch_id_len = strlen(
              (const char*)p_port_table->szconnected_switch_id);
          odbc_rc = BindInputParameter_SQL_BINARY(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              ODBCM_SIZE_256/*column size in DB table*/,
              0/**decimal point */,
              p_port_table->szconnected_switch_id/*buffer to carry values*/,
              sizeof(p_port_table->szconnected_switch_id)-1/**buffer length*/,
              p_connected_switch_id_len/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_CONNECTED_PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              ODBCM_SIZE_32,
              0,
              p_port_table->szconnected_port_id,
              sizeof(p_port_table->szconnected_port_id)-1,
              NULL);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_CONNECTEDNEIGHBOR_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
          odbc_rc = BindInputParameter_SQL_CHAR(
              r_hstmt,
              ++col_no,
              ODBCM_SIZE_3,
              0,
              p_port_table->szconnectedneighbor_valid,
              sizeof(p_port_table->szconnectedneighbor_valid)-1,
              NULL);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;

      default:
        break;
    }
    /*In case of bind failure return the parameter bind error */
    if (odbc_rc == SQL_ERROR||odbc_rc == SQL_INVALID_HANDLE) {
      pfc_log_error("ODBCM::DBVarbind::port_table"
          "bind parameter error");
      /**in case of error while binding return to application caller with the
       * error code */
      return ODBCM_RC_PARAM_BIND_ERROR;
    }
    if ((*i).p_table_attribute_value != NULL && log_flag == 0) {
      if (odbc_rc != SQL_SUCCESS || odbc_rc != SQL_SUCCESS_WITH_INFO)
        ODBCMUtils::OdbcmHandleDiagnosticsPrint(SQL_HANDLE_STMT, r_hstmt);
      /**reset flag value 1 */
      log_flag = 1;
    } else {
      pfc_log_debug("ODBCM::**NO bind**i/p:PORT_TABLE:%s:datatype=%d:",
          ODBCManager::get_ODBCManager()->GetColumnName(
          ((*i).table_attribute_name)).c_str(),
          (*i).request_attribute_type);
    }
  }  // for loop
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : Output binding function for port_table
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry  
 *                r_hstmt     - statement handler which carries the SQL Query
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::bind_port_table_output(
  std:: vector<TableAttrSchema> &column_attr/*DBTableSchema->rowlist_ entry*/,
        HSTMT &r_hstmt/**statement handler which carries the SQL Query*/) {
  int64_t odbc_rc = SQL_SUCCESS;  // odbc APIs return code
  uint16_t col_no = 0;  // column number

  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator i;
  uint8_t log_flag = 1;

  /**Loop for iterate all the elements in the vector, the TableAttrSchema
   * table_attribute_name value will be compared and corresponding
   * structure member will be binded here*/
  for (i = column_attr.begin(); i != column_attr.end(); ++i) {
    if ((*i).p_table_attribute_value == NULL) {
      pfc_log_debug("ODBCM::DBVarbind::bind value structure is empty ");
      continue;
    }
    switch ((*i).table_attribute_name) {
      case CTR_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
    /**binding structure buffer member for controller name output,
     * column size is ODBCM_SIZE_32,
     * Data type CHAR[32],
     * and buffer size will passed as length of value,
     * ptr to indicates available no. of bytes return */
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              p_port_table->szcontroller_name/*buffer to fetch values*/,
              ODBCM_SIZE_32+1,
              (&p_port_table->cbname)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          odbc_rc = BindCol_SQL_BINARY(
              r_hstmt,
              ++col_no,
              p_port_table->szswitch_id,
              ODBCM_SIZE_256,
              p_switch_id1_len/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
       break;
      case PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_port_table->szport_id,
              ODBCM_SIZE_32+1,
              (&p_port_table->cbportid)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_NUMBER:
        if ((*i).request_attribute_type == DATATYPE_UINT32) {
          odbc_rc = BindCol_SQL_BIGINT(
              r_hstmt,
              ++col_no,
              reinterpret_cast<SQLBIGINT*>(&p_port_table->sport_number),
              sizeof(SQLBIGINT),
              (&p_port_table->cbportnumber)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_port_table->szdescription,
              ODBCM_SIZE_128+1,
              (&p_port_table->cbdesc)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_ADMIN_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              reinterpret_cast<SQLSMALLINT*>(&p_port_table->sadmins_status),
              sizeof(SQLSMALLINT),
              (&p_port_table->cbadminstatus)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_DIRECTION:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              reinterpret_cast<SQLSMALLINT*>(&p_port_table->sdirection),
              sizeof(SQLSMALLINT),
              (&p_port_table->cbdirection)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_TRUNK_ALL_VLAN:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_INTEGER(
              r_hstmt,
              ++col_no,
              reinterpret_cast<SQLINTEGER*>(&p_port_table->strunk_allowed_vlan),
              sizeof(SQLINTEGER),
              (&p_port_table->cbtavlan)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              reinterpret_cast<SQLSMALLINT*>(&p_port_table->soper_status),
              sizeof(SQLSMALLINT),
              (&p_port_table->cboperstatus)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_MAC_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_6) {
          odbc_rc = BindCol_SQL_BINARY(
              r_hstmt,
              ++col_no,
              p_port_table->smac_address,
              ODBCM_SIZE_6,
              p_mac_len/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_DUPLEX:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              reinterpret_cast<SQLSMALLINT*>(&p_port_table->sduplex),
              sizeof(SQLSMALLINT),
              (&p_port_table->cbduplex)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_SPEED:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
          odbc_rc = BindCol_SQL_BINARY(
              r_hstmt,
              ++col_no,
              &p_port_table->sspeed,
              sizeof(p_port_table->sspeed),
              p_speed_len/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_ALARM_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
          odbc_rc = BindCol_SQL_BINARY(
              r_hstmt,
              ++col_no,
              &p_port_table->salarms_status,
              sizeof(p_port_table->salarms_status),
              p_alarms_status_len/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_LOGIC_PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
          odbc_rc = BindCol_SQL_BINARY(
              r_hstmt,
              ++col_no,
              p_port_table->slogical_port_id,
              sizeof(p_port_table->slogical_port_id)-1,
              p_logicalport_id1_len/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_11) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_port_table->svalid,
              ODBCM_SIZE_11+1,
              (&p_port_table->cbvalid)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_CONNECTED_CONTROLLER_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
    /**binding structure buffer member for controller name output,
     * column size is ODBCM_SIZE_32,
     * Data type CHAR[32],
     * and buffer size will passed as length of value,
     * ptr to indicates available no. of bytes return */
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              p_port_table->szconnected_controller_id/*buffer to fetch values*/,
              ODBCM_SIZE_32+1,
              (&p_port_table->cbconnctrid)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_CONNECTED_SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          odbc_rc = BindCol_SQL_BINARY(
              r_hstmt,
              ++col_no,
              p_port_table->szconnected_switch_id,
              ODBCM_SIZE_256,
              p_connected_switch_id_len/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
       break;
      case PORT_CONNECTED_PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_port_table->szconnected_port_id,
              ODBCM_SIZE_32+1,
              (&p_port_table->cbconnportid)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case PORT_CONNECTEDNEIGHBOR_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_port_table->szconnectedneighbor_valid,
              ODBCM_SIZE_3+1,
              (&p_port_table->cbconnvalid)
              /*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;

      default:
        break;
    }
    /*In case of bind failure return the parameter bind error */
    if (odbc_rc == SQL_ERROR||odbc_rc == SQL_INVALID_HANDLE) {
      pfc_log_error("ODBCM::DBVarbind:: port_table_output: "
          "port_table bind parameter error ");
      /**in case of error while binding return to application caller with the
       * error code */
      return ODBCM_RC_PARAM_BIND_ERROR;
    }
    if (log_flag == 0) {
      /**reset flag value 1*/
      log_flag = 1;
    }
  }
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : To fill the port_table values into structure.
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry 
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::fill_port_table(
    std::vector<TableAttrSchema> &column_attr
    /*DBTableSchema->rowlist_entry*/) {
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator i;
  /**Loop for iterate all the elements in the vector, the TableAttrSchema
  * table_attribute_name value will be compared and corresponding
  * structure member will be filled*/
  for (i = column_attr.begin(); i != column_attr.end(); ++i) {
    if ((*i).p_table_attribute_value == NULL) {
      pfc_log_debug("ODBCM::DBVarbind::bind value structure is empty ");
      continue;
    }
    switch ((*i).table_attribute_name) {
      case CTR_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for controller name CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> cn_value;
          cn_value  = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
              ((*i).p_table_attribute_value));
       /**clear the allocated buffer memory to receive the controller_name
       * from caller*/
          ODBCM_MEMSET(p_port_table->szcontroller_name, 0, ODBCM_SIZE_32+1);
      /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_port_table->szcontroller_name,
              &cn_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "szcontroller_name = %s",  p_port_table->szcontroller_name);
        }
        break;
      case SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for switch id CHAR[256]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_256]> switchid_value;
          switchid_value  = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_256]>*)
              ((*i).p_table_attribute_value));
       /**clear the allocated buffer memory to receive the controller_name
       * from caller*/
          ODBCM_MEMSET(p_port_table->szswitch_id, 0, ODBCM_SIZE_256+1);
      /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_port_table->szswitch_id,
              &switchid_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "szswitch_id = %s",  p_port_table->szswitch_id);
        }
        break;
      case PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for port id CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> id_value;
          id_value  = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_port_table->szport_id, 0, ODBCM_SIZE_32+1);
      /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_port_table->szport_id,
              &id_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "szport_id = %s",  p_port_table->szport_id);
        }
        break;
      case PORT_NUMBER:
        if ((*i).request_attribute_type == DATATYPE_UINT32) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for port number CHAR[32]*/
          ColumnAttrValue <uint32_t> pn_value =
            *((ColumnAttrValue <uint32_t>*)((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_port_table->sport_number, 0, sizeof(SQLBIGINT));
          p_port_table->sport_number = pn_value.value;
          odbcm_debug_info(
              "ODBCM::DBVarbind::fill:PORT_TABLE:sport_number=%"PFC_PFMT_d64,
                           p_port_table->sport_number);
        }
        break;
      case PORT_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for description CHAR[128]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_128]> desc_value =
            *((ColumnAttrValue <uint8_t[128]>*)((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_port_table->szdescription, 0, ODBCM_SIZE_128+1);
      /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_port_table->szdescription,
              &desc_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE: "
              "szDescription = %s", p_port_table->szdescription);
        }
        break;
      case PORT_ADMIN_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for admin status CHAR[16]*/
          ColumnAttrValue <uint16_t> admin_value;
          admin_value =
            *((ColumnAttrValue <uint16_t>*)((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_port_table->sadmins_status, 0,
              sizeof(p_port_table->sadmins_status));
          p_port_table->sadmins_status = admin_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "sadmins_status = %d", p_port_table->sadmins_status);
        }
        break;
      case PORT_DIRECTION:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for direction CHAR[16]*/
          ColumnAttrValue <uint16_t> port_direction_value;
          port_direction_value = *((ColumnAttrValue <uint16_t>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_port_table->sdirection, 0,
              sizeof(p_port_table->sdirection));
          p_port_table->sdirection = port_direction_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "sdirection = %d", p_port_table->sdirection);
        }
        break;
      case PORT_TRUNK_ALL_VLAN:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for trunk allowed vlan CHAR[16]*/
          ColumnAttrValue <uint16_t> trunk_allowed_vlan_value;
          trunk_allowed_vlan_value = *((ColumnAttrValue <uint16_t>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_port_table->strunk_allowed_vlan, 0,
              sizeof(p_port_table->strunk_allowed_vlan));
          p_port_table->strunk_allowed_vlan = trunk_allowed_vlan_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "strunk_allowed_vlan = %d",
              static_cast<int> (p_port_table->strunk_allowed_vlan));
        }
        break;
      case PORT_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for oper status CHAR[16]*/
          ColumnAttrValue <uint16_t> oper_value;
          oper_value =
            *((ColumnAttrValue <uint16_t>*)((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_port_table->soper_status, 0,
              sizeof(p_port_table->soper_status));
          p_port_table->soper_status = oper_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "soper_status = %d", p_port_table->soper_status);
        }
        break;
      case PORT_MAC_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_6) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for mac address CHAR[6]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_6]> ma_value;
          ma_value  = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_6]>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_port_table->smac_address, 0, ODBCM_SIZE_6+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_port_table->smac_address,
              &ma_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "smac_address = %02x:%02x:%02x:%02x:%02x:%02x ",
               p_port_table->smac_address[0], p_port_table->smac_address[1],
               p_port_table->smac_address[2], p_port_table->smac_address[3],
               p_port_table->smac_address[4], p_port_table->smac_address[5]);
        }
        break;
      case PORT_DUPLEX:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for duplex CHAR[16]*/
          ColumnAttrValue <uint16_t> duplex_value;
          duplex_value = *((ColumnAttrValue <uint16_t>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_port_table->sduplex, 0,
              sizeof(p_port_table->sduplex));
          p_port_table->sduplex = duplex_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "sduplex = %d", p_port_table->sduplex);
        }
        break;
      case PORT_SPEED:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for speed CHAR[64]*/
          ColumnAttrValue <uint64_t> speed_value;
          speed_value = *((ColumnAttrValue <uint64_t>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_port_table->sspeed, 0,
              sizeof(p_port_table->sspeed));
          p_port_table->sspeed = speed_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "sspeed = %"PFC_PFMT_d64, p_port_table->sspeed);
        }
        break;
      case PORT_ALARM_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for alarm status CHAR[64]*/
          ColumnAttrValue <uint64_t> alarm_value;
          alarm_value = *((ColumnAttrValue <uint64_t>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_port_table->salarms_status, 0,
              sizeof(p_port_table->salarms_status));
          p_port_table->salarms_status = alarm_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "salarms_status = %" PFC_PFMT_d64, p_port_table->salarms_status);
        }
        break;
      case PORT_LOGIC_PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for logical port id CHAR[320]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_320]> log_id_value;
          log_id_value  = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_320]>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_port_table->slogical_port_id, 0, ODBCM_SIZE_320+1);
      /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_port_table->slogical_port_id,
              &log_id_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "slogical_port_id = %s",  p_port_table->slogical_port_id);
        }
        break;
      case PORT_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_11) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for valid CHAR[11]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_11]> valid_val;
          valid_val =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_11]>*)
                ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_port_table->svalid, 0, ODBCM_SIZE_11+1);
      /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_port_table->svalid,
              &valid_val.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "sValid = %s", p_port_table->svalid);
        }
        break;
      case PORT_CONNECTED_CONTROLLER_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for controller name CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> nbrcn_value;
          nbrcn_value  = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
              ((*i).p_table_attribute_value));
       /**clear the allocated buffer memory to receive the controller_name
       * from caller*/
          ODBCM_MEMSET(p_port_table->szconnected_controller_id,
              0, ODBCM_SIZE_32+1);
      /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_port_table->szconnected_controller_id,
              &nbrcn_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "szconnected_controller_id = %s",
              p_port_table->szconnected_controller_id);
        }
        break;
      case PORT_CONNECTED_SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for switch id CHAR[256]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_256]> nbrswitchid_value;
          nbrswitchid_value  = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_256]>*)
              ((*i).p_table_attribute_value));
       /**clear the allocated buffer memory to receive the controller_name
       * from caller*/
          ODBCM_MEMSET(p_port_table->szconnected_switch_id, 0,
              ODBCM_SIZE_256+1);
      /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_port_table->szconnected_switch_id,
              &nbrswitchid_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "szconnectedswitch_id = %s", p_port_table->szconnected_switch_id);
        }
        break;
      case PORT_CONNECTED_PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for port id CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> nbrportid_value;
          nbrportid_value  = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_port_table->szconnected_port_id, 0, ODBCM_SIZE_32+1);
      /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_port_table->szconnected_port_id,
              &nbrportid_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "szconnected_port_id = %s",  p_port_table->szconnected_port_id);
        }
        break;
      case PORT_CONNECTEDNEIGHBOR_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for valid CHAR[3]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_3]> valid_val;
          valid_val =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_3]>*)
                ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_port_table->szconnectedneighbor_valid,
              0, ODBCM_SIZE_3+1);
      /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_port_table->szconnectedneighbor_valid,
              &valid_val.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:PORT_TABLE:"
              "szconnectedneighbor_valid = %s",
              p_port_table->szconnectedneighbor_valid);
        }
        break;

      default:
        break;
    }
  }
  /*return the final status to caller*/
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : To fetch the port_table values
 *                (which are stored in binded buffer) 
 *                and store into TableAttSchema
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::fetch_port_table(
    std::vector<TableAttrSchema> &column_attr
  /*DBTableSchema->rowlist_entry*/) {
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator i;
  /**Loop for iterate all the elements in the vector, the TableAttrSchema
   * table_attribute_name value will be compared and corresponding
   * structure member will be fetched */
  for (i = column_attr.begin(); i != column_attr.end(); ++i) {
    switch ((*i).table_attribute_name) {
      case CTR_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_32+1],
                                val_controller_name);
          ODBCM_MEMCPY(
              val_controller_name->value,
              p_port_table->szcontroller_name,
              sizeof(p_port_table->szcontroller_name));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "szcontroller_name = %s", val_controller_name->value);
          (*i).p_table_attribute_value = val_controller_name;
        }
        break;
      case SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_256+1],
                                            val_switch_id);
          ODBCM_MEMCPY(
              val_switch_id->value,
              p_port_table->szswitch_id,
              *p_switch_id1_len);
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "szswitch_id = %s", val_switch_id->value);
          (*i).p_table_attribute_value =val_switch_id;
        }
        break;
      case PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_32+1],
                                         val_port_id);
          ODBCM_MEMCPY(
              val_port_id->value,
              p_port_table->szport_id,
              sizeof(p_port_table->szport_id));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "szport_id = %s", val_port_id->value);
          (*i).p_table_attribute_value = val_port_id;
        }
        break;
      case PORT_NUMBER:
        if ((*i).request_attribute_type == DATATYPE_UINT32) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint32_t, no_value);
          no_value->value = p_port_table->sport_number;
          (*i).p_table_attribute_value = no_value;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "port_number = %d", no_value->value);
        }
        break;
      case PORT_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_128+1],
                                   val_description);
          ODBCM_MEMCPY(
              val_description->value,
              p_port_table->szdescription,
              sizeof(p_port_table->szdescription));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "szdescription = %s", val_description->value);
          (*i).p_table_attribute_value = val_description;
        }
        break;
      case PORT_ADMIN_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, admin_status_value);
          admin_status_value->value = p_port_table->sadmins_status;
          (*i).p_table_attribute_value = admin_status_value;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "sadmins_status = %d", admin_status_value->value);
        }
        break;
      case PORT_DIRECTION:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, port_direction_value);
          port_direction_value->value = p_port_table->sdirection;
          (*i).p_table_attribute_value = port_direction_value;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "sport_direction= %d", port_direction_value->value);
        }
        break;
      case PORT_TRUNK_ALL_VLAN:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, trunk_allowed_vlan_value);
          trunk_allowed_vlan_value->value = p_port_table->strunk_allowed_vlan;
          (*i).p_table_attribute_value = trunk_allowed_vlan_value;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "strunk_allowed_vlan= %d", trunk_allowed_vlan_value->value);
        }
        break;
      case PORT_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, oper_status_value);
          oper_status_value->value = p_port_table->soper_status;
          (*i).p_table_attribute_value = oper_status_value;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "soper_status = %d", oper_status_value->value);
        }
        break;
      case PORT_MAC_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_6) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_6+1],
                           val_mac_address);
          ODBCM_MEMCPY(
              val_mac_address->value,
              p_port_table->smac_address,
              sizeof(p_port_table->smac_address));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "smac_address = %02x:%02x:%02x:%02x:%02x:%02x ",
                p_port_table->smac_address[0], p_port_table->smac_address[1],
                p_port_table->smac_address[2], p_port_table->smac_address[3],
                p_port_table->smac_address[4], p_port_table->smac_address[5]);
          (*i).p_table_attribute_value = val_mac_address;
        }
        break;
      case PORT_DUPLEX:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, duplex_value);
          duplex_value->value = p_port_table->sduplex;
          (*i).p_table_attribute_value = duplex_value;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "sduplex= %d", duplex_value->value);
        }
        break;
      case PORT_SPEED:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint64_t,  speed_value);
          speed_value->value = p_port_table->sspeed;
          (*i).p_table_attribute_value = speed_value;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "sspeed= %" PFC_PFMT_u64, speed_value->value);
        }
        break;
      case PORT_ALARM_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint64_t,  alarm_value);
          alarm_value->value = p_port_table->salarms_status;
          (*i).p_table_attribute_value = alarm_value;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "salarms_status= %" PFC_PFMT_u64, alarm_value->value);
        }
        break;
      case PORT_LOGIC_PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_320],
                       val_log_id);
          ODBCM_MEMCPY(
              val_log_id->value,
              p_port_table->slogical_port_id,
              *p_logicalport_id1_len);
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "slogical_port_id = %s", val_log_id->value);
          (*i).p_table_attribute_value = val_log_id;
        }
        break;
      case PORT_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_11) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_11+1],
              valid_value);
          ODBCM_MEMCPY(
              valid_value->value,
              p_port_table->svalid,
              sizeof(p_port_table->svalid));
          (*i).p_table_attribute_value = valid_value;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "svalid = %s", valid_value->value);
        }
        break;
      case PORT_CONNECTED_CONTROLLER_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_32+1],
                                val_connected_controller_id);
          ODBCM_MEMCPY(
              val_connected_controller_id->value,
              p_port_table->szconnected_controller_id,
              sizeof(p_port_table->szconnected_controller_id));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "szconnected_controller_id = %s",
              val_connected_controller_id->value);
          (*i).p_table_attribute_value = val_connected_controller_id;
        }
        break;
      case PORT_CONNECTED_SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_256+1],
                                            val_connected_switch_id);
          ODBCM_MEMCPY(
              val_connected_switch_id->value,
              p_port_table->szconnected_switch_id,
              *p_connected_switch_id_len);
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "szconnected_switch_id = %s", val_connected_switch_id->value);
          (*i).p_table_attribute_value =val_connected_switch_id;
        }
        break;
      case PORT_CONNECTED_PORT_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_32+1],
                                         val_connected_port_id);
          ODBCM_MEMCPY(
              val_connected_port_id->value,
              p_port_table->szconnected_port_id,
              sizeof(p_port_table->szconnected_port_id));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "szconnected_port_id = %s", val_connected_port_id->value);
          (*i).p_table_attribute_value = val_connected_port_id;
        }
        break;
      case PORT_CONNECTEDNEIGHBOR_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_3+1],
              connected_valid_value);
          ODBCM_MEMCPY(
              connected_valid_value->value,
              p_port_table->szconnectedneighbor_valid,
              sizeof(p_port_table->szconnectedneighbor_valid));
          (*i).p_table_attribute_value = connected_valid_value;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:PORT_TABLE: "
              "szconnectedneighbor_valid = %s", connected_valid_value->value);
        }
        break;

      default:
        break;
    }
  }
  return ODBCM_RC_SUCCESS;
}
/**EOF*/
