/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    Physical ITC
 * @file     physicalitc.cc
 *
 */

#include "physical_itc.hh"
#include "itc_import_request.hh"
#include "itc_db_config.hh"
#include "itc_notification_request.hh"
#include "itc_configuration_request.hh"
#include "itc_read_request.hh"

using unc::uppl::PhysicalLayer;
using unc::uppl::ImportRequest;
using unc::uppl::NotificationRequest;

/*  InternalTransactionCoordinator
 *  @Description    : constructor function
 *  @param[in]: NA
 *  @return   : NA
 * */
InternalTransactionCoordinator::InternalTransactionCoordinator() {
  // physical_core_= NULL;
  transaction_req_= NULL;
  db_config_req_= NULL;
  system_state_change_req_= NULL;
  audit_req_= NULL;
  audit_state_ = AUDIT_END;
  import_state_ = IMPORT_END;
  trans_state_ = TRANS_END;
  config_request_status_ = false;
}

/*  ~InternalTransactionCoordinator
 *  @Description    : Destructor function
 *  @param[in]:  NA
 *  @return   :  NA
 * */
InternalTransactionCoordinator::~InternalTransactionCoordinator() {
  if (transaction_req_ != NULL) {
    delete transaction_req_;
    transaction_req_ = NULL;
  }
  if (db_config_req_ != NULL) {
    delete db_config_req_;
    db_config_req_ = NULL;
  }
  if (system_state_change_req_ != NULL) {
    delete system_state_change_req_;
    system_state_change_req_ = NULL;
  }
  if (audit_req_ != NULL) {
    delete audit_req_;
    audit_req_ = NULL;
  }
}

/*  transaction_req
 *  @Description    : This function creates a new instance of TransactionRequest
 *  @param[in]: NA
 *  @return   : TransactionRequest
 * */
TransactionRequest *InternalTransactionCoordinator::transaction_req() {
  if (transaction_req_ == NULL)
    transaction_req_= new TransactionRequest();
  return transaction_req_;
}

/* audit_req
 *  @Description    : This function creates a new instance of AuditRequest
 *  @param[in]: NA
 *  @return   : AuditRequest
 * */
AuditRequest *InternalTransactionCoordinator::audit_req() {
  // create AuditReq object
  if (audit_req_== NULL)
    audit_req_= new AuditRequest();
  return audit_req_;
}

/* db_config_req
 *  @Description    : This function creates a new instance of DBConfigurationRequest
 *  @param[in]: NA
 *  @return   : DBConfigurationRequest
 * */
DBConfigurationRequest *InternalTransactionCoordinator::db_config_req() {
  // create DBConfigReq object
  if (db_config_req_== NULL)
    db_config_req_= new DBConfigurationRequest();
  return db_config_req_;
}

/* system_state_change_req
 *  @Description    : This function creates a new instance of SystemStateChangeRequest
 *  @param[in]: NA
 *  @return   : SystemStateChangeRequest
 * */
SystemStateChangeRequest
*InternalTransactionCoordinator::system_state_change_req() {
  if (system_state_change_req_== NULL)
    system_state_change_req_= new SystemStateChangeRequest();
  return system_state_change_req_;
}

/*  PerformConfigIdValidation
 *  @Description    : This function calls the validate funciton in physical core
 *  @param[in]: sesssion argument, config id, session id
 *  @return   : Success or associated error code
 * */
UpplReturnCode InternalTransactionCoordinator::PerformConfigIdValidation(
    ServerSession &session,
    uint32_t sessionId,
    uint32_t configId) {
  // pass config id received in the message to the physical core for validation

  UpplReturnCode validation_status = PhysicalLayer::get_instance() \
      ->get_physical_core() \
      ->ValidateConfigId(sessionId, configId);
  return validation_status;
}

/*  ProcessReq
 *  @Description : This function gets the request type and invokes request obj
 *  @param[in]: ipc session id, configuration id and session
 *  @return   : Success or associated error code
 * */
UpplReturnCode InternalTransactionCoordinator::ProcessReq(
    ServerSession &session,
    pfc_ipcid_t service_id) {
  UpplReturnCode resp_code = UPPL_RC_SUCCESS;
  if (service_id == 0 || service_id == 1) {
    // create request header object to get arguments from session
    physical_request_header obj_req_hdr;
    int parse_ret = PhyUtil::sessGetReqHeader(session, obj_req_hdr);
    if (parse_ret != 0) {
      pfc_log_error("Unable to parse ipc structure. BAD_REQUEST error");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    PhyUtil::printReqHeader(obj_req_hdr);
    physical_response_header rsh;
    PhyUtil::getRespHeaderFromReqHeader(obj_req_hdr, rsh);

    // get system state to discard the config requests in case of standby
    UpplReturnCode standby_status = UPPL_RC_SUCCESS;
    standby_status = PhysicalLayer::get_instance()->get_physical_core() \
        ->ValidateStandbyRequests(obj_req_hdr.operation);

    if (standby_status != UPPL_RC_SUCCESS) {
      rsh.result_code = UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
      pfc_log_error("Config not allowed in standby");
      int err = PhyUtil::sessOutRespHeader(session, rsh);
      if (err != 0) {
        return UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UPPL_RC_SUCCESS;
    }
    if (obj_req_hdr.data_type < UNC_DT_STATE ||
        obj_req_hdr.data_type > UNC_DT_AUDIT) {
      rsh.result_code = UPPL_RC_ERR_DATATYPE_NOT_SUPPORTED;
      pfc_log_error("datatype not supported");
      int err = PhyUtil::sessOutRespHeader(session, rsh);
      if (err != 0) {
        return UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UPPL_RC_SUCCESS;
    }
    switch (obj_req_hdr.operation) {
      case UNC_OP_CREATE:
      case UNC_OP_DELETE:
      case UNC_OP_UPDATE:
      {
        if (service_id != 0) {
          pfc_log_error(
              "Config Operation is provided with ServiceId other than 0");
          return UPPL_RC_ERR_BAD_REQUEST;
        }
        resp_code = ProcessConfigRequest(session, obj_req_hdr, rsh);
        break;
      }
      case UNC_OP_CONTROL:
      {
        pfc_log_info("Inside control request request - NOT_SUPPORTED");
        rsh.result_code = UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
        int err = PhyUtil::sessOutRespHeader(session, rsh);
        if (err != 0) {
          return UPPL_RC_ERR_IPC_WRITE_ERROR;
        }
        break;
      }
      case UNC_OP_READ:
      case UNC_OP_READ_NEXT:
      case UNC_OP_READ_BULK:
      case UNC_OP_READ_SIBLING_BEGIN:
      case UNC_OP_READ_SIBLING:
      case UNC_OP_READ_SIBLING_COUNT:
      {
        if (service_id != 1) {
          pfc_log_error(
              "Read Operation is provided with ServiceId other than 1");
          return UPPL_RC_ERR_BAD_REQUEST;
        }
        resp_code = ProcessReadRequest(session, obj_req_hdr, rsh);
        break;
      }
      default:
        rsh.result_code = UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
        pfc_log_error("Operation not supported");
        int err = PhyUtil::sessOutRespHeader(session, rsh);
        if (err != 0) {
          return UPPL_RC_ERR_IPC_WRITE_ERROR;
        }
        return UPPL_RC_SUCCESS;
    }
  } else if (service_id == 2) {
    uint32_t operation;
    session.getArgument(0, operation);
    pfc_log_debug("Received operation %d in service_id %d",
                  operation, service_id);
    switch (operation) {
      case UNC_OP_IS_CANDIDATE_DIRTY:
      {
        uint8_t dirty_status  = 0;
        ODBCM_RC_STATUS db_status = ODBCM_RC_SUCCESS;
        resp_code = UPPL_RC_SUCCESS;
        db_status = PhysicalLayer::get_instance()->get_odbc_manager()->
            IsCandidateDirty();
        if (db_status == ODBCM_RC_CANDIDATE_DIRTY) {
          dirty_status = 1;
        } else if (db_status != ODBCM_RC_SUCCESS) {
          resp_code = UPPL_RC_ERR_DB_GET;
        }
        session.addOutput(operation);
        session.addOutput((uint32_t)resp_code);
        session.addOutput(dirty_status);
        break;
      }
      case UNC_OP_IMPORT_CONTROLLER_CONFIG:
      case UNC_OP_MERGE_CONTROLLER_CONFIG:
      case UNC_OP_CLEAR_IMPORT_CONFIG:
      {
        resp_code = ProcessImportRequest(session, operation);
        break;
      }
      default:
        pfc_log_error("Operation not allowed due to bad request type");
        session.addOutput(operation);
        session.addOutput((uint32_t)UPPL_RC_ERR_OPERATION_NOT_SUPPORTED);
    }
  }
  return resp_code;
}

/* ProcessEvent
 *  @Description    : This function process the received notification event
 *  by creating notify req object
 *  @param[in]: ipc event
 *  @return   : response code
 **/
UpplReturnCode InternalTransactionCoordinator::ProcessEvent(
    const IpcEvent &event) {
  UpplReturnCode resp_code = UPPL_RC_SUCCESS;
  NotificationRequest notify_req;
  pfc_bool_t resp = notify_req.ProcessEvent(event);
  if (resp == PFC_TRUE) {
    resp_code = UPPL_RC_SUCCESS;
  } else {
    resp_code = UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  return resp_code;
}

/* ProcessConfigRequest
 *  @Description    : This function process the received configuration
 *  requests from north bound
 *  @param[in]: ipc session, parsed request header and response header
 *  @return   : response code and response header
 **/
UpplReturnCode InternalTransactionCoordinator::ProcessConfigRequest(
    ServerSession &session,
    physical_request_header obj_req_hdr,
    physical_response_header &rsh) {
  UpplReturnCode resp_code = UPPL_RC_SUCCESS;
  UpplReturnCode validate_status = PerformConfigIdValidation(
      session,
      obj_req_hdr.client_sess_id,
      obj_req_hdr.config_id);
  if (validate_status != UPPL_RC_SUCCESS) {
    if (validate_status != UPPL_RC_ERR_INVALID_CONFIGID) {
      pfc_log_error("ITC::Process Req:: Config id validation failed");
    }
    if (validate_status != UPPL_RC_ERR_INVALID_SESSIONID) {
      pfc_log_error("ITC::Process Req:: Session id validation failed");
    }
    rsh.result_code = validate_status;
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (err != 0) {
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return resp_code;
  }

  pfc_log_info("Inside configuration request");
  if ((audit_state() != AUDIT_END) || (import_state() != IMPORT_END)) {
    rsh.result_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_error("Config not allowed- operation in progress");
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (err != 0) {
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return resp_code;
  }

  if (config_request_status() == false) {
    set_config_request_status(true);
    // create configuration req object to invoke processreq function.
    ConfigurationRequest configuration_req;
    resp_code = (UpplReturnCode) configuration_req.ProcessReq(session);
    set_config_request_status(false);
  } else {
    rsh.result_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_error("Parallel Config operations are not allowed");
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (err != 0) {
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  return resp_code;
}

/* ProcessReadRequest
 *  @Description    : This function process the received read
 *  requests from north bound
 *  @param[in]: ipc session, parsed request header and response header
 *  @return   : response code and response header
 **/
UpplReturnCode InternalTransactionCoordinator::ProcessReadRequest(
    ServerSession &session,
    physical_request_header obj_req_hdr,
    physical_response_header &rsh) {
  UpplReturnCode resp_code = UPPL_RC_SUCCESS;
  if ((obj_req_hdr.data_type != UNC_DT_STATE) &&
      (obj_req_hdr.data_type != UNC_DT_CANDIDATE) &&
      (obj_req_hdr.data_type != UNC_DT_RUNNING) &&
      (obj_req_hdr.data_type != UNC_DT_STARTUP)) {
    rsh.result_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_error("operation not allowed for this dt state");
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (err != 0) {
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }
  if (obj_req_hdr.data_type == UNC_DT_CANDIDATE) {
    UpplReturnCode validate_status = PerformConfigIdValidation(
        session,
        obj_req_hdr.client_sess_id,
        obj_req_hdr.config_id);
    if (validate_status != UPPL_RC_SUCCESS) {
      if (validate_status != UPPL_RC_ERR_INVALID_CONFIGID) {
        pfc_log_error("ITC::Process Req:: Config id validation failed");
      }
      if (validate_status != UPPL_RC_ERR_INVALID_SESSIONID) {
        pfc_log_error("ITC::Process Req:: Session id validation failed");
      }
      rsh.result_code = validate_status;
      int err = PhyUtil::sessOutRespHeader(session, rsh);
      if (err != 0) {
        return UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UPPL_RC_SUCCESS;
    }
  }
  // Form read request object to invoke process request function
  ReadRequest read_req;
  resp_code = (UpplReturnCode) read_req.ProcessReq(session);
  return resp_code;
}

/* ProcessImportRequest
 *  @Description    : This function process the received import
 *  requests from north bound
 *  @param[in]: ipc session and import operation type
 *  @return   : response code
 **/
UpplReturnCode InternalTransactionCoordinator::ProcessImportRequest(
    ServerSession &session,
    uint32_t operation) {
  if (PhysicalLayer::get_instance()->get_physical_core()
      ->get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_info("Import Operations not allowed in standby");
    session.addOutput(operation);
    session.addOutput(UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY);
    return UPPL_RC_SUCCESS;
  }
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  key_ctr_t obj_key_ctr;
  memset(&obj_key_ctr, 0, sizeof(key_ctr_t));
  if (operation == UNC_OP_IMPORT_CONTROLLER_CONFIG) {
    // get controller name from ipc structure
    const char* ctr_name;
    int err = session.getArgument(1, ctr_name);
    if (err != 0) {
      pfc_log_info(
          "ctr_name is not present in UNC_OP_IMPORT_CONTROLLER_CONFIG");
      return UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_debug("Controller name received %s", ctr_name);
    memcpy(obj_key_ctr.controller_name, ctr_name, strlen(ctr_name)+1);
  }
  if ((audit_state() != AUDIT_END) || (import_state() != IMPORT_END) ||
      (config_request_status() == true)) {
    ret_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_error("Import not allowed- operation in progress");
  } else {
    // create import request object to invoke processreq
    ImportRequest import_req;
    ret_code = import_req.ProcessRequest(operation,
                                         obj_key_ctr);
  }
  session.addOutput(operation);
  session.addOutput((uint32_t)ret_code);
  return UPPL_RC_SUCCESS;
}
