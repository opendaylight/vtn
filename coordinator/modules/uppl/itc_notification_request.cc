/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * @brief	ITC Notification Request
 * @file     	itc_notification_request.cc
 */

#include "itc_notification_request.hh"
#include "itc_kt_port.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_link.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_boundary.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_logical_member_port.hh"
#include "ipct_util.hh"


using unc::uppl::NotificationRequest;

/**NotificationRequest
 * @Description:NotificationRequest constructor
 * @param[in]:none
 * @return   :none
 **/
NotificationRequest::NotificationRequest() {
}

/**NotificationRequest
 * @Description:NotificationRequest destructor
 * @param[in]:none
 * @return   :none
 **/
NotificationRequest::~NotificationRequest() {
}

/**ProcessEvent
 * @Description:ITC triggers this function when notification request is
 * received.This function invoke ProcessNotificationEvents() and
 * ProcessAlarmEvents
 * @param[in]:event-an object of IpcEvent
 * @return   :pfc_bool_t.PFC_TRUE is returned if this module is initialized
 * successfully otherwise PFC_FALSE is returned to denote error
 **/
pfc_bool_t NotificationRequest::ProcessEvent(const IpcEvent &event) {
  pfc_ipcevtype_t event_type(event.getType());
  if (event_type == UNC_PHYSICAL_EVENTS ||
      event_type == UNC_CTLR_STATE_EVENTS) {
    UncRespCode err = ProcessNotificationEvents(event);
    if (err != UNC_RC_SUCCESS) {
      pfc_log_error("ProcessNotificationEvents failed error no: %d", err);
      return PFC_FALSE;
    }
  } else if (event_type == UNC_ALARMS) {
    UncRespCode err = ProcessAlarmEvents(event);
    if (err != UNC_RC_SUCCESS) {
      pfc_log_error("ProcessAlarmEvents failed error no: %d", err);
      return PFC_FALSE;
    }
  } else {
    pfc_log_error("Invalid event type error no: %d",
                  UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED);
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

/**InvokeKtDriverEvent
 * @Description:This function invokes HandleDriverEvents() based 
 *              on the keytype and operation
 * @param[in]:
 * operation-type of operation which can be UNC_OP_CREATE/DELETE/UPDATE
 * data_type-type of database,UNC_DT_*
 * key_struct-void pointer that will point to any kt's key structure
 * new_val_struct-void pointer that will point to any kt's value structure
 * old_val_struct-void pointer that will point to any kt's value structure
 * key_type-any one of unc_key_type_t
 * @return   :Success or associated error code
 **/

UncRespCode NotificationRequest::InvokeKtDriverEvent(
    OdbcmConnectionHandler *db_conn,
    uint32_t operation,
    uint32_t data_type,
    void *key_struct,
    void *new_val_struct,
    void *old_val_struct,
    uint32_t key_type) {
  Kt_State_Base *ObjStateNotify = NULL;
  UncRespCode status = UNC_RC_SUCCESS;
  switch (key_type) {
    case UNC_KT_PORT: {
      ObjStateNotify = new Kt_Port();
      break;
    }
    case UNC_KT_SWITCH: {
      ObjStateNotify = new Kt_Switch();
      break;
    }
    case UNC_KT_LINK: {
      ObjStateNotify = new Kt_Link();
      break;
    }
    case UNC_KT_CTR_DOMAIN: {
      ObjStateNotify = new Kt_Ctr_Domain();
      break;
    }
    case UNC_KT_LOGICAL_PORT: {
      ObjStateNotify = new Kt_LogicalPort();
      break;
    }
    default: {
      pfc_log_error("Invalid key type\n");
      return UNC_UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED;
    }
  }
  switch (operation) {
    case UNC_OP_CREATE: {
      status = ObjStateNotify->HandleDriverEvents(db_conn, key_struct,
                                                  operation,
                                                  data_type, key_type, NULL,
                                                  new_val_struct);
      pfc_log_debug(
          "Return status of KT create HandleDriverEvents: %d", status);
      delete ObjStateNotify;
      break;
    }
    case UNC_OP_UPDATE: {
      status = ObjStateNotify->HandleDriverEvents(db_conn, key_struct,
                                                  operation,
                                                  data_type, key_type,
                                                  old_val_struct,
                                                  new_val_struct);
      pfc_log_debug(
          "Return status of update HandleDriverEvents: %d", status);
      delete ObjStateNotify;
      break;
    }
    case UNC_OP_DELETE: {
      status = ObjStateNotify->HandleDriverEvents(db_conn, key_struct,
                                                  operation,
                                                  data_type, key_type,
                                                  NULL, NULL);
      pfc_log_debug(
          "Return status of delete HandleDriverEvents: %d", status);
      delete ObjStateNotify;
      break;
    }
    default:
    {
      pfc_log_error("Invalid operation type\n");
      delete ObjStateNotify;
      return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
    }
  }
  return status;
}

/**ProcessNotificationEvents
 * @Description:This function process Notification events recieved 
 *              for various key types
 * @param[in]:event- an object of IpcEvent,contains the event posted by driver.
 * @return   :Success or associated error code
 **/
UncRespCode NotificationRequest::ProcessNotificationEvents(
    const IpcEvent &event) {
  pfc_log_info("Inside ProcessNotificationEvents of NotificationRequest");
  UncRespCode status = UNC_RC_SUCCESS;
  ClientSession sess(event.getSession());

  /*validate valid response count for Notification event structure*/
  uint32_t resp_count = sess.getResponseCount();
  if (resp_count < (uint32_t)6) {
    pfc_log_error("Invalid event structure - Resp Count %d", resp_count);
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  driver_event_header event_header;
  uint32_t header_parse = PhyUtil::sessGetDriverEventHeader(sess, event_header);
  if (header_parse != 0) {
    pfc_log_error("Unable to parse event header successfully");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  uint32_t data_type = event_header.data_type;
  uint32_t operation = event_header.operation;
  /*process port add, port update and port delete events*/
  switch (event_header.key_type) {
    case UNC_KT_PORT:
    {
      status = ProcessPortEvents(&sess,
                                 data_type,
                                 operation);
      if (status == UNC_UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_SWITCH:
    {
      status = ProcessSwitchEvents(&sess,
                                   data_type,
                                   operation);
      if (status == UNC_UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_LINK:
    {
      status = ProcessLinkEvents(&sess,
                                 data_type,
                                 operation);
      if (status == UNC_UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_CONTROLLER:
    {
      status = ProcessControllerEvents(&sess,
                                       data_type,
                                       operation);
      if (status == UNC_UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_CTR_DOMAIN:
    {
      status = ProcessDomainEvents(&sess,
                                   data_type,
                                   operation);
      if (status == UNC_UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_LOGICAL_PORT:
    {
      status = ProcessLogicalPortEvents(&sess,
                                        data_type,
                                        operation);
      if (status == UNC_UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT:
    {
      status = ProcessLogicalMemeberPortEvents(&sess,
                                               data_type,
                                               operation);
      if (status == UNC_UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    default:
    {
      pfc_log_error("Invalid key type\n");
      return UNC_UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED;
    }
  }
  return status;
}

/***GetNotificationDT
 * @Description:This function checks whether audit in progress for the
 * controller and use IMPORT db to store the notifications received from driver
 * @param[in]:controller_name-controller id
 * @param[out]:datatype-type of database,UNC_DT_*
 * @return   :void
 **/
void NotificationRequest::GetNotificationDT(OdbcmConnectionHandler *db_conn,
                                            string controller_name,
                                            uint32_t &data_type) {
  pfc_bool_t is_controller_in_audit = PhysicalLayer::get_instance()->
      get_ipc_connection_manager()->
      IsControllerInAudit(controller_name);
  if (is_controller_in_audit == PFC_TRUE) {
    data_type = (uint32_t)UNC_DT_IMPORT;
  }
  pfc_log_debug("GetNotificationDT data_type %d", data_type);
}

/***ProcessAlarmEvents
 * @Description:This function Process alarm events recieved for various 
 *              key types
 * @param[in]:event-an object of IpcEvent,contains the event posted by driver
 * @return   :Success or associated error code
 **/
UncRespCode NotificationRequest::ProcessAlarmEvents(const IpcEvent &event) {
  pfc_log_info("Inside ProcessAlarmEvents of NotificationRequest");
  // Check for MergeImportRunning Lock
  ScopedReadWriteLock eventDoneLock(
      PhysicalLayer::get_events_done_lock_(), PFC_FALSE);  // read lock
  UncRespCode db_ret = UNC_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteSb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  }
  UncRespCode status = UNC_RC_SUCCESS;
  ClientSession sess(event.getSession());

  /*validate valid response count for Notification event structure*/
  uint32_t resp_count = sess.getResponseCount();
  if (resp_count < (uint32_t)7) {
    pfc_log_error("Invalid alarm structure");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  /*Get key type, operation type and alarm type from IpcEvent*/
  driver_alarm_header alarm_header;
  uint32_t header_parse = PhyUtil::sessGetDriverAlarmHeader(sess, alarm_header);
  if (header_parse != 0) {
    pfc_log_error("Unable to parse alarms header successfully");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  uint32_t data_type = alarm_header.data_type;
  pfc_log_debug("Alarm Type %d", alarm_header.alarm_type);
  if (alarm_header.key_type == UNC_KT_CONTROLLER) {
    /*process controller related alarm*/
    key_ctr key_ctr;
    memset(&key_ctr, '\0', sizeof(key_ctr));
    int read_err = sess.getResponse((uint32_t)6, key_ctr);
    if (read_err != 0) {
      pfc_log_error("Key not received for controller");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("%s", IpctUtil::get_string(key_ctr).c_str());
    if ((alarm_header.operation != UNC_OP_CREATE) &&
        (alarm_header.operation != UNC_OP_DELETE)) {
      pfc_log_info("Invalid alarm operation : %d",
                   alarm_header.operation);
      return UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
    if (alarm_header.alarm_type == UNC_PHYS_PATH_FAULT) {
      val_phys_path_fault_alarm_t val_ctr_alarm_struct;
      int read_err = sess.getResponse((uint32_t)7, val_ctr_alarm_struct);
      if (read_err != 0) {
        pfc_log_error("Value not received for controller");
        return UNC_UPPL_RC_ERR_BAD_REQUEST;
      }
      string controller_name = reinterpret_cast<char *>
      (key_ctr.controller_name);
      GetNotificationDT(&db_conn, controller_name, data_type);
      Kt_Controller NotifyController;
      status = NotifyController.HandleDriverAlarms(
          &db_conn, data_type, alarm_header.alarm_type, alarm_header.operation,
          reinterpret_cast<void*>(&key_ctr),
          reinterpret_cast<void*>(&val_ctr_alarm_struct));
      pfc_log_debug(
          "Return status of controller HandleDriverAlarms: %d", status);
    } else {
      pfc_log_info("Invalid alarm type for UNC_KT_CONTROLLER: %d",
                   alarm_header.alarm_type);
      return UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
  } else  if (alarm_header.key_type ==  UNC_KT_CTR_DOMAIN) {
    /*process domain related alarm*/
    key_ctr_domain_t key_ctr_domain;
    memset(&key_ctr_domain, '\0', sizeof(key_ctr_domain_t));
    int read_err = sess.getResponse((uint32_t)6, key_ctr_domain);
    if (read_err != 0) {
      pfc_log_error("Key not received for ctr domain");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("%s", IpctUtil::get_string(key_ctr_domain).c_str());
    if ((alarm_header.operation != UNC_OP_CREATE) &&
        (alarm_header.operation != UNC_OP_DELETE)) {
      pfc_log_info("Invalid alarm operation : %d",
                   alarm_header.operation);
      return UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
    if (alarm_header.alarm_type ==  UNC_COREDOMAIN_SPLIT) {
      string controller_name = reinterpret_cast<char *>
      (key_ctr_domain.ctr_key.controller_name);
      Kt_Base *NotifyDomain = new Kt_Ctr_Domain();
      if (NotifyDomain == NULL) {
        pfc_log_error("Memory not allocated for Notifydomain");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetNotificationDT(&db_conn, controller_name, data_type);
      status = NotifyDomain->HandleDriverAlarms(
          &db_conn, data_type, alarm_header.alarm_type, alarm_header.operation,
          reinterpret_cast<void*>(&key_ctr_domain), NULL);
      pfc_log_debug(
          "Return status of domain HandleDriverAlarms: %d", status);
      delete NotifyDomain;
    } else {
      pfc_log_info("Invalid alarm type for UNC_KT_CTR_DOMAIN: %d",
                   alarm_header.alarm_type);
      return UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
  }  else  if (alarm_header.key_type ==  UNC_KT_LOGICAL_PORT) {
    /*process logical port related alarm*/
    key_logical_port_t key_logicalport;
    memset(&key_logicalport, '\0', sizeof(key_logical_port_t));
    int read_err = sess.getResponse((uint32_t)6, key_logicalport);
    if (read_err != 0) {
      pfc_log_error("Key not received for logical port");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("%s", IpctUtil::get_string(key_logicalport).c_str());
    if ((alarm_header.operation != UNC_OP_CREATE) &&
        (alarm_header.operation != UNC_OP_DELETE)) {
      pfc_log_info("Invalid alarm operation : %d",
                   alarm_header.operation);
      return UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
    if (alarm_header.alarm_type == UNC_SUBDOMAIN_SPLIT) {
      string controller_name = reinterpret_cast<char *>
      (key_logicalport.domain_key.ctr_key.controller_name);
      Kt_Base *NotifyLogicalPort = new Kt_LogicalPort();
      if (NotifyLogicalPort  == NULL) {
        pfc_log_error("Memory not allocated for NotifyLogicalPort");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetNotificationDT(&db_conn, controller_name, data_type);
      status = NotifyLogicalPort->HandleDriverAlarms(
          &db_conn, data_type, alarm_header.alarm_type, alarm_header.operation,
          reinterpret_cast<void*>(&key_logicalport), NULL);
      pfc_log_debug(
          "Return status of sub_domain HandleDriverAlarms: %d", status);
      delete NotifyLogicalPort;
    } else {
      pfc_log_info("Invalid alarm type for UNC_KT_LOGICAL_PORT: %d",
                   alarm_header.alarm_type);
      return UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
  }  else  if (alarm_header.key_type ==  UNC_KT_PORT) {
    /*process port related alarm*/
    key_port_t port_key;
    memset(&port_key, '\0', sizeof(key_port_t));
    int read_err = sess.getResponse((uint32_t)6, port_key);
    if (read_err != 0) {
      pfc_log_error("Key not received for port");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("%s", IpctUtil::get_string(port_key).c_str());
    if ((alarm_header.operation != UNC_OP_CREATE) &&
        (alarm_header.operation != UNC_OP_DELETE)) {
      pfc_log_info("Invalid alarm operation : %d",
                   alarm_header.operation);
      return UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
    if ((alarm_header.alarm_type ==  UNC_DEFAULT_FLOW) ||
        (alarm_header.alarm_type ==  UNC_PORT_DIRECTION) ||
        (alarm_header.alarm_type ==  UNC_PORT_CONGES)) {
      string controller_name = reinterpret_cast<char *>
      (port_key.sw_key.ctr_key.controller_name);
      Kt_Base *NotifyPort = new Kt_Port();
      if (NotifyPort  == NULL) {
        pfc_log_error("Memory not allocated for NotifyPort");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetNotificationDT(&db_conn, controller_name, data_type);
      status = NotifyPort->HandleDriverAlarms(
          &db_conn, data_type, alarm_header.alarm_type, alarm_header.operation,
          reinterpret_cast<void*>(&port_key), NULL);
      pfc_log_debug(
          "Return status of port HandleDriverAlarms: %d", status);
      delete NotifyPort;
    } else {
      pfc_log_info("Invalid alarm type for UNC_KT_PORT: %d",
                   alarm_header.alarm_type);
      return UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
  }  else  if (alarm_header.key_type ==  UNC_KT_SWITCH) {
    /*process controller related alarm*/
    key_switch_t switch_key;
    memset(&switch_key, '\0', sizeof(key_switch_t));
    int read_err = sess.getResponse((uint32_t)6, switch_key);
    if (read_err != 0) {
      pfc_log_error("Key not received for switch");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("%s", IpctUtil::get_string(switch_key).c_str());
    if ((alarm_header.operation != UNC_OP_CREATE) &&
        (alarm_header.operation != UNC_OP_DELETE)) {
      pfc_log_info("Invalid alarm operation : %d",
                   alarm_header.operation);
      return UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
    if ((alarm_header.alarm_type ==  UNC_FLOW_ENT_FULL) ||
        (alarm_header.alarm_type ==  UNC_OFS_LACK_FEATURES)) {
      string controller_name = reinterpret_cast<char *>
      (switch_key.ctr_key.controller_name);
      Kt_Base *NotifySwitch = new Kt_Switch();
      if (NotifySwitch  == NULL) {
        pfc_log_error("Memory not allocated for NotifySwitch");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetNotificationDT(&db_conn, controller_name, data_type);
      status = NotifySwitch->HandleDriverAlarms(
          &db_conn, data_type, alarm_header.alarm_type, alarm_header.operation,
          reinterpret_cast<void*>(&switch_key), NULL);
      pfc_log_debug(
          "Return status of switch HandleDriverAlarms: %d", status);
      delete NotifySwitch;
    } else {
      pfc_log_info("Invalid alarm type for UNC_KT_SWITCH: %d",
                   alarm_header.alarm_type);
      return UNC_UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
  } else {
    pfc_log_info("Invalid key type : %d", alarm_header.key_type);
  }
  return status;
}

/**ProcessPortEvents
 * @Description:This function processes events recieved for port
 * @param[in]:
 * sess-ClientSession object used to retrieve the response arguments
 * data_type-type of database,UNC_DT_*
 * operation-type of operation UNC_OP_CREATE/UPDATE/DELETE
 * @return   :Success or associated error code
 **/
UncRespCode NotificationRequest::ProcessPortEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  // Check for MergeImportRunning Lock
  ScopedReadWriteLock eventDoneLock(
      PhysicalLayer::get_events_done_lock_(), PFC_FALSE);  // read lock
  UncRespCode db_ret = UNC_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteSb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  }
  UncRespCode status = UNC_RC_SUCCESS;
  key_port_t port_key;
  memset(&port_key, '\0', sizeof(key_port_t));
  int read_err = sess->getResponse((uint32_t)5, port_key);
  if (read_err != 0) {
    pfc_log_error("Key not received for port");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(port_key).c_str());
  string controller_name = reinterpret_cast<char *>
  (port_key.sw_key.ctr_key.controller_name);
  GetNotificationDT(&db_conn, controller_name, data_type);
  val_port_st old_val_port, new_val_port;
  // val
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_port);
    if (read_err != 0) {
      pfc_log_error("New value not received for port");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("NEWVAL: %s", IpctUtil::get_string(new_val_port).c_str());
  }
  // old val
  if (operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)7, old_val_port);
    if (read_err != 0) {
      pfc_log_error("Old value not received for port");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("OLDVAL: %s",
                 IpctUtil::get_string(old_val_port).c_str());
  }
  // call driver event
  status = InvokeKtDriverEvent(&db_conn, operation, data_type,
                               reinterpret_cast<void*>(&port_key),
                               reinterpret_cast<void*>(&new_val_port),
                               reinterpret_cast<void*>(&old_val_port),
                               UNC_KT_PORT);
  return status;
}

/***ProcessSwitchEvents
 * @Description:This function processes events recieved for switch
 * @param[in]:
 * sess-ClientSession object used to retrieve the response arguments 
 * data_type-type of database,UNC_DT_*
 * operation-type of operation UNC_OP_CREATE/UPDATE/DELETE
 * @return   :Success or associated error code
 **/
UncRespCode NotificationRequest::ProcessSwitchEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  // Check for MergeImportRunning Lock
  ScopedReadWriteLock eventDoneLock(
      PhysicalLayer::get_events_done_lock_(), PFC_FALSE);  // read lock
  UncRespCode db_ret = UNC_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteSb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  }
  UncRespCode status = UNC_RC_SUCCESS;
  /*process switch add, switch update and switch delete events*/
  key_switch_t switch_key;
  memset(&switch_key, '\0', sizeof(key_switch_t));
  int read_err = sess->getResponse((uint32_t)5, switch_key);
  if (read_err != 0) {
    pfc_log_error("Key not received for switch");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(switch_key).c_str());
  string controller_name = reinterpret_cast<char *>
  (switch_key.ctr_key.controller_name);
  GetNotificationDT(&db_conn, controller_name, data_type);
  val_switch_st old_val_switch, new_val_switch;
  memset(&old_val_switch, '\0', sizeof(val_switch_st));
  memset(&new_val_switch, '\0', sizeof(val_switch_st));
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_switch);
    if (read_err != 0) {
      pfc_log_error("New value not received for switch");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("NEWVAL: %s",
                 IpctUtil::get_string(new_val_switch).c_str());
  }
  if (operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)7, old_val_switch);
    if (read_err != 0) {
      pfc_log_error("Old value not received for switch");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("OLDVAL: %s",
                 IpctUtil::get_string(old_val_switch).c_str());
  }
  status = InvokeKtDriverEvent(&db_conn, operation, data_type,
                               reinterpret_cast<void*>(&switch_key),
                               reinterpret_cast<void*>(&new_val_switch),
                               reinterpret_cast<void*>(&old_val_switch),
                               UNC_KT_SWITCH);
  return status;
}


/***ProcessLinkEvents
 * @Description:This function processes events recieved for link
 * @param[in]:
 * sess-ClientSession object used to retrieve the resposne arguments
 * data_type-type of database,UNC_DT_*
 * operation-type of operation UNC_OP_CREATE/UPDATE/DELETE
 * @return   :Success or associated error code
 **/
UncRespCode NotificationRequest:: ProcessLinkEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  // Check for MergeImportRunning Lock
  ScopedReadWriteLock eventDoneLock(
      PhysicalLayer::get_events_done_lock_(), PFC_FALSE);  // read lock
  UncRespCode db_ret = UNC_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteSb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  }
  UncRespCode status = UNC_RC_SUCCESS;
  /*process link add, link update and link delete events*/
  key_link_t key_link;
  memset(&key_link, '\0', sizeof(key_link_t));
  int read_err = sess->getResponse((uint32_t)5, key_link);
  if (read_err != 0) {
    pfc_log_error("Key not received for link");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(key_link).c_str());
  string controller_name = reinterpret_cast<char *>
  (key_link.ctr_key.controller_name);
  GetNotificationDT(&db_conn, controller_name, data_type);
  val_link_st old_val_link, new_val_link;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_link);
    if (read_err != 0) {
      pfc_log_error("New value not received for link");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("NEWVAL: %s",
                 IpctUtil::get_string(new_val_link).c_str());
  }
  if (operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)7, old_val_link);
    if (read_err != 0) {
      pfc_log_error("Old value not received for link");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("OLDVAL: %s",
                 IpctUtil::get_string(old_val_link).c_str());
  }
  status = InvokeKtDriverEvent(&db_conn, operation, data_type,
                               reinterpret_cast<void*>(&key_link),
                               reinterpret_cast<void*>(&new_val_link),
                               reinterpret_cast<void*>(&old_val_link),
                               UNC_KT_LINK);
  return status;
}

/***ProcessControllerEvents
 * @Description:This function processes events recieved for Controller
 * @param[in]:
 * sess-ClientSession object used to retrieve the response arguments
 * data_type-type of database,UNC_DT_*
 * operation-type of operation UNC_OP_CREATE/UPDATE/DELETE
 * @return   :Success or associated error code
 **/
UncRespCode NotificationRequest:: ProcessControllerEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  UncRespCode status = UNC_RC_SUCCESS;
  /*process controller add, controller update and controller delete events*/
  key_ctr_t key_ctr;
  memset(&key_ctr, '\0', sizeof(key_ctr_t));
  val_ctr_st old_val_ctr, new_val_ctr;
  int read_err = sess->getResponse((uint32_t)5, key_ctr);
  if (read_err != 0) {
    pfc_log_error("Key not received for controller");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(key_ctr).c_str());
  Kt_Controller NotifyController;
  read_err = sess->getResponse((uint32_t)6, new_val_ctr);
  if (read_err != 0) {
    pfc_log_error("New value not received for controller");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("NEWVAL: %s", IpctUtil::get_string(new_val_ctr).c_str());
  read_err = sess->getResponse((uint32_t)7, old_val_ctr);
  if (read_err != 0) {
    pfc_log_error("Old value not received for controller");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("OLDVAL: %s", IpctUtil::get_string(old_val_ctr).c_str());
  pfc_bool_t is_events_done = PFC_FALSE;
  uint8_t driver_oper_status = new_val_ctr.oper_status;
  UncRespCode db_ret = UNC_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteSb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  }
  if (driver_oper_status == CONTROLLER_EVENTS_DONE) {
    // CONTROLLER_OPER_UP can be set
    // Its same as enum UPPL_CONTROLLER_OPER_UP
    new_val_ctr.oper_status = CONTROLLER_OPER_UP;
    string controller_name = reinterpret_cast<char *>
    (key_ctr.controller_name);
    pfc_log_info(
        "Received end of events notification for controller %s",
        controller_name.c_str());
    IPCConnectionManager *ipc_mgr = PhysicalLayer::get_instance()->
        get_ipc_connection_manager();
    pfc_bool_t is_controller_in_audit = ipc_mgr->
        IsControllerInAudit(controller_name);
    if (is_controller_in_audit == PFC_TRUE) {
      pfc_log_debug("Calling MergeAuditDbToRunning");
      // To cancel the already running timer in Audit
      UncRespCode cancel_ret = ipc_mgr->CancelTimer(controller_name);
      if (cancel_ret != UNC_RC_SUCCESS) {
        pfc_log_info("Failure in cancelling timer for controller %s",
                     controller_name.c_str());
      }
      AuditRequest audit_req;
      UncRespCode merge_auditdb =
          audit_req.MergeAuditDbToRunning(&db_conn, reinterpret_cast<char *>
      (key_ctr.controller_name));
      if (merge_auditdb != UNC_RC_SUCCESS) {
        pfc_log_info("Merge of audit and running db failed");
      }
    } else {
      pfc_log_info("End of events received non-audit controller %s",
                   controller_name.c_str());
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    is_events_done = PFC_TRUE;
  }
  status = NotifyController.HandleDriverEvents(
      &db_conn, reinterpret_cast<void*>(&key_ctr),
      operation,
      data_type,
      reinterpret_cast<void*>(&old_val_ctr),
      reinterpret_cast<void*>(&new_val_ctr),
      is_events_done);
  pfc_log_info(
      "Return status of controller update HandleDriverEvents: %d",
      status);
  return status;
}

/***ProcessDomainEvents
 * @Description:This function processes events recieved for domain
 * @param[in]:
 * sess-ClientSession object used to retrieve reposne arguments 
 * data_type-type of database,UNC_DT_*
 * operation-type of operation UNC_OP_CREATE/UPDATE/DELETE
 * @return   :Success or associated error code
 **/
UncRespCode NotificationRequest:: ProcessDomainEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  // Check for MergeImportRunning Lock
  ScopedReadWriteLock eventDoneLock(
      PhysicalLayer::get_events_done_lock_(), PFC_FALSE);  // read lock
  UncRespCode db_ret = UNC_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteSb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  }
  UncRespCode status = UNC_RC_SUCCESS;
  /*process domain add, domain update and domain delete events*/
  key_ctr_domain_t key_ctr_domain;
  memset(&key_ctr_domain, '\0', sizeof(key_ctr_domain_t));
  int read_err = sess->getResponse((uint32_t)5, key_ctr_domain);
  if (read_err != 0) {
    pfc_log_error("Key not received for controller");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(key_ctr_domain).c_str());
  string controller_name = reinterpret_cast<char *>
  (key_ctr_domain.ctr_key.controller_name);
  GetNotificationDT(&db_conn, controller_name, data_type);
  val_ctr_domain_st new_val_ctr_domain_t;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_ctr_domain_t);
    if (read_err != 0) {
      pfc_log_error("New value not received for ctr domain");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("NEWVAL: %s",
                 IpctUtil::get_string(new_val_ctr_domain_t).c_str());
  }
  status = InvokeKtDriverEvent(
      &db_conn, operation, data_type,
      reinterpret_cast<void*>(&key_ctr_domain),
      reinterpret_cast<void*>(&new_val_ctr_domain_t),
      NULL, UNC_KT_CTR_DOMAIN);
  return status;
}

/***ProcessLogicalPortEvents
 * @Description:This function processes events recieved for LogicalPort
 * @param[in]:
 * sess-ClientSession object,used to retrieve the response arguments
 * data_type-type of database,UNC_DT_*
 * operation-type of operation UNC_OP_CREATE/UPDATE/DELETE
 * @return   :Success or associated error code
 **/
UncRespCode NotificationRequest:: ProcessLogicalPortEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  // Check for MergeImportRunning Lock
  ScopedReadWriteLock eventDoneLock(
      PhysicalLayer::get_events_done_lock_(), PFC_FALSE);  // read lock
  UncRespCode db_ret = UNC_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteSb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  }
  UncRespCode status = UNC_RC_SUCCESS;
  key_logical_port_t key_logical_port;
  memset(&key_logical_port, '\0', sizeof(key_logical_port_t));
  int read_err = sess->getResponse((uint32_t)5, key_logical_port);
  if (read_err != 0) {
    pfc_log_error("Key not received for logical port");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(key_logical_port).c_str());
  string controller_name = reinterpret_cast<char *>
  (key_logical_port.domain_key.ctr_key.controller_name);
  GetNotificationDT(&db_conn, controller_name, data_type);
  val_logical_port_st new_val_logical_port_t;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_logical_port_t);
    if (read_err != 0) {
      pfc_log_error("New value not received for logical port");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("NEWVAL: %s",
                 IpctUtil::get_string(new_val_logical_port_t).c_str());
  }
  status = InvokeKtDriverEvent(
      &db_conn, operation, data_type,
      reinterpret_cast<void*>(&key_logical_port),
      reinterpret_cast<void*>(&new_val_logical_port_t),
      NULL, UNC_KT_LOGICAL_PORT);
  return status;
}

/***ProcessLogicalMemeberPortEvents
 * @Description:This function processes events recieved for 
 * LogicalMemeberPort
 * @param[in]:
 * sess-ClientSession object,used to retrieve the response arguments
 * data_type-type of database,UNC_DT_*
 * operation-type of operation UNC_OP_CREATE/UPDATE/DELETE
 * @return   :Success or associated error code
 **/
UncRespCode NotificationRequest:: ProcessLogicalMemeberPortEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  // Check for MergeImportRunning Lock
  ScopedReadWriteLock eventDoneLock(
      PhysicalLayer::get_events_done_lock_(), PFC_FALSE);  // read lock
  UncRespCode db_ret = UNC_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteSb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  }
  UncRespCode status = UNC_RC_SUCCESS;
  key_logical_member_port_t logical_member_port_key;
  memset(&logical_member_port_key, '\0', sizeof(key_logical_member_port_t));
  int read_err = sess->getResponse((uint32_t)5, logical_member_port_key);
  if (read_err != 0) {
    pfc_log_error("Key not received for logical port");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(logical_member_port_key).c_str());
  string controller_name =
      reinterpret_cast<char *>
  (logical_member_port_key.logical_port_key.
      domain_key.ctr_key.controller_name);
  GetNotificationDT(&db_conn, controller_name, data_type);
  Kt_State_Base *NotifyLogicalMemberPort = new Kt_LogicalMemberPort();
  if (NotifyLogicalMemberPort == NULL) {
    pfc_log_error("Memory not allocated for NotifyLogicalMemberPort_\n");
    return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  status = NotifyLogicalMemberPort->HandleDriverEvents(
      &db_conn, reinterpret_cast<void*>(&logical_member_port_key),
      operation,
      data_type, UNC_KT_LOGICAL_MEMBER_PORT, NULL, NULL);
  pfc_log_info(
      "Return status of logical member port HandleDriverEvents: %d", status);
  delete NotifyLogicalMemberPort;
  return status;
}
