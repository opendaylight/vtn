/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UNC_TCLIB_MSG_UTIL_HH_
#define _UNC_TCLIB_MSG_UTIL_HH_

#include <pfcxx/module.hh>
#include <uncxx/tclib/tclib_defs.hh>
#include <unc/tc/external/tc_services.h>
#include <tclib_struct_defs.hh>

namespace unc {
namespace tclib {

class TcLibMsgUtil {
 public:
  /**
   * @brief       Get of commit transaction arguments from the session  
   * @param[in]   session pointer of server session from where data will be read 
   * @param[out]  commit_trans_msg structure variable after reading from session
   * @retval      TC_SUCCESS Get data from session is success
   * @retval      TC_FAILURE Get data from session is failed
   */
  static TcCommonRet GetCommitTransactionMsg(pfc::core::ipc::ServerSession
                                             *session,
                                             TcCommitTransactionMsg
                                             &commit_trans_msg);

  /**
   * @brief       Get of commit driver vote/global arguments from the session
   * @param[in]   session pointer of server session from where data will be read 
   * @param[out]  drv_vote_global_msg structure variable after reading 
   *              from session
   * @retval      TC_SUCCESS Get data from session is success
   * @retval      TC_FAILURE Get data from session is failed
   */
  static TcCommonRet GetCommitDrvVoteGlobalMsg(pfc::core::ipc::ServerSession
                                               *session,
                                               TcCommitDrvVoteGlobalMsg
                                               &drv_vote_global_msg);

  /**
   * @brief       Get of commit driver result arguments from the session
   * @param[in]   session pointer of server session from where data will be read 
   * @param[out]  drv_result_msg structure variable after reading from session
   * @retval      TC_SUCCESS Get data from session is success
   * @retval      TC_FAILURE Get data from session is failed
   */
  static TcCommonRet GetCommitDrvResultMsg(pfc::core::ipc::ServerSession
                                           *session,
                                           TcCommitDrvResultMsg
                                           &drv_result_msg);

  /**
   * @brief       Get of commit global abort arguments from the session
   * @param[in]   session pointer of server session from where data will be read 
   * @param[out]  global_abort_msg structure variable after reading from session
   * @retval      TC_SUCCESS Get data from session is success
   * @retval      TC_FAILURE Get data from session is failed
   */
  static TcCommonRet GetCommitGlobalAbortMsg(pfc::core::ipc::ServerSession
                                             *session,
                                             TcCommitGlobalAbortMsg
                                             &global_abort_msg);

  /**
   * @brief       Get of audit transaction arguments from the session
   * @param[in]   session pointer of server session from where data will be read 
   * @param[out]  audit_trans_msg structure variable after reading from session
   * @retval      TC_SUCCESS Get data from session is success
   * @retval      TC_FAILURE Get data from session is failed
   */
  static TcCommonRet GetAuditTransactionMsg(pfc::core::ipc::ServerSession
                                            *session,
                                            TcAuditTransactionMsg
                                            &audit_trans_msg);

  /**
   * @brief       Get of audit driver vote/global arguments from the session
   * @param[in]   session pointer of server session from where data will be read 
   * @param[out]  drv_vote_global_msg structure variable after reading 
   *              from session
   * @retval      TC_SUCCESS Get data from session is success
   * @retval      TC_FAILURE Get data from session is failed
   */
  static TcCommonRet GetAuditDrvVoteGlobalMsg(pfc::core::ipc::ServerSession
                                              *session,
                                              TcAuditDrvVoteGlobalMsg
                                              &drv_vote_global_msg);

  /**
   * @brief       Get of audit driver result arguments from the session
   * @param[in]   session pointer of server session from where data will be read 
   * @param[out]  drv_result_msg structure variable after reading from session
   * @retval      TC_SUCCESS Get data from session is success
   * @retval      TC_FAILURE Get data from session is failed
   */
  static TcCommonRet GetAuditDrvResultMsg(pfc::core::ipc::ServerSession
                                           *session,
                                           TcAuditDrvResultMsg
                                           &drv_result_msg);

  /**
   * @brief       Get of audit global abort arguments from the session
   * @param[in]   session pointer of server session from where data will be read 
   * @param[out]  global_abort_msg structure variable after reading from session
   * @retval      TC_SUCCESS Get data from session is success
   * @retval      TC_FAILURE Get data from session is failed
   */
  static TcCommonRet GetAuditGlobalAbortMsg(pfc::core::ipc::ServerSession
                                             *session,
                                             TcAuditGlobalAbortMsg
                                             &global_abort_msg);

  /**
   * @brief       Get of audit config related arguments from the session
   * @param[in]   session pointer of server session from where data will be read 
   * @param[out]  audit_config_msg structure variable after reading from session
   * @retval      TC_SUCCESS Get data from session is success
   * @retval      TC_FAILURE Get data from session is failed
   */
  static TcCommonRet GetAuditConfigMsg(pfc::core::ipc::ServerSession *session,
                                       TcAuditConfigMsg &audit_config_msg);
};
}  // tclib
}  // unc

#endif /* _UNC_TCLIB_MSG_UTIL_HH_ */
