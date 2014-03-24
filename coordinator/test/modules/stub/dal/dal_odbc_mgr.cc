/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "dal_odbc_mgr.hh"

namespace unc {
namespace upll {
namespace dal {

std::map<DalOdbcMgr::Method, DalResultCode> DalOdbcMgr::method_resultcode_map;
bool  DalOdbcMgr::exists_= false;
uint32_t DalOdbcMgr::sibling_count_= 3;
uint32_t DalOdbcMgr::count = 0;
std::map<uint8_t, DalResultCode> DalOdbcMgr::resultcodes;

DalOdbcMgr::DalOdbcMgr(void) {
}

DalOdbcMgr::~DalOdbcMgr(void) {
}

DalOdbcMgr* DalOdbcMgr::GetAlarmRwConn() {
  return new DalOdbcMgr();
}

upll_rc_t DalOdbcMgr::DalTxClose(DalOdbcMgr *dom, bool commit) {
  return UPLL_RC_SUCCESS;
}

void ReleaseRwConn(DalOdbcMgr *dom) {
}

DalResultCode DalOdbcMgr::Init(void) {
  return stub_getMappedResultCode(DalOdbcMgr::INIT);
}

DalResultCode DalOdbcMgr::ConnectToDb(const DalConnType conn_type) const {
  return stub_getMappedResultCode(DalOdbcMgr::CONNECT);
}

DalResultCode DalOdbcMgr::DisconnectFromDb() const {
  return stub_getMappedResultCode(DalOdbcMgr::DISCONNECT);
}

DalResultCode DalOdbcMgr::CommitTransaction() const {
  return stub_getMappedResultCode(DalOdbcMgr::COMMIT);
}

DalResultCode DalOdbcMgr::RollbackTransaction() const {
  return stub_getMappedResultCode(DalOdbcMgr::ROLLBACK);
}

DalConnType DalOdbcMgr::get_conn_type() {
  return conn_type_;
}

DalResultCode DalOdbcMgr::GetSingleRecord(
  const UpllCfgType cfg_type, const DalTableIndex table_index,
  const DalBindInfo *output_and_matching_attr_info) {
  return stub_getMappedResultCode(DalOdbcMgr::SINGLE);
}

DalResultCode DalOdbcMgr::GetMultipleRecords(
  const UpllCfgType cfg_type, const DalTableIndex table_index,
  const size_t max_record_count,
  const DalBindInfo *output_and_matching_attr_info, DalCursor **cursor) {
  return initCursor(cursor, output_and_matching_attr_info, MULTIPLE);
}

DalResultCode DalOdbcMgr::GetNextRecord(const DalCursor *cursor) {
  return resultcodes[count++];
}

DalResultCode DalOdbcMgr::CloseCursor(DalCursor *cursor, bool delete_bind) {
  if (cursor != NULL) {
    cursor->CloseCursor(delete_bind);
    delete cursor;
  }
  return stub_getMappedResultCode(DalOdbcMgr::CLOSE_CURSOR);
}

DalResultCode DalOdbcMgr::RecordExists(const UpllCfgType cfg_type,
                                       const DalTableIndex table_index,
                                       const DalBindInfo *matching_attr_info,
                                       bool *existence) {
  *existence = exists_;
  return stub_getMappedResultCode(DalOdbcMgr::RECORD_EXISTS);
}

DalResultCode DalOdbcMgr::GetSiblingBegin(
  const UpllCfgType cfg_type, const DalTableIndex table_index,
  const size_t max_record_count,
  const DalBindInfo *output_and_matching_attr_info, DalCursor **cursor)  {
  return initCursor(cursor, output_and_matching_attr_info, SIBLING_BEGIN);
}

DalResultCode  DalOdbcMgr::GetSiblingRecords(
  const UpllCfgType cfg_type, const DalTableIndex table_index,
  const size_t max_record_count,
  const DalBindInfo *output_and_matching_attr_info, DalCursor **cursor)  {
  return initCursor(cursor, output_and_matching_attr_info, SIBLING_COUNT);
}

DalResultCode DalOdbcMgr::GetSiblingCount(const UpllCfgType cfg_type,
                                          const DalTableIndex table_index,
                                          const DalBindInfo *matching_attr_info,
                                          uint32_t *count) {
  return stub_getMappedResultCode(DalOdbcMgr::SIBLING_COUNT);
}

DalResultCode DalOdbcMgr::GetRecordCount(const UpllCfgType cfg_type,
                                         const DalTableIndex table_index,
                                         const DalBindInfo *matching_attr_info,
                                         uint32_t *count)  {
  *count = sibling_count_;
  return stub_getMappedResultCode(DalOdbcMgr::RECORD_COUNT);
}

DalResultCode DalOdbcMgr::DeleteRecords(const UpllCfgType cfg_type,
                                        const DalTableIndex table_index,
                                        const DalBindInfo *matching_attr_info) {
  return stub_getMappedResultCode(DalOdbcMgr::DELETE_RECORD);
}

DalResultCode DalOdbcMgr::CreateRecord(const UpllCfgType cfg_type,
                                       const DalTableIndex table_index,
                                       const DalBindInfo *input_attr_info) {
  return stub_getMappedResultCode(DalOdbcMgr::UPDATE_RECORD);
}

DalResultCode DalOdbcMgr::UpdateRecords(
  const UpllCfgType cfg_type, const DalTableIndex table_index,
  const DalBindInfo *input_and_matching_attr_info) {
  return stub_getMappedResultCode(DalOdbcMgr::CREATE_RECORD);
}

DalResultCode DalOdbcMgr::GetDeletedRecords(const UpllCfgType cfg_type_1,
                                            const UpllCfgType cfg_type_2,
                                            const DalTableIndex table_index,
                                            const size_t max_record_count,
                                            const DalBindInfo *output_attr_info,
                                            DalCursor **cursor) {
  return initCursor(cursor, output_attr_info, GET_DELETED_RECORDS);
}

DalResultCode DalOdbcMgr::GetCreatedRecords(const UpllCfgType cfg_type_1,
                                            const UpllCfgType cfg_type_2,
                                            const DalTableIndex table_index,
                                            const size_t max_record_count,
                                            const DalBindInfo *output_attr_info,
                                            DalCursor **cursor)  {
  return initCursor(cursor, output_attr_info, GET_CREATED_RECORDS);
}

DalResultCode DalOdbcMgr::GetUpdatedRecords(
  const UpllCfgType cfg_type_1, const UpllCfgType cfg_type_2,
  const DalTableIndex table_index, const size_t max_record_count,
  const DalBindInfo *cfg_1_output_and_match_attr_info,
  const DalBindInfo *cfg_2_output_and_match_attr_info, DalCursor **cursor)  {
  return initCursor(cursor, cfg_1_output_and_match_attr_info,
                    cfg_2_output_and_match_attr_info, GET_UPDATED_RECORDS);
}

DalResultCode DalOdbcMgr::CopyEntireRecords(
  const UpllCfgType dest_cfg_type, const UpllCfgType src_cfg_type,
  const DalTableIndex table_index, const DalBindInfo *output_attr_info) {
  return stub_getMappedResultCode(DalOdbcMgr::COPY_ENTIRE);
}

DalResultCode DalOdbcMgr::CopyModifiedRecords(
                                    const UpllCfgType dest_cfg_type,
                                    const UpllCfgType src_cfg_type,
                                    const DalTableIndex table_index,
                                    const DalBindInfo *bind_info,
                                    const unc_keytype_operation_t op) const {
  return stub_getMappedResultCode(DalOdbcMgr::COPY_MODIFY);
}

DalResultCode DalOdbcMgr::CopyModifiedInsertRecords(
  const UpllCfgType dest_cfg_type, const UpllCfgType src_cfg_type,
  const DalTableIndex table_index,
  const DalBindInfo *output_and_match_attr_info) {
  return stub_getMappedResultCode(DalOdbcMgr::COPY_MODIFY_INSERT);
}

DalResultCode DalOdbcMgr::CopyMatchingRecords(
  const UpllCfgType dest_cfg_type, const UpllCfgType src_cfg_type,
  const DalTableIndex table_index,
  const DalBindInfo *output_and_match_attr_info) {
  return stub_getMappedResultCode(DalOdbcMgr::COPY_MATCHING);
}

DalResultCode DalOdbcMgr::ExecuteAppQueryMultipleRecords(
  const std::string query_stmt, const size_t max_record_count,
  const DalBindInfo *bind_info, DalCursor **cursor) {
  return initCursor(cursor, bind_info, EXECUTE_APPQUERY_MULTIPLE);
}

DalResultCode DalOdbcMgr::CheckRecordsIdentical(
  const UpllCfgType cfg_type_1, const UpllCfgType cfg_type_2,
  const DalTableIndex table_index, const DalBindInfo *matching_attr_info,
  bool *identical) {
  return stub_getMappedResultCode(DalOdbcMgr::CHECK_IDENTICAL);
}

DalResultCode DalOdbcMgr::ExecuteQuery(SQLHANDLE *dal_stmt_handle,
                                       const std::string *query_stmt,
                                       const DalBindInfo *bind_info,
                                       const uint32_t max_count) {
  return stub_getMappedResultCode(DalOdbcMgr::EXECUTE_QUERY);
}

DalResultCode DalOdbcMgr::stub_getMappedResultCode(
  DalOdbcMgr::Method methodType) const {
  if (0 != method_resultcode_map.count(methodType)) {
    return method_resultcode_map[methodType];
  }
  return kDalRcGeneralError;
}

}  // namespace dal
}  // namespace upll
}  // namespace unc

