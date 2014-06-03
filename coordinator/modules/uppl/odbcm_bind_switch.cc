/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *  @brief   ODBC Manager
 *  @file    odbcm_bind_switch.cc
 */

#include "odbcm_db_varbind.hh"
#include "odbcm_common.hh"
#include "odbcm_utils.hh"
#include "odbcm_db_tableschema.hh"
#include "odbcm_mgr.hh"

using unc::uppl::DBVarbind;

/**
 * @Description : Function to bind input parameter of switch_table
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 *                r_hstmt     - statement handler which carries the SQL Query
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::bind_switch_table_input(
    std::vector<TableAttrSchema> &column_attr/*DBTableSchema->rowlist_entry*/,
    HSTMT &r_hstmt/**statement handler which carries the SQL Query*/) {

  SQLRETURN odbc_rc = SQL_SUCCESS;  // odbc API's return code initialize with 0
  SQLUSMALLINT col_no = 0;  // column number
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator i;
  /**Flag to decide printing logs */
  uint8_t log_flag = 1;

  /**Loop for iterate all the elements in the vector, the TableAttrSchema
   * table_attribute_name value will be compared and corresponding
   * structure member will be binded here*/
  for (i = column_attr.begin(); i != column_attr.end(); ++i) {
     /**binding structure buffer member for controller name input,
       * column size is ODBCM_SIZE_32,
       * Data type CHAR[32],
       * and buffer size will passed as length of value */
    if ((*i).p_table_attribute_value == NULL) {
      pfc_log_debug("ODBCM::DBVarbind::bind value structure is empty ");
      continue;
    }
    switch ((*i).table_attribute_name) {
      case CTR_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/ ,
              ODBCM_SIZE_32/*column size in DB table*/,
              0/**decimal point */,
              p_switch_table->szController_name/*buffer to carry values*/,
              sizeof(p_switch_table->szController_name)-1/**buffer length*/,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
         /**binding structure buffer member for switch_id input,
           * column size is ODBCM_SIZE_256,
           * Data type CHAR[256], this char data will be converted into
           * binary before store into database table. switch_id may have non
           * printable characters as well, To allow non printable character
           * from 0-255, the binary is chose*/
          *p_switch_id1_len = strlen((const char*)p_switch_table->szswitch_id);
          odbc_rc = BindInputParameter_SQL_BINARY(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              ODBCM_SIZE_256/*column size in DB table*/,
              0/**decimal point */,
              p_switch_table->szswitch_id/*buffer to carry values*/,
              sizeof(p_switch_table->szswitch_id)-1,
              p_switch_id1_len/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
         /**binding structure buffer member for description,
           * column size is ODBCM_SIZE_128,
           * data type CHAR[128]*/
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt,
              ++col_no ,
              ODBCM_SIZE_128,
              0,
              p_switch_table->szdescription,
              sizeof(p_switch_table->szdescription)-1,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_MODEL:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_16) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt,
              ++col_no ,
              ODBCM_SIZE_16,
              0,
              p_switch_table->szmodel,
              sizeof(p_switch_table->szmodel)-1,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_IP_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_IPV4) {
          // DATATYPE_UINT32
          odbc_rc = BindInputParameter_SQL_BIGINT(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLBIGINT*>(&p_switch_table->szip_address),
              0,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_IPV6_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_IPV6) {
          *p_ipv6_len = strlen((const char*)p_switch_table->szipv6_address);
          odbc_rc = BindInputParameter_SQL_BINARY(
              r_hstmt,
              ++col_no,
              0,
              0,
              p_switch_table->szipv6_address,
              sizeof(p_switch_table->szipv6_address)-1,
              p_ipv6_len/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_ADMIN_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLSMALLINT*>(&p_switch_table->sadmin_status),
              0,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_DOMAIN_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
         /**binding structure buffer member for domainname,
           * column size is ODBCM_SIZE_32,
           * data type CHAR[32]*/
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              ODBCM_SIZE_32,
              0,
              p_switch_table->szdomain_name,
              sizeof(p_switch_table->szdomain_name)-1,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLSMALLINT*>(&p_switch_table->soper_status),
              0,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_MANUFACTURER:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
         /**binding structure buffer member for manufacturer,
           * column size is ODBCM_SIZE_256,
           * data type CHAR[256]*/
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt,
              ++col_no ,
              ODBCM_SIZE_256,
              0,
              p_switch_table->szmanufacturer/*buffer to carry values*/,
              sizeof(p_switch_table->szmanufacturer)-1,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_HARDWARE:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
         /**binding structure buffer member for hardware,
           * column size is ODBCM_SIZE_256,
           * data type CHAR[256]*/
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt,
              ++col_no ,
              ODBCM_SIZE_256,
              0,
              p_switch_table->szhardware/*buffer to carry values*/,
              sizeof(p_switch_table->szhardware)-1,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_SOFTWARE:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
         /**binding structure buffer member for software,
           * column size is ODBCM_SIZE_256,
           * data type CHAR[256]*/
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt,
              ++col_no ,
              ODBCM_SIZE_256,
              0,
              p_switch_table->szsoftware/*buffer to carry values*/,
              sizeof(p_switch_table->szsoftware)-1,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_ALARM_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
          *p_alarms_status_len = sizeof(SQLLEN);
          odbc_rc = BindInputParameter_SQL_BINARY(
              r_hstmt,
             ++col_no,
              0,
              0,
              &p_switch_table->salarms_status/*buffer to carry values*/,
              sizeof(p_switch_table->salarms_status),
              p_alarms_status_len/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_11) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt/**sql statement handler*/,
              ++col_no ,
              ODBCM_SIZE_11,
              0,
              p_switch_table->svalid/*buffer to carry values*/,
              sizeof(p_switch_table->svalid)-1,
               NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      default:
        break;
    }
    if (odbc_rc == SQL_ERROR || odbc_rc == SQL_INVALID_HANDLE) {
      pfc_log_error("ODBCM::DBVarbind::bind_switch_table_input"
          "bind parameter error :: SQL_ERROR or SQL_INVALID_HANDLE");
      /**in case of error while binding return to application caller with the
       * error code */
      return ODBCM_RC_PARAM_BIND_ERROR;
    }
    if ((*i).p_table_attribute_value != NULL && log_flag == 0) {
      if (odbc_rc != SQL_SUCCESS)
        ODBCMUtils::OdbcmHandleDiagnosticsPrint(SQL_HANDLE_STMT, r_hstmt);
      /**reset flag value 1 */
      log_flag = 1;
    } else {
      pfc_log_debug("ODBCM::DBVarbind::**NO bind**i/p:SWITCH_TABLE:%s:"
          "datatype=%d:", ODBCManager::get_ODBCManager()->GetColumnName(
          ((*i).table_attribute_name)).c_str(),
          (*i).request_attribute_type);
    }
  }  // for loop
  return ODBCM_RC_SUCCESS;
}
/**
 * @Description : Output binding function for link_table
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 *                r_hstmt     - statement handler which carries the SQL Query
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::bind_switch_table_output(
  std::vector<TableAttrSchema> &column_attr/*DBTableSchema->rowlist_entry*/,
        HSTMT &r_hstmt/**statement handler which carries the SQL Query*/) {
  SQLRETURN odbc_rc = SQL_SUCCESS;  // odbc APIs return code
  uint16_t col_no = 0;  // column number
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator i;
  /**Flag to decide printing logs */
  uint8_t log_flag = 1;

  /**Loop for iterate all the elements in the vector, the TableAttrSchema
   * table_attribute_name value will be compared and corresponding
   * structure member will be binded here*/
  for (i = column_attr.begin(); i != column_attr.end(); ++i) {
    /**binding structure buffer member for controller name output,
     * column size is ODBCM_SIZE_32,
     * Data type CHAR[32],
     * and buffer size will passed as length of value,
     * ptr to indicates available no. of bytes return */
    if ((*i).p_table_attribute_value == NULL) {
      pfc_log_debug("ODBCM::DBVarbind::bind value structure is empty ");
      continue;
    }
    switch ((*i).table_attribute_name) {
      case CTR_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              p_switch_table->szController_name/*buffer to fetch values*/,
              ODBCM_SIZE_32+1,
              /**no.of bytes available to return*/
              (&p_switch_table->cbname));
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          odbc_rc = BindCol_SQL_BINARY(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              p_switch_table->szswitch_id,
              sizeof(p_switch_table->szswitch_id)-1,
              p_switch_id1_len/*buffer to fetch values*/);
         /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              p_switch_table->szdescription,
              ODBCM_SIZE_128+1,
              (&p_switch_table->cbdesc)
              /*buffer to fetch values*/);
         /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_MODEL:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_16) {
          odbc_rc =  BindCol_SQL_VARCHAR(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              p_switch_table->szmodel,
              ODBCM_SIZE_16+1,
              (&p_switch_table->cbmodel));
          /**set flag value 0 to print column binding details */
           log_flag = 0;
        }
        break;
      case SWITCH_IP_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_IPV4) {
          odbc_rc = BindCol_SQL_BIGINT(
              r_hstmt,
              ++col_no/*parameter number (sequential order)*/,
              reinterpret_cast<SQLBIGINT*>(&p_switch_table->szip_address),
              sizeof(SQLBIGINT),
              (&p_switch_table->cbipaddr)
              /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_IPV6_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_IPV6) {
          odbc_rc = BindCol_SQL_BINARY(
              r_hstmt,
              ++col_no/*parameter number (sequential order)*/,
              reinterpret_cast<SQLCHAR*>(p_switch_table->szipv6_address),
              sizeof(p_switch_table->szipv6_address)-1,
              p_ipv6_len);
         /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_ADMIN_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
              r_hstmt,
              ++col_no/*parameter number (sequential order)*/,
              reinterpret_cast<SQLSMALLINT*>(&p_switch_table->sadmin_status),
              sizeof(SQLSMALLINT),
              (&p_switch_table->cbadminstatus));
         /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_DOMAIN_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
               r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              p_switch_table->szdomain_name,
              ODBCM_SIZE_32+1,
              (&p_switch_table->cbdomainname));
         /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
              r_hstmt,
              ++col_no/*parameter number (sequential order)*/,
              reinterpret_cast<SQLSMALLINT*>(&p_switch_table->soper_status),
              sizeof(SQLSMALLINT),
              (&p_switch_table->cboperstatus));
         /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_MANUFACTURER:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no/*parameter number (sequential order)*/,
              p_switch_table->szmanufacturer,
              ODBCM_SIZE_256+1,
              (&p_switch_table->cbmanufacturer));
         /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_HARDWARE:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          odbc_rc = BindCol_SQL_VARCHAR(r_hstmt,
              ++col_no/*parameter number (sequential order)*/,
              p_switch_table->szhardware,
              ODBCM_SIZE_256+1,
              (&p_switch_table->cbhardware));
         /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_SOFTWARE:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          odbc_rc = BindCol_SQL_VARCHAR(r_hstmt,
              ++col_no/*parameter number (sequential order)*/,
              p_switch_table->szsoftware,
              ODBCM_SIZE_256+1,
              (&p_switch_table->cbsoftware));
         /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_ALARM_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
          odbc_rc = BindCol_SQL_BINARY(
              r_hstmt,
              ++col_no/*parameter number (sequential order)*/,
              &p_switch_table->salarms_status,
              sizeof(p_switch_table->salarms_status),
              p_alarms_status_len);
         /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case SWITCH_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_11) {
          odbc_rc = BindCol_SQL_VARCHAR(r_hstmt,
              ++col_no/*parameter number (sequential order)*/,
              p_switch_table->svalid,
              ODBCM_SIZE_11+1,
              (&p_switch_table->cbvalid));
         /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      default:
        break;
    }
    /*In case of bind failure return the parameter bind error */
    if (odbc_rc == SQL_ERROR || odbc_rc == SQL_INVALID_HANDLE) {
      pfc_log_error("ODBCM::DBVarbind::bind_switch_table_output"
          "switch_table bind parameter error");
      /**in case of error while binding return to application caller with the
       * error code */
      return ODBCM_RC_PARAM_BIND_ERROR;
    }
    if (log_flag == 0) {
    /**reset flag value 1*/
      log_flag = 1;
    }
  }  // for loop
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : To fill the switch_table values into structure.
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::fill_switch_table(
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
           * for controller_name CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> cn_value =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
            ((*i).p_table_attribute_value));
          /**clear the allocated buffer memory to receive the controller_name
           * from caller*/
          ODBCM_MEMSET(p_switch_table->szController_name, 0, ODBCM_SIZE_32+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_switch_table->szController_name,
              &cn_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE: "
              "szController_name=%s", p_switch_table->szController_name);
        }
        break;
      case SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
         /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           *  for switch id*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_256]> sw_value =
              *((ColumnAttrValue <uint8_t[256]>*)
           ((*i).p_table_attribute_value));
          /**copying the value from template to binded buffer */
          ODBCM_MEMSET(p_switch_table->szswitch_id, 0, ODBCM_SIZE_256+1);
          ODBCM_MEMCPY(
              p_switch_table->szswitch_id,
              &sw_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE: "
              "szswitch_id = %s", p_switch_table->szswitch_id);
        }
        break;
      case SWITCH_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
         /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           */
          ColumnAttrValue <uint8_t[ODBCM_SIZE_128]> desc_value =
              *((ColumnAttrValue <uint8_t[128]>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_switch_table->szdescription, 0, ODBCM_SIZE_128+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_switch_table->szdescription,
              &desc_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE: "
              "szdescription = %s", p_switch_table->szdescription);
        }
        break;
      case SWITCH_MODEL:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_16) {
          ColumnAttrValue <uint8_t[ODBCM_SIZE_16]> model_value =
              *((ColumnAttrValue <uint8_t[16]>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_switch_table->szmodel, 0, ODBCM_SIZE_16+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_switch_table->szmodel,
              &model_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE: "
              "szmodel= %s", p_switch_table->szmodel);
        }
        break;
          case SWITCH_IP_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_IPV4) {
         /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
             * for ivp4*/
          ColumnAttrValue <uint32_t> ipv4_value =
              *((ColumnAttrValue <uint32_t>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_switch_table->szip_address, 0, sizeof(SQLBIGINT));
          p_switch_table->szip_address = ipv4_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE:szIp_address"
              " = %s", ODBCMUtils::get_ip_string(
                  p_switch_table->szip_address).c_str());
        }
        break;
      case SWITCH_IPV6_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_IPV6) {
         /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
             * for ipv6 */
          ColumnAttrValue <char[ODBCM_SIZE_16]> *ipv6_value =
                    ((ColumnAttrValue<char[ODBCM_SIZE_16]>*)
                    ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_switch_table->szipv6_address, 0, ODBCM_SIZE_16+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_switch_table->szipv6_address,
              &ipv6_value->value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE:"
              "szIpv6_address = %s", ODBCMUtils::get_ipv6_string(
                  p_switch_table->szipv6_address).c_str());
        }
        break;
      case SWITCH_ADMIN_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          ColumnAttrValue <uint16_t> admin_status_value =
              *((ColumnAttrValue <uint16_t>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_switch_table->sadmin_status, 0, sizeof(SQLSMALLINT));
          p_switch_table->sadmin_status = admin_status_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE:"
              "sadmin_status=%d", p_switch_table->sadmin_status);
        }
        break;
      case SWITCH_DOMAIN_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
         /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           */
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> dn_value =
              *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
                  ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_switch_table->szdomain_name, 0, ODBCM_SIZE_32+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_switch_table->szdomain_name,
              &dn_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE:"
              "szdomain_name = %s", p_switch_table->szdomain_name);
        }
        break;
      case SWITCH_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
         /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           */
          ColumnAttrValue <uint16_t> oper_status_value =
              *((ColumnAttrValue <uint16_t>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_switch_table->soper_status, 0, sizeof(SQLSMALLINT));
          p_switch_table->soper_status = oper_status_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE:"
              "soper_status=%d", p_switch_table->soper_status);
        }
        break;
      case SWITCH_MANUFACTURER:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
         /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
            * for manufacturer CHAR[256]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_256]> manufacturer_value =
              *((ColumnAttrValue <uint8_t[256]>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_switch_table->szmanufacturer, 0, ODBCM_SIZE_256+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_switch_table->szmanufacturer,
              &manufacturer_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE: "
              "szmanufacturer = %s", p_switch_table->szmanufacturer);
        }
        break;
      case SWITCH_HARDWARE:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
         /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           * for hardware CHAR[256]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_256]> hardware_value =
              *((ColumnAttrValue <uint8_t[ODBCM_SIZE_256]>*)
                  ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_switch_table->szhardware, 0, ODBCM_SIZE_256+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_switch_table->szhardware,
              &hardware_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE: "
              "szhardware = %s", p_switch_table->szhardware);
        }
        break;
      case SWITCH_SOFTWARE:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           * for software CHAR[256]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_256]> software_value =
              *((ColumnAttrValue <uint8_t[ODBCM_SIZE_256]>*)
                  ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_switch_table->szsoftware, 0, ODBCM_SIZE_256+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
             p_switch_table->szsoftware,
             &software_value.value,
             (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE: "
              "szsoftware = %s", p_switch_table->szsoftware);
        }
        break;
      case SWITCH_ALARM_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
          /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           * for alarm_status uint64_t*/
          ColumnAttrValue <uint64_t> alarms_status;
          alarms_status = *((ColumnAttrValue <uint64_t>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_switch_table->salarms_status, 0,
                       sizeof(p_switch_table->salarms_status));
          p_switch_table->salarms_status = alarms_status.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE: "
              "salarms_status = %"PFC_PFMT_d64, p_switch_table->salarms_status);
        }
        break;
      case SWITCH_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_11) {
         /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           * for valid CHAR[11]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_11]> valid_value =
              *((ColumnAttrValue <uint8_t[ODBCM_SIZE_11]>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_switch_table->svalid, 0, ODBCM_SIZE_11+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_switch_table->svalid,
              &valid_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:SWITCH_TABLE: "
              "sValid = %s", p_switch_table->svalid);
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
 * @Description : To fetch the switch_table values
 *                (which is stored in  SWITCH_TABLE_ struct)
 *                and store into TableAttSchema
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::fetch_switch_table(
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
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_32+1],
              val_controller_name);
          ODBCM_MEMCPY(
              val_controller_name->value,
              p_switch_table->szController_name,
              sizeof(p_switch_table->szController_name));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "controller_name = %s" , val_controller_name->value);
          (*i).p_table_attribute_value = val_controller_name;
        }
        break;
      case SWITCH_ID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_256+1],
              sw_value);
          ODBCM_MEMCPY(
              sw_value->value,
              p_switch_table->szswitch_id,
              *p_switch_id1_len);
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "switch_id = %s", sw_value->value);
          (*i).p_table_attribute_value = sw_value;
        }
        break;
      case SWITCH_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_128+1],
              desc_value);
          ODBCM_MEMCPY(
              desc_value->value,
              p_switch_table->szdescription,
              sizeof(p_switch_table->szdescription));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "description = %s", desc_value->value);
          (*i).p_table_attribute_value = desc_value;
        }
        break;
      case SWITCH_MODEL:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_16) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_16+1],
              model_value);
          ODBCM_MEMCPY(
              model_value->value,
              p_switch_table->szmodel,
              sizeof(p_switch_table->szmodel));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "model = %s", model_value->value);
          (*i).p_table_attribute_value = model_value;
        }
        break;
      case SWITCH_IP_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_IPV4) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint32_t, ip_value);
          ip_value->value = p_switch_table->szip_address;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "from db ip_address = %s",
              ODBCMUtils::get_ip_string(p_switch_table->szip_address).c_str());
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "ip_address = %s",
              ODBCMUtils::get_ip_string(ip_value->value).c_str());
          (*i).p_table_attribute_value = ip_value;
        }
        break;
      case SWITCH_IPV6_ADDRESS:
        if ((*i).request_attribute_type == DATATYPE_IPV6) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_16+1],
              ipv6_value);
          ODBCM_MEMCPY(
             ipv6_value->value,
             p_switch_table->szipv6_address,
             *p_ipv6_len);
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE:"
              "szIpv6_address=%s", (ODBCMUtils::get_ipv6_string(
                      p_switch_table->szipv6_address)).c_str());
          (*i).p_table_attribute_value = ipv6_value;
        }
        break;
      case SWITCH_ADMIN_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, admin_status);
          admin_status->value = p_switch_table->sadmin_status;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "sadmin_status = %d", admin_status->value);
          (*i).p_table_attribute_value = admin_status;
        }
        break;
      case SWITCH_DOMAIN_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_32+1],
              val_domain_name);
          ODBCM_MEMCPY(
              val_domain_name->value,
              p_switch_table->szdomain_name,
              sizeof(p_switch_table->szdomain_name));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "domain_name = %s" , val_domain_name->value);
          (*i).p_table_attribute_value = val_domain_name;
        }
        break;
      case SWITCH_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, oper_status_value);
          oper_status_value->value = p_switch_table->soper_status;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "oper_status = %d", oper_status_value->value);
          (*i).p_table_attribute_value = oper_status_value;
        }
        break;
      case SWITCH_MANUFACTURER:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_256+1],
              manufacturer_value);
          ODBCM_MEMCPY(
              manufacturer_value->value,
              p_switch_table->szmanufacturer,
              sizeof(p_switch_table->szmanufacturer));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "manufacturer = %s", manufacturer_value->value);
          (*i).p_table_attribute_value = manufacturer_value;
        }
        break;
      case SWITCH_HARDWARE:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_256+1],
              hardware_value);
          ODBCM_MEMCPY(
              hardware_value->value,
              p_switch_table->szhardware,
              sizeof(p_switch_table->szhardware));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "hardware = %s", hardware_value->value);
          (*i).p_table_attribute_value = hardware_value;
        }
        break;
      case SWITCH_SOFTWARE:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_256+1],
              software_value);
          ODBCM_MEMCPY(
              software_value->value,
              p_switch_table->szsoftware,
              sizeof(p_switch_table->szsoftware));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "software = %s", software_value->value);
          (*i).p_table_attribute_value = software_value;
        }
        break;
      case SWITCH_ALARM_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT64) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint64_t,
              alarms_status_value);
          alarms_status_value->value = p_switch_table->salarms_status;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "alarms_status = %" PFC_PFMT_u64, alarms_status_value->value);
          (*i).p_table_attribute_value = alarms_status_value;
        }
        break;
      case SWITCH_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_11) {
           /**ColumnAttrValue is a template to send the fetched values to
            * caller. typecast it into void*, memory will be allocated
            * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_11+1],
              valid_value);
          ODBCM_MEMCPY(
             valid_value->value,
             p_switch_table->svalid,
             sizeof(p_switch_table->svalid));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:SWITCH_TABLE: "
              "valid = %s", valid_value->value);
          (*i).p_table_attribute_value = valid_value;
        }
        break;
      default:
        break;
    }
  }
  return ODBCM_RC_SUCCESS;
}
/**EOF*/
