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
 *  @file    odbcm_bind_boundary.cc
 */

#include "odbcm_db_varbind.hh"
#include "odbcm_common.hh"
#include "odbcm_utils.hh"
#include "odbcm_db_tableschema.hh"
#include "odbcm_mgr.hh"

using unc::uppl::DBVarbind;

/**
 * @Description : Function to bind input parameter of  boundary_table
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 *                r_hstmt     - statement handler which carries the SQL Query
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::bind_boundary_table_input(
  std::vector<TableAttrSchema>& column_attr/*DBTableSchema->rowlist_entry*/,
  HSTMT &r_hstmt/**statement handler which carries the SQL Query*/) {
  SQLRETURN odbc_rc = SQL_SUCCESS;  // odbc API's return code initialize with 0
  SQLUSMALLINT col_no = 0;  // column number
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator v_iter;
  /**Flag to decide printing logs */
  uint8_t log_flag = 1;

  /**Loop for iterate all the elements in the vector, the TableAttrSchema
   * table_attribute_name value will be compared and corresponding
   * structure member will be binded here*/
  for (v_iter = column_attr.begin(); v_iter != column_attr.end(); ++v_iter) {
    if ((*v_iter).p_table_attribute_value == NULL) {
      pfc_log_debug("ODBCM::DBVarbind::bind value structure is empty ");
      continue;
    }
    switch ((*v_iter).table_attribute_name) {
      case BDRY_ID:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**binding structure buffer member for boundart id input,
          * column size is ODBCM_SIZE_32,
          * Data type CHAR[32],
          * and buffer size will passed as length of value */
          odbc_rc = BindInputParameter_SQL_VARCHAR(
          r_hstmt/**sql statement handler*/,
          ++col_no/*parameter number (sequential order)*/,
          ODBCM_SIZE_32/*column size in DB table*/,
          0/**decimal point */,
          p_boundary_table->szboundary_id/*buffer to carry values*/,
          sizeof(p_boundary_table->szboundary_id)-1/**buffer length*/,
          NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_DESCRIPTION:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          /**binding structure buffer member for description input,
          * column size is ODBCM_SIZE_128,
          * Data type CHAR[128], this char data will be converted into
          * binary before store into database table.*/
          odbc_rc = BindInputParameter_SQL_VARCHAR(
          r_hstmt/**sql statement handler*/,
          ++col_no/*parameter number (sequential order)*/,
          ODBCM_SIZE_128/*column size in DB table*/,
          0/**decimal point */,
          p_boundary_table->szdescription/*buffer to carry values*/,
          sizeof(p_boundary_table->szdescription)-1,
          NULL/**strlen or NULL*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_CTR_NAME1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**binding structure buffer member for controller name,
          * column size is ODBCM_SIZE_32,
          * data type CHAR[32]*/
          odbc_rc = BindInputParameter_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_32,
          0,
          p_boundary_table->szcontroller_name1,
          sizeof(p_boundary_table->szcontroller_name1)-1,
          NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_DM_NAME1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_32,
          0,
          p_boundary_table->szdomain_name1,
          sizeof(p_boundary_table-> szdomain_name1)-1,
          NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_PORT_ID1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
          *p_logicalport_id1_len =
          strlen((const char*)p_boundary_table->szlogical_port_id1);
          odbc_rc = BindInputParameter_SQL_BINARY(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_320,
          0,
          p_boundary_table->szlogical_port_id1,
          sizeof(p_boundary_table->szlogical_port_id1)-1,
          p_logicalport_id1_len);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_CTR_NAME2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_32,
          0,
          p_boundary_table->szcontroller_name2,
          sizeof(p_boundary_table-> szcontroller_name2)-1,
          NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_DM_NAME2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindInputParameter_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_32,
          0,
          p_boundary_table->szdomain_name2,
          sizeof(p_boundary_table-> szdomain_name2)-1,
          NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_PORT_ID2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
          *p_logicalport_id2_len =
          strlen((const char*)p_boundary_table->szlogical_port_id2);
          odbc_rc = BindInputParameter_SQL_BINARY(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_320,
          0,
          p_boundary_table->szlogical_port_id2,
          sizeof(p_boundary_table->szlogical_port_id2)-1,
          p_logicalport_id2_len);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_OPER_STATUS:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
          r_hstmt,
          ++col_no,
          0,
          0,
          reinterpret_cast<SQLSMALLINT*>(&p_boundary_table->soper_status),
          0,
          NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_VALID:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_8) {
          odbc_rc = BindInputParameter_SQL_CHAR(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_8,
          0,
          p_boundary_table->svalid,
          sizeof(p_boundary_table->svalid)-1,
          NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_ROW_STATUS:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindInputParameter_SQL_SMALLINT(
          r_hstmt,
          ++col_no,
          0,
          0,
          reinterpret_cast<SQLSMALLINT*>(&p_boundary_table->scs_row_status),
          0,
          NULL);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_ATTR:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_8) {
          odbc_rc = BindInputParameter_SQL_CHAR(
          r_hstmt,
          ++col_no,
          ODBCM_SIZE_8,
          0,
          p_boundary_table->scs_attr,
          sizeof(p_boundary_table->scs_attr)-1,
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
      pfc_log_error("ODBCM::DBVarbind::bind_boundary_table_input"
          "bind parameter error");
      /**in case of error while binding return to application caller with the
       * error code */
      return ODBCM_RC_PARAM_BIND_ERROR;
    }
    if ((*v_iter).p_table_attribute_value != NULL && log_flag == 0) {
      if (odbc_rc != SQL_SUCCESS)
        ODBCMUtils::OdbcmHandleDiagnosticsPrint(SQL_HANDLE_STMT, r_hstmt);
      /**reset flag value 1 */
      log_flag = 1;
    } else {
      pfc_log_debug("ODBCM::**NO bind**i/p:BOUNDARY_TABLE:%s:datatype=%d:",
          ODBCManager::get_ODBCManager()->GetColumnName(
          ((*v_iter).table_attribute_name)).c_str(),
          (*v_iter).request_attribute_type);
    }
  }  // for loop
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : Output binding function for boundary_table
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 *                r_hstmt     - statement handler which carries the SQL Query
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::bind_boundary_table_output(
    std::vector<TableAttrSchema> &column_attr/*DBTableSchema->rowlist_entry*/,
        HSTMT &r_hstmt/**statement handler which carries the SQL Query*/) {
  SQLRETURN   odbc_rc = SQL_SUCCESS;  // odbc APIs return code
  uint16_t    col_no = 0;  // column number
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator v_iter;
  /**Flag to decide printing logs */
  uint8_t log_flag = 1;

  /**Loop for iterate all the elements in the vector, the TableAttrSchema
   * table_attribute_name value will be compared and corresponding
   * structure member will be binded here*/
  for (v_iter = column_attr.begin(); v_iter != column_attr.end(); ++v_iter) {
    if ((*v_iter).p_table_attribute_value == NULL) {
      pfc_log_debug("ODBCM::DBVarbind::bind value structure is empty ");
      continue;
    }

    switch ((*v_iter).table_attribute_name) {
      case BDRY_ID:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**binding structure buffer member for boundary id output,
          * column size is ODBCM_SIZE_32,
          * Data type CHAR[32],
          * and buffer size will passed as length of value,
          * ptr to indicates available no. of bytes return */
          odbc_rc = BindCol_SQL_VARCHAR(
          r_hstmt/**sql statement handler*/,
          ++col_no/*parameter number (sequential order)*/,
          p_boundary_table->szboundary_id/*buffer to fetch values*/,
          ODBCM_SIZE_32+1,
          /**no.of bytes available to return*/
          (&p_boundary_table->cbbid)
          /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
         }
        break;
      case BDRY_DESCRIPTION:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          /**binding structure buffer member for description output,
          * column size is ODBCM_SIZE_128,
          * Data type CHAR[128],
          * and buffer size will passed as length of value,
          * ptr to indicates available no. of bytes return,
          * Binary type values will be converted and stored into char buffer */
          odbc_rc = BindCol_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          p_boundary_table->szdescription,
          ODBCM_SIZE_128+1,
          (&p_boundary_table->cbdesc)
          /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_CTR_NAME1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          p_boundary_table->szcontroller_name1,
          ODBCM_SIZE_32+1,
          (&p_boundary_table->cbctrname1)
          /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_DM_NAME1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          p_boundary_table->szdomain_name1,
          ODBCM_SIZE_32+1,
          (&p_boundary_table->cbdomname1)
          /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_PORT_ID1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
          odbc_rc = BindCol_SQL_BINARY(
          r_hstmt,
          ++col_no,
          p_boundary_table->szlogical_port_id1,
          sizeof(p_boundary_table->szlogical_port_id1)-1,
          p_logicalport_id1_len /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_CTR_NAME2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          p_boundary_table->szcontroller_name2,
          ODBCM_SIZE_32+1,
          (&p_boundary_table->cbctrname2)
          /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_DM_NAME2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          odbc_rc = BindCol_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          p_boundary_table->szdomain_name2,
          ODBCM_SIZE_32+1,
          (&p_boundary_table->cbdomname2)/*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_PORT_ID2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
          odbc_rc = BindCol_SQL_BINARY(
          r_hstmt,
          ++col_no,
          p_boundary_table->szlogical_port_id2,
          sizeof(p_boundary_table->szlogical_port_id2)-1,
          p_logicalport_id2_len /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_OPER_STATUS:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
          r_hstmt,
          ++col_no,
          reinterpret_cast<SQLSMALLINT*>(&p_boundary_table->soper_status),
          sizeof(SQLSMALLINT),
          (&p_boundary_table->cboperstatus)
          /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_VALID:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_8) {
          odbc_rc = BindCol_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          p_boundary_table->svalid,
          ODBCM_SIZE_8+1,
          (&p_boundary_table->cbvalid)
          /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_ROW_STATUS:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT16) {
          odbc_rc = BindCol_SQL_SMALLINT(
          r_hstmt,
          ++col_no,
          reinterpret_cast<SQLSMALLINT*>(&p_boundary_table->scs_row_status),
          sizeof(SQLSMALLINT),
          (&p_boundary_table->cbrowstatus)
          /*buffer to fetch values*/);
          /**set flag value 0 to print column binding details */
          log_flag = 0;
        }
        break;
      case BDRY_ATTR:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_8) {
          odbc_rc = BindCol_SQL_VARCHAR(
          r_hstmt,
          ++col_no,
          p_boundary_table->scs_attr,
          ODBCM_SIZE_8+1,
          (&p_boundary_table->cbcsattr)
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
      pfc_log_error("ODBCM::DBVarbind::bind_boundary_table_output"
          "bind parameter error");
      /**in case of error while binding return to application caller with the
       * error code */
      return ODBCM_RC_PARAM_BIND_ERROR;
    }
    if (log_flag == 0) {
      log_flag = 1;
    }
  }
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : To fill the boundary_table values into structure.
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::fill_boundary_table(
    std::vector<TableAttrSchema> &column_attr
    /*DBTableSchema->rowlist_entry*/) {
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator v_iter;
  /**Loop for iterate all the elements in the vector, the TableAttrSchema
  * table_attribute_name value will be compared and corresponding
  * structure member will be filled*/
  for (v_iter = column_attr.begin(); v_iter != column_attr.end(); ++v_iter) {
    if ((*v_iter).p_table_attribute_value == NULL) {
      pfc_log_debug("ODBCM::DBVarbind::bind value structure is empty ");
      continue;
    }
    switch ((*v_iter).table_attribute_name) {
      case BDRY_ID:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for controller_name CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> bi_value =
           *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
           ((*v_iter).p_table_attribute_value));
          /**clear the allocated buffer memory to receive the controller_name
          * from caller*/
          ODBCM_MEMSET(p_boundary_table->szboundary_id, 0,
                   sizeof(p_boundary_table->szboundary_id));
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_boundary_table->szboundary_id,
              &bi_value.value,
              (*v_iter).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE: "
            "szboundary_id= %s", p_boundary_table->szboundary_id);
        }
        break;
      case BDRY_DESCRIPTION:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for description CHAR[128]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_128]> desc_value =
           *((ColumnAttrValue <uint8_t[128]>*)
           ((*v_iter).p_table_attribute_value));
          ODBCM_MEMSET(p_boundary_table->szdescription, 0,
                   sizeof(p_boundary_table->szdescription));
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
              p_boundary_table->szdescription,
              &desc_value.value,
              (*v_iter).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE: "
            "szdescription = %s", p_boundary_table->szdescription);
        }
        break;
      case BDRY_CTR_NAME1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for controller name1 CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> cn_value =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
            ((*v_iter).p_table_attribute_value));
          ODBCM_MEMSET(p_boundary_table->szcontroller_name1, 0,
                   sizeof(p_boundary_table->szcontroller_name1));
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
            p_boundary_table->szcontroller_name1,
            &cn_value.value,
            (*v_iter).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE: "
             "szcontroller_name1= %s", p_boundary_table->szcontroller_name1);
        }
        break;
      case BDRY_DM_NAME1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for domain name1 CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> dn_value =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
            ((*v_iter).p_table_attribute_value));
          ODBCM_MEMSET(p_boundary_table->szdomain_name1, 0,
                   sizeof(p_boundary_table->szdomain_name1));
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
            p_boundary_table->szdomain_name1,
            &dn_value.value,
            (*v_iter).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE: "
            "szdomain_name1= %s", p_boundary_table->szdomain_name1);
        }
        break;
      case BDRY_PORT_ID1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for port id1 CHAR[320]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_320]> pi_value =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_320]>*)
            ((*v_iter).p_table_attribute_value));
          ODBCM_MEMSET(p_boundary_table->szlogical_port_id1, 0,
                   sizeof(p_boundary_table->szlogical_port_id1));
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
            p_boundary_table->szlogical_port_id1,
            &pi_value.value,
            (*v_iter).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE:"
             " szlogical_port_id1= %s", p_boundary_table->szlogical_port_id1);
        }
        break;
      case BDRY_CTR_NAME2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for controller name2 CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> cn2_value =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
            ((*v_iter).p_table_attribute_value));
          ODBCM_MEMSET(p_boundary_table->szcontroller_name2, 0,
                   sizeof(p_boundary_table->szcontroller_name2));
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
            p_boundary_table->szcontroller_name2,
            &cn2_value.value,
            (*v_iter).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE: "
            "szcontroller_name2= %s", p_boundary_table->szcontroller_name2);
        }
        break;
      case BDRY_DM_NAME2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for domain name2 CHAR[32]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> dn2_value =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
            ((*v_iter).p_table_attribute_value));
          ODBCM_MEMSET(p_boundary_table->szdomain_name2, 0,
                   sizeof(p_boundary_table->szdomain_name2));
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
            p_boundary_table->szdomain_name2,
            &dn2_value.value,
            (*v_iter).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE: "
            "szdomain_name2= %s", p_boundary_table->szdomain_name2);
        }
        break;
      case BDRY_PORT_ID2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for port id2 CHAR[320]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_320]> pi2_value =
            *((ColumnAttrValue <uint8_t[ODBCM_SIZE_320]>*)
            ((*v_iter).p_table_attribute_value));
          ODBCM_MEMSET(p_boundary_table->szlogical_port_id2, 0,
                   sizeof(p_boundary_table->szlogical_port_id2));
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
            p_boundary_table->szlogical_port_id2,
            &pi2_value.value,
            (*v_iter).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE:"
             " szlogical_port_id2= %s", p_boundary_table->szlogical_port_id2);
        }
        break;
      case BDRY_OPER_STATUS:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT16) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for oper status uint16_t*/
          ColumnAttrValue <uint16_t> rs_value =
          *((ColumnAttrValue <uint16_t>*)((*v_iter).p_table_attribute_value));
          ODBCM_MEMSET(&p_boundary_table->soper_status, 0,
                         sizeof(p_boundary_table->soper_status));
          /**copying the value from template to binded buffer */
          p_boundary_table->soper_status = rs_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE: "
            "soper_status= %d", p_boundary_table->soper_status);
        }
        break;
      case BDRY_VALID:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_8) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for valid CHAR[8]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_8]> valid_value =
            *((ColumnAttrValue<uint8_t[ODBCM_SIZE_8]>*)
            ((*v_iter).p_table_attribute_value));
          ODBCM_MEMSET(p_boundary_table->svalid, 0,
                   sizeof(p_boundary_table->svalid));
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
            p_boundary_table->svalid,
            &valid_value.value,
            (*v_iter).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE: "
            "sValid = %s", p_boundary_table->svalid);
        }
        break;
      case BDRY_ROW_STATUS:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT16) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for row status uint16_t*/
          ColumnAttrValue <uint16_t> rs_value =
          *((ColumnAttrValue <uint16_t>*)((*v_iter).p_table_attribute_value));
          ODBCM_MEMSET(&p_boundary_table->scs_row_status, 0,
                   sizeof(p_boundary_table->scs_row_status));
          p_boundary_table->scs_row_status = rs_value.value;
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE: "
             "scs_row_status = %d", p_boundary_table->scs_row_status);
        }
        break;
      case BDRY_ATTR:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_8) {
          /**ColumnAttrValue is a template to receive the void* values from
          * caller and typecast it into appropriate data type,
          * for attr CHAR[8]*/
          ColumnAttrValue <uint8_t[ODBCM_SIZE_8]> csa_value =
            *((ColumnAttrValue<uint8_t[ODBCM_SIZE_8]>*)
            ((*v_iter).p_table_attribute_value));
          ODBCM_MEMSET(p_boundary_table->scs_attr, 0,
                   sizeof(p_boundary_table->scs_attr));
          /**copying the value from template to binded buffer */
          ODBCM_MEMCPY(
            p_boundary_table->scs_attr,
            &csa_value.value,
            (*v_iter).table_attribute_length);
          odbcm_debug_info("ODBCM::DBVarbind::fill:BOUNDARY_TABLE: "
            "scs_attr = %s", p_boundary_table->scs_attr);
        }
        break;
     default:
       break;
    }
  }
  return ODBCM_RC_SUCCESS;
}

/**
 * @Description : To fetch the boundary_table values
 *                (which are stored in binded buffer)
 *                and store into TableAttSchema 
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 * @return      : ODBCM_RC_SUCCESS          - All bind operations success
 *                ODBCM_RC_PARAM_BIND_ERROR - Any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::fetch_boundary_table(
  std::vector<TableAttrSchema> &column_attr
  /*DBTableSchema->rowlist_entry*/) {
  /**Vector iterator to take the TableAttrSchema structures*/
  std::vector< TableAttrSchema >::iterator v_iter;
  /**Loop for iterate all the elements in the vector, the TableAttrSchema
   * table_attribute_name value will be compared and corresponding
   * structure member will be fetched */
  for (v_iter = column_attr.begin(); v_iter != column_attr.end(); ++v_iter) {
    switch ((*v_iter).table_attribute_name) {
      case BDRY_ID:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(
              uint8_t[ODBCM_SIZE_32+1],
              val_boundary_id);
          ODBCM_MEMCPY(
              val_boundary_id->value,
              p_boundary_table->szboundary_id,
              sizeof(p_boundary_table->szboundary_id));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
            "boundary_id = %s" , val_boundary_id->value);
            (*v_iter).p_table_attribute_value = val_boundary_id;
        }
        break;
      case BDRY_DESCRIPTION:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_128+1],
              desc_value);
          ODBCM_MEMCPY(
              desc_value->value,
              p_boundary_table->szdescription,
              sizeof(p_boundary_table->szdescription));
         odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
            "description = %s", desc_value->value);
            (*v_iter).p_table_attribute_value = desc_value;
        }
        break;
      case BDRY_CTR_NAME1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_32+1],
              val_controller_name1);
          ODBCM_MEMCPY(
              val_controller_name1->value,
              p_boundary_table->szcontroller_name1,
              sizeof(p_boundary_table->szcontroller_name1));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
            "controller_name1= %s" , val_controller_name1->value);
            (*v_iter).p_table_attribute_value = val_controller_name1;
        }
        break;
      case BDRY_DM_NAME1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_32+1],
              val_domain_name1);
          ODBCM_MEMCPY(
              val_domain_name1->value,
              p_boundary_table->szdomain_name1,
              sizeof(p_boundary_table->szdomain_name1));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
            "domain_name1= %s" , val_domain_name1->value);
            (*v_iter).p_table_attribute_value = val_domain_name1;
        }
        break;
      case BDRY_PORT_ID1:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_320+1],
              val_logical_port_id1);
          ODBCM_MEMCPY(
              val_logical_port_id1->value,
              p_boundary_table->szlogical_port_id1,
              *p_logicalport_id1_len);
          odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
              "logical_port_id1= %s" , val_logical_port_id1->value);
            (*v_iter).p_table_attribute_value = val_logical_port_id1;
        }
        break;
      case BDRY_CTR_NAME2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_32+1],
              val_controller_name2);
          ODBCM_MEMCPY(
              val_controller_name2->value,
              p_boundary_table->szcontroller_name2,
              sizeof(p_boundary_table->szcontroller_name2));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
              "controller_name2= %s" , val_controller_name2->value);
            (*v_iter).p_table_attribute_value = val_controller_name2;
        }
        break;
      case BDRY_DM_NAME2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_32+1],
              val_domain_name2);
          ODBCM_MEMCPY(
              val_domain_name2->value,
              p_boundary_table->szdomain_name2,
              sizeof(p_boundary_table->szdomain_name2));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
              "domain_name2= %s" , val_domain_name2->value);
            (*v_iter).p_table_attribute_value = val_domain_name2;
        }
        break;
      case BDRY_PORT_ID2:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_320+1],
              val_logical_port_id2);
          ODBCM_MEMCPY(
              val_logical_port_id2->value,
              p_boundary_table->szlogical_port_id2,
              *p_logicalport_id2_len);
          odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
            "logical_port_id2= %s" , val_logical_port_id2->value);
            (*v_iter).p_table_attribute_value = val_logical_port_id2;
        }
        break;
      case BDRY_OPER_STATUS:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT16) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, rs_value);
              rs_value->value = p_boundary_table->soper_status;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
              "oper_status= %d", rs_value->value);
          (*v_iter).p_table_attribute_value = rs_value;
        }
        break;
      case BDRY_VALID:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_8) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_8+1],
               valid_value);
          ODBCM_MEMCPY(
               valid_value->value,
               p_boundary_table->svalid,
               sizeof(p_boundary_table->svalid));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
              "valid = %s", valid_value->value);
              (*v_iter).p_table_attribute_value = valid_value;
        }
        break;
      case BDRY_ROW_STATUS:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT16) {
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint16_t, rs_value);
              rs_value->value = p_boundary_table->scs_row_status;
          odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
              "cs_row_status = %d", rs_value->value);
         (*v_iter).p_table_attribute_value = rs_value;
        }
        break;
      case BDRY_ATTR:
        if ((*v_iter).request_attribute_type == DATATYPE_UINT8_ARRAY_8) {
          /**ColumnAttrValue is a template to send the fetched values to
          * caller. typecast it into void*, memory will be allocated
          * for the template to send to caller*/
          ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(uint8_t[ODBCM_SIZE_8+1], cs_value);
          ODBCM_MEMCPY(
              cs_value->value,
              p_boundary_table->scs_attr,
              sizeof(p_boundary_table->scs_attr));
          odbcm_debug_info("ODBCM::DBVarbind::fetch:BOUNDARY_TABLE: "
              "cs_attr = %s", cs_value->value);
          (*v_iter).p_table_attribute_value = cs_value;
      }
      break;
     default:
       break;
     }
  }
  return ODBCM_RC_SUCCESS;
}
/**EOF*/
