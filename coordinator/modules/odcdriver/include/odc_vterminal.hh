/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_VTERMINAL_HH_
#define _ODC_VTERMINAL_HH_

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <odc_flowfilter_template.hh>
#include <odc_rest.hh>
#include <odc_vtn.hh>
#include <odl_vterm.hh>
#include <unc/upll_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <tclib_module.hh>
#include <vector>
#include <string>
#include <sstream>
namespace unc {
namespace odcdriver {

class OdcVterminalCommand : public unc::driver::vtn_driver_command
      <key_vterm_t, val_vterm_t>, public VtermParser {
private:
unc::restjson::ConfFileValues_t conf_values_;
 std::string vtn_name_;

public:
// Constructor
OdcVterminalCommand(unc::restjson::ConfFileValues_t
                                         conf_values)
: conf_values_(conf_values) {
    }

// Destructor
//OdcVterminalCommand::~OdcVterminalCommand() {
//}

// Constructs command to create Vtn and send it to rest interface to send to
// controller
UncRespCode 
    create_cmd(key_vterm_t& key_vterm, val_vterm_t& val_vterm,
                                            unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
 std::string url = "";
 url.append(get_base_url());
 url.append(get_url_tail(key_vterm, val_vterm));

 json_object *jobj_req_body = unc::restjson::JsonBuildParse::create_json_obj();
 pfc_log_debug("calling function create_vterm_request");
 int retval = create_vterm_request(jobj_req_body, key_vterm, val_vterm);
 pfc_log_debug("returned fron create");
 if (retval != UNC_RC_SUCCESS)
    return UNC_DRV_RC_ERR_GENERIC;

 pfc_log_debug("Request body:%s", unc::restjson::JsonBuildParse::get_json_string(jobj_req_body));
        //unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(), ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
       // unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(url, HTTP_METHOD_POST, NULL, conf_values_);

 int resp_code = send_httprequest(ctr_ptr, url, conf_values_,HTTP_METHOD_POST, jobj_req_body);
 pfc_log_debug("response code returned in create vterm is %d", resp_code);
 if (HTTP_201_RESP_CREATED != resp_code) {
   pfc_log_debug("check if vtn is stand-alone");

    //  check if vtn is stand-alone
    UncRespCode ret_code = UNC_DRV_RC_ERR_GENERIC;
    std::vector<unc::vtndrvcache::ConfigNode*> child_list;
    child_list.clear();
    std::string vtn_name = reinterpret_cast<const char*>
        (key_vterm.vtn_key.vtn_name);
    pfc_log_debug("VTN name str:%s", vtn_name.c_str());
    void *parent_key = &key_vterm;
ret_code = fetch_config(ctr_ptr, parent_key, child_list);
    int vtn_child_size = static_cast<int> (child_list.size());
    pfc_log_debug("VTN child_list... size: %d", vtn_child_size);
    if (ret_code == UNC_RC_SUCCESS) {
      if (vtn_child_size == 0) {
        pfc_log_debug("delete stand-alone vtn");
        //  delete stand-alone vtn
        key_vtn_t key_vtn;
        val_vtn_t val_vtn;
        memset(&key_vtn, 0, sizeof(key_vtn_t));
        memset(&val_vtn, 0, sizeof(val_vtn_t));
        memcpy(key_vtn.vtn_name, key_vterm.vtn_key.vtn_name,
               sizeof(key_vterm.vtn_key.vtn_name));
        pfc_log_debug("VTN name:%s", key_vtn.vtn_name);
        OdcVtnCommand vtn_obj(conf_values_);
        vtn_obj.delete_cmd(key_vtn, val_vtn, ctr_ptr);
      }
    }
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}

// Update Vterminal
UncRespCode 
update_cmd(key_vterm_t& key, val_vterm_t& val,
                                            unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string url = "";
  url.append(get_base_url());
  url.append(get_url_tail(key, val));

  std::string vterminal_name = reinterpret_cast<char *>(key.vterminal_name);
   if (0 == strlen(vterminal_name.c_str()))
      return UNC_DRV_RC_ERR_GENERIC;

  vtn_name_ = reinterpret_cast<char *>(key.vtn_key.vtn_name);
  if (0 == strlen(vtn_name_.c_str()))
     return UNC_DRV_RC_ERR_GENERIC;

  json_object *jobj_req_body = unc::restjson::JsonBuildParse::create_json_obj();
  pfc_log_debug("calling function create_vterm_request");
  int retval = create_vterm_request(jobj_req_body, key, val);
  pfc_log_debug("update create return");
  if (retval != UNC_RC_SUCCESS)
     return UNC_DRV_RC_ERR_GENERIC;

   pfc_log_debug("Request body:%s", unc::restjson::JsonBuildParse::get_json_string(jobj_req_body));
        //unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(), ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
        //unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(url, HTTP_METHOD_PUT, NULL, conf_values_);

   int resp_code = send_httprequest(ctr_ptr, url, conf_values_, HTTP_METHOD_PUT, jobj_req_body );
   pfc_log_debug("Response code from Ctl for vterm update cmd : %d ", resp_code);
   if (HTTP_200_RESP_OK != resp_code) {
   pfc_log_error("vterm is not updated , resp_code is : %d", resp_code);
     return UNC_DRV_RC_ERR_GENERIC;
      }
    return UNC_RC_SUCCESS;
    }



// Delete Vterminal
UncRespCode 
delete_cmd(key_vterm_t& key_vterm,
                                            val_vterm_t& val_vterm,
                       unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  pfc_log_debug("VTERM_DELETE_CMD");
  PFC_ASSERT(ctr_ptr != NULL);
  std::string url = "";
  url.append(get_base_url());
  url.append(get_url_tail(key_vterm, val_vterm));

  std::string vterminal_name = reinterpret_cast<char *>(key_vterm.vterminal_name);
  pfc_log_debug("vterminal_name:%s",vterminal_name.c_str());
  if (0 == strlen(vterminal_name.c_str()))
     return UNC_DRV_RC_ERR_GENERIC;

  vtn_name_ = reinterpret_cast<char *>(key_vterm.vtn_key.vtn_name);
  if (0 == strlen(vtn_name_.c_str()))
     return UNC_DRV_RC_ERR_GENERIC;
        //unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(), ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
        //unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(url, HTTP_METHOD_DELETE, NULL, conf_values_);

  int resp_code = send_httprequest(ctr_ptr, url, conf_values_,HTTP_METHOD_DELETE, NULL);
  pfc_log_debug("Response code from Ctl for delete vterm : %d ", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
     pfc_log_error("vterm delete is not success , resp_code id: %d", resp_code);
     return UNC_DRV_RC_ERR_GENERIC;
    }
   return UNC_RC_SUCCESS;
      }


 std::string get_url_tail(key_vterm_t &key_in,
                               val_vterm_t &val_in) {
   std::string url(reinterpret_cast<char*>(key_in.vtn_key.vtn_name));
   url.append("/vterminals/");
   url.append(reinterpret_cast<char*>(key_in.vterminal_name));
   return url;
   }

 std::string get_base_url() {
   std::string url = "";
   url.append(BASE_URL);
   url.append(CONTAINER_NAME);
   url.append(VTNS);
   url.append("/");
   return url;
      }



// fetch all child configuration for the parent key type(root)
UncRespCode 
fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector) {
  key_vterm_t key;
  val_vterm_t val;
  std::string controller_name_ =  ctr_ptr->get_controller_id();
  std::string domain_name;
  pfc_log_debug("fetch_config for OdcVterminalCommand");
  key_vtn_t* parent_vtn = reinterpret_cast<key_vtn_t*> (parent_key);
  std::string vtn_name = reinterpret_cast<const char*>
      (parent_vtn->vtn_name);
  pfc_log_debug("parent_vtn_name:%s",vtn_name.c_str());
  pfc_log_debug("%s:vtn_name", vtn_name.c_str());
  if ((0 == strlen(vtn_name.c_str()))){
      pfc_log_error("empty_vtn");
      return UNC_DRV_RC_ERR_GENERIC;
      }
        std::string url = get_base_url();
        pfc_log_debug("uri is %s",url.c_str());
        url.append(vtn_name);
        url.append("/vterminals");
        pfc_log_debug("url:%s", url.c_str());

       unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(), ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
       unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request( url, HTTP_METHOD_GET, NULL, conf_values_);
        pfc_log_debug("url:%s", url.c_str());


        if (HTTP_200_RESP_OK != response->code) {
          pfc_log_error("%d error resp ", response->code);
          return UNC_DRV_RC_ERR_GENERIC;
        }
        char *data = NULL;
        if (NULL != response->write_data) {
         if (NULL != response->write_data->memory) {
           data = response->write_data->memory;
           pfc_log_debug("vterminal present : %s", data);
         }
        }

        json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
        if (json_object_is_type(jobj, json_type_null)) {
          pfc_log_error("json_object_is_type error");
          json_object_put(jobj);
          return UNC_DRV_RC_ERR_GENERIC;
        }
        VtermParser obj(vtn_name, controller_name_, domain_name);
        pfc_log_debug("calling vterm_response");
        int ret_val = obj.parse_vterm_response(jobj,ctr_ptr,conf_values_, key, val, cfg_node_vector);
        pfc_log_debug("leaving vterm_response");
        if (restjson::REST_OP_SUCCESS != ret_val)
              return UNC_DRV_RC_ERR_GENERIC;
        return UNC_RC_SUCCESS;
    }
};

}
}
#endif
