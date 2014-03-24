/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include "include/odbcm_mgr.hh"

namespace unc {
namespace uppl {

ODBCManager *ODBCManager::ODBCManager_ = NULL;
std::map<ODBCManager::Method, ODBCM_RC_STATUS>
                                 ODBCManager::method_resultcode_map;
bool  ODBCManager::exists_= false;
uint32_t  ODBCManager::sibling_count = 0;

ODBCManager::ODBCManager(void) {
}

ODBCManager::~ODBCManager(void) {
}

ODBCM_RC_STATUS
ODBCManager::stub_getMappedResultCode(ODBCManager::Method methodType) {
  if (0 != method_resultcode_map.count(methodType)) {
    return method_resultcode_map[methodType];
  }
  return ODBCM_RC_GENERAL_ERROR;
}

ODBCM_RC_STATUS
ODBCManager::ODBCM_Initialize() {
  return ODBCM_RC_SUCCESS;
}

ODBCM_RC_STATUS
ODBCManager::CloseRwConnection() {
  return ODBCM_RC_SUCCESS;
}

ODBCManager *
ODBCManager::get_ODBCManager() {
  /*Allocate the memory for ODBCManager only if its NULL*/
  ODBCManager *obj(ODBCManager_);
  if (obj == NULL) {
    obj = new ODBCManager();
    if (NULL == obj) {
      pfc_log_fatal("ODBCM::ODBCManager::get_ODBCManager: "
          "Error in memory allocation for ODBCManager_ !!! ");
      return NULL;
    }
    ODBCManager_ = obj;
  }
  return obj;
}

ODBCM_RC_STATUS
ODBCManager::OpenDBConnection(OdbcmConnectionHandler *conn_obj) {
  return ODBCM_RC_SUCCESS;
}

ODBCM_RC_STATUS
ODBCManager::CloseDBConnection(OdbcmConnectionHandler *conn_obj) {
  return ODBCM_RC_SUCCESS;
}

ODBCM_RC_STATUS
ODBCManager:: AssignDBConnection(OdbcmConnectionHandler *&db_conn,
                                 uint32_t session_id, uint32_t config_id) {
  return ODBCM_RC_SUCCESS;
}

ODBCM_RC_STATUS
ODBCManager::PoolDBConnection(OdbcmConnectionHandler *&conn_obj,
                      uint32_t session_id, uint32_t config_id) {
   return ODBCM_RC_SUCCESS;
}

ODBCM_RC_STATUS
ODBCManager::FreeingConnections(bool IsAllOrUnused) {
  return ODBCM_RC_SUCCESS;
}

std::string
ODBCManager::GetTableName(ODBCMTable table_id) {
  switch (table_id) {
    case CTR_TABLE:
      return UPPL_CTR_TABLE;
    case CTR_DOMAIN_TABLE:
      return UPPL_CTR_DOMAIN_TABLE;
    case LOGICALPORT_TABLE:
      return UPPL_LOGICALPORT_TABLE;
    case LOGICAL_MEMBERPORT_TABLE:
      return UPPL_LOGICAL_MEMBER_PORT_TABLE;
    case SWITCH_TABLE:
      return UPPL_SWITCH_TABLE;
    case PORT_TABLE:
      return UPPL_PORT_TABLE;
    case LINK_TABLE:
      return UPPL_LINK_TABLE;
    case BOUNDARY_TABLE:
      return UPPL_BOUNDARY_TABLE;
    default:
      return "";
  }
}

std::string
ODBCManager::GetColumnName(ODBCMTableColumns col_id) {
  return "";
}


ODBCM_RC_STATUS
ODBCManager::IsRowExists(unc_keytype_datatype_t, DBTableSchema&,
                         OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::ISROWEXISTS);
}

ODBCM_RC_STATUS
ODBCManager::CreateOneRow(unc_keytype_datatype_t, DBTableSchema&,
                          OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::CREATEONEROW);
}

ODBCM_RC_STATUS
ODBCManager::UpdateOneRow(unc_keytype_datatype_t, DBTableSchema&,
                          OdbcmConnectionHandler *conn_obj, bool IsInternal) {
  return stub_getMappedResultCode(ODBCManager::UPDATEONEROW);
}

ODBCM_RC_STATUS
ODBCManager::DeleteOneRow(unc_keytype_datatype_t, DBTableSchema&,
                          OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::DELETEONEROW);
}

ODBCM_RC_STATUS
ODBCManager::GetOneRow(unc_keytype_datatype_t, DBTableSchema&,
                       OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::GETONEROW);
}

ODBCM_RC_STATUS
ODBCManager::GetBulkRows(unc_keytype_datatype_t, uint32_t, DBTableSchema&,
                         unc_keytype_operation_t,
                         OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::GETBULKROWS);
}

ODBCM_RC_STATUS
ODBCManager::GetRowCount(unc_keytype_datatype_t, std::string,
                         uint32_t&, OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::GETROWCOUNT);
}

ODBCM_RC_STATUS
ODBCManager::ClearOneRow(unc_keytype_datatype_t, DBTableSchema&,
                         OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::CLEARONEROW);
}

ODBCM_RC_STATUS
ODBCManager::GetSiblingCount(unc_keytype_datatype_t, DBTableSchema&,
                             uint32_t &count, OdbcmConnectionHandler *conn_obj) {
  count = sibling_count;
  return stub_getMappedResultCode(ODBCManager::GETSIBLINGCOUNT);
}

ODBCM_RC_STATUS
ODBCManager::GetSiblingCount(unc_keytype_datatype_t, DBTableSchema&,
                             uint32_t &count, std::vector<ODBCMOperator>,
                             OdbcmConnectionHandler *conn_obj) {
  count = sibling_count;
  return stub_getMappedResultCode(ODBCManager::GETSIBLINGCOUNT_FILTER);
}

ODBCM_RC_STATUS
ODBCManager::GetSiblingRows(unc_keytype_datatype_t, uint32_t,
                            DBTableSchema&, std::vector<ODBCMOperator>,
                            unc_keytype_operation_t,
                            OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::GETSIBLINGROWS);
}

ODBCM_RC_STATUS
ODBCManager::GetModifiedRows(unc_keytype_datatype_t, DBTableSchema&,
                             OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::GETMODIFIEDROWS);
}

ODBCM_RC_STATUS
ODBCManager::CopyDatabase(unc_keytype_datatype_t, unc_keytype_datatype_t,
                          OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::COPYDATABASE);
}

/**To clear given key instance rows in all the tables in DB*/
ODBCM_RC_STATUS
ODBCManager::ClearOneInstance(unc_keytype_datatype_t,
                              std::string controller_name,
                              OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::CLEARONEINSTANCE);
}

/**clear the entries in the database tables.*/
ODBCM_RC_STATUS
ODBCManager::ClearDatabase(unc_keytype_datatype_t,
                           OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::CLEARDATABASE);
}

ODBCM_RC_STATUS
ODBCManager::IsCandidateDirty(OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::ISCANDIDATEDIRTY);
}

ODBCM_RC_STATUS
ODBCManager::CommitAllConfiguration(unc_keytype_datatype_t,
                                    unc_keytype_datatype_t,
                                    OdbcmConnectionHandler *conn_obj) {
  return stub_getMappedResultCode(ODBCManager::COMMITALLCONFIG);
}
}  // namespace uppl
}  // namespace unc
