/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 *  @brief    Audit Request handling
 *  @file     itc_audit_request.cc
 */

#include <vector>
#include "physicallayer.hh"
#include "physical_itc.hh"
#include "itc_audit_request.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_logical_member_port.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_port.hh"
#include "itc_kt_port_neighbor.hh"
#include "itc_kt_link.hh"
#include "itc_kt_boundary.hh"
#include "ipc_client_configuration_handler.hh"
#include "tclib_module.hh"
#include "itc_notification_request.hh"

using unc::tclib::TcLibModule;
using unc::tclib::TcControllerResult;
map<string, commit_version> unc::uppl::AuditRequest::comm_ver_;
int16_t AuditRequest::ctr_oper_status_before_audit = INVALID_OPERSTATUS;
pfc_bool_t AuditRequest::IsControllerNotUp = false;
AuditType AuditRequest::audit_type_ = AUDIT_NORMAL;
namespace unc {
namespace uppl {

/**AuditRequest
 * @Description : This function initializes the member data
 * @param[in]   : None
 * @return      : None
 * */
AuditRequest::AuditRequest() {
}

/**~AuditRequest
 * @Description : This function releases the  memory allocated to
 *                pointer member data
 * @param[in]   : None
 * @return      : None
 * */
AuditRequest::~AuditRequest() {
}

/**StartAudit
 * @Description : This function performs the required pre condition validations
 *                such as checking of the existence of the controller and its
 *                operation status and set its value again.
 * @param[in]   : driver_id - Specifies one of the following controller type,
 *                PFC,VNP,Bypass    
 *                controller_id - controller name to which the audit to start
 * @return      : UNC_RC_SUCCESS if the Audit is success for the controller 
 *                or UNC_UPPL_RC_ERR_*
 * */

UncRespCode AuditRequest::StartAudit(OdbcmConnectionHandler *db_conn,
                                        unc_keytype_ctrtype_t driver_id,
                                        string controller_id,
                                        TcAuditType  audit_type,
                                        uint64_t commit_number,
                                        uint64_t commit_date,
                                        string commit_application) {
  pfc_log_info("Processing StartAudit commit_number: %" PFC_PFMT_u64
               ", commit_date: %" PFC_PFMT_u64
               ", commit_application : %s audit_type:%d",
            commit_number, commit_date, commit_application.c_str(), audit_type);
  Kt_Controller KtObj;
  uint8_t oper_status = 0;
  key_ctr_t obj_key_ctr;
  memset(obj_key_ctr.controller_name, '\0',
                    sizeof(obj_key_ctr.controller_name));
  memcpy(obj_key_ctr.controller_name, controller_id.c_str(),
         controller_id.length()+1);
  if (audit_type != TC_AUDIT_SIMPLIFIED &&
         audit_type != TC_AUDIT_REALNETWORK) {
    commit_version ver_commit;
    memset(&ver_commit, 0, sizeof(commit_version));
    ver_commit.commit_number = commit_number;
    ver_commit.commit_date = commit_date;
    memcpy(&ver_commit.commit_application, commit_application.c_str(),
        commit_application.length()+1);
    comm_ver_[controller_id] =  ver_commit;
  }
  /* Checks controller existence and its oper status */
  pfc_log_debug("Get controller oper Status");
  val_ctr_st_t obj_val_ctr_st;
  UncRespCode read_status = UNC_RC_SUCCESS;
  ReadWriteLock *rwlock = NULL;
  TcLibModule* tclib_ptr = static_cast<TcLibModule*>
        (TcLibModule::getInstance(TCLIB_MODULE_NAME));
  PhysicalLayer::ctr_oprn_mutex_.lock();
  PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, true);
  if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    tclib_ptr->TcLibWriteControllerInfo(controller_id.c_str(),
                                        UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE,
                                        0);
    PhysicalLayer::ctr_oprn_mutex_.unlock();
    return UNC_UPPL_RC_ERR_AUDIT_FAILURE;
  }
  read_status = KtObj.GetOperStatus(db_conn, UNC_DT_RUNNING,
                          reinterpret_cast<void *>(&obj_key_ctr), oper_status);
  if (read_status == UNC_RC_SUCCESS) {
    // Copy the operstatus in ctr_oper_status_before_audit variable
    AuditRequest::ctr_oper_status_before_audit = oper_status;
    pfc_log_debug("StartAudit::ctr_oper_status_before_audit is: %d",
                   AuditRequest::ctr_oper_status_before_audit);
    // if audit_type is TC_AUDIT_REALNETWORK,
    // fill audit_type_ with AUDIT_REALNETWORK
    if (audit_type == TC_AUDIT_REALNETWORK) {
      audit_type_ = AUDIT_REALNETWORK;
      pfc_log_debug("StartAudit::TC_AUDIT_REALNETWORK");
    }
    // Checking TC_AUDIT_REALNETWORK pre-condition
    if (audit_type == TC_AUDIT_REALNETWORK &&
            oper_status != UPPL_CONTROLLER_OPER_UP) {
      pfc_log_debug("Controller operstatus is not UP for TC_AUDIT_REALNETWORK");
      AuditRequest::IsControllerNotUp =true;
      PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, false);
      PhysicalLayer::ctr_oprn_mutex_.unlock();
      return UNC_UPPL_RC_ERR_AUDIT_FAILURE;
    }
    // Checking the audit precondition
    if (oper_status == UPPL_CONTROLLER_OPER_DOWN ||
        oper_status == UPPL_CONTROLLER_OPER_UP ||
        oper_status == UPPL_CONTROLLER_OPER_WAITING_AUDIT) {
      obj_val_ctr_st.oper_status = UPPL_CONTROLLER_OPER_AUDITING;
      memset(obj_val_ctr_st.valid, '\0', sizeof(obj_val_ctr_st.valid));
      obj_val_ctr_st.valid[kIdxOperStatus] = UNC_VF_VALID;
      UncRespCode handle_oper_status = KtObj.HandleOperStatus(
          db_conn, UNC_DT_RUNNING, reinterpret_cast<void *>(&obj_key_ctr),
          reinterpret_cast<void *>(&obj_val_ctr_st),
          true);
      pfc_log_debug("Handle Oper Status return: %d", handle_oper_status);
      // Set the EventsStartReceived flag as false
      map<string, CtrOprnStatus> :: iterator it;
      it = PhysicalLayer::ctr_oprn_status_.find(controller_id);
      if (it != PhysicalLayer::ctr_oprn_status_.end()) {
        CtrOprnStatus ctr_oprn =  it->second;
        ctr_oprn.EventsStartReceived =  false;
        PhysicalLayer::ctr_oprn_status_[controller_id] = ctr_oprn;
      }
      PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, false);
      PhysicalLayer::ctr_oprn_mutex_.unlock();
      return UNC_RC_SUCCESS;
    } else {
      // return UNC_UPPL_RC_ERR_AUDIT_FAILURE;
    }
  } else {
    pfc_log_error("Unable to get controller oper status");
    tclib_ptr->TcLibWriteControllerInfo(controller_id.c_str(),
                                        UNC_RC_INTERNAL_ERR,
                                        0);
    PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, false);
    PhysicalLayer::ctr_oprn_mutex_.unlock();
    return UNC_UPPL_RC_ERR_AUDIT_FAILURE;
  }
  PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, false);
  PhysicalLayer::ctr_oprn_mutex_.unlock();
  return UNC_RC_SUCCESS;
}

/**StartAuditTransaction
 * @Description : This function is invoked when TC sends AuditTransactionStart
 *                to Physical Core
 * @param[in]   : session_id - ipc session id used for TC validation
 *                driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit to start
 * @return      : UNC_RC_SUCCESS if the AuditTransaction is success for the
 *                controller or UNC_UPPL_RC_ERR_* for audit transaction failure
 * */
UncRespCode AuditRequest::StartAuditTransaction(
    uint32_t session_id,
    unc_keytype_ctrtype_t driver_id,
    string controller_id)  {
  return UNC_RC_SUCCESS;
}

/**HandleAuditVoteRequest
 * @Description : This function is invoked when TC sends AuditVoteRequest to
 *                Physical Core and here,get the controller details for the
 *                controller where the audit occurs
 * @param[in]   : session_id - ipc session id used for TC validation
 *                driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit occurs
 * @param[out]  : driver_info - contains the controller list
 * @return      : UNC_RC_SUCCESS if the AuditVoteRequest is success or 
 *                UNC_UPPL_RC_ERR_* if AuditVoteRequest is failed.
 * */
UncRespCode AuditRequest::HandleAuditVoteRequest(
    OdbcmConnectionHandler *db_conn,
    uint32_t session_id,
    uint32_t driver_id,
    string controller_id,
    TcDriverInfoMap &driver_info) {
  Kt_Controller KtObj;
  std::vector<std::string> controllers;
  controllers.push_back(controller_id);
  key_ctr_t obj_key_ctr;
  memset(obj_key_ctr.controller_name,
         '\0',
         sizeof(obj_key_ctr.controller_name));
  memcpy(obj_key_ctr.controller_name,
         controller_id.c_str(),
         controller_id.length()+1);
  vector<void *> vect_ctr_key, vect_ctr_val;
  vect_ctr_key.push_back(reinterpret_cast<void *>(&obj_key_ctr));
  UncRespCode read_status = KtObj.ReadInternal(
      db_conn, vect_ctr_key, vect_ctr_val,
      (unc_keytype_datatype_t)UNC_DT_RUNNING,
      UNC_OP_READ);
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_error("Could not get details for controller %s",
                  controller_id.c_str());
    TcLibModule* tclib_ptr = static_cast<TcLibModule*>
        (TcLibModule::getInstance(TCLIB_MODULE_NAME));
    tclib_ptr->TcLibWriteControllerInfo(controller_id.c_str(),
                                        UNC_RC_INTERNAL_ERR,
                                        0);
    return UNC_UPPL_RC_ERR_AUDIT_FAILURE;
  }
  pfc_log_debug("Read operation is success");
  val_ctr_st_t *obj_val_ctr =
      (reinterpret_cast <val_ctr_st_t*>(vect_ctr_val[0]));
  if (obj_val_ctr != NULL) {
    unc_keytype_ctrtype_t controller_type =
        (unc_keytype_ctrtype_t)
        (PhyUtil::uint8touint(obj_val_ctr->controller.type));
    pfc_log_debug("Controller Type is %d", controller_type);
    // Release memory allocated for key struct
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(vect_ctr_key[0]);
    if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
    }
    // delete the val memory
    delete obj_val_ctr;
    obj_val_ctr = NULL;
  }
  return UNC_RC_SUCCESS;
}

/**HandleAuditGlobalCommit
 * @Description : This function is invoked when TC sends AuditGlobalCommit
 *                to Physical Core
 * @param[in]   : session_id - ipc session id used for TC validation
 *                driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit occurs
 *                driver_info - contains the controller list
 * @param[out]  : audit result - specifies the TC audit result 
 * @return      : UNC_RC_SUCCESS if the AuditGlobalCommit is success for the
 *                controller or UNC_UPPL_RC_ERR_* for the failure
 * */

UncRespCode AuditRequest::HandleAuditGlobalCommit(
    uint32_t session_id,
    uint32_t driver_id,
    string controller_id,
    TcDriverInfoMap& driver_info,
    TcAuditResult& audit_result) {
  audit_result = unc::tclib::TC_AUDIT_SUCCESS;
  return UNC_RC_SUCCESS;
}

/**HandleAuditDriverResult
 * @Description : This function is invoked when TC sends AuditDriverResult
 *                to Physical Core
 * @param[in]   : session_id - ipc session id used for TC validation
 *                controller_id - controller name in which the audit occurs
 *                commitphase - 
 *                driver_result - 
 * @return      : UNC_RC_SUCCESS if HandleAuditDriverResult is success for the
 *                controller or UNC_UPPL_RC_ERR_* for the failure 
 * */
UncRespCode AuditRequest::HandleAuditDriverResult(
            OdbcmConnectionHandler *db_conn,
            uint32_t session_id,
            string controller_id,
            TcCommitPhaseType commitphase,
            TcCommitPhaseResult driver_result,
            TcAuditResult& audit_result) {
  pfc_log_info("HandleAuditDriverResult drv_res.size=%" PFC_PFMT_SIZE_T,
                                          driver_result.size());
  audit_result = unc::tclib::TC_AUDIT_SUCCESS;
  if (commitphase != unc::tclib::TC_AUDIT_GLOBAL_COMMIT_PHASE) {
    return UNC_RC_SUCCESS;
  }
  pfc_bool_t update_version = false;
  key_ctr_t key_ctr_obj;
  memset(&key_ctr_obj, 0, sizeof(key_ctr_t));
  memcpy(&key_ctr_obj.controller_name, controller_id.c_str(),
                          controller_id.length()+1);
  val_ctr_commit_ver_t ctrlr_val;
  memset(&ctrlr_val, 0, sizeof(val_ctr_commit_ver_t));
  if (driver_result.size() == 0) {
    // Update the commit version received in StartAudit
    map<string, commit_version>::iterator it_cv;
    it_cv = AuditRequest::comm_ver_.find(controller_id);
    pfc_log_debug("size of Commitversion map:%" PFC_PFMT_SIZE_T ,
                            AuditRequest::comm_ver_.size());
    if (it_cv != AuditRequest::comm_ver_.end()) {
      ctrlr_val.commit_number = it_cv->second.commit_number;
      ctrlr_val.commit_date = it_cv->second.commit_date;
      memcpy(&ctrlr_val.commit_application,
                      (it_cv->second.commit_application),
                      sizeof(ctrlr_val.commit_application));
      update_version = true;
    }
  } else {
    std::vector<TcControllerResult>::iterator viter = driver_result.begin();
    for ( ; viter != driver_result.end(); viter++) {
      TcControllerResult tcResult = (*viter);
      ctrlr_val.commit_number = tcResult.commit_number;
      ctrlr_val.commit_date = tcResult.commit_date;
      memcpy(&ctrlr_val.commit_application,
         tcResult.commit_application.c_str(),
         tcResult.commit_application.length()+1);
      update_version = true;
      break;
    }
  }
  if (update_version == false) {
    return UNC_RC_SUCCESS;
  }
  ctrlr_val.valid[kIdxCtrCommitNumber] = UNC_VF_VALID;
  ctrlr_val.valid[kIdxCtrCommitDate] = UNC_VF_VALID;
  ctrlr_val.valid[kIdxCtrCommitApplication] = UNC_VF_VALID;
  pfc_log_info("Audit: Commit_number is %" PFC_PFMT_u64 ", commit_date is %"
                PFC_PFMT_u64
        " commit_application:%s", ctrlr_val.commit_number,
        ctrlr_val.commit_date, ctrlr_val.commit_application);
  Kt_Controller KtCtrObj;
  UncRespCode update_status = KtCtrObj.UpdateKeyInstance(db_conn,
                reinterpret_cast<void*>(&key_ctr_obj),
               reinterpret_cast<void*>(&ctrlr_val),
               UNC_DT_RUNNING, UNC_KT_CONTROLLER,
               (pfc_bool_t)true);
  if (update_status != UNC_RC_SUCCESS) {
    audit_result = unc::tclib::TC_AUDIT_FAILURE;
    if (update_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
      // Already fatal logged in KtCtrObj.UpdateKeyInstance
      return update_status;
    } else {
      UPPL_LOG_FATAL(
        "Audit: commit version update is failed for controller %s, status: %d",
         key_ctr_obj.controller_name, update_status);
      return UNC_UPPL_RC_ERR_DB_UPDATE;
    }
  }

  return UNC_RC_SUCCESS;
}

/**HandleAuditAbort
 * @Description : This function is invoked when TC sends AuditGlobalAbort
 *                to Physical Core
 * @param[in]   : session_id - ipc session id used for TC validation
 *                driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit occurs
 *                operation_phase - 
 * @return      : UNC_RC_SUCCESS if the AuditGlobalAbort is success for the
 *                controller or UNC_UPPL_RC_ERR_* for the failure 
 * */

UncRespCode AuditRequest::HandleAuditAbort(
    uint32_t session_id,
    unc_keytype_ctrtype_t driver_id,
    string controller_id,
    TcAuditOpAbortPhase operation_phase) {
  pfc_log_debug("Returning success for HandleAuditAbort");
  return UNC_RC_SUCCESS;
}

/**HandleAuditCancel
 * @Description : This function is invoked when TC sends cancel audit when
 *                commit/abort request arrives with cancel flag enable
 * @param[in]   : session_id - ipc session id used for TC validation
 *                driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit occurs 
 * @return      : UNC_RC_SUCCESS if the AuditCancel is success for the
 *                controller or UNC_UPPL_RC_ERR_* for the failure 
 * */

UncRespCode AuditRequest::HandleAuditCancel(
    uint32_t session_id,
    unc_keytype_ctrtype_t driver_id,
    string controller_id) {
  pfc_log_debug("Returning success for HandleAuditCancel");
  return UNC_RC_SUCCESS;
}

/**EndAuditTransaction
 * @Description : This function is used to end the Audit Transaction
 * @param[in]   : session_id - ipc session id used for TC validation
 *                driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit occurs 
 * @return      : UNC_RC_SUCCESS if the EndAuditTransaction is success for the
 *                controller or UNC_UPPL_RC_ERR_* for the failure
 * */
UncRespCode AuditRequest::EndAuditTransaction(
    uint32_t session_id,
    unc_keytype_ctrtype_t& drivertype,
    string controller_id) {
  return UNC_RC_SUCCESS;
}

/**EndAudit
 * @Description : This function is used to end the Audit and adds the
 *                controller to audit list
 * @param[in]   : driver_id - Specifies one of the following controller type-
 *                PFC,VNP,Bypass
 *                controller_id - controller name in which the audit occurs
 *                audit result - specifies the TC audit result
 * @return      : UNC_RC_SUCCESS if the End Audit is success for the controller
 *                or returns UNC_UPPL_RC_ERR_ if the Audit is failed
 * */
UncRespCode AuditRequest::EndAudit(OdbcmConnectionHandler *db_conn,
                                      unc_keytype_ctrtype_t driver_id,
                                      string controller_id,
                                      TcAuditResult audit_result) {
  pfc_log_info("EndAudit cname:%s ares:%d",
               controller_id.c_str(), audit_result);
  uint8_t oper_status = 0;
  Kt_Controller kt_controller;
  key_ctr_t key_ctr_obj;
  memset(&key_ctr_obj, '\0', sizeof(key_ctr_t));
  memcpy(key_ctr_obj.controller_name,
         controller_id.c_str(), controller_id.length()+1);
  void *key_ctr_prt = reinterpret_cast<void *>(&key_ctr_obj);
  val_ctr_st_t obj_val_ctr_st;
  memset(&obj_val_ctr_st, '\0', sizeof(val_ctr_st_t));

  if (audit_result == unc::tclib::TC_AUDIT_SUCCESS) {
    // Clear the taskqueue
    PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
    taskq_util->clear_task_queue(controller_id);
  }
  // Clear the event handling failure alarm
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
     get_physical_core();
  physical_core->remove_ctr_from_alarm_status_map(controller_id, "3");

  PhysicalLayer* physical_layer = PhysicalLayer::get_instance();
  uint8_t db_oper_status = UPPL_CONTROLLER_OPER_AUDITING;
  UncRespCode read_status;
  ReadWriteLock *rwlock = NULL;
  PhysicalLayer::ctr_oprn_mutex_.lock();
  PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, true);
  read_status = kt_controller.GetOperStatus(db_conn, UNC_DT_RUNNING,
                                key_ctr_prt, db_oper_status);
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_error("Unable to get controller oper status");
  }
  uint8_t controller_actual_oper_status = UPPL_CONTROLLER_OPER_DOWN;
  map<string, CtrOprnStatus> :: iterator it =
    PhysicalLayer::ctr_oprn_status_.find(controller_id);
  if (it != PhysicalLayer::ctr_oprn_status_.end()) {
    CtrOprnStatus ctr_oprn =  it->second;
    controller_actual_oper_status = ctr_oprn.ActualOperStatus;
  }

  if (audit_result == unc::tclib::TC_AUDIT_SUCCESS) {
    if (db_oper_status != UPPL_CONTROLLER_OPER_DOWN) {
      pfc_log_debug("Setting oper status as UP inside  if"
                    "(db_oper_status != UPPL_CONTROLLER_OPER_DOWN");
      // Add the controller to controller_in_audit vector
      IPCConnectionManager *ipc_mgr = PhysicalLayer::get_instance()->
          get_ipc_connection_manager();
      if (ipc_mgr != NULL) {
        pfc_log_debug("Adding controller to audit list %s",
                      controller_id.c_str());
        ipc_mgr->addControllerToAuditList(controller_id);
        PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, false);
        PhysicalLayer::ctr_oprn_mutex_.unlock();
        // Clear the taskqueue
        PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
        taskq_util->clear_task_queue(controller_id);
        ODBCM_RC_STATUS clear_status =
        PhysicalLayer::get_instance()->get_odbc_manager()->
        ClearOneInstance(UNC_DT_IMPORT, controller_id, db_conn);
        if (clear_status != ODBCM_RC_SUCCESS) {
          pfc_log_info("Import DB clearing failed");
        }
        // Reset actual id as empty
        string actual_id = "";
        UncRespCode status = kt_controller.SetActualControllerId(db_conn,
                                                  key_ctr_prt, actual_id,
                                         UNC_DT_RUNNING, UNC_VF_INVALID);
        if (status != UNC_RC_SUCCESS) {
          // log error
          pfc_log_error("EndAudit:actual_id reset failed for running");
        }
        status = kt_controller.SetActualControllerId(db_conn,
                                      key_ctr_prt, actual_id,
                           UNC_DT_CANDIDATE, UNC_VF_INVALID);
        if (status != UNC_RC_SUCCESS) {
          // log error
          pfc_log_error("EndAudit:actual_id reset failed for candidate");
        }
        // Start timer for processing audit notification from driver
        uint32_t ret = ipc_mgr->StartNotificationTimer(db_conn, controller_id);
        pfc_log_debug("Start Timer return code %d for controller %s",
                    ret, controller_id.c_str());
      } else {
        pfc_log_debug("IPC Connection Manager Object is NULL");
        PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, false);
        PhysicalLayer::ctr_oprn_mutex_.unlock();
      }
      map<string, vector<struct unc::uppl::alarm_buffer*> > ::iterator it =
                     NotificationRequest::map_alarm_buff.find(controller_id);
      if (it != NotificationRequest::map_alarm_buff.end()) {
        for (vector<struct unc::uppl::alarm_buffer*> ::iterator next_iter =
                                                   it->second.begin();
            next_iter != it->second.end();
            next_iter++) {
          delete (*next_iter);
          pfc_log_trace("next vector entry is deleted");
        }
        it->second.clear();
        NotificationRequest::map_alarm_buff.erase(it);
      }
    } else {
      PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, false);
      PhysicalLayer::ctr_oprn_mutex_.unlock();
    }
    // "user initiated audit/Driver audit recovery alarm will be sent
    // if there is an occurence alarm
    // send audit success (recovery) alarm
    UncRespCode alarms_status = physical_layer->get_physical_core()->
        SendControllerAuditSuccessAlarm(controller_id);
    pfc_log_debug("Alarm status: %d", alarms_status);
  } else if (audit_result == unc::tclib::TC_AUDIT_CANCELLED) {
    if (db_oper_status != UPPL_CONTROLLER_OPER_DOWN) {
      if (AuditRequest::ctr_oper_status_before_audit != INVALID_OPERSTATUS) {
        obj_val_ctr_st.oper_status = controller_actual_oper_status;
        memset(obj_val_ctr_st.valid, '\0', sizeof(obj_val_ctr_st.valid));
        obj_val_ctr_st.valid[kIdxOperStatus] = UNC_VF_VALID;
        UncRespCode handle_oper_status = kt_controller.HandleOperStatus(
            db_conn, UNC_DT_RUNNING,
            key_ctr_prt,
            reinterpret_cast< void *>(&obj_val_ctr_st),
            true);
        pfc_log_debug("EndAudit:handle_oper_status ret = %d",
                       handle_oper_status);
        // Setting the EventStartReceived flag as true
        map<string, CtrOprnStatus> :: iterator it;
        it = PhysicalLayer::ctr_oprn_status_.find(controller_id);
        if (it != PhysicalLayer::ctr_oprn_status_.end()) {
          CtrOprnStatus ctr_oprn =  it->second;
          ctr_oprn.EventsStartReceived =  true;
          PhysicalLayer::ctr_oprn_status_[controller_id] = ctr_oprn;
        }
      }
    }
    PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, false);
    PhysicalLayer::ctr_oprn_mutex_.unlock();
  } else if (audit_result == unc::tclib::TC_AUDIT_FAILURE) {
    bool AuditFailureAlarm = false;
    if (controller_actual_oper_status != UPPL_CONTROLLER_OPER_DOWN) {
      if (AuditRequest::IsControllerNotUp == true) {
        // Do Nothing
        pfc_log_debug("Inside IsControllerNotUp == true loop");
      } else {
        if (db_oper_status == UPPL_CONTROLLER_OPER_AUDITING &&
            audit_type_ == AUDIT_REALNETWORK) {
          pfc_log_debug("Inside AUDITING && AUDIT_REALNETWORK loop");
          oper_status = UPPL_CONTROLLER_OPER_UP;
          obj_val_ctr_st.oper_status = oper_status;
          memset(obj_val_ctr_st.valid, '\0', sizeof(obj_val_ctr_st.valid));
          obj_val_ctr_st.valid[kIdxOperStatus] = UNC_VF_VALID;
          UncRespCode handle_oper_status = kt_controller.HandleOperStatus(
              db_conn, UNC_DT_RUNNING,
              key_ctr_prt,
              reinterpret_cast<void *>(&obj_val_ctr_st),
              true);
          pfc_log_debug("EndAudit:handle_oper_status ret = %d",
                         handle_oper_status);
        } else {
          oper_status = UPPL_CONTROLLER_OPER_WAITING_AUDIT;
          obj_val_ctr_st.oper_status = oper_status;
          memset(obj_val_ctr_st.valid, '\0', sizeof(obj_val_ctr_st.valid));
          obj_val_ctr_st.valid[kIdxOperStatus] = UNC_VF_VALID;
          UncRespCode handle_oper_status = kt_controller.HandleOperStatus(
            db_conn, UNC_DT_RUNNING,
            key_ctr_prt,
            reinterpret_cast<void *>(&obj_val_ctr_st),
            true);
          pfc_log_debug("EndAudit: handle_oper_status ret = %d",
                         handle_oper_status);
          // send audit failure alarm (manual audit/driver audit)
          AuditFailureAlarm = true;
        }
      }
    } else if (controller_actual_oper_status == UPPL_CONTROLLER_OPER_DOWN) {
        obj_val_ctr_st.oper_status = UPPL_CONTROLLER_OPER_DOWN;
        memset(obj_val_ctr_st.valid, '\0', sizeof(obj_val_ctr_st.valid));
        obj_val_ctr_st.valid[kIdxOperStatus] = UNC_VF_VALID;
        UncRespCode handle_oper_status = kt_controller.HandleOperStatus(
            db_conn, UNC_DT_RUNNING,
            key_ctr_prt,
            reinterpret_cast< void *>(&obj_val_ctr_st),
            true);
        pfc_log_debug("FAILED-EndAudit:CtrDOWN:handle_oper_status ret = %d",
                       handle_oper_status);
        // send audit failure alarm (manual audit/driver audit)
        AuditFailureAlarm = true;
    }
    if (AuditFailureAlarm == true) {
      UncRespCode alarms_status = physical_layer->get_physical_core()->
      SendControllerAuditFailureAlarm(controller_id);
      pfc_log_debug("Alarm status: %d", alarms_status);
      AuditFailureAlarm = false;
    }
    PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, false);
    PhysicalLayer::ctr_oprn_mutex_.unlock();
  } else {
    PHY_OPERSTATUS_LOCK(controller_id, read_status, rwlock, false);
    PhysicalLayer::ctr_oprn_mutex_.unlock();
  }
  /* Clearing the entry from commit version map */
  map<string, commit_version>::iterator it_cv;
  it_cv = AuditRequest::comm_ver_.find(controller_id);
  if (it_cv != AuditRequest::comm_ver_.end()) {
    AuditRequest::comm_ver_.erase(it_cv);
  }
  AuditRequest::ctr_oper_status_before_audit = INVALID_OPERSTATUS;
  AuditRequest::IsControllerNotUp = false;
  AuditRequest::audit_type_ = AUDIT_NORMAL;
  return UNC_RC_SUCCESS;
}

/**MergeAuditDbToRunning
 * @Description : This function is used to Merge audit and running db at
 *                end Audit and clear the import db.
 * @param[in]   : controller name - controller name in which the audit occurs
 * @return      : UNC_RC_SUCCESS if the updation of running DB with the
 *                imported value is success or UNC_UPPL_RC_ERR_* for failure of 
 *                merging
 * */
UncRespCode AuditRequest::MergeAuditDbToRunning(
    OdbcmConnectionHandler *db_conn,
    string controller_name) {
  pfc_log_info("MergeAuditDbToRunning-controller_name: %s",
               controller_name.c_str());
  pfc_bool_t is_controller_in_audit =
             PhysicalLayer::get_instance()->get_ipc_connection_manager()->
                                           IsControllerInAudit(controller_name);
  if (is_controller_in_audit == PFC_FALSE) {
    // Not required to do merge, return SUCCESS
    return UNC_RC_SUCCESS;
  }
  UncRespCode return_code = UNC_RC_SUCCESS;
  // Remove the controller_name from controller_in_audit list
  return_code = PhysicalLayer::get_instance()->get_ipc_connection_manager()->
      removeControllerFromAuditList(controller_name);
  if (return_code != UNC_RC_SUCCESS) {
    // Not required to do merge, return SUCCESS
    return UNC_RC_SUCCESS;
  }
  for (unsigned int index = 0;
      index < STATE_OBJECTS; ++index) {
    void *key_struct = NULL;
    uint32_t key_type;
    Kt_Base *class_pointer = GetClassPointerAndKey(
        (AuditStateObjects)index, controller_name, key_struct, key_type);
    if (class_pointer == NULL) {
      if (key_struct != NULL) {
        FreeKeyAndValueStruct(key_struct, NULL, index);
      }
      continue;
    }
    vector<void*> key_running, key_import;
    key_running.push_back(key_struct);
    key_import.push_back(key_struct);
    vector<void*> val_running, val_import;
    pfc_log_debug("Reading from running db for index %d", index);
    UncRespCode read_running_status = class_pointer->ReadInternal(
        db_conn, key_running,
        val_running,
        (uint32_t) UNC_DT_RUNNING,
        (uint32_t) UNC_OP_READ_SIBLING_BEGIN);
    // Read from Import DB
    pfc_log_debug("Reading from import db for index %d", index);
    UncRespCode read_import_status = class_pointer->ReadInternal(
        db_conn, key_import,
        val_import,
        (uint32_t) UNC_DT_IMPORT,
        (uint32_t) UNC_OP_READ_SIBLING_BEGIN);
    if (read_running_status != UNC_RC_SUCCESS &&
        read_import_status != UNC_RC_SUCCESS) {
      pfc_log_info(
          "Reading values from both import and running db failed for index %d",
          index);
      delete class_pointer;
      if (key_struct != NULL) {
        FreeKeyAndValueStruct(key_struct, NULL, index);
      }
      continue;
    }
    pfc_log_debug("No.of entries from running db %"
                  PFC_PFMT_SIZE_T, key_running.size());
    pfc_log_debug("No.of entries from import db %"
                  PFC_PFMT_SIZE_T, key_import.size());
    // Iterate RUNNING and compare with IMPORT
    // Update Running if values mismatch with Import
    // Delete entries from Running if not found in Import
    UpdateRunningDbWithImportDb(key_import, val_import,
                                key_running, val_running,
                                index, class_pointer,
                                db_conn, key_type);
    // Iterate IMPORT and compare with RUNNING
    // And create entries in running db if not present
    if (index != Notfn_Port_Neighbor)
      AddToRunningDbFromImportDb(key_import, val_import,
                               key_running, val_running,
                               index, class_pointer,
                               db_conn, key_type);
    // Free the readInternal key and value structures
    FreeKeyAndValueStruct(key_import, val_import,
                          key_running, val_running, index);
    delete class_pointer;
    if (key_struct != NULL) {
      FreeKeyAndValueStruct(key_struct, NULL, index);
    }
  }
  // Clear import db entries
  ODBCM_RC_STATUS clear_status =
      PhysicalLayer::get_instance()->get_odbc_manager()->
      ClearOneInstance(UNC_DT_IMPORT, controller_name, db_conn);
  if (clear_status != ODBCM_RC_SUCCESS) {
    pfc_log_info("Import DB clearing failed");
  }
  // Send path fault Alarm
  SendAlarmAndClear(controller_name, db_conn);

  // Send UPPL_CONTROLLER_OPER_EVENTS_MERGED status notification to UPLL
  uint8_t oper_status_db = UPPL_CONTROLLER_OPER_UP,
          new_oper_status = UPPL_CONTROLLER_OPER_EVENTS_MERGED;
  key_ctr_t ctr_key_struct;
  memset(&ctr_key_struct, 0, sizeof(key_ctr_t));
  strncpy(reinterpret_cast<char*>(ctr_key_struct.controller_name),
                               controller_name.c_str(),
                               controller_name.length());
  Kt_Controller KtObj;
  return_code = KtObj.SendOperStatusNotification(ctr_key_struct,
                                        oper_status_db, new_oper_status);
  if (return_code == UNC_RC_SUCCESS)
    pfc_log_info("EVENTS_MERGED notification is sent to UPLL");
  else
    pfc_log_error("Err in sending EVENTS_MERGED notification to UPLL");
  // Send path fault Alarm
  return return_code;
}

/** SendAlarm
 * @Description : This function sends the alarm from the store buffer after
 *                 Merging the Audit datbase to State Database
 *
 * */
void AuditRequest::SendAlarmAndClear(string controller_name,
                                     OdbcmConnectionHandler *db_conn) {
  Kt_Controller NotifyController;
  map<string, vector<struct unc::uppl::alarm_buffer*> > ::iterator it =
                     NotificationRequest::map_alarm_buff.find(controller_name);
  if (it != NotificationRequest::map_alarm_buff.end()) {
    for (uint32_t size = 0; size < it->second.size(); size++) {
      NotifyController.HandleDriverAlarms(
                   db_conn,
                   UNC_DT_STATE,
                   it->second[size]->alarm_type,
                   it->second[size]->oper_type,
                   it->second[size]->key_struct,
                   it->second[size]->val_struct);
    }
    for (vector<struct unc::uppl::alarm_buffer*> ::iterator next_iter =
                                                     it->second.begin();
        next_iter != it->second.end();
        next_iter++) {
      delete (*next_iter);
      pfc_log_trace("next vector entry is deleted");
    }

    it->second.clear();
    NotificationRequest::map_alarm_buff.erase(it);
  }
}


/**GetClassPointerAndKey
 * @Description : This function is used to get the key and class pointer
 *                of the appropriate kt_classes
 * @param[in]   : audit_key_type -
 *                controller name - controller name in which the audit occurs
 *                key - Specifies the key of the appropriate key types
 *                key_type - specifies the appropriate key types
 * @return      : Returns the class pointer to the appropriate kt classes.
 * */
Kt_Base* AuditRequest::GetClassPointerAndKey(
    AuditStateObjects audit_key_type,
    string controller_name,
    void* &key,
    uint32_t &key_type) {
  Kt_Base* class_pointer = NULL;
  switch (audit_key_type) {
    case Notfn_Ctr_Domain: {
      class_pointer = new Kt_Ctr_Domain();
      key_ctr_domain_t *obj_key = new key_ctr_domain_t;
      memset(obj_key, 0, sizeof(key_ctr_domain_t));
      memcpy(obj_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_CTR_DOMAIN;
      break;
    }
    case Notfn_Logical_Port: {
      class_pointer = new Kt_LogicalPort();
      key_logical_port_t *obj_key = new key_logical_port_t;
      memset(obj_key, 0, sizeof(key_logical_port_t));
      memcpy(obj_key->domain_key.ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_LOGICAL_PORT;
      break;
    }
    case Notfn_Logical_Member_Port: {
      class_pointer = new Kt_LogicalMemberPort();
      key_logical_member_port_t * obj_key = new key_logical_member_port_t;
      memset(obj_key, 0, sizeof(key_logical_member_port_t));
      memcpy(obj_key->logical_port_key.domain_key.
             ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_LOGICAL_MEMBER_PORT;
      break;
    }
    case Notfn_Switch: {
      class_pointer = new Kt_Switch();
      key_switch_t *obj_key = new key_switch_t;
      memset(obj_key, 0, sizeof(key_switch_t));
      memcpy(obj_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_SWITCH;
      break;
    }
    case Notfn_Port: {
      class_pointer = new Kt_Port();
      key_port_t *obj_key = new key_port_t;
      memset(obj_key, 0, sizeof(key_port_t));
      memcpy(obj_key->sw_key.ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_PORT;
      break;
    }
    case Notfn_Port_Neighbor: {
      class_pointer = new Kt_Port_Neighbor();
      key_port_t *obj_key = new key_port_t;
      memset(obj_key, 0, sizeof(key_port_t));
      memcpy(obj_key->sw_key.ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_PORT_NEIGHBOR;
      break;
    }
    case Notfn_Link: {
      class_pointer = new Kt_Link();
      key_link_t *obj_key = new key_link_t;
      memset(obj_key, 0, sizeof(key_link_t));
      memcpy(obj_key->ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      key = reinterpret_cast<void *>(obj_key);
      key_type = UNC_KT_LINK;
      break;
    }
  }
  return class_pointer;
}

/**FreeKeyAndValueStruct
 * @Description : This function clears the key struct and value struct
 *                of the appropriate key types
 * @param[in]   : key struct - the key for the appropriate key types
 *                value struct - the value structure for the appropriate
 *                key types
 *                key type - UNC_KT_* specifies the appropriate key types
 * @return      : returns void * key structure for  the successfull deletion
 *                of key and value structures of the key types
 * */
void AuditRequest::FreeKeyAndValueStruct(void* key_struct,
                                         void* value_struct,
                                         unsigned int key_type) {
  switch (key_type) {
    case Notfn_Ctr_Domain: {
      key_ctr_domain_t *obj_key = reinterpret_cast<key_ctr_domain_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      val_ctr_domain_st_t *obj_val =
          reinterpret_cast<val_ctr_domain_st_t*> (value_struct);
      if (obj_val != NULL) {
        delete obj_val;
        obj_val = NULL;
      }
      break;
    }
    case Notfn_Logical_Port: {
      key_logical_port_t *obj_key = reinterpret_cast<key_logical_port_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      val_logical_port_st_t *obj_val =
          reinterpret_cast<val_logical_port_st_t*> (value_struct);
      if (obj_val != NULL) {
        delete obj_val;
        obj_val = NULL;
      }
      break;
    }
    case Notfn_Logical_Member_Port: {
      key_logical_member_port_t *obj_key =
          reinterpret_cast<key_logical_member_port_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      break;
    }
    case Notfn_Switch: {
      key_switch_t *obj_key = reinterpret_cast<key_switch_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      val_switch_st_t *obj_val =
          reinterpret_cast<val_switch_st_t*> (value_struct);
      if (obj_val != NULL) {
        delete obj_val;
        obj_val = NULL;
      }
      break;
    }
    case Notfn_Port: {
      key_port_t *obj_key = reinterpret_cast<key_port_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      val_port_st_t *obj_val =
          reinterpret_cast<val_port_st_t*> (value_struct);
      if (obj_val != NULL) {
        delete obj_val;
        obj_val = NULL;
      }
      break;
    }
    case Notfn_Port_Neighbor: {
      key_port_t *obj_key = reinterpret_cast<key_port_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      val_port_st_neighbor_t *obj_val =
          reinterpret_cast<val_port_st_neighbor_t*> (value_struct);
      if (obj_val != NULL) {
        delete obj_val;
        obj_val = NULL;
      }
      break;
    }
    case Notfn_Link: {
      key_link_t *obj_key = reinterpret_cast<key_link_t *>
      (key_struct);
      if (obj_key != NULL) {
        delete obj_key;
        obj_key = NULL;
      }
      val_port_st_t *obj_val =
          reinterpret_cast<val_port_st_t*> (value_struct);
      if (obj_val != NULL) {
        delete obj_val;
        obj_val = NULL;
      }
      break;
    }
  }
  return;
}

/**
 * @Description : This overloaded function is called when audit notification
 * time out expires
 * @param[in]   : None
 * @return      : None
 * */
void AuditNotification::operator() ()  {
  pfc_log_debug("Received Audit Notification timer out controller %s",
                controller_name_.c_str());
  AuditRequest audit_req;
  UncRespCode db_ret = UNC_RC_SUCCESS;
  PHY_DB_SB_CXN_LOCK();
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteSb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    pfc_log_error("Error in opening DB connection");
  } else {
    audit_req.MergeAuditDbToRunning(&db_conn, controller_name_);
  }
}

/**
 * @Description : This function compares running db and import db
 *                If values mismatch in import db and running db, running db
 *                will be updated using values from import db
 *                If key instances available in running db is not available in
 *                import db, it will be deleted from running db
 * @param[in]   : Vector of Key, Value available in running and import db
 * @return      : None
 * */
void AuditRequest::UpdateRunningDbWithImportDb(
    const vector<void *> &key_import,
    const vector<void *> &val_import,
    const vector<void *> &key_running,
    const vector<void *> &val_running,
    unsigned int index,
    Kt_Base *class_pointer,
    OdbcmConnectionHandler *db_conn,
    uint32_t key_type) {
  vector<void*> cur_key_running,cur_val_running;
  for (unsigned int iIndex = 0;
      iIndex < (uint32_t)key_running.size(); ++iIndex) {
   {  //Begin a new block level scope for the RW DB-lock
    ScopedReadWriteLock eventDoneLock(PhysicalLayer::get_events_done_lock_(),
                                    PFC_TRUE); 
   if( cur_key_running.size() != 0 ) {
     if(cur_val_running.size() != 0) 
       FreeKeyAndValueStruct(cur_key_running[0], cur_val_running[0], index); 
     else
       FreeKeyAndValueStruct(cur_key_running[0], NULL, index); 
   }
   cur_val_running.clear();
   cur_key_running.clear();

   cur_key_running.push_back(key_running[iIndex]);
   UncRespCode read_running_status = class_pointer->ReadInternal(
        db_conn, cur_key_running,
        cur_val_running,
        (uint32_t) UNC_DT_RUNNING,
        (uint32_t) UNC_OP_READ);
    //  if read is failed. continue. Between Commit Thread can remove a record
    if (read_running_status != UNC_RC_SUCCESS) {
      pfc_log_error("UpdateRunningDbWithImportDb:keynotfound");
      continue;
    }
    bool present_in_import = false;
    for (unsigned int jIndex = 0;
        jIndex < (uint32_t)key_import.size(); ++jIndex) {
      // check whether key is same
      pfc_bool_t is_same_key = class_pointer->CompareKeyStruct(
          key_running[iIndex],
          key_import[jIndex]);
      if (is_same_key == PFC_FALSE) {
        continue;
      }
      present_in_import = true;
      // check whether value is same
      if (index == Notfn_Logical_Member_Port) {
        continue;
      }
      pfc_bool_t is_same_value = class_pointer->CompareValueStruct(
          cur_val_running[0],
          val_import[jIndex]);
      if (is_same_value == PFC_TRUE) {
        continue;
      }
      // Update running db with import value
      pfc_log_debug("Update running db with import db value");
      void *old_val_struct = NULL;
      UncRespCode update_status = class_pointer->
          UpdateKeyInstance(
              db_conn, reinterpret_cast<void *>(key_running[iIndex]),
              reinterpret_cast<void *>(val_import[jIndex]),
              (uint32_t)UNC_DT_STATE,
              key_type,
              old_val_struct);
      if (update_status != UNC_RC_SUCCESS) {
        pfc_log_error("Update failed for existing key");
        continue;
      }
      if (index != Notfn_Port_Neighbor) {
      // Send value change notification to north bound - val_st
        UncRespCode notfn_status =
          class_pointer->ConfigurationChangeNotification(
              (uint32_t)UNC_DT_STATE,
              key_type,
              (uint32_t)UNC_OP_UPDATE,
              reinterpret_cast<void *>(key_running[iIndex]),
              reinterpret_cast<void *>(val_running[iIndex]),
              reinterpret_cast<void *>(val_import[jIndex]));
        if (notfn_status != UNC_RC_SUCCESS) {
          pfc_log_error("Failed sending update notification");
        }
      }
      // Old value structure memory clean up
      if (old_val_struct != NULL) {
        class_pointer->ClearValueStructure(key_type,
                        old_val_struct);
        old_val_struct = NULL;
      }
      break;
    }
    if (present_in_import == false) {
      // Delete instance from running db
      pfc_log_debug("Delete existing entries from running db");
      UncRespCode delete_status = class_pointer->DeleteKeyInstance(
          db_conn, reinterpret_cast<void *>(key_running[iIndex]),
          (uint32_t)UNC_DT_STATE,
          key_type);
      if (delete_status != UNC_RC_SUCCESS) {
        pfc_log_error("Delete failed for existing key");
      }
      if (index != Notfn_Port_Neighbor) {
      // Send value change notification to north bound - val_st
        UncRespCode notfn_status =
          class_pointer->ConfigurationChangeNotification(
              (uint32_t)UNC_DT_STATE,
              key_type,
              (uint32_t)UNC_OP_DELETE,
              reinterpret_cast<void *>(key_running[iIndex]),
              NULL,
              NULL);
        if (notfn_status != UNC_RC_SUCCESS) {
          pfc_log_error("Failed sending delete notification");
        }
      }
    }
   }  //ScopedLock is Auto Released
  pthread_yield();     //Get Rescheduled again for commit thread to get lock
  } 
  if ( cur_key_running.size()!= 0 ) {
     if(cur_val_running.size() != 0) 
       FreeKeyAndValueStruct(cur_key_running[0], cur_val_running[0], index); 
     else
       FreeKeyAndValueStruct(cur_key_running[0], NULL, index); 
  }
}

/**
 * @Description : This function compares running db and import db *
 *                If key instances available in import db is not available in
 *                running db, it will be added to running db
 * @param[in]   : Vector of Key, Value available in running and import db
 * @return      : None
 * */
void AuditRequest::AddToRunningDbFromImportDb(
    const vector<void *> &key_import,
    const vector<void *> &val_import,
    const vector<void *> &key_running,
    const vector<void *> &val_running,
    unsigned int index,
    Kt_Base *class_pointer,
    OdbcmConnectionHandler *db_conn,
    uint32_t key_type) {
  for (unsigned int iIndex = 0;
      iIndex < (uint32_t)key_import.size(); ++iIndex) {
    bool present_in_running = false;
    for (unsigned int jIndex = 0;
        jIndex < (uint32_t)key_running.size(); ++jIndex) {
      pfc_bool_t is_same_key = class_pointer->CompareKeyStruct(
          key_import[iIndex],
          key_running[jIndex]);
      // Check if key already present in running db
      if (is_same_key == PFC_TRUE) {
        present_in_running = true;
        break;
      }
    }
    if (present_in_running == true) {
      continue;
    }
    {
    ScopedReadWriteLock eventDoneLock(PhysicalLayer::get_events_done_lock_(),
                                    PFC_TRUE);
    // Create instance in running db
    UncRespCode create_status = UNC_RC_SUCCESS;
    // Validate key
    UncRespCode validate_status = UNC_RC_SUCCESS;
    if (index == Notfn_Logical_Member_Port) {
      validate_status = class_pointer->ValidateRequest(
          db_conn,
          reinterpret_cast<void *>(key_import[iIndex]),
          NULL,
          UNC_OP_CREATE, UNC_DT_STATE, key_type);
    } else {
      validate_status = class_pointer->ValidateRequest(
          db_conn,
          reinterpret_cast<void *>(key_import[iIndex]),
          reinterpret_cast<void *>(val_import[iIndex]),
          UNC_OP_CREATE, UNC_DT_STATE, key_type);
    }
    if (validate_status != UNC_RC_SUCCESS) {
      pfc_log_debug("Validation failed for index %d", index);
      continue;
    }
    pfc_log_debug("Create new entries in db for index: %d", index);
    if (index == Notfn_Logical_Member_Port) {
      create_status = class_pointer->CreateKeyInstance(
          db_conn, reinterpret_cast<void *>(key_import[iIndex]),
          NULL,
          (uint32_t)UNC_DT_STATE,
          key_type);
    } else {
      create_status = class_pointer->CreateKeyInstance(
          db_conn, reinterpret_cast<void *>(key_import[iIndex]),
          reinterpret_cast<void *>(val_import[iIndex]),
          (uint32_t)UNC_DT_STATE,
          key_type);
    }
    if (create_status == UNC_RC_SUCCESS) {
      // Send value change notification to north bound - val_st
      UncRespCode notfn_status = UNC_RC_SUCCESS;
      if (index == Notfn_Logical_Member_Port) {
        notfn_status =
            class_pointer->ConfigurationChangeNotification(
                (uint32_t)UNC_DT_STATE,
                key_type,
                (uint32_t)UNC_OP_CREATE,
                reinterpret_cast<void *>(key_import[iIndex]),
                NULL,
                NULL);
      } else {
        notfn_status =
            class_pointer->ConfigurationChangeNotification(
                (uint32_t)UNC_DT_STATE,
                key_type,
                (uint32_t)UNC_OP_CREATE,
                reinterpret_cast<void *>(key_import[iIndex]),
                NULL,
                reinterpret_cast<void *>(val_import[iIndex]));
      }
      if (notfn_status != UNC_RC_SUCCESS) {
        pfc_log_error("Failed sending create notification");
      }
    } else {
      pfc_log_error("Create failed for new key");
    }
    }
    pthread_yield();
  }
  return;
}

/**
 * @Description : This function clears the internal structure contains key
 *                and values of import and running db
 * @param[in]   : Vector of Key, Value available in running and import db
 * @return      : None
 * */
void AuditRequest::FreeKeyAndValueStruct(
    const vector<void *> &key_import,
    const vector<void *> &val_import,
    const vector<void *> &key_running,
    const vector<void *> &val_running,
    unsigned int index) {
  for (unsigned int iIndex = 0;
      iIndex < (uint32_t)key_import.size(); ++iIndex) {
    if (index == Notfn_Logical_Member_Port) {
      FreeKeyAndValueStruct(key_import[iIndex],
                            NULL,
                            index);
    } else {
      FreeKeyAndValueStruct(key_import[iIndex],
                            val_import[iIndex],
                            index);
    }
  }
  for (unsigned int iIndex = 0;
      iIndex < (uint32_t)key_running.size(); ++iIndex) {
    if (index == Notfn_Logical_Member_Port) {
      FreeKeyAndValueStruct(key_running[iIndex],
                            NULL,
                            index);
    } else {
      FreeKeyAndValueStruct(key_running[iIndex],
                            val_running[iIndex],
                            index);
    }
  }
}

}  // namespace uppl
}  // namespace unc
