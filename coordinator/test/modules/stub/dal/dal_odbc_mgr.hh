/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * dal_odbc_mgr.hh
 *   contians DalOdbcMgr class definition
 */

#ifndef __DAL_ODBC_MGR_HH__
#define __DAL_ODBC_MGR_HH__

#include <map>
#include <set>
#include <string>

#include "include/dal_defines.hh"
#include "include/dal_conn_intf.hh"
#include "include/dal_schema.hh"
#include "dal_bind_info.hh"
#include "dal_cursor.hh"
#include "dal_dml_intf.hh"
#include "uncxx/tclib/tclib_defs.hh"
#include "uncxx/upll_log.hh"
#include "cxx/pfcxx/synch.hh"

using namespace std;

namespace unc {
namespace upll {
namespace dal {

// class DalBindInfo;
// class DalCursor;
// typedef upll_keytype_datatype_t UpllCfgType;
class DalOdbcMgr :public DalConnIntf, public DalDmlIntf {
  public:
  enum Method {
    INIT,
    CONNECT,
    DISCONNECT,
    COMMIT,
    ROLLBACK,
    SINGLE,
    MULTIPLE,
    NEXT,
    CLOSE_CURSOR,
    RECORD_EXISTS,
    SIBLING_BEGIN,
    SIBLING_RECORDS,
    SIBLING_COUNT,
    RECORD_COUNT,
    DELETE_RECORD,
    UPDATE_RECORD,
    CREATE_RECORD,
    GET_DELETED_RECORDS,
    GET_CREATED_RECORDS,
    GET_UPDATED_RECORDS,
    COPY_ENTIRE,
    COPY_MODIFY,
    COPY_MODIFY_INSERT,
    COPY_MATCHING,
    CHECK_IDENTICAL,
    EXECUTE_APPQUERY_MULTIPLE,
    EXECUTE_APPQUERY_MODIFY,
    EXECUTE_APPQUERY_SINGLE,
    EXECUTE_QUERY,
  };
    DalOdbcMgr(void);

    ~DalOdbcMgr(void);

    DalResultCode Init(void);
    DalResultCode ConnectToDb(const DalConnType conn_type);
    DalResultCode DisconnectFromDb();

    DalResultCode CommitTransaction();

    DalResultCode RollbackTransaction();

    DalConnType get_conn_type();
    inline DalConnState get_conn_state() { return kDalDbDisconnected; }
    inline uint32_t get_write_count() { return write_count_; }
    inline void reset_write_count() { write_count_ = 0; }


    DalResultCode GetSingleRecord(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *ouput_and_matching_attr_info);

    DalResultCode GetMultipleRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor);

    DalResultCode GetNextRecord(const DalCursor *cursor);

    DalResultCode CloseCursor(DalCursor *cursor, bool);

    DalResultCode RecordExists(const UpllCfgType cfg_type,
                               const DalTableIndex table_index,
                               const DalBindInfo *matching_attr_info,
                               bool *existence);

    DalResultCode GetSiblingBegin(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor);

    DalResultCode GetSiblingRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor);

    DalResultCode GetSiblingCount(const UpllCfgType cfg_type,
                                  const DalTableIndex table_index,
                                  const DalBindInfo *matching_attr_info,
                                  uint32_t *count);

    DalResultCode GetRecordCount(const UpllCfgType cfg_type,
                                 const DalTableIndex table_index,
                                 const DalBindInfo *matching_attr_info,
                                 uint32_t *count);

    DalResultCode DeleteRecords(const UpllCfgType cfg_type,
                                const DalTableIndex table_index,
                                const DalBindInfo *matching_attr_info,
                                const bool truncate,
                                const TcConfigMode cfg_mode,
                                const uint8_t* vtn_name = NULL);

    DalResultCode CreateRecord(const UpllCfgType cfg_type,
                               const DalTableIndex table_index,
                               const DalBindInfo *input_attr_info,
                               const TcConfigMode cfg_mode,
                               const uint8_t* vtn_name = NULL);

    DalResultCode UpdateRecords(
                    string query_statement,
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *input_and_matching_attr_info,
                    const TcConfigMode cfg_mode,
                    const uint8_t* vtn_name = NULL);

    DalResultCode UpdateRecords(
                        const UpllCfgType cfg_type,
                        const DalTableIndex table_index,
                        const DalBindInfo *input_and_matching_attr_info,
                        const TcConfigMode cfg_mode,
                        const uint8_t* vtn_name = NULL);


    DalResultCode GetDeletedRecords(const UpllCfgType cfg_type_1,
                                    const UpllCfgType cfg_type_2,
                                    const DalTableIndex table_index,
                                    const size_t max_record_count,
                                    const DalBindInfo *output_attr_info,
                                    DalCursor **cursor,
                                    const TcConfigMode cfg_mode,
                                    const uint8_t* vtn_name = NULL);


    DalResultCode GetCreatedRecords(const UpllCfgType cfg_type_1,
                                    const UpllCfgType cfg_type_2,
                                    const DalTableIndex table_index,
                                    const size_t max_record_count,
                                    const DalBindInfo *output_attr_info,
                                    DalCursor **cursor,
                                    const TcConfigMode cfg_mode,
                                    const uint8_t* vtn_name = NULL);
    DalResultCode GetUpdatedRecords(
                    const UpllCfgType cfg_type_1,
                    const UpllCfgType cfg_type_2,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *cfg_1_output_and_match_attr_info,
                    const DalBindInfo *cfg_2_output_and_match_attr_info,
                    DalCursor **cursor,
                    const TcConfigMode cfg_mode,
                    const uint8_t* vtn_name = NULL);
    DalResultCode ClearCreateUpdateFlags(const DalTableIndex table_index,
                                    const UpllCfgType cfg_type,
                                    const TcConfigMode cfg_mode,
                                    const uint8_t *vtn_name,
                                    const bool create,
                                    const bool update) const;

    DalResultCode CopyEntireRecords(const UpllCfgType dest_cfg_type,
                                    const UpllCfgType src_cfg_type,
                                    const DalTableIndex table_index,
                                    const DalBindInfo *output_attr_info);

    DalResultCode CopyModifiedRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info,
                    const unc_keytype_operation_t op,
                    const TcConfigMode cfg_mode,
                    const uint8_t* vtn_name = NULL);

    DalResultCode CopyModifiedInsertRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info);


    DalResultCode CopyMatchingRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info,
                    const TcConfigMode cfg_mode,
                    const uint8_t* vtn_name = NULL);

    DalResultCode CheckRecordsIdentical(const UpllCfgType cfg_type_1,
                                        const UpllCfgType cfg_type_2,
                                        const DalTableIndex table_index,
                                        const DalBindInfo *matching_attr_info,
                                        bool *identical,
                                        const TcConfigMode cfg_mode,
                                        const uint8_t* vtn_name = NULL);

    DalResultCode ExecuteAppQuery(std::string query_stmt,
                            const UpllCfgType cfg_type,
                            const DalTableIndex table_index,
                            const DalBindInfo *bind_info,
                            const unc_keytype_operation_t dirty_op,
                            const TcConfigMode cfg_mode,
                            const uint8_t* vtn_name = NULL);

    DalResultCode ExecuteAppQueryMultipleRecords(
                         const std::string query_stmt,
                         const size_t max_record_count,
                         const DalBindInfo *bind_info,
                         DalCursor **cursor);

  DalResultCode DeleteRecordsInVtnMode(const UpllCfgType cfg_type,
                                       const DalTableIndex table_index,
                                       const DalBindInfo *bind_info,
                                       const bool truncate,
                                       const CfgModeType cfg_mode,
                                       const uint8_t* vtn_name) const;
    inline void ClearDirty() const {
      delete_dirty.clear();
      create_dirty.clear();
      update_dirty.clear();
    }
    inline void MakeAllDirty() const {
      delete_dirty.clear();
      create_dirty.clear();
      update_dirty.clear();
      for (uint16_t tbl_idx = schema::table::kDbiVtnTbl;
           tbl_idx < schema::table::kDalNumTables; tbl_idx++) {
        delete_dirty.insert(tbl_idx);
        create_dirty.insert(tbl_idx);
        update_dirty.insert(tbl_idx);
      }
    }

    // Add all the tables to dirty list
    inline void MakeAllTableDirtyInCache() const {
      UPLL_FUNC_TRACE;
      delete_dirty.clear();
      create_dirty.clear();
      update_dirty.clear();
      for (uint16_t tbl_idx = schema::table::kDbiVtnTbl;
           tbl_idx < schema::table::kDalNumTables; tbl_idx++) {
        delete_dirty.insert(tbl_idx);
        create_dirty.insert(tbl_idx);
        update_dirty.insert(tbl_idx);
      }
    }  // DalOdbcMgr::MakeAllTableDirtyInCache

    DalResultCode ExecuteQuery(SQLHANDLE *dal_stmt_handle,
                         const std::string *query_stmt,
                         const DalBindInfo *bind_info,
                         const uint32_t max_count);

    static void stub_setResultcode(DalOdbcMgr::Method methodType,
                                   DalResultCode res_code) {
      method_resultcode_map.insert(std::make_pair(methodType, res_code));
    }

    static void stub_setSingleRecordExists(bool exists) {
        exists_= exists;
    }
    inline void ClearDirtyTblCache(const TcConfigMode cfg_mode,
                                   const uint8_t* vtn_name) const {
      delete_dirty.clear();
      create_dirty.clear();
      update_dirty.clear();
    }

    inline bool IsAnyTableDirtyShallow() const {
      return ((delete_dirty.size() > 0) ||
              (create_dirty.size() > 0) ||
              (update_dirty.size() > 0));
    }
    inline bool IsTableDirtyShallow(const DalTableIndex table_index,
                                    const TcConfigMode cfg_mode,
                                    const uint8_t* vtn_name = NULL) const {
      return !((delete_dirty.end() == delete_dirty.find(table_index)) &&
               (create_dirty.end() == create_dirty.find(table_index)) &&
               (update_dirty.end() == update_dirty.find(table_index)));
    }

    inline bool IsTableDirtyShallowForOp(const DalTableIndex table_index,
                                         const unc_keytype_operation_t op,
                                         const TcConfigMode cfg_mode,
                                         const uint8_t* vtn_name = NULL) const {
      if (UNC_OP_DELETE == op) {
        return !(delete_dirty.end() == delete_dirty.find(table_index));
      } else if (op == UNC_OP_CREATE) {
        return !(create_dirty.end() == create_dirty.find(table_index));
      } else if (op == UNC_OP_UPDATE) {
        return !(update_dirty.end() == update_dirty.find(table_index));
      }
      return false;
    }

    static void clearStubData() {
      method_resultcode_map.clear();
    }
    DalResultCode  ExecuteAppQueryModifyRecord(
        const UpllCfgType cfg_type,
        const DalTableIndex table_index,
        const std::string query_stmt,
        const DalBindInfo *bind_info,
        const unc_keytype_operation_t op,
        const TcConfigMode cfg_mode,
        const uint8_t* vtn_name = NULL);

    DalResultCode ExecuteAppQuerySingleRecord(
        const std::string query_stmt,
        const DalBindInfo *bind_info);
    DalResultCode UpdateDirtyTblCacheFromDB() const; 
   DalResultCode ClearAllDirtyTblInDB(UpllCfgType cfg_type,
                                      const TcConfigMode cfg_mode,
                                      const uint8_t* vtn_name = NULL) const;
   DalResultCode ClearGlobalDirtyTblCacheAndDB(
       const DalTableIndex table_index,
       const unc_keytype_operation_t op) const;
  inline void set_wr_exclusion_on_runn() { wr_exclusion_on_runn_ = true; }
  private:
    bool CheckRunnUpdateQuery(const std::string *query_stmt) const;

    void AcquireRunnExclusiveLock() {
      wr_exclusion_runn_mutex_.lock();
      wr_exclusion_runn_mutex_acqd_ = true;
    }

    void ReleaseRunnExclusiveLock() {
      wr_exclusion_runn_mutex_.unlock();
      wr_exclusion_runn_mutex_acqd_ = false;
    }


    DalResultCode stub_getMappedResultCode(Method);
    static std::map<DalOdbcMgr::Method, DalResultCode> method_resultcode_map;
    static  bool exists_;
    mutable DalConnType conn_type_;
    mutable set<uint32_t> create_dirty;
    mutable set<uint32_t> delete_dirty;
    mutable set<uint32_t> update_dirty;
    mutable uint32_t write_count_;
    pfc::core::Mutex wr_exclusion_runn_mutex_;
    bool wr_exclusion_on_runn_;
    mutable bool wr_exclusion_runn_mutex_acqd_;
    mutable pfc::core::Mutex wr_exclusion_var_mutex_;
};  // class DalOdbcMgr

}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_ODBC_MGR_HH__
