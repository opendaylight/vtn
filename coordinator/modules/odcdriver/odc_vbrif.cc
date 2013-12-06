/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vbrif.hh>

namespace unc {
namespace odcdriver {
// Constructor
OdcVbrIfCommand::OdcVbrIfCommand() {
  ODC_FUNC_TRACE;
}

// Destructor
OdcVbrIfCommand::~OdcVbrIfCommand() {
}

uint32_t OdcVbrIfCommand::validate_logical_port_id(
    const std::string& logical_port_id) {
  ODC_FUNC_TRACE;
  pfc_log_debug("logical port received : %s", logical_port_id.c_str());
  if (logical_port_id.compare(0, 3, "PP-") != 0) {
    return ODC_DRV_FAILURE;
  }
  if ((logical_port_id.size() < 28) ||
      (logical_port_id.compare(5, 1, ":") != 0) ||
      (logical_port_id.compare(8, 1, ":") !=0) ||
      (logical_port_id.compare(11, 1, ":") != 0) ||
      (logical_port_id.compare(14, 1, ":") !=0) ||
      (logical_port_id.compare(17, 1, ":") !=0) ||
      (logical_port_id.compare(20, 1, ":") !=0) ||
      (logical_port_id.compare(23, 1, ":") !=0) ||
      (logical_port_id.compare(26, 1, "-") !=0))  {
    pfc_log_error("Invalid logical_port_id");
    return ODC_DRV_FAILURE;
  }
  pfc_log_debug("Valid logical_port id");
  return ODC_DRV_SUCCESS;
}
// Creates Request Body for Port Map
json_object* OdcVbrIfCommand::create_request_body_port_map(
    pfcdrv_val_vbr_if_t& vbrif_val) {
  ODC_FUNC_TRACE;
  std::string switch_id = "";
  std::string port_name = "";

  if (UNC_VF_VALID !=
      vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]) {
    pfc_log_error("portmap - logical port valid flag is not set");
    return NULL;
  }

  std::string logical_port_id =
      reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id);

  if (logical_port_id.length() > 0) {
    pfc_log_debug("logical_port_id is %s", logical_port_id.c_str());
    if (logical_port_id.compare(0, 3, "PP-") == 0) {
      switch_id = logical_port_id.substr(3, 23);
      port_name = logical_port_id.substr(27);
    }
  }
  if ((switch_id.empty()) || (port_name.empty())) {
    pfc_log_error("port name or switch id is empty");
    return NULL;
  }
  pfc_log_debug("port name : %s", port_name.c_str());
  pfc_log_debug("switch id : %s", switch_id.c_str());
  unc::restjson::JsonBuildParse json_obj;
  json_object *jobj_parent = json_obj.create_json_obj();
  json_object *jobj_node = json_obj.create_json_obj();
  json_object *jobj_port = json_obj.create_json_obj();
  std::string vlanid =  "";
  if ((UNC_VF_VALID == vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM])
      &&
     (UNC_VF_VALID == vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM])) {
    if (vbrif_val.val_vbrif.portmap.tagged == UPLL_VLAN_TAGGED) {
      std::ostringstream convert_vlanid;
      convert_vlanid << vbrif_val.val_vbrif.portmap.vlan_id;
      vlanid.append(convert_vlanid.str());
    }
  }
  if (!(vlanid.empty())) {
    uint32_t ret_val = json_obj.build("vlan", vlanid, jobj_parent);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      json_object_put(jobj_parent);
      json_object_put(jobj_node);
      json_object_put(jobj_port);
      return NULL;
    }
  }
  uint32_t ret_val = json_obj.build("type", "OF", jobj_node);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error in framing req body of type");
    json_object_put(jobj_parent);
    json_object_put(jobj_node);
    json_object_put(jobj_port);
    return NULL;
  }
  if (!(switch_id.empty())) {
    ret_val = json_obj.build("id", switch_id, jobj_node);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("Failed in framing json request body for id");
      json_object_put(jobj_parent);
      json_object_put(jobj_node);
      json_object_put(jobj_port);
      return NULL;
    }
  }
  ret_val = json_obj.build("node", jobj_node, jobj_parent);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Failed in framing json request body for node");
    json_object_put(jobj_parent);
    json_object_put(jobj_node);
    json_object_put(jobj_port);
    return NULL;
  }

  if (!(port_name.empty())) {
    ret_val = json_obj.build("name", port_name, jobj_port);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("Failed in framing json request body for name");
      json_object_put(jobj_parent);
      json_object_put(jobj_node);
      json_object_put(jobj_port);
      return NULL;
    }
  }
  ret_val = json_obj.build("port", jobj_port, jobj_parent);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_debug("Failed in framing json request body for port");
    json_object_put(jobj_parent);
    json_object_put(jobj_node);
    json_object_put(jobj_port);
    return NULL;
  }

  return jobj_parent;
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
    pfc_log_error("vtn name is empty");
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
drv_resp_code_t OdcVbrIfCommand::create_cmd(key_vbr_if_t& vbrif_key,
                                            pfcdrv_val_vbr_if_t& vbrif_val,
                                            unc::driver::controller*
                                            ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string ip_address = ctr_ptr->get_host_address();
  std::string vbrif_url = get_vbrif_url(vbrif_key);
  if (vbrif_url.empty()) {
    pfc_log_error("vbrif url is empty");
    return DRVAPI_RESPONSE_FAILURE;
  }
  json_object* vbrif_json_request_body = create_request_body(vbrif_val);
  json_object* port_map_json_req_body = NULL;
  unc::restjson::JsonBuildParse json_obj;
  const char* vbrif_req_body = NULL;
  const char* port_map_req_body = NULL;
  if (!(json_object_is_type(vbrif_json_request_body, json_type_null))) {
    vbrif_req_body = json_obj.get_string(vbrif_json_request_body);
  }
  pfc_log_debug("%s Request body in create_cmd vbrif ", vbrif_req_body);
  if (vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    std::string logical_port_id =
        reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id);
    uint32_t logical_port_retval = validate_logical_port_id(logical_port_id);
    if (logical_port_retval != DRVAPI_RESPONSE_SUCCESS) {
      pfc_log_error("logical port id is Invalid");
      json_object_put(vbrif_json_request_body);
      return DRVAPI_RESPONSE_FAILURE;
    }
    port_map_json_req_body = create_request_body_port_map(vbrif_val);
    if (json_object_is_type(port_map_json_req_body, json_type_null)) {
      pfc_log_error("request body is null");
      json_object_put(vbrif_json_request_body);
      return DRVAPI_RESPONSE_FAILURE;
    }
    port_map_req_body = json_obj.get_string(port_map_json_req_body);
    pfc_log_debug("%s request body for portmap ", port_map_req_body);
  }

  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr_ptr , user_name, password);
  pfc_log_debug("Request body formed in create vbrif_cmd : %s", vbrif_req_body);
  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);

  restjson::RestClient rest_client_obj(ip_address, vbrif_url,
                                       odc_port, restjson::HTTP_METHOD_POST);
  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out,
      vbrif_req_body);
  json_object_put(vbrif_json_request_body);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    json_object_put(port_map_json_req_body);
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctl for vbrif create_cmd: %d", resp_code);
  rest_client_obj.clear_http_response();
  if (HTTP_201_RESP_CREATED != resp_code) {
    pfc_log_error("create vbrif is not success , resp_code %d", resp_code);
    json_object_put(port_map_json_req_body);
    return DRVAPI_RESPONSE_FAILURE;
  }
  // VBR_IF successful...Check for PortMap
  if (vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    std::string port_map_url = "";
    port_map_url.append(vbrif_url);
    port_map_url.append("/portmap");
    restjson::RestClient rest_client_obj_port_map(ip_address,
                                              port_map_url,
                                              odc_port,
                                              unc::restjson::HTTP_METHOD_PUT);
    unc::restjson::HttpResponse_t* port_map_response =
        rest_client_obj_port_map.send_http_request(
        user_name, password, connect_time_out, req_time_out,
        port_map_req_body);
    json_object_put(port_map_json_req_body);
    if (NULL == port_map_response) {
      pfc_log_error("Error Occured while getting httpresponse");
      return DRVAPI_RESPONSE_FAILURE;
    }
    int resp_code = port_map_response->code;
    rest_client_obj_port_map.clear_http_response();
    if (HTTP_200_RESP_OK != resp_code) {
      pfc_log_error("port map  update is not success,rep_code: %d", resp_code);
      return DRVAPI_RESPONSE_FAILURE;
    }
  } else {
    pfc_log_debug("No Port map");
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

// Update command for vbrif command
drv_resp_code_t OdcVbrIfCommand::update_cmd(key_vbr_if_t& vbrif_key,
                                            pfcdrv_val_vbr_if_t& val,
                                            unc::driver::controller
                                            *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string ip_address = ctr_ptr->get_host_address();
  std::string vbrif_url = get_vbrif_url(vbrif_key);
  if (vbrif_url.empty()) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  json_object* vbrif_json_request_body = create_request_body(val);
  json_object* port_map_json_req_body = NULL;
  unc::restjson::JsonBuildParse json_obj;
  const char* vbrif_req_body  = NULL;
  const char* port_map_req_body = NULL;
  if (!(json_object_is_type(vbrif_json_request_body, json_type_null))) {
    vbrif_req_body  = json_obj.get_string(vbrif_json_request_body);
  }
  pfc_log_debug("%s , Request body in update cmd vbrif", vbrif_req_body);
  if (val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    std::string logical_port_id =
        reinterpret_cast<char*>(val.val_vbrif.portmap.logical_port_id);
    uint32_t logical_port_retval = validate_logical_port_id(logical_port_id);
    if (logical_port_retval != DRVAPI_RESPONSE_SUCCESS) {
      json_object_put(vbrif_json_request_body);
      pfc_log_error("logical port id is invalid");
      return DRVAPI_RESPONSE_FAILURE;
    }
    port_map_json_req_body = create_request_body_port_map(val);
    if (json_object_is_type(port_map_json_req_body, json_type_null)) {
      pfc_log_error("port map req body is null");
      json_object_put(vbrif_json_request_body);
      return DRVAPI_RESPONSE_FAILURE;
    }
    port_map_req_body = json_obj.get_string(port_map_json_req_body);
    pfc_log_debug("%s request body for portmap ", port_map_req_body);
  }

  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr_ptr , user_name, password);
  pfc_log_debug("Request body formed in update vbrif_cmd : %s", vbrif_req_body);
  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);

  restjson::RestClient rest_client_obj(ip_address, vbrif_url,
                                       odc_port, restjson::HTTP_METHOD_PUT);
  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out,
      vbrif_req_body);
  json_object_put(vbrif_json_request_body);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    json_object_put(port_map_json_req_body);
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctl for update vbrif : %d ", resp_code);
  rest_client_obj.clear_http_response();
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("update vbrif is not successful %d", resp_code);
    json_object_put(port_map_json_req_body);
    return DRVAPI_RESPONSE_FAILURE;
  }

  uint32_t port_map_resp_code = ODC_DRV_FAILURE;
  unc::restjson::HttpResponse_t* port_map_response = NULL;
  // VBR_IF successful...Check for PortMap
  if ((val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID)
      && (val.valid[PFCDRV_IDX_VAL_VBRIF] == UNC_VF_VALID)) {
    std::string port_map_url = "";
    port_map_url.append(vbrif_url);
    port_map_url.append("/portmap");
    restjson::RestClient rest_client_obj_port_map(ip_address, port_map_url,
                                                  odc_port,
                                                  restjson::HTTP_METHOD_PUT);
    port_map_response = rest_client_obj_port_map.send_http_request(
        user_name, password, connect_time_out, req_time_out,
        port_map_req_body);
    json_object_put(port_map_json_req_body);
    if (NULL == port_map_response) {
      pfc_log_error("Error Occured while getting httpresponse");
      return DRVAPI_RESPONSE_FAILURE;
    }
    port_map_resp_code = port_map_response->code;
    rest_client_obj_port_map.clear_http_response();
    if (HTTP_200_RESP_OK != port_map_resp_code) {
      pfc_log_error("update portmap is not successful %d", resp_code);
      return DRVAPI_RESPONSE_FAILURE;
    }
  } else if ((val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE)
             && (val.valid[PFCDRV_IDX_VAL_VBRIF] == UNC_VF_VALID)) {
    std::string port_map_url = "";
    port_map_url.append(vbrif_url);
    port_map_url.append("/portmap");
    restjson::RestClient rest_client_obj_port_map(ip_address,
                                                  port_map_url,
                                                  odc_port,
                                   restjson::HTTP_METHOD_DELETE);
    port_map_response = rest_client_obj_port_map.send_http_request(
        user_name, password, connect_time_out, req_time_out,
        NULL);
    if (NULL == port_map_response) {
      pfc_log_error("Error Occured while getting httpresponse");
      return DRVAPI_RESPONSE_FAILURE;
    }
    port_map_resp_code = port_map_response->code;
    rest_client_obj_port_map.clear_http_response();
    if (HTTP_200_RESP_OK != port_map_resp_code) {
      pfc_log_error("delete portmap is not successful %d", resp_code);
      return DRVAPI_RESPONSE_FAILURE;
    }
  }
  pfc_log_debug("Response code from Ctl for portmap:%d", port_map_resp_code);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Delete Command for vbr if
drv_resp_code_t OdcVbrIfCommand::delete_cmd(key_vbr_if_t& vbrif_key,
                                            pfcdrv_val_vbr_if_t& val,
                                  unc::driver::controller* ctr_ptr) {
  ODC_FUNC_TRACE;
  pfc_log_debug("%s Enter delete_cmd", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string vbrif_url = get_vbrif_url(vbrif_key);
  pfc_log_debug("vbrif_url:%s", vbrif_url.c_str());
  if (vbrif_url.empty()) {
    pfc_log_error("brif url is empty");
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::string ip_address = ctr_ptr->get_host_address();
  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr_ptr , user_name, password);

  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);

  restjson::RestClient rest_client_obj(ip_address, vbrif_url,
                                       odc_port, restjson::HTTP_METHOD_DELETE);
  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out,
      NULL);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctl for delete vbrif : %d ", resp_code);
  rest_client_obj.clear_http_response();
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("delete vbrif is not successful %d", resp_code);
    return DRVAPI_RESPONSE_FAILURE;
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

// Creates request body for vbr if
json_object* OdcVbrIfCommand::create_request_body(
    pfcdrv_val_vbr_if_t& val_vbrif) {
  ODC_FUNC_TRACE;
  unc::restjson::JsonBuildParse json_obj;
  json_object *jobj = json_obj.create_json_obj();
  int ret_val = 1;

  if (UNC_VF_VALID == val_vbrif.val_vbrif.valid[UPLL_IDX_DESC_VBRI]) {
    char* description =
        reinterpret_cast<char*>(val_vbrif.val_vbrif.description);
    pfc_log_debug("%s description", description);
    if (0 != strlen(description)) {
      ret_val = json_obj.build("description", description, jobj);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error in building vbrif req_body description");
        return NULL;
      }
    }
  }
  if (UNC_VF_VALID == val_vbrif.val_vbrif.valid[UPLL_IDX_ADMIN_STATUS_VBRI]) {
    if (UPLL_ADMIN_ENABLE == val_vbrif.val_vbrif.admin_status) {
      ret_val = json_obj.build("enabled", "true", jobj);
    } else if (UPLL_ADMIN_DISABLE == val_vbrif.val_vbrif.admin_status) {
      ret_val = json_obj.build("enabled", "false", jobj);
    }
    if (restjson::REST_OP_SUCCESS != ret_val) {
      return NULL;
    }
  }
  return jobj;
}

// Gets username password form controller or else conf file
void OdcVbrIfCommand::get_username_password(unc::driver::controller* ctr_ptr,
                                            std::string &user_name,
                                            std::string &password) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string user_ctr = ctr_ptr->get_user_name();
  std::string pass_ctr = ctr_ptr->get_pass_word();

  if ((user_ctr.empty()) ||
      (pass_ctr.empty())) {
    read_user_name_password(user_name, password);
  } else {
    user_name = ctr_ptr->get_user_name();
    password = ctr_ptr->get_pass_word();
  }
}

// reads username password from conf file else from default value
void OdcVbrIfCommand::read_user_name_password(std::string &user_name,
                                              std::string &password) {
  ODC_FUNC_TRACE;
  pfc::core::ModuleConfBlock set_user_password_blk(SET_USER_PASSWORD_BLK);
  if (set_user_password_blk.getBlock() != PFC_CFBLK_INVALID) {
    user_name = set_user_password_blk.getString(
        CONF_USER_NAME, DEFAULT_USER_NAME.c_str());

    password  = set_user_password_blk.getString(
        CONF_PASSWORD, DEFAULT_PASSWORD.c_str());
    pfc_log_debug("%s: Block Handle is Valid,user_name_ %s", PFC_FUNCNAME,
                  user_name.c_str());
  }  else {
    user_name = DEFAULT_USER_NAME;
    password  = DEFAULT_PASSWORD;
    pfc_log_debug("%s: Block Handle is InValid,get default user_name%s",
                  PFC_FUNCNAME, user_name.c_str());
  }
}

// Reads set opt params from conf file if it fails reads from defs file
void OdcVbrIfCommand:: read_conf_file(uint32_t &odc_port,
                                      uint32_t &connection_time_out,
                                      uint32_t &request_time_out) {
  ODC_FUNC_TRACE;
  pfc::core::ModuleConfBlock set_opt_blk(SET_OPT_BLK);
  if (set_opt_blk.getBlock() != PFC_CFBLK_INVALID) {
    odc_port  = set_opt_blk.getUint32(CONF_ODC_PORT, DEFAULT_ODC_PORT);

    request_time_out = set_opt_blk.getUint32(
        CONF_REQ_TIME_OUT, DEFAULT_REQ_TIME_OUT);

    connection_time_out = set_opt_blk.getUint32(
        CONF_CONNECT_TIME_OUT, DEFAULT_CONNECT_TIME_OUT);
    pfc_log_debug("%s: Block Handle is Valid, odc_port_ %d", PFC_FUNCNAME,
                  odc_port);

  } else {
    odc_port   =  DEFAULT_ODC_PORT;
    connection_time_out = DEFAULT_CONNECT_TIME_OUT;
    request_time_out = DEFAULT_REQ_TIME_OUT;
    pfc_log_debug("%s: Block Handle is Invalid,set default Value %d",
                  PFC_FUNCNAME, odc_port);
  }
}

//  fetch child configurations for the parent kt(vbr)
drv_resp_code_t OdcVbrIfCommand::fetch_config(unc::driver::controller* ctr,
                                     void* parent_key,
                                     std::vector<unc::vtndrvcache::ConfigNode *>
                                     &cfgnode_vector) {
  ODC_FUNC_TRACE;
  pfc_log_debug("fetch  OdcVbrIfCommand config");

  key_vbr_t* parent_vbr = reinterpret_cast<key_vbr_t*> (parent_key);
  std::string parent_vtn_name =
      reinterpret_cast<const char*> (parent_vbr->vtn_key.vtn_name);
  std::string parent_vbr_name =
      reinterpret_cast<const char*> (parent_vbr->vbridge_name);

  pfc_log_debug("parent_vtn_name:%s, parent_vbr_name:%s",
                parent_vtn_name.c_str(), parent_vbr_name.c_str());

  return get_vbrif_list(parent_vtn_name, parent_vbr_name, ctr, cfgnode_vector);
}

// Getting  vbridge child if available
drv_resp_code_t OdcVbrIfCommand::get_vbrif_list(std::string vtn_name,
                  std::string vbr_name, unc::driver::controller* ctr,
       std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr != NULL);
  if ((0 == strlen(vtn_name.c_str())) || (0 == strlen(vbr_name.c_str()))) {
    pfc_log_error("Empty vtn / vbr name");
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::string ipaddress = ctr->get_host_address();
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  url.append(vtn_name);
  url.append("/vbridges/");
  url.append(vbr_name);
  url.append("/interfaces");

  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr, user_name, password);
  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);

  restjson::RestClient rest_client_obj(ipaddress, url,
                                       odc_port, restjson::HTTP_METHOD_GET);
  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out,
      NULL);
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code from Ctlfor get vbrif : %d ", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("rest_util_obj send_request fail");
    rest_client_obj.clear_http_response();
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data = response->write_data->memory;
      drv_resp_code_t parse_ret = DRVAPI_RESPONSE_FAILURE;
      parse_ret = parse_vbrif_response(vtn_name, vbr_name, url, ctr, data,
                                       cfgnode_vector);
      rest_client_obj.clear_http_response();
      return parse_ret;
    }
  }
  rest_client_obj.clear_http_response();
  return DRVAPI_RESPONSE_FAILURE;
}
drv_resp_code_t OdcVbrIfCommand::parse_vbrif_response(std::string vtn_name,
                                     std::string vbr_name, std::string url,
                                  unc::driver::controller* ctr, char *data,
            std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  json_object* jobj = unc::restjson::JsonBuildParse::
      get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_null");
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t array_length = 0;
  json_object *json_obj_vbrif = NULL;

  uint32_t ret_val = unc::restjson::JsonBuildParse::parse(jobj, "interface",
                                                          -1, json_obj_vbrif);
  if (json_object_is_type(json_obj_vbrif, json_type_null)) {
    pfc_log_error("json vbrif is null");
    json_object_put(jobj);
    return DRVAPI_RESPONSE_FAILURE;
  }

  if (restjson::REST_OP_SUCCESS != ret_val) {
    json_object_put(jobj);
    pfc_log_error("JsonBuildParse::parse fail");
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (json_object_is_type(json_obj_vbrif, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj,
                                                              "interface");
  }

  pfc_log_debug("interface array_length:%d", array_length);
  drv_resp_code_t resp_code = DRVAPI_RESPONSE_FAILURE;
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    pfc_log_debug("inside array_length for loop");
    resp_code = fill_config_node_vector(vtn_name, vbr_name, json_obj_vbrif,
                                          arr_idx, url, ctr, cfgnode_vector);
    if (DRVAPI_RESPONSE_FAILURE == resp_code) {
      json_object_put(jobj);
      return DRVAPI_RESPONSE_FAILURE;
    }
  }
  json_object_put(jobj);
  return DRVAPI_RESPONSE_SUCCESS;
}
// Reading port-map from active controller
json_object* OdcVbrIfCommand::read_portmap(unc::driver::controller* ctr,
                                           std::string url, int &resp_code) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr != NULL);
  std::string ip_address = ctr->get_host_address();

  url.append("/portmap");
  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr , user_name, password);
  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);
  restjson::RestClient rest_client_obj(ip_address, url,
                                       odc_port, restjson::HTTP_METHOD_GET);

  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out,
      NULL);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return NULL;
  }
  resp_code = response->code;
  pfc_log_debug("Response code from Ctlfor portmap read : %d ", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    rest_client_obj.clear_http_response();
    return NULL;
  }

  json_object *jobj = NULL;
  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data = response->write_data->memory;
      jobj =  unc::restjson::JsonBuildParse::get_json_object(data);
      rest_client_obj.clear_http_response();
      return jobj;
    }
  }
  rest_client_obj.clear_http_response();
  return NULL;
}

drv_resp_code_t OdcVbrIfCommand::fill_config_node_vector(std::string vtn_name,
                                  std::string vbr_name, json_object *json_obj,
               uint32_t arr_idx, std::string url, unc::driver::controller* ctr,
                std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_vbr_if_t key_vbr_if;
  pfcdrv_val_vbr_if_t val_vbr_if;
  memset(&key_vbr_if, 0, sizeof(key_vbr_if_t));
  memset(&val_vbr_if, 0, sizeof(pfcdrv_val_vbr_if_t));

  std::string name = "";
  std::string description = "";
  std::string entity_state = "";
  uint32_t ret_val = unc::restjson::JsonBuildParse::parse(json_obj,
                                                          "name",
                                                          arr_idx, name);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("JsonBuildParse::parse fail.");
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("vbr_if name: %s", name.c_str());
  if (strlen(name.c_str()) == 0) {
    pfc_log_error("NO vbr_if name");
    return DRVAPI_RESPONSE_FAILURE;
  }

  ret_val = unc::restjson::JsonBuildParse::parse(json_obj,
                                                 "description",
                                                 arr_idx, description);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("vbr_if description: %s", description.c_str());
  //  Fills the vbrif KEY structure
  strncpy(reinterpret_cast<char*> (key_vbr_if.vbr_key.vtn_key.vtn_name),
          vtn_name.c_str(), sizeof(key_vbr_if.vbr_key.vtn_key.vtn_name) - 1);
  pfc_log_debug(" vtn name in vbrif:%s", reinterpret_cast<char*>
                (key_vbr_if.vbr_key.vtn_key.vtn_name));
  strncpy(reinterpret_cast<char*> (key_vbr_if.vbr_key.vbridge_name),
          vbr_name.c_str(), sizeof(key_vbr_if.vbr_key.vbridge_name) - 1);
  pfc_log_error(" vbr name in vbrif:%s",
                reinterpret_cast<char*> (key_vbr_if.vbr_key.vbridge_name));

  strncpy(reinterpret_cast<char*> (key_vbr_if.if_name), name.c_str(),
          sizeof(key_vbr_if.if_name) - 1);
  pfc_log_debug(" vbrif name:%s",
                reinterpret_cast<char*> (key_vbr_if.if_name));

  //  Fills vbrif VAL structure
  val_vbr_if.valid[PFCDRV_IDX_VAL_VBRIF] = UNC_VF_VALID;

  if (0 == strlen(description.c_str())) {
    val_vbr_if.val_vbrif.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
  } else {
    val_vbr_if.val_vbrif.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_VALID;
    strncpy(reinterpret_cast<char*> (val_vbr_if.val_vbrif.description),
            description.c_str(), sizeof(val_vbr_if.val_vbrif.description) - 1);
  }
  std::string admin_status = "";
  ret_val = unc::restjson::JsonBuildParse::parse(json_obj,
                                                 "enabled",
                                                 arr_idx, admin_status);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  val_vbr_if.val_vbrif.valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
  if (admin_status.compare("true") == 0) {
    val_vbr_if.val_vbrif.admin_status = UPLL_ADMIN_ENABLE;
  } else if (admin_status.compare("false") == 0) {
    val_vbr_if.val_vbrif.admin_status = UPLL_ADMIN_DISABLE;
  }

  url.append("/");
  url.append(name);
  int resp_code = 0;
  json_object *jobj = read_portmap(ctr, url, resp_code);
  if (0 == resp_code) {
    pfc_log_error("Error while reading portmap");
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("Response code from Ctl for portmap read : %d ", resp_code);
  if (HTTP_204_NO_CONTENT == resp_code) {
    pfc_log_debug("No Content for portmap received");
    val_vbr_if.val_vbrif.valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
  } else if (HTTP_200_RESP_OK == resp_code) {
    pfc_log_debug("json response for read_port_map : %s:",
                  unc::restjson::JsonBuildParse::get_string(jobj));
    if (json_object_is_type(jobj, json_type_null)) {
      pfc_log_error("null jobj no portmap");
      return DRVAPI_RESPONSE_FAILURE;
    } else {
      val_vbr_if.val_vbrif.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
      std::string vlanid = "0";
      ret_val = unc::restjson::JsonBuildParse::parse(jobj, "vlan", -1,
                                                     vlanid);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        json_object_put(jobj);
        pfc_log_debug("vlan parse error");
        return DRVAPI_RESPONSE_FAILURE;
      }
      pfc_log_debug("vlan id in portmap read %s", vlanid.c_str());
      if (0 == atoi(vlanid.c_str())) {
        val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] =
                                                   UNC_VF_INVALID;
        pfc_log_debug("untagged");
        val_vbr_if.val_vbrif.portmap.tagged = UPLL_VLAN_UNTAGGED;
        val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      } else {
        pfc_log_debug("vlan id valid");
        val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
        val_vbr_if.val_vbrif.portmap.vlan_id = atoi(vlanid.c_str());
        pfc_log_debug("%s  vlan id ", vlanid.c_str());
        pfc_log_debug("vlan id tagged");
        val_vbr_if.val_vbrif.portmap.tagged = UPLL_VLAN_TAGGED;
        val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      }
      json_object *jobj_node = NULL;
      json_object *jobj_port = NULL;
      ret_val = unc::restjson::JsonBuildParse::parse(jobj, "node", -1,
                                                     jobj_node);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_debug("node is null");
        json_object_put(jobj);
        return DRVAPI_RESPONSE_FAILURE;
      }
      ret_val = unc::restjson::JsonBuildParse::parse(jobj, "port",
                                                     -1, jobj_port);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        json_object_put(jobj);
        pfc_log_debug("Parsing error in port");
        return DRVAPI_RESPONSE_FAILURE;
      }
      std::string node_id = "";
      std::string port_name = "";
      std::string logical_port = "PP-";

      if ((json_object_is_type(jobj_node, json_type_null)) ||
          (json_object_is_type(jobj_port, json_type_null))) {
        pfc_log_error("node or port json object is null");
        return DRVAPI_RESPONSE_FAILURE;
      }

      ret_val = unc::restjson::JsonBuildParse::parse(jobj_node, "id",
                                                     -1, node_id);
      if (ret_val) {
        json_object_put(jobj);
        pfc_log_debug("id parse error");
        return DRVAPI_RESPONSE_FAILURE;
      }
      ret_val = unc::restjson::JsonBuildParse::parse(jobj_port, "name",
                                                     -1, port_name);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        json_object_put(jobj);
        pfc_log_debug("name parse error");
        return DRVAPI_RESPONSE_FAILURE;
      }
      logical_port.append(node_id);
      logical_port.append("-");
      logical_port.append(port_name);
      pfc_log_debug("logical port id %s", logical_port.c_str());
      strncpy(reinterpret_cast<char*>
              (val_vbr_if.val_vbrif.portmap.logical_port_id),
              logical_port.c_str(),
              sizeof(val_vbr_if.val_vbrif.portmap.logical_port_id) - 1);
      pfc_log_debug("%s logical port id in readportmap " ,
                    reinterpret_cast<char*>
                    (val_vbr_if.val_vbrif.portmap.logical_port_id));
      val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
          UNC_VF_VALID;


      json_object_put(jobj);
    }
  } else {
    pfc_log_error("Error in response code while reading port map:%d",
                  resp_code);
    return DRVAPI_RESPONSE_FAILURE;
  }

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vbr_if_t,
                                             pfcdrv_val_vbr_if_t,
                                             uint32_t> (&key_vbr_if,
                                             &val_vbr_if,
                                             uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  return DRVAPI_RESPONSE_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc

