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
 * @file    odbcm_db_tableschema.cc
 *
 */

#include <iomanip>
#include <sstream>
#include "odbcm_db_tableschema.hh"
#include "odbcm_utils.hh"
#include "odbcm_mgr.hh"
#include "odbcm_common.hh"
namespace unc {
namespace uppl {

/**
 * @Description :Constructor to create/initialize the instance of 
 * DBTableSchema
 * @param[in]   : None
 * @return      : void
 **/
DBTableSchema::DBTableSchema()
:
      /** Initialize all the members */
      db_return_status_(ROW_VALID) {
      table_name_ = UNKNOWN_TABLE;
      frame_explicit_order_ = "";
}

/**
 * @Description :This function is automatically invoked when
 * DBTableSchema object is destroyed
 * @param[in]   : None
 * @return      : void
 **/
DBTableSchema::~DBTableSchema() {
  FreeDBTableSchema();
  primary_keys_.clear();
}

/**
 * @Description : To get the table name from DBTableSchema 
 * @param[in]   : None
 * @return      : ODBCMTable
 **/
ODBCMTable DBTableSchema::get_table_name() {
  return table_name_;
}

/**
 * @Description : To set the table name in DBTableSchema
 * @param[in]   : ODBCMTable table_name
 * @return      : void
 **/
void DBTableSchema::set_table_name(unc::uppl::ODBCMTable table_name) {
  table_name_ = table_name;
}

/**
 * @Description : To get the primarykeys filled in DBTableSchema
 * @param[in]   : None
 * @return      : vector 
 **/
std::vector <std::string>  DBTableSchema::get_primary_keys() {
  return primary_keys_;
}

/**
 * @Description : To get the DBTableSchema row_list  
 * @param[in]   : None
 * @return      : list
 **/
std::list < std::vector <TableAttrSchema> >& DBTableSchema::get_row_list() {
  return row_list_;
}

/**
 * @Description : To fill the DBTableSchema row_list
 * @param[in]   : attributes_vector - vector object specifies the
 *                column attributes on the table
 * @return      : void
 **/
void DBTableSchema::PushBackToRowList(
  std::vector <TableAttrSchema>
  attributes_vector) {
  row_list_.push_back(attributes_vector);
}

/**
 * @Description : To fill the DBTableSchema primarykey vector
 * @param[in]   : attribute_name - specifies the attribute name
 * @return      : void
 **/
void DBTableSchema::PushBackToPrimaryKeysVector(
  std::string attribute_name) {
  primary_keys_.push_back(attribute_name);
}

/**
 * @Description : To set the DBTableSchema primarykey vector
 * @param[in]   : primary_keys - vector holds the primarykeys
 *                of the specified table
 * @return      : void
 **/
void DBTableSchema::set_primary_keys(
  std::vector <std::string> primary_keys) {
  primary_keys_ = primary_keys;
}

/**
 * @Description : To set the DBTableSchema row_list
 * @param[in]   : row_list - specifies rows and each row column
 *                attributes on the table
 * @return      : void
 **/
void DBTableSchema::set_row_list(
  std::list <std::vector<TableAttrSchema> > row_list) {
  row_list_ = row_list;
}

/**
*@Description : To free the allocated memory in DBTableSchema
*@param[in]   : none
*@return      : void
**/
void DBTableSchema::FreeDBTableSchema() {
  uint32_t                                              row_count = 0;
  std::vector<TableAttrSchema>                          attr_vector;
  std::vector< TableAttrSchema >::iterator              iter_vector;
  std::list <std::vector<TableAttrSchema> >::iterator   iter_list;

  /** Traverse the list to get the attribute vector */
  for (row_count = 1, iter_list = row_list_.begin();
      iter_list != row_list_.end(); ++iter_list, row_count++) {
    attr_vector = *iter_list;
    /** Traverse the vector to get attribute information */
    for (iter_vector = attr_vector.begin();
        iter_vector != attr_vector.end(); ++iter_vector) {
      /** Free the memory inside the vector */
      if ((*iter_vector).p_table_attribute_value != NULL) {
        ::operator delete((*iter_vector).p_table_attribute_value);
        //  free((*iter_vector).p_table_attribute_value);
        (*iter_vector).p_table_attribute_value = NULL;
      }
      /** Clear the vector */
    }  // for list
    attr_vector.clear();
  }  // for vector
  /** Clear the list after the traversal */
  row_list_.clear();
}

/**
*@Description : delete and free the single rowlist memory in DBTableSchema 
*@param[in]   : none
*@return      : void
**/
void DBTableSchema::DeleteRowListFrontElement() {
  std::list <std::vector<TableAttrSchema> >::iterator iter_list =
                                      row_list_.begin();
  std::vector< TableAttrSchema >::iterator iter_vector;
  std::vector<TableAttrSchema> attr_vector = *iter_list;
  /** Traverse the vector to get attribute information */
  for (iter_vector = attr_vector.begin();
      iter_vector != attr_vector.end(); ++iter_vector) {
    /** Free the memory inside the vector */
    if ((*iter_vector).p_table_attribute_value != NULL) {
      ::operator delete((*iter_vector).p_table_attribute_value);
      (*iter_vector).p_table_attribute_value = NULL;
    }
  }
  attr_vector.clear();
  row_list_.pop_front();
}

/**
 * *@Description : To print the char buffer values in DBTableSchema
 * *@param[in]   : buffer - to print the column attribute values, 
 *                 length - specifies the of the buffer
 *                 column_name - specifies the column name
 * *@return      : char* 
 * **/
inline std::string DBTableSchema::Odbcm_PrintCharBuffer(uint8_t* buffer,
                          int length, ODBCMTableColumns column_name) {
  int index = 0;
  std::ostringstream buffer_stream;
  buffer_stream.str("");
  buffer_stream << "| " << std::setw(20) <<
  ODBCManager::get_ODBCManager()->
              GetColumnName(column_name).c_str() << ": ";
  while (index < length) {
    if ('\0' != buffer[index])
      buffer_stream << buffer[index];
    index++;
  }
  buffer = NULL;
  return buffer_stream.str();
}

/**
*@Description : To print the values in DBTableSchema
*@param[in]   : None
*@return      : void
**/
void DBTableSchema::PrintDBTableSchema() {
  uint32_t                                    row_count = 0;
  std::vector<TableAttrSchema>                attr_vector;
  std::vector<std::string> :: iterator        iter_primarykey;
  std::vector< TableAttrSchema >::iterator    iter_vector;
  std::list <std::vector<TableAttrSchema> >::iterator   iter_list;
  /** Print the contents of DBTableSchema object */
  pfc_log_debug("+---------------------------------------+");
  pfc_log_debug("| Table               : %s",
                ODBCManager::get_ODBCManager()->
                GetTableName(get_table_name()).c_str());
  /** Traverse the primary key vector to display pkeys */
  std::ostringstream  primary_key;
  primary_key << "| Primary keys        : ";
  for (iter_primarykey = primary_keys_.begin();
      iter_primarykey  !=  primary_keys_.end();
      iter_primarykey++) {
    primary_key << (*iter_primarykey).c_str() << ";";
  }  // for

  pfc_log_debug("%s", (primary_key.str()).c_str());
  pfc_log_debug("+---------------------------------------+");
  ColumnAttrValue <uint16_t> *rs_value16 = NULL;
  ColumnAttrValue <uint32_t> *rs_value32 = NULL;
  ColumnAttrValue <uint32_t> *iprs_value = NULL;
  ColumnAttrValue <uint64_t> *rs_value64 = NULL;
  ColumnAttrValue <uint8_t[16]> *ip6rs_value = NULL;
  ColumnAttrValue <uint8_t[2]> *rs_valuea2 = NULL;
  ColumnAttrValue <uint8_t[3]> *rs_valuea3 = NULL;
  ColumnAttrValue <uint8_t[6]> *rs_valuea6 = NULL;
  ColumnAttrValue <uint8_t[8]> *rs_valuea8 = NULL;
  ColumnAttrValue <uint8_t[10]> *rs_valuea10 = NULL;
  ColumnAttrValue <uint8_t[11]> *rs_valuea11 = NULL;
  ColumnAttrValue <uint8_t[16]> *rs_valuea16 = NULL;
  ColumnAttrValue <uint8_t[32]> *rs_valuea32 = NULL;
  ColumnAttrValue <uint8_t[128]> *rs_valuea128 = NULL;
  ColumnAttrValue <uint8_t[256]> *rs_valuea256 = NULL;
  ColumnAttrValue <uint8_t[257]> *rs_valuea257 = NULL;
  ColumnAttrValue <uint8_t[320]> *rs_valuea320 = NULL;
  /** Traverse the list to get the attribute vector */
  for (row_count = 1, iter_list = row_list_.begin();
      iter_list != row_list_.end(); ++iter_list, row_count++) {
    attr_vector = *iter_list;
    pfc_log_debug("|                 ROW : %d", row_count);
    /** Traverse the vector to get attribute information */
    std::ostringstream row_values;
    for (iter_vector = attr_vector.begin();
        iter_vector != attr_vector.end(); ++iter_vector) {
      rs_value16 = NULL;
      rs_value32 = NULL;
      iprs_value = NULL;
      rs_value64 = NULL;
      ip6rs_value = NULL;
      rs_valuea2 = NULL;
      rs_valuea3 = NULL;
      rs_valuea6 = NULL;
      rs_valuea8 = NULL;
      rs_valuea11 = NULL;
      rs_valuea16 = NULL;
      rs_valuea32 = NULL;
      rs_valuea128 = NULL;
      rs_valuea256 = NULL;
      rs_valuea257 = NULL;
      rs_valuea320 = NULL;
      row_values.str("");
      row_values << "| " << std::setw(20) << ODBCManager::get_ODBCManager()->
        GetColumnName((*iter_vector).table_attribute_name).c_str() << ": ";
      ODBCMTableColumns attr_name((*iter_vector).table_attribute_name);

      switch ((*iter_vector).request_attribute_type) {
        case DATATYPE_UINT16:
          rs_value16 = ((ColumnAttrValue<uint16_t>*)
            ((*iter_vector).p_table_attribute_value));
          row_values << rs_value16->value;
          pfc_log_debug("%s", row_values.str().c_str());
          break;
        case DATATYPE_UINT32:
          rs_value32 = ((ColumnAttrValue<uint32_t>*)
            ((*iter_vector).p_table_attribute_value));
          row_values << rs_value32->value;
          pfc_log_debug("%s", row_values.str().c_str());
          break;
        case DATATYPE_UINT64:
          rs_value64 = ((ColumnAttrValue<uint64_t>*)
            ((*iter_vector).p_table_attribute_value));
          row_values << rs_value64->value;
          pfc_log_debug("%s", row_values.str().c_str());
          break;
        case DATATYPE_IPV4:
          iprs_value = ((ColumnAttrValue<uint>*)
            ((*iter_vector).p_table_attribute_value));
          struct sockaddr_in saddr;
          saddr.sin_addr.s_addr =  iprs_value->value;
          row_values << inet_ntoa(saddr.sin_addr);
          pfc_log_debug("%s", row_values.str().c_str());
          break;
        case DATATYPE_IPV6:
          ip6rs_value = ((ColumnAttrValue<uint8_t[16]>*)
            ((*iter_vector).p_table_attribute_value));
          row_values << ODBCMUtils::get_ipv6_string(ip6rs_value->value);
          pfc_log_debug("%s", row_values.str().c_str());
          break;
        case DATATYPE_UINT8_ARRAY_2:
          rs_valuea2 = ((ColumnAttrValue<uint8_t[2]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea2->value, 2,
                        attr_name).c_str());
          break;
        case DATATYPE_UINT8_ARRAY_3:
          rs_valuea3 = ((ColumnAttrValue<uint8_t[3]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea3->value, 3,
                        attr_name).c_str());
          break;
        case DATATYPE_UINT8_ARRAY_6:
          rs_valuea6 = ((ColumnAttrValue<uint8_t[6]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea6->value, 6,
                       attr_name).c_str());
          break;
        case DATATYPE_UINT8_ARRAY_8:
          rs_valuea8 = ((ColumnAttrValue<uint8_t[8]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea8->value, 8,
                        attr_name).c_str());
          break;
        case DATATYPE_UINT8_ARRAY_10:
          rs_valuea10 = ((ColumnAttrValue<uint8_t[10]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea10->value, 10,
                        attr_name).c_str());
          break;
        case DATATYPE_UINT8_ARRAY_11:
          rs_valuea11 = ((ColumnAttrValue<uint8_t[11]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea11->value, 11,
                        attr_name).c_str());
          break;
        case DATATYPE_UINT8_ARRAY_16:
          rs_valuea16 = ((ColumnAttrValue<uint8_t[16]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea16->value, 16,
                        attr_name).c_str());
          break;
        case DATATYPE_UINT8_ARRAY_32:
          rs_valuea32 = ((ColumnAttrValue<uint8_t[32]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea32->value, 32,
                        attr_name).c_str());
          break;
        case DATATYPE_UINT8_ARRAY_128:
          rs_valuea128 = ((ColumnAttrValue<uint8_t[128]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea128->value, 128,
                       attr_name).c_str());
          break;
        case DATATYPE_UINT8_ARRAY_256:
          rs_valuea256 = ((ColumnAttrValue<uint8_t[256]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea256->value, 256,
                       attr_name).c_str());
          break;
        case DATATYPE_UINT8_ARRAY_257:
          rs_valuea257 = ((ColumnAttrValue<uint8_t[257]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea257->value, 257,
                        attr_name).c_str());
          break;
        case DATATYPE_UINT8_ARRAY_320:
          rs_valuea320 = ((ColumnAttrValue<uint8_t[320]>*)
            ((*iter_vector).p_table_attribute_value));
          pfc_log_debug("%s", Odbcm_PrintCharBuffer(rs_valuea320->value, 320,
                        attr_name).c_str());
          break;
        default:
          row_values.str("");
          break;
      }
    }
    pfc_log_debug("+---------------------------------------+");
  }  // for
}

}  // namespace uppl
}  // namespace unc
