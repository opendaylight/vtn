/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *  @brief   ODBC Manager
 *  @file    odbcm_bind_logicalport.cc
 */

#include "odbcm_db_varbind.hh"
#include "odbcm_common.hh"
#include "odbcm_utils.hh"
#include "odbcm_db_tableschema.hh"

using unc::uppl::DBVarbind;

/**
 * @Description : function to bind input param of logicalport_table
 * +1 is added where ever the char array is used to store '\0'.
 * @param[in]   : vector<TableAttrSchema> &column_attr, HSTMT &r_hstmt
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS DBVarbind::bind_logicalport_table_input(
    std::vector<TableAttrSchema> &column_attr/*DBTableSchema->rowlist_ entry*/,
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
  /*In case of bind failure return the parameter bind error */
    if (odbc_rc == SQL_ERROR||odbc_rc == SQL_INVALID_HANDLE) {
      pfc_log_error("ODBCM::DBVarbind::logicalport_common_table"
          "bind parameter error  ");
      /**in case of error while binding return to application caller with the
       * error code */
      return ODBCM_RC_PARAM_BIND_ERROR;
    }

    ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_32/**data type char(32)*/,
        CTR_NAME /*attribute name MACRO*/) {
      /**binding structure buffer member for controller name input,
       * column size is ODBCM_SIZE_32,
       * Data type CHAR[32],
       * and buffer size will passed as length of value */
        odbc_rc = BindInputParameter_SQL_VARCHAR(
          r_hstmt/**sql statement handler*/,
          ++col_no /*parameter number (sequential order)*/,
          ODBCM_SIZE_32/*column size in DB table*/,
          0/**decimal point */,
          p_logicalport_table->szController_name /*buffer to carry values*/,
          sizeof(p_logicalport_table->szController_name)-1/**buffer length*/,
          NULL/**strlen or NULL*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_32/**data type char(32)*/,
        DOMAIN_NAME /*attribute name MACRO*/) {
      /**binding structure buffer member for domain name input,
       * column size is ODBCM_SIZE_32,
       * Data type CHAR[32],
       * and buffer size will passed as length of value */
        odbc_rc = BindInputParameter_SQL_VARCHAR(
          r_hstmt/**sql statement handler*/,
          ++col_no /*parameter number (sequential order)*/,
          ODBCM_SIZE_32/*column size in DB table*/,
          0/**decimal point */,
          p_logicalport_table->szdomain_name /*buffer to carry values*/,
          sizeof(p_logicalport_table->szdomain_name)-1/**buffer length*/,
          NULL/**strlen or NULL*/);
      /**set flag value 0 to print column binding details */
          log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_320/**data type char(320)*/,
        LP_PORT_ID/*attribute name MACRO*/) {
      /**binding structure buffer member for port_id input,
       * column size is ODBCM_SIZE_320,
       * Data type CHAR[320], this char data will be converted into
       * binary before store into database table. switch_id may have non
       * printable characters as well, To allow non printable character
       * from 0-320, the binary is chose*/
      *p_logicalport_id1_len =
          strlen((const char*)p_logicalport_table->szport_id);
      odbc_rc = BindInputParameter_SQL_BINARY(
        r_hstmt/**sql statement handler*/,
        ++col_no/*parameter number (sequential order)*/,
        ODBCM_SIZE_320/*column size in DB table*/,
        0/**decimal point */,
        p_logicalport_table->szport_id/*buffer to carry values*/,
        sizeof(p_logicalport_table->szport_id)-1,
        p_logicalport_id1_len /**strlen or NULL*/);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_128/**data type char(128)*/,
        LP_DESCRIPTION/*attribute name MACRO*/) {
      odbc_rc = BindInputParameter_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_128,
          0,
          p_logicalport_table->szdescription,
          sizeof(p_logicalport_table->szdescription)-1,
          NULL);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT16/**data type char(16)*/,
        LP_PORT_TYPE/*attribute name MACRO*/) {
      odbc_rc = BindInputParameter_SQL_SMALLINT(
           r_hstmt,
           ++col_no,
           0,
           0,
           reinterpret_cast<SQLSMALLINT*>(&p_logicalport_table->sport_type),
           0,
           NULL);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_256/**data type char(256)*/,
        LP_SWITCH_ID/*attribute name MACRO*/) {
      *p_switch_id1_len = strlen((const char*)p_logicalport_table->szswitch_id);
      odbc_rc = BindInputParameter_SQL_BINARY(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_256,
          0,
          p_logicalport_table->szswitch_id,
          sizeof(p_logicalport_table->szswitch_id)-1,
          p_switch_id1_len);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_32/**data type char(32)*/,
         LP_PHYSICAL_PORT_ID/*attribute name MACRO*/) {
      odbc_rc = BindInputParameter_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_32,
          0,
          p_logicalport_table->szphysical_port_id,
          sizeof(p_logicalport_table->szphysical_port_id)-1,
          NULL);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT16/**data type char(16)*/,
        LP_OPER_DOWN_CRITERIA/*attribute name MACRO*/) {
      odbc_rc = BindInputParameter_SQL_SMALLINT(
          r_hstmt,
          ++col_no,
          0,
          0,
          reinterpret_cast<SQLSMALLINT*>
          (&p_logicalport_table->soper_down_criteria),
          0,
          NULL);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT16/**data type char(16)*/,
        LP_OPER_STATUS/*attribute name MACRO*/) {
      odbc_rc = BindInputParameter_SQL_SMALLINT(
          r_hstmt,
          ++col_no,
          0,
          0,
          reinterpret_cast<SQLSMALLINT*>(&p_logicalport_table->soper_status),
          0,
          NULL);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    }  ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
          DATATYPE_UINT8_ARRAY_6/**data type char(6)*/,
          LP_CTR_VALID/*attribute name MACRO*/) {
      odbc_rc = BindInputParameter_SQL_CHAR(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_6,
          0,
          p_logicalport_table->svalid,
          sizeof(p_logicalport_table->svalid)-1,
          NULL);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    }
    if ((*i).p_table_attribute_value != NULL && log_flag == 0) {
      if (odbc_rc != SQL_SUCCESS)
        ODBCMUtils::OdbcmHandleDiagnosticsPrint(SQL_HANDLE_STMT, r_hstmt);
      /**reset flag value 1 */
      log_flag = 1;
    } else {
      pfc_log_info("ODBCM::**NO bind**i/p:LOGICALPORT_TABLE:%s:datatype=%d:"
          , ((*i).table_attribute_name).c_str(),
          (*i).request_attribute_type);
    }
  }  // for loop
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : Output binding function for logicalport_table
 * +1 is added where ever the char array is used to store '\0'.
 * @param[in]   : vector<TableAttrSchema> &column_attr, HSTMT &r_hstmt
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS DBVarbind::bind_logicalport_table_output(
    std::vector<TableAttrSchema> &column_attr/*DBTableSchema->rowlist_ entry*/,
        HSTMT &r_hstmt/**statement handler which carries the SQL Query*/) {
  SQLRETURN odbc_rc = SQL_SUCCESS;  // odbc APIs return code
  uint16_t col_no = 0;  // column number
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator i;
  /**Flag to decide printing logs */
  uint8_t log_flag = 1;
  /** Pointer to value that indicates the number of bytes available to return*/
  SQLINTEGER  indptr = 0;
   /**Loop for iterate all the elements in the vector, the TableAttrSchema
   * table_attribute_name value will be compared and corresponding
   * structure member will be binded here*/
  for (i = column_attr.begin(); i != column_attr.end(); ++i) {
    /*In case of bind failure return the parameter bind error */
    if (odbc_rc == SQL_ERROR || odbc_rc == SQL_INVALID_HANDLE) {
      pfc_log_error("ODBCM::DBVarbind::bind_logicalport_table_output"
          "logicalport_table bind parameter error");
      /**in case of error while binding return to application caller with the
       * error code */
      return ODBCM_RC_PARAM_BIND_ERROR;
    }
    /**binding structure buffer member for controller name output,
     * column size is ODBCM_SIZE_32,
     * Data type CHAR[32],
     * and buffer size will passed as length of value,
     * ptr to indicates available no. of bytes return */
    ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_32,
        CTR_NAME/*attribute name MACRO*/) {
      odbc_rc = BindCol_SQL_VARCHAR(
          r_hstmt/**sql statement handler*/,
          ++col_no/*parameter number (sequential order)*/,
          p_logicalport_table->szController_name/*buffer to fetch values*/,
          ODBCM_SIZE_32+1,
          /**no.of bytes available to return*/
          reinterpret_cast<SQLLEN*>(&indptr));
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_32,
        DOMAIN_NAME/*attribute name MACRO*/) {
      odbc_rc = BindCol_SQL_VARCHAR(
          r_hstmt/**sql statement handler*/,
          ++col_no/*parameter number (sequential order)*/,
          p_logicalport_table->szdomain_name/*buffer to fetch values*/,
          ODBCM_SIZE_32+1,
          /**no.of bytes available to return*/
          reinterpret_cast<SQLLEN*>(&indptr));
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_320,
        LP_PORT_ID/*attribute name MACRO*/) {
      /**binding structure buffer member for controller name output,
       * column size is ODBCM_SIZE_320,
       * Data type CHAR[320],
       * and buffer size will passed as length of value,
       * ptr to indicates available no. of bytes return,
       * Binary type values will be converted and stored into char buffer */
      odbc_rc = BindCol_SQL_BINARY(
          r_hstmt,
          ++col_no,
          p_logicalport_table->szport_id,
          sizeof(p_logicalport_table->szport_id)-1,
          p_logicalport_id1_len/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_128,
        LP_DESCRIPTION/*attribute name MACRO*/) {
      odbc_rc = BindCol_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          p_logicalport_table->szdescription,
          ODBCM_SIZE_128+1,
          reinterpret_cast<SQLLEN*>(&indptr)/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT16,
        LP_PORT_TYPE/*attribute name MACRO*/) {
      odbc_rc = BindCol_SQL_SMALLINT(r_hstmt,
          ++col_no,
          reinterpret_cast<SQLSMALLINT*>(&p_logicalport_table->sport_type),
          sizeof(SQLSMALLINT),
          reinterpret_cast<SQLLEN*>(&indptr)/*buffer to fetch values*/);
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
       DATATYPE_UINT8_ARRAY_256,
       LP_SWITCH_ID/*attribute name MACRO*/) {
      odbc_rc = BindCol_SQL_BINARY(
          r_hstmt,
          ++col_no,
          p_logicalport_table->szswitch_id,
          sizeof(p_logicalport_table->szswitch_id)-1,
          p_switch_id1_len/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_32,
        LP_PHYSICAL_PORT_ID /*attribute name MACRO*/) {
      odbc_rc = BindCol_SQL_VARCHAR(r_hstmt,
          ++col_no,
          p_logicalport_table->szphysical_port_id,
          ODBCM_SIZE_32+1,
          reinterpret_cast<SQLLEN*>(&indptr)/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT16,
        LP_OPER_DOWN_CRITERIA/*attribute name MACRO*/) {
      odbc_rc = BindCol_SQL_SMALLINT(r_hstmt,
          ++col_no,
          reinterpret_cast<SQLSMALLINT*>
          (&p_logicalport_table->soper_down_criteria),
          sizeof(SQLSMALLINT),
          reinterpret_cast<SQLLEN*>(&indptr)/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT16,
        LP_OPER_STATUS/*attribute name MACRO*/) {
      odbc_rc = BindCol_SQL_SMALLINT(r_hstmt,
          ++col_no,
          reinterpret_cast<SQLSMALLINT*>(&p_logicalport_table->soper_status),
          sizeof(SQLSMALLINT),
          reinterpret_cast<SQLLEN*>(&indptr)/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
       DATATYPE_UINT8_ARRAY_6,
       LP_CTR_VALID/*attribute name MACRO*/) {
      odbc_rc = BindCol_SQL_VARCHAR(r_hstmt,
          ++col_no,
          p_logicalport_table->svalid,
          ODBCM_SIZE_6+1,
          reinterpret_cast<SQLLEN*>(&indptr)/*buffer to fetch values*/);
      /**set flag value 0 to print column binding details */
      log_flag = 0;
    }
    if (log_flag == 0) {
      /**reset flag value 1*/
      log_flag = 1;
    }
  }
  return ODBCM_RC_SUCCESS;
}
/**
 * @Description : To fill the logicalport_table values into structure.
 * @param[in]   : std::vector<TableAttrSchema> &column_attr
 * @return      : ODBCM_RC_STATUS
 **/

ODBCM_RC_STATUS DBVarbind::fill_logicalport_table(
    std::vector<TableAttrSchema> &column_attr
    /*DBTableSchema->rowlist_ entry*/) {
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator i;
  /**Loop for iterate all the elements in the vector, the TableAttrSchema
  * table_attribute_name value will be compared and corresponding
  * structure member will be filled*/
  for (i = column_attr.begin(); i != column_attr.end(); ++i) {
    ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
      DATATYPE_UINT8_ARRAY_32/**data type char(32)*/,
      CTR_NAME/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for controller_name CHAR[32]*/
      ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> cn_value =
        *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
        ((*i).p_table_attribute_value));
      /**clear the allocated buffer memory to receive the controller_name
       * from caller*/
      ODBCM_MEMSET(p_logicalport_table->szController_name, 0, ODBCM_SIZE_32+1);
      /**copying the value from template to binded buffer */
      ODBCM_MEMCPY(
          p_logicalport_table->szController_name,
          &cn_value.value,
          (*i).table_attribute_length);
      odbcm_debug_info("ODBCM::DBVarbind::fill:LOGICALPORT_TABLE: "
          "szController_name = %s", p_logicalport_table->szController_name);
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
      DATATYPE_UINT8_ARRAY_32/**data type char(32)*/,
      DOMAIN_NAME/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for domain_name CHAR[32]*/
      ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> dn_value =
        *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
        ((*i).p_table_attribute_value));
      /**clear the allocated buffer memory to receive the domain_name
       * from caller*/
      ODBCM_MEMSET(p_logicalport_table->szdomain_name, 0, ODBCM_SIZE_32+1);
      /**copying the value from template to binded buffer */
      ODBCM_MEMCPY(
          p_logicalport_table->szdomain_name,
          &dn_value.value,
          (*i).table_attribute_length);
      odbcm_debug_info("ODBCM::DBVarbind::fill:LOGICALPORT_TABLE: "
          "szdomain_name=%s", p_logicalport_table->szdomain_name);
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
       DATATYPE_UINT8_ARRAY_320,
       LP_PORT_ID/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for port_id CHAR[320]*/
      ColumnAttrValue <uint8_t[ODBCM_SIZE_320]> port_id_value =
        *((ColumnAttrValue <uint8_t[ODBCM_SIZE_320]>*)
            ((*i).p_table_attribute_value));
      ODBCM_MEMSET(p_logicalport_table->szport_id, 0, ODBCM_SIZE_320+1);
      /**copying the value from template to binded buffer */
      ODBCM_MEMCPY(
          p_logicalport_table->szport_id,
          &port_id_value.value,
          (*i).table_attribute_length);
      odbcm_debug_info("ODBCM::DBVarbind::fill:LOGICALPORT_TABLE: "
          "szport_id = %s", p_logicalport_table->szport_id);
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_128,
        LP_DESCRIPTION/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for description CHAR[128]*/
      ColumnAttrValue <uint8_t[ODBCM_SIZE_128]> desc_value =
        *((ColumnAttrValue <uint8_t[128]>*)((*i).p_table_attribute_value));
      ODBCM_MEMSET(p_logicalport_table->szdescription, 0, ODBCM_SIZE_128+1);
      /**copying the value from template to binded buffer */
      ODBCM_MEMCPY(
          p_logicalport_table->szdescription,
          &desc_value.value,
          (*i).table_attribute_length);
      odbcm_debug_info("ODBCM::DBVarbind::fill:LOGICALPORT_TABLE: "
          "szDescription = %s", p_logicalport_table->szdescription);
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT16,
        LP_PORT_TYPE/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for port_type uint16_t*/
      ColumnAttrValue <uint16_t> port_type_value =
        *((ColumnAttrValue <uint16_t>*)((*i).p_table_attribute_value));
       ODBCM_MEMSET(&p_logicalport_table->sport_type, 0, sizeof(SQLSMALLINT));
       /**copying the value from template to binded buffer */
      p_logicalport_table->sport_type = port_type_value.value;
      odbcm_debug_info("ODBCM::DBVarbind::fill:LOGICALPORT_TABLE:sport_type=%d",
          p_logicalport_table->sport_type);
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_256,
        LP_SWITCH_ID/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for switch_id CHAR[256]*/
      ColumnAttrValue <uint8_t[ODBCM_SIZE_256]> sid_value =
        *((ColumnAttrValue <uint8_t[ODBCM_SIZE_256]>*)
            ((*i).p_table_attribute_value));
      ODBCM_MEMSET(p_logicalport_table->szswitch_id, 0, ODBCM_SIZE_256+1);
      /**copying the value from template to binded buffer */
      ODBCM_MEMCPY(
          p_logicalport_table->szswitch_id,
          &sid_value.value,
          (*i).table_attribute_length);
      odbcm_debug_info("ODBCM::DBVarbind::fill:LOGICALPORT_TABLE: "
          "szswitch_id = %s", p_logicalport_table->szswitch_id);
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_32,
        LP_PHYSICAL_PORT_ID/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for physical_port_id CHAR[32]*/
      ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> pid_value =
        *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
            ((*i).p_table_attribute_value));
      ODBCM_MEMSET(p_logicalport_table->szphysical_port_id, 0, ODBCM_SIZE_32+1);
      /**copying the value from template to binded buffer */
      ODBCM_MEMCPY(
          p_logicalport_table->szphysical_port_id,
          &pid_value.value,
          (*i).table_attribute_length);
      odbcm_debug_info("ODBCM::DBVarbind::fill:LOGICALPORT_TABLE:"
          "szphysical_port_id = %s", p_logicalport_table->szphysical_port_id);
     } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT16,
        LP_OPER_DOWN_CRITERIA/*attribute name MACRO*/) {
     /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for oper_down_criteria uint16_t*/
      ColumnAttrValue <uint16_t> oper_down_criteria_value =
        *((ColumnAttrValue <uint16_t>*)((*i).p_table_attribute_value));
     /**copying the value from template to binded buffer */
      p_logicalport_table->soper_down_criteria =
          oper_down_criteria_value.value;
      odbcm_debug_info("ODBCM::DBVarbind::fill:LOGICALPORT_TABLE:"
          "oper_down_criteria =%d",
          p_logicalport_table->soper_down_criteria);
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT16,
        LP_OPER_STATUS/*attribute name MACRO*/) {
     /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for oper_status uint16_t*/
      ColumnAttrValue <uint16_t> oper_st_value =
        *((ColumnAttrValue <uint16_t>*)((*i).p_table_attribute_value));
     /**copying the value from template to binded buffer */
      p_logicalport_table->soper_status = oper_st_value.value;
      odbcm_debug_info("ODBCM::DBVarbind::fill:LOGICALPORT_TABLE:"
          "soper_status=%d", p_logicalport_table->soper_status);
    } ODBCM_COMPARE_ATTRNAME_DATATYPE((*i)/**TableAttrSchema structure*/,
       DATATYPE_UINT8_ARRAY_6,
       LP_CTR_VALID/*attribute name MACRO*/) {
     /**ColumnAttrValue is a template to receive the void* values from
       * caller and typecast it into appropriate data type,
       * for valid CHAR[6]*/
      ColumnAttrValue <uint8_t[ODBCM_SIZE_6]> valid_value = *((ColumnAttrValue
            <uint8_t[ODBCM_SIZE_6]>*)((*i).p_table_attribute_value));
      /**copying the value from template to binded buffer */
      ODBCM_MEMSET(p_logicalport_table->svalid, 0, ODBCM_SIZE_6+1);
      ODBCM_MEMCPY(p_logicalport_table->svalid, &valid_value.value,
          (*i).table_attribute_length);
      odbcm_debug_info("ODBCM::DBVarbind::fill:LOGICALPORT_TABLE: "
          "sValid = %s", p_logicalport_table->svalid);
    }
  }
  /*return the final status to caller*/
  return ODBCM_RC_SUCCESS;
}



/**
 * @Description : To fetch the logicalport_common_table values
 * (which are stored in binded buffer) and store into TableAttSchema
 * @param[in]   : vector<TableAttrSchema> &column_attr
 * @return      : ODBCM_RC_STATUS
 **/
ODBCM_RC_STATUS DBVarbind::fetch_logicalport_table(
    std::vector<TableAttrSchema> &column_attr
  /*DBTableSchema->rowlist_ entry*/) {
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator i;
  /**Loop for iterate all the elements in the vector, the TableAttrSchema
   * table_attribute_name value will be compared and corresponding
   * structure member will be fetched */
  for (i = column_attr.begin(); i != column_attr.end(); ++i) {
    ODBCM_COMPARE_FETCH_DATA((*i)/**TableAttrSchema structure*/,
      DATATYPE_UINT8_ARRAY_32/**data type char(32)*/,
      CTR_NAME/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
      ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
          uint8_t[ODBCM_SIZE_32+1],
          val_controller_name);
      ODBCM_MEMCPY(
          val_controller_name->value,
          p_logicalport_table->szController_name,
          sizeof(p_logicalport_table->szController_name));
      odbcm_debug_info("ODBCM::DBVarbind::fetch:LOGICALPORT_TABLE: "
          "controller_name = %s" , val_controller_name->value);
      (*i).p_table_attribute_value = val_controller_name;
    } ODBCM_COMPARE_FETCH_DATA((*i)/**TableAttrSchema structure*/,
       DATATYPE_UINT8_ARRAY_32/**data type char(32)*/,
       DOMAIN_NAME/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
      ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
          uint8_t[ODBCM_SIZE_32+1],
          val_domain_name);
      ODBCM_MEMCPY(
          val_domain_name->value,
          p_logicalport_table->szdomain_name,
          sizeof(p_logicalport_table->szdomain_name));
      odbcm_debug_info("ODBCM::DBVarbind::fetch:LOGICALPORT_TABLE: "
          "szdomain_name = %s" , val_domain_name->value);
      (*i).p_table_attribute_value =val_domain_name;
    } ODBCM_COMPARE_FETCH_DATA((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_320,
        LP_PORT_ID/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
      ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
          uint8_t[ODBCM_SIZE_320+1],
          val_port_id);
      ODBCM_MEMCPY(
          val_port_id->value,
          p_logicalport_table->szport_id,
          *p_logicalport_id1_len);
      odbcm_debug_info("ODBCM::DBVarbind::fetch:LOGICALPORT_TABLE: "
          "port_id = %s" , val_port_id->value);
      (*i).p_table_attribute_value = val_port_id;
    } ODBCM_COMPARE_FETCH_DATA((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_128,
        LP_DESCRIPTION/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
      ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
        uint8_t[ODBCM_SIZE_128+1],
        desc_value);
      ODBCM_MEMCPY(
          desc_value->value,
          p_logicalport_table->szdescription,
          sizeof(p_logicalport_table->szdescription));
      odbcm_debug_info("ODBCM::DBVarbind::fetch:LOGICALPORT_TABLE: "
          "description = %s", desc_value->value);
      (*i).p_table_attribute_value = desc_value;
    } ODBCM_COMPARE_FETCH_DATA((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT16,
        LP_PORT_TYPE/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
      ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, port_type_value);
      port_type_value->value = p_logicalport_table->sport_type;
      odbcm_debug_info("ODBCM::DBVarbind::fetch:LOGICALPORT_TABLE: "
          "port_type = %d", port_type_value->value);
      (*i).p_table_attribute_value = port_type_value;
    } ODBCM_COMPARE_FETCH_DATA((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_256,
        LP_SWITCH_ID/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
      ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
          uint8_t[ODBCM_SIZE_256+1],
          val_switch_id);
      ODBCM_MEMCPY(
          val_switch_id->value,
          p_logicalport_table->szswitch_id,
          *p_switch_id1_len);
      odbcm_debug_info("ODBCM::DBVarbind::fetch:LOGICALPORT_TABLE: "
          "switch_id = %s" , val_switch_id->value);
      (*i).p_table_attribute_value = val_switch_id;
    } ODBCM_COMPARE_FETCH_DATA((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT8_ARRAY_32,
        LP_PHYSICAL_PORT_ID/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
      ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_32+1],
          val_physical_port_id);
      ODBCM_MEMCPY(
          val_physical_port_id->value,
          &p_logicalport_table->szphysical_port_id,
          sizeof(p_logicalport_table->szphysical_port_id));
      odbcm_debug_info("ODBCM::DBVarbind::fetch:LOGICALPORT_TABLE: "
          "physical_port_id = %s" , val_physical_port_id->value);
      (*i).p_table_attribute_value = val_physical_port_id;
    } ODBCM_COMPARE_FETCH_DATA((*i)/**TableAttrSchema structure*/,
        DATATYPE_UINT16,
        LP_OPER_DOWN_CRITERIA/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
      ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, oper_down_criteria_value);
      oper_down_criteria_value->value =
          p_logicalport_table->soper_down_criteria;
      odbcm_debug_info("ODBCM::DBVarbind::fetch:LOGICALPORT_TABLE: "
          "oper_down_criteria = %d", oper_down_criteria_value->value);
      (*i).p_table_attribute_value = oper_down_criteria_value;
    } ODBCM_COMPARE_FETCH_DATA((*i)/**TableAttrSchema structure*/,
         DATATYPE_UINT16,
         LP_OPER_STATUS/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
      ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, oper_status_value);
      oper_status_value->value = p_logicalport_table->soper_status;
      odbcm_debug_info("ODBCM::DBVarbind::fetch:LOGICALPORT_TABLE: "
          "soper_status = %d", oper_status_value->value);
      (*i).p_table_attribute_value = oper_status_value;
    } ODBCM_COMPARE_FETCH_DATA((*i)/**TableAttrSchema structure*/,
         DATATYPE_UINT8_ARRAY_6,
         LP_CTR_VALID/*attribute name MACRO*/) {
      /**ColumnAttrValue is a template to send the fetched values to
        * caller. typecast it into void*, memory will be allocated
        * for the template to send to caller*/
      ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_6+1], valid_value);
      ODBCM_MEMCPY(
          valid_value->value,
         &p_logicalport_table->svalid,
          sizeof(p_logicalport_table->svalid));
      odbcm_debug_info("ODBCM::DBVarbind::fetch:LOGICALPORT_TABLE: "
          "valid = %s", valid_value->value);
      (*i).p_table_attribute_value = valid_value;
    }
  }
  return ODBCM_RC_SUCCESS;
}
/**EOF*/
