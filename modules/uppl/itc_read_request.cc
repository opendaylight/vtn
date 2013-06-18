/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    ITC Read Req
 * @file     itc_Read_request.cc
 *
 */

#include "itc_kt_base.hh"
#include "itc_kt_root.hh"
#include "itc_kt_controller.hh"
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

/*  ReadRequest
 *  @Description    : constructor function
 *  @param[in]: NA
 *  @return   : NA
 * */

ReadRequest::ReadRequest() {
  memset(&key_root_obj, 0, sizeof(key_root_t));
  memset(&key_ctr_obj, 0, sizeof(key_ctr_t));
  memset(&val_ctr_obj, 0, sizeof(val_ctr_t));
  memset(&key_domain_obj, 0, sizeof(key_ctr_domain_t));
  memset(&val_domain_obj, 0, sizeof(val_ctr_domain_t));
  memset(&key_logical_port_obj, 0, sizeof(key_logical_port_t));
  memset(&val_logical_port_obj, 0, sizeof(val_logical_port_st_t));
  memset(&key_logical_member_port_obj, 0, sizeof(key_logical_member_port_t));
  memset(&key_switch_obj, 0, sizeof(key_switch_t));
  memset(&val_switch_obj, 0, sizeof(val_switch_st_t));
  memset(&key_port_obj, 0, sizeof(key_port_t));
  memset(&val_port_obj, 0, sizeof(val_port_st_t));
  memset(&key_link_obj, 0, sizeof(key_link_t));
  memset(&val_link_obj, 0, sizeof(val_link_st_t));
  memset(&key_boundary_obj, 0, sizeof(key_boundary_t));
  memset(&val_boundary_obj, 0, sizeof(val_boundary_t));
}

/* ~ReadRequest
 * @Description    : Destructor function
 * @param[in]:  NA
 * @return   :  NA
 * */
ReadRequest::~ReadRequest() {
  // destructor functionality
}

/*  ProcessReq
 *  @Description    : Creates the respective Kt class object to
 *  process the readreq operation
 *  @param[in]: ipc struct, service id, session id, configuration id,
 *  session object
 *  @return   : read operation response success/failure.
 * */
UpplReturnCode ReadRequest::ProcessReq(ServerSession &session) {
  Kt_Base *KtObj = NULL;

  physical_request_header obj_req_hdr;

  //  populate header from ipc message
  int parse_ret = PhyUtil::sessGetReqHeader(session, obj_req_hdr);
  if (parse_ret != 0) {
    pfc_log_error("Unable to parse ipc structure. BAD_REQUEST error");
    return UPPL_RC_ERR_BAD_REQUEST;
  }

  UpplReturnCode resp_code = UPPL_RC_SUCCESS, return_code = UPPL_RC_SUCCESS;
  physical_response_header rsh;
  PhyUtil::getRespHeaderFromReqHeader(obj_req_hdr, rsh);

  void* key_struct = NULL;
  void* val_struct = NULL;
  uint32_t operation_type = obj_req_hdr.operation;
  uint32_t key_type = obj_req_hdr.key_type;
  // get KeyType and create the respective object to invoke appropriate Kt class
  switch (key_type) {
    case UNC_KT_ROOT:
    {
      if (operation_type == UNC_OP_READ ||
          operation_type == UNC_OP_READ_SIBLING ||
          operation_type == UNC_OP_READ_SIBLING_BEGIN ||
          operation_type == UNC_OP_READ_SIBLING_COUNT) {
        rsh.result_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
      }
      KtObj = new Kt_Root();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Root");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      memset(&key_root_obj, 0, sizeof(key_root_t));
      // The root key in request is not considered
      key_struct = static_cast<void*> (&key_root_obj);
      break;
    }
    case  UNC_KT_CONTROLLER:
    {
      KtObj = new Kt_Controller();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Controller");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetControllerStructure(session, key_struct, val_struct, rsh);
      break;
    }
    case UNC_KT_CTR_DOMAIN:
    {
      KtObj = new Kt_Ctr_Domain();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Ctr_Domain");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetDomainStructure(session, key_struct, val_struct, rsh);
      break;
    }
    case UNC_KT_LOGICAL_PORT:
    {
      KtObj = new Kt_LogicalPort();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_LogicalPort");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetLogicalPortStructure(session, key_struct, val_struct, rsh);
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT:
    {
      KtObj = new Kt_LogicalMemberPort();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_LogicalMemberPort");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetLogicalMemberPortStructure(session, key_struct, val_struct, rsh);
      break;
    }
    case UNC_KT_SWITCH:
    {
      KtObj = new Kt_Switch();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Switch");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetSwitchStructure(session, key_struct, val_struct, rsh);
      break;
    }
    case UNC_KT_PORT:
    {
      KtObj = new Kt_Port();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Port");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetPortStructure(session, key_struct, val_struct, rsh);
      break;
    }
    case UNC_KT_LINK:
    {
      KtObj = new Kt_Link();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Link");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetLinkStructure(session, key_struct, val_struct, rsh);
      break;
    }
    case UNC_KT_BOUNDARY:
    {
      KtObj = new Kt_Boundary();
      if (KtObj  == NULL) {
        pfc_log_error("Memory not allocated for Kt_Boundary");
        return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      GetBoundaryStructure(session, key_struct, val_struct, rsh);
      break;
    }
    default:
    {
      rsh.result_code = UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED;
    }
  }
  if (rsh.result_code != UPPL_RC_SUCCESS) {
    if (KtObj != NULL) {
      delete KtObj;
      KtObj = NULL;
    }
    pfc_log_info("Read service failed with error %d", rsh.result_code);
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (err != 0) {
      return_code = UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      return_code = UPPL_RC_SUCCESS;
    }
    return return_code;
  }
  switch (operation_type) {
    case UNC_OP_READ:
    case UNC_OP_READ_NEXT:
    case UNC_OP_READ_BULK:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_COUNT:
    {
      // form validate request for READ operation
      resp_code = KtObj->ValidateRequest(key_struct,
                                         NULL,
                                         operation_type,
                                         obj_req_hdr.data_type,
                                         key_type);
      break;
    }
    default:
      resp_code = UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
      break;
  }
  if (resp_code != UPPL_RC_SUCCESS) {
    // validation failed call add out put
    rsh.result_code = resp_code;
    pfc_log_error("read validation failed");
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (KtObj != NULL) {
      err |= KtObj->AddKeyStructuretoSession(obj_req_hdr.key_type,
                                             &session, key_struct);
    }
    if (err != 0) {
      return_code = UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      return_code = UPPL_RC_SUCCESS;
    }
  } else {
    return_code = ProcessReadOperation(session, KtObj, obj_req_hdr,
                                       key_struct, val_struct,
                                       operation_type);
  }
  if (KtObj != NULL) {
    delete KtObj;
    KtObj = NULL;
  }
  return return_code;
}


/*  ProcessReq
 *  @Description    : Creates the respective Kt class object
 *   to process the readreq operation
 *  @param[in]: ipc struct, service id, session id, configuration id,
 *   session object
 *  @return   : read operation response success/failure.
 * */
UpplReturnCode ReadRequest::ProcessReadOperation(
    ServerSession &session,
    Kt_Base *KtObj,
    physical_request_header obj_req_hdr,
    void* key_struct,
    void* val_struct,
    uint32_t operation_type) {
  UpplReturnCode resp_code = UPPL_RC_SUCCESS, return_code = UPPL_RC_SUCCESS;
  physical_response_header rsh;
  PhyUtil::getRespHeaderFromReqHeader(obj_req_hdr, rsh);
  pfc_bool_t response_sent = PFC_FALSE;
  switch (operation_type) {
    case UNC_OP_READ:
    {
      // Invoke Read operation for respective KT class
      resp_code = KtObj->Read(obj_req_hdr.client_sess_id,
                              obj_req_hdr.config_id,
                              key_struct,
                              val_struct,
                              obj_req_hdr.data_type,
                              session,
                              obj_req_hdr.option1,
                              obj_req_hdr.option2);
      response_sent = PFC_TRUE;
      break;
    }
    case UNC_OP_READ_NEXT:
    {
      // Invoke Read Next operation for respective KT class
      resp_code = KtObj->ReadNext(key_struct,
                                  obj_req_hdr.data_type,
                                  obj_req_hdr.option1,
                                  obj_req_hdr.option2);
      if (resp_code == UPPL_RC_SUCCESS) {
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
    }
    case UNC_OP_READ_BULK:
    {
      // bool is_header_added = false;
      uint32_t max_rep_ct = obj_req_hdr.max_rep_count;
      if (max_rep_ct > UPPL_MAX_REP_CT) {
        pfc_log_info("User requested more than 10000 records!!");
        max_rep_ct = UPPL_MAX_REP_CT;
      }
      // Invoke Read Bulk operation for respective KT class
      resp_code = KtObj->ReadBulk(key_struct,
                                  obj_req_hdr.data_type,
                                  obj_req_hdr.option1,
                                  obj_req_hdr.option2,
                                  max_rep_ct,
                                  -1,
                                  false,
                                  false);
      if (resp_code == UPPL_RC_SUCCESS) {
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
    }
    case UNC_OP_READ_SIBLING_BEGIN:
    {
      // Invoke Read Sibling Begin operation for respective KT class
      resp_code = KtObj->ReadSiblingBegin(obj_req_hdr.client_sess_id,
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
    }
    case UNC_OP_READ_SIBLING:
    {
      // Invoke Read Sibling operation for respective KT class
      resp_code = KtObj->ReadSibling(obj_req_hdr.client_sess_id,
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
    }
    case UNC_OP_READ_SIBLING_COUNT:
    {
      // Invoke Read Sibling Count operation for respective KT class
      resp_code = KtObj->ReadSiblingCount(obj_req_hdr.client_sess_id,
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
    }
    default:
      resp_code = UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  if (!response_sent) {
    rsh.result_code = resp_code;
    pfc_log_error("read validation failed");
    int err = PhyUtil::sessOutRespHeader(session, rsh);
    if (err != 0) {
      return_code = UPPL_RC_ERR_IPC_WRITE_ERROR;
    } else {
      return_code = UPPL_RC_SUCCESS;
    }
  }
  return return_code;
}

UpplReturnCode ReadRequest::FrameReadBulkResponse(ServerSession &session,
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
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
      get_physical_core();
  InternalTransactionCoordinator *itc_trans  =
      physical_core->get_internal_transaction_coordinator();
  vector<BulkReadBuffer> bulk_read_buffer = itc_trans->get_readbulk_buffer();
  vector<BulkReadBuffer> :: iterator iter = bulk_read_buffer.begin();
  for (; iter != bulk_read_buffer.end(); ++iter) {
    if ((*iter).value_type == IS_KEY) { ++max_rep_ct;}
  }
  rsh.max_rep_count = max_rep_ct;
  if (bulk_read_buffer.empty()) {
    pfc_log_debug("No instances available to read");
    rsh.result_code = UPPL_RC_ERR_NO_SUCH_INSTANCE;
  }
  int err = PhyUtil::sessOutRespHeader(session, rsh);
  iter = bulk_read_buffer.begin();
  int vect_size = bulk_read_buffer.size();
  for (; iter != bulk_read_buffer.end(); ++iter, --vect_size) {
    BulkReadBuffer obj_buffer = (*iter);
    unc_key_type_t key_type = obj_buffer.key_type;
    ValueType value_type = obj_buffer.value_type;
    if (key_type == UNC_KT_CONTROLLER) {
      AddControllerStructure(session, obj_buffer, err);
    }
    if (key_type == UNC_KT_CTR_DOMAIN) {
      AddDomainStructure(session, obj_buffer, err);
    }
    if (key_type == UNC_KT_LOGICAL_PORT) {
      AddLogicalPortStructure(session, obj_buffer, err);
    }
    if (key_type == UNC_KT_LOGICAL_MEMBER_PORT) {
      AddLogicalMemberPortStructure(session, obj_buffer, err);
    }
    if (key_type == UNC_KT_SWITCH) {
      AddSwitchStructure(session, obj_buffer, err);
    }
    if (key_type == UNC_KT_PORT) {
      AddPortStructure(session, obj_buffer, err);
    }
    if (key_type == UNC_KT_LINK) {
      AddLinkStructure(session, obj_buffer, err);
    }
    if (key_type == UNC_KT_BOUNDARY) {
      AddBoundaryStructure(session, obj_buffer, err);
    }
    if (vect_size != 1 && value_type == IS_SEPARATOR) {
      err |= session.addOutput();  // NULL
    }
    if (err != 0) {
      pfc_log_error("Server session addOutput failed");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  itc_trans->FlushBulkReadBuffer();
  return UPPL_RC_SUCCESS;
}

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
  pfc_log_debug("%s", IpctUtil::get_string(key_ctr_obj).c_str());
  int val_read = session.getArgument(9, val_ctr_obj);
  val_struct = static_cast<void*> (&val_ctr_obj);
  if (val_read != 0) {
    pfc_log_debug("%s", IpctUtil::get_string(val_ctr_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

void ReadRequest::GetDomainStructure(ServerSession &session,
                                     void * &key_struct,
                                     void * &val_struct,
                                     physical_response_header &rsh) {
  // populate key_domain_obj structure
  rsh.result_code = session.getArgument(8, key_domain_obj);
  key_struct = static_cast<void*> (&key_domain_obj);
  pfc_log_debug("%s", IpctUtil::get_string(key_domain_obj).c_str());
  // populate val_domain_obj structure
  memset(&val_domain_obj, 0, sizeof(val_domain_obj));
  int val_read = session.getArgument(9, val_domain_obj);
  val_struct = static_cast<void*> (&val_domain_obj);
  if (val_read != 0) {
    pfc_log_debug("%s", IpctUtil::get_string(val_domain_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

void ReadRequest::GetLogicalPortStructure(ServerSession &session,
                                          void * &key_struct,
                                          void * &val_struct,
                                          physical_response_header &rsh) {
  // populate key_logical_port structure
  session.getArgument(8, key_logical_port_obj);
  key_struct = static_cast<void*> (&key_logical_port_obj);
  pfc_log_debug("%s", IpctUtil::get_string(key_logical_port_obj).c_str());
  // populate val_logical_port structure
  memset(&val_logical_port_obj, 0, sizeof(val_logical_port_obj));
  int val_read = session.getArgument(9, val_logical_port_obj);
  val_struct = static_cast<void*> (&val_logical_port_obj);
  if (val_read != 0) {
    pfc_log_debug("%s", IpctUtil::get_string(val_logical_port_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

void ReadRequest::GetLogicalMemberPortStructure(ServerSession &session,
                                                void * &key_struct,
                                                void * &val_struct,
                                                physical_response_header &rsh) {
  // populate key_logical_member_port structure
  rsh.result_code = session.getArgument(8, key_logical_member_port_obj);
  key_struct = static_cast<void*> (&key_logical_member_port_obj);
  pfc_log_debug("%s",
                IpctUtil::get_string(key_logical_member_port_obj).c_str());
  return;
}

void ReadRequest::GetSwitchStructure(ServerSession &session,
                                     void * &key_struct,
                                     void * &val_struct,
                                     physical_response_header &rsh) {
  // populate key_switch_obj structure
  rsh.result_code = session.getArgument(8, key_switch_obj);
  key_struct = static_cast<void*> (&key_switch_obj);
  pfc_log_debug("%s", IpctUtil::get_string(key_switch_obj).c_str());
  // populate val_switch structure
  memset(&val_switch_obj, 0, sizeof(val_switch_obj));
  int val_read = session.getArgument(9, val_switch_obj);
  val_struct = static_cast<void*> (&val_switch_obj);
  if (val_read != 0) {
    pfc_log_debug("%s", IpctUtil::get_string(val_switch_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

void ReadRequest::GetPortStructure(ServerSession &session,
                                   void * &key_struct,
                                   void * &val_struct,
                                   physical_response_header &rsh) {
  // populate key_port structure
  rsh.result_code = session.getArgument(8, key_port_obj);
  key_struct = static_cast<void*> (&key_port_obj);
  pfc_log_debug("%s", IpctUtil::get_string(key_port_obj).c_str());
  // populate val_port structure
  memset(&val_port_obj, 0, sizeof(val_port_obj));
  int val_read = session.getArgument(9, val_port_obj);
  val_struct = static_cast<void*> (&val_port_obj);
  if (val_read != 0) {
    pfc_log_debug("%s", IpctUtil::get_string(val_port_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

void ReadRequest::GetLinkStructure(ServerSession &session,
                                   void * &key_struct,
                                   void * &val_struct,
                                   physical_response_header &rsh) {
  // populate key_link_obj structure
  rsh.result_code = session.getArgument(8, key_link_obj);
  key_struct = static_cast<void*> (&key_link_obj);
  pfc_log_debug("%s", IpctUtil::get_string(key_link_obj).c_str());
  // populate val_link structure
  memset(&val_link_obj, 0, sizeof(val_link_obj));
  int val_read = session.getArgument(9, val_link_obj);
  val_struct = static_cast<void*> (&val_link_obj);
  if (val_read != 0) {
    pfc_log_debug("%s", IpctUtil::get_string(val_link_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

void ReadRequest::GetBoundaryStructure(ServerSession &session,
                                       void * &key_struct,
                                       void * &val_struct,
                                       physical_response_header &rsh) {
  // populate key_boundary_obj structure
  rsh.result_code = session.getArgument(8, key_boundary_obj);
  key_struct = static_cast<void*> (&key_boundary_obj);
  pfc_log_debug("%s", IpctUtil::get_string(key_boundary_obj).c_str());
  // populate val_boundary_obj structure
  memset(&val_boundary_obj, 0, sizeof(val_boundary_obj));
  int val_read = session.getArgument(9, val_boundary_obj);
  val_struct = static_cast<void*> (&val_boundary_obj);
  if (val_read != 0) {
    pfc_log_debug("%s", IpctUtil::get_string(val_boundary_obj).c_str());
  } else {
    pfc_log_debug("No value structure provided in read request");
  }
  return;
}

void ReadRequest::AddControllerStructure(ServerSession &session,
                                         BulkReadBuffer obj_buffer,
                                         int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  if (value_type == IS_KEY) {
    ADD_KEY_TO_SESSION(err, session,
                       (uint32_t)key_type,
                       reinterpret_cast<key_ctr_t*>(obj_buffer.value),
                       key_ctr_t);
  }
  if (value_type == IS_VALUE) {
    ADD_VALUE_TO_SESSION(err, session,
                         reinterpret_cast<val_ctr_t*>(obj_buffer.value),
                         val_ctr_t);
  }
  if (value_type == IS_STATE_VALUE) {
    ADD_VALUE_TO_SESSION(err, session,
                         reinterpret_cast<val_ctr_st_t*>(obj_buffer.value),
                         val_ctr_st_t);
  }
  return;
}

void ReadRequest::AddDomainStructure(ServerSession &session,
                                     BulkReadBuffer obj_buffer,
                                     int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  if (value_type == IS_KEY) {
    ADD_KEY_TO_SESSION(err, session,
                       (uint32_t)key_type,
                       reinterpret_cast<key_ctr_domain_t*>(obj_buffer.value),
                       key_ctr_domain_t);
  }
  if (value_type == IS_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_ctr_domain_t*>(obj_buffer.value),
        val_ctr_domain_t);
  }
  if (value_type == IS_STATE_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_ctr_domain_st_t*>(obj_buffer.value),
        val_ctr_domain_st_t);
  }
  return;
}

void ReadRequest::AddLogicalPortStructure(ServerSession &session,
                                          BulkReadBuffer obj_buffer,
                                          int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  if (value_type == IS_KEY) {
    ADD_KEY_TO_SESSION(
        err, session,
        (uint32_t)key_type,
        reinterpret_cast<key_logical_port_t*>(obj_buffer.value),
        key_logical_port_t);
  }
  if (value_type == IS_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_logical_port_t*>(obj_buffer.value),
        val_logical_port_t);
  }
  if (value_type == IS_STATE_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_logical_port_st_t*>(obj_buffer.value),
        val_logical_port_st_t);
  }
  return;
}

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

void ReadRequest::AddSwitchStructure(ServerSession &session,
                                     BulkReadBuffer obj_buffer,
                                     int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;

  if (value_type == IS_KEY) {
    ADD_KEY_TO_SESSION(
        err, session,
        (uint32_t)key_type,
        reinterpret_cast<key_switch_t*>(obj_buffer.value),
        key_switch_t);
  }
  if (value_type == IS_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_switch_t*>(obj_buffer.value),
        val_switch_t);
  }
  if (value_type == IS_STATE_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_switch_st_t*>(obj_buffer.value),
        val_switch_st_t);
  }
  return;
}

void ReadRequest::AddPortStructure(ServerSession &session,
                                   BulkReadBuffer obj_buffer,
                                   int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;

  if (value_type == IS_KEY) {
    ADD_KEY_TO_SESSION(
        err, session,
        (uint32_t)key_type,
        reinterpret_cast<key_port_t*>(obj_buffer.value),
        key_port_t);
  }
  if (value_type == IS_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_port_t*>(obj_buffer.value),
        val_port_t);
  }
  if (value_type == IS_STATE_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_port_st_t*>(obj_buffer.value),
        val_port_st_t);
  }
  return;
}

void ReadRequest::AddLinkStructure(ServerSession &session,
                                   BulkReadBuffer obj_buffer,
                                   int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  if (value_type == IS_KEY) {
    ADD_KEY_TO_SESSION(
        err, session,
        (uint32_t)key_type,
        reinterpret_cast<key_link_t*>(obj_buffer.value),
        key_link_t);
  }
  if (value_type == IS_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_link_t*>(obj_buffer.value),
        val_link_t);
  }
  if (value_type == IS_STATE_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_link_st_t*>(obj_buffer.value),
        val_link_st_t);
  }
  return;
}

void ReadRequest::AddBoundaryStructure(ServerSession &session,
                                       BulkReadBuffer obj_buffer,
                                       int &err) {
  unc_key_type_t key_type = obj_buffer.key_type;
  ValueType value_type = obj_buffer.value_type;
  if (value_type == IS_KEY) {
    ADD_KEY_TO_SESSION(
        err, session,
        (uint32_t)key_type,
        reinterpret_cast<key_boundary_t*>(obj_buffer.value),
        key_boundary_t);
  }
  if (value_type == IS_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_boundary_t*>(obj_buffer.value),
        val_boundary_t);
  }
  if (value_type == IS_STATE_VALUE) {
    ADD_VALUE_TO_SESSION(
        err, session,
        reinterpret_cast<val_boundary_st_t*>(obj_buffer.value),
        val_boundary_st_t);
  }
  return;
}

