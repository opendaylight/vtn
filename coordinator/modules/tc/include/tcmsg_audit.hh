/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef UNC_TC_TCMSG_AUDIT_H_
#define UNC_TC_TCMSG_AUDIT_H_
#include "tcmsg.hh"
#include <uncxx/tclib/tclib_defs.hh>
#include <string>

namespace unc {
namespace tc {

#define COMMIT_INFO_START_POS 3

/*Base class for audit operation*/
class  TcMsgAudit : public TcMsg {
 public:
  TcMsgAudit(uint32_t sess_id, unc::tclib::TcMsgOperType oper);
  /*settter function for controller-id and driver-id*/
  void SetData(uint32_t config_id,
               std::string controller_id,
               unc_keytype_ctrtype_t driver_id);
  void SetAuditType(TcAuditType audit_type);

  std::string controller_id_;
  unc_keytype_ctrtype_t driver_id_;
  TcAuditType audit_type_;

 protected:
  /*methods to send abort and trans-end notifications*/
  TcOperRet SendAbortRequest(AbortOnFailVector abort_on_fail_);
  TcOperRet SendAuditTransEndRequest(AbortOnFailVector abort_on_fail_,
                                      unc::tclib::TcMsgOperType oper);
};

/*class to handle Audit/Transaction Start and End*/
class AuditTransaction : public TcMsgAudit {
 public:
  AuditTransaction(uint32_t sess_id, unc::tclib::TcMsgOperType oper);
  TcOperRet Execute();
  void SetReconnect(pfc_bool_t force_reconnect);
  void IsUserAudit(pfc_bool_t user_audit);

  void SetCommitInfo(uint64_t commit_number,
                     uint64_t commit_date,
                     std::string commit_application);
  pfc_bool_t reconnect_controller_;
  uint64_t commit_number_;
  uint64_t commit_date_;
  std::string commit_application_;
  pfc_bool_t user_audit_;

 protected:
  TcOperRet SendRequest(std::string channel_name);
};
/*class to handle Audit Vote and Global commit notifications*/
class TwoPhaseAudit : public TcMsgAudit {
 public:
  TwoPhaseAudit(uint32_t sess_id, unc::tclib::TcMsgOperType oper);
  TcOperRet Execute();

 protected:
  /*methods to send vote/global commit to UPLL and UPPL*/
  TcOperRet SendRequest(std::string channel_name);
  /*method to collect controller-info from vote requests*/
  TcOperRet GetControllerInfo(pfc::core::ipc::ClientSession* sess);
  /*method to send vote request to driver*/
  TcOperRet SendRequestToDriver();
  /*methods to create and set driver result notification header*/
  TcOperRet CreateSessionsToForwardDriverResult();
  TcOperRet SetSessionToForwardDriverResult(pfc::core::ipc::ClientSession*
                                            tmpsess);
  /*method to handle response from UPLL/UPPL for Driver Result*/
  TcOperRet HandleDriverResultResponse(pfc::core::ipc::ClientSession* tmpsess);

  /*driver-id and controller list map*/
  TcDriverInfoMap driverinfo_map_;
  /*set of driver-ids*/
  DriverSet driver_set_;
  /*daemon name(UPLL/UPPL) and driver-id map*/
  TcDriverSetMap driverset_map_;
};


class GetDriverId : public TcMsgAudit {
   public:
    GetDriverId(uint32_t sess_id, unc::tclib::TcMsgOperType oper);
    TcOperRet Execute();
    unc_keytype_ctrtype_t GetResult();
};


}  // namespace tc
}  // namespace unc

#endif
