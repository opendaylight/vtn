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
#include <vtn_drv_module.hh>
#include <controller_fw.hh>
#include <driver/driver_interface.hh>
#include <vtndrvintf_defs.h>
#include <map>
#include <string>


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
            unc::driver::keyif_drv_request_header_t &request_header);

       /**
       * @brief - Create,Delete,Update Of KT
       * @param[in] ServerSession, keyif_drv_request_header_t, ControllerFramework
       * @retval - drv_resp_code_t
       **/

        drv_resp_code_t
        execute(pfc::core::ipc::ServerSession &sess,
                unc::driver::keyif_drv_request_header_t &request_header,
                unc::driver::ControllerFramework* ctrl_int);


       /**
       * @brief - Handle Response
       * @param[in] ServerSession, keyif_drv_request_header_t
       * @retval - drv_resp_code_t
       **/

        drv_resp_code_t
        handle_response(pfc::core::ipc::ServerSession &sess,
            unc::driver::keyif_drv_request_header_t &request_header);


       /**
       * @brief - Response Header Creation
       * @param[in] keyif_drv_response_header_t, keyif_drv_request_header_t
       * @retval - void 
       */

        void
        create_response_header(unc::driver::keyif_drv_request_header_t &reqhdr,
            unc::driver::keyif_drv_response_header_t &resphdr);


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

      private:
        key key_generic_;
        val val_generic_;
        unc_keytype_operation_t operation_;
        command_ptr *command_ptr_;
        unc_key_type_t  keytype_;
        drv_resp_code_t resp_code_;
};


/**
* @brief: Default Constructor
**/

template<typename key, typename val, typename command_ptr>
KtRequestHandler<key, val, command_ptr>::KtRequestHandler():command_ptr_(NULL), resp_code_(DRVAPI_RESPONSE_FAILURE) { } 

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
  dynamic_cast <unc::vtndrvcache::CacheElementUtil
  <key, val, uint32_t> * >(cfgptr);

  if(cache_util_ptr == NULL) {
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
  dynamic_cast <unc::vtndrvcache::CacheElementUtil
  <key, val, uint32_t> * >(cfgptr);

  if(cache_util_ptr == NULL) {
    pfc_log_error("%s: cache_util_ptr is null", PFC_FUNCNAME);
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
                   unc::driver::keyif_drv_request_header_t &request_header) {
  if (sess.getArgument(INPUT_KEY_STRUCT_INDEX, key_generic_)) {
     pfc_log_info("%s: Exting Function.getArg Failed to read key struct."
     " rc=%u", PFC_FUNCNAME, 2);
      return DRVAPI_RESPONSE_CTRLAPI_FAILURE;
  }
  if (sess.getArgument(INPUT_VAL_STRUCT_INDEX, val_generic_)) {
       pfc_log_info("%s: No value struct present.", PFC_FUNCNAME);
  }

  operation_ = (unc_keytype_operation_t) request_header.header.operation;
  pfc_log_info("%u: operation_.", operation_);
  keytype_ =  request_header.key_type;
  return DRVAPI_RESPONSE_SUCCESS;
}


/**
* @brief - Create , Delete and Update Command execution  
**/

template<typename key, typename val, class command_ptr>
drv_resp_code_t
KtRequestHandler<key, val, command_ptr>::execute(
               pfc::core::ipc::ServerSession &sess,
               unc::driver::keyif_drv_request_header_t &request_header,
               unc::driver::ControllerFramework* ctrl_int) {
  pfc_log_info("%s: Entering Function.", PFC_FUNCNAME);

  std::string ctrl_name = std::string(request_header.controller_name);

  unc::driver::driver*  drv_ptr = NULL;
  unc::driver::controller* ctrl_ptr = NULL;

  resp_code_ = (drv_resp_code_t) ctrl_int->GetDriverByControllerName(ctrl_name, &ctrl_ptr, &drv_ptr);
   pfc_log_info("%u:resp_code_. GetDriverByControllerName", resp_code_);

  if (resp_code_) {
      pfc_log_error("%s:GetDriverByControllerName failed with re_code,%u",
                 PFC_FUNCNAME, resp_code_);
      return resp_code_;
  }


  PFC_ASSERT(drv_ptr != NULL);

  PFC_ASSERT(ctrl_ptr !=NULL);

  unc::driver::driver_command * drv_command_ptr_ =
                          drv_ptr->get_driver_command(keytype_);

  PFC_ASSERT(drv_command_ptr_ != NULL);

  command_ptr_ = static_cast<command_ptr *> (drv_command_ptr_);

  PFC_ASSERT(command_ptr_ != NULL);


  if (!drv_ptr->is_2ph_commit_support_needed()) {
      if (ctrl_ptr->keytree_ptr == NULL) {
              pfc_log_info("Cache for controller created");
              ctrl_ptr->keytree_ptr = unc::vtndrvcache::KeyTree::create_cache();
      }
      unc::vtndrvcache::ConfigNode *cfgptr =
                new unc::vtndrvcache::CacheElementUtil<key, val, uint32_t> (
                                                                  &key_generic_,
                                                                  &val_generic_,
                                                                    operation_);
      resp_code_ = (drv_resp_code_t) ctrl_ptr->keytree_ptr->append_commit_node(cfgptr);
      pfc_log_info("%u:resp_code_ AppendCommitNode", resp_code_);

      if (resp_code_) {
              pfc_log_info("%s:Exiting Function. AppendCommitNode fail",
                           PFC_FUNCNAME);
              if(cfgptr)
                delete cfgptr;
              return resp_code_;
      } 
  } else {
    switch (operation_) {
       case UNC_OP_CREATE:
       pfc_log_info("%s: Translate Create Command string", PFC_FUNCNAME);
       resp_code_ = command_ptr_->create_cmd(key_generic_, val_generic_, ctrl_ptr);

       break;

    case UNC_OP_DELETE:
      pfc_log_info("%s: Translate Delete Command string", PFC_FUNCNAME);

      resp_code_ = command_ptr_->delete_cmd(key_generic_, val_generic_, ctrl_ptr);

      break;

    case UNC_OP_UPDATE:
      pfc_log_info("%s: Translate Update Command string", PFC_FUNCNAME);

      resp_code_ = command_ptr_->update_cmd(key_generic_, val_generic_, ctrl_ptr);

      break;

    default:

      pfc_log_info("%s: Invalid operation  ", PFC_FUNCNAME);

      resp_code_ = DRVAPI_RESPONSE_FAILURE;

      break;
    }
  }
  pfc_log_info("%s: Exiting Function.", PFC_FUNCNAME);
  return resp_code_;
}


/**
* @brief:Template Specialization for parsing KT_Controller structures(Key,Val)
**/

template<>
drv_resp_code_t
KtRequestHandler<key_ctr_t, val_ctr_t, controller_command>::parse_request(
                      pfc::core::ipc::ServerSession &sess,
                      unc::driver::keyif_drv_request_header_t &request_header) {
  pfc_log_info("%s: Entering Function.", PFC_FUNCNAME);
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

  return DRVAPI_RESPONSE_SUCCESS;
  pfc_log_info("%s: Exiting Function.", PFC_FUNCNAME);
}


/**
* @brief:Template Specialization for KT Controller command Execution  
**/

template<>
drv_resp_code_t
KtRequestHandler<key_ctr_t, val_ctr_t, controller_command>::execute(
               pfc::core::ipc::ServerSession &sess,
               unc::driver::keyif_drv_request_header_t &request_header,
               unc::driver::ControllerFramework* ctrl_int) {
     pfc_log_info("%s: Entering Function.", PFC_FUNCNAME);
     unc::driver::driver *drv_ptr = NULL;
     unc::driver::controller* ctl_ptr = NULL;
     std::string ctrl_name = std::string(request_header.controller_name);

     drv_ptr = ctrl_int->GetDriverInstance((unc_keytype_ctrtype_t)
                                            val_generic_.type);
     PFC_ASSERT(drv_ptr != NULL);
     

     switch (request_header.header.operation) {
       case UNC_OP_CREATE: {
           pfc_log_info("%s: Creates new controller ", PFC_FUNCNAME);
           ctl_ptr = drv_ptr->add_controller(key_generic_, val_generic_);
           ctrl_int->AddController(ctrl_name, ctl_ptr, drv_ptr);
           if(ctl_ptr)
             resp_code_ = DRVAPI_RESPONSE_SUCCESS;
           else
             resp_code_ = DRVAPI_RESPONSE_FAILURE;
           break;
       }

       case UNC_OP_UPDATE: {
           pfc_log_info("%s: Updated Controller ", PFC_FUNCNAME);
           ctl_ptr = drv_ptr->update_controller(key_generic_,
                                                val_generic_,
                                                ctl_ptr);
           ctrl_int->UpdateControllerConfiguration(ctrl_name, ctl_ptr, drv_ptr);
           if(ctl_ptr)
             resp_code_ = DRVAPI_RESPONSE_SUCCESS;
           else
             resp_code_ = DRVAPI_RESPONSE_FAILURE;

           break;
       }

       case UNC_OP_DELETE: {
           pfc_log_info("%s: deleted Controller ", PFC_FUNCNAME);
           if(ctl_ptr) {
             drv_ptr->delete_controller(ctl_ptr);
             ctrl_int->RemoveControllerConfiguration(ctrl_name);
             if(ctl_ptr == NULL)
                resp_code_ = DRVAPI_RESPONSE_SUCCESS;
             else
                resp_code_ = DRVAPI_RESPONSE_FAILURE;
           }
           break;
       }
       default: {
           pfc_log_info("%s: Operation not Support ", PFC_FUNCNAME);
           resp_code_ = DRVAPI_RESPONSE_FAILURE;
           break;
       }
    }
    pfc_log_info("%s: Exiting Function.", PFC_FUNCNAME);
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
     pfc_log_info("%s: Entering Function.", PFC_FUNCNAME);


     memset(&key_generic_, 0, sizeof(key));
     memset(&val_generic_, 0, sizeof(val));

     if (ctrl_int == NULL) {
       pfc_log_info("ctrl_int is null\n");
       return DRVAPI_RESPONSE_FAILURE;
     }


     resp_code_ = parse_request(sess, request_header);

     if (!resp_code_) {
       resp_code_ = execute(sess, request_header, ctrl_int);
     } else {
       pfc_log_error("%s:  Failed to parse key and val struct(err = %u)",
       PFC_FUNCNAME, resp_code_);
     }

     resp_code_ = handle_response(sess, request_header);

     if (resp_code_) {
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

  if(ptr == NULL) {
     pfc_log_debug("CacheElementUtil ptr is NULL");
     return DRVAPI_RESPONSE_FAILURE;
  }
  unc_key_type_t keytype = cfgptr->get_type();
  unc::driver::driver_command * drv_command_ptr_ =
      drv_ptr->get_driver_command(keytype);

  PFC_ASSERT(drv_command_ptr_ != NULL);
  command_ptr * config_cmd_ptr = NULL;
  config_cmd_ptr = static_cast<command_ptr *> (drv_command_ptr_);
  //unc_keytype_operation_t operat = cfgptr->get_operation();
  uint32_t operat = cfgptr->get_operation();

  resp_code_ = config_cmd_ptr->validate_op(
                                   *(ptr->getkey()),
                                   *(ptr->getval()),
                                   conn,
                                    operat);
  if (resp_code_ != DRVAPI_RESPONSE_SUCCESS) {
       pfc_log_debug("%s: Validate op failed with resp_code ,%u",
       PFC_FUNCNAME,resp_code_);
       return resp_code_;
  }

  switch (operat) {
    case UNC_OP_CREATE:
       pfc_log_debug("%s: Translate Create Command string", PFC_FUNCNAME);
       resp_code_ = config_cmd_ptr->create_cmd(*(ptr->getkey()), *(ptr->getval()), conn);

       break;

    case UNC_OP_DELETE:

      pfc_log_debug("%s: Translate Delete Command string", PFC_FUNCNAME);

      resp_code_ = config_cmd_ptr->delete_cmd(*(ptr->getkey()), *(ptr->getval()), conn);

      break;

    case UNC_OP_UPDATE:

      pfc_log_debug("%s: Translate Update Command string", PFC_FUNCNAME);

      resp_code_ = config_cmd_ptr->update_cmd(*(ptr->getkey()), *(ptr->getval()), conn);

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
               unc::driver::keyif_drv_request_header_t &request_header) {
  pfc_log_debug("Entring Function %s..", PFC_FUNCNAME);

  unc::driver::keyif_drv_response_header_t resp_hdr;
  resp_hdr.result = resp_code_;
  create_response_header(request_header, resp_hdr);
  resp_code_ = populate_response_header(sess, resp_hdr);
  if (resp_code_) {
      pfc_log_error("%s: populate_response_header failed with ret_code ,%u",
      PFC_FUNCNAME,resp_code_);
      return resp_code_;
  }
  if (resp_hdr.header.data_type != UNC_DT_RUNNING) {
    uint32_t ret_code = sess.addOutput(key_generic_);
    if (ret_code) {
      pfc_log_error("%s: addOutput failed with ret_code ,%u", PFC_FUNCNAME,ret_code);
      return DRVAPI_RESPONSE_FAILURE;
    }
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
                        unc::driver::keyif_drv_response_header_t &resphdr) {
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

  err = sess.addOutput(resp_hdr.result);

  if (err) {
    pfc_log_error("%s: Failed to send resp code:(err = %d)", PFC_FUNCNAME, err);
    return DRVAPI_RESPONSE_FAILURE;
  }

  /* Sends key_type */
  err = sess.addOutput((uint32_t) resp_hdr.key_type);
  if (err) {
    pfc_log_error("%s: Failed to send driver data key type: (err = %d)",
                  PFC_FUNCNAME, err);
    return DRVAPI_RESPONSE_FAILURE;
  }

  pfc_log_trace("%s: Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}
}  // driver
}  // unc
#endif
