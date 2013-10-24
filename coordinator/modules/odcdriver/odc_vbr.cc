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

#include <odc_vbr.hh>
#include <string>

namespace unc {
namespace odcdriver {

// Constructor
ODCVBRCommand::ODCVBRCommand()
    : age_interval_("600") {
      pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
      pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
    }

// Destructor
ODCVBRCommand::~ODCVBRCommand() {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
}

// Create Command and send Request to Controller
drv_resp_code_t ODCVBRCommand::create_cmd(key_vbr_t& key_vbr,
                                          val_vbr_t& val_vbr,
                                          unc::driver::controller
                                          *ctr_ptr) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string vbr_url = get_vbr_url(key_vbr);
  if (0 == strlen(vbr_url.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  const char* request = create_request_body(val_vbr);
  uint32_t resp = get_controller_response_code(vbr_url, ctr_ptr,
                                               unc::restjson
                                               ::HTTP_METHOD_POST,
                                               request);
  pfc_log_debug("resp_code for create vbr is %d", resp);
  if (RESP_CREATED != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

//  Command to update vtn  and Send request to Controller
drv_resp_code_t ODCVBRCommand::update_cmd(key_vbr_t& key_vbr,
                                          val_vbr_t& val_vbr,
                                          unc::driver::controller
                                          *ctr_ptr) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string vbr_url = get_vbr_url(key_vbr);
  if (0 == strlen(vbr_url.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  const char* request = create_request_body(val_vbr);
  uint32_t resp = get_controller_response_code(vbr_url, ctr_ptr,
                                               unc::restjson
                                               ::HTTP_METHOD_PUT,
                                               request);
  if (RESP_OK != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("resp_code for update vbr is %d", resp);
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Delete Request send to the Controller
drv_resp_code_t ODCVBRCommand::delete_cmd(key_vbr_t& key_vbr,
                                          val_vbr_t& val_vbr,
                                          unc::driver::controller
                                          *ctr_ptr) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  std::string vbr_url = get_vbr_url(key_vbr);
  pfc_log_debug("vbr_url:%s", vbr_url.c_str());
  if (0 == strlen(vbr_url.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t resp = get_controller_response_code(vbr_url, ctr_ptr,
                                               unc::restjson
                                               ::HTTP_METHOD_DELETE,
                                               NULL);
  if (RESP_OK != resp) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Creates Request Body
const char* ODCVBRCommand::create_request_body(const val_vbr_t& val_vbr) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  unc::restjson::JsonBuildParse json_obj;
  json_object *jobj = json_obj.create_json_obj();
  const char* description = reinterpret_cast<const char*>
      (val_vbr.vbr_description);
  uint32_t ret_val = 1;
  if (0 != strlen(description)) {
    ret_val = json_obj.build(jobj, "description", description);
    if (ret_val) {
      return NULL;
    }
  }
  ret_val = json_obj.build(jobj, "ageInterval", age_interval_);
  if (ret_val) {
    return NULL;
  }
  const char* req_body = json_obj.json_obj_to_json_string(jobj);
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return req_body;
}

// validates the operation
drv_resp_code_t ODCVBRCommand::validate_op(key_vbr_t& key_vbr,
                                           val_vbr_t& val_vbr,
                                           unc::driver::controller*
                                           ctr,
                                           uint32_t operation) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  drv_resp_code_t resp_code = DRVAPI_RESPONSE_FAILURE;
  switch (operation) {
    case UNC_OP_CREATE:
      resp_code = validate_create_vbr(key_vbr, ctr);
      break;
    case UNC_OP_UPDATE:
      resp_code = validate_update_vbr(key_vbr, ctr);
      break;
    case UNC_OP_DELETE:
      resp_code = validate_delete_vbr(key_vbr, ctr);
      break;
    default:
      pfc_log_debug("Unknown operation received %d", operation);
      break;
  }
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return resp_code;
}

// Validates create vbr
drv_resp_code_t ODCVBRCommand::validate_create_vbr(key_vbr_t& key_vbr,
                                                   unc::driver::controller*
                                                   ctr) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  uint32_t resp_code = ODC_DRV_FAILURE;
  resp_code = is_vtn_exists_in_controller(key_vbr, ctr);
  if (RESP_NOT_FOUND == resp_code) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (RESP_OK == resp_code) {
    resp_code = is_vbr_exists_in_controller(key_vbr, ctr);
    if (RESP_NOT_FOUND == resp_code) {
      return DRVAPI_RESPONSE_SUCCESS;
    }
  }
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_FAILURE;
}

// Validate delete vbridge
drv_resp_code_t ODCVBRCommand::validate_delete_vbr(key_vbr_t& key_vbr,
                                                   unc::driver::controller*
                                                   ctr) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  uint32_t vbr_resp_code = ODC_DRV_FAILURE;
  vbr_resp_code = is_vbr_exists_in_controller(key_vbr, ctr);
  if (RESP_OK == vbr_resp_code) {
    pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_SUCCESS;
  }
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_FAILURE;
}

// Validates update vbridge
drv_resp_code_t ODCVBRCommand::validate_update_vbr(key_vbr_t& key_vbr,
                                                   unc::driver::controller*
                                                   ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  uint32_t vbr_resp_code = ODC_DRV_FAILURE;
  vbr_resp_code = is_vbr_exists_in_controller(key_vbr, ctr);
  if (RESP_OK == vbr_resp_code) {
    pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_SUCCESS;
  }
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_FAILURE;
}

// Checks the exists in controlle ror not
uint32_t ODCVBRCommand::is_vtn_exists_in_controller(const key_vbr_t& key_vbr,
                                                    unc::driver::controller*
                                                    ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  const char* vtnname = NULL;
  vtnname = reinterpret_cast<const char*>(key_vbr.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    return ODC_DRV_FAILURE;
  }
  std::string url = "";
  url.append(VTN_URL);
  url.append("/");
  url.append(vtnname);
  uint32_t response_code = get_controller_response_code(url, ctr,
                                                        unc::restjson
                                                        ::HTTP_METHOD_GET,
                                                        NULL);
  pfc_log_debug("Response code from Controller is : %d ", response_code);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return response_code;
}

// Checks vbr exists in controller or not
uint32_t ODCVBRCommand::is_vbr_exists_in_controller(key_vbr_t& key_vbr,
                                                    unc::driver::controller*
                                                    ctr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  std::string vbr_url = get_vbr_url(key_vbr);
  if (0 == strlen(vbr_url.c_str())) {
    return ODC_DRV_FAILURE;
  }
  uint32_t response_code = get_controller_response_code(vbr_url, ctr,
                                                        unc::restjson
                                                        ::HTTP_METHOD_GET,
                                                        NULL);
  pfc_log_debug("Response code from Controller is : %d ", response_code);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return response_code;
}

// Gets the Controller Response
uint32_t ODCVBRCommand::get_controller_response_code(std::string url,
                                                     unc::driver::controller*
                                                     ctr,
                                                     unc::restjson
                                                     ::HttpMethod method,
                                                     const char* request_body) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);

  PFC_ASSERT(ctr != NULL);
  std::string ipaddress = ctr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return ODC_DRV_FAILURE;
  }
  if (( method != unc::restjson::HTTP_METHOD_POST ) &&
      (method != unc::restjson::HTTP_METHOD_PUT) &&
      (method != unc::restjson::HTTP_METHOD_DELETE) &&
      (method != unc::restjson::HTTP_METHOD_GET)) {
    pfc_log_debug("Invalid Method : %d", method);
    return ODC_DRV_FAILURE;
  }
  std::string username_ctr = ctr->get_user_name();
  std::string password_ctr = ctr->get_pass_word();

  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  uint32_t retval = ODC_DRV_FAILURE;
  if ((0 == strlen(username_ctr.c_str()))
      || (0 == strlen(password_ctr.c_str()))) {
    retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  } else {
    retval = rest_util_obj.set_username_password(username_ctr, password_ctr);
  }
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.set_timeout(CONNECTION_TIME_OUT, REQUEST_TIME_OUT);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.create_request_header(url, method);
  if (ODC_DRV_FAILURE == retval) {
    pfc_log_debug("Failure in create request header : %d", retval);
    return retval;
  }
  if ((method == unc::restjson::HTTP_METHOD_POST) ||
      (method == unc::restjson::HTTP_METHOD_PUT)) {
    if (NULL != request_body) {
      pfc_log_debug("Request Body  : %s", request_body);
      retval = rest_util_obj.set_request_body(request_body);
      if (ODC_DRV_FAILURE == retval) {
        pfc_log_debug("Failure in set Request Body %d", retval);
        return DRVAPI_RESPONSE_FAILURE;
      }
    }
  }
  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  pfc_log_debug("Response code from Controller is : %d ", resp);
  pfc_log_debug("Exiting function ... %s", PFC_FUNCNAME);
  return resp;
}

drv_resp_code_t ODCVBRCommand::parse_vbrif_append_vector(std::string vtn_name,
                                                  std::string vbr_name,
                                                  json_object *json_obj,
                                                  uint32_t arr_idx,
                                                  std::string url,
                                                  unc::driver
                                                  ::controller* ctr,
                                                  std::vector< unc::vtndrvcache
                                                  ::ConfigNode *>
                                                  &cfgnode_vector) {
  pfc_log_debug("%s Enter function", PFC_FUNCNAME);
  key_vbr_if_t key_vbr_if;
  pfcdrv_val_vbr_if_t val_vbr_if;
  memset(&key_vbr_if, 0, sizeof(key_vbr_if_t));
  memset(&val_vbr_if, 0, sizeof(pfcdrv_val_vbr_if_t));

  std::string name = "";
  std::string description = "";
  std::string entity_state = "";
  uint32_t ret_val = unc::restjson::JsonBuildParse::parse(json_obj,
                                                     "name", arr_idx, name);
  if (ret_val) {
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
  if (ret_val) {
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
  url.append("/");
  url.append(name);
  val_vbr_if.val_vbrif.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  json_object *jobj = read_portmap(ctr, url);
  pfc_log_debug("json response %s:",
                unc::restjson::JsonBuildParse::json_obj_to_json_string(jobj));
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_debug("null jobj no portmap");
  } else {
    std::string vlanid = "0";
    ret_val = unc::restjson::JsonBuildParse::parse(jobj, "vlan", -1,
                                                   vlanid);
    if (ret_val) {
      pfc_log_debug("vlan parse error");
      return DRVAPI_RESPONSE_FAILURE;
    }
    pfc_log_debug("vlan id in portmap read %s", vlanid.c_str());
    if (0 == atoi(vlanid.c_str())) {
      val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
      pfc_log_debug("untagged");
      val_vbr_if.val_vbrif.portmap.tagged = UPLL_VLAN_UNTAGGED;
      val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
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
    pfc_log_debug("node");
    if (ret_val) {
      pfc_log_debug("node is null");
      return DRVAPI_RESPONSE_FAILURE;
    }
    ret_val = unc::restjson::JsonBuildParse::parse(jobj, "port",
                                                   -1, jobj_port);
    if (ret_val) {
      pfc_log_debug("port is null");
      return DRVAPI_RESPONSE_FAILURE;
    }
    std::string node_id = "";
    std::string port_name = "";
    std::string logical_port = "PP-";

    if ((!json_object_is_type(jobj_node, json_type_null)) &&
        (!json_object_is_type(jobj_port, json_type_null))) {
      ret_val = unc::restjson::JsonBuildParse::parse(jobj_node, "id",
                                                     -1, node_id);
      if (ret_val) {
        pfc_log_debug("id parse error");
        return DRVAPI_RESPONSE_FAILURE;
      }
      ret_val = unc::restjson::JsonBuildParse::parse(jobj_port, "name",
                                                     -1, port_name);
      if (ret_val) {
        pfc_log_debug("name parse error");
        return DRVAPI_RESPONSE_FAILURE;
      }
      if ((0 == strlen(node_id.c_str())) || (0 == strlen(port_name.c_str()))) {
        pfc_log_debug("port or node is 0");
        val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
            UNC_VF_INVALID;
      } else {
        logical_port.append(node_id);
        logical_port.append("-");
        logical_port.append(port_name);
        strncpy(reinterpret_cast<char*>
                (val_vbr_if.val_vbrif.portmap.logical_port_id),
                logical_port.c_str(),
                sizeof(val_vbr_if.val_vbrif.portmap.logical_port_id) - 1);
        pfc_log_debug("%s logical port id in readportmap " ,
                      reinterpret_cast<char*>
                      (val_vbr_if.val_vbrif.portmap.logical_port_id));
        val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
            UNC_VF_VALID;
      }
    } else {
      val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
          UNC_VF_INVALID;
    }
  }

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil
      <key_vbr_if_t, pfcdrv_val_vbr_if_t, uint32_t> (
          &key_vbr_if,
          &val_vbr_if,
          uint32_t(UNC_OP_READ));

  cfgnode_vector.push_back(cfgptr);
  pfc_log_debug("%s Exit function", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Getting  vbridge child if available
drv_resp_code_t ODCVBRCommand::get_vbr_child(std::string vtn_name,
                                             std::string vbr_name,
                                             unc::driver::controller* ctr,
                                             std::vector< unc::vtndrvcache
                                             ::ConfigNode *>
                                             &cfgnode_vector) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  if ((0 == strlen(vtn_name.c_str())) || (0 == strlen(vbr_name.c_str()))) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  std::string url = "";
  url.append(VTN_URL);
  url.append("/");
  url.append(vtn_name);
  url.append("/vbridges/");
  url.append(vbr_name);
  url.append("/interfaces");

  PFC_ASSERT(ctr != NULL);
  std::string ipaddress = ctr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::string username_ctr = ctr->get_user_name();
  std::string password_ctr = ctr->get_pass_word();

  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  uint32_t retval = ODC_DRV_FAILURE;
  if ((0 == strlen(username_ctr.c_str()))
      || (0 == strlen(password_ctr.c_str()))) {
    retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  } else {
    retval = rest_util_obj.set_username_password(username_ctr, password_ctr);
  }
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }
  retval = rest_util_obj.set_timeout(CONNECTION_TIME_OUT, REQUEST_TIME_OUT);
  if (ODC_DRV_FAILURE == retval) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  retval = rest_util_obj.create_request_header(url,
                                               unc::restjson::HTTP_METHOD_GET);
  if (ODC_DRV_FAILURE == retval) {
    pfc_log_error("rest_util_obj.create_request_header fail.");
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  if (RESP_OK != resp) {
    pfc_log_error("rest_util_obj.send_request_and_get_response_code fail");
    return DRVAPI_RESPONSE_FAILURE;
  }
  unc::restjson::HttpContent_t* content = rest_util_obj.get_response_body();
  if (content == NULL) {
    pfc_log_error("content is null");
    return DRVAPI_RESPONSE_FAILURE;
  }
  char *data = content->memory;
  pfc_log_debug("data-:%s", data);
  drv_resp_code_t parse_ret = DRVAPI_RESPONSE_FAILURE;
  parse_ret = parse_vbrif_resp_data(vtn_name, vbr_name, url, ctr, data,
                                    cfgnode_vector);
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return parse_ret;
}
drv_resp_code_t ODCVBRCommand::parse_vbrif_resp_data(std::string vtn_name,
                                                   std::string vbr_name,
                                                   std::string url,
                                                   unc::driver::controller*
                                                   ctr,
                                                   char *data,
                                                   std::vector< unc::vtndrvcache
                                                   ::ConfigNode *>
                                                   &cfgnode_vector) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  json_object* jobj = unc::restjson::JsonBuildParse::
      string_to_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_type fail");
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t array_length = 0;
  json_object *json_obj_vbrif = NULL;

  uint32_t ret_val = unc::restjson::JsonBuildParse::parse(jobj, "interface",
                                                     -1, json_obj_vbrif);
  if (ret_val) {
    pfc_log_error("JsonBuildParse::parse fail");
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (json_object_is_type(json_obj_vbrif, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj,
                                                              "interface");
  }

  pfc_log_debug("interface array_length:%d", array_length);

  if (json_object_is_type(json_obj_vbrif, json_type_null)) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  drv_resp_code_t resp_code = DRVAPI_RESPONSE_FAILURE;
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    pfc_log_debug("inside array_length for loop");
    resp_code = parse_vbrif_append_vector(vtn_name, vbr_name, json_obj_vbrif,
                                          arr_idx, url, ctr, cfgnode_vector);
    if (DRVAPI_RESPONSE_FAILURE == resp_code) {
      return DRVAPI_RESPONSE_FAILURE;
    }
  }
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Reading port-map from active controller
json_object* ODCVBRCommand::read_portmap(unc::driver::controller* ctr,
                                         std::string url) {
  pfc_log_debug("%s Entering Function ... ", PFC_FUNCNAME);
  url.append("/portmap");

  json_object* jobj = NULL;
  PFC_ASSERT(ctr != NULL);
  std::string ipaddress = ctr->get_host_address();
  if (0 == strlen(ipaddress.c_str())) {
    return jobj;
  }
  std::string username_ctr = ctr->get_user_name();
  std::string password_ctr = ctr->get_pass_word();

  unc::restjson::RestClient rest_util_obj(ipaddress, ODC_PORT);
  uint32_t retval = ODC_DRV_FAILURE;
  if ((0 == strlen(username_ctr.c_str()))
      || (0 == strlen(password_ctr.c_str()))) {
    retval = rest_util_obj.set_username_password(USER_NAME, PASS_WORD);
  } else {
    retval = rest_util_obj.set_username_password(username_ctr, password_ctr);
  }
  if (ODC_DRV_FAILURE == retval) {
    return jobj;
  }
  retval = rest_util_obj.set_timeout(CONNECTION_TIME_OUT, REQUEST_TIME_OUT);
  if (ODC_DRV_FAILURE == retval) {
    return jobj;
  }
  retval = rest_util_obj.create_request_header(url,
                                               unc::restjson::HTTP_METHOD_GET);
  if (ODC_DRV_FAILURE == retval) {
    return jobj;
  }
  uint32_t resp = rest_util_obj.send_request_and_get_response_code();
  if (RESP_OK != resp) {
    return jobj;
  }
  unc::restjson::HttpContent_t* content = rest_util_obj.get_response_body();
  if (content == NULL) {
    return jobj;
  }
  char *data = content->memory;
  jobj =  unc::restjson::JsonBuildParse::string_to_json_object(data);
  pfc_log_debug("%s Exiting Function ... ", PFC_FUNCNAME);
  return jobj;
}

// Constructing URL for vbridge,inject request to controller
std::string ODCVBRCommand::get_vbr_url(key_vbr_t& key_vbr) {
  pfc_log_debug("%s Entering function", PFC_FUNCNAME);
  char* vtnname = NULL;
  char* vbrname = NULL;
  std::string url = "";
  url.append(VTN_URL);
  url.append("/");
  vtnname = reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    return "";
  }
  url.append(vtnname);
  vbrname = reinterpret_cast<char*>(key_vbr.vbridge_name);
  if (0 == strlen(vbrname)) {
    return "";
  }
  url.append("/vbridges/");
  url.append(vbrname);
  pfc_log_debug("%s Exiting function", PFC_FUNCNAME);
  return url;
}
}  // namespace odcdriver
}  // namespace unc
