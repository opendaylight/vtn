/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "tcmsg.hh"
#include "tcmsg_commit.hh"
#include "tcmsg_audit.hh"

namespace unc {
namespace tc {

pfc_bool_t       TcMsg::audit_cancel_flag_ = PFC_FALSE;
pfc::core::Mutex TcMsg::audit_cancel_flag_mutex_;

unc_keytype_ctrtype_t TcMsg::notify_driver_id_ = UNC_CT_UNKNOWN;
pfc::core::Mutex      TcMsg::notify_driver_id_mutex_;

std::string      TcMsg::notify_controller_id_;
pfc::core::Mutex TcMsg::notify_controller_id_mutex_;


/*!\brief Parameterized constructor of TcMsg.
 * @param sess_id - session identifier.
 * @param oper - operation type
 * */
TcMsg::TcMsg(uint32_t sess_id, tclib::TcMsgOperType oper)
  :session_id_(sess_id), opertype_(oper),
    sess_(NULL), upll_sess_(NULL), uppl_sess_(NULL),
    conn_(0), upll_conn_(0), uppl_conn_(0),
    audit_result_(tclib::TC_AUDIT_SUCCESS),
    trans_result_(tclib::TRANS_END_SUCCESS) {
}


TcMsg::~TcMsg() {
  pfc_log_debug("Deleting TcMsg");
  TcUtilRet ret = TCUTIL_RET_SUCCESS;
  if (sess_) {
    ret = TcClientSessionUtils::tc_session_close(&sess_, conn_);
    if (ret != TCUTIL_RET_SUCCESS) {
      pfc_log_fatal("pfc_ipcclnt_altclose of sess_ failed");
    }
  }
  if (upll_sess_) {
    ret = TcClientSessionUtils::tc_session_close(&upll_sess_, upll_conn_);
    if (ret != TCUTIL_RET_SUCCESS) {
      pfc_log_fatal("pfc_ipcclnt_altclose of upll_sess_ failed");
    }
  }
  if (uppl_sess_) {
    ret = TcClientSessionUtils::tc_session_close(&uppl_sess_, uppl_conn_);
    if (ret != TCUTIL_RET_SUCCESS) {
      pfc_log_fatal("pfc_ipcclnt_altclose of uppl_sess_ failed");
    }
  }
}

/*!\brief This method returns appropriate sub-class instance based on
 * tclib::TcMsgOperType,to TC service handler.Parameterized constructors of
 * sub-classes are initialized.
 * @param[in] sess_id - session identifier.
 * @param[in] oper - operation type.
 * @result[out] TcMsg* - Base class pointer
 * */
TcMsg* TcMsg::CreateInstance(uint32_t sess_id,
                             tclib::TcMsgOperType oper,
                             TcChannelNameMap daemon_names) {
  pfc_log_debug("CreateInstance() entry::sess_id:%d oper:%d",
                sess_id, oper);

  TcMsg* ptr_tcmsg = NULL;

  switch (oper) {
    case tclib::MSG_NOTIFY_CONFIGID: {
      ptr_tcmsg = new TcMsgNotifyConfigId(sess_id, oper);
      break;
    }
    case tclib::MSG_SETUP_COMPLETE:
    case tclib::MSG_SETUP: {
      ptr_tcmsg = new TcMsgSetUp(sess_id, oper);
      break;
    }
    case tclib::MSG_COMMIT_TRANS_START:
    case tclib::MSG_COMMIT_TRANS_END: {
      ptr_tcmsg = new CommitTransaction(sess_id, oper);
      break;
    }
    case tclib::MSG_COMMIT_VOTE:
    case tclib::MSG_COMMIT_GLOBAL: {
      ptr_tcmsg = new TwoPhaseCommit(sess_id, oper);
      break;
    }
    case tclib::MSG_AUDIT_START:
    case tclib::MSG_AUDIT_TRANS_START:
    case tclib::MSG_AUDIT_TRANS_END:
    case tclib::MSG_AUDIT_END: {
      ptr_tcmsg = new AuditTransaction(sess_id, oper);
      break;
    }
    case tclib::MSG_AUDIT_VOTE:
    case tclib::MSG_AUDIT_GLOBAL: {
      ptr_tcmsg = new TwoPhaseAudit(sess_id, oper);
      break;
    }
    case tclib::MSG_SAVE_CONFIG:
    case tclib::MSG_CLEAR_CONFIG: {
      ptr_tcmsg = new TcMsgToStartupDB(sess_id, oper);
      break;
    }
    case tclib::MSG_ABORT_CANDIDATE: {
      ptr_tcmsg = new AbortCandidateDB(sess_id, oper);
      break;
    }
    case tclib::MSG_AUDITDB: {
      ptr_tcmsg = new TcMsgAuditDB(sess_id, oper);
      break;
    }
    case tclib::MSG_GET_DRIVERID: {
      ptr_tcmsg = new GetDriverId(sess_id, oper);
      break;
    }
    case tclib::MSG_AUTOSAVE_ENABLE:
    case tclib::MSG_AUTOSAVE_DISABLE: {
      ptr_tcmsg = new TcMsgAutoSave(sess_id, oper);
      break;
    }
    default: {
      pfc_log_error("TcMsg::CreateInstance() exit::Invalid opertype:%d", oper);
      return NULL;
    }
  }

  if (PFC_EXPECT_TRUE(ptr_tcmsg == NULL)) {
    pfc_log_fatal("could not create an instance of TcMsg class");
    return NULL;
  }
  PFC_ASSERT(TCOPER_RET_SUCCESS == daemon_names.empty());
  ptr_tcmsg->channel_names_ = daemon_names;

  return ptr_tcmsg;
}

/*TcMsgAutoSave class constructor*/
TcMsgAutoSave::TcMsgAutoSave(uint32_t sess_id,
                           tclib::TcMsgOperType oper)
  :TcMsg(sess_id, oper) {
  save_version_ = 0;
}

void TcMsgAutoSave::SetData(unc_keytype_datatype_t target_db,
                            TcServiceType fail_oper,
                            uint64_t save_version) {
  save_version_ = save_version;
}

TcOperRet TcMsgAutoSave::Execute() {
  pfc_log_debug("TcMsgAutoSave::Execute() entry");

  pfc_ipcid_t service_id = 0;
  pfc_ipcresp_t resp = 0;
   pfc_ipcconn_t conn = 0;
  std::string channel_name;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  /*channel_names_ map should not be empty */

  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());

  /*set notification order*/
  pfc_log_info("Sending autosave notification to tclib");
  notifyorder_.push_back(TC_UPLL);
  notifyorder_.push_back(TC_UPPL);
  if (opertype_ == tclib::MSG_AUTOSAVE_ENABLE) {
    service_id = tclib::TCLIB_AUTOSAVE_ENABLE;
  } else if (opertype_ == tclib::MSG_AUTOSAVE_DISABLE) {
    service_id = tclib::TCLIB_AUTOSAVE_DISABLE;
  } else {
    pfc_log_error("unexpected operation type:%d", opertype_);
    return TCOPER_RET_FAILURE;
  }

  /* send request to recipient module(s) */

  for (NotifyList::iterator list_iter = notifyorder_.begin();
       list_iter != notifyorder_.end(); list_iter++) {
    channel_name = GetChannelName(*list_iter);
    if (PFC_EXPECT_TRUE(channel_name.empty())) {
      pfc_log_error("channel_name is empty");
      return TCOPER_RET_FAILURE;
    }

    sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                                           service_id,
                                                           conn, PFC_TRUE);

    if (NULL == sess_) {
      return TCOPER_RET_FATAL;
    }

    // When notifying enable/disable AUTOSAVE, clear startup config
    // or save running to startup is executed. So set timeout as infinite.
    if (sess_->setTimeout(NULL) != TCOPER_RET_SUCCESS) {
      pfc_log_warn("Cannot set timeout as Infinite. Operation may timeout");
    }

    util_resp = TcClientSessionUtils::set_uint32(sess_, session_id_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      return ReturnUtilResp(util_resp);
    }

    util_resp = TcClientSessionUtils::set_uint64(sess_, save_version_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      return ReturnUtilResp(util_resp);
    }

    pfc_log_info("notify %s with sessid(%d) save_ver(%"PFC_PFMT_u64 
                 "),srv_id %d",
                 channel_name.c_str(), session_id_, save_version_, service_id);

    util_resp = TcClientSessionUtils::tc_session_invoke(sess_, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      TcClientSessionUtils::tc_session_close(&sess_, conn);
      return ReturnUtilResp(util_resp);
    }

    TcClientSessionUtils::tc_session_close(&sess_, conn);

    if (PFC_EXPECT_TRUE(resp != tclib::TC_SUCCESS)) {
      pfc_log_info("Failure response from %s", channel_name.c_str());
      break;
    }
    pfc_log_info("Success response from %s", channel_name.c_str());
  }

  notifyorder_.clear();
  pfc_log_info("TcMsgAutoSave::Execute() exit");
  return RespondToTc(resp);
}


/*!brief this method returns the channel name of daemon from TcChannelNameMap*/
std::string TcMsg::GetChannelName(TcDaemonName daemon_id)  {
  std::string channel;

  if (PFC_EXPECT_TRUE(channel_names_.find(daemon_id) != channel_names_.end())) {
    channel = channel_names_.find(daemon_id)->second;
  } else {
    return "";
  }
  return channel;
}

/*!brief this method returns daemon name for given channel name */
TcDaemonName TcMsg::GetDaemonName(std::string const & channel_name) {
  for (TcChannelNameMap::iterator it = channel_names_.begin();
       it !=  channel_names_.end();
       ++it) {
    if (it->second == channel_name) {
      return it->first;
    }
  }
  return TC_NONE;
}

/*!\brief This method is invoked by TC service handler at startup to fetch the
 * controller type of each driver module.
 * @param[in] channel_name - channel name of driver module.
 * @param[out] ctrtype - controller type of driver module.
 * @result - TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 * */
TcOperRet TcMsg::GetControllerType(std::string channel_name,
                                   unc_keytype_ctrtype_t *ctrtype) {
  pfc_log_info("GetControllerType() entry - channel_name:%s",
                channel_name.c_str());
  pfc_ipcresp_t resp = 0;
  pfc_ipcconn_t conn = 0;

  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_name.empty());

  /*create client session*/
  pfc::core::ipc::ClientSession* sess_ =
      TcClientSessionUtils::create_tc_client_session(channel_name,
                            tclib::TCLIB_CONTROLLER_TYPE, conn, PFC_FALSE);
  if (NULL == sess_) {
      return TCOPER_RET_FATAL;
  }

  TcUtilRet ret = TcClientSessionUtils::tc_session_invoke(sess_, resp);
  if (PFC_EXPECT_TRUE(ret == TCUTIL_RET_SUCCESS)) {
    /*controller type is returned in resp*/
    *ctrtype = (unc_keytype_ctrtype_t) resp;
  } else {
    TcClientSessionUtils::tc_session_close(&sess_, conn);
    return TCOPER_RET_FATAL;
  }
  pfc_log_info("GetControllerType() exit - controller_type::%d",
                *ctrtype);

  TcClientSessionUtils::tc_session_close(&sess_, conn);
  return TCOPER_RET_SUCCESS;
}

/*This method maps unc_keytype_ctrtype_t driver_id to TcDaemonName.*/
TcDaemonName TcMsg::MapTcDriverId(unc_keytype_ctrtype_t driver_id) {
  TcDaemonName drv_daemon = TC_NONE;

  switch (driver_id) {
    case UNC_CT_PFC: {
      drv_daemon = TC_DRV_OPENFLOW;
      break;
    }
    case UNC_CT_VNP: {
      drv_daemon = TC_DRV_OVERLAY;
      break;
    }
   /* case UNC_CT_LEGACY: {
      drv_daemon = TC_DRV_LEGACY;
      break;
    }*/
    case UNC_CT_POLC: {
      drv_daemon = TC_DRV_POLC;
      break;
    }
    case UNC_CT_VAN: {
      drv_daemon = TC_DRV_VAN;
      break;
    }
    case UNC_CT_ODC: {
      drv_daemon = TC_DRV_ODC;
      break;
    }
    default: {
       pfc_log_info("Invalid ControllerType:%d", driver_id);
      return TC_NONE;
    }
  }

  pfc_log_debug("unc_ctrtype:%d TcDaemonName:%d", driver_id, drv_daemon);
  return drv_daemon;
}

/*Get/set functions for audit operation end result*/
tclib::TcAuditResult TcMsg::GetAuditResult() {
  return audit_result_;
}

TcOperRet TcMsg::SetAuditResult(tclib::TcAuditResult result) {
  if (PFC_EXPECT_TRUE(result == tclib::TC_AUDIT_SUCCESS) ||
      PFC_EXPECT_TRUE(result == tclib::TC_AUDIT_FAILURE) ||
      PFC_EXPECT_TRUE(result == tclib::TC_AUDIT_CANCELLED) ) {
    audit_result_ = result;
  } else {
    pfc_log_warn("Invalid input for SetAuditResult():%d", result);
    return TCOPER_RET_FAILURE;
  }
  pfc_log_debug("audit_result_:%d", audit_result_);
  return TCOPER_RET_SUCCESS;
}

/*Get/set functions for commit operation end result*/
tclib::TcTransEndResult TcMsg::GetTransResult() {
  return trans_result_;
}

TcOperRet TcMsg::SetTransResult(tclib::TcTransEndResult result) {
  if (PFC_EXPECT_TRUE(result == tclib::TRANS_END_SUCCESS) ||
      PFC_EXPECT_TRUE(result == tclib::TRANS_END_FAILURE)) {
    trans_result_ = result;
  } else {
    pfc_log_error("Invalid input for SetTransResult():%d", result);
    return TCOPER_RET_FAILURE;
  }
  pfc_log_debug("trans_result_:%d", trans_result_);
  return TCOPER_RET_SUCCESS;
}

/*this virtual method to retrieve driver_id_ - impltd in GetDriverId class*/
unc_keytype_ctrtype_t  TcMsg::GetResult() {
  return UNC_CT_UNKNOWN;
}

/*!\brief This internal method forwards result from client session to server
 * session
 * @result - TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE
 * */
TcOperRet
TcMsg::ForwardResponseInternal(pfc::core::ipc::ServerSession& srv_sess,
                               pfc::core::ipc::ClientSession* clnt_sess,
                               pfc_bool_t decr_resp) {
  uint32_t respcount = clnt_sess->getResponseCount();
  if (PFC_EXPECT_TRUE(respcount == 0)) {
    pfc_log_info("session is empty");
    return TCOPER_RET_SUCCESS;
  }

  uint32_t from_index = 0, to_index = 0;
  if (decr_resp == PFC_TRUE) {
    to_index = respcount - 1;
  } else {
    to_index = respcount;
  }

  int32_t ipc_ret = clnt_sess->forwardTo(srv_sess, from_index, to_index);
  if (ipc_ret == ESHUTDOWN || ipc_ret == ECANCELED ||
      ipc_ret == ECONNABORTED || ipc_ret == ECONNRESET) {
    pfc_log_error("%s forwardTo failed!!", __FUNCTION__);
    return TCOPER_RET_FATAL;
  } else if (ipc_ret != TCOPER_RET_SUCCESS) {
    pfc_log_fatal("%s forwardTo failed!!", __FUNCTION__);
    return TCOPER_RET_FATAL;
  }

  pfc_log_info("data size forwarded to VTN: %d", to_index);
  return TCOPER_RET_SUCCESS;
}

/*!\brief method to forward the response from other modules to VTN.
 *@param srv_sess - VTN session parameter.
 *@result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FATAL
 **/

TcOperRet
TcMsg::ForwardResponseToVTN(pfc::core::ipc::ServerSession& srv_sess) {
  pfc_log_debug("TcMsg::ForwardResponseToVTN() entry");
  TcOperRet ret = TCOPER_RET_SUCCESS;

  if (sess_) {
    pfc_log_info("forward client session data");
    ret = ForwardResponseInternal(srv_sess, sess_, PFC_FALSE);
    if (ret != TCOPER_RET_SUCCESS) {
      pfc_log_info("forwarding session data to VTN failed");
      return TCOPER_RET_FATAL;
    }
  }

  if (upll_sess_) {
    pfc_log_info("forward UPLL session data");
    if (PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_GLOBAL)) {
      /*filtering audit_result of global commit*/
      ret = ForwardResponseInternal(srv_sess, upll_sess_, PFC_TRUE);
    } else {
      ret = ForwardResponseInternal(srv_sess, upll_sess_, PFC_FALSE);
    }
    if (ret != TCOPER_RET_SUCCESS) {
      pfc_log_info("forwarding UPLL session data to VTN failed");
      return TCOPER_RET_FATAL;
    }
  }

  if (uppl_sess_) {
    pfc_log_info("forward UPPL session data");
    if (PFC_EXPECT_TRUE(opertype_ == tclib::MSG_AUDIT_GLOBAL)) {
      /*filtering audit_result of global commit*/
      ret = ForwardResponseInternal(srv_sess, uppl_sess_, PFC_TRUE);
    } else {
      ret = ForwardResponseInternal(srv_sess, uppl_sess_, PFC_FALSE);
    }
    if (ret != TCOPER_RET_SUCCESS) {
      pfc_log_info("forwarding UPPL session data to VTN failed");
      return TCOPER_RET_FATAL;
    }
  }
  pfc_log_debug("TcMsg::ForwardResponseToVTN() exit");
  return TCOPER_RET_SUCCESS;
}

/*This method maps pfc_ipcresp_t resp to TcOperRet value*/
TcOperRet
TcMsg::RespondToTc(pfc_ipcresp_t resp) {
  if (PFC_EXPECT_TRUE(tclib::TC_SUCCESS == resp)) {
    return TCOPER_RET_SUCCESS;
  } else if (PFC_EXPECT_TRUE(tclib::TC_FAILURE == resp)) {
    return TCOPER_RET_FAILURE;
  } else if (PFC_EXPECT_TRUE(tclib::TC_LAST_DB_OPER_FAILURE == resp)) {
    return TCOPER_RET_LAST_DB_OP_FAILED;
  }
  return TCOPER_RET_UNKNOWN;
}

/*This method maps TcUtilRet ret to TcOperRet value.*/
TcOperRet
TcMsg::ReturnUtilResp(TcUtilRet ret) {
  TcOperRet oper_ret = TCOPER_RET_UNKNOWN;

  if (PFC_EXPECT_TRUE(TCUTIL_RET_SUCCESS == ret)) {
    oper_ret =  TCOPER_RET_SUCCESS;
  } else if (PFC_EXPECT_TRUE(TCUTIL_RET_FAILURE == ret)) {
    oper_ret =  TCOPER_RET_FAILURE;
  } else if (PFC_EXPECT_TRUE(TCUTIL_RET_FATAL == ret)) {
    oper_ret =  TCOPER_RET_FATAL;
  }
  pfc_log_info("ReturnUtilResp: Received(%u); Returned(%u)",
               ret, oper_ret);
  return oper_ret;
}

/*method to validate the operational cause of failover*/
TcOperRet
TcMsg::ValidateAuditDBAttributes(unc_keytype_datatype_t data_base,
                                 TcServiceType operation) {
  switch (operation) {
    case TC_OP_CANDIDATE_COMMIT:
    case TC_OP_CANDIDATE_COMMIT_TIMED:
    case TC_OP_USER_AUDIT:
    case TC_OP_DRIVER_AUDIT: {
      PFC_VERIFY(data_base == UNC_DT_RUNNING);
      break;
    }
    case TC_OP_CANDIDATE_ABORT: 
    case TC_OP_CANDIDATE_ABORT_TIMED: {
      PFC_VERIFY(data_base == UNC_DT_CANDIDATE);
      break;
    }
    case TC_OP_RUNNING_SAVE:
    case TC_OP_CLEAR_STARTUP: {
      PFC_VERIFY(data_base == UNC_DT_STARTUP);
      break;
    }
    case TC_OP_INVALID: {
      PFC_VERIFY(data_base == UNC_DT_INVALID);
      break;
    }
    default: {
      return TCOPER_RET_FAILURE;
    }
  }
  return TCOPER_RET_SUCCESS;
}

/*!\brief Parameterized constructor of TcMsgSetUp.
 * @param[in] sess_id - session identifier.
 * @param[in] oper - operation type
 * */
TcMsgSetUp::TcMsgSetUp(uint32_t sess_id,
                       tclib::TcMsgOperType oper)
  :TcMsg(sess_id, oper) {
}


/*!\brief SetData will set the validity  flag of the StartUpConfiguration
 * @param[in] startupconfig_valid_flag_  - validity flag
 * */
void TcMsgSetUp::SetData(pfc_bool_t autosave_enabled) {
  autosave_enabled_ = autosave_enabled;
}

/*!\brief Tcservice handler invokes this method to service
 * SETUP/SETUP_COMPLETE requests.
 * @result TCCommonRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE/TCOPER_RET_FATAL
 * */
TcOperRet TcMsgSetUp::Execute() {
  pfc_log_debug("TcMsgSetUp::Execute() entry");
  pfc_ipcid_t service_id = 0;
  pfc_ipcresp_t resp = 0;
  std::string channel_name;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());

  /*set notification order*/
  switch (opertype_) {
    case tclib::MSG_SETUP: {
      pfc_log_info("sending SETUP notification");
      notifyorder_.push_back(TC_UPLL);
      notifyorder_.push_back(TC_UPPL);
      service_id = tclib::TCLIB_SETUP;
      break;
    }
    case tclib::MSG_SETUP_COMPLETE: {
      pfc_log_info("sending SETUP COMPLETE notification");
      notifyorder_.push_back(TC_UPLL);
      notifyorder_.push_back(TC_UPPL);
      service_id = tclib::TCLIB_SETUP_COMPLETE;
      break;
    }
    default: {
      pfc_log_error("Invalid opertype:%d", opertype_);
      return TCOPER_RET_FAILURE;
    }
  }

  /* send request to recipient module(s) */
  for (NotifyList::iterator list_iter = notifyorder_.begin();
       list_iter != notifyorder_.end(); list_iter++) {
    channel_name = GetChannelName(*list_iter);
    if (PFC_EXPECT_TRUE(channel_name.empty())) {
      pfc_log_error("channel_name is empty");
      return TCOPER_RET_FAILURE;
    }

    sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                                           service_id,
                                                           conn_, PFC_TRUE);
    if (NULL == sess_) {
      return TCOPER_RET_FATAL;
    }

    util_resp = TcClientSessionUtils::set_uint8(sess_, autosave_enabled_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      return ReturnUtilResp(util_resp);
    }
    pfc_log_info("autosave_enabled?: %d", autosave_enabled_);
    pfc_log_info("notify %s with srv_id %d",
                 channel_name.c_str(), service_id);

    util_resp = TcClientSessionUtils::tc_session_invoke(sess_, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      return ReturnUtilResp(util_resp);
    }

    TcClientSessionUtils::tc_session_close(&sess_, conn_);

    if (PFC_EXPECT_TRUE(resp != tclib::TC_SUCCESS)) {
      pfc_log_info("Failure response from %s", channel_name.c_str());
      break;
    }
    pfc_log_info("Success response from %s", channel_name.c_str());
  }

  notifyorder_.clear();
  pfc_log_info("TcMsgSetUp::Execute() exit");
  return RespondToTc(resp);
}

/*!\brief Parameterized constructor of TcMsgNotifyConfigId.
 * @param[in] sess_id - session identifier.
 * @param[in] oper - operation type
 * */
TcMsgNotifyConfigId::TcMsgNotifyConfigId(uint32_t sess_id,
                                         tclib::TcMsgOperType oper)
  :TcMsg(sess_id, oper), config_id_(0) {
}

/*!\brief setter function for data member config_id_.
 * @param[in] config_id - configuration mode identifier. *
 * */
void TcMsgNotifyConfigId::SetData(uint32_t config_id, std::string controller_id,
                                  unc_keytype_ctrtype_t driver_id) {
  config_id_ = config_id;
}

/*!\brief setter function for data member config_id_, config_mode_ and vtn_name_.
 * @param[in] config_id - configuration mode identifier.
 * @param[in] config_mode - configuration mode type.
 * @param[in] vtn_name - vtn_name for configuration type VTN. 
 * */
void TcMsgNotifyConfigId::SetData(uint32_t config_id, TcConfigMode config_mode, 
                                            std::string vtn_name) {
  config_id_ = config_id;
  config_mode_ = config_mode;
  if (config_mode == TC_CONFIG_VTN) {
    vtn_name_ = vtn_name;
  } else {
    vtn_name_.clear();
  }
}

/*!\brief TC service handler invokes this method to notify the config_id and
 * session_id to UPLL and UPPL modules.
 * @result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE/TCOPER_RET_FATAL
 * */
TcOperRet TcMsgNotifyConfigId::Execute() {
  pfc_log_debug("TcMsgNotifyConfigId::Execute() entry");
  std::string channel_name;
  pfc_ipcresp_t resp = 0;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());

  notifyorder_.push_back(TC_UPLL);
  notifyorder_.push_back(TC_UPPL);
  pfc_log_debug("sending NOTIFY CONFIG-ID");

  for (NotifyList::iterator list_iter = notifyorder_.begin();
          list_iter != notifyorder_.end(); list_iter++) {
    channel_name = GetChannelName(*list_iter);
    if (PFC_EXPECT_TRUE(channel_name.empty())) {
      pfc_log_error("channel_name is empty");
      return TCOPER_RET_FAILURE;
    }

    sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                  tclib::TCLIB_NOTIFY_SESSION_CONFIG,
                                  conn_, PFC_FALSE);
    if (NULL == sess_) {
      return TCOPER_RET_FATAL;
    }

    util_resp = TcClientSessionUtils::set_uint32(sess_, session_id_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      return ReturnUtilResp(util_resp);
    }

    util_resp = TcClientSessionUtils::set_uint32(sess_, config_id_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      return ReturnUtilResp(util_resp);
    }

    util_resp = TcClientSessionUtils::set_uint8(sess_, config_mode_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      return ReturnUtilResp(util_resp);
    }

    util_resp = TcClientSessionUtils::set_string(sess_, vtn_name_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      return ReturnUtilResp(util_resp);
    }
    pfc_log_info("notify %s - session_id_:%u config_id_:%u"
                 " config_mode_:%d vtn_name_:%s",
                 channel_name.c_str(), session_id_, config_id_,
                  config_mode_, vtn_name_.c_str());

    util_resp = TcClientSessionUtils::tc_session_invoke(sess_, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      return ReturnUtilResp(util_resp);
    }

    TcClientSessionUtils::tc_session_close(&sess_, conn_);

    if (PFC_EXPECT_TRUE(resp != tclib::TC_SUCCESS)) {
      pfc_log_info("Failure response from %s", channel_name.c_str());
      break;
    }
    pfc_log_info("Success response from %s", channel_name.c_str());
  }

  pfc_log_debug("TcMsgNotifyConfigId::Execute() exit");
  notifyorder_.clear();
  return RespondToTc(resp);
}

/*!\brief Parameterized constructor of TcMsgToStartupDB
 * @param[in] sess_id - session identifier.
 * @param[in] oper - operation type
 * */
TcMsgToStartupDB::TcMsgToStartupDB(uint32_t sess_id,
                                   tclib::TcMsgOperType oper)
  :TcMsg(sess_id, oper) {
  save_version_ = 0;
}

void TcMsgToStartupDB::SetData(unc_keytype_datatype_t target_db,
                               TcServiceType fail_oper,
                               uint64_t save_version) {
  save_version_ = save_version;
}

/*!\brief TC service handler invokes this method to send save/clear request
 * to startup datastore
 * @result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE/TCOPER_RET_FATAL
 * */
TcOperRet TcMsgToStartupDB::Execute() {
  pfc_log_debug("TcMsgToStartupDB::Execute() entry");
  std::string channel_name;
  pfc_ipcresp_t resp = 0;
  pfc_ipcid_t service_id = 0;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;
  notifyorder_.clear();

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());

  notifyorder_.push_back(TC_UPLL);
  notifyorder_.push_back(TC_UPPL);

  if (PFC_EXPECT_TRUE(opertype_ == tclib::MSG_SAVE_CONFIG)) {
    service_id = tclib::TCLIB_SAVE_CONFIG;
    pfc_log_info("sending SAVE STARTUP notification");
  } else if (PFC_EXPECT_TRUE(opertype_ == tclib::MSG_CLEAR_CONFIG)) {
    pfc_log_info("sending CLEAR STARTUP notification");
    service_id = tclib::TCLIB_CLEAR_STARTUP;
  } else {
    pfc_log_error("Invalid opertype_:%d", opertype_);
    return TCOPER_RET_FAILURE;
  }

  for (NotifyList::iterator list_iter = notifyorder_.begin();
          list_iter != notifyorder_.end(); list_iter++) {
    channel_name = GetChannelName(*list_iter);
    if (PFC_EXPECT_TRUE(channel_name.empty())) {
      pfc_log_error("channel_name is empty");
      return TCOPER_RET_FAILURE;
    }

    /*Create session for the given module name and service id*/
    sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                                           service_id,
                                                           conn_);
    if (NULL == sess_) {
      return TCOPER_RET_FATAL;
    }
    /*append data to channel */
    if (PFC_EXPECT_TRUE(session_id_ > 0)) {
      util_resp = TcClientSessionUtils::set_uint32(sess_, session_id_);
      if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
        return ReturnUtilResp(util_resp);
      }

      if (opertype_ == tclib::MSG_SAVE_CONFIG) {
        pfc_log_debug("%s Setting save version in TcMsg", __FUNCTION__);
        util_resp = TcClientSessionUtils::set_uint64(sess_, save_version_);
        if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
          return ReturnUtilResp(util_resp);
        }
      }
    } else {
      pfc_log_error("Invalid Session ID");
      return TCOPER_RET_FAILURE;
    }
    pfc_log_info("notify %s - session_id_:%d",
                  channel_name.c_str(), session_id_);
    /*Invoke the session */
    util_resp = TcClientSessionUtils::tc_session_invoke(sess_, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      return ReturnUtilResp(util_resp);
    }
    TcClientSessionUtils::tc_session_close(&sess_, conn_);

    if (PFC_EXPECT_TRUE(resp != tclib::TC_SUCCESS)) {
      pfc_log_info("Failure response from %s", channel_name.c_str());
      break;
    }
    pfc_log_info("Success response from %s", channel_name.c_str());
  }

  pfc_log_debug("TcMsgToStartupDB::Execute() exit");
  notifyorder_.clear();
  return RespondToTc(resp);
}

/*!\brief Parameterized constructor of TcMsgAuditDB
 * @param sess_id - session identifier.
 * @param oper - operation type
 * */
TcMsgAuditDB::TcMsgAuditDB(uint32_t sess_id,
                           tclib::TcMsgOperType oper)
  :TcMsg(sess_id, oper), target_db_(UNC_DT_INVALID),
                         fail_oper_(TC_OP_INVALID),
                         version_(0), 
                         config_mode_(TC_CONFIG_INVALID) {
  vtn_name_.clear();
}


/*!\brief setter function to set the target datastore and failed operation
 * @param[in] target_db - target db for audit
 * @param[in] fail_oper - Operation which caused the failover.
 * @param[in] version - Abort / save version
 * */
void TcMsgAuditDB::SetData(unc_keytype_datatype_t target_db,
                           TcServiceType fail_oper,
                           uint64_t version) {
  target_db_ = target_db;
  fail_oper_ = fail_oper;
  version_   = version;
}

/*!\brief setter function to set the config_mode and vtn_name 
 * @param[in] config_mode 
 * @param[in] vtn_name.
 * */
void TcMsgAuditDB::SetData(uint32_t config_id,
                           TcConfigMode config_mode,
                           std::string vtn_name) {
  config_mode_ = config_mode;
  vtn_name_ = vtn_name;
}

/*!\brief TC service handler invokes this method to send audit db request
 * to UPLL and UPPL
 * @result TcOperRet - TCOPER_RET_SUCCESS/TCOPER_RET_FAILURE/TCOPER_RET_FATAL
 * */
TcOperRet TcMsgAuditDB::Execute() {
  pfc_log_debug("TcMsgAuditDB::Execute() entry");
  std::string channel_name;
  pfc_ipcresp_t resp = 0;
  TcUtilRet util_resp = TCUTIL_RET_SUCCESS;

  /*channel_names_ map should not be empty */
  PFC_ASSERT(TCOPER_RET_SUCCESS == channel_names_.empty());
  if (TCOPER_RET_SUCCESS != TcMsg::ValidateAuditDBAttributes(target_db_,
                                                             fail_oper_)) {
    pfc_log_error("Invalid AuditDB Attributes");
    return TCOPER_RET_FAILURE;
  }

  if (fail_oper_ == TC_OP_RUNNING_SAVE ||
      fail_oper_ == TC_OP_CANDIDATE_ABORT ||
      fail_oper_ == TC_OP_CANDIDATE_ABORT_TIMED) {
    notifyorder_.push_back(TC_UPLL);
    notifyorder_.push_back(TC_UPPL);
  } else {
    notifyorder_.push_back(TC_UPPL);
    notifyorder_.push_back(TC_UPLL);
  }
  pfc_log_info("sending AUDIT-DB notification");
  for (NotifyList::iterator list_iter = notifyorder_.begin();
       list_iter != notifyorder_.end(); list_iter++) {
    channel_name = GetChannelName(*list_iter);
    if (PFC_EXPECT_TRUE(channel_name.empty())) {
      pfc_log_error("channel_name is empty");
      return TCOPER_RET_FAILURE;
    }

    /*Create session for the given module name and service id*/
    sess_ = TcClientSessionUtils::create_tc_client_session(channel_name,
                                  tclib::TCLIB_AUDIT_CONFIG, conn_, PFC_TRUE);
    if (NULL == sess_) {
      return TCOPER_RET_FATAL;
    }
    /*append data to channel */
    util_resp = TcClientSessionUtils::set_uint8(sess_, target_db_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("%s Error setting target_db [%d] in session",
                    __FUNCTION__, target_db_);
      return ReturnUtilResp(util_resp);
    }
    util_resp = TcClientSessionUtils::set_uint8(sess_, fail_oper_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("%s Error setting fail_oper [%d] in session",
                    __FUNCTION__, fail_oper_);
      return ReturnUtilResp(util_resp);
    }
    
    util_resp = TcClientSessionUtils::set_uint8(sess_, config_mode_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("%s Error setting config_mode [%d] in session",
                    __FUNCTION__, config_mode_);
      return ReturnUtilResp(util_resp);
    }

    util_resp = TcClientSessionUtils::set_string(sess_, vtn_name_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("%s Error setting vtn_name [%s] in session",
                    __FUNCTION__, vtn_name_.c_str());
      return ReturnUtilResp(util_resp);
    }

    util_resp = TcClientSessionUtils::set_uint64(sess_, version_);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      pfc_log_error("%s Error setting abort/save version [%"PFC_PFMT_u64
                    "] in session", __FUNCTION__, version_);
      return ReturnUtilResp(util_resp);
    }

    pfc_log_info("notify %s - session[%u] target_db_:%d, fail_oper_:%d, "
                 "config_mode:%d, vtn_name:%s, "
                 "version_:%"PFC_PFMT_u64,
                 channel_name.c_str(), session_id_, target_db_, fail_oper_,
                 config_mode_, vtn_name_.c_str(),
                 version_);

    /*Invoke the session */
    util_resp = TcClientSessionUtils::tc_session_invoke(sess_, resp);
    if (PFC_EXPECT_TRUE(util_resp != TCUTIL_RET_SUCCESS)) {
      return ReturnUtilResp(util_resp);
    }

    TcClientSessionUtils::tc_session_close(&sess_, conn_);

    if (PFC_EXPECT_TRUE(resp == tclib::TC_FAILURE)) {
      pfc_log_info("Failure response from %s", channel_name.c_str());
      break;
    }

    if (resp == tclib::TC_LAST_DB_OPER_FAILURE) {
      pfc_log_warn("AuditDB: Received TC_LAST_DB_OPER_FAILURE"
                   "Stopping AuditDB");
      break;
    }

    pfc_log_info("Success response from %s", channel_name.c_str());
  }
  /*return server response */
  pfc_log_debug("TcMsgAuditDB::Execute() exit");
  notifyorder_.clear();
  return RespondToTc(resp);
}

 /* Set and Get Methods for auditCancelFlag */
void TcMsg::SetAuditCancelFlag(pfc_bool_t audit_cancelled) {
  pfc::core::ScopedMutex m(audit_cancel_flag_mutex_);
  audit_cancel_flag_ = audit_cancelled;
}

pfc_bool_t TcMsg::GetAuditCancelFlag()  {
  pfc::core::ScopedMutex m(audit_cancel_flag_mutex_);
  return audit_cancel_flag_;
}

void TcMsg::SetNotifyDriverId(unc_keytype_ctrtype_t id) {
  pfc::core::ScopedMutex m(notify_driver_id_mutex_);
  notify_driver_id_ = id;
}

unc_keytype_ctrtype_t TcMsg::GetNotifyDriverId() {
  pfc::core::ScopedMutex m(notify_driver_id_mutex_);
  return notify_driver_id_;
}

void TcMsg::SetNotifyControllerId(std::string const & ctr_id) {
  pfc::core::ScopedMutex m(notify_controller_id_mutex_);
  notify_controller_id_ = ctr_id;
}

std::string TcMsg::GetNotifyControllerId() {
  pfc::core::ScopedMutex m(notify_controller_id_mutex_);
  return notify_controller_id_;
}

}  // namespace tc
}  // namespace unc
