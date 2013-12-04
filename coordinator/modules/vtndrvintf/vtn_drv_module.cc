/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <vtn_drv_module.hh>

namespace unc {
namespace driver {

/**
 * @brief     : constructor
 */
VtnDrvIntf::VtnDrvIntf(const pfc_modattr_t* attr)
: Module(attr), taskq_(NULL), ctrl_inst_(NULL) {
  ODC_FUNC_TRACE;
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

  uint32_t concurrency = 1;
  taskq_ = pfc::core::TaskQueue::create(concurrency);
  ControllerFramework *ctrl_inst_= new ControllerFramework(taskq_);
  PFC_ASSERT(ctrl_inst_ != NULL);
  set_controller_instance(ctrl_inst_);

  //  Initialize KT_Handler_Instance
  KtHandler* ctrl_req = new unc::driver::KtRequestHandler<key_ctr_t,
      val_ctr_t,
      controller_command>(NULL);

  if (ctrl_req == NULL) {
    pfc_log_error("controller request handler is NULL");
    return PFC_FALSE;
  }
  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          UNC_KT_CONTROLLER,
          ctrl_req));

  KtHandler* kt_root_req = new unc::driver::KtRequestHandler<key_root_t,
      val_root_t,
      unc::driver::root_driver_command>(&map_kt_);
  if (kt_root_req == NULL) {
    pfc_log_error("KT_ROOT request handler is NULL");
    return PFC_FALSE;
  }
  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          UNC_KT_ROOT,
          kt_root_req));

  KtHandler* vtn_req = new unc::driver::KtRequestHandler<key_vtn_t,
      val_vtn_t,
      unc::driver::vtn_driver_command>(NULL);
  if (vtn_req == NULL) {
    pfc_log_error("vtn request handler is NULL");
    return PFC_FALSE;
  }
  map_kt_.insert(std::pair<unc_key_type_t,
                     unc::driver::KtHandler*>(UNC_KT_VTN, vtn_req));

  KtHandler* vbr_req = new unc::driver::KtRequestHandler<key_vbr_t,
                                 val_vbr_t,
                                 unc::driver::vbr_driver_command> (NULL);
  if (vbr_req == NULL) {
    pfc_log_error("vbr request handler is NULL");
    return PFC_FALSE;
  }

  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          UNC_KT_VBRIDGE,
          vbr_req));

  KtHandler* vbrif_req = new unc::driver::KtRequestHandler<key_vbr_if_t,
      pfcdrv_val_vbr_if_t,
      unc::driver::vbrif_driver_command>(NULL);
  if (vbrif_req == NULL) {
    pfc_log_error("vbr interface request handler is NULL");
    return PFC_FALSE;
  }

  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          UNC_KT_VBR_IF,
          vbrif_req));

  KtHandler* vbrvlanmap_req = new unc::driver::KtRequestHandler<key_vlan_map_t,
      val_vlan_map_t,
      unc::driver::vbrvlanmap_driver_command> (NULL);

  if (vbrvlanmap_req == NULL) {
    pfc_log_error("vbrvlanmap request handler is NULL");
    return PFC_FALSE;
  }

  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
          UNC_KT_VBR_VLANMAP,
          vbrvlanmap_req));

  unc::tclib::TcLibModule* tclib_obj =
      static_cast<unc::tclib::TcLibModule*>(pfc::core::Module::getInstance(
              "tclib"));
  if (tclib_obj == NULL) {
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
pfc_ipcresp_t VtnDrvIntf::ipcService(pfc::core::ipc::ServerSession& sess,
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
  // Set Timeout as infinite for audit operation
  if (request_hdr.key_type == UNC_KT_ROOT) {
    sess.setTimeout(NULL);
  }

  if (map_kt_.empty()) {
    pfc_log_debug("map_kt empty");
    return PFC_IPCRESP_FATAL;
  }
    KtHandler *hnd_ptr = get_kt_handler(request_hdr.key_type);

  if (hnd_ptr == NULL) {
    pfc_log_debug("Key Type Not Supported %d", request_hdr.key_type);
    return PFC_IPCRESP_FATAL;
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
  if (drv_obj == NULL) {
    pfc_log_error("driver handler is NULL");
    return VTN_DRV_RET_FAILURE;
  }
  unc_keytype_ctrtype_t ctr_type = drv_obj->get_controller_type();
  ctrl_inst_->RegisterDriver(ctr_type, drv_obj);
  return VTN_DRV_RET_SUCCESS;
}
}  // namespace driver
}  // namespace unc
PFC_MODULE_IPC_DECL(unc::driver::VtnDrvIntf, 3);
