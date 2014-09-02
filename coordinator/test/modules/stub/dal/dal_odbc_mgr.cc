/*      Copyright (c) 2012 NEC Corporation                        */
/*      NEC CONFIDENTIAL AND PROPRIETARY                          */
/*      All rights reserved by NEC Corporation.                   */
/*      This program must be used solely for the purpose for      */
/*      which it was furnished by NEC Corporation.   No part      */
/*      of this program may be reproduced  or  disclosed  to      */
/*      others,  in  any form,  without  the  prior  written      */
/*      permission of NEC Corporation.    Use  of  copyright      */
/*      notice does not evidence publication of the program.      */

#include "dal_odbc_mgr.hh"

namespace unc {
namespace upll {
namespace dal {

std::map<DalOdbcMgr::Method,DalResultCode> DalOdbcMgr::method_resultcode_map;
bool  DalOdbcMgr::exists_=false;

DalOdbcMgr::DalOdbcMgr(void) {

}
DalOdbcMgr::~DalOdbcMgr(void) {
}

DalResultCode DalOdbcMgr::Init(void) {
	return stub_getMappedResultCode(DalOdbcMgr::INIT);
}
DalResultCode DalOdbcMgr::ConnectToDb(const DalConnType conn_type) {
	return stub_getMappedResultCode(DalOdbcMgr::CONNECT);
}

DalResultCode DalOdbcMgr::DisconnectFromDb() {
	return stub_getMappedResultCode(DalOdbcMgr::DISCONNECT);
}

DalResultCode DalOdbcMgr::CommitTransaction(){
	return stub_getMappedResultCode(DalOdbcMgr::COMMIT);

}

DalResultCode DalOdbcMgr::RollbackTransaction() {
	return stub_getMappedResultCode(DalOdbcMgr::ROLLBACK);
}
DalConnType DalOdbcMgr::get_conn_type() {
	return conn_type_;
}
DalResultCode DalOdbcMgr::GetSingleRecord(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *ouput_and_matching_attr_info) {
	return stub_getMappedResultCode(DalOdbcMgr::SINGLE);

}

DalResultCode DalOdbcMgr::GetMultipleRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor) {
        *cursor=new DalCursor();
	return stub_getMappedResultCode(DalOdbcMgr::MULTIPLE);

}

DalResultCode DalOdbcMgr::GetNextRecord(const DalCursor *cursor) {
	return stub_getMappedResultCode(DalOdbcMgr::NEXT);
}

DalResultCode DalOdbcMgr::CloseCursor(DalCursor *cursor, bool) {
	return stub_getMappedResultCode(DalOdbcMgr::CLOSE_CURSOR);
}

DalResultCode DalOdbcMgr::RecordExists(const UpllCfgType cfg_type,
                               const DalTableIndex table_index,
                               const DalBindInfo *matching_attr_info,
                               bool *existence) {
		 *existence=exists_;
	return stub_getMappedResultCode(DalOdbcMgr::RECORD_EXISTS);
}

DalResultCode DalOdbcMgr::GetSiblingBegin(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor)  {
	return stub_getMappedResultCode(DalOdbcMgr::SIBLING_BEGIN);
}

DalResultCode  DalOdbcMgr::GetSiblingRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor)  {
	return stub_getMappedResultCode(DalOdbcMgr::SIBLING_COUNT);
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
	return stub_getMappedResultCode(DalOdbcMgr::RECORD_COUNT);
}

DalResultCode DalOdbcMgr::DeleteRecords(const UpllCfgType cfg_type,
                                const DalTableIndex table_index,
                                const DalBindInfo *matching_attr_info,
                                const bool truncate) {
	return stub_getMappedResultCode(DalOdbcMgr::DELETE_RECORD);
}

DalResultCode DalOdbcMgr::CreateRecord(const UpllCfgType cfg_type,
                               const DalTableIndex table_index,
                               const DalBindInfo *input_attr_info) {
	return stub_getMappedResultCode(DalOdbcMgr::CREATE_RECORD);
}

DalResultCode DalOdbcMgr::UpdateRecords(
                    string query_statement,
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *input_and_matching_attr_info) {
	return stub_getMappedResultCode(DalOdbcMgr::UPDATE_RECORD);
}

DalResultCode DalOdbcMgr::UpdateRecords(
                        const UpllCfgType cfg_type,
                        const DalTableIndex table_index,
                        const DalBindInfo *input_and_matching_attr_info) {
	return stub_getMappedResultCode(DalOdbcMgr::UPDATE_RECORD);
}


DalResultCode DalOdbcMgr::GetDeletedRecords(const UpllCfgType cfg_type_1,
                                    const UpllCfgType cfg_type_2,
                                    const DalTableIndex table_index,
                                    const size_t max_record_count,
                                    const DalBindInfo *output_attr_info,
                                    DalCursor **cursor) {

	return stub_getMappedResultCode(DalOdbcMgr::GET_DELETED_RECORDS);
}

DalResultCode DalOdbcMgr::GetCreatedRecords(const UpllCfgType cfg_type_1,
                                    const UpllCfgType cfg_type_2,
                                    const DalTableIndex table_index,
                                    const size_t max_record_count,
                                    const DalBindInfo *output_attr_info,
                                    DalCursor **cursor)  {
	return stub_getMappedResultCode(DalOdbcMgr::GET_CREATED_RECORDS);
}

DalResultCode DalOdbcMgr::GetUpdatedRecords(
                    const UpllCfgType cfg_type_1,
                    const UpllCfgType cfg_type_2,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *cfg_1_output_and_match_attr_info,
                    const DalBindInfo *cfg_2_output_and_match_attr_info,
                    DalCursor **cursor)  {
	return stub_getMappedResultCode(DalOdbcMgr::GET_UPDATED_RECORDS);
}

DalResultCode DalOdbcMgr::CopyEntireRecords(const UpllCfgType dest_cfg_type,
                                    const UpllCfgType src_cfg_type,
                                    const DalTableIndex table_index,
                                    const DalBindInfo *output_attr_info) {
	return stub_getMappedResultCode(DalOdbcMgr::COPY_ENTIRE);
}

DalResultCode DalOdbcMgr::CopyModifiedRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info,
                    const unc_keytype_operation_t op)  {
	return stub_getMappedResultCode(DalOdbcMgr::COPY_MODIFY);
}
DalResultCode DalOdbcMgr::CopyModifiedInsertRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info) {

	return stub_getMappedResultCode(DalOdbcMgr::COPY_MODIFY_INSERT);
}

DalResultCode DalOdbcMgr::CopyMatchingRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info) {
	return stub_getMappedResultCode(DalOdbcMgr::COPY_MATCHING);
}

DalResultCode DalOdbcMgr::ExecuteAppQuery(std::string query_stmt,
                            const UpllCfgType cfg_type,
                            const DalTableIndex table_index,
                            const DalBindInfo *bind_info,
                            const unc_keytype_operation_t dirty_op) {
  return stub_getMappedResultCode(DalOdbcMgr::EXECUTE_QUERY);
}

DalResultCode DalOdbcMgr::ExecuteAppQueryMultipleRecords(
                         const std::string query_stmt,
                         const size_t max_record_count,
                         const DalBindInfo *bind_info,
                         DalCursor **cursor) {
	return stub_getMappedResultCode(DalOdbcMgr::EXECUTE_APPQUERY_MULTIPLE);
}
DalResultCode
DalOdbcMgr::ExecuteAppQueryModifyRecord(
    const UpllCfgType cfg_type,
    const DalTableIndex table_index,
    const std::string query_stmt,
    const DalBindInfo *bind_info,
    const unc_keytype_operation_t op) {
	return stub_getMappedResultCode(DalOdbcMgr::EXECUTE_APPQUERY_MODIFY);
}
DalResultCode
DalOdbcMgr::ExecuteAppQuerySingleRecord(
    const std::string query_stmt,
    const DalBindInfo *bind_info)  {
	return stub_getMappedResultCode(DalOdbcMgr::EXECUTE_APPQUERY_SINGLE);
}

DalResultCode
DalOdbcMgr::UpdateDirtyTblCacheFromDB() const{
  return kDalRcSuccess;
}

DalResultCode DalOdbcMgr::CheckRecordsIdentical(const UpllCfgType cfg_type_1,
                                        const UpllCfgType cfg_type_2,
                                        const DalTableIndex table_index,
                                        const DalBindInfo *matching_attr_info,
                                        bool *identical) {
	return stub_getMappedResultCode(DalOdbcMgr::CHECK_IDENTICAL);

}

DalResultCode DalOdbcMgr::ExecuteQuery(SQLHANDLE *dal_stmt_handle,
                         const std::string *query_stmt,
                         const DalBindInfo *bind_info,
                         const uint32_t max_count) {
        return stub_getMappedResultCode(DalOdbcMgr::EXECUTE_QUERY);
}
DalResultCode DalOdbcMgr::stub_getMappedResultCode(DalOdbcMgr::Method methodType) {

	 if (0 != method_resultcode_map.count(methodType))
		 {
			 return method_resultcode_map[methodType];
		 }
		 return kDalRcGeneralError;
}

DalResultCode
DalOdbcMgr::ClearCreateUpdateFlags(const DalTableIndex table_index,
                                   const UpllCfgType cfg_type) const {

  return kDalRcSuccess;
}

DalResultCode
DalOdbcMgr::ClearAllDirtyTblInDB(UpllCfgType cfg_type) const {
  return kDalRcSuccess;
}

}  // namespace dal
}  // namespace upll
}  // namespace unc

