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
 * @file    ODBCManager.hh
 *
 */

#ifndef _ODBCM_MGR_HH_
#define _ODBCM_MGR_HH_
#include <unc/keytype.h>
#include <vector>
#include <map>
#include <string>
#include "odbcm_common.hh"
#include "odbcm_db_tableschema.hh"


namespace unc {
namespace uppl {
/**macro for ending the current transaction with rollback or commit state*/
#define ODBCM_END_TRANSACTION(__conn__, __trans__) \
  if (NULL != __conn__) {\
    SQLRETURN transaction_rc = SQLEndTran(SQL_HANDLE_DBC, __conn__, __trans__);\
    ODBCM_DBC_HANDLE_CHECK(__conn__, transaction_rc); \
    if (transaction_rc != ODBCM_RC_SUCCESS &&\
        transaction_rc != ODBCM_RC_SUCCESS_WITH_INFO) \
      status = ODBCM_RC_TRANSACTION_ERROR; \
  }

/**macro to allocate memory for SQL statement handler*/
#define ODBCM_STATEMENT_CREATE(__connhandle__, __stmt__) \
  if (NULL != __connhandle__) { \
    odbc_rc = SQLAllocHandle(SQL_HANDLE_STMT, __connhandle__, &__stmt__); \
    ODBCM_STMT_HANDLE_CHECK(__stmt__, __connhandle__, odbc_rc); \
}

/**macro to free the statement handle*/
#define ODBCM_FREE_STMT(__stmt__) \
  if (NULL != __stmt__) \
    odbc_rc = SQLFreeHandle(SQL_HANDLE_STMT, __stmt__); \
  if (odbc_rc != ODBCM_RC_SUCCESS) \
    status = ODBCM_RC_MEMORY_ERROR;

/**macro to free the memory allocated object*/
#define ODBCM_FREE_MEMORY(__object__) \
  if (NULL != (__object__)) \
    delete __object__;

/**macro to allocate memory for string object*/
#define ODBCM_STRING_MEM_ALLOCATE(__str__) \
  __str__ = new std::string();

/**macro to create object instance from class*/
#define ODBCM_CREATE_OBJECT(__obj__, __class__) \
  __obj__ = new __class__();                    \
  if (__obj__ == NULL) \
    pfc_log_fatal("ODBCM::ODBCManager:: " \
              "Internal object memory allocation is failed");

/**macro to set odbc connection attributes
 * 1. transaction flag setting - READ_COMMITTED
 * 2. transaction flag setting - AUTOCOMMIT OFF
 * */
#define ODBCM_SET_CONNECTION_ATTRIBUTE(__connhandle__, __odbcrc__) \
  __odbcrc__ = SQLSetConnectAttr(__connhandle__, SQL_ATTR_TXN_ISOLATION, \
      (SQLPOINTER)SQL_TXN_READ_COMMITTED, 0);  \
  ODBCM_DBC_HANDLE_CHECK(__connhandle__, __odbcrc__); \
  if (__odbcrc__ != ODBCM_RC_SUCCESS && \
  __odbcrc__ != ODBCM_RC_SUCCESS_WITH_INFO) { \
    pfc_log_info("ODBCM::ODBCManager:: " \
                 "Error in SQLSetConnectAttr"); \
    return ODBCM_RC_CONNECTION_ERROR; \
  } \
  __odbcrc__ = SQLSetConnectAttr(__connhandle__, SQL_ATTR_AUTOCOMMIT, \
      (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_NTS); \
  ODBCM_DBC_HANDLE_CHECK(__connhandle__, __odbcrc__); \
  if (__odbcrc__ != ODBCM_RC_SUCCESS && \
     __odbcrc__ != ODBCM_RC_SUCCESS_WITH_INFO) { \
    pfc_log_info("ODBCM::ODBCManager:: " \
               "Error in SQLSetConnectAttr"); \
               return ODBCM_RC_CONNECTION_ERROR; \
  } \

/** common variable declaration to make connection with DBMS
 *  - structure to store configuration file parsed values
 */
typedef struct {
    std::string dsn;
    uint32_t time_out;
}db_conf_info;

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
    // Closes the Read Write connections
    ODBCM_RC_STATUS CloseRwConnection();

  private:
    /**constructor of ODBCManager class
     * InputParam: None*/
    ODBCManager();
    /* ODBCManager instance */
    static ODBCManager *ODBCManager_;
    db_conf_info conf_parser_;  // odbcm.conf file information
    uint8_t IsODBCManager_initialized;
    // hold all DB table names
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
};
}  // namespace uppl
}  // namespace unc
#endif /* _ODBCM_MGR_HH_ */
/**EOF*/
