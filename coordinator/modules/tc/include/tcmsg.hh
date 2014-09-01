/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_TC_TCMSG_H_
#define UNC_TC_TCMSG_H_
#include <pfc/log.h>
#include <unc/tc/external/tc_services.h>
#include <uncxx/tclib/tclib_defs.hh>
#include <uncxx/tc/libtc_common.hh>
#include <pfcxx/ipc_server.hh>
#include <pfcxx/ipc_client.hh>
#include <tc_module_data.hh>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>

namespace unc {
namespace tc {

/*saves daemon names to handle failure scenario*/
typedef std::vector<TcDaemonName> AbortOnFailVector;
/*saves modules names to send notifications in appropriate order*/
typedef std::list<TcDaemonName> NotifyList;
/*saves list of controller-ids*/
typedef std::list<std::string> ControllerList;
/*driver-id and controller-list map*/
typedef std::map<unc_keytype_ctrtype_t, ControllerList> TcDriverInfoMap;
/*list of driver-ids*/
typedef std::set<unc_keytype_ctrtype_t> DriverSet;
/*daemon-name and driver-id map*/
typedef std::map<TcDaemonName, DriverSet> TcDriverSetMap;
/*return response from TcMsg to TcOper*/
typedef enum {
  TCOPER_RET_SUCCESS = 0,
  TCOPER_RET_FAILURE,
  TCOPER_RET_FATAL,
  TCOPER_RET_ABORT,
  TCOPER_RET_NO_DRIVER,
  TCOPER_RET_UNKNOWN,
  TCOPER_RET_SIMPLIFIED_AUDIT
}TcOperRet;
/*class to send notifications to all intended modules*/
class TcMsg {
 public:
  TcMsg(uint32_t sess_id, tclib::TcMsgOperType oper);

  virtual ~TcMsg();

  static TcMsg* CreateInstance(uint32_t sess_id,
                               tclib::TcMsgOperType oper,
                               TcChannelNameMap daemon_names);
  /*retrieves the controller-type at startup*/
  static TcOperRet GetControllerType(std::string channel_name,
                                     unc_keytype_ctrtype_t *ctrtype);

  static TcOperRet ValidateAuditDBAttributes(unc_keytype_datatype_t data_base,
                                             TcServiceType operation);
  /*setter functions*/

  virtual void SetData(pfc_bool_t autosave_enabled){}
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
  tclib::TcAuditResult GetAuditResult();
  TcOperRet SetAuditResult(tclib::TcAuditResult result);
  tclib::TcTransEndResult GetTransResult();
  TcOperRet SetTransResult(tclib::TcTransEndResult result);
  virtual void SetReconnect(pfc_bool_t force_reconnect){}
  virtual void IsUserAudit(pfc_bool_t user_audit){}
  /*method to send response to VTN*/
  TcOperRet ForwardResponseInternal(pfc::core::ipc::ServerSession& srv_sess,
                                    pfc::core::ipc::ClientSession* clnt_sess,
                                    pfc_bool_t decr_resp);
  TcOperRet ForwardResponseToVTN(pfc::core::ipc::ServerSession&
                                 srv_sess);

  /*user session identifier*/
  uint32_t session_id_;
  tclib::TcMsgOperType opertype_;
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
  tclib::TcAuditResult audit_result_;
  /*transaction result*/
  tclib::TcTransEndResult trans_result_;

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

/* Handles Autosave enable/disable */
class TcMsgAutoSave : public TcMsg {
 public:
  TcMsgAutoSave(uint32_t sess_id, tclib::TcMsgOperType oper);
  TcOperRet Execute();
};

/*handles SETUP and SETUP_COMPLETE notifications*/
class TcMsgSetUp : public TcMsg {
 public:
  TcMsgSetUp(uint32_t sess_id, tclib::TcMsgOperType oper);
  void SetData (pfc_bool_t autosave_enabled);
  TcOperRet Execute();
 private:
  pfc_bool_t autosave_enabled_;
};
/*handles NOTIFY_CONFIGID notification*/
class TcMsgNotifyConfigId : public TcMsg {
   public:
    TcMsgNotifyConfigId(uint32_t sess_id, tclib::TcMsgOperType oper);
    void SetData(uint32_t config_id,
                 std::string controller_id,
                 unc_keytype_ctrtype_t driver_id);
    TcOperRet Execute();

   private:
    uint32_t config_id_;
};
/*handles SAVE and CLEAR StartupDB notifications*/
class TcMsgToStartupDB : public TcMsg {
 public:
  TcMsgToStartupDB(uint32_t sess_id, tclib::TcMsgOperType oper);
  TcOperRet Execute();
};
/*handles AuditDB notifications*/
class TcMsgAuditDB : public TcMsg {
 public:
  TcMsgAuditDB(uint32_t sess_id, tclib::TcMsgOperType oper);
  void SetData(unc_keytype_datatype_t target_db, TcServiceType fail_oper);
  TcOperRet Execute();

 protected:
  unc_keytype_datatype_t target_db_;
  TcServiceType fail_oper_;
};

}   // namespace tc
}   // namespace unc
#endif
