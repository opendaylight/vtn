/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT DataflowV2 implementation
 * @file    itc_kt_dataflow_v2.cc
 *
 */

#include "itc_kt_dataflow_v2.hh"
#include "ipc_client_configuration_handler.hh"
#include "itc_kt_controller.hh"
#include "odbcm_db_varbind.hh"
#include "ipct_util.hh"
#include "itc_kt_ctr_domain.hh"
#include "capa_module.hh"
using unc::uppl::PhysicalLayer;
using unc::uppl::ODBCMUtils;
using unc::uppl::ODBCMTableColumns;
using unc::uppl::IPCClientDriverHandler;

/** Constructor
 * @Description : This function initializes member variables
 * and fills the attribute syntax map used for validation
 * @param[in] : None
 * @return    : None
 * */
Kt_DataflowV2::Kt_DataflowV2() {
  // Populate structure to be used for syntax validation
  if (attr_syntax_map_all.find(UNC_KT_DATAFLOW_V2) ==
      attr_syntax_map_all.end()) {
    Fill_Attr_Syntax_Map();
  }
}

/** Destructor
 * @Description : This function frees the boundary map built
 * @param[in] : None
 * @return    : None
 * */
Kt_DataflowV2::~Kt_DataflowV2() {
  pfc_log_debug("Inside the destructor of Kt_DataflowV2");
}


/** PerformSyntaxValidation
 * @Description : This function performs syntax validation for
 *  UNC_KT_DATAFLOW_V2
 * @param[in]
 * key_struct - the key for the kt dataflow_v2 instance
 * value_struct - the value for the kt dataflow_v2 instance
 * data_type - UNC_DT_*,type of database
 * operation_type - UNC_OP*,type of operation
 * @param[out] : none
 * @return    : UNC_RC_SUCCESS is returned when the validation is successful
 * UNC_UPPL_RC_ERR_* is returned when validation is failure
 * */
UncRespCode Kt_DataflowV2::PerformSyntaxValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  pfc_log_trace("Entered into : %s", __func__);
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_DATAFLOW_V2];
  // Validate key structure
  key_dataflow_v2_t *key = reinterpret_cast<key_dataflow_v2_t*>(key_struct);
  string value = reinterpret_cast<char*>(key->controller_name);
  IS_VALID_STRING_KEY(CTR_NAME_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Error in controller_name attribute");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  if (val_struct != NULL) {
    val_dataflow_v2_t *val = reinterpret_cast<val_dataflow_v2_t*>(val_struct);
    uint64_t val_flowid = val->flow_id;
    VALIDATE_MANDATORY_FIELD(CTR_DATAFLOW_FLOWID_STR, mandatory);
    if (mandatory == PFC_TRUE && val->valid[0] != UNC_VF_VALID) {
      pfc_log_debug("Mandatory attribute flow_id is not available in"
                    "READ operation");
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    VALIDATE_INT_FIELD(CTR_DATAFLOW_FLOWID_STR, val_flowid, ret_code);
    if (ret_code != UNC_RC_SUCCESS) {
      pfc_log_debug("Error in flow_id attribute :%"PFC_PFMT_u64, val_flowid);
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    pfc_log_error("Value struct is mandatory:%d", UNC_UPPL_RC_ERR_CFG_SYNTAX);
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  return UNC_RC_SUCCESS;
}

/** PerformSemanticValidation
 * @Description : This function performs semantic validation
 * for UNC_KT_DATAFLOW_V2
 * @param[in] : key_struct - specifies key instance of KT_DataflowV2
 * value_struct - specifies value of KT_DATAFLOW_V2,value of unc_key_type_t
 * operation - UNC_OP*,type of operation
 * data_type - UNC_DT*,type of database
 * @return    : UNC_RC_SUCCESS if semantic valition is successful
 * or UNC_UPPL_RC_ERR_* if failed
 * */
UncRespCode Kt_DataflowV2::PerformSemanticValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  pfc_log_trace("Entered into : %s", __func__);

  // Not allowing the STANDBY request
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  if (physical_layer->get_physical_core()->get_system_state() == \
       UPPL_SYSTEM_ST_STANDBY) {
    pfc_log_error("System is in Standby mode");
    return UNC_UPPL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
  }

  UncRespCode ret_code = UNC_RC_SUCCESS;
  uint32_t nums = 0;
  const uint8_t *attrs = 0;

  key_dataflow_v2_t *obj_key_dataflow =
                reinterpret_cast<key_dataflow_v2_t*>(key_struct);
  key_ctr_t obj_key_ctr;
  memset(&obj_key_ctr, '\0', sizeof(key_ctr_t));
  memcpy(obj_key_ctr.controller_name, obj_key_dataflow->controller_name,
         sizeof(obj_key_dataflow->controller_name));
  void *key_str = reinterpret_cast <void *>(&obj_key_ctr);

  unc::capa::CapaModule *capa = reinterpret_cast<unc::capa::CapaModule *>(
    pfc::core::Module::getInstance("capa"));
  if (capa == NULL) {
    UPPL_LOG_FATAL("CapaModule is not found");
    return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
  }
  Kt_Controller KtObj;
  vector<void *> vect_key_ctr;
  vect_key_ctr.push_back(key_str);
  vector<void *> vect_val_ctr;
  // vector which will holds the boundary values
  ret_code = KtObj.ReadInternal(db_conn, vect_key_ctr,
                                      vect_val_ctr,
                                      UNC_DT_RUNNING,
                                      UNC_OP_READ);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Read of Controller failed:%d", ret_code);
    return ret_code;
  }

  key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(vect_key_ctr[0]);
  val_ctr_st_t *val_ctr = reinterpret_cast <val_ctr_st_t*>
                                           (vect_val_ctr[0]);
  if (val_ctr == NULL || ctr_key == NULL) {
    if (ctr_key != NULL) {  // if ctr_key is NULL, delete key
      delete ctr_key;
      ctr_key = NULL;
    }
    if (val_ctr != NULL) {  // if val_ctr is NULL, delete val
     delete val_ctr;
     val_ctr = NULL;
    }
    return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
  }

  unc_keytype_ctrtype_t controller_type =
                           (unc_keytype_ctrtype_t)val_ctr->controller.type;
  uint8_t oper_status_db = val_ctr->oper_status;
  string version = (const char*)val_ctr->controller.version;

  pfc_log_debug("controller_type=%d oper_status_db=%d config version=%s",
                 (uint16_t)controller_type, oper_status_db, version.c_str());

  if (controller_type != UNC_CT_PFC &&
      controller_type != UNC_CT_ODC) {
    ret_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_error("DFV2 Read is allowed only on PFC and ODC controller:%d",
                   ret_code);
  } else if (oper_status_db != UPPL_CONTROLLER_OPER_UP) {
    ret_code = UNC_UPPL_RC_ERR_CTRLR_DISCONNECTED;
    pfc_log_error("DFV2 Readis allowed only in Up Ctrl");
  }
  if (ret_code != UNC_RC_SUCCESS) {
    if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
    }
    if (val_ctr != NULL) {
     delete val_ctr;
     val_ctr = NULL;
    }
    return ret_code;
  }
  // Call CapaModule and check for actual version

  bool ret_actual = capa->GetReadCapability(controller_type,
                       version,
                       UNC_KT_DATAFLOW_V2,
                       &nums,
                       &attrs);
  if (ret_actual != true) {
    ret_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_info("KEY TYPE is NOT supported for version : %s"
                 " and ret_code: %d", version.c_str(), ret_code);
  }
  if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
  }
  if (val_ctr != NULL) {
      delete val_ctr;
      val_ctr = NULL;
  }
  return ret_code;
}


/** PerformRead
 * @Description : This function reads the instance of KT_DataflowV2 based on
 *                  operation type - READ
 * @param[in] : session_id-ipc session id used for TC validation
 * configuration_id-ipc configuration id used for TC validation
 * key_struct-void* to dataflow_v2 key structure
 * value_struct-void* to dataflow_v2 value structure
 * data_type-UNC_DT_*,type of database
 * operation _type-UNC_OP_*,type of database
 * sess- object of ServerSession
 * option1,option2-UNC_OPT1/OPT2_*,additional infor read operations
 * max_rep_ct-max. no of records to be read
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*,
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_DataflowV2::PerformRead(OdbcmConnectionHandler *db_conn,
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
  pfc_log_debug("PerformRead:oper=%d,dt=%d",
               operation_type, data_type);
  if (operation_type == UNC_OP_READ) {
    max_rep_ct = 1;
  }
  key_dataflow_v2_t *obj_key_dataflow =
             reinterpret_cast<key_dataflow_v2_t*>(key_struct);
  val_dataflow_v2_t *obj_val_dataflow =
             reinterpret_cast<val_dataflow_v2_t*>(val_struct);
  UncRespCode resp_code = UNC_RC_SUCCESS;
  if (option1 != UNC_OPT1_NORMAL) {
    resp_code = UNC_UPPL_RC_ERR_INVALID_OPTION1;
    pfc_log_error("Invalid option1 specified for read operation:%d",
                   resp_code);
  }
  if (resp_code == UNC_RC_SUCCESS && option2 != UNC_OPT2_NONE) {
    resp_code = UNC_UPPL_RC_ERR_INVALID_OPTION2;
    pfc_log_error("Invalid option2 specified for read operation:%d",
                     resp_code);
  }
  if (resp_code == UNC_RC_SUCCESS && data_type != UNC_DT_STATE) {
    resp_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_error("Data type other than DT-STATE not allowed: %d",
                       resp_code);
  }
  if (resp_code != UNC_RC_SUCCESS) {
    physical_response_header rsh = {session_id,
        configuration_id,
        operation_type,
        max_rep_ct,
        option1,
        option2,
        data_type,
        static_cast<uint32_t>(resp_code)};
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_DATAFLOW_V2);
    err |= sess.addOutput(*obj_key_dataflow);
    err |= sess.addOutput(*obj_val_dataflow);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header:%d",
                    UNC_UPPL_RC_ERR_IPC_WRITE_ERROR);
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }
  pfc_timespec_t extd_tout;
  extd_tout.tv_sec = PFC_DEF_TIMEOUT;
  extd_tout.tv_nsec = 0;
  sess.setTimeout(&extd_tout);
  pfc_log_trace("Extending the timeout to 3600 seconds");
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
           get_physical_core();
  max_dataflow_traverse_count_ = physical_core->uppl_max_dataflowtraversal_;
  pfc_log_debug("max_dataflow_traverse_count_ (from conf file) = %d",
                                                max_dataflow_traverse_count_);
  key_dataflow_v2_t key_copy;
  memcpy(&key_copy, key_struct, sizeof(key_dataflow_v2_t));
  string empty = "";
  DataflowDetail* df_segm = NULL;
  UncRespCode ret_code = RequestSingleFlow(db_conn, session_id,
                                     configuration_id,
                                     key_struct, val_struct, sess,
                                     true, empty, df_segm);
  physical_response_header rsh = {session_id,
        configuration_id,
        operation_type,
        max_rep_ct,
        option1,
        option2,
        data_type,
        (uint32_t)ret_code};
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_DATAFLOW_V2);
  err |= sess.addOutput(key_copy);
  err |= sess.addOutput(*obj_val_dataflow);
  if (ret_code != UNC_RC_SUCCESS) {
    if (df_segm != NULL) {
      delete df_segm;
      df_segm = NULL;
    }
    pfc_log_error("Querying 1st PFC failed:%d", ret_code);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header:%d",
          UNC_UPPL_RC_ERR_IPC_WRITE_ERROR);
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  } else {
    err = df_util_.sessOutDataflows(sess);
    if (df_segm != NULL) {
      delete df_segm;
      df_segm = NULL;
    }
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header:%d",
                    UNC_UPPL_RC_ERR_IPC_WRITE_ERROR);
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  pfc_log_trace("Exiting PerformRead");
  return UNC_RC_SUCCESS;
}

/** RequestSingleFlow
 * @Description : This function recieves the dataflows from driver and checks
 * for boundary egress port and traverse to the neighbour.
 * @param[in] : session_id-ipc session id used for TC validation
 * configuration_id-ipc configuration id used for TC validation
 * key_struct-void* to ctr key structure
 * sess- object of ServerSession
 * is_head_node - flag to indicate whether head node
 * parentnode - reference of the parent node
 * lastPfcNode - reference of the last PFC node
 * @return    : UNC_RC_SUCCESS or UNC_UPPL_RC_ERR*,
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_DataflowV2::RequestSingleFlow(OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct,
                                          void* val_struct,
                                          ServerSession &sess,
                                          bool is_head_node,
                                          string &ingress_bdry_id,
                                          DataflowDetail*& df_segm) {
  pfc_log_trace("Entered into : %s", __func__);
  key_dataflow_v2_t *obj_key_dataflow =
                       reinterpret_cast<key_dataflow_v2_t*>(key_struct);
  val_dataflow_v2_t *obj_val_dataflow =
                       reinterpret_cast<val_dataflow_v2_t*>(val_struct);
  string controller_name =
                    reinterpret_cast<char*>(obj_key_dataflow->controller_name);
  string domain_id = "";
  UncRespCode err_code = PerformSemanticValidation(db_conn, key_struct, NULL,
                                                  UNC_OP_READ, UNC_DT_RUNNING);
  if (err_code != UNC_RC_SUCCESS) {
    pfc_log_info("PerformSemanticValidation failed with error %d", err_code);
    return err_code;
  }
  driver_request_header rqh = {uint32_t(0), uint32_t(0), controller_name,
        domain_id, static_cast<uint32_t>(UNC_OP_READ), uint32_t(0),
        (uint32_t)0, (uint32_t)0, static_cast<uint32_t>(UNC_DT_STATE),
        static_cast<uint32_t>(UNC_KT_CTR_DATAFLOW)};
  ClientSession *cli_session = NULL;
  int err1 = 0;
  UncRespCode driver_response = UNC_RC_SUCCESS;
  unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
  driver_response = PhyUtil::get_controller_type(db_conn, controller_name,
                                          ctr_type, UNC_DT_RUNNING);
  if (driver_response != UNC_RC_SUCCESS) {
    pfc_log_debug("Failed to get controller type with response code :%d",
                   driver_response);
    return driver_response;
  }
  IPCClientDriverHandler drv_handler(ctr_type, driver_response);
  cli_session = drv_handler.ResetAndGetSession();
  if (driver_response != UNC_RC_SUCCESS) {
    pfc_log_debug("Creation of session to Driver failed:%d", driver_response);
    return driver_response;
  }
  err1 = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
  key_ctr_dataflow_t obj_key_ctr_df;
  memset(&obj_key_ctr_df, '\0', sizeof(key_ctr_dataflow_t));
  memcpy(obj_key_ctr_df.ctr_key.controller_name,
          obj_key_dataflow->controller_name,
          sizeof(obj_key_dataflow->controller_name));
  obj_key_ctr_df.flow_id = obj_val_dataflow->flow_id;
  err1 |= cli_session->addOutput(obj_key_ctr_df);
  pfc_log_debug("Key to Driver:%s",
              DataflowCmn::get_string(obj_key_ctr_df).c_str());
  if (err1 != 0) {
    pfc_log_error("addOutput failed for driver_request_header:%d",
                   UNC_UPPL_RC_ERR_IPC_WRITE_ERROR);
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  pfc_timespec_t extd_tout;
  extd_tout.tv_sec = PFC_DEF_TIMEOUT;
  extd_tout.tv_nsec = 0;
  cli_session->setTimeout(&extd_tout);
  pfc_log_trace("Extending the timeout to 3600 seconds");
  // Send the request to driver
  driver_response_header rsp;
  driver_response = drv_handler.SendReqAndGetResp(rsp);
  if (driver_response != UNC_RC_SUCCESS) {
      pfc_log_error("Read request to Driver failed for controller %s"
      " with response %d, err1=%d", controller_name.c_str(),
      driver_response, err1);
    return driver_response;
  }
  key_ctr_dataflow_t key_ctr_df;
  uint32_t keytype;
  uint32_t resp_pos = 10;
  err1 = cli_session->getResponse(resp_pos++, keytype);
  if (err1 != 0) {
    pfc_log_error("Read keytype error for position resp_pos=%d", resp_pos);
    return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
  }
  err1 = cli_session->getResponse(resp_pos++, key_ctr_df);
  if (err1 != 0) {
    pfc_log_error("Read key structure error for position resp_pos=%d",
                   resp_pos);
    return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
  }
  df_segm = new DataflowDetail(kidx_val_df_data_flow_cmn, ctr_type);
  if (df_segm == NULL) {
    pfc_log_debug("Memory not allocated for df_segm");
    return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  err1 = df_segm->sessReadDataflow(*cli_session, resp_pos);
  if (err1 != 0) {
    delete df_segm;
    df_segm = NULL;
    pfc_log_error("Read dataflow from session error "
                           "for position resp_pos=%d", resp_pos);
    return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
  }

  std::string ctr_name((const char*)obj_key_dataflow->controller_name);
  pfc_log_debug("ctr_name passed to fill_ctrlr_dom_count_map-"
                " RequestSingleFlow:%s", ctr_name.c_str());
  UncRespCode fill_status = fill_ctrlr_dom_count_map(db_conn,
                  ctr_name);
  if (fill_status != UNC_RC_SUCCESS) {
    pfc_log_debug("Map is not filled :%d", fill_status);
    return fill_status;
  }
  DataflowCmn *df_cmn = NULL;
  df_cmn = new DataflowCmn(is_head_node, df_segm);
  pfc_log_trace("before calling df_util_ appendFlow");
  df_cmn->apply_action();
  uint32_t ret = df_util_.appendFlow(df_cmn);
  if (ret != 0) {
    pfc_log_info("df_util_ appendFlow error=%d", ret);
    if (df_cmn != NULL) {
      delete df_cmn;
      df_cmn = NULL;
    }
    return UNC_UPPL_RC_FAILURE;
  }
  vector<DataflowCmn* >* CtrlrFlows;
  CtrlrFlows = df_util_.get_firstCtrlrFlows();
  if (CtrlrFlows->size() == 0) {
    pfc_log_info("firstCtrlrFlows- vector is empty");
    return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  }
  key_dataflow_t *key = new key_dataflow_t;
  memset(key, '\0', sizeof(key_dataflow_t));
  memcpy(key->controller_name, df_segm->df_common->controller_name,
            sizeof(key->controller_name));
  memcpy(key->switch_id, df_segm->df_common->egress_switch_id,
            sizeof(key->switch_id));
  memcpy(key->port_id, df_segm->df_common->out_port,
            sizeof(key->port_id));
  map <UncDataflowFlowMatchType, void *>::iterator output_matches_iter;
  output_matches_iter = df_cmn->output_matches.find(UNC_MATCH_VLAN_ID);
  if (output_matches_iter != df_cmn->output_matches.end()) {
    val_df_flow_match_vlan_id_t *prev =
       reinterpret_cast<val_df_flow_match_vlan_id_t *>
       ((*output_matches_iter).second);
    key->vlan_id = prev->vlan_id;
  }
  output_matches_iter = df_cmn->output_matches.find(UNC_MATCH_DL_SRC);
  if (output_matches_iter != df_cmn->output_matches.end()) {
    val_df_flow_match_dl_addr_t *prev =
       reinterpret_cast<val_df_flow_match_dl_addr_t *>
       ((*output_matches_iter).second);
    memcpy(key->src_mac_address, prev->dl_addr,
                          sizeof(key->src_mac_address));
  }
  void *key_struct2 = reinterpret_cast< void* >(key);
  err_code = checkFlowLimitAndTraverse(db_conn, session_id, configuration_id,
                              sess, key_struct2, CtrlrFlows,
                              is_head_node, ingress_bdry_id);
  delete key;
  key = NULL;
  return err_code;
}

/** Fill_Attr_Syntax_Map
 * @Description : This function populates the values to be used for attribute
 * validation
 * @param[in] : None
 * @return    : void
 * */
void Kt_DataflowV2::Fill_Attr_Syntax_Map() {
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;
  Kt_Class_Attr_Syntax objKeyCtrNameAttrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 31, true, ""};
  attr_syntax_map[CTR_NAME_STR] = objKeyCtrNameAttrSyntax;
  Kt_Class_Attr_Syntax objKeyFlowIdAttrSyntax =
  { PFC_IPCTYPE_UINT64, 1, 18446744073709551615ULL, 0, 0, true, ""};
  attr_syntax_map[CTR_DATAFLOW_FLOWID_STR] = objKeyFlowIdAttrSyntax;
  attr_syntax_map_all[UNC_KT_DATAFLOW_V2] = attr_syntax_map;
}


