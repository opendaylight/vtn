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
 * @file    odbcm_query_processor.hh
 *
 */

#ifndef _ODBCM_QUERY_PROCESSOR_H_
#define _ODBCM_QUERY_PROCESSOR_H_

#include <unc/uppl_common.h>
#include <sstream>
#include <string>
#include "physical_common_def.hh"
#include "odbcm_db_tableschema.hh"
#include "odbcm_db_varbind.hh"
#include "odbcm_common.hh"
#include "odbcm_mgr.hh"

namespace unc {
namespace uppl {

class QueryProcessor {
  public:
    /*
     * Constructor for QueryProcessor
     */
    QueryProcessor();
    /*
     * Destructor for QueryProcessor
     */
    ~QueryProcessor();
    /*
     * This prepares the database query statement before
     * execution
     */
    ODBCM_RC_STATUS PrepareQueryStatement(std::string , HSTMT &);
    /*
     * This executes the database query for write operation
     * in database
     */
    ODBCM_RC_STATUS ExecuteEditDBQuery(UpplDbOperationType, const HSTMT&);
    /*
     * This executes the database query for read operation
     * in database
     */
    ODBCM_RC_STATUS ExecuteReadDBQuery(UpplDbOperationType, HSTMT&);
    /*
     * This executes the query statement
     */
    ODBCM_RC_STATUS ExecuteQueryDirect
      (UpplDbOperationType , std::string , const HSTMT &);
    /*
     * This is executing a transaction in database
     */
    ODBCM_RC_STATUS ExecuteTransaction
      (UpplDbOperationType, std::string *, const HSTMT&);
    /*
     * This is for any group operation in the database
     */
    ODBCM_RC_STATUS ExecuteGroupOperationQuery
      (UpplDbOperationType, const HSTMT&);
};
}  // namespace uppl
}  // namespace unc
#endif /* _ODBCM_QUERY_PROCESSOR_H_ */
/* EOF */

