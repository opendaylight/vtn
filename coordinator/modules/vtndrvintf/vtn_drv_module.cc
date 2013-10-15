/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the
 * terms of the Eclipse Public License v1.0 which
 * accompanies this
 * distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#include <vtn_drv_module.hh>

namespace unc {
namespace driver {

/**
 * VtnDrvIntf():
 * @brief     : constructor
 */

VtnDrvIntf::VtnDrvIntf(const pfc_modattr_t* attr)
: Module(attr), taskq_(NULL), ctrl_inst_(NULL) {
  pfc_log_debug("Entering constructor %s..", PFC_FUNCNAME);
  pfc_log_debug("Existing constructor %s..", PFC_FUNCNAME);
}

/**
 * VtnDrvIntf():
 * @brief     : destructor
 */
VtnDrvIntf::~VtnDrvIntf() {
  pfc_log_debug("Entering default desstructor %s..", PFC_FUNCNAME);
  if (ctrl_inst_)
    delete ctrl_inst_;
  pfc_log_debug("Existing default desstructor %s..", PFC_FUNCNAME);
}

/**
 * Init:
 * @brief     : This Function is called to load the vtndrvintf module
 * @param[in] : None
 * @retval    : pfc_bool_t
 */
pfc_bool_t VtnDrvIntf::init(void) {
  pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);

  uint32_t concurrency = 1;
  taskq_ = pfc::core::TaskQueue::create(concurrency);
  ControllerFramework *ctrl_inst_= new ControllerFramework(taskq_);
  set_controller_instance(ctrl_inst_);

  /****Initialize KT_Handler_Instance*****/
  KtHandler* vtn_req = new unc::driver::KtRequestHandler<key_vtn_t,
                                         val_vtn_t,
                                         unc::driver::vtn_driver_command>();
  if (vtn_req == NULL) {
    pfc_log_error("vtn requrest handler is NULL");
  return PFC_FALSE;
  }
  map_kt_.insert(std::pair<unc_key_type_t,
                     unc::driver::KtHandler*>(UNC_KT_VTN, vtn_req));

  KtHandler* vbr_req = new unc::driver::KtRequestHandler<key_vbr_t,
                                 val_vbr_t,
                                 unc::driver::vbr_driver_command> ();
  if (vbr_req == NULL) {
    pfc_log_error("vbr requrest handler is NULL");
  return PFC_FALSE;
  }

  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
                                          UNC_KT_VBRIDGE,
                                          vbr_req));

  KtHandler* vbrif_req = new unc::driver::KtRequestHandler<key_vbr_if_t,
                              pfcdrv_val_vbr_if_t,
                              unc::driver::vbrif_driver_command>();
  if (vbrif_req == NULL) {
    pfc_log_error("vbr interface requrest handler is NULL");
  return PFC_FALSE;
  }

  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
                                          UNC_KT_VBR_IF,
                                          vbrif_req));

  KtHandler* ctrl_req = new unc::driver::KtRequestHandler<key_ctr_t,
                                          val_ctr_t,
                                          controller_command>();

  if (ctrl_req == NULL) {
    pfc_log_error("controller requrest handler is NULL");
  return PFC_FALSE;
  }
  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
                                          UNC_KT_CONTROLLER,
                                          ctrl_req));

  KtHandler* ktroot_req = new unc::driver::KtRequestHandler<key_root_t,
                                   val_root_t,
                                   unc::driver::root_driver_command>();
  if (ktroot_req == NULL) {
    pfc_log_error("KT_ROOT requrest handler is NULL");
    return PFC_FALSE;
  }
  map_kt_.insert(std::pair<unc_key_type_t, unc::driver::KtHandler*>(
                                          UNC_KT_ROOT,
                                          ktroot_req));

  unc::tclib::TcLibModule* TcLibModule_obj =
        static_cast<unc::tclib::TcLibModule*>(pfc::core::Module::getInstance(
        "tclib"));
  if (TcLibModule_obj == NULL) {
    pfc_log_error("tclib getInstance failed");
    return PFC_FALSE;
    }

  if (ctrl_inst_ == NULL) {
    pfc_log_error("controller instance not created");
    return PFC_FALSE;
  }

  TcLibModule_obj->TcLibRegisterHandler(new DriverTxnInterface(ctrl_inst_,
                                                               map_kt_));
  pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
  return PFC_TRUE;
}


/**
 * fini:
 * @brief     : This Function is called to unload the vtndrvintf module
 * @param[in] : void
 * @retval    : pfc_bool_t
 */
pfc_bool_t VtnDrvIntf::fini(void) {
  pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);

  if (taskq_) {
    delete taskq_;
    taskq_ = NULL;
  }

  if (ctrl_inst_) {
    delete ctrl_inst_;
    ctrl_inst_ = NULL;
  }

  pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
  return PFC_TRUE;
}


/**
 * ipcService:
 * @brief     : This Function recevies the ipc request and process the same
 * @param[in] : sess
 * @retval    : pfc_ipcresp_t
 */
pfc_ipcresp_t VtnDrvIntf::ipcService(pfc::core::ipc::ServerSession& sess,
                                     pfc_ipcid_t service) {
  pfc_log_debug("%s: Entering function..", PFC_FUNCNAME);
  uint32_t success=0;

  pfc::core::ipc::ServerSession* p_sess=&sess;
  keyif_drv_request_header_t request_hdr;
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
    KtHandler *ptr_KT_Handler = get_kt_handler(request_hdr.key_type);

  if (ptr_KT_Handler == NULL) {
    pfc_log_debug("Key Type Not Supported %d", request_hdr.key_type);
    return PFC_IPCRESP_FATAL;
  }

  ptr_KT_Handler->handle_request(sess, request_hdr, ctrl_inst_);

  pfc_log_debug("%s: Exiting function..", PFC_FUNCNAME);
  return success;
}


/**
 * GetRequestHeader:
 * @brief     : This function parse the session and fills
 *              keyif_drv_request_header_t
 * @param[in] : sess, request_hdr
 * @retval    : VtnDrvRetEnum
 */
VtnDrvRetEnum VtnDrvIntf::get_request_header(
                                  pfc::core::ipc::ServerSession* sess,
                                  keyif_drv_request_header_t &request_hdr) {
  pfc_log_debug("Entering Function %s..", PFC_FUNCNAME);

  PFC_ASSERT(sess != NULL);

  uint32_t err = 0, keytype;

  err = sess->getArgument(IPC_KEY_TYPE_INDEX, keytype);

  if (err) {
    pfc_log_error("Failed to receive key type: (err = %d)", err);
    return VTN_DRV_RET_FAILURE;
  }

  request_hdr.key_type=(unc_key_type_t)keytype;
  pfc_log_debug("Keytype is %d", request_hdr.key_type);

  const char *ctr_name;
  err = sess->getArgument(IPC_CONTROLLER_ID_INDEX, ctr_name);

  if (err) {
    pfc_log_error("Failed to receive controller id: (err = %d)", err);
    return VTN_DRV_RET_FAILURE;
  }

  strncpy(reinterpret_cast<char*>(request_hdr.controller_name), ctr_name,
      sizeof(request_hdr.controller_name) - 1);

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

  pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
  return VTN_DRV_RET_SUCCESS;
}

/**
 * get_kt_handler:
 * @brief     : This Function  returns the  kt_handler for
 *              the appropriate key types
 * @param[in] : kt
 * @retval    : kt_handler
 */
KtHandler* VtnDrvIntf::get_kt_handler(unc_key_type_t kt) {
  return map_kt_.find(kt)->second;
}

/**
 * RegisterDriver:
 * @brief     : This Function is called to register the driver handler
 *              with respect to the controller type
 * @param[in] : *drvobj
 * @retval    : uint32_t
 */
VtnDrvRetEnum VtnDrvIntf:: register_driver(driver *drvobj) {
  unc_keytype_ctrtype_t ctr_type = drvobj->get_controller_type();
  ctrl_inst_->RegisterDriver(ctr_type, drvobj);
  return VTN_DRV_RET_SUCCESS;
}
}  // namespace driver
}  // namespace unc
PFC_MODULE_IPC_DECL(unc::driver::VtnDrvIntf, 3);
