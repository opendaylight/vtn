/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _REQUEST_TEMPLATE_H_
#define _REQUEST_TEMPLATE_H_

#include <kt_handler.hh>
#include <controller_fw.hh>
#include <driver/driver_interface.hh>
#include <audit_keytypes.h>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <memory>

namespace unc {
namespace driver {

extern audit_key_type audit_key[AUDIT_KT_SIZE];

typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;

/**
 * @brief KtRequestHandler provides function for handling request and response
 *
 **/
template<typename key, typename val>
class KtRequestHandler : public KtHandler {
 public:
  /*
   * @brief - Default Constructor
   */
  explicit KtRequestHandler(kt_handler_map* ktmap): kt_map_(ktmap) {}

  /*
   * @brief - Default Destructor
   */
  ~KtRequestHandler();

  /**
   * @brief    - This method handles request received from platform layer
   * @param[in]- ServerSession, odl_drv_request_header_t, ControllerFramework*
   * @retval   - UncRespCode
   **/
  UncRespCode
      handle_request(pfc::core::ipc::ServerSession &sess,
                     unc::driver::odl_drv_request_header_t &request_header,
                     unc::driver::ControllerFramework* ctrl_int);

  /**
   * @brief    - This method forms the command from ConfigNode pointer
   * @param[in]- ConfigNode*, controller*, driver*
   * @retval   - UncRespCode
   **/
  UncRespCode
  execute_cmd(unc::vtndrvcache::ConfigNode *cfgptr,
                  unc::driver::controller* ctl_ptr,
                  unc::driver::driver* drv_ptr);

  /**
   * @brief     - This method retrieves the key and val structures
   * @param[in] - ServerSession, odl_drv_request_header_t, key, val
   * @retval    -  UncRespCode
   **/


  /**
   * @brief    - This method retrieves the Key struct from ConfigNode pointer
   * @param[in]- ConfigNode*
   * @retval   - void*
   **/
  void* get_key_struct(unc::vtndrvcache::ConfigNode *cfgptr);

  /**
   * @brief    - This method retrieves the value struct from ConfigNode pointer
   * @param[in]- ConfigNode*
   * @retval   - void*
   **/
  void* get_val_struct(unc::vtndrvcache::ConfigNode *cfgptr);


 private:
  UncRespCode
      parse_request(pfc::core::ipc::ServerSession &sess,
                    unc::driver::odl_drv_request_header_t &request_header,
                    key &key_generic_,
                    val &val_generic1_,
                    val &val_generic2_);

  /**
   * @brief    - This method executes the Create, Delete, Update and Read requests of
   *             Keytypes
   * @param[in]- ServerSession, odl_drv_request_header_t, ControllerFramework*
   *             UncRespCode, key, val
   * @retval   - UncRespCode
   **/
  UncRespCode
      execute(pfc::core::ipc::ServerSession &sess,
              unc::driver::odl_drv_request_header_t &request_header,
              unc::driver::ControllerFramework* ctrl_int,
              key &key_generic_,
              val &val_generic1_,
              val &val_generic2_);

  /**
   * @brief     - This method handles response from controller
   *              and send response to platform layer
   * @param[in] - ServerSession, odl_drv_request_header_t, ControllerFramework*,
   *              key, val, UncRespCode
   * @retval    - UncRespCode
   **/
  UncRespCode
  handle_response(pfc::core::ipc::ServerSession &sess,
                      unc::driver::odl_drv_request_header_t &request_header,
                      unc::driver::ControllerFramework* ctrl_int,
                      key &key_generic_,
                      val &val_generic_,
                      UncRespCode &resp_code_);

  /**
   * @brief    - This method creates the Response Header
   * @param[in]- odl_drv_response_header_t,
   *             odl_drv_request_header_t, UncRespCode
   * @retval   - void
   */
  void
  create_response_header(unc::driver::odl_drv_request_header_t &reqhdr,
                             unc::driver::odl_drv_response_header_t &resphdr,
                             UncRespCode &resp_code_);

  /**
   * @brief    - This method populates the Response Header
   * @param[in]- ServerSession, odl_drv_response_header_t
   * @retval   - UncRespCode
   **/
  UncRespCode
  populate_response_header(pfc::core::ipc::ServerSession &sess,
                         unc::driver::odl_drv_response_header_t &resp_hdr);


  KtHandler* get_handler(unc_key_type_t keytype);
  kt_handler_map* kt_map_;
};


/**
 * @brief: Default Destructor
 **/

template<typename key, typename val>
KtRequestHandler<key, val>::~KtRequestHandler() {}

  /**
   * @brief    - This method retrieves the Key struct from ConfigNode pointer
   * @param[in]- ConfigNode*
   * @retval   - void*
   **/
template<typename key, typename val>
void* KtRequestHandler<key, val>::
get_key_struct(unc::vtndrvcache::ConfigNode *cfgptr) {
  unc::vtndrvcache::CacheElementUtil <key, val, val, uint32_t> * cache_util_ptr =
      static_cast <unc::vtndrvcache::CacheElementUtil<key, val, val, uint32_t> * >
      (cfgptr);

  if (cache_util_ptr == NULL) {
    pfc_log_error("%s: cache_util_ptr is null", PFC_FUNCNAME);
    return NULL;
  }

  return cache_util_ptr->get_key_structure();
}

  /**
   * @brief    - This method retrieves the Value struct from ConfigNode pointer
   * @param[in]- ConfigNode*
   * @retval   - void*
   **/
template<typename key, typename val>
void* KtRequestHandler<key, val>::
get_val_struct(unc::vtndrvcache::ConfigNode *cfgptr) {
  unc::vtndrvcache::CacheElementUtil<key, val, val, uint32_t> * cache_util_ptr =
      static_cast <unc::vtndrvcache::CacheElementUtil
      <key, val, val, uint32_t> * >(cfgptr);

  if (cache_util_ptr == NULL) {
    pfc_log_error("%s: cache_util_ptr is null for get_val_struct",
                  PFC_FUNCNAME);
    return NULL;
  }


  return cache_util_ptr->get_val_structure();
}

  /**
   * @brief     - This method retrieves the key and val structures
   * @param[in] - ServerSession, odl_drv_request_header_t, key, val
   * @retval    -  UncRespCode
   **/
template<typename key, typename val>
UncRespCode
KtRequestHandler<key, val>::parse_request(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    key &key_generic_,
    val &val_generic1_,
    val &val_generic2_) {
  if (sess.getArgument(INPUT_KEY_STRUCT_INDEX, key_generic_)) {
    pfc_log_debug("%s: Exting Function.getArg Failed to read key struct."
                  " rc=%u", PFC_FUNCNAME, 2);
    return UNC_DRV_RC_MISSING_KEY_STRUCT;
  }
  if (sess.getArgument(INPUT_VAL_STRUCT_INDEX, val_generic1_)) {
    pfc_log_debug("%s: No value struct present.", PFC_FUNCNAME);
  }
 if (request_header.header.operation == UNC_OP_UPDATE) {
   if (sess.getArgument(INPUT_VAL_STRUCT_INDEX+1, val_generic2_)) {
     pfc_log_debug("%s: No odl value struct present.", PFC_FUNCNAME);
   }
 }
  return UNC_RC_SUCCESS;
}

  /**
   * @brief    - This method executes the Create, Delete, Update requests of
   *             Keytypes
   * @param[in]- ServerSession, odl_drv_request_header_t, ControllerFramework*
   *             UncRespCode, key, val
   * @retval   - UncRespCode
   **/
template<typename key, typename val>
UncRespCode
KtRequestHandler<key, val>::execute(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    unc::driver::ControllerFramework* ctrl_int,
    key &key_generic_,
    val &val_generic1_,
    val &val_generic2_) {
  ODC_FUNC_TRACE;

  std::string ctrl_name = std::string(request_header.controller_name);

  unc::driver::driver*  drv_ptr = NULL;

  unc::driver::controller* ctrl_ptr = NULL;

  UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;

  controller_operation util_obj(ctrl_int, WRITE_TO_CONTROLLER, ctrl_name);
  if (util_obj.get_controller_status() == PFC_FALSE) {
    pfc_log_debug("%s Controller not exist, send disconnected",
                                 PFC_FUNCNAME);
    return UNC_RC_CTR_DISCONNECTED;
  }
  ctrl_ptr = util_obj.get_controller_handle();
  drv_ptr = util_obj.get_driver_handle();

  PFC_ASSERT(drv_ptr != NULL);

  PFC_ASSERT(ctrl_ptr !=NULL);

  pfc_log_debug("getting conn status..ctr nt NULL");
  if (ctrl_ptr->get_connection_status() == CONNECTION_DOWN) {
    // check controller connection status, if down send disconnected
    pfc_log_debug("%s Controller status is down, send disconnected",
                                 PFC_FUNCNAME);
    return UNC_RC_CTR_DISCONNECTED;
  }

  unc::driver::driver_command * drv_command_ptr_ =
      drv_ptr->create_driver_command(request_header.key_type);

  PFC_ASSERT(drv_command_ptr_ != NULL);

  unc::driver::vtn_driver_command<key, val> * command_ptr_ =
      static_cast<unc::driver::vtn_driver_command<key, val>*>(drv_command_ptr_);
  // command_class *command_ptr_ =
  // static_cast<command_class *> (drv_command_ptr_);

  PFC_ASSERT(command_ptr_ != NULL);


  if (!drv_ptr->is_2ph_commit_support_needed()) {
    if (ctrl_ptr->controller_cache == NULL) {
      pfc_log_debug("Cache for controller created");
      ctrl_ptr->controller_cache = unc::vtndrvcache::KeyTree::create_cache();
    }
    unc::vtndrvcache::ConfigNode *cfgptr =
        new unc::vtndrvcache::CacheElementUtil<key, val, val, uint32_t> (
            &key_generic_,
            &val_generic1_,
            &val_generic2_,
            request_header.header.operation);
    if (cfgptr == NULL) {
      pfc_log_debug("%s:Exiting Function. Not able to create ConfigNode",
                    PFC_FUNCNAME);
      delete command_ptr_;
      if (ctrl_ptr->controller_cache != NULL)
        delete ctrl_ptr->controller_cache;
      return UNC_DRV_RC_ERR_GENERIC;
    }

    uint32_t size = ctrl_ptr->controller_cache->cfg_list_count();
    pfc_log_debug("before add .... size is %d", size);
    resp_code_  = ctrl_ptr->controller_cache->append_commit_node(
        cfgptr);
    size = 0;
    size = ctrl_ptr->controller_cache->cfg_list_count();
    pfc_log_debug("after add .... size is %d", size);

    if (resp_code_ != UNC_RC_SUCCESS) {
      pfc_log_debug("%s:Exiting Function. AppendCommitNode fail",
                    PFC_FUNCNAME);
      if (cfgptr != NULL)
        delete cfgptr;
      delete command_ptr_;
      return resp_code_;
    } else  {
      pfc_log_debug("Caching Success");
      resp_code_= UNC_RC_SUCCESS;
    }
  } else {
    switch (request_header.header.operation) {
      case UNC_OP_CREATE:
        pfc_log_debug("%s: Translate Create Command string", PFC_FUNCNAME);
        resp_code_ = command_ptr_->create_cmd(key_generic_,
                                              val_generic1_,
                                              ctrl_ptr);

        break;

      case UNC_OP_DELETE:
        pfc_log_debug("%s: Translate Delete Command string", PFC_FUNCNAME);

        resp_code_ = command_ptr_->delete_cmd(key_generic_,
                                              val_generic1_,
                                              ctrl_ptr);

        break;

      case UNC_OP_UPDATE:
        pfc_log_debug("%s: Translate Update Command string", PFC_FUNCNAME);

        resp_code_ = command_ptr_->update_cmd(key_generic_,
                                              val_generic2_,
                                              val_generic1_,
                                              ctrl_ptr);

        break;

      default:

        pfc_log_debug("%s: Invalid operation  ", PFC_FUNCNAME);

        resp_code_ = UNC_DRV_RC_INVALID_OPERATION;

        break;
    }
  }
  delete command_ptr_;
  return resp_code_;
}

  /**
   * @brief     - This method is the Template Specialization for parsing
   *              KT_Controller structures(Key, Val)
   * @param[in] - ServerSession, odl_drv_request_header_t, key_ctr_t, val_ctr_t
   * @retval    -  UncRespCode
   **/
template<>
UncRespCode
KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>::parse_request(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    key_ctr_t &key_generic_,
    val_ctr_commit_ver_t &val_generic1_,
    val_ctr_commit_ver_t &val_generic2_) {
  ODC_FUNC_TRACE;
  uint32_t ret_value = 0;
  ret_value = sess.getArgument(INPUT_KEY_STRUCT_INDEX, key_generic_);
  if (ret_value != 0) {
    pfc_log_error("%s: GetArgument failed to read key struct "
                  "for UNC_KT_CONTROLLER (err = %u)",
                  PFC_FUNCNAME, ret_value);
    return UNC_DRV_RC_MISSING_KEY_STRUCT;
  }

  if ((request_header.header.operation == UNC_OP_CREATE)
      || (request_header.header.operation == UNC_OP_UPDATE)) {
    ret_value = sess.getArgument(INPUT_VAL_STRUCT_INDEX, val_generic1_);
    if (ret_value != 0) {
      pfc_log_error("%s: GetArgument failed to read value struct "
                    "for UNC_KT_CONTROLLER (err = %u)",
                    PFC_FUNCNAME, ret_value);
      return UNC_DRV_RC_MISSING_VAL_STRUCT;
    }
  }

  return UNC_RC_SUCCESS;
}

  /**
   * @brief     - This method is template Specialization for parsing KT_ROOT
   * @param[in] - ServerSession, odl_drv_request_header_t, key_root_t
   *              val_root_t
   * @retval    - UncRespCode
   **/
template<>
UncRespCode
KtRequestHandler<key_root_t, val_root_t>::parse_request(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    key_root_t &key_generic_,
    val_root_t &val_generic1_,
    val_root_t &val_generic2_) {
  request_header.key_type = UNC_KT_ROOT;
  return UNC_RC_SUCCESS;
}
  /**
   * @brief     - This method retrieves the key and val structures
   * @param[in] - ServerSession, odl_drv_request_header_t, key, val
   * @retval    -  UncRespCode
   **/
template<>
UncRespCode
KtRequestHandler<key_vtn_flowfilter_t, val_flowfilter_t>::parse_request(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    key_vtn_flowfilter_t &key_generic_,
    val_flowfilter_t &val_generic1_,
    val_flowfilter_t &val_generic2_) {
  if (sess.getArgument(INPUT_KEY_STRUCT_INDEX, key_generic_)) {
    pfc_log_debug("%s: Exting Function.getArg Failed to read key struct."
                  " rc=%u", PFC_FUNCNAME, 2);
    return UNC_DRV_RC_MISSING_KEY_STRUCT;
  }
  if (sess.getArgument(INPUT_VAL_STRUCT_INDEX, val_generic1_)) {
    pfc_log_debug("%s: No value struct present.", PFC_FUNCNAME);
  }
  if ( key_generic_.input_direction == UPLL_FLOWFILTER_DIR_OUT)
    return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;

  return UNC_RC_SUCCESS;
}
  /**
   * @brief    - This method is the template Specialization for
   *             KT ROOT command Execution
   * @param[in]- ServerSession, odl_drv_request_header_t, ControllerFramework*
   *             UncRespCode, key_root_t, val_root_t
   * @retval   - UncRespCode
   **/
template<>
UncRespCode
KtRequestHandler<key_root_t, val_root_t>::execute(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    unc::driver::ControllerFramework* ctrl_int,
    key_root_t &key_generic_,
    val_root_t &val_generic1_,
    val_root_t &val_generic2_) {
  ODC_FUNC_TRACE;

  std::string ctrl_name = std::string(request_header.controller_name);
  unc::driver::driver*  drv_ptr = NULL;
  unc::driver::controller* ctrl_ptr = NULL;

  UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;

  controller_operation util_obj(ctrl_int, WRITE_TO_CONTROLLER, ctrl_name);
  ctrl_ptr = util_obj.get_controller_handle();
  drv_ptr = util_obj.get_driver_handle();

  PFC_ASSERT(drv_ptr != NULL);

  PFC_ASSERT(ctrl_ptr !=NULL);

  if (ctrl_ptr->controller_cache == NULL) {
    pfc_log_debug("Cache for controller created, ctrl_name:%s",
                  ctrl_name.c_str());
    ctrl_ptr->controller_cache = unc::vtndrvcache::KeyTree::create_cache();
  } else {
    pfc_log_debug("re-create Cache for controller, ctrl_name:%s",
                  ctrl_name.c_str());
    delete  ctrl_ptr->controller_cache;
    ctrl_ptr->controller_cache = NULL;
    ctrl_ptr->controller_cache = unc::vtndrvcache::KeyTree::create_cache();
  }

  pfc_log_debug("audit_key_type size:%u", AUDIT_KT_SIZE);

  uint32_t config_cache_size =
                     ctrl_ptr->controller_cache->audit_cfg_list_count();
          pfc_log_debug("before for.... size is %d", config_cache_size);
  uint32_t audit_kt_size = 0;

  for (int audit_kt_index = 0; audit_kt_index < AUDIT_KT_SIZE;
                               audit_kt_index++) {
    unc::driver::driver_command * drv_command_ptr_=
        drv_ptr->create_driver_command(audit_key[audit_kt_index].key_type);
    PFC_ASSERT(drv_command_ptr_ != NULL);

   /* if ( audit_key[audit_kt_index].keytype == UNC_KT_ROOT )
      break;*/

    pfc_log_debug("audit index:%u", audit_kt_index);
    if (audit_key[audit_kt_index].parent_key_type == UNC_KT_ROOT) {
      pfc_log_debug("parent is UNC_KT_ROOT");
      std::vector<unc::vtndrvcache::ConfigNode*> cfg_list;

      // fetch all root child configuration
      resp_code_ = drv_command_ptr_->fetch_config(ctrl_ptr, NULL, cfg_list);
      pfc_log_debug("vtn resp_code_%u", resp_code_);
      if (resp_code_ != UNC_RC_SUCCESS) {
        pfc_log_error("%s:GetDriverByControllerName failed VTN .rt, %u",
                       PFC_FUNCNAME, resp_code_);
        delete drv_command_ptr_;
        return resp_code_;
      }
      pfc_log_debug("%u:cfg_list.size for vtn audit index:%u",
                    static_cast<int> (cfg_list.size()), audit_kt_index);

      ctrl_ptr->controller_cache->append_audit_configuration_list(cfg_list);
      cfg_list.clear();
      audit_kt_size = ctrl_ptr->controller_cache->audit_cfg_list_count();
      pfc_log_debug("%d:audit_kt_size for vtn index: %d",
                    audit_kt_size, audit_kt_index);
    } else {
      pfc_log_debug("parent is  NOT UNC_KT_ROOT");

      std::auto_ptr<unc::vtndrvcache::CommonIterator>
          itr_ptr(ctrl_ptr->controller_cache->create_iterator());

      unc::vtndrvcache::ConfigNode* cfgnode;

      for (cfgnode = itr_ptr->AuditFirstItem();
           itr_ptr->IsDone() == false;
           cfgnode = itr_ptr->NextItem() ) {
        std::vector<unc::vtndrvcache::ConfigNode*> cfg_list;
        if (audit_key[audit_kt_index].parent_key_type ==
                                      cfgnode->get_type_name()) {
          KtHandler* kt_hnd =
              get_handler(audit_key[audit_kt_index].parent_key_type);
          void* parent_key = kt_hnd->get_key_struct(cfgnode);

          resp_code_= drv_command_ptr_->fetch_config(ctrl_ptr,
                                                     parent_key,
                                                     cfg_list);
          pfc_log_debug("resp_code_%u", resp_code_);
          if (resp_code_!= UNC_RC_SUCCESS) {
            pfc_log_error("%s:GetDriverByControllerName failed .rt, %u",
                           PFC_FUNCNAME, resp_code_);
            delete drv_command_ptr_;
            return resp_code_;
          }
          pfc_log_debug("cfg_list NOT root:%u",
                        static_cast<int> (cfg_list.size()));
          pfc_log_debug("resp_code_%u", resp_code_);
          if (cfg_list.size() > 0) {
            ctrl_ptr->controller_cache->
                append_audit_configuration_list(cfg_list);
          }
          cfg_list.clear();
          audit_kt_size = ctrl_ptr->controller_cache->audit_cfg_list_count();
          pfc_log_debug("%d:audit_kt_size for vtn index: %d",
                        audit_kt_size, audit_kt_index);
        }
      }  // inner for loop  for vtn read end
    }
    delete drv_command_ptr_;
  }  // audit ket type for loop end

  return resp_code_;
}

template<typename key, typename val>
KtHandler*
KtRequestHandler<key, val>::get_handler(unc_key_type_t keytype) {
  std::map <unc_key_type_t, KtHandler*>::iterator iter = kt_map_->begin();
  iter = kt_map_->find(keytype);
  KtHandler* hnd_ptr = NULL;
  if (iter != kt_map_->end()) {
    hnd_ptr = iter->second;
  }
  PFC_ASSERT(hnd_ptr != NULL);
  return hnd_ptr;
}

/**
 * @brief    - This method is the template Specialization for KT Controller command Execution
 * @param[in]- ServerSession, odl_drv_request_header_t, ControllerFramework*
 *             UncRespCode, key_root_t, val_root_t
 * @retval   - UncRespCode
 **/
template<>
UncRespCode
KtRequestHandler<key_ctr_t, val_ctr_commit_ver_t>::execute(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    unc::driver::ControllerFramework* ctrl_int,
    key_ctr_t &key_generic_,
    val_ctr_commit_ver_t &val_generic1_,
    val_ctr_commit_ver_t &val_generic2_) {
  ODC_FUNC_TRACE;
  unc::driver::driver *drv_ptr = NULL;
  unc::driver::controller* ctl_ptr = NULL;
  std::string ctrl_name = std::string(request_header.controller_name);

  UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;
  switch (request_header.header.operation) {
    case UNC_OP_CREATE: {
      pfc_log_debug("%s: Creates new controller ", PFC_FUNCNAME);
      controller_operation util_obj(ctrl_int, CONTROLLER_ADD, ctrl_name,
                            (unc_keytype_ctrtype_t) val_generic1_.controller.type);
      drv_ptr = util_obj.get_driver_handle();
      PFC_ASSERT(drv_ptr != NULL);
      ctl_ptr = drv_ptr->add_controller(key_generic_, val_generic1_.controller);
      if (ctl_ptr != NULL) {
        resp_code_ = UNC_RC_SUCCESS;
      } else {
        resp_code_ = UNC_DRV_RC_ERR_GENERIC;
        return resp_code_;
      }
      ctrl_int->AddController(ctrl_name, ctl_ptr, drv_ptr);
      break;
    }

    case UNC_OP_UPDATE: {
      pfc_log_debug("%s: Updated Controller ", PFC_FUNCNAME);
      controller_operation util_obj(ctrl_int, CONTROLLER_UPDATE, ctrl_name);
      ctl_ptr = util_obj.get_controller_handle();
      drv_ptr = util_obj.get_driver_handle();

      PFC_ASSERT(drv_ptr != NULL);
      PFC_ASSERT(ctl_ptr != NULL);

      resp_code_ = ctrl_int->UpdateControllerConfiguration(ctrl_name,
                             ctl_ptr, drv_ptr, key_generic_, val_generic1_.controller);
      break;
    }

    case UNC_OP_DELETE: {
      pfc_log_debug("%s: deleted Controller ", PFC_FUNCNAME);
      controller_operation util_obj(ctrl_int, CONTROLLER_DELETE, ctrl_name);
      ctl_ptr = util_obj.get_controller_handle();
      drv_ptr = util_obj.get_driver_handle();

      PFC_ASSERT(drv_ptr != NULL);
      PFC_ASSERT(ctl_ptr != NULL);

      resp_code_ =
        ctrl_int->RemoveControllerConfiguration(ctrl_name,
                                                  ctl_ptr,
                                                 drv_ptr);

      break;
    }
    default: {
      pfc_log_debug("%s: Operation not Support ", PFC_FUNCNAME);
      resp_code_ = UNC_DRV_RC_INVALID_OPERATION;
      break;
    }
  }
  return resp_code_;
}

  /**
   * @brief    - This method handles request received from platform layer
   * @param[in]- ServerSession, odl_drv_request_header_t, ControllerFramework*
   * @retval   - UncRespCode
   **/
template<typename key, typename val>
UncRespCode
KtRequestHandler<key, val>::handle_request(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    unc::driver::ControllerFramework* ctrl_int) {
  ODC_FUNC_TRACE;

  UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;
  key key_generic_;
  val val_generic1_;
  val val_generic2_;
  memset(&key_generic_, 0, sizeof(key));
  memset(&val_generic1_, 0, sizeof(val));
  memset(&val_generic2_, 0, sizeof(val));

  PFC_ASSERT(ctrl_int != NULL);


  resp_code_ = parse_request(sess, request_header, key_generic_, val_generic1_,
                             val_generic2_);

  if (resp_code_ == UNC_RC_SUCCESS) {
    resp_code_ = execute(sess, request_header, ctrl_int,
                         key_generic_, val_generic1_,
                         val_generic2_);
    if (resp_code_ != UNC_RC_SUCCESS) {
       pfc_log_error("%s: execute return err with resp_code(err = %u)",
               PFC_FUNCNAME, resp_code_);
    }
  } else {
    pfc_log_error("%s:  Failed to parse key and val struct(err = %u)",
                  PFC_FUNCNAME, resp_code_);
  }

  resp_code_ = handle_response(sess, request_header, ctrl_int,
                       key_generic_, val_generic1_, resp_code_);

  if (resp_code_ != UNC_RC_SUCCESS) {
    pfc_log_error("%s:. Failed to send response(err = %u)",
                  PFC_FUNCNAME, resp_code_);
  }

  return resp_code_;
}

  /**
   * @brief    - This method forms the command from ConfigNode pointer
   * @param[in]- ConfigNode*, controller*, driver*
   * @retval   - UncRespCode
   **/
template<typename key, typename val>
UncRespCode
KtRequestHandler<key, val>::execute_cmd(
    unc::vtndrvcache::ConfigNode *cfgptr,
    unc::driver::controller* conn,
    unc::driver::driver* drv_ptr) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::CacheElementUtil<key, val, val, uint32_t> * cache_element_ptr =
      dynamic_cast <unc::vtndrvcache::CacheElementUtil
      <key, val, val, uint32_t> * > (cfgptr);

  UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;

  PFC_ASSERT(cache_element_ptr != NULL);

  unc_key_type_t keytype = cfgptr->get_type_name();
  unc::driver::driver_command * driver_cmd_ptr =
      drv_ptr->create_driver_command(keytype);

  // command_class * config_cmd_ptr = NULL;
  // config_cmd_ptr = static_cast<command_class *> (drv_command_ptr_);
  unc::driver::vtn_driver_command<key, val> * config_cmd_ptr =
      static_cast<unc::driver::vtn_driver_command<key, val> *>(driver_cmd_ptr);
  uint32_t config_operation = cfgptr->get_operation();
  pfc_log_debug("keytype:%u, config_operation:%u", keytype, config_operation);

  switch (config_operation) {
    case UNC_OP_CREATE:
      pfc_log_debug("%s: Translate Create Command string", PFC_FUNCNAME);
      resp_code_ = config_cmd_ptr->create_cmd(
                   *(cache_element_ptr->get_key_structure()),
                   *(cache_element_ptr->get_val_structure()), conn);

      break;
    case UNC_OP_DELETE:
      pfc_log_debug("%s: Translate Delete Command string", PFC_FUNCNAME);

      resp_code_ = config_cmd_ptr->delete_cmd(
                              *(cache_element_ptr->get_key_structure()),
                              *(cache_element_ptr->get_val_structure()), conn);
      break;
    case UNC_OP_UPDATE:
      pfc_log_debug("%s: Translate Update Command string", PFC_FUNCNAME);

      resp_code_ = config_cmd_ptr->update_cmd(
                              *(cache_element_ptr->get_key_structure()),
                              *(cache_element_ptr->get_old_val_structure()),
                              *(cache_element_ptr->get_val_structure()),
                              conn);

      break;
    default:
      pfc_log_error("%s: Invalid operation  ", PFC_FUNCNAME);
      resp_code_ = UNC_DRV_RC_INVALID_OPERATION;

      pfc_log_debug("deleted config_cmd_ptr");
      delete config_cmd_ptr;
      config_cmd_ptr = NULL;

      return resp_code_;
  }

  pfc_log_debug("deleted config_cmd_ptr");
  delete config_cmd_ptr;
  config_cmd_ptr = NULL;
  return resp_code_;
}

  /**
   * @brief     - This method handles response from controller,
   *              doesnot support audit
   * @param[in] - ServerSession, odl_drv_request_header_t, ControllerFramework*,
   *              key, val, UncRespCode
   * @retval    - UncRespCode
   **/
template<typename key, typename val>
UncRespCode
KtRequestHandler<key, val>::handle_response(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    unc::driver::ControllerFramework* ctrl_int,
    key &key_generic_,
    val &val_generic_,
    UncRespCode &controller_resp_code_ ) {
  ODC_FUNC_TRACE;

  unc::driver::odl_drv_response_header_t resp_hdr;
  uint32_t err_= 0;
  create_response_header(request_header, resp_hdr, controller_resp_code_);
  UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;
  resp_code_ = populate_response_header(sess, resp_hdr);
  if (resp_code_ != UNC_RC_SUCCESS) {
    pfc_log_error("%s: populate_response_header failed with ret_code , %u",
                  PFC_FUNCNAME, resp_code_);
    return resp_code_;
  }

  err_ = sess.addOutput(resp_hdr.result);
  if (err_ != 0) {
    pfc_log_error("%s: Failed to send resp code:(err = %d)",
                  PFC_FUNCNAME, resp_code_);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  err_ = sess.addOutput((uint32_t) resp_hdr.key_type);
  if (err_ != 0) {
    pfc_log_error("%s: Failed to send driver data key type: (err = %d)",
                  PFC_FUNCNAME, resp_code_);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if (resp_hdr.header.data_type != UNC_DT_RUNNING) {
    uint32_t ret_code = sess.addOutput(key_generic_);
    if (ret_code) {
      pfc_log_error("%s: addOutput failed with ret_code , %u",
                    PFC_FUNCNAME, ret_code);
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  return UNC_RC_SUCCESS;
}


  /**
   * @brief     - This method handles response from controller for Audit
   * @param[in] - ServerSession, odl_drv_request_header_t, ControllerFramework*,
   *              key_root_t, val_root_t, UncRespCode
   * @retval    - UncRespCode
   **/
template<>
UncRespCode
KtRequestHandler<key_root_t, val_root_t>::handle_response(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    unc::driver::ControllerFramework* ctrl_int,
    key_root_t &key_generic_,
    val_root_t &val_generic_,
    UncRespCode &controller_resp_code_ ) {
  ODC_FUNC_TRACE;
  controller* ctr = NULL;
  memset(&key_generic_, 0, sizeof(key_root_t));

  unc::driver::odl_drv_response_header_t resp_hdr;
  create_response_header(request_header, resp_hdr, controller_resp_code_);
  UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;
  resp_code_ = populate_response_header(sess, resp_hdr);
  if (resp_code_ != UNC_RC_SUCCESS) {
    pfc_log_error("%s: populate_response_header failed"
                  "with ret_code root , %u", PFC_FUNCNAME, resp_code_);
    return resp_code_;
  }
  std::string ctr_name(resp_hdr.controller_name);
  if (resp_hdr.header.data_type == UNC_DT_RUNNING) {
    pfc_log_debug("UNC_DT_RUNNING processing");
    controller_operation util_obj(ctrl_int, READ_FROM_CONTROLLER, ctr_name);
    ctr = util_obj.get_controller_handle();

    if (ctr != NULL) {
      if (ctr->controller_cache == NULL) {
        pfc_log_error("Not getting controller_cache ");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      uint32_t size = ctr->controller_cache->audit_cfg_list_count();
      pfc_log_debug("config node size is %d for controller %s",
                    size, ctr_name.c_str());
      // Not Success and Error Case check for Reps
      if ((size == 0) &&
          (resp_hdr.result == UNC_RC_NO_SUCH_INSTANCE)) {
        pfc_log_debug("%s: Value list empty and Response"
                      "header success", PFC_FUNCNAME);
        pfc_log_debug("%s: Adding Root key only", PFC_FUNCNAME);
        sess.addOutput((uint32_t) UNC_RC_NO_SUCH_INSTANCE);
        sess.addOutput((uint32_t) UNC_KT_ROOT);
        sess.addOutput(key_generic_);

        // delete cache
        delete  ctr->controller_cache;
        ctr->controller_cache = NULL;

        pfc_log_trace("ResponseHandler::%s Exiting", PFC_FUNCNAME);
        return UNC_RC_SUCCESS;
      } else if (resp_hdr.result != UNC_RC_SUCCESS) {
        pfc_log_debug("%s: Response header result Error",
                      PFC_FUNCNAME);
        pfc_log_debug("%s: Adding Root Key only", PFC_FUNCNAME);
        sess.addOutput((uint32_t) resp_hdr.result);
        sess.addOutput((uint32_t) UNC_KT_ROOT);
        sess.addOutput(key_generic_);
        // delete cache
        delete  ctr->controller_cache;
        ctr->controller_cache = NULL;
        pfc_log_trace("ResponseHandler::%s Exiting", PFC_FUNCNAME);
        return UNC_DRV_RC_ERR_GENERIC;
      }

      std::auto_ptr<unc::vtndrvcache::CommonIterator>
              itr_ptr(ctr->controller_cache->create_iterator());

      pfc_log_trace("Audit cfgnode need to add");
      int err_ = sess.addOutput(resp_hdr.result);
      if (err_ != 0) {
        pfc_log_fatal("%s: Failed to send resp code audit:(err = %d)",
                      PFC_FUNCNAME, err_);
        return UNC_DRV_RC_ERR_GENERIC;
      }

      bool first = true;
      unc::vtndrvcache::ConfigNode *cfgnode = NULL;

      for (cfgnode = itr_ptr->AuditFirstItem();
           itr_ptr->IsDone() == false;
           cfgnode = itr_ptr->NextItem() ) {
        if (cfgnode == NULL) {
          pfc_log_debug("cfgnode is NULL before get_type");
          delete  ctr->controller_cache;
          ctr->controller_cache = NULL;

          return UNC_DRV_RC_ERR_GENERIC;
        }
        if (first) {
          first = false;
        } else {
          sess.addOutput();
          pfc_log_debug("Add empty success");
        }
        unc_key_type_t keytype = cfgnode->get_type_name();
        pfc_log_debug("%u, keytype", keytype);

        uint32_t ret_code = sess.addOutput((uint32_t)keytype);
        if (ret_code) {
          pfc_log_error("%s: addOutput failed for keytype , %u",
                        PFC_FUNCNAME, ret_code);
          delete  ctr->controller_cache;
          ctr->controller_cache = NULL;

          return UNC_DRV_RC_ERR_GENERIC;
        }
        pfc_log_debug("Add key type success");

        void* key = get_key_struct(cfgnode);
        void* val = get_val_struct(cfgnode);
        pfc_ipcstdef_t* key_sdf = VtnDrvIntf::key_map.find(keytype)->second;
        pfc_ipcstdef_t* val_sdf = VtnDrvIntf::val_map.find(keytype)->second;
        ret_code = sess.addOutput(*key_sdf, key);
        if (ret_code) {
          pfc_log_error("%s: addOutput failed for key_sdf with ret_code , %u",
                        PFC_FUNCNAME, ret_code);
          delete  ctr->controller_cache;
          ctr->controller_cache = NULL;

          return UNC_DRV_RC_ERR_GENERIC;
        }

        pfc_log_debug("Add key success");
        ret_code = sess.addOutput(*val_sdf, val);
        if (ret_code) {
          pfc_log_error("%s: addOutput failed for val_sdf with ret_code , %u",
                        PFC_FUNCNAME, ret_code);
          delete  ctr->controller_cache;
          ctr->controller_cache = NULL;

          return UNC_DRV_RC_ERR_GENERIC;
        }
        pfc_log_debug("Add val success");
      }  // for loop
      // delete cache
      delete  ctr->controller_cache;
      ctr->controller_cache = NULL;
    } else {
      pfc_log_error("%s: ctr is NULL", PFC_FUNCNAME);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    pfc_log_debug("UNC_DT_RUNNING processing complete");
  } else {
       pfc_log_error("%s: unsupported datatype %u",
          PFC_FUNCNAME, resp_hdr.header.data_type);
       return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}

  /**
   * @brief    - This method creates the Response Header
   * @param[in]- odl_drv_response_header_t,
   *             odl_drv_request_header_t, UncRespCode
   * @retval   - void
   */
template<typename key, typename val>
void
KtRequestHandler<key, val>::create_response_header(
    unc::driver::odl_drv_request_header_t &reqhdr,
    unc::driver::odl_drv_response_header_t &resphdr,
    UncRespCode &resp_code_) {
  memset(&resphdr, 0, sizeof(resphdr));

  /* Copy parameters from the request header to the response header. */
  memcpy(reinterpret_cast<void *>(&resphdr.header),
         reinterpret_cast<void *>(&reqhdr.header),
         sizeof(resphdr.header));

  char * resphdr_domain_id = reinterpret_cast<char*>
                             (&resphdr.header.domain_id);

  char * reqhdr_domain_id = reinterpret_cast<char*>
                             (&reqhdr.header.domain_id);

  pfc_log_debug("%s: resphdr.header.domain_id %s", PFC_FUNCNAME,
                resphdr_domain_id);

  pfc_log_debug("%s: reqhdr.header.domain_id %s", PFC_FUNCNAME,
                reqhdr_domain_id);

  /* Set the result code. */
  pfc_log_trace("%s: Result code: %u", PFC_FUNCNAME, resp_code_);

  resphdr.result = resp_code_;

  memcpy(resphdr.controller_name, reqhdr.controller_name,
         sizeof(resphdr.controller_name));
  pfc_log_trace("REQ  controller_name : %s", reqhdr.controller_name);

  resphdr.key_type = reqhdr.key_type;
}

  /**
   * @brief    - This method populates the Response Header
   * @param[in]- ServerSession, odl_drv_response_header_t
   * @retval   - UncRespCode
   **/
template<typename key, typename val>
UncRespCode
KtRequestHandler<key, val>::populate_response_header(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_response_header_t &resp_hdr) {
  ODC_FUNC_TRACE;

  uint32_t err = UNC_DRV_RC_ERR_GENERIC;

  const char* ctr_name =
      reinterpret_cast<const char*> (resp_hdr.controller_name);

  pfc_log_debug("%s: Request received from "
                "platfotm layer. controller_name = %s \n"
                "session_id = %u \n"
                "config_id = %u \n"
                "operation = %u \n"
                "option1 = %u \n"
                "option2 = %u \n"
                "data_type = %u \n"
                "result = %u \n", PFC_FUNCNAME, ctr_name,
                resp_hdr.header.session_id,
                resp_hdr.header.config_id, resp_hdr.header.operation,
                resp_hdr.header.option1, resp_hdr.header.option2,
                resp_hdr.header.data_type, resp_hdr.result);

  /* Sends session_id */
  err = sess.addOutput(resp_hdr.header.session_id);
  if (err) {
    pfc_log_fatal("%s: Failed to send client session id: (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends config_id */
  err = sess.addOutput(resp_hdr.header.config_id);
  if (err) {
    pfc_log_fatal("%s: Failed to send configurationid: (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends controller_name */
  const char* controller_name =
      reinterpret_cast<const char *> (resp_hdr.controller_name);
  err = sess.addOutput(controller_name);
  if (err) {
    pfc_log_fatal("%s: Failed to send controller id: (err = %d)", PFC_FUNCNAME,
                  err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends domain-id*/
  const char* domain_id =
      reinterpret_cast<const char *> (resp_hdr.header.domain_id);
  err = sess.addOutput(domain_id);
  if (err) {
    pfc_log_fatal("%s: Failed to send domain id:(err = %d)", PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends operation */
  err = sess.addOutput((uint32_t) resp_hdr.header.operation);
  if (err) {
    pfc_log_fatal("%s: Failed to send driver data operation (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends max-rep-count */
  err = sess.addOutput(resp_hdr.header.max_rep_count);
  if (err) {
    pfc_log_fatal("%s: Failed to send driver data max_rep_count: (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends option1 */
  err = sess.addOutput(resp_hdr.header.option1);
  if (err) {
    pfc_log_fatal("%s: Failed to send driver data option1 : (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends option2 */
  err = sess.addOutput(resp_hdr.header.option2);
  if (err) {
    pfc_log_fatal("%s: Failed to send driver data option2 : (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends data type */
  err = sess.addOutput((uint32_t) resp_hdr.header.data_type);
  if (err) {
    pfc_log_fatal("%s: Failed to send driver data data_type: (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  return UNC_RC_SUCCESS;
}
}  //  namespace driver
}  //  namespace unc
#endif
