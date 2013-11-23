/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vbr_vlanmap.hh>

namespace unc {
namespace odcdriver {
// Constructor
OdcVbrVlanMapCommand::OdcVbrVlanMapCommand() {
  ODC_FUNC_TRACE;
}

// Destructor
OdcVbrVlanMapCommand::~OdcVbrVlanMapCommand() {
  ODC_FUNC_TRACE;
}

// fetch child configurations for the parent kt(vbr)
drv_resp_code_t OdcVbrVlanMapCommand::fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector <unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  return get_vbrvlanmap_list(parent_key, ctr, cfgnode_vector);
}

// Validates VLAN exists or not
drv_resp_code_t OdcVbrVlanMapCommand::validate_vlan_exist(
                                              key_vlan_map_t &key_vlan_map,
                                              val_vlan_map_t &val_vlan_map,
                                              unc::driver::controller *ctr,
                                               pfc_bool_t &is_switch_exist,
                                                    std::string &port_id) {
  ODC_FUNC_TRACE;
  if (key_vlan_map.logical_port_id_valid != 0) {
    // Validate request received with SwitchID
    drv_resp_code_t ret_val =
        check_switch_already_exists(key_vlan_map,
                                val_vlan_map, ctr, is_switch_exist, port_id);
    return ret_val;
  } else {
    // Validate request received without SwitchID
    drv_resp_code_t  ret_val = check_ANY_already_exists
        (key_vlan_map, val_vlan_map, ctr, is_switch_exist, port_id);
      return ret_val;
  }
}

// Check  "ANY" vlan-id (vlan-id not associated with any switch)
// exists in controller
drv_resp_code_t
OdcVbrVlanMapCommand::check_ANY_already_exists(key_vlan_map_t &key_vlan_map,
                                              val_vlan_map_t  &val_vlan_map,
                                              unc::driver::controller *ctr,
                                              pfc_bool_t &is_switch_exist,
                                              std::string &port_id) {
  ODC_FUNC_TRACE;
  uint vlan_id_req = val_vlan_map.vlan_id;
  std::string vtn_name_req =
      reinterpret_cast<char*>(key_vlan_map.vbr_key.vtn_key.vtn_name);
  std::string vbr_name_req =
      reinterpret_cast<char*>(key_vlan_map.vbr_key.vbridge_name);
  std::string switch_id_req = NODE_TYPE_ANY;
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  PFC_ASSERT(odc_ctr != NULL);
  std::vector<std::string> vtn_vbr_vlan_vector = odc_ctr->vlan_vector;

  // If vlan-id is received as 0xFFFF from UPLL,its translated as 0
  // to check with controller
  if (vlan_id_req == 0xFFFF) {
    vlan_id_req = 0;
  }

  for (std::vector<std::string>::iterator it = vtn_vbr_vlan_vector.begin();
       it != vtn_vbr_vlan_vector.end(); it++) {
    std::string vtn_vbr_vlan_data = *it;
    size_t vlan_occurence = vtn_vbr_vlan_data.find_last_of(PERIOD);
    if (vlan_occurence == std::string::npos) {
      pfc_log_error("%s: Error in parsing data", PFC_FUNCNAME);
      return DRVAPI_RESPONSE_FAILURE;
    }
    std::string vlan_id = vtn_vbr_vlan_data.substr(vlan_occurence+1);
    uint vlan_id_ctr = atoi(vlan_id.c_str());
    std::string vtn_name_ctr = "";
    std::string vbr_name_ctr = "";
    std::string switch_id_ctr = "";

    parse_data_from_vector(vtn_vbr_vlan_data, vtn_name_ctr,
                           vbr_name_ctr, switch_id_ctr);
    if ((vtn_name_ctr.empty()) ||
        (vbr_name_ctr.empty()) ||
        (switch_id_ctr.empty())) {
      pfc_log_error("Error in parsing values ");
      return DRVAPI_RESPONSE_FAILURE;
    }

    if (vlan_id_req != vlan_id_ctr) {
      if ((vtn_name_req.compare(vtn_name_ctr) == 0) &&
          (vbr_name_req.compare(vbr_name_ctr) == 0) &&
          (switch_id_ctr.compare(NODE_TYPE_ANY) == 0)) {
            pfc_log_debug("%s: Request Received & Controller Node - ANY NODE."
                          "VLANID is different for same VTN & VBR",
                          PFC_FUNCNAME);
            is_switch_exist = PFC_TRUE;
            port_id.append(NODE_TYPE_ANY);
            port_id.append(PERIOD);
            std::ostringstream val_id_str_format;
            val_id_str_format << vlan_id_ctr;
            port_id.append(val_id_str_format.str());
          } else {
            pfc_log_debug("%s: Request Received - ANY Node."
                          "Controller Node - SWID Present."
                          "VLANID is different for same VTN & VBR",
                          PFC_FUNCNAME);
          }
    } else {
      pfc_log_error("%s: VLANID Conflict for Node type ANY!!!", PFC_FUNCNAME);
      return DRVAPI_RESPONSE_FAILURE;
    }
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

// Check if switch id exists in controller
drv_resp_code_t OdcVbrVlanMapCommand::check_switch_already_exists(
    key_vlan_map_t &key_vlan_map,
    val_vlan_map_t &val_vlan_map,
    unc::driver::controller *ctr_ptr,
    pfc_bool_t &is_switch_exist,
    std::string &port_id) {
  ODC_FUNC_TRACE;
  uint vlan_id_req = val_vlan_map.vlan_id;
  std::string vtn_name_req = reinterpret_cast<char*>
      (key_vlan_map.vbr_key.vtn_key.vtn_name);
  std::string vbr_name_req = reinterpret_cast<char*>
      (key_vlan_map.vbr_key.vbridge_name);

  // If vlan-id is received as 0xFFFF from UPLL,its translated as 0
  // to check with controller
  if (vlan_id_req == 0xFFFF) {
    vlan_id_req = 0;
  }

  std::string switch_id_req = "";
  std::string logical_port_req = NODE_TYPE_OF;
  switch_id_req = reinterpret_cast<char*>(key_vlan_map.logical_port_id);
  // logical_port_id received in key structure should be prefixed with SW-
  if (switch_id_req.compare(0, 3, SW_PREFIX) == 0) {
    logical_port_req.append(switch_id_req.substr(3));
  } else {
    pfc_log_error("%s: Request logical port is not in the format of SW-",
                  PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }

  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr_ptr);
  PFC_ASSERT(odc_ctr != NULL);
  std::vector<std::string> vtn_vbr_vlan_vector = odc_ctr->vlan_vector;
  for (std::vector<std::string>::iterator it = vtn_vbr_vlan_vector.begin();
       it != vtn_vbr_vlan_vector.end(); it++) {
    std::string vtn_vbr_vlan_data = *it;
    size_t vlan_occurence = vtn_vbr_vlan_data.find_last_of(PERIOD);
    if (vlan_occurence == std::string::npos) {
      pfc_log_error("%s: Error in parsing vlan-id", PFC_FUNCNAME);
      return DRVAPI_RESPONSE_FAILURE;
    }
    std::string vlan_id = vtn_vbr_vlan_data.substr(vlan_occurence+1);
    uint vlan_id_ctr = atoi(vlan_id.c_str());
    std::string vtn_name_ctr = "";
    std::string vbr_name_ctr = "";
    std::string switch_id_ctr = "";
    parse_data_from_vector(vtn_vbr_vlan_data, vtn_name_ctr, vbr_name_ctr,
                           switch_id_ctr);
    if ( (vtn_name_ctr.empty()) ||
         (vbr_name_ctr.empty()) ||
         (switch_id_ctr.empty())) {
      pfc_log_error("%s: Error in parsing values", PFC_FUNCNAME);
      return DRVAPI_RESPONSE_FAILURE;
    }

    if (vlan_id_req == vlan_id_ctr) {
      if (vtn_name_req.compare(vtn_name_ctr) == 0) {
        if (vbr_name_req.compare(vbr_name_ctr) == 0) {
          if (switch_id_ctr == NODE_TYPE_ANY) {
            pfc_log_error("%s:VLAN ID Conflict for SWID in Request & "
                              "ANY in Controller %s",
                              PFC_FUNCNAME, switch_id_ctr.c_str());
            return DRVAPI_RESPONSE_FAILURE;
          } else if (logical_port_req.compare(switch_id_ctr) == 0) {
            pfc_log_error("%s: VLANID Conflict for SWID in Request & "
                          "Controller (%s)!!!",
                          PFC_FUNCNAME, switch_id_ctr.c_str());
            return DRVAPI_RESPONSE_FAILURE;
          }
        } else {
          pfc_log_error("%s:VLAN id Conflict in different vbridges "
                        "under same VTN", PFC_FUNCNAME);
          return DRVAPI_RESPONSE_FAILURE;
        }
      } else {
        pfc_log_error("%s:VLAN id Conflict in different "
                      "vbridges under different VTN", PFC_FUNCNAME);
        return DRVAPI_RESPONSE_FAILURE;
      }
    } else {
      if ((vtn_name_req.compare(vtn_name_ctr) == 0) &&
          (vbr_name_req.compare(vbr_name_ctr) == 0) &&
          (logical_port_req.compare(switch_id_ctr) == 0)) {
        pfc_log_debug("%s:VLAN id is different for same SWID "
                      "in same vbridges/VTN", PFC_FUNCNAME);
        is_switch_exist = PFC_TRUE;
        port_id.append(switch_id_ctr);
        port_id.append(PERIOD);
        std::ostringstream val_id_str_format;
        val_id_str_format << vlan_id_ctr;
        port_id.append(val_id_str_format.str());
      }
    }
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

// parses the vector and gets vtn, vbr, sw id as out param
void OdcVbrVlanMapCommand::parse_data_from_vector(
                            const std::string &vtn_vbr_vlan_data,
                            std::string &vtn_name_ctr,
                            std::string &vbr_name_ctr,
                            std::string &switch_id_ctr) {
  ODC_FUNC_TRACE;
  std::string vtn_vbr_sw_data = vtn_vbr_vlan_data;
  std::size_t dot_occurence = vtn_vbr_sw_data.find(PERIOD);
  uint32_t flag = 0;
  while (dot_occurence != std::string::npos) {
    std::string parse_data  = vtn_vbr_sw_data.substr(0, dot_occurence);
    if (flag == VTN_PARSE_FLAG) {
      vtn_name_ctr = parse_data;
      pfc_log_debug("%s: VTN name (%s)", PFC_FUNCNAME, vtn_name_ctr.c_str());
    } else if (flag == VBR_PARSE_FLAG) {
      vbr_name_ctr = parse_data;
      pfc_log_debug("%s: Vbr name (%s)", PFC_FUNCNAME, vbr_name_ctr.c_str());
    } else if (flag == SWID_PARSE_FLAG) {
      switch_id_ctr = parse_data;
      pfc_log_debug("%s: Switch id (%s)", PFC_FUNCNAME,
                    switch_id_ctr.c_str());
      break;
    }
    vtn_vbr_sw_data = vtn_vbr_sw_data.substr(dot_occurence+1);
    dot_occurence = vtn_vbr_sw_data.find(PERIOD);
    flag++;
  }
}

// Getting vbridge vlanmap if available
drv_resp_code_t OdcVbrVlanMapCommand::get_vbrvlanmap_list(void* parent_key,
                                                          unc::driver
                                                          ::controller* ctr,
                                                          std::vector
                                                          < unc::vtndrvcache
                                                          ::ConfigNode *>
                                                          &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_vbr_t* parent_vbr = reinterpret_cast<key_vbr_t*> (parent_key);
  PFC_ASSERT(parent_vbr != NULL);
  std::string vtn_name =
      reinterpret_cast<const char*> (parent_vbr->vtn_key.vtn_name);
  std::string vbr_name =
      reinterpret_cast<const char*> (parent_vbr->vbridge_name);

  PFC_ASSERT(ctr != NULL);
  if ((0 == strlen(vtn_name.c_str())) || (0 == strlen(vbr_name.c_str()))) {
    pfc_log_error("%s: Empty VTN/VBR name", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }

  // Construct URL to retrieve vlanmaps from controller
  // URL should be of the format "/controller/nb/v2/vtn/default/vtns/{vtnname}
  // /vbridges/{vbridgeName}/vlanmaps"
  std::string ipaddress = ctr->get_host_address();
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  url.append(vtn_name);
  url.append("/vbridges/");
  url.append(vbr_name);
  url.append("/vlanmaps");

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
    pfc_log_error("%s:Error Occured while getting http response", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("%s: Response code (%d) from Ctrl for get vbrvlanmap",
                PFC_FUNCNAME, resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("%s: Error Response from Controller", PFC_FUNCNAME);
    if (HTTP_404_NOT_FOUND == resp_code) {
      pfc_log_error("%s: Specified container/VTN/vBridge doesn't exist err:-%u",
                    PFC_FUNCNAME, resp_code);
    } else if (HTTP_503_SERVICE_UNAVAILABLE == resp_code) {
      pfc_log_error("%s:VTN Manager service not operating inside controller %u",
                    PFC_FUNCNAME, resp_code);
    }
    rest_client_obj.clear_http_response();
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data = response->write_data->memory;
      drv_resp_code_t parse_ret = DRVAPI_RESPONSE_FAILURE;
      // Parse the vlan-map GET response from controller
      parse_ret = parse_vbrvlanmap_response(parent_key, ctr, data,
                                            cfgnode_vector);
      rest_client_obj.clear_http_response();
      return parse_ret;
    }
  }
  rest_client_obj.clear_http_response();
  return DRVAPI_RESPONSE_FAILURE;
}

// Parse the VBR_VLANMAP data
drv_resp_code_t OdcVbrVlanMapCommand::parse_vbrvlanmap_response(
                                                            void *parent_key,
                                                unc::driver::controller* ctr,
                                                                  char *data,
               std::vector < unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  json_object* jobj = unc::restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("%s: json_object_is_null", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t array_length = 0;
  json_object *json_obj_vbrvlanmap = NULL;

  // Parse vlanmap from response,which may include 0 or more vlanmap
  // configurations
  uint32_t ret_val = unc::restjson::JsonBuildParse::parse(jobj, "vlanmap",
                                                      -1, json_obj_vbrvlanmap);
  if (json_object_is_type(json_obj_vbrvlanmap, json_type_null)) {
    pfc_log_error("%s: json vbrvlanmap is null", PFC_FUNCNAME);
    json_object_put(jobj);
    return DRVAPI_RESPONSE_FAILURE;
  }

  if (restjson::REST_OP_SUCCESS != ret_val) {
    json_object_put(jobj);
    pfc_log_error("%s: JsonBuildParse::parse fail", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  // If more than 0 vlanmap configurations are present,retrieve the array length
  if (json_object_is_type(json_obj_vbrvlanmap, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj,
                                                              "vlanmap");
  }

  drv_resp_code_t resp_code = DRVAPI_RESPONSE_FAILURE;

  // Loop for all vlanmap configurations and cache it
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    resp_code = fill_config_node_vector(parent_key, ctr, json_obj_vbrvlanmap,
                                        arr_idx, cfgnode_vector);
    if (DRVAPI_RESPONSE_FAILURE == resp_code) {
      json_object_put(jobj);
      pfc_log_error("%s: fill_config_node_vector failed", PFC_FUNCNAME);
      return DRVAPI_RESPONSE_FAILURE;
    }
  }
  json_object_put(jobj);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Parse VBR_VLANMAP and append it to confignode vector
drv_resp_code_t OdcVbrVlanMapCommand::fill_config_node_vector(void *parent_key,
                                                  unc::driver::controller* ctr,
                                                             json_object *jobj,
                                                              uint32_t arr_idx,
                std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_vlan_map_t key_vlan_map;
  val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(val_vlan_map_t));
  val_vlan_map.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  std::string vlan = "0";
  int ret_val = unc::restjson::JsonBuildParse::parse(jobj, "vlan", arr_idx,
                                                     vlan);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("%s: Parse error for vlanid", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("%s: Vlan id parsed %s", PFC_FUNCNAME, vlan.c_str());
  std::string id = "";
  ret_val = unc::restjson::JsonBuildParse::parse(jobj, "id", arr_idx,
                                                 id);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("%s: Parse error for id", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("%s: ID parsed %s", PFC_FUNCNAME, id.c_str());
  if (id.empty()) {
    pfc_log_error("%s: id is empty", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  val_vlan_map.vlan_id = atoi(vlan.c_str());
  if (val_vlan_map.vlan_id == 0) {
    val_vlan_map.vlan_id = 0xFFFF;
    pfc_log_debug("%s: Vlan id untagged set as %d", PFC_FUNCNAME,
                  val_vlan_map.vlan_id);
  }
  json_object *jobj_node = NULL;
  ret_val = unc::restjson::JsonBuildParse::parse(jobj, "node", arr_idx,
                                                 jobj_node);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("%s: Parse Error for node", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }

  if (json_object_is_type(jobj_node, json_type_null)) {
    pfc_log_error("%s: Node json object is null", PFC_FUNCNAME);
  } else  {
    std::string switch_id = SW_PREFIX;
    std::string node_id = "";
    ret_val = unc::restjson::JsonBuildParse::parse(jobj_node, "id",
                                                  -1, node_id);

    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("%s: Parse Error for ID", PFC_FUNCNAME);
      return DRVAPI_RESPONSE_FAILURE;
    }
    switch_id.append(node_id);
    strncpy(reinterpret_cast<char*>
            (key_vlan_map.logical_port_id),
            switch_id.c_str(),
            strlen(switch_id.c_str()));
    key_vlan_map.logical_port_id_valid = 1;
  }
  key_vbr_t* parent_vbr = reinterpret_cast<key_vbr_t*> (parent_key);
  PFC_ASSERT(parent_vbr != NULL);
  std::string parent_vtn_name =
      reinterpret_cast<const char*> (parent_vbr->vtn_key.vtn_name);
  std::string parent_vbr_name =
      reinterpret_cast<const char*> (parent_vbr->vbridge_name);

  if ((parent_vtn_name.empty()) || (parent_vbr_name.empty())) {
    pfc_log_error("%s: VTN/VBR name is empty", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  strncpy(reinterpret_cast<char*> (key_vlan_map.vbr_key.vtn_key.vtn_name),
          parent_vtn_name.c_str(), strlen(parent_vtn_name.c_str()));

  strncpy(reinterpret_cast<char*> (key_vlan_map.vbr_key.vbridge_name),
          parent_vbr_name.c_str(), strlen(parent_vbr_name.c_str()));

  // Cache the parsed vlanmap configurations
  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil
      <key_vlan_map_t, val_vlan_map_t, uint32_t>
      (&key_vlan_map, &val_vlan_map, uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);

  std::string vtn_vbr_vlan = generate_string_for_vector(parent_vtn_name,
                                                        parent_vbr_name, id);

  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  PFC_ASSERT(odc_ctr != NULL);
  std::vector<std::string> vtn_vbr_vlan_vector = odc_ctr->vlan_vector;
  pfc_bool_t is_data_exist = PFC_FALSE;
  for (std::vector<std::string>::iterator it = vtn_vbr_vlan_vector.begin();
       it != vtn_vbr_vlan_vector.end(); it++) {
    std::string vtn_vbr_vlan_data = *it;
    if (vtn_vbr_vlan_data.compare(vtn_vbr_vlan) == 0) {
      pfc_log_debug("%s: Entry (%s) already exist", PFC_FUNCNAME,
                    vtn_vbr_vlan_data.c_str());
      is_data_exist = PFC_TRUE;
    }
  }
  if (PFC_FALSE == is_data_exist) {
    odc_ctr->vlan_vector.push_back(vtn_vbr_vlan);
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

std::string OdcVbrVlanMapCommand::generate_string_for_vector(
    const std::string &vtn_name,
    const std::string &vbr_name,
    const std::string &vlan_id) {
  ODC_FUNC_TRACE;
  if ((vtn_name.empty()) ||
      (vbr_name.empty()) ||
      (vlan_id.empty())) {
      pfc_log_error("%s: VTN/VBR/Switch is empty ", PFC_FUNCNAME);
      return "";
  }
  std::string vtn_vbr_vlan = "";
  vtn_vbr_vlan.append(vtn_name);
  vtn_vbr_vlan.append(PERIOD);
  vtn_vbr_vlan.append(vbr_name);
  vtn_vbr_vlan.append(PERIOD);
  vtn_vbr_vlan.append(vlan_id);
  return vtn_vbr_vlan;
}

// Generate Mapping ID
std::string OdcVbrVlanMapCommand::generate_vlanmap_id(
    key_vlan_map_t& vlanmap_key,
    std::string str_vlanid) {
  ODC_FUNC_TRACE;

  // Mapid generated if LPID is set,will be of the form - OF-{SWID}.{VlanID}
  // Mapid generated if LPID is not set,will be of the form - ANY.{VlanID}

  if (str_vlanid.compare(UNTAGGED_VLANID) == 0) {
    str_vlanid = "0";
    pfc_log_debug("%s: Vlan id is 0XFFFF %s", PFC_FUNCNAME, str_vlanid.c_str());
  }
  std::string str_mapid = "";
  if (vlanmap_key.logical_port_id_valid != 0) {
    pfc_log_debug("%s: Logical_port_id is valid", PFC_FUNCNAME);
    std::string logical_port_id = reinterpret_cast<char*>
        (vlanmap_key.logical_port_id);
    std::string switch_id = "";
    if (0 != strlen(logical_port_id.c_str())) {
      switch_id = logical_port_id.substr(3, 23);
    }
    str_mapid.append(NODE_TYPE_OF);
    str_mapid.append(switch_id);
    str_mapid.append(PERIOD);
    str_mapid.append(str_vlanid);
  } else {
    pfc_log_debug("%s: Logical_port_id is Not set", PFC_FUNCNAME);
    str_mapid.append(NODE_TYPE_ANY);
    str_mapid.append(PERIOD);
    str_mapid.append(str_vlanid);
  }
  pfc_log_debug("%s: Generated MapId:[%s] ", PFC_FUNCNAME, str_mapid.c_str());
  return str_mapid;
}

// Create Command for vbr vlanmap
drv_resp_code_t OdcVbrVlanMapCommand::create_cmd(
    key_vlan_map_t& vlanmap_key,
    val_vlan_map_t& vlanmap_val,
    unc::driver::controller* ctr_ptr) {

  ODC_FUNC_TRACE;
  return create_update_cmd(vlanmap_key, vlanmap_val, ctr_ptr);
}

// Update Command for vbr vlanmap
drv_resp_code_t OdcVbrVlanMapCommand::update_cmd(
    key_vlan_map_t& vlanmap_key,
    val_vlan_map_t& vlanmap_val,
    unc::driver::controller* ctr_ptr) {

  ODC_FUNC_TRACE;
  return create_update_cmd(vlanmap_key, vlanmap_val, ctr_ptr);
}

// Delete vlan-map from controller
drv_resp_code_t OdcVbrVlanMapCommand::del_existing_vlanmap(
    key_vlan_map_t& vlanmap_key,
    unc::driver::controller*
    ctr_ptr, const std::string &str_mapping_id) {
  ODC_FUNC_TRACE;
  std::string vlanid = "";
  std::string del_vbr_vlanmap_url = get_vbrvlanmap_url(vlanmap_key);
  if (del_vbr_vlanmap_url.empty()) {
    pfc_log_error("%s: vlanmap url is empty", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (str_mapping_id.empty()) {
    pfc_log_error("%s: MapID received is empty", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }

  del_vbr_vlanmap_url.append(str_mapping_id);
  pfc_log_debug("%s: Final MapId url for delete %s", PFC_FUNCNAME,
                del_vbr_vlanmap_url.c_str());

  std::string ip_address = ctr_ptr->get_host_address();
  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr_ptr , user_name, password);

  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);

  restjson::RestClient rest_client_obj(ip_address, del_vbr_vlanmap_url,
                                       odc_port, restjson::HTTP_METHOD_DELETE);

  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out, NULL);

  if (NULL == response) {
    pfc_log_error("%s: Error Occured while getting httpresponse", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("%s: Response code from Ctl for delete vlanmap (%d) ",
                PFC_FUNCNAME, resp_code);

  rest_client_obj.clear_http_response();

  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("%s: Delete vlanmap is not successful %d",
                  PFC_FUNCNAME, resp_code);
    return DRVAPI_RESPONSE_FAILURE;
  }

  std::string vtn_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name);
  std::string vbr_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name);
  std::string vtn_vbr_vlan_delete = generate_string_for_vector(vtn_name_req,
                                                vbr_name_req, str_mapping_id);
  if (vtn_vbr_vlan_delete.empty()) {
    pfc_log_debug("vtn/vbr/id is empty");
    return DRVAPI_RESPONSE_FAILURE;
  }


  // Entry from search vector is removed
  delete_from_vector(ctr_ptr, vtn_vbr_vlan_delete);

  return DRVAPI_RESPONSE_SUCCESS;
}

// Create or Update Command for vbr vlanmap
drv_resp_code_t OdcVbrVlanMapCommand::create_update_cmd(
    key_vlan_map_t& vlanmap_key,
    val_vlan_map_t& vlanmap_val,
    unc::driver::controller* ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  std::string ip_address = ctr_ptr->get_host_address();
  pfc_bool_t vlan_map_exists = PFC_FALSE;
  std::string strmapid = "";
  if (vlanmap_key.logical_port_id_valid != 0) {
    pfc_log_debug("%s: Logical_port_id is valid", PFC_FUNCNAME);
    std::string logical_port_id = reinterpret_cast<char*>
        (vlanmap_key.logical_port_id);
    odc_drv_resp_code_t ret_val = validate_logical_port_id(logical_port_id);
    if (ret_val != ODC_DRV_SUCCESS) {
      pfc_log_error("%s: Validation for logical_port[%s] failed ",
                    PFC_FUNCNAME, logical_port_id.c_str());
      return DRVAPI_RESPONSE_FAILURE;
    }
  }
  // Validate if vlanid already exists in controller
  drv_resp_code_t validate_resp = validate_vlan_exist(vlanmap_key,
                                                      vlanmap_val,
                                                      ctr_ptr,
                                                      vlan_map_exists,
                                                      strmapid);

  if (validate_resp != DRVAPI_RESPONSE_SUCCESS) {
    pfc_log_error("%s Validation of vlanmap failed, validate_resp(%u)",
                  PFC_FUNCNAME, validate_resp);
    return DRVAPI_RESPONSE_FAILURE;
  }
  // If vlanid exists for same SwitchID/ANY delete the existing vlanmap
  // and create a new vlanmap
  if (vlan_map_exists == PFC_TRUE) {
    if (del_existing_vlanmap(vlanmap_key, ctr_ptr, strmapid) !=
        DRVAPI_RESPONSE_SUCCESS) {
      pfc_log_error("%s Delete of vlanmap failed", PFC_FUNCNAME);
      return DRVAPI_RESPONSE_FAILURE;
    }
  }

  std::string vbr_vlanmap_url = get_vbrvlanmap_url(vlanmap_key);
  pfc_log_debug("%s: Vlanmap url %s ", PFC_FUNCNAME, vbr_vlanmap_url.c_str());
  if (vbr_vlanmap_url.empty()) {
    pfc_log_error("%s: Vlanmap url is empty", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  unc::restjson::JsonBuildParse json_obj;
  json_object* vbrvlanmap_json_request_body = create_request_body(
      vlanmap_key, vlanmap_val);
  const char* str_vlanmap_reqbody = json_obj.get_string(
      vbrvlanmap_json_request_body);
  pfc_log_debug("%s: Request body for vlanmap: %s ", PFC_FUNCNAME,
                str_vlanmap_reqbody);

  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr_ptr , user_name, password);
  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);

  restjson::RestClient rest_client_obj(ip_address, vbr_vlanmap_url,
                                       odc_port, restjson::HTTP_METHOD_POST);
  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out,
      str_vlanmap_reqbody);
  json_object_put(vbrvlanmap_json_request_body);
  if (NULL == response) {
    pfc_log_error("%s: Error Occured while getting httpresponse",
                  PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("%s: Response code from Ctl for vlanmap create_cmd: %d",
                PFC_FUNCNAME, resp_code);
  rest_client_obj.clear_http_response();
  if (HTTP_201_RESP_CREATED != resp_code) {
    pfc_log_error("%s: Create/Update Vlanmap Failure.Response code %d",
                  PFC_FUNCNAME, resp_code);
    json_object_put(vbrvlanmap_json_request_body);
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::string vtn_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name);
  std::string vbr_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name);

  uint vlanid =  vlanmap_val.vlan_id;
  std::ostringstream convert_vlanid;
  convert_vlanid << vlanid;
  std::string map_id = generate_vlanmap_id(vlanmap_key, convert_vlanid.str());
  std::string vtn_vbr_vlan_update = generate_string_for_vector(vtn_name_req,
                          vbr_name_req, map_id);
  if (vtn_vbr_vlan_update.empty()) {
    pfc_log_error("vtn/vbr/switch id is empty in %s", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  update_vector(ctr_ptr,  vtn_vbr_vlan_update);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Deletes particular entry from vlan-map vector
void OdcVbrVlanMapCommand::delete_from_vector(unc::driver::controller *ctr ,
                                              std::string vtn_vbr_vlan) {
  ODC_FUNC_TRACE;
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  PFC_ASSERT(odc_ctr != NULL);
  for (uint iterator = 0; iterator < odc_ctr->vlan_vector.size(); iterator++) {
    if (odc_ctr->vlan_vector.at(iterator).compare(vtn_vbr_vlan) == 0) {
      odc_ctr->vlan_vector.erase(odc_ctr->vlan_vector.begin()+iterator);
    }
  }
}

// Updates particular entry in vector
void OdcVbrVlanMapCommand::update_vector(unc::driver::controller *ctr,
                                          std::string vtn_vbr_vlan) {
  ODC_FUNC_TRACE;
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  PFC_ASSERT(odc_ctr != NULL);
  pfc_bool_t is_data_exist = PFC_FALSE;
  std::vector<std::string> vtn_vbr_vlan_vector = odc_ctr->vlan_vector;
  for (std::vector<std::string>::iterator it = vtn_vbr_vlan_vector.begin();
       it != vtn_vbr_vlan_vector.end(); it++) {
    std::string vtn_vbr_vlan_data = *it;
    if (vtn_vbr_vlan_data.compare(vtn_vbr_vlan) == 0) {
      pfc_log_debug("%s: Vlan-map Already exist", PFC_FUNCNAME);
      is_data_exist = PFC_TRUE;
    }
  }
  if (PFC_FALSE == is_data_exist) {
    odc_ctr->vlan_vector.push_back(vtn_vbr_vlan);
  }
}

// Generates vlan id from the vlan-map vector
std::string
OdcVbrVlanMapCommand::generate_vlanid_from_vector(unc::driver::controller *ctr,
                                                  key_vlan_map_t &vlanmap_key) {
  ODC_FUNC_TRACE;
  std::string vtn_name_req = reinterpret_cast<char*>
      (vlanmap_key.vbr_key.vtn_key.vtn_name);
  std::string vbr_name_req = reinterpret_cast<char*>
      (vlanmap_key.vbr_key.vbridge_name);
  std::string logical_portid_req = reinterpret_cast<char*>
      (vlanmap_key.logical_port_id);

  if ((0 == strlen(vtn_name_req.c_str())) ||
      (0 == strlen(vbr_name_req.c_str()))) {
    pfc_log_error("%s: Empty VTN/VBR name", PFC_FUNCNAME);
    return "";
  }

  std::string vtn_vbr_vlan_req = "";
  vtn_vbr_vlan_req.append(vtn_name_req);
  vtn_vbr_vlan_req.append(PERIOD);
  vtn_vbr_vlan_req.append(vbr_name_req);
  vtn_vbr_vlan_req.append(PERIOD);
  if (vlanmap_key.logical_port_id_valid != 0) {
    if (logical_portid_req.compare(0, 3, SW_PREFIX) == 0) {
      logical_portid_req.erase(0, 2);
    } else {
      return "";
    }
    vtn_vbr_vlan_req.append("OF");
    vtn_vbr_vlan_req.append(logical_portid_req);
  } else {
    vtn_vbr_vlan_req.append(NODE_TYPE_ANY);
  }
  pfc_log_debug("%s: Search string formed from VTN,VBR,VlanID %s",
                PFC_FUNCNAME, vtn_vbr_vlan_req.c_str());

  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  PFC_ASSERT(odc_ctr != NULL);

  // Each Entry in vector will be of the form vtnname.vbrname.mapid
  // Retrive vlanid from the string
  std::vector<std::string> vtn_vbr_vlan_vector = odc_ctr->vlan_vector;
  for (std::vector<std::string>::iterator it
       = vtn_vbr_vlan_vector.begin();
       it != vtn_vbr_vlan_vector.end(); it++) {
    std::string vtn_vbr_vlan_data = *it;
    size_t occurence = vtn_vbr_vlan_data.find_last_of(PERIOD);
    std::string vtn_vbr_vlan_ctr = vtn_vbr_vlan_data.substr(0, occurence);
    if (vtn_vbr_vlan_req.compare(vtn_vbr_vlan_ctr) == 0) {
      return vtn_vbr_vlan_data.substr(occurence+1);
    }
  }
  return "";
}

// Delete Command for vbr vlan-map
drv_resp_code_t OdcVbrVlanMapCommand::delete_cmd(key_vlan_map_t&
                                                 vlanmap_key,
                                                 val_vlan_map_t&
                                                 vlanmap_val,
                                                 unc::driver
                                                 ::controller*
                                                 ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  key_vbr_t parent_vbr;
  std::string del_vlanid;

  memcpy(parent_vbr.vtn_key.vtn_name, vlanmap_key.vbr_key.vtn_key.vtn_name,
         sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name));
  memcpy(parent_vbr.vbridge_name, vlanmap_key.vbr_key.vbridge_name,
         sizeof(vlanmap_key.vbr_key.vbridge_name));

  std::string str_vlan_id = generate_vlanid_from_vector(ctr_ptr, vlanmap_key);
  if (str_vlan_id.empty()) {
    pfc_log_error("%s: Empty vlan id returned", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::string logical_portid_req = reinterpret_cast<char*>
      (vlanmap_key.logical_port_id);
  std::string str_mapping_id = "";
  if (vlanmap_key.logical_port_id_valid != 0) {
    if (logical_portid_req.compare(0, 3, SW_PREFIX) == 0) {
      logical_portid_req.erase(0, 2);
    }
    std::string logical_port_OF = "OF";
    logical_port_OF.append(logical_portid_req);
    str_mapping_id.append(logical_port_OF);
  } else {
    str_mapping_id.append(NODE_TYPE_ANY);
  }
  str_mapping_id.append(PERIOD);
  str_mapping_id.append(str_vlan_id);
  pfc_log_debug("%s: Mapping id formed in delete %s", PFC_FUNCNAME,
                str_mapping_id.c_str());

  // Delete URL to be sent to controller will be of the form "/controller/nb/v2/
  // vtn/default/vtns/{VTNName}/vbridges/{VBRName}/vlanmaps/{mapId}"
  std::string vbr_vlanmap_url = get_vbrvlanmap_url(vlanmap_key);

  if (vbr_vlanmap_url.empty()) {
    pfc_log_error("%s: vlanmap url is empty", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  vbr_vlanmap_url.append(str_mapping_id);
  pfc_log_debug("%s: Vlanmap delete URL :%s", PFC_FUNCNAME,
                vbr_vlanmap_url.c_str());
  std::string ip_address = ctr_ptr->get_host_address();
  std::string user_name = "";
  std::string password = "";
  get_username_password(ctr_ptr , user_name, password);

  uint32_t odc_port = 0;
  uint32_t connect_time_out = 0;
  uint32_t req_time_out = 0;
  read_conf_file(odc_port, connect_time_out, req_time_out);

  restjson::RestClient rest_client_obj(ip_address, vbr_vlanmap_url,
                                       odc_port, restjson::HTTP_METHOD_DELETE);
  unc::restjson::HttpResponse_t* response = rest_client_obj.send_http_request(
      user_name, password, connect_time_out, req_time_out,
      NULL);
  if (NULL == response) {
    pfc_log_error("%s: Error Occured while getting httpresponse", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  int resp_code = response->code;
  pfc_log_debug("%s: Response code from Ctrl for delete vlanmap(%d)",
                PFC_FUNCNAME, resp_code);
  rest_client_obj.clear_http_response();
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("%s: Delete of vlanmap Failed.Response Code %d",
                  PFC_FUNCNAME, resp_code);
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::string vtn_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name);
  std::string vbr_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name);

  std::string vtn_vbr_vlan_delete = generate_string_for_vector(vtn_name_req,
                                               vbr_name_req, str_mapping_id);
  if (vtn_vbr_vlan_delete.empty()) {
    pfc_log_error("vtn/vbr/switch id is empty in %s", PFC_FUNCNAME);
    return DRVAPI_RESPONSE_FAILURE;
  }
  delete_from_vector(ctr_ptr, vtn_vbr_vlan_delete);
  return DRVAPI_RESPONSE_SUCCESS;
}

// Creates request body for vbr vlanmap
json_object* OdcVbrVlanMapCommand::create_request_body(
    key_vlan_map_t& vlanmap_key, val_vlan_map_t& vlanmap_val) {
  ODC_FUNC_TRACE;
  unc::restjson::JsonBuildParse json_obj;
  json_object *jobj_parent = json_obj.create_json_obj();
  json_object *jobj_node = json_obj.create_json_obj();
  uint32_t ret_val = 1;
  std::string vlanid;

  if (UNC_VF_VALID == vlanmap_val.valid[UPLL_IDX_VLAN_ID_VM]) {
    if (vlanmap_val.vlan_id == 0xFFFF) {
      pfc_log_debug("%s: Vlan Untagged ", PFC_FUNCNAME);
      vlanid = "0";
    } else {
      pfc_log_debug("%s: Vlan Tagged ", PFC_FUNCNAME);
      std::ostringstream convert_vlanid;
      convert_vlanid << vlanmap_val.vlan_id;
      vlanid.append(convert_vlanid.str());
    }
    pfc_log_debug("%s: Vlanid: %s", PFC_FUNCNAME, vlanid.c_str());
    if (!vlanid.empty()) {
      ret_val = json_obj.build("vlan", vlanid, jobj_parent);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("%s: Error in building vlanid in vlanmap", PFC_FUNCNAME);
        json_object_put(jobj_parent);
        json_object_put(jobj_node);
        return NULL;
      }
    }
  }

  if (vlanmap_key.logical_port_id_valid != 0) {
    ret_val = json_obj.build("type", "OF", jobj_node);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("%s: Error in building type in vlanmap", PFC_FUNCNAME);
      json_object_put(jobj_parent);
      json_object_put(jobj_node);
      return NULL;
    }

    std::string logical_port_id = reinterpret_cast<char*>
        (vlanmap_key.logical_port_id);
    std::string switch_id = "";
    if (0 != strlen(logical_port_id.c_str())) {
      switch_id = logical_port_id.substr(3, 23);
      pfc_log_debug("%s: Logical_port_id(%s) ", PFC_FUNCNAME,
                    logical_port_id.c_str());
      pfc_log_debug("%s: Switch id(%s) ", PFC_FUNCNAME,
                    switch_id.c_str());
      ret_val = json_obj.build("id", switch_id, jobj_node);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("%s: Error in building SwitchId in vlanmap",
                      PFC_FUNCNAME);
        json_object_put(jobj_parent);
        json_object_put(jobj_node);
        return NULL;
      }
    }

    ret_val = json_obj.build("node", jobj_node, jobj_parent);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("%s: Error in building node in vlanmap", PFC_FUNCNAME);
      json_object_put(jobj_parent);
      json_object_put(jobj_node);
      return NULL;
    }
  }
  return jobj_parent;
}

// Form URL for vbr vlanmap to send request to controller
std::string OdcVbrVlanMapCommand::get_vbrvlanmap_url(
                                             key_vlan_map_t& vbr_vlanmap_key) {
  ODC_FUNC_TRACE;
  char* vtnname = NULL;
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append("/");
  vtnname = reinterpret_cast<char*>(vbr_vlanmap_key.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("%s: VTN name is empty", PFC_FUNCNAME);
    return "";
  }
  url.append(vtnname);

  char* vbrname =
      reinterpret_cast<char*>(vbr_vlanmap_key.vbr_key.vbridge_name);
  if (0 == strlen(vbrname)) {
    pfc_log_error("%s: VBR name is empty", PFC_FUNCNAME);
    return "";
  }
  url.append("/vbridges/");
  url.append(vbrname);

  url.append("/vlanmaps/");
  return url;
}

// Validates the format of logical port id received from UPLL
odc_drv_resp_code_t OdcVbrVlanMapCommand::validate_logical_port_id(
    const std::string& logical_port_id) {
  ODC_FUNC_TRACE;
  pfc_log_debug("%s: Logical port received : %s", PFC_FUNCNAME,
                logical_port_id.c_str());
  if (logical_port_id.compare(0, 3, SW_PREFIX) != 0) {
    pfc_log_error("%s: Logical_port_id doesn't have SW- prefix", PFC_FUNCNAME);
    return ODC_DRV_FAILURE;
  }
  if ((logical_port_id.size() != 26) ||
      (logical_port_id.compare(5, 1, ":") != 0) ||
      (logical_port_id.compare(8, 1, ":") !=0) ||
      (logical_port_id.compare(11, 1, ":") != 0) ||
      (logical_port_id.compare(14, 1, ":") !=0) ||
      (logical_port_id.compare(17, 1, ":") !=0) ||
      (logical_port_id.compare(20, 1, ":") !=0) ||
      (logical_port_id.compare(23, 1, ":") !=0)) {
    pfc_log_error("%s: Invalid logical_port_id", PFC_FUNCNAME);
    return ODC_DRV_FAILURE;
  }
  pfc_log_debug("%s: Valid logical_port id", PFC_FUNCNAME);
  return ODC_DRV_SUCCESS;
}

// Gets username password form controller or else conf file
void OdcVbrVlanMapCommand::get_username_password(unc::driver
                                                 ::controller*
                                                 ctr_ptr,
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

// Reads username password from conf file else from default value
void OdcVbrVlanMapCommand::read_user_name_password(std::string &user_name,
                                                   std::string &password) {
  ODC_FUNC_TRACE;
  pfc::core::ModuleConfBlock set_user_password_blk(SET_USER_PASSWORD_BLK);
  if (set_user_password_blk.getBlock() != PFC_CFBLK_INVALID) {
    user_name = set_user_password_blk.getString(
        CONF_USER_NAME, DEFAULT_USER_NAME.c_str());

    password  = set_user_password_blk.getString(
        CONF_PASSWORD, DEFAULT_PASSWORD.c_str());
    pfc_log_debug("%s: Block Handle is Valid, user_name_ %s", PFC_FUNCNAME,
                  user_name.c_str());
  }  else {
    user_name = DEFAULT_USER_NAME;
    password  = DEFAULT_PASSWORD;
    pfc_log_debug("%s: Block Handle is InValid, get default user_name%s",
                  PFC_FUNCNAME, user_name.c_str());
  }
}

// Reads set opt params from conf file if it fails reads from defs file
void OdcVbrVlanMapCommand::read_conf_file(uint32_t &odc_port,
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
    pfc_log_debug("%s: Block Handle is Invalid, set default Value %d",
                  PFC_FUNCNAME, odc_port);
  }
}

}  // namespace odcdriver
}  // namespace unc
