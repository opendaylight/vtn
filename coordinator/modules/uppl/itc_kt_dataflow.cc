/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Dataflow implementation
 * @file    itc_kt_dataflow.cc
 *
 */

#include "itc_kt_dataflow.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_logical_member_port.hh"
#include "itc_kt_root.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_port.hh"
#include "itc_kt_boundary.hh"
#include "odbcm_db_varbind.hh"
#include "ipct_util.hh"
#include "itc_read_request.hh"
#include "ipc_client_configuration_handler.hh"
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
Kt_Dataflow::Kt_Dataflow() {
  // Populate structure to be used for syntax validation
  if (attr_syntax_map_all.find(UNC_KT_DATAFLOW) ==
      attr_syntax_map_all.end()) {
    Fill_Attr_Syntax_Map();
  }
  max_dataflow_traverse_count_ = 0;
}

/** Destructor
 * @Description : This function frees the boundary map built
 * @param[in] : None
 * @return    : None
 * */
Kt_Dataflow::~Kt_Dataflow() {
  pfc_log_debug("Inside the destructor of Kt_Dataflow");
  multimap<string, boundary_val> *bdry_map = get_boundary_map();
  bdry_map->clear();
  map <string, uint32_t>* is_validated_map = get_is_validated_map();
  is_validated_map->clear();
}


/**ReadBulk
 * @Description : This function is not allowed for Dataflow.
 * @param[in] : db_conn - OdbcmConnectionHandler 
 * key_struct - key for dataflow
 * data_type - UNC_DT_*
 * max_rep_ct - max repetition count
 * child_index - index of children KTs
 * parent_call - flag to indicate whether parent Kt called
 * is_read_next - flag to indicate whether called from ReadNext function
 * read_req - ReadRequest pointer
 * @param[out] : none
 * @return    : UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED is returned the response
 * */
UncRespCode Kt_Dataflow::ReadBulk(OdbcmConnectionHandler *db_conn,
                                 void* key_struct,
                                 uint32_t data_type,
                                 uint32_t &max_rep_ct,
                                 int child_index,
                                 pfc_bool_t parent_call,
                                 pfc_bool_t is_read_next,
                                 ReadRequest *read_req) {
  pfc_log_debug("ReadBulk operation is not allowed in KT_DATAFLOW");
  return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
}


/** PerformSyntaxValidation
 * @Description : This function performs syntax validation for
 *  UNC_KT_DATAFLOW
 * @param[in]
 * key_struct - the key for the kt dataflow instance
 * value_struct - the value for the kt dataflow instance
 * data_type - UNC_DT_*,type of database
 * operation_type - UNC_OP*,type of operation
 * @param[out] : none
 * @return    : UNC_RC_SUCCESS is returned when the validation is successful
 * UNC_UPPL_RC_ERR_* is returned when validation is failure
 * */
UncRespCode Kt_Dataflow::PerformSyntaxValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  pfc_log_trace("Entered into : %s", __func__);
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_DATAFLOW];
  // Validate key structure
  key_dataflow_t *key = reinterpret_cast<key_dataflow_t*>(key_struct);
  string value = reinterpret_cast<char*>(key->controller_name);
  IS_VALID_STRING_KEY(CTR_NAME_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Error in controller_name attribute");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  value = reinterpret_cast<char*>(key->switch_id);
  IS_VALID_STRING_KEY(SWITCH_ID_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Error in switch_id attribute");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->port_id);
  IS_VALID_STRING_KEY(PORT_ID_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Error in port_id attribute");
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  uint16_t int_value = key->vlan_id;
  IS_VALID_VLAN_ID(VLAN_ID_STR, int_value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    if (key->vlan_id != 0xFFFF) {
      pfc_log_debug("Error in vlan_id attribute");
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UNC_RC_SUCCESS;
}

/** PerformSemanticValidation
 * @Description : This function performs semantic validation
 * for UNC_KT_DATAFLOW
 * @param[in] : key_struct - specifies key instance of KT_Dataflow
 * value_struct - specifies value of KT_DATAFLOW,value of unc_key_type_t
 * operation - UNC_OP*,type of operation
 * data_type - UNC_DT*,type of database
 * @return    : UNC_RC_SUCCESS if semantic valition is successful
 * or UNC_UPPL_RC_ERR_* if failed
 * */
UncRespCode Kt_Dataflow::PerformSemanticValidation(
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

  key_dataflow_t *obj_key_dataflow =
                reinterpret_cast<key_dataflow_t*>(key_struct);
  key_ctr_t obj_key_ctr;
  memset(&obj_key_ctr, '\0', sizeof(key_ctr_t));
  memcpy(obj_key_ctr.controller_name, obj_key_dataflow->controller_name,
         sizeof(obj_key_dataflow->controller_name));
  void *key_str = reinterpret_cast <void *>(&obj_key_ctr);

  unc::capa::CapaModule *capa = reinterpret_cast<unc::capa::CapaModule *>(
    pfc::core::Module::getInstance("capa"));
  if (capa == NULL) {
    UPPL_LOG_FATAL("CapaModule is not found:%s", __FUNCTION__);
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
    pfc_log_debug("Read Ctr info failed :%d", ret_code);
    return ret_code;
  }
  key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(vect_key_ctr[0]);
  val_ctr_st_t *val_ctr_vect = reinterpret_cast <val_ctr_st_t*>
                                           (vect_val_ctr[0]);
  if (val_ctr_vect == NULL || ctr_key == NULL) {
    if (ctr_key != NULL) {
      delete ctr_key;
      ctr_key = NULL;
    }
    return UNC_UPPL_RC_ERR_CFG_SEMANTIC;
  }

  unc_keytype_ctrtype_t controller_type =
                           (unc_keytype_ctrtype_t)val_ctr_vect->controller.type;
  uint8_t oper_status_db = val_ctr_vect->oper_status;
  string version = (const char*)val_ctr_vect->controller.version;

  pfc_log_debug("controller_type=%d, oper_status_db=%d, config version=%s",
      (uint16_t)controller_type, oper_status_db, version.c_str());

  if (controller_type != UNC_CT_PFC &&
      controller_type != UNC_CT_ODC) {
    ret_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    pfc_log_error("DF Read is allowed only for PFC or ODC controller"
                  " :%d", ret_code);
  } else if (oper_status_db != UPPL_CONTROLLER_OPER_UP) {
    ret_code = UNC_UPPL_RC_ERR_CTRLR_DISCONNECTED;
    pfc_log_error("DF Read is allowed only for UP Ctrs :%d", ret_code);
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

  bool ret_actual = capa->GetReadCapability(controller_type,
                       version,
                       UNC_KT_DATAFLOW,
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
  if (val_ctr_vect != NULL) {
      delete val_ctr_vect;
      val_ctr_vect = NULL;
  }

  return ret_code;
}

/** getDomainType
 * @Description : This function gets the type of given domain
 * @param[in]   : key_struct - specifies key instance of CtrDomain
 *                value_struct - Ignored
 *                data_type - UNC_DT*,type of database
 * @param[out]  :  domain_type - type of domain
 * @return      :  UNC_RC_SUCCESS if the operation is successful
 *                or UNC_UPPL_RC_ERR_* if failed
 * */
UncRespCode Kt_Dataflow::getDomainType(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    UpplDomainType& domain_type) {
  pfc_log_trace("Entered into : %s", __func__);
  UncRespCode ret_code = UNC_RC_SUCCESS;
  key_ctr_domain_t *obj_key_domain =
        reinterpret_cast <key_ctr_domain_t*>(key_struct);
  // Read the ctr_domain_table
  Kt_Ctr_Domain Ctr_Domain_Obj;
  vector<void *> vect_key_domain;
  vect_key_domain.push_back(obj_key_domain);
  vector<void *> vect_val_domain;
  ret_code = Ctr_Domain_Obj.ReadInternal(db_conn, vect_key_domain,
                                      vect_val_domain,
                                      UNC_DT_RUNNING,
                                      UNC_OP_READ);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Read ctr domain info failed :%d", ret_code);
    return ret_code;
  }
  key_ctr_domain_t *domain_key =
    reinterpret_cast<key_ctr_domain_t*>(vect_key_domain[0]);
  val_ctr_domain_st_t *val_domain_vect = reinterpret_cast <val_ctr_domain_st_t*>
                                           (vect_val_domain[0]);
  // release the memory
  if (val_domain_vect == NULL || domain_key == NULL) {
    if (domain_key != NULL) {
      pfc_log_debug("Delete the domain key values");
      delete domain_key;
      domain_key = NULL;
    }
    if (val_domain_vect != NULL) {
     delete val_domain_vect;
     val_domain_vect = NULL;
    }
    pfc_log_debug("no ctr_domain key/val struct found");
    return UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
  }

  domain_type = (UpplDomainType)val_domain_vect->domain.type;
  if (domain_key != NULL) {
    delete domain_key;
    domain_key = NULL;
  }
  if (val_domain_vect != NULL) {
    delete val_domain_vect;
    val_domain_vect = NULL;
  }
  return ret_code;
}

/** PerformRead
 * @Description : This function reads the instance of KT_Dataflow based on
 *                  operation type - READ
 * @param[in] : session_id-ipc session id used for TC validation
 * configuration_id-ipc configuration id used for TC validation
 * key_struct-void* to ctr key structure
 * value_struct-void* to ctr value structure
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
UncRespCode Kt_Dataflow::PerformRead(OdbcmConnectionHandler *db_conn,
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
  key_dataflow_t *obj_key_dataflow =
             reinterpret_cast<key_dataflow_t*>(key_struct);
  UncRespCode resp_code = UNC_RC_SUCCESS;
  if (option1 != UNC_OPT1_NORMAL) {
    resp_code = UNC_UPPL_RC_ERR_INVALID_OPTION1;
    pfc_log_error("Invalid option1 specified for read operation:%d",
                   resp_code);
  }
  if (resp_code == UNC_RC_SUCCESS && option2 != UNC_OPT2_NONE &&
      option2 != UNC_OPT2_NO_TRAVERSING) {
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
    err |= sess.addOutput((uint32_t)UNC_KT_DATAFLOW);
    err |= sess.addOutput(*obj_key_dataflow);
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
  //  store the max_dataflow_traverse_count value which is gotton from uppl.conf
  PhysicalCore *physical_core = PhysicalLayer::get_instance()->
           get_physical_core();
  max_dataflow_traverse_count_ = physical_core->uppl_max_dataflowtraversal_;
  pfc_log_debug("max_dataflow_traverse_count_ (from conf file) = %d",
                                                max_dataflow_traverse_count_);
  key_dataflow_t key_copy;
  memcpy(&key_copy, key_struct, sizeof(key_dataflow_t));
  string empty = "";
  UncRespCode ret_code = UNC_RC_SUCCESS;
  // domain type validation (if leaf/spine reject the request)
  key_switch_t obj_key_switch;
  memset(&obj_key_switch, '\0', sizeof(key_switch_t));
  memcpy(obj_key_switch.ctr_key.controller_name,
         obj_key_dataflow->controller_name,
         sizeof(obj_key_dataflow->controller_name));
  memcpy(obj_key_switch.switch_id, obj_key_dataflow->switch_id,
         sizeof(obj_key_dataflow->switch_id));
  void *key_str_switch = reinterpret_cast <void *>(&obj_key_switch);
  // Read the switch_table
  Kt_Switch Switch_Obj;
  vector<void *> vect_key_switch;
  vect_key_switch.push_back(key_str_switch);
  vector<void *> vect_val_switch;
  ret_code = Switch_Obj.ReadInternal(db_conn, vect_key_switch,
                                      vect_val_switch,
                                      UNC_DT_RUNNING,
                                      UNC_OP_READ);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Read switch info failed :%d", ret_code);
  } else {
    key_switch_t *switch_key =
               reinterpret_cast<key_switch_t*>(vect_key_switch[0]);
    val_switch_st_t *switch_value = reinterpret_cast <val_switch_st_t*>
                                           (vect_val_switch[0]);
    if (switch_value == NULL || switch_key == NULL) {
      if (switch_key != NULL) {
        pfc_log_debug("Delete the swith key values");
        delete switch_key;
        switch_key = NULL;
      }
      if (switch_value != NULL) {
        delete switch_value;
        switch_value = NULL;
      }
      pfc_log_debug("Return the semantic error");
      return UNC_UPPL_RC_ERR_DB_GET;
    }
    key_ctr_domain_t obj_key_domain;
    memset(&obj_key_domain, '\0', sizeof(key_ctr_domain_t));
    memcpy(obj_key_domain.ctr_key.controller_name,
                 obj_key_switch.ctr_key.controller_name,
                 sizeof(obj_key_switch.ctr_key.controller_name));
    // copy the domain_name from switch value structure to
    // ctr_domain key structure
    memcpy(obj_key_domain.domain_name, switch_value->switch_val.domain_name,
    sizeof(switch_value->switch_val.domain_name));
    delete switch_key;
    switch_key = NULL;
    delete switch_value;
    switch_value = NULL;
    // get the domain type from the domain_table
    UpplDomainType dom_type = UPPL_DOMAIN_TYPE_DEFAULT;
    ret_code = getDomainType(db_conn, &obj_key_domain, NULL,
                                     UNC_DT_RUNNING, dom_type);
    if (ret_code != UNC_RC_SUCCESS) {
      pfc_log_debug("Read domain type info failed :%d", ret_code);
    }
    //  else - dom_type default is UPPL_DOMAIN_TYPE_DEFAULT, so next if skips
    if (dom_type == UPPL_DOMAIN_TYPE_PF_LEAF ||
             dom_type == UPPL_DOMAIN_TYPE_PF_SPINE) {
      physical_response_header rsh = {session_id,
               configuration_id,
               operation_type,
               max_rep_ct,
               option1,
               option2,
               data_type,
               static_cast<uint32_t>(UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED)};
      int err = PhyUtil::sessOutRespHeader(sess, rsh);
      err |= sess.addOutput((uint32_t)UNC_KT_DATAFLOW);
      err |= sess.addOutput(*obj_key_dataflow);
      if (err != 0) {
        pfc_log_info("addOutput failed for physical_response_header:%d",
                    UNC_UPPL_RC_ERR_IPC_WRITE_ERROR);
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      pfc_log_debug("Operation not allowed for leaf/spine domain");
      return UNC_RC_SUCCESS;
    }
  }
  ret_code = traversePFC(db_conn, session_id, configuration_id,
                                     key_struct, sess, true, NULL, NULL, empty,
                                     option2);
  physical_response_header rsh = {session_id,
        configuration_id,
        operation_type,
        max_rep_ct,
        option1,
        option2,
        data_type,
        (uint32_t)ret_code};
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t)UNC_KT_DATAFLOW);
  err |= sess.addOutput(key_copy);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_error("Querying 1st PFC failed:%d", ret_code);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header:%d",
          UNC_UPPL_RC_ERR_IPC_WRITE_ERROR);
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  } else {
    err = df_util_.sessOutDataflows(sess);
    if (err != 0) {
      pfc_log_error("addOutput failed for physical_response_header:%d",
                    UNC_UPPL_RC_ERR_IPC_WRITE_ERROR);
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
  }
  pfc_log_trace("Exiting PerformRead");
  return UNC_RC_SUCCESS;
}

/** traversePFC
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
UncRespCode Kt_Dataflow::traversePFC(OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct,
                                          ServerSession &sess,
                                          bool is_head_node,
                                          DataflowCmn *parentnode,
                                          DataflowCmn *lastPfcNode,
                                          string &ingress_bdry_id,
                                          uint32_t option2) {
  pfc_log_trace("Entered into : %s", __func__);
  key_dataflow_t *obj_key_dataflow =
                       reinterpret_cast<key_dataflow_t*>(key_struct);
  string controller_name =
                    reinterpret_cast<char*>(obj_key_dataflow->controller_name);
  string domain_id = "";
  map <string, uint32_t>* is_validated_map = get_is_validated_map();
  map <string, uint32_t>:: iterator valdt_map_iter;
  valdt_map_iter = is_validated_map->find(controller_name);
  if (valdt_map_iter == is_validated_map->end()) {
    UncRespCode err_code = PerformSemanticValidation(db_conn, key_struct, NULL,
                                                  UNC_OP_READ, UNC_DT_RUNNING);
    is_validated_map->insert(std::pair<string, uint32_t>
                                                  (controller_name, err_code));
    if (err_code != UNC_RC_SUCCESS) {
      pfc_log_info("PerformSemanticValidation failed with error %d", err_code);
      return err_code;
    }
  } else {
    UncRespCode err_code = (UncRespCode)valdt_map_iter->second;
    if (err_code != UNC_RC_SUCCESS) {
      pfc_log_debug("PerformSemanticValidation failed with error %d", err_code);
      return err_code;
    }
  }
  map <UncDataflowFlowMatchType, void *>::iterator output_matches_iter;
  // skip vlan filtering for vnp and polc
  if (!is_head_node) {
    if (parentnode->df_segment->df_common->controller_type != UNC_CT_VNP &&
      parentnode->df_segment->df_common->controller_type != UNC_CT_POLC) {
      // retrieve the VLAN ID from out_matches.
      output_matches_iter = lastPfcNode->output_matches.find(UNC_MATCH_VLAN_ID);
      if (output_matches_iter != lastPfcNode->output_matches.end()) {
        val_df_flow_match_vlan_id_t *prev =
reinterpret_cast<val_df_flow_match_vlan_id_t *>((*output_matches_iter).second);
        obj_key_dataflow->vlan_id =  prev->vlan_id;
      }
    } else {
      pfc_log_info("vlan id skipped for VNP and POLC");
    }
  }
  // retrieve the src MAC address from out_matches
  if (!is_head_node) {
    output_matches_iter = lastPfcNode->output_matches.find(UNC_MATCH_DL_SRC);
    if (output_matches_iter != lastPfcNode->output_matches.end()) {
      val_df_flow_match_dl_addr_t *prev =
reinterpret_cast<val_df_flow_match_dl_addr_t *>((*output_matches_iter).second);
      memcpy(obj_key_dataflow->src_mac_address, prev->dl_addr,
                           sizeof(obj_key_dataflow->src_mac_address));
    }
  }

  vector<DataflowDetail*> pfc_flows;
  pfc_log_debug("Searching pfc_flows from map size %" PFC_PFMT_SIZE_T
               "for key %s", df_util_.pfc_flows.size(),
               DataflowCmn::get_string(*obj_key_dataflow).c_str());
  map<key_dataflow_t, vector<DataflowDetail*> > ::iterator iter;
  for (iter =df_util_.pfc_flows.begin();
      iter != df_util_.pfc_flows.end(); iter++) {
    if (DataflowCmn::Compare(iter->first, *obj_key_dataflow)) {
      break;
    }
  }
  if (iter == df_util_.pfc_flows.end()) {
    pfc_log_info("Searching pfc_flows from map not found");

    driver_request_header rqh = {uint32_t(0), uint32_t(0), controller_name,
        domain_id, static_cast<uint32_t>(UNC_OP_READ), uint32_t(0),
        (uint32_t)0, (uint32_t)0, static_cast<uint32_t>(UNC_DT_STATE),
        static_cast<uint32_t>(UNC_KT_DATAFLOW)};
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
      pfc_log_error("Creation of session to driver failed: %d",
                     driver_response);
      return driver_response;
    }
    err1 = PhyUtil::sessOutDriverReqHeader(*cli_session, rqh);
    err1 |= cli_session->addOutput(*obj_key_dataflow);
    pfc_log_debug("Key to Driver:%s",
                DataflowCmn::get_string(*obj_key_dataflow).c_str());
    if (err1 != 0) {
      pfc_log_error("addOutput failed for driver_request_header");
      if (is_head_node) {
        pfc_log_info("Returning IPC_WRITE_ERROR");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      } else {
        if ((parentnode->next.size() == 0) &&
           (parentnode->addl_data->reason == UNC_DF_RES_SUCCESS))
          parentnode->addl_data->reason = UNC_DF_RES_SYSTEM_ERROR;
        return UNC_RC_SUCCESS;
      }
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
      if (is_head_node) {
        pfc_log_error("Read request to Driver failed for controller %s"
        " with response %d, err1=%d", controller_name.c_str(),
        driver_response, err1);
      } else {
        pfc_log_error("Read request to Driver failed for controller %s"
        " ,flow_id %" PFC_PFMT_u64 " with response %d, err1=%d",
        controller_name.c_str(), parentnode->df_segment->df_common->flow_id,
        driver_response, err1);
      }
      return driver_response;
    }

    uint32_t total_flow_count = 0;
    key_dataflow_t key_df;
    uint32_t keytype;
    uint32_t resp_pos = 10;
    err1 = cli_session->getResponse(resp_pos++, keytype);
    if (err1 != 0) {
      pfc_log_error("Read keytype error for position resp_pos=%d", resp_pos);
      return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
    }
    err1 = cli_session->getResponse(resp_pos++, key_df);
    if (err1 != 0) {
      pfc_log_error("Read keytype error for position resp_pos=%d", resp_pos);
      return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
    }
    err1 = cli_session->getResponse(resp_pos++, total_flow_count);
    if (err1 != 0) {
      pfc_log_error("Read keytype error for position resp_pos=%d", resp_pos);
      return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
    }
    for (uint32_t i = 0; i < total_flow_count; i++) {
      pfc_log_trace("Reading flow %d from driver ", i);
      DataflowDetail *df_segm = new DataflowDetail(kidx_val_df_data_flow_cmn,
                                                   ctr_type);
      if (df_segm == NULL) {
        pfc_log_debug("Memory not allocated for df_segm");
        return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
      }
      err1 = df_segm->sessReadDataflow(*cli_session, resp_pos);
      if (err1 != 0) {
        delete df_segm;
        pfc_log_error("Read dataflow from session error "
                             "for position resp_pos=%d", resp_pos);
        return UNC_UPPL_RC_ERR_DRIVER_COMMUNICATION_FAILURE;
      }
      pfc_flows.push_back(df_segm);
    }
    pfc_log_info("Read %d flows from driver ", total_flow_count);
    df_util_.pfc_flows.insert(
                    std::pair<key_dataflow_t, vector<DataflowDetail *> >
                    (*obj_key_dataflow, pfc_flows));
    pfc_log_info("Got pfc_flows from driver. flows.size=%" PFC_PFMT_SIZE_T
               "for key %s", pfc_flows.size(),
               DataflowCmn::get_string(*obj_key_dataflow).c_str());
  } else {
    pfc_flows = iter->second;
    pfc_log_debug("Got pfc_flows from map. flows.size=%" PFC_PFMT_SIZE_T
                 "for key %s", pfc_flows.size(),
                 DataflowCmn::get_string(*obj_key_dataflow).c_str());
  }

  std::string ctr_name((const char*)obj_key_dataflow->controller_name);
  pfc_log_debug("ctr_name passed to fill_ctrlr_dom_count_map-traversePFC:%s"
                  , ctr_name.c_str());
  UncRespCode ret_code = fill_ctrlr_dom_count_map(db_conn,
                  ctr_name);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Controller-Domain count Map is not filled:%d", ret_code);
    return ret_code;
  }
  //  pfc flows stored in the vector pfc_flows are iterated here
  for (uint32_t i = 0; i < pfc_flows.size(); i++) {
    DataflowDetail *df_segm = pfc_flows[i];
    DataflowCmn *df_cmn = NULL;
    if (is_head_node) {
      df_cmn = new DataflowCmn(is_head_node, df_segm);
      pfc_log_trace("before calling df_util_ appendFlow");
      if (option2 != UNC_OPT2_NO_TRAVERSING)
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
    } else {
      df_cmn = new DataflowCmn(is_head_node, df_segm);
      bool match_result = df_cmn->check_match_condition
                                      (lastPfcNode->output_matches);
      pfc_log_debug("match_result:%d", match_result);
      if (match_result) {
        pfc_log_debug("2nd flow (id=%" PFC_PFMT_u64
                      ") is matching with 1st flow (id=%" PFC_PFMT_u64  ")",
                      df_cmn->df_segment->df_common->flow_id,
                      parentnode->df_segment->df_common->flow_id);
        df_cmn->apply_action();
        df_cmn->parent_node = parentnode;
        UncDataflowReason ret =
               parentnode->
                appendFlow(df_cmn, *(df_util_.get_ctrlr_dom_count_map()));
        if (ret == UNC_DF_RES_EXCEEDS_HOP_LIMIT) {
          delete df_cmn;
          df_cmn = NULL;
        }
      } else {
        pfc_log_debug("2nd flow (id=%" PFC_PFMT_u64
                      ") is not matching with 1st flow (id=%" PFC_PFMT_u64
                      ") so ignoring", df_cmn->df_segment->df_common->flow_id,
                      parentnode->df_segment->df_common->flow_id);
        if (df_cmn != NULL) {
          delete df_cmn;
          df_cmn = NULL;
        }
      }
    }
  }
  if (is_head_node) {
    vector<DataflowCmn* >* firstCtrlrFlows = df_util_.get_firstCtrlrFlows();
    if (firstCtrlrFlows->size() == 0) {
      pfc_log_info("firstCtrlrFlows- vector is empty");
      return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    }
    // If option2 is UNC_OPT2_NO_TRAVERSING, UPPL wil just return the retrived
    // flows to NB. It will not look for egress info and traverse further.With
    // this output, user can confirm whether there are any flows from that
    // point.
    if (option2 == UNC_OPT2_NO_TRAVERSING) {
      pfc_log_debug("Option2 is NO_TRAVERSING,hence returning the flows");
      return UNC_RC_SUCCESS;
    }
  } else {
    pfc_log_debug("Non is_head_node, next.size=%"
                 PFC_PFMT_SIZE_T ", existing reason=%d",
                 parentnode->next.size(), parentnode->addl_data->reason);
    if (parentnode->next.size() == 0 && parentnode->addl_data->reason ==
                                UNC_DF_RES_SUCCESS) {  // Preserving old reason
      if (parentnode->df_segment->df_common->controller_type == UNC_CT_PFC ||
          parentnode->df_segment->df_common->controller_type == UNC_CT_ODC) {
                                            //  if parentnode is PFC type
        parentnode->addl_data->reason = UNC_DF_RES_FLOW_NOT_FOUND;
      } else {
        parentnode->addl_data->reason = UNC_DF_RES_DST_NOT_REACHED;
      }
      return UNC_RC_SUCCESS;
    }
  }
  vector<DataflowCmn* >* CtrlrFlows;
  if (is_head_node) {
    CtrlrFlows = df_util_.get_firstCtrlrFlows();
  } else {
    CtrlrFlows = &(parentnode->next);
  }
  ret_code = checkFlowLimitAndTraverse(db_conn, session_id, configuration_id,
                              sess, key_struct, CtrlrFlows,
                              is_head_node, ingress_bdry_id);
  return ret_code;
}


/** traverseVNP
 * @Description : This function creates the dataflow node and is called when
 * the neighbour found is of type VNP
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

UncRespCode Kt_Dataflow::traverseVNP(OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct,
                                          ServerSession &sess,
                                          bool is_head_node,
                                          DataflowCmn *parentnode,
                                          DataflowCmn *lastPfcNode,
                                          string& ingress_bdry_id) {
  pfc_log_trace("Entered into : %s", __func__);
  // set magic number to vland id so that driver shall skip filtering with vlan
  key_dataflow_t *obj_key_dataflow =
                     reinterpret_cast<key_dataflow_t*>(key_struct);
  obj_key_dataflow->vlan_id = 0xfff0;
  UncRespCode ret_code = checkBoundaryAndTraverse(db_conn, session_id,
           configuration_id, key_struct, sess, false,
           parentnode, lastPfcNode, ingress_bdry_id);
  pfc_log_debug("checkBoundaryAndTraverse returned %d", ret_code);
  return ret_code;
}

/** traverseUnknown
 * @Description : This function creates the dataflow node and is called when
 * the neighbour found is of type Unknown
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
UncRespCode Kt_Dataflow::traverseUNKNOWN(OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct,
                                          ServerSession &sess,
                                          bool is_head_node,
                                          DataflowCmn *parentnode,
                                          DataflowCmn *lastPfcNode,
                                          string& ingress_bdry_id) {
  pfc_log_trace("Entered into : %s", __func__);
  UncRespCode ret_code = checkBoundaryAndTraverse(db_conn, session_id,
          configuration_id, key_struct, sess, false,
          parentnode, lastPfcNode, ingress_bdry_id);
  pfc_log_debug("checkBoundaryAndTraverse returned %d", ret_code);
  return ret_code;
}

/** checkBoundaryAndTraverse
 * @Description : This function finds the neighbour controller of the given
 * controller and invokes resoective traversXXX function based on the ctr type
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
UncRespCode Kt_Dataflow::checkBoundaryAndTraverse(
                                          OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct,
                                          ServerSession &sess,
                                          bool is_head_node,
                                          DataflowCmn *source_node,
                                          DataflowCmn *lastPfcNode,
                                          string &ingress_bdry_id) {
  pfc_log_trace("Entered into : %s", __func__);
  UncRespCode ret_code = UNC_RC_SUCCESS;
  boundary_val obj_bdry_val;
  memset(&obj_bdry_val, '\0', sizeof(boundary_val));
  if (((source_node->df_segment->df_common->controller_type == UNC_CT_PFC) ||
      (source_node->df_segment->df_common->controller_type == UNC_CT_ODC)) &&
     ((source_node->df_segment->df_common->
             valid[kidxDfDataFlowEgressSwitchId] == UNC_VF_INVALID) ||
      (source_node->df_segment->df_common->valid[kidxDfDataFlowOutPort] ==
             UNC_VF_INVALID) ||
      (source_node->df_segment->df_common->
           valid[kidxDfDataFlowOutDomain] == UNC_VF_INVALID))) {
    pfc_log_debug("Valid bits for egress attributes are set as invalid");
    source_node->addl_data->reason = UNC_DF_RES_SUCCESS;
    return UNC_RC_SUCCESS;
  }
  obj_bdry_val.controller_type =
      source_node->df_segment->df_common->controller_type;
  if (source_node->df_segment->df_common->controller_name[0] == '\0') {
    pfc_log_debug("Controller name is empty");
    if ((source_node->next.size() == 0) &&
       (source_node->addl_data->reason == UNC_DF_RES_SUCCESS))
      source_node->addl_data->reason = UNC_DF_RES_SYSTEM_ERROR;
    return UNC_RC_SUCCESS;
  }
  memcpy(obj_bdry_val.controller_name,
         source_node->df_segment->df_common->controller_name,
         sizeof(obj_bdry_val.controller_name));
  if (source_node->df_segment->df_common->out_domain[0] == '\0') {
    if ((source_node->next.size() == 0) &&
       (source_node->addl_data->reason == UNC_DF_RES_SUCCESS))
      source_node->addl_data->reason = UNC_DF_RES_SYSTEM_ERROR;
    pfc_log_debug("Domain name is empty");
    return UNC_RC_SUCCESS;
  }
  memcpy(obj_bdry_val.domain_name,
         source_node->df_segment->df_common->out_domain,
         sizeof(obj_bdry_val.domain_name));

  if (source_node->df_segment->df_common->controller_type != UNC_CT_UNKNOWN) {
    if (source_node->df_segment->df_common->egress_switch_id[0] == '\0') {
      if ((source_node->next.size() == 0) &&
          (source_node->addl_data->reason == UNC_DF_RES_SUCCESS))
        source_node->addl_data->reason = UNC_DF_RES_SYSTEM_ERROR;
      pfc_log_debug("Switch id is empty");
      return UNC_RC_SUCCESS;
    }
    memcpy(obj_bdry_val.lp_str.switch_id,
           source_node->df_segment->df_common->egress_switch_id,
         sizeof(obj_bdry_val.lp_str.switch_id));
    if (source_node->df_segment->df_common->out_port[0] == '\0') {
      if ((source_node->next.size() == 0) &&
          (source_node->addl_data->reason == UNC_DF_RES_SUCCESS))
        source_node->addl_data->reason = UNC_DF_RES_SYSTEM_ERROR;
      pfc_log_debug("Port no. is empty");
      return UNC_RC_SUCCESS;
    }
    memcpy(obj_bdry_val.lp_str.port_id,
         source_node->df_segment->df_common->out_port,
         sizeof(obj_bdry_val.lp_str.port_id));
  }
  pfc_log_debug("ingress_bdry_id = %s", ingress_bdry_id.c_str());
  if (!ingress_bdry_id.empty()) {
    memcpy(obj_bdry_val.boundary_id, ingress_bdry_id.c_str(),
            ingress_bdry_id.length() + 1);
  } else {
    pfc_log_debug("ingress_bdry_id is NULL");
  }

  list<boundary_val> list_ctr_nbrs;
  UncRespCode read_status = FindNeighbourCtr(db_conn, lastPfcNode,
                                    &obj_bdry_val, list_ctr_nbrs,
                                    ingress_bdry_id);
  pfc_log_info("Size of list_ctr_nbrs:%" PFC_PFMT_SIZE_T, list_ctr_nbrs.size());
  if ((read_status != UNC_RC_SUCCESS) || (list_ctr_nbrs.size() == 0)) {
    pfc_log_trace("Read of neighbours failed :%d", read_status);
    // for VNP and BYPASS add this error, PFC ignore.
    if (obj_bdry_val.controller_type == UNC_CT_PFC ||
        obj_bdry_val.controller_type == UNC_CT_ODC) {
      source_node->addl_data->reason = UNC_DF_RES_SUCCESS;
    } else {
      source_node->addl_data->reason = UNC_DF_RES_DST_NOT_REACHED;
    }
    return UNC_RC_SUCCESS;
  }
  list<boundary_val>::iterator nbrs_ctr_iter = list_ctr_nbrs.begin();
  for ( ; nbrs_ctr_iter != list_ctr_nbrs.end(); nbrs_ctr_iter++) {
    string current_bdry_id = (const char*)((*nbrs_ctr_iter).boundary_id);
    pfc_log_debug("current_boundary_id:%s ingress_bdry_id:%s",
                   current_bdry_id.c_str(), ingress_bdry_id.c_str());
    boundary_val obj_bval = *nbrs_ctr_iter;
    if (((*nbrs_ctr_iter).controller_type == UNC_CT_PFC) ||
        ((*nbrs_ctr_iter).controller_type == UNC_CT_ODC)) {
      key_dataflow_t obj_key_df;
      memcpy(obj_key_df.controller_name, obj_bval.controller_name,
             sizeof(obj_bval.controller_name));
      memcpy(obj_key_df.switch_id, obj_bval.lp_str.switch_id,
             sizeof(obj_bval.lp_str.switch_id));
      memcpy(obj_key_df.port_id, obj_bval.lp_str.port_id,
             sizeof(obj_bval.lp_str.port_id));
      key_dataflow_t *obj_key_dataflow =
                       reinterpret_cast<key_dataflow_t*>(key_struct);
      obj_key_df.vlan_id = obj_key_dataflow->vlan_id;
      memcpy(obj_key_df.src_mac_address, obj_key_dataflow->src_mac_address,
             sizeof(obj_key_df.src_mac_address));
      pfc_log_info("checkBoundaryAndTraverse controller_type UNC_CT_PFC/ODC");
      UpplDomainType dom_type = UPPL_DOMAIN_TYPE_DEFAULT;
      // fill the controller name and domain name
      key_ctr_domain_t obj_key_domain;
      memset(&obj_key_domain, '\0', sizeof(key_ctr_domain_t));
      memcpy(obj_key_domain.ctr_key.controller_name, obj_bval.controller_name,
             sizeof(obj_bval.controller_name));
      memcpy(obj_key_domain.domain_name, obj_bval.domain_name,
             sizeof(obj_bval.domain_name));
      // get the domain type from the domain_table
      ret_code = getDomainType(db_conn, &obj_key_domain, NULL,
                                           UNC_DT_RUNNING , dom_type);
      if (ret_code != UNC_RC_SUCCESS) {
        pfc_log_debug("Failed to get the domain type :%d", ret_code);
      }
      if (dom_type != UPPL_DOMAIN_TYPE_PF_LEAF &&
            dom_type != UPPL_DOMAIN_TYPE_PF_SPINE) {
        ret_code = traversePFC(db_conn, session_id, configuration_id,
                           reinterpret_cast<void*>(&obj_key_df), sess, false,
                           source_node, lastPfcNode, current_bdry_id,
                           UNC_OPT2_NONE);
        pfc_log_debug("traversePFC returned %d", ret_code);
        if (ret_code == UNC_UPPL_RC_ERR_DB_ACCESS) {
          return ret_code;  // For other errors, we are setting reason.
        }
      } else {
        ret_code = UNC_UPPL_RC_ERR_INVALID_STATE;
      }
      if (source_node->next.size() == 0) {
        if ((ret_code != UNC_RC_SUCCESS) &&
            (source_node->addl_data->reason == UNC_DF_RES_SUCCESS)) {
          if (ret_code == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE)
            source_node->addl_data->reason = UNC_DF_RES_FLOW_NOT_FOUND;
          else if (ret_code == UNC_UPPL_RC_ERR_CTRLR_DISCONNECTED)
            source_node->addl_data->reason = UNC_DF_RES_CTRLR_DISCONNECTED;
          else if (ret_code == UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED)
            source_node->addl_data->reason = UNC_DF_RES_OPERATION_NOT_SUPPORTED;
          else if (ret_code == UNC_UPPL_RC_ERR_INVALID_STATE)
            source_node->addl_data->reason = UNC_DF_RES_DOMAIN_NOT_SUPPORTED;
          else
            source_node->addl_data->reason = UNC_DF_RES_SYSTEM_ERROR;
        }
      } else {
        // Already flow appended, so donot overwrite the reason
        pfc_log_debug("Preserving the old reason:%d",
                        source_node->addl_data->reason);
      }
    } else if (((*nbrs_ctr_iter).controller_type == UNC_CT_VNP) ||
          ((*nbrs_ctr_iter).controller_type == UNC_CT_POLC) ||
          ((*nbrs_ctr_iter).controller_type == UNC_CT_UNKNOWN)) {
      list<boundary_val> keys;
      list<boundary_val> vals;
      ret_code = getkeysfrom_boundary_map((const char*)obj_bval.controller_name,
                                         keys, vals,
                                         (const char*)obj_bval.domain_name,
                                         obj_bval.controller_type);
      if (ret_code != UNC_RC_SUCCESS) {
        if ((source_node->next.size() == 0) &&
            source_node->addl_data->reason == UNC_DF_RES_SUCCESS) {
          if (ret_code == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE)
            source_node->addl_data->reason = UNC_DF_RES_DST_NOT_REACHED;
          else
            source_node->addl_data->reason = UNC_DF_RES_SYSTEM_ERROR;
        }
      }
      pfc_log_info("getkeysfrom_boundary_map returned keys.size=%"
                     PFC_PFMT_SIZE_T, keys.size());
      // This count is to track which is the last non pfc node
      // the count is incremented by one when boundary id is found to be source
      uint32_t count = 0;
      // Fix for ticket 2137--start
      list<boundary_val>::iterator keys_iter, vals_iter, iter_key1,
                         iter_val1, iter2;
      bool is_key_iterated = false;
      if (obj_bval.controller_type == UNC_CT_POLC ||
         (obj_bval.controller_type == UNC_CT_VNP)) {
        for (iter_key1 = keys.begin(), iter_val1 = vals.begin(),
          iter2 = ++keys.begin(); iter2 != keys.end(); iter2++) {
          pfc_log_debug("value1 for comparison:\n"
                    "controller_name:%s\ndomain_name:%s\nswitch_id:%s\n"
                    "port_id:%s\n",
                     (*iter_key1).controller_name, (*iter_key1).domain_name,
                     (*iter_key1).lp_str.switch_id,
                     (*iter_key1).lp_str.port_id);
          pfc_log_debug("value2 for comparison:\n"
                    "controller_name:%s\ndomain_name:%s\nswitch_id:%s\n"
                    "port_id:%s\n",
                     (*iter2).controller_name, (*iter2).domain_name,
                     (*iter2).lp_str.switch_id, (*iter2).lp_str.port_id);

          UPPL_COMPARE_STRUCT(*iter_key1, *iter2, is_key_iterated);
          if (is_key_iterated == true) {
            iter_key1 = keys.erase(iter_key1);
            iter_val1 = vals.erase(iter_val1);
            iter2 = iter_key1;
            pfc_log_debug("Erasing a duplicate entry for PORT_GROUP");
            is_key_iterated = false;
            continue;
          }
          iter_key1++;
          iter_val1++;
        }
        pfc_log_info("getkeysfrom_boundary_map keys.size after"
                 " erasing duplicate entries=%"
                   PFC_PFMT_SIZE_T, keys.size());
      }
      // Fix for ticket 2137--end
      for (keys_iter = keys.begin(), vals_iter = vals.begin();
           keys_iter != keys.end(); keys_iter++, vals_iter++) {
        bool ret1 = false;
        // Comparing the keys
        UPPL_COMPARE_STRUCT(obj_bval, (*keys_iter), ret1);
        bool ret2 = false;
        // Comparing the neighbour values
        pfc_log_debug("value1 for comparison:\nport_type = %d\n"
                    "controller_name:%s\ndomain_name:%s\nswitch_id:%s\n"
                    "port_id:%s\nboundary_id:%s\ncurrent boundary_id:%s",
                     obj_bdry_val.lp_str.port_type,
                     obj_bdry_val.controller_name, obj_bdry_val.domain_name,
                     obj_bdry_val.lp_str.switch_id, obj_bdry_val.lp_str.port_id,
                     obj_bdry_val.boundary_id, current_bdry_id.c_str());
        pfc_log_debug("value2 for comparison:\nport_type = %d\n"
                    "controller_name:%s\ndomain_name:%s\nswitch_id:%s\n"
                    "port_id:%s\nboundary_id:%s\n",
                     (*vals_iter).lp_str.port_type,
                     (*vals_iter).controller_name, (*vals_iter).domain_name,
                     (*vals_iter).lp_str.switch_id, (*vals_iter).lp_str.port_id,
                     (*vals_iter).boundary_id);

        if (((*vals_iter).lp_str.port_type == UPPL_LP_SUBDOMAIN) ||
           ((*vals_iter).lp_str.port_type == UPPL_LP_TRUNK_PORT) ||
           ((*vals_iter).lp_str.port_type == UPPL_LP_SWITCH) ||
           ((*vals_iter).lp_str.port_type == UPPL_LP_PORT_GROUP)) {
          pfc_log_debug("ret2 is false from COMPARE_STRUCT");
          if (strcmp((const char*)((*vals_iter).boundary_id),
                                (const char*)(current_bdry_id.c_str())) == 0) {
            pfc_log_debug("Boundary Id is same. Is vlan, src mac changed?");
            if (lastPfcNode->is_vlan_src_mac_changed_ == false) {
               pfc_log_debug("Boundary Id is same && vlan,"
                   "src mac are also not changed");
               ret2 = true;
               count++;
            }
          } else {
            if (source_node->parent_node != NULL &&
                ((source_node->parent_node->df_segment->
                  df_common->controller_type != UNC_CT_PFC) &&
                 (source_node->parent_node->df_segment->
                  df_common->controller_type != UNC_CT_ODC))) {
              pfc_log_debug("PBP Boundary Id is not same nonpfc_ingr_bdry_id %s"
                "current_bdry_id %s", source_node->parent_node->addl_data->
                              nonpfc_ingr_bdry_id.c_str(),
                              current_bdry_id.c_str());
              if (strcmp((const char*)(source_node->parent_node->
                                     addl_data->nonpfc_ingr_bdry_id.c_str()),
                      (const char*)((*vals_iter).boundary_id)) == 0) {
                pfc_log_debug("PBP Boundary Id is not same."
                  "Is vlan, src mac changed?");
                if (lastPfcNode->is_vlan_src_mac_changed_ == false) {
                  pfc_log_debug("PBP Boundary Id is not same && vlan,"
                   "src mac are also not changed");
                  ret2 = true;
                }
              }
            } else {
              pfc_log_debug("PBP source_node->parent_node is PFC");
            }
          }
        } else {
          UPPL_COMPARE_STRUCT(obj_bdry_val, (*vals_iter), ret2);
        }
        DataflowDetail *df_segment =
            new DataflowDetail(kidx_val_df_data_flow_cmn,
               unc_keytype_ctrtype_t((*nbrs_ctr_iter).controller_type));
        if (df_segment == NULL) {
          pfc_log_debug("Memory not allocated for df_segment");
          return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
        }
        DataflowCmn *df_cmn = new DataflowCmn(is_head_node, df_segment);
        if (df_cmn == NULL) {
          delete df_segment;
          df_segment = NULL;
          pfc_log_debug("Memory not allocated for df_cmn");
          return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
        }
        pfc_log_debug("PBP obj_val.boundary_id=%s", obj_bval.boundary_id);
        pfc_log_debug("PBP (*vals_iter).boundary_id=%s",
                         (*vals_iter).boundary_id);
        pfc_log_debug("PBP current_bdry_id=%s", current_bdry_id.c_str());
        df_cmn->addl_data->nonpfc_ingr_bdry_id =
                           (const char *)obj_bval.boundary_id;

        if ((ret1 == true) && (ret2 == true)) {
          if ((keys.size() == 1) || count == (uint32_t)keys.size()) {
            // Create a node
            UncDataflowReason ret =
                    CreateDfCmnNodeForNonPfc(db_conn, df_segment, source_node,
                                     df_cmn, &obj_bval,
                                     (*keys_iter), false, ret_code);
            if (ret != UNC_DF_RES_SUCCESS) {
              if (df_cmn != NULL) {
                delete df_cmn;
                df_cmn = NULL;
              }
              if (ret_code != UNC_RC_SUCCESS)
                return ret_code;
            }
          } else {
            if (df_cmn != NULL) {
              delete df_cmn;
              df_cmn = NULL;
            }
          }
          pfc_log_debug("Ignoring source key");
          continue;
        }
        UncDataflowReason ret = CreateDfCmnNodeForNonPfc(db_conn, df_segment,
                               source_node,
                               df_cmn, &obj_bval, (*keys_iter), true, ret_code);
        if (ret != UNC_DF_RES_SUCCESS) {
          if (df_cmn != NULL) {
            delete df_cmn;
            df_cmn = NULL;
          }
          if (ret_code != UNC_RC_SUCCESS)
            return ret_code;
          continue;
        }
      UpplDomainType dom_type = UPPL_DOMAIN_TYPE_DEFAULT;
        if ((*nbrs_ctr_iter).controller_type == UNC_CT_VNP ||
             (*nbrs_ctr_iter).controller_type == UNC_CT_POLC ) {
          ret_code = traverseVNP(db_conn, session_id, configuration_id,
                            key_struct, sess, false, df_cmn, lastPfcNode,
                            current_bdry_id);
          if (ret_code == UNC_UPPL_RC_ERR_DB_ACCESS) {
            return ret_code;
          }
        } else if ((*nbrs_ctr_iter).controller_type == UNC_CT_UNKNOWN) {
          key_dataflow_t obj_key_df_tgt;
          memcpy(obj_key_df_tgt.controller_name, (*vals_iter).controller_name,
             sizeof((*vals_iter).controller_name));
          memcpy(obj_key_df_tgt.switch_id, (*vals_iter).lp_str.switch_id,
             sizeof((*vals_iter).lp_str.switch_id));
          memcpy(obj_key_df_tgt.port_id, (*vals_iter).lp_str.port_id,
             sizeof((*vals_iter).lp_str.port_id));
          key_dataflow_t *obj_key_dataflow =
                       reinterpret_cast<key_dataflow_t*>(key_struct);
          obj_key_df_tgt.vlan_id = obj_key_dataflow->vlan_id;
          memcpy(obj_key_df_tgt.src_mac_address,
               obj_key_dataflow->src_mac_address,
             sizeof(obj_key_df_tgt.src_mac_address));
          if ((*vals_iter).controller_type == UNC_CT_PFC ||
            (*vals_iter).controller_type == UNC_CT_ODC) {
            // fill the controller name and domain name
            key_ctr_domain_t obj_key_domain;
            memset(&obj_key_domain, '\0', sizeof(key_ctr_domain_t));
            memcpy(obj_key_domain.ctr_key.controller_name,
                     (*vals_iter).controller_name,
                     sizeof((*vals_iter).controller_name));
            memcpy(obj_key_domain.domain_name, (*vals_iter).domain_name,
             sizeof((*vals_iter).domain_name));
            // get the domain type from the domain_table
            ret_code = getDomainType(db_conn, &obj_key_domain, NULL,
                                           UNC_DT_RUNNING, dom_type);
            if (ret_code != UNC_RC_SUCCESS) {
              pfc_log_debug("Failed to get the domain type :%d", ret_code);
            }
            if (dom_type != UPPL_DOMAIN_TYPE_PF_LEAF &&
                        dom_type != UPPL_DOMAIN_TYPE_PF_SPINE) {
              ret_code = traversePFC(db_conn, session_id, configuration_id,
                            &obj_key_df_tgt, sess, false, df_cmn, lastPfcNode,
                            current_bdry_id, UNC_OPT2_NONE);
              if (ret_code == UNC_UPPL_RC_ERR_DB_ACCESS) {
                return ret_code;
              }
            } else {
              ret_code = UNC_UPPL_RC_ERR_INVALID_STATE;
            }
          } else if ((*vals_iter).controller_type == UNC_CT_VNP ||
                    (*vals_iter).controller_type == UNC_CT_POLC) {
            ret_code = traverseVNP(db_conn, session_id, configuration_id,
                            &obj_key_df_tgt, sess, false, df_cmn, lastPfcNode,
                            current_bdry_id);
            if (ret_code == UNC_UPPL_RC_ERR_DB_ACCESS) {
              return ret_code;
            }
          }
        }
        pfc_log_trace("traverseXXX returned %d", ret_code);
        if ((ret_code != UNC_RC_SUCCESS) &&
           (df_cmn->addl_data->reason == UNC_DF_RES_SUCCESS)) {
          if (ret_code == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE)
            df_cmn->addl_data->reason = UNC_DF_RES_DST_NOT_REACHED;
          else if (ret_code == UNC_UPPL_RC_ERR_CTRLR_DISCONNECTED)
            df_cmn->addl_data->reason = UNC_DF_RES_CTRLR_DISCONNECTED;
          else if (ret_code == UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED)
            df_cmn->addl_data->reason = UNC_DF_RES_OPERATION_NOT_SUPPORTED;
          else if (ret_code == UNC_UPPL_RC_ERR_INVALID_STATE)
            df_cmn->addl_data->reason = UNC_DF_RES_DOMAIN_NOT_SUPPORTED;
          else
            df_cmn->addl_data->reason = UNC_DF_RES_SYSTEM_ERROR;
        }
      }
      bool restart_1stloop = false;
      do {
        pfc_log_debug("PBP Erasing the intermediate flows size=%"
                   PFC_PFMT_SIZE_T, source_node->next.size());
        restart_1stloop = false;
        for (vector<DataflowCmn *>::iterator iter_st =
                                      source_node->next.begin();
             iter_st != source_node->next.end(); iter_st++) {
          bool continue_from_1stloop = false;
          for (vector<DataflowCmn *>::iterator iter_curr = iter_st+1;
                 iter_curr != source_node->next.end(); iter_curr++) {
            DataflowCmn *st = reinterpret_cast<DataflowCmn *>(*iter_st);
            DataflowCmn *curr = reinterpret_cast<DataflowCmn *>(*iter_curr);
            if ((strcmp((const char*)st->df_segment->df_common->controller_name,
             (const char*)curr->df_segment->df_common->controller_name) == 0) &&
               (strcmp((const char*)st->df_segment->df_common->out_domain,
               (const char*)curr->df_segment->df_common->out_domain) == 0)) {
              pfc_log_debug("PBP cname %s and dname %s"
                "matching curr->next.size=%"
                PFC_PFMT_SIZE_T",st->next.size=%"PFC_PFMT_SIZE_T,
                (const char*)st->df_segment->df_common->controller_name,
                (const char*)curr->df_segment->df_common->out_domain,
                curr->next.size(), st->next.size());
              if (curr->next.size() == 0) {
                if (source_node->next.size()>1)
                  source_node->head->total_flow_count--;
                pfc_log_debug("PBP Erasing the intermediate flow");
                delete curr;
                source_node->next.erase(iter_curr);
                continue_from_1stloop = true;
                restart_1stloop = true;
              } else if (st->next.size() == 0) {
                if (source_node->next.size()>1)
                  source_node->head->total_flow_count--;
                pfc_log_debug("PBP Erasing the intermediate flow");
                delete st;
                source_node->next.erase(iter_st);
                continue_from_1stloop = true;
                restart_1stloop = true;
              }
            }
            if (continue_from_1stloop)
              break;
          }
          if (restart_1stloop)
            break;
        }
      } while (restart_1stloop);
      pfc_log_debug("PBP AFTER flows size=%"PFC_PFMT_SIZE_T,
                     source_node->next.size());
    }
  }
  return UNC_RC_SUCCESS;
}


/** Fill_Attr_Syntax_Map
 * @Description : This function populates the values to be used for attribute
 * validation
 * @param[in] : None
 * @return    : void
 * */
void Kt_Dataflow::Fill_Attr_Syntax_Map() {
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;
  Kt_Class_Attr_Syntax objKeyCtrNameAttrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 31, true, ""};
  attr_syntax_map[CTR_NAME_STR] = objKeyCtrNameAttrSyntax;
  Kt_Class_Attr_Syntax objKeyDomNameAttrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 31, true, ""};
  attr_syntax_map[DOMAIN_NAME_STR] = objKeyDomNameAttrSyntax;
  Kt_Class_Attr_Syntax objKeySwitchIdAttrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 255, true, ""};
  attr_syntax_map[SWITCH_ID_STR] = objKeySwitchIdAttrSyntax;
  Kt_Class_Attr_Syntax objKeyPortIdAttrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 31, true, ""};
  attr_syntax_map[PORT_ID_STR] = objKeyPortIdAttrSyntax;
  Kt_Class_Attr_Syntax objKeyVlanIdAttrSyntax =
  { PFC_IPCTYPE_UINT16, 1, 4095, 0, 0, true, ""};
  attr_syntax_map[VLAN_ID_STR] = objKeyVlanIdAttrSyntax;
  Kt_Class_Attr_Syntax objKeyMacAddrAttrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 6, true, ""};
  attr_syntax_map[PORT_MAC_ADDRESS_STR] = objKeyMacAddrAttrSyntax;
  attr_syntax_map_all[UNC_KT_DATAFLOW] = attr_syntax_map;
}


/** FindNeighbourCtr
 * @Description : This function returns the neighbours of the given controller
 * @param[in]   : OdbcmConnectionHandler, neighbour_ctr_key - key to find from 
 * map
 * @param[out]  : found_nbrs - list of neighbours
 * @return    : UncRespCode UNC_RC_SUCCESS if successfull or UNC_UPPL_RC_ERR*
 *              if any Read failed
 * */
UncRespCode Kt_Dataflow::FindNeighbourCtr(OdbcmConnectionHandler *db_conn,
             DataflowCmn *lastPfcNode,
             boundary_val *neighbour_ctr_key, list<boundary_val> &found_nbrs,
             string &ingress_bdry_id) {
  pfc_log_trace("Entered into : %s", __func__);
  UncRespCode read_status = UNC_RC_SUCCESS;
  read_status = PrepareBoundaryMap(db_conn);
  if (read_status != UNC_RC_SUCCESS)
    return read_status;
  if ((get_boundary_map())->empty()) {
    pfc_log_debug("boundary_map is empty !! ");
    return UNC_UPPL_RC_FAILURE;
  }
  read_status = getBoundaryPorts(lastPfcNode, neighbour_ctr_key,
                                  found_nbrs, ingress_bdry_id);
  if (read_status != UNC_RC_SUCCESS)
    pfc_log_debug("found-Neighbour count = %" PFC_PFMT_SIZE_T,
                  found_nbrs.size());
  return read_status;
}

/** PrepareBoundaryMap
 * @Description : This function prepare the Boudary map by reading Bdry table
 * @param[in] : OdbcmConnectionHandler,
 * @return    : UncRespCode UNC_RC_SUCCESS if successfull or UNC_UPPL_RC_ERR*
 *              if any Read failed
 * */
UncRespCode Kt_Dataflow::PrepareBoundaryMap
                            (OdbcmConnectionHandler *db_conn) {
  pfc_log_trace("Entered into : %s map.size=%" PFC_PFMT_SIZE_T,
                 __func__, boundary_map_.size());
  if (boundary_map_.size()>0) return UNC_RC_SUCCESS;
  //  ReadInternal of Kt_Boundary
  Kt_Boundary obj_Kt_Boundary;
  key_boundary_t *obj_key_boundary = new key_boundary_t;
  memset(obj_key_boundary, 0, sizeof(key_boundary_t));
  string boundary_id = "";
  memcpy(obj_key_boundary->boundary_id, boundary_id.c_str(),
         boundary_id.length());
  void* key_struct = reinterpret_cast<void*>(obj_key_boundary);
  vector<void *> vect_key_boundary;
  vect_key_boundary.push_back(key_struct);
  vector<void *> vect_val_boundary;
  // vector which will holds the boundary values
  vector<boundary_record>* bdry_rec_vect = get_boundary_tbl_vect();
  vector<boundary_record>::iterator bdry_rec_vect_iter;

  UncRespCode ret_code = obj_Kt_Boundary.ReadInternal(db_conn,
                                      vect_key_boundary,
                                      vect_val_boundary,
                                      UNC_DT_RUNNING,
                                      UNC_OP_READ_SIBLING_BEGIN);

  if (NULL != obj_key_boundary) {
    delete obj_key_boundary;
    pfc_log_debug("obj_key_boundary is deleted");
    obj_key_boundary = NULL;
  }
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Read of Boundary failed");
    if (ret_code == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE)
      return UNC_RC_SUCCESS;
    return ret_code;
  }
  // Fill boundary_tbl_vect_ vector-using the ReadInternal return val structrue
  for (uint32_t index1 = 0;
        index1 < vect_key_boundary.size();
        ++index1) {
     val_boundary_st *val_boundary_vect = reinterpret_cast <val_boundary_st*>
                                           (vect_val_boundary[index1]);
     key_boundary_t *key_boundary_vect = reinterpret_cast <key_boundary_t*>
                                           (vect_key_boundary[index1]);
     boundary_record obj_bdry_rec;
     memset(&obj_bdry_rec, '\0', sizeof(obj_bdry_rec));
     memcpy(obj_bdry_rec.boundary_id, key_boundary_vect->boundary_id,
            sizeof(obj_bdry_rec.boundary_id));
     memcpy(obj_bdry_rec.ctr_name1,
            val_boundary_vect->boundary.controller_name1,
            sizeof(obj_bdry_rec.ctr_name1));
     memcpy(obj_bdry_rec.ctr_name2,
            val_boundary_vect->boundary.controller_name2,
            sizeof(obj_bdry_rec.ctr_name2));
     memcpy(obj_bdry_rec.dom_name1, val_boundary_vect->boundary.domain_name1,
            sizeof(obj_bdry_rec.dom_name1));
     memcpy(obj_bdry_rec.dom_name2, val_boundary_vect->boundary.domain_name2,
            sizeof(obj_bdry_rec.dom_name2));
     memcpy(obj_bdry_rec.lp_id1, val_boundary_vect->boundary.logical_port_id1,
            sizeof(obj_bdry_rec.lp_id1));
     memcpy(obj_bdry_rec.lp_id2, val_boundary_vect->boundary.logical_port_id2,
            sizeof(obj_bdry_rec.lp_id2));
     if ((obj_bdry_rec.lp_id1[0] == '\0') ||
         (val_boundary_vect->oper_status == UPPL_BOUNDARY_OPER_UNKNOWN))
       obj_bdry_rec.is_filled1 = true;
     else
       obj_bdry_rec.is_filled1 = false;
     if ((obj_bdry_rec.lp_id2[0] == '\0') ||
         (val_boundary_vect->oper_status == UPPL_BOUNDARY_OPER_UNKNOWN))
       obj_bdry_rec.is_filled2 = true;
     else
       obj_bdry_rec.is_filled2 = false;
     bdry_rec_vect->push_back(obj_bdry_rec);
     pfc_log_debug("Inserting into vector<boundary_record>:%d", index1);
     // Clearing the key and value void* in the vector.
     ::operator delete(vect_val_boundary[index1]);
     ::operator delete(vect_key_boundary[index1]);
  }
  // Clearing the vectors
  vect_val_boundary.clear();
  vect_key_boundary.clear();

  // Update switch_id port_type and port_id into boundary_tbl_vect_
  for (uint16_t index2 = 0; index2 < bdry_rec_vect->size(); ++index2) {
    string ctr_name, dom_name, lp_id, lp_map_key="";
    string separator = "&";
    stringstream ss;
    boundary_record obj_bdry_rec = (*bdry_rec_vect)[index2];
    if (obj_bdry_rec.is_filled1 == false) {
      ctr_name = (const char*)obj_bdry_rec.ctr_name1;
      dom_name = (const char*)obj_bdry_rec.dom_name1;
      ret_code = PrepareCollectiveLPMap(db_conn, ctr_name, dom_name);
      if (ret_code != UNC_RC_SUCCESS) {
        pfc_log_info("Read of LP failed :%d", ret_code);
        return ret_code;
      }
      lp_id = (const char*)obj_bdry_rec.lp_id1;
      ss << ctr_name <<separator << dom_name << separator << lp_id;
      string lp_map_key = ss.str();
      ret_code = update_boundary_tbl_vect(lp_map_key, index2,
                 (uint8_t)UPPL_LEFT_PART);
      if (ret_code != UNC_RC_SUCCESS) {
        pfc_log_info("update_boundary_tbl_vect returned failure:%d", ret_code);
        return ret_code;
      }
    }
    if (obj_bdry_rec.is_filled2 == false) {
      ctr_name = (const char*)obj_bdry_rec.ctr_name2;
      dom_name = (const char*)obj_bdry_rec.dom_name2;
      ret_code = PrepareCollectiveLPMap(db_conn, ctr_name, dom_name);
      if (ret_code != UNC_RC_SUCCESS) {
        pfc_log_info("Read of LP failed :%d", ret_code);
        return ret_code;
      }
      ss.str(std::string());
      lp_id = (const char*)obj_bdry_rec.lp_id2;
      ss << ctr_name <<separator << dom_name << separator << lp_id;
      string lp_map_key = ss.str();
      ret_code = update_boundary_tbl_vect(lp_map_key, index2,
                  (uint8_t)UPPL_RIGHT_PART);
      if (ret_code != UNC_RC_SUCCESS) {
        pfc_log_info("update_boundary_tbl_vect returned failure:%d", ret_code);
        return ret_code;
      }
    }
  }

  bool to_be_continued = false;
  do {
    to_be_continued = false;
    for (uint16_t index3 = 0; index3 < bdry_rec_vect->size(); ++index3) {
      if (((*bdry_rec_vect)[index3].is_filled1 == false) ||
         ((*bdry_rec_vect)[index3].is_filled2 == false)) {
        pfc_log_debug("Switch id and Port id are not filled, hence"
             " removing the entry from the boundary vector");
        bdry_rec_vect->erase(bdry_rec_vect->begin() + index3);
        to_be_continued = true;
        break;
      }
    }
  } while ( to_be_continued == true);

  /*
  //  For testing
  for (uint16_t index2 = 0; index2 < bdry_rec_vect->size(); ++index2) {
    boundary_record test = (*bdry_rec_vect)[index2];
    stringstream ss1;
    stringstream ss2;
    stringstream ss3;
    stringstream ss4;
    ss1 << test.ctr_name1 << "&" << test.dom_name1 << "&" << test.lp_id1;
    ss2 << test.sw_id1 << "&" << test.port_id1;
    ss3 << test.ctr_name2 << "&" << test.dom_name2 << "&" <<test.lp_id2;
    ss4 << test.sw_id2 << "&" << test.port_id2;
    pfc_log_debug("entry in bdry vector first half:%s", ss1.str().c_str());
    pfc_log_debug("entry in bdry vector first half swidportid:%s",
                 ss2.str().c_str());
    pfc_log_debug("entry in bdry vector second half:%s", ss3.str().c_str());
    pfc_log_debug("entry in bdry vector second half swidportid:%s",
                 ss4.str().c_str());
  }
  */
  ret_code = fill_boundary_map(db_conn);
  if (ret_code != UNC_RC_SUCCESS) {
    pfc_log_info("Parsing of Bdry value structures failed:%d", ret_code);
    return ret_code;
  }
  return ret_code;
}

/** 
 * @Description : This function get the all possible neighbout boundary 
 * ports from the boundary map 
 * @param[in] :  boundary_val *neighbour_ctr_key
 * @param[in] :  list<boundary_val> &found_nbrs
 * @return    : UncRespCode UNC_RC_SUCCESS if successfull or UNC_UPPL_RC_ERR*
 * */
UncRespCode Kt_Dataflow::getBoundaryPorts(DataflowCmn *lastPfcNode,
               boundary_val *neighbour_ctr_key,
               list<boundary_val> &found_nbrs,
               string &ingress_bdry_id) {
  std::pair<multimap<string, boundary_val>::iterator,
           multimap<string, boundary_val>::iterator> bdry_map_range_iter;
  multimap<string, boundary_val> *bdry_map = get_boundary_map();
  multimap<string, boundary_val>::iterator bdry_map_iter;
  string separator = "&";
  string new_map_key_str = "";
  std::stringstream ss;
  ss.str("");
  if (strlen((const char*) neighbour_ctr_key->controller_name) != 0)
    ss << neighbour_ctr_key->controller_name;
  else
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  if (strlen((const char*) neighbour_ctr_key->domain_name) != 0)
    ss << separator << neighbour_ctr_key->domain_name;
  else
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  ss << separator << neighbour_ctr_key->lp_str.switch_id;
  ss << separator << neighbour_ctr_key->lp_str.port_id;
  new_map_key_str = ss.str();
  pfc_log_info("key to find in bdry map:%s", new_map_key_str.c_str());

  bdry_map->begin();
  // Returns multiple neighbours
  bdry_map_range_iter = bdry_map->equal_range(new_map_key_str);
  for (bdry_map_iter = bdry_map_range_iter.first;
       bdry_map_iter != bdry_map_range_iter.second; bdry_map_iter++) {
    pfc_log_trace("Inside interation of Bdry map");
    if (bdry_map_iter == bdry_map->end()) {
      pfc_log_debug("Element does not exist");
      return UNC_UPPL_RC_FAILURE;
    }
    boundary_val obj_bdry_val = bdry_map_iter->second;
    if ((!ingress_bdry_id.empty()) &&
        (strcmp((const char*)obj_bdry_val.boundary_id,
                 ingress_bdry_id.c_str()) == 0) &&
        (lastPfcNode->is_vlan_src_mac_changed_ != true)) {
      pfc_log_debug("Ignoring originating boundary ports");
    } else {
      found_nbrs.push_back(obj_bdry_val);
      pfc_log_trace("Pushing back an elemnt");
    }
  }
  pfc_log_trace("Count of found neighbours is %" PFC_PFMT_SIZE_T,
                found_nbrs.size());
  if (found_nbrs.empty()) {
    pfc_log_debug("No Neighbour found:%d", UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE);
    return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  }
  return UNC_RC_SUCCESS;
}

/** BRecordDebugPrint
 *  @Description : this inline function stores the boundary_record struct
 *  values into string
 *  @param[in] : boundary_record
 *  @return    : string
 */
inline std::string BRecordDebugPrint(boundary_record& oboundary_record,
                   UpplBoundaryTblSection part) {
  stringstream ss;
  ss.str(std::string());
  if (part == UPPL_LEFT_PART) {
    ss << oboundary_record.sw_id1 << " " << oboundary_record.port_id1
     << " " << oboundary_record.is_filled1 << " " <<
     PhyUtil::uint8tostr(oboundary_record.port_type1);
  } else if (part == UPPL_RIGHT_PART) {
    ss << oboundary_record.sw_id2 << " " << oboundary_record.port_id2
     << " " << oboundary_record.is_filled2 << " " <<
     PhyUtil::uint8tostr(oboundary_record.port_type2);
  } else {
    pfc_log_info("invalid part value in BRecordDebugPrint");
  }
  return ss.str();
}

/** update_boundary_tbl_vect
 * @Description : This function fills the switch id and port id from LP
 * and LMP map
 * @param[in] : lp_map_key-key to find from LP map
 *  bdry_iter_pos-iterator position
 *  part-indicates which section of bdry table
 * @return    : UncRespCode UNC_RC_SUCCESS if successfull or UNC_UPPL_RC_ERR*
 *              if any Read failed
 * */
UncRespCode Kt_Dataflow::update_boundary_tbl_vect(string lp_map_key,
                           uint16_t bdry_iter_pos, uint8_t part) {
  pfc_log_trace("Entered into : %s", __func__);
  map<string, lp_struct>* lp_map = get_LP_map();
  map<string, lp_struct>::iterator lp_map_iter;
  vector<boundary_record>* bdry_rec_vect = get_boundary_tbl_vect();
  uint16_t bdry_iter_pos_cpy = bdry_iter_pos;
  boundary_record obj_bdry_iter_pos;
  if (bdry_rec_vect->empty()) {
    pfc_log_error("The vector is empty");
    return UNC_UPPL_RC_FAILURE;
  }
  obj_bdry_iter_pos = (*bdry_rec_vect)[bdry_iter_pos];
  pfc_log_debug("Key to find in LP map:%s", lp_map_key.c_str());
  lp_map_iter = lp_map->find(lp_map_key);
  if (lp_map_iter == lp_map->end()) {
    pfc_log_debug("No entry in LP Map");
    return UNC_RC_SUCCESS;
  }
  pfc_log_info("Key found from LP map:%s", lp_map_key.c_str());
  if (lp_map_iter->second.port_type == UPPL_LP_SUBDOMAIN ||
     lp_map_iter->second.port_type == UPPL_LP_TRUNK_PORT ||
     lp_map_iter->second.port_type == UPPL_LP_PORT_GROUP) {
    multimap<string, lmp_struct>* lmp_map = get_LMP_map();
    multimap<string, lmp_struct>::iterator lmp_map_iter;
    std::pair<multimap<string, lmp_struct>::iterator,
              multimap<string, lmp_struct>::iterator>lmp_map_iter2;
    lmp_map_iter2 = lmp_map->equal_range(lp_map_key);
    for (lmp_map_iter = lmp_map_iter2.first;
        lmp_map_iter != lmp_map_iter2.second; lmp_map_iter++) {
      if (part == UPPL_LEFT_PART) {
        memcpy(obj_bdry_iter_pos.sw_id1, lmp_map_iter->second.switch_id,
               sizeof(obj_bdry_iter_pos.sw_id1));
        memcpy(obj_bdry_iter_pos.port_id1, lmp_map_iter->second.port_id,
               sizeof(obj_bdry_iter_pos.port_id1));
        obj_bdry_iter_pos.port_type1 = lp_map_iter->second.port_type;
        obj_bdry_iter_pos.is_filled1 = true;
        pfc_log_debug("part 1 LMP:%s",
               BRecordDebugPrint(obj_bdry_iter_pos, UPPL_LEFT_PART).c_str());
      } else if (part == UPPL_RIGHT_PART) {
        memcpy(obj_bdry_iter_pos.sw_id2, lmp_map_iter->second.switch_id,
               sizeof(obj_bdry_iter_pos.sw_id2));
        memcpy(obj_bdry_iter_pos.port_id2, lmp_map_iter->second.port_id,
               sizeof(obj_bdry_iter_pos.port_id2));
        obj_bdry_iter_pos.is_filled2 = true;
        obj_bdry_iter_pos.port_type2 = lp_map_iter->second.port_type;
        pfc_log_debug("part 2 LMP:%s",
              BRecordDebugPrint(obj_bdry_iter_pos, UPPL_RIGHT_PART).c_str());
      }
      if (lmp_map_iter == lmp_map_iter2.first) {
        (*bdry_rec_vect)[bdry_iter_pos] = obj_bdry_iter_pos;
        pfc_log_info("Copying the updated object to the vector object");
      } else {
        // Avoiding insert of first instance as already one object inserted
        bdry_rec_vect->push_back(obj_bdry_iter_pos);
        pfc_log_info("Inserting additional record for LMP port");
      }
    }
    lmp_map->clear();  // Clearing LMP map
  } else if (lp_map_iter->second.port_type == UPPL_LP_SWITCH) {
    multimap<string, port_struct>* pp_map = get_PP_map();
    multimap<string, port_struct>::iterator pp_map_iter;
    std::pair<multimap<string, port_struct>::iterator,
              multimap<string, port_struct>::iterator>pp_map_iter2;
    pp_map_iter2 = pp_map->equal_range(lp_map_key);
    for (pp_map_iter = pp_map_iter2.first;
        pp_map_iter != pp_map_iter2.second; pp_map_iter++) {
      if (part == UPPL_LEFT_PART) {
        memcpy(obj_bdry_iter_pos.sw_id1, lp_map_iter->second.switch_id,
               sizeof(obj_bdry_iter_pos.sw_id1));
        memcpy(obj_bdry_iter_pos.port_id1, pp_map_iter->second.port_id,
               sizeof(obj_bdry_iter_pos.port_id1));
        obj_bdry_iter_pos.port_type1 = lp_map_iter->second.port_type;
        obj_bdry_iter_pos.is_filled1 = true;
        pfc_log_debug("part 1 PP:%s",
                BRecordDebugPrint(obj_bdry_iter_pos, UPPL_LEFT_PART).c_str());
      } else if (part == UPPL_RIGHT_PART) {
        memcpy(obj_bdry_iter_pos.sw_id2, lp_map_iter->second.switch_id,
               sizeof(obj_bdry_iter_pos.sw_id2));
        memcpy(obj_bdry_iter_pos.port_id2, pp_map_iter->second.port_id,
               sizeof(obj_bdry_iter_pos.port_id2));
        obj_bdry_iter_pos.is_filled2 = true;
        obj_bdry_iter_pos.port_type2 = lp_map_iter->second.port_type;
        pfc_log_debug("part 2 PP:%s",
               BRecordDebugPrint(obj_bdry_iter_pos, UPPL_RIGHT_PART).c_str());
      }
      if (pp_map_iter == pp_map_iter2.first) {
        (*bdry_rec_vect)[bdry_iter_pos] = obj_bdry_iter_pos;
        pfc_log_info("Copying the updated object to the vector object");
      } else {
        // Avoiding insert of first instance as already one object inserted
        bdry_rec_vect->push_back(obj_bdry_iter_pos);
        pfc_log_info("Inserting additional record for Physical ports"
                      " of the given switch");
      }
    }
    pp_map->clear();  // Clearing PP map
  } else { /*for lp port*/
    if (part == UPPL_LEFT_PART) {
      memcpy(obj_bdry_iter_pos.sw_id1, lp_map_iter->second.switch_id,
             sizeof(obj_bdry_iter_pos.sw_id1));
      memcpy(obj_bdry_iter_pos.port_id1, lp_map_iter->second.port_id,
             sizeof(obj_bdry_iter_pos.port_id1));
      obj_bdry_iter_pos.is_filled1 = true;
      obj_bdry_iter_pos.port_type1 = lp_map_iter->second.port_type;
      pfc_log_debug("part 1 LP test print:%s",
          BRecordDebugPrint(obj_bdry_iter_pos, UPPL_LEFT_PART).c_str());
    } else if (part == UPPL_RIGHT_PART) {
      memcpy(obj_bdry_iter_pos.sw_id2, lp_map_iter->second.switch_id,
             sizeof(obj_bdry_iter_pos.sw_id2));
      memcpy(obj_bdry_iter_pos.port_id2, lp_map_iter->second.port_id,
             sizeof(obj_bdry_iter_pos.port_id2));
      obj_bdry_iter_pos.port_type2 = lp_map_iter->second.port_type;
      obj_bdry_iter_pos.is_filled2 = true;
      pfc_log_debug("part 2 LP:%s",
          BRecordDebugPrint(obj_bdry_iter_pos, UPPL_RIGHT_PART).c_str());
    }
    (*bdry_rec_vect)[bdry_iter_pos] = obj_bdry_iter_pos;
  for (uint16_t index = bdry_iter_pos+1;
      index < bdry_rec_vect->size(); ++index) {
    boundary_record obj_bdry_rec = (*bdry_rec_vect)[index];
    if (part == UPPL_LEFT_PART) {
      if ((!memcmp(obj_bdry_rec.ctr_name1, obj_bdry_iter_pos.ctr_name1,
              sizeof(obj_bdry_iter_pos.ctr_name1))) &&
         (!memcmp(obj_bdry_rec.dom_name1, obj_bdry_iter_pos.dom_name1,
                  sizeof(obj_bdry_iter_pos.dom_name1)))&&
         (!memcmp(obj_bdry_rec.lp_id1, obj_bdry_iter_pos.lp_id1,
                  sizeof(obj_bdry_iter_pos.lp_id1)))) {
        pfc_log_debug("Copying the info from already filled vector entry");
        memcpy(obj_bdry_rec.sw_id1, obj_bdry_iter_pos.sw_id1,
               sizeof(obj_bdry_rec.sw_id1));
        memcpy(obj_bdry_rec.port_id1, obj_bdry_iter_pos.port_id1,
               sizeof(obj_bdry_rec.port_id1));
        obj_bdry_rec.port_type1 = obj_bdry_iter_pos.port_type1;
        obj_bdry_rec.is_filled1 = true;
        pfc_log_debug("part 1 LP remaining:%s",
            BRecordDebugPrint(obj_bdry_iter_pos, UPPL_LEFT_PART).c_str());
      } else if ((!memcmp(obj_bdry_rec.ctr_name2, obj_bdry_iter_pos.ctr_name1,
                         sizeof(obj_bdry_iter_pos.ctr_name1))) &&
                 (!memcmp(obj_bdry_rec.dom_name2, obj_bdry_iter_pos.dom_name1,
                          sizeof(obj_bdry_iter_pos.dom_name1))) &&
                 (!memcmp(obj_bdry_rec.lp_id2, obj_bdry_iter_pos.lp_id1,
                          sizeof(obj_bdry_iter_pos.lp_id1)))) {
        pfc_log_debug("Copying the info from already filled vector entry");
        memcpy(obj_bdry_rec.sw_id2, obj_bdry_iter_pos.sw_id1,
               sizeof(obj_bdry_rec.sw_id1));
        memcpy(obj_bdry_rec.port_id2, obj_bdry_iter_pos.port_id1,
               sizeof(obj_bdry_rec.port_id1));
        obj_bdry_rec.port_type2 = obj_bdry_iter_pos.port_type1;
        obj_bdry_rec.is_filled2 = true;
      }
    } else if (part == UPPL_RIGHT_PART) {
      if ((!memcmp(obj_bdry_rec.ctr_name1, obj_bdry_iter_pos.ctr_name2,
              sizeof(obj_bdry_iter_pos.ctr_name2))) &&
         (!memcmp(obj_bdry_rec.dom_name1, obj_bdry_iter_pos.dom_name2,
                  sizeof(obj_bdry_iter_pos.dom_name2))) &&
         (!memcmp(obj_bdry_rec.lp_id1, obj_bdry_iter_pos.lp_id2,
                  sizeof(obj_bdry_iter_pos.lp_id2)))) {
        pfc_log_debug("Copying the info from already filled vector entry");
        memcpy(obj_bdry_rec.sw_id1, obj_bdry_iter_pos.sw_id2,
               sizeof(obj_bdry_rec.sw_id1));
        memcpy(obj_bdry_rec.port_id1, obj_bdry_iter_pos.port_id2,
               sizeof(obj_bdry_rec.port_id1));
        obj_bdry_rec.port_type1 = obj_bdry_iter_pos.port_type2;
        obj_bdry_rec.is_filled1 = true;
        pfc_log_info("part 2 LP remaining:%s",
            BRecordDebugPrint(obj_bdry_iter_pos, UPPL_RIGHT_PART).c_str());
      } else if ((!memcmp(obj_bdry_rec.ctr_name2, obj_bdry_iter_pos.ctr_name2,
                         sizeof(obj_bdry_iter_pos.ctr_name2))) &&
                 (!memcmp(obj_bdry_rec.dom_name2, obj_bdry_iter_pos.dom_name2,
                          sizeof(obj_bdry_iter_pos.dom_name2))) &&
                 (!memcmp(obj_bdry_rec.lp_id2, obj_bdry_iter_pos.lp_id2,
                          sizeof(obj_bdry_iter_pos.lp_id2)))) {
        pfc_log_debug("Copying the info from already filled vector entry");
        memcpy(obj_bdry_rec.sw_id2, obj_bdry_iter_pos.sw_id2,
               sizeof(obj_bdry_rec.sw_id1));
        memcpy(obj_bdry_rec.port_id2, obj_bdry_iter_pos.port_id2,
               sizeof(obj_bdry_rec.port_id1));
        obj_bdry_rec.port_type2 = obj_bdry_iter_pos.port_type2;
        obj_bdry_rec.is_filled2 = true;
      }
    }
    (*bdry_rec_vect)[index] = obj_bdry_rec;
  }
  }
  bdry_iter_pos = bdry_iter_pos_cpy;  // Setting back the position
  /*
  //  For Testing
  stringstream ss1;
  stringstream ss2;
  ss1 << obj_bdry_iter_pos.boundary_id << ";" << obj_bdry_iter_pos.ctr_name1 
      << "&" << obj_bdry_iter_pos.dom_name1
      << "&" << obj_bdry_iter_pos.sw_id1 << "&" << obj_bdry_iter_pos.port_id1;
  ss2 << obj_bdry_iter_pos.ctr_name2 << "&" << obj_bdry_iter_pos.dom_name2
      << "&" << obj_bdry_iter_pos.sw_id2 << "&" << obj_bdry_iter_pos.port_id2;
  pfc_log_info("bdry record first half inside FillSwitchId func:%s",
               ss1.str().c_str());
  pfc_log_info("bdry record second half inside FillSwitchId func:%s",
               ss2.str().c_str());
  */
  lp_map->clear();  // Clearing LP map
  return UNC_RC_SUCCESS;
}


/** PrepareCollectiveLPMap
 * @Description : This function build the LP map by reading the LP table
 * @param[in] : OdbcmConnectionHandler,
 * ctr_name-controller_name
 * dom_name-domain_name
 * @return    : UncRespCode UNC_RC_SUCCESS if successfull or UNC_UPPL_RC_ERR*
 *              if any Read failed
 * */
UncRespCode Kt_Dataflow::PrepareCollectiveLPMap(
              OdbcmConnectionHandler *db_conn, string ctr_name,
              string dom_name) {
  pfc_log_trace("Entered into : %s", __func__);
  UncRespCode read_status = UNC_RC_SUCCESS;
  map<string, lp_struct>* lp_map = get_LP_map();
  map<string, lp_struct>::iterator lp_map_iter;
  // ReadInternal of Kt_LogicalPort
  Kt_LogicalPort obj_Kt_LogicalPort;
  key_logical_port_t *obj_key_logical_port = new key_logical_port_t;
  memset(obj_key_logical_port, 0, sizeof(key_logical_port_t));
  memcpy(obj_key_logical_port->domain_key.ctr_key.controller_name,
         ctr_name.c_str(),
         ctr_name.length());
  memcpy(obj_key_logical_port->domain_key.domain_name,
         dom_name.c_str(),
         dom_name.length());
  string port_id = "";
  memcpy(obj_key_logical_port->port_id,
         port_id.c_str(),
         port_id.length());
  void* key_struct = reinterpret_cast<void*>(obj_key_logical_port);
  vector<void *> vect_key_lp;
  vect_key_lp.push_back(key_struct);
  vector<void *> vect_val_lp;

  read_status = obj_Kt_LogicalPort.ReadInternal(db_conn,
                                      vect_key_lp,
                                      vect_val_lp,
                                      UNC_DT_RUNNING,
                                      UNC_OP_READ_SIBLING_BEGIN);
  delete obj_key_logical_port;
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_info("Read of Logical Ports failed:%d", read_status);
    if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE)
      return UNC_RC_SUCCESS;
    return read_status;
  }
  // Traversing LP values and building the LP map
  unsigned int index_lp = 0;
  for (; index_lp < vect_key_lp.size();
        ++index_lp) {
    key_logical_port_t *key_lp = reinterpret_cast<key_logical_port_t *>
                                    (vect_key_lp[index_lp]);
    string logical_port_id = (const char *)key_lp->port_id;
    string ctr_name1 = (const char *)key_lp->domain_key.ctr_key.controller_name;
    string dom_name1 = (const char *)key_lp->domain_key.domain_name;
    std::stringstream ss;
    string separator = "&";
    ss << ctr_name1 << separator << dom_name1 << separator << logical_port_id;
    string lp_map_key = ss.str();
    val_logical_port_st *val_lp_st = reinterpret_cast<val_logical_port_st *>
                                    (vect_val_lp[index_lp]);

    lp_struct obj_lp_struct;
    memset(&obj_lp_struct, '\0', sizeof(obj_lp_struct));
    //  for non logical member port values
    if ((val_lp_st->logical_port.port_type != UPPL_LP_TRUNK_PORT) &&
       (val_lp_st->logical_port.port_type != UPPL_LP_SUBDOMAIN) &&
       (val_lp_st->logical_port.port_type !=  UPPL_LP_PORT_GROUP)) {
      memcpy(obj_lp_struct.switch_id, val_lp_st->logical_port.switch_id,
             sizeof(obj_lp_struct.switch_id));
      if (val_lp_st->logical_port.port_type != UPPL_LP_SWITCH)
        memcpy(obj_lp_struct.port_id, val_lp_st->logical_port.physical_port_id,
             sizeof(obj_lp_struct.port_id));
    }
       // Checking the LP type and calling ReadInternal of LMP
    if ((val_lp_st->logical_port.port_type == UPPL_LP_TRUNK_PORT) ||
       (val_lp_st->logical_port.port_type == UPPL_LP_SUBDOMAIN) ||
       (val_lp_st->logical_port.port_type ==  UPPL_LP_PORT_GROUP)) {
       string description = (const char *)val_lp_st->logical_port.description;
       if (description == UPPL_LP_MAC_FORWARDING_DESC) {
         // Clearing the key and value void* in the vector.
         pfc_log_debug("LP Description is PF MAC Forwarding SubDomain");
         ::operator delete(vect_val_lp[index_lp]);
         ::operator delete(vect_key_lp[index_lp]);
         continue;
       }
    }

    obj_lp_struct.port_type = val_lp_st->logical_port.port_type;
    lp_map->insert(std::pair<string, lp_struct>
                          (lp_map_key, obj_lp_struct));

       // Checking the LP type and calling ReadInternal of LMP
    if ((val_lp_st->logical_port.port_type == UPPL_LP_TRUNK_PORT) ||
       (val_lp_st->logical_port.port_type == UPPL_LP_SUBDOMAIN) ||
       (val_lp_st->logical_port.port_type == UPPL_LP_PORT_GROUP)) {
      read_status = PrepareLMPMap(db_conn, key_lp);
      /*if (read_status != UNC_RC_SUCCESS) {
        pfc_log_info("Read of LMP failed");
        if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE)
          return UNC_RC_SUCCESS;
        return read_status;
      }*/
    } else if (val_lp_st->logical_port.port_type == UPPL_LP_SWITCH) {
      key_port_t *key_pt = new key_port_t;
      memcpy(key_pt->sw_key.ctr_key.controller_name,
             key_lp->domain_key.ctr_key.controller_name,
             sizeof(key_lp->domain_key.ctr_key.controller_name));
      memcpy(key_pt->sw_key.switch_id, val_lp_st->logical_port.switch_id,
           sizeof(key_pt->sw_key.switch_id));
      string phy_port_id = "";
      memcpy(key_pt->port_id,
                 phy_port_id.c_str(),
                sizeof(key_pt->port_id));
      read_status = PreparePPMap(db_conn, key_pt, lp_map_key);
      delete key_pt;
      /*if (read_status != UNC_RC_SUCCESS) {
        pfc_log_info("Read of Ports failed");
        if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE)
          return UNC_RC_SUCCESS;
        return read_status;
      }*/
    }

    // Clearing the key and value void* in the vector.
     ::operator delete(vect_val_lp[index_lp]);
     ::operator delete(vect_key_lp[index_lp]);
     if (read_status != UNC_RC_SUCCESS) {
       if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE)
         continue;
       pfc_log_info("Read of Ports failed:%d", read_status);
       break;
     }
  }
  for (index_lp = index_lp + 1 ; index_lp < vect_key_lp.size();
                        ++index_lp) {
    ::operator delete(vect_val_lp[index_lp]);
    ::operator delete(vect_key_lp[index_lp]);
  }
  // Clearing the vectors
  vect_val_lp.clear();
  vect_key_lp.clear();
  /*
  for (lp_map_iter = lp_map->begin();
       lp_map_iter != lp_map->end(); lp_map_iter++) {
    string key = lp_map_iter->first;
    lp_struct lp_st = lp_map_iter->second;
    stringstream ss1;
    ss1 << key << lp_st.switch_id << " " << lp_st.port_id;
    pfc_log_debug("LP map entry:%s", ss1.str().c_str());
    pfc_log_debug("port type:%d", lp_st.port_type);
  }
  */
  if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE)
    return UNC_RC_SUCCESS;
  return read_status;
}


/** PrepareCollectiveLMPMap
 * @Description : This function build the LMP map by reading the LMP table
 * @param[in] : OdbcmConnectionHandler,
 * key_lp- LP key structure
 * @return    : UncRespCode UNC_RC_SUCCESS if successfull or UNC_UPPL_RC_ERR*
 *              if any Read failed
 * */
UncRespCode Kt_Dataflow::PrepareLMPMap(
                 OdbcmConnectionHandler *db_conn, key_logical_port_t *key_lp) {
  pfc_log_trace("Entered into : %s", __func__);
  UncRespCode read_status = UNC_RC_SUCCESS;
  multimap<string, lmp_struct>* lmp_map = get_LMP_map();
  multimap<string, lmp_struct>::iterator lmp_map_iter;
  // ReadInternal of Kt_LogicalMemberPort
  Kt_LogicalMemberPort obj_Kt_LogicalMemberPort;
  key_logical_member_port_t *obj_key_logical_member_port
       = new key_logical_member_port_t;
  memcpy(obj_key_logical_member_port->
           logical_port_key.domain_key.ctr_key.controller_name,
         key_lp->domain_key.ctr_key.controller_name,
         sizeof(obj_key_logical_member_port->
           logical_port_key.domain_key.ctr_key.controller_name));
  memcpy(obj_key_logical_member_port->logical_port_key.domain_key.domain_name,
         key_lp->domain_key.domain_name,
         sizeof(obj_key_logical_member_port->
         logical_port_key.domain_key.domain_name));
  memcpy(obj_key_logical_member_port->logical_port_key.port_id,
         key_lp->port_id,
         sizeof(obj_key_logical_member_port->logical_port_key.port_id));
  string switch_id = "";
  memcpy(obj_key_logical_member_port->switch_id,
         switch_id.c_str(),
         sizeof(obj_key_logical_member_port->switch_id));
  string phy_port_id = "";
  memcpy(obj_key_logical_member_port->physical_port_id,
         phy_port_id.c_str(),
         sizeof(obj_key_logical_member_port->physical_port_id));
  void *key_struct = reinterpret_cast<void*>(obj_key_logical_member_port);
  vector<void *> vect_key_lmp;
  vect_key_lmp.push_back(key_struct);
  vector<void *> vect_val_lmp;
  read_status = obj_Kt_LogicalMemberPort.ReadInternal(db_conn,
                                      vect_key_lmp,
                                      vect_val_lmp,
                                      UNC_DT_RUNNING,
                                      UNC_OP_READ_SIBLING_BEGIN);
  delete obj_key_logical_member_port;
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_info("Read of LMP values failed:%d", read_status);
    return read_status;
  }
  // Traversing LMP values and building the LMP map
  for (unsigned int index_lmp = 0;
      index_lmp < vect_key_lmp.size();
      ++index_lmp) {
    key_logical_member_port_t *key_lmp =
               reinterpret_cast<key_logical_member_port_t *>
                               (vect_key_lmp[index_lmp]);
    string lmp_logical_port_id =
       (const char *)key_lmp->logical_port_key.port_id;
    string lmp_controller_name =
       (const char *)key_lmp->
       logical_port_key.domain_key.ctr_key.controller_name;
    string lmp_domain_name =
       (const char *)key_lmp->logical_port_key.domain_key.domain_name;
    stringstream ss;
    string separator = "&";
    ss << lmp_controller_name << separator << lmp_domain_name << separator
       << lmp_logical_port_id;
    string lmp_map_key = ss.str();
    lmp_struct obj_lmp_struct;
    memcpy(obj_lmp_struct.switch_id, key_lmp->switch_id,
           sizeof(obj_lmp_struct.switch_id));
    memcpy(obj_lmp_struct.port_id, key_lmp->physical_port_id,
           sizeof(obj_lmp_struct.port_id));
    lmp_map->insert(std::pair<string, lmp_struct>
                          (lmp_map_key, obj_lmp_struct));
    // Clearing the key and value void* in the vector.
     ::operator delete(vect_key_lmp[index_lmp]);
  }   // end of for loop of LMP vector traversal
  /*for (lmp_map_iter = lmp_map->begin();
      lmp_map_iter != lmp_map->end(); lmp_map_iter++) {
    string key = lmp_map_iter->first;
    lmp_struct lmp_st = lmp_map_iter->second;
    stringstream ss1;
    ss1 << key << " " << lmp_st.switch_id << " " << lmp_st.port_id;
    pfc_log_debug("LMP map entry:%s", ss1.str().c_str());
  }*/
  return read_status;
}

/** PreparePPMap
 * @Description : This function build the PP map by reading the LMP table
 * @param[in] : OdbcmConnectionHandler,
 * key_lp- Port key structure
 * @return    : UncRespCode UNC_RC_SUCCESS if successfull or UNC_UPPL_RC_ERR*
 *              if any Read failed
 * */
UncRespCode Kt_Dataflow::PreparePPMap(
                 OdbcmConnectionHandler *db_conn, key_port_t *key_pt,
                 string pp_map_key) {
  pfc_log_trace("Entered into : %s", __func__);
  UncRespCode read_status = UNC_RC_SUCCESS;
  multimap<string, port_struct>* pp_map = get_PP_map();
  multimap<string, port_struct>::iterator pp_map_iter;
  // ReadInternal of Kt_Port
  Kt_Port obj_Kt_Port;
  void *key_struct = reinterpret_cast<void*>(key_pt);
  vector<void *> vect_key_pp;
  vect_key_pp.push_back(key_struct);
  vector<void *> vect_val_pp;
  read_status = obj_Kt_Port.ReadInternal(db_conn,
                                      vect_key_pp,
                                      vect_val_pp,
                                      UNC_DT_RUNNING,
                                      UNC_OP_READ_SIBLING_BEGIN);
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_info("Read of Physical Ports values failed:%d", read_status);
    return read_status;
  }
  // Traversing PP values and building the PP map
  for (unsigned int index_pp = 0;
      index_pp < vect_key_pp.size();
      ++index_pp) {
    key_port_t *key_pp =
               reinterpret_cast<key_port_t *>
                               (vect_key_pp[index_pp]);
    port_struct obj_port_struct;
    memcpy(obj_port_struct.port_id, key_pp->port_id,
           sizeof(obj_port_struct.port_id));
    pp_map->insert(std::pair<string, port_struct>
                          (pp_map_key, obj_port_struct));
    // Clearing the key and value void* in the vector.
    ::operator delete(vect_key_pp[index_pp]);
    ::operator delete(vect_val_pp[index_pp]);
  }   // end of for loop of PP vector traversal
  // Clearing the vectors
  vect_val_pp.clear();
  vect_key_pp.clear();
  /*
  for (pp_map_iter = pp_map->begin();
      pp_map_iter != pp_map->end(); pp_map_iter++) {
    string key = pp_map_iter->first;
    port_struct pp_st = pp_map_iter->second;
    stringstream ss1;
    ss1 << key << " " << pp_st.port_id;
    pfc_log_info("PP map entry:%s", ss1.str().c_str());
  }
  */
  return read_status;
}


/** fill_boundary_map
 * @Description : This function build the Bdry map by traversiong the vector
 * @param[in] : OdbcmConnectionHandler,
 * @return    : UncRespCode UNC_RC_SUCCESS if successfull or UNC_UPPL_RC_ERR*
 *              if any Read failed
 * */
UncRespCode Kt_Dataflow::fill_boundary_map(
                        OdbcmConnectionHandler* db_conn) {
  pfc_log_trace("Entered into : %s", __func__);
  UncRespCode ret_code = UNC_RC_SUCCESS;
  multimap<string, boundary_val>* bdry_map = get_boundary_map();
  multimap<string, boundary_val>::iterator bdry_map_iter;
  vector<boundary_record>* bdry_rec_vect = get_boundary_tbl_vect();
  vector<boundary_record>::iterator bdry_rec_vec_iter;
  map<string, uint8_t> ctr_type_map;
  string ctr_name, dom_name, lp_id, sw_id, port_id = "";
  string separator = "&";
  string mapkey_str = "";
  std::stringstream ss;
  ss.str("");
  boundary_val obj_bdry_val;
  memset(&obj_bdry_val, '\0', sizeof(obj_bdry_val));
  //  Traversing the BDRY values and building the map
  for (unsigned int index = 0;
        index < bdry_rec_vect->size();
        ++index) {
    ss.str("");
    mapkey_str = "";
    boundary_record bdry_rec_st = (*bdry_rec_vect)[index];
    ctr_name = (const char *)bdry_rec_st.ctr_name1;
    dom_name = (const char *)bdry_rec_st.dom_name1;
    sw_id = (const char *)bdry_rec_st.sw_id1;
    port_id = (const char *)bdry_rec_st.port_id1;
    ss << ctr_name << separator << dom_name << separator
                          << sw_id << separator << port_id;
    mapkey_str = ss.str();
    string ctr2_name = (const char*)bdry_rec_st.ctr_name2;
    unc_keytype_ctrtype_t ctr_type = UNC_CT_UNKNOWN;
    if (ctr_type_map.find(ctr2_name) == ctr_type_map.end()) {
      ret_code = PhyUtil::get_controller_type(
               db_conn, ctr2_name,
               ctr_type,
              (unc_keytype_datatype_t)UNC_DT_RUNNING);
      pfc_log_debug("Controller type - return code %d, value %s",
                  ret_code, ctr2_name.c_str());
      if (ret_code != UNC_RC_SUCCESS) {
        pfc_log_error("error in getting the controller type: %d", ret_code);
        if (ret_code == UNC_UPPL_RC_ERR_DB_ACCESS) {
          pfc_log_debug("Returning as there is error in DB access");
          return ret_code;
        } else {
          ret_code = UNC_RC_SUCCESS;
          continue;
        }
      }
      ctr_type_map.insert(std::pair<string, uint8_t>(ctr2_name, ctr_type));
      obj_bdry_val.controller_type = ctr_type;
    } else {
      obj_bdry_val.controller_type = (ctr_type_map.find(ctr2_name))->second;
    }
    memcpy(obj_bdry_val.controller_name, bdry_rec_st.ctr_name2,
           sizeof(obj_bdry_val.controller_name));
    memcpy(obj_bdry_val.domain_name, bdry_rec_st.dom_name2,
           sizeof(obj_bdry_val.domain_name));
    memcpy(obj_bdry_val.logical_port_id, bdry_rec_st.lp_id2,
           sizeof(obj_bdry_val.logical_port_id));
    memcpy(obj_bdry_val.lp_str.switch_id, bdry_rec_st.sw_id2,
           sizeof(obj_bdry_val.lp_str.switch_id));
    memcpy(obj_bdry_val.lp_str.port_id, bdry_rec_st.port_id2,
           sizeof(obj_bdry_val.lp_str.port_id));
    obj_bdry_val.lp_str.port_type = bdry_rec_st.port_type2;
    memcpy(obj_bdry_val.boundary_id, bdry_rec_st.boundary_id,
                           sizeof(bdry_rec_st.boundary_id));
    bool isPresentAlready = false;
    multimap<string, boundary_val>::iterator tmp_iter;
    for (tmp_iter = bdry_map->begin(); tmp_iter != bdry_map->end();
                tmp_iter ++) {
      if (tmp_iter->first == mapkey_str) {
        bool comp_status = false;
        UPPL_COMPARE_STRUCT(tmp_iter->second, obj_bdry_val, comp_status);
        if (comp_status == true) {
          isPresentAlready = true;
          break;
        }
      }
    }
    if (isPresentAlready == false) {
      bdry_map->insert(std::pair<string, boundary_val>
                             (mapkey_str, obj_bdry_val));
    }
    memset(&obj_bdry_val, '\0', sizeof(obj_bdry_val));
    ctr_name = (const char *)bdry_rec_st.ctr_name2;
    dom_name = (const char *)bdry_rec_st.dom_name2;
    sw_id = (const char *)bdry_rec_st.sw_id2;
    port_id = (const char *)bdry_rec_st.port_id2;
    ss.str("");
    mapkey_str = "";
    ss << ctr_name << separator << dom_name << separator
                       << sw_id << separator << port_id;
    mapkey_str = ss.str();
    ctr2_name = (const char*)bdry_rec_st.ctr_name1;
    ctr_type = UNC_CT_UNKNOWN;
    if (ctr_type_map.find(ctr2_name) == ctr_type_map.end()) {
      ret_code = PhyUtil::get_controller_type(
               db_conn, ctr2_name,
               ctr_type,
              (unc_keytype_datatype_t)UNC_DT_RUNNING);
      pfc_log_debug("Controller type - return code %d, value %s",
                ret_code, ctr2_name.c_str());
      if (ret_code != UNC_RC_SUCCESS) {
        pfc_log_error("error in getting the controller type: %d", ret_code);
        if (ret_code == UNC_UPPL_RC_ERR_DB_ACCESS) {
          pfc_log_debug("Returning as there is error in DB Access");
          return ret_code;
        } else  {
          ret_code = UNC_RC_SUCCESS;
          continue;
        }
      }
      ctr_type_map.insert(std::pair<string, uint8_t>(ctr2_name, ctr_type));
      obj_bdry_val.controller_type = (uint8_t)ctr_type;
    } else {
      obj_bdry_val.controller_type = (ctr_type_map.find(ctr2_name))->second;
    }
    memcpy(obj_bdry_val.controller_name, bdry_rec_st.ctr_name1,
           sizeof(obj_bdry_val.controller_name));
    memcpy(obj_bdry_val.domain_name, bdry_rec_st.dom_name1,
           sizeof(obj_bdry_val.domain_name));
    memcpy(obj_bdry_val.logical_port_id, bdry_rec_st.lp_id1,
           sizeof(obj_bdry_val.logical_port_id));
    memcpy(obj_bdry_val.lp_str.switch_id, bdry_rec_st.sw_id1,
           sizeof(obj_bdry_val.lp_str.switch_id));
    memcpy(obj_bdry_val.lp_str.port_id, bdry_rec_st.port_id1,
           sizeof(obj_bdry_val.lp_str.port_id));
    obj_bdry_val.lp_str.port_type = bdry_rec_st.port_type1;
    memcpy(obj_bdry_val.boundary_id, bdry_rec_st.boundary_id,
                           sizeof(bdry_rec_st.boundary_id));

    isPresentAlready = false;
    for (tmp_iter = bdry_map->begin(); tmp_iter != bdry_map->end();
          tmp_iter ++) {
      if (tmp_iter->first == mapkey_str) {
        bool comp_status = false;
        UPPL_COMPARE_STRUCT(tmp_iter->second, obj_bdry_val, comp_status);
        if (comp_status == true) {
          isPresentAlready = true;
          break;
        }
      }
    }
    if (isPresentAlready == false) {
      bdry_map->insert(std::pair<string, boundary_val>
                            (mapkey_str, obj_bdry_val));
    }
  }  // End of traversal of Bdry vect values
  bdry_rec_vect->clear();
  ctr_type_map.clear();
  // For Testing printing boundary map
  for (bdry_map_iter = bdry_map->begin(); bdry_map_iter != bdry_map->end();
       bdry_map_iter ++) {
    pfc_log_debug("Key:%s", (bdry_map_iter->first).c_str());
    stringstream test;
    char typeCh = static_cast<char>(bdry_map_iter->second.controller_type + 48);
    test << bdry_map_iter->second.controller_name << "..." << typeCh
         << "..." << bdry_map_iter->second.domain_name << "..."
         << bdry_map_iter->second.logical_port_id << "..."
         << bdry_map_iter->second.lp_str.switch_id << "..."
         << bdry_map_iter->second.lp_str.port_id << "..."
         << bdry_map_iter->second.boundary_id << "..."
         << PhyUtil::uint8tostr(bdry_map_iter->second.lp_str.port_type);
    pfc_log_debug("Value:%s", test.str().c_str());
  }
  return ret_code;
}

/**fill_ctrlr_dom_count_map 
 * @Description : 
 * @param[in] : 
 * @return    : 
 *              
 * */

UncRespCode Kt_Dataflow::fill_ctrlr_dom_count_map(
              OdbcmConnectionHandler *db_conn, string ctr_name) {
  map <std::string, uint32_t> *count_map = df_util_.get_ctrlr_dom_count_map();
  map <std::string, uint32_t>::iterator cmap_iter;
  if (ctr_name.empty())
    return UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST;
  pfc_log_debug("controller name recv. %s:%s ", __func__, ctr_name.c_str());
  cmap_iter = count_map->find(ctr_name);
  if (cmap_iter != count_map->end()) {
    pfc_log_debug("domain count of this controller is already exists ");
    return UNC_RC_SUCCESS;
  }
  DBTableSchema kt_ctr_domain_dbtableschema;
  uint32_t count = 0;
  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, ctr_name,
                        ctr_name.length(),
                        DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  string domain_name = "";
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  vect_prim_keys.push_back(DOMAIN_NAME_STR);
  kt_ctr_domain_dbtableschema.set_table_name(unc::uppl::CTR_DOMAIN_TABLE);
  kt_ctr_domain_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_ctr_domain_dbtableschema.set_row_list(row_list);
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  ODBCM_RC_STATUS db_status =
      physical_layer->get_odbc_manager()->GetSiblingCount(UNC_DT_RUNNING,
                kt_ctr_domain_dbtableschema, count, db_conn);
  if (db_status != ODBCM_RC_SUCCESS)
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  count = count * 2;  //  To allow the same boundary traversal for hairpin L3
                      //  the domain count is doubled here.
  count_map->insert(std::pair<std::string, uint32_t>(ctr_name, count));
  pfc_log_debug("domain count (after doubled) of this controller is added"
                                           " %s: %d ", ctr_name.c_str(), count);

  // For tetsing, printinig ctrlr_dom_count_map
  for (cmap_iter = count_map->begin(); cmap_iter != count_map->end();
       cmap_iter++) {
    pfc_log_debug("key-ctr_name:%s-value-dom_count:%d",
                  (cmap_iter->first).c_str(), cmap_iter->second);
  }
  if (count_map->empty())
    return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  return UNC_RC_SUCCESS;
}
/**
 * @Description :
 * @param[in] :
 * @return    :
 *
 * */
UncRespCode Kt_Dataflow::getkeysfrom_boundary_map(string ctr_name,
                               list<boundary_val> &found_keys,
                               list<boundary_val> &found_vals,
                               string ingress_domain_name,
                               uint8_t ctr_type) {
  multimap<string, boundary_val> *bdry_map  = get_boundary_map();
  multimap<string, boundary_val>::iterator bdry_map_iter = bdry_map->begin();
  size_t pos = 0;
  uint8_t w_count;
  string totalF, word, delimiter = "&";
  pfc_log_debug("input ctr_name:%s ingress domain name:%s",
                  ctr_name.c_str(), ingress_domain_name.c_str());
  for (; bdry_map_iter != bdry_map->end(); bdry_map_iter++) {
    //  non PFC
    totalF = (*bdry_map_iter).first;
    if (totalF.empty())
      return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    pfc_log_debug("bdry_map.key is :%s ",  totalF.c_str());
    pos = totalF.find(delimiter);
    word = totalF.substr(0, pos);  /* controller_name*/
    if (ctr_name.compare(word) != 0) {
      pfc_log_debug("controller name not matching");
      continue;
    }
    w_count = 0;
    boundary_val obj_boundary_val;
    memset(&obj_boundary_val, '\0', sizeof(boundary_val));
    /*controller_name&domain_name&switch_id&port_id*/
    while (w_count < 4/*key has only four values */) {
      pfc_log_debug("w_count=%d totalF=%s", w_count, totalF.c_str());
      pos = totalF.find(delimiter);
      word = totalF.substr(0, pos);
      // filling boundary_val strucure
      if (w_count == 0)
        memcpy(&obj_boundary_val.controller_name, word.c_str(), word.length());
      else if (w_count == 1)
        memcpy(&obj_boundary_val.domain_name, word.c_str(), word.length());
      else if (w_count == 2)
        memcpy(&obj_boundary_val.lp_str.switch_id, word.c_str(), word.length());
      else
        memcpy(obj_boundary_val.lp_str.port_id, word.c_str(), word.length());
      totalF.erase(0, pos + delimiter.length());
      w_count++;
    }
    obj_boundary_val.controller_type = ctr_type;
    if (((obj_boundary_val.controller_type == UNC_CT_VNP) ||
         (obj_boundary_val.controller_type == UNC_CT_POLC) ||
        (obj_boundary_val.controller_type == UNC_CT_UNKNOWN)) &&
        (strcmp((const char*)obj_boundary_val.domain_name,
                ingress_domain_name.c_str()) == 0)) {
      found_keys.push_back(obj_boundary_val);
      found_vals.push_back((*bdry_map_iter).second);
    }
  }
  if (found_keys.size() == 0)
    return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  return UNC_RC_SUCCESS;
}

UncRespCode Kt_Dataflow::checkFlowLimitAndTraverse(
                                         OdbcmConnectionHandler *db_conn,
                                         uint32_t session_id,
                                         uint32_t configuration_id,
                                         ServerSession &sess,
                                         void* key_struct,
                                         vector<DataflowCmn*>* node,
                                         bool is_head_node,
                                         string &ingress_bdry_id) {
    vector<DataflowCmn *>::iterator iter_flow = node->begin();
    while (iter_flow != node->end()) {
      if ((*iter_flow)->df_segment->df_common->controller_type != UNC_CT_PFC &&
          (*iter_flow)->df_segment->df_common->controller_type != UNC_CT_ODC) {
        iter_flow++;
        pfc_log_trace("Inside the != PFC if cond.");
        continue;
      }
        // Checking the particular flow is traversed
      DataflowCmn *traverse_flow_cmn =
                            reinterpret_cast<DataflowCmn *>(*iter_flow);
      pfc_log_debug("controller_name:%s",
               (const char*)
               ((*iter_flow)->df_segment->df_common->controller_name));
      if (traverse_flow_cmn->addl_data->reason !=
           UNC_DF_RES_EXCEEDS_FLOW_LIMIT) {
        UncRespCode ret_code = checkBoundaryAndTraverse(db_conn, session_id,
                     configuration_id, key_struct, sess, false,
                     traverse_flow_cmn, traverse_flow_cmn, ingress_bdry_id);
        pfc_log_debug("checkBoundaryAndTraverse returned %d", ret_code);
        if (ret_code == UNC_UPPL_RC_ERR_DB_ACCESS) {
          return ret_code;
        }
        vector<DataflowCmn *>::iterator match_flow_iter =  iter_flow + 1;
        unsigned int no_of_dataflow = 1;
        while (match_flow_iter != node->end()) {
          DataflowCmn *traverse_match_flow_cmn =
                                 reinterpret_cast<DataflowCmn *>
                                  (*match_flow_iter);
          if ((traverse_flow_cmn->next.size() > 0) &&
              (traverse_match_flow_cmn->addl_data->reason !=
                  UNC_DF_RES_EXCEEDS_FLOW_LIMIT)) {
              pfc_log_trace("Need to compare subsequent flows");

            if (traverse_match_flow_cmn->CompareDataflow(traverse_flow_cmn) ==
                     true) {
              no_of_dataflow++;
              pfc_log_debug("CompareDataflow returns true, no_of_dataflow=%d"
                " max_dataflow_traverse_count_=%d",
                  no_of_dataflow, max_dataflow_traverse_count_);
              if (no_of_dataflow > max_dataflow_traverse_count_) {
                traverse_match_flow_cmn->addl_data->reason =
                                               UNC_DF_RES_EXCEEDS_FLOW_LIMIT;
                pfc_log_debug("Setting reason UNC_DF_RES_EXCEEDS_FLOW_LIMIT");
                if (is_head_node)
                  traverse_match_flow_cmn->addl_data->controller_count = 1;
              }
            }
          }
          match_flow_iter++;
        }
      }
      iter_flow++;
    }
    return UNC_RC_SUCCESS;
}

UncDataflowReason Kt_Dataflow::CreateDfCmnNodeForNonPfc(
                                          OdbcmConnectionHandler *db_conn,
                                          DataflowDetail *df_segment,
                                          DataflowCmn *source_node,
                                          DataflowCmn *df_cmn,
                                          boundary_val *ingress_obj_bval,
                                          boundary_val &egress_obj_bval,
                                          bool is_egress,
                                          UncRespCode &err_code) {
  string ctr_name = (const char*)ingress_obj_bval->controller_name;
  pfc_log_info("Creating DataflowCmn node for %s", ctr_name.c_str());
  memcpy(df_cmn->df_segment->df_common->controller_name,
  ingress_obj_bval->controller_name,
             sizeof(ingress_obj_bval->controller_name));
  df_cmn->df_segment->df_common->controller_type =
             ingress_obj_bval->controller_type;
  memcpy(df_cmn->df_segment->df_common->in_domain,
                  ingress_obj_bval->domain_name,
                  sizeof(ingress_obj_bval->domain_name));
  if (ingress_obj_bval->controller_type != UNC_CT_UNKNOWN) {
    memcpy(df_cmn->df_segment->df_common->ingress_switch_id,
             ingress_obj_bval->lp_str.switch_id,
             sizeof(ingress_obj_bval->lp_str.switch_id));
    memcpy(df_cmn->df_segment->df_common->in_port,
                ingress_obj_bval->lp_str.port_id,
                sizeof(ingress_obj_bval->lp_str.port_id));
  }
  if (is_egress) {
    memcpy(df_cmn->df_segment->df_common->out_domain,
            egress_obj_bval.domain_name,
             sizeof(egress_obj_bval.domain_name));
    memcpy(df_cmn->df_segment->df_common->egress_switch_id,
             egress_obj_bval.lp_str.switch_id,
             sizeof(egress_obj_bval.lp_str.switch_id));
    memcpy(df_cmn->df_segment->df_common->out_port,
             egress_obj_bval.lp_str.port_id,
             sizeof(egress_obj_bval.lp_str.port_id));
  } else {
    if (df_cmn->df_segment->df_common->controller_type == UNC_CT_VNP ||
        df_cmn->df_segment->df_common->controller_type == UNC_CT_POLC) {
             df_cmn->df_segment->df_common->
                    valid[kidxDfDataFlowEgressSwitchId] = 0;
             df_cmn->df_segment->df_common->valid[kidxDfDataFlowOutPort] = 0;
             df_cmn->df_segment->df_common->valid[kidxDfDataFlowOutDomain] = 0;
    } else if (
      df_cmn->df_segment->df_common->controller_type ==
                    UNC_CT_UNKNOWN) {
           // Setting valid bit for out_domain as invalid
      df_cmn->df_segment->df_common->valid[kidxDfDataFlowOutDomain] = 0;
    }
  }
  pfc_log_debug("node:%s",
  DataflowCmn::get_string(*df_cmn->df_segment->df_common).c_str());
  err_code = fill_ctrlr_dom_count_map(db_conn, ctr_name);
  if (err_code != UNC_RC_SUCCESS) {
    pfc_log_debug("Controller-Domain Count Map is not filled:%d", err_code);
    return UNC_DF_RES_SYSTEM_ERROR;
  }
  pfc_log_debug("Return of fill_ctrlr_dom_count_map:%d", err_code);
  UncDataflowReason ret = source_node->appendFlow(
                    df_cmn, *(df_util_.get_ctrlr_dom_count_map()));
  if (ret != UNC_DF_RES_SUCCESS) {
     pfc_log_debug("Flow not appended ret=%d", ret);
     return ret;
  }
  df_cmn->parent_node = source_node;

  if (is_egress == false) {
    if ((df_cmn->next.size() == 0) &&
       (df_cmn->addl_data->reason == UNC_DF_RES_SUCCESS))
    df_cmn->addl_data->reason = UNC_DF_RES_DST_NOT_REACHED;
  }
  pfc_log_debug("aNode is appended to flow");
  return UNC_DF_RES_SUCCESS;
}

