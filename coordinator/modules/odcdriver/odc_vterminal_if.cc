/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vterminal_if.hh>

namespace unc {
namespace odcdriver {
// Constructor
OdcVtermIfCommand::OdcVtermIfCommand
    (unc::restjson::ConfFileValues_t conf_values)
    : conf_file_values_(conf_values) {
      ODC_FUNC_TRACE;
    }

// Destructor
OdcVtermIfCommand::~OdcVtermIfCommand() {
}

odc_drv_resp_code_t OdcVtermIfCommand::validate_logical_port_id(
    const std::string& logical_port_id) {
  ODC_FUNC_TRACE;
  pfc_log_debug("logical port received : %s", logical_port_id.c_str());
  //  First 3 characters should be PP-
  if ((logical_port_id.compare(0, 3, PP_PREFIX) != 0) &&
      (logical_port_id.size() < 28)) {
    pfc_log_error("Logical port id is not with PP- prefix");
    return ODC_DRV_FAILURE;
  }

  //  Split Switch id and prefix
  std::string switch_port = logical_port_id.substr(3);
  size_t hyphen_occurence = switch_port.find("-");
  if (hyphen_occurence == std::string::npos) {
    pfc_log_error("Error in Validating the port");
    return ODC_DRV_FAILURE;
  }
  //  Split switch id alone without port name
  std::string switch_id = switch_port.substr(0, hyphen_occurence);
  size_t base_occurence = switch_id.find(":");
  std::string switch_base = switch_id.substr(0, base_occurence);
  pfc_log_info("switch_base:%s", switch_base.c_str());
  if ((switch_base.compare("openflow"))) {
    pfc_log_error("Invalid switch id format supported by Vtn Manager");
    return ODC_DRV_FAILURE;
  }
  pfc_log_debug("Switch id in port map %s", switch_id.c_str());
  pfc_log_debug("Valid logical_port id");
  return ODC_DRV_SUCCESS;
}

//  Check the format of logical port id. If it is invalid converts to proper
//  format
odc_drv_resp_code_t OdcVtermIfCommand::check_logical_port_id_format(
    std::string& logical_port_id) {
  ODC_FUNC_TRACE;
  // If First SIX digits are in the format PP-OF: change to PP-
  if (logical_port_id.compare(0, 6, PP_OF_PREFIX) == 0) {
    logical_port_id.replace(logical_port_id.find(PP_OF_PREFIX),
                            PP_OF_PREFIX.length(), PP_PREFIX);
  }
  odc_drv_resp_code_t logical_port_retval = validate_logical_port_id(
      logical_port_id);
  if (logical_port_retval != ODC_DRV_SUCCESS) {
    pfc_log_error("Failed during validation of logical port id %s",
                  logical_port_id.c_str());
    return ODC_DRV_FAILURE;
  }
  return logical_port_retval;
}

// Creates Request Body for Port Map
json_object* OdcVtermIfCommand::create_request_body_port_map(
    val_vterm_if_t& vterm_if_val, const std::string &logical_port_id) {
  ODC_FUNC_TRACE;
  std::string switch_id = "";
  std::string port_name = "";
  pfc_log_debug("VALUE RECEIVED for LOGICAL PORT %u" ,
                vterm_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]);

  pfc_log_debug("logical_port_id is %s", logical_port_id.c_str());\
      std::string switch_port = logical_port_id.substr(3);
  size_t hyphen_occurence = switch_port.find("-");
  std::string of_switch_id = switch_port.substr(0, hyphen_occurence);
  port_name = switch_port.substr(hyphen_occurence+1);
  if ((of_switch_id.empty()) || (port_name.empty())) {
    pfc_log_error("port name or switch id is empty");
    return NULL;
  }
  // convert switch id from unsigned decimal to hex string
  int switch_val = atoi(of_switch_id.substr(9).c_str());
  std::stringstream stream;
  stream << std::hex << switch_val;
  switch_id = stream.str();
  pfc_log_debug("port name : %s", port_name.c_str());
  pfc_log_debug("switch id : %s", switch_id.c_str());
  json_object *jobj_parent = unc::restjson::JsonBuildParse::create_json_obj();
  json_object *jobj_node = unc::restjson::JsonBuildParse::create_json_obj();
  json_object *jobj_port = unc::restjson::JsonBuildParse::create_json_obj();
  int vlanid(0);
  if ((UNC_VF_VALID == vterm_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM])
      &&
      (UNC_VF_VALID == vterm_if_val.portmap.valid[UPLL_IDX_TAGGED_PM])) {
    if (vterm_if_val.portmap.tagged == UPLL_VLAN_TAGGED) {
      vlanid = vterm_if_val.portmap.vlan_id;
    }
  }
  if (vlanid >= 0) {
    uint32_t ret_val = unc::restjson::JsonBuildParse::build<int>
        ("vlan", vlanid, jobj_parent);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      json_object_put(jobj_parent);
      json_object_put(jobj_node);
      json_object_put(jobj_port);
      return NULL;
    }
  }
  uint32_t ret_val = unc::restjson::JsonBuildParse::build<std::string>
      ("type", "OF", jobj_node);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error in framing req body of type");
    json_object_put(jobj_parent);
    json_object_put(jobj_node);
    json_object_put(jobj_port);
    return NULL;
  }
  if (!(switch_id.empty())) {
    ret_val = unc::restjson::JsonBuildParse::build<std::string>
        ("id", switch_id, jobj_node);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("Failed in framing json request body for id");
      json_object_put(jobj_parent);
      json_object_put(jobj_node);
      json_object_put(jobj_port);
      return NULL;
    }
  }
  ret_val = unc::restjson::JsonBuildParse::build<json_object*>
      ("node", jobj_node, jobj_parent);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Failed in framing json request body for node");
    json_object_put(jobj_parent);
    json_object_put(jobj_node);
    json_object_put(jobj_port);
    return NULL;
  }

  if (!(port_name.empty())) {
    ret_val = unc::restjson::JsonBuildParse::build<std::string>
        ("name", port_name, jobj_port);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("Failed in framing json request body for name");
      json_object_put(jobj_parent);
      json_object_put(jobj_node);
      json_object_put(jobj_port);
      return NULL;
    }
  }
  ret_val = unc::restjson::JsonBuildParse::build <json_object*>
      ("port", jobj_port, jobj_parent);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_debug("Failed in framing json request body for port");
    json_object_put(jobj_parent);
    json_object_put(jobj_node);
    json_object_put(jobj_port);
    return NULL;
  }

  return jobj_parent;
}


json_object* OdcVtermIfCommand::create_request_body(val_vterm_if_t&
                                                    val_vterm_if) {
  ODC_FUNC_TRACE;
  json_object *jobj = unc::restjson::JsonBuildParse::create_json_obj();
  int ret_val = 1;

  if (UNC_VF_VALID == val_vterm_if.valid[UPLL_IDX_DESC_VTERMI]) {
    char* description =
        reinterpret_cast<char*>(val_vterm_if.description);
    pfc_log_debug("%s description", description);
    if (0 != strlen(description)) {
      ret_val = unc::restjson::JsonBuildParse::build("description",
                                                     description,
                                                     jobj);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error in building vterm if req_body description");
        return NULL;
      }
    }
  }
  if (UNC_VF_VALID == val_vterm_if.valid[UPLL_IDX_ADMIN_STATUS_VTERMI]) {
    if (UPLL_ADMIN_ENABLE == val_vterm_if.admin_status) {
      ret_val = unc::restjson::JsonBuildParse::build("enabled", "true", jobj);
    } else if (UPLL_ADMIN_DISABLE == val_vterm_if.admin_status) {
      ret_val = unc::restjson::JsonBuildParse::build("enabled", "false", jobj);
    }
    if (restjson::REST_OP_SUCCESS != ret_val) {
      return NULL;
    }
  }
  return jobj;
}


// Create Command for vterm_if
UncRespCode OdcVtermIfCommand::create_cmd(key_vterm_if_t& vterm_if_key,
                                        val_vterm_if_t& vterm_if_val,
                                        unc::driver::controller*
                                        ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string vterm_if_url = get_vterm_if_url(vterm_if_key);
  if (vterm_if_url.empty()) {
    pfc_log_error("vterm_if url is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  json_object* vterm_if_json_request_body = create_request_body(vterm_if_val);
  json_object* port_map_json_req_body = NULL;
  if (vterm_if_val.valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) {
    if (UNC_VF_VALID !=
        vterm_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]) {
      pfc_log_error("portmap - logical port valid flag is not set");
      json_object_put(vterm_if_json_request_body);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    std::string logical_port_id =
        reinterpret_cast<char*>(vterm_if_val.portmap.logical_port_id);
    odc_drv_resp_code_t logical_port_retval = check_logical_port_id_format(
        logical_port_id);
    if (logical_port_retval != ODC_DRV_SUCCESS) {
      pfc_log_error("logical port id is Invalid");
      json_object_put(vterm_if_json_request_body);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    port_map_json_req_body = create_request_body_port_map(vterm_if_val,
                                                          logical_port_id);
    if (json_object_is_type(port_map_json_req_body, json_type_null)) {
      pfc_log_error("request body is null");
      json_object_put(vterm_if_json_request_body);
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }

  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                                        ctr_ptr->get_user_name(),
                                        ctr_ptr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
    vterm_if_url, restjson::HTTP_METHOD_POST,
    unc::restjson::JsonBuildParse::get_json_string(vterm_if_json_request_body),
    conf_file_values_);

  json_object_put(vterm_if_json_request_body);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    json_object_put(port_map_json_req_body);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctl for vterm_if create_cmd: %d",
                resp_code);
  if (HTTP_201_RESP_CREATED != resp_code) {
    pfc_log_error("create vterm_if is not success , resp_code %d", resp_code);
    json_object_put(port_map_json_req_body);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  // VBR_IF successful...Check for PortMap
  if (vterm_if_val.valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) {
    std::string port_map_url = "";
    port_map_url.append(vterm_if_url);
    port_map_url.append("/portmap");
    unc::restjson::HttpResponse_t* port_map_response =
        rest_util_obj.send_http_request(
            port_map_url,
            restjson::HTTP_METHOD_PUT,
            unc::restjson::JsonBuildParse::get_json_string(
                port_map_json_req_body),
            conf_file_values_);
    json_object_put(port_map_json_req_body);
    if (NULL == port_map_response) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    int resp_code = port_map_response->code;
    if (HTTP_200_RESP_OK != resp_code) {
      pfc_log_error("port map  update is not success,rep_code: %d", resp_code);
      return UNC_DRV_RC_ERR_GENERIC;
    }
  } else {
    pfc_log_debug("No Port map");
  }
  return UNC_RC_SUCCESS;
}



// Form URL for vterminal interface ,inject request to controller
std::string OdcVtermIfCommand::get_vterm_if_url(key_vterm_if_t& vterm_if_key) {
  ODC_FUNC_TRACE;
  char* vtnname = NULL;
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  vtnname = reinterpret_cast<char*>(vterm_if_key.vterm_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("vtn name is empty");
    return "";
  }
  url.append(vtnname);

  char* vterminalname = reinterpret_cast<char*>
                                      (vterm_if_key.vterm_key.vterminal_name);
  if (0 == strlen(vterminalname)) {
    pfc_log_error("vterminal name is empty");
    return "";
  }
  url.append("/vterminals/");
  url.append(vterminalname);

  char* intfname = reinterpret_cast<char*>(vterm_if_key.if_name);
  if (0 == strlen(intfname)) {
    pfc_log_error("interface name is empty");
    return "";
  }
  url.append("/interfaces/");
  url.append(intfname);
  return url;
}



// Update Cmd for Vterm If
UncRespCode OdcVtermIfCommand::update_cmd(key_vterm_if_t& vterm_if_key,
                       val_vterm_if_t& vterm_if_val_old,
                       val_vterm_if_t& vterm_if_val_new,
                       unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string vterm_if_url = get_vterm_if_url(vterm_if_key);
  if (vterm_if_url.empty()) {
    return UNC_DRV_RC_ERR_GENERIC;
  }
  json_object* vterm_if_json_request_body = create_request_body(vterm_if_val_new);
  json_object* port_map_json_req_body = NULL;
  if (vterm_if_val_new.valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) {
    if (UNC_VF_INVALID ==
        vterm_if_val_new.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]) {
      pfc_log_error("portmap - logical port valid flag is not set");
      json_object_put(vterm_if_json_request_body);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    std::string logical_port_id =
        reinterpret_cast<char*>(vterm_if_val_new.portmap.logical_port_id);
    odc_drv_resp_code_t logical_port_retval = check_logical_port_id_format(
        logical_port_id);
    if (logical_port_retval != ODC_DRV_SUCCESS) {
      json_object_put(vterm_if_json_request_body);
      pfc_log_error("logical port id is invalid");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    port_map_json_req_body = create_request_body_port_map(vterm_if_val_new,
                                                          logical_port_id);
    if (json_object_is_type(port_map_json_req_body, json_type_null)) {
      pfc_log_error("port map req body is null");
      json_object_put(vterm_if_json_request_body);
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }

  pfc_log_debug("Request body formed in update vterm_if_cmd : %s",
                unc::restjson::JsonBuildParse::get_json_string
                (vterm_if_json_request_body));

  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                                        ctr_ptr->get_user_name(),
                                       ctr_ptr->get_pass_word());
      unc::restjson::HttpResponse_t* response =
          rest_util_obj.send_http_request(vterm_if_url,
                                 restjson::HTTP_METHOD_PUT,
                                 unc::restjson::JsonBuildParse::get_json_string
                                 (vterm_if_json_request_body),
                                 conf_file_values_);

      json_object_put(vterm_if_json_request_body);
      if (NULL == response) {
        pfc_log_error("Error Occured while getting httpresponse");
        json_object_put(port_map_json_req_body);
        return UNC_DRV_RC_ERR_GENERIC;
      }
      int resp_code = response->code;
      pfc_log_debug("Response code from Ctl for update vterm_if : %d ",
                    resp_code);
      if (HTTP_200_RESP_OK != resp_code) {
        pfc_log_error("update vterm_if is not successful %d", resp_code);
        json_object_put(port_map_json_req_body);
        return UNC_DRV_RC_ERR_GENERIC;
      }

      uint32_t port_map_resp_code = ODC_DRV_FAILURE;
      unc::restjson::HttpResponse_t* port_map_response = NULL;
      // VTERM_IF successful...Check for PortMap
      if (vterm_if_val_new.valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) {
        std::string port_map_url = "";
        port_map_url.append(vterm_if_url);
        port_map_url.append("/portmap");
        pfc_log_debug("Request body formed in update portmap: %s",
           unc::restjson::JsonBuildParse::get_json_string
                                              (port_map_json_req_body));
        port_map_response = rest_util_obj.send_http_request(
            port_map_url, restjson::HTTP_METHOD_PUT,
            unc::restjson::JsonBuildParse::get_json_string
                                                (port_map_json_req_body),
                                                 conf_file_values_);

        json_object_put(port_map_json_req_body);
        if (NULL == port_map_response) {
          pfc_log_error("Error Occured while getting httpresponse");
          return UNC_DRV_RC_ERR_GENERIC;
        }
        port_map_resp_code = port_map_response->code;
        if (HTTP_200_RESP_OK != port_map_resp_code) {
          pfc_log_error("update portmap is not successful %d",
                        port_map_resp_code);
          return UNC_DRV_RC_ERR_GENERIC;
        }
      } else if ((vterm_if_val_new.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE)
              && (vterm_if_val_new.valid[PFCDRV_IDX_VAL_VBRIF] == UNC_VF_VALID)) {
        std::string port_map_url = "";
        port_map_url.append(vterm_if_url);
        port_map_url.append("/portmap");
        port_map_response = rest_util_obj.send_http_request(
            port_map_url, restjson::HTTP_METHOD_DELETE, NULL,
            conf_file_values_);

        if (NULL == port_map_response) {
          pfc_log_error("Error Occured while getting httpresponse");
          return UNC_DRV_RC_ERR_GENERIC;
        }
        port_map_resp_code = port_map_response->code;
        if (HTTP_200_RESP_OK != port_map_resp_code) {
          pfc_log_error("delete portmap is not successful %d",
                        port_map_resp_code);
          return UNC_DRV_RC_ERR_GENERIC;
        }
      }
      pfc_log_debug("Response code from Ctl for portmap:%d",
                    port_map_resp_code);
      return UNC_RC_SUCCESS;
    }

UncRespCode OdcVtermIfCommand::delete_cmd(key_vterm_if_t& vterm_if_key,
                       val_vterm_if_t& vterm_if_val,
                       unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  pfc_log_debug("%s Enter delete_cmd", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string vterm_if_url = get_vterm_if_url(vterm_if_key);
  pfc_log_debug("vterm_if_url:%s", vterm_if_url.c_str());
  if (vterm_if_url.empty()) {
    pfc_log_error("brif url is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                                        ctr_ptr->get_user_name(),
                                        ctr_ptr->get_pass_word());
  unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(vterm_if_url,
                                      restjson::HTTP_METHOD_DELETE, NULL,
                                      conf_file_values_);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctl for delete vterm_if : %d ", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("delete vterm_if is not successful %d", resp_code);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}

UncRespCode OdcVtermIfCommand::get_vterm_if_list(
    std::string vtn_name,
    std::string vterminal_name,
    unc::driver::controller* ctr,
    std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr != NULL);
  if ((0 == strlen(vtn_name.c_str())) || (0 ==
                                          strlen(vterminal_name.c_str()))) {
    pfc_log_error("Empty vtn / vterminal name");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string url = "";
  url.append(RESTCONF_BASE);
  url.append(VTNS);
  url.append("/");
  url.append(vtn_name);
  url.append("/vterminal/");
  url.append(vterminal_name);
  url.append("/interfaces");

  unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                                        ctr->get_user_name(),
                                        ctr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
      url, restjson::HTTP_METHOD_GET, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctlfor get vterm_if : %d ", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("rest_util_obj send_request fail");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data = response->write_data->memory;
      UncRespCode parse_ret = UNC_DRV_RC_ERR_GENERIC;
      parse_ret = parse_vterm_if_response(vtn_name, vterminal_name,
                                          url, ctr, data,
                                          cfgnode_vector);
      return parse_ret;
    }
  }
  return UNC_DRV_RC_ERR_GENERIC;
}

UncRespCode OdcVtermIfCommand::fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  pfc_log_debug("fetch  OdcVbrIfCommand config");

  key_vterm_t* parent_vterminal = reinterpret_cast<key_vterm_t*> (parent_key);
  std::string parent_vtn_name =
      reinterpret_cast<const char*> (parent_vterminal->vtn_key.vtn_name);
  std::string parent_vterminal_name =
      reinterpret_cast<const char*> (parent_vterminal->vterminal_name);

  pfc_log_debug("parent_vtn_name:%s, parent_vterminal_name:%s",
                parent_vtn_name.c_str(), parent_vterminal_name.c_str());

  return get_vterm_if_list(parent_vtn_name, parent_vterminal_name,
                                            ctr, cfgnode_vector);
}

UncRespCode OdcVtermIfCommand::parse_vterm_if_response(
    std::string vtn_name,
    std::string vterminal_name,
    std::string url,
    unc::driver::controller* ctr,
    char *data,
    std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  json_object* jobj = unc::restjson::JsonBuildParse::
      get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_debug("No vterm interface present");
    return UNC_RC_SUCCESS;
  }
  uint32_t array_length = 0;
  json_object *json_obj_vterm_if = NULL;

  uint32_t ret_val = unc::restjson::JsonBuildParse::parse(jobj, "interface",
                                                      -1, json_obj_vterm_if);
  if (json_object_is_type(json_obj_vterm_if, json_type_null)) {
    pfc_log_error("json vterm_if is not present");
    json_object_put(jobj);
    return UNC_RC_SUCCESS;
  }

  if (restjson::REST_OP_SUCCESS != ret_val) {
    json_object_put(jobj);
    pfc_log_error("JsonBuildParse::parse fail");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if (json_object_is_type(json_obj_vterm_if, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj,
                                                              "interface");
  }

  pfc_log_debug("interface array_length:%d", array_length);
  UncRespCode resp_code = UNC_DRV_RC_ERR_GENERIC;
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    pfc_log_debug("inside array_length for loop");
    resp_code = fill_config_node_vector(vtn_name, vterminal_name,
                                        json_obj_vterm_if,
                                       arr_idx, url, ctr, cfgnode_vector);
    if (UNC_DRV_RC_ERR_GENERIC == resp_code) {
      json_object_put(jobj);
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  json_object_put(jobj);
  return UNC_RC_SUCCESS;
}

UncRespCode OdcVtermIfCommand::fill_config_node_vector(
    std::string vtn_name,
    std::string vterminal_name,
    json_object *json_obj,
    uint32_t arr_idx,
    std::string url,
    unc::driver::controller* ctr,
    std::vector< unc::vtndrvcache::ConfigNode *>&cfgnode_vector) {

  ODC_FUNC_TRACE;
  key_vterm_if_t key_vterm_if;
  val_vterm_if_t val_vterm_if;
  memset(&key_vterm_if, 0, sizeof(key_vterm_if_t));
  memset(&val_vterm_if, 0, sizeof(val_vterm_if_t));

  std::string name = "";
  std::string description = "";
  std::string entity_state = "";
  uint32_t ret_val = unc::restjson::JsonBuildParse::parse(json_obj,
                                                          "name",
                                                          arr_idx, name);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("JsonBuildParse::parse fail.");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_debug("vterm_if name: %s", name.c_str());
  if (strlen(name.c_str()) == 0) {
    pfc_log_error("NO vterm_if name");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = unc::restjson::JsonBuildParse::parse(json_obj,
                                                 "description",
                                                 arr_idx, description);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_debug("vterm_if description: %s", description.c_str());
  //  Fills the vtermif KEY structure
  strncpy(reinterpret_cast<char*> (key_vterm_if.vterm_key.vtn_key.vtn_name),
       vtn_name.c_str(), sizeof(key_vterm_if.vterm_key.vtn_key.vtn_name) - 1);
  pfc_log_debug(" vtn name in vtermif:%s", reinterpret_cast<char*>
                (key_vterm_if.vterm_key.vtn_key.vtn_name));
  strncpy(reinterpret_cast<char*> (key_vterm_if.vterm_key.vterminal_name),
    vterminal_name.c_str(), sizeof(key_vterm_if.vterm_key.vterminal_name) - 1);
  pfc_log_error(" vterminal name in vtermif:%s",
            reinterpret_cast<char*> (key_vterm_if.vterm_key.vterminal_name));

  strncpy(reinterpret_cast<char*> (key_vterm_if.if_name), name.c_str(),
          sizeof(key_vterm_if.if_name) - 1);
  pfc_log_debug(" vtermif name:%s",
           reinterpret_cast<char*> (key_vterm_if.if_name));

  if (0 == strlen(description.c_str())) {
    val_vterm_if.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  } else {
    val_vterm_if.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
    strncpy(reinterpret_cast<char*> (val_vterm_if.description),
            description.c_str(), sizeof(val_vterm_if.description) - 1);
  }
  std::string admin_status = "";
  ret_val = unc::restjson::JsonBuildParse::parse(json_obj,
                                                 "enabled",
                                                 arr_idx, admin_status);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    return UNC_DRV_RC_ERR_GENERIC;
  }

  val_vterm_if.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
  if (admin_status.compare("true") == 0) {
    val_vterm_if.admin_status = UPLL_ADMIN_ENABLE;
  } else if (admin_status.compare("false") == 0) {
    val_vterm_if.admin_status = UPLL_ADMIN_DISABLE;
  }

  url.append("/");
  url.append(name);
  int resp_code = 0;
  json_object *jobj = read_portmap(ctr, url, resp_code);
  if (0 == resp_code) {
    pfc_log_error("Error while reading portmap");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_debug("Response code from Ctl for portmap read : %d ", resp_code);
  if (HTTP_204_NO_CONTENT == resp_code) {
    pfc_log_debug("No Content for portmap received");
    val_vterm_if.valid[UPLL_IDX_PM_VTERMI] = UNC_VF_INVALID;
  } else if (HTTP_200_RESP_OK == resp_code) {
    pfc_log_debug("json response for read_port_map");
    if (json_object_is_type(jobj, json_type_null)) {
      pfc_log_error("null jobj no portmap");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      val_vterm_if.valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
      int vlanid(0);
      ret_val = unc::restjson::JsonBuildParse::parse(jobj, "vlan", -1,
                                                     vlanid);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        json_object_put(jobj);
        pfc_log_debug("vlan parse error");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      pfc_log_debug("vlan id in portmap read %d", vlanid);
      if (0 == vlanid) {
        val_vterm_if.portmap.valid[UPLL_IDX_VLAN_ID_PM] =
            UNC_VF_INVALID;
        pfc_log_debug("untagged");
        val_vterm_if.portmap.tagged = UPLL_VLAN_UNTAGGED;
        val_vterm_if.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      } else {
        pfc_log_debug("vlan id valid");
        val_vterm_if.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
        val_vterm_if.portmap.vlan_id = vlanid;
        pfc_log_debug("%d  vlan id ", vlanid);
        pfc_log_debug("vlan id tagged");
        val_vterm_if.portmap.tagged = UPLL_VLAN_TAGGED;
        val_vterm_if.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      }
      json_object *jobj_node = NULL;
      json_object *jobj_port = NULL;
      ret_val = unc::restjson::JsonBuildParse::parse(jobj, "node", -1,
                                                     jobj_node);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_debug("node is null");
        json_object_put(jobj);
        return UNC_DRV_RC_ERR_GENERIC;
      }
      ret_val = unc::restjson::JsonBuildParse::parse(jobj, "port",
                                                     -1, jobj_port);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        json_object_put(jobj);
        pfc_log_debug("Parsing error in port");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      std::string node_id = "";
      std::string port_name = "";
      std::string logical_port = "PP-OF:";

      if ((json_object_is_type(jobj_node, json_type_null)) ||
          (json_object_is_type(jobj_port, json_type_null))) {
        pfc_log_error("node or port json object is null");
        return UNC_DRV_RC_ERR_GENERIC;
      }

      ret_val = unc::restjson::JsonBuildParse::parse(jobj_node, "id",
                                                     -1, node_id);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        json_object_put(jobj);
        pfc_log_debug("id parse error");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      ret_val = unc::restjson::JsonBuildParse::parse(jobj_port, "name",
                                                     -1, port_name);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        json_object_put(jobj);
        pfc_log_debug("name parse error");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      unc::odcdriver::OdcController *odc_ctr =
          reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
      PFC_ASSERT(odc_ctr != NULL);
      std::string switch_val = odc_ctr->frame_openflow_switchid(node_id);
      if (switch_val.empty()) {
        pfc_log_error("%s:switch id empty", PFC_FUNCNAME);
        return UNC_DRV_RC_ERR_GENERIC;
      }
      logical_port.append(switch_val);
      logical_port.append(HYPHEN);
      logical_port.append(port_name);
      pfc_log_debug("logical port id %s", logical_port.c_str());
      strncpy(reinterpret_cast<char*>
              (val_vterm_if.portmap.logical_port_id),
              logical_port.c_str(),
              sizeof(val_vterm_if.portmap.logical_port_id) - 1);
      pfc_log_debug("%s logical port id in readportmap " ,
                    reinterpret_cast<char*>
                    (val_vterm_if.portmap.logical_port_id));
      val_vterm_if.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
          UNC_VF_VALID;


      json_object_put(jobj);
    }
  } else {
    pfc_log_error("Error in response code while reading port map:%d",
                  resp_code);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  unc::vtndrvcache::ConfigNode *cfgptr =
          new unc::vtndrvcache::CacheElementUtil<key_vterm_if_t,
                                                 val_vterm_if_t,
                                                 val_vterm_if_t,
                                                 uint32_t> (&key_vterm_if,
                                                 &val_vterm_if, &val_vterm_if,
                                                 uint32_t(UNC_OP_READ));
      PFC_ASSERT(cfgptr != NULL);
      cfgnode_vector.push_back(cfgptr);
      return UNC_RC_SUCCESS;
}

json_object* OdcVtermIfCommand::read_portmap(unc::driver::controller* ctr,
                           std::string url,
                           int &resp_code) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr != NULL);

  url.append("/portmap");

  unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                                ctr->get_user_name(), ctr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
      url, restjson::HTTP_METHOD_GET, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return NULL;
  }
  resp_code = response->code;
  pfc_log_debug("Response code from Ctlfor portmap read : %d ", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    return NULL;
  }

  json_object *jobj = NULL;
  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data = response->write_data->memory;
      jobj =  unc::restjson::JsonBuildParse::get_json_object(data);
      return jobj;
    }
  }
  return NULL;
}
}  // namespace odcdriver
}  // namespace unc

