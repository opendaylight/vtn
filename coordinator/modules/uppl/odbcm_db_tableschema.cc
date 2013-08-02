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
 * @file    odbcm_db_tableschema.cc
 *
 */

#include <iomanip>
#include <sstream>
#include "odbcm_db_tableschema.hh"
#include "odbcm_utils.hh"
namespace unc {
namespace uppl {

/**
 * @Description :
 * @param[in]   : None
 * @return      :
 **/
DBTableSchema::DBTableSchema() {
  /** Initialize all the members */
  db_return_status_ = ROW_VALID;
}

/**
 * @Description :
 * @param[in]   : None
 * @return      :
 **/
DBTableSchema::~DBTableSchema() {
  FreeDBTableSchema();
  primary_keys_.clear();
}

/**
 * @Description :
 * @param[in]   : None
 * @return      :
 **/
std::string DBTableSchema::get_table_name() {
  return table_name_;
}

/**
 * @Description :
 * @param[in]   : None
 * @return      :
 **/
void DBTableSchema::set_table_name(std::string table_name) {
  table_name_ = table_name;
}

/**
 * @Description :
 * @param[in]   : None
 * @return      :
 **/
std::vector <std::string>  DBTableSchema::get_primary_keys() {
  return primary_keys_;
}

/**
 * @Description :
 * @param[in]   : None
 * @return      :
 **/
std::list < std::vector <TableAttrSchema> > DBTableSchema::get_row_list() {
  return row_list_;
}

/**
 * @Description :
 * @param[in]   : None
 * @return      :
 **/
void DBTableSchema::PushBackToRowList(
  std::vector <TableAttrSchema>
  attributes_vector) {
  row_list_.push_back(attributes_vector);
}

/**
 * @Description :
 * @param[in]   : None
 * @return      :
 **/
void DBTableSchema::PushBackToPrimaryKeysVector(
  std::string attribute_name) {
  primary_keys_.push_back(attribute_name);
}

/**
 * @Description :
 * @param[in]   : None
 * @return      :
 **/
void DBTableSchema::set_primary_keys(
  std::vector <std::string> primary_keys) {
  primary_keys_ = primary_keys;
}

/**
 * @Description :
 * @param[in]   : None
 * @return      :
 **/
void DBTableSchema::set_row_list(
  std::list <std::vector<TableAttrSchema> > row_list) {
  row_list_ = row_list;
}

/**
*@Description : To free the allocated memory in DBTableSchema
*@param[in]   : DBTableSchema*
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
*@Description : To print the values in DBTableSchema
*@param[in]   : DBTableSchema*
*@return      : void
**/
void DBTableSchema::PrintDBTableSchema() {
  uint32_t                                    row_count = 0;
  uint32_t                                    index = 0;
  std::vector<TableAttrSchema>                attr_vector;
  std::vector<std::string> :: iterator        iter_primarykey;
  std::vector< TableAttrSchema >::iterator    iter_vector;
  std::list <std::vector<TableAttrSchema> >::iterator   iter_list;
  /** Print the contents of DBTableSchema object */
  pfc_log_info("+---------------------------------------+");
  pfc_log_info("| Table               : %s", (get_table_name()).c_str());
  /** Traverse the primary key vector to display pkeys */
  std::ostringstream  primary_key;
  primary_key << "| Primary keys        : ";
  for (iter_primarykey = primary_keys_.begin();
      iter_primarykey  !=  primary_keys_.end();
      iter_primarykey++) {
    primary_key << (*iter_primarykey).c_str() << ";";
  }  // for
  pfc_log_info("%s", (primary_key.str()).c_str());
  pfc_log_info("+---------------------------------------+");
  /*
   * Macro to print the char buffer
   */
#define ODBCM_PRINT_CHAR_BUFFER(buffer, length) \
  index = 0;                                    \
  while (index < length) {                          \
    if ('\0' != buffer[index])                      \
      row_values << buffer[index];                  \
    index++; }

  /** Traverse the list to get the attribute vector */
  std::ostringstream row_values;
  for (row_count = 1, iter_list = row_list_.begin();
      iter_list != row_list_.end(); ++iter_list, row_count++) {
    attr_vector = *iter_list;
    pfc_log_info("|                 ROW : %d", row_count);
    /** Traverse the vector to get attribute information */
    for (iter_vector = attr_vector.begin();
        iter_vector != attr_vector.end(); ++iter_vector) {
      row_values.str("");
      row_values << "| " << std::setw(20) <<
        (*iter_vector).table_attribute_name.c_str() << ": ";
      /** If attribute datatype is DATATYPE_UINT8_ARRAY_9 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_9) {
        ColumnAttrValue <uint8_t[9]> *rs_value =
          ((ColumnAttrValue<uint8_t[9]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 9);
      }
      /** If attribute datatype is DATATYPE_UINT8_ARRAY_8 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_8) {
        ColumnAttrValue <uint8_t[8]> *rs_value =
          ((ColumnAttrValue<uint8_t[8]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 8);
      }
     /** If attribute datatype is DATATYPE_UINT8_ARRAY_6 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_6) {
        ColumnAttrValue <uint8_t[6]> *rs_value =
          ((ColumnAttrValue<uint8_t[6]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 6);
      }
      /** If attribute datatype is DATATYPE_UINT8_ARRAY_3 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_3) {
        ColumnAttrValue <uint8_t[3]> *rs_value =
          ((ColumnAttrValue<uint8_t[3]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 3);
      }
      /** If attribute datatype is DATATYPE_UINT8_ARRAY_2 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_2) {
        ColumnAttrValue <uint8_t[2]> *rs_value =
          ((ColumnAttrValue<uint8_t[2]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 2);
      }
     /** If attribute datatype is DATATYPE_UINT8_ARRAY_11 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_11) {
        ColumnAttrValue <uint8_t[11]> *rs_value =
          ((ColumnAttrValue<uint8_t[11]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 11);
      }
      /** If attribute datatype is DATATYPE_UINT8_ARRAY_16 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_16) {
        ColumnAttrValue <uint8_t[16]> *rs_value =
          ((ColumnAttrValue<uint8_t[16]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 16);
      }
      /** If attribute datatype is DATATYPE_IPV4 */
      if (((*iter_vector).request_attribute_type == DATATYPE_IPV4)) {
        ColumnAttrValue <uint32_t> *rs_value =
          ((ColumnAttrValue<uint>*)
          ((*iter_vector).p_table_attribute_value));
        struct sockaddr_in saddr;
        saddr.sin_addr.s_addr =  rs_value->value;
        row_values << inet_ntoa(saddr.sin_addr);
      }
      /** If attribute datatype is DATATYPE_IPV6 */
      if ((*iter_vector).request_attribute_type == DATATYPE_IPV6) {
        ColumnAttrValue <uint8_t[16]> *rs_value =
          ((ColumnAttrValue<uint8_t[16]>*)
          ((*iter_vector).p_table_attribute_value));
        //  ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 16);
        row_values << ODBCMUtils::get_ipv6_string(rs_value->value);
      }
      /** If attribute datatype is DATATYPE_UINT8_ARRAY_32 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_32) {
        ColumnAttrValue <uint8_t[32]> *rs_value =
          ((ColumnAttrValue<uint8_t[32]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 32);
      }
      /** If attribute datatype is DATATYPE_UINT8_ARRAY_128 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_128) {
        ColumnAttrValue <uint8_t[128]> *rs_value =
          ((ColumnAttrValue<uint8_t[128]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 128);
      }
      /** If attribute datatype is DATATYPE_UINT8_ARRAY_257 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_257) {
        ColumnAttrValue <uint8_t[257]> *rs_value =
          ((ColumnAttrValue<uint8_t[257]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 257);
      }
      /** If attribute datatype is DATATYPE_UINT8_ARRAY_256 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_256) {
        ColumnAttrValue <uint8_t[256]> *rs_value =
          ((ColumnAttrValue<uint8_t[256]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 256);
      }
      /** If attribute datatype is DATATYPE_UINT8_ARRAY_320 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT8_ARRAY_320) {
        ColumnAttrValue <uint8_t[320]> *rs_value =
          ((ColumnAttrValue<uint8_t[320]>*)
          ((*iter_vector).p_table_attribute_value));
        ODBCM_PRINT_CHAR_BUFFER(rs_value->value, 320);
      }
      /** If attribute datatype is DATATYPE_UINT16 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT16) {
        ColumnAttrValue <uint16_t> *rs_value =
         ((ColumnAttrValue<uint16_t>*)
         ((*iter_vector).p_table_attribute_value));
        row_values << rs_value-> value;
      }
      /** If attribute datatype is DATATYPE_UINT32 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT32) {
        ColumnAttrValue <uint32_t> *rs_value =
          ((ColumnAttrValue<uint32_t>*)
          ((*iter_vector).p_table_attribute_value));
        row_values << rs_value->value;
      }
     /** If attribute datatype is DATATYPE_UINT64 */
      if ((*iter_vector).request_attribute_type == DATATYPE_UINT64) {
        ColumnAttrValue <uint64_t> *rs_value =
          ((ColumnAttrValue<uint64_t>*)
          ((*iter_vector).p_table_attribute_value));
        row_values << rs_value->value;
      }
      pfc_log_info("%s", (row_values.str()).c_str());
    }
      pfc_log_info("+---------------------------------------+");
  }  // for
}

}  // namespace uppl
}  // namespace unc
