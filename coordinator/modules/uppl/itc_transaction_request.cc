/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "ipct_util.hh"
#include "itc_audit_request.hh"

using unc::tclib::TcLibModule;
using unc::tclib::TcControllerResult;

/**TransactionRequest
 * @Description : This function initializes the member data
 * @param[in]   : None
 * @return      : None
 * */
TransactionRequest::TransactionRequest() {
}

/**~TransactionRequest
 * @Description : This function release any memory allocated to a
 *                pointer member data.
 * @param[in]   : None
 * @return      : None
 * */
TransactionRequest::~TransactionRequest() {
}

/** GetModifiedConfiguration 
 * @Description : This function is used to get the modified configurations
 *                from the Candidate Database with respect to row status 
 * @param[in]   : row_status - Denotes the row status of the kt_controller,
 *                kt_domain and kt_boundary
 * @return      : UNC_RC_SUCCESS if the modified configuration obtained or
 *                UNC_UPPL_RC_ERR_* for failure
 */
UncRespCode TransactionRequest::GetModifiedConfiguration(
    OdbcmConnectionHandler *db_conn,
    CsRowStatus row_status, TcConfigMode config_mode) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  ret_code = GetModifiedController(db_conn, row_status, config_mode);
  if (ret_code != UNC_RC_SUCCESS) {
    return ret_code;
  }
  ret_code = GetModifiedDomain(db_conn, row_status);
  if (ret_code != UNC_RC_SUCCESS) {
    return ret_code;
  }
  ret_code = GetModifiedBoundary(db_conn, row_status, config_mode);
  if (ret_code != UNC_RC_SUCCESS) {
    return ret_code;
  }
  return UNC_RC_SUCCESS;
}

/** StartTransaction
 * @Description : This function is called when Transaction Start is received
 *                from TC. The trans_state_ will be initialized to TRANS_START
 *                if the current trans_state is TRANS_END, audit state is
 *                AUDIT_END and import_state is IMPORT_END.If not, error will
 *                be returned to TC.Then get the modified configuration from
 *                the candidate database and send the configuration to driver
 * @param[in]   : session_id - ipc session id used for TC validation
 *                config_id - configuration id used for TC validation
 * @return      : UNC_RC_SUCCESS if the start Transaction is success and set
 *                the trans_state as TRANS_START_SUCCESS or returns
 *                UNC_UPPL_RC_ERR_* if the start transaction is failed and set
 *                the trans_state as TRANS_START_FAILURE
 * */
UncRespCode TransactionRequest::StartTransaction(
    OdbcmConnectionHandler *db_conn,
    uint32_t session_id,
    uint32_t config_id,
    TcConfigMode config_mode) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();

  // If config_mode is not real-network or Global mode, return success
  if ((config_mode != TC_CONFIG_REAL) &&
                      (config_mode != TC_CONFIG_GLOBAL)) {
    itc_trans->set_trans_state(TRANS_START_SUCCESS);
    return UNC_RC_SUCCESS;
  }
  pfc_log_info("TransactionRequest::StartTransaction");
  pfc_log_debug("trans_state()= %d", itc_trans->trans_state());
  if (itc_trans->trans_state() == TRANS_END) {
    pfc_log_debug("Inside itc_trans->trans_state() == TRANS_END");
    itc_trans->set_trans_state(TRANS_START);
    ClearMaps();
    // Getting the Created configurations
    if ((GetModifiedConfiguration(db_conn,
                                  CREATED, config_mode) != UNC_RC_SUCCESS)) {
      pfc_log_debug("Inside GetCreatedUpdatedConfiguration CREATE !="
          "UNC_RC_SUCCESS");
      itc_trans->set_trans_state(TRANS_END);
      return UNC_UPPL_RC_ERR_TRANSACTION_START;
    }
    // Getting the Updated configurations and sending to driver
    if ((GetModifiedConfiguration(db_conn,
                                  UPDATED, config_mode) !=  UNC_RC_SUCCESS)) {
      pfc_log_debug("Inside GetCreatedUpdatedConfiguration UPDATE !="
          "UNC_RC_SUCCESS");
      itc_trans->set_trans_state(TRANS_END);
      return UNC_UPPL_RC_ERR_TRANSACTION_START;
    }
    // Getting the Deleted configurations and sending to driver
    if ((GetModifiedConfiguration(db_conn,
                                  DELETED, config_mode) !=  UNC_RC_SUCCESS)) {
      pfc_log_debug("Inside GetDeletedConfiguration != UNC_RC_SUCCESS");
      itc_trans->set_trans_state(TRANS_END);
      return UNC_UPPL_RC_ERR_TRANSACTION_START;
    }
    // Update the commit version as 0 for the create controllers in candidate
    UpdateCommitVersion(db_conn);
  } else {
    pfc_log_info("Inside itc_trans->trans_state() != TRANS_END");
    itc_trans->set_trans_state(TRANS_END);
    return UNC_UPPL_RC_ERR_INVALID_TRANSACT_START_REQ;
  }
  itc_trans->set_trans_state(TRANS_START_SUCCESS);
  pfc_log_debug("TransactionRequest::StartTransaction::trans_state()= %d",
                itc_trans->trans_state());
  return UNC_RC_SUCCESS;
}

/** UpdateCommitVersion 
 * @Descritption: This function updated the commit version of the create 
 *                controller
 *                in the canidate database
 * @param[in]   : None 
 * @param[out]  : None 
 * @return      : UNC_RC_SUCCESS if update of commit version in candidate is
 *                successfull else return the error code
*/ 

void TransactionRequest::UpdateCommitVersion(OdbcmConnectionHandler *db_conn) {
  vector<key_ctr_t>::iterator it_ctr;
  it_ctr = controller_created_.begin();
  while (it_ctr < controller_created_.end()) {
    //  val structure for commit version
    val_ctr_commit_ver_t ctr_commit_val;
    memset(&ctr_commit_val, 0, sizeof(val_ctr_commit_ver_t));
    ctr_commit_val.valid[kIdxCtrCommitNumber] = UNC_VF_VALID;
    ctr_commit_val.valid[kIdxCtrCommitDate] = UNC_VF_VALID;
    ctr_commit_val.valid[kIdxCtrCommitApplication] = UNC_VF_VALID;

    Kt_Controller kt_ctr;
    key_ctr_t ctr_key = *it_ctr;
    UncRespCode update_status = kt_ctr.UpdateKeyInstance(db_conn,
                 &ctr_key,
                 &ctr_commit_val,
                 UNC_DT_CANDIDATE, UNC_KT_CONTROLLER,
                 (pfc_bool_t)true/*commit version update*/);
    if (update_status != UNC_RC_SUCCESS) {
      pfc_log_error(
          "comm-ver update failed for ctr:%s with status:%d",
              ctr_key.controller_name, update_status);
    } else {
      pfc_log_debug("Updated comm-ver in candidate");
    }
    it_ctr++;
  }
}

/**HandleVoteRequest
 * @Description : This function is invoked when the HandleVoteRequest is
 *                received from TC. This function checks whether trans_state_
 *                has the TRANS_START_SUCCESS as precondition, and sets the
 *                trans_state_ to VOTE_WAIT_DRIVER_RESULT .Sends the updated
 *                Controllers list to TC. In case of error, returns error to TC
 * @param[in]   : session_id - ipc session id used for TC validation
 *                config_id - configuration id used for TC validation
 * @param[out]  : driver_info - map that contains the updated controller list
 * @return      : UNC_RC_SUCCESS if VoteRequest is successfull or
 *                UNC_UPPL_RC_ERR_* if VoteRequest is failed
 * */
UncRespCode TransactionRequest::HandleVoteRequest(uint32_t session_id,
                                                     uint32_t config_id,
                                                     TcConfigMode config_mode,
                                                     TcDriverInfoMap
                                                     &driver_info) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();
  // If config_mode is not real-network or Global mode, return success
  if ((config_mode != TC_CONFIG_REAL) &&
                      (config_mode != TC_CONFIG_GLOBAL)) {
    itc_trans->set_trans_state(VOTE_WAIT_DRIVER_RESULT);
    return UNC_RC_SUCCESS;
  }

  pfc_log_debug("trans_state()= %d", itc_trans->trans_state());

  if (itc_trans->trans_state() == TRANS_START_SUCCESS) {
    pfc_log_debug("Inside itc_trans->trans_state() == TRANS_START_SUCCESS");
    // Function will check whether any audit/import is going on
    itc_trans->set_trans_state(VOTE_BEGIN);
    itc_trans->set_trans_state(VOTE_WAIT_DRIVER_RESULT);
  } else {
    pfc_log_info("Inside itc_trans->trans_state() != TRANS_START_SUCCESS");
    return UNC_UPPL_RC_ERR_VOTE_INVALID_REQ;
  }
  pfc_log_debug("TransactionRequest::VoteRequest:trans_state()= %d",
                itc_trans->trans_state());
  return UNC_RC_SUCCESS;
}

/** HandleDriverResult
 * @Description : This function is invoked when HandleDriverResult is received
 *                from TC.This function used to handle Driver vote result based
 *                on CommitPhase type. Transction is committed if the phase is
 *                TC_COMMIT_GLOBAL_COMMIT_PHASE and sets the trans_state to
 *                GLOBAL_COMMIT_SUCCESS
 * @param[in]   : session_id - ipc session id used for TC validation
 *                config_id - configuration id used for TC validation
 *                phase - specifies the TC commit phase type.Its a enum value
 *                driver_result - specifies the TC commit phase result.
 *                Its a enum value
 * @return      : UNC_RC_SUCCESS if the HandleDriverResult is successful or
 *                returns UNC_UPPL_RC_ERR_* if HandleDriverResult is failed
 **/
UncRespCode TransactionRequest::HandleDriverResult(
    OdbcmConnectionHandler *db_conn,
    uint32_t session_id,
    uint32_t config_id,
    TcConfigMode config_mode,
    TcCommitPhaseType phase,
    TcCommitPhaseResult driver_result) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();
  if (phase != unc::tclib::TC_COMMIT_VOTE_PHASE &&
      phase != unc::tclib::TC_COMMIT_GLOBAL_COMMIT_PHASE) {
    return UNC_UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
  }

  if (phase == unc::tclib::TC_COMMIT_VOTE_PHASE) {
    if (itc_trans->trans_state() == VOTE_WAIT_DRIVER_RESULT) {
      // If config_mode is not real-network or Global mode, return success
      if ((config_mode != TC_CONFIG_REAL) &&
                           (config_mode != TC_CONFIG_GLOBAL)) {
        itc_trans->set_trans_state(VOTE_SUCCESS);
        return UNC_RC_SUCCESS;
      }
      itc_trans->set_trans_state(VOTE_SUCCESS);
    } else {
      pfc_log_info("TransactionRequest::HandleDriverResult:VOTE_INVALID_REQ");
      return UNC_UPPL_RC_ERR_VOTE_INVALID_REQ;
    }
  }
  if (phase == unc::tclib::TC_COMMIT_GLOBAL_COMMIT_PHASE &&
      itc_trans->trans_state() == GLOBAL_COMMIT_WAIT_DRIVER_RESULT) {
    itc_trans->set_trans_state(GLOBAL_COMMIT_DRIVER_RESULT);

    //  store commit version (3 attributes) into controller cand,running table
    std::vector<TcControllerResult>::iterator viter = driver_result.begin();
    Kt_Controller kt_ctr;
    pfc_log_debug("size of driver_result is %" PFC_PFMT_SIZE_T ,
                   driver_result.size());
    // Failover Scenario: Resetting the commit version
    if ((driver_result.size() == 0) && (session_id == 0) && (config_id == 0)) {
      pfc_log_info("Handling commit version:Failover sceario");
      Kt_Controller kt_ctr;
      vector<void *> vect_ctr_key, vect_ctr_val;
      key_ctr_t key_ctr_obj;
      memset(&key_ctr_obj, '\0', sizeof(key_ctr_t));
      vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));

      // Getting the controller list from running Database
      UncRespCode read_status = kt_ctr.ReadInternal(db_conn, vect_ctr_key,
                                                   vect_ctr_val,
                                                   UNC_DT_RUNNING,
                                                   UNC_OP_READ_SIBLING_BEGIN);
      if (read_status != UNC_RC_SUCCESS &&
                      read_status != UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
        pfc_log_info("read from RUNNING db is failed with status %d",
                      read_status);
        return read_status;
      }
      UncRespCode update_status = UNC_RC_SUCCESS;
      for (uint32_t ctrIndex = 0; ctrIndex < vect_ctr_key.size();
           ctrIndex ++) {
          key_ctr_t *ctr_key =
            reinterpret_cast<key_ctr_t*>(vect_ctr_key[ctrIndex]);
        string controller_name = (const char*)ctr_key->controller_name;
        pfc_log_debug("controller_name: %s", controller_name.c_str());
        val_ctr_commit_ver_t ctrlr_val;
        memset(&ctrlr_val, 0, sizeof(val_ctr_commit_ver_t));
        //  key, val structure framing
        ctrlr_val.valid[kIdxCtrCommitNumber] = UNC_VF_VALID;
        ctrlr_val.valid[kIdxCtrCommitDate] = UNC_VF_VALID;
        ctrlr_val.valid[kIdxCtrCommitApplication] = UNC_VF_VALID;
        update_status = kt_ctr.UpdateKeyInstance(db_conn,
                      reinterpret_cast<void*>(ctr_key),
                     reinterpret_cast<void*>(&ctrlr_val),
                     UNC_DT_RUNNING, UNC_KT_CONTROLLER,
                     (pfc_bool_t)true/*commit version update*/);
        if (update_status != UNC_RC_SUCCESS) {
          pfc_log_error(
              "commit version update is failed for controller %s, status: %d",
                         key_ctr_obj.controller_name, update_status);
          break;
        }
      }  // for loop close
      for (uint32_t ctrIndex = 0; ctrIndex < vect_ctr_key.size();
           ctrIndex++) {
        key_ctr_t *ctr_key =
                   reinterpret_cast<key_ctr_t*>(vect_ctr_key[ctrIndex]);
        if (ctr_key != NULL) {
          delete ctr_key;
          ctr_key = NULL;
        }
        val_ctr_st_t *ctr_val =
            reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[ctrIndex]);
        if (ctr_val != NULL) {
          delete ctr_val;
          ctr_val = NULL;
        }
      }
      if (update_status != UNC_RC_SUCCESS) {
        return update_status;
      }
    }
    // Normal Scenario:Updating the Commit version
    viter = driver_result.begin();
    for ( ; viter != driver_result.end(); viter++) {
      TcControllerResult tcResult = *viter;
      //  key to update the commit version
      key_ctr_t key_ctr_obj;
      //  val structure for commit version
      val_ctr_commit_ver_t ctrlr_val;
      memset(&key_ctr_obj, 0, sizeof(key_ctr_t));
      memset(&ctrlr_val, 0, sizeof(val_ctr_commit_ver_t));
      //  key, val structure framing
      memcpy(&key_ctr_obj.controller_name, tcResult.controller_id.c_str(),
                            tcResult.controller_id.length()+1);
      ctrlr_val.commit_number = tcResult.commit_number;
      ctrlr_val.commit_date = tcResult.commit_date;
      memcpy(&ctrlr_val.commit_application,
             tcResult.commit_application.c_str(),
             tcResult.commit_application.length()+1);
      ctrlr_val.valid[kIdxCtrCommitNumber] = UNC_VF_VALID;
      ctrlr_val.valid[kIdxCtrCommitDate] = UNC_VF_VALID;
      ctrlr_val.valid[kIdxCtrCommitApplication] = UNC_VF_VALID;
      ctrlr_val.valid[kIdxCtrVal] = UNC_VF_VALID;
      pfc_log_info("cname:%s Commit_number:%" PFC_PFMT_u64
                   ", date is %" PFC_PFMT_u64 ", app:%s",
                tcResult.controller_id.c_str(),
                ctrlr_val.commit_number, ctrlr_val.commit_date,
                ctrlr_val.commit_application);
      UncRespCode update_status = kt_ctr.UpdateKeyInstance(db_conn,
                    reinterpret_cast<void*>(&key_ctr_obj),
                   reinterpret_cast<void*>(&ctrlr_val),
                   UNC_DT_CANDIDATE, UNC_KT_CONTROLLER,
                   (pfc_bool_t)true/*commit version update*/);
      if (update_status != UNC_RC_SUCCESS) {
        ClearMaps();
        if (update_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
          return update_status;  // fatal is already logged inside update fn
        } else {
          UPPL_LOG_FATAL(
            "commit version update is FAILED for controller %s, status: %d",
                       key_ctr_obj.controller_name, update_status);
          return UNC_UPPL_RC_ERR_DB_UPDATE;
        }
      }
      update_status = kt_ctr.UpdateKeyInstance(db_conn,
                    reinterpret_cast<void*>(&key_ctr_obj),
                   reinterpret_cast<void*>(&ctrlr_val),
                   UNC_DT_RUNNING, UNC_KT_CONTROLLER,
                   (pfc_bool_t)true/*commit version update*/);
      if (update_status != UNC_RC_SUCCESS) {
        if (update_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
          ClearMaps();
          return update_status;  // fatal is already logged inside update fn
        } else {
          pfc_bool_t isFreshCtrlr = false;
          vector<key_ctr_t> :: iterator iter;
          for (iter = controller_created_.begin();
              iter != controller_created_.end(); iter ++) {
            key_ctr_t k = *iter;
            if (strcmp(tcResult.controller_id.c_str(),
                      (const char *)k.controller_name) == 0) {
              pfc_log_debug("fresh contrlr true");
              isFreshCtrlr = true;
              break;
            }
          }
          if (isFreshCtrlr) {
          pfc_log_info(
            "ignore commit version update for fresh ctr %s, status: %d",
                       key_ctr_obj.controller_name, update_status);
          } else {
          ClearMaps();
          UPPL_LOG_FATAL(
            "commit version update is failed for controller %s, status: %d",
                       key_ctr_obj.controller_name, update_status);
          return UNC_UPPL_RC_ERR_DB_UPDATE;
          }
        }
      }
    }
    // If config_mode is not real-network or Global mode, return success
    if ((config_mode != TC_CONFIG_REAL) &&
                             (config_mode != TC_CONFIG_GLOBAL)) {
      itc_trans->set_trans_state(GLOBAL_COMMIT_SUCCESS);
      return UNC_RC_SUCCESS;
    }
    // Checking whether there is any modified configuration
    if (controller_created_.empty() && controller_deleted_.empty() &&
        controller_updated_.empty() && domain_created_.empty()&&
        domain_deleted_.empty() && domain_updated_.empty() &&
        boundary_created_.empty() && boundary_deleted_.empty() &&
        boundary_updated_.empty()) {
      itc_trans->set_trans_state(GLOBAL_COMMIT_SUCCESS);
      pfc_log_debug("CommitPhase:There are no modified configurations\n");
      return UNC_RC_SUCCESS;
    }

/*
    // check whether it is already in running db otherwise delete
    // the entries from the vector
    vector<key_ctr_t> :: iterator it_controller1 =
      controller_deleted.begin();
    for ( ; it_controller1 != controller_deleted.end();) {
       pfc_log_debug(
          "Checking the deleted entries in running database:Inside for loop");

      key_ctr_t key_ctr_obj = *it_controller1;
      vector<string> vect_ctr_key_value;
      string controller_name = reinterpret_cast<char *>
        (key_ctr_obj.controller_name);
      pfc_log_debug(
           "DeletedControlleris %s", key_ctr_obj.controller_name);
      vect_ctr_key_value.push_back(controller_name);
      Kt_Controller kt_controller;
      UncRespCode key_exist_running = kt_controller.IsKeyExists(
        db_conn, UNC_DT_RUNNING,
        vect_ctr_key_value);
      if (key_exist_running == UNC_RC_SUCCESS) {
        ++it_controller1;
        continue;
      } else if (key_exist_running == UNC_UPPL_RC_ERR_DB_ACCESS) {
        UPPL_LOG_FATAL(
          "TransactionRequest:Connection Error");
        return UNC_UPPL_RC_ERR_DB_ACCESS;
      } else {
        pfc_log_debug(
           "Controller entry is not available in running");
        it_controller1  = controller_deleted.erase(it_controller1);
      }
    }
*/
    /* Storing the Old values of deleted controller */
    vector<key_ctr_t> :: iterator it_ctr =
        controller_deleted_.begin();
    key_ctr_t key_controller_obj;
    for (; it_ctr != controller_deleted_.end();) {
      key_controller_obj = *it_ctr;
      pfc_log_debug("HandleDriverResult:GlobalCommitPhase");
      pfc_log_debug("Deleted Controller is %s ",
                        key_controller_obj.controller_name);
      vector<void *> vect_ctrlr_key, vect_ctrlr_val;
      vect_ctrlr_key.push_back(reinterpret_cast<void *>(&key_controller_obj));
      if (kt_ctr.ReadInternal(db_conn, vect_ctrlr_key, vect_ctrlr_val,
                                     UNC_DT_RUNNING,
                                     UNC_OP_READ) != UNC_RC_SUCCESS) {
        it_ctr = controller_deleted_.erase(it_ctr);
        continue;
      }
      vector<string> vect_controller_id;
      vector<val_ctr_commit_ver_t> vect_val_ctr_cv;
      uint32_t max_rep_ct = 1;
      UncRespCode read_status = kt_ctr.ReadCtrCommitVerFromDB(db_conn,
                               reinterpret_cast<void*>(&key_controller_obj),
                               NULL,
                               UNC_DT_RUNNING, UNC_OP_READ,
                               max_rep_ct, vect_val_ctr_cv, vect_controller_id);
      if (read_status != UNC_RC_SUCCESS) {
        pfc_log_info("Error in read commit version from DB");
        continue;
      }

      val_ctr_commit_ver_t ctr_val_struct =  vect_val_ctr_cv[0];
      val_ctr_st_t val_st =
                  *(reinterpret_cast<val_ctr_st_t*>(vect_ctrlr_val[0]));
      ctr_val_struct.controller = val_st.controller;

      string ctr_name =
                reinterpret_cast<char*>(key_controller_obj.controller_name);
      // Storing the old updated value of controller in the map
      controller_old_del_val_[ctr_name] = ctr_val_struct;
      // Release memory allocated for key struct
      key_ctr_t *ctrlr_key = reinterpret_cast<key_ctr_t*>(vect_ctrlr_key[0]);
      if (ctrlr_key != NULL) {
        delete ctrlr_key;
        ctrlr_key = NULL;
      }
      val_ctr_st_t *ctrlr_val =
                    reinterpret_cast<val_ctr_st_t*>(vect_ctrlr_val[0]);
      if (ctrlr_val != NULL) {
        delete ctrlr_val;
        ctrlr_val = NULL;
      }
      ++it_ctr;
    }
    /* Storing the Old values of updated controller */
    vector<key_ctr_t> :: iterator it_controller =
        controller_updated_.begin();
    key_ctr_t key_ctr_obj;
    Kt_Controller kt_controller;
    vector<void *> vec_old_val_ctr;
    for (; it_controller != controller_updated_.end();) {
      key_ctr_obj = *it_controller;
      pfc_log_debug("HandleDriverResult:GlobalCommitPhase");
      pfc_log_debug("Updated Controller is %s ", key_ctr_obj.controller_name);
      vector<void *> vect_ctr_key, vect_ctr_val;
      vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
      if (kt_controller.ReadInternal(db_conn, vect_ctr_key, vect_ctr_val,
                                     UNC_DT_RUNNING,
                                     UNC_OP_READ) != UNC_RC_SUCCESS) {
        // Remove the updated key from updated vector
        it_controller = controller_updated_.erase(it_controller);
        continue;
      }
      vector<string> vect_controller_id;
      vector<val_ctr_commit_ver_t> vect_val_ctr_cv;
      uint32_t max_rep_ct = 1;
      UncRespCode read_status = kt_controller.ReadCtrCommitVerFromDB(db_conn,
                               reinterpret_cast<void*>(&key_ctr_obj),
                               NULL,
                               UNC_DT_RUNNING, UNC_OP_READ,
                               max_rep_ct, vect_val_ctr_cv, vect_controller_id);
      if (read_status != UNC_RC_SUCCESS) {
        pfc_log_info("Error in read commit version from DB");
        continue;
      }

    val_ctr_commit_ver_t ctr_val_struct =  vect_val_ctr_cv[0];
    val_ctr_st_t val_st =  *(reinterpret_cast<val_ctr_st_t*>(vect_ctr_val[0]));
    ctr_val_struct.controller = val_st.controller;
    ctr_val_struct.valid[kIdxCtrVal] = UNC_VF_VALID;

    vec_old_val_ctr.push_back(reinterpret_cast<val_ctr_st_t*>(
                                vect_ctr_val[0]));  // for northbound
      string controller_name =
                reinterpret_cast<char*>(key_ctr_obj.controller_name);
      // Storing the old updated value of controller in the map
      controller_old_upd_val_[controller_name] =
                 ctr_val_struct;  // for driver
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
    vector<key_ctr_domain_t> :: iterator it_domain = domain_updated_.begin();
    for (; it_domain != domain_updated_.end();) {
      key_ctr_domain_obj = *it_domain;
      pfc_log_debug("TxnClass:Updated Controller is: %s ",
                    key_ctr_domain_obj.ctr_key.controller_name);
      pfc_log_debug("TxnClass:Updated Domain:s %s ",
                    key_ctr_domain_obj.domain_name);
      vector<void *> vect_domain_key;
      vector<void *> vect_domain_val_st;
      vect_domain_key.push_back(reinterpret_cast<void*>(&key_ctr_domain_obj));
      if (kt_domain.ReadInternal(db_conn, vect_domain_key,
                                 vect_domain_val_st,
                                 UNC_DT_RUNNING,
                                 UNC_OP_READ) != UNC_RC_SUCCESS) {
        // Remove the updated key from updated vector
        it_domain = domain_updated_.erase(it_domain);
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
    vector<key_boundary_t> :: iterator it_boundary;
    key_boundary_t key_boundary_obj;
    Kt_Boundary kt_boundary;
    vector<void *> vec_old_val_boundary;
    {  // scope area for eventDoneLock
    ScopedReadWriteLock eventDoneLock(PhysicalLayer::get_events_done_lock_(),
                                    PFC_TRUE);  // write lock

    /* Storing the Old values of updated  boundary to send notification*/
    it_boundary = boundary_updated_.begin();
    for (; it_boundary != boundary_updated_.end();) {
      key_boundary_obj = *it_boundary;
      pfc_log_debug("TxnClass:Updated Boundary:  %s ",
                    key_boundary_obj.boundary_id);
      vector<void *> vect_boundary_key;
      vector<void *> vect_boundary_val_st;
      vect_boundary_key.push_back(reinterpret_cast<void*>(&key_boundary_obj));
      if (kt_boundary.ReadInternal(db_conn, vect_boundary_key,
                                   vect_boundary_val_st,
                                   UNC_DT_RUNNING,
                                   UNC_OP_READ) != UNC_RC_SUCCESS) {
        // Remove the updated key from updated vector
        it_boundary = boundary_updated_.erase(it_boundary);
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
        CommitAllConfiguration(UNC_DT_CANDIDATE, UNC_DT_RUNNING, db_conn);
    if (db_commit_status == ODBCM_RC_SUCCESS) {
      pfc_log_info("Configuration Committed Successfully");
    } else if (db_commit_status == ODBCM_RC_CONNECTION_ERROR) {
      ClearMaps();
      UPPL_LOG_FATAL("Committing Configuration Failed - DB Access Error");
      return UNC_UPPL_RC_ERR_FATAL_COPYDB_CANDID_RUNNING;
    } else {
      ClearMaps();
      UPPL_LOG_FATAL("Committing Configuration Failed");
      return UNC_UPPL_RC_ERR_FATAL_COPYDB_CANDID_RUNNING;
    }
    } // End of eventDoneLock
    UncRespCode notfn_status = SendControllerNotification(db_conn,
                                                             vec_old_val_ctr);
    if (notfn_status != UNC_RC_SUCCESS) {
      pfc_log_error("Ctr Notfn falied with status:%d", notfn_status);
    }
    notfn_status = SendDomainNotification(db_conn, vec_old_val_ctr_domain);
    if (notfn_status != UNC_RC_SUCCESS) {
      pfc_log_error("Domain Notfn falied with status:%d", notfn_status);
    }
    // For all deleted controllers, remove the state entries as well
    it_controller = controller_deleted_.begin();
    for ( ; it_controller != controller_deleted_.end(); ++it_controller) {
      key_ctr_t key_ctr_obj = *it_controller;
      string controller_name =
          reinterpret_cast<char*>(key_ctr_obj.controller_name);
      pfc_log_info("Removing State/Import db entries for controller %s",
                   controller_name.c_str());
      ODBCM_RC_STATUS clear_status =
          PhysicalLayer::get_instance()->get_odbc_manager()->
          ClearOneInstance(UNC_DT_STATE, controller_name, db_conn);
      if (clear_status !=  ODBCM_RC_SUCCESS) {
        ClearMaps();
        UPPL_LOG_FATAL("Error during Clearing the state db");
        TcLibModule* tclib_ptr = static_cast<TcLibModule*>
         (TcLibModule::getInstance(TCLIB_MODULE_NAME));
        tclib_ptr->TcLibWriteControllerInfo(controller_name.c_str(),
                                            UNC_RC_INTERNAL_ERR,
                                            0);
        return UNC_UPPL_RC_ERR_CLEAR_DB;
      }
      clear_status =
          PhysicalLayer::get_instance()->get_odbc_manager()->
          ClearOneInstance(UNC_DT_IMPORT, controller_name, db_conn);
      if (clear_status !=  ODBCM_RC_SUCCESS) {
        UPPL_LOG_FATAL("Error during Clearing the import db");
        TcLibModule* tclib_ptr = static_cast<TcLibModule*>
         (TcLibModule::getInstance(TCLIB_MODULE_NAME));
        tclib_ptr->TcLibWriteControllerInfo(controller_name.c_str(),
                                            UNC_RC_INTERNAL_ERR,
                                            0);
        return UNC_UPPL_RC_ERR_CLEAR_DB;
      }
      // remove the deleted controller entry from alarm_status_map
      physical_core->remove_ctr_from_alarm_status_map(controller_name, "1");
      physical_core->remove_ctr_from_alarm_status_map(controller_name, "2");
      physical_core->remove_ctr_from_alarm_status_map(controller_name, "3");
      // remove controller specific task queue from task map
      PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
      taskq_util->del_task_queue(controller_name);
    }
    itc_trans->set_trans_state(GLOBAL_COMMIT_SUCCESS);
    pfc_log_debug("TransactionRequest::HandleDriverResult:trans_state()= %d",
                  itc_trans->trans_state());
      
    {  // scope area for eventDoneLock
    ScopedReadWriteLock eventDoneLock(PhysicalLayer::get_events_done_lock_(),
                                    PFC_TRUE);  // write lock
    // Update Boundary oper status
    it_boundary = boundary_created_.begin();
    for (; it_boundary != boundary_created_.end();) {
      key_boundary_obj = *it_boundary;
      pfc_log_debug("TxnClass:Created Boundary:  %s ",
                    key_boundary_obj.boundary_id);
      vector<void *> vect_boundary_key;
      vector<void *> vect_boundary_val_st;
      vect_boundary_key.push_back(reinterpret_cast<void*>(&key_boundary_obj));
      if (kt_boundary.ReadInternal(db_conn, vect_boundary_key,
                                   vect_boundary_val_st,
                                   UNC_DT_RUNNING,
                                   UNC_OP_READ) != UNC_RC_SUCCESS) {
        continue;
      }
      vector<OperStatusHolder> ref_oper_status;
      UncRespCode operstatus_return =
          kt_boundary.HandleOperStatus(
              db_conn, UNC_DT_RUNNING,
              reinterpret_cast<void*>(&key_boundary_obj),
              vect_boundary_val_st[0],
              ref_oper_status);
      pfc_log_debug("HandleOperStatus in Create: %d", operstatus_return);
      kt_boundary.ClearOperStatusHolder(ref_oper_status);
      // Release memory allocated for key struct
      key_boundary_t *boundary_key =
          reinterpret_cast<key_boundary_t*>(vect_boundary_key[0]);
      if (boundary_key != NULL) {
        delete boundary_key;
        boundary_key = NULL;
      }
      // Release memory for val structure
      val_boundary_st_t *bdry_st = reinterpret_cast<val_boundary_st*>
      (vect_boundary_val_st[0]);
      if (bdry_st != NULL) {
        delete bdry_st;
        bdry_st = NULL;
      }
      ++it_boundary;
    }
    pfc_log_debug("Starting to send the Notification after"
        " committing configuration");
    notfn_status = SendBoundaryNotification(db_conn, vec_old_val_boundary);
    if (notfn_status != UNC_RC_SUCCESS) {
      pfc_log_error("Bdry Notfn falied with status:%d", notfn_status);
    }
    // No need to return failure,since Notification failure alarm already raised
  }  // scope area for eventDoneLock end
  }
  return UNC_RC_SUCCESS;
}

/**HandleGlobalCommitRequest
 * @Description : This function handles the Global Commit Request sent by TC.
 *                Checks whether the transaction state is VOTE_SUCCESS as a
 *                precondition and Set the transaction state to
 *                GLOBAL_COMMIT_WAIT_DRIVER_RESULT
 * @param[in]   : session_id - ipc session id used for TC validation
 *                config_id - configuration id used for TC validation
 * @param[out]  : driver_info - contains the controller list 
 * @return      : UNC_RC_SUCCESS if the HandleGlobalCommitRequest is
 *                successful or returns UNC_UPPL_RC_ERR_* if
 *                HandleGlobalCommitRequest is failed
 * */
UncRespCode TransactionRequest::HandleGlobalCommitRequest(
    uint32_t session_id,
    uint32_t config_id,
    TcConfigMode config_mode,
    TcDriverInfoMap
    &driver_info) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans =
      physical_core->get_internal_transaction_coordinator();
  // If config_mode is not real-network or Global mode, return success
  if ((config_mode != TC_CONFIG_REAL) &&
                          (config_mode != TC_CONFIG_GLOBAL)) {
    itc_trans->set_trans_state(GLOBAL_COMMIT_WAIT_DRIVER_RESULT);
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("GlobalCommit trans_state()= %d", itc_trans->trans_state());

  if (itc_trans->trans_state() == VOTE_SUCCESS) {
    pfc_log_debug("itc_trans->trans_state() == VOTE_SUCCESS");
    itc_trans->set_trans_state(GLOBAL_COMMIT_BEGIN);
    itc_trans->set_trans_state(GLOBAL_COMMIT_WAIT_DRIVER_RESULT);
  } else {
    pfc_log_debug("itc_trans->trans_state() != VOTE_SUCCESS");
    return UNC_UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
  }
  pfc_log_debug(
      "TransactionRequest::HandleGlobalCommitRequest:trans_state()= %d",
      itc_trans->trans_state());
  return UNC_RC_SUCCESS;
}

/** AbortTransaction
 * @Description : This function is used to abort the transaction
 *                Sets the transaction state to TRANS_END
 * @param[in]   : session_id - ipc session id used for TC validation
 *                config_id - configuration id used for TC validation
 *                operation_phase - denotes the TC commit operation phase
 * @return      : UNC_RC_SUCCESS if the AbortTransaction is successful or
 *                returns UNC_UPPL_RC_ERR_* if AbortTransaction is failed
 * */
UncRespCode TransactionRequest::AbortTransaction(uint32_t session_id,
                                                    uint32_t config_id,
                                                    TcConfigMode config_mode,
                                                    TcCommitOpAbortPhase
                                                    operation_phase) {
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans =
      physical_core->get_internal_transaction_coordinator();
  // If config_mode is not real-network or Global mode, return success
  if ((config_mode != TC_CONFIG_REAL) &&
                              (config_mode != TC_CONFIG_GLOBAL)) {
    itc_trans-> set_trans_state(TRANS_END);
    return UNC_RC_SUCCESS;
  }
  pfc_log_info("TransactionRequest::AbortTxn called with opn phase %d",
               operation_phase);
  pfc_log_debug("trans_state()= %d", itc_trans->trans_state());
  ClearMaps();
  if (operation_phase == unc::tclib::COMMIT_TRANSACTION_START) {
    pfc_log_debug("AbortTxn COMMIT_TXN_START - Nothing to do");
  } else if (operation_phase == unc::tclib::COMMIT_VOTE_REQUEST) {
    pfc_log_debug("AbortTxn COMMIT_VOTE_REQ - Nothing to do");
  }
  itc_trans-> set_trans_state(TRANS_END);
  return UNC_RC_SUCCESS;
}

/** EndTransaction
 * @Description : This function is used to end the transaction.Checks whether
 *                whether there is any modified controller configuration and if
 *                its there send notification to driver and set the transaction
 *                state as TRANS_END
 * @param[in]   : session_id - ipc session id used for TC validation
 *                config_id - configuration id used for TC validation
 *                trans_res - specifies the TC transaction end result
 * @return      : UNC_RC_SUCCESS if the EndTransaction is successful or
 *                returns UNC_UPPL_RC_ERR_* if EndTransaction is failed
 * */
UncRespCode TransactionRequest::EndTransaction(
    OdbcmConnectionHandler *db_conn,
    uint32_t session_id,
    uint32_t config_id,
    TcConfigMode config_mode,
    TcTransEndResult trans_res,
    bool audit_flag) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();
  // If config_mode is not real-network or Global mode, return success
  if ((config_mode != TC_CONFIG_REAL) &&
                               (config_mode != TC_CONFIG_GLOBAL)) {
    itc_trans->set_trans_state(TRANS_END);
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("trans_state()= %d", itc_trans->trans_state());
  // Checking the result of the Transaction
  if (trans_res == unc::tclib::TRANS_END_FAILURE) {
    itc_trans->set_trans_state(TRANS_END);
    pfc_log_info("End Transaction:FailureResponse from TC\n");
    ClearMaps();
    return UNC_RC_SUCCESS;
  }
  // Checking whether there is any modified controller configuration
  if (controller_deleted_.empty() && controller_created_.empty() &&
      controller_updated_.empty()) {
    itc_trans->set_trans_state(TRANS_END);
    return UNC_RC_SUCCESS;
  }
  // Sending Committed Controller Configuration to Logical
  ret_code  = SendControllerConfigToLogical(db_conn);
  if (ret_code != UNC_RC_SUCCESS) {
    UPPL_LOG_FATAL("Unable to send the Controller Info to Logical");
    return ret_code;
  }
  string controller_name = "";
  string driver_name = "";
  unc_keytype_ctrtype_t controller_type;
  key_ctr_t key_ctr_obj;
  Kt_Controller kt_controller;
  UncRespCode err = UNC_RC_SUCCESS;
  IPCClientDriverHandler pfc_drv_handler(UNC_CT_PFC, err);
  if (err != UNC_RC_SUCCESS) {
    ClearMaps();
    UPPL_LOG_FATAL("Cannot open session to PFC driver");
    return err;
  }
  IPCClientDriverHandler vnp_drv_handler(UNC_CT_VNP, err);
  if (err != UNC_RC_SUCCESS) {
    ClearMaps();
    UPPL_LOG_FATAL("Cannot open session to VNP driver");
    return err;
  }
  IPCClientDriverHandler polc_drv_handler(UNC_CT_POLC, err);
  if (err != UNC_RC_SUCCESS) {
    ClearMaps();
    UPPL_LOG_FATAL("Cannot open session to POLC driver");
    return err;
  }
  IPCClientDriverHandler odc_drv_handler(UNC_CT_ODC, err);
  if (err != UNC_RC_SUCCESS) {
    ClearMaps();
    UPPL_LOG_FATAL("Cannot open session to ODC driver");
    return err;
  }
  // Sending the 'Delete' Controller Request to Driver
  vector<key_ctr_t> :: iterator it_controller = controller_deleted_.begin();
  for ( ; it_controller != controller_deleted_.end(); ++it_controller) {
    key_ctr_obj = *it_controller;
    controller_name = reinterpret_cast<char*>(key_ctr_obj.controller_name);
    if (controller_type_map_.find(controller_name) !=
        controller_type_map_.end()) {
      controller_type =
          (unc_keytype_ctrtype_t)controller_type_map_[controller_name];
      if (physical_core->GetDriverName(controller_type, driver_name)
          != UNC_RC_SUCCESS)  {
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
      } else if (controller_type == UNC_CT_POLC) {
        pfc_log_debug("POLC Controller Type");
        cli_session = polc_drv_handler.ResetAndGetSession();
      } else if (controller_type == UNC_CT_ODC) {
        pfc_log_debug("ODC Controller Type");
        cli_session = odc_drv_handler.ResetAndGetSession();
      } else {
        pfc_log_info("DRIVER SUPPORT NOT ADDED YET FOR"
            "UNKNOWN type");
        continue;
      }
      if (audit_flag != false) {
      string domain_id = "";
      driver_request_header rqh = {uint32_t(0), uint32_t(0), controller_name,
          domain_id, UNC_OP_DELETE, uint32_t(0),
          (uint32_t)0, (uint32_t)0, UNC_DT_RUNNING,
          UNC_KT_CONTROLLER};
      int err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
      err |= cli_session->addOutput(key_ctr_obj);
      pfc_log_info("KEY:%s", IpctUtil::get_string(key_ctr_obj).c_str());
      val_ctr_commit_ver_t val_ctr_old;
      memset(&val_ctr_old, '\0', sizeof(val_ctr_commit_ver_t));
      map<string, val_ctr_commit_ver_t> :: iterator it_ctr_oldval =
                              controller_old_del_val_.find(controller_name);
      if (it_ctr_oldval != controller_old_del_val_.end()) {
        val_ctr_old = (it_ctr_oldval->second);
      }
      err |= cli_session->addOutput(val_ctr_old);
      pfc_log_info("OLD VAL:%s",
                          IpctUtil::get_string(val_ctr_old).c_str());
      // Send the request to driver
      UncRespCode driver_response = UNC_RC_SUCCESS;
      driver_response_header rsp;
      if (controller_type == UNC_CT_PFC) {
        driver_response = pfc_drv_handler.SendReqAndGetResp(rsp);
      } else if (controller_type == UNC_CT_VNP) {
        driver_response = vnp_drv_handler.SendReqAndGetResp(rsp);
      } else if (controller_type == UNC_CT_POLC) {
        driver_response = polc_drv_handler.SendReqAndGetResp(rsp);
      } else if (controller_type == UNC_CT_ODC) {
        driver_response = odc_drv_handler.SendReqAndGetResp(rsp);
      }
      if (err != 0 || driver_response != UNC_RC_SUCCESS) {
        pfc_log_error("Delete response from driver for controller %s"
            "is %d err=%d", controller_name.c_str(), driver_response, err);
      }
      // remove the deleted controller entry from alarm_status_map
      physical_core->remove_ctr_from_alarm_status_map(controller_name, "1");
      physical_core->remove_ctr_from_alarm_status_map(controller_name, "2");
      physical_core->remove_ctr_from_alarm_status_map(controller_name, "3");
      /* Clearing the entry from commit version map */
      map<string, commit_version>::iterator it_cv;
      it_cv = AuditRequest::comm_ver_.find(controller_name);
      if (it_cv != AuditRequest::comm_ver_.end()) {
        AuditRequest::comm_ver_.erase(it_cv);
      }
      // remove controller specific task queue from task map
      PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
      taskq_util->del_task_queue(controller_name);
      // Deleting the readwrite lock of this controller from CtrlUtil map
      map<string, CtrOprnStatus>::iterator it;
      PhysicalLayer::ctr_oprn_mutex_.lock();
      it = PhysicalLayer::ctr_oprn_status_.find(controller_name);
      if (it != PhysicalLayer::ctr_oprn_status_.end()) {
        if (it->second.rwlock_oper_st != NULL) delete it->second.rwlock_oper_st;
        PhysicalLayer::ctr_oprn_status_.erase(it);
      }
      PhysicalLayer::ctr_oprn_mutex_.unlock();
    }
    } else {
      pfc_log_error("Could not able to find type for %s",
                    controller_name.c_str());
    }
  }
  pfc_log_debug("End Trans:Deleted Controller Iterated ");
  // Sending the 'Created' Controller Configuration Notification
  SendControllerInfo(db_conn, UNC_OP_CREATE, session_id, config_id, audit_flag);
  pfc_log_debug("End Trans:Created Controller Iterated ");
  // Sending the 'Updated' Controller Request to Driver
  SendControllerInfo(db_conn, UNC_OP_UPDATE, session_id, config_id, audit_flag);
  pfc_log_debug("End Trans:Updated Controller Iterated ");
  itc_trans->set_trans_state(TRANS_END);
  pfc_log_debug("End Trans:Response Code = %d", ret_code);
  controller_old_upd_val_.clear();
  controller_old_del_val_.clear();
  ClearMaps();
  return ret_code;
}

/** ClearMaps
 * @Description : Clear all maps and vectors that contains controller ,domain
 *                and boundary lists
 * @param[in]   : None
 * @return      : void
 * */
void TransactionRequest::ClearMaps() {
  /* Clearing the contents of the previously stored controller */
  if (!controller_created_.empty()) controller_created_.clear();
  if (!controller_updated_.empty()) controller_updated_.clear();
  if (!controller_deleted_.empty()) controller_deleted_.clear();
  controller_type_map_.clear();
  /* Clearing the contents of the previously stored domain */
  if (!domain_created_.empty()) domain_created_.clear();
  if (!domain_updated_.empty()) domain_updated_.clear();
  if (!domain_deleted_.empty()) domain_deleted_.clear();
  /* Clearing the contents of the previously stored boundaries */
  if (!boundary_created_.empty()) boundary_created_.clear();
  if (!boundary_updated_.empty()) boundary_updated_.clear();
  if (!boundary_deleted_.empty()) boundary_deleted_.clear();
}

/**SendControllerNotification
 * @Description : This function is to send notification to north bound for
 *                the modified controllers
 * @param[in]   : vec_old_val_ctr - Vector for storing the old value struct
 *                of the controller
 * @return      : UNC_RC_SUCCESS if the notification of modified controllers
 *                success or returns UNC_UPPL_RC_ERR_* if its failed
 * */
UncRespCode TransactionRequest::SendControllerNotification(
    OdbcmConnectionHandler *db_conn,
    vector<void *> vec_old_val_ctr) {
  // Sending the notification of deleted  controllers
  uint32_t oper_type = UNC_OP_DELETE;
  void *dummy_old_val_ctr = NULL;
  void *dummy_new_val_ctr = NULL;
  vector<key_ctr_t> :: iterator it_controller = controller_deleted_.begin();
  Kt_Controller kt_controller;
  for (; it_controller != controller_deleted_.end(); ++it_controller) {
    key_ctr_t key_ctr_obj = *it_controller;
    pfc_log_debug("Sending Notification for Deleted Controller: %s",
                  key_ctr_obj.controller_name);
    UncRespCode nofn_status =
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
  it_controller = controller_created_.begin();
  for (; it_controller != controller_created_.end(); ++it_controller) {
    key_ctr_t key_ctr_obj = *it_controller;
    pfc_log_debug("Sending Notification for Created Controller: %s",
                  key_ctr_obj.controller_name);
    vector<void *> vect_ctr_key, vect_ctr_val;
    vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
    UncRespCode retCode = kt_controller.ReadInternal(db_conn,
                                                        vect_ctr_key,
                                                        vect_ctr_val,
                                                        UNC_DT_CANDIDATE,
                                                        UNC_OP_READ);
    if (retCode == UNC_RC_SUCCESS) {
      UncRespCode nofn_status =
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
    } else  if (retCode == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UNC_UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  // Sending the notification of updated Controller
  oper_type = UNC_OP_UPDATE;
  it_controller = controller_updated_.begin();
  vector<void *> :: iterator it_ctr_old = vec_old_val_ctr.begin();
  for (; (it_controller != controller_updated_.end()) &&
  (it_ctr_old != vec_old_val_ctr.end());
  ++it_controller, ++it_ctr_old) {
    key_ctr_t key_ctr_obj = *it_controller;
    pfc_log_debug("Sending Notification for Updated Controller: %s",
                  key_ctr_obj.controller_name);
    vector<void *> vect_ctr_key, vect_ctr_val;
    vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
    UncRespCode retCode = kt_controller.ReadInternal(db_conn,
                                                        vect_ctr_key,
                                                        vect_ctr_val,
                                                        UNC_DT_CANDIDATE,
                                                        UNC_OP_READ);
    if (retCode == UNC_RC_SUCCESS) {
      UncRespCode nofn_status =
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
      // delete the old value
      val_ctr_st_t *ctr_old_val =
          reinterpret_cast<val_ctr_st_t*>(*it_ctr_old);
      if (ctr_old_val != NULL) {
        delete ctr_old_val;
        ctr_old_val = NULL;
      }
    } else  if (retCode == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UNC_UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  return UNC_RC_SUCCESS;
}

/**SendDomainNotification
 * @Description : This function is to send notification to north bound for
 *                the modified domain
 * @param[in]   : vec_old_val_ctr - Vector for storing the old value struct
 *                of the domain
 * @return      : UNC_RC_SUCCESS if the notification of modified domain
 *                success or returns UNC_UPPL_RC_ERR_* if its failed
 * */
UncRespCode TransactionRequest::SendDomainNotification(
    OdbcmConnectionHandler *db_conn,
    vector<void *> vec_old_val_ctr_domain) {
  /*Sending the notification of deleted unknown domain */
  uint32_t oper_type = UNC_OP_DELETE;
  vector<key_ctr_domain_t> :: iterator it_domain = domain_deleted_.begin();
  Kt_Ctr_Domain kt_domain;
  for (; it_domain != domain_deleted_.end(); ++it_domain) {
    key_ctr_domain_t key_ctr_domain_obj = *it_domain;
    void *key_ctr_domain_ptr = reinterpret_cast<void *>
    (&key_ctr_domain_obj);
    // For deleted domain, update the oper status in boundary
    int ret_notfn = kt_domain.InvokeBoundaryNotifyOperStatus(
        db_conn, UNC_DT_RUNNING, key_ctr_domain_ptr);
    pfc_log_debug("Boundary Invoke Operation return %d", ret_notfn);
    UncRespCode nofn_status =
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
  it_domain = domain_updated_.begin();
  vector<void *> :: iterator it_domain_old = vec_old_val_ctr_domain.begin();
  pfc_log_debug("TxnClass:Sending Notification for Updated Domain:");
  for (; (it_domain != domain_updated_.end()) &&
  (it_domain_old != vec_old_val_ctr_domain.end());
  ++it_domain, ++it_domain_old) {
    key_ctr_domain_t key_ctr_domain_obj = *it_domain;
    void *key_ctr_domain_ptr = reinterpret_cast<void *>
    (&key_ctr_domain_obj);
    vector<void *> vect_key_struct;
    vector<void *> vect_new_val;
    vect_key_struct.push_back(key_ctr_domain_ptr);
    pfc_log_debug("TxnClass:Controllername: %s",
                  key_ctr_domain_obj.ctr_key.controller_name);
    UncRespCode retCode = kt_domain.ReadInternal(db_conn, vect_key_struct,
                                                    vect_new_val,
                                                    UNC_DT_CANDIDATE,
                                                    UNC_OP_READ);

    if (retCode == UNC_RC_SUCCESS) {
      void *val_ctr_domain_new = vect_new_val[0];
      UncRespCode nofn_status =
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
    } else  if (retCode == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UNC_UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  /* Sending the notification of created unknown domain */
  oper_type = UNC_OP_CREATE;
  it_domain = domain_created_.begin();
  pfc_log_debug("TxnClass:Sending Notification for Created Domain:");
  for (; it_domain != domain_created_.end(); ++it_domain) {
    key_ctr_domain_t key_ctr_domain_obj = *it_domain;
    vector<void *> vect_key_domain;
    vector<void *> vect_val_domain;
    vect_key_domain.push_back(reinterpret_cast<void*>(&key_ctr_domain_obj));
    // For created domain, update the oper status in boundary
    int ret_notfn = kt_domain.InvokeBoundaryNotifyOperStatus(
        db_conn, UNC_DT_RUNNING, reinterpret_cast<void *>(&key_ctr_domain_obj));
    pfc_log_debug("Domain Invoke Operation return %d", ret_notfn);

    UncRespCode retCode = kt_domain.ReadInternal(db_conn, vect_key_domain,
                                                    vect_val_domain,
                                                    UNC_DT_CANDIDATE,
                                                    UNC_OP_READ);
    if (retCode == UNC_RC_SUCCESS) {
      void *val_ctr_domain_new = vect_val_domain[0];
      UncRespCode nofn_status =
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
    } else  if (retCode == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UNC_UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  return UNC_RC_SUCCESS;
}

/**SendBoundaryNotification
 * @Description : This function is to send notification to north bound for
 *                the modified boundary
 * @param[in]   : vec_old_val_ctr - Vector for storing the old value struct
 *                of the boundary
 * @return      : UNC_RC_SUCCESS if the notification of modified boundary
 *                success or returns UNC_UPPL_RC_ERR_* if its failed
 * */
UncRespCode TransactionRequest::SendBoundaryNotification(
    OdbcmConnectionHandler *db_conn,
    vector<void *> vec_old_val_boundary) {
  /*Sending the notification of deleted  boundary */
  Kt_Boundary kt_boundary;
  uint32_t oper_type = UNC_OP_DELETE;
  vector<key_boundary_t> :: iterator it_boundary = boundary_deleted_.begin();
  for (; it_boundary != boundary_deleted_.end(); ++it_boundary) {
    key_boundary_t key_boundary_obj = *it_boundary;
    void *key_boundary_ptr = reinterpret_cast<void *>(&key_boundary_obj);
    UncRespCode nofn_status =
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
  it_boundary = boundary_created_.begin();
  key_boundary_t new_val_boundary;
  for (; it_boundary != boundary_created_.end(); ++it_boundary) {
    key_boundary_t key_boundary_obj = *it_boundary;
    void *key_boundary_ptr = reinterpret_cast<void *>(&key_boundary_obj);
    vector<void *> vect_boundary_key;
    vector<void *> vect_boundary_val_st;
    vect_boundary_key.push_back(reinterpret_cast<void*>(&key_boundary_obj));

    UncRespCode retCode = kt_boundary.ReadInternal(db_conn,
                                                      vect_boundary_key,
                                                      vect_boundary_val_st,
                                                      UNC_DT_CANDIDATE,
                                                      UNC_OP_READ);
    if (retCode == UNC_RC_SUCCESS) {
      void *val_boundary_new = reinterpret_cast<void *>
      (&new_val_boundary);
      UncRespCode nofn_status =
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
    } else  if (retCode == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UNC_UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  // Sending the notification of updated  boundary
  oper_type = UNC_OP_UPDATE;
  it_boundary = boundary_updated_.begin();
  vector<void *> :: iterator it_boundary_old = vec_old_val_boundary.begin();
  for (; (it_boundary != boundary_updated_.end()) &&
  (it_boundary_old != vec_old_val_boundary.end());
  ++it_boundary, ++it_boundary_old) {
    void *key_boundary_ptr = reinterpret_cast<void *>
    (&(*it_boundary));
    vector<void *> vect_boundary_key;
    vector<void *> vect_boundary_val_st;
    key_boundary_t key_boundary_obj = *it_boundary;
    vect_boundary_key.push_back(reinterpret_cast<void*>(&key_boundary_obj));
    UncRespCode retCode = kt_boundary.ReadInternal(db_conn,
                                                      vect_boundary_key,
                                                      vect_boundary_val_st,
                                                      UNC_DT_CANDIDATE,
                                                      UNC_OP_READ);
    if (retCode == UNC_RC_SUCCESS) {
      val_boundary_st_t *val_boundary = reinterpret_cast<val_boundary_st_t*>
          (vect_boundary_val_st[0]);
      UncRespCode nofn_status =
          kt_boundary.ConfigurationChangeNotification(
              (uint32_t)UNC_DT_RUNNING,
              (uint32_t)UNC_KT_BOUNDARY,
              oper_type,
              key_boundary_ptr,
              *it_boundary_old,
              val_boundary);
      pfc_log_debug("Notification Status %d", nofn_status);
      // clear the key and val memory
      key_boundary_t *key_boundary = reinterpret_cast<key_boundary_t*>
      (vect_boundary_key[0]);
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
    } else  if (retCode == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
      // Do nothing
    } else {
      return UNC_UPPL_RC_ERR_COMMIT_OPERATION_NOT_ALLOWED;
    }
  }
  return UNC_RC_SUCCESS;
}

/**SendControllerInfo
 * @Description : This function is to send controller information
 *                to driver by creating the ipc session with PFC driver
 *                handler and VNP driver handler
 * @param[in]   : session_id - ipc session id used for TC validation
 *                config_id - configuration id used for TC validation
 *                operation_type - UNC_OP_* specifies the operation
 * @return      : void
 * */
void TransactionRequest::SendControllerInfo(OdbcmConnectionHandler *db_conn,
                                            uint32_t operation_type,
                                            uint32_t session_id,
                                            uint32_t config_id,
                                            bool audit_flag) {
  vector<key_ctr_t> controller_info;
  UncRespCode err = UNC_RC_SUCCESS;
  IPCClientDriverHandler pfc_drv_handler(UNC_CT_PFC, err);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_error("Cannot open session to PFC driver");
    return;
  }
  IPCClientDriverHandler vnp_drv_handler(UNC_CT_VNP, err);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_error("Cannot open session to VNP driver");
    return;
  }
  IPCClientDriverHandler polc_drv_handler(UNC_CT_POLC, err);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_error("Cannot open session to POLC driver");
    return;
  }
  IPCClientDriverHandler odc_drv_handler(UNC_CT_ODC, err);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_error("Cannot open session to ODC driver");
    return;
  }
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  if (operation_type == UNC_OP_CREATE) {
    controller_info = controller_created_;
  } else if (operation_type == UNC_OP_UPDATE) {
    controller_info = controller_updated_;
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
    UncRespCode retCode = kt_controller.ReadInternal(db_conn,
                                                        vect_key_ctr,
                                                        vect_ctr_val,
                                                        UNC_DT_CANDIDATE,
                                                        UNC_OP_READ);
    if (retCode != UNC_RC_SUCCESS) {
      pfc_log_info("ReadInternal failed for controller %s",
                    controller_name.c_str());
      continue;
    }
    unc_keytype_ctrtype_t controller_type = UNC_CT_UNKNOWN;
    string driver_name = "";
    val_ctr_st_t *val_ctr_new = reinterpret_cast<val_ctr_st_t*>
    (vect_ctr_val[0]);
    if (val_ctr_new == NULL) {
      continue;
    }
    // Sending the controller info to driver
    controller_type =
        (unc_keytype_ctrtype_t)
        (PhyUtil::uint8touint(val_ctr_new->controller.type));
    if (physical_core->GetDriverName(controller_type, driver_name)
        != UNC_RC_SUCCESS)  {
      pfc_log_info("TxnEnd:Unable to get Driver Name from Physical Core");
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

    //  read commit version from DT_RUNNING
    vector<string> vect_controller_id;
    vector<val_ctr_commit_ver_t> vect_val_ctr_cv;
    val_ctr_commit_ver_t cv_val_struct;
    memset(&cv_val_struct, 0, sizeof(val_ctr_commit_ver_t));
    uint32_t max_rep_ct = 1;

    pfc_log_trace("Calling ReadCtrCommitVerFromDB");
    UncRespCode read_status = kt_controller.ReadCtrCommitVerFromDB(db_conn,
                               reinterpret_cast<void*>(&key_ctr_obj),
                               NULL/*reinterpret_cast<void*>(&cv_val_struct)*/,
                               UNC_DT_RUNNING, UNC_OP_READ,
                               max_rep_ct, vect_val_ctr_cv, vect_controller_id);
    if (read_status != UNC_RC_SUCCESS) {
      pfc_log_info("Error in read commit version from DB");
      continue;
    }
    val_ctr_commit_ver_t ctr_val_struct =  vect_val_ctr_cv[0];
    memcpy(&ctr_val_struct.controller, &val_ctr_new->controller,
                     sizeof(val_ctr_new->controller));
    ctr_val_struct.valid[kIdxCtrVal] = UNC_VF_VALID;
    pfc_log_debug("Controller name is %s and driver name is %s",
                  controller_name.c_str(), driver_name.c_str());

    ClientSession *cli_session = NULL;
    if (controller_type == UNC_CT_PFC) {
      pfc_log_debug("PFC Controller type");
      cli_session = pfc_drv_handler.ResetAndGetSession();
    } else if (controller_type == UNC_CT_VNP) {
      pfc_log_debug("VNP Controller type");
      cli_session = vnp_drv_handler.ResetAndGetSession();
    } else if (controller_type == UNC_CT_POLC) {
      pfc_log_debug("POLC Controller type");
      cli_session = polc_drv_handler.ResetAndGetSession();
    } else if (controller_type == UNC_CT_ODC) {
      pfc_log_debug("ODC Controller type");
      cli_session = odc_drv_handler.ResetAndGetSession();
    } else {
      pfc_log_info("DRIVER SUPPORT NOT ADDED YET FOR"
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

    string domain_id = "";
    driver_request_header rqh = {uint32_t(0), uint32_t(0), controller_name,
        domain_id, operation_type, uint32_t(0),
        (uint32_t)0, (uint32_t)0, UNC_DT_RUNNING,
        UNC_KT_CONTROLLER};

    int err = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
    err |= cli_session->addOutput(key_ctr_obj);
    err |= cli_session->addOutput(ctr_val_struct);
    pfc_log_info("%s", IpctUtil::get_string(key_ctr_obj).c_str());
    pfc_log_info("NEW VAL :%s", IpctUtil::get_string(ctr_val_struct).c_str());

    // Adding the old updated value strucutre in the session
    if (operation_type == UNC_OP_UPDATE) {
      map<string, val_ctr_commit_ver_t > :: iterator it_ctr_oldval =
                                  controller_old_upd_val_.find(controller_name);
      val_ctr_commit_ver_t val_ctr_old;
      memset(&val_ctr_old, '\0', sizeof(val_ctr_commit_ver_t));
      if (it_ctr_oldval != controller_old_upd_val_.end()) {
        val_ctr_old = (it_ctr_oldval->second);
      }
      err |= cli_session->addOutput(val_ctr_old);
      pfc_log_info("OLD VAL :%s", IpctUtil::get_string(val_ctr_old).c_str());
      bool IsIpChanged = false;
      // Check if the ip address is added
      if ((val_ctr_old.controller.ip_address.s_addr == 0 ) &&
        (ctr_val_struct.controller.ip_address.s_addr != 0)) {
        IsIpChanged = false;
      }
      // Check if ip is modified with other ip
      if ((val_ctr_old.controller.ip_address.s_addr !=
              ctr_val_struct.controller.ip_address.s_addr)
          && (val_ctr_old.controller.ip_address.s_addr != 0 )
          && (ctr_val_struct.controller.ip_address.s_addr != 0)) {
        IsIpChanged = true;
      }
      // Check if ip address is deleted
      if ((val_ctr_old.controller.ip_address.s_addr != 0 &&
         (ctr_val_struct.controller.ip_address.s_addr == 0))) {
        IsIpChanged = true;
      }
      if (IsIpChanged == true) {
        PhysicalLayer::ctr_oprn_mutex_.lock();
        // set the IsIpChanged flag to true
        map<string, CtrOprnStatus> :: iterator it =
            PhysicalLayer::ctr_oprn_status_.find(controller_name);
        if (it != PhysicalLayer::ctr_oprn_status_.end()) {
          CtrOprnStatus ctr_util =  it->second;
          ctr_util.IsIPChanged =  IsIpChanged;
          PhysicalLayer::ctr_oprn_status_[controller_name] = ctr_util;
        }
        PhysicalLayer::ctr_oprn_mutex_.unlock();
        // Release all associated entries in state db
        pfc_log_debug("Removing State entries for controller %s",
                     controller_name.c_str());
        ODBCM_RC_STATUS clear_status =
          PhysicalLayer::get_instance()->get_odbc_manager()->
          ClearOneInstance(UNC_DT_STATE, controller_name, db_conn);
        if (clear_status != ODBCM_RC_SUCCESS) {
          pfc_log_info("State DB clearing failed");
        }
        clear_status =
          PhysicalLayer::get_instance()->get_odbc_manager()->
          ClearOneInstance(UNC_DT_IMPORT, controller_name, db_conn);
        if (clear_status != ODBCM_RC_SUCCESS) {
          pfc_log_info("Import DB clearing failed");
        }
        ReadWriteLock *rwlock = NULL;
        PhysicalLayer::ctr_oprn_mutex_.lock();
        PHY_OPERSTATUS_LOCK(controller_name, read_status, rwlock, true);
        // Set the controller status as down
        val_ctr_st_t ctr_val;
        memset(ctr_val.valid, 0 , sizeof(ctr_val.valid));
        ctr_val.valid[kIdxOperStatus] = 1;
        ctr_val.oper_status = UPPL_CONTROLLER_OPER_DOWN;
        UncRespCode operation_status = kt_controller.HandleOperStatus(
          db_conn,
          UNC_DT_RUNNING,
          &key_ctr_obj,
          &ctr_val,
          true);
        if (operation_status != UNC_RC_SUCCESS) {
          pfc_log_error("Unable to set the oper status of controller as down");
        }

        PHY_OPERSTATUS_LOCK(controller_name, read_status, rwlock, false);
        PhysicalLayer::ctr_oprn_mutex_.unlock();
        // Send the Controller Disconnect alarm
        PhysicalLayer::get_instance()->get_physical_core()->
          SendControllerDisconnectAlarm(controller_name);
        // Set the Actual version as empty
        string act_version = "";
        UncRespCode status = kt_controller.SetActualVersion(
                                   db_conn, &key_ctr_obj, act_version,
                                   UNC_DT_RUNNING, UNC_VF_INVALID);
        if (status != UNC_RC_SUCCESS) {
          pfc_log_error("act_version reset operation failed for running");
        }
        status = kt_controller.SetActualVersion(db_conn, &key_ctr_obj,
            act_version, UNC_DT_CANDIDATE, UNC_VF_INVALID);
        if (status != UNC_RC_SUCCESS) {
          pfc_log_error("act_version reset operation failed for candidate");
        }
        // Reset actual id as empty
        string actual_id = "";
        status = kt_controller.SetActualControllerId(db_conn, &key_ctr_obj,
                        actual_id, UNC_DT_RUNNING, UNC_VF_INVALID);
        if (status != UNC_RC_SUCCESS) {
          // log error
          pfc_log_error("actual_id reset operation failed for running");
        }
        status = kt_controller.SetActualControllerId(db_conn, &key_ctr_obj,
                            actual_id, UNC_DT_CANDIDATE, UNC_VF_INVALID);
        if (status != UNC_RC_SUCCESS) {
          // log error
          pfc_log_error("actual_id reset operation failed for candidate");
        }
      }
    }
    // Create a task queue to process particular controller's events
    if (operation_type == UNC_OP_CREATE) {
      PhyEventTaskqUtil *taskq_util = NotificationManager::get_taskq_util();
      UncRespCode status = taskq_util->create_task_queue(controller_name);
      if (status != UNC_RC_SUCCESS)
        UPPL_LOG_FATAL("Event Handling taskq creation error !!");
      CtrOprnStatus oCtrOprnStatus;
      oCtrOprnStatus.rwlock_oper_st = new ReadWriteLock();
      PhysicalLayer::ctr_oprn_mutex_.lock();
      PhysicalLayer::ctr_oprn_status_.insert(std::pair<string, CtrOprnStatus>
                                         (controller_name, oCtrOprnStatus));
      PhysicalLayer::ctr_oprn_mutex_.unlock();
    }
    // Send the request to driver
    UncRespCode driver_response = UNC_RC_SUCCESS;
    if (audit_flag != false) {
    driver_response_header rsp;

    if (controller_type == UNC_CT_PFC) {
      driver_response = pfc_drv_handler.SendReqAndGetResp(rsp);
    } else if (controller_type == UNC_CT_VNP) {
      driver_response = vnp_drv_handler.SendReqAndGetResp(rsp);
    } else if (controller_type == UNC_CT_POLC) {
      driver_response = polc_drv_handler.SendReqAndGetResp(rsp);
    } else if (controller_type == UNC_CT_ODC) {
      driver_response = odc_drv_handler.SendReqAndGetResp(rsp);
    }
    }
    delete val_ctr_new;
    val_ctr_new = NULL;
    key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>
    (vect_key_ctr[0]);
    if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
    }
    if (err != 0 || driver_response != UNC_RC_SUCCESS) {
      pfc_log_error("Create request to Driver failed for controller %s"
          " with response %d, err=%d", controller_name.c_str(),
          driver_response, err);
      //  continue; // Subsequent ctr_oprn_status_ code must be executed
    }
  }
}

/**GetModifiedController
 * @Description : This function is used to get the modified controllers
 *                from the Candidate Database.
 * @param[in]   : row_status - specifies the row status of modified
 *                row of kt_controller
 * @return      : UNC_RC_SUCCESS if GetModifiedControllers is successful or
 *                UNC_UPPL_RC_ERR_* in case of failure  
 * */
UncRespCode TransactionRequest::GetModifiedController(
    OdbcmConnectionHandler *db_conn,
    CsRowStatus row_status, TcConfigMode config_mode) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  unc_keytype_ctrtype_t controller_type;

  // Gets the Modified Controller Configuration
  Kt_Controller kt_controller;
  vector<void*> vec_key_ctr_modified;
  ret_code = kt_controller.GetModifiedRows(
      db_conn, vec_key_ctr_modified,
      row_status);
  pfc_log_debug("Controller:GetModifiedRows return code = %d", ret_code);
  if (ret_code == UNC_UPPL_RC_ERR_DB_ACCESS ||
                         ret_code == UNC_UPPL_RC_ERR_DB_GET) {
    pfc_log_info(
          "Error retrieving GetModifiedRows, return txn error");
    TcLibModule* tclib_ptr = static_cast<TcLibModule*>
        (TcLibModule::getInstance(TCLIB_MODULE_NAME));
    tclib_ptr->TcLibWriteControllerInfo("",
                                        UNC_RC_INTERNAL_ERR,
                                        0);
    return UNC_UPPL_RC_ERR_TRANSACTION_START;
  }
  ret_code = UNC_RC_SUCCESS;
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
    UncRespCode key_exist_running = kt_controller.IsKeyExists(
        db_conn, UNC_DT_RUNNING,
        vect_ctr_key_value);
    if (key_exist_running == UNC_UPPL_RC_ERR_DB_ACCESS) {
      // Error retrieving information from database, send failure
      pfc_log_info(
          "Error retrieving information from running db, return txn error");
      TcLibModule* tclib_ptr = static_cast<TcLibModule*>
          (TcLibModule::getInstance(TCLIB_MODULE_NAME));
      tclib_ptr->TcLibWriteControllerInfo(controller_name.c_str(),
                                          UNC_RC_INTERNAL_ERR,
                                          0);
      ret_code = UNC_UPPL_RC_ERR_TRANSACTION_START;
      break;
    } else if (key_exist_running != UNC_RC_SUCCESS) {
      pfc_log_debug(
          "Controller entry in is not available in running");
    }
    if (row_status == CREATED) {
      controller_created_.push_back(*ptr_key_ctr);
      if (key_exist_running == UNC_RC_SUCCESS) {
        controller_deleted_.push_back(*ptr_key_ctr);
        is_controller_recreated = PFC_TRUE;
      }
    }
    if (row_status == UPDATED)
      controller_updated_.push_back(*ptr_key_ctr);
    if (row_status == DELETED) {
      if (key_exist_running == UNC_RC_SUCCESS) {
        controller_deleted_.push_back(*ptr_key_ctr);
        // Check whether CONTROLLER is referred in Running DB in Logical layer
        if (config_mode == TC_CONFIG_REAL) {
          UncRespCode validate_status = kt_controller.SendSemanticRequestToUPLL(
                                               ptr_key_ctr, UNC_DT_RUNNING);
          if (validate_status != UNC_RC_SUCCESS) {
            // log error and send error response
            pfc_log_error("Controller is referred in Logical, "
                                    "so delete is not allowed");
            return UNC_UPPL_RC_FAILURE;
          }
        }
      }
    }
    //  Freeing the Memory allocated in controller class
    if (PhyUtil::get_controller_type(
        db_conn, controller_name,
        controller_type, UNC_DT_CANDIDATE) == UNC_RC_SUCCESS) {
      if (row_status == DELETED) {
        pfc_log_debug(
            "Controller %s of type %d is marked for DELETION",
            controller_name.c_str(), controller_type);
        controller_type_map_[controller_name] = controller_type;
      }
    } else {
      pfc_log_info("Error retrieving controller type from candidate");
      TcLibModule* tclib_ptr = static_cast<TcLibModule*>
        (TcLibModule::getInstance(TCLIB_MODULE_NAME));
      tclib_ptr->TcLibWriteControllerInfo(controller_name.c_str(),
                                          UNC_RC_INTERNAL_ERR,
                                          0);
      ret_code = UNC_UPPL_RC_ERR_TRANSACTION_START;
      break;
    }
    if (is_controller_recreated == PFC_TRUE) {
      // Get existing controller type from RUNNING
      if (PhyUtil::get_controller_type(
          db_conn,  controller_name,
          controller_type, UNC_DT_RUNNING) == UNC_RC_SUCCESS) {
        pfc_log_debug(
            "Controller %s of type %d is marked for RECREATION",
            controller_name.c_str(), controller_type);
        controller_type_map_[controller_name] = controller_type;
      } else {
        pfc_log_info("Error retrieving controller type from running");
        TcLibModule* tclib_ptr = static_cast<TcLibModule*>
          (TcLibModule::getInstance(TCLIB_MODULE_NAME));
        tclib_ptr->TcLibWriteControllerInfo(controller_name.c_str(),
                                          UNC_RC_INTERNAL_ERR,
                                          0);
        ret_code = UNC_UPPL_RC_ERR_TRANSACTION_START;
        break;
      }
    }
  }
  for (uint32_t config_count = 0; \
  config_count < vec_key_ctr_modified.size(); config_count++) {
    key_ctr_t *ptr_key_ctr = reinterpret_cast<key_ctr_t *>
    (vec_key_ctr_modified[config_count]);
    if (ptr_key_ctr == NULL) {
      continue;
    }
    delete ptr_key_ctr;
    ptr_key_ctr = NULL;
  }
  pfc_log_debug("Modified Controllers Iterated properly");
  return ret_code;
}

/**GetModifiedDomain
 * @Description : This function is used to get the modified domains
 *                from the Candidate Database.
 * @param[in]   : row_status - specifies the row status of modified
 *                row of kt_domain
 * @return      : UNC_RC_SUCCESS if GetModifiedDomain is successful or
 *                UNC_UPPL_RC_ERR_* in case of failure
 * */
UncRespCode TransactionRequest::GetModifiedDomain(
    OdbcmConnectionHandler *db_conn,
    CsRowStatus row_status) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  /* Getting the Modified Unknown Domain Configuration */
  Kt_Ctr_Domain kt_ctr_domain;
  vector<void*> vec_key_ctr_domain_modified;
  ret_code = kt_ctr_domain.GetModifiedRows(
      db_conn, vec_key_ctr_domain_modified,
      row_status);
  pfc_log_debug("Domain:GetModifiedRows return code = %d", ret_code);
  if (ret_code == UNC_UPPL_RC_ERR_DB_ACCESS ||
                            ret_code == UNC_UPPL_RC_ERR_DB_GET) {
    pfc_log_info(
          "Error retrieving GetModifiedRows, return txn error");
    TcLibModule* tclib_ptr = static_cast<TcLibModule*>
          (TcLibModule::getInstance(TCLIB_MODULE_NAME));
    tclib_ptr->TcLibWriteControllerInfo("",
                                        UNC_RC_INTERNAL_ERR,
                                        0);
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
      domain_created_.push_back(*ptr_key_ctr_domain);
    if (row_status == UPDATED)
      domain_updated_.push_back(*ptr_key_ctr_domain);
    if (row_status == DELETED) {
      // check whether it is already in running
      vector<string> vect_domain_key_value;
      vect_domain_key_value.push_back(
          (const char*)ptr_key_ctr_domain->ctr_key.controller_name);
      vect_domain_key_value.push_back(domain_name);
      UncRespCode key_exist_running = kt_ctr_domain.IsKeyExists(
          db_conn, UNC_DT_RUNNING,
          vect_domain_key_value);
      if (key_exist_running == UNC_RC_SUCCESS) {
        domain_deleted_.push_back(*ptr_key_ctr_domain);
      } else if (key_exist_running == UNC_UPPL_RC_ERR_DB_ACCESS) {
        // Error retrieving information from database, send failure
        pfc_log_info(
            "Error retrieving information from running db, return txn error");
        TcLibModule* tclib_ptr = static_cast<TcLibModule*>
          (TcLibModule::getInstance(TCLIB_MODULE_NAME));
        tclib_ptr->TcLibWriteControllerInfo(
          (const char*)ptr_key_ctr_domain->ctr_key.controller_name,
          UNC_RC_INTERNAL_ERR,
          0);
        return UNC_UPPL_RC_ERR_TRANSACTION_START;
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
  return UNC_RC_SUCCESS;
}

/**GetModifiedBoundary
 * @Description : This function is used to get the modified boundary
 *                from the Candidate Database.
 * @param[in]   : row_status - specifies the row status of modified
 *                row of kt_boundary
 * @return      : UNC_RC_SUCCESS if GetModifiedBoundary is successful or
 *                UNC_UPPL_RC_ERR_* in case of failure
 * */
UncRespCode TransactionRequest::GetModifiedBoundary(
    OdbcmConnectionHandler *db_conn,
    CsRowStatus row_status, TcConfigMode config_mode) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  //  Getting the Modified Boundary Configuration
  Kt_Boundary kt_boundary;
  vector<void*> vec_key_boundary_modified;
  ret_code = kt_boundary.GetModifiedRows(
      db_conn, vec_key_boundary_modified,
      row_status);
  pfc_log_debug("Controller:GetModifiedRows return code = %d", ret_code);
  if (ret_code == UNC_UPPL_RC_ERR_DB_ACCESS ||
                              ret_code == UNC_UPPL_RC_ERR_DB_GET) {
     pfc_log_info(
          "Error retrieving GetModifiedRows from running db, return txn error");
    TcLibModule* tclib_ptr = static_cast<TcLibModule*>
          (TcLibModule::getInstance(TCLIB_MODULE_NAME));
    tclib_ptr->TcLibWriteControllerInfo("",
                                        UNC_RC_INTERNAL_ERR,
                                        0);
    return UNC_UPPL_RC_ERR_TRANSACTION_START;
  }
  for (uint32_t config_count = 0; \
  config_count < vec_key_boundary_modified.size(); config_count++) {
    key_boundary_t *ptr_key_boundary = reinterpret_cast<key_boundary_t *>
    (vec_key_boundary_modified[config_count]);
    if (ptr_key_boundary == NULL) {
      continue;
    }
    if (row_status == CREATED)
      boundary_created_.push_back(*ptr_key_boundary);
    if (row_status == UPDATED)
      boundary_updated_.push_back(*ptr_key_boundary);
    if (row_status == DELETED) {
      // check whether it is already in running
      vector<string> vect_bdry_key_value;
      vect_bdry_key_value.push_back(
          (const char*)ptr_key_boundary->boundary_id);
      UncRespCode key_exist_running = kt_boundary.IsKeyExists(
          db_conn, UNC_DT_RUNNING,
          vect_bdry_key_value);
      if (key_exist_running == UNC_RC_SUCCESS) {
        boundary_deleted_.push_back(*ptr_key_boundary);
        // Check whether Boundary is referred in Running DB in Logical layer
        if (config_mode == TC_CONFIG_REAL) {
          UncRespCode validate_status = kt_boundary.SendSemanticRequestToUPLL(
                                                              ptr_key_boundary,
                                                             UNC_DT_RUNNING);
          if (validate_status != UNC_RC_SUCCESS) {
            // log error and send error response
            pfc_log_error("Boundary is referred in Logical, "
                                      "so delete is not allowed");
            return UNC_UPPL_RC_FAILURE;
          }
        }
      } else if (key_exist_running == UNC_UPPL_RC_ERR_DB_ACCESS) {
        // Error retrieving information from database, send failure
        pfc_log_info(
            "Error retrieving information from running db, return txn error");
        TcLibModule* tclib_ptr = static_cast<TcLibModule*>
          (TcLibModule::getInstance(TCLIB_MODULE_NAME));
        tclib_ptr->TcLibWriteControllerInfo("",
                                        UNC_RC_INTERNAL_ERR,
                                        0);

        return UNC_UPPL_RC_ERR_TRANSACTION_START;
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
  return UNC_RC_SUCCESS;
}

/**SendControllerConfigToLogical
 * @Description : This function is used to send the modified controller 
 *                configuration to logical.
 * @param[in]   : None
 * @return      : UNC_RC_SUCCESS if configuration is sent propertly 
 *                UNC_UPPL_RC_ERR_* in case of failure
 * */

UncRespCode TransactionRequest::SendControllerConfigToLogical(
                                  OdbcmConnectionHandler *db_conn) {
  UncRespCode logical_result = UNC_RC_SUCCESS;
  Kt_Controller kt_controller;
  // Sending the Deleted Controller's Info
  vector<key_ctr_t> :: iterator it_controller = controller_deleted_.begin();
  for (; it_controller != controller_deleted_.end(); ++it_controller) {
    key_ctr_t key_ctr_obj = *it_controller;
    logical_result =
        kt_controller.SendUpdatedControllerInfoToUPLL(
            UNC_DT_RUNNING,
            UNC_OP_DELETE,
            UNC_KT_CONTROLLER,
            reinterpret_cast<void*>(&key_ctr_obj),
            NULL);
    if (logical_result != UNC_RC_SUCCESS) {
      pfc_log_error("SendUpdatedControllerInfoToUPLL failed in DELETE with"
                     "error code:%d", logical_result);
      return logical_result;
    }
  }

  // Sending Created Controllers Info
  it_controller = controller_created_.begin();
  for (; it_controller != controller_created_.end(); ++it_controller) {
    key_ctr_t key_ctr_obj = *it_controller;
    vector<void *> vect_ctr_key, vect_ctr_val;
    vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
    logical_result = kt_controller.ReadInternal(db_conn,
                                                        vect_ctr_key,
                                                        vect_ctr_val,
                                                        UNC_DT_CANDIDATE,
                                                        UNC_OP_READ);
    if (logical_result != UNC_RC_SUCCESS) {
      pfc_log_error("SendUpdatedControllerInfoToUPLL failed-CRE"
          "ctr:%s errcode:%d", key_ctr_obj.controller_name, logical_result);
      return logical_result;
    }
    val_ctr_st_t *val_ctr_new = reinterpret_cast<val_ctr_st_t*>
                                 (vect_ctr_val[0]);
    key_ctr_t *key_ctr_new = reinterpret_cast<key_ctr_t*>(vect_ctr_key[0]);
    logical_result =
      kt_controller.SendUpdatedControllerInfoToUPLL(
          UNC_DT_RUNNING,
          UNC_OP_CREATE,
          UNC_KT_CONTROLLER,
          vect_ctr_key[0],
          reinterpret_cast<void*>(&val_ctr_new->controller));
    if (logical_result != UNC_RC_SUCCESS) {
      pfc_log_error("SendUpdatedControllerInfoToUPLL failed-CRE"
          "ctr:%s errcode:%d", key_ctr_obj.controller_name, logical_result);
      delete key_ctr_new;
      key_ctr_new = NULL;
      delete val_ctr_new;
      val_ctr_new = NULL;
      return logical_result;
    }
    delete key_ctr_new;
    key_ctr_new = NULL;
    delete val_ctr_new;
    val_ctr_new = NULL;
  }

  // Sending Updated Controllers Info
  it_controller = controller_updated_.begin();
  for (; it_controller != controller_updated_.end(); ++it_controller) {
    key_ctr_t key_ctr_obj = *it_controller;
    vector<void *> vect_ctr_key, vect_ctr_val;
    vect_ctr_key.push_back(reinterpret_cast<void *>(&key_ctr_obj));
    logical_result = kt_controller.ReadInternal(db_conn,
                                                        vect_ctr_key,
                                                        vect_ctr_val,
                                                        UNC_DT_CANDIDATE,
                                                        UNC_OP_READ);
    if (logical_result != UNC_RC_SUCCESS) {
      pfc_log_error("SendUpdatedControllerInfoToUPLL failed-UPD"
          "ctr:%s errcode:%d", key_ctr_obj.controller_name, logical_result);
      return logical_result;
    }
    val_ctr_st_t *val_ctr_new = reinterpret_cast<val_ctr_st_t*>
                                 (vect_ctr_val[0]);
    key_ctr_t *key_ctr_new = reinterpret_cast<key_ctr_t*>(vect_ctr_key[0]);
    logical_result =
      kt_controller.SendUpdatedControllerInfoToUPLL(
          UNC_DT_RUNNING,
          UNC_OP_UPDATE,
          UNC_KT_CONTROLLER,
          vect_ctr_key[0],
          reinterpret_cast<void*>(&val_ctr_new->controller));
    if (logical_result != UNC_RC_SUCCESS) {
      pfc_log_error("SendUpdatedControllerInfoToUPLL failed-UPDATE"
          "ctr:%s errcode:%d", key_ctr_obj.controller_name, logical_result);
      delete key_ctr_new;
      key_ctr_new = NULL;
      delete val_ctr_new;
      val_ctr_new = NULL;
      return logical_result;
    }
    delete key_ctr_new;
    key_ctr_new = NULL;
    delete val_ctr_new;
    val_ctr_new = NULL;
  }
  return logical_result;
}
