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
 * @file    odbcm_db_tableschema.hh
 *
 */
#ifndef _ODBCM_DB_TABLESCHEMA_HH_
#define _ODBCM_DB_TABLESCHEMA_HH_

#include <unc/uppl_common.h>
#include <string>
#include <list>
#include <vector>
#include "odbcm_common.hh"

namespace unc {
namespace uppl {

class DBTableSchema {
  public:
    /* 
     * Constructor for DBTableSchema 
     */
    DBTableSchema();
    /* 
     * Destination for DBTableSchema 
     */
    ~DBTableSchema();
    /*
     * This corresponds to table name in db
     */
    ODBCMTable table_name_;
    /*
     * This contains primary keys of the table
     */
    std::vector <std::string> primary_keys_;
    /*
     * This contains the list of database row information
     */
    std::list <std::vector<TableAttrSchema> > row_list_;
    /*
     * Return status from database
     */
    CsRowStatus db_return_status_;

    std::string frame_explicit_order_;

    /*
     * Get the table name
     */
    ODBCMTable get_table_name();
    /*
     * Set the table name 
     */
    void set_table_name(ODBCMTable table_name);
    /*
     * Get the vector containing primary keys
     */
    std::vector <std::string>  get_primary_keys();
    /*
     * Get the list containing all the attribute vectors 
     */
    std::list < std::vector <TableAttrSchema> >& get_row_list();
    /*
     * To push the primary keys in to primary_key vector
     */
    void PushBackToPrimaryKeysVector(std::string attribute_name);
    /*
     * To push the attribute vector to the row_list_
     */
    void PushBackToRowList(std::vector <TableAttrSchema> attributes_vector);
    /*
     * To set the primary keys
     */
    void set_primary_keys(std::vector <std::string>);
    /*
     * To set the row list 
     */
    void set_row_list(std::list <std::vector<TableAttrSchema> >);
    /*
     * To free the memory in DBTableSchema
     */
    void FreeDBTableSchema();
    /* to clear a particular row's allocated memory in row_list_
     *
     * */
    void DeleteRowListFrontElement();
    /*
     * Method to print the database schema information
     */
    void PrintDBTableSchema();

  private:
    /*
    * To print the char buffer values in DBTableSchema
    */ 
    inline std::string Odbcm_PrintCharBuffer(uint8_t*, int, ODBCMTableColumns);
};  // class DBTableSchema

}  // namespace uppl
}  // namespace unc

#endif  // _ODBCM_DB_TABLESCHEMA_HH_
