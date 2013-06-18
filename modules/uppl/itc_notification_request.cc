/*
 * Copyright (c) 2012-2013 NEC Corporation
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

/**
 * @Description:NotificationRequest constructor
 * * @param[in]:void
 * * @return   :void
 **/
NotificationRequest::NotificationRequest() {
}

/**
 * @Description:NotificationRequest destructor
 * * @param[in]:void
 * * @return   :void
 **/
NotificationRequest::~NotificationRequest() {
}

/**
 * @Description:This function invoke ProcessNotificationEvents() and
 *  ProcessAlarmEvents
 * * @param[in]:const IpcEvent &event
 * * @return   :pfc_bool_t
 **/
pfc_bool_t NotificationRequest::ProcessEvent(const IpcEvent &event) {
  pfc_log_info("Inside ProcessEvent of NotificationRequest");
  pfc_ipcevtype_t event_type(event.getType());
  if (event_type == UNC_PHYSICAL_EVENTS ||
      event_type == UNC_CTLR_STATE_EVENTS) {
    UpplReturnCode err = ProcessNotificationEvents(event);
    if (err != UPPL_RC_SUCCESS) {
      pfc_log_error("ProcessNotificationEvents failed error no: %d", err);
      return PFC_FALSE;
    }
  } else if (event_type == UNC_ALARMS) {
    UpplReturnCode err = ProcessAlarmEvents(event);
    if (err != UPPL_RC_SUCCESS) {
      pfc_log_error("ProcessAlarmEvents failed error no: %d", err);
      return PFC_FALSE;
    }
  } else {
    pfc_log_error("Invalid event type error no: %d",
                  UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED);
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

UpplReturnCode NotificationRequest::InvokeKtDriverEvent(
    uint32_t operation,
    uint32_t data_type,
    void *key_struct,
    void *new_val_struct,
    void *old_val_struct,
    uint32_t key_type) {
  Kt_State_Base *ObjStateNotify = NULL;
  UpplReturnCode status = UPPL_RC_SUCCESS;
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
      return UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED;
    }
  }
  switch (operation) {
    case UNC_OP_CREATE: {
      status = ObjStateNotify->HandleDriverEvents(key_struct, operation,
                                                  data_type, key_type, NULL,
                                                  new_val_struct);
      pfc_log_info(
          "Return status of KT create HandleDriverEvents: %d", status);
      delete ObjStateNotify;
      break;
    }
    case UNC_OP_UPDATE: {
      status = ObjStateNotify->HandleDriverEvents(key_struct, operation,
                                                  data_type, key_type,
                                                  old_val_struct,
                                                  new_val_struct);
      pfc_log_info(
          "Return status of update HandleDriverEvents: %d", status);
      delete ObjStateNotify;
      break;
    }
    case UNC_OP_DELETE: {
      status = ObjStateNotify->HandleDriverEvents(key_struct, operation,
                                                  data_type, key_type,
                                                  NULL, NULL);
      pfc_log_info(
          "Return status of delete HandleDriverEvents: %d", status);
      delete ObjStateNotify;
      break;
    }
    default:
    {
      pfc_log_error("Invalid operation type\n");
      delete ObjStateNotify;
      return UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
    }
  }
  return status;
}

/**
 * @Description:This function process Notification events
 * * @param[in]:const IpcEvent &event
 * * @return   :pfc_bool_t
 **/
UpplReturnCode NotificationRequest::
ProcessNotificationEvents(const IpcEvent &event) {
  pfc_log_info("Inside ProcessNotificationEvents of NotificationRequest");
  UpplReturnCode status = UPPL_RC_SUCCESS;
  ClientSession sess(event.getSession());

  /*validate valid response count for Notification event structure*/
  uint32_t resp_count = sess.getResponseCount();
  if (resp_count < (uint32_t)6) {
    pfc_log_error("Invalid event structure - Resp Count %d", resp_count);
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  driver_event_header event_header;
  uint32_t header_parse = PhyUtil::sessGetDriverEventHeader(sess, event_header);
  if (header_parse != 0) {
    pfc_log_error("Unable to parse event header successfully");
    return UPPL_RC_ERR_BAD_REQUEST;
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
      if (status == UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_SWITCH:
    {
      status = ProcessSwitchEvents(&sess,
                                   data_type,
                                   operation);
      if (status == UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_LINK:
    {
      status = ProcessLinkEvents(&sess,
                                 data_type,
                                 operation);
      if (status == UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_CONTROLLER:
    {
      status = ProcessControllerEvents(&sess,
                                       data_type,
                                       operation);
      if (status == UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_CTR_DOMAIN:
    {
      status = ProcessDomainEvents(&sess,
                                   data_type,
                                   operation);
      if (status == UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_LOGICAL_PORT:
    {
      status = ProcessLogicalPortEvents(&sess,
                                        data_type,
                                        operation);
      if (status == UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT:
    {
      status = ProcessLogicalMemeberPortEvents(&sess,
                                               data_type,
                                               operation);
      if (status == UPPL_RC_ERR_BAD_REQUEST) {
        return status;
      }
      break;
    }
    default:
    {
      pfc_log_error("Invalid key type\n");
      return UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED;
    }
  }
  return status;
}

/***
 * * @Description:This function checks whether audit in progress for the controller
 * and use IMPORT db to store the notifications received from driver
 * * * @param[in]:controller_name, datatype
 * * * @return   :void
 **/
void NotificationRequest::GetNotificationDT(string controller_name,
                                            uint32_t &data_type) {
  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
  if (PhyUtil::get_controller_type(
      controller_name,
      ctr_type,
      (unc_keytype_datatype_t)data_type) == UPPL_RC_SUCCESS) {
    pfc_log_debug("Received Controller Type %d", ctr_type);
  }
  NotificationManager *nfn_mgr = PhysicalLayer::get_instance()->
      get_ipc_connection_manager()->
      get_notification_manager(ctr_type);
  if (nfn_mgr != NULL) {
    pfc_bool_t is_controller_in_audit = PhysicalLayer::get_instance()->
        get_ipc_connection_manager()->
        IsControllerInAudit(controller_name);
    if (is_controller_in_audit == PFC_TRUE) {
      data_type = (uint32_t)UNC_DT_IMPORT;
    }
  }
  pfc_log_debug("GetNotificationDT data_type %d", data_type);
}

/***
 * * @Description:This function Process alarm events
 * * * @param[in]:const IpcEvent &event
 * * * @return   :pfc_bool_t
 **/
UpplReturnCode NotificationRequest::ProcessAlarmEvents(const IpcEvent &event) {
  pfc_log_info("Inside ProcessAlarmEvents of NotificationRequest");
  UpplReturnCode status = UPPL_RC_SUCCESS;
  ClientSession sess(event.getSession());

  /*validate valid response count for Notification event structure*/
  uint32_t resp_count = sess.getResponseCount();
  if (resp_count < (uint32_t)7) {
    pfc_log_error("Invalid alarm structure");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  /*Get key type, operation type and alarm type from IpcEvent*/
  driver_alarm_header alarm_header;
  uint32_t header_parse = PhyUtil::sessGetDriverAlarmHeader(sess, alarm_header);
  if (header_parse != 0) {
    pfc_log_error("Unable to parse alarms header successfully");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  uint32_t data_type = alarm_header.data_type;
  pfc_log_debug("Alarm Type %d", alarm_header.alarm_type);
  if (alarm_header.key_type == UNC_KT_CONTROLLER) {
    /*process controller related alarm*/
    key_ctr key_ctr;
    int read_err = sess.getResponse((uint32_t)6, key_ctr);
    if (read_err != 0) {
      pfc_log_error("Key not received for controller");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("%s", IpctUtil::get_string(key_ctr).c_str());
    if (alarm_header.alarm_type == UNC_PHYS_PATH_FAULT) {
      val_phys_path_fault_alarm_t val_ctr_alarm_struct;
      int read_err = sess.getResponse((uint32_t)7, val_ctr_alarm_struct);
      if (read_err != 0) {
        pfc_log_error("Value not received for controller");
        return UPPL_RC_ERR_BAD_REQUEST;
      }
      string controller_name = reinterpret_cast<char *>
      (key_ctr.controller_name);
      GetNotificationDT(controller_name, data_type);
      Kt_Controller NotifyController;
      status = NotifyController.HandleDriverAlarms(
          data_type, alarm_header.alarm_type, alarm_header.operation,
          reinterpret_cast<void*>(&key_ctr),
          reinterpret_cast<void*>(&val_ctr_alarm_struct));
      pfc_log_info(
          "Return status of controller HandleDriverAlarms: %d", status);
    } else {
      pfc_log_info("Invalid alarm type for UNC_KT_CONTROLLER: %d",
                   alarm_header.alarm_type);
      return UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
  } else  if (alarm_header.key_type ==  UNC_KT_CTR_DOMAIN) {
    /*process domain related alarm*/
    key_ctr_domain_t key_ctr_domain;
    int read_err = sess.getResponse((uint32_t)6, key_ctr_domain);
    if (read_err != 0) {
      pfc_log_error("Key not received for ctr domain");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("%s", IpctUtil::get_string(key_ctr_domain).c_str());
    Kt_Base *NotifyDomain = new Kt_Ctr_Domain();
    if (NotifyDomain == NULL) {
      pfc_log_error("Memory not allocated for Notifydomain");
      return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    }
    if (alarm_header.alarm_type ==  UNC_COREDOMAIN_SPLIT) {
      string controller_name = reinterpret_cast<char *>
      (key_ctr_domain.ctr_key.controller_name);
      GetNotificationDT(controller_name, data_type);
      status = NotifyDomain->HandleDriverAlarms(
          data_type, alarm_header.alarm_type, alarm_header.operation,
          reinterpret_cast<void*>(&key_ctr_domain), NULL);
      pfc_log_info(
          "Return status of domain HandleDriverAlarms: %d", status);
      delete NotifyDomain;
    } else {
      pfc_log_info("Invalid alarm type for UNC_KT_CTR_DOMAIN: %d",
                   alarm_header.alarm_type);
      delete NotifyDomain;
      return UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
  }  else  if (alarm_header.key_type ==  UNC_KT_LOGICAL_PORT) {
    /*process logical port related alarm*/
    key_logical_port_t key_logicalport;
    int read_err = sess.getResponse((uint32_t)6, key_logicalport);
    if (read_err != 0) {
      pfc_log_error("Key not received for logical port");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("%s", IpctUtil::get_string(key_logicalport).c_str());
    Kt_Base *NotifyLogicalPort = new Kt_LogicalPort();
    if (NotifyLogicalPort  == NULL) {
      pfc_log_error("Memory not allocated for NotifyLogicalPort");
      return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    }
    if (alarm_header.alarm_type == UNC_SUBDOMAIN_SPLIT) {
      string controller_name = reinterpret_cast<char *>
      (key_logicalport.domain_key.ctr_key.controller_name);
      GetNotificationDT(controller_name, data_type);
      status = NotifyLogicalPort->HandleDriverAlarms(
          data_type, alarm_header.alarm_type, alarm_header.operation,
          reinterpret_cast<void*>(&key_logicalport), NULL);
      pfc_log_info(
          "Return status of sub_domain HandleDriverAlarms: %d", status);
      delete NotifyLogicalPort;
    } else {
      pfc_log_info("Invalid alarm type for UNC_KT_LOGICAL_PORT: %d",
                   alarm_header.alarm_type);
      delete NotifyLogicalPort;
      return UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
  }  else  if (alarm_header.key_type ==  UNC_KT_PORT) {
    /*process port related alarm*/
    key_port_t port_key;
    int read_err = sess.getResponse((uint32_t)6, port_key);
    if (read_err != 0) {
      pfc_log_error("Key not received for port");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("%s", IpctUtil::get_string(port_key).c_str());
    Kt_Base *NotifyPort = new Kt_Port();
    if (NotifyPort  == NULL) {
      pfc_log_error("Memory not allocated for NotifyPort");
      return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    }
    if ((alarm_header.alarm_type ==  UNC_DEFAULT_FLOW) ||
        (alarm_header.alarm_type ==  UNC_PORT_DIRECTION) ||
        (alarm_header.alarm_type ==  UNC_PORT_CONGES)) {
      string controller_name = reinterpret_cast<char *>
      (port_key.sw_key.ctr_key.controller_name);
      GetNotificationDT(controller_name, data_type);
      status = NotifyPort->HandleDriverAlarms(
          data_type, alarm_header.alarm_type, alarm_header.operation,
          reinterpret_cast<void*>(&port_key), NULL);
      pfc_log_info(
          "Return status of port HandleDriverAlarms: %d", status);
      delete NotifyPort;
    } else {
      pfc_log_info("Invalid alarm type for UNC_KT_PORT: %d",
                   alarm_header.alarm_type);
      delete NotifyPort;
      return UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
  }  else  if (alarm_header.key_type ==  UNC_KT_SWITCH) {
    /*process controller related alarm*/
    key_switch_t switch_key;
    int read_err = sess.getResponse((uint32_t)6, switch_key);
    if (read_err != 0) {
      pfc_log_error("Key not received for switch");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("%s", IpctUtil::get_string(switch_key).c_str());
    Kt_Base *NotifySwitch = new Kt_Switch();
    if (NotifySwitch  == NULL) {
      pfc_log_error("Memory not allocated for NotifySwitch");
      return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    }
    if ((alarm_header.alarm_type ==  UNC_FLOW_ENT_FULL) ||
        (alarm_header.alarm_type ==  UNC_OFS_LACK_FEATURES)) {
      string controller_name = reinterpret_cast<char *>
      (switch_key.ctr_key.controller_name);
      GetNotificationDT(controller_name, data_type);
      status = NotifySwitch->HandleDriverAlarms(
          data_type, alarm_header.alarm_type, alarm_header.operation,
          reinterpret_cast<void*>(&switch_key), NULL);
      pfc_log_info(
          "Return status of switch HandleDriverAlarms: %d", status);
      delete NotifySwitch;
    } else {
      pfc_log_info("Invalid alarm type for UNC_KT_SWITCH: %d",
                   alarm_header.alarm_type);
      delete NotifySwitch;
      return UPPL_RC_ERR_NOTIFICATION_NOT_SUPPORTED;
    }
  } else {
    pfc_log_info("Invalid key type : %d", alarm_header.key_type);
  }
  return status;
}

UpplReturnCode NotificationRequest::ProcessPortEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  key_port_t port_key;
  int read_err = sess->getResponse((uint32_t)5, port_key);
  if (read_err != 0) {
    pfc_log_error("Key not received for port");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(port_key).c_str());
  string controller_name = reinterpret_cast<char *>
  (port_key.sw_key.ctr_key.controller_name);
  GetNotificationDT(controller_name, data_type);
  val_port_st old_val_port, new_val_port;
  // val
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_port);
    if (read_err != 0) {
      pfc_log_error("New value not received for port");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("%s", IpctUtil::get_string(new_val_port).c_str());
  }
  // old val
  if (operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)7, old_val_port);
    if (read_err != 0) {
      pfc_log_error("Old value not received for port");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("OLDVAL: %s",
                 IpctUtil::get_string(old_val_port).c_str());
  }
  // call driver event
  status = InvokeKtDriverEvent(operation, data_type,
                               reinterpret_cast<void*>(&port_key),
                               reinterpret_cast<void*>(&new_val_port),
                               reinterpret_cast<void*>(&old_val_port),
                               UNC_KT_PORT);
  return status;
}

UpplReturnCode NotificationRequest::ProcessSwitchEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  /*process switch add, switch update and switch delete events*/
  key_switch_t switch_key;
  int read_err = sess->getResponse((uint32_t)5, switch_key);
  if (read_err != 0) {
    pfc_log_error("Key not received for switch");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(switch_key).c_str());
  string controller_name = reinterpret_cast<char *>
  (switch_key.ctr_key.controller_name);
  GetNotificationDT(controller_name, data_type);
  val_switch_st old_val_switch, new_val_switch;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_switch);
    if (read_err != 0) {
      pfc_log_error("New value not received for switch");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("NEWVAL: %s",
                 IpctUtil::get_string(new_val_switch).c_str());
  }
  if (operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)7, old_val_switch);
    if (read_err != 0) {
      pfc_log_error("Old value not received for switch");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("OLDVAL: %s",
                 IpctUtil::get_string(old_val_switch).c_str());
  }
  status = InvokeKtDriverEvent(operation, data_type,
                               reinterpret_cast<void*>(&switch_key),
                               reinterpret_cast<void*>(&new_val_switch),
                               reinterpret_cast<void*>(&old_val_switch),
                               UNC_KT_SWITCH);
  return status;
}

UpplReturnCode NotificationRequest:: ProcessLinkEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  /*process link add, link update and link delete events*/
  key_link_t key_link;
  int read_err = sess->getResponse((uint32_t)5, key_link);
  if (read_err != 0) {
    pfc_log_error("Key not received for link");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(key_link).c_str());
  string controller_name = reinterpret_cast<char *>
  (key_link.ctr_key.controller_name);
  GetNotificationDT(controller_name, data_type);
  val_link_st old_val_link, new_val_link;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_link);
    if (read_err != 0) {
      pfc_log_error("New value not received for link");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("NEWVAL: %s",
                 IpctUtil::get_string(new_val_link).c_str());
  }
  if (operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)7, old_val_link);
    if (read_err != 0) {
      pfc_log_error("Old value not received for link");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("OLDVAL: %s",
                 IpctUtil::get_string(old_val_link).c_str());
  }
  status = InvokeKtDriverEvent(operation, data_type,
                               reinterpret_cast<void*>(&key_link),
                               reinterpret_cast<void*>(&new_val_link),
                               reinterpret_cast<void*>(&old_val_link),
                               UNC_KT_LINK);
  return status;
}

UpplReturnCode NotificationRequest:: ProcessControllerEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  /*process controller add, controller update and controller delete events*/
  key_ctr key_ctr;
  val_ctr_st old_val_ctr, new_val_ctr;
  int read_err = sess->getResponse((uint32_t)5, key_ctr);
  if (read_err != 0) {
    pfc_log_error("Key not received for controller");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(key_ctr).c_str());
  Kt_Controller NotifyController;
  read_err = sess->getResponse((uint32_t)6, new_val_ctr);
  if (read_err != 0) {
    pfc_log_error("New value not received for controller");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("NEWVAL: %s", IpctUtil::get_string(new_val_ctr).c_str());
  read_err = sess->getResponse((uint32_t)7, old_val_ctr);
  if (read_err != 0) {
    pfc_log_error("Old value not received for controller");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("OLDVAL: %s", IpctUtil::get_string(old_val_ctr).c_str());
  pfc_bool_t is_events_done = PFC_FALSE;
  uint8_t driver_oper_status = new_val_ctr.oper_status;
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
      uint32_t time_out_id = ipc_mgr->getTimeOutId(controller_name);
      ipc_mgr->notfn_timer_->cancel(time_out_id);
      AuditRequest audit_req;
      UpplReturnCode merge_auditdb =
          audit_req.MergeAuditDbToRunning(reinterpret_cast<char *>
      (key_ctr.controller_name));
      if (merge_auditdb != UPPL_RC_SUCCESS) {
        pfc_log_info("Merge of audit and running db failed");
      }
    } else {
      pfc_log_info("End of events received non-audit controller %s",
                   controller_name.c_str());
    }
    is_events_done = PFC_TRUE;
  }
  status = NotifyController.HandleDriverEvents(
      reinterpret_cast<void*>(&key_ctr),
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

UpplReturnCode NotificationRequest:: ProcessDomainEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  /*process domain add, domain update and domain delete events*/
  key_ctr_domain_t key_ctr_domain;
  int read_err = sess->getResponse((uint32_t)5, key_ctr_domain);
  if (read_err != 0) {
    pfc_log_error("Key not received for controller");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(key_ctr_domain).c_str());
  string controller_name = reinterpret_cast<char *>
  (key_ctr_domain.ctr_key.controller_name);
  GetNotificationDT(controller_name, data_type);
  val_ctr_domain_st new_val_ctr_domain_t;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_ctr_domain_t);
    if (read_err != 0) {
      pfc_log_error("New value not received for ctr domain");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("NEWVAL: %s",
                 IpctUtil::get_string(new_val_ctr_domain_t).c_str());
  }
  status = InvokeKtDriverEvent(
      operation, data_type,
      reinterpret_cast<void*>(&key_ctr_domain),
      reinterpret_cast<void*>(&new_val_ctr_domain_t),
      NULL, UNC_KT_CTR_DOMAIN);
  return status;
}

UpplReturnCode NotificationRequest:: ProcessLogicalPortEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  key_logical_port_t key_logical_port;
  int read_err = sess->getResponse((uint32_t)5, key_logical_port);
  if (read_err != 0) {
    pfc_log_error("Key not received for logical port");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(key_logical_port).c_str());
  string controller_name = reinterpret_cast<char *>
  (key_logical_port.domain_key.ctr_key.controller_name);
  GetNotificationDT(controller_name, data_type);
  val_logical_port_st new_val_logical_port_t;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_logical_port_t);
    if (read_err != 0) {
      pfc_log_error("New value not received for logical port");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("NEWVAL: %s",
                 IpctUtil::get_string(new_val_logical_port_t).c_str());
  }
  status = InvokeKtDriverEvent(
      operation, data_type,
      reinterpret_cast<void*>(&key_logical_port),
      reinterpret_cast<void*>(&new_val_logical_port_t),
      NULL, UNC_KT_LOGICAL_PORT);
  return status;
}

UpplReturnCode NotificationRequest:: ProcessLogicalMemeberPortEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  key_logical_member_port_t logical_member_port_key;
  int read_err = sess->getResponse((uint32_t)5, logical_member_port_key);
  if (read_err != 0) {
    pfc_log_error("Key not received for logical port");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(logical_member_port_key).c_str());
  string controller_name =
      reinterpret_cast<char *>
  (logical_member_port_key.logical_port_key.
      domain_key.ctr_key.controller_name);
  GetNotificationDT(controller_name, data_type);
  Kt_State_Base *NotifyLogicalMemberPort = new Kt_LogicalMemberPort();
  if (NotifyLogicalMemberPort == NULL) {
    pfc_log_error("Memory not allocated for NotifyLogicalMemberPort_\n");
    return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  status = NotifyLogicalMemberPort->HandleDriverEvents(
      reinterpret_cast<void*>(&logical_member_port_key),
      operation,
      data_type, UNC_KT_LOGICAL_MEMBER_PORT, NULL, NULL);
  pfc_log_info(
      "Return status of logical member port HandleDriverEvents: %d", status);
  delete NotifyLogicalMemberPort;
  return status;
}
