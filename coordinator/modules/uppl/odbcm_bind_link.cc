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
 *  @file    odbcm_bind_link.cc
 */

#include "odbcm_db_varbind.hh"
#include "odbcm_common.hh"
#include "odbcm_utils.hh"
#include "odbcm_db_tableschema.hh"
#include "odbcm_mgr.hh"

using unc::uppl::DBVarbind;

/**
 * @Description : Function to bind input parameter of link_table
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 *                r_hstmt     - statement handler which carries the SQL Query
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::bind_link_table_input(
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
            p_link_table->szcontroller_name/*buffer to carry values*/,
            sizeof(p_link_table->szcontroller_name)-1/**buffer length*/,
            NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case LINK_SWITCH_ID1:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          /**binding structure buffer member for switch_id1 input,
          * column size is ODBCM_SIZE_256,
          * Data type CHAR[256], this char data will be converted into
          * binary before store into database table. switch_id may have non
          * printable characters as well, To allow non printable character
          * from 0-255, the binary is chose*/
          *p_switch_id1_len = strlen((const char*)p_link_table->szswitch_id1);
          odbc_rc = BindInputParameter_SQL_BINARY(
            r_hstmt/**sql statement handler*/,
            ++col_no/*parameter number (sequential order)*/,
            ODBCM_SIZE_256/*column size in DB table*/,
            0/**decimal point */,
            p_link_table->szswitch_id1/*buffer to carry values*/,
            sizeof(p_link_table->szswitch_id1)-1,
            p_switch_id1_len/**strlen or NULL*/);
            /**set flag value 0 to print column binding details */
            log_flag = 0;
        }
        break;
      case LINK_PORT_ID1:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**binding structure buffer member for port_id1,
          * column size is ODBCM_SIZE_32,
          * data type CHAR[32]*/
          odbc_rc = BindInputParameter_SQL_VARCHAR(
            r_hstmt,
            ++col_no,
            ODBCM_SIZE_32,
            0,
            p_link_table->szport_id1,
            sizeof(p_link_table->szport_id1)-1,
            NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case LINK_SWITCH_ID2:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          *p_switch_id2_len = strlen((const char*)p_link_table->szswitch_id2);
          odbc_rc = BindInputParameter_SQL_BINARY(
            r_hstmt,
            ++col_no,
            ODBCM_SIZE_256,
            0,
            p_link_table->szswitch_id2,
            ODBCM_SIZE_256,
            p_switch_id2_len);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case LINK_PORT_ID2:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
            r_hstmt,
            ++col_no,
            ODBCM_SIZE_32,
            0,
            p_link_table->szport_id2,
            sizeof(p_link_table->szport_id2)-1,
            NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case LINK_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
            r_hstmt,
            ++col_no,
            ODBCM_SIZE_128,
            0,
            p_link_table->szdescription,
            sizeof(p_link_table->szdescription)-1,
            NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case LINK_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
            r_hstmt,
            ++col_no,
            0,
            0,
            reinterpret_cast<SQLSMALLINT*>(&p_link_table->soper_status),
            0,
            NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case LINK_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_2) {
          odbc_rc = BindInputParameter_SQL_CHAR(
            r_hstmt,
            ++col_no,
            ODBCM_SIZE_2,
            0,
            p_link_table->svalid,
            sizeof(p_link_table->svalid)-1,
            NULL);
          /**set flag value 0 to print column binding details */
         log_flag = 0;
       }
       break;
      default:
        break;
    }
    /*In case of bind failure return the parameter bind error */
    if (odbc_rc == SQL_ERROR || odbc_rc == SQL_INVALID_HANDLE) {
      pfc_log_error("ODBCM::DBVarbind::bind_link_table_input"
          "link_table bind parameter error");
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
      pfc_log_debug("ODBCM::**NO bind**i/p:LINK_TABLE:%s:datatype=%d:"
          , ODBCManager::get_ODBCManager()->GetColumnName(
          ((*i).table_attribute_name)).c_str(),
          (*i).request_attribute_type);
    }
  }  // for loop end
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : Output binding function for link_table
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 *                r_hstmt     - statement handler which carries the SQL Query
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::bind_link_table_output(
    std::vector<TableAttrSchema> &column_attr/*DBTableSchema->rowlist_ entry*/,
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
              p_link_table->szcontroller_name/*buffer to fetch values*/,
              ODBCM_SIZE_32+1,
          /**no.of bytes available to return*/
            (&p_link_table->cbname));
          /**set flag value 0 to print column binding details */
            log_flag = 0;
        }
        break;
      case LINK_SWITCH_ID1:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          /**binding structure buffer member for controller name output,
          * column size is ODBCM_SIZE_256,
          * Data type CHAR[246],
          * and buffer size will passed as length of value,
          * ptr to indicates available no. of bytes return,
          * Binary type values will be converted and stored into char buffer */
          *p_switch_id1_len = strlen((const char*)p_link_table->szswitch_id1);
          odbc_rc = BindCol_SQL_BINARY(
              r_hstmt,
              ++col_no,
              p_link_table->szswitch_id1,
              sizeof(p_link_table->szswitch_id1),
              p_switch_id1_len/*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
            log_flag = 0;
        }
        break;
      case LINK_PORT_ID1:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_link_table->szport_id1, ODBCM_SIZE_32+1,
              (&p_link_table->cbport1)
              /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
            log_flag = 0;
        }
        break;
      case LINK_SWITCH_ID2:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          *p_switch_id2_len = strlen((const char*)p_link_table->szswitch_id2);
          odbc_rc = BindCol_SQL_BINARY(
              r_hstmt,
              ++col_no,
              p_link_table->szswitch_id2,
              sizeof(p_link_table->szswitch_id2),
              p_switch_id2_len/*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
              log_flag = 0;
        }
        break;
      case LINK_PORT_ID2:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_link_table->szport_id2,
              ODBCM_SIZE_32+1,
              (&p_link_table->cbport2)
              /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
              log_flag = 0;
        }
        break;
      case LINK_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_link_table->szdescription,
              ODBCM_SIZE_128+1,
              (&p_link_table->cbdesc)
              /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
              log_flag = 0;
        }
        break;
      case LINK_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              reinterpret_cast<SQLSMALLINT*>(&p_link_table->soper_status),
              sizeof(SQLSMALLINT),
              (&p_link_table->cboperstatus)
              /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
            log_flag = 0;
        }
        break;
      case LINK_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_2) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_link_table->svalid,
              ODBCM_SIZE_2+1,
              (&p_link_table->cbvalid)
              /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
              log_flag = 0;
        }
        break;
      default:
        break;
    }
    /*In case of bind failure return the parameter bind error */
    if (odbc_rc == SQL_ERROR || odbc_rc == SQL_INVALID_HANDLE) {
      pfc_log_error("ODBCM::DBVarbind::bind_link_table_output"
          "link_table bind parameter error");
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
 * @Description : To fill the link_table values into structure.
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::fill_link_table(
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
          ODBCM_MEMSET(p_link_table->szcontroller_name, 0, ODBCM_SIZE_32+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_link_table->szcontroller_name,
              &cn_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:LINK_TABLE: "
            "szController_name = %s", p_link_table->szcontroller_name);
        }
        break;
      case LINK_SWITCH_ID1:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for switch_id1 CHAR[256]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_256]> sid1_value =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_256]>*)
            ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_link_table->szswitch_id1, 0, ODBCM_SIZE_256+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_link_table->szswitch_id1,
              &sid1_value.value,
              (*i).table_attribute_length+1);
          odbcm_debug_info("ODBCM::DBVarbind::fill:LINK_TABLE:"
             "szswitch_id1 = %s", p_link_table->szswitch_id1);
        }
        break;
      case LINK_PORT_ID1:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for port_id1 CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> pn1_value =
              *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_link_table->szport_id1, 0, ODBCM_SIZE_32+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_link_table->szport_id1,
              &pn1_value.value,
              (*i).table_attribute_length);
        odbcm_debug_info("ODBCM::DBVarbind::fill:LINK_TABLE: szport_id1 = %s",
            p_link_table->szport_id1);
        }
        break;
      case LINK_SWITCH_ID2:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for switch_id2 CHAR[256]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_256]> sid2_value =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_256]>*)
            ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_link_table->szswitch_id2, 0, ODBCM_SIZE_256+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_link_table->szswitch_id2,
              &sid2_value.value,
              (*i).table_attribute_length+1);
        odbcm_debug_info("ODBCM::DBVarbind::fill:LINK_TABLE: szswitch_id2 = %s",
            p_link_table->szswitch_id2);
        }
        break;
      case LINK_PORT_ID2:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for port_id2 CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> pn2_value =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
            ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_link_table->szport_id2, 0, ODBCM_SIZE_32+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_link_table->szport_id2,
              &pn2_value.value,
              (*i).table_attribute_length);
        odbcm_debug_info("ODBCM::DBVarbind::fill:LINK_TABLE: szport_id2 = %s",
            p_link_table->szport_id2);
        }
        break;
      case LINK_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for description CHAR[128]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_128]> desc_value =
            *((ColumnAttrValue <uint8_t[128]>*)((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_link_table->szdescription, 0, ODBCM_SIZE_128+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_link_table->szdescription,
              &desc_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:LINK_TABLE: "
              "szDescription = %s", p_link_table->szdescription);
        }
        break;
      case LINK_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for oper_status uint16_t*/
          ColumnAttrValue <uint16_t> oper_st_value =
              *((ColumnAttrValue <uint16_t>*)((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_link_table->soper_status, 0, sizeof(SQLSMALLINT));
          /**copying the value from template to binded buffer */
          p_link_table->soper_status = oper_st_value.value;
        odbcm_debug_info("ODBCM::DBVarbind::fill:LINK_TABLE:soper_status=%d",
          p_link_table->soper_status);
        }
        break;
      case LINK_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_2) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for valid CHAR[2]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_2]> valid_value =
              *((ColumnAttrValue
              <uint8_t[ODBCM_SIZE_2]>*)((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_link_table->svalid, 0, ODBCM_SIZE_2+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_link_table->svalid,
              &valid_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:LINK_TABLE: "
            "sValid = %s", p_link_table->svalid);
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
 * @Description : To fetch the link_table values
 *                (which are stored in binded buffer) 
 *                and store into TableAttSchema
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::fetch_link_table(
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
              p_link_table->szcontroller_name,
              sizeof(p_link_table->szcontroller_name));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:LINK_TABLE: "
              "controller_name = %s" , val_controller_name->value);
          (*i).p_table_attribute_value = val_controller_name;
        }
        break;
      case LINK_SWITCH_ID1:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_256+1],
              val_switch_id1);
          ODBCM_MEMCPY(
              val_switch_id1->value,
              p_link_table->szswitch_id1,
              *p_switch_id1_len);
          odbcm_debug_info("ODBCM::DBVarbind::fetch:LINK_TABLE: "
              "switch_id1 = %s" , val_switch_id1->value);
          (*i).p_table_attribute_value = val_switch_id1;
        }
        break;
      case LINK_PORT_ID1:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_32+1],
              val_port_name1);
          ODBCM_MEMCPY(
              val_port_name1->value,
              p_link_table->szport_id1,
              sizeof(p_link_table->szport_id1));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:LINK_TABLE: "
              "port_name1 = %s" , val_port_name1->value);
          (*i).p_table_attribute_value = val_port_name1;
        }
        break;
      case LINK_SWITCH_ID2:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_256+1],
              val_switch_id2);
          ODBCM_MEMCPY(
              val_switch_id2->value,
              p_link_table->szswitch_id2,
              *p_switch_id2_len);
          odbcm_debug_info("ODBCM::DBVarbind::fetch:LINK_TABLE: "
              "switch_id2 = %s" , val_switch_id2->value);
          (*i).p_table_attribute_value = val_switch_id2;
        }
        break;
      case LINK_PORT_ID2:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_32+1],
              val_port_name2);
          ODBCM_MEMCPY(
              val_port_name2->value,
              p_link_table->szport_id2,
              sizeof(p_link_table->szport_id2));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:LINK_TABLE: "
              "port_name2 = %s" , val_port_name2->value);
          (*i).p_table_attribute_value = val_port_name2;
        }
        break;
      case LINK_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_128+1],
              desc_value);
          ODBCM_MEMCPY(
              desc_value->value,
              p_link_table->szdescription,
              sizeof(p_link_table->szdescription));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:LINK_TABLE: "
              "description = %s", desc_value->value);
          (*i).p_table_attribute_value = desc_value;
        }
        break;
      case LINK_OPER_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, oper_status_value);
              oper_status_value->value = p_link_table->soper_status;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:LINK_TABLE: "
              "soper_status = %d", oper_status_value->value);
          (*i).p_table_attribute_value = oper_status_value;
        }
        break;
      case LINK_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_2) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
        ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_2+1], valid_value);
        ODBCM_MEMCPY(
             valid_value->value,
             p_link_table->svalid,
             sizeof(p_link_table->svalid));
        odbcm_debug_info("ODBCM::DBVarbind::fetch:LINK_TABLE: "
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
