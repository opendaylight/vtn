/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <vtn_drv_module.hh>
#include <pfcxx/ipc_server.hh>
#include <request_template.hh>
#include "unc/unc_events.h"

namespace unc {
namespace driver {

/**
 * @brief     : constructor
 */
VtnDrvIntf::VtnDrvIntf(const pfc_modattr_t* attr)
: Module(attr), taskq_(NULL), ctrl_inst_(NULL),
    Domain_event_(PFC_FALSE) {
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

  //  Initialize KT_Handler_Instance
  KtHandler* ctrl_req = new unc::driver::KtRequestHandler<key_ctr_t,
      val_ctr_t,
      controller_command>(NULL);

  if (NULL == ctrl_req) {
    pfc_log_error("controller request handler is NULL");
    return PFC_FALSE;
  }
  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          UNC_KT_CONTROLLER,
          ctrl_req));

  KtHandler* kt_root_req = new unc::driver::KtRequestHandler<key_root_t,
      val_root_t,
      unc::driver::root_driver_command>(&map_kt_);
  if (NULL == kt_root_req) {
    pfc_log_error("KT_ROOT request handler is NULL");
    return PFC_FALSE;
  }
  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          UNC_KT_ROOT,
          kt_root_req));

  KtHandler* vtn_req = new unc::driver::KtRequestHandler<key_vtn_t,
      val_vtn_t,
      unc::driver::vtn_driver_command>(NULL);
  if (NULL == vtn_req) {
    pfc_log_error("vtn request handler is NULL");
    return PFC_FALSE;
  }
  map_kt_.insert(std::pair<unc_key_type_t,
                 unc::driver::KtHandler*>(UNC_KT_VTN, vtn_req));

  KtHandler* vbr_req = new unc::driver::KtRequestHandler<key_vbr_t,
                           val_vbr_t,
                           unc::driver::vbr_driver_command> (NULL);
  if (NULL == vbr_req) {
    pfc_log_error("vbr request handler is NULL");
    return PFC_FALSE;
  }

  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          UNC_KT_VBRIDGE,
          vbr_req));

  KtHandler* vbrif_req = new unc::driver::KtRequestHandler<key_vbr_if_t,
      pfcdrv_val_vbr_if_t,
      unc::driver::vbrif_driver_command>(NULL);
  if (NULL == vbrif_req) {
    pfc_log_error("vbr interface request handler is NULL");
    return PFC_FALSE;
  }

  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          UNC_KT_VBR_IF,
          vbrif_req));

  KtHandler* vbrvlanmap_req = new unc::driver::KtRequestHandler<key_vlan_map_t,
      val_vlan_map_t,
      unc::driver::vbrvlanmap_driver_command> (NULL);

  if (NULL == vbrvlanmap_req) {
    pfc_log_error("vbrvlanmap request handler is NULL");
    return PFC_FALSE;
  }

  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          UNC_KT_VBR_VLANMAP,
          vbrvlanmap_req));

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
  drv_resp_code_t resp_code = DRVAPI_RESPONSE_FAILURE;
  memset(&request_hdr, 0, sizeof(request_hdr));
  VtnDrvRetEnum result = get_request_header(p_sess, request_hdr);

  if (VTN_DRV_RET_SUCCESS != result) {
    pfc_log_error("VtnDrvIntfservice::%s Failed to get argument err=%d",
                 PFC_FUNCNAME, result);
    return PFC_IPCRESP_FATAL;
  }

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
  if (resp_code != DRVAPI_RESPONSE_SUCCESS) {
    pfc_log_debug("handle_request fail for key:%d", request_hdr.key_type);
    return PFC_IPCRESP_FATAL;
  }
  return DRVAPI_RESPONSE_SUCCESS;
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
    pfc_log_debug("%s: Block Handle is Valid,Time interval %d", PFC_FUNCNAME,
                  conf_parser_.time_interval);
  } else {
    conf_parser_.time_interval = default_time_interval;
    pfc_log_debug("%s: Block Handle is Invalid,set default Value %d",
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

  pfc_log_debug("%s: Posting domain event", PFC_FUNCNAME);
  Domain_event_ = PFC_TRUE;
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
  int err = 0;
  std::string controller_name =
      (const char*)key_struct.domain_key.ctr_key.controller_name;
  std::string domain_name= (const char*)key_struct.domain_key.domain_name;
  std::string logical_port_id = (const char*)key_struct.port_id;
  pfc_log_debug("%s,key structure fields:"
                "controller_name: %s,"
                "domain_id: %s,"
                "logicalport_id: %s", PFC_FUNCNAME, controller_name.c_str(),
                domain_name.c_str(), logical_port_id.c_str());
  if (Domain_event_ == PFC_FALSE) {
    domain_event(controller_name, domain_name);  //  post domain event to UPPL
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
  int err = 0;
  std::string controller_name =
      (const char*)key_struct.ctr_key.controller_name;
  std::string switch_id = (const char*)key_struct.switch_id;

  pfc_log_debug("%s,key structure fields:"
                "controller_name: %s,"
                "switch_id: %s,", PFC_FUNCNAME, controller_name.c_str(),
                switch_id.c_str());
  pfc_log_debug("Switch event Operation received is %d", operation);
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
  std::string controller_name =
      (const char*)key_struct.ctr_key.controller_name;
  std::string switch_id = (const char*)key_struct.switch_id;

  pfc_log_debug("%s,key structure fields:"
                "controller_name: %s,"
                "switch_id: %s,", PFC_FUNCNAME, controller_name.c_str(),
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

  pfc_log_debug("%s,key structure fields:"
                "controller_name: %s,"
                "switch_id: %s,"
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

  pfc_log_debug("%s,key structure fields:"
                "controller_name: %s,"
                "switch_id: %s,"
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

  pfc_log_debug("%s,key structure fields:"
                "controller_name: %s,"
                "switch_id1: %s,"
                "port_id1: %s", PFC_FUNCNAME, controller_name.c_str(),
                switch_id1.c_str(), port_id1.c_str());

  pfc_log_debug("switch_id2: %s,"
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

  pfc_log_debug("%s,key structure fields:"
                "controller_name: %s,"
                "switch_id1: %s,"
                "port_id1: %s", PFC_FUNCNAME, controller_name.c_str(),
                switch_id1.c_str(), port_id1.c_str());

  pfc_log_debug("switch_id2: %s,"
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

}  // namespace driver
}  // namespace unc
PFC_MODULE_IPC_DECL(unc::driver::VtnDrvIntf, 3);
