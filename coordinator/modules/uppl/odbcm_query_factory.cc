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
 * @file    odbcm_query_factory.cc
 *
 */
#include <unc/keytype.h>
#include <sstream>
#include "odbcm_query_factory.hh"
#include "odbcm_query_processor.hh"
#include "odbcm_utils.hh"

namespace unc {
namespace uppl {

/**
 * @Description :Constructor function which will create/initialize
 * the instance of QueryFactory.
 * @param[in]   : None
 * @return      : None
 **/
QueryFactory::QueryFactory() {
  GetQuery              = NULL;
  GetDBSpecificQuery    = NULL;
  GetTwoDBQuery         = NULL;
  GetSingleDBQuery      = NULL;
  GetIsDBQuery          = NULL;
  GetClearInstanceQuery = NULL;
//  GetTwoTableQuery      = NULL;
  GetBulkRowQuery       = NULL;
  CommitTableQuery      = NULL;
  GetCountQuery         = NULL;
  GetFilterCountQuery   = NULL;
  GetSiblingFilterQuery = NULL;
}

/**
 * @Description :This function is automatically invoked when
 * QueryFactory object is destroyed
 * @param[in]   : None
 * @return      : None
 **/
QueryFactory::~QueryFactory() {
}

/**
 * @Description : This function gets the query statement handler
 *                for creating a row in the database
 * @param[in]   : unc_keytype_datatype_t, DBTableSchema&
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_createonerow(
    unc_keytype_datatype_t db_name,
    DBTableSchema &db_table_schema) {
  std::string             prefix;
  uint16_t                loop1 = 0, loop2 = 0;
  std::ostringstream      create_query, values;
  std::vector<TableAttrSchema> :: iterator              iter_vector;
  std::list<std::vector<TableAttrSchema> > ::iterator   iter_list;
  /** Get the table name prefix */
  if (db_name == UNC_DT_CANDIDATE)
    prefix = "c_";
  else if (db_name == UNC_DT_RUNNING)
    prefix = "r_";
  else if (db_name == UNC_DT_STARTUP)
    prefix = "s_";
  else if (db_name == UNC_DT_IMPORT)
    prefix = "i_";
  else if (db_name == UNC_DT_STATE)
    prefix = "r_";
  else
    return ODBCM_NULL_STRING;
  create_query << "INSERT INTO " << prefix << db_table_schema.table_name_<< " ";
  /** Traverse the list to get the Attribute vector */
  /** In this case, this traversal will be only once since list
   * contains only one row info */
  for (loop1 = 0, iter_list = db_table_schema.row_list_.begin();
      iter_list != db_table_schema.row_list_.end(); iter_list++, loop1++) {
    /** This vector contains all attributes of a row in a table */
    std::vector<TableAttrSchema>attributes_vector = *iter_list;
    create_query << "(";
    /** Get the column names  and values */
    for (loop2 = 0, iter_vector = attributes_vector.begin();
        iter_vector != attributes_vector.end(); iter_vector++, loop2++) {
      /** Get attribute name of a row */
      create_query << (*iter_vector).table_attribute_name;
      /** For the last attribute comma is not required*/
      if (loop2  !=  attributes_vector.size()-1) {
        create_query << ",";
      }
        values << "?";
        if (loop2  !=  attributes_vector.size()-1) {
          values << ",";
        }
    }  // for attribute vectors
  }  // for list of rows
  /** Frame the final query for one row creation */
  if (db_name == UNC_DT_CANDIDATE) {
    create_query << ",cs_row_status) VALUES (" << values.str() << "," <<
        CREATED << ");";
  } else {
    create_query << ") VALUES (" << values.str() << ");";
  }
  pfc_log_info("ODBCM::QueryFactory::CreateOneRow: Query: \"%s\"",
    (create_query.str()).c_str());
  return create_query.str();
}

/**
 * @Description : This method gets the query statement handler
 *                for updating a row in the database.
 * @param[in]   : unc_keytype_datatype_t, DBTableSchema&
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_updateonerow(
    unc_keytype_datatype_t db_name, DBTableSchema &db_table_schema) {
  uint16_t            loop1 = 0;
  uint16_t            loop2 = 0;
  uint16_t            loop3 = 0;
  std::string         prefix;
  std::ostringstream  update_query;
  std::ostringstream  where_query;
  std::vector<std::string> :: iterator                iter_keys;
  std::vector<TableAttrSchema> :: iterator            iter_vector;
  std::list<std::vector<TableAttrSchema> > ::iterator iter_list;
  /** Get the table name prefix */
  if (db_name == UNC_DT_CANDIDATE)
    prefix = "c_";
  else if (db_name == UNC_DT_RUNNING)
    prefix = "r_";
  else if (db_name == UNC_DT_STARTUP)
    prefix = "s_";
  else if (db_name == UNC_DT_IMPORT)
    prefix = "i_";
  else if (db_name == UNC_DT_STATE)
    prefix = "r_";
  else
    return ODBCM_NULL_STRING;
  /** Start framing the update query */

  if (db_name == UNC_DT_CANDIDATE) {
    if (db_table_schema.db_return_status_ == CREATED) {
      /**to keep the immediate created rows status as created*/
      update_query << "UPDATE " << prefix << \
        db_table_schema.table_name_<< " SET cs_row_status = " << CREATED << ",";
    } else {
      update_query << "UPDATE " << prefix << \
        db_table_schema.table_name_<< " SET cs_row_status = " << UPDATED << ",";
    }
  } else {
    update_query << "UPDATE " << prefix << \
        db_table_schema.table_name_<< " " <<" SET ";
  }
  /** Get the primary keys. Needed since its not required to SET the pk */
  std::vector <std::string> primarykeys = db_table_schema.get_primary_keys();
  /** Now, we use the actual values of primary key instead of '?' 
    while framing update query */
  /** Framing the WHERE part of the query using primary keys */
  for (loop1 = 0, iter_list = db_table_schema.row_list_.begin();
        iter_list != db_table_schema.row_list_.end(); iter_list++, loop1++) {
    /** This vector contains all attributes of a row in a table */
    std::vector<TableAttrSchema>attributes_vector = *iter_list;
    /** Get the column names  and values */
    for (loop2 = 0, iter_vector = attributes_vector.begin();
        iter_vector != attributes_vector.end(); iter_vector++, loop2++) {
      for (loop3 = 0, iter_keys = primarykeys.begin();
          iter_keys != primarykeys.end(); iter_keys++, loop3++) {
        if (((*iter_keys).compare((*iter_vector).table_attribute_name) == 0) &&
            ((*iter_vector).request_attribute_type ==
                DATATYPE_UINT8_ARRAY_32)) {
            ColumnAttrValue <uint8_t[ODBCM_SIZE_32]> column_value;
            column_value = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_32]>*)
                ((*iter_vector).p_table_attribute_value));
            uint8_t temp[ODBCM_SIZE_32+1];
            ODBCM_MEMSET(&temp, '\0', ODBCM_SIZE_32+1);
            ODBCM_MEMCPY(&temp, &column_value.value, ODBCM_SIZE_32);
            uint16_t len = strlen((const char*)temp);
            temp[len+1] = '\0';
            pfc_log_info("ODBCM::QueryFactory::UpdateQuery():"
                " (*iter_vector).length %d value = %s\n",
                (*iter_vector).table_attribute_length,
                temp);
            where_query << (*iter_keys).c_str() << " = '" <<
                temp << "'";
        } else if (
            ((*iter_keys).compare((*iter_vector).table_attribute_name) == 0) &&
            ((*iter_vector).request_attribute_type ==
                DATATYPE_UINT8_ARRAY_320)) {
            ColumnAttrValue <uint8_t[ODBCM_SIZE_320]> column_value;
            column_value = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_320]>*)
                ((*iter_vector).p_table_attribute_value));
            uint8_t btemp[ODBCM_SIZE_320+1];
            ODBCM_MEMSET(&btemp, '\0', ODBCM_SIZE_320+1);
            ODBCM_MEMCPY(&btemp, &column_value.value, ODBCM_SIZE_320);
            uint16_t len = strlen((const char*)btemp);
            btemp[len+1] = '\0';
            pfc_log_info("ODBCM::QueryFactory::UpdateQuery():"
                " (*iter_vector).length %d value = %s\n",
                (*iter_vector).table_attribute_length,
                btemp);
            where_query << (*iter_keys).c_str() << " = '" <<
                btemp << "'";
        } else if (
            ((*iter_keys).compare((*iter_vector).table_attribute_name) == 0) &&
            ((*iter_vector).request_attribute_type ==
                DATATYPE_UINT8_ARRAY_256)) {
            ColumnAttrValue <uint8_t[ODBCM_SIZE_256]> column_value;
            column_value = *((ColumnAttrValue <uint8_t[ODBCM_SIZE_256]>*)
                ((*iter_vector).p_table_attribute_value));

            uint8_t stemp[ODBCM_SIZE_256+1];
            ODBCM_MEMSET(&stemp, '\0', ODBCM_SIZE_256+1);
            ODBCM_MEMCPY(&stemp, &column_value.value, ODBCM_SIZE_256);
            uint16_t len = strlen((const char*)stemp);
            stemp[len+1] = '\0';
            pfc_log_info("ODBCM::QueryFactory::UpdateQuery():"
                "(*iter_vector).length %d value = %s\n",
                (*iter_vector).table_attribute_length,
                column_value.value);
            where_query << (*iter_keys).c_str() << " = '" <<
                stemp << "'";
          }

          if (((*iter_keys).compare((*iter_vector).table_attribute_name) == 0)
              && loop3 != primarykeys.size()-1) {
            where_query << " AND ";
          }
      }  // primary_key loop
    }  // list vector loop
  }  // list loop
  /** Traverse the list to get the Attribute vector */
  /** In this case, this traversal will be only once since
     list contains only one row info */
  uint32_t pkey_size =  primarykeys.size();

  for (loop1 = 0, iter_list = db_table_schema.row_list_.begin();
      iter_list != db_table_schema.row_list_.end(); iter_list++, loop1++) {
    /** This vector contains all attributes of a row in a table */
    std::vector<TableAttrSchema>attributes_vector = *iter_list;
    if (attributes_vector.size() <= 0) {
      pfc_log_info("ODBCM::QueryFactory::UpdateQuery(): Values are "
          " not found in attributes_vector");
      return ODBCM_NULL_STRING;
    }
    if (pkey_size <= 0) {
      pfc_log_info("ODBCM::QueryFactory::UpdateQuery(): Pkeys are "
          " not found in pkey vector");
      return ODBCM_NULL_STRING;
    }
    if (attributes_vector.size() <= pkey_size) {
      pfc_log_info("ODBCM::QueryFactory::UpdateQuery(): primary keys"
          " not found in attributes_vector");
      return ODBCM_NULL_STRING;
    }
    /** Get the column names  and values */
    for (loop2 = 0, iter_vector = attributes_vector.begin() + pkey_size;
        iter_vector != attributes_vector.end(); iter_vector++, loop2++) {
      update_query << (*iter_vector).table_attribute_name <<"= ?";
      if (loop2  !=  attributes_vector.size()-(pkey_size+1))
        update_query << ",";
    }  // for list of rows
  }
  /** Frame the final query for one row updation */
  update_query << " WHERE " << where_query.str() << ";";
  pfc_log_info("ODBCM::QueryFactory::UpdateOneRow: Query: \"%s\"",
               (update_query.str()).c_str());
  return update_query.str();
}

/**
 * @Description : This method will get the query statement handler
 *                for deleting a row in the database.
 * @param[in]   : unc_keytype_datatype_t, DBTableSchema& 
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_deleteonerow(unc_keytype_datatype_t db_name,
                                              DBTableSchema &db_table_schema) {
  uint16_t            loop1 = 0;
  std::string         prefix;
  std::ostringstream  delete_query;
  std::vector <std::string>             primarykeys;
  std::vector<std::string> :: iterator  iter_vector;
  /** Get the table name prefix */
  if (db_name == UNC_DT_CANDIDATE)
    prefix = "c_";
  else if (db_name == UNC_DT_RUNNING)
    prefix = "r_";
  else if (db_name == UNC_DT_STARTUP)
    prefix = "s_";
  else if (db_name == UNC_DT_IMPORT)
    prefix = "i_";
  else if (db_name == UNC_DT_STATE)
    prefix = "r_";
  else
    return ODBCM_NULL_STRING;
  if (db_name == UNC_DT_CANDIDATE) {
    delete_query << "UPDATE " << prefix << db_table_schema.table_name_;
    delete_query << " SET cs_row_status = " << DELETED << " WHERE ";
  } else {
    delete_query << "DELETE FROM " << prefix << db_table_schema.table_name_;
    delete_query << " WHERE ";
  }
  /** Use only primary keys after WHERE in the sql query */
  primarykeys = db_table_schema.get_primary_keys();
  for (loop1 = 0, iter_vector = primarykeys.begin();
      iter_vector != primarykeys.end(); iter_vector++, loop1++) {
    delete_query << (*iter_vector).c_str() << " = ?";
    if (loop1 != primarykeys.size()-1) {
      delete_query << " AND ";
    }
  }  // for
  delete_query << ";";
  pfc_log_info("ODBCM::QueryFactory::DeleteOneRow: Query: \"%s\"",
      (delete_query.str()).c_str());

  return delete_query.str();
}

/**
 * @Description : To frame the query for deleting a row in a db table
 * @param[in]   : unc_keytype_datatype_t, DBTableSchema& 
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_clearonerow(unc_keytype_datatype_t db_name,
                                              DBTableSchema &db_table_schema) {
  uint16_t                    loop1 = 0;
  std::string                 prefix;
  std::ostringstream          clearone_query;
  std::vector<std::string> :: iterator  iter_keys;
  /** Get the table name prefix */
  if (db_name == UNC_DT_CANDIDATE)
    prefix = "c_";
  else if (db_name == UNC_DT_RUNNING)
    prefix = "r_";
  else if (db_name == UNC_DT_STARTUP)
    prefix = "s_";
  else if (db_name == UNC_DT_IMPORT)
    prefix = "i_";
  else if (db_name == UNC_DT_STATE)
    prefix = "r_";
  else
    return ODBCM_NULL_STRING;

  clearone_query << "DELETE FROM " << prefix << db_table_schema.table_name_;
  clearone_query << " WHERE ";
  /** Use only primary keys after WHERE in the sql query */
  std::vector <std::string> primarykeys = db_table_schema.get_primary_keys();
  for (loop1 = 0, iter_keys = primarykeys.begin();
      iter_keys != primarykeys.end(); iter_keys++, loop1++) {
    clearone_query << (*iter_keys).c_str() << " = ? ";
    if (loop1 != primarykeys.size()-1) {
      clearone_query << " AND ";
    }
  }  // for
  clearone_query << ";";
  pfc_log_info("ODBCM::QueryFactory::ClearOneRow: Query: \"%s\"",
      (clearone_query.str()).c_str());
  return clearone_query.str();
}

/**
 * @Description : This function will get the query statement handler for
 *                fetching a row in the database
 * @param[in]   : unc_keytype_datatype_t, DBTableSchema& 
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_getonerow(unc_keytype_datatype_t db_name,
                                           DBTableSchema &db_table_schema) {
  uint16_t            loop1 = 0;
  uint16_t            loop2 = 0;
  uint16_t            loop3 = 0;
  std::string         prefix;
  std::ostringstream  get_query;  // stream to store the query
  std::ostringstream  get_columns;  // stream to store the query
  std::vector<std::string> :: iterator           iter_keys;
  std::vector<TableAttrSchema> :: iterator       iter_vector;
  std::list<std::vector<TableAttrSchema> > ::iterator iter_list;
  /** Get the table name prefix */
  if (db_name == UNC_DT_CANDIDATE)
    prefix = "c_";
  else if (db_name == UNC_DT_RUNNING)
    prefix = "r_";
  else if (db_name == UNC_DT_STARTUP)
    prefix = "s_";
  else if (db_name == UNC_DT_IMPORT)
    prefix = "i_";
  else if (db_name == UNC_DT_STATE)
    prefix = "r_";
  else
    return ODBCM_NULL_STRING;

  get_query << "SELECT  ";
  /** Traverse the list to get the Attribute vector */
  /** In this case, this traversal will be only once since list
   *  contains only one row info */
  for (loop1 = 0, iter_list = db_table_schema.row_list_.begin();
      iter_list != db_table_schema.row_list_.end(); iter_list++, loop1++) {
    /** This vector contains all attributes of a row in a table */
    std::vector<TableAttrSchema>attributes_vector = *iter_list;
    /** Get the column names  and values */
    for (loop2 = 0, iter_vector = attributes_vector.begin();
        iter_vector != attributes_vector.end(); iter_vector++, loop2++) {
      get_query << (*iter_vector).table_attribute_name;
      if (loop2  !=  attributes_vector.size()-1) {
        get_query << " ,";
      }
    }  // for attribute vectors
  }  // for list of rows
  /** Use only primary keys after WHERE in the sql query */
  std::vector <std::string> primarykeys = db_table_schema.get_primary_keys();
  for (loop3 = 0, iter_keys = primarykeys.begin();
      iter_keys != primarykeys.end(); iter_keys++, loop3++) {
      get_columns << (*iter_keys).c_str() << " = ?";
    if (loop3 != primarykeys.size()-1) {
      get_columns << " AND ";
    }
  }  // for
  get_query << " FROM " << prefix << db_table_schema.table_name_ << " WHERE "
      << get_columns.str() << " ORDER BY " << primarykeys.front() <<";";

  pfc_log_info("ODBCM::QueryFactory::GetOneRow: Query: \"%s\"",
      (get_query.str()).c_str());
  return get_query.str();
}

/**
 * @Description : This function will check whether the given
 *                row exists in the given table
 * @param[in]   : unc_keytype_datatype_t,  DBTableSchema&
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_isrowexists(unc_keytype_datatype_t db_name,
                                             DBTableSchema &db_table_schema) {
  uint16_t                    loop1 = 0;
  std::string                 prefix;
  std::ostringstream          exists_query;
  std::vector <std::string>   primarykeys;
  std::vector<std::string> :: iterator  iter_keys;

  if (db_name == UNC_DT_CANDIDATE) {
     prefix = "c_";
     exists_query << "SELECT EXISTS(SELECT * FROM "<< prefix
         << db_table_schema.table_name_  << "), cs_row_status";
  } else if (db_name == UNC_DT_RUNNING) {
     prefix = "r_";
     exists_query << "SELECT EXISTS(SELECT * FROM "<< prefix
         << db_table_schema.table_name_  << ")";
  } else if (db_name == UNC_DT_STARTUP) {
     prefix = "s_";
     exists_query << "SELECT EXISTS(SELECT * FROM "<< prefix
         << db_table_schema.table_name_  << ")";
  } else if (db_name == UNC_DT_IMPORT) {
     prefix = "i_";
     exists_query << "SELECT EXISTS(SELECT * FROM "<< prefix
         << db_table_schema.table_name_  << ")";
  } else if (db_name == UNC_DT_STATE) {
     prefix = "r_";
     exists_query << "SELECT EXISTS(SELECT * FROM "<< prefix
         << db_table_schema.table_name_  << ")";
  } else {
    pfc_log_info("ODBCM::QueryFactory::operation_isrowexists:"
        "unc_keytype_datatype_t does not match ");
    return ODBCM_NULL_STRING;
  }

  exists_query << " from "<< prefix <<
      db_table_schema.table_name_ << " WHERE ";
  /** Use only primary keys after WHERE in the sql query */
  primarykeys = db_table_schema.get_primary_keys();
  for (loop1 = 0, iter_keys = primarykeys.begin();
      iter_keys != primarykeys.end(); iter_keys++, loop1++) {
    exists_query << (*iter_keys).c_str() << " = ?";
    if (loop1 != primarykeys.size()-1) {
      exists_query << " AND ";
    }
  }  // for
  exists_query << ";";
  pfc_log_info("ODBCM::QueryFactory::IsRowExists: Query: \"%s\"",
      (exists_query.str()).c_str());
  return exists_query.str();
}

/**
 * @Description : This function will get the query for fetching
 *                the modified rows in the table.
 * @param[in]   : unc_keytype_datatype_t, DBTableSchema&
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_getmodifiedrows
                                      (unc_keytype_datatype_t db_name,
                                       DBTableSchema &db_table_schema) {
  uint16_t            loop1 = 0;
  uint16_t            loop2 = 0;
  std::string         prefix;
  std::ostringstream  get_query, where_query;  // stream to store the query
  std::ostringstream  get_columns;  // stream to store the query
  std::vector<std::string> :: iterator                iter_keys;
  std::vector<TableAttrSchema> :: iterator            iter_vector;
  std::list<std::vector<TableAttrSchema> > ::iterator iter_list;
  /** Get the prefix for tablenaem based on db_tpe */
  if (db_name == UNC_DT_CANDIDATE)
    prefix = "c_";
  else if (db_name == UNC_DT_RUNNING)
    prefix = "r_";
  else if (db_name == UNC_DT_STARTUP)
    prefix = "s_";
  else if (db_name == UNC_DT_IMPORT)
    prefix = "i_";
  else if (db_name == UNC_DT_STATE)
    prefix = "r_";
  else
    return ODBCM_NULL_STRING;
  /** Start framing the query */
  get_query << "SELECT  ";
  /** Traverse the list to get the Attribute vector */
  /** In this case, this traversal will be only once since list
   *  contains only one row info */
  for (loop1 = 0, iter_list = db_table_schema.row_list_.begin();
      iter_list != db_table_schema.row_list_.end(); iter_list++, loop1++) {
    /** This vector contains all attributes of a row in a table */
    std::vector<TableAttrSchema>attributes_vector = *iter_list;
    /** Get the column names  and values */
    for (loop2 = 0, iter_vector = attributes_vector.begin();
        iter_vector != attributes_vector.end(); iter_vector++, loop2++) {
      /** Get attribute name of a row */
      get_query << (*iter_vector).table_attribute_name;
      if (loop2  !=  attributes_vector.size()-1)
        get_query << " ,";
      /** Frame where query part using row_status in db_table_schema */
      if (attributes_vector[loop2].table_attribute_name.
          compare("cs_row_status") == 0) {
        ColumnAttrValue <uint16_t> *rs_value =
            ((ColumnAttrValue <uint16_t>*)
                ((*iter_vector).p_table_attribute_value));
        where_query << "cs_row_status = " << rs_value->value;
        pfc_log_info("ODBCM::QueryFactory::GetModifiedRows: "
            "cs_row_status:%d", rs_value->value);
      }
    }  // for attribute vectors
  }  // for list of rows
  //  get primary key (first key) for order by clause
  std::vector <std::string> p_key = db_table_schema.get_primary_keys();
  std::string key_value = p_key.front();
  get_query << " FROM " << prefix
      << db_table_schema.table_name_
      << " WHERE " << where_query.str()
      << " order by " << key_value << ";";
  pfc_log_info("ODBCM::QueryFactory::GetModifiedRows(): Query: \"%s\"",
    (get_query.str()).c_str());
  return get_query.str();
}

/**
 * @Description : To frame query for copying the db in below cases.
 *                1. During bootup: Startup --> Candidate
 *                2. During backup: Running --> Startup
 *                3. During commit: Candidate --> Running
 * @param[in]   : unc_keytype_datatype_t, unc_keytype_datatype_t
 * @return      : SQLQUERY*
 **/
SQLQUERY* QueryFactory::operation_copydatabase(
    unc_keytype_datatype_t src_db_name,
    unc_keytype_datatype_t dst_db_name) {
  char                                 *p_src_table = NULL;
  char                                 *p_dst_table = NULL;
  uint16_t                             loop1 = 0, loop2 = 0;
  std::string                          *p_copy_db = NULL;
  ODBCManager                          *p_odbc_mgr = NULL;
  std::ostringstream                   copy_query;
  std::vector <std::string>                 src_vector, dst_vector;
  std::vector <std::string> :: iterator     iter_srcvector, iter_dstvector;
  std::map <int, std::vector<std::string> > db_table_list_map;

  /** Copy db should be called only for scenarios in Description */
  if (!((src_db_name == UNC_DT_STARTUP && dst_db_name == UNC_DT_CANDIDATE) ||
      (src_db_name == UNC_DT_CANDIDATE && dst_db_name == UNC_DT_RUNNING) ||
      (src_db_name == UNC_DT_RUNNING && dst_db_name == UNC_DT_CANDIDATE) ||
      (src_db_name == UNC_DT_RUNNING && dst_db_name == UNC_DT_STARTUP))) {
    pfc_log_error("ODBCM::QueryFactory::CopyDatabase: Invalid copy"
      " from %s->%s", g_log_db_name[src_db_name],
        g_log_db_name[dst_db_name]);
    return NULL;
  }

  p_odbc_mgr = ODBCManager::get_ODBCManager();
  /** Allocate memory to store the queries. For all the scenarios 
    * in copy db, the number of tables involved is 3 
    * Totally 6 queries (3 for clear dst_db and 3 for copying)*/
  p_copy_db = new std::string[2*ODBCM_MAX_UPPL_TABLES + 1];
  /** Null validation */
  if (p_copy_db == NULL) {
    pfc_log_error("ODBCM::QueryFactory::CopyDatabase: "
      "p_copy_db:%p is NULL", p_copy_db);
    return NULL;
  }
  /** Set the end_of_query offset */
  p_copy_db[2*ODBCM_MAX_UPPL_TABLES] = "end_of_query";
  /** If list size is 0, return */
  db_table_list_map = p_odbc_mgr->get_db_table_list_map_();
  if (db_table_list_map.size() <=  0) {
    delete [] p_copy_db;
    return NULL;
  }
  src_vector = (db_table_list_map.find(src_db_name))->second;
  dst_vector = (db_table_list_map.find(dst_db_name))->second;
  /** Frame the query for every table in DB */
  pfc_log_info("ODBCM::QueryFactory::CopyDatabase: %s->%s",
    g_log_db_name[src_db_name], g_log_db_name[dst_db_name]);

  /** Frame the query for clearing all the rows in given db*/
  for (loop1 = 0, iter_dstvector = dst_vector.begin();
    iter_dstvector != dst_vector.end(); iter_dstvector++, loop1++ ) {
    if ((*iter_dstvector).compare("r_"UPPL_CTR_DOMAIN_TABLE) == 0) {
      copy_query << "DELETE FROM " << (*iter_dstvector) <<
          " WHERE " << CTR_NAME << " IN (SELECT " << CTR_NAME <<
          " FROM r_" << UPPL_CTR_TABLE << " WHERE " << CTR_TYPE <<
          "=" << UNC_CT_UNKNOWN << ");";
    } else {
      copy_query << "TRUNCATE " << (*iter_dstvector) << ";";
    }
    pfc_log_info("ODBCM::QueryFactory::CopyDatabase: "
        "Clear Query:%d is \"%s\"", loop1, (copy_query.str()).c_str());
    p_copy_db[loop1] = copy_query.str();
    /** Reset the query */
    copy_query.str("");
  }
  /*swap the first two queries in order to do the delete operation
   * for ctr_domain_table before controller_table*/
  if (p_copy_db[0].compare("") != 0 && p_copy_db[1].compare("") != 0) {
    std::string tmp = p_copy_db[0];
    p_copy_db[0] = p_copy_db[1];
    p_copy_db[1] = tmp;
    pfc_log_info("ODBCM::QueryFactory::CopyDatabase: "
        "after Swap queries \n Query:[0] %s \n Query[1]: %s",
        p_copy_db[0].c_str(), p_copy_db[1].c_str());
  }
  /**to get the controller_table name of src database*/
  iter_srcvector = src_vector.begin();
  std::string ctr_table = (*iter_srcvector);
  for (loop2 = ODBCM_MAX_UPPL_TABLES, iter_srcvector = src_vector.begin(),
      iter_dstvector = dst_vector.begin();
      iter_srcvector != src_vector.end(),
      iter_dstvector != dst_vector.end();
      iter_srcvector++, iter_dstvector++, loop2++) {
    /** Take the pointer to _ and check both tables are same */
    p_src_table = strstr(const_cast <char*>((*iter_dstvector).c_str()), "_");
    p_dst_table = strstr(const_cast <char*>((*iter_srcvector).c_str()), "_");
    pfc_log_info("ODBCM::QueryFactory::CopyDatabase: Table ."
              "loop:%d, %s->%s", loop2, p_src_table, p_dst_table);
    /** Compare the tables */
    if (strcmp(p_src_table, p_dst_table) == 0) {
      /** Copy all the entries from src table to dst table */
      if (strcmp(p_src_table, "_"UPPL_CTR_DOMAIN_TABLE) == 0 &&
          strcmp(p_dst_table, "_"UPPL_CTR_DOMAIN_TABLE) == 0) {
        copy_query << "INSERT INTO " << (*iter_dstvector) <<
          " SELECT * FROM " << (*iter_srcvector) <<
          " WHERE " << CTR_NAME << " IN (SELECT " << CTR_NAME << " from " <<
          ctr_table.c_str() << " WHERE " << CTR_TYPE << "=" <<
          UNC_CT_UNKNOWN << ");";
        /*Unknown domain type = 0 (UNC_CT_UNKNOWN)*/
      } else {
        copy_query << "INSERT INTO " << (*iter_dstvector) <<
            " SELECT * FROM " << (*iter_srcvector) << " ;";
      }
      pfc_log_info("ODBCM::QueryFactory::CopyDatabase: "
          "Copy Query:%d is \"%s\"", loop2, (copy_query.str()).c_str());
      /** Index should be less than 3 */
      if (loop2 < 2*ODBCM_MAX_UPPL_TABLES)
        p_copy_db[loop2] = copy_query.str();
      /** Reset the query for next table */
      copy_query.str("");
    } else {
      pfc_log_info("ODBCM::QueryFactory::CopyDatabase: Table mismatch."
          "loop:%d, %s->%s", loop2, p_src_table, p_dst_table);
      /** Do nothing */
      continue;
    }
  }  // for
  return p_copy_db;
}
/**
 * @Description : This function will be called by PhysicalCore when IPC client
 * @param[in]   : unc_keytype_datatype_t
 * @return      : SQLQUERY*
 **/
SQLQUERY* QueryFactory::operation_cleardatabase
(unc_keytype_datatype_t db_name) {
  uint16_t               loop = 0;
  uint16_t               table_count = 0;
  std::string            *p_clear_db = NULL;
  ODBCManager            *p_odbc_mgr = NULL;
  std::ostringstream     clear_query;
  std::vector<std::string>                    db_vector;
  std::vector<std::string> :: iterator        iter_vector;
  std::map <int, std::vector<std::string> >   db_table_list_map;
  /** Allocate memory for query based on the db_type */
  if (db_name == UNC_DT_STARTUP) {
    table_count = ODBCM_MAX_STARTUP_TABLES;
  }  else if (db_name == UNC_DT_CANDIDATE) {
    table_count = ODBCM_MAX_CANDIDATE_TABLES;
  } else if (db_name == UNC_DT_RUNNING) {
    table_count = ODBCM_MAX_RUNNING_TABLES;
  } else if (db_name == UNC_DT_STATE) {
    table_count = ODBCM_MAX_STATE_TABLES+1;
    /*UNC_DT_CANDIDATE - CTR_DOMAIN table*/
  } else if (db_name == UNC_DT_IMPORT) {
    table_count = ODBCM_MAX_IMPORT_TABLES;
  } else {
    pfc_log_error("ODBCM::QueryFactory::ClearDatabase: "
      "Invalid request. db_name: %s", g_log_db_name[db_name]);
    return NULL;
  }
  p_odbc_mgr = ODBCManager::get_ODBCManager();
  p_clear_db = new std::string[table_count+1];
  /** Null validation */
  if (p_clear_db == NULL) {
    pfc_log_error("ODBCM::QueryFactory::ClearDatabase: "
      "p_clear_db:%p is NULL", p_clear_db);
    return NULL;
  }
  /** Get the table list map */
  db_table_list_map = p_odbc_mgr->get_db_table_list_map_();
  db_vector = db_table_list_map.find(db_name)->second;
  pfc_log_info("ODBCM::QueryFactory::ClearDatabase: %s",
    g_log_db_name[db_name]);
  /** Frame the query for clearing all the rows in given db*/
  for (loop = 0, iter_vector = db_vector.begin();
    iter_vector != db_vector.end(); iter_vector++, loop++ ) {
    clear_query << "TRUNCATE " << (*iter_vector) << ";";
    pfc_log_info("ODBCM::QueryFactory::ClearDatabase: "
        "Query:%d is \"%s\"", loop, (clear_query.str()).c_str());
    p_clear_db[loop] = clear_query.str();
    /** Reset the query */
    clear_query.str("");
  }

  if (db_name == UNC_DT_STATE) {
    clear_query << "DELETE FROM " << "r_"UPPL_CTR_DOMAIN_TABLE <<
        " WHERE " << CTR_NAME << " IN (SELECT " << CTR_NAME <<
        " FROM r_" << UPPL_CTR_TABLE << " WHERE " << CTR_TYPE <<
        "!=" << UNC_CT_UNKNOWN << ");";
    pfc_log_info("ODBCM::QueryFactory::ClearDatabase: "
        "CTR_DOMAIN Query is:%d  \"%s\"", loop, (clear_query.str()).c_str());
    p_clear_db[loop] = clear_query.str();
    /** Reset the query */
    clear_query.str("");
  }
  p_clear_db[table_count] = "end_of_query";
  return p_clear_db;
}

/**
 * @Description : To clear all controller entries from in db
 * @param[in]   : unc_keytype_datatype_t, string&
 * @return      : SQLQUERY*
 **/
SQLQUERY* QueryFactory::operation_clearoneinstance(
    unc_keytype_datatype_t db_name, std::string& controller_name) {
  uint16_t                        loop1 = 0;
  uint16_t                        table_count = 0;
  std::ostringstream              query;
  std::string                     *p_clear_inst_query = NULL;
  ODBCManager                     *p_odbc_mgr = NULL;
  std::vector<std::string>                  src_vector;
  std::vector<std::string> :: iterator      iter_srcvector;
  std::map <int, std::vector<std::string> > db_table_list_map;
  /** Get the number of tables in the given db */
  if (db_name == UNC_DT_STARTUP) {
    table_count = ODBCM_MAX_STARTUP_TABLES;
  }  else if (db_name == UNC_DT_CANDIDATE) {
    table_count = ODBCM_MAX_CANDIDATE_TABLES;
  } else if (db_name == UNC_DT_RUNNING) {
    table_count = ODBCM_MAX_RUNNING_TABLES;
  } else if (db_name == UNC_DT_STATE) {
    table_count = ODBCM_MAX_STATE_TABLES + 1;
    /*+1 to add the DT_STATE ctr_domain_table delete*/
  } else if (db_name == UNC_DT_IMPORT) {
    table_count = ODBCM_MAX_IMPORT_TABLES;
  } else {
    pfc_log_error("ODBCM::QueryFactory::ClearOneInstance: "
      "Invalid request. db_name: %s", g_log_db_name[db_name]);
    return NULL;
  }
  p_odbc_mgr = ODBCManager::get_ODBCManager();
  /** Allocate memory for storing the query */
  p_clear_inst_query = new std::string[table_count+1];
  if (p_clear_inst_query == NULL) {
    pfc_log_error("ODBCM::QueryFactory::ClearOneInstance: "
        "p_clear_inst_query:%p is NULL", p_clear_inst_query);
    return NULL;
  }
  p_clear_inst_query[table_count] = "end_of_query";
  db_table_list_map = p_odbc_mgr->get_db_table_list_map_();
  /** If no entry in map return NULL */
  if (db_table_list_map.size() <= 0) {
    delete []p_clear_inst_query;
    return NULL;
  }
  /** Get the tables specific to database type */
  src_vector = (db_table_list_map.find(db_name))->second;
  for (loop1 = 0, iter_srcvector = src_vector.begin();
      iter_srcvector != src_vector.end();
      iter_srcvector++, loop1++ ) {
    query << "DELETE FROM " << (*iter_srcvector) << " WHERE ";
    char *p_table = const_cast <char*>((*iter_srcvector).c_str());
    if ((strcmp(p_table, "s_boundary_table") == 0) ||
        (strcmp(p_table, "c_boundary_table") == 0) ||
        (strcmp(p_table, "r_boundary_table") == 0) ||
        (strcmp(p_table, "i_boundary_table") == 0)) {
      query << "controller_name1 ='" << controller_name.c_str() << "' OR ";
      query << "controller_name2 ='" << controller_name.c_str() << "';";
    } else {
      query << "controller_name ='" << controller_name.c_str() << "';";
    }
    pfc_log_info("ODBCM::QueryFactory::ClearOneInstance(): "
        "Query:%d is \"%s\"", loop1, (query.str()).c_str());
    if (loop1 < table_count)
      p_clear_inst_query[loop1] = query.str();
    query.str("");
  }
  if (db_name == UNC_DT_STATE) {
    query << "DELETE FROM " << "r_"UPPL_CTR_DOMAIN_TABLE << " WHERE ";
    if (loop1 < table_count) {
      query << "controller_name ='" << controller_name.c_str() << "';";
      p_clear_inst_query[loop1] = query.str();
    }
    pfc_log_info("ODBCM::QueryFactory::ClearOneInstance(): "
        "Query:%d is \"%s\"", loop1, (query.str()).c_str());
    query.str("");
  }
  return p_clear_inst_query;
}

/**
 * @Description : To frame the query to check candidate db is 
 *                dirty or not. i.e. To check is there any row
 *                entry with cs_row_status != APPLIED in db.
 *                cs_row_status 
 * @param[in]   : None
 * @return      : SQLQUERY*
 **/
SQLQUERY* QueryFactory::operation_iscandidatedirty() {
  uint16_t                          loop1 = 0;
  std::string                       *p_cand_db = NULL;
  ODBCManager                       *p_odbc_mgr = NULL;
  std::ostringstream                isdirty_query;
  std::vector<std::string>                  cand_vector;
  std::map<int, std::vector<std::string> >  db_table_list_map;
  std::vector<std::string> :: iterator      iter_vector;
  /** Get odbcmanager object */
  p_odbc_mgr = ODBCManager::get_ODBCManager();
  /** Allocate memory to store the queries */
  p_cand_db = new std::string[ODBCM_MAX_CANDIDATE_TABLES];
  /** Null validation */
  if (p_cand_db == NULL) {
    pfc_log_error("ODBCM::QueryFactory::IsCandidateDirty: "
        "p_cand_db:%p is NULL", p_cand_db);
    return NULL;
  }
  /** Get the table list map*/
  db_table_list_map = p_odbc_mgr->get_db_table_list_map_();
  if (db_table_list_map.size() <= 0) {
    pfc_log_debug("ODBCM::QueryFactory::IsCandidateDirty: "
      "Table list size is invalid:%d",
      static_cast<int>(db_table_list_map.size()));
    delete []p_cand_db;
    return NULL;
  }
  /** Get the list of tables mapped to candidate database */
  cand_vector = (db_table_list_map.find(UNC_DT_CANDIDATE))->second;
  /** Frame the query for every table in CANDIDATE_DB */
  for (loop1 = 0, iter_vector = cand_vector.begin();
    iter_vector != cand_vector.end(), loop1 < ODBCM_MAX_CANDIDATE_TABLES;
    iter_vector++, loop1++ ) {
    /** Reset the query for next table */
    isdirty_query.str("");
    isdirty_query << "SELECT * FROM " << (*iter_vector)
      <<" WHERE cs_row_status != " << APPLIED << ";";
    pfc_log_info("ODBCM::QueryFactory::IsCandidateDirty: "
        "Query:%d is \"%s\"", loop1, (isdirty_query.str()).c_str());
    p_cand_db[loop1] = isdirty_query.str();
  }
  return p_cand_db;
}

/**
 * @Description : This function will be called by PhysicalCore 
 *                when IPC client When max_rep_ct is 0, No rows 
 *                has to be returned,As per NEC expectation, 
 *                when max_rep_ct is 0, it will be success case 
 *                but with no value structure 
 *                For all getbulk operation, including sibling, 
 *                db can perform the following 
 *                1.  Sort the rows based on primary key. 
 *                  e.g. let us consider 4 controller sorted in db.
 *                  controller1
 *                  controller3
 *                  controller4
 *                  controller5
 *                2.  With Controller1 as key, If VTN requests 
 *                ReadBulk with max_rep_ct 3, kt_controller will call
 *                GetBulk with max_rep_ct as 3.  DB will return 
 *                Controller3, Controller4 and Controller5 
 *                If VTN requests ReadNext , kt_controller will call 
 *                GetBulk with max_rep_ct as 1. DB will return Controller3 
 *                If VTN requests ReadSibling with max_rep_ct 2, 
 *                kt_controller will call GetBulk with max_rep_ct as 2. 
 *                DB will return Controller3 and Controller4
 *                If VTN requests ReadSiblingBegin , kt_controller will 
 *                call GetBulk with max_rep_ct as 1. DB will return Controller3
 *                3.  If VTN queries for Controller2 which is non-existent,
 *                DB can return from Controller3 instance
 *                4.  If VTN queries without any value for Controller 
 *                primary key, DB can return from first controller instance 
 *                i.e Controller1
 * @param[in]   : unc_keytype_datatype_t,
 *                uint32_t, 
 *                DBTableSchema, 
 *                unc_keytype_operation_t
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_getbulkrows(unc_keytype_datatype_t db_name,
                                            uint32_t max_repetition_count,
                                            DBTableSchema &db_table_schema,
                                            unc_keytype_operation_t op_type) {
  /** Initialise the local variables */
  uint32_t              loop1 = 0;
  uint32_t              loop2 = 0;
  uint32_t              loop3 = 0;
  std::string           prefix;
  std::ostringstream    getbulk_query;
  std::ostringstream    getbulk_primarykey;
  std::ostringstream    getbulk_where;
  std::vector<std::string> :: iterator                iter_primarykey;
  std::vector<TableAttrSchema> :: iterator            iter_vector;
  std::list<std::vector<TableAttrSchema> > ::iterator iter_list;
  /** Get the prefix for tablenaem based on db_tpe */
  if (db_name == UNC_DT_CANDIDATE) {
     prefix = "c_";
  } else if (db_name == UNC_DT_RUNNING) {
     prefix = "r_";
  } else if (db_name == UNC_DT_STARTUP) {
     prefix = "s_";
  } else if (db_name == UNC_DT_IMPORT) {
     prefix = "i_";
  } else if (db_name == UNC_DT_STATE) {
     prefix = "r_";
  } else {
    return ODBCM_NULL_STRING;
  }
  getbulk_query << "SELECT  ";
  getbulk_where << " WHERE ";
  /** Traverse the list to get the Attribute vector In this case,
   * this traversal will be only once since list contains only one row info */
  for (loop1 = 0, iter_list = db_table_schema.row_list_.begin();
      iter_list != db_table_schema.row_list_.end(); iter_list++, loop1++) {
    /** This vector contains all attributes of a row in a table */
    std::vector<TableAttrSchema>attributes_vector = *iter_list;
    /** Get the column names  and values */
    for (loop2 = 0, iter_vector = attributes_vector.begin();
        iter_vector != attributes_vector.end(); iter_vector++, loop2++) {
      getbulk_query << (*iter_vector).table_attribute_name;
      if (loop2  !=  attributes_vector.size()-1)
        getbulk_query << ",";
    }  // for attribute vectors
    //  Where clause query framing
    /*Traverse the vector to get the primarykeys to frame order by query*/
    for (loop3 = 0, iter_primarykey = db_table_schema.primary_keys_.begin();
        iter_primarykey  !=  db_table_schema.primary_keys_.end();
        iter_primarykey++, loop3++) {
      //  adding primarykeys and values into query
      if ((loop3  !=  db_table_schema.primary_keys_.size()-1) &&
          ((op_type == UNC_OP_READ_BULK) ||
           (op_type == UNC_OP_READ_SIBLING) ||
           (op_type == UNC_OP_READ_SIBLING_BEGIN))) {
        // to support read sibling xx
        getbulk_where << (*iter_primarykey) << " = ?";
        getbulk_where << " and ";
        getbulk_primarykey << (*iter_primarykey) << ",";
      } else {
        getbulk_where << (*iter_primarykey) << " > ?";
        getbulk_primarykey << (*iter_primarykey);
      }
    }  // for primarykey vector
  }  // for list of rows

  getbulk_query << " FROM " << prefix << db_table_schema.table_name_ <<
  getbulk_where.str() << "  ORDER BY  " << getbulk_primarykey.str()
  << " ASC  LIMIT " << max_repetition_count;

  pfc_log_info("ODBCM::QueryFactory::GetBulkRows: Query is \"%s\"",
      (getbulk_query.str()).c_str());

  return getbulk_query.str();
}

/**
 * @Description : This method is invoked when getsiblingcount
 *                request comes, this will fetch the sibling count from table
 * @param[in]   : unc_keytype_datatype_t, DBTableSchema&
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_getsiblingcount(
        unc_keytype_datatype_t db_name,
        DBTableSchema &db_table_schema) {
  /** Initialise the local variables */
  uint32_t            loop1 = 0;
  uint8_t             orderby_flag = 0, speccondition_flag = 0;
  std::string         prefix;
  std::ostringstream  query, orderby_query, special_condition;
  std::vector <std::string>             primarykeys;
  std::vector<std::string> :: iterator  iter_keys;
  /** Based on the db type, get the prefix for table name */
  if (db_name == UNC_DT_CANDIDATE) {
    prefix = "c_";
    special_condition << " AND cs_row_status != " << DELETED << " ";
    speccondition_flag = 1;
  } else if (db_name == UNC_DT_RUNNING) {
    prefix = "r_";
    if (db_table_schema.table_name_.compare(UPPL_CTR_TABLE) == 0 ||
        db_table_schema.table_name_.compare(UPPL_CTR_DOMAIN_TABLE) == 0 ||
        db_table_schema.table_name_.compare(UPPL_BOUNDARY_TABLE) ==0) {
      special_condition << " AND cs_row_status != " << DELETED << " ";
      speccondition_flag = 1;
    }
  } else if (db_name == UNC_DT_STARTUP) {
    prefix = "s_";
  } else if (db_name == UNC_DT_IMPORT) {
    prefix = "i_";
  } else if (db_name == UNC_DT_STATE) {
    prefix = "r_";
  } else {
    return ODBCM_NULL_STRING;
  }
  query << "SELECT * FROM " << prefix << db_table_schema.table_name_
    << " WHERE ";
  /** Get the primary keys */
  primarykeys = db_table_schema.get_primary_keys();
  for (loop1 = 0, iter_keys = primarykeys.begin();
      iter_keys != primarykeys.end(); iter_keys++, loop1++) {
    /** Fill the primary keys in the query */
    if (loop1  !=  db_table_schema.primary_keys_.size()-1) {
      query << (*iter_keys).c_str() << " = ? ";
      query << " AND ";
    } else {
      query << (*iter_keys).c_str() << " > ? ";
    }
    /** In ORDER BY, we always take the first primary key */
    if (0 == orderby_flag) {
      orderby_query << " ORDER BY " << (*iter_keys).c_str() << " ;";
      orderby_flag = 1;
    }
  }
  /* Fill the table name in the query */
  /* Example: Query should as below */
  /* SELECT controller_name FROM c_controller_common_table WHERE
   * controller_name > ? ORDER BY controller_name */
  if (speccondition_flag == 1) {
    query << special_condition.str();
  }
  query << orderby_query.str();
  pfc_log_info("ODBCM::QueryFactory::GetSiblingCount:: "
    "Query is \"%s\"", (query.str()).c_str());
  return query.str();
}
/**
 * @Description : To frame query for getting the sibling count based upon
 * filter options in request.
 * @param[in]   : unc_keytype_datatype_t, DBTableSchema&, vector<ODBCMOperator>
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_getsiblingcount_with_filter(
    unc_keytype_datatype_t db_name,
    DBTableSchema& db_table_schema,
    std::vector<ODBCMOperator> filter_operators) {
  std::string prefix;
  uint8_t             speccondition_flag = 0;
  /**stream to store the query*/
  std::ostringstream query, orderby_query, special_condition;
  uint32_t loop1 = 0;
  /**To traverse the primary key vector*/
  std::vector<std::string> :: iterator iter_keys;
  /**To traverse the primary key vector*/
  std::vector<ODBCMOperator> :: iterator iter_operators;
  uint8_t orderby_flag = 0;
  /* Based on the db type, get the prefix for table name */
  if (db_name == UNC_DT_CANDIDATE) {
    prefix = "c_";
    special_condition << " AND cs_row_status != " << DELETED << " ";
    speccondition_flag = 1;
  } else if (db_name == UNC_DT_RUNNING) {
    prefix = "r_";
    if (db_table_schema.table_name_.compare(UPPL_CTR_TABLE) == 0 ||
        db_table_schema.table_name_.compare(UPPL_CTR_DOMAIN_TABLE) == 0 ||
        db_table_schema.table_name_.compare(UPPL_BOUNDARY_TABLE) ==0) {
      special_condition << " AND cs_row_status != " << DELETED << " ";
      speccondition_flag = 1;
    }
  } else if (db_name == UNC_DT_STARTUP) {
    prefix = "s_";
  } else if (db_name == UNC_DT_IMPORT) {
    prefix = "i_";
  } else if (db_name == UNC_DT_STATE) {
    prefix = "r_";
  } else {
    return ODBCM_NULL_STRING;
  }
  query << "SELECT * FROM " << prefix << db_table_schema.table_name_
    << " WHERE ";
  /* Get the primary keys */
  std::vector <std::string> primarykeys = db_table_schema.get_primary_keys();
  for (loop1 = 0, iter_keys = primarykeys.begin(),
      iter_operators = filter_operators.begin();
      iter_keys != primarykeys.end(); iter_keys++, iter_operators++, loop1++) {
    /* Fill the primary keys in the query */
    const int op = static_cast<int>(*iter_operators);
    switch (op) {
      case UNKNOWN_OPERATOR:
        query << (*iter_keys).c_str() << " = ? ";
        pfc_log_debug("ODBCM::QueryFactory::GetSiblingCount(filter)::"
            "invalid operator found");
        return ODBCM_NULL_STRING;
        break;
      case EQUAL:
        query << (*iter_keys).c_str() << " = ? ";
        break;
      case NOT_EQUAL:
        query << (*iter_keys).c_str() << " != ? ";
        break;
      case GREATER:
        query << (*iter_keys).c_str() << " > ? ";
        break;
      case GREATER_EQUAL:
        query << (*iter_keys).c_str() << " >= ? ";
        break;
      case LESSER:
        query << (*iter_keys).c_str() << " < ? ";
        break;
      case LESSER_EQUAL:
        query << (*iter_keys).c_str() << " <= ? ";
        break;
      default:
        query << (*iter_keys).c_str() << " = ? ";
        pfc_log_debug("ODBCM::QueryFactory::GetSiblingCount(filter)::"
            "invalid operator found");
        return ODBCM_NULL_STRING;
        break;
    }
    /* In ORDER BY, we always take the first primary key */
    if (0 == orderby_flag) {
      orderby_query << " ORDER BY " << (*iter_keys).c_str() << " ;";
      orderby_flag = 1;
    }
    if (loop1 != primarykeys.size()-1) {
      query << " AND ";
    }
  }
  /** Fill the table name in the query */
  /** Example: Query should as below */
  /** SELECT controller_name FROM c_controller_common_table WHERE
   * controller_name > ? ORDER BY controller_name */
  if (speccondition_flag == 1) {
    query << special_condition.str();
  }
  query << orderby_query.str();
  pfc_log_info("ODBCM::QueryFactory::GetSiblingCount:: "
    "Query is \"%s\"", (query.str()).c_str());
  return query.str();
}

/**
 * @Description : To frame query for getting the sibling rows based upon
 * filter options in request.
 * @param[in]   : unc_keytype_datatype_t, DBTableSchema&, vector<ODBCMOperator>
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_getsiblingrows(
    unc_keytype_datatype_t      db_name,
    DBTableSchema&              db_table_schema,
    uint32_t                    max_repetition_count,
    std::vector<ODBCMOperator>  filter_operators,
    unc_keytype_operation_t     op_type) {
  std::string           prefix;
  std::ostringstream    getsibling_query;
  uint32_t              loop1, loop2, loop3;
  std::ostringstream    getsibling_primarykey;
  std::ostringstream    getsibling_where;
  std::vector<ODBCMOperator> :: iterator                iter_operators;
  std::vector<std::string> :: iterator                  iter_primarykey;
  std::vector<TableAttrSchema> :: iterator              iter_vector;
  std::list <std::vector<TableAttrSchema> > ::iterator  iter_list;

  if (db_name == UNC_DT_CANDIDATE) {
     prefix = "c_";
  } else if (db_name == UNC_DT_RUNNING) {
     prefix = "r_";
  } else if (db_name == UNC_DT_STARTUP) {
     prefix = "s_";
  } else if (db_name == UNC_DT_IMPORT) {
     prefix = "i_";
  } else if (db_name == UNC_DT_STATE) {
     prefix = "r_";
  } else {
    return ODBCM_NULL_STRING;
  }
  getsibling_query << "SELECT  ";
  getsibling_where << " where ";
  /** Traverse the list to get the Attribute vector In this case,
   * this traversal will be only once since list contains only one row info */
  for (loop1 = 0, iter_list = db_table_schema.row_list_.begin();
      iter_list != db_table_schema.row_list_.end(); iter_list++, loop1++) {
    /* This vector contains all attributes of a row in a table */
    std::vector<TableAttrSchema>attributes_vector = *iter_list;
    /* Get the column names  and values */
    for (loop2 = 0, iter_vector = attributes_vector.begin();
        iter_vector != attributes_vector.end(); iter_vector++, loop2++) {
      getsibling_query << (*iter_vector).table_attribute_name;
      if (loop2  !=  attributes_vector.size()-1)
        getsibling_query << ",";
    }  // for attribute vectors
    //  Where clause query framing
    /*Traverse the vector to get the primarykeys to frame order by query*/
    for (loop3 = 0, iter_primarykey = db_table_schema.primary_keys_.begin(),
        iter_operators = filter_operators.begin();
        iter_primarykey  !=  db_table_schema.primary_keys_.end();
        iter_primarykey++, iter_operators++, loop3++) {
      const int op = static_cast<int>(*iter_operators);
      switch (op) {
        case UNKNOWN_OPERATOR:
          getsibling_where << (*iter_primarykey) << " = ? ";
          pfc_log_debug("ODBCM::QueryFactory::GetSiblingRows(filter)::"
              "invalid operator found");
          return ODBCM_NULL_STRING;
          break;
        case EQUAL:
          getsibling_where << (*iter_primarykey) << " = ? ";
          break;
        case NOT_EQUAL:
          getsibling_where << (*iter_primarykey) << " != ? ";
          break;
        case GREATER:
          getsibling_where << (*iter_primarykey) << " > ? ";
          break;
        case GREATER_EQUAL:
          getsibling_where << (*iter_primarykey) << " >= ? ";
          break;
        case LESSER:
          getsibling_where << (*iter_primarykey) << " < ? ";
          break;
        case LESSER_EQUAL:
          getsibling_where << (*iter_primarykey) << " <= ? ";
          break;
        default:
          getsibling_where << (*iter_primarykey) << " = ? ";
          pfc_log_debug("ODBCM::QueryFactory::GetSiblingRows(filter)::"
              "invalid operator found");
          return ODBCM_NULL_STRING;
          break;
      }
      if (loop3  !=  db_table_schema.primary_keys_.size()-1) {
        getsibling_where << " and ";
        getsibling_primarykey << (*iter_primarykey) << ",";
      } else {
        getsibling_primarykey << (*iter_primarykey);
      }
    }  // for primarykey vector
  }  // for list of rows

  getsibling_query << " FROM " << prefix << db_table_schema.table_name_ <<
  getsibling_where.str() << "  ORDER BY  " << getsibling_primarykey.str()
  << " ASC  LIMIT " << max_repetition_count;
  pfc_log_info("ODBCM::QueryFactory::GetSiblingRows: Query is \"%s\"",
        (getsibling_query.str()).c_str());
  return getsibling_query.str();
}

/**
 * @Description : This method is invoked when getsiblingcount
 *                request comes, this will fetch the sibling count from table
 * @param[in]   : unc_keytype_datatype_t, std::string
 * @return      : SQLQUERY
 **/
SQLQUERY QueryFactory::operation_getrowcount(unc_keytype_datatype_t db_name,
                                            std::string table_name) {
  std::string         prefix;
  std::ostringstream  query;
  /** Based on the db type, get the prefix for table name */
  if (db_name == UNC_DT_CANDIDATE)
    prefix = "c_";
  else if (db_name == UNC_DT_RUNNING)
    prefix = "r_";
  else if (db_name == UNC_DT_STARTUP)
    prefix = "s_";
  else if (db_name == UNC_DT_IMPORT)
    prefix = "i_";
  else if (db_name == UNC_DT_STATE)
    prefix = "r_";
  else
    return ODBCM_NULL_STRING;
  query << "SELECT * FROM " << prefix << table_name;
  pfc_log_info("ODBCM::QueryFactory::GetRowCount:: "
    "Query is \"%s\"", (query.str()).c_str());
  return query.str();
}

/**
 * @Description : To commit all the configuration from candidate to 
 *                running database. Below operations follows after 
 *                invoking this method.
 *                STEP1: Change row_status in Candidate db 
 *                STEP1.1: Change row_status CREATED/UPDATED --> APPLIED
 *                STEP1.2: Remove row_staus(DELETED) drom the table
 *                STEP2: TRUNCATE all the tables in Running db
 *                STEP3: Copy Candidate->Running if above steps are success
 *                copydatabase will be reused for step3.
 *
 * @param[in]   : unc_data_type_t src_db,
 *                unc_data_type_t dst_db,
 *                uint32_t query_type
 * @return      : SQLQUERY
 **/
SQLQUERY* QueryFactory::operation_commit_all_config(
    unc_keytype_datatype_t src_db_name,
    unc_keytype_datatype_t dst_db_name,
    uint32_t query_type) {
  /** Initialise the local variables */
  uint16_t                                            loop = 0;
  uint16_t                                            index = 0;
  std::string                                         *p_commit_db = NULL;
  ODBCManager                                         *p_odbc_mgr = NULL;
  std::ostringstream                                  commit_query;
  std::vector<std::string>                            src_vector;
  std::vector<std::string>                            dst_vector;
  std::vector<std::string> :: iterator                iter_srcvector;
  std::vector<std::string> :: iterator                iter_dstvector;
  std::map<int, std::vector<std::string> >            db_table_list_map;
  std::map<int, std::vector<std::string> >::iterator  iter_src;
  std::map<int, std::vector<std::string> >::iterator  iter_dst;
  /** Always src->dst should be candidate->running */
  if (src_db_name != UNC_DT_CANDIDATE ||
      dst_db_name != UNC_DT_RUNNING) {
    pfc_log_error("ODBCM::QueryFactory::CommitAllConfiguration: "
      "Invalid src_db:%d or dst_db:%d", src_db_name, dst_db_name);
  }
  /** Get the odbc manager object */
  p_odbc_mgr  = ODBCManager::get_ODBCManager();
  /** Allocate memory for query */
  p_commit_db = new std::string[ODBCM_MAX_UPPL_TABLES+1];
  /** Null validation */
  if (p_commit_db == NULL) {
    pfc_log_error("ODBCM::QueryFactory::CommitAllConfiguration: "
      "p_commit_db:%p is NULL", p_commit_db);
    return NULL;
  }
  p_commit_db[ODBCM_MAX_UPPL_TABLES] = "end_of_query";
  /** Get the table list */
  db_table_list_map = p_odbc_mgr->get_db_table_list_map_();
  /** Table list is empty, return NULL */
  if (db_table_list_map.size() <= 0) {
    pfc_log_debug("ODBCM::QueryFactory::CommitAllConfiguration: "
      "Table list size is invalid:%d",
      static_cast<int>(db_table_list_map.size()));
    delete []p_commit_db;
    return NULL;
  }
  /** Get the table vector for both candidate and running */
  src_vector = (db_table_list_map.find(src_db_name))->second;
  dst_vector = (db_table_list_map.find(dst_db_name))->second;

  switch (query_type) {
    case CHANGE_ROWSTATUS1:
      /** STEP1: Change row_status in Candidate db */
      /** STEP1.1: Change row_status CREATED/UPDATED --> APPLIED */
      for (loop = 0, iter_srcvector = src_vector.begin();
          iter_srcvector != src_vector.end(); iter_srcvector++, loop++) {
        commit_query << "UPDATE " << (*iter_srcvector) << " SET "
          << "cs_row_status = " << APPLIED << " WHERE cs_row_status = "
          << CREATED << " OR cs_row_status = " << UPDATED
          << " OR cs_row_status = " << ROW_VALID
          << " OR cs_row_status = " << NOTAPPLIED
          << " OR cs_row_status = " << PARTIALLY_APPLIED << ";";
        p_commit_db[loop] = commit_query.str();
        pfc_log_info("ODBCM::QueryFactory::CommitAllConfiguration: "
            "CHANGE_ROWSTATUS1:Query:%d is \"%s\"", loop,
            (commit_query.str()).c_str());
        commit_query.str("");
      }
      break;
    case CHANGE_ROWSTATUS2:
      /** STEP1.2: Remove row_staus(DELETED) drom the table */
      for (loop = 0, iter_srcvector = src_vector.begin();
          iter_srcvector != src_vector.end(); iter_srcvector++, loop++) {
        commit_query << "DELETE FROM " << (*iter_srcvector) << " WHERE "
          << " cs_row_status = " << DELETED
          << " OR cs_row_status = " << ROW_INVALID << ";";
        p_commit_db[loop] = commit_query.str();
        pfc_log_info("ODBCM::QueryFactory::CommitAllConfiguration: "
            "CHANGE_ROWSTATUS2:Query:%d is \"%s\"", loop,
            (commit_query.str()).c_str());
        commit_query.str("");
      }
      break;
    case COPY_STATE_INFO:
      /**STEP1.3: Copy state information from running to candidate*/
      loop = 0;
      /**copy the actual_version, oper_status STATE details into 
      * candidate controller_table*/
      commit_query << "UPDATE " <<  "c_"UPPL_CTR_TABLE <<
      " SET " << CTR_ACTUAL_VERSION << " = " << "r_"UPPL_CTR_TABLE << "." <<
      CTR_ACTUAL_VERSION << ", " << CTR_OPER_STATUS << " = " <<
      "r_"UPPL_CTR_TABLE << "." << CTR_OPER_STATUS << " from " <<
      "r_"UPPL_CTR_TABLE << " where " << "c_"UPPL_CTR_TABLE <<
      "." << CTR_NAME << " = " << "r_"UPPL_CTR_TABLE << "." <<
      CTR_NAME << ";";
      p_commit_db[loop] = commit_query.str();
      pfc_log_info("ODBCM::QueryFactory::CommitAllConfiguration: "
          "COPY_STATE_INFO:Query:%d is \"%s\"", loop,
          (commit_query.str()).c_str());
      commit_query.str("");
      loop++;
      /**copy oper_status STATE details into candidate ctr_domain_table*/
      commit_query << "UPDATE " <<  "c_"UPPL_CTR_DOMAIN_TABLE <<
      " SET " << CTR_OPER_STATUS << " = " <<
      "r_"UPPL_CTR_DOMAIN_TABLE << "." << CTR_OPER_STATUS << " from " <<
      "r_"UPPL_CTR_DOMAIN_TABLE << " where " << "c_"UPPL_CTR_DOMAIN_TABLE <<
      "." << CTR_NAME << " = " << "r_"UPPL_CTR_DOMAIN_TABLE << "." <<
      CTR_NAME << ";";
      p_commit_db[loop] = commit_query.str();
      pfc_log_info("ODBCM::QueryFactory::CommitAllConfiguration: "
          "COPY_STATE_INFO:Query:%d is \"%s\"", loop,
          (commit_query.str()).c_str());
      commit_query.str("");
      loop++;
      /**copy oper_status STATE details into candidate boundary_table*/
      commit_query << "UPDATE " <<  "c_"UPPL_BOUNDARY_TABLE <<
      " SET " << CTR_OPER_STATUS << " = " <<
      "r_"UPPL_BOUNDARY_TABLE << "." << CTR_OPER_STATUS << " from " <<
      "r_"UPPL_BOUNDARY_TABLE << " where " << "c_"UPPL_BOUNDARY_TABLE <<
      "." << BDRY_ID << " = " << "r_"UPPL_BOUNDARY_TABLE<< "." <<
      BDRY_ID << ";";
      p_commit_db[loop] = commit_query.str();
      pfc_log_info("ODBCM::QueryFactory::CommitAllConfiguration: "
          "COPY_STATE_INFO:Query:%d is \"%s\"", loop,
          (commit_query.str()).c_str());
      commit_query.str("");
      break;
    case TRUNCATE_RUNNING:
      /** STEP2: TRUNCATE all the tables in Running db */
      for (loop = 0, index = 0, iter_dstvector = dst_vector.begin();
          iter_dstvector != dst_vector.end(); iter_dstvector++, loop++) {
        char *p_table = const_cast <char*>((*iter_dstvector).c_str());
          if (strcmp(p_table, "r_"UPPL_CTR_DOMAIN_TABLE) == 0) {
            commit_query << "DELETE FROM " << (*iter_dstvector) <<
                " WHERE " << CTR_NAME << " IN (SELECT " << CTR_NAME <<
                " FROM r_" << UPPL_CTR_TABLE <<" WHERE " << CTR_TYPE <<
                "=" << UNC_CT_UNKNOWN << ");";
        }
        if ((strcmp(p_table, "r_"UPPL_CTR_TABLE) == 0) ||
            (strcmp(p_table, "r_"UPPL_BOUNDARY_TABLE) == 0)) {
            commit_query << "TRUNCATE " << (*iter_dstvector) << ";";
          }
          if (index >= ODBCM_MAX_UPPL_TABLES) {
            delete []p_commit_db;
            return NULL;
          }

          p_commit_db[index] = commit_query.str();
          pfc_log_info("ODBCM::QueryFactory::CommitAllConfiguration: "
              "TRUNCATE_RUNNING:Query:%d is \"%s\"", loop,
               (commit_query.str()).c_str());
          commit_query.str("");
          index++;
      }
      /*swap the first two queries in order to do the delete operation
       * for ctr_domain_table before controller_table*/
      if (p_commit_db[0].compare("") != 0 && p_commit_db[1].compare("") != 0) {
        std::string tmp = p_commit_db[0];
        p_commit_db[0] = p_commit_db[1];
        p_commit_db[1] = tmp;
        pfc_log_info("ODBCM::QueryFactory::CommitAllConfiguration: "
            "after Swap queries \n Query:[0] %s \n Query[1]: %s",
            p_commit_db[0].c_str(), p_commit_db[1].c_str());
      }
      break;
    case COPY_CANDIDATE_TO_RUNNING:
      /** STEP4: Copy database from Candidate to Running */
      /** Do Nothing */
      break;
    default:
        pfc_log_info("ODBCM::QueryFactory::CommitAllConfiguration: "
          "Invalid operation");
      break;
  }
  return p_commit_db;
}

/**
 * @Description : This function set the function pointer with
 *                appropriate method to frame thw query
 * @param[in]   : int
 * @return      : None
 **/
void QueryFactory::SetOperation(int operation_type) {
  switch (operation_type) {
    case CREATEONEROW:
      GetQuery = &QueryFactory::operation_createonerow;
      break;
    case UPDATEONEROW:
      GetQuery = &QueryFactory::operation_updateonerow;
      break;
    case DELETEONEROW:
      GetQuery = &QueryFactory::operation_deleteonerow;
      break;
    case CLEARONEROW:
      GetQuery = &QueryFactory::operation_clearonerow;
      break;
    case ISROWEXISTS:
      GetQuery = &QueryFactory::operation_isrowexists;
      break;
    case GETMODIFIEDROWS:
      GetQuery = &QueryFactory::operation_getmodifiedrows;
      break;
    case GETONEROW:
      GetDBSpecificQuery = &QueryFactory::operation_getonerow;
      break;
    case COPYDATABASE:
      GetTwoDBQuery = &QueryFactory::operation_copydatabase;
      break;
    case CLEARDATABASE:
      GetSingleDBQuery = &QueryFactory::operation_cleardatabase;
      break;
    case ISCANDIDATEDIRTY:
      GetIsDBQuery = &QueryFactory::operation_iscandidatedirty;
      break;
    case GETBULKROWS:
      GetBulkRowQuery = &QueryFactory::operation_getbulkrows;
      break;
    case GETSIBLINGCOUNT:
      GetQuery = &QueryFactory::operation_getsiblingcount;
      break;
    case GETSIBLINGCOUNT_FILTER:
      GetFilterCountQuery =
          &QueryFactory::operation_getsiblingcount_with_filter;
      break;
    case GETSIBLINGROWS:
      GetSiblingFilterQuery = &QueryFactory::operation_getsiblingrows;
      break;
    case GETROWCOUNT:
      GetCountQuery = &QueryFactory::operation_getrowcount;
        break;
    case COMMITALLCONFIG:
      CommitTableQuery = &QueryFactory::operation_commit_all_config;
      break;
    case CLEARONEINSTANCE:
      GetClearInstanceQuery = &QueryFactory::operation_clearoneinstance;
      break;
    default:
      pfc_log_debug("ODBCM::QueryFactory::SetOperation: "
        "Invalid operation:%d", operation_type);
      break;
  }
}
}  // namespace uppl
}  // namespace unc
