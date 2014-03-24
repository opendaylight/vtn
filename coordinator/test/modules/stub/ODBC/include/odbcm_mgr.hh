/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */



#ifndef _ODBCM_MGR_HH_
#define _ODBCM_MGR_HH_

#include <odbcm_common.hh>
#include <odbcm_db_tableschema.hh>
#include <vector>
#include <string>
#include <map>

namespace unc {
namespace uppl {

class QueryFactory;
class QueryProcessor;
class DBVarbind;
class OdbcmConnectionHandler;

/**ODBMCManager is a class, exposes the API methods to UPPL managers and
 * internal transaction coordinator (ITC)
 * - Singleton class
 * */
class ODBCManager {
 public:
  enum Method {
    CREATEONEROW = 0,
    UPDATEONEROW,
    DELETEONEROW,
    CLEARONEROW,
    GETONEROW,
    GETBULKROWS,
    GETSIBLINGCOUNT,
    GETSIBLINGCOUNT_FILTER,
    GETSIBLINGROWS,
    GETROWCOUNT,
    COPYDATABASE,
    CLEARDATABASE,
    ISCANDIDATEDIRTY,
    ISROWEXISTS,
    GETMODIFIEDROWS,
    COMMITALLCONFIG,
    CLEARONEINSTANCE,
    CLEARALLROWS
  };
  /**Destructor of ODBCManager class
   * InputParam: None*/
  ~ODBCManager();
  /**static method to return the odbcmanager singleton object instance
   * to the caller
   * InputParam: None
   * */
  static ODBCManager *get_ODBCManager();
  /**Initializer of all ODBC Manager resources and establish connection
   * with DBMS using the odbc driver, filling the required maps and parsing
   * configuration file and update into local structure.
   * */
  ODBCM_RC_STATUS ODBCM_Initialize();

  ODBCM_RC_STATUS OpenDBConnection(OdbcmConnectionHandler *conn_obj);
  /**close the connection at the end of application, free the allocated
   * handlers and environment */
  ODBCM_RC_STATUS CloseDBConnection(OdbcmConnectionHandler *conn_obj);

  ODBCM_RC_STATUS AssignDBConnection(OdbcmConnectionHandler *&db_conn,
                                     uint32_t session_id, uint32_t config_id = 0);
  ODBCM_RC_STATUS PoolDBConnection(OdbcmConnectionHandler *&conn_obj,
                                   uint32_t session_id, uint32_t config_id = 0);
  ODBCM_RC_STATUS FreeingConnections(bool IsAllOrUnused);

  /** getter method for db_table_list_map_ private member*/
  std::map<int, std::vector<std::string> >& get_db_table_list_map_();
  /** getter method for odbcm_tables_column_map_ private member*/
  std::map<ODBCMTableColumns, std::string>& get_odbcm_tables_column_map_();
  /**passing table name string and get the id of table */
  ODBCMTable get_table_id(std::string table_name);
  /**passing table name string and get the id of table */
  std::string GetTableName(ODBCMTable);
  /**
   * This method creates single key tree instance entry in the given DB table.
   * When row entry is created in UNC_DT_CANDIDATE database table,
   * cs_row_status column will be set as CREATED . During commit phase these
   * rows will be created in running configuration and the cs_row_status
   * will be set accordingly.
   */
  ODBCM_RC_STATUS CreateOneRow(unc_keytype_datatype_t/**Database type*/,
                               DBTableSchema&
                               /**object which carries the table
                                 ,pkeys,column names with data type*/,
                               OdbcmConnectionHandler *conn_obj);
  /**
   * This method updates one row of single keytree instance in the
   * database table. The cs_row_status column will be set as updated. During
   * commit phase the update will be moved to running configuration and the
   * row_status will be set accordingly.
   * */
  ODBCM_RC_STATUS UpdateOneRow(unc_keytype_datatype_t/**Database type*/,
                               DBTableSchema&
                               /**object which carries the table,pkeys,
                                * column names with data type*/,
                               OdbcmConnectionHandler *conn_obj,
                               bool IsInternal = false);
  /**
   * This method deletes the attributes of single key tree
   * instance in a row, the row entry will not be actually deleted in the case
   * of UNC_DT_CANDIDATE. only the row_status column will be changed to
   * DELETED. During commit phase the row will be cleared out.
   * In other databases, the entry will be deleted in this method and the
   * transaction will be committed.
   * */
  ODBCM_RC_STATUS DeleteOneRow(unc_keytype_datatype_t/**Database type*/,
                               DBTableSchema&
                               /**object which carries the table,pkeys,
                                * column names with data type*/,
                               OdbcmConnectionHandler *conn_obj);
  /**
   * This method fetches single row from the database table and
   * return the result set (fill the table rows in the DBTableSchema ref.)and
   * return the status to caller application
   * */
  ODBCM_RC_STATUS GetOneRow(unc_keytype_datatype_t/**Database type*/,
                            DBTableSchema&
                            /*object which carries the table,
                              pkeys,column names with data type*/,
                            OdbcmConnectionHandler *conn_obj);
  /**
   * This method will fetch the one or more number of rows in the db table
   * based upon the given max_repetition_count and condition or filter
   * criteria.
   * */
  ODBCM_RC_STATUS GetBulkRows(unc_keytype_datatype_t/**Database type*/,
                              uint32_t,
                              DBTableSchema&
                              /*object which carries the table,
                                pkeys,column names with data type*/,
                              unc_keytype_operation_t /**operation type*/,
                              OdbcmConnectionHandler *conn_obj);
  /**
   * To find the row value presence in the database table.
   * */
  ODBCM_RC_STATUS IsRowExists(unc_keytype_datatype_t/**Database type*/,
                              DBTableSchema&
                              /**object which carries the table,
                                pkeys,column names with data type*/,
                              OdbcmConnectionHandler *conn_obj);
  /**To get the row count of the given table*/
  ODBCM_RC_STATUS GetRowCount(unc_keytype_datatype_t/**Database type*/,
                              std::string/** name of the table*/,
                              uint32_t& /**return value count*/,
                              OdbcmConnectionHandler *conn_obj);
  /** this method clears single row in the database table,
   * based on the condition given in the DBTableSchema that row will be
   * identified and deleted permanently.*/
  ODBCM_RC_STATUS ClearOneRow(unc_keytype_datatype_t/**Database type*/,
                              DBTableSchema&
                              /**object which carries the table,
                                pkeys,column names with data type*/,
                              OdbcmConnectionHandler *conn_obj);
  /**To get the sibling rows count*/
  ODBCM_RC_STATUS GetSiblingCount(unc_keytype_datatype_t/**Database type*/,
                                  DBTableSchema&
                                  /**object which carries the
                                    table,pkeys,column names with data type*/,
                                  uint32_t& /**return value count*/,
                                  OdbcmConnectionHandler *conn_obj);
  /**To get the sibling rows count based upon given filter*/
  ODBCM_RC_STATUS GetSiblingCount(unc_keytype_datatype_t/**Database type*/,
                                  DBTableSchema&
                                  /**object which carries the
                                    table,pkeys,column names with data type*/,
                                  uint32_t& /**return value count*/,
                                  std::vector<ODBCMOperator>
                                  /**operator to decide
                                    the filter while framing query (where
                                    clause) */,
                                  OdbcmConnectionHandler *conn_obj);
  /**To return the sibiling rows in the given table and based upon the given
   * filter criteria*/
  ODBCM_RC_STATUS GetSiblingRows(unc_keytype_datatype_t/**database type */,
                                 uint32_t/**maximum repetition count*/,
                                 DBTableSchema&
                                 /**object which carries the
                                   table,pkeys,column names with data type*/,
                                 std::vector<ODBCMOperator>
                                 /*arithmetic operators to frame read sibling query*/,
                                 unc_keytype_operation_t
                                 /**operation type siblingbegin/sibling*/,
                                 OdbcmConnectionHandler *conn_obj);
  /**This method will copy one databases all contents to another database */
  ODBCM_RC_STATUS CopyDatabase(unc_keytype_datatype_t /**Database type*/,
                               unc_keytype_datatype_t /**Database type*/,
                               OdbcmConnectionHandler *conn_obj);
  /**To clear given key instance rows in all the tables in DB*/
  ODBCM_RC_STATUS ClearOneInstance(unc_keytype_datatype_t /**Database type*/,
                                   std::string controller_name /**keyvalue*/,
                                   OdbcmConnectionHandler *conn_obj);
  /**clear the entries in the database tables.*/
  ODBCM_RC_STATUS ClearDatabase(unc_keytype_datatype_t /**Database type*/,
                                OdbcmConnectionHandler *conn_obj);
  ODBCM_RC_STATUS IsCandidateDirty(OdbcmConnectionHandler *conn_obj);
  /**To get the modified rows, it will return CREATE/UPDATED/DELETED
   * rows_status rows*/
  ODBCM_RC_STATUS GetModifiedRows(unc_keytype_datatype_t /**Database type*/,
                                  DBTableSchema&
                                  /**object which carries the
                                    table,pkeys,column names with data type*/,
                                  OdbcmConnectionHandler *conn_obj);
  /** To commit all the configuration from candidate to
   * running database.
   * */
  ODBCM_RC_STATUS CommitAllConfiguration(unc_keytype_datatype_t
                                         /**Database type*/,
                                         unc_keytype_datatype_t
                                         /**Database type*/,
                                         OdbcmConnectionHandler *conn_obj);
  /**Freeing allocated memory for class object, table structure,
   * statement handler*/
  void ODBCMFreeingMemory(HSTMT, ODBCMTable, DBVarbind*,
                          QueryFactory*,
                          QueryProcessor*);
  std::string GetColumnName(ODBCMTableColumns);
  ODBCM_RC_STATUS CloseRwConnection();

  static void stub_setResultcode(ODBCManager::Method methodType,
                                          ODBCM_RC_STATUS res_code) {
    method_resultcode_map.insert(std::make_pair(methodType, res_code));
  }

  static void stub_setSingleRecordExists(bool exists) {
    exists_ = exists;
  }

  static void stub_setSiblingCount(uint32_t count) {
    sibling_count = count;
  }

  static void clearStubData() {
    method_resultcode_map.clear();
    exists_ = false;
    sibling_count = 0;
  }

 private:
  /**constructor of ODBCManager class
   * InputParam: None*/
  ODBCManager();
  /* ODBCManager instance */
  static ODBCManager *ODBCManager_;
  uint8_t IsODBCManager_initialized;
  std::map <int, std::vector<std::string> > db_table_list_map_;
  SQLHENV phy_conn_env_;    // Handle ODBC environment
  /**Collection of all uppl tables column names with id*/
  std::map <ODBCMTableColumns, std::string> odbcm_tables_column_map_;
  /**return the phy_conn_env_*/
  inline SQLHENV get_phy_connection_env_() const;

  /**initialize the ODBCMTableColumnsMap, fill all tables columns
   * from the static array*/
  ODBCM_RC_STATUS initialize_odbcm_tables_column_map_(void);
  /**initialize the map with database and corresponding tables on
   * each database. */
  ODBCM_RC_STATUS initialize_db_table_list_map_(void);
  /**allocate connection handler for rw_conn_handle_*/
  inline ODBCM_RC_STATUS set_rw_connection_handle_(SQLHDBC&);
  /**allocate connection handler for ro_conn_handle_*/
  inline ODBCM_RC_STATUS set_ro_connection_handle_(SQLHDBC&);
  /**allocate environment handler for phy_conn_env_*/
  inline ODBCM_RC_STATUS set_phy_connection_env_();
  /**parse the odbcm.conf file and update db_conf_info structure*/
  ODBCM_RC_STATUS ParseConfigFile();
  /**during the physical daemon initialize, allocate the connection
   * environment which will be common for all the connection and also
   * this method will enable the connection pooling */
  ODBCM_RC_STATUS InitializeConnectionEnv();
  // Connection handles to store nb and sb rw connection
  OdbcmConnectionHandler *rw_nb_conn_obj_;
  OdbcmConnectionHandler *rw_sb_conn_obj_;

  ODBCM_RC_STATUS stub_getMappedResultCode(Method);
  static std::map<ODBCManager::Method, ODBCM_RC_STATUS> method_resultcode_map;
  static  bool exists_;
  static uint32_t sibling_count;
};
}  // namespace uppl
}  // namespace unc
#endif /* _ODBCM_MGR_HH_ */
/**EOF*/
