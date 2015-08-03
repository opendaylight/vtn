/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Ctr_Dataflow implementation
 * @file    itc_kt_ctr_Dataflow.cc
 *
 */
#include "ipc_client_configuration_handler.hh"
#include "itc_kt_ctr_dataflow.hh"
#include "itc_kt_controller.hh"
#include "odbcm_utils.hh"
#include "itc_read_request.hh"
#include "odbcm_db_varbind.hh"
#include "uncxx/dataflow.hh"
#include "capa_module.hh"

using unc::uppl::PhysicalLayer;
using unc::uppl::ODBCMUtils;
using unc::uppl::IPCClientDriverHandler;
using unc::dataflow::DataflowCmn;
using unc::dataflow::DataflowUtil;
using unc::dataflow::DataflowDetail;

/** Constructor
 * @Description : This function fills the syntax map used for validation
 * @param[in] : None
 * @return    : None
 * */
Kt_Ctr_Dataflow::Kt_Ctr_Dataflow() {
  if (attr_syntax_map_all.find(UNC_KT_CTR_DATAFLOW) ==
      attr_syntax_map_all.end()) {
    // Populate Structure to be used for syntax validation
    Fill_Attr_Syntax_Map();
  }
}

/** Destructor
 * @Description : Empty Destructor
 * @param[in] : None
 * @return    : None
 * */
Kt_Ctr_Dataflow::~Kt_Ctr_Dataflow() {
}

/* * ReadInternal
 * @Description : This function reads the given instance of Kt Ctr_Dataflow
 * @param[in] : key_struct - key for kt ctr_Dataflow instance
 * value_struct - value for kt ctr_Dataflow instance
 * data_type - UNC_DT_*, read allowed in STATE
 * operation_type-type of operation,can be UNC_OP_READ
 * @return    : UNC_RC_SUCCESS is returned when the response is added to
 * ipc session successfully
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Ctr_Dataflow::ReadInternal(OdbcmConnectionHandler *db_conn,
                                       vector<void *> &key_val,
                                       vector<void *> &val_struct,
                                       uint32_t data_type,
                                       uint32_t operation_type) {
  pfc_log_debug("ReadInternal:KT_CTR_DATAFLOW");
  return UNC_RC_SUCCESS;
}

/** ReadBulk
 * @Description : This function reads the max_rep_ct number of instances
 *  of the KT_Ctr_Dataflow
 * @param[in] :
 * key_struct - key for switch instance
 * data_type - UNC_DT_* , ReadBulk supported for STATE
 * max_rep_ct - specifies no of rows to be returned
 * parent_call - indicates whether parent has called this read bulk
 * is_read_next - indicates whether this function is invoked from read_next
 * @return    : UNC_RC_SUCCESS is returned when the response is added to
 * ipc session successfully
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess
 * */
UncRespCode Kt_Ctr_Dataflow::ReadBulk(OdbcmConnectionHandler *db_conn,
                                   void* key_struct,
                                   uint32_t data_type,
                                   uint32_t &max_rep_ct,
                                   int child_index,
                                   pfc_bool_t parent_call,
                                   pfc_bool_t is_read_next,
                                   ReadRequest *read_req) {
  pfc_log_info("ReadBulk operation is not allowed in Kt_Ctr_Dataflow");
  return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
}

/** PerformSyntaxValidation
 * @Description : This function performs syntax validation for KT_CTR_DATAFLOW
 * @param[in] : key_struct - key for kt ctr_Dataflow instance
 * value_struct - value for kt ctr_Dataflow instance
 * operation - UNC_OP_*,type of operation READ
 * data_type - UNC_DT_*,type of database STATE
 * @return    : UNC_RC_SUCCESS is returned when the validation is successful
 * UNC_UPPL_RC_ERR_* is returned when validation is failure or associated error code
 * */
UncRespCode Kt_Ctr_Dataflow::PerformSyntaxValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  pfc_log_debug("PerformSyntax Validation:KT_CTR_DATAFLOW");
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;

  // Validate Key Structure
  key_ctr_dataflow *key = reinterpret_cast<key_ctr_dataflow_t*>(key_struct);
  string ctrl_name = reinterpret_cast<char*>(key->ctr_key.controller_name);
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_CTR_DATAFLOW];
  IS_VALID_STRING_KEY(CTR_NAME_STR, ctrl_name, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Syntax validation failed for attribute controller_name:%d",
        ret_code);
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  uint64_t val = key->flow_id;
  if (val <= 0) {
    ret_code = UNC_UPPL_RC_ERR_CFG_SYNTAX;
    pfc_log_debug("Syntax validation failed for attribute flow_id:%d",
        ret_code);
  }
  return ret_code;
}

/** PerformSemanticValidation
 * @Description : This function performs semantic validation
 *                  for UNC_KT_CTR_DATAFLOW
 * @param[in] : key_struct - specifies key instance of KT_CTR_DATAFLOW
 * value_struct - specifies value of KT_CTR_DATAFLOW
 * operation - UNC_OP*,type of operation READ
 * data_type - UNC_DT*,type of database STATE
 * @return    : UNC_RC_SUCCESS if semantic validation is successful
 * or UNC_UPPL_RC_ERR_* if failed
 * */
UncRespCode Kt_Ctr_Dataflow::PerformSemanticValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_log_trace("PerformSemanticValidation:KT_CTR_DATAFLOW");
  uint32_t nums = 0;
  const uint8_t *attrs = 0;
  if (operation != UNC_OP_READ || data_type != UNC_DT_STATE) {
    pfc_log_debug("operation is not supported !!");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  // Not allowing the STANDBY request
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  if (physical_layer->get_physical_core()->get_system_state() == \
       UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("System is in Standby mode");
    return UNC_UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
  }
  unc::capa::CapaModule *capa = reinterpret_cast<unc::capa::CapaModule *>(
    pfc::core::Module::getInstance("capa"));
  if (capa == NULL) {
     UPPL_LOG_FATAL("%s:%d: CapaModule is not found", __FUNCTION__, __LINE__);
     return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  }
  // collect controller type, actual version, configured version, oper_status
  Kt_Controller KtObj;
  vector<void *> vect_key_ctr;
  vect_key_ctr.push_back(key_struct);
  vector<void *> vect_val_ctr;
  ret_code = KtObj.ReadInternal(db_conn, vect_key_ctr, vect_val_ctr,
                              UNC_DT_RUNNING, UNC_OP_READ);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Read of Controller failed");
    return ret_code;
  }
  key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(vect_key_ctr[0]);
  val_ctr_st *val_ctr_vect =
                     reinterpret_cast <val_ctr_st*>(vect_val_ctr[0]);
  if (val_ctr_vect == NULL || ctr_key == NULL) {
    if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
    }
    return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
  }
  unc_keytype_ctrtype_t ctr_type =
           (unc_keytype_ctrtype_t) val_ctr_vect->controller.type;
  // configured version
  string version = (const char*)val_ctr_vect->controller.version;
  pfc_log_debug("controller_type=%d oper_status_db=%d config version=%s",
                 val_ctr_vect->controller.type, val_ctr_vect->oper_status,
                 version.c_str());

  if (val_ctr_vect->controller.type != UNC_CT_PFC &&
      val_ctr_vect->controller.type != UNC_CT_ODC) {
    pfc_log_error("Read operation is NOT allowed for non-PFC/ODC controllers");
    ret_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else if (val_ctr_vect->oper_status != UPPL_CONTROLLER_OPER_UP) {
    pfc_log_error("Read operation is NOT allowed if the controller is DOWN");
    ret_code = UNC_UPPL_RC_ERR_CTRLR_DISCONNECTED;
  }
  if (ret_code != UNC_RC_SUCCESS) {
    if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
    }
    if (val_ctr_vect != NULL) {
      delete val_ctr_vect;
      val_ctr_vect = NULL;
    }
    return ret_code;
  }

  // Call CapaModule and check for actual version
  bool ret_actual = capa->GetReadCapability(ctr_type,
                       version,
                       UNC_KT_CTR_DATAFLOW,
                       &nums,
                       &attrs);
  pfc_log_debug("return of GetReadCapability = %d", ret_actual);
  if (ret_actual != true) {
    pfc_log_info("KEY TYPE is NOT supported for version : %s",
                       version.c_str());
     ret_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }

  pfc_log_debug("KEY TYPE is supported for version : %s",
                      version.c_str());
  if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
  }
  if (val_ctr_vect != NULL) {
      delete val_ctr_vect;
      val_ctr_vect = NULL;
  }
  return ret_code;
}

/** PerformRead
 * @Description : This function reads the instance of KT_Ctr_Dataflow based on
 * operation type - READ 
 * @param[in] : ipc session id - ipc session id used for TC validation
 * configuration id - configuration id used for TC validation
 * key_struct - key instance of KT_Ctr_Dataflow
 * value_struct - value instance of Kt_Ctr_Dataflow
 * data_type - UNC_DT_* , read allowed in STATE,UNC_DT_*
 * operation_type -  specifies the operation type,UNC_OP_*
 * sess - ipc server session where the response has to be added
 * option1, option2 - additional condition associated with read operation
 * max_rep_ct - specifies no of rows to be returned
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR_*
 * */
UncRespCode Kt_Ctr_Dataflow::PerformRead(OdbcmConnectionHandler *db_conn,
                                      uint32_t session_id,
                                      uint32_t configuration_id,
                                      void* key_struct,
                                      void* val_struct,
                                      uint32_t data_type,
                                      uint32_t operation_type,
                                      ServerSession &sess,
                                      uint32_t option1,
                                      uint32_t option2,
                                      uint32_t max_rep_ct) {
  pfc_log_debug("PerformRead oper=%d,dt=%d",
               operation_type, data_type);

  physical_response_header rsh = {session_id,
      configuration_id,
      operation_type,
      max_rep_ct,
      option1,
      option2,
      data_type,
      static_cast<uint32_t>(0)};
  if (operation_type == UNC_OP_READ) {
    max_rep_ct = 1;
  }
  key_ctr_dataflow_t *obj_key_ctr_dataflow
      = reinterpret_cast<key_ctr_dataflow_t*>(key_struct);

  if (option1 != UNC_OPT1_NORMAL && option1 != UNC_OPT1_DETAIL) {
    pfc_log_error("Invalid option1 specified for read operation");
    rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION1;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CTR_DATAFLOW);
    err |= sess.addOutput(*obj_key_ctr_dataflow);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  rsh.result_code = PerformSemanticValidation(db_conn, key_struct,
                         val_struct, operation_type, data_type);
  if (rsh.result_code != UNC_RC_SUCCESS) {
    pfc_log_error("PerformSemanticValidation failed %d", rsh.result_code);
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CTR_DATAFLOW);
    err |= sess.addOutput(*obj_key_ctr_dataflow);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  string controller_name =
     reinterpret_cast<char*>(obj_key_ctr_dataflow->ctr_key.controller_name);
  string domain_id = "";
  uint64_t flow_id = obj_key_ctr_dataflow->flow_id;

  UncRespCode sess_err = UNC_RC_SUCCESS;
  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
  sess_err = PhyUtil::get_controller_type(db_conn,
                                          controller_name,
                                          ctr_type, UNC_DT_RUNNING);
  if (sess_err != UNC_RC_SUCCESS) {
    pfc_log_debug("Failed to get controller type with response code :%d",
                     sess_err);
    rsh.result_code = sess_err;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CTR_DATAFLOW);
    err |= sess.addOutput(*obj_key_ctr_dataflow);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("creating IPCClientDriverHandler object,"
                " with ctr type and result code variable is passed");
  IPCClientDriverHandler pfc_drv_handler(ctr_type, sess_err);
  if (sess_err != UNC_RC_SUCCESS) {
    pfc_log_error("Cannot open session to %d driver", ctr_type);
    return sess_err;
  }
  ClientSession *cli_session = pfc_drv_handler.ResetAndGetSession();
  driver_request_header rqh = {uint32_t(0), uint32_t(0), controller_name,
      domain_id, static_cast<uint32_t>(UNC_OP_READ), uint32_t(0),
      option1, (uint32_t)0, static_cast<uint32_t>(UNC_DT_STATE),
      static_cast<uint32_t>(UNC_KT_CTR_DATAFLOW)};
  int err1 = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
  err1 |= cli_session->addOutput(*obj_key_ctr_dataflow);
  if (err1 != 0) {
    pfc_log_error("addOutput failed for driver_request_header");
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  // Send the request to driver
  UncRespCode driver_response = UNC_RC_SUCCESS;
  driver_response_header rsp;

  driver_response = pfc_drv_handler.SendReqAndGetResp(rsp);
  if (driver_response != UNC_RC_SUCCESS) {
    pfc_log_error("Read request to Driver failed for controller %s"
        " ,flow_id %" PFC_PFMT_u64 " with response %d", controller_name.c_str(),
        flow_id, driver_response);
    rsh.result_code = driver_response;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_CTR_DATAFLOW);
    err |= sess.addOutput(*obj_key_ctr_dataflow);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }

  int read_err_status = UNC_RC_SUCCESS;
  uint32_t resp_pos = 12;
  DataflowDetail *df_segment =
    new DataflowDetail(kidx_val_df_data_flow_cmn, ctr_type);
  val_df_data_flow_st_t obj_val_df_data_flow_st;
  memset(&obj_val_df_data_flow_st, 0, sizeof(val_df_data_flow_st_t));
  if (option1 == UNC_OPT1_NORMAL) {
    //  READ THE RESPONSE FROM DRIVER session as per section 11.9 in FD API doc
    pfc_log_debug("call sessReadDataflow fn from ctr_dataflow for read-normal");
    read_err_status |= df_segment->sessReadDataflow(*cli_session, resp_pos);
  } else if (option1 == UNC_OPT1_DETAIL) {
    //  READ THE RESPONSE FROM DRIVER session  as per section 11.9 in FD API doc
    read_err_status |= cli_session->getResponse(resp_pos,
                                                 obj_val_df_data_flow_st);
    pfc_log_debug("%s",
             (DataflowUtil::get_string(obj_val_df_data_flow_st).c_str()));
    resp_pos++;
    pfc_log_debug("call sessReadDataflow fn from ctr_dataflow for read-detail");
    read_err_status |= df_segment->sessReadDataflow(*cli_session, resp_pos);
  }
  int putresp_pos = 10;
  rsh.result_code = read_err_status;
  rsh.max_rep_count = max_rep_ct;
  if (read_err_status != UNC_RC_SUCCESS) {
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    if (err != 0) {
      delete df_segment;
      pfc_log_error("Failure in addOutput");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    delete df_segment;
    return UNC_RC_SUCCESS;
  }
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_CTR_DATAFLOW);
  err |= sess.addOutput(*obj_key_ctr_dataflow);
  if (err != 0) {
    delete df_segment;
    pfc_log_error("Failure in addOutput");
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  string str1 = DataflowCmn::get_string(*obj_key_ctr_dataflow);
  pfc_log_debug("%s", str1.c_str());
  if (option1 == UNC_OPT1_NORMAL) {
    //  WRITE THE RESPONSE TO NORTHBOUND session
    //  as per section 6.11 in FD API doc
    pfc_log_debug("call sessOutDataflow from ctr_dataflow for read-normal");
    DataflowCmn df_cmn(true, df_segment);
    err |= df_cmn.sessOutDataflow(sess, putresp_pos);
  } else if (option1 == UNC_OPT1_DETAIL) {
    //  WRITE THE RESPONSE TO NORTHBOUND session
    //  as per section 6.11 in FD API doc
    err |= sess.addOutput(obj_val_df_data_flow_st);
    string str = DataflowUtil::get_string(obj_val_df_data_flow_st);
    pfc_log_debug("%s", str.c_str());
    pfc_log_debug("call sessOutDataflow from ctr_dataflow for read-detail");
    putresp_pos = 11;
    DataflowCmn df_cmn(true, df_segment);
    err |= df_cmn.sessOutDataflow(sess, putresp_pos);
  }
  if (err != 0) {
    pfc_log_error("Failure in addOutput");
    delete df_segment;
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  delete df_segment;
  return UNC_RC_SUCCESS;
}

/** Fill_Attr_Syntax_Map
 * @Description : This function fills the attributes associated
 * with the class
 * @param[in] : none
 * @return    : none
 * */
void Kt_Ctr_Dataflow::Fill_Attr_Syntax_Map() {
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;

  Kt_Class_Attr_Syntax key_attrib1 =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[CTR_NAME_STR] = key_attrib1;

  Kt_Class_Attr_Syntax key_attrib2 =
  { PFC_IPCTYPE_UINT64, 1, 18446744073709551615ULL, 0, 0, true, "" };
  attr_syntax_map[CTR_DATAFLOW_FLOWID_STR] = key_attrib2;

  attr_syntax_map_all[UNC_KT_CTR_DATAFLOW] = attr_syntax_map;
}
