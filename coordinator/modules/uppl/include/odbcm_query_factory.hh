/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   ODBC Manager
 * @file    odbcm_query_factory.hh
 *
 */

#ifndef _ODBCM_QUERY_FACTORY_HH_
#define _ODBCM_QUERY_FACTORY_HH_

#include <unc/keytype.h>
#include <string>
#include <vector>
#include "odbcm_common.hh"
#include "odbcm_db_tableschema.hh"
#include "odbcm_mgr.hh"

namespace unc {
namespace uppl {
  /** 
    * This class implements the methods for framing
    * queries which are required for database 
    */
  class QueryFactory {
    public:
      QueryFactory();
      ~QueryFactory();
      /** This method is invoked to set the function
        * pointer with appropriate method */
      void SetOperation(int operation_type);
      /**** Function pointers to class member functions ****/
      /** Function pointer for below methods
       * operation_createonerow, 
       * operation_deleteonerow
       * operation_clearonerow
       * operation_isrowexists
       * operation_getmodifiedrows */
      std::string(QueryFactory::*GetQuery)
          (unc_keytype_datatype_t, DBTableSchema&);
      /* operation_updateonerow */
      std::string(QueryFactory::*GetQueryWithBool)
          (unc_keytype_datatype_t, DBTableSchema&, bool);
      /** Function pointer for operation_getsiblingCount with filters */
      std::string(QueryFactory::*GetFilterCountQuery)
          (unc_keytype_datatype_t, DBTableSchema&, std::vector<ODBCMOperator>);
      /** Function pointer for operation_getsiblingrows with filters */
      std::string(QueryFactory::*GetSiblingFilterQuery)
          (unc_keytype_datatype_t, DBTableSchema&, uint32_t,
              std::vector<ODBCMOperator>, unc_keytype_operation_t);
      /** Function pointer for operation_getrowcount */
      std::string(QueryFactory::*GetCountQuery)
          (unc_keytype_datatype_t, std::string);
      /** Function pointer for operation_getonerow */
      std::string(QueryFactory::*GetDBSpecificQuery)
          (unc_keytype_datatype_t, DBTableSchema&);
      /** Function pointer for operation_copydatabase */
      std::string* (QueryFactory::*GetTwoDBQuery)
          (unc_keytype_datatype_t, unc_keytype_datatype_t);
      /** Function pointer for operation_cleardatabase */
      std::string* (QueryFactory::*GetSingleDBQuery)
          (unc_keytype_datatype_t);
      /** Function pointer for operation_iscandidatedirty */
      std::string* (QueryFactory::*GetIsDBQuery)();
      /** Function pointer for operation_commit_all_config */
      std::string* (QueryFactory::*CommitTableQuery)
          (unc_keytype_datatype_t, unc_keytype_datatype_t, uint32_t);
      /** Function pointer for operation_getbulkrows */
      std::string(QueryFactory::*GetBulkRowQuery)
          (unc_keytype_datatype_t, uint32_t, DBTableSchema&,
          unc_keytype_operation_t);
      /** Function pointer for operation_clearoneinstance */
      std::string* (QueryFactory::*GetClearInstanceQuery)
          (unc_keytype_datatype_t, std::string&);

      std::string getOrderByString(ODBCMTable, std::vector <std::string>&);

    private:
      /** To frame query for creating a row in db */
      SQLQUERY operation_createonerow
          (unc_keytype_datatype_t, DBTableSchema &);
      /** To frame query for updating a row in db */
      SQLQUERY operation_updateonerow
          (unc_keytype_datatype_t, DBTableSchema &, bool);
      /** To frame query for deleting a row in db
        * (Change cs_row_status to DELETED) */
      SQLQUERY operation_deleteonerow
          (unc_keytype_datatype_t, DBTableSchema &);
      /** To frame query for deleting permanenty in db */
      SQLQUERY operation_clearonerow
          (unc_keytype_datatype_t, DBTableSchema &);
      /** To frame query to check whether a row exists */
      SQLQUERY operation_isrowexists
          (unc_keytype_datatype_t, DBTableSchema &);
      /** To frame query for retrieving a row */
      SQLQUERY operation_getonerow
          (unc_keytype_datatype_t, DBTableSchema &);
      /** To frame query for retrieving modified rows
        * w.r.t cs_row_status */
      SQLQUERY operation_getmodifiedrows
          (unc_keytype_datatype_t, DBTableSchema &);
      /** To frame query for clearing a database */
      SQLQUERY* operation_cleardatabase
          (unc_keytype_datatype_t);
      /** To frame query to copy data from one to other db */
      SQLQUERY* operation_copydatabase
          (unc_keytype_datatype_t, unc_keytype_datatype_t);
      /** To frame query for clearing a controller
        * instance in all tables in a db */
      SQLQUERY* operation_clearoneinstance
          (unc_keytype_datatype_t, std::string&);
      /** To frame query to check candidate db is dirty or
        * not. Candidate db is assumed to be dirty when
        * cs_row_status is DELETED in any table*/
      SQLQUERY* operation_iscandidatedirty();
      /** To frame query for retrieving bulk rows from db */
      SQLQUERY operation_getbulkrows
          (unc_keytype_datatype_t, uint32_t, DBTableSchema &,
           unc_keytype_operation_t);
      /** To frame query for getting the sibling count
        * This method will use > query to fetch the count */
      SQLQUERY operation_getsiblingcount
          (unc_keytype_datatype_t, DBTableSchema&);
      /** To frame query for getting the sibling count with filter options
       */
      SQLQUERY operation_getsiblingcount_with_filter(
          unc_keytype_datatype_t,
          DBTableSchema&,
          std::vector<ODBCMOperator>);
      /** To frame query for getting the sibling rows based upon filter options
       */
      SQLQUERY operation_getsiblingrows(
          unc_keytype_datatype_t,
          DBTableSchema&, uint32_t,
          std::vector<ODBCMOperator>, unc_keytype_operation_t);
      /** To frame query for retrieving the no of rows in a table */
      SQLQUERY operation_getrowcount
          (unc_keytype_datatype_t, std::string);
      /** This method will be invoked during commit operation
        * Step1: Change row_status to APPLIED for CREATED/UPDATED 
        *         and delete the row_status(DELETED) from db
        * Step2: Truncate all the tables in running 
        * Step3: Copy candidate data to running db */
      SQLQUERY* operation_commit_all_config
          (unc_keytype_datatype_t, unc_keytype_datatype_t, uint32_t);
  };
}  // namespace uppl
}  // namespace unc

#endif  // _ODBCM_QUERY_FACTORY_HH_
/* EOF */
