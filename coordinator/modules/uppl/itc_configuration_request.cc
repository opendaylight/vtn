/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    ITC Configuration Req
 * @file     itc_configuration_request.cc
 *
 */

#include "itc_kt_base.hh"
#include "itc_kt_root.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_boundary.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_link.hh"
#include "itc_kt_port.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_logical_member_port.hh"
#include "itc_configuration_request.hh"
#include "ipct_util.hh"

using unc::uppl::PhysicalLayer;

/* ConfigurationRequest
 * @Description : This function initializes the member variables
 *                and allocates the memory for the key instances of
 *                kt_root,kt_controller,kt_domain,kt_boundary 
 * @param[in]   : None
 * @return      : None
 * */

ConfigurationRequest::ConfigurationRequest() {
  memset(&key_root_obj, 0, sizeof(key_root_t));
  memset(&key_ctr_obj, 0, sizeof(key_ctr_t));
  memset(&val_ctr_obj, 0, sizeof(val_ctr_t));
  memset(&key_domain_obj, 0, sizeof(key_ctr_domain_t));
  memset(&val_domain_obj, 0, sizeof(val_ctr_domain_t));
  memset(&key_boundary_obj, 0, sizeof(key_boundary_t));
  memset(&val_boundary_obj, 0, sizeof(val_boundary_t));
}

/* ~ConfigurationRequest
 * @Description : This function releases the memory allocated to
 *                pointer member data
 * @param[in]   : None
 * @param[out]  : None
 * @return      : None
 * */
ConfigurationRequest::~ConfigurationRequest() {
}

/* ProcessReq
 * @Description : This function creates the respective Kt class object to
 *                process the configuration request received from north bound
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present
 * @return      : UNC_RC_SUCCESS is returned when the response is added to
 *                ipc session successfully otherwise Common error code is
 *                returned when ipc response could not be added to session.
 * */
UncRespCode ConfigurationRequest::ProcessReq(
    ServerSession &session,
    physical_request_header &obj_req_hdr) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  PhysicalCore* physical_core = physical_layer->get_physical_core();
  if (physical_core->system_transit_state_ == true) {
    pfc_log_error("UNC is in state transit mode ");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }

  Kt_Base *KtObj = NULL;
  UncRespCode db_ret = UNC_RC_SUCCESS;
  OPEN_DB_CONNECTION(unc::uppl::kOdbcmConnReadWriteNb, db_ret);
  if (db_ret != UNC_RC_SUCCESS) {
    UPPL_LOG_FATAL("DB Connection failure for operation %d",
                  obj_req_hdr.operation);
    return db_ret;
  }
  UncRespCode return_code = UNC_RC_SUCCESS, resp_code = UNC_RC_SUCCESS;

  void* key_struct = NULL;
  void* val_struct = NULL;

  physical_response_header rsh;
  PhyUtil::getRespHeaderFromReqHeader(obj_req_hdr, rsh);

  resp_code = ValidateReq(&db_conn, session, obj_req_hdr,
                          key_struct, val_struct, KtObj);
  if (resp_code != UNC_RC_SUCCESS) {
    // validation failed call add output
    // return the actual response
    pfc_log_error("Config validation failed %d", resp_code);
    rsh.result_code = resp_code;
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (KtObj != NULL) {
      err |= KtObj->AddKeyStructuretoSession(obj_req_hdr.key_type,
                                             &session, key_struct);
      delete KtObj;
      KtObj = NULL;
    }
    if (err != 0) {
      return_code = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      return_code = UNC_RC_SUCCESS;
    }
    return return_code;
  }
  switch (obj_req_hdr.operation) {
    case UNC_OP_CREATE:
    {
      resp_code = KtObj->Create(&db_conn, obj_req_hdr.client_sess_id,
                                obj_req_hdr.config_id,
                                key_struct,
                                val_struct,
                                obj_req_hdr.data_type,
                                session);
      break;
    }
    case UNC_OP_UPDATE:
    {
      // Invoke Update operation for respective KT class
      resp_code = KtObj->Update(&db_conn, obj_req_hdr.client_sess_id,
                                obj_req_hdr.config_id,
                                key_struct,
                                val_struct,
                                obj_req_hdr.data_type,
                                session);
      break;
    }
    case UNC_OP_DELETE:
    {
      // Invoke Delete operation for respective KT class
      resp_code = KtObj->Delete(&db_conn, obj_req_hdr.client_sess_id,
                                obj_req_hdr.config_id,
                                key_struct,
                                obj_req_hdr.data_type,
                                session);
      break;
    }
    default:
      // Already handled
      break;
  }
  if (resp_code != UNC_RC_SUCCESS) {
    // validation failed call add out put
    // return the actual response
    pfc_log_error("Config validation failed");
    rsh.result_code = resp_code;
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    err |= KtObj->AddKeyStructuretoSession(obj_req_hdr.key_type, &session,
                                           key_struct);
    if (err == 0) {
      resp_code = UNC_RC_SUCCESS;
    } else {
      resp_code = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  if (KtObj != NULL) {
    delete KtObj;
    KtObj = NULL;
  }
  if (resp_code == UNC_UPPL_RC_ERR_IPC_WRITE_ERROR) {
    // It's a common error code
    return_code = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  pfc_log_debug("Returning %d from config request handler", return_code);
  return return_code;
}

/* ValidateReq
 * @Description : This function validates the request received from north bound
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present
 *                key struct - the key instance for appropriate key types
 *                value struct - the value struct for the appropriate key types
 *                obj_req_hdr - object of physical request header 
 *                KtObj - Object of the base class to invoke appropriate
 *                kt class
 * @return      : UNC_RC_SUCCESS if validation is successful
 *                or UNC_UPPL_RC_ERR_* if validation is failed
 * */
UncRespCode ConfigurationRequest::ValidateReq(
    OdbcmConnectionHandler *db_conn,
    ServerSession &session,
    physical_request_header obj_req_hdr,
    void* &key_struct, void* &val_struct,
    Kt_Base* &KtObj) {
  UncRespCode return_code = UNC_RC_SUCCESS, resp_code = UNC_RC_SUCCESS;
  physical_response_header rsh;
  PhyUtil::getRespHeaderFromReqHeader(obj_req_hdr, rsh);
  uint32_t key_type = obj_req_hdr.key_type;
  //  other than kt_controller keytype create is not allowed from NB
  // if unc is running in coexists mode
  UncMode unc_mode = PhysicalLayer::get_instance()->\
                        get_physical_core()->getunc_mode();
  if (unc_mode == UNC_COEXISTS_MODE && key_type != UNC_KT_CONTROLLER) {
    pfc_log_error("unc coexists mode will support only kt_controller keytype");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  // create the respective object to invoke appropriate Kt class
  switch (key_type) {
    case UNC_KT_ROOT:
    {
      KtObj = new Kt_Root();
      if (KtObj == NULL) {
        pfc_log_error("Resource allocation error");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      // The root key in request is not considered
      key_struct = static_cast<void*> (&key_root_obj);
      val_struct = NULL;
      break;
    }

    case UNC_KT_CONTROLLER:
    {
      resp_code = ValidateController(db_conn, session,
                                     obj_req_hdr.data_type,
                                     obj_req_hdr.operation,
                                     key_struct,
                                     val_struct);
      if (resp_code != UNC_RC_SUCCESS) {
        return resp_code;
      }
      KtObj = new Kt_Controller();
      if (KtObj == NULL) {
        pfc_log_error("Resource allocation error");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      //  check the uncmode, ctrtype and return error
      //  if ctrtype is non-pfc controller
      if (unc_mode == UNC_COEXISTS_MODE) {
        if (obj_req_hdr.operation == UNC_OP_CREATE &&
            val_ctr_obj.type != UNC_CT_PFC) {
          pfc_log_error("non-pfc type controller create is not supported in"
            "unc coexists mode");
          return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
        }
        if (obj_req_hdr.operation == UNC_OP_DELETE) {
          //  retrieve the controller type from db
          unc_keytype_ctrtype_t type = UNC_CT_UNKNOWN;
          key_ctr_t *obj_key_ctr= reinterpret_cast<key_ctr_t*>(key_struct);
          UncRespCode ctr_type_status =
            PhyUtil::get_controller_type(db_conn,
                   reinterpret_cast<const char*>(obj_key_ctr->controller_name),
                   type, (unc_keytype_datatype_t) obj_req_hdr.data_type);
          if (ctr_type_status !=  UNC_RC_SUCCESS) {
            pfc_log_error(
            "Operation %d is not allowed as controller instance %s not exists",
            obj_req_hdr.operation, obj_key_ctr->controller_name);
            return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
          }
          pfc_log_debug("Controller type: %d", type);
          if (type != UNC_CT_PFC) {
            pfc_log_error("non-pfc type controller delete is not supported in"
            "unc coexists mode");
            return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
          }
        }
        if (obj_req_hdr.operation == UNC_OP_UPDATE) {
          pfc_log_error("kt_controller update is not supported in"
            "unc coexists mode");
          return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
        }
        if (obj_req_hdr.operation == UNC_OP_CREATE) {
          UncRespCode ctr_count_status =
                   KtObj->ValidateControllerCount(
                        db_conn, key_struct, val_struct, UNC_DT_CANDIDATE);
          if (ctr_count_status != UNC_RC_SUCCESS) {
            pfc_log_error("kt_controller count is exceeds (>1) in"
            "unc coexists mode");
            return ctr_count_status;
          }
        }
      }
      break;
    }
    case UNC_KT_CTR_DOMAIN:
    {
      resp_code = ValidateDomain(db_conn, session,
                                 obj_req_hdr.data_type,
                                 obj_req_hdr.operation,
                                 key_struct,
                                 val_struct);
      if (resp_code != UNC_RC_SUCCESS) {
        return resp_code;
      }
      KtObj = new Kt_Ctr_Domain();
      if (KtObj == NULL) {
        pfc_log_error("Resource allocation error");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      break;
    }
    case UNC_KT_BOUNDARY:
    {
      resp_code = ValidateBoundary(db_conn, session,
                                   obj_req_hdr.data_type,
                                   obj_req_hdr.operation,
                                   key_struct,
                                   val_struct);
      if (resp_code != UNC_RC_SUCCESS) {
        return resp_code;
      }
      KtObj = new Kt_Boundary();
      if (KtObj == NULL) {
        pfc_log_error("Resource allocation error");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      break;
    }
    case UNC_KT_LOGICAL_PORT:
    case UNC_KT_PORT:
    case UNC_KT_LOGICAL_MEMBER_PORT:
    case UNC_KT_SWITCH:
    case UNC_KT_LINK:
    case UNC_KT_CTR_DATAFLOW:
    case UNC_KT_DATAFLOW:
    case UNC_KT_DATAFLOW_V2:
    {
      pfc_log_error("Operation not allowed for %d", key_type);
      return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    }
    default:
    {
      pfc_log_error("Key type not supported");
      return UNC_UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED;
    }
  }

  switch (obj_req_hdr.operation) {
    case UNC_OP_CREATE:
    case UNC_OP_UPDATE:
    case UNC_OP_DELETE:
    {
      // form validate request for CREATE operation
      resp_code = KtObj->ValidateRequest(db_conn, key_struct,
                                         val_struct,
                                         obj_req_hdr.operation,
                                         obj_req_hdr.data_type,
                                         key_type);
      break;
    }
    default:
      resp_code = UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
      break;
  }
  if (resp_code != UNC_RC_SUCCESS) {
    // validation failed call add output
    // return the actual response
    pfc_log_error("Config validation failed");
    return resp_code;
  }
  pfc_log_debug("Returning %d from config validate request handler",
                return_code);
  return return_code;
}

/* ValidateController
 * @Description : This function validates the value struct and the scalability
 *                and also checks the capability for UNC_KT_CONTROLLER
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present
 *                data_type - The data_type UNC_DT_CANDIDATE is only allowed
 *                operation - contains UNC_OP_CREATE or UNC_OP_UPDATE
 *                key struct - specifies key instance of KT_Controller
 *                value struct - specifies value structure of KT_CONTROLLER
 * @return      : UNC_RC_SUCCESS if scalability number is within range
 *                or UNC_UPPL_RC_ERR_* if not
 * * */
UncRespCode ConfigurationRequest::ValidateController(
    OdbcmConnectionHandler *db_conn,
    ServerSession &session,
    uint32_t data_type,
    uint32_t operation,
    void* &key_struct,
    void* &val_struct) {
  UncRespCode resp_code = UNC_RC_SUCCESS;
  if (data_type != UNC_DT_CANDIDATE) {
    pfc_log_info("Operation not allowed in given data type %d",
                 data_type);
    resp_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return resp_code;
  }
  // populate key_ctr structure
  int err = session.getArgument(8, key_ctr_obj);
  pfc_log_debug("%s", IpctUtil::get_string(key_ctr_obj).c_str());
  key_struct = static_cast<void*> (&key_ctr_obj);
  if (err == 0 &&
      (operation == UNC_OP_CREATE ||
          operation == UNC_OP_UPDATE)) {
    // populate val_ctr structure
    err |= session.getArgument(9, val_ctr_obj);
    val_struct = static_cast<void*> (&val_ctr_obj);
    pfc_log_debug("%s", IpctUtil::get_string(val_ctr_obj).c_str());
  }
  if (err != 0) {
    pfc_log_error("Not able to read val_ctr_obj, err is %d", err);
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  if (operation != UNC_OP_CREATE) {
    pfc_log_debug("Validation not required for other than create");
    return UNC_RC_SUCCESS;
  }
  return UNC_RC_SUCCESS;
}

/* ValidateDomain
 * @Description : This function validates the value struct of the
 *                UNC_KT_DOMAIN
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present
 *                data_type - The data_type UNC_DT_CANDIDATE is only allowed
 *                operation - contains UNC_OP_CREATE or UNC_OP_UPDATE
 *                key struct - specifies key instance of KT_Domain
 *                value struct - specifies value structure of KT_Domain
 * @return      : UNC_RC_SUCCESS if the validation is success
 *                or UNC_UPPL_RC_ERR_* if validation is failed
 * */
UncRespCode ConfigurationRequest::ValidateDomain(
    OdbcmConnectionHandler *db_conn,
    ServerSession &session,
    uint32_t data_type,
    uint32_t operation,
    void* &key_struct,
    void* &val_struct) {
  UncRespCode resp_code = UNC_RC_SUCCESS;
  if (data_type != UNC_DT_CANDIDATE) {
    pfc_log_info("Operation not allowed in given data type %d",
                 data_type);
    resp_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return resp_code;
  }
  // populate key_domain_obj structure
  int err = session.getArgument(8, key_domain_obj);
  key_struct = static_cast<void*> (&key_domain_obj);
  pfc_log_debug("%s", IpctUtil::get_string(key_domain_obj).c_str());
  if (err == 0 &&
      (operation == UNC_OP_CREATE ||
          operation == UNC_OP_UPDATE)) {
    // populate val_domain_obj structure
    err |= session.getArgument(9, val_domain_obj);
    val_struct = static_cast<void*> (&val_domain_obj);
    pfc_log_debug("%s", IpctUtil::get_string(val_domain_obj).c_str());
  }
  if (err != 0) {
    pfc_log_error("Not able to read val_domain_obj, err is %d", err);
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  return UNC_RC_SUCCESS;
}

/* ValidateBoundary
 * @Description : This function validates the value struct of the
 *                UNC_KT_BOUNDARY
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present
 *                data_type - The data_type UNC_DT_CANDIDATE is only allowed
 *                operation - contains UNC_OP_CREATE or UNC_OP_UPDATE
 *                key struct - specifies key instance of KT_Boundary
 *                value struct - specifies value structure of KT_Boundary
 * @return      : UNC_RC_SUCCESS if the validation is success
 *                or UNC_UPPL_RC_ERR_* if validation is failed
 * */
UncRespCode ConfigurationRequest::ValidateBoundary(
    OdbcmConnectionHandler *db_conn,
    ServerSession &session,
    uint32_t data_type,
    uint32_t operation,
    void* &key_struct,
    void* &val_struct) {
  UncRespCode resp_code = UNC_RC_SUCCESS;
  if (data_type != UNC_DT_CANDIDATE) {
    pfc_log_info("Operation not allowed in given data type %d",
                 data_type);
    resp_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return resp_code;
  }
  // populate key_boundary_obj structure
  int err = session.getArgument(8, key_boundary_obj);
  if (err != 0) {
    pfc_log_error("Not able to read key_boundary_obj, err is %d", err);
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  key_struct = static_cast<void*> (&key_boundary_obj);
  pfc_log_debug("%s", IpctUtil::get_string(key_boundary_obj).c_str());
  if (err == 0 &&
      (operation == UNC_OP_CREATE ||
          operation == UNC_OP_UPDATE)) {
    // populate val_boundary_obj structure
    err |= session.getArgument(9, val_boundary_obj);
    val_struct = static_cast<void*> (&val_boundary_obj);
    pfc_log_debug("%s", IpctUtil::get_string(val_boundary_obj).c_str());
  }
  if (err != 0) {
    pfc_log_error("Not able to read val_boundary_obj, err is %d", err);
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  return UNC_RC_SUCCESS;
}
