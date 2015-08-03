/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <tc_operations.hh>


namespace unc {
namespace tc {

#define UNC_READ_STATUS_OPER_ARG_COUNT_MIN  2
#define UNC_READ_STATUS_OPER_ARG_COUNT_MAX  2
#define UNC_READ_STATUS_OPER_ARG_COUNT      2
pfc::core::Mutex TcReadStatusOperations::read_status_db_lock_;
uint64_t TcReadStatusOperations::commit_number_ = 1;
uint64_t TcReadStatusOperations::save_number_ = 1;
TcReadStatusType TcReadStatusOperations::runningDB_status_ =
                                        TC_DB_STATUS_CONFIRMED;
TcReadStatusType TcReadStatusOperations::startupDB_status_ =
                                        TC_DB_STATUS_CONFIRMED;

TcReadStatusOperations::TcReadStatusOperations(TcLock* tc_lock_,
                           pfc::core::ipc::ServerSession* sess_,
                           TcDbHandler* db_handler,
                           TcChannelNameMap& unc_map_):
    TcOperations(tc_lock_, sess_, db_handler, unc_map_) {
}

TcReadStatusOperations::~TcReadStatusOperations() {
}

/*
 * @brief Init commit/save number and status
 */

void TcReadStatusOperations::Init() {
  commit_number_  = 1;
  save_number_    = 1;
  runningDB_status_ = TC_DB_STATUS_CONFIRMED;
  startupDB_status_ = TC_DB_STATUS_CONFIRMED;
}

/*
 * @brief Validate the operation type from input
 */
TcOperStatus TcReadStatusOperations::TcValidateOperType() {
  pfc_log_trace("tc_read_status_oper: Validate Oper Type");
  if (tc_oper_ < TC_OP_READ_RUNNING_STATUS ||
      tc_oper_ > TC_OP_READ_STARTUP_STATUS) {
    pfc_log_error("TcValidateOperType opertype < TC_OP_READ_RUNNING_STATUS or"
                  " > TC_OP_READ_STARTUP_STATUS");
    return TC_INVALID_OPERATION_TYPE;
  }
  return TC_OPER_SUCCESS;
}

/*
 *  @brief Return Minimum argument count for config operations
 */
uint32_t TcReadStatusOperations::TcGetMinArgCount() {
  return UNC_READ_STATUS_OPER_ARG_COUNT_MIN;
}

/*
 * @brief Validate the operation paramaters for the service
 */
TcOperStatus TcReadStatusOperations::TcValidateOperParams() {
  return TC_OPER_SUCCESS;
} 

/*
 * @brief Secure exclusion for the operation
 */
TcOperStatus TcReadStatusOperations::TcGetExclusion() {
  return TC_OPER_SUCCESS;
} 

TcOperStatus TcReadStatusOperations::HandleLockRet(TcLockRet lock_ret) {
  return TC_OPER_SUCCESS;
}

/*
 * @brief create msg list for the operation
 */
TcOperStatus TcReadStatusOperations::TcCreateMsgList() {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Execute read status operation
 */
TcOperStatus TcReadStatusOperations::Execute() {
  return TC_OPER_SUCCESS;
}

TcOperStatus TcReadStatusOperations::FillTcMsgData(TcMsg*, TcMsgOperType) {
  return TC_OPER_SUCCESS;
}

/*
 * @brief release exclusion after the operation
 */
TcOperStatus TcReadStatusOperations::TcReleaseExclusion() {
  return TC_OPER_SUCCESS;
}

/*
 * @brief Final send response method
 */
TcOperStatus TcReadStatusOperations::
           SendAdditionalResponse(TcOperStatus oper_stat) {
  if (tc_oper_ == TC_OP_READ_RUNNING_STATUS)  {
    uint64_t commit_number;
    TcReadStatusType runningDB_status;
    if (GetRunningStatus(commit_number, runningDB_status) == TC_OPER_SUCCESS) {
      TcUtilRet ret = TcServerSessionUtils::set_uint8(ssess_, 
                                            runningDB_status);
      if (ret != TCUTIL_RET_SUCCESS ) {
       pfc_log_error("SetReadStatusOperation: setting runningDB_status failed");
       return TC_OPER_FAILURE;
      }
      ret = TcServerSessionUtils::set_uint64(ssess_,  commit_number);
      if (ret != TCUTIL_RET_SUCCESS ) {
        pfc_log_error("SetReadStatusOperation: setting commit_number failed");
        return TC_OPER_FAILURE;
      }
    }
  } else if (tc_oper_ == TC_OP_READ_STARTUP_STATUS) {
    uint64_t save_number;
    TcReadStatusType startupDB_status;
    if (GetStartupStatus(save_number, startupDB_status) == TC_OPER_SUCCESS) {
      TcUtilRet ret = TcServerSessionUtils::set_uint8(ssess_, 
                                            startupDB_status);
      if ( ret != TCUTIL_RET_SUCCESS ) {
        pfc_log_error("TcReadStatusOperation: setting startupDB_status failed");
        return TC_OPER_FAILURE;
      }
      ret = TcServerSessionUtils::set_uint64(ssess_, save_number);
      if ( ret != TCUTIL_RET_SUCCESS ) {
        pfc_log_error("TcReadStatusOperation: setting save_number failed");
        return TC_OPER_FAILURE;
      }
    }
  } else  {
    return TC_INVALID_OPERATION_TYPE;
  }
  return oper_stat;
}

/*
 *  @brief Check the number of input arguments for the operation
 */
TcOperStatus TcReadStatusOperations::TcCheckOperArgCount(uint32_t avail_count) {
  pfc_log_trace("tc_read_status_oper: Check count of Input Arguments");
  if ( tc_oper_ == TC_OP_READ_RUNNING_STATUS ||
       tc_oper_ == TC_OP_READ_STARTUP_STATUS ) {
    if ( avail_count != UNC_READ_STATUS_OPER_ARG_COUNT ) {
      pfc_log_error("TcCheckOperArgCount aq args expected(%u) received(%u)",\
                    UNC_READ_STATUS_OPER_ARG_COUNT, avail_count);
      return TC_OPER_INVALID_INPUT;
    }
  } else {
    pfc_log_error("%s Invalid operation type: %d", __FUNCTION__, tc_oper_);
    return TC_INVALID_OPERATION_TYPE;
  }
  return TC_OPER_SUCCESS;
}

/*
 *  @brief Get commit number and DB status of Running configuration
 */
TcOperStatus TcReadStatusOperations::GetRunningStatus(uint64_t& commit_number,
                                           TcReadStatusType& runningDB_status) {
  pfc::core::ScopedMutex r(read_status_db_lock_);
  commit_number = commit_number_;
  runningDB_status = runningDB_status_;
  return TC_OPER_SUCCESS;
}

/*
 *  @brief Get save number and DB status of Startup configuration
 */
TcOperStatus TcReadStatusOperations::GetStartupStatus(uint64_t& save_number,
                                          TcReadStatusType& startupDB_status) {
  pfc::core::ScopedMutex r(read_status_db_lock_);
  save_number = save_number_;
  startupDB_status = startupDB_status_;
  return TC_OPER_SUCCESS;
}

/*
 *  @brief Set Running status as TC_DB_STATUS_CONFIRMED
 */
TcOperStatus TcReadStatusOperations::SetRunningStatus()  {
  pfc::core::ScopedMutex r(read_status_db_lock_);
  runningDB_status_ = TC_DB_STATUS_UPDATING;
  return TC_OPER_SUCCESS;
}

/*
 *  @brief Set Startup status as TC_DB_STATUS_CONFIRMED
 */
TcOperStatus TcReadStatusOperations::SetStartupStatus() {
  pfc::core::ScopedMutex r(read_status_db_lock_);
  startupDB_status_ = TC_DB_STATUS_UPDATING;
  return TC_OPER_SUCCESS; 
}

/*
 *  @brief Set Running status as TC_DB_STATUS_UPDATING and
 *  increment commit number
 */
TcOperStatus TcReadStatusOperations::SetRunningStatusIncr() {
  pfc::core::ScopedMutex r(read_status_db_lock_);
  commit_number_++;
  runningDB_status_ = TC_DB_STATUS_CONFIRMED;
  return TC_OPER_SUCCESS;
}

/*
 *  @brief Set Startup status as TC_DB_STATUS_UPDATING and
 *  increment save number
 */
TcOperStatus TcReadStatusOperations::SetStartupStatusIncr() {
  pfc::core::ScopedMutex r(read_status_db_lock_);
  save_number_++;
  startupDB_status_ = TC_DB_STATUS_CONFIRMED;
  return TC_OPER_SUCCESS;
}

}  // namespace tc
}  // namespace unc

