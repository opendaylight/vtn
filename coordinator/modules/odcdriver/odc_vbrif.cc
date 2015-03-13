/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vbrif.hh>
#include <odl_vbrif.hh>
#include <odl_vbrif_portmap.hh>
namespace unc {
namespace odcdriver {


// Constructor
OdcVbrIfCommand::OdcVbrIfCommand(unc::restjson::ConfFileValues_t conf_values)
: conf_file_values_(conf_values) {
  ODC_FUNC_TRACE;
}

// Destructor
OdcVbrIfCommand::~OdcVbrIfCommand() {
}

// Form URL for vbridge interface ,inject request to controller
std::string OdcVbrIfCommand::get_vbrif_url(key_vbr_if_t& vbrif_key) {
  ODC_FUNC_TRACE;
  char* vtnname = NULL;
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  vtnname = reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("vtn_name is empty");
    return "";
  }
  url.append(vtnname);

  char* vbrname = reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name);
  if (0 == strlen(vbrname)) {
    pfc_log_error("vbr name is empty");
    return "";
  }
  url.append("/vbridges/");
  url.append(vbrname);

  char* intfname = reinterpret_cast<char*>(vbrif_key.if_name);
  if (0 == strlen(intfname)) {
    pfc_log_error("interface name is empty");
    return "";
  }
  url.append("/interfaces/");
  url.append(intfname);
  return url;
}

// Create Command for vbrif
UncRespCode OdcVbrIfCommand::create_cmd(key_vbr_if_t& key,
                                            pfcdrv_val_vbr_if_t& val,
                                            unc::driver::controller*
                                            ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string vbrif_url = get_vbrif_url(key);
  if (vbrif_url.empty()) {
    pfc_log_error("vbrif url is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  json_object *vbrif_req_body = unc::restjson::JsonBuildParse::create_json_obj();
  json_object *portmap_req_body = unc::restjson::JsonBuildParse::create_json_obj();
  VbrifParser obj;
  int ret_val = obj.create_vbr_if_request(vbrif_req_body, key, val);
  if(ret_val != UNC_RC_SUCCESS) {
    json_object_put(vbrif_req_body);
    json_object_put(portmap_req_body);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if (val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    if (UNC_VF_VALID !=
      val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]) {
      pfc_log_error("portmap - logical port valid flag is not set");
      json_object_put(vbrif_req_body);
      json_object_put(portmap_req_body);
      return UNC_DRV_RC_ERR_GENERIC;
    }
   VbrifportmapParser obj; 
    ret_val = obj.create_vbr_if_portmap_request(portmap_req_body, key,
                                                          val);
     if(ret_val != UNC_RC_SUCCESS) {
      json_object_put(vbrif_req_body);
      json_object_put(portmap_req_body);
      return UNC_DRV_RC_ERR_GENERIC;
     }
   }

  //unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
    //              ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
  //unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
    // vbrif_url, restjson::HTTP_METHOD_POST,
     //unc::restjson::JsonBuildParse::get_json_string(vbrif_req_body),
     //conf_file_values_);
  int resp_code = obj.send_httprequest(ctr_ptr, vbrif_url, conf_file_values_, restjson::HTTP_METHOD_POST, vbrif_req_body);
  json_object_put(vbrif_req_body);
  //if (NULL == response) {
  if (0 == resp_code) {
    pfc_log_error("Error Occured while getting httpresponse");
    json_object_put(portmap_req_body);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  //int resp_code = response->code;
  pfc_log_debug("Response code from Ctl for vbrif create_cmd: %d", resp_code);
  if (HTTP_201_RESP_CREATED != resp_code) {
    pfc_log_error("create vbrif is not success , resp_code %d", resp_code);
    json_object_put(portmap_req_body);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  // VBR_IF successful...Check for PortMap
  if (val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    std::string port_map_url = "";
    port_map_url.append(vbrif_url);
    port_map_url.append("/portmap");
    /*unc::restjson::HttpResponse_t* port_map_response =
        rest_util_obj.send_http_request(
            port_map_url,
            restjson::HTTP_METHOD_PUT,
            unc::restjson::JsonBuildParse::get_json_string(
                portmap_req_body),
            conf_file_values_);*/
    int resp_code = obj.send_httprequest(ctr_ptr, port_map_url,conf_file_values_,  restjson::HTTP_METHOD_PUT,portmap_req_body);
    json_object_put(portmap_req_body);
    //if (NULL == port_map_response) {
    if (0== resp_code) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    //int resp_code = port_map_response->code;
    if (HTTP_200_RESP_OK != resp_code) {
      pfc_log_error("port map  update is not success,rep_code: %d", resp_code);
      return UNC_DRV_RC_ERR_GENERIC;
    }
  } else {
    pfc_log_debug("No Port map");
  }
  return UNC_RC_SUCCESS;
}


// Update command for vbrif command
UncRespCode OdcVbrIfCommand::update_cmd(key_vbr_if_t& key,
                                        pfcdrv_val_vbr_if_t& val,
                                        unc::driver::controller
                                      *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string vbrif_url = get_vbrif_url(key);
  if (vbrif_url.empty()) {
    return UNC_DRV_RC_ERR_GENERIC;
  }

  pfc_log_debug("Create Portmap entering");
  
  json_object *vbrif_req_body = unc::restjson::JsonBuildParse::create_json_obj();
  json_object *portmap_req_body = unc::restjson::JsonBuildParse::create_json_obj();
 VbrifParser obj;
  int ret_val =obj. create_vbr_if_request(vbrif_req_body, key, val);
   if(ret_val != UNC_RC_SUCCESS) {
     json_object_put(vbrif_req_body);
     json_object_put(portmap_req_body);
     return UNC_DRV_RC_ERR_GENERIC;
   }

  if (val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    if (UNC_VF_VALID !=
      val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]) {
      pfc_log_error("portmap - logical port valid flag is not set");
      json_object_put(vbrif_req_body);
      json_object_put(portmap_req_body);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    VbrifportmapParser obj;
    ret_val = obj.create_vbr_if_portmap_request(portmap_req_body, key,
                                                          val);
     if(ret_val != UNC_RC_SUCCESS) {
      json_object_put(vbrif_req_body);
      json_object_put(portmap_req_body);
      return UNC_DRV_RC_ERR_GENERIC;
     }
   }

  /*unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                                        ctr_ptr->get_user_name(),
                                        ctr_ptr->get_pass_word());
  unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(vbrif_url,
                                restjson::HTTP_METHOD_PUT,
                                unc::restjson::JsonBuildParse::get_json_string
                                (vbrif_req_body),
                                conf_file_values_);*/
  int resp_code = obj.send_httprequest(ctr_ptr,vbrif_url, conf_file_values_, restjson::HTTP_METHOD_PUT,vbrif_req_body);

  json_object_put(vbrif_req_body);
  //if (NULL == response) {
  if (0 == resp_code) {
    pfc_log_error("Error Occured while getting httpresponse");
    json_object_put(portmap_req_body);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  //int resp_code = response->code;
  pfc_log_debug("Response code from Ctl for update vbrif : %d ", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("update vbrif is not successful %d", resp_code);
    json_object_put(portmap_req_body);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  uint32_t port_map_resp_code = ODC_DRV_FAILURE;
  //unc::restjson::HttpResponse_t* port_map_response = NULL;
  // VBR_IF successful...Check for PortMap
  if ((val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID)
      && (val.valid[PFCDRV_IDX_VAL_VBRIF] == UNC_VF_VALID)) {
    std::string port_map_url = "";
    port_map_url.append(vbrif_url);
    port_map_url.append("/portmap");
    pfc_log_debug("Request body formed in update portmap: %s",
       unc::restjson::JsonBuildParse::get_json_string(portmap_req_body));
    /*port_map_response = rest_util_obj.send_http_request(
        port_map_url, restjson::HTTP_METHOD_PUT,
        unc::restjson::JsonBuildParse::get_json_string(portmap_req_body),
        conf_file_values_);*/
     int port_map_response = obj.send_httprequest(ctr_ptr,
        port_map_url, conf_file_values_, restjson::HTTP_METHOD_PUT,portmap_req_body);
        

    json_object_put(portmap_req_body);
    if (0 == port_map_response) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    //port_map_resp_code = port_map_response->code;
    if (HTTP_200_RESP_OK != port_map_response) {
      pfc_log_error("update portmap is not successful %d", port_map_resp_code);
      return UNC_DRV_RC_ERR_GENERIC;
    }
  } else if ((val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE)
             && (val.valid[PFCDRV_IDX_VAL_VBRIF] == UNC_VF_VALID)) {
    std::string port_map_url = "";
    port_map_url.append(vbrif_url);
    port_map_url.append("/portmap");
    //port_map_response = rest_util_obj.send_http_request(
      //  port_map_url, restjson::HTTP_METHOD_DELETE, NULL, conf_file_values_);
    int port_map_response = obj.send_httprequest(ctr_ptr,
        port_map_url, conf_file_values_,restjson::HTTP_METHOD_DELETE, NULL);

    if (0 == port_map_response) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    //port_map_resp_code = port_map_response->code;
    if (HTTP_200_RESP_OK != port_map_resp_code) {
      pfc_log_error("delete portmap is not successful %d", port_map_resp_code);
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  pfc_log_debug("Response code from Ctl for portmap:%d", port_map_resp_code);
  
  return UNC_RC_SUCCESS;
}

// Delete Command for vbr if
UncRespCode OdcVbrIfCommand::delete_cmd(key_vbr_if_t& vbrif_key,
                                            pfcdrv_val_vbr_if_t& val,
                                  unc::driver::controller* ctr_ptr) {
  ODC_FUNC_TRACE;
  pfc_log_debug("%s Enter delete_cmd", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string vbrif_url = get_vbrif_url(vbrif_key);
  pfc_log_debug("vbrif_url:%s", vbrif_url.c_str());
  if (vbrif_url.empty()) {
    pfc_log_error("brif url is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());
  unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(vbrif_url,
                        restjson::HTTP_METHOD_DELETE, NULL, conf_file_values_);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctl for delete vbrif : %d ", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("delete vbrif is not successful %d", resp_code);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}

//  fetch child configurations for the parent kt(vbr)
UncRespCode OdcVbrIfCommand::fetch_config(unc::driver::controller* ctr,
                                     void* parent_key,
                                     std::vector<unc::vtndrvcache::ConfigNode *>
                                     &cfgnode_vector) {
  ODC_FUNC_TRACE;
  pfc_log_debug("fetch  OdcVbrIfCommand config");
  key_vbr_if_t key;
  pfcdrv_val_vbr_if_t val;
  key_vbr_if_t* parent_vbr_if = reinterpret_cast<key_vbr_if_t*> (parent_key);
  std::string vtn_name =
      reinterpret_cast<const char*> (parent_vbr_if->vbr_key.vtn_key.vtn_name);
  std::string vbr_name =
      reinterpret_cast<const char*> (parent_vbr_if->vbr_key.vbridge_name);

  //std::string if_name = reinterpret_cast<const char*> (parent_vbr_if->if_name);
  //pfc_log_debug("if_name:%s",if_name.c_str());

  pfc_log_debug("vtn_name:%s, parent_vbr_name:%s",
                vtn_name.c_str(), vbr_name.c_str());
  if ((0 == strlen(vtn_name.c_str())) || (0 == strlen(vbr_name.c_str()))) {
    pfc_log_error("Empty vtn / vbr name/if_name");
    return UNC_DRV_RC_ERR_GENERIC;
  }
 
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  url.append(vtn_name);
  url.append("/vbridges/");
  url.append(vbr_name);
  url.append("/interfaces");

  unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                                 ctr->get_user_name(), ctr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
                    url, restjson::HTTP_METHOD_GET, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctlfor get vbrif : %d ", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("rest_util_obj send_request fail");
    return UNC_DRV_RC_ERR_GENERIC;
  }
 char *data = NULL;
            if (NULL != response->write_data) {
             if (NULL != response->write_data->memory) {
               data = response->write_data->memory;
               pfc_log_debug("vtns present : %s", data);
             }
            }

  
  json_object* jobj = unc::restjson::JsonBuildParse::
      get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_null");
    return UNC_DRV_RC_ERR_GENERIC;
  }
 
  VbrifParser obj(vtn_name, vbr_name, url);
  pfc_log_debug("entering in to VbrifParser obj(vtn_name, vbr_name, url);");
  int ret_val = obj.parse_vbr_if_response(jobj, ctr, conf_file_values_, key,val,cfgnode_vector);
  pfc_log_debug("leaving from vbrifparser obj");
  if(UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("parse_vbr_if_response failed");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  return UNC_RC_SUCCESS;


 


  

}

}  // namespace odcdriver
}  // namespace unc

