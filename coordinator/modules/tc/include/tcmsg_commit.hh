/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_TC_TCMSG_COMMIT_H_
#define UNC_TC_TCMSG_COMMIT_H_
#include "tcmsg.hh"
#include <uncxx/tclib/tclib_defs.hh>
#include <string>
#include <map>

namespace unc {
namespace tc {

/*class to handle commit operations*/
class TcMsgCommit : public TcMsg {
 public:
  TcMsgCommit(uint32_t sess_id,
              unc::tclib::TcMsgOperType oper);
  /*setter method for config-id*/
  void SetData(uint32_t config_id, std::string controller_id,
               unc_keytype_ctrtype_t driver_id);

  uint32_t config_id_;

 protected:
  /*methods to send abort and trans-end notifications*/
  TcOperRet SendAbortRequest(AbortOnFailVector abort_on_fail_);
  TcOperRet SendTransEndRequest(AbortOnFailVector abort_on_fail_);
};

/*class to handle candidate abort notifications*/
class AbortCandidateDB : public TcMsgCommit {
 public:
  uint64_t abort_version_;
  AbortCandidateDB(uint32_t sess_id,
                   unc::tclib::TcMsgOperType oper);
  TcOperRet Execute();
  void SetData(unc_keytype_datatype_t target_db,
               TcServiceType fail_oper,
               uint64_t version);
};
/*class to handle Transaction Start and End*/
class CommitTransaction : public TcMsgCommit {
 public:
  CommitTransaction(uint32_t sess_id,
                    unc::tclib::TcMsgOperType oper);
  TcOperRet Execute();

 protected:
  TcOperRet SendRequest(std::string channel_name);
};
/*class to handle vote and global commit notifications*/
class TwoPhaseCommit : public TcMsgCommit {
 public:
  TwoPhaseCommit(uint32_t sess_id,
                 unc::tclib::TcMsgOperType oper);
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

  //Method to handle driver not present and send dummoy resp to UPPL/UPLL
  TcOperRet HandleDriverNotPresent(unc_keytype_ctrtype_t);

  /*driver-id and controller list map*/
  TcDriverInfoMap driverinfo_map_;
  /*set of driver-ids*/
  DriverSet driver_set_;
  /*daemon name(UPLL/UPPL) and driver-id map*/
  TcDriverSetMap driverset_map_;
};

}  // namespace tc
}  // namespace unc

#endif
