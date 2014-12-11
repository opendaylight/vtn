/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_VTERM_HH_
#define _ODC_VTERM_HH_

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <odc_vtn.hh>
#include <odc_flowfilter_template.hh>
#include <odl_vterm.hh>
#include <odc_rest.hh>
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
  std::string vtn_name_, controller_name_;
  
public:
  OdcVterminalCommand(unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values){}

  UncRespCode
  create_cmd(key_vterm_t& key, val_vterm_t& val,
             unc::driver::controller *ctr) {
    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr != NULL);
    std::string url = "";
    url.append(get_base_url());
    url.append(get_url_tail(key, val));
  
    json_object *jobj_req_body = unc::restjson::JsonBuildParse::create_json_obj();
    pfc_log_debug("calling function create_vbr_request");

    int retval = create_vterm_request(jobj_req_body, key, val);
    if (retval != UNC_RC_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC; 
    
    unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                          ctr->get_user_name(), ctr->get_pass_word());
    unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(url,
          restjson::HTTP_METHOD_POST,
          unc::restjson::JsonBuildParse::get_json_string(jobj_req_body),
          conf_values_);
     json_object_put(jobj_req_body);
     if (NULL == response) {
       pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
   int resp_code = response->code;
   pfc_log_debug("response code returned in create vterminal is %d", resp_code);
    if (HTTP_201_RESP_CREATED != resp_code) {
      pfc_log_debug("check if vtn is stand-alone");
      //  check if vtn is stand-alone
      UncRespCode ret_code = UNC_DRV_RC_ERR_GENERIC;
      std::vector<unc::vtndrvcache::ConfigNode*> child_list;
      child_list.clear();
      std::string vtn_name = reinterpret_cast<const char*>
                                     (key.vtn_key.vtn_name);
      pfc_log_debug("VTN name str:%s", vtn_name.c_str());
      ret_code = get_vterminal_list(vtn_name, ctr, child_list);
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
          memcpy(key_vtn.vtn_name, key.vtn_key.vtn_name,
                sizeof(key.vtn_key.vtn_name));
          pfc_log_debug("VTN name:%s", key_vtn.vtn_name);
          OdcVtnCommand vtn_obj(conf_values_);
          vtn_obj.delete_cmd(key_vtn, val_vtn, ctr);
       }
     }
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}

  UncRespCode
  update_cmd(key_vterm_t& key, val_vterm_t& val,
             unc::driver::controller *ctr) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  delete_cmd(key_vterm_t& key, val_vterm_t& val,
             unc::driver::controller *ctr) {
    ODC_FUNC_TRACE;
    pfc_log_debug("VTERM DELETE_CMD");
    PFC_ASSERT(ctr != NULL);
    std::string url = "";
    url.append(get_base_url());
    url.append(get_url_tail(key, val));
    
    unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                                        ctr->get_user_name(),
                                        ctr->get_pass_word());
    unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
         url, restjson::HTTP_METHOD_DELETE, NULL, conf_values_);

    if (NULL == response) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    int resp_code = response->code;
    pfc_log_debug("Response code from Ctl for delete vterminal : %d ",
                resp_code);
    if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("vterminal delete is not success , resp_code id: %d",
                  resp_code);
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

  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector){

    controller_name_ = ctr->get_controller_id();
    key_vtn_t* parent_vtn = reinterpret_cast<key_vtn_t*> (parent_key);
    vtn_name_ = reinterpret_cast<const char*>
                     (parent_vtn->vtn_name);
    pfc_log_debug("%s:vtn_name", vtn_name_.c_str());
    return get_vterminal_list(vtn_name_, ctr, cfgnode_vector);
  }

UncRespCode get_vterminal_list(std::string vtnname,
                               unc::driver::controller* ctr,
                               std::vector<unc::vtndrvcache::ConfigNode *>
                               &cfg_node_vector);
UncRespCode parse_vterminal_response(
     char *data,
     std::string vtn_name,
     unc::driver::controller* ctr,
     std::vector< unc::vtndrvcache::ConfigNode *> &cfg_node_vector);
};

}
}

#endif
