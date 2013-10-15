/*
 * Copyright (c) 2012-2013 NEC Corporation
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
#include <vtndrvintf_defs.h>
#include <map>
#include <string>
#include <utility>
#include <vector>


namespace unc {
namespace driver {

/**
* @brief KtRequestHandler provides function for handling request and response
*
**/

template<typename key, typename val, typename command_ptr>
class KtRequestHandler : public KtHandler {
      public:
      /*
      * @brief Default Constructor
      */

      KtRequestHandler();

       /*
       * @brief Default Destructor
       */

      ~KtRequestHandler();

       /**
       * @brief -   Handling Request received from platform
       * @param[in] ServerSession, keyif_drv_request_header_t,ControllerFramework
       * @retval -  drv_resp_code_t
       **/

       drv_resp_code_t
       handle_request(pfc::core::ipc::ServerSession &sess,
              unc::driver::keyif_drv_request_header_t &request_header,
              unc::driver::ControllerFramework* ctrl_int);

       /**
       * @brief -   Retrieve key and val struct
       * @param[in] ServerSession, keyif_drv_request_header_t
       * @retval -  drv_resp_code_t
       **/

        drv_resp_code_t
        parse_request(pfc::core::ipc::ServerSession &sess,
                      unc::driver::keyif_drv_request_header_t &request_header,
                      key &key_generic_,
                      val &val_generic_);

       /**
       * @brief - Create,Delete,Update Of KT
       * @param[in] ServerSession, keyif_drv_request_header_t, ControllerFramework
       * @retval - drv_resp_code_t
       **/

        drv_resp_code_t
        execute(pfc::core::ipc::ServerSession &sess,
                unc::driver::keyif_drv_request_header_t &request_header,
                unc::driver::ControllerFramework* ctrl_int,
                drv_resp_code_t &resp_code_,
                key &key_generic_,
                val &val_generic_);


       /**
       * @brief - Handle Response
       * @param[in] ServerSession, keyif_drv_request_header_t
       * @retval - drv_resp_code_t
       **/

        drv_resp_code_t
        handle_response(pfc::core::ipc::ServerSession &sess,
            unc::driver::keyif_drv_request_header_t &request_header,
            unc::driver::ControllerFramework* ctrl_int,
            key &key_generic_,
            val &val_generic_,
            drv_resp_code_t &resp_code_);


       /**
       * @brief - Response Header Creation
       * @param[in] keyif_drv_response_header_t, keyif_drv_request_header_t
       * @retval - void 
       */

        void
        create_response_header(unc::driver::keyif_drv_request_header_t &reqhdr,
            unc::driver::keyif_drv_response_header_t &resphdr,
            drv_resp_code_t &resp_code_);


       /**
       * @brief - Populating Response Header
       * @param[in] ServerSession, keyif_drv_response_header_t
       * @retval - drv_resp_code_t
       **/

        drv_resp_code_t
        populate_response_header(pfc::core::ipc::ServerSession &sess,
            unc::driver::keyif_drv_response_header_t &resp_hdr);


       /**
       * @brief - Converting ConfigNode pointer to Command 
       * @param[in] ConfigNode*, controller*, driver*
       * @retval - drv_resp_code_t
       **/

        drv_resp_code_t
        execute_cmd(unc::vtndrvcache::ConfigNode *cfgptr,
            unc::driver::controller* ctl_ptr,
            unc::driver::driver* drv_ptr);


       /**
       * @brief - Retrieve Key struct
       * @param[in] ConfigNode*
       * @retval - drv_resp_code_t
       **/

        void* get_key_struct(unc::vtndrvcache::ConfigNode *cfgptr);


       /**
       * @brief - Retrieve Val struct
       * @param[in] ConfigNode*
       * @retval - drv_resp_code_t
       **/

        void* get_val_struct(unc::vtndrvcache::ConfigNode *cfgptr);

       /**
       * @brief -   Adding config nodes to vtncache
       * @param[in] cfg_list
       * @param[in] keytree_ptr
       * @retval -  void
       **/ 
        void
        fill_cache(std::vector<unc::vtndrvcache::ConfigNode*> &cfg_list,
        unc::vtndrvcache::KeyTree *keytree_ptr,
        root_driver_command* command_ptr_root,
        unc::driver::controller* ctl_ptr,
        drv_resp_code_t &resp_code_);

       /**
       * @brief -  Initializing map for STDEF 
       * @retval -  void
       **/ 
        void initialize_map();

        private:
        std::map<unc_key_type_t, pfc_ipcstdef_t*> key_map_;
        std::map<unc_key_type_t, pfc_ipcstdef_t*> val_map_;
};


/**
* @brief: Default Constructor
**/

template<typename key, typename val, typename command_ptr>
KtRequestHandler<key, val, command_ptr>::KtRequestHandler() { }

/**
* @brief: Default Destructor 
**/


template<typename key, typename val, typename command_ptr>
KtRequestHandler<key, val, command_ptr>::~KtRequestHandler() { }


/**
* @brief -  Retrieve key structure
**/

template<typename key, typename val, typename command_ptr>
void* KtRequestHandler<key, val, command_ptr>::
      get_key_struct(unc::vtndrvcache::ConfigNode *cfgptr) {
  unc::vtndrvcache::CacheElementUtil <key, val, uint32_t> * cache_util_ptr =
      static_cast <unc::vtndrvcache::CacheElementUtil<key, val, uint32_t> * >
      (cfgptr);

  if (cache_util_ptr == NULL) {
    pfc_log_error("%s: cache_util_ptr is null", PFC_FUNCNAME);
    return NULL;
  }

  return cache_util_ptr->getkey();
}

/**
* @brief -  Retrieve Value Structure
**/

template<typename key, typename val, typename command_ptr>
void* KtRequestHandler<key, val, command_ptr>::
      get_val_struct(unc::vtndrvcache::ConfigNode *cfgptr) {
  unc::vtndrvcache::CacheElementUtil<key, val, uint32_t> * cache_util_ptr =
                    static_cast <unc::vtndrvcache::CacheElementUtil
  <key, val, uint32_t> * >(cfgptr);

  if (cache_util_ptr == NULL) {
    pfc_log_error("%s: cache_util_ptr is null for get_val_struct",
                  PFC_FUNCNAME);
    return NULL;
  }


  return cache_util_ptr->getval();
}


/**
* @brief -  parse key and value structure
**/


template<typename key, typename val, typename command_ptr>
drv_resp_code_t
KtRequestHandler<key, val, command_ptr>::parse_request(
                   pfc::core::ipc::ServerSession &sess,
                   unc::driver::keyif_drv_request_header_t &request_header,
                   key &key_generic_,
                   val &val_generic_) {
  if (sess.getArgument(INPUT_KEY_STRUCT_INDEX, key_generic_)) {
     pfc_log_debug("%s: Exting Function.getArg Failed to read key struct."
     " rc=%u", PFC_FUNCNAME, 2);
      return DRVAPI_RESPONSE_CTRLAPI_FAILURE;
  }
  if (sess.getArgument(INPUT_VAL_STRUCT_INDEX, val_generic_)) {
       pfc_log_debug("%s: No value struct present.", PFC_FUNCNAME);
  }

  return DRVAPI_RESPONSE_SUCCESS;
}


/**
* @brief - Create , Delete and Update Command execution  
**/

template<typename key, typename val, typename command_ptr>
drv_resp_code_t
KtRequestHandler<key, val, command_ptr>::execute(
               pfc::core::ipc::ServerSession &sess,
               unc::driver::keyif_drv_request_header_t &request_header,
               unc::driver::ControllerFramework* ctrl_int,
               drv_resp_code_t &resp_code_,
               key &key_generic_,
               val &val_generic_) {
  pfc_log_debug("%s: Entering Function.", PFC_FUNCNAME);

  std::string ctrl_name = std::string(request_header.controller_name);

  unc::driver::driver*  drv_ptr = NULL;
  unc::driver::controller* ctrl_ptr = NULL;
  drv_resp_code_t ret_val = DRVAPI_RESPONSE_FAILURE;
  VtnDrvRetEnum ctl_retval = VTN_DRV_RET_FAILURE;
  resp_code_ = DRVAPI_RESPONSE_FAILURE;

  ctl_retval = ctrl_int->GetDriverByControllerName(ctrl_name,
                                                   &ctrl_ptr,
                                                   &drv_ptr);
  pfc_log_debug("%u:resp_code_. GetDriverByControllerName",
                ctl_retval);

  if (ctl_retval == VTN_DRV_RET_FAILURE) {
      pfc_log_error("%s:GetDriverByControllerName failed with re_code,%u",
                 PFC_FUNCNAME, resp_code_);
      return DRVAPI_RESPONSE_FAILURE;
  }

  PFC_ASSERT(drv_ptr != NULL);

  PFC_ASSERT(ctrl_ptr !=NULL);

  unc::driver::driver_command * drv_command_ptr_ =
                          drv_ptr->get_driver_command(request_header.key_type);

  PFC_ASSERT(drv_command_ptr_ != NULL);

  command_ptr *command_ptr_ = static_cast<command_ptr *> (drv_command_ptr_);

  PFC_ASSERT(command_ptr_ != NULL);


  if (!drv_ptr->is_2ph_commit_support_needed()) {
      if (ctrl_ptr->keytree_ptr == NULL) {
              pfc_log_debug("Cache for controller created");
              ctrl_ptr->keytree_ptr = unc::vtndrvcache::KeyTree::create_cache();
      }
      unc::vtndrvcache::ConfigNode *cfgptr =
                new unc::vtndrvcache::CacheElementUtil<key, val, uint32_t> (
                                          &key_generic_,
                                          &val_generic_,
                                          request_header.header.operation);
      if (cfgptr == NULL) {
        pfc_log_debug("%s:Exiting Function. Not able to create ConfigNode",
                           PFC_FUNCNAME);
        delete command_ptr_;
        return DRVAPI_RESPONSE_FAILURE;
      }

      uint32_t size = ctrl_ptr->keytree_ptr->cfg_list_count();
      pfc_log_debug("before add .... size is %d", size);
      ret_val  = (drv_resp_code_t) ctrl_ptr->keytree_ptr->append_commit_node(
                                                          cfgptr);
      size = 0;
      size = ctrl_ptr->keytree_ptr->cfg_list_count();
      pfc_log_debug("after add .... size is %d", size);

      if (ret_val) {
              pfc_log_debug("%s:Exiting Function. AppendCommitNode fail",
                           PFC_FUNCNAME);
              if (cfgptr != NULL)
                delete cfgptr;
              delete command_ptr_;
              return resp_code_;
      } else  {
        pfc_log_debug("Caching Success");
        resp_code_= DRVAPI_RESPONSE_SUCCESS;
      }
  } else {
    switch (request_header.header.operation) {
       case UNC_OP_CREATE:
       pfc_log_debug("%s: Translate Create Command string", PFC_FUNCNAME);
       resp_code_ = command_ptr_->create_cmd(key_generic_,
                                             val_generic_,
                                             ctrl_ptr);

       break;

    case UNC_OP_DELETE:
      pfc_log_debug("%s: Translate Delete Command string", PFC_FUNCNAME);

      resp_code_ = command_ptr_->delete_cmd(key_generic_,
                                            val_generic_,
                                            ctrl_ptr);

      break;


    case UNC_OP_UPDATE:
      pfc_log_debug("%s: Translate Update Command string", PFC_FUNCNAME);

      resp_code_ = command_ptr_->update_cmd(key_generic_,
                                            val_generic_,
                                            ctrl_ptr);

      break;

    default:

      pfc_log_debug("%s: Invalid operation  ", PFC_FUNCNAME);

      resp_code_ = DRVAPI_RESPONSE_FAILURE;

      break;
    }
  }
  pfc_log_debug("%s: Exiting Function.", PFC_FUNCNAME);
  delete command_ptr_;
  return resp_code_;
}


/**
* @brief:Template Specialization for parsing KT_Controller structures(Key,Val)
**/

template<>
drv_resp_code_t
KtRequestHandler<key_ctr_t, val_ctr_t, controller_command>::parse_request(
                      pfc::core::ipc::ServerSession &sess,
                      unc::driver::keyif_drv_request_header_t &request_header,
                      key_ctr_t &key_generic_,
                      val_ctr_t &val_generic_) {
  pfc_log_debug("%s: Entering Function.", PFC_FUNCNAME);
  uint32_t ret_value = DRVAPI_RESPONSE_FAILURE;
  ret_value = sess.getArgument(INPUT_KEY_STRUCT_INDEX, key_generic_);
  if (ret_value) {
      pfc_log_error("%s: GetArgument failed to read key struct "
            "for UNC_KT_CONTROLLER (err = %u)",
             PFC_FUNCNAME, ret_value);
      return DRVAPI_RESPONSE_MISSING_KEY_STRUCT;
  }

  if ((request_header.header.operation == UNC_OP_CREATE)
        || (request_header.header.operation == UNC_OP_UPDATE)) {
       ret_value = sess.getArgument(INPUT_VAL_STRUCT_INDEX, val_generic_);
       if (ret_value) {
           pfc_log_error("%s: GetArgument failed to read value struct "
                        "for UNC_KT_CONTROLLER (err = %u)",
                         PFC_FUNCNAME, ret_value);
           return DRVAPI_RESPONSE_MISSING_VAL_STRUCT;
       }
  }

  pfc_log_debug("%s: Exiting Function.", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

/**
 * @brief:Template Specialization for parsing KT_ROOT
**/

template<>
drv_resp_code_t
KtRequestHandler<key_root_t, val_root_t, root_driver_command>::parse_request(
                      pfc::core::ipc::ServerSession &sess,
                      unc::driver::keyif_drv_request_header_t &request_header,
                      key_root_t &key_generic_,
                      val_root_t &val_generic_) {
         request_header.key_type = UNC_KT_ROOT;
         return DRVAPI_RESPONSE_SUCCESS;
}

/**
 * @brief:Template Specialization for KT ROOT command Execution
**/

template<>
drv_resp_code_t
KtRequestHandler<key_root_t, val_root_t, root_driver_command>::execute(
               pfc::core::ipc::ServerSession &sess,
               unc::driver::keyif_drv_request_header_t &request_header,
               unc::driver::ControllerFramework* ctrl_int,
               drv_resp_code_t &resp_code_,
               key_root_t &key_generic_,
               val_root_t &val_generic_) {
  pfc_log_debug("%s: Entering Function.", PFC_FUNCNAME);

  std::string ctrl_name = std::string(request_header.controller_name);
  drv_resp_code_t ret_val_ = DRVAPI_RESPONSE_FAILURE;
  unc::driver::driver*  drv_ptr = NULL;
  unc::driver::controller* ctrl_ptr = NULL;
  VtnDrvRetEnum ctl_retval = VTN_DRV_RET_FAILURE;
  resp_code_ = DRVAPI_RESPONSE_FAILURE;
  ctl_retval = ctrl_int->GetDriverByControllerName(ctrl_name,
                                                   &ctrl_ptr,
                                                   &drv_ptr);
  pfc_log_debug("%u:ctl_retval. GetDriverByControllerName,ctrl_name%s",
               ctl_retval, ctrl_name.c_str());

  if (ctl_retval == VTN_DRV_RET_FAILURE) {
      pfc_log_error("%s:GetDriverByControllerName failed .rt,%u",
                 PFC_FUNCNAME, resp_code_);
      return resp_code_;
  }

  PFC_ASSERT(drv_ptr != NULL);

  PFC_ASSERT(ctrl_ptr !=NULL);

  unc::driver::driver_command * drv_command_ptr_=
                  drv_ptr->get_driver_command(request_header.key_type);

  PFC_ASSERT(drv_command_ptr_ != NULL);

  root_driver_command * command_ptr_root = static_cast<root_driver_command *>
                                           (drv_command_ptr_);

  PFC_ASSERT(command_ptr_root != NULL);

  std::vector<unc::vtndrvcache::ConfigNode*> cfg_list;

  if (ctrl_ptr->keytree_ptr == NULL) {
        pfc_log_debug("Cache for controller created, ctrl_name:%s",
                     ctrl_name.c_str());
       ctrl_ptr->keytree_ptr = unc::vtndrvcache::KeyTree::create_cache();
  } else {
    pfc_log_debug("re-create Cache for controller, ctrl_name:%s",
                 ctrl_name.c_str());
    delete  ctrl_ptr->keytree_ptr;
    ctrl_ptr->keytree_ptr = NULL;
    ctrl_ptr->keytree_ptr = unc::vtndrvcache::KeyTree::create_cache();
  }


  cfg_list.clear();

  ret_val_ = command_ptr_root->read_root_child(cfg_list, ctrl_ptr);
  pfc_log_debug("%dVTN..cfg_list.size:", static_cast<int> (cfg_list.size()));

  if (ret_val_) {
    pfc_log_error("%s:Reading all vtn failed", PFC_FUNCNAME);
    delete command_ptr_root;
    return ret_val_;
  } else {
    pfc_log_debug("%s:Reading all vtn done", PFC_FUNCNAME);
    ret_val_ = (drv_resp_code_t) ctrl_ptr->keytree_ptr->append_audit_node(
                                                        cfg_list);
    if (ret_val_ != TREE_OK) {
      pfc_log_debug("append_audit_node fail ..%s", PFC_FUNCNAME);
      delete command_ptr_root;
      return  DRVAPI_RESPONSE_FAILURE;
    }
    fill_cache(cfg_list, ctrl_ptr->keytree_ptr,
       command_ptr_root, ctrl_ptr, resp_code_);
  }

  delete command_ptr_root;
  return DRVAPI_RESPONSE_SUCCESS;
}

/**
* @brief: Recursive Function for filling cache
**/
template<typename key, typename val, typename command_ptr>
void
KtRequestHandler<key, val, command_ptr>::fill_cache(
                         std::vector<unc::vtndrvcache::ConfigNode*> &cfg_list,
                         unc::vtndrvcache::KeyTree *keytree_ptr,
                         root_driver_command* command_ptr_root,
                         unc::driver::controller* ctrl_ptr,
                         drv_resp_code_t &resp_code_) {
  pfc_log_debug("inside fill_cache.");
  std::vector<unc::vtndrvcache::ConfigNode*>::iterator it_node =
                                                cfg_list.begin();
  std::vector<unc::vtndrvcache::ConfigNode*>::iterator it_node_end =
                                                cfg_list.end();
  for (; it_node != it_node_end; it_node++) {
        pfc_log_debug("for loop continue.");
        std::vector<unc::vtndrvcache::ConfigNode*> cfg_child_list;
        unc::vtndrvcache::ConfigNode * cfg_ptr =
             static_cast<unc::vtndrvcache::ConfigNode*>(*it_node);
        cfg_child_list.clear();
        pfc_log_debug("before read_all_child");
        uint32_t ret_code = command_ptr_root->read_all_child(cfg_ptr,
                                           cfg_child_list, ctrl_ptr);

        pfc_log_debug("cfg_child_list.size():%d",
                      static_cast<int> (cfg_child_list.size()));
        if (!ret_code) {
            pfc_log_debug("ret_code: %u", ret_code);
            if (cfg_child_list.size()) {
               resp_code_ =
               (drv_resp_code_t)keytree_ptr->append_audit_node(cfg_child_list);
               if (resp_code_) {
                 pfc_log_error("%s:append_audit_node failed"
                               "child with resp_code %u",
                       PFC_FUNCNAME, resp_code_);
                 break;
               }
               fill_cache(cfg_child_list, keytree_ptr, command_ptr_root,
                          ctrl_ptr, resp_code_);
            } else {
               pfc_log_debug("NO child for this node...");
            }
        } else {
          pfc_log_debug("This NO is not exist. NO append_audit_node");
        }
  }
}



/**
* @brief:Template Specialization for KT Controller command Execution  
**/

template<>
drv_resp_code_t
KtRequestHandler<key_ctr_t, val_ctr_t, controller_command>::execute(
               pfc::core::ipc::ServerSession &sess,
               unc::driver::keyif_drv_request_header_t &request_header,
               unc::driver::ControllerFramework* ctrl_int,
               drv_resp_code_t &resp_code_,
               key_ctr_t &key_generic_,
               val_ctr_t &val_generic_) {
     pfc_log_debug("%s: Entering Function.", PFC_FUNCNAME);
     unc::driver::driver *drv_ptr = NULL;
     unc::driver::controller* ctl_ptr = NULL;
     std::string ctrl_name = std::string(request_header.controller_name);


     switch (request_header.header.operation) {
       case UNC_OP_CREATE: {
           pfc_log_debug("%s: Creates new controller ", PFC_FUNCNAME);
           drv_ptr = ctrl_int->GetDriverInstance((unc_keytype_ctrtype_t)
                                                val_generic_.type);
           PFC_ASSERT(drv_ptr != NULL);
           ctl_ptr = drv_ptr->add_controller(key_generic_, val_generic_);
           if (ctl_ptr) {
             resp_code_ = DRVAPI_RESPONSE_SUCCESS;
           } else {
             resp_code_ = DRVAPI_RESPONSE_FAILURE;
             return resp_code_;
           }
           ctrl_int->AddController(ctrl_name, ctl_ptr, drv_ptr);
           break;
       }

       case UNC_OP_UPDATE: {
           pfc_log_debug("%s: Updated Controller ", PFC_FUNCNAME);
           VtnDrvRetEnum ctl_retval = VTN_DRV_RET_FAILURE;
           ctl_retval = ctrl_int->GetControllerInstance(ctrl_name,
                                                  &ctl_ptr, &drv_ptr);
           if (ctl_retval) {
             pfc_log_error("%s:GetDriverByControllerName failed with rtcode,%u",
                           PFC_FUNCNAME, resp_code_);
               return DRVAPI_RESPONSE_FAILURE;
           }

           PFC_ASSERT(drv_ptr != NULL);
           PFC_ASSERT(ctl_ptr != NULL);

           unc::driver::controller* update_ctl_ptr =
               drv_ptr->update_controller(key_generic_,
                                          val_generic_,
                                          ctl_ptr);

           VtnDrvRetEnum update_status =
               ctrl_int->UpdateControllerConfiguration(ctrl_name,
                                                       update_ctl_ptr,
                                                       drv_ptr);

           if (update_status == VTN_DRV_RET_SUCCESS)
             resp_code_ = DRVAPI_RESPONSE_SUCCESS;
           else
             resp_code_ = DRVAPI_RESPONSE_FAILURE;

           break;
       }

       case UNC_OP_DELETE: {
           pfc_log_debug("%s: deleted Controller ", PFC_FUNCNAME);
           VtnDrvRetEnum ctl_retval = VTN_DRV_RET_FAILURE;
           ctl_retval = ctrl_int->GetControllerInstance(ctrl_name,
                        &ctl_ptr, &drv_ptr);
           if (ctl_retval) {
             pfc_log_error("%s:GetDriverByControllerName failed with rtcode,%u",
                           PFC_FUNCNAME, resp_code_);
               return DRVAPI_RESPONSE_FAILURE;
           }

           PFC_ASSERT(drv_ptr != NULL);
           PFC_ASSERT(ctl_ptr != NULL);

           VtnDrvRetEnum delete_status =
               ctrl_int->RemoveControllerConfiguration(ctrl_name, ctl_ptr,
                                                       drv_ptr);

           if (delete_status == VTN_DRV_RET_SUCCESS)
              resp_code_ = DRVAPI_RESPONSE_SUCCESS;
           else
              resp_code_ = DRVAPI_RESPONSE_FAILURE;
           break;
       }
       default: {
           pfc_log_debug("%s: Operation not Support ", PFC_FUNCNAME);
           resp_code_ = DRVAPI_RESPONSE_FAILURE;
           break;
       }
    }
    pfc_log_debug("%s: Exiting Function.", PFC_FUNCNAME);
    return resp_code_;
}

/**
* @brief - Handling Request for the platform
**/

template<typename key, typename val, class command_ptr>
drv_resp_code_t
KtRequestHandler<key, val, command_ptr>::handle_request(
               pfc::core::ipc::ServerSession &sess,
               unc::driver::keyif_drv_request_header_t &request_header,
               unc::driver::ControllerFramework* ctrl_int) {
     pfc_log_debug("%s: Entering Function.", PFC_FUNCNAME);

     drv_resp_code_t ret_val = DRVAPI_RESPONSE_FAILURE;
     drv_resp_code_t resp_code_ = DRVAPI_RESPONSE_FAILURE;
     key key_generic_;
     val val_generic_;
     memset(&key_generic_, 0, sizeof(key));
     memset(&val_generic_, 0, sizeof(val));

     if (ctrl_int == NULL) {
       pfc_log_debug("ctrl_int is null\n");
       return DRVAPI_RESPONSE_FAILURE;
     }


     ret_val = parse_request(sess, request_header, key_generic_, val_generic_);

     if (!ret_val) {
       resp_code_ = execute(sess, request_header, ctrl_int,
                    resp_code_, key_generic_, val_generic_);
     } else {
       pfc_log_error("%s:  Failed to parse key and val struct(err = %u)",
       PFC_FUNCNAME, resp_code_);
     }

     ret_val = handle_response(sess, request_header, ctrl_int,
                      key_generic_, val_generic_, resp_code_);

     if (ret_val) {
        pfc_log_error("%s:. Failed to send response(err = %u)",
        PFC_FUNCNAME, resp_code_);
     }

     return resp_code_;
}



/**
* @brief - Converting ConfigNode pointer to command 
**/

template<typename key, typename val, class command_ptr>
drv_resp_code_t
KtRequestHandler<key, val, command_ptr>::execute_cmd(
               unc::vtndrvcache::ConfigNode *cfgptr,
               unc::driver::controller* conn,
               unc::driver::driver* drv_ptr) {
  unc::vtndrvcache::CacheElementUtil<key, val, uint32_t> * ptr =
  dynamic_cast <unc::vtndrvcache::CacheElementUtil
  <key, val, uint32_t> * > (cfgptr);

  drv_resp_code_t resp_code_ = DRVAPI_RESPONSE_FAILURE;

  if (ptr == NULL) {
     pfc_log_debug("CacheElementUtil ptr is NULL");
     return DRVAPI_RESPONSE_FAILURE;
  }
  unc_key_type_t keytype = cfgptr->get_type();
  unc::driver::driver_command * drv_command_ptr_ =
      drv_ptr->get_driver_command(keytype);

  PFC_ASSERT(drv_command_ptr_ != NULL);
  command_ptr * config_cmd_ptr = NULL;
  config_cmd_ptr = static_cast<command_ptr *> (drv_command_ptr_);
  uint32_t operat = cfgptr->get_operation();

  resp_code_ = config_cmd_ptr->validate_op(
                                   *(ptr->getkey()),
                                   *(ptr->getval()),
                                   conn,
                                    operat);
  if (resp_code_ != DRVAPI_RESPONSE_SUCCESS) {
       pfc_log_debug("%s: Validate op failed with resp_code ,%u",
       PFC_FUNCNAME, resp_code_);
       return resp_code_;
  }

  switch (operat) {
    case UNC_OP_CREATE:
       pfc_log_debug("%s: Translate Create Command string", PFC_FUNCNAME);
       resp_code_ = config_cmd_ptr->create_cmd(*(ptr->getkey()),
                   *(ptr->getval()), conn);

       break;

    case UNC_OP_DELETE:

      pfc_log_debug("%s: Translate Delete Command string", PFC_FUNCNAME);

      resp_code_ = config_cmd_ptr->delete_cmd(*(ptr->getkey()),
                   *(ptr->getval()), conn);

      break;

    case UNC_OP_UPDATE:

      pfc_log_debug("%s: Translate Update Command string", PFC_FUNCNAME);

      resp_code_ = config_cmd_ptr->update_cmd(*(ptr->getkey()),
                   *(ptr->getval()), conn);

      break;

    default:

      pfc_log_debug("%s: Invalid operation  ", PFC_FUNCNAME);

      resp_code_ = DRVAPI_RESPONSE_FAILURE;

      break;
  }

  return resp_code_;
}

/**
* @brief - Sending Response to the platform
**/

template<typename key, typename val, class command_ptr>
drv_resp_code_t
KtRequestHandler<key, val, command_ptr>::handle_response(
               pfc::core::ipc::ServerSession &sess,
               unc::driver::keyif_drv_request_header_t &request_header,
               unc::driver::ControllerFramework* ctrl_int,
               key &key_generic_,
               val &val_generic_,
               drv_resp_code_t &resp_code_) {
  pfc_log_debug("Entring Function %s..", PFC_FUNCNAME);

  unc::driver::keyif_drv_response_header_t resp_hdr;
  uint32_t err_= 0;
  resp_hdr.result = resp_code_;
  create_response_header(request_header, resp_hdr, resp_code_);
  resp_code_ = populate_response_header(sess, resp_hdr);
  if (resp_code_) {
    pfc_log_error("%s: populate_response_header failed with ret_code ,%u",
                       PFC_FUNCNAME, resp_code_);
    return resp_code_;
  }

  err_ = sess.addOutput(resp_hdr.result);
  if (err_ != 0) {
    pfc_log_error("%s: Failed to send resp code:(err = %d)",
                  PFC_FUNCNAME, resp_code_);
    return DRVAPI_RESPONSE_FAILURE;
  }

  err_ = sess.addOutput((uint32_t) resp_hdr.key_type);
  if (err_ != 0) {
    pfc_log_error("%s: Failed to send driver data key type: (err = %d)",
                  PFC_FUNCNAME, resp_code_);
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (resp_hdr.header.data_type != UNC_DT_RUNNING) {
    uint32_t ret_code = sess.addOutput(key_generic_);
    if (ret_code) {
      pfc_log_error("%s: addOutput failed with ret_code ,%u",
                    PFC_FUNCNAME, ret_code);
      return DRVAPI_RESPONSE_FAILURE;
    }
  }
  pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}


template<>
drv_resp_code_t
KtRequestHandler<key_root_t, val_root_t, root_driver_command>::handle_response(
                pfc::core::ipc::ServerSession &sess,
                unc::driver::keyif_drv_request_header_t &request_header,
                unc::driver::ControllerFramework* ctrl_int,
                key_root_t &key_generic_,
                val_root_t &val_generic_,
                drv_resp_code_t &resp_code_) {
        pfc_log_debug("Entring Function %s..", PFC_FUNCNAME);
        driver* drv = NULL;
        controller* ctr = NULL;
        memset(&key_generic_, 0, sizeof(key_root_t));

        VtnDrvRetEnum ctl_retval = VTN_DRV_RET_FAILURE;
        unc::driver::keyif_drv_response_header_t resp_hdr;
        resp_hdr.result = resp_code_;
        pfc_log_debug("handle_response, resp_code_: %u", resp_code_);
        pfc_log_debug("resp_hdr.result, resp_hdr.result: %u", resp_hdr.result);
        create_response_header(request_header, resp_hdr, resp_code_);
        resp_code_ = populate_response_header(sess, resp_hdr);
        if (resp_code_) {
                pfc_log_error("%s: populate_response_header failed"
                      "with ret_code root ,%u", PFC_FUNCNAME, resp_code_);
                return resp_code_;
        }
        std::string ctr_name = resp_hdr.controller_name;
        if (resp_hdr.header.data_type == UNC_DT_RUNNING) {
           pfc_log_debug("UNC_DT_RUNNING processing");
           ctl_retval = ctrl_int->GetDriverByControllerName(ctr_name,
                                                            &ctr,
                                                            &drv);

           if (ctl_retval == VTN_DRV_RET_SUCCESS) {
               unc::vtndrvcache::CommonIterator* itr_ptr =
                          ctr->keytree_ptr->get_iterator();
               if (itr_ptr == NULL) {
                pfc_log_error("%s: exit. itr_ptr is null", PFC_FUNCNAME);
                return DRVAPI_RESPONSE_FAILURE;
               }
               uint32_t size = ctr->keytree_ptr->cfg_list_cnt();
               pfc_log_debug("config node size is %d for controller %s",
                             size, ctr_name.c_str());
               unc::vtndrvcache::ConfigNode *cfgnode = NULL;
            //  Not Success and Error Case check for Reps
               if ((size == 0) &&
                (resp_hdr.result == DRVAPI_RESPONSE_NO_SUCH_INSTANCE)) {
                  pfc_log_debug("%s: Value list empty and Response"
                           "header success", PFC_FUNCNAME);
                  pfc_log_debug("%s: Adding Root key only", PFC_FUNCNAME);
                  sess.addOutput((uint32_t) DRVAPI_RESPONSE_NO_SUCH_INSTANCE);
                  sess.addOutput((uint32_t) UNC_KT_ROOT);
                  sess.addOutput(key_generic_);
                  pfc_log_trace("ResponseHandler::%s Exiting", PFC_FUNCNAME);
                  return DRVAPI_RESPONSE_SUCCESS;
               } else if (resp_hdr.result != DRVAPI_RESPONSE_SUCCESS) {
                  pfc_log_debug("%s: Response header result Error",
                                PFC_FUNCNAME);
                  pfc_log_debug("%s: Adding Root Key only", PFC_FUNCNAME);
                  sess.addOutput((uint32_t) DRVAPI_RESPONSE_FAILURE);
                  sess.addOutput((uint32_t) UNC_KT_ROOT);
                  sess.addOutput(key_generic_);
                  pfc_log_trace("ResponseHandler::%s Exiting", PFC_FUNCNAME);
                  return DRVAPI_RESPONSE_FAILURE;
               }

               pfc_log_trace("Audit cfgnode need to add");
               int err_ = sess.addOutput(resp_hdr.result);
               if (err_ != 0) {
                 pfc_log_error("%s: Failed to send resp code audit:(err = %d)",
                      PFC_FUNCNAME, resp_code_);
                 return DRVAPI_RESPONSE_FAILURE;
               }

               bool first = true;

               for (cfgnode = itr_ptr->AuditFirstItem();
                          itr_ptr->IsDone() == false;
                          cfgnode = itr_ptr->NextItem() ) {
                  if (cfgnode == NULL) {
                      pfc_log_debug("cfgnode is NULL before get_type");
                  }
                  if (first) {
                      first = false;
                  } else {
                    sess.addOutput();
                    pfc_log_debug("Add empty success");
                  }
                  unc_key_type_t keytype = cfgnode->get_type();
                  pfc_log_debug("%u,keytype", keytype);

               uint32_t ret_code = sess.addOutput((uint32_t)keytype);
               if (ret_code) {
                  pfc_log_error("%s: addOutput failed with keytype ,%u",
                       PFC_FUNCNAME, ret_code);
                  if (itr_ptr != NULL)
                     delete itr_ptr;
                  return DRVAPI_RESPONSE_FAILURE;
               }
               pfc_log_debug("Add key type success");

               void* key = get_key_struct(cfgnode);
               void* val = get_val_struct(cfgnode);
               initialize_map();
               pfc_ipcstdef_t* key_sdf =key_map_.find(keytype)->second;
               pfc_ipcstdef_t* val_sdf =val_map_.find(keytype)->second;
               ret_code = sess.addOutput(*key_sdf, key);
               if (ret_code) {
                  pfc_log_error("%s: addOutput failed with ret_code ,%u",
                     PFC_FUNCNAME, ret_code);
                  if (itr_ptr != NULL)
                     delete itr_ptr;
                  return DRVAPI_RESPONSE_FAILURE;
               }

               pfc_log_debug("Add key success");
               ret_code = sess.addOutput(*val_sdf, val);
               if (ret_code) {
                 pfc_log_error("%s: addOutput failed with ret_code ,%u",
                    PFC_FUNCNAME, ret_code);
                 return DRVAPI_RESPONSE_FAILURE;
               }
               pfc_log_debug("Add val success");
      }  // for loop
        if (itr_ptr != NULL)
          delete itr_ptr;
      } else {
         pfc_log_error("%s: unable to get controller/driver pointer",
                 PFC_FUNCNAME);
         return DRVAPI_RESPONSE_FAILURE;
      }
          pfc_log_debug("UNC_DT_RUNNING processing complete");
     }
        pfc_log_debug("Exiting Function %s..", PFC_FUNCNAME);
        return DRVAPI_RESPONSE_SUCCESS;
}


/**
* @brief - Creating Response Header
**/

template<typename key, typename val, class command_ptr>
void
KtRequestHandler<key, val, command_ptr>::create_response_header(
                        unc::driver::keyif_drv_request_header_t &reqhdr,
                        unc::driver::keyif_drv_response_header_t &resphdr,
                        drv_resp_code_t &resp_code_) {
  memset(&resphdr, 0, sizeof(resphdr));

  /* Copy parameters from the request header to the response header. */
  memcpy(reinterpret_cast<void *>(&resphdr.header),
        reinterpret_cast<void *>(&reqhdr.header),
         sizeof(resphdr.header));

  /* Set the result code. */
  pfc_log_trace("%s: Result code: %u", PFC_FUNCNAME, resp_code_);

  resphdr.result = resp_code_;

  memcpy(resphdr.controller_name, reqhdr.controller_name,
         sizeof(resphdr.controller_name));
  pfc_log_trace("REQ  controller_name : %s", reqhdr.controller_name);

  memcpy(resphdr.header.domain_id, reqhdr.header.domain_id,
         sizeof(resphdr.header.domain_id));
  pfc_log_trace("RESP controller_name : %s", resphdr.controller_name);

  resphdr.key_type = reqhdr.key_type;
}


/**
* @brief - populating response header  
**/

template<typename key, typename val, class command_ptr>
drv_resp_code_t
KtRequestHandler<key, val, command_ptr>::populate_response_header(
    pfc::core::ipc::ServerSession &sess,
  unc::driver::keyif_drv_response_header_t &resp_hdr) {
  pfc_log_trace("%s: Entering function", PFC_FUNCNAME);

  uint32_t err = DRVAPI_RESPONSE_FAILURE;

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
      "result = %u \n", PFC_FUNCNAME, ctr_name, resp_hdr.header.session_id,
      resp_hdr.header.config_id, resp_hdr.header.operation,
      resp_hdr.header.option1, resp_hdr.header.option2,
      resp_hdr.header.data_type, resp_hdr.result);

  /* Sends session_id */
  err = sess.addOutput(resp_hdr.header.session_id);
  if (err) {
    pfc_log_error("%s: Failed to send client session id: (err = %d)",
                  PFC_FUNCNAME, err);
    return DRVAPI_RESPONSE_FAILURE;
  }

  /* Sends config_id */
  err = sess.addOutput(resp_hdr.header.config_id);
  if (err) {
    pfc_log_error("%s: Failed to send configurationid: (err = %d)",
                  PFC_FUNCNAME, err);
    return DRVAPI_RESPONSE_FAILURE;
  }

  /* Sends controller_name */
  const char* controller_name =
      reinterpret_cast<const char *> (resp_hdr.controller_name);
  err = sess.addOutput(controller_name);
  if (err) {
    pfc_log_error("%s: Failed to send controller id: (err = %d)", PFC_FUNCNAME,
                  err);
    return DRVAPI_RESPONSE_FAILURE;
  }

  /* Sends domain-id*/
  const char* domain_id =
      reinterpret_cast<const char *> (resp_hdr.header.domain_id);
  err = sess.addOutput(domain_id);
  if (err) {
    pfc_log_error("%s: Failed to send domain id:(err = %d)", PFC_FUNCNAME, err);
    return DRVAPI_RESPONSE_FAILURE;
  }

  /* Sends operation */
  err = sess.addOutput((uint32_t) resp_hdr.header.operation);
  if (err) {
    pfc_log_error("%s: Failed to send driver data operation (err = %d)",
                  PFC_FUNCNAME, err);
    return DRVAPI_RESPONSE_FAILURE;
  }

  /* Sends max-rep-count */
  err = sess.addOutput(resp_hdr.header.max_rep_count);
  if (err) {
    pfc_log_error("%s: Failed to send driver data max_rep_count: (err = %d)",
                  PFC_FUNCNAME, err);
    return DRVAPI_RESPONSE_FAILURE;
  }

  /* Sends option1 */
  err = sess.addOutput(resp_hdr.header.option1);
  if (err) {
    pfc_log_error("%s: Failed to send driver data option1 : (err = %d)",
                  PFC_FUNCNAME, err);
    return DRVAPI_RESPONSE_FAILURE;
  }

  /* Sends option2 */
  err = sess.addOutput(resp_hdr.header.option2);
  if (err) {
    pfc_log_error("%s: Failed to send driver data option2 : (err = %d)",
                  PFC_FUNCNAME, err);
    return DRVAPI_RESPONSE_FAILURE;
  }

  /* Sends data type */
  err = sess.addOutput((uint32_t) resp_hdr.header.data_type);
  if (err) {
    pfc_log_error("%s: Failed to send driver data data_type: (err = %d)",
                  PFC_FUNCNAME, err);
    return DRVAPI_RESPONSE_FAILURE;
  }

  pfc_log_trace("%s: Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

template<typename key, typename val, typename command_ptr>
void
KtRequestHandler<key, val, command_ptr>::initialize_map() {
  uint32_t loop = 0;
  unc_key_type_t KT[3] = {UNC_KT_VTN, UNC_KT_VBRIDGE, UNC_KT_VBR_IF};
  for (loop = 0; loop < 3; loop++) {
    switch (KT[loop]) {
      case UNC_KT_VTN:
        {
        pfc_ipcstdef_t *stdef_k = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_k, key_vtn);
        pfc_ipcstdef_t *stdef_v = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_v, val_vtn);
        key_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                  stdef_k));
        val_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                  stdef_v));
        break;
        }
      case UNC_KT_VBRIDGE:
        {
        pfc_ipcstdef_t *stdef_kvbr = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_kvbr, key_vbr);
        pfc_ipcstdef_t *stdef_vbr = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_vbr, val_vbr);
        key_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                  stdef_kvbr));
        val_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                  stdef_vbr));
        break;
        }
      case UNC_KT_VBR_IF:
        {
        pfc_ipcstdef_t *stdef_kvbrif = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_kvbrif, key_vbr_if);
        pfc_ipcstdef_t *stdef_vbrif = new pfc_ipcstdef_t;
        PFC_IPC_STDEF_INIT(stdef_vbrif, pfcdrv_val_vbr_if);
        key_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                stdef_kvbrif));
        val_map_.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                stdef_vbrif));
        break;
        }
      default:
        break;
    }
  }
}
}  // namespace driver
}  // namespace unc
#endif
