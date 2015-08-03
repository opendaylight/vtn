/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <vtn_drv_module.hh>
#include <request_template.hh>
#include <read_handler.hh>

namespace unc {
namespace driver {

std::map<unc_key_type_t, pfc_ipcstdef_t*>VtnDrvIntf::key_map;
std::map<unc_key_type_t, pfc_ipcstdef_t*>VtnDrvIntf::val_map;
/**
 * @brief     : constructor
 */
VtnDrvIntf::VtnDrvIntf(const pfc_modattr_t* attr)
: Module(attr), taskq_(NULL), ctrl_inst_(NULL) {
  ODC_FUNC_TRACE;
  memset(&conf_parser_, 0, sizeof(conf_info));
}

/**
 * VtnDrvIntf():
 * @brief     : destructor
 */
VtnDrvIntf::~VtnDrvIntf() {
  ODC_FUNC_TRACE;
  std::map <unc_key_type_t, unc::driver::KtHandler*> ::iterator map_it;
  for (map_it = map_kt_.begin(); map_it != map_kt_.end(); map_it++) {
       delete (map_it)->second;
  }
  map_kt_.clear();
}

/**
 * @brief     : This Function is called to load the vtndrvintf module
 * @param[in] : None
 * @retval    : PFC_FALSE/PFC_TRUE
 */
pfc_bool_t VtnDrvIntf::init(void) {
  ODC_FUNC_TRACE;
  read_conf_file();
  uint32_t concurrency = 2;
  taskq_ = pfc::core::TaskQueue::create(concurrency);
  ControllerFramework *ctrl_inst_= new ControllerFramework(taskq_,
                                        conf_parser_.time_interval);
  PFC_ASSERT(ctrl_inst_ != NULL);
  set_controller_instance(ctrl_inst_);
  initialize_map();

  create_handler<key_root_t, val_root_t>(UNC_KT_ROOT);
  create_handler<key_ctr_t, val_ctr_commit_ver_t>(UNC_KT_CONTROLLER);
  create_handler<key_vtn_t, val_vtn_t>(UNC_KT_VTN);
  create_handler<key_vbr_t, val_vbr_t>(UNC_KT_VBRIDGE);
  create_handler<key_vbr_if_t, pfcdrv_val_vbr_if_t>
      (UNC_KT_VBR_IF);
  create_handler<key_vlan_map_t, pfcdrv_val_vlan_map_t>
      (UNC_KT_VBR_VLANMAP);
  create_handler<key_flowlist, val_flowlist>
      (UNC_KT_FLOWLIST);
  create_handler<key_flowlist_entry, val_flowlist_entry>
      (UNC_KT_FLOWLIST_ENTRY);
  create_handler<key_vtn_flowfilter, val_flowfilter>
      (UNC_KT_VTN_FLOWFILTER);
  create_handler<key_vtn_flowfilter_entry, val_vtn_flowfilter_entry>
      (UNC_KT_VTN_FLOWFILTER_ENTRY);
  create_handler<key_vbr_flowfilter, val_flowfilter>
      (UNC_KT_VBR_FLOWFILTER);
  create_handler<key_vbr_flowfilter_entry, val_flowfilter_entry>
      (UNC_KT_VBR_FLOWFILTER_ENTRY);
  create_handler<key_vbr_if_flowfilter, pfcdrv_val_vbrif_vextif>
      (UNC_KT_VBRIF_FLOWFILTER);
  create_handler<key_vbr_if_flowfilter_entry, pfcdrv_val_flowfilter_entry>
      (UNC_KT_VBRIF_FLOWFILTER_ENTRY);
  create_handler<key_vterm, val_vterm>(UNC_KT_VTERMINAL);
  create_handler<key_vterm_if, val_vterm_if>(UNC_KT_VTERM_IF);
  create_handler<key_vterm_if_flowfilter, val_flowfilter>
      (UNC_KT_VTERMIF_FLOWFILTER);
  create_handler<key_vterm_if_flowfilter_entry, val_flowfilter_entry>
      (UNC_KT_VTERMIF_FLOWFILTER_ENTRY);


  unc::tclib::TcLibModule* tclib_obj =
      static_cast<unc::tclib::TcLibModule*>(pfc::core::Module::getInstance(
              "tclib"));
  if (NULL == tclib_obj) {
    pfc_log_error("tclib getInstance failed");
    return PFC_FALSE;
  }

  tclib_obj->TcLibRegisterHandler(new DriverTxnInterface(ctrl_inst_,
                                                          map_kt_));

  return PFC_TRUE;
}


/**
 * @brief     : This Function is called to unload the vtndrvintf module
 * @param[in] : None
 * @retval    : PFC_TRUE
 */
pfc_bool_t VtnDrvIntf::fini(void) {
  ODC_FUNC_TRACE;

  std::map<unc_key_type_t, pfc_ipcstdef_t*> :: iterator map_key;
  for (map_key = key_map.begin(); map_key != key_map.end(); map_key++) {
    delete map_key->second;
    map_key->second = NULL;
  }
  for (map_key = val_map.begin(); map_key != val_map.end(); map_key++) {
    delete map_key->second;
    map_key->second = NULL;
  }

  if (!key_map.empty()) {
    key_map.clear();
  }
  if (!val_map.empty()) {
    val_map.clear();
  }
  if (taskq_) {
    delete taskq_;
    taskq_ = NULL;
  }

  if (ctrl_inst_) {
    delete ctrl_inst_;
    ctrl_inst_ = NULL;
  }
  if (!map_kt_.empty()) {
    map_kt_.clear();
  }
  return PFC_TRUE;
}


/**
 * @brief     : This Function recevies the ipc request and process the same
 * @param[in] : sess, service
 * @retval    : PFC_IPCRESP_FATAL/PFC_IPCINT_EVSESS_OK
 */
pfc_ipcresp_t VtnDrvIntf::ipcService(pfc::core::ipc::ServerSession &sess,
                                     pfc_ipcid_t service) {
  ODC_FUNC_TRACE;

  pfc::core::ipc::ServerSession* p_sess=&sess;
  odl_drv_request_header_t request_hdr;
  UncRespCode resp_code = UNC_DRV_RC_ERR_GENERIC;
  memset(&request_hdr, 0, sizeof(request_hdr));
  VtnDrvRetEnum result = get_request_header(p_sess, request_hdr);

  if (VTN_DRV_RET_SUCCESS != result) {
    pfc_log_error("VtnDrvIntfservice::%s Failed to get argument err=%d",
                 PFC_FUNCNAME, result);
    return PFC_IPCRESP_FATAL;
  }

  if ((request_hdr.header.operation == UNC_OP_READ) &&
      (request_hdr.key_type != UNC_KT_ROOT) &&
      (request_hdr.key_type != UNC_KT_CONTROLLER)) {
    // Read Request support
    //
    pfc_log_info("Read Request");
    KtHandler *read_handler = new unc::driver::KtReadRequestHandler();

    resp_code = read_handler->handle_request(sess, request_hdr, ctrl_inst_);
    if (resp_code != UNC_RC_SUCCESS) {
      pfc_log_debug("handle_request fail for key:%d", request_hdr.key_type);
      return PFC_IPCRESP_FATAL;
    }

  } else {
    // Not Read request

    if (map_kt_.empty()) {
      pfc_log_debug("map_kt empty");
      return PFC_IPCRESP_FATAL;
    }
    KtHandler *hnd_ptr = get_kt_handler(request_hdr.key_type);

    if (NULL == hnd_ptr) {
      pfc_log_debug("Key Type Not Supported %d", request_hdr.key_type);
      return PFC_IPCRESP_FATAL;
    }

    // Set Timeout as infinite for audit operation
    if (request_hdr.key_type == UNC_KT_ROOT) {
      sess.setTimeout(NULL);
    }

    resp_code = hnd_ptr->handle_request(sess, request_hdr, ctrl_inst_);
    if (resp_code != UNC_RC_SUCCESS) {
      pfc_log_debug("handle_request fail for key:%d", request_hdr.key_type);
      return PFC_IPCRESP_FATAL;
    }
  }
  return UNC_RC_SUCCESS;
}


/**
 * @brief     : This function parse the session and fills
 *              odl_drv_request_header_t
 * @param[in] : sess, request_hdr
 * @retval    : VTN_DRV_RET_FAILURE /VTN_DRV_RET_SUCCESS
 */
VtnDrvRetEnum VtnDrvIntf::get_request_header(
                                  pfc::core::ipc::ServerSession* sess,
                                  odl_drv_request_header_t &request_hdr) {
  ODC_FUNC_TRACE;

  PFC_ASSERT(sess != NULL);
  uint32_t err = 0, keytype;
  err = sess->getArgument(IPC_SESSION_ID_INDEX,
                          request_hdr.header.session_id);

  if (err) {
    pfc_log_error("Failed to receive client session id: (err = %d)", err);
    return VTN_DRV_RET_FAILURE;
  }

  err = sess->getArgument(IPC_CONFIG_ID_INDEX,
                          request_hdr.header.config_id);

  if (err) {
    pfc_log_error("Failed to receive configurationid: (err = %d)", err);
    return VTN_DRV_RET_FAILURE;
  }

  const char *ctr_name;
  err = sess->getArgument(IPC_CONTROLLER_ID_INDEX, ctr_name);

  if (err) {
    pfc_log_error("Failed to receive controller id: (err = %d)", err);
    return VTN_DRV_RET_FAILURE;
  }

  strncpy(reinterpret_cast<char*>(request_hdr.controller_name), ctr_name,
      sizeof(request_hdr.controller_name) - 1);


  const char *domain_name;
  err = sess->getArgument(IPC_DOMAIN_ID_INDEX, domain_name);

  if (err) {
    pfc_log_error("Failed to receive domain id: (err = %d)", err);
    return VTN_DRV_RET_FAILURE;
  }

  strncpy(reinterpret_cast<char*>(request_hdr.header.domain_id), domain_name,
      sizeof(request_hdr.header.domain_id) - 1);

  err = sess->getArgument(IPC_OPERATION_INDEX,
                          request_hdr.header.operation);

  if (err) {
    pfc_log_error("Failed to receive operation: (err = %d)", err);
    return VTN_DRV_RET_FAILURE;
  }

  err = sess->getArgument(IPC_OPTION1_INDEX,
                          request_hdr.header.option1);

  if (err) {
    pfc_log_error("Failed to receive option1 : (err = %d)", err);
    return VTN_DRV_RET_FAILURE;
  }

  err = sess->getArgument(IPC_OPTION2_INDEX,
                          request_hdr.header.option2);

  if (err) {
    pfc_log_error("Failed to receive option2 : (err = %d)", err);
    return VTN_DRV_RET_FAILURE;
  }

  err = sess->getArgument(IPC_DATA_TYPE_INDEX,
                          request_hdr.header.data_type);

  if (err) {
    pfc_log_error("Failed to receive data_type: (err = %d)", err);
    return VTN_DRV_RET_FAILURE;
  }


  err = sess->getArgument(IPC_KEY_TYPE_INDEX, keytype);

  if (err) {
    pfc_log_error("Failed to receive key type: (err = %d)", err);
    return VTN_DRV_RET_FAILURE;
  }

  request_hdr.key_type=(unc_key_type_t)keytype;
  pfc_log_debug("Keytype is %d", request_hdr.key_type);
  return VTN_DRV_RET_SUCCESS;
}

/**
 * @brief     : This Function  returns the  kt_handler for
 *              the appropriate key types
 * @param[in] : key type
 * @retval    : KtHandler*
 */
KtHandler* VtnDrvIntf::get_kt_handler(unc_key_type_t kt) {
  std::map <unc_key_type_t, unc::driver::KtHandler*>:: iterator
      iter = map_kt_.begin();
  iter =  map_kt_.find(kt);
  if (iter != map_kt_.end()) {
    return iter->second;
  }
  return NULL;
}

/**
 * @brief     : This Function  returns the  kt_handler for
 *              the appropriate key types
 * @param[in] : key type
 * @retval    : KtHandler*
 */
KtHandler* VtnDrvIntf::get_read_kt_handler(unc_key_type_t kt) {
  std::map <unc_key_type_t, unc::driver::KtHandler*>:: iterator
      iter = read_map_kt_.begin();
  iter =  read_map_kt_.find(kt);
  if (iter != read_map_kt_.end()) {
    return iter->second;
  }
  return NULL;
}


/**
 * @brief     : This Function is called to register the driver handler
 *              with respect to the controller type
 * @param[in] : *drvobj
 * @retval    : VTN_DRV_RET_SUCCESS
 */
VtnDrvRetEnum VtnDrvIntf:: register_driver(driver *drv_obj) {
  if (NULL == drv_obj) {
    pfc_log_error("driver handler is NULL");
    return VTN_DRV_RET_FAILURE;
  }
  unc_keytype_ctrtype_t ctr_type = drv_obj->get_controller_type();
  ctrl_inst_->RegisterDriver(ctr_type, drv_obj);
  return VTN_DRV_RET_SUCCESS;
}

/**
 * @brief     - Read configuration file of vtndrvintf
 * @param[in] - None
 * @return    - None
 */
void VtnDrvIntf::read_conf_file() {
  ODC_FUNC_TRACE;
  pfc::core::ModuleConfBlock drv_block(timeinterval_conf_blk);
  if (drv_block.getBlock() != PFC_CFBLK_INVALID) {
    conf_parser_.time_interval =
           drv_block.getUint32("physical_attributes_read_interval",
                                        default_time_interval);
    pfc_log_debug("%s: Block Handle is Valid, Time interval %d", PFC_FUNCNAME,
                  conf_parser_.time_interval);
  } else {
    conf_parser_.time_interval = default_time_interval;
    pfc_log_debug("%s: Block Handle is Invalid, set default Value %d",
                  PFC_FUNCNAME, conf_parser_.time_interval);
  }
}

/**
 * @brief     : Method to post domain create events to UPPL
 * @param[in] : ctr_name, Domain_name
 * @retval    : None
 */
void VtnDrvIntf::domain_event(std::string controller_name,
                              std::string domain_name) {
  ODC_FUNC_TRACE;

  unc_event_mask_t mask_type = UNC_PHYSICAL_EVENTS;
  int err = 0;
  uint32_t controller_id_size = 0;
  uint32_t domain_id_size = 0;
  key_ctr_domain_t key_ctr_domain;
  val_ctr_domain_st val_ctr_domain_status;

  controller_id_size = sizeof(key_ctr_domain.ctr_key.controller_name);
  memset(&key_ctr_domain, 0, sizeof(key_ctr_domain_t));
  memset(&val_ctr_domain_status, 0, sizeof(val_ctr_domain_st));
  memset(key_ctr_domain.ctr_key.controller_name, '\0', controller_id_size);
  strncpy(reinterpret_cast<char *> (key_ctr_domain.ctr_key.controller_name),
          controller_name.c_str(), strlen(controller_name.c_str()));
  domain_id_size = sizeof(key_ctr_domain.domain_name);
  memset(key_ctr_domain.domain_name, '\0', domain_id_size);
  strncpy(reinterpret_cast<char *> (key_ctr_domain.domain_name),
          domain_name.c_str(),
          strlen(domain_name.c_str()));
  val_ctr_domain_status.valid[VAL_DOMAIN_STRUCT] = UNC_VF_VALID;
  val_ctr_domain_status.domain.type = UPPL_DOMAIN_TYPE_DEFAULT;
  val_ctr_domain_status.domain.valid[VAL_DOMAIN_EVENT_ATTR1] = UNC_VF_VALID;
  val_ctr_domain_status.domain.valid[VAL_DOMAIN_EVENT_ATTR2] = UNC_VF_INVALID;
  val_ctr_domain_status.valid[VAL_DOMAIN_EVENT_ATTR2] = UNC_VF_INVALID;
  pfc::core::ipc::ServerEvent domain_event((uint32_t) mask_type, err);
  domain_event.addOutput(controller_name);
  domain_event.addOutput(domain_name);
  domain_event.addOutput((uint32_t) UNC_OP_CREATE);
  domain_event.addOutput((uint32_t) UNC_DT_STATE);
  domain_event.addOutput((uint32_t) UNC_KT_CTR_DOMAIN);
  domain_event.addOutput(key_ctr_domain);
  domain_event.addOutput(val_ctr_domain_status);
  domain_event.post();

  pfc_log_debug("%s: Posting domain event for controller : %s", PFC_FUNCNAME,
                                                 controller_name.c_str());
}

/**
 * @brief     : Method to post logical port create/delete events to UPPL
 * @param[in] : oper_type, key_struct, val_struct
 * @retval    : None
 */
void VtnDrvIntf::logicalport_event(oper_type operation,
                                   key_logical_port_t &key_struct,
                                   val_logical_port_st_t &val_struct ) {
  ODC_FUNC_TRACE;
  unc_event_mask_t mask_type = UNC_PHYSICAL_EVENTS;
  pfc_bool_t Domain_event = PFC_TRUE;
  int err = 0;
  std::string controller_name =
      (const char*)key_struct.domain_key.ctr_key.controller_name;
  std::string domain_name= (const char*)key_struct.domain_key.domain_name;
  std::string logical_port_id = (const char*)key_struct.port_id;
  pfc_log_debug("%s, key structure fields:"
                "controller_name: %s, "
                "domain_id: %s, "
                "logicalport_id: %s", PFC_FUNCNAME, controller_name.c_str(),
                domain_name.c_str(), logical_port_id.c_str());
  Domain_event = ctrl_inst_->GetDomainFlag(controller_name);
  if (Domain_event == PFC_FALSE) {
    domain_event(controller_name, domain_name);  //  post domain event to UPPL
    ctrl_inst_->SetDomainFlag(controller_name, PFC_TRUE);
  }

  pfc_log_debug("logicalport Operation received is %d", operation);
  switch (operation) {
    case VTN_LP_CREATE:
      {
        pfc::core::ipc::ServerEvent phys_lp_event((uint32_t) mask_type, err);
        phys_lp_event.addOutput(controller_name);
        phys_lp_event.addOutput(domain_name);
        phys_lp_event.addOutput((uint32_t) UNC_OP_CREATE);
        phys_lp_event.addOutput((uint32_t) UNC_DT_STATE);
        phys_lp_event.addOutput((uint32_t) UNC_KT_LOGICAL_PORT);
        phys_lp_event.addOutput(key_struct);
        phys_lp_event.addOutput(val_struct);
        phys_lp_event.post();
      }
      break;
    case VTN_LP_DELETE:
      {
        pfc::core::ipc::ServerEvent phys_lp_event((uint32_t) mask_type, err);
        phys_lp_event.addOutput(controller_name);
        phys_lp_event.addOutput(domain_name);
        phys_lp_event.addOutput((uint32_t) UNC_OP_DELETE);
        phys_lp_event.addOutput((uint32_t) UNC_DT_STATE);
        phys_lp_event.addOutput((uint32_t) UNC_KT_LOGICAL_PORT);
        phys_lp_event.addOutput(key_struct);
        phys_lp_event.post();
      }
      break;
    default:
      pfc_log_debug("%s:Invalid operation type:%d", PFC_FUNCNAME, operation);
      break;
  }
}

/**
 * @brief     : Method to post switch create/delete events to UPPL
 * @param[in] : oper_type, key_switch, val_switch_st
 * @retval    : None
 */
void VtnDrvIntf::switch_event(oper_type operation,
                              key_switch_t &key_struct,
                              val_switch_st_t &val_struct ) {
  ODC_FUNC_TRACE;
  unc_event_mask_t mask_type = UNC_PHYSICAL_EVENTS;
  pfc_bool_t Start_event = PFC_TRUE;
  int err = 0;
  controller* ctr = NULL;
  std::string controller_name =
      (const char*)key_struct.ctr_key.controller_name;
  std::string switch_id = (const char*)key_struct.switch_id;
  controller_operation util_obj(ctrl_inst_, READ_FROM_CONTROLLER, controller_name);
  ctr = util_obj.get_controller_handle();

  pfc_log_debug("%s, key structure fields:"
                "controller_name: %s, "
                "switch_id: %s, ", PFC_FUNCNAME, controller_name.c_str(),
                switch_id.c_str());
  pfc_log_debug("Switch event Operation received is %d", operation);
 if ( ctr->get_audit_result() == PFC_TRUE ) {
  Start_event = ctrl_inst_->GetEventFlag(controller_name);
  if ((Start_event == PFC_FALSE)) {
    event_start(controller_name);  //  post domain event to UPPL
    ctrl_inst_->SetEventFlag(controller_name, PFC_TRUE);
  }
 }
  switch (operation) {
    case VTN_SWITCH_CREATE:
      {
        pfc::core::ipc::ServerEvent phys_switch_event((uint32_t)
                                                      mask_type, err);
        phys_switch_event.addOutput(controller_name);
        phys_switch_event.addOutput(DEFAULT_DOMAIN_ID);
        phys_switch_event.addOutput((uint32_t) UNC_OP_CREATE);
        phys_switch_event.addOutput((uint32_t) UNC_DT_STATE);
        phys_switch_event.addOutput((uint32_t) UNC_KT_SWITCH);
        phys_switch_event.addOutput(key_struct);
        phys_switch_event.addOutput(val_struct);
        phys_switch_event.post();
      }
      break;
    case VTN_SWITCH_DELETE:
      {
        pfc::core::ipc::ServerEvent phys_switch_event((uint32_t)
                                                      mask_type, err);
        phys_switch_event.addOutput(controller_name);
        phys_switch_event.addOutput(DEFAULT_DOMAIN_ID);
        phys_switch_event.addOutput((uint32_t) UNC_OP_DELETE);
        phys_switch_event.addOutput((uint32_t) UNC_DT_STATE);
        phys_switch_event.addOutput((uint32_t) UNC_KT_SWITCH);
        phys_switch_event.addOutput(key_struct);
        phys_switch_event.post();
      }
      break;
    default:
        pfc_log_debug("%s:Invalid operation type:%d", PFC_FUNCNAME, operation);
      break;
  }
}

/**
 * @brief     : Method to post switch update events to UPPL
 * @param[in] : oper_type, key_struct, val_struct
 * @retval    : None
 */
void VtnDrvIntf::switch_event(oper_type operation,
                              key_switch_t &key_struct,
                              val_switch_st_t &new_val_struct,
                              val_switch_st_t &old_val_struct) {
  ODC_FUNC_TRACE;
  unc_event_mask_t mask_type = UNC_PHYSICAL_EVENTS;
  int err = 0;
//  pfc_bool_t Start_event = PFC_TRUE;
  std::string controller_name =
      (const char*)key_struct.ctr_key.controller_name;
  std::string switch_id = (const char*)key_struct.switch_id;

  pfc_log_debug("%s, key structure fields:"
                "controller_name: %s, "
                "switch_id: %s, ", PFC_FUNCNAME, controller_name.c_str(),
                switch_id.c_str());
  pfc_log_debug("Update Switch event Operation received is %d", operation);
  if (operation == VTN_SWITCH_UPDATE) {
    pfc::core::ipc::ServerEvent phys_switch_event((uint32_t)
                                                  mask_type, err);
    phys_switch_event.addOutput(controller_name);
    phys_switch_event.addOutput(DEFAULT_DOMAIN_ID);
    phys_switch_event.addOutput((uint32_t) UNC_OP_UPDATE);
    phys_switch_event.addOutput((uint32_t) UNC_DT_STATE);
    phys_switch_event.addOutput((uint32_t) UNC_KT_SWITCH);
    phys_switch_event.addOutput(key_struct);
    phys_switch_event.addOutput(new_val_struct);
    phys_switch_event.addOutput(old_val_struct);
    phys_switch_event.post();
  } else {
    pfc_log_debug("%s, Invalid operation, PFC_FUNCNAME", PFC_FUNCNAME);
  }
}

/**
 * @brief     : Method to post port create/delete events to UPPL
 * @param[in] : oper_type, key_port_t, val_port_st
 * @retval    : None
 */
void VtnDrvIntf::port_event(oper_type operation,
                            key_port_t &key_struct,
                            val_port_st_t &val_struct ) {
  ODC_FUNC_TRACE;
  unc_event_mask_t mask_type = UNC_PHYSICAL_EVENTS;
  int err = 0;
  std::string controller_name =
      (const char*)key_struct.sw_key.ctr_key.controller_name;
  std::string switch_id = (const char*)key_struct.sw_key.switch_id;
  std::string port_id = (const char*)key_struct.port_id;

  pfc_log_debug("%s, key structure fields:"
                "controller_name: %s, "
                "switch_id: %s, "
                "port_id: %s", PFC_FUNCNAME, controller_name.c_str(),
                switch_id.c_str(), port_id.c_str());
  pfc_log_debug("Port event Operation received is %d", operation);
  switch (operation) {
    case VTN_PORT_CREATE:
      {
        pfc::core::ipc::ServerEvent phys_port_event((uint32_t) mask_type, err);
        phys_port_event.addOutput(controller_name);
        phys_port_event.addOutput(DEFAULT_DOMAIN_ID);
        phys_port_event.addOutput((uint32_t) UNC_OP_CREATE);
        phys_port_event.addOutput((uint32_t) UNC_DT_STATE);
        phys_port_event.addOutput((uint32_t) UNC_KT_PORT);
        phys_port_event.addOutput(key_struct);
        phys_port_event.addOutput(val_struct);
        phys_port_event.post();
      }
      break;
    case VTN_PORT_DELETE:
      {
        pfc::core::ipc::ServerEvent phys_port_event((uint32_t) mask_type, err);
        phys_port_event.addOutput(controller_name);
        phys_port_event.addOutput(DEFAULT_DOMAIN_ID);
        phys_port_event.addOutput((uint32_t) UNC_OP_DELETE);
        phys_port_event.addOutput((uint32_t) UNC_DT_STATE);
        phys_port_event.addOutput((uint32_t) UNC_KT_PORT);
        phys_port_event.addOutput(key_struct);
        phys_port_event.post();
      }
      break;
    default:
        pfc_log_debug("%s:Invalid operation type:%d", PFC_FUNCNAME, operation);
      break;
  }
}

/**
 * @brief     : Method to post port update events to UPPL
 * @param[in] : oper_type, key_port_t, val_port_st
 * @retval    : None
 */
void VtnDrvIntf::port_event(oper_type operation,
                            key_port_t &key_struct,
                            val_port_st_t &new_val_struct,
                            val_port_st_t &old_val_struct) {
  ODC_FUNC_TRACE;
  unc_event_mask_t mask_type = UNC_PHYSICAL_EVENTS;
  int err = 0;
  std::string controller_name =
      (const char*)key_struct.sw_key.ctr_key.controller_name;
  std::string switch_id = (const char*)key_struct.sw_key.switch_id;
  std::string port_id = (const char*)key_struct.port_id;

  pfc_log_debug("%s, key structure fields:"
                "controller_name: %s, "
                "switch_id: %s, "
                "port_id: %s", PFC_FUNCNAME, controller_name.c_str(),
                switch_id.c_str(), port_id.c_str());
  pfc_log_debug("Update Port event Operation received [%d]", operation);
  if (operation == VTN_PORT_UPDATE) {
    pfc::core::ipc::ServerEvent phys_port_event((uint32_t) mask_type, err);
    phys_port_event.addOutput(controller_name);
    phys_port_event.addOutput(DEFAULT_DOMAIN_ID);
    phys_port_event.addOutput((uint32_t) UNC_OP_UPDATE);
    phys_port_event.addOutput((uint32_t) UNC_DT_STATE);
    phys_port_event.addOutput((uint32_t) UNC_KT_PORT);
    phys_port_event.addOutput(key_struct);
    phys_port_event.addOutput(new_val_struct);
    phys_port_event.addOutput(old_val_struct);
    phys_port_event.post();
  } else {
    pfc_log_debug("%s, Invalid operation, PFC_FUNCNAME", PFC_FUNCNAME);
  }
}

/**
 * @brief     : Method to post link create/delete events to UPPL
 * @param[in] : oper_type, key_link_t, val_link_st
 * @retval    : None
 */
void VtnDrvIntf::link_event(oper_type operation,
                            key_link_t &key_struct,
                            val_link_st_t &val_struct ) {
  ODC_FUNC_TRACE;
  unc_event_mask_t mask_type = UNC_PHYSICAL_EVENTS;
  int err = 0;
  std::string controller_name =
      (const char*)key_struct.ctr_key.controller_name;
  std::string switch_id1 = (const char*)key_struct.switch_id1;
  std::string port_id1 = (const char*)key_struct.port_id1;
  std::string switch_id2 = (const char*)key_struct.switch_id2;
  std::string port_id2 = (const char*)key_struct.port_id2;

  pfc_log_debug("%s, key structure fields:"
                "controller_name: %s, "
                "switch_id1: %s, "
                "port_id1: %s", PFC_FUNCNAME, controller_name.c_str(),
                switch_id1.c_str(), port_id1.c_str());

  pfc_log_debug("switch_id2: %s, "
                "port_id2: %s", switch_id2.c_str(), port_id2.c_str());

  pfc_log_debug("Link event Operation received is %d", operation);
  switch (operation) {
    case VTN_LINK_CREATE:
      {
        pfc::core::ipc::ServerEvent phys_link_event((uint32_t) mask_type, err);
        phys_link_event.addOutput(controller_name);
        phys_link_event.addOutput(DEFAULT_DOMAIN_ID);
        phys_link_event.addOutput((uint32_t) UNC_OP_CREATE);
        phys_link_event.addOutput((uint32_t) UNC_DT_STATE);
        phys_link_event.addOutput((uint32_t) UNC_KT_LINK);
        phys_link_event.addOutput(key_struct);
        phys_link_event.addOutput(val_struct);
        phys_link_event.post();
      }
      break;
    case VTN_LINK_DELETE:
      {
        pfc::core::ipc::ServerEvent phys_link_event((uint32_t) mask_type, err);
        phys_link_event.addOutput(controller_name);
        phys_link_event.addOutput(DEFAULT_DOMAIN_ID);
        phys_link_event.addOutput((uint32_t) UNC_OP_DELETE);
        phys_link_event.addOutput((uint32_t) UNC_DT_STATE);
        phys_link_event.addOutput((uint32_t) UNC_KT_LINK);
        phys_link_event.addOutput(key_struct);
        phys_link_event.post();
      }
      break;
    default:
      pfc_log_debug("%s:Invalid operation type:%d", PFC_FUNCNAME, operation);
      break;
  }
}

/**
 * @brief     : Method to post link update events to UPPL
 * @param[in] : oper_type, key_link_t, val_link_st
 * @retval    : None
 */
void VtnDrvIntf::link_event(oper_type operation,
                            key_link_t &key_struct,
                            val_link_st_t &new_val_struct,
                            val_link_st_t &old_val_struct) {
  ODC_FUNC_TRACE;
  unc_event_mask_t mask_type = UNC_PHYSICAL_EVENTS;
  int err = 0;

  std::string controller_name =
      (const char*)key_struct.ctr_key.controller_name;
  std::string switch_id1 = (const char*)key_struct.switch_id1;
  std::string port_id1 = (const char*)key_struct.port_id1;
  std::string switch_id2 = (const char*)key_struct.switch_id2;
  std::string port_id2 = (const char*)key_struct.port_id2;

  pfc_log_debug("%s, key structure fields:"
                "controller_name: %s, "
                "switch_id1: %s, "
                "port_id1: %s", PFC_FUNCNAME, controller_name.c_str(),
                switch_id1.c_str(), port_id1.c_str());

  pfc_log_debug("switch_id2: %s, "
                "port_id2: %s", switch_id2.c_str(), port_id2.c_str());
  pfc_log_debug("Update Link event Operation received is %d", operation);

  if (operation == VTN_LINK_UPDATE) {
    pfc::core::ipc::ServerEvent phys_link_event((uint32_t) mask_type, err);
    phys_link_event.addOutput(controller_name);
    phys_link_event.addOutput(DEFAULT_DOMAIN_ID);
    phys_link_event.addOutput((uint32_t) UNC_OP_UPDATE);
    phys_link_event.addOutput((uint32_t) UNC_DT_STATE);
    phys_link_event.addOutput((uint32_t) UNC_KT_LINK);
    phys_link_event.addOutput(key_struct);
    phys_link_event.addOutput(new_val_struct);
    phys_link_event.addOutput(old_val_struct);
    phys_link_event.post();
  } else {
    pfc_log_debug("%s, Invalid operation, PFC_FUNCNAME", PFC_FUNCNAME);
  }
}


/**
 * @brief     : Method to post EVENT START to UPPL
 * @param[in] : Controller name
 * @retval    : None
**/

void VtnDrvIntf::event_start(std::string ctr_name) {
  ODC_FUNC_TRACE;
  unc_event_mask_t mask_type = UNC_CTLR_STATE_EVENTS;
  int err = 0;
  //unc::driver::controller* ctr_ptr;
  const std::string ctr_version = "1.0.0.0";
  key_ctr_t key_ctr;
  val_ctr_st_t val_ctr_new;
  val_ctr_st_t val_ctr_old;
  memset(&key_ctr, 0, sizeof(key_ctr_t));
  memset(&val_ctr_new, 0, sizeof(val_ctr_st_t));
  memset(&val_ctr_old, 0, sizeof(val_ctr_st_t));
  val_ctr_new.valid[1] = UNC_VF_VALID;
  val_ctr_new.valid[2] = UNC_VF_VALID;
//  std::string ctr_name =
//              (const char*)key_struct.ctr_key.controller_name;
  memcpy(key_ctr.controller_name, ctr_name.c_str(), ctr_name.length() + 1);
  memcpy(val_ctr_new.actual_version,
         ctr_version.c_str(), ctr_version.length() + 1);
  pfc::core::ipc::ServerEvent phys_ctr_event((uint32_t) mask_type, err);
  val_ctr_new.oper_status = CONTROLLER_EVENTS_START;
  phys_ctr_event.addOutput(ctr_name);
//  phys_ctr_event.addOutput(controller_name);
  phys_ctr_event.addOutput(DEFAULT_DOMAIN_ID);
  phys_ctr_event.addOutput((uint32_t) UNC_OP_UPDATE);
  phys_ctr_event.addOutput((uint32_t) UNC_DT_STATE);
  phys_ctr_event.addOutput((uint32_t) UNC_KT_CONTROLLER);
  phys_ctr_event.addOutput(key_ctr);
  phys_ctr_event.addOutput(val_ctr_new);
  phys_ctr_event.addOutput(val_ctr_old);
  pfc_log_debug("%s: Posting CONTROLLER_EVENTS_START to UPPL, Controller_name: %s", PFC_FUNCNAME, ctr_name.c_str());
  phys_ctr_event.post();
}

/**
 * @brief     : Method to create Kt handler
 * @param[in] : unc_key_type_t
 * @retval    : None
 */
template <typename key, typename value>
void VtnDrvIntf::create_handler(unc_key_type_t keytype)  {
  if (keytype == UNC_KT_ROOT) {
    KtHandler* handler_= new KtRequestHandler<key, value>(&map_kt_);
  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          keytype,
          handler_));
  } else {
  KtHandler* handler_= new KtRequestHandler<key, value>(NULL);
  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          keytype,
          handler_));
  }
}

/**
 * @Description :Method to fill the map pfc_ipcstdef_t pointer against keytype
 * @param[in]   :NONE
 * @return      :NONE
 **/
void  VtnDrvIntf::initialize_map() {
  ODC_FUNC_TRACE;
  POPULATE_STDEF(key_vtn, val_vtn, UNC_KT_VTN, stdefk_vtn, stdefv_vtn);
  POPULATE_STDEF(key_vbr, val_vbr, UNC_KT_VBRIDGE, stdefk_vbr, stdefv_vbr);
#if 0
  POPULATE_STDEF(key_vbr_if, val_vbr_if, UNC_KT_VBR_IF, stdefk_vbrif,
                 stdefv_vbrif);
#endif
  POPULATE_STDEF(key_vbr_if, pfcdrv_val_vbr_if, UNC_KT_VBR_IF, stdefk_vbrif,
                 stdefv_vbrif);
  POPULATE_STDEF(key_vlan_map, val_vlan_map, UNC_KT_VBR_VLANMAP, stdefk_vlan,
                 stdefv_vlan);
  POPULATE_STDEF(key_flowlist, val_flowlist, UNC_KT_FLOWLIST, stdefk_fl,
                 stdefv_fl);
  POPULATE_STDEF(key_flowlist_entry, val_flowlist_entry, UNC_KT_FLOWLIST_ENTRY,
                 stdefk_fle, stdefv_fle);
  POPULATE_STDEF(key_vtn_flowfilter, val_flowfilter, UNC_KT_VTN_FLOWFILTER,
                 stdefk_ff, stdefv_ff);
  POPULATE_STDEF(key_vtn_flowfilter_entry, val_vtn_flowfilter_entry,
                 UNC_KT_VTN_FLOWFILTER_ENTRY, stdefk_vtn_fle, stdefv_vtn_fle);
  POPULATE_STDEF(key_vbr_flowfilter, val_flowfilter, UNC_KT_VBR_FLOWFILTER,
                 stdefk_vbr_fl, stdefv_vbr_fl);
  POPULATE_STDEF(key_vbr_flowfilter_entry, val_flowfilter_entry,
                 UNC_KT_VBR_FLOWFILTER_ENTRY, stdefk_vbr_fle, stdefv_vbr_fle);
  POPULATE_STDEF(key_vbr_if_flowfilter, pfcdrv_val_vbrif_vextif,
                 UNC_KT_VBRIF_FLOWFILTER, stdefk_vbrif_fl, stdefv_vbrif_fl);
  POPULATE_STDEF(key_vbr_if_flowfilter_entry, pfcdrv_val_flowfilter_entry,
                 UNC_KT_VBRIF_FLOWFILTER_ENTRY, stdefk_vbrif_fle,
                 stdefv_vbrif_fle);
  POPULATE_STDEF(key_vterm, val_vterm, UNC_KT_VTERMINAL, stdefk_vtrem,
                 stdefv_vterm);
  POPULATE_STDEF(key_vterm_if, val_vterm_if, UNC_KT_VTERM_IF, stdefk_vtrem_if,
                 stdefv_vterm_if);
  POPULATE_STDEF(key_vterm_if_flowfilter, val_flowfilter,
                 UNC_KT_VTERMIF_FLOWFILTER, stdefk_vtrem_if_ff,
                 stdefv_vterm_if_ff);
  POPULATE_STDEF(key_vterm_if_flowfilter_entry, val_flowfilter_entry,
                 UNC_KT_VTERMIF_FLOWFILTER_ENTRY, stdefk_vtrem_if_fle,
                 stdefv_vterm_if_fle);
}

}  // namespace driver
}  // namespace unc
PFC_MODULE_IPC_DECL(unc::driver::VtnDrvIntf, 3);
