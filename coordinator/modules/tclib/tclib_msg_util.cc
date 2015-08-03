/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <tclib_msg_util.hh>
#include <uncxx/tc/libtc_common.hh>

namespace unc {
namespace tclib {

/**
 * @brief       Get of commit transaction arguments from the session  
 * @param[in]   session pointer of server session from where data will be read 
 * @param[out]  commit_trans_msg structure variable after reading from session
 * @retval      TC_SUCCESS Get data from session is success
 * @retval      TC_FAILURE Get data from session is failed
 */
TcCommonRet TcLibMsgUtil::GetCommitTransactionMsg(
                             pfc::core::ipc::ServerSession *session,
                             TcCommitTransactionMsg &commit_trans_msg) {
  /* Retrieval of TcCommitTransactionMsg from IPC API */
  TcCommonRet ret = TC_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint32_t argcount = 0, idx = 0;
  uint8_t oper_type = 0;

  if (session == NULL) {
    pfc_log_error("%s %d Invalid session", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  argcount = session->getArgCount();
  pfc_log_debug("%s %d session arg count %d",
               __FUNCTION__, __LINE__, argcount);

  // argcount empty check
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is %d",
                   __FUNCTION__, __LINE__, argcount);
    return TC_FAILURE;
  }

  // oper_type
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx,
                                                      &oper_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  commit_trans_msg.oper_type = (TcMsgOperType) oper_type;

  // session_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &commit_trans_msg.session_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // config_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &commit_trans_msg.config_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // trans_end_result
  if (commit_trans_msg.oper_type == MSG_COMMIT_TRANS_END) {
    uint8_t end_result = 0;
    idx++;
    util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &end_result);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }
    commit_trans_msg.end_result = (TcTransEndResult) end_result;
  }

  return ret;
}

/**
 * @brief       Get of commit driver vote/global arguments from the session
 * @param[in]   session pointer of server session from where data will be read 
 * @param[out]  drv_vote_global_msg structure variable after reading 
 *              from session
 * @retval      TC_SUCCESS Get data from session is success
 * @retval      TC_FAILURE Get data from session is failed
 */
TcCommonRet TcLibMsgUtil::GetCommitDrvVoteGlobalMsg(
                             pfc::core::ipc::ServerSession *session,
                             TcCommitDrvVoteGlobalMsg &drv_vote_global_msg) {
  /* Retrieval of TcCommitDrvVoteGlobalMsg from IPC API */
  TcCommonRet ret = TC_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint32_t argcount = 0, idx = 0;
  uint8_t oper_type = 0;
  std::string controller_id;

  if (session == NULL) {
    pfc_log_error("%s %d Invalid session", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  argcount = session->getArgCount();
  pfc_log_debug("%s %d session arg count %d",
               __FUNCTION__, __LINE__, argcount);

  // argcount empty check
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is %d",
                   __FUNCTION__, __LINE__, argcount);
    return TC_FAILURE;
  }

  // oper_type
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &oper_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  drv_vote_global_msg.oper_type = (TcMsgOperType) oper_type;

  // session_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &drv_vote_global_msg.session_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // config_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &drv_vote_global_msg.config_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // cntrl_count
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx,
                                           &drv_vote_global_msg.cntrl_count);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  /* iterate for the cntrl_count times
   * and push the controller ids into
   * the list */
  for (uint8_t i = 0; i < drv_vote_global_msg.cntrl_count; i++) {
    // controller_id
    idx++;
    util_ret = tc::TcServerSessionUtils::get_string(session, idx,
                                                         controller_id);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }

    drv_vote_global_msg.cntrl_list.push_back(controller_id);
  }
  pfc_log_info("%s oper_type %d session_id %d config_id %d cntrl_count %d",
               __FUNCTION__, drv_vote_global_msg.oper_type,
               drv_vote_global_msg.session_id, drv_vote_global_msg.config_id,
               drv_vote_global_msg.cntrl_count);
  return ret;
}

/**
 * @brief       Get of commit driver result arguments from the session
 * @param[in]   session pointer of server session from where data will be read 
 * @param[out]  drv_result_msg structure variable after reading from session
 * @retval      TC_SUCCESS Get data from session is success
 * @retval      TC_FAILURE Get data from session is failed
 */
TcCommonRet TcLibMsgUtil::GetCommitDrvResultMsg(
                             pfc::core::ipc::ServerSession *session,
                             TcCommitDrvResultMsg &drv_result_msg) {
  /* Retrieval of TcCommitDrvResultMsg from IPC API */
  TcCommonRet ret = TC_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint8_t phase = 0, oper_type = 0;
  uint32_t argcount = 0, idx = 0;

  if (session == NULL) {
    pfc_log_error("%s %d Invalid session", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  argcount = session->getArgCount();
  pfc_log_debug("%s %d session arg count %d",
               __FUNCTION__, __LINE__, argcount);

  // argcount empty check
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is %d",
                   __FUNCTION__, __LINE__, argcount);
    return TC_FAILURE;
  }

  // oper_type
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &oper_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  drv_result_msg.oper_type = (TcMsgOperType) oper_type;

  // session_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &drv_result_msg.session_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // config_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &drv_result_msg.config_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // phase
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &phase);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  drv_result_msg.phase = (TcCommitPhaseType)phase;

  pfc_log_info("%s oper_type %d session_id %d config_id %d phase %d",
               __FUNCTION__, drv_result_msg.oper_type,
               drv_result_msg.session_id, drv_result_msg.config_id,
               drv_result_msg.phase);
  return ret;
}

/**
 * @brief       Get of commit global abort arguments from the session
 * @param[in]   session pointer of server session from where data will be read 
 * @param[out]  global_abort_msg structure variable after reading from session
 * @retval      TC_SUCCESS Get data from session is success
 * @retval      TC_FAILURE Get data from session is failed
 */
TcCommonRet TcLibMsgUtil::GetCommitGlobalAbortMsg(
                             pfc::core::ipc::ServerSession *session,
                             TcCommitGlobalAbortMsg &global_abort_msg) {
  /* Retrieval of TcCommitGlobalAbortMsg from IPC API */
  TcCommonRet ret = TC_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint8_t phase = 0, oper_type = 0;
  uint32_t argcount = 0, idx = 0;

  if (session == NULL) {
    pfc_log_error("%s %d Invalid session", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  argcount = session->getArgCount();
  pfc_log_info("%s %d session arg count %d",
               __FUNCTION__, __LINE__, argcount);

  // argcount empty check
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is %d",
                   __FUNCTION__, __LINE__, argcount);
    return TC_FAILURE;
  }

  // oper_type
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &oper_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  global_abort_msg.oper_type = (TcMsgOperType) oper_type;

  // session_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &global_abort_msg.session_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // config_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &global_abort_msg.config_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // phase
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &phase);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  global_abort_msg.commit_oper_phase = (TcCommitOpAbortPhase)phase;

  pfc_log_info("%s oper_type %d session_id %d config_id %d phase %d",
               __FUNCTION__, global_abort_msg.oper_type,
               global_abort_msg.session_id, global_abort_msg.config_id,
               global_abort_msg.commit_oper_phase);
  return ret;
}

/**
 * @brief       Get of audit transaction arguments from the session
 * @param[in]   session pointer of server session from where data will be read 
 * @param[out]  audit_trans_msg structure variable after reading from session
 * @retval      TC_SUCCESS Get data from session is success
 * @retval      TC_FAILURE Get data from session is failed
 */
TcCommonRet TcLibMsgUtil::GetAuditTransactionMsg(
                             pfc::core::ipc::ServerSession *session,
                             TcAuditTransactionMsg &audit_trans_msg) {
  /* Retrieval of TcAuditTransactionMsg from IPC API */
  TcCommonRet ret = TC_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint32_t argcount = 0, idx = 0;
  uint8_t oper_type = 0, driver_id = 0;

  if (session == NULL) {
    pfc_log_error("%s %d Invalid session", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  argcount = session->getArgCount();
  pfc_log_debug("%s %d session arg count %d",
               __FUNCTION__, __LINE__, argcount);

  // argcount empty check
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is %d",
                   __FUNCTION__, __LINE__, argcount);
    return TC_FAILURE;
  }

  // oper_type
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &oper_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  audit_trans_msg.oper_type = (TcMsgOperType)oper_type;

  // session_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &audit_trans_msg.session_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // driver_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &driver_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  audit_trans_msg.driver_id = (unc_keytype_ctrtype_t) driver_id;

  // controller_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_string(session, idx,
                                            audit_trans_msg.controller_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  if (audit_trans_msg.oper_type == MSG_AUDIT_START) {
    /*reconnect attribute*/
    uint8_t reconnect = 0;
    idx++;
    util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &reconnect);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }
    audit_trans_msg.reconnect_controller = (pfc_bool_t)reconnect;
    
    /* AuditType attribute */
    uint8_t audit_type = 0;
    idx++;
    util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &audit_type);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }
    audit_trans_msg.audit_type = (TcAuditType)audit_type;

    /*commit number attribute*/
    idx++;
    util_ret = tc::TcServerSessionUtils::get_uint64(session, idx,
                                            &audit_trans_msg.commit_number);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }
    /*commit date attribute*/
    idx++;
    util_ret = tc::TcServerSessionUtils::get_uint64(session, idx,
                                            &audit_trans_msg.commit_date);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }
    /*commit application attribute*/
    idx++;
    util_ret = tc::TcServerSessionUtils::get_string(session, idx,
                                            audit_trans_msg.commit_application);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }
  }

  // audit_result
  if (audit_trans_msg.oper_type == MSG_AUDIT_END) {
    uint8_t audit_result = 0;
    idx++;
    util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &audit_result);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }
    audit_trans_msg.audit_result = (TcAuditResult) audit_result;
    pfc_log_info("%s audit end audit_result %d",
                 __FUNCTION__, audit_trans_msg.audit_result);
  }

  // trans_end_result
  if (audit_trans_msg.oper_type == MSG_AUDIT_TRANS_END) {
    uint8_t end_result = 0;
    idx++;
    util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &end_result);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }
    audit_trans_msg.end_result = (TcTransEndResult) end_result;
    pfc_log_info("%s audit transaction end end_result %d",
                 __FUNCTION__, audit_trans_msg.end_result);
  }

  return ret;
}

/**
 * @brief       Get of audit driver vote/global arguments from the session
 * @param[in]   session pointer of server session from where data will be read 
 * @param[out]  drv_vote_global_msg structure variable after reading 
 *              from session
 * @retval      TC_SUCCESS Get data from session is success
 * @retval      TC_FAILURE Get data from session is failed
 */
TcCommonRet TcLibMsgUtil::GetAuditDrvVoteGlobalMsg(
                             pfc::core::ipc::ServerSession *session,
                             TcAuditDrvVoteGlobalMsg &drv_vote_global_msg) {
  /* Retrieval of TcAuditDrvVoteGlobalMsg from IPC API */
  TcCommonRet ret = TC_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint32_t argcount = 0, idx = 0;
  uint8_t oper_type = 0;
  std::string controller_id;

  if (session == NULL) {
    pfc_log_error("%s %d Invalid session", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  argcount = session->getArgCount();
  pfc_log_info("%s %d session arg count %d",
               __FUNCTION__, __LINE__, argcount);

  // argcount empty check
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is %d",
                   __FUNCTION__, __LINE__, argcount);
    return TC_FAILURE;
  }

  // oper_type
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &oper_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  drv_vote_global_msg.oper_type = (TcMsgOperType)oper_type;

  // session_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &drv_vote_global_msg.session_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // controller_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_string(session, idx,
                                            drv_vote_global_msg.controller_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // cntrl_count
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx,
                                           &drv_vote_global_msg.cntrl_count);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  /* iterate for the cntrl_count times
   * and push the controller ids into
   * the list */
  for (uint8_t i = 0; i < drv_vote_global_msg.cntrl_count; i++) {
    // controller_id
    idx++;
    util_ret = tc::TcServerSessionUtils::get_string(session, idx,
                                                         controller_id);
    if (util_ret != tc::TCUTIL_RET_SUCCESS) {
      pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                    __FUNCTION__, __LINE__, util_ret);
      return TC_FAILURE;
    }

    drv_vote_global_msg.cntrl_list.push_back(controller_id);
  }

  pfc_log_info("%s oper_type %d session_id %d controller_id %s cntrl_count %d",
               __FUNCTION__, drv_vote_global_msg.oper_type,
               drv_vote_global_msg.session_id,
               drv_vote_global_msg.controller_id.c_str(),
               drv_vote_global_msg.cntrl_count);
  return ret;
}

/**
 * @brief       Get of audit driver result arguments from the session
 * @param[in]   session pointer of server session from where data will be read 
 * @param[out]  drv_result_msg structure variable after reading from session
 * @retval      TC_SUCCESS Get data from session is success
 * @retval      TC_FAILURE Get data from session is failed
 */
TcCommonRet TcLibMsgUtil::GetAuditDrvResultMsg(
                             pfc::core::ipc::ServerSession *session,
                             TcAuditDrvResultMsg &drv_result_msg) {
  /* Retrieval of TcAuditDrvResultMsg from IPC API */
  TcCommonRet ret = TC_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint8_t phase = 0, oper_type = 0;
  uint32_t argcount = 0, idx = 0;

  if (session == NULL) {
    pfc_log_error("%s %d Invalid session", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  argcount = session->getArgCount();
  pfc_log_info("%s %d session arg count %d",
               __FUNCTION__, __LINE__, argcount);

  // argcount empty check
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is %d",
                   __FUNCTION__, __LINE__, argcount);
    return TC_FAILURE;
  }

  // oper_type
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &oper_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  drv_result_msg.oper_type = (TcMsgOperType) oper_type;

  // session_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &drv_result_msg.session_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // controller_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_string(session, idx,
                                            drv_result_msg.controller_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // phase
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &phase);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  drv_result_msg.phase = (TcCommitPhaseType)phase;

  pfc_log_info("%s oper_type %d session_id %d controller_id %s phase %d",
               __FUNCTION__, drv_result_msg.oper_type,
               drv_result_msg.session_id, drv_result_msg.controller_id.c_str(),
               drv_result_msg.phase);
  return ret;
}

/**
 * @brief       Get of audit global abort arguments from the session
 * @param[in]   session pointer of server session from where data will be read 
 * @param[out]  global_abort_msg structure variable after reading from session
 * @retval      TC_SUCCESS Get data from session is success
 * @retval      TC_FAILURE Get data from session is failed
 */
TcCommonRet TcLibMsgUtil::GetAuditGlobalAbortMsg(
                             pfc::core::ipc::ServerSession *session,
                             TcAuditGlobalAbortMsg &global_abort_msg) {
  /* Retrieval of TcAuditGlobalAbortMsg from IPC API */
  TcCommonRet ret = TC_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint8_t phase = 0, driver_id = 0, oper_type = 0;
  uint32_t argcount = 0, idx = 0;

  if (session == NULL) {
    pfc_log_error("%s %d Invalid session", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  argcount = session->getArgCount();
  pfc_log_info("%s %d session arg count %d",
               __FUNCTION__, __LINE__, argcount);

  // argcount empty check
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is %d",
                   __FUNCTION__, __LINE__, argcount);
    return TC_FAILURE;
  }

  // oper_type
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &oper_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  global_abort_msg.oper_type = (TcMsgOperType) oper_type;

  // session_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint32(session, idx,
                                            &global_abort_msg.session_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // driver_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &driver_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  global_abort_msg.driver_id = (unc_keytype_ctrtype_t)driver_id;

  // controller_id
  idx++;
  util_ret = tc::TcServerSessionUtils::get_string(session, idx,
                                            global_abort_msg.controller_id);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }

  // phase
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &phase);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils failed with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  global_abort_msg.audit_oper_phase = (TcAuditOpAbortPhase)phase;

  pfc_log_info("%s oper_type %d session_id %d driver_id %d controller_id %s"
               " phase %d", __FUNCTION__, global_abort_msg.oper_type,
               global_abort_msg.session_id, global_abort_msg.driver_id,
               global_abort_msg.controller_id.c_str(),
               global_abort_msg.audit_oper_phase);
  return ret;
}

/**
 * @brief       Get of audit config related arguments from the session
 * @param[in]   session pointer of server session from where data will be read 
 * @param[out]  audit_config_msg structure variable after reading from session
 * @retval      TC_SUCCESS Get data from session is success
 * @retval      TC_FAILURE Get data from session is failed
 */
TcCommonRet TcLibMsgUtil::GetAuditConfigMsg(
                             pfc::core::ipc::ServerSession *session,
                             TcAuditConfigMsg &audit_config_msg) {
  /* Retrieval of TcAuditGlobalAbortMsg from IPC API */
  TcCommonRet ret = TC_SUCCESS;
  tc::TcUtilRet util_ret = tc::TCUTIL_RET_SUCCESS;
  uint8_t db_type = 0, service_type = 0, config_mode = 0;
  uint64_t version = 0;
  std::string vtn_name;
  uint32_t argcount = 0, idx = 0;

  if (session == NULL) {
    pfc_log_error("%s %d Invalid session", __FUNCTION__, __LINE__);
    return TC_FAILURE;
  }

  argcount = session->getArgCount();
  pfc_log_info("%s %d session arg count %d",
               __FUNCTION__, __LINE__, argcount);

  // argcount empty check
  if (argcount == IPC_DEFAULT_ARG_COUNT) {
    pfc_log_error("%s %d Argument count is %d",
                   __FUNCTION__, __LINE__, argcount);
    return TC_FAILURE;
  }

  /*read from the session and update the session and config id*/

  // db_type
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx, &db_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils fail dbtype with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  audit_config_msg.db_type = (unc_keytype_datatype_t)db_type;

  // service_type
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx,
                                                      &service_type);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils fail op with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  audit_config_msg.service_type = (TcServiceType)service_type;

  // config_mode
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint8(session, idx,
                                                      &config_mode);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils fail config_mode with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  audit_config_msg.config_mode = (TcConfigMode)config_mode;

  // vtn_name
  idx++;
  util_ret = tc::TcServerSessionUtils::get_string(session, idx,
                                                      vtn_name);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils fail vtn_name with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  audit_config_msg.vtn_name = vtn_name;

  // Version
  idx++;
  util_ret = tc::TcServerSessionUtils::get_uint64(session, idx,
                                                  &version);
  if (util_ret != tc::TCUTIL_RET_SUCCESS) {
    pfc_log_error("%s %d TcServerSessionUtils fail version with %d",
                  __FUNCTION__, __LINE__, util_ret);
    return TC_FAILURE;
  }
  audit_config_msg.version = version;

  pfc_log_info("%s db_type %d service_type %d version %"PFC_PFMT_u64
               "conf_mode %d vtn-name %s", __FUNCTION__,
               audit_config_msg.db_type, audit_config_msg.service_type,
               audit_config_msg.version,
               audit_config_msg.config_mode, audit_config_msg.vtn_name.c_str());
  return ret;
}
}   // namespace tclib
}  // namespace unc
