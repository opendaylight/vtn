/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <uncxx/tc/libtc_common.hh>
#include <pfcxx/module.hh>
#include "tcmsg.hh"
#include "tc_module.hh"
#include "tc_db_handler.hh"
#include "tc_operations.hh"
#include "tc_lock.hh"

int stub_srv_uint8 = 0;
int stub_srv_int32 = 0;
int stub_srv_uint32 = 0;
int stub_srv_string = 0;
int stub_opertype = 0;

#define CNTRL_COUNT 3
#define DRIVER_COUNT 2

using namespace std;
using namespace unc::tc;
using namespace unc::tclib;


class TestTcDbHandler : public TcDbHandler {
 public:
  TestTcDbHandler(std::string dsn_name):TcDbHandler(dsn_name) { }
  ~TestTcDbHandler() { }
  /*Initialises DB connection and sets the default values*/
  TcOperRet InitDB();
  /*methods to access TC_UNC_CONF_TABLE*/
  TcOperRet UpdateConfTable(pfc_bool_t auto_save);
  TcOperRet GetConfTable(pfc_bool_t* auto_save);
  /*methods to access TC_RECOVERY_TABLE*/
  TcOperRet UpdateRecoveryTable(unc_keytype_datatype_t data_base,
                                TcServiceType operation);
  TcOperRet GetRecoveryTable(unc_keytype_datatype_t* db,
                             TcServiceType* oper);
  /*checks whether the row exists with a
   *                                           * value*/
  TcOperRet IsRowExists(std::string table_name, std::string attribute);

 private:
  /*DB connection methods*/
  TcOperRet SetConnectionEnv();
  TcOperRet OpenDBConnection() {
    return TCOPER_RET_SUCCESS;
  }
  TcOperRet CloseDBConnection();
  /*methods to initialize
   *                                                           * TC DB tables*/
  TcOperRet InitTcDbTables();
  TcOperRet SetDefaultConfTable();
  TcOperRet SetDefaultRecoveryTable();
  /*method to retrieve SQLDriverConnect connection string*/
  std::string GetDBConnectString();
  /*methods to print readable strings in logs*/
  std::string ConvertOptoString(SQLINTEGER op);
  std::string ConvertDbasetoString(SQLINTEGER dbase);
  std::string ConvertAutoSaveString(pfc_bool_t autosave);
  /*method to log SQL error msg*/
  void GetErrorReason(SQLRETURN sqlret,
                      SQLSMALLINT handletype,
                      SQLHANDLE handle);

  std::string dsn_name_;   /*DB DSN name*/
  SQLHENV db_env_;         /*Environment handle for application*/
  SQLHDBC db_conn_handle_; /*Connection handle */
};


class TcTaskqUtil {
 public:
  explicit TcTaskqUtil(uint32_t concurrency, int32_t alarm_id);
  ~TcTaskqUtil();
  pfc::core::TaskQueue* taskq_;
  pfc::core::Timer* timed_;
  int32_t auditq_alarm_;
  int PostReadTimer(uint32_t session_id,
                    uint32_t timeout,
                    TcLock* tclock,
                    TcChannelNameMap& unc_map);
  int DispatchAuditDriverRequest(std::string controller_id,
                                 TcDbHandler* tc_db_hdlr,
                                 TcLock* tclock,
                                 TcChannelNameMap& unc_map,
                                 unc_keytype_ctrtype_t driver_id) {
    return 0;
  }
};

class TcMsg {
 public:
  TcMsg(uint32_t sess_id, unc::tclib::TcMsgOperType oper);

  virtual ~TcMsg();

  static TcMsg* CreateInstance(uint32_t sess_id,
                               unc::tclib::TcMsgOperType oper,
                               TcChannelNameMap daemon_names);
  /*retrieves the controller-type at startup*/
  static TcOperRet GetControllerType(std::string channel_name,
                                     unc_keytype_ctrtype_t *ctrtype);

  static TcOperRet ValidateAuditDBAttributes(unc_keytype_datatype_t data_base,
                                             TcServiceType operation);
  /*setter functions*/
  virtual void SetData(unc_keytype_datatype_t target_db,
                       TcServiceType fail_oper) {}

  virtual void SetData(uint32_t config_id,
                       std::string controller_id,
                       unc_keytype_ctrtype_t driver_id) {}
  /*method to get controller-type for user audit*/
  virtual unc_keytype_ctrtype_t  GetResult();
  /*method to send notifications in appropriate order*/
  virtual TcOperRet Execute() = 0;
  /*get-set functions for trans/audit operation results*/
  TcAuditResult GetAuditResult();
  TcOperRet SetAuditResult(TcAuditResult result);
  TcTransEndResult GetTransResult();
  TcOperRet SetTransResult(TcTransEndResult result);
  /*method to send response to VTN*/
  TcOperRet ForwardResponseInternal(pfc::core::ipc::ServerSession& srv_sess,
                                    pfc::core::ipc::ClientSession* clnt_sess,
                                    pfc_bool_t decr_resp);
  TcOperRet ForwardResponseToVTN(pfc::core::ipc::ServerSession&
                                 srv_sess);

  static void SetAuditCancelFlag(pfc_bool_t audit_cancelled);

  /*user session identifier*/
  uint32_t session_id_;
  unc::tclib::TcMsgOperType opertype_;
  /*channel name sof all active modules*/
  TcChannelNameMap channel_names_;
  /*IPC session parameters*/
  pfc::core::ipc::ClientSession* sess_;
  pfc::core::ipc::ClientSession* upll_sess_;
  pfc::core::ipc::ClientSession* uppl_sess_;
  /*IPC connection ids*/
  pfc_ipcconn_t conn_;
  pfc_ipcconn_t upll_conn_;
  pfc_ipcconn_t uppl_conn_;
  /*audit result*/
  TcAuditResult audit_result_;
  /*transaction result*/
  TcTransEndResult trans_result_;
  static pfc::core::Mutex audit_cancel_flag_mutex_; 
 protected:
  /*gets channel name of module*/
  std::string GetChannelName(TcDaemonName tc_client);
  /*mapping function for driver_ids*/
  TcDaemonName MapTcDriverId(unc_keytype_ctrtype_t driver_id);
  /*response mapping methods*/
  TcOperRet RespondToTc(pfc_ipcresp_t resp);
  TcOperRet ReturnUtilResp(TcUtilRet ret);
  NotifyList notifyorder_;
  AbortOnFailVector abort_on_fail_;
};



TcUtilRet
TcServerSessionUtils::set_srv_timeout(pfc::core::ipc::ServerSession* ssess,
                                      const pfc_timespec_t *timeout) {
  if (stub_srv_uint32 == 0) {
    pfc_log_info("get_uint32::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else {
    return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::get_uint8(
    pfc::core::ipc::ServerSession* ssess,
    uint32_t index,
    uint8_t* data) {
  if (stub_srv_uint8 == 0) {
    pfc_log_info("TcServerSessionUtils::get_uint8::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else if (stub_srv_uint8 == -1)  {
    pfc_log_info("TcServerSessionUtils::get_uint8::TCUTIL_RET_FATAL");
    return TCUTIL_RET_FATAL;
  } else {
    return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::get_string(
    pfc::core::ipc::ServerSession* ssess,
    uint32_t index,
    std::string& data) {
  if (stub_srv_string) {
    pfc_log_info("get_string::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else {
    return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::get_uint32(pfc::core::ipc::ServerSession* ssess,
                                           uint32_t index,
                                           uint32_t* data) {
  if ( stub_srv_uint32 == 0 ) {
    pfc_log_info("get_uint32::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else if (stub_srv_uint32 == -1) {
    pfc_log_info("get_uint32::TCUTIL_RET_FATAL");
    return TCUTIL_RET_FATAL;
  } else {
    return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::set_uint32(pfc::core::ipc::ServerSession* ssess,
                                           uint32_t audit_result ) {
  if ( stub_srv_uint32 == 0 ) {
    pfc_log_info("set_uint32::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else if (stub_srv_uint32 == -1) {
    pfc_log_info("set_uint32::TCUTIL_RET_FATAL");
    return TCUTIL_RET_FATAL;
  } else {
      return TCUTIL_RET_SUCCESS;
  }
}


TcUtilRet TcServerSessionUtils::set_uint8(pfc::core::ipc::ServerSession* ssess,
                                           uint8_t audit_result ) {
  if ( stub_srv_uint8 == 0 ) {
    pfc_log_info("set_uint8::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else if (stub_srv_uint8 == -1) {
    pfc_log_info("set_uint8::TCUTIL_RET_FATAL");
    return TCUTIL_RET_FATAL;
  } else {
      return TCUTIL_RET_SUCCESS;
  }
}

TcUtilRet TcServerSessionUtils::get_int32(pfc::core::ipc::ServerSession* ssess,
                                           uint32_t index,
                                           int32_t* data) {
  if ( stub_srv_int32 == 0 ) {
    pfc_log_info("get_int32::TCUTIL_RET_FAILURE");
    return TCUTIL_RET_FAILURE;
  } else if (stub_srv_int32 == -1)  {
    pfc_log_info("get_int32::TCUTIL_RET_FATAL");
    return TCUTIL_RET_FATAL;  
  }else {
    return TCUTIL_RET_SUCCESS;
  }
}

