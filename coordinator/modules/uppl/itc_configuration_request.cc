/*
 * Copyright (c) 2012-2013 NEC Corporation
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
 * @Description    : constructor function
 * @param[in]: NA
 * @return   : NA
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
 * @Description    : Destructor function
 * @param[in]:  NA
 * @return   :  NA
 * */
ConfigurationRequest::~ConfigurationRequest() {
}

/*  ProcessReq
 *  @Description: Creates the respective Kt class object
 *  to process the config operation
 *  @param[in]: ipc struct, service id, session id,
 *  configuration id, session object
 *  @return   : config operation response success/failure.
 * */
UpplReturnCode ConfigurationRequest::ProcessReq(ServerSession &session) {
  Kt_Base *KtObj = NULL;
  physical_request_header obj_req_hdr;
  // populate header from ipc message
  if (0 != PhyUtil::sessGetReqHeader(session, obj_req_hdr)) {
    pfc_log_error("Unable to parse ipc structure. BAD_REQUEST error");
    return UPPL_RC_ERR_BAD_REQUEST;
  }

  UpplReturnCode return_code = UPPL_RC_SUCCESS, resp_code = UPPL_RC_SUCCESS;

  void* key_struct = NULL;
  void* val_struct = NULL;

  physical_response_header rsh;
  PhyUtil::getRespHeaderFromReqHeader(obj_req_hdr, rsh);

  resp_code = ValidateReq(session, obj_req_hdr,
                          key_struct, val_struct, KtObj);
  if (resp_code != UPPL_RC_SUCCESS) {
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
      return_code = UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      return_code = UPPL_RC_SUCCESS;
    }
    return return_code;
  }
  switch (obj_req_hdr.operation) {
    case UNC_OP_CREATE:
    {
      resp_code = KtObj->Create(obj_req_hdr.client_sess_id,
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
      resp_code = KtObj->Update(obj_req_hdr.client_sess_id,
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
      resp_code = KtObj->Delete(obj_req_hdr.client_sess_id,
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
  if (resp_code != UPPL_RC_SUCCESS) {
    // validation failed call add out put
    // return the actual response
    pfc_log_error("Config validation failed");
    rsh.result_code = resp_code;
    if (PhyUtil::sessOutRespHeader(session, rsh) == 0) {
      resp_code = UPPL_RC_SUCCESS;
    } else {
      resp_code = UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  if (KtObj != NULL) {
    delete KtObj;
    KtObj = NULL;
  }
  if (resp_code == UPPL_RC_ERR_IPC_WRITE_ERROR) {
    // It's a common error code
    return_code = UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  pfc_log_info("Returning %d from config request handler", return_code);
  return return_code;
}


/*  ValidateReq
 *  @Description    : validates the request received from north bound
 *  @param[in]: session, request header, key struct, value struct and
 *  Base class object
 *  @return   : config operation response success/failure.
 * */
UpplReturnCode ConfigurationRequest::ValidateReq(
    ServerSession &session,
    physical_request_header obj_req_hdr,
    void* &key_struct, void* &val_struct,
    Kt_Base* &KtObj) {
  UpplReturnCode return_code = UPPL_RC_SUCCESS, resp_code = UPPL_RC_SUCCESS;
  physical_response_header rsh;
  PhyUtil::getRespHeaderFromReqHeader(obj_req_hdr, rsh);
  uint32_t key_type = obj_req_hdr.key_type;
  // create the respective object to invoke appropriate Kt class
  switch (key_type) {
    case UNC_KT_ROOT:
    {
      KtObj = new Kt_Root();
      if (KtObj == NULL) {
        pfc_log_error("Resource allocation error");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      // The root key in request is not considered
      key_struct = static_cast<void*> (&key_root_obj);
      val_struct = NULL;
      break;
    }
    case UNC_KT_CONTROLLER:
    {
      resp_code = ValidateController(session,
                                     obj_req_hdr.data_type,
                                     obj_req_hdr.operation,
                                     key_struct,
                                     val_struct);
      if (resp_code != UPPL_RC_SUCCESS) {
        return resp_code;
      }
      KtObj = new Kt_Controller();
      if (KtObj == NULL) {
        pfc_log_error("Resource allocation error");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      break;
    }
    case UNC_KT_CTR_DOMAIN:
    {
      resp_code = ValidateDomain(session,
                                 obj_req_hdr.data_type,
                                 obj_req_hdr.operation,
                                 key_struct,
                                 val_struct);
      if (resp_code != UPPL_RC_SUCCESS) {
        return resp_code;
      }
      KtObj = new Kt_Ctr_Domain();
      if (KtObj == NULL) {
        pfc_log_error("Resource allocation error");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      break;
    }
    case UNC_KT_BOUNDARY:
    {
      resp_code = ValidateBoundary(session,
                                   obj_req_hdr.data_type,
                                   obj_req_hdr.operation,
                                   key_struct,
                                   val_struct);
      if (resp_code != UPPL_RC_SUCCESS) {
        return resp_code;
      }
      KtObj = new Kt_Boundary();
      if (KtObj == NULL) {
        pfc_log_error("Resource allocation error");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      break;
    }
    case UNC_KT_LOGICAL_PORT:
    case UNC_KT_PORT:
    case UNC_KT_LOGICAL_MEMBER_PORT:
    case UNC_KT_SWITCH:
    case UNC_KT_LINK:
    {
      pfc_log_error("Operation not allowed");
      return UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    }
    default:
    {
      pfc_log_error("Key type not supported");
      return UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED;
    }
  }

  switch (obj_req_hdr.operation) {
    case UNC_OP_CREATE:
    case UNC_OP_UPDATE:
    case UNC_OP_DELETE:
    {
      // form validate request for CREATE operation
      resp_code = KtObj->ValidateRequest(key_struct,
                                         val_struct,
                                         obj_req_hdr.operation,
                                         obj_req_hdr.data_type,
                                         key_type);
      break;
    }
    default:
      resp_code = UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
      break;
  }
  if (resp_code != UPPL_RC_SUCCESS) {
    // validation failed call add output
    // return the actual response
    pfc_log_error("Config validation failed");
    return resp_code;
  }
  pfc_log_debug("Returning %d from config validate request handler",
                return_code);
  return return_code;
}

UpplReturnCode ConfigurationRequest::ValidateController(ServerSession &session,
                                                        uint32_t data_type,
                                                        uint32_t operation,
                                                        void* &key_struct,
                                                        void* &val_struct) {
  UpplReturnCode resp_code = UPPL_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  if (data_type != UNC_DT_CANDIDATE) {
    pfc_log_info("Operation not allowed in given data type %d",
                 data_type);
    resp_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
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
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  if (operation != UNC_OP_CREATE) {
    pfc_log_debug("Validation not required for other than create");
    return UPPL_RC_SUCCESS;
  }
  if (val_ctr_obj.type != UNC_CT_PFC) {
    pfc_log_debug(
        "Capability validation not done for non-PFC controller");
    return UPPL_RC_SUCCESS;
  }
  // controller capability check
  string version = reinterpret_cast<const char*> (val_ctr_obj.version);

  resp_code = physical_layer->get_physical_core()->
      ValidateKeyTypeInCtrlrCap(version,
                                (uint32_t)UNC_KT_CONTROLLER);
  if (resp_code != UPPL_RC_SUCCESS) {
    pfc_log_error("Key type validation failed in capability check");
    return resp_code;
  }
  // validate value capability
  Kt_Controller KtObj;
  resp_code = KtObj.ValidateCtrlrValueCapability(version,
                                                 (uint32_t)UNC_KT_CONTROLLER);
  if (resp_code != UPPL_RC_SUCCESS) {
    pfc_log_error("Attribute validation failure");
    return resp_code;
  }
  // validate scalability after basic validation
  // Since it requires db call
  resp_code = KtObj.ValidateCtrlrScalability(
      version,
      (uint32_t)UNC_KT_CONTROLLER,
      data_type);
  if (resp_code != UPPL_RC_SUCCESS) {
    pfc_log_error("scalability range exceeded");
    return resp_code;
  }
  return UPPL_RC_SUCCESS;
}

UpplReturnCode ConfigurationRequest::ValidateDomain(ServerSession &session,
                                                    uint32_t data_type,
                                                    uint32_t operation,
                                                    void* &key_struct,
                                                    void* &val_struct) {
  UpplReturnCode resp_code = UPPL_RC_SUCCESS;
  if (data_type != UNC_DT_CANDIDATE) {
    pfc_log_info("Operation not allowed in given data type %d",
                 data_type);
    resp_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
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
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  return UPPL_RC_SUCCESS;
}

UpplReturnCode ConfigurationRequest::ValidateBoundary(ServerSession &session,
                                                      uint32_t data_type,
                                                      uint32_t operation,
                                                      void* &key_struct,
                                                      void* &val_struct) {
  UpplReturnCode resp_code = UPPL_RC_SUCCESS;
  if (data_type != UNC_DT_CANDIDATE) {
    pfc_log_info("Operation not allowed in given data type %d",
                 data_type);
    resp_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return resp_code;
  }
  // populate key_boundary_obj structure
  int err = session.getArgument(8, key_boundary_obj);
  if (err != 0) {
    pfc_log_error("Not able to read key_boundary_obj, err is %d", err);
    return UPPL_RC_ERR_BAD_REQUEST;
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
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  return UPPL_RC_SUCCESS;
}
