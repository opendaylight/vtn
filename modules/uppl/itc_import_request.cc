/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    ImportRequest
 * @file     import.cc
 *
 */

#include "itc_import_request.hh"
using unc::uppl::ImportRequest;

/**ImportRequest
 * @Description   : Constructor which will initializes the member data
 * @param[in]     : None
 * @Return        : void
 * **/
ImportRequest::ImportRequest()  {
}

/** ~ImportRequest()
 * @Description   : Destructor to release any memory allocated to pointer member data
 * @param[in]     : None
 * @Return        : void
 * **/
ImportRequest::~ImportRequest()  {
}

/**ProcessRequest()
 * @Description    : This function receives the import request and process that request
 * @param[in]      : unc_keytype_operation_t, key_struct
 * @Return         : UpplReturnCode(enum)
 **/
UpplReturnCode ImportRequest::ProcessRequest(uint32_t unc_keytype_operation_t,
                                             key_ctr_t obj_key_ctr)  {
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  pfc_log_info("Process the import request");
  switch (unc_keytype_operation_t)  {
    case UNC_OP_IMPORT_CONTROLLER_CONFIG:
      result_code = StartImport(obj_key_ctr);
      if (result_code != UPPL_RC_SUCCESS) {
        pfc_log_info("Import Request:Candidate is dirty");
      }
      break;
    case UNC_OP_MERGE_CONTROLLER_CONFIG:
      result_code = MergeConfiguration(obj_key_ctr);
      break;
    case UNC_OP_CLEAR_IMPORT_CONFIG:
      result_code = ClearImportConfig(obj_key_ctr);
      break;
    default:
      result_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  return result_code;
}

/**StartImport()
 * @Description    : This function checks whether candidate is dirty
 * @param[in]      : key_struct
 * @Return         : UpplReturnCode(enum)
 **/

UpplReturnCode ImportRequest::StartImport(key_ctr_t obj_key_ctr)  {
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();
  // Check the import precondition
  Kt_Controller KtObj;
  uint8_t oper_status = 0;
  /* Checks controller existence and its oper status */
  pfc_log_info("Get the oper Status");
  UpplReturnCode read_status = KtObj.GetOperStatus(
      UNC_DT_RUNNING, &obj_key_ctr, oper_status);
  if (read_status == UPPL_RC_SUCCESS) {
    pfc_log_debug("Received oper_status %d", oper_status);
    if (oper_status == UPPL_CONTROLLER_OPER_AUDITING) {
      // Controller audit is in progress, return error
      pfc_log_info("Controller audit is in progress, return error");
      return UPPL_RC_ERR_INVALID_STATE;
    }
  }
  ODBCM_RC_STATUS db_status = ODBCM_RC_SUCCESS;
  db_status = PhysicalLayer::get_instance()->get_odbc_manager()->
      IsCandidateDirty();
  pfc_log_debug("Candidate Dirty status %d", db_status);
  if (itc_trans->trans_state() != TRANS_END ||
      db_status == ODBCM_RC_CANDIDATE_DIRTY) {
    pfc_log_info("Start Import Unsuccessful - Candidate is dirty");
    result_code = UPPL_RC_ERR_CANDIDATE_IS_DIRTY;
  }
  return result_code;
}

/**MergeConfiguration()
 * @Description    : This function returns success for Merge Import Operation.
 * @param[in]      : key_struct
 * @Return         : UpplReturnCode(enum)
 **/

UpplReturnCode ImportRequest::MergeConfiguration(key_ctr_t obj_key_ctr) {
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  pfc_log_info("Returning Success for MergeConfiguration");
  return result_code;
}

/**ClearImportConfig()
 * @Description    : This function returns success for Clear import configuration
 * @param[in]      : key_struct
 * @Return         : UpplReturnCode(enum)
 **/
UpplReturnCode ImportRequest::ClearImportConfig(key_ctr_t obj_key_ctr) {
  UpplReturnCode result_code = UPPL_RC_SUCCESS;
  pfc_log_info("Returning Success for ClearImportConfig");
  return result_code;
}
