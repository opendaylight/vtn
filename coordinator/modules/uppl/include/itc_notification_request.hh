/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * @brief	ITC Notification Request
 * @file        itc_notification_request.hh
 *
 */

#ifndef _ITC_NOTIFICATION_REQUEST_HH_
#define _ITC_NOTIFICATION_REQUEST_HH_

#include <pfcxx/module.hh>
#include <unc/keytype.h>
#include <vector>
#include <map>
#include <string>
#include "physical_itc_req.hh"
#include "odbcm_connection.hh"
using pfc::core::ipc::ClientSession;
using unc::uppl::OdbcmConnectionHandler;
using std::map;
namespace unc {
namespace uppl {

/* Struct to store the alarm detail */
struct alarm_buffer {
  alarm_buffer(uint32_t alarmtype, uint32_t opertype):
                   alarm_type(alarmtype), oper_type(opertype),
                   key_struct(NULL), val_struct(NULL) {
  }
  ~alarm_buffer() {
    if (key_struct != NULL) {
      ::operator delete(key_struct);
      key_struct = NULL;
    }
    if (val_struct != NULL) {
      ::operator delete(val_struct);
      val_struct = NULL;
    }
  }
  uint32_t alarm_type;
  uint32_t oper_type;
  void* key_struct;
  void* val_struct;
};

class NotificationRequest : public ITCReq {
  public:
  /*NotificationRequest constructor*/
  NotificationRequest();
  /*NotificationRequest destructor*/
  ~NotificationRequest();
  /*This function invoke ProcessNotificationEvents to validate
   *Key_Type with event message
        and call corresponding Key_Type class function*/
  pfc_bool_t ProcessEvent(const IpcEvent &event);
  /*map object to store the controller and related alarms */
  static map<string, vector<alarm_buffer*> > map_alarm_buff;
  UncRespCode InvokeKtDriverEvent(
      OdbcmConnectionHandler *db_conn,
      uint32_t operation,
      uint32_t data_type,
      void *key_struct,
      void *new_val_struct,
      void *old_val_struct,
      uint32_t key_type);
  /* This is a map to keep the alarms which has been sent to node manager */
  void FillAlarmDetails(uint32_t alarm_type,
                        uint32_t oper_type,
                        void* key_struct,
                        void* val_struct);
  void GetNotificationDT(OdbcmConnectionHandler *db_conn,
                         string controller_name,
                         uint32_t &data_type);

  private:
  UncRespCode ProcessPortEvents(ClientSession *sess,
                                   uint32_t data_type,
                                   uint32_t operation);
  UncRespCode ProcessPortNeighborEvents(ClientSession *sess,
                                   uint32_t data_type,
                                   uint32_t operation);
  UncRespCode ProcessSwitchEvents(ClientSession *sess,
                                     uint32_t data_type,
                                     uint32_t operation);
  UncRespCode ProcessLinkEvents(ClientSession *sess,
                                   uint32_t data_type,
                                   uint32_t operation);
  UncRespCode ProcessControllerEvents(ClientSession *sess,
                                         uint32_t data_type,
                                         uint32_t operation);
  UncRespCode ProcessDomainEvents(ClientSession *sess,
                                     uint32_t data_type,
                                     uint32_t operation);
  UncRespCode ProcessLogicalPortEvents(ClientSession *sess,
                                          uint32_t data_type,
                                          uint32_t operation);
  UncRespCode ProcessLogicalMemeberPortEvents(
      ClientSession *sess,
      uint32_t data_type,
      uint32_t operation);
  /*This function process notification events*/
  UncRespCode ProcessNotificationEvents(const IpcEvent &event);
  /*This function process alarm events*/
  UncRespCode ProcessAlarmEvents(const IpcEvent &event);
};
}  // namespace uppl
}  // namespace unc

#endif  // _ITC_NOTIFICATION_REQUEST_HH_
