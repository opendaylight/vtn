/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    ITC Read Req
 * @file     itc_read_request.cc
 *
 */

#include "itc_kt_base.hh"
#include "itc_kt_root.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_dataflow.hh"
#include "itc_kt_dataflow_v2.hh"
#include "itc_kt_ctr_dataflow.hh"
#include "itc_kt_switch.hh"
#include "itc_read_request.hh"
#include "itc_kt_boundary.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_link.hh"
#include "itc_kt_port.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_logical_member_port.hh"
#include "ipct_util.hh"
using unc::uppl::PhysicalLayer;

/**ReadRequest
 * @Description : This function initializes the member data
 * @param[in]   : None
 * @return      : None
 * */
ReadRequest::ReadRequest() : key_root_obj(), key_ctr_obj(), val_ctr_obj(),
  key_dataflow_obj(), key_dataflow_v2_obj(), val_dataflow_v2_obj(),
  key_ctr_dataflow_obj(), key_domain_obj(),
  val_domain_obj(), key_logical_port_obj(), val_logical_port_obj(),
  key_logical_member_port_obj(), key_switch_obj(), val_switch_obj(),
  key_port_obj(), val_port_obj(), key_link_obj(), val_link_obj(),
  key_boundary_obj(), val_boundary_obj() {
}

/**~ReadRequest
 * @Description : This function releases the memory allocated to
 *                pointer member data
 * @param[in]   : None
 * @return      : None
 * */
ReadRequest::~ReadRequest() {
  FlushBulkReadBuffer();
}

/**ProcessReq
 * @Description : This function receives the message from internal transaction
 *                coordinator and frames the database request for Read
 *                operation.Creates the respective Kt class object to process
 *                the read operation
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present
 * @return      : UNC_RC_SUCCESS if processing the Read request is successful
 *                or UNC_UPPL_RC_ERR_* in case of failure
 * */
UncRespCode ReadRequest::ProcessReq(ServerSession &session,
                                       physical_request_header &obj_req_hdr) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  PhysicalCore* physical_core = physical_layer->get_physical_core();
  if (physical_core->system_transit_state_ == true) {
    pfc_log_error("UNC is in state transit mode ");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }

  Kt_Base *KtObj = NULL;
  UncRespCode resp_code = UNC_RC_SUCCESS, return_code = UNC_RC_SUCCESS;
  physical_response_header rsh;
  PhyUtil::getRespHeaderFromReqHeader(obj_req_hdr, rsh);
  OdbcmConnectionHandler *db_conn = NULL;
  OPEN_DB_CONNECTION_WITH_POOL(unc::uppl::kOdbcmConnReadOnly,
                               resp_code, db_conn,
                               obj_req_hdr.client_sess_id,
                               obj_req_hdr.config_id);
  if (resp_code != UNC_RC_SUCCESS) {
    pfc_log_error("DB Connection failure for operation %d",
                  obj_req_hdr.operation);
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (err != 0) {
      pfc_log_error("Error writing response to session");
    }
    return resp_code;
  }
  void* key_struct = NULL;
  void* val_struct = NULL;
  // get KeyType and create the respective object to invoke appropriate Kt class
  switch (obj_req_hdr.key_type) {
    case UNC_KT_ROOT:
      if (obj_req_hdr.operation == UNC_OP_READ ||
          obj_req_hdr.operation == UNC_OP_READ_SIBLING ||
          obj_req_hdr.operation == UNC_OP_READ_SIBLING_BEGIN ||
          obj_req_hdr.operation == UNC_OP_READ_SIBLING_COUNT) {
        rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      }
      KtObj = new Kt_Root();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Root");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      memset(&key_root_obj, 0, sizeof(key_root_t));
      // The root key in request is not considered
      key_struct = static_cast<void*> (&key_root_obj);
      break;

    case UNC_KT_DATAFLOW:
      if ((obj_req_hdr.operation != UNC_OP_READ) ||
          (obj_req_hdr.data_type != UNC_DT_STATE)) {
        pfc_log_error("KtDataflow supports only Read Operation");
        rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      }
      GetDataflowStructure(session, key_struct, rsh);
      KtObj = new Kt_Dataflow();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Dataflow");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      break;
    case UNC_KT_DATAFLOW_V2:
      if ((obj_req_hdr.operation != UNC_OP_READ) ||
          (obj_req_hdr.data_type != UNC_DT_STATE)) {
        pfc_log_error("KtDataflow supports only Read Operation");
        rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      }
      GetDataflowV2Structure(session, key_struct, val_struct, rsh);
      KtObj = new Kt_DataflowV2();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_DataflowV2");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      break;

    case  UNC_KT_CONTROLLER:
      KtObj = new Kt_Controller();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Controller");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetControllerStructure(session, key_struct, val_struct, rsh);
      break;
    case UNC_KT_CTR_DATAFLOW:
      if ((obj_req_hdr.operation != UNC_OP_READ) ||
          (obj_req_hdr.data_type != UNC_DT_STATE)) {
        pfc_log_error("Kt_Ctr_Dataflow supports only Read Operation");
        rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      }
      GetCtrDataflowStructure(session, key_struct, rsh);
      KtObj = new Kt_Ctr_Dataflow();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Ctr_Dataflow");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      break;
    case UNC_KT_CTR_DOMAIN:
      KtObj = new Kt_Ctr_Domain();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Ctr_Domain");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetDomainStructure(session, key_struct, val_struct, rsh);
      break;
    case UNC_KT_LOGICAL_PORT:
      KtObj = new Kt_LogicalPort();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_LogicalPort");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetLogicalPortStructure(session, key_struct, val_struct, rsh);
      break;
    case UNC_KT_LOGICAL_MEMBER_PORT:
      KtObj = new Kt_LogicalMemberPort();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_LogicalMemberPort");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetLogicalMemberPortStructure(session, key_struct, val_struct, rsh);
      break;
    case UNC_KT_SWITCH:
      KtObj = new Kt_Switch();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Switch");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetSwitchStructure(session, key_struct, val_struct, rsh);
      break;
    case UNC_KT_PORT:
      KtObj = new Kt_Port();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Port");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetPortStructure(session, key_struct, val_struct, rsh);
      break;
    case UNC_KT_LINK:
      KtObj = new Kt_Link();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Link");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetLinkStructure(session, key_struct, val_struct, rsh);
      break;
    case UNC_KT_BOUNDARY:
      KtObj = new Kt_Boundary();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Boundary");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetBoundaryStructure(session, key_struct, val_struct, rsh);
      break;
    default:
      rsh.result_code = UNC_UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED;
      break;
  }
  if (rsh.result_code != UNC_RC_SUCCESS) {
    if (KtObj != NULL) {
      delete KtObj;
      KtObj = NULL;
    }
    pfc_log_info("Read service failed with error %d", rsh.result_code);
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (err != 0) {
      return_code = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      return_code = UNC_RC_SUCCESS;
    }
    return return_code;
  }
  switch (obj_req_hdr.operation) {
    case UNC_OP_READ:
    case UNC_OP_READ_NEXT:
    case UNC_OP_READ_BULK:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_COUNT:
      // form validate request for READ operation
      if (obj_req_hdr.key_type != UNC_KT_DATAFLOW_V2)
        resp_code = KtObj->ValidateRequest(db_conn,
                                         key_struct,
                                         NULL,
                                         obj_req_hdr.operation,
                                         obj_req_hdr.data_type,
                                         obj_req_hdr.key_type);
      else
        resp_code = KtObj->ValidateRequest(db_conn,
                                         key_struct,
                                         val_struct,
                                         obj_req_hdr.operation,
                                         obj_req_hdr.data_type,
                                         obj_req_hdr.key_type);

      break;
    default:
      resp_code = UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
      break;
  }
  if (resp_code != UNC_RC_SUCCESS) {
    // validation failed call add out put
    rsh.result_code = resp_code;
    pfc_log_error("read validation failed");
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (KtObj != NULL) {
      err |= KtObj->AddKeyStructuretoSession(obj_req_hdr.key_type,
                                             &session, key_struct);
    }
    if (err != 0) {
      return_code = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      return_code = UNC_RC_SUCCESS;
    }
  } else {
    return_code = ProcessReadOperation(db_conn, session, KtObj, obj_req_hdr,
                                       key_struct, val_struct,
                                       obj_req_hdr.operation);
  }
  if (KtObj != NULL) {
    delete KtObj;
    KtObj = NULL;
  }
  return return_code;
}

/**ProcessReadOperation
 * @Description : Function processes various Read types of operation by
 *                Creating the respective Kt class object and returns
 *                the processing result.
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present
 *                key_struct - the key instance for appropriate key types
 *                val_struct - the value struct for the appropriate key types
 *                operation_type - UNC_OP_* specifies the operation type
 *                KtObj - Object of the base class to invoke appropriate
 *                Kt class
 *                obj_req_hdr - object of physical request header
 * @return      : UNC_RC_SUCCESS if processing the Read request is successful
 *                or UNC_UPPL_RC_ERR_* in case of failure
 * */
UncRespCode ReadRequest::ProcessReadOperation(
    OdbcmConnectionHandler *db_conn,
    ServerSession &session,
    Kt_Base *KtObj,
    physical_request_header &obj_req_hdr,
    void* key_struct,
    void* val_struct,
    uint32_t operation_type) {
  UncRespCode resp_code = UNC_RC_SUCCESS, return_code = UNC_RC_SUCCESS;
  pfc_bool_t response_sent = PFC_FALSE;
  uint32_t max_rep_ct = obj_req_hdr.max_rep_count;
  switch (operation_type) {
    case UNC_OP_READ:
      // Invoke Read operation for respective KT class
      resp_code = KtObj->Read(db_conn,
                              obj_req_hdr.client_sess_id,
                              obj_req_hdr.config_id,
                              key_struct,
                              val_struct,
                              obj_req_hdr.data_type,
                              session,
                              obj_req_hdr.option1,
                              obj_req_hdr.option2);
      response_sent = PFC_TRUE;
      break;
    case UNC_OP_READ_NEXT:
      // Invoke Read Next operation for respective KT class
      resp_code = KtObj->ReadNext(db_conn,
                                  obj_req_hdr.client_sess_id,
                                  key_struct,
                                  obj_req_hdr.data_type,
                                  this);
      if (resp_code == UNC_RC_SUCCESS) {
        response_sent = PFC_TRUE;
        return_code = FrameReadBulkResponse(session,
                                            obj_req_hdr.client_sess_id,
                                            obj_req_hdr.config_id,
                                            operation_type,
                                            obj_req_hdr.data_type,
                                            obj_req_hdr.option1,
                                            obj_req_hdr.option2);
        pfc_log_debug("Session framing response %d", return_code);
      }
      break;
    case UNC_OP_READ_BULK:
      if (max_rep_ct > UPPL_MAX_REP_CT) {
        pfc_log_debug("User requested more than %d records!!", UPPL_MAX_REP_CT);
        max_rep_ct = UPPL_MAX_REP_CT;
      }
      // Invoke Read Bulk operation for respective KT class
      resp_code = KtObj->ReadBulk(db_conn,
                                  key_struct,
                                  obj_req_hdr.data_type,
                                  max_rep_ct,
                                  -1,
                                  false,
                                  false,
                                  this);
      if (resp_code == UNC_RC_SUCCESS) {
        response_sent = PFC_TRUE;
        return_code = FrameReadBulkResponse(session,
                                            obj_req_hdr.client_sess_id,
                                            obj_req_hdr.config_id,
                                            operation_type,
                                            obj_req_hdr.data_type,
                                            obj_req_hdr.option1,
                                            obj_req_hdr.option2);
        pfc_log_debug("Session framing response %d", return_code);
      }
      break;
    case UNC_OP_READ_SIBLING_BEGIN:
      // Invoke Read Sibling Begin operation for respective KT class
      resp_code = KtObj->ReadSiblingBegin(db_conn,
                                          obj_req_hdr.client_sess_id,
                                          obj_req_hdr.config_id,
                                          key_struct,
                                          val_struct,
                                          obj_req_hdr.data_type,
                                          session,
                                          obj_req_hdr.option1,
                                          obj_req_hdr.option2,
                                          obj_req_hdr.max_rep_count);
      response_sent = PFC_TRUE;
      break;
    case UNC_OP_READ_SIBLING:
      // Invoke Read Sibling operation for respective KT class
      resp_code = KtObj->ReadSibling(db_conn,
                                     obj_req_hdr.client_sess_id,
                                     obj_req_hdr.config_id,
                                     key_struct,
                                     val_struct,
                                     obj_req_hdr.data_type,
                                     session,
                                     obj_req_hdr.option1,
                                     obj_req_hdr.option2,
                                     obj_req_hdr.max_rep_count);
      response_sent = PFC_TRUE;
      break;
    case UNC_OP_READ_SIBLING_COUNT:
      // Invoke Read Sibling Count operation for respective KT class
      resp_code = KtObj->ReadSiblingCount(db_conn,
                                          obj_req_hdr.client_sess_id,
                                          obj_req_hdr.config_id,
                                          key_struct,
                                          val_struct,
                                          obj_req_hdr.key_type,
                                          obj_req_hdr.data_type,
                                          session,
                                          obj_req_hdr.option1,
                                          obj_req_hdr.option2);
      response_sent = PFC_TRUE;
      break;
    default:
      resp_code = UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
      break;
  }
  if (!response_sent) {
    physical_response_header rsh;
    PhyUtil::getRespHeaderFromReqHeader(obj_req_hdr, rsh);
    rsh.result_code = resp_code;
    pfc_log_error("read operation failed: %d", resp_code);
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    err |= KtObj->AddKeyStructuretoSession(obj_req_hdr.key_type, &session,
                                           key_struct);
    if (err != 0) {
      return_code = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      return_code = UNC_RC_SUCCESS;
    }
  }
  return return_code;
}

/**FrameReadBulkResponse
 * @Description : This function is to frame the ReadBulk response for
 *                the respective key types
 * @param[in]   : &session -  Object of ServerSession where the request
 *                argument present
 *                session_id - ipc session id used for TC validation
 *                config_id - configuration id used for TC validation
 *                operation - UNC_OP_* specifies the operation
 *                data_type - UNC_DT_* specifies the database type
 *                option1,option2 - specifies any additional
 *                condition for read operation
 * @return      : UNC_RC_SUCCESS is returned when the response is added
 *                to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not
 *                be added to sess.
 * */
UncRespCode ReadRequest::FrameReadBulkResponse(ServerSession &session,
                                                  uint32_t session_id,
                                                  uint32_t config_id,
                                                  uint32_t operation,
                                                  uint32_t data_type,
                                                  uint32_t option1,
                                                  uint32_t option2) {
  uint32_t max_rep_ct = 0;
  // Fill the IPC response message in session
  physical_response_header rsh = {session_id,
      config_id,
      operation,
      max_rep_ct,
      option1,
      option2,
      data_type,
      0};
  vector<BulkReadBuffer> :: iterator iter = vect_bulk_read_buffer.begin();
  for (; iter != vect_bulk_read_buffer.end(); ++iter) {
    if ((*iter).value_type == IS_KEY) { ++max_rep_ct;}
  }
  rsh.max_rep_count = max_rep_ct;
  if (vect_bulk_read_buffer.empty()) {
    pfc_log_debug("No instances available to read");
    rsh.result_code = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  }
  int err = PhyUtil::sessOutRespHeader(session, rsh);
  iter = vect_bulk_read_buffer.begin();
  int vect_size = vect_bulk_read_buffer.size();
  for (; iter != vect_bulk_read_buffer.end(); ++iter, --vect_size) {
    BulkReadBuffer obj_buffer = (*iter);
    unc_key_type_t key_type = obj_buffer.key_type;
    ValueType value_type = obj_buffer.value_type;
    switch (key_type) {
      case UNC_KT_CONTROLLER:
        AddControllerStructure(session, obj_buffer, err);
        break;
      case UNC_KT_CTR_DOMAIN:
        AddDomainStructure(session, obj_buffer, err);
        break;
      case UNC_KT_LOGICAL_PORT:
        AddLogicalPortStructure(session, obj_buffer, err);
        break;
      case UNC_KT_LOGICAL_MEMBER_PORT:
        AddLogicalMemberPortStructure(session, obj_buffer, err);
        break;
      case UNC_KT_SWITCH:
        AddSwitchStructure(session, obj_buffer, err);
        break;
      case UNC_KT_PORT:
        AddPortStructure(session, obj_buffer, err);
        break;
      case UNC_KT_LINK:
        AddLinkStructure(session, obj_buffer, err);
        break;
      case UNC_KT_BOUNDARY:
        AddBoundaryStructure(session, obj_buffer, err);
        break;
      default:
        break;
    }
    if (vect_size != 1 && value_type == IS_SEPARATOR) {
      err |= session.addOutput();  // NULL
    }
    if (err != 0) {
      pfc_log_error("Server session addOutput failed");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  return UNC_RC_SUCCESS;
}

/**GetControllerStructure
 * @Description : This function is to get the value structure of the controller
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present 
 *                key_struct - the key for the kt_controller instance
 *                val_struct - the value structure for the kt_controller
 *                &rsh - object of the physical response header
 * @return      : void 
 * */
void ReadRequest::GetControllerStructure(ServerSession &session,
                                         void * &key_struct,
                                         void * &val_struct,
                                         physical_response_header &rsh) {
  // populate key_ctr structure
  memset(&key_ctr_obj, 0, sizeof(key_ctr_t));
  rsh.result_code = session.getArgument(8, key_ctr_obj);
  key_struct = static_cast<void*> (&key_ctr_obj);
  // populate val_ctr structure
  memset(&val_ctr_obj, 0, sizeof(val_ctr_obj));
  pfc_log_info("%s", IpctUtil::get_string(key_ctr_obj).c_str());
  int val_read = session.getArgument(9, val_ctr_obj);
  if (val_read == 0) {
    val_struct = static_cast<void*> (&val_ctr_obj);
    pfc_log_info("%s", IpctUtil::get_string(val_ctr_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

/**GetDataflowStructure
 * @Description : This function is to get the value structure of the Dataflow
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present 
 *                key_struct - the key for the kt_Dataflow instance
 *                val_struct - the value structure for the ktDataflow
 *                &rsh - object of the physical response header
 * @return      : void 
 * */
void ReadRequest::GetDataflowStructure(ServerSession &session,
                                         void * &key_struct,
                                         physical_response_header &rsh) {
  // populate key_ctr structure
  memset(&key_dataflow_obj, 0, sizeof(key_dataflow_t));
  rsh.result_code = session.getArgument(8, key_dataflow_obj);
  pfc_log_info("%s", DataflowCmn::get_string(key_dataflow_obj).c_str());
  key_struct = static_cast<void*> (&key_dataflow_obj);
  return;
}

/**GetDataflowV2Structure
 * @Description : This function is to get the value structure of the Dataflow
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present 
 *                key_struct - the key for the kt_Dataflow instance
 *                val_struct - the value structure for the ktDataflow
 *                &rsh - object of the physical response header
 * @return      : void 
 * */
void ReadRequest::GetDataflowV2Structure(ServerSession &session,
                                         void * &key_struct,
                                         void * &val_struct,
                                         physical_response_header &rsh) {
  // populate key_ctr structure
  memset(&key_dataflow_v2_obj, 0, sizeof(key_dataflow_v2_t));
  memset(&val_dataflow_v2_obj, 0, sizeof(val_dataflow_v2_t));
  rsh.result_code = UNC_RC_SUCCESS;
  int read_val = session.getArgument(8, key_dataflow_v2_obj);
  read_val |= session.getArgument(9, val_dataflow_v2_obj);
  if (read_val != 0) {
    pfc_log_info("Key or Val structure is not recieved");
    rsh.result_code = UNC_UPPL_RC_ERR_CFG_SYNTAX;
  } else {
    pfc_log_info("%s", DataflowCmn::get_string(key_dataflow_v2_obj).c_str());
    key_struct = static_cast<void*> (&key_dataflow_v2_obj);
    pfc_log_info("%s", DataflowCmn::get_string(val_dataflow_v2_obj).c_str());
    val_struct = static_cast<void*> (&val_dataflow_v2_obj);
  }
  return;
}


/**GetCtrDataflowStructure
 * @Description : This function is to get the value structure of the CtrDataflow
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present 
 *                key_struct - the key for the kt_Ctr_Dataflow instance
 *                val_struct - the value structure for the ktCtrDataflow
 *                &rsh - object of the physical response header
 * @return      : void 
 * */
void ReadRequest::GetCtrDataflowStructure(ServerSession &session,
                                         void * &key_struct,
                                         physical_response_header &rsh) {
  // populate key_ctr structure
  memset(&key_ctr_dataflow_obj, 0, sizeof(key_ctr_dataflow_t));
  rsh.result_code = session.getArgument(8, key_ctr_dataflow_obj);
  key_struct = static_cast<void*> (&key_ctr_dataflow_obj);
  pfc_log_info("%s", DataflowCmn::get_string(key_ctr_dataflow_obj).c_str());
  return;
}

/**GetDomainStructure
 * @Description : This function is to get the value structure of the domain
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present 
 *                key_struct - the key for the kt_domain instance
 *                val_struct - the value structure for the kt_domain
 *                &rsh - object of the physical response header
 * @return      : void 
 * */
void ReadRequest::GetDomainStructure(ServerSession &session,
                                     void * &key_struct,
                                     void * &val_struct,
                                     physical_response_header &rsh) {
  // populate key_domain_obj structure
  memset(&key_domain_obj, 0, sizeof(key_domain_obj));
  rsh.result_code = session.getArgument(8, key_domain_obj);
  key_struct = static_cast<void*> (&key_domain_obj);
  pfc_log_info("%s", IpctUtil::get_string(key_domain_obj).c_str());
  // populate val_domain_obj structure
  memset(&val_domain_obj, 0, sizeof(val_domain_obj));
  int val_read = session.getArgument(9, val_domain_obj);
  if (val_read == 0) {
    val_struct = static_cast<void*> (&val_domain_obj);
    pfc_log_info("%s", IpctUtil::get_string(val_domain_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

/**GetLogicalPortStructure
 * @Description : This function is to get the value structure of the
 *                logical port
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present 
 *                key_struct - the key for the kt_logicalport instance
 *                val_struct - the value structure for the kt_logicalport
 *                &rsh - object of the physical response header
 * @return      : void
 * */
void ReadRequest::GetLogicalPortStructure(ServerSession &session,
                                          void * &key_struct,
                                          void * &val_struct,
                                          physical_response_header &rsh) {
  // populate key_logical_port structure
  memset(&key_logical_port_obj, 0, sizeof(key_logical_port_t));
  session.getArgument(8, key_logical_port_obj);
  key_struct = static_cast<void*> (&key_logical_port_obj);
  pfc_log_info("%s", IpctUtil::get_string(key_logical_port_obj).c_str());
  // populate val_logical_port structure
  memset(&val_logical_port_obj, 0, sizeof(val_logical_port_obj));
  int val_read = session.getArgument(9, val_logical_port_obj);
  if (val_read == 0) {
    val_struct = static_cast<void*> (&val_logical_port_obj);
    pfc_log_info("%s", IpctUtil::get_string(val_logical_port_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

/**GetLogicalMemberPortStructure
 * @Description : This function is to get the value structure of the
 *                logical member port
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present 
 *                key_struct - the key for the kt_logical_member_port instance
 *                val_struct - the value structure for the
 *                kt_logical_member_port
 *                &rsh - object of the physical response header
 * @return      : void
 * */
void ReadRequest::GetLogicalMemberPortStructure(ServerSession &session,
                                                void * &key_struct,
                                                void * &val_struct,
                                                physical_response_header &rsh) {
  // populate key_logical_member_port structure
  memset(&key_logical_member_port_obj, 0, sizeof(key_logical_member_port_t));
  rsh.result_code = session.getArgument(8, key_logical_member_port_obj);
  key_struct = static_cast<void*> (&key_logical_member_port_obj);
  pfc_log_debug("%s",
                IpctUtil::get_string(key_logical_member_port_obj).c_str());
  return;
}

/**GetSwitchStructure
 * @Description : This function is to get the value structure of the switch
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present 
 *                key_struct - the key for the kt_switch instance
 *                val_struct - the value structure for the kt_switch
 *                &rsh - object of the physical response header
 * @return      : void
 * */
void ReadRequest::GetSwitchStructure(ServerSession &session,
                                     void * &key_struct,
                                     void * &val_struct,
                                     physical_response_header &rsh) {
  // populate key_switch_obj structure
  memset(&key_switch_obj, 0, sizeof(key_switch_t));
  rsh.result_code = session.getArgument(8, key_switch_obj);
  key_struct = static_cast<void*> (&key_switch_obj);
  pfc_log_info("%s", IpctUtil::get_string(key_switch_obj).c_str());
  // populate val_switch structure
  memset(&val_switch_obj, 0, sizeof(val_switch_obj));
  int val_read = session.getArgument(9, val_switch_obj);
  if (val_read == 0) {
    val_struct = static_cast<void*> (&val_switch_obj);
    pfc_log_info("%s", IpctUtil::get_string(val_switch_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

/**GetPortStructure
 * @Description : This function is to get the value structure of the port
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present 
 *                key_struct - the key for the kt_port instance
 *                val_struct - the value structure for the kt_port
 *                &rsh - object of the physical response header
 * @return      : void
 * */
void ReadRequest::GetPortStructure(ServerSession &session,
                                   void * &key_struct,
                                   void * &val_struct,
                                   physical_response_header &rsh) {
  // populate key_port structure
  memset(&key_port_obj, 0, sizeof(key_port_t));
  rsh.result_code = session.getArgument(8, key_port_obj);
  key_struct = static_cast<void*> (&key_port_obj);
  pfc_log_info("%s", IpctUtil::get_string(key_port_obj).c_str());
  // populate val_port structure
  memset(&val_port_obj, 0, sizeof(val_port_obj));
  int val_read = session.getArgument(9, val_port_obj);
  if (val_read == 0) {
    val_struct = static_cast<void*> (&val_port_obj);
    pfc_log_info("%s", IpctUtil::get_string(val_port_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

/**GetLinkStructure
 * @Description : This function is to get the value structure of the link
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present 
 *                key_struct - the key for the kt_link instance
 *                val_struct - the value structure for the kt_link
 *                &rsh - object of the physical response header
 * @return      : void
 * */
void ReadRequest::GetLinkStructure(ServerSession &session,
                                   void * &key_struct,
                                   void * &val_struct,
                                   physical_response_header &rsh) {
  // populate key_link_obj structure
  memset(&key_link_obj, 0, sizeof(key_link_t));
  rsh.result_code = session.getArgument(8, key_link_obj);
  key_struct = static_cast<void*> (&key_link_obj);
  pfc_log_info("%s", IpctUtil::get_string(key_link_obj).c_str());
  // populate val_link structure
  memset(&val_link_obj, 0, sizeof(val_link_obj));
  int val_read = session.getArgument(9, val_link_obj);
  if (val_read == 0) {
    val_struct = static_cast<void*> (&val_link_obj);
    pfc_log_info("%s", IpctUtil::get_string(val_link_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

/**GetBoundaryStructure
 * @Description : This function is to get the value structure of the boundary
 * @param[in]   : session - Object of ServerSession where the request
 *                argument present 
 *                key_struct - the key for the kt_boundary instance
 *                val_struct - the value structure for the kt_boundary
 *                &rsh - object of the physical response header
 * @return      : void
 * */
void ReadRequest::GetBoundaryStructure(ServerSession &session,
                                       void * &key_struct,
                                       void * &val_struct,
                                       physical_response_header &rsh) {
  // populate key_boundary_obj structure
  memset(&key_boundary_obj, 0, sizeof(key_boundary_t));
  rsh.result_code = session.getArgument(8, key_boundary_obj);
  key_struct = static_cast<void*> (&key_boundary_obj);
  pfc_log_info("%s", IpctUtil::get_string(key_boundary_obj).c_str());
  // populate val_boundary_obj structure
  memset(&val_boundary_obj, 0, sizeof(val_boundary_obj));
  int val_read = session.getArgument(9, val_boundary_obj);
  if (val_read == 0) {
    val_struct = static_cast<void*> (&val_boundary_obj);
    pfc_log_info("%s", IpctUtil::get_string(val_boundary_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

/**AddControllerStructure
 * @Description : This function is to add the value structure of the controller
 * @param[in]   : &session - Object of ServerSession where the request
 *                argument present
 *                obj_buffer - object of the read buffer
 *                &err - refers to the error value
 * @return      : Void 
 * */
void ReadRequest::AddControllerStructure(ServerSession &session,
                                         BulkReadBuffer obj_buffer,
                                         int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY :
      ADD_KEY_TO_SESSION(err, session,
                         (uint32_t)key_type,
                         reinterpret_cast<key_ctr_t*>(obj_buffer.value),
                         key_ctr_t);
      break;
    case IS_VALUE:
      ADD_VALUE_TO_SESSION(err, session,
                           reinterpret_cast<val_ctr_t*>(obj_buffer.value),
                           val_ctr_t);
      break;
    case IS_STATE_VALUE:
      ADD_VALUE_TO_SESSION(err, session,
                           reinterpret_cast<val_ctr_st_t*>(obj_buffer.value),
                           val_ctr_st_t);
      break;
    default:
      break;
  }
  return;
}

/**AddDomainStructure
 * @Description : This function is to add the value structure of the domain
 * @param[in]   : &session - Object of ServerSession where the request
 *                argument present
 *                obj_buffer - object of the read buffer
 *                &err - refers to the error value
 * @return      : Void 
 * */
void ReadRequest::AddDomainStructure(ServerSession &session,
                                     BulkReadBuffer obj_buffer,
                                     int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      ADD_KEY_TO_SESSION(err, session,
                         (uint32_t)key_type,
                         reinterpret_cast<key_ctr_domain_t*>(obj_buffer.value),
                         key_ctr_domain_t);
      break;
    case IS_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_ctr_domain_t*>(obj_buffer.value),
          val_ctr_domain_t);
      break;
    case IS_STATE_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_ctr_domain_st_t*>(obj_buffer.value),
          val_ctr_domain_st_t);
      break;
    default:
      break;
  }
  return;
}

/**AddLogicalPortStructure
 * @Description : This function is to add the value structure of the
 *                logical port
 * @param[in]   : &session - Object of ServerSession where the request
 *                argument present
 *                obj_buffer - object of the read buffer
 *                &err - refers to the error value
 * @return      : Void 
 * */
void ReadRequest::AddLogicalPortStructure(ServerSession &session,
                                          BulkReadBuffer obj_buffer,
                                          int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      ADD_KEY_TO_SESSION(
          err, session,
          (uint32_t)key_type,
          reinterpret_cast<key_logical_port_t*>(obj_buffer.value),
          key_logical_port_t);
      break;
    case IS_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_logical_port_t*>(obj_buffer.value),
          val_logical_port_t);
      break;
    case IS_STATE_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_logical_port_st_t*>(obj_buffer.value),
          val_logical_port_st_t);
      break;
    default:
      break;
  }
  return;
}

/**AddLogicalMemberPortStructure
 * @Description : This function is to add the value structure of the
 *                logical member port
 * @param[in]   : &session - Object of ServerSession where the request
 *                argument present
 *                obj_buffer - object of the read buffer
 *                &err - refers to the error value
 * @return      : Void 
 * */
void ReadRequest::AddLogicalMemberPortStructure(ServerSession &session,
                                                BulkReadBuffer obj_buffer,
                                                int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  if (value_type == IS_KEY) {
    ADD_KEY_TO_SESSION(
        err, session,
        (uint32_t)key_type,
        reinterpret_cast<key_logical_member_port_t*>(obj_buffer.value),
        key_logical_member_port_t);
  }
  return;
}

/**AddSwitchStructure
 * @Description : This function is to add the value structure of the switch
 * @param[in]   : &session - Object of ServerSession where the request
 *                argument present
 *                obj_buffer - object of the read buffer
 *                &err - refers to the error value
 * @return      : Void 
 * */
void ReadRequest::AddSwitchStructure(ServerSession &session,
                                     BulkReadBuffer obj_buffer,
                                     int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      ADD_KEY_TO_SESSION(
          err, session,
          (uint32_t)key_type,
          reinterpret_cast<key_switch_t*>(obj_buffer.value),
          key_switch_t);
      break;
    case IS_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_switch_t*>(obj_buffer.value),
          val_switch_t);
      break;
    case IS_STATE_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_switch_st_t*>(obj_buffer.value),
          val_switch_st_t);
      break;
    default:
      break;
  }
  return;
}

/**AddPortStructure
 * @Description : This function is to add the value structure of the port
 * @param[in]   : &session - Object of ServerSession where the request
 *                argument present
 *                obj_buffer - object of the read buffer
 *                &err - refers to the error value
 * @return      : Void 
 * */
void ReadRequest::AddPortStructure(ServerSession &session,
                                   BulkReadBuffer obj_buffer,
                                   int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      ADD_KEY_TO_SESSION(
          err, session,
          (uint32_t)key_type,
          reinterpret_cast<key_port_t*>(obj_buffer.value),
          key_port_t);
      break;
    case IS_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_port_t*>(obj_buffer.value),
          val_port_t);
      break;
    case IS_STATE_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_port_st_t*>(obj_buffer.value),
          val_port_st_t);
      break;
    default:
      break;
  }
  return;
}

/**AddLinkStructure
 * @Description : This function is to add the value structure of the link
 * @param[in]   : &session - Object of ServerSession where the request
 *                argument present
 *                obj_buffer - object of the read buffer
 *                &err - refers to the error value
 * @return      : Void 
 * */
void ReadRequest::AddLinkStructure(ServerSession &session,
                                   BulkReadBuffer obj_buffer,
                                   int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      ADD_KEY_TO_SESSION(
          err, session,
          (uint32_t)key_type,
          reinterpret_cast<key_link_t*>(obj_buffer.value),
          key_link_t);
      break;
    case IS_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_link_t*>(obj_buffer.value),
          val_link_t);
      break;
    case IS_STATE_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_link_st_t*>(obj_buffer.value),
          val_link_st_t);
      break;
    default:
      break;
  }
  return;
}

/**AddBoundaryStructure
 * @Description : This function is to add the value structure of the boundary
 * @param[in]   : &session - Object of ServerSession where the request
 *                argument present
 *                obj_buffer - object of the read buffer
 *                &err - refers to the error value
 * @return      : Void 
 * */
void ReadRequest::AddBoundaryStructure(ServerSession &session,
                                       BulkReadBuffer obj_buffer,
                                       int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      ADD_KEY_TO_SESSION(
          err, session,
          (uint32_t)key_type,
          reinterpret_cast<key_boundary_t*>(obj_buffer.value),
          key_boundary_t);
      break;
    case IS_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_boundary_t*>(obj_buffer.value),
          val_boundary_t);
      break;
    case IS_STATE_VALUE:
      ADD_VALUE_TO_SESSION(
          err, session,
          reinterpret_cast<val_boundary_st_t*>(obj_buffer.value),
          val_boundary_st_t);
      break;
    default:
      break;
  }
  return;
}

/**FlushBulkReadBuffer
 * @Description : This function is clears the internal buffer used for
 *                Read Bulk
 * @param[in]   : None
 * @return      : None
 * */
void ReadRequest::FlushBulkReadBuffer() {
  vector<BulkReadBuffer> :: iterator iter = vect_bulk_read_buffer.begin();
  // Delete the memory
  for (; iter != vect_bulk_read_buffer.end(); ++iter) {
    BulkReadBuffer obj_buffer = (*iter);
    unc_key_type_t key_type = obj_buffer.key_type;
    switch (key_type) {
      case UNC_KT_CONTROLLER:
        ClearControllerStructure(obj_buffer);
        break;
      case UNC_KT_CTR_DOMAIN:
        ClearDomainStructure(obj_buffer);
        break;
      case UNC_KT_LOGICAL_PORT:
        ClearLogicalPortStructure(obj_buffer);
        break;
      case UNC_KT_LOGICAL_MEMBER_PORT:
        ClearLogicalMemberPortStructure(obj_buffer);
        break;
      case UNC_KT_SWITCH:
        ClearSwitchStructure(obj_buffer);
        break;
      case UNC_KT_PORT:
        ClearPortStructure(obj_buffer);
        break;
      case UNC_KT_LINK:
        ClearLinkStructure(obj_buffer);
        break;
      case UNC_KT_BOUNDARY:
        ClearBoundaryStructure(obj_buffer);
        break;
      default:
        break;
    }
  }
  // Clear the vector
  vect_bulk_read_buffer.clear();
}

/**ClearControllerStructure
 * @Description : This function is to clear the value structure
 *                of the controller
 * @param[in]   : obj_buffer - object of the read buffer
 * @return      : Void
 * */
void ReadRequest::ClearControllerStructure(BulkReadBuffer obj_buffer) {
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      CLEAR_KEY(reinterpret_cast<key_ctr_t*>(obj_buffer.value),
                key_ctr_t);
      break;
    case IS_VALUE:
      CLEAR_VALUE(reinterpret_cast<val_ctr_t*>(obj_buffer.value),
                  val_ctr_t);
      break;
    case IS_STATE_VALUE:
      CLEAR_VALUE(reinterpret_cast<val_ctr_st_t*>(obj_buffer.value),
                  val_ctr_st_t);
      break;
    default:
      break;
  }
  return;
}

/**ClearDomainStructure
 * @Description : This function is to clear the value structure of the domain
 * @param[in]   : obj_buffer - object of the read buffer
 * @return      : Void
 * */
void ReadRequest::ClearDomainStructure(BulkReadBuffer obj_buffer) {
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      CLEAR_KEY(reinterpret_cast<key_ctr_domain_t*>(obj_buffer.value),
                key_ctr_domain_t);
      break;
    case IS_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_ctr_domain_t*>(obj_buffer.value),
          val_ctr_domain_t);
      break;
    case IS_STATE_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_ctr_domain_st_t*>(obj_buffer.value),
          val_ctr_domain_st_t);
      break;
    default:
      break;
  }
  return;
}

/**ClearLogicalPortStructure
 * @Description : This function is to clear the value structure of the
 *                logical port
 * @param[in]   : obj_buffer - object of the read buffer
 * @return      : Void
 * */
void ReadRequest::ClearLogicalPortStructure(BulkReadBuffer obj_buffer) {
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      CLEAR_KEY(reinterpret_cast<key_logical_port_t*>(obj_buffer.value),
                key_logical_port_t);
      break;
    case IS_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_logical_port_t*>(obj_buffer.value),
          val_logical_port_t);
      break;
    case IS_STATE_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_logical_port_st_t*>(obj_buffer.value),
          val_logical_port_st_t);
      break;
    default:
      break;
  }
  return;
}

/**ClearLogicalMemberPortStructure
 * @Description : This function is to clear the value structure of the
 *                logical member port
 * @param[in]   : obj_buffer - object of the read buffer
 * @return      : Void
 * */
void ReadRequest::ClearLogicalMemberPortStructure(BulkReadBuffer obj_buffer) {
  ValueType value_type = obj_buffer.value_type;
  if (value_type == IS_KEY) {
    CLEAR_KEY(reinterpret_cast<key_logical_member_port_t*>(obj_buffer.value),
              key_logical_member_port_t);
  }
  return;
}

/**ClearSwitchStructure
 * @Description : This function is to clear the value structure of the switch
 * @param[in]   : obj_buffer - object of the read buffer
 * @return      : Void
 * */
void ReadRequest::ClearSwitchStructure(BulkReadBuffer obj_buffer) {
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      CLEAR_KEY(reinterpret_cast<key_switch_t*>(obj_buffer.value),
                key_switch_t);
      break;
    case IS_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_switch_t*>(obj_buffer.value),
          val_switch_t);
      break;
    case IS_STATE_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_switch_st_t*>(obj_buffer.value),
          val_switch_st_t);
      break;
    default:
      break;
  }
  return;
}

/**ClearPortStructure
 * @Description : This function is to clear the value structure of the port
 * @param[in]   : obj_buffer - object of the read buffer
 * @return      : Void
 * */
void ReadRequest::ClearPortStructure(BulkReadBuffer obj_buffer) {
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      CLEAR_KEY(reinterpret_cast<key_port_t*>(obj_buffer.value),
                key_port_t);
      break;
    case IS_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_port_t*>(obj_buffer.value),
          val_port_t);
      break;
    case IS_STATE_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_port_st_t*>(obj_buffer.value),
          val_port_st_t);
      break;
    default:
      break;
  }
  return;
}

/**ClearLinkStructure
 * @Description : This function is to clear the value structure of the link
 * @param[in]   : obj_buffer - object of the read buffer
 * @return      : Void
 * */
void ReadRequest::ClearLinkStructure(BulkReadBuffer obj_buffer) {
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      CLEAR_KEY(reinterpret_cast<key_link_t*>(obj_buffer.value),
                key_link_t);
      break;
    case IS_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_link_t*>(obj_buffer.value),
          val_link_t);
      break;
    case IS_STATE_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_link_st_t*>(obj_buffer.value),
          val_link_st_t);
      break;
    default:
      break;
  }
  return;
}

/**ClearBoundaryStructure
 * @Description : This function is to clear the value structure of the boundary
 * @param[in]   : obj_buffer - object of the read buffer
 * @return      : Void
 * */
void ReadRequest::ClearBoundaryStructure(BulkReadBuffer obj_buffer) {
  ValueType value_type = obj_buffer.value_type;
  switch (value_type) {
    case IS_KEY:
      CLEAR_KEY(reinterpret_cast<key_boundary_t*>(obj_buffer.value),
                key_boundary_t);
      break;
    case IS_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_boundary_t*>(obj_buffer.value),
          val_boundary_t);
      break;
    case IS_STATE_VALUE:
      CLEAR_VALUE(
          reinterpret_cast<val_boundary_st_t*>(obj_buffer.value),
          val_boundary_st_t);
      break;
    default:
      break;
  }
  return;
}
