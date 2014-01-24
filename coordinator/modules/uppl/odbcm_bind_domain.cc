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
 *  @file    odbcm_bind_domain.cc
 */

#include <stdio.h>
#include <sstream>
#include "odbcm_db_varbind.hh"
#include "odbcm_common.hh"
#include "odbcm_utils.hh"
#include "odbcm_db_tableschema.hh"
#include "odbcm_mgr.hh"

using unc::uppl::DBVarbind;

/*
 * @Description : Function to bind input parameter of Domain_table
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 *                r_hstmt     - statement handler which carries the SQL Query
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::bind_domain_table_input(
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
              p_domain_table->szcontroller_name/*buffer to carry values*/,
              sizeof(p_domain_table->szcontroller_name)-1/**buffer length*/,
              NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              ODBCM_SIZE_32,
              0,
              p_domain_table->szdomain_name,
              sizeof(p_domain_table->szdomain_name)-1,
              NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_TYPE:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLSMALLINT*>
              (&p_domain_table->stype),
              0,
              NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              ODBCM_SIZE_128,
              0,
              p_domain_table->szdescription,
              sizeof(p_domain_table->szdescription)-1,
              NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_OP_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLSMALLINT*>(&p_domain_table->soper_status),
              0,
              NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
          odbc_rc = BindInputParameter_SQL_CHAR(
              r_hstmt,
              ++col_no,
              ODBCM_SIZE_3,
              0,
              p_domain_table->svalid,
              sizeof(p_domain_table->svalid)-1,
              NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_CS_ROW_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              0,
              0,
              reinterpret_cast<SQLSMALLINT*>(&p_domain_table->scs_row_status),
              0,
              NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_CS_ATTR:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
          odbc_rc = BindInputParameter_SQL_CHAR(
              r_hstmt,
              ++col_no,
              ODBCM_SIZE_3,
              0,
              p_domain_table->scs_attr,
              sizeof(p_domain_table->scs_row_status),
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
      pfc_log_error("ODBCM::DBVarbind::ctr_domain_table"
          "bind parameter error");
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
      pfc_log_debug("ODBCM::DBVarbind::No bind i/p:DOMAIN_TABLE:%s:datatype=%d:"
                   , ODBCManager::get_ODBCManager()-> \
                    GetColumnName((*i).table_attribute_name).c_str(),
                    (*i).request_attribute_type);
    }
  }  // for loop end
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : Output binding function for Domain_table
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 *                r_hstmt     - statement handler which carries the SQL Query
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::bind_domain_table_output(
    std::vector<TableAttrSchema> &column_attr/*DBTableSchema->rowlist_entry*/,
    HSTMT &r_hstmt/**statement handler which carries the SQL Query*/) {
  int64_t odbc_rc = SQL_SUCCESS;  // odbc APIs return code
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
    /**binding structure buffer member for controller name output,
     * column size is ODBCM_SIZE_32,
     * Data type CHAR[32],
     * and buffer size will passed as length of value,
     * ptr to indicates available no. of bytes return */

    switch ((*i).table_attribute_name) {
      case CTR_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt/**sql statement handler*/,
              ++col_no/*parameter number (sequential order)*/,
              p_domain_table->szcontroller_name/*buffer to fetch values*/,
              ODBCM_SIZE_32+1,
              /**no.of bytes available to return*/
              (&p_domain_table->cbname));
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_domain_table->szdomain_name,
              ODBCM_SIZE_32+1,
              (&p_domain_table->cbdomain));
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_TYPE:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              reinterpret_cast<SQLSMALLINT*>(&p_domain_table->stype),
              sizeof(SQLSMALLINT),
              (&p_domain_table->cbtype));
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_domain_table->szdescription,
              ODBCM_SIZE_128+1,
              (&p_domain_table->cbdesc));
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_OP_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              reinterpret_cast<SQLSMALLINT*>
              (&p_domain_table->soper_status),
              sizeof(SQLSMALLINT),
              (&p_domain_table->cboperstatus));
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_domain_table->svalid,
              ODBCM_SIZE_3+1,
              (&p_domain_table->cbvalid));
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_CS_ROW_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
              r_hstmt,
              ++col_no,
              reinterpret_cast<SQLSMALLINT*>(&p_domain_table->scs_row_status),
              sizeof(SQLSMALLINT),
              (&p_domain_table->cbrowstatus));
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case DOMAIN_CS_ATTR:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
          odbc_rc = BindCol_SQL_VARCHAR(
              r_hstmt,
              ++col_no,
              p_domain_table->scs_attr,
              ODBCM_SIZE_3+1,
              (&p_domain_table->cbcsattr));
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      default:
        break;
    }
    /*In case of bind failure return the parameter bind error */
    if (odbc_rc == SQL_ERROR||odbc_rc == SQL_INVALID_HANDLE) {
      pfc_log_error("ODBCM::DBVarbind:: ctr_domain_table: "
          "bind parameter error :: %" PFC_PFMT_d64, odbc_rc);
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
 * @Description : To fill the Domain_table values into structure.
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::fill_domain_table(
  std::vector<TableAttrSchema>&column_attr
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
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> domain_cn_value;
          domain_cn_value  = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
              ((*i).p_table_attribute_value));
          /**clear the allocated buffer memory to receive the controller_name
           * from caller*/
          ODBCM_MEMSET(p_domain_table->szcontroller_name, 0, ODBCM_SIZE_32+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_domain_table->szcontroller_name,
              &domain_cn_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:DOMAIN_TABLE:"
              "szController_name = %s",  p_domain_table->szcontroller_name);
        }
        break;
      case DOMAIN_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           * for domain_name CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> domain_dn_value;
          domain_dn_value  = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
              ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_domain_table->szdomain_name, 0, ODBCM_SIZE_32+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_domain_table->szdomain_name,
              &domain_dn_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:DOMAIN_TABLE:"
              "szdomain_name = %s",  p_domain_table->szdomain_name);
        }
        break;
      case DOMAIN_TYPE:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           * for type unit16_t*/
          ColumnAttrValue <uint16_t> type_value;
          type_value =
            *((ColumnAttrValue <uint16_t>*)((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_domain_table->stype, 0,
              sizeof(SQLSMALLINT));
          p_domain_table->stype = type_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:DOMAIN_TABLE:"
              "stype = %d", p_domain_table->stype);
        }
        break;
      case DOMAIN_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           * for description CHAR[128]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_128]> desc_value =
            *((ColumnAttrValue <uint8_t[128]>*)
                ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_domain_table->szdescription, 0, ODBCM_SIZE_128+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_domain_table->szdescription,
              &desc_value.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:DOMAIN_TABLE: "
              "szDescription = %s", p_domain_table->szdescription);
        }
        break;
      case DOMAIN_OP_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           * for oper_status unit16_t*/
          ColumnAttrValue <uint16_t> oper_value;
          oper_value =
            *((ColumnAttrValue <uint16_t>*)((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_domain_table->soper_status, 0,
              sizeof(SQLSMALLINT));
          p_domain_table->soper_status = oper_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:DOMAIN_TABLE:"
              "soper_status = %d", p_domain_table->soper_status);
        }
        break;
      case DOMAIN_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
          /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           * for valid CHAR[3]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_3]> domain_valid_val;
          domain_valid_val =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_3]>*)
                ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_domain_table->svalid, 0, ODBCM_SIZE_3+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_domain_table->svalid,
              &domain_valid_val.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:DOMAIN_TABLE:"
              "sValid = %s", p_domain_table->svalid);
        }
        break;
      case DOMAIN_CS_ROW_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           * for cs_row_status unit16_t*/
          ColumnAttrValue <uint16_t> cs_row_value;
          cs_row_value =
            *((ColumnAttrValue <uint16_t>*)
                ((*i).p_table_attribute_value));
          ODBCM_MEMSET(&p_domain_table->scs_row_status, 0,
              sizeof(SQLSMALLINT));
          p_domain_table->scs_row_status = cs_row_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:DOMAIN_TABLE:"
              "scs_row_status = %d", p_domain_table->scs_row_status);
        }
        break;
      case DOMAIN_CS_ATTR:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
          /**ColumnAttrValue is a template to receive the void* values from
           * caller and typecast it into appropriate data type,
           * for cs_attr CHAR[3]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_3]> domain_attr_val;
          domain_attr_val =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_3]>*)
                ((*i).p_table_attribute_value));
          ODBCM_MEMSET(p_domain_table->scs_attr, 0, ODBCM_SIZE_3+1);
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_domain_table->scs_attr,
              &domain_attr_val.value,
              (*i).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:DOMAIN_TABLE:"
              "scs_attr = %s", p_domain_table->scs_attr);
        }
        break;
      default:
        break;
    }
  }
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : To fetch the Domain_table values
 *                (which are stored in binded buffer)
 *                and store into TableAttSchema
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::fetch_domain_table(
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
           *caller. typecast it into void*, memory will be allocated
           * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_32+1],
              val_controller_name);
          ODBCM_MEMCPY(val_controller_name->value,
              p_domain_table->szcontroller_name,
              sizeof(p_domain_table->szcontroller_name));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:DOMAIN_TABLE: "
              "szController_name = %s", val_controller_name->value);
          (*i).p_table_attribute_value = val_controller_name;
        }
        break;
      case DOMAIN_NAME:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to send the fetched values to
           *caller. typecast it into void*, memory will be allocated
           * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_32+1],
              val_domain_name);
          ODBCM_MEMCPY(
              val_domain_name->value,
              p_domain_table->szdomain_name,
              sizeof(p_domain_table->szdomain_name));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:DOMAIN_TABLE: "
              "szdomain_name = %s", val_domain_name->value);
          (*i).p_table_attribute_value = val_domain_name;
        }
        break;
      case DOMAIN_TYPE:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          /**ColumnAttrValue is a template to send the fetched values to
           *caller. typecast it into void*, memory will be allocated
           * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, type_value);
          type_value->value = p_domain_table->stype;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:DOMAIN_TABLE: "
              "type = %d", type_value->value);
          (*i).p_table_attribute_value = type_value;
        }
        break;
      case DOMAIN_DESCRIPTION:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          /**ColumnAttrValue is a template to send the fetched values to
           *caller. typecast it into void*, memory will be allocated
           * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_128+1],
              val_description);
          ODBCM_MEMCPY(
              val_description->value,
              p_domain_table->szdescription,
              sizeof(p_domain_table->szdescription));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:DOMAIN_TABLE: "
              "szdescription = %s", val_description->value);
          (*i).p_table_attribute_value = val_description;
        }
        break;
      case DOMAIN_OP_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          // reinterpret_cast<void*>(val_description);
          /**ColumnAttrValue is a template to send the fetched values to
           *caller. typecast it into void*, memory will be allocated
           * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, oper_status_value);
          oper_status_value->value = p_domain_table->soper_status;
          // reinterpret_cast<void*>(oper_status_value);
          odbcm_debug_info("ODBCM::DBVarbind::fetch:DOMAIN_TABLE: "
              "soper_status = %d", oper_status_value->value);
          (*i).p_table_attribute_value = oper_status_value;
        }
        break;
      case DOMAIN_VALID:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
          /**ColumnAttrValue is a template to send the fetched values to
           *caller. typecast it into void*, memory will be allocated
           * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_3+1],
              domain_valid_value);
          ODBCM_MEMCPY(
              domain_valid_value->value,
              &p_domain_table->svalid,
              sizeof(p_domain_table->svalid));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:DOMAIN_TABLE: svalid = %s",
              domain_valid_value->value);
          (*i).p_table_attribute_value = domain_valid_value;
        }
        break;
      case DOMAIN_CS_ROW_STATUS:
        if ((*i).request_attribute_type == DATATYPE_UINT16) {
          /**ColumnAttrValue is a template to send the fetched values to
           *caller. typecast it into void*, memory will be allocated
           * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, cs_row_status_value);
          cs_row_status_value->value = p_domain_table->scs_row_status;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:DOMAIN_TABLE: "
              "scs_row_status = %d", cs_row_status_value->value);
          (*i).p_table_attribute_value = cs_row_status_value;
        }
        break;
      case DOMAIN_CS_ATTR:
        if ((*i).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
          /**ColumnAttrValue is a template to send the fetched values to
           *caller. typecast it into void*, memory will be allocated
           * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_3+1],
              domain_attr_value);
          ODBCM_MEMCPY(
              domain_attr_value->value,
              &p_domain_table->scs_attr,
              sizeof(p_domain_table->scs_attr));
        odbcm_debug_info("ODBCM::DBVarbind::fetch:DOMAIN_TABLE: scs_attr = %s",
              domain_attr_value->value);
          (*i).p_table_attribute_value = domain_attr_value;
        }
        break;
      default:
        break;
    }
  }
  return ODBCM_RC_SUCCESS;
}
/**EOF*/
