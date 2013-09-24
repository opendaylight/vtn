/*      Copyright (c) 2012 NEC Corporation                        */
/*      NEC CONFIDENTIAL AND PROPRIETARY                          */
/*      All rights reserved by NEC Corporation.                   */
/*      This program must be used solely for the purpose for      */
/*      which it was furnished by NEC Corporation.   No part      */
/*      of this program may be reproduced  or  disclosed  to      */
/*      others,  in  any form,  without  the  prior  written      */
/*      permission of NEC Corporation.    Use  of  copyright      */
/*      notice does not evidence publication of the program.      */

/**
 * dal_odbc_mgr.hh
 *   contians DalOdbcMgr class definition
 */

#ifndef __DAL_ODBC_MGR_HH__
#define __DAL_ODBC_MGR_HH__

#include "include/dal_defines.hh"
#include "include/dal_conn_intf.hh"
#include "include/dal_schema.hh"
#include "dal_bind_info.hh"
#include "dal_cursor.hh"
#include "dal_dml_intf.hh"
//#include "upll/upll_log.hh"
#include <iostream>
#include <map>
#include <list>
using namespace std;
namespace unc {
namespace upll {
namespace dal {

//class DalBindInfo;
//class DalCursor;
//typedef upll_keytype_datatype_t UpllCfgType;
class DalOdbcMgr :public DalConnIntf, public DalDmlIntf {
  public:

	enum Method
	{
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
                EXECUTE_QUERY,
	};
    DalOdbcMgr(void);

    ~DalOdbcMgr(void);

    DalResultCode Init(void);
    DalResultCode ConnectToDb(const DalConnType conn_type) ;
    DalResultCode DisconnectFromDb();

    DalResultCode CommitTransaction();

    DalResultCode RollbackTransaction();

    DalConnType get_conn_type();

    DalResultCode GetSingleRecord(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *ouput_and_matching_attr_info) ;

    DalResultCode GetMultipleRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor);

    DalResultCode GetNextRecord(const DalCursor *cursor);

    DalResultCode CloseCursor(DalCursor *cursor,bool);

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
                                const DalBindInfo *matching_attr_info);

    DalResultCode CreateRecord(const UpllCfgType cfg_type,
                               const DalTableIndex table_index,
                               const DalBindInfo *input_attr_info);

    DalResultCode UpdateRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *input_and_matching_attr_info);

    DalResultCode GetDeletedRecords(const UpllCfgType cfg_type_1,
                                    const UpllCfgType cfg_type_2,
                                    const DalTableIndex table_index,
                                    const size_t max_record_count,
                                    const DalBindInfo *output_attr_info,
                                    DalCursor **cursor);


    DalResultCode GetCreatedRecords(const UpllCfgType cfg_type_1,
                                    const UpllCfgType cfg_type_2,
                                    const DalTableIndex table_index,
                                    const size_t max_record_count,
                                    const DalBindInfo *output_attr_info,
                                    DalCursor **cursor);
    DalResultCode GetUpdatedRecords(
                    const UpllCfgType cfg_type_1,
                    const UpllCfgType cfg_type_2,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *cfg_1_output_and_match_attr_info,
                    const DalBindInfo *cfg_2_output_and_match_attr_info,
                    DalCursor **cursor);

    DalResultCode CopyEntireRecords(const UpllCfgType dest_cfg_type,
                                    const UpllCfgType src_cfg_type,
                                    const DalTableIndex table_index,
                                    const DalBindInfo *output_attr_info);

    DalResultCode CopyModifiedRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info);

    DalResultCode CopyModifiedInsertRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info);


    DalResultCode CopyMatchingRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info);

    DalResultCode CheckRecordsIdentical(const UpllCfgType cfg_type_1,
                                        const UpllCfgType cfg_type_2,
                                        const DalTableIndex table_index,
                                        const DalBindInfo *matching_attr_info,
                                        bool *identical);

    DalResultCode ExecuteAppQueryMultipleRecords(
                         const std::string query_stmt,
                         const size_t max_record_count,
                         const DalBindInfo *bind_info,
                         DalCursor **cursor) ;
    DalResultCode ExecuteQuery(SQLHANDLE *dal_stmt_handle,
                         const std::string *query_stmt,
                         const DalBindInfo *bind_info,
                         const uint32_t max_count);

    static void stub_setResultcode(DalOdbcMgr::Method methodType ,DalResultCode res_code) {
    	method_resultcode_map.insert(std::make_pair(methodType,res_code));
    }

    static void stub_setSingleRecordExists(bool exists) {
        exists_=exists;
    }
    static void stub_setSiblingCount(uint32_t sibling_count1) {
        sibling_count_=sibling_count1;
    }

    static void clearStubData() {
        	method_resultcode_map.clear();
                count=0;
                resultcodes.clear();  
        }
    static void stub_setNextRecordResultCodes( const std::map<uint8_t,DalResultCode>&  amap) {
      resultcodes=amap;
    }
  private:
    DalResultCode stub_getMappedResultCode(Method);
    static std::map<DalOdbcMgr::Method,DalResultCode> method_resultcode_map;
    static  bool exists_;
    static uint32_t sibling_count_;
    mutable DalConnType conn_type_;
    static map<uint8_t,DalResultCode> resultcodes;
    static uint32_t count;


};  // class DalOdbcMgr

}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_ODBC_MGR_HH__
