/*
 * Copyright (c) 2012-2015 NEC Corporation
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
// static variable initialization
InternalTransactionCoordinator* InternalTransactionCoordinator::
internal_transaction_coordinator_ = NULL;

/**InternalTransactionCoordinator 
 * @Description : Constructor which will invoke the respective configuration
 *                classes
 *                variables
 * @param[in]   : None
 * @return      : None
 **/
InternalTransactionCoordinator::InternalTransactionCoordinator()
:
                          config_request_status_(false),
                          trans_state_(TRANS_END),
                          transaction_req_(NULL),
                          db_config_req_(NULL),
                          system_state_change_req_(NULL),
                          audit_req_(NULL) {
}

/**
 * @Description : The function returns singleton instance of
 * PhysicalCore class
 */
InternalTransactionCoordinator* InternalTransactionCoordinator::
get_internaltransactioncoordinator() {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  physical_layer->phyitc_mutex_.lock();
  if (internal_transaction_coordinator_ == NULL) {
    internal_transaction_coordinator_ =  new InternalTransactionCoordinator();
  }
  physical_layer->phyitc_mutex_.unlock();
  return internal_transaction_coordinator_;
}

/**~InternalTransactionCoordinator 
 * @Description : Destructor which delete all the pointers to the 
 *                configuration classes
 * @param[in]   : None
 * @return      : None
 **/
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

/**transaction_req
 * @Description : This function creates a new instance of TransactionRequest
 * @param[in]   : None
 * @return      : Pointer to TransactionRequest class is returned
 **/
TransactionRequest *InternalTransactionCoordinator::transaction_req() {
  if (transaction_req_ == NULL)
    transaction_req_= new TransactionRequest();
  return transaction_req_;
}

/**audit_req 
 * @Description : This function creates a new instance of AuditRequest
 * @param[in]   : None
 * @return      : Pointer to the AuditRequest class is returned
 **/
AuditRequest *InternalTransactionCoordinator::audit_req() {
  // create AuditReq object
  if (audit_req_== NULL)
    audit_req_= new AuditRequest();
  return audit_req_;
}

/**db_config_req 
 * @Description : This function creates a new instance of
 *                DBConfigurationRequest
 * @param[in]   : None
 * @return      : Pointer to the DBConfigurationRequest class is returned
 **/
DBConfigurationRequest *InternalTransactionCoordinator::db_config_req() {
  // create DBConfigReq object
  if (db_config_req_== NULL)
    db_config_req_= new DBConfigurationRequest();
  return db_config_req_;
}

/**system_state_change_req
 * @Description : This function creates a new instance of
 *                SystemStateChangeRequest
 * @param[in]   : None
 * @return      : Pointer to the SystemStateChangeRequest class is returned
 **/
SystemStateChangeRequest
*InternalTransactionCoordinator::system_state_change_req() {
  if (system_state_change_req_== NULL)
    system_state_change_req_= new SystemStateChangeRequest();
  return system_state_change_req_;
}

/**PerformConfigIdValidation 
 * @Description    : This function validates the config id received in the
 *                   message by calling the validate funciton in physical core
 * @param[in]      : session - Object of ServerSession where the request
 *                   argument present
 *                   session_id - ipc session id used for TC validation
 *                   config_id - configuration id used for TC validation
 * @return         : UNC_RC_SUCCESS - will be returned if the config ID
 *                   and session ID are valid
 *                   UNC_UPPL_RC_ERR_INVALID_CONFIGID will be returned if 
 *                   config id is invalid or UNC_UPPL_RC_ERR_INVALID_SESSIONID
 *                   will be returned if session id is invalid
 **/
UncRespCode InternalTransactionCoordinator::PerformConfigIdValidation(
    ServerSession &session,
    uint32_t sessionId,
    uint32_t configId) {
  // pass config id received in the message to the physical core for validation

  UncRespCode validation_status = PhysicalLayer::get_instance() \
      ->get_physical_core() \
      ->ValidateConfigId(sessionId, configId);
  return validation_status;
}

/**ProcessReq 
 * @Description : This function validates the request based on the service id
 *                specified. For CREATE/UPDATE/DELETE operations the service
 *                id must be 0. For READ operations the service id must be 1
 * @param[in]   : session - Object of ServerSession
                  service_id - type of ipc service id
 * @return      : UNC_RC_SUCCESS - will be returned if service id validation 
 *                is success for appropriate operation type else UNC_UPPL_RC_ERR_*
 *                will be returned. 
 **/
UncRespCode InternalTransactionCoordinator::ProcessReq(
    ServerSession &session,
    pfc_ipcid_t service_id) {
  UncRespCode resp_code = UNC_RC_SUCCESS;
  int err = 0;
  if (service_id == 1 || service_id == 0) {
    // create request header object to get arguments from session
    physical_request_header obj_req_hdr;
    int parse_ret = PhyUtil::sessGetReqHeader(session, obj_req_hdr);
    if (parse_ret != 0) {
      pfc_log_error("Unable to parse ipc structure. BAD_REQUEST error");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    PhyUtil::printReqHeader(obj_req_hdr);
    physical_response_header rsh;
    PhyUtil::getRespHeaderFromReqHeader(obj_req_hdr, rsh);

    // get system state to discard the config requests in case of standby
    UncRespCode standby_status = UNC_RC_SUCCESS;
    standby_status = PhysicalLayer::get_instance()->get_physical_core() \
        ->ValidateStandbyRequests(obj_req_hdr.operation);

    if (standby_status != UNC_RC_SUCCESS) {
      rsh.result_code = UNC_UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
      pfc_log_error("Config not allowed in standby");
      err = PhyUtil::sessOutRespHeader(session, rsh);
      if (err != 0) {
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UNC_RC_SUCCESS;
    }
    if (obj_req_hdr.data_type < UNC_DT_STATE ||
        obj_req_hdr.data_type > UNC_DT_AUDIT) {
      rsh.result_code = UNC_UPPL_RC_ERR_DATATYPE_NOT_SUPPORTED;
      pfc_log_error("datatype not supported");
      err = PhyUtil::sessOutRespHeader(session, rsh);
      if (err != 0) {
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UNC_RC_SUCCESS;
    }
    switch (obj_req_hdr.operation) {
      case UNC_OP_CREATE:
      case UNC_OP_DELETE:
      case UNC_OP_UPDATE: {
        if (service_id != 0) {
          pfc_log_error(
              "Config Operation is provided with ServiceId other than 0");
          return UNC_UPPL_RC_ERR_BAD_REQUEST;
        }
        //  checking unc_mode and session id, if session_id is not valid
        //  with respect to uncmode, return UNC_UPPL_RC_ERR_INVALID_SESSIONID
        UncMode unc_mode = PhysicalLayer::get_instance()->\
                          get_physical_core()->getunc_mode();
        if (unc_mode == UNC_COEXISTS_MODE &&
            rsh.client_sess_id != USESS_ID_STARTUPPROXY) {
          pfc_log_error("unc_mode is UNC_COEXISTS_MODE but session id is"
              " NOT USESS_ID_STARTUPPROXY");
          return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
        } else if (unc_mode == UNC_SEPARATE_MODE &&
                   rsh.client_sess_id == USESS_ID_STARTUPPROXY) {
          pfc_log_error("unc_mode is UNC_SEPARATE_MODE but session id is"
              " USESS_ID_STARTUPPROXY");
          return UNC_UPPL_RC_ERR_INVALID_SESSIONID;
        }
        resp_code = ProcessConfigRequest(session, obj_req_hdr, rsh);
        break;
       }
      case UNC_OP_CONTROL:
        pfc_log_info("Inside control request request - NOT_SUPPORTED");
        rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
        err = PhyUtil::sessOutRespHeader(session, rsh);
        if (err != 0) {
          return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
        }
        break;
      case UNC_OP_READ:
      case UNC_OP_READ_NEXT:
      case UNC_OP_READ_BULK:
      case UNC_OP_READ_SIBLING_BEGIN:
      case UNC_OP_READ_SIBLING:
      case UNC_OP_READ_SIBLING_COUNT:
        if (service_id != 1) {
          pfc_log_error(
              "Read Operation is provided with ServiceId other than 1");
          return UNC_UPPL_RC_ERR_BAD_REQUEST;
        }
        resp_code = ProcessReadRequest(session, obj_req_hdr, rsh);
        break;
      default:
        rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
        pfc_log_error("Operation not supported");
        err = PhyUtil::sessOutRespHeader(session, rsh);
        if (err != 0) {
          return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
        }
        return UNC_RC_SUCCESS;
    }
  } else if (service_id == 2) {
    uint32_t operation = UNC_OP_INVALID;  // cpptest issue fix
    int err = session.getArgument(0, operation);
    if (err != 0) {
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    pfc_log_info("Received operation %d in service_id %d",
                  operation, service_id);
    switch (operation) {
      case UNC_OP_IS_CANDIDATE_DIRTY:
        resp_code = ProcessIsCandidateDirty(session, operation);
        break;
      case UNC_OP_IMPORT_CONTROLLER_CONFIG:
      case UNC_OP_MERGE_CONTROLLER_CONFIG:
      case UNC_OP_CLEAR_IMPORT_CONFIG:
        resp_code = ProcessImportRequest(session, operation);
        break;
      default:
        pfc_log_error("Operation not allowed due to bad request type");
        session.addOutput(operation);
        session.addOutput((uint32_t)UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED);
    }
  }
  return resp_code;
}

/**ProcessEvent
 * @Description : This function process the received notification event
 *                by creating notify req object
 * @param[in]   : object of IpcEvent class
 * @return      : UNC_RC_SUCCESS is returned when the response is added
 *                to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not
 *                be added to sess.
 **/
UncRespCode InternalTransactionCoordinator::ProcessEvent(
    const IpcEvent &event) {
  UncRespCode resp_code = UNC_RC_SUCCESS;
  NotificationRequest notify_req;
  pfc_bool_t resp = notify_req.ProcessEvent(event);
  if (resp == PFC_TRUE) {
    resp_code = UNC_RC_SUCCESS;
  } else {
    resp_code = UNC_UPPL_RC_ERR_NOTIFICATION_HANDLING_FAILED;
  }
  return resp_code;
}

/**ProcessConfigRequest
 * @Description : This function process the received configuration
 *                requests from north bound
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present
 *                obj_req_hdr - object of physical request header
 *                &rsh - object of the physical response header 
 * @return      : UNC_RC_SUCCESS if validation of config_id is success
 *                or UNC_UPPL_RC_ERR_* if validation fails.
 **/
UncRespCode InternalTransactionCoordinator::ProcessConfigRequest(
    ServerSession &session,
    physical_request_header &obj_req_hdr,
    physical_response_header &rsh) {
  UncRespCode resp_code = UNC_RC_SUCCESS;
  UncRespCode validate_status = PerformConfigIdValidation(
      session,
      obj_req_hdr.client_sess_id,
      obj_req_hdr.config_id);
  if (validate_status != UNC_RC_SUCCESS) {
    if (validate_status == UNC_UPPL_RC_ERR_INVALID_CONFIGID) {
      pfc_log_error("ITC::Process Req:: Config id validation failed");
    }
    if (validate_status == UNC_UPPL_RC_ERR_INVALID_SESSIONID) {
      pfc_log_error("ITC::Process Req:: Session id validation failed");
    }
    if (validate_status == UNC_UPPL_RC_FAILURE) {
      pfc_log_error("ITC::Process Req:: Config mode validation failed");
    }
    rsh.result_code = validate_status;
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (err != 0) {
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return resp_code;
  }
  if (config_request_status() == false) {
    set_config_request_status(true);
    // create configuration req object to invoke processreq function.
    ConfigurationRequest configuration_req;
    resp_code = (UncRespCode) configuration_req.ProcessReq(session,
                                                              obj_req_hdr);
    set_config_request_status(false);
  } else {
    rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_error("Parallel Config operations are not allowed");
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (err != 0) {
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  return resp_code;
}

/**ProcessReadRequest
 * @Description : This function process the received read
 *                requests from north bound
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present
 *                obj_req_hdr - object of physical request header
 *                &rsh - object of the physical response header
 * @return      : UNC_RC_SUCCESS if validation of config_id is success
 *                or UNC_UPPL_RC_ERR_* if validation fails.
 **/
UncRespCode InternalTransactionCoordinator::ProcessReadRequest(
    ServerSession &session,
    physical_request_header &obj_req_hdr,
    physical_response_header &rsh) {
  if (!(obj_req_hdr.data_type == UNC_DT_STATE ||
      obj_req_hdr.data_type == UNC_DT_RUNNING ||
      obj_req_hdr.data_type == UNC_DT_CANDIDATE ||
      obj_req_hdr.data_type == UNC_DT_STARTUP)) {
    rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_error("operation not allowed for this dt state");
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (err != 0) {
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  //  skip the config id validation if logical's fixed session_id
  if (obj_req_hdr.data_type == UNC_DT_CANDIDATE &&
                        USESS_ID_UPLL != obj_req_hdr.client_sess_id) {
      UncRespCode validate_status = PerformConfigIdValidation(
          session, obj_req_hdr.client_sess_id,
          obj_req_hdr.config_id);
    if (validate_status != UNC_RC_SUCCESS) {
      if (validate_status == UNC_UPPL_RC_ERR_INVALID_CONFIGID) {
        pfc_log_error("ITC::Process Req:: Config id validation failed");
      }
      if (validate_status == UNC_UPPL_RC_ERR_INVALID_SESSIONID) {
        pfc_log_error("ITC::Process Req:: Session id validation failed");
      }
      if (validate_status == UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED) {
        pfc_log_error("ITC::Process Req:: Config mode validation failed");
      }
      rsh.result_code = validate_status;
      int err = PhyUtil::sessOutRespHeader(session, rsh);
      if (err != 0) {
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      return UNC_RC_SUCCESS;
    }
  }
  // Form read request object to invoke process request function
  ReadRequest read_req;
  return (UncRespCode) read_req.ProcessReq(session, obj_req_hdr);
}

/**ProcessImportRequest
 * @Description : This function process the received import
 *                requests from north bound
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present
 *                operation - UNC_OP_* specifies the operation
 * @return      : UNC_RC_SUCCESS is returned when the response is added
 *                to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not
 *                be added to sess.
 **/
UncRespCode InternalTransactionCoordinator::ProcessImportRequest(
    ServerSession &session,
    uint32_t operation) {
  if (PhysicalLayer::get_instance()->get_physical_core()
      ->get_system_state() == UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_info("Import Operations not allowed in standby");
    session.addOutput(operation);
    session.addOutput(UNC_UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY);
    return UNC_RC_SUCCESS;
  }
  UncRespCode ret_code = UNC_RC_SUCCESS;
  key_ctr_t obj_key_ctr;
  memset(&obj_key_ctr, 0, sizeof(key_ctr_t));
  uint32_t session_id, config_id;
  if (operation == UNC_OP_IMPORT_CONTROLLER_CONFIG) {
    // get controller name from ipc structure
    const char* ctr_name;
    int err = session.getArgument(1, ctr_name);
    err |= session.getArgument(2, session_id);
    err |= session.getArgument(3, config_id);
    if (err != 0) {
      pfc_log_info(
          "Attributes are missing in UNC_OP_IMPORT_CONTROLLER_CONFIG");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
    pfc_log_debug("Controller name received %s", ctr_name);
    memcpy(obj_key_ctr.controller_name, ctr_name, strlen(ctr_name)+1);
  } else if (operation == UNC_OP_MERGE_CONTROLLER_CONFIG) {
    int err = session.getArgument(1, session_id);
    err |= session.getArgument(2, config_id);
    if (err != 0) {
      pfc_log_info(
          "Attributes are missing in UNC_OP_MERGE_CONTROLLER_CONFIG");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
  } else if (operation == UNC_OP_CLEAR_IMPORT_CONFIG) {
    int err = session.getArgument(1, session_id);
    err |= session.getArgument(2, config_id);
    if (err != 0) {
      pfc_log_info(
          "Attributes are missing in UNC_OP_CLEAR_IMPORT_CONFIG");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
    }
  }
  if (config_request_status() == true) {
    ret_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_error("Import not allowed- operation in progress");
  } else {
    UncRespCode db_ret = UNC_RC_SUCCESS;
    OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteNb, db_ret);
    if (db_ret != UNC_RC_SUCCESS) {
      UPPL_LOG_FATAL("DB Connection failure for operation %d",
                    operation);
      return db_ret;
    }
    // create import request object to invoke processreq
    ImportRequest import_req;
    ret_code = import_req.ProcessRequest(&db_conn, operation,
                                         obj_key_ctr, session_id, config_id);
    if (ret_code == UNC_RC_SUCCESS) {
      if (operation == UNC_OP_IMPORT_CONTROLLER_CONFIG) {
        controller_in_import_ = (const char*) obj_key_ctr.controller_name;
      } else if (operation == UNC_OP_CLEAR_IMPORT_CONFIG) {
        controller_in_import_.clear();
      }
    }
  }
  session.addOutput(operation);
  session.addOutput((uint32_t)ret_code);
  return UNC_RC_SUCCESS;
}

/**ProcessIsCandidateDirty
 * @Description : This function process the IsCandidateDirty
 *                request from north bound
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present
 *                operation - UNC_OP_* specifies the operation
 * @return      : UNC_RC_SUCCESS is returned when the response is added
 *                to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not
 *                be added to sess.
 **/
UncRespCode InternalTransactionCoordinator::ProcessIsCandidateDirty(
    ServerSession &session,
    uint32_t operation) {
  UncRespCode db_ret = UNC_RC_SUCCESS, resp_code = UNC_RC_SUCCESS;
  // Checking the mode if it is global
  uint32_t session_id, config_id;
  int err = 0;
  err |= session.getArgument(1, session_id);
  err |= session.getArgument(2, config_id);
  if (err != 0) {
    pfc_log_info(
          "Attributes are missing in ISCandidateDirtry");
      return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  if (session_id != USESS_ID_TC) {
    TcConfigMode config_mode =  TC_CONFIG_REAL;
    string vtn_name= "";
    resp_code = physical_core->GetConfigMode(session_id,
                      config_id, config_mode, vtn_name);
    if (resp_code == UNC_RC_SUCCESS) {
      if (config_mode != TC_CONFIG_GLOBAL &&
                config_mode != TC_CONFIG_REAL) {
        resp_code = UNC_UPPL_RC_ERR_INVALID_CONFIGID;
        pfc_log_error("IsCandidateDirty failed,Mode is not Global/REAL");
      }
    }
  }
  uint8_t dirty_status  = 0;
  if (resp_code == UNC_RC_SUCCESS) {
    OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteNb, db_ret);
    if (db_ret != UNC_RC_SUCCESS) {
      UPPL_LOG_FATAL("DB Connection failure for operation %d",
                operation);
      return db_ret;
    }
    ODBCM_RC_STATUS db_status =
      PhysicalLayer::get_instance()->get_odbc_manager()->
      IsCandidateDirty(&db_conn);
    if (db_status == ODBCM_RC_CANDIDATE_DIRTY) {
      dirty_status = 1;
    } else if (db_status != ODBCM_RC_SUCCESS) {
      resp_code = UNC_UPPL_RC_ERR_DB_GET;
    }
  }
  err = session.addOutput(operation);
  err |= session.addOutput((uint32_t)resp_code);
  err |= session.addOutput(dirty_status);
  if (err != 0) {
    pfc_log_info("Error in sending IsCandidateDirty response");
    resp_code = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  return resp_code;
}
