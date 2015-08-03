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
 * @file     	itc_notification_request.cc
 */

#include "itc_notification_request.hh"
#include "itc_kt_port.hh"
#include "itc_kt_port_neighbor.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_link.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_boundary.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_logical_member_port.hh"
#include "ipct_util.hh"


using unc::uppl::NotificationRequest;
map<string, vector<struct unc::uppl::alarm_buffer*> >
                        NotificationRequest::map_alarm_buff;

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
  } else if (event_type == UNC_UPPL_ALARMS) {
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
    case UNC_KT_PORT_NEIGHBOR: {
      ObjStateNotify = new Kt_Port_Neighbor();
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
  pfc_log_trace("Inside ProcessNotificationEvents of NotificationRequest");
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
    case UNC_KT_PORT_NEIGHBOR:
    {
      status = ProcessPortNeighborEvents(&sess,
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
  data_type = (uint32_t)UNC_DT_STATE;
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
  pfc_log_trace("Inside ProcessAlarmEvents of NotificationRequest");
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
      EventAlarmDetail event_detail(TQ_ALARM);
      event_detail.alarm_type = alarm_header.alarm_type;
      event_detail.operation = alarm_header.operation;
      event_detail.data_type = data_type;
      event_detail.key_type = alarm_header.key_type;
      //  allocating memory for key structure - key is mandatory param
      key_ctr_t *key = (key_ctr_t*)malloc(sizeof(key_ctr_t));
      if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      memcpy(key, &key_ctr, sizeof(key_ctr_t));
      event_detail.key_struct = reinterpret_cast<void *>(key);
      event_detail.key_size = sizeof(key_ctr_t);
      //  allocating memory for new val structure
      val_phys_path_fault_alarm_t *n_val_ctr_alarm_struct =
        (val_phys_path_fault_alarm_t*) malloc
                        (sizeof(val_phys_path_fault_alarm_t));
      if (n_val_ctr_alarm_struct == NULL)
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      memcpy(n_val_ctr_alarm_struct, &val_ctr_alarm_struct,
          sizeof(val_phys_path_fault_alarm_t));
      event_detail.new_val_struct =
                    reinterpret_cast<void*>(n_val_ctr_alarm_struct);
      event_detail.val_size = sizeof(val_phys_path_fault_alarm_t);
      event_detail.old_val_struct = NULL;
      //  get taskq for this controller, and dipatch a new task into taskqueue.
      PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
      if (taskq_util->DispatchNotificationEvent(
                      event_detail, controller_name) != 0) {
        pfc_log_error("Dispatch event failed , alarm type %d",
                                     alarm_header.alarm_type);
        return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
      }
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
    if (alarm_header.alarm_type ==  UNC_COREDOMAIN_SPLIT ||
        alarm_header.alarm_type ==  UNC_DOMAIN_SPLIT) {
      string controller_name = reinterpret_cast<char *>
      (key_ctr_domain.ctr_key.controller_name);
      EventAlarmDetail event_detail(TQ_ALARM);
      event_detail.alarm_type = alarm_header.alarm_type;
      event_detail.operation = alarm_header.operation;
      event_detail.data_type = data_type;
      event_detail.key_type = alarm_header.key_type;
      //  allocating memory for key structure - key is mandatory param
      key_ctr_domain_t *key =
        (key_ctr_domain_t*)malloc(sizeof(key_ctr_domain_t));
      if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      memcpy(key, &key_ctr_domain, sizeof(key_ctr_domain_t));
      event_detail.key_struct = reinterpret_cast<void *>(key);
      event_detail.key_size = sizeof(key_ctr_domain_t);
      event_detail.new_val_struct = NULL;
      event_detail.old_val_struct = NULL;
      event_detail.val_size = 0;
      //  get taskq for this controller, and dipatch a new task into taskqueue.
      PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
      if (taskq_util->DispatchNotificationEvent(
                      event_detail, controller_name) != 0) {
        pfc_log_error("Dispatch event failed , alarm type %d",
                                        alarm_header.alarm_type);
        return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
      }
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
      EventAlarmDetail event_detail(TQ_ALARM);
      event_detail.alarm_type = alarm_header.alarm_type;
      event_detail.operation = alarm_header.operation;
      event_detail.data_type = data_type;
      event_detail.key_type = alarm_header.key_type;
      //  allocating memory for key structure - key is mandatory param
      key_logical_port_t *key =
        (key_logical_port_t*)malloc(sizeof(key_logical_port_t));
      if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      memcpy(key, &key_logicalport, sizeof(key_logical_port_t));
      event_detail.key_struct = reinterpret_cast<void *>(key);
      event_detail.key_size = sizeof(key_logical_port_t);
      event_detail.new_val_struct = NULL;
      event_detail.old_val_struct = NULL;
      event_detail.val_size = 0;
      //  get taskq for this controller, and dipatch a new task into taskqueue.
      PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
      if (taskq_util->DispatchNotificationEvent(
                      event_detail, controller_name) != 0) {
        pfc_log_error("Dispatch event failed , alarm type %d",
                                      alarm_header.alarm_type);
        return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
      }
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
      EventAlarmDetail event_detail(TQ_ALARM);
      event_detail.alarm_type = alarm_header.alarm_type;
      event_detail.operation = alarm_header.operation;
      event_detail.data_type = data_type;
      event_detail.key_type = alarm_header.key_type;
      //  allocating memory for key structure - key is mandatory param
      key_port_t *key = (key_port_t*)malloc(sizeof(key_port_t));
      if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      memcpy(key, &port_key, sizeof(key_port_t));
      event_detail.key_struct = reinterpret_cast<void *>(key);
      event_detail.key_size = sizeof(key_port_t);
      event_detail.new_val_struct = NULL;
      event_detail.old_val_struct = NULL;
      event_detail.val_size = 0;
      //  get taskq for this controller, and dipatch a new task into taskqueue.
      PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
      if (taskq_util->DispatchNotificationEvent(
                      event_detail, controller_name) != 0) {
        pfc_log_error("Dispatch event failed , alarm type %d",
                                    alarm_header.alarm_type);
        return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
      }
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
        (alarm_header.alarm_type ==  UNC_OFS_LACK_FEATURES) ||
        (alarm_header.alarm_type ==  UNC_OFS_DISABLED)) {
      string controller_name = reinterpret_cast<char *>
      (switch_key.ctr_key.controller_name);
      EventAlarmDetail event_detail(TQ_ALARM);
      event_detail.alarm_type = alarm_header.alarm_type;
      event_detail.operation = alarm_header.operation;
      event_detail.data_type = data_type;
      event_detail.key_type = alarm_header.key_type;
      //  allocating memory for key structure - key is mandatory param
      key_switch_t *key = (key_switch_t*)malloc(sizeof(key_switch_t));
      if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      memcpy(key, &switch_key, sizeof(key_switch_t));
      event_detail.key_struct = reinterpret_cast<void *>(key);
      event_detail.key_size = sizeof(key_switch_t);
      event_detail.new_val_struct = NULL;
      event_detail.old_val_struct = NULL;
      event_detail.val_size = 0;
      //  get taskq for this controller, and dipatch a new task into taskqueue.
      PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
      if (taskq_util->DispatchNotificationEvent(
                      event_detail, controller_name) != 0) {
        pfc_log_error("Dispatch event failed , alarm type %d",
                                       alarm_header.alarm_type);
        return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
      }
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
  EventAlarmDetail event_detail(TQ_EVENT);
  event_detail.operation = operation;
  event_detail.data_type = data_type;
  event_detail.key_type = UNC_KT_PORT;
  //  allocating memory for key structure - key is mandatory param
  key_port_t* key = (key_port_t*)malloc(sizeof(key_port_t));
  if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  memcpy(key, &port_key, sizeof(key_port_t));
  event_detail.key_struct = reinterpret_cast<void *>(key);
  event_detail.key_size = sizeof(key_port_t);
  //  allocating memory for new val structure
  val_port_st* n_val_struct = NULL;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    n_val_struct = (val_port_st*)malloc(sizeof(val_port_st));
    if (n_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(n_val_struct, &new_val_port, sizeof(val_port_st));
  }
  event_detail.new_val_struct = reinterpret_cast<void*>(n_val_struct);
  event_detail.val_size = sizeof(val_port_st);
  //  allocating memory for old val structure
  val_port_st* o_val_struct = NULL;
  if (operation == UNC_OP_UPDATE) {
    o_val_struct = (val_port_st*)malloc(sizeof(val_port_st));
    if (o_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(o_val_struct, &old_val_port, sizeof(val_port_st));
  }
  event_detail.old_val_struct = reinterpret_cast<void*>(o_val_struct);
  //  get taskq for this controller, and dipatch a new task into taskqueue.
  PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
  if (taskq_util->DispatchNotificationEvent(
                  event_detail, controller_name) != 0) {
    pfc_log_error("Dispatch event failed ");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  return status;
}

/**ProcessPortNeighborEvents
 * @Description:This function processes events recieved for port
 * @param[in]:
 * sess-ClientSession object used to retrieve the response arguments
 * data_type-type of database,UNC_DT_*
 * operation-type of operation UNC_OP_CREATE/UPDATE/DELETE
 * @return   :Success or associated error code
 **/
UncRespCode NotificationRequest::ProcessPortNeighborEvents(
    ClientSession *sess,
    uint32_t data_type,
    uint32_t operation) {
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
  val_port_st_neighbor old_val_port, new_val_port;
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
  EventAlarmDetail event_detail(TQ_EVENT);
  event_detail.operation = operation;
  event_detail.data_type = data_type;
  event_detail.key_type = UNC_KT_PORT_NEIGHBOR;
  //  allocating memory for key structure - key is mandatory param
  key_port_t* key = (key_port_t*)malloc(sizeof(key_port_t));
  if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  memcpy(key, &port_key, sizeof(key_port_t));
  event_detail.key_struct = reinterpret_cast<void *>(key);
  event_detail.key_size = sizeof(key_port_t);
  //  allocating memory for new val structure
  val_port_st_neighbor_t* n_val_struct = NULL;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    n_val_struct = (val_port_st_neighbor*)malloc(sizeof(val_port_st_neighbor));
    if (n_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(n_val_struct, &new_val_port, sizeof(val_port_st_neighbor));
  }
  event_detail.new_val_struct = reinterpret_cast<void*>(n_val_struct);
  event_detail.val_size = sizeof(val_port_st_neighbor);
  //  allocating memory for old val structure
  val_port_st_neighbor_t* o_val_struct = NULL;
  if (operation == UNC_OP_UPDATE) {
    o_val_struct = (val_port_st_neighbor*)malloc(sizeof(val_port_st_neighbor));
    if (o_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(o_val_struct, &old_val_port, sizeof(val_port_st_neighbor));
  }
  event_detail.old_val_struct = reinterpret_cast<void*>(o_val_struct);
  //  get taskq for this controller, and dipatch a new task into taskqueue.
  PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
  if (taskq_util->DispatchNotificationEvent(
                  event_detail, controller_name) != 0) {
    pfc_log_error("Dispatch event failed ");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
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
  EventAlarmDetail event_detail(TQ_EVENT);
  event_detail.operation = operation;
  event_detail.data_type = data_type;
  event_detail.key_type = UNC_KT_SWITCH;
  //  allocating memory for key structure - key is mandatory param
  key_switch_t* key = (key_switch_t*)malloc(sizeof(key_switch_t));
  if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  memcpy(key, &switch_key, sizeof(key_switch_t));
  event_detail.key_struct = reinterpret_cast<void *>(key);
  event_detail.key_size = sizeof(key_switch_t);
  //  allocating memory for new val structure
  val_switch_st* n_val_struct = NULL;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    n_val_struct = (val_switch_st*)malloc(sizeof(val_switch_st));
    if (n_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(n_val_struct, &new_val_switch, sizeof(val_switch_st));
  }
  event_detail.new_val_struct = reinterpret_cast<void*>(n_val_struct);
  event_detail.val_size = sizeof(val_switch_st);
  //  allocating memory for old val structure
  val_switch_st* o_val_struct = NULL;
  if (operation == UNC_OP_UPDATE) {
    o_val_struct = (val_switch_st*)malloc(sizeof(val_switch_st));
    if (o_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(o_val_struct, &old_val_switch, sizeof(val_switch_st));
  }
  event_detail.old_val_struct = reinterpret_cast<void*>(o_val_struct);
  //  get taskq for this controller, and dipatch a new task into taskqueue.
  PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
  if (taskq_util->DispatchNotificationEvent(
                  event_detail, controller_name) != 0) {
    pfc_log_error("Dispatch event failed ");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
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
  EventAlarmDetail event_detail(TQ_EVENT);
  event_detail.operation = operation;
  event_detail.data_type = data_type;
  event_detail.key_type = UNC_KT_LINK;
  //  allocating memory for key structure - key is mandatory param
  key_link_t* key = (key_link_t*)malloc(sizeof(key_link_t));
  if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  memcpy(key, &key_link, sizeof(key_link_t));
  event_detail.key_struct = reinterpret_cast<void *>(key);
  event_detail.key_size = sizeof(key_link_t);
  //  allocating memory for new val structure
  val_link_st* n_val_struct = NULL;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    n_val_struct = (val_link_st*)malloc(sizeof(val_link_st));
    if (n_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(n_val_struct, &new_val_link, sizeof(val_link_st));
  }
  event_detail.new_val_struct = reinterpret_cast<void*>(n_val_struct);
  event_detail.val_size = sizeof(val_link_st);
  //  allocating memory for old val structure
  val_link_st* o_val_struct = NULL;
  if (operation == UNC_OP_UPDATE) {
    o_val_struct = (val_link_st*)malloc(sizeof(val_link_st));
    if (o_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(o_val_struct, &old_val_link, sizeof(val_link_st));
  }
  event_detail.old_val_struct = reinterpret_cast<void*>(o_val_struct);
  //  get taskq for this controller, and dipatch a new task into taskqueue.
  PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
  if (taskq_util->DispatchNotificationEvent(
                  event_detail, controller_name) != 0) {
    pfc_log_error("Dispatch event failed ");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
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

  EventAlarmDetail event_detail(TQ_EVENT);
  event_detail.operation = operation;
  event_detail.data_type = data_type;
  event_detail.key_type = UNC_KT_CONTROLLER;
  //  allocating memory for key structure - key is mandatory param
  key_ctr_t* key = (key_ctr_t*)malloc(sizeof(key_ctr_t));
  if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  memcpy(key, &key_ctr, sizeof(key_ctr_t));
  event_detail.key_struct = reinterpret_cast<void *>(key);
  event_detail.key_size = sizeof(key_ctr_t);
  //  allocating memory for new val structure
  val_ctr_st* n_val_struct = NULL;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    n_val_struct = (val_ctr_st*)malloc(sizeof(val_ctr_st));
    if (n_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(n_val_struct, &new_val_ctr, sizeof(val_ctr_st));
  }
  event_detail.new_val_struct = reinterpret_cast<void*>(n_val_struct);
  event_detail.val_size = sizeof(val_ctr_st);
  //  allocating memory for old val structure
  val_ctr_st* o_val_struct = NULL;
  if (operation == UNC_OP_UPDATE) {
    o_val_struct = (val_ctr_st*)malloc(sizeof(val_ctr_st));
    if (o_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(o_val_struct, &old_val_ctr, sizeof(val_ctr_st));
  }
  event_detail.old_val_struct = reinterpret_cast<void*>(o_val_struct);
  //  get taskq for this controller, and dipatch a new task into taskqueue.
  PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
  string controller_name = reinterpret_cast<char *>
    (key_ctr.controller_name);
  if (taskq_util->DispatchNotificationEvent(
                  event_detail, controller_name) != 0) {
    pfc_log_error("Dispatch event failed ");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
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
  UncRespCode status = UNC_RC_SUCCESS;
  /*process domain add, domain update and domain delete events*/
  key_ctr_domain_t key_ctr_domain;
  memset(&key_ctr_domain, '\0', sizeof(key_ctr_domain_t));
  int read_err = sess->getResponse((uint32_t)5, key_ctr_domain);
  if (read_err != 0) {
    pfc_log_error("Key not received for ctrdomain");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("%s", IpctUtil::get_string(key_ctr_domain).c_str());
  string controller_name = reinterpret_cast<char *>
  (key_ctr_domain.ctr_key.controller_name);
  val_ctr_domain_st new_val_ctr_domain_t;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_ctr_domain_t);
    if (read_err != 0) {
      pfc_log_error("New value not received for ctr domain");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("NEWVAL: %s",
                 IpctUtil::get_string(new_val_ctr_domain_t).c_str());
    new_val_ctr_domain_t.oper_status = UPPL_CTR_DOMAIN_OPER_UP;
    new_val_ctr_domain_t.valid[kIdxDomainStOperStatus] = UNC_VF_VALID;
  }
  EventAlarmDetail event_detail(TQ_EVENT);
  event_detail.operation = operation;
  event_detail.data_type = data_type;
  event_detail.key_type = UNC_KT_CTR_DOMAIN;
  //  allocating memory for key structure - key is mandatory param
  key_ctr_domain_t* key = (key_ctr_domain_t*)malloc(sizeof(key_ctr_domain_t));
  if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  memcpy(key, &key_ctr_domain, sizeof(key_ctr_domain_t));
  event_detail.key_struct = reinterpret_cast<void *>(key);
  event_detail.key_size = sizeof(key_ctr_domain_t);
  //  allocating memory for new val structure
  val_ctr_domain_st* n_val_struct = NULL;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    n_val_struct = (val_ctr_domain_st*)malloc(sizeof(val_ctr_domain_st));
    if (n_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(n_val_struct, &new_val_ctr_domain_t, sizeof(val_ctr_domain_st));
  }
  event_detail.new_val_struct = reinterpret_cast<void*>(n_val_struct);
  event_detail.val_size = sizeof(val_ctr_domain_st);
  event_detail.old_val_struct = NULL;
  //  get taskq for this controller, and dipatch a new task into taskqueue.
  PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
  if (taskq_util->DispatchNotificationEvent(
                  event_detail, controller_name) != 0) {
    pfc_log_error("Dispatch event failed ");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
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
  val_logical_port_st new_val_logical_port_t;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    read_err = sess->getResponse((uint32_t)6, new_val_logical_port_t);
    if (read_err != 0) {
      pfc_log_error("New value not received for logical port");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_info("NEWVAL: %s",
                 IpctUtil::get_string(new_val_logical_port_t).c_str());
    // When there is no member, the LP oper status will
    // be down for SD, TP, PG and MG type
    if (operation == UNC_OP_CREATE &&
       (new_val_logical_port_t.logical_port.port_type == UPPL_LP_SUBDOMAIN ||
       new_val_logical_port_t.logical_port.port_type == UPPL_LP_MAPPING_GROUP ||
       new_val_logical_port_t.logical_port.port_type == UPPL_LP_TRUNK_PORT ||
       new_val_logical_port_t.logical_port.port_type == UPPL_LP_PORT_GROUP)) {
      new_val_logical_port_t.oper_status = UPPL_LOGICAL_PORT_OPER_DOWN;
      new_val_logical_port_t.valid[kIdxLogicalPortStOperStatus] = UNC_VF_VALID;
    }
  }
  EventAlarmDetail event_detail(TQ_EVENT);
  event_detail.operation = operation;
  event_detail.data_type = data_type;
  event_detail.key_type = UNC_KT_LOGICAL_PORT;
  //  allocating memory for key structure - key is mandatory param
  key_logical_port_t* key =
             (key_logical_port_t*)malloc(sizeof(key_logical_port_t));
  if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  memcpy(key, &key_logical_port, sizeof(key_logical_port_t));
  event_detail.key_struct = reinterpret_cast<void *>(key);
  event_detail.key_size = sizeof(key_logical_port_t);
  //  allocating memory for new val structure
  val_logical_port_st* n_val_struct = NULL;
  if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
    n_val_struct = (val_logical_port_st*)malloc(sizeof(val_logical_port_st));
    if (n_val_struct == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
    memcpy(n_val_struct, &new_val_logical_port_t, sizeof(val_logical_port_st));
  }
  event_detail.new_val_struct = reinterpret_cast<void*>(n_val_struct);
  event_detail.val_size = sizeof(val_logical_port_st);
  event_detail.old_val_struct = NULL;
  //  get taskq for this controller, and dipatch a new task into taskqueue.
  PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
  if (taskq_util->DispatchNotificationEvent(
                  event_detail, controller_name) != 0) {
    pfc_log_error("Dispatch event failed ");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
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
  EventAlarmDetail event_detail(TQ_EVENT);
  event_detail.operation = operation;
  event_detail.data_type = data_type;
  event_detail.key_type = UNC_KT_LOGICAL_MEMBER_PORT;
  //  allocating memory for key structure - key is mandatory param
  key_logical_member_port_t* key = (key_logical_member_port_t*)malloc
                                   (sizeof(key_logical_member_port_t));
  if (key == NULL) return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  memcpy(key, &logical_member_port_key, sizeof(key_logical_member_port_t));
  event_detail.key_struct = reinterpret_cast<void *>(key);
  event_detail.key_size = sizeof(key_logical_member_port_t);
  //  allocating memory for new val structure
  event_detail.new_val_struct = NULL;
  event_detail.old_val_struct = NULL;
  event_detail.val_size = 0;
  //  get taskq for this controller, and dipatch a new task into taskqueue.
  PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
  if (taskq_util->DispatchNotificationEvent(
                  event_detail, controller_name) != 0) {
    pfc_log_error("Dispatch event failed ");
    return UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  return status;
}

/**
 * * @Description : This function keeps the alarm details in the vector and store it in map
 * * @param[in] : alarm type, operation type, key struct and val struct
 * * @return    : void
 * */

void NotificationRequest::FillAlarmDetails(uint32_t alarm_type,
                                 uint32_t oper_type,
                                 void* key_struct,
                                 void* val_struct) {
  key_ctr_t *obj_key_ctr= new key_ctr_t;
  memcpy(obj_key_ctr, reinterpret_cast<key_ctr_t*>(key_struct),
        sizeof(key_ctr_t));
  string controller_name = reinterpret_cast<const char*>
                 (obj_key_ctr->controller_name);
  val_phys_path_fault_alarm_t *obj_val = new val_phys_path_fault_alarm_t;
  memcpy(obj_val, reinterpret_cast<val_phys_path_fault_alarm_t*>(val_struct),
      sizeof(val_phys_path_fault_alarm_t));
  pfc_log_debug("FillAlarmdetails:Controller name is %s",
                               obj_key_ctr->controller_name);
  pfc_log_debug("Switch name is %s", obj_val->ingress_ofs_dpid);
  alarm_buffer *buff_alarm = new alarm_buffer(alarm_type, oper_type);
  buff_alarm->key_struct = reinterpret_cast<void*>(obj_key_ctr);
  buff_alarm->val_struct = reinterpret_cast<void*>(obj_val);
  vector <struct unc::uppl::alarm_buffer*> vec_alarm_buff;
  // Checking if any alarm for same controller exists
  map<string, vector<struct unc::uppl::alarm_buffer*> > ::iterator it =
                     map_alarm_buff.find(controller_name);
  if (it != map_alarm_buff.end()) {
    pfc_log_debug("Entry found for previous Alarm for same controller");
    vec_alarm_buff = it->second;
  }
  vec_alarm_buff.push_back(buff_alarm);
  map_alarm_buff[controller_name] = vec_alarm_buff;
}

