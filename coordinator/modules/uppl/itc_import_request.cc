/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    ImportRequest
 * @file     itc_import_request.cc
 **/

#include "itc_import_request.hh"
using unc::uppl::ImportRequest;

/**ImportRequest
 * @Description : This function initializes the member data
 * @param[in]   : None
 * @Return      : None
 * */
ImportRequest::ImportRequest()  {
}

/**~ImportRequest
 * @Description : This function releases memory allocated to
 *                pointer member data
 * @param[in]   : None
 * @Return      : None
 * */
ImportRequest::~ImportRequest()  {
}

/**ProcessRequest
 * @Description : This function receives the import request from the internal
 *                transaction coordinator and returns the processing result
 * @param[in]   : unc_keytype_operation_t - UNC_OP_* operations related to
 *                import
 *                key_struct - specifies key instance of Kt_Controller
 * @Return      : UNC_RC_SUCCESS is returned when the response
 *                is added to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc
 *                response could not be added to session
 * */
UncRespCode ImportRequest::ProcessRequest(OdbcmConnectionHandler *db_conn,
                                             uint32_t unc_keytype_operation_t,
                                             key_ctr_t obj_key_ctr)  {
  UncRespCode result_code = UNC_RC_SUCCESS;
  pfc_log_info("Process the import request");
  switch (unc_keytype_operation_t)  {
    case UNC_OP_IMPORT_CONTROLLER_CONFIG:
      result_code = StartImport(db_conn, obj_key_ctr);
      if (result_code != UNC_RC_SUCCESS) {
        pfc_log_info("Import Request Failed");
      }
      break;
    case UNC_OP_MERGE_CONTROLLER_CONFIG:
      result_code = MergeConfiguration();
      break;
    case UNC_OP_CLEAR_IMPORT_CONFIG:
      result_code = ClearImportConfig();
      break;
    default:
      result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  return result_code;
}

/**StartImport
 * @Description : This function is invoked when the import request comes from
 *                internal transaction coordinator.It Checks the oper_status
 *                of the controller and whether candidate is dirty or not
 *                and returns the response
 * @param[in]   : key_struct - specifies key instance of KT_Controller
 * @Return      : UNC_RC_SUCCESS is returned when the response
 *                is added to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc
 *                response could not be added to session 
 * */
UncRespCode ImportRequest::StartImport(OdbcmConnectionHandler *db_conn,
                                          key_ctr_t obj_key_ctr)  {
  UncRespCode result_code = UNC_RC_SUCCESS;
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();
  // Check the import precondition
  Kt_Controller KtObj;
  uint8_t oper_status = 0;
  /* Checks controller existence and its oper status */
  pfc_log_debug("Get controller oper Status");
  UncRespCode read_status = KtObj.GetOperStatus(
      db_conn, UNC_DT_RUNNING, &obj_key_ctr, oper_status);
  if (read_status == UNC_RC_SUCCESS) {
    pfc_log_debug("Received oper_status %d", oper_status);
    if (oper_status == UPPL_CONTROLLER_OPER_AUDITING) {
      // Controller audit is in progress, return error
      pfc_log_info("Controller audit is in progress, return error");
      return UNC_UPPL_RC_ERR_INVALID_STATE;
    }
  }
  ODBCM_RC_STATUS db_status = ODBCM_RC_SUCCESS;
  db_status = PhysicalLayer::get_instance()->get_odbc_manager()->
      IsCandidateDirty(db_conn);
  pfc_log_debug("Candidate Dirty status %d", db_status);
  if (itc_trans->trans_state() != TRANS_END ||
      db_status == ODBCM_RC_CANDIDATE_DIRTY) {
    pfc_log_info("Start Import Unsuccessful - Candidate is dirty");
    return UNC_UPPL_RC_ERR_CANDIDATE_IS_DIRTY;
  }
  uint8_t audit_flag = UPPL_AUTO_AUDIT_DISABLED;
  result_code = KtObj.CheckAuditFlag(db_conn, obj_key_ctr, audit_flag);
  if (result_code == UNC_RC_SUCCESS && audit_flag == UPPL_AUTO_AUDIT_ENABLED) {
     pfc_log_info("Audit is enable,Import not allowed");
     return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  return result_code;
}

/**MergeConfiguration
 * @Description : This function returns success for Merge
 *                configuration Operation.
 * @param[in]   : key_struct - specifies key instance of KT_Controller
 * @Return      : UNC_RC_SUCCESS if the merge is successful
 * */
UncRespCode ImportRequest::MergeConfiguration() {
  UncRespCode result_code = UNC_RC_SUCCESS;
  pfc_log_info("Returning Success for MergeConfiguration");
  return result_code;
}

/**ClearImportConfig
 * @Description    : This function returns success for Clear import
 *                   configuration
 * @param[in]      : key_struct - specifies key instance of KT_Controller
 * @Return         : UNC_RC_SUCCESS if the clear import configuration is
 *                   successful
 * */
UncRespCode ImportRequest::ClearImportConfig() {
  UncRespCode result_code = UNC_RC_SUCCESS;
  pfc_log_info("Returning Success for ClearImportConfig");
  return result_code;
}
