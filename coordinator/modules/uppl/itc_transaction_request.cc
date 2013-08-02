/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    Transaction Handler
 * @file     itc_transact_request.cc
 *
 */

#include "physicallayer.hh"
#include "physical_itc.hh"
#include "odbcm_common.hh"
#include "odbcm_db_tableschema.hh"
#include "odbcm_mgr.hh"
#include "itc_transaction_request.hh"
#include "itc_kt_boundary.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_ctr_domain.hh"
#include "tclib_module.hh"
#include "ipc_client_configuration_handler.hh"

/** TransactionRequest
 * * @Description : This function is used to initialise the member data.
 * * * @param[in] : None
 * * * @return    : void
 * */
TransactionRequest::TransactionRequest() {
}
/** ~TransactionRequest
 * * @Description : This function is used to release any memory
                    allocated to a pointer member data.
 * * * @param[in] : None
 * * * @return    : void
 * */
TransactionRequest::~TransactionRequest() {
}

/** GetModifiedConfiguration : This function is used to get the modified
 * configurations from the Candidate Database.
 * @param[in] : None
 * @return    : Success or associated error code
 */
UpplReturnCode TransactionRequest::GetModifiedConfiguration(
    uint32_t session_id,
    uint32_t config_id,
    CsRowStatus row_status) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  ret_code = GetModifiedController(row_status);
  if (ret_code != UPPL_RC_SUCCESS) {
    return ret_code;
  }
  ret_code = GetModifiedDomain(row_status);
  if (ret_code != UPPL_RC_SUCCESS) {
    return ret_code;
  }
  ret_code = GetModifiedBoundary(row_status);
  if (ret_code != UPPL_RC_SUCCESS) {
    return ret_code;
  }
  return UPPL_RC_SUCCESS;
}

/** StartTransaction
 * * @Description : This function is used to start transaction as soon as
                    it is received from TC.
 * * * @param[in] : config_id and session_id
 * * * @return    : Success or associated error code
 * */
UpplReturnCode TransactionRequest::StartTransaction(uint32_t session_id,
                                                    uint32_t config_id) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();
  vector<string>vec_controller_oflow;
  vector<string>vec_controller_vnp;

  pfc_log_info("TransactionRequest::StartTransaction-------");
  pfc_log_debug("trans_state()= %d", itc_trans->trans_state());
  if (itc_trans->trans_state() == TRANS_END) {
    pfc_log_debug("Inside itc_trans->trans_state() == TRANS_END");
    itc_trans->set_trans_state(TRANS_START);
    ClearMaps();
    // Getting the Created configurations
    if ((GetModifiedConfiguration(session_id,
                                  config_id,
                                  CREATED) != UPPL_RC_SUCCESS)) {
      pfc_log_debug("Inside GetCreatedUpdatedConfiguration CREATE !="
          "UPPL_RC_SUCCESS");
      itc_trans->set_trans_state(TRANS_END);
      return UPPL_RC_ERR_TRANSACTION_START;
    }
    // Getting the Updated configurations and sending to driver
    if ((GetModifiedConfiguration(session_id,
                                  config_id, UPDATED) !=  UPPL_RC_SUCCESS)) {
      pfc_log_debug("Inside GetCreatedUpdatedConfiguration UPDATE !="
          "UPPL_RC_SUCCESS");
      itc_trans->set_trans_state(TRANS_END);
      return UPPL_RC_ERR_TRANSACTION_START;
    }
    // Getting the Deleted configurations and sending to driver
    if ((GetModifiedConfiguration(session_id,
                                  config_id,
                                  DELETED) !=  UPPL_RC_SUCCESS)) {
      pfc_log_debug("Inside GetDeletedConfiguration != UPPL_RC_SUCCESS");
      itc_trans->set_trans_state(TRANS_END);
      return UPPL_RC_ERR_TRANSACTION_START;
    }
  } else {
    pfc_log_debug("Inside itc_trans->trans_state() != TRANS_END");
    itc_trans->set_trans_state(TRANS_END);
    return UPPL_RC_ERR_INVALID_TRANSACT_START_REQ;
  }
  // Assigning the modified controllers in vector
  vec_controller_oflow.assign(set_controller_oflow.begin(),
                              set_controller_oflow.end());
  vec_controller_vnp.assign(set_controller_vnp.begin(),
                            set_controller_vnp.end());
  // Storing the  Drivers and associated controllers in the  Map
  if (!vec_controller_oflow.empty()) {
    driver_controller_info_map_.insert(
        std::pair<unc_keytype_ctrtype_t,
        vector<string> >(UNC_CT_PFC, vec_controller_oflow) );
  }
  if (!vec_controller_vnp.empty()) {
    driver_controller_info_map_.insert(
        std::pair<unc_keytype_ctrtype_t,
        vector<string> >(UNC_CT_VNP, vec_controller_vnp) );
  }
  itc_trans->set_trans_state(TRANS_START_SUCCESS);
  pfc_log_debug("TransactionRequest::StartTransaction::trans_state()= %d",
                itc_trans->trans_state());
  pfc_log_info("TransactionRequest::StartTransaction is Successful-------");
  return UPPL_RC_SUCCESS;
}

/** HandleVoteRequest
 * * @Description : This function is used to handle the vote Request.
 * * * @param[in] : config_id, session_id and driver_info
 * * * @return    : Success or associated error code
 * */
UpplReturnCode TransactionRequest::HandleVoteRequest(uint32_t session_id,
                                                     uint32_t config_id,
                                                     TcDriverInfoMap
                                                     &driver_info) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();
  pfc_log_info("TransactionRequest::HandleVoteRequest-------");
  pfc_log_debug("trans_state()= %d", itc_trans->trans_state());

  if (itc_trans->trans_state() == TRANS_START_SUCCESS) {
    pfc_log_debug("Inside itc_trans->trans_state() == TRANS_START_SUCCESS");
    // Function will check whether any audit/import is going on
    itc_trans->set_trans_state(VOTE_BEGIN);
    /* Check the map and store it in the driver info */
    /* PHYSICAL SHOULD NOT SEND UPDATED CONTROLLER LIST
    driver_info = driver_controller_info_map_;
     */
    itc_trans->set_trans_state(VOTE_WAIT_DRIVER_RESULT);
  } else {
    pfc_log_debug("Inside itc_trans->trans_state() != TRANS_START_SUCCESS");
    return UPPL_RC_ERR_VOTE_INVALID_REQ;
  }
  pfc_log_debug("TransactionRequest::VoteRequest:trans_state()= %d",
                itc_trans->trans_state());
  pfc_log_info("TransactionRequest::VoteRequest is Successful-------");
  return UPPL_RC_SUCCESS;
}

/** HandleDriverVoteResult
 * * @Description : This function is used to handle Driver vote result.
 * * * @param[in] : config_id, session_id and driver_id,TcCommitPhaseResult,
 *                  TcControllerTypeRetMap
 * * * @return    : Success or associated error code
 *
 */
UpplReturnCode TransactionRequest::HandleDriverResult(
    uint32_t session_id,
    uint32_t config_id,
    TcCommitPhaseType phase,
    TcCommitPhaseResult driver_result) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();
  pfc_log_info("TransactionRequest::HandleDriverResult-------");
  pfc_log_debug("trans_state()= %d", itc_trans->trans_state());
  pfc_log_debug("phase = %d", phase);

  if (phase == unc::tclib::TC_COMMIT_VOTE_PHASE) {
    pfc_log_debug("Inside Phase == TC_COMMIT_VOTE_PHASE");
    if (itc_trans->trans_state() == VOTE_WAIT_DRIVER_RESULT) {
      pfc_log_debug("itc_trans->trans_state() == VOTE_WAIT_DRIVER_RESULT");
      itc_trans->set_trans_state(VOTE_SUCCESS);
      pfc_log_info(
          "TransactionRequest::HandleDriverResult:VotePhase is Successful----");
      pfc_log_debug("TransactionRequest::HandleDriverResult:trans_state()= %d",
                    itc_trans->trans_state());
    } else {
      return UPPL_RC_ERR_VOTE_INVALID_REQ;
    }
  }
  if (phase == unc::tclib::TC_COMMIT_GLOBAL_COMMIT_PHASE &&
      itc_trans->trans_state() == GLOBAL_COMMIT_WAIT_DRIVER_RESULT) {
    itc_trans->set_trans_state(GLOBAL_COMMIT_DRIVER_RESULT);
    /* Storing the Old values of updated controller */
    vector<key_ctr_t> :: iterator it_controller =
        controller_updated.begin();
    key_ctr_t key_ctr_obj;
    Kt_Controller kt_controller;
    vector<void *> vec_old_val_ctr;
    for (; it_controller != controller_updated.end();) {
      key_ctr_obj = *it_controller;
      pfc_log_debug("HandleDriverResult:GlobalCommitPhase");
      pfc_log_debug("Updated Controller is %s ", key_ctr_obj.controller_name);
      vector<void *> vect_ctr_key, vect_ctr_val;
      vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
      if (kt_controller.ReadInternal(vect_ctr_key, vect_ctr_val,
                                     UNC_DT_RUNNING,
                                     UNC_OP_READ) != UPPL_RC_SUCCESS) {
        // Remove the updated key from updated vector
        controller_updated.erase(it_controller++);
        continue;
      }
      vec_old_val_ctr.push_back(vect_ctr_val[0]);
      // Release memory allocated for key struct
      key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(vect_ctr_key[0]);
      if (ctr_key != NULL) {
        delete ctr_key;
        ctr_key = NULL;
      }
      ++it_controller;
    }
    /* Storing the Old values of updated unknown domain */
    key_ctr_domain_t key_ctr_domain_obj;
    vector<void *> vec_old_val_ctr_domain;
    Kt_Ctr_Domain kt_domain;
    vector<key_ctr_domain_t> :: iterator it_domain = domain_updated.begin();
    for (; it_domain != domain_updated.end();) {
      key_ctr_domain_obj = *it_domain;
      pfc_log_debug("TxnClass:Updated Controller is: %s ",
                    key_ctr_domain_obj.ctr_key.controller_name);
      pfc_log_debug("TxnClass:Updated Domain:s %s ",
                    key_ctr_domain_obj.domain_name);
      vector<void *> vect_domain_key;
      vector<void *> vect_domain_val_st;
      vect_domain_key.push_back(reinterpret_cast<void*>(&key_ctr_domain_obj));
      if (kt_domain.ReadInternal(vect_domain_key,
                                 vect_domain_val_st,
                                 UNC_DT_RUNNING,
                                 UNC_OP_READ) != UPPL_RC_SUCCESS) {
        // Remove the updated key from updated vector
        domain_updated.erase(it_domain++);
        continue;
      }
      vec_old_val_ctr_domain.push_back(vect_domain_val_st[0]);
      // Release memory allocated for key struct
      key_ctr_domain_t *domain_key =
          reinterpret_cast<key_ctr_domain_t*>(vect_domain_key[0]);
      if (domain_key != NULL) {
        delete domain_key;
        domain_key = NULL;
      }
      ++it_domain;
    }
    /* Storing the Old values of updated  boundary */
    key_boundary_t key_boundary_obj;
    vector<void *> vec_old_val_boundary;
    Kt_Boundary kt_boundary;
    vector<key_boundary_t> :: iterator it_boundary =
        boundary_updated.begin();
    for (; it_boundary != boundary_updated.end();) {
      key_boundary_obj = *it_boundary;
      pfc_log_debug("TxnClass:Updated Boundary:  %s ",
                    key_boundary_obj.boundary_id);
      vector<void *> vect_boundary_key;
      vector<void *> vect_boundary_val_st;
      vect_boundary_key.push_back(reinterpret_cast<void*>(&key_boundary_obj));
      if (kt_boundary.ReadInternal(vect_boundary_key, vect_boundary_val_st,
                                   UNC_DT_RUNNING,
                                   UNC_OP_READ) != UPPL_RC_SUCCESS) {
        // Remove the updated key from updated vector
        boundary_updated.erase(it_boundary++);
        continue;
      }
      vec_old_val_boundary.push_back(vect_boundary_val_st[0]);
      // Release memory allocated for key struct
      key_boundary_t *boundary_key =
          reinterpret_cast<key_boundary_t*>(vect_boundary_key[0]);
      if (boundary_key != NULL) {
        delete boundary_key;
        boundary_key = NULL;
      }
      ++it_boundary;
    }
    ODBCM_RC_STATUS db_commit_status = PhysicalLayer::get_instance()->
        get_odbc_manager()->
        CommitAllConfiguration(UNC_DT_CANDIDATE, UNC_DT_RUNNING);
    if (db_commit_status == ODBCM_RC_SUCCESS) {
      pfc_log_info("Configuration Committed Successfully");
    } else {
      pfc_log_fatal("Committing Configuration Failed");
      return UPPL_RC_ERR_FATAL_COPYDB_CANDID_RUNNING;
    }
    // For all deleted controllers, remove the state entries as well
    it_controller = controller_deleted.begin();
    for ( ; it_controller != controller_deleted.end(); ++it_controller) {
      key_ctr_t key_ctr_obj = *it_controller;
      string controller_name =
          reinterpret_cast<char*>(key_ctr_obj.controller_name);
      pfc_log_info("Removing State entries for controller %s",
                   controller_name.c_str());
      ODBCM_RC_STATUS clear_status =
          PhysicalLayer::get_instance()->get_odbc_manager()->
          ClearOneInstance(UNC_DT_STATE, controller_name);
      if (clear_status != ODBCM_RC_SUCCESS) {
        pfc_log_info("State DB clearing failed");
      }
    }
    itc_trans->set_trans_state(GLOBAL_COMMIT_SUCCESS);
    pfc_log_debug("TransactionRequest::HandleDriverResult:trans_state()= %d",
                  itc_trans->trans_state());
    pfc_log_debug(" Transaction is Committed !!!");
    pfc_log_info("Starting to send the Notification after"
        " committing configuration");
    UpplReturnCode notfn_status = SendControllerNotification(vec_old_val_ctr);
    if (notfn_status != UPPL_RC_SUCCESS) {
      return notfn_status;
    }
    notfn_status = SendDomainNotification(vec_old_val_ctr_domain);
    if (notfn_status != UPPL_RC_SUCCESS) {
      return notfn_status;
    }
    notfn_status = SendBoundaryNotification(vec_old_val_boundary);
    if (notfn_status != UPPL_RC_SUCCESS) {
      return notfn_status;
    }
  }
  if (phase != unc::tclib::TC_COMMIT_VOTE_PHASE &&
      phase != unc::tclib::TC_COMMIT_GLOBAL_COMMIT_PHASE) {
    return UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
  }
  return UPPL_RC_SUCCESS;
}

/** HandleGlobalCommitRequest
 * * @Description : This function is used to handle Global Commit Request.
 * * * @param[in] : config_id, session_id and driver_info
 * * * @return    : void
 * */
UpplReturnCode TransactionRequest::HandleGlobalCommitRequest(
    uint32_t session_id,
    uint32_t config_id,
    TcDriverInfoMap
    &driver_info) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans =
      physical_core->get_internal_transaction_coordinator();
  pfc_log_info("TransactionRequest::HandleGlobalCommitRequest-------");
  pfc_log_debug("trans_state()= %d", itc_trans->trans_state());

  if (itc_trans->trans_state() == VOTE_SUCCESS) {
    pfc_log_debug("itc_trans->trans_state() == VOTE_SUCCESS");
    itc_trans->set_trans_state(GLOBAL_COMMIT_BEGIN);
    /* Local validation :  and send the list to TC */
    // Set the map with the stored list in the UpdatePhase
    /* PHYSICAL SHOULD NOT SEND UPDATED CONTROLLER LIST
    driver_info = driver_controller_info_map_;
     */
    itc_trans->set_trans_state(GLOBAL_COMMIT_WAIT_DRIVER_RESULT);
  } else {
    pfc_log_debug("itc_trans->trans_state() != VOTE_SUCCESS");
    return UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
  }
  pfc_log_debug(
      "TransactionRequest::HandleGlobalCommitRequest:trans_state()= %d",
      itc_trans->trans_state());
  return UPPL_RC_SUCCESS;
}

/** AbortTransaction
 * * @Description : This function is used to abort the transaction.
 * * * @param[in] : config_id, session_id , driver_info and operation_phase
 * * * @return    : Success or associated error code
 * */
UpplReturnCode TransactionRequest::AbortTransaction(uint32_t session_id,
                                                    uint32_t config_id,
                                                    TcCommitOpAbortPhase
                                                    operation_phase) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans =
      physical_core->get_internal_transaction_coordinator();
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  pfc_log_info("TransactionRequest::AbortTxn called with opn phase %d",
               operation_phase);
  pfc_log_debug("trans_state()= %d", itc_trans->trans_state());
  DBConfigurationRequest dbconfig;
  // Send the controller info to logical
  result_code = dbconfig.SendDeletedControllerToLogical();
  if (result_code != UPPL_RC_SUCCESS) {
    return result_code;
  }
  result_code = dbconfig.SendCreatedControllerToLogical();
  if (result_code != UPPL_RC_SUCCESS) {
    return result_code;
  }
  result_code = dbconfig.SendUpdatedControllerToLogical();
  if (result_code != UPPL_RC_SUCCESS) {
    return result_code;
  }
  ClearMaps();
  if (operation_phase == unc::tclib::COMMIT_TRANSACTION_START) {
    pfc_log_info("AbortTxn COMMIT_TXN_START - Nothing to do");
  } else if (operation_phase == unc::tclib::COMMIT_VOTE_REQUEST) {
    pfc_log_info("AbortTxn COMMIT_VOTE_REQ - Nothing to do");
  }
  itc_trans-> set_trans_state(TRANS_END);
  return UPPL_RC_SUCCESS;
}

/** EndTransaction
 * * @Description : This function is used to end the transaction.
 * * * @param[in] : None
 * * * @return    : void
 * */
UpplReturnCode TransactionRequest::EndTransaction(uint32_t session_id,
                                                  uint32_t config_id) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();
  pfc_log_info("TransactionRequest::EndTransaction-------");
  pfc_log_debug("trans_state()= %d", itc_trans->trans_state());
  string controller_name, driver_name;
  unc_keytype_ctrtype_t controller_type;
  key_ctr_t key_ctr_obj;
  Kt_Controller kt_controller;
  int err = 0;
  IPCClientDriverHandler vnp_drv_handler(UNC_CT_VNP, err);
  IPCClientDriverHandler pfc_drv_handler(UNC_CT_PFC, err);
  if (err != 0) {
    pfc_log_error("Error in getting driver client session");
    return UPPL_RC_ERR_COMMIT_UPDATE_DRIVER_FAILURE;
  }
  // Sending the 'Delete' Controller Request to Driver
  vector<key_ctr_t> :: iterator it_controller = controller_deleted.begin();
  for ( ; it_controller != controller_deleted.end(); ++it_controller) {
    key_ctr_obj = *it_controller;
    UpplReturnCode logical_result =
        kt_controller.SendUpdatedControllerInfoToUPLL(
            UNC_DT_RUNNING,
            UNC_OP_DELETE,
            UNC_KT_CONTROLLER,
            reinterpret_cast<void*>(&key_ctr_obj),
            NULL);
    pfc_log_debug("Logical return code: %d", logical_result);
    controller_name = reinterpret_cast<char*>(key_ctr_obj.controller_name);
    if (controller_type_map.find(controller_name) !=
        controller_type_map.end()) {
      controller_type =
          (unc_keytype_ctrtype_t)controller_type_map[controller_name];
      if (physical_core->GetDriverName(controller_type, driver_name)
          != UPPL_RC_SUCCESS)  {
        pfc_log_debug("Unable to get the Driver Name from Physical Core");
        continue;
      }
      pfc_log_debug("Controller name is %s and driver name is %s",
                    controller_name.c_str(), driver_name.c_str());
      ClientSession *cli_session = NULL;
      if (controller_type == UNC_CT_PFC) {
        pfc_log_debug("PFC Controller Type");
        cli_session = pfc_drv_handler.ResetAndGetSession();
      } else if (controller_type == UNC_CT_VNP) {
        pfc_log_debug("VNP Controller Type");
        cli_session = vnp_drv_handler.ResetAndGetSession();
      } else {
        pfc_log_debug("DRIVER SUPPORT NOT ADDED YET FOR"
            "UNKNOWN type");
        continue;
      }
      string domain_id;
      driver_request_header rqh = {uint32_t(0), uint32_t(0), controller_name,
          domain_id, UNC_OP_DELETE, uint32_t(0),
          (uint32_t)0, (uint32_t)0, UNC_DT_RUNNING,
          UNC_KT_CONTROLLER};
      int err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
      err |= cli_session->addOutput(key_ctr_obj);
      // Send the request to driver
      UpplReturnCode driver_response = UPPL_RC_SUCCESS;
      driver_response_header rsp;
      if (controller_type == UNC_CT_PFC) {
        driver_response = pfc_drv_handler.SendReqAndGetResp(rsp);
      }
      if (controller_type == UNC_CT_VNP) {
        driver_response = vnp_drv_handler.SendReqAndGetResp(rsp);
      }
      if (err != 0 || driver_response != UPPL_RC_SUCCESS) {
        pfc_log_error("Delete response from driver for controller %s"
            "is %d err=%d", controller_name.c_str(), driver_response, err);
      }
    } else {
      pfc_log_error("Could not able to find type for %s",
                    controller_name.c_str());
    }
  }
  pfc_log_debug("End Trans:Deleted Controller Iterated ");
  // Sending the 'Created' Controller Configuration Notification
  SendControllerInfo(UNC_OP_CREATE, session_id, config_id);
  pfc_log_debug("End Trans:Created Controller Iterated ");
  // Sending the 'Updated' Controller Request to Driver
  SendControllerInfo(UNC_OP_UPDATE, session_id, config_id);
  pfc_log_debug("End Trans:Updated Controller Iterated ");
  itc_trans->set_trans_state(TRANS_END);
  pfc_log_debug("End Trans:Response Code = %d", ret_code);
  return ret_code;
}

/** ClearMaps
 * * @Description : Clear all maps and vectors
 * * * @param[in] : None
 * * * @return    : void
 * */
void TransactionRequest::ClearMaps() {
  /* Clearing the contents of the previously stored controller */
  if (!controller_created.empty()) controller_created.clear();
  if (!controller_updated.empty()) controller_updated.clear();
  if (!controller_deleted.empty()) controller_deleted.clear();
  controller_type_map.clear();
  /* Clearing the contents of the previously stored domain */
  if (!domain_created.empty()) domain_created.clear();
  if (!domain_updated.empty()) domain_updated.clear();
  if (!domain_deleted.empty()) domain_deleted.clear();
  /* Clearing the contents of the previously stored boundaries */
  if (!boundary_created.empty()) boundary_created.clear();
  if (!boundary_updated.empty()) boundary_updated.clear();
  if (!boundary_deleted.empty()) boundary_deleted.clear();
  /* Clearing the contents of the previously stored controllers
   * related to specific type */
  if (!set_controller_oflow.empty()) set_controller_oflow.clear();
  if (!set_controller_vnp.empty())set_controller_vnp.clear();
}

/** SendControllerNotification
 * * @Description : Send notification to north bound for the modified
 * controllers
 * * * @param[in] : vector of old value structures
 * * * @return    : void
 * */
UpplReturnCode TransactionRequest::SendControllerNotification(
    vector<void *> vec_old_val_ctr) {
  // Sending the notification of deleted  controllers
  uint32_t oper_type = UNC_OP_DELETE;
  void *dummy_old_val_ctr = NULL;
  void *dummy_new_val_ctr = NULL;
  vector<key_ctr_t> :: iterator it_controller = controller_deleted.begin();
  Kt_Controller kt_controller;
  for (; it_controller != controller_deleted.end(); ++it_controller) {
    key_ctr_t key_ctr_obj = *it_controller;
    pfc_log_debug("Sending Notification for Deleted Controller: %s",
                  key_ctr_obj.controller_name);
    UpplReturnCode nofn_status =
        kt_controller.ConfigurationChangeNotification(
            (uint32_t)UNC_DT_RUNNING,
            (uint32_t)UNC_KT_CONTROLLER,
            oper_type,
            reinterpret_cast<void*> (&key_ctr_obj),
            dummy_old_val_ctr, dummy_new_val_ctr);
    pfc_log_debug("Notification Status %d", nofn_status);
  }
  // Sending the notification of created controllers
  oper_type = UNC_OP_CREATE;
  it_controller = controller_created.begin();
  for (; it_controller != controller_created.end(); ++it_controller) {
    key_ctr_t key_ctr_obj = *it_controller;
    pfc_log_debug("Sending Notification for Created Controller: %s",
                  key_ctr_obj.controller_name);
    vector<void *> vect_ctr_key, vect_ctr_val;
    vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
    UpplReturnCode retCode = kt_controller.ReadInternal(vect_ctr_key,
                                                        vect_ctr_val,
                                                        UNC_DT_CANDIDATE,
                                                        UNC_OP_READ);
    if (retCode == UPPL_RC_SUCCESS) {
      UpplReturnCode nofn_status =
          kt_controller.ConfigurationChangeNotification(
              (uint32_t)UNC_DT_RUNNING,
              (uint32_t)UNC_KT_CONTROLLER,
              oper_type,
              reinterpret_cast<void*> (&key_ctr_obj),
              dummy_old_val_ctr, vect_ctr_val[0]);
      pfc_log_debug("Notification Status %d", nofn_status);
      // Release memory allocated for key structure
      key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(vect_ctr_key[0]);
      val_ctr_st_t *ctr_val =
          reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]);
      if (ctr_key != NULL) {
        delete ctr_key;
        ctr_key = NULL;
      }
      // delete the val memory
      if (ctr_val != NULL) {
        delete ctr_val;
        ctr_val = NULL;
      }
    } else  if (retCode == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  // Sending the notification of updated Controller
  oper_type = UNC_OP_UPDATE;
  it_controller = controller_updated.begin();
  vector<void *> :: iterator it_ctr_old = vec_old_val_ctr.begin();
  for (; it_controller != controller_updated.end(),
  it_ctr_old != vec_old_val_ctr.end();
  ++it_controller, ++it_ctr_old) {
    key_ctr_t key_ctr_obj = *it_controller;
    pfc_log_debug("Sending Notification for Updated Controller: %s",
                  key_ctr_obj.controller_name);
    vector<void *> vect_ctr_key, vect_ctr_val;
    vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
    UpplReturnCode retCode = kt_controller.ReadInternal(vect_ctr_key,
                                                        vect_ctr_val,
                                                        UNC_DT_CANDIDATE,
                                                        UNC_OP_READ);
    if (retCode == UPPL_RC_SUCCESS) {
      UpplReturnCode nofn_status =
          kt_controller.ConfigurationChangeNotification(
              (uint32_t)UNC_DT_RUNNING,
              (uint32_t)UNC_KT_CONTROLLER,
              oper_type,
              reinterpret_cast<void*> (&key_ctr_obj),
              *it_ctr_old,
              vect_ctr_val[0]);
      pfc_log_debug("Notification Status %d", nofn_status);
      // Release memory allocated for key struct
      key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(vect_ctr_key[0]);
      val_ctr_st_t *ctr_val =
          reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]);
      if (ctr_key != NULL) {
        delete ctr_key;
        ctr_key = NULL;
      }
      // delete the val memory
      if (ctr_val != NULL) {
        delete ctr_val;
        ctr_val = NULL;
      }
    } else  if (retCode == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  return UPPL_RC_SUCCESS;
}

/** SendDomainNotification
 * * @Description : Send notification to north bound for the modified domain
 * * * @param[in] : vector of old value structures
 * * * @return    : void
 * */
UpplReturnCode TransactionRequest::SendDomainNotification(
    vector<void *> vec_old_val_ctr_domain) {
  /*Sending the notification of deleted unknown domain */
  uint32_t oper_type = UNC_OP_DELETE;
  vector<key_ctr_domain_t> :: iterator it_domain = domain_deleted.begin();
  Kt_Ctr_Domain kt_domain;
  for (; it_domain != domain_deleted.end(); ++it_domain) {
    key_ctr_domain_t key_ctr_domain_obj = *it_domain;
    void *key_ctr_domain_ptr = reinterpret_cast<void *>
    (&key_ctr_domain_obj);
    // For deleted domain, update the oper status in boundary
    int ret_notfn = kt_domain.InvokeBoundaryNotifyOperStatus(
        UNC_DT_RUNNING, key_ctr_domain_ptr);
    pfc_log_debug("Boundary Invoke Operation return %d", ret_notfn);
    UpplReturnCode nofn_status =
        kt_domain.ConfigurationChangeNotification(
            (uint32_t)UNC_DT_RUNNING,
            (uint32_t)UNC_KT_CTR_DOMAIN,
            oper_type,
            key_ctr_domain_ptr,
            NULL,
            NULL);
    pfc_log_debug("Notification Status %d", nofn_status);
  }
  // Sending the notification of updated unknown domain
  oper_type = UNC_OP_UPDATE;
  it_domain = domain_updated.begin();
  vector<void *> :: iterator it_domain_old = vec_old_val_ctr_domain.begin();
  pfc_log_debug("TxnClass:Sending Notification for Updated Domain:");
  for (; it_domain != domain_updated.end(),
  it_domain_old != vec_old_val_ctr_domain.end();
  ++it_domain, ++it_domain_old) {
    key_ctr_domain_t key_ctr_domain_obj = *it_domain;
    void *key_ctr_domain_ptr = reinterpret_cast<void *>
    (&key_ctr_domain_obj);
    vector<void *> vect_key_struct;
    vector<void *> vect_new_val;
    vect_key_struct.push_back(key_ctr_domain_ptr);
    pfc_log_debug("TxnClass:Controllername: %s",
                  key_ctr_domain_obj.ctr_key.controller_name);
    UpplReturnCode retCode = kt_domain.ReadInternal(vect_key_struct,
                                                    vect_new_val,
                                                    UNC_DT_CANDIDATE,
                                                    UNC_OP_READ);

    if (retCode == UPPL_RC_SUCCESS) {
      void *val_ctr_domain_new = vect_new_val[0];
      UpplReturnCode nofn_status =
          kt_domain.ConfigurationChangeNotification(
              (uint32_t)UNC_DT_RUNNING,
              (uint32_t)UNC_KT_CTR_DOMAIN,
              oper_type,
              key_ctr_domain_ptr,
              *it_domain_old,
              val_ctr_domain_new);
      pfc_log_debug("Notification Status %d", nofn_status);
      // Clear the value structures
      key_ctr_domain_t *key_domain = reinterpret_cast<key_ctr_domain_t*>
      (vect_key_struct[0]);
      if (key_domain != NULL) {
        delete key_domain;
        key_domain = NULL;
      }
      val_ctr_domain_st_t *val_domain =
          reinterpret_cast<val_ctr_domain_st_t*>(val_ctr_domain_new);
      if (val_domain != NULL) {
        delete val_domain;
        val_domain = NULL;
      }
      // Clear it_domain_old
      val_ctr_domain_st_t *val_domain_old =
          reinterpret_cast<val_ctr_domain_st_t*>(*it_domain_old);
      if (val_domain_old != NULL) {
        delete val_domain_old;
        val_domain_old = NULL;
      }
    } else  if (retCode == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  /* Sending the notification of created unknown domain */
  oper_type = UNC_OP_CREATE;
  it_domain = domain_created.begin();
  pfc_log_debug("TxnClass:Sending Notification for Created Domain:");
  for (; it_domain != domain_created.end(); ++it_domain) {
    key_ctr_domain_t key_ctr_domain_obj = *it_domain;
    vector<void *> vect_key_domain;
    vector<void *> vect_val_domain;
    vect_key_domain.push_back(reinterpret_cast<void*>(&key_ctr_domain_obj));
    // For created domain, update the oper status in boundary
    int ret_notfn = kt_domain.InvokeBoundaryNotifyOperStatus(
        UNC_DT_RUNNING, reinterpret_cast<void *>(&key_ctr_domain_obj));
    pfc_log_debug("Domain Invoke Operation return %d", ret_notfn);

    UpplReturnCode retCode = kt_domain.ReadInternal(vect_key_domain,
                                                    vect_val_domain,
                                                    UNC_DT_CANDIDATE,
                                                    UNC_OP_READ);
    if (retCode == UPPL_RC_SUCCESS) {
      void *val_ctr_domain_new = vect_val_domain[0];
      UpplReturnCode nofn_status =
          kt_domain.ConfigurationChangeNotification(
              (uint32_t)UNC_DT_RUNNING,
              (uint32_t)UNC_KT_CTR_DOMAIN,
              oper_type,
              reinterpret_cast<void *>(&key_ctr_domain_obj),
              NULL,
              val_ctr_domain_new);
      pfc_log_debug("Notification Status %d", nofn_status);
      // Clear the value structures
      key_ctr_domain_t *key_domain = reinterpret_cast<key_ctr_domain_t*>
      (vect_key_domain[0]);
      val_ctr_domain_st_t *val_domain =
          reinterpret_cast<val_ctr_domain_st_t*> (vect_val_domain[0]);
      if (key_domain != NULL) {
        delete key_domain;
        key_domain = NULL;
      }
      if (val_domain != NULL) {
        delete val_domain;
        val_domain = NULL;
      }
    } else  if (retCode == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  return UPPL_RC_SUCCESS;
}

/** SendBoundaryNotification
 * * @Description : Send notification to north bound for the modified
 * boundaries
 * * * @param[in] : vector of old value structures
 * * * @return    : void
 * */
UpplReturnCode TransactionRequest::SendBoundaryNotification(
    vector<void *> vec_old_val_boundary) {
  /*Sending the notification of deleted  boundary */
  Kt_Boundary kt_boundary;
  uint32_t oper_type = UNC_OP_DELETE;
  vector<key_boundary_t> :: iterator it_boundary = boundary_deleted.begin();
  for (; it_boundary != boundary_deleted.end(); ++it_boundary) {
    key_boundary_t key_boundary_obj = *it_boundary;
    void *key_boundary_ptr = reinterpret_cast<void *>(&key_boundary_obj);
    UpplReturnCode nofn_status =
        kt_boundary.ConfigurationChangeNotification(
            (uint32_t)UNC_DT_RUNNING,
            (uint32_t)UNC_KT_BOUNDARY,
            oper_type,
            key_boundary_ptr,
            NULL, NULL);
    pfc_log_debug("Notification Status %d", nofn_status);
  }
  /* Sending the notification of created boundary */
  oper_type = UNC_OP_CREATE;
  it_boundary = boundary_created.begin();
  key_boundary_t new_val_boundary;
  for (; it_boundary != boundary_created.end(); ++it_boundary) {
    key_boundary_t key_boundary_obj = *it_boundary;
    void *key_boundary_ptr = reinterpret_cast<void *>(&key_boundary_obj);
    vector<void *> vect_boundary_key;
    vector<void *> vect_boundary_val_st;
    vect_boundary_key.push_back(reinterpret_cast<void*>(&key_boundary_obj));

    UpplReturnCode retCode = kt_boundary.ReadInternal(vect_boundary_key,
                                                      vect_boundary_val_st,
                                                      UNC_DT_CANDIDATE,
                                                      UNC_OP_READ);
    if (retCode == UPPL_RC_SUCCESS) {
      void *val_boundary_new = reinterpret_cast<void *>
      (&new_val_boundary);
      UpplReturnCode nofn_status =
          kt_boundary.ConfigurationChangeNotification(
              (uint32_t)UNC_DT_RUNNING,
              (uint32_t)UNC_KT_BOUNDARY,
              oper_type,
              key_boundary_ptr, 0,
              val_boundary_new);
      pfc_log_debug("Notification Status %d", nofn_status);
      // clear the key and val memory
      key_boundary_t *key_boundary = reinterpret_cast<key_boundary_t*>
      (vect_boundary_key[0]);
      val_boundary_st_t *val_boundary = reinterpret_cast<val_boundary_st_t*>
      (vect_boundary_val_st[0]);
      if (key_boundary != NULL) {
        delete key_boundary;
        key_boundary = NULL;
      }
      if (val_boundary != NULL) {
        delete val_boundary;
        val_boundary = NULL;
      }
    } else  if (retCode == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  // Sending the notification of updated  boundary
  oper_type = UNC_OP_UPDATE;
  it_boundary = boundary_updated.begin();
  vector<void *> :: iterator it_boundary_old = vec_old_val_boundary.begin();
  for (; it_boundary != boundary_updated.end(),
  it_boundary_old != vec_old_val_boundary.end();
  ++it_boundary, ++it_boundary_old) {
    void *key_boundary_ptr = reinterpret_cast<void *>
    (&(*it_boundary));
    vector<void *> vect_boundary_key;
    vector<void *> vect_boundary_val_st;
    key_boundary_t key_boundary_obj = *it_boundary;
    vect_boundary_key.push_back(reinterpret_cast<void*>(&key_boundary_obj));
    UpplReturnCode retCode = kt_boundary.ReadInternal(vect_boundary_key,
                                                      vect_boundary_val_st,
                                                      UNC_DT_CANDIDATE,
                                                      UNC_OP_READ);
    if (retCode == UPPL_RC_SUCCESS) {
      UpplReturnCode nofn_status =
          kt_boundary.ConfigurationChangeNotification(
              (uint32_t)UNC_DT_RUNNING,
              (uint32_t)UNC_KT_BOUNDARY,
              oper_type,
              key_boundary_ptr,
              *it_boundary_old,
              vect_boundary_val_st[0]);
      pfc_log_debug("Notification Status %d", nofn_status);
      // clear the key and val memory
      key_boundary_t *key_boundary = reinterpret_cast<key_boundary_t*>
      (vect_boundary_key[0]);
      val_boundary_st_t *val_boundary = reinterpret_cast<val_boundary_st_t*>
      (vect_boundary_val_st[0]);
      if (key_boundary != NULL) {
        delete key_boundary;
        key_boundary = NULL;
      }
      if (val_boundary != NULL) {
        delete val_boundary;
        val_boundary = NULL;
      }
      // Clear it_boundary_old
      val_boundary_st_t *val_boundary_old =
          reinterpret_cast<val_boundary_st_t*>(*it_boundary_old);
      if (val_boundary_old != NULL) {
        delete val_boundary_old;
        val_boundary_old = NULL;
      }
    } else  if (retCode == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  return UPPL_RC_SUCCESS;
}

/** SendControllerInfo
 * * @Description : Send controller information to driver
 * * * @param[in] : operation type
 * * * @return    : void
 * */
void TransactionRequest::SendControllerInfo(uint32_t operation_type,
                                            uint32_t session_id,
                                            uint32_t config_id) {
  vector<key_ctr_t> controller_info;
  int err = 0;
  IPCClientDriverHandler pfc_drv_handler(UNC_CT_PFC, err);
  IPCClientDriverHandler vnp_drv_handler(UNC_CT_VNP, err);
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  if (operation_type == UNC_OP_CREATE) {
    controller_info = controller_created;
  } else if (operation_type == UNC_OP_UPDATE) {
    controller_info = controller_updated;
  }
  vector<key_ctr_t> :: iterator it_controller = controller_info.begin();
  Kt_Controller kt_controller;
  for (; it_controller != controller_info.end(); ++it_controller) {
    key_ctr_t key_ctr_obj = *it_controller;
    string controller_name = reinterpret_cast<char *>
    (key_ctr_obj.controller_name);
    pfc_log_debug("End Transaction: Controller name is:  %s",
                  controller_name.c_str());
    vector<void *> vect_key_ctr, vect_ctr_val;
    vect_key_ctr.push_back(reinterpret_cast<void *>(&key_ctr_obj));
    UpplReturnCode retCode = kt_controller.ReadInternal(vect_key_ctr,
                                                        vect_ctr_val,
                                                        UNC_DT_CANDIDATE,
                                                        UNC_OP_READ);
    if (retCode != UPPL_RC_SUCCESS) {
      pfc_log_debug("ReadInternal failed for controller");
      continue;
    }
    unc_keytype_ctrtype_t controller_type = UNC_CT_UNKNOWN;
    string driver_name;
    val_ctr_st_t *val_ctr_new = reinterpret_cast<val_ctr_st_t*>
    (vect_ctr_val[0]);
    if (val_ctr_new == NULL) {
      continue;
    }
    // Inform logical
    UpplReturnCode logical_result =
        kt_controller.SendUpdatedControllerInfoToUPLL(
            UNC_DT_RUNNING,
            operation_type,
            UNC_KT_CONTROLLER,
            vect_key_ctr[0],
            reinterpret_cast<void*>(&val_ctr_new->controller));
    pfc_log_debug("Logical return code: %d", logical_result);
    // Sending the controller info to driver
    controller_type =
        (unc_keytype_ctrtype_t)
        (PhyUtil::uint8touint(val_ctr_new->controller.type));
    if (physical_core->GetDriverName(controller_type, driver_name)
        != UPPL_RC_SUCCESS)  {
      pfc_log_debug("TxnEnd:Unable to get Driver Name from Physical Core");
      delete val_ctr_new;
      val_ctr_new = NULL;
      key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
      (vect_key_ctr[0]);
      if (ctr_key != NULL) {
        delete ctr_key;
        ctr_key = NULL;
      }
      continue;
    }
    pfc_log_debug("Controller name is %s and driver name is %s",
                  controller_name.c_str(), driver_name.c_str());
    ClientSession *cli_session = NULL;
    if (controller_type == UNC_CT_PFC) {
      pfc_log_debug("PFC Controller type");
      cli_session = pfc_drv_handler.ResetAndGetSession();
    } else if (controller_type == UNC_CT_VNP) {
      pfc_log_debug("VNP Controller type");
      cli_session = vnp_drv_handler.ResetAndGetSession();
    } else {
      pfc_log_debug("DRIVER SUPPORT NOT ADDED YET FOR"
          " UNKNOWN type");
      delete val_ctr_new;
      val_ctr_new = NULL;
      key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
      (vect_key_ctr[0]);
      if (ctr_key != NULL) {
        delete ctr_key;
        ctr_key = NULL;
      }
      continue;
    }

    string domain_id;
    driver_request_header rqh = {uint32_t(0), uint32_t(0), controller_name,
        domain_id, operation_type, uint32_t(0),
        (uint32_t)0, (uint32_t)0, UNC_DT_RUNNING,
        UNC_KT_CONTROLLER};

    int err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
    err |= cli_session->addOutput(key_ctr_obj);
    err |= cli_session->addOutput(val_ctr_new->controller);
    // Send the request to driver
    UpplReturnCode driver_response = UPPL_RC_SUCCESS;
    driver_response_header rsp;

    if (controller_type == UNC_CT_PFC) {
      driver_response = pfc_drv_handler.SendReqAndGetResp(rsp);
    }
    if (controller_type == UNC_CT_VNP) {
      driver_response = vnp_drv_handler.SendReqAndGetResp(rsp);
    }
    delete val_ctr_new;
    val_ctr_new = NULL;
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
    (vect_key_ctr[0]);
    if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
    }
    if (err != 0 || driver_response != UPPL_RC_SUCCESS) {
      pfc_log_error("Create request to Driver failed for controller %s"
          " with response %d, err=%d", controller_name.c_str(),
          driver_response, err);
      continue;
    }
  }
}

/** GetModifiedController : This function is used to get the modified
 * controllers from the Candidate Database.
 * @param[in] : row_status
 * @return    : Success or associated error code
 */
UpplReturnCode TransactionRequest::GetModifiedController(
    CsRowStatus row_status) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  unc_keytype_ctrtype_t controller_type;
  pfc_log_info("Get Modified Controller for Row Status: %d", row_status);

  // Gets the Modified Controller Configuration
  Kt_Controller kt_controller;
  vector<void*> vec_key_ctr_modified;
  ret_code = kt_controller.GetModifiedRows(
      vec_key_ctr_modified,
      row_status);
  pfc_log_debug("Controller:GetModifiedRows return code = %d", ret_code);
  if (ret_code == UPPL_RC_ERR_DB_ACCESS) {
    return ret_code;
  }
  for (uint32_t config_count = 0; \
  config_count < vec_key_ctr_modified.size(); config_count++) {
    key_ctr_t *ptr_key_ctr = reinterpret_cast<key_ctr_t *>
    (vec_key_ctr_modified[config_count]);
    if (ptr_key_ctr == NULL) {
      continue;
    }
    string controller_name(reinterpret_cast<char*>(
        ptr_key_ctr->controller_name));
    pfc_log_debug("Controller name is:  %s",
                  ptr_key_ctr->controller_name);
    pfc_bool_t is_controller_recreated = PFC_FALSE;
    // check whether it is already in running
    vector<string> vect_ctr_key_value;
    vect_ctr_key_value.push_back(controller_name);
    UpplReturnCode key_exist_running = kt_controller.IsKeyExists(
        UNC_DT_RUNNING,
        vect_ctr_key_value);
    if (key_exist_running == UPPL_RC_ERR_DB_ACCESS) {
      // Error retrieving information from database, send failure
      pfc_log_info(
          "Error retrieving information from running db, return txn error");
      return UPPL_RC_ERR_TRANSACTION_START;
    } else if (key_exist_running != UPPL_RC_SUCCESS) {
      pfc_log_debug(
          "Controller entry in is not available in running");
    }
    if (row_status == CREATED) {
      controller_created.push_back(*ptr_key_ctr);
      if (key_exist_running == UPPL_RC_SUCCESS) {
        controller_deleted.push_back(*ptr_key_ctr);
        is_controller_recreated = PFC_TRUE;
      }
    }
    if (row_status == UPDATED)
      controller_updated.push_back(*ptr_key_ctr);
    if (row_status == DELETED) {
      if (key_exist_running == UPPL_RC_SUCCESS) {
        controller_deleted.push_back(*ptr_key_ctr);
      }
    }
    //  Freeing the Memory allocated in controller class
    delete ptr_key_ctr;
    ptr_key_ctr = NULL;
    if (PhyUtil::get_controller_type(
        controller_name,
        controller_type, UNC_DT_CANDIDATE) == UPPL_RC_SUCCESS) {
      if (controller_type  == UNC_CT_PFC) {
        set_controller_oflow.insert(controller_name);
      } else if (controller_type  == UNC_CT_VNP) {
        set_controller_vnp.insert(controller_name);
      }
      if (row_status == DELETED) {
        pfc_log_debug(
            "Controller %s of type %d is marked for DELETION",
            controller_name.c_str(), controller_type);
        controller_type_map[controller_name] = controller_type;
      }
    } else {
      pfc_log_info("Error retrieving controller type from candidate");
      return UPPL_RC_ERR_TRANSACTION_START;
    }
    if (is_controller_recreated == PFC_TRUE) {
      // Get existing controller type from RUNNING
      if (PhyUtil::get_controller_type(
          controller_name,
          controller_type, UNC_DT_RUNNING) == UPPL_RC_SUCCESS) {
        pfc_log_debug(
            "Controller %s of type %d is marked for RECREATION",
            controller_name.c_str(), controller_type);
        controller_type_map[controller_name] = controller_type;
      } else {
        pfc_log_info("Error retrieving controller type from running");
        return UPPL_RC_ERR_TRANSACTION_START;
      }
    }
  }
  pfc_log_debug("Modified Controllers Iterated properly");
  return UPPL_RC_SUCCESS;
}

/** GetModifiedDomain : This function is used to get the modified
 * domain from the Candidate Database.
 * @param[in] : row_status
 * @return    : Success or associated error code
 */
UpplReturnCode TransactionRequest::GetModifiedDomain(
    CsRowStatus row_status) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_log_info("Get Modified Domain for Row Status: %d", row_status);
  /* Getting the Modified Unknown Domain Configuration */
  Kt_Ctr_Domain kt_ctr_domain;
  vector<void*> vec_key_ctr_domain_modified;
  ret_code = kt_ctr_domain.GetModifiedRows(
      vec_key_ctr_domain_modified,
      row_status);
  pfc_log_debug("Domain:GetModifiedRows return code = %d", ret_code);
  if (ret_code == UPPL_RC_ERR_DB_ACCESS) {
    return ret_code;
  }
  for (uint32_t config_count = 0; \
  config_count < vec_key_ctr_domain_modified.size(); config_count++) {
    key_ctr_domain_t *ptr_key_ctr_domain =
        reinterpret_cast<key_ctr_domain_t *>
    (vec_key_ctr_domain_modified[config_count]);
    if (ptr_key_ctr_domain == NULL) {
      continue;
    }
    string domain_name(reinterpret_cast<char*>(
        ptr_key_ctr_domain->domain_name));
    pfc_log_debug("Start Transaction: Controller name is:  %s",
                  ptr_key_ctr_domain->ctr_key.controller_name);
    pfc_log_debug("Start Transaction: Domain name is:  %s",
                  ptr_key_ctr_domain->domain_name);
    if (row_status == CREATED)
      domain_created.push_back(*ptr_key_ctr_domain);
    if (row_status == UPDATED)
      domain_updated.push_back(*ptr_key_ctr_domain);
    if (row_status == DELETED) {
      // check whether it is already in running
      vector<string> vect_domain_key_value;
      vect_domain_key_value.push_back(
          (const char*)ptr_key_ctr_domain->ctr_key.controller_name);
      vect_domain_key_value.push_back(domain_name);
      UpplReturnCode key_exist_running = kt_ctr_domain.IsKeyExists(
          UNC_DT_RUNNING,
          vect_domain_key_value);
      if (key_exist_running == UPPL_RC_SUCCESS) {
        domain_deleted.push_back(*ptr_key_ctr_domain);
      } else if (key_exist_running == UPPL_RC_ERR_DB_ACCESS) {
        // Error retrieving information from database, send failure
        pfc_log_info(
            "Error retrieving information from running db, return txn error");
        return UPPL_RC_ERR_TRANSACTION_START;
      } else {
        pfc_log_debug(
            "Deleted entry in candidate is not available in running-ignoring");
      }
    }
    //  Freeing the Memory allocated in Domain class
    delete ptr_key_ctr_domain;
    ptr_key_ctr_domain = NULL;
  }
  pfc_log_debug("Modified Domain iterated properly");
  return UPPL_RC_SUCCESS;
}

/** GetModifiedBoundary : This function is used to get the modified
 * boundary from the Candidate Database.
 * @param[in] : row_status
 * @return    : Success or associated error code
 */
UpplReturnCode TransactionRequest::GetModifiedBoundary(
    CsRowStatus row_status) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_log_info("Get Modified Boundary for Row Status: %d", row_status);
  //  Getting the Modified Boundary Configuration
  Kt_Boundary kt_boundary;
  vector<void*> vec_key_boundary_modified;
  ret_code = kt_boundary.GetModifiedRows(
      vec_key_boundary_modified,
      row_status);
  pfc_log_debug("Controller:GetModifiedRows return code = %d", ret_code);
  if (ret_code == UPPL_RC_ERR_DB_ACCESS) {
    return ret_code;
  }
  for (uint32_t config_count = 0; \
  config_count < vec_key_boundary_modified.size(); config_count++) {
    key_boundary_t *ptr_key_boundary = reinterpret_cast<key_boundary_t *>
    (vec_key_boundary_modified[config_count]);
    if (ptr_key_boundary == NULL) {
      continue;
    }
    if (row_status == CREATED)
      boundary_created.push_back(*ptr_key_boundary);
    if (row_status == UPDATED)
      boundary_updated.push_back(*ptr_key_boundary);
    if (row_status == DELETED) {
      // check whether it is already in running
      vector<string> vect_bdry_key_value;
      vect_bdry_key_value.push_back(
          (const char*)ptr_key_boundary->boundary_id);
      UpplReturnCode key_exist_running = kt_boundary.IsKeyExists(
          UNC_DT_RUNNING,
          vect_bdry_key_value);
      if (key_exist_running == UPPL_RC_SUCCESS) {
        boundary_deleted.push_back(*ptr_key_boundary);
      } else if (key_exist_running == UPPL_RC_ERR_DB_ACCESS) {
        // Error retrieving information from database, send failure
        pfc_log_info(
            "Error retrieving information from running db, return txn error");
        return UPPL_RC_ERR_TRANSACTION_START;
      } else {
        pfc_log_debug(
            "Deleted entry in candidate is not available in running-ignoring");
      }
    }
    //  Freeing the Memory allocated in Boundary class
    delete ptr_key_boundary;
    ptr_key_boundary = NULL;
  }
  pfc_log_debug("Modified Boundary iterated properly");
  return UPPL_RC_SUCCESS;
}
