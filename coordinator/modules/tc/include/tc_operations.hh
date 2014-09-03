/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_TC_TCOPERATIONS_H_
#define UNC_TC_TCOPERATIONS_H_
#include <tcmsg.hh>
#include <tc_lock.hh>
#include <tc_db_handler.hh>
#include <uncxx/tc/libtc_common.hh>
#include <unc/tc/external/tc_services.h>
#include<pfcxx/timer.hh>
#include<pfcxx/task_queue.hh>
#include <map>
#include <string>
#include <list>
#include <functional>
#include <deque>

typedef unc::tclib::TcMsgOperType  TcMsgOperType;

namespace unc {
namespace tc {

typedef enum {
  INPUT_VALIDATION = 1,
  GET_EXCLUSION_PHASE,
  CREATE_MSG_LIST,
  EXECUTE_PHASE,
  RELEASE_EXCLUSION_PHASE,
  SEND_RESPONSE_PHASE
}TcOperEnum;

/*
 * @brief TcOperations Class to be extended by all operations
 *        Provides virtual functions to extend and Dispatch 
 *        Method that invokes the functions
 */
class TcOperations {
 public:
  /* To Read User Input and write output */
  pfc::core::ipc::ServerSession* ssess_;

  /* To Perform DB Operations */
  TcDbHandler* db_hdlr_;

  /* List of Channel NAmes */
  TcChannelNameMap& unc_oper_channel_map_;
  /* To Get/Release Exclusion */
  TcLock* tclock_;

  /* Session ID of the user session */
  uint32_t session_id_;

  /* The Operation requested */
  TcServiceType tc_oper_;

  /* List of messages processed for the operation */
  std::list <TcMsgOperType> TcOperMessageList;

  /* To Track Operation and Revoke if needed */
  TcOperEnum tc_oper_status_;

  /*flag is set when Audit DB fails in startup phase*/
  pfc_bool_t audit_db_fail_;


  TcOperations(TcLock* tclock,
               pfc::core::ipc::ServerSession* sess,
               TcDbHandler* tc_db_,
               TcChannelNameMap& unc_map_);

  virtual ~TcOperations() {
    if ( db_hdlr_ != NULL )
      delete db_hdlr_;
  }

  /* Virtual Functions List */
  /* These have some default definitions in TcOperations */
  /* They can be overridden in the child class */
  virtual TcOperStatus HandleArgs();
  virtual TcOperStatus Execute();
  virtual TcOperStatus SendResponse(TcOperStatus oper_status_);
  virtual TcOperStatus HandleMsgRet(TcOperRet ret);
  virtual TcOperStatus GetOperType();
  virtual TcOperStatus GetSessionId();
  virtual TcOperStatus SetOperType();
  virtual TcOperStatus SetSessionId();
  virtual TcOperStatus SetOperStatus(TcOperStatus resp_out);
  virtual TcOperStatus Dispatch();
  virtual TcOperStatus RevokeOperation(TcOperStatus oper_resp_);

  /* Pure Virtual Functions List */
  /* Mandatory methods for any Tc Service */
  virtual uint32_t TcGetMinArgCount()=0;
  virtual TcOperStatus TcValidateOperType()=0;
  virtual TcOperStatus TcValidateOperParams()=0;
  virtual TcOperStatus TcCheckOperArgCount(uint32_t sess_arg_count)=0;
  virtual TcOperStatus  HandleLockRet(TcLockRet LockRet) =0;
  virtual TcOperStatus TcGetExclusion()=0;
  virtual TcOperStatus TcReleaseExclusion()=0;
  virtual TcOperStatus TcCreateMsgList()=0;
  virtual TcOperStatus FillTcMsgData(TcMsg*, TcMsgOperType)=0;
  virtual TcOperStatus SendAdditionalResponse(TcOperStatus oper_status)=0;
};


/*
 * @brief TcConfigOperations
 *        Provides Methods to handle
 *        Config Request from User
 */
class TcConfigOperations: public TcOperations {
 public:
  uint32_t config_id_;
  int32_t timeout_;
  pthread_cond_t * cond_var_;
  pthread_mutex_t mutex_var_;
  TcConfigOperations(TcLock* tclock,
                     pfc::core::ipc::ServerSession* sess,
                     TcDbHandler* tc_db_,
                     TcChannelNameMap& unc_map_);
  ~TcConfigOperations();
  void operator() ();
  uint32_t TcGetMinArgCount();
  TcOperStatus TcCheckOperArgCount(uint32_t sess_arg_count);
  TcOperStatus TcValidateOperType();
  TcOperStatus TcValidateOperParams();
  TcOperStatus TcGetExclusion();
  TcOperStatus TcReleaseExclusion();
  TcOperStatus TcCreateMsgList();
  TcOperStatus FillTcMsgData(TcMsg*, TcMsgOperType);
  TcOperStatus SendAdditionalResponse(TcOperStatus);
  TcOperStatus  HandleLockRet(TcLockRet LockRet);
  TcOperStatus  SetConfigId();
  pfc_bool_t IsConfigModeAvailable();

  void SetConfigModeAvailability(pfc_bool_t status);
  pfc_bool_t IsConfigReqQueueEmpty();
  pfc_bool_t IsConfigAcquireAllowed();
  void InsertConfigRequest();
  TcConfigOperations * RetrieveConfigRequest();
  void RemoveConfigRequest(uint32_t sess_id);
  void HandleConfigRelease();
  static pfc_bool_t IsStateChangedToSby();
  static void SetStateChangedToSby(pfc_bool_t state);
  static void   ClearConfigAcquisitionQueue();
  TcOperStatus Dispatch();
  TcOperStatus HandleTimedConfigAcquisition();
 private:
  static pfc_bool_t config_mode_available_;
  static pfc::core::Mutex config_mode_available_lock_;
  static std::deque<TcConfigOperations*> config_req_queue_;
  static pfc::core::Mutex config_req_queue_lock_;
  static pfc_bool_t state_changed_to_sby_;
  static pfc::core::Mutex state_changed_lock_;
};

/*
 * @brief TcStartUpOperations
 *        Provides Methods to handle
 *        TC operations on ACT
 */
class TcStartUpOperations: public TcOperations {
 public:
  pfc_bool_t is_switch_;
  TcServiceType fail_oper_;
  unc_keytype_datatype_t database_type_;

  TcStartUpOperations(TcLock* tclock,
                      pfc::core::ipc::ServerSession* sess,
                      TcDbHandler* tc_db_,
                      TcChannelNameMap& unc_map_,
                      pfc_bool_t is_switch);
  uint32_t TcGetMinArgCount();
  TcOperStatus TcCheckOperArgCount(uint32_t sess_arg_count);
  TcOperStatus TcValidateOperType();
  TcOperStatus TcValidateOperParams();
  TcOperStatus TcGetExclusion();
  TcOperStatus TcReleaseExclusion();
  TcOperStatus TcCreateMsgList();
  TcOperStatus FillTcMsgData(TcMsg*, TcMsgOperType);
  TcOperStatus HandleArgs();
  TcOperStatus HandleLockRet(TcLockRet LockRet);
  TcOperStatus SendAdditionalResponse(TcOperStatus oper_stat);
  TcOperStatus SendResponse(TcOperStatus oper_status);
  TcOperStatus SendAuditDBFailNotice(uint32_t alarm_id);
};

/*
 * @brief TcDbOperations
 *        Provides Methods to handle
 *        DB operations (Save/Clear StartUp)
 *        requests from user
 */
class TcDbOperations: public TcOperations {
 public:
  TcDbOperations(TcLock* tclock,
                 pfc::core::ipc::ServerSession* sess,
                 TcDbHandler* tc_db_,
                 TcChannelNameMap& unc_map_);
  uint32_t TcGetMinArgCount();
  TcOperStatus TcCheckOperArgCount(uint32_t sess_arg_count);
  TcOperStatus TcValidateOperType();
  TcOperStatus TcValidateOperParams();
  TcOperStatus TcGetExclusion();
  TcOperStatus TcReleaseExclusion();
  TcOperStatus TcCreateMsgList();
  TcOperStatus FillTcMsgData(TcMsg*, TcMsgOperType);
  TcOperStatus SendAdditionalResponse(TcOperStatus);
  TcOperStatus  HandleLockRet(TcLockRet LockRet);
  TcOperStatus HandleMsgRet(TcOperRet ret);
  TcOperStatus Dispatch();
};

class TcTaskqUtil;

/*
 * @brief TcReadOperations 
 *        Provides Methods to handle
 *        read operations
 *        requests from user
 */
class TcReadOperations: public TcOperations {
 public:
  TcTaskqUtil* read_handle_;
  pfc_bool_t arg_timeout_;
  uint32_t timeout_;
  TcReadOperations(TcLock* tclock,
                   pfc::core::ipc::ServerSession* sess,
                   TcDbHandler* tc_db_,
                   TcChannelNameMap& unc_map_,
                   TcTaskqUtil* read_taskq);
  uint32_t TcGetMinArgCount();
  TcOperStatus TcCheckOperArgCount(uint32_t sess_arg_count);
  TcOperStatus TcValidateOperType();
  TcOperStatus TcValidateOperParams();
  TcOperStatus TcGetExclusion();
  TcOperStatus Execute();
  TcOperStatus TcReleaseExclusion();
  TcOperStatus TcCreateMsgList();
  TcOperStatus FillTcMsgData(TcMsg*, TcMsgOperType);
  TcOperStatus SendAdditionalResponse(TcOperStatus oper_stat);
  TcOperStatus  HandleLockRet(TcLockRet LockRet);
};

/*
 * @brief TcAutoSaveOperations
 *        Provides Methods to handle
 *        AutoSave enable/disable
 *        requests from user
 */
class TcAutoSaveOperations: public TcOperations {
 public:
  pfc_bool_t autosave_;
  TcAutoSaveOperations(TcLock* tclock,
                       pfc::core::ipc::ServerSession* sess,
                       TcDbHandler* tc_db_,
                       TcChannelNameMap& unc_map_);
  uint32_t TcGetMinArgCount();
  TcOperStatus TcCheckOperArgCount(uint32_t sess_arg_count);
  TcOperStatus TcValidateOperType();
  TcOperStatus TcValidateOperParams();
  TcOperStatus TcGetExclusion();
  TcOperStatus TcReleaseExclusion();
  TcOperStatus TcCreateMsgList();
  TcOperStatus FillTcMsgData(TcMsg*, TcMsgOperType);
  TcOperStatus SendAdditionalResponse(TcOperStatus);
  TcOperStatus  HandleLockRet(TcLockRet LockRet);
  TcOperStatus SetAutoSave();
  TcOperStatus Execute();
};

/*
 * @brief TcCandidateOperations
 *        Provides Methods to handle
 *        abort/commit Candidiate database
 */
class TcCandidateOperations: public TcOperations {
 public:
  TcCandidateOperations(TcLock* tclock,
                        pfc::core::ipc::ServerSession* sess,
                        TcDbHandler* tc_db_,
                        TcChannelNameMap& unc_map_);

  ~TcCandidateOperations();
  uint32_t config_id_;
  TcMsg* resp_tc_msg_;
  unc::tclib::TcTransEndResult trans_result_;
  pfc_bool_t autosave_enabled_;
  TcOperStatus user_response_;
  uint32_t TcGetMinArgCount();
  TcOperStatus TcCheckOperArgCount(uint32_t sess_arg_count);
  TcOperStatus TcValidateOperType();
  TcOperStatus TcValidateOperParams();
  TcOperStatus TcGetExclusion();
  TcOperStatus TcReleaseExclusion();
  TcOperStatus TcCreateMsgList();
  TcOperStatus FillTcMsgData(TcMsg*, TcMsgOperType);
  TcOperStatus SendAdditionalResponse(TcOperStatus);
  TcOperStatus SendResponse(TcOperStatus);
  TcOperStatus  HandleLockRet(TcLockRet LockRet);
  TcOperStatus SetConfigId();
  TcOperStatus Execute();
  pfc_bool_t  TransStartMsg();
  pfc_bool_t  TransEndMsg();
  pfc_bool_t  TransVoteMsg();
  pfc_bool_t  TransGlobalCommitMsg();
};


/*
 * @brief TcAuditOperations
 *        Provides Methods to handle
 *        User/Driver Audit Requests
 */
class TcAuditOperations: public TcOperations {
 public:
  std::string controller_id_;
  unc_keytype_ctrtype_t driver_id_;
  TcMsg* resp_tc_msg_;
  TcOperStatus user_response_;
  TcTaskqUtil* audit_handle_;
  unc::tclib::TcAuditResult audit_result_;
  unc::tclib::TcTransEndResult trans_result_;
  pfc_bool_t api_audit_;
  pfc_bool_t force_reconnect_;

  TcAuditOperations(TcLock* tclock,
                    pfc::core::ipc::ServerSession* sess,
                    TcDbHandler* tc_db_,
                    TcChannelNameMap& unc_map_,
                    TcTaskqUtil* audit_);
  ~TcAuditOperations();
  uint32_t TcGetMinArgCount();
  TcOperStatus TcCheckOperArgCount(uint32_t sess_arg_count);
  TcOperStatus TcValidateOperType();
  TcOperStatus TcValidateOperParams();
  TcOperStatus TcGetExclusion();
  TcOperStatus TcReleaseExclusion();
  TcOperStatus TcCreateMsgList();
  TcOperStatus FillTcMsgData(TcMsg*, TcMsgOperType);
  TcOperStatus SendAdditionalResponse(TcOperStatus);
  TcOperStatus HandleLockRet(TcLockRet LockRet);
  TcOperStatus GetSessionId();
  TcOperStatus SetAuditOperationStatus();
  TcOperStatus Execute();
  pfc_bool_t  AuditTransStart();
  pfc_bool_t  AuditTransEnd();
  pfc_bool_t  AuditVote();
  pfc_bool_t  AuditGlobalCommit();
  pfc_bool_t  AuditStart();
  pfc_bool_t  AuditEnd();
  pfc_bool_t  GetDriverType();
};

/*
 * @brief TcTaskqUtil Class, wrapper to TaskQ
 *        with Tc specific operations
 */
class TcTaskqUtil {
 public:
  explicit TcTaskqUtil(uint32_t concurrency);
  ~TcTaskqUtil();
  pfc::core::TaskQueue* taskq_;
  pfc::core::Timer* timed_;
  int PostReadTimer(uint32_t session_id,
                    uint32_t timeout,
                    TcLock* tclock,
                    TcChannelNameMap& unc_map);
  int DispatchAuditDriverRequest(std::string controller_id,
                                 TcDbHandler* tc_db_hdlr,
                                 TcLock* tclock,
                                 TcChannelNameMap& unc_map,
                                 unc_keytype_ctrtype_t driver_id);
};


/*
 * @brief ReadParams Class used to post read timeouts
 */
class ReadParams : public std::unary_function < void, void > {
 public:
  uint32_t session_id_;
  TcLock* tclock_;
  TcChannelNameMap& unc_channel_map_;

  ReadParams(uint32_t session_id,
             TcLock* tclock,
             TcChannelNameMap& unc_map);
  void operator() ()  {
    HandleReadTimeout();
  }
  void HandleReadTimeout(void);
};

/*
 * @brief AuditParams Class used to handle driver audit
 */
class  AuditParams : public std::unary_function < void, void > {
 public:
  std::string  controller_id_;
  TcDbHandler* audit_db_hdlr_;
  TcLock* tclock_;
  TcChannelNameMap& unc_channel_map_;
  unc_keytype_ctrtype_t driver_id_;

  AuditParams(std::string controller_id,
              TcDbHandler* db_handler,
              TcLock* tclock,
              TcChannelNameMap& unc_map,
              unc_keytype_ctrtype_t driver_id);

  void operator() ()  {
    HandleDriverAudit();
  }
  void HandleDriverAudit(void);
};
}  // namespace tc
}  // namespace unc
#endif
