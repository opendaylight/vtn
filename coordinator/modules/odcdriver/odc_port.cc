/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_port.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcPort::OdcPort(unc::restjson::ConfFileValues_t conf_values)
    : conf_file_values_(conf_values) {
      ODC_FUNC_TRACE;
    }

// Destructor
OdcPort::~OdcPort() {
  link_map_.clear();
  pfc_log_info("odcport link_map cleared");
}


// Get logical port information in the controller
UncRespCode OdcPort::fetch_config(
    unc::driver::controller *ctr_ptr,
    key_switch_t *parent_switch,
    const pfc_bool_t cache_empty) {
  ODC_FUNC_TRACE;

  PFC_VERIFY(ctr_ptr != NULL);

  if (NULL == parent_switch) {
    pfc_log_error("parent_switch is NULL");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::vector<unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  std::string switch_id = reinterpret_cast<const char*>
      (parent_switch->switch_id);

  if (switch_id.empty()) {
    pfc_log_error("Switch id is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string url = "";
  url.append(BASE_SW_URL);
  url.append(CONTAINER_NAME);
  url.append(NODE);
  url.append(NODE_OF);
  url.append(SLASH);
  url.append(switch_id);

  pfc_log_trace("Url for port %s", url.c_str());
  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                                        ctr_ptr->get_user_name(),
                                        ctr_ptr->get_pass_word());

  unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(
          url, restjson::HTTP_METHOD_GET, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  int resp_code = response->code;
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("Response code is not OK , resp : %d", resp_code);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data = response->write_data->memory;
      pfc_log_trace("All nodes : %s", data);
      UncRespCode ret_val =
          parse_port_response(ctr_ptr, data, cfgnode_vector);
      pfc_log_debug("Number of Ports in Switch %s is, %d", switch_id.c_str(),
                    static_cast<int>(cfgnode_vector.size()));
      if (UNC_RC_SUCCESS != ret_val) {
        pfc_log_error("Error while parsing the port response");
        return ret_val;
      }
      // compare with cahe
      ret_val = compare_with_cache(ctr_ptr,
                                   cfgnode_vector, switch_id, cache_empty);
      pfc_log_debug("Response from compare_with_cache is %d", ret_val);
      return ret_val;
    }
  }
  pfc_log_error("Response in NULL");
  return UNC_DRV_RC_ERR_GENERIC;
}

// parse each port and append to cache
UncRespCode OdcPort::fill_config_node_vector(
    unc::driver::controller *ctr_ptr,
    json_object *json_obj_node_conn_prop,
    int arr_idx,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_port_t key_port;
  val_port_st_t val_port;
  memset(&key_port, 0, sizeof(key_port_t));
  memset(&val_port, 0, sizeof(val_port_st_t));

  json_object *json_obj_node_conn = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn_prop,
                                                     "nodeconnector",
                                                     arr_idx,
                                                     json_obj_node_conn);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_conn, json_type_null))) {
    pfc_log_error(" Error while parsing node");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  std::string node_conn_id = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn,
                                            "id",
                                            -1,
                                            node_conn_id);
  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (node_conn_id.empty())) {
    pfc_log_error(" Error while parsing node_conn_id");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  json_object *json_obj_node = NULL;
  std::string node_type = "";
  std::string node_id   = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn,
                                            "node",
                                            -1,
                                            json_obj_node);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node, json_type_null))) {
    pfc_log_error(" Error while parsing port or json obj is NULL");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn,
                                            "type",
                                            -1,
                                            node_type);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (node_type.empty())) {
    pfc_log_error(" Error while parsing node_type");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if (node_type.compare("SW") == 0) {
    pfc_log_debug("SW type is received");
    return UNC_RC_SUCCESS;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_node, "id",
                                            -1, node_id);
  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (node_id.empty())) {
    pfc_log_error(" Error while parsing node id");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  std::string name_value = "";
  uint state_value = 0;
  uint config_value = 0;
  unsigned long long speed = 0;
  ret_val = parse_port_properties_value(arr_idx,
                                        json_obj_node_conn_prop,
                                        name_value,
                                        state_value,
                                        config_value,
                                        speed);

  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error in  parsing node conn properties");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_port.port_id), name_value.c_str(),
          strlen(name_value.c_str()));

  strncpy(reinterpret_cast<char*> (key_port.sw_key.switch_id), node_id.c_str(),
          strlen(node_id.c_str()));

  std::string ctr_name = ctr_ptr->get_controller_id();
  strncpy(reinterpret_cast<char*> (key_port.sw_key.ctr_key.controller_name),
          ctr_name.c_str(), strlen(ctr_name.c_str()));

  // Fill Value Structure
  val_port.valid[VAL_PORT_STRUCT_ATTR] = UNC_VF_VALID;

  std::string port_id = "PP-OF:";
  port_id.append(node_id);
  port_id.append(HYPHEN);
  port_id.append(name_value);
  pfc_log_debug("Port id formed in logical port : %s", port_id.c_str());
  // Fill link map
  std::string head_conn = node_id;
  std::string port_value = name_value;
  port_value.append(PIPE_SEPARATOR);
  std::ostringstream str_state_val;
  str_state_val << state_value;
  port_value.append(str_state_val.str());
  port_value.append(PIPE_SEPARATOR);
  std::ostringstream str_conf_val;
  str_conf_val << config_value;
  port_value.append(str_conf_val.str());
  head_conn.append(PIPE_SEPARATOR);
  head_conn.append(node_conn_id);
  link_map_[head_conn] = port_value;

  pfc_log_debug("link details framed %s | %s", head_conn.c_str(),
                port_value.c_str());
  strncpy(reinterpret_cast<char*> (val_port.logical_port_id), port_id.c_str(),
          strlen(port_id.c_str()));
  val_port.valid[kIdxPortLogicalPortId] = UNC_VF_VALID;

  if (ADMIN_UP == config_value) {
    val_port.port.admin_status = UPPL_PORT_ADMIN_UP;
    val_port.port.valid[VAL_PORT_EVENT_ATTR3] = UNC_VF_VALID;
  } else if (ADMIN_DOWN == config_value)  {
    val_port.port.admin_status = UPPL_PORT_ADMIN_DOWN;
    val_port.port.valid[VAL_PORT_EVENT_ATTR3] = UNC_VF_VALID;
  }

  val_port.port.port_number = atoi(node_conn_id.c_str());
  val_port.port.valid[VAL_PORT_EVENT_ATTR1] = UNC_VF_VALID;

  if (unc::driver::CONNECTION_UP == ctr_ptr->get_connection_status()) {
    if ((ADMIN_UP == config_value) &&
        (EDGE_UP == state_value)) {
      val_port.oper_status = UPPL_PORT_OPER_UP;
      val_port.valid[VAL_PORT_EVENT_ATTR2] = UNC_VF_VALID;
    } else {
      val_port.oper_status = UPPL_PORT_OPER_DOWN;
      val_port.valid[VAL_PORT_EVENT_ATTR2] = UNC_VF_VALID;
    }
  } else {
    val_port.oper_status = UPPL_PORT_OPER_UNKNOWN;
    val_port.valid[VAL_PORT_EVENT_ATTR2] = UNC_VF_VALID;
  }
  pfc_log_debug("oper status of port is %d", val_port.oper_status);

  val_port.speed = speed;
  val_port.valid[VAL_PORT_EVENT_ATTR6] = UNC_VF_VALID;

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_port_t,
          val_port_st_t, val_port_st_t,
          uint32_t>
      (&key_port, &val_port, &val_port, uint32_t(UNC_OP_READ));
  PFC_VERIFY(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  return UNC_RC_SUCCESS;
}

// parse the port properties vale
UncRespCode OdcPort::parse_port_properties_value(
    int arr_idx,
    json_object *json_obj_node_conn,
    std::string &name_value,
    uint &state_value,
    uint &config_value,
    unsigned long long &speed) {
  ODC_FUNC_TRACE;
  json_object *json_obj_node_prop = NULL;
  int ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn,
                                                "properties",
                                                arr_idx,
                                                json_obj_node_prop);
  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_prop, json_type_null))) {
    pfc_log_error(" Error while parsing node properties");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  json_object *json_obj_node_prop_name = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop,
                                            "name",
                                            -1,
                                            json_obj_node_prop_name);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_prop_name, json_type_null))) {
    pfc_log_error(" Error while parsing node name");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop_name,
                                            "value",
                                            -1,
                                            name_value);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (name_value.empty())) {
    pfc_log_error(" Error while parsing node name value");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  json_object *json_obj_node_prop_state = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop,
                                            "state",
                                            -1,
                                            json_obj_node_prop_state);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_prop_state, json_type_null))) {
    pfc_log_error(" Error while parsing node state properties");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop_state,
                                            "value",
                                            -1,
                                            state_value);

  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error while parsing node state value properties");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  json_object *json_obj_node_prop_config = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop,
                                            "config",
                                            -1,
                                            json_obj_node_prop_config);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_prop_config, json_type_null))) {
    pfc_log_error(" Error while parsing node config properties");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop_config,
                                            "value",
                                            -1,
                                            config_value);

  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error while parsing node config properties");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  json_object *json_obj_node_prop_speed = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop,
                                            "bandwidth",
                                            -1,
                                            json_obj_node_prop_speed);

  /*if ((restjson::REST_OP_SUCCESS != ret_val) ||
    (json_object_is_type(json_obj_node_prop_speed, json_type_null))) {
    pfc_log_error(" Error while parsing node bandwidth properties");
    return UNC_DRV_RC_ERR_GENERIC;
    }*/
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop_speed,
                                            "value",
                                            -1,
                                            speed);

  /*if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error while parsing node bandwidth properties");
    return UNC_DRV_RC_ERR_GENERIC;
    }*/

  return UNC_RC_SUCCESS;
}

//  Compare whether cache empty or not
UncRespCode OdcPort::compare_with_cache(
    unc::driver::controller *ctr_ptr,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
    const std::string &switch_id, const pfc_bool_t cache_empty) {
  ODC_FUNC_TRACE;
  if (NULL == ctr_ptr->physical_port_cache) {
    pfc_log_error("Cache is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::list<std::string> port_list;
  UncRespCode ret_val = UNC_DRV_RC_ERR_GENERIC;

  if (PFC_TRUE == cache_empty) {
    for (std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
         cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it) {
      unc::vtndrvcache::ConfigNode *cfgnode_cache = *it;
      if (NULL == cfgnode_cache) {
        pfc_log_error("cfgnode_cache is NULL");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      ret_val = add_event(ctr_ptr, cfgnode_cache, port_list);
      if (UNC_RC_SUCCESS != ret_val) {
        pfc_log_error("Error in adding to cache");
        return ret_val;
      }
    }
  } else {
    ret_val = verify_in_cache(ctr_ptr, cfgnode_vector, switch_id, port_list);
    return ret_val;
  }
  return UNC_RC_SUCCESS;
}

//  Notify physical about the port event
void OdcPort::notify_physical(
    unc::driver::oper_type type, key_port_t *key_port,
    val_port_st_t *val_port, val_port_st_t *val_old_port) {
  ODC_FUNC_TRACE;
  unc::driver::VtnDrvIntf* disp_inst = static_cast<unc::driver::VtnDrvIntf*>
      (pfc::core::Module::getInstance("vtndrvintf"));
  PFC_VERIFY(disp_inst != NULL);
  std::string port_id = reinterpret_cast<const char*> (key_port->port_id);

  pfc_log_debug("Port event received %d", type);
  if (type == unc::driver::VTN_PORT_UPDATE) {
    disp_inst->port_event(unc::driver::VTN_PORT_UPDATE, *key_port,
                          *val_port, *val_old_port);
  } else {
    disp_inst->port_event(type, *key_port, *val_port);
    notify_logical_port_physical(type, key_port, val_port, val_old_port);
  }
}

// Notify Logical port to physical
void OdcPort::notify_logical_port_physical(
    unc::driver::oper_type type,
    key_port_t *key_port,
    val_port_st_t *val_port, val_port_st_t *val_old_port) {
  ODC_FUNC_TRACE;

  unc::driver::VtnDrvIntf* disp_inst = static_cast<unc::driver::VtnDrvIntf*>
      (pfc::core::Module::getInstance("vtndrvintf"));

  PFC_VERIFY(disp_inst != NULL);

  // Fills logical port structure
  key_logical_port_t key_logical_port;
  val_logical_port_st_t val_logical_port;
  fill_logical_port_structure(key_port, val_port,
                              key_logical_port,
                              val_logical_port);

  // If PORT update request received fill old logical port structure
  if (type == unc::driver::VTN_PORT_CREATE) {
    disp_inst->logicalport_event(unc::driver::VTN_LP_CREATE,
                                 key_logical_port,
                                 val_logical_port);

  } else if ((type == unc::driver::VTN_PORT_DELETE) ||
             (type == unc::driver::VTN_LP_DELETE)) {
    disp_inst->logicalport_event(unc::driver::VTN_LP_DELETE, key_logical_port,
                                 val_logical_port);
  }
}

// fills logical port structure for CREATE/ DELETE
void OdcPort::fill_logical_port_structure(
    key_port_t *key_port,
    val_port_st_t *val_port,
    key_logical_port_t &key_logical_port,
    val_logical_port_st_t &val_logical_port) {
  ODC_FUNC_TRACE;
  memset(&key_logical_port, 0, sizeof(key_logical_port_t));
  memset(&val_logical_port, 0, sizeof(val_logical_port_st_t));

  const std::string logical_port_id =
      reinterpret_cast<const char*> (val_port->logical_port_id);

  //  Fills Key Structure
  strncpy(reinterpret_cast<char*>
          (key_logical_port.port_id), logical_port_id.c_str(),
          strlen(logical_port_id.c_str()));

  const std::string ctr_name =
      reinterpret_cast<const char*> (key_port->sw_key.ctr_key.controller_name);
  strncpy(reinterpret_cast<char*>
          (key_logical_port.domain_key.ctr_key.controller_name),
          ctr_name.c_str(), strlen(ctr_name.c_str()));

  strncpy(reinterpret_cast<char*> (key_logical_port.domain_key.domain_name),
          DOM_NAME.c_str(), strlen(DOM_NAME.c_str()));

  // Fills Val Structure
  fill_logical_port_val_structure(key_port, val_port,
                                  val_logical_port);
}

// Fils the logical port value structure
void OdcPort::fill_logical_port_val_structure(
    key_port_t *key_port,
    val_port_st_t *val_port,
    val_logical_port_st_t &val_logical_port) {
  ODC_FUNC_TRACE;
  memset(&val_logical_port, 0, sizeof(val_logical_port_st_t));

  val_logical_port.valid[VAL_LOGICAL_PORT_VALID]= UNC_VF_VALID;
  std::string switch_id = reinterpret_cast<const char*>
      (key_port->sw_key.switch_id);
  strncpy(reinterpret_cast<char*> (val_logical_port.logical_port.switch_id),
          switch_id.c_str(), strlen(switch_id.c_str()));

  val_logical_port.logical_port.valid[VAL_LOGICAL_PORT_SWITCHID] = UNC_VF_VALID;

  std::string port_id = reinterpret_cast<const char*> (key_port->port_id);
  strncpy(reinterpret_cast<char*>
          (val_logical_port.logical_port.physical_port_id),
          port_id.c_str(), strlen(port_id.c_str()));

  val_logical_port.logical_port.valid[VAL_LOGICAL_PORT_PHYPORTID]
      = UNC_VF_VALID;

  val_logical_port.logical_port.oper_down_criteria =
      UNC_OPERSTATUS_CRITERIA_ANY;
  val_logical_port.logical_port.valid[VAL_OPERSTATUS_CRITERIA]
      = UNC_VF_VALID;

  val_logical_port.oper_status = val_port->oper_status;
  if (val_port->valid[VAL_PORT_EVENT_ATTR2] == UNC_VF_VALID) {
    val_logical_port.valid[VAL_PORT_EVENT_ATTR2] =  UNC_VF_VALID;
  }
  val_logical_port.logical_port.port_type = UPPL_LP_SWITCH;
  val_logical_port.logical_port.valid[VAL_PORT_EVENT_ATTR2] = UNC_VF_VALID;
}

//  Update the local list maintained for delete
void OdcPort::update_list(key_port_t *key_port,
                          std::list<std::string> &port_list) {
  ODC_FUNC_TRACE;
  std::string switch_port_id = reinterpret_cast<const char*>
      (key_port->sw_key.switch_id);
  std::string port_id = reinterpret_cast<const char*> (key_port->port_id);
  switch_port_id.append(HYPHEN);
  switch_port_id.append(port_id);

  std::list <std::string>::iterator result = std::find(
      port_list.begin(),
      port_list.end(), switch_port_id);

  // Push into list only if the entry not exist already
  if (result == port_list.end()) {
    port_list.push_front(switch_port_id);
  }
}

//  Add event for Port
UncRespCode OdcPort::add_event(unc::driver::controller *ctr_ptr,
                               unc::vtndrvcache::ConfigNode *cfg_node,
                               std::list<std::string> &port_list) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, val_port_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_port_t, val_port_st_t, val_port_st_t, uint32_t>*> (cfg_node);

  key_port_t *key_port = cfgptr_cache->get_key_structure();
  val_port_st_t *val_port = cfgptr_cache->get_val_structure();

  if ((NULL == key_port) || (NULL == val_port)) {
    pfc_log_error("key_port/val_port is NULL");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  //  Send Notification to UPPL
  notify_physical(unc::driver::VTN_PORT_CREATE, key_port, val_port, NULL);

  //  Append to cache
  UncRespCode  ret_val =
      ctr_ptr->physical_port_cache->append_Physical_attribute_node(cfg_node);
  if (UNC_RC_SUCCESS != ret_val) {
    delete_config_node(cfg_node);
    pfc_log_error(" Error in adding to cache");
    return ret_val;
  }
  update_list(key_port, port_list);
  return ret_val;
}

// Update event for Port
UncRespCode OdcPort::update_event(unc::driver::controller *ctr_ptr,
                                  unc::vtndrvcache::ConfigNode *cfg_node,
                                  val_port_st_t *val_new_port,
                                  std::list<std::string> &port_list) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, val_port_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_port_t, val_port_st_t, val_port_st_t, uint32_t>*> (cfg_node);

  key_port_t *key_port = cfgptr_cache->get_key_structure();
  val_port_st_t *val_port = cfgptr_cache->get_val_structure();

  if ((NULL == key_port) || (NULL == val_port) || (NULL == val_new_port)) {
    pfc_log_error("key_port/val_port is NULL");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  // Send notification to UPPL
  notify_physical(unc::driver::VTN_PORT_UPDATE, key_port,
                  val_port, val_new_port);

  // Append to cache
  UncRespCode  ret_val =
      ctr_ptr->physical_port_cache->update_physical_attribute_node(cfg_node);
  if (UNC_RC_SUCCESS != ret_val) {
    delete_config_node(cfg_node);
    pfc_log_error(" Error in updating to cache");
    return ret_val;
  }

  update_list(key_port, port_list);
  return ret_val;
}

// Check Delete event for Port
UncRespCode OdcPort::delete_event(unc::driver::controller *ctr_ptr,
                                  const std::string &switch_id,
                                  std::list<std::string> &port_list) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  std::map<std::string, unc::vtndrvcache::ConfigNode *> cfg_node_delete_map;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr_ptr->physical_port_cache->create_iterator());

  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    if (NULL == cfgnode_cache) {
      pfc_log_error("cfgnode is NULL before get_type");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_PORT == key_type) {
      unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, val_port_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, val_port_st_t, uint32_t>*> (cfgnode_cache);

      key_port_t *key_port_cache = cfgptr_cache->get_key_structure();
      if (NULL == key_port_cache) {
        pfc_log_error("key_port_cache is NULL");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      std::string switch_id_cache = reinterpret_cast<const char*>
          (key_port_cache->sw_key.switch_id);
      std::string sw_port_id = "";
      std::string port_id = reinterpret_cast<const char*>
          (key_port_cache->port_id);

      sw_port_id.append(switch_id_cache);
      sw_port_id.append(HYPHEN);
      sw_port_id.append(port_id);

      if ((switch_id.compare(switch_id_cache) == 0) &&
          !(port_id.empty())) {
        std::list <std::string>::iterator result = std::find(port_list.begin(),
                                                             port_list.end(),
                                                             sw_port_id);
        if (result == port_list.end()) {
          pfc_log_debug("Port %s to be deleted", switch_id.c_str());
          cfg_node_delete_map[sw_port_id] = cfgnode_cache;
        }
      }
    }
  }
  return delete_port(ctr_ptr, cfg_node_delete_map);
}

UncRespCode OdcPort::delete_port(
    unc::driver::controller *ctr_ptr,
    const std::map<std::string,
    unc::vtndrvcache::ConfigNode *> &cfg_node_delete_map) {
  ODC_FUNC_TRACE;

  std::map<std::string, unc::vtndrvcache::ConfigNode *>::
      const_iterator iter;
  for (iter = cfg_node_delete_map.begin();
       iter != cfg_node_delete_map.end(); ++iter) {
    unc::vtndrvcache::ConfigNode *cfg_node = iter->second;
    if (NULL == cfg_node) {
      pfc_log_error("cfgnode is NULL before get_type");
      return UNC_DRV_RC_ERR_GENERIC;
    }

    unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, val_port_st_t, uint32_t>
        *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
        <key_port_t, val_port_st_t, val_port_st_t, uint32_t>*> (cfg_node);

    key_port_t *key_port = cfgptr_cache->get_key_structure();
    val_port_st_t *val_port = cfgptr_cache->get_val_structure();

    if ((NULL == key_port) || (NULL == val_port)) {
      pfc_log_error("key_port/val_port is NULL");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    // Send notification to UPPL
    notify_physical(unc::driver::VTN_PORT_DELETE, key_port, val_port, NULL);

    // Delete from cache
    UncRespCode  ret_val =
        ctr_ptr->physical_port_cache->delete_physical_attribute_node(cfg_node);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error(" Error in deleting in cache");
      return ret_val;
    }
  }
  return UNC_RC_SUCCESS;
}

// Deletes config node
void OdcPort::delete_config_node(unc::vtndrvcache::ConfigNode *cfg_node) {
  ODC_FUNC_TRACE;
  if (NULL != cfg_node) {
    delete cfg_node;
    cfg_node = NULL;
  }
}

//  Verify in cache whether port aleady exists or not
UncRespCode OdcPort::verify_in_cache(
    unc::driver::controller *ctr_ptr,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
    const std::string &switch_id,
    std::list<std::string> &port_list) {
  ODC_FUNC_TRACE;
  for (
      std::vector<unc::vtndrvcache::ConfigNode *>::iterator ctr_iterator =
      cfgnode_vector.begin();
      ctr_iterator != cfgnode_vector.end();
      ctr_iterator++) {
    unc::vtndrvcache::ConfigNode *cfg_node = *ctr_iterator;
    if (NULL == cfg_node) {
      pfc_log_error("cfg_node is NULL");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, val_port_st_t, uint32_t>
        *cfgnode_ctr = static_cast<unc::vtndrvcache::CacheElementUtil
        <key_port_t, val_port_st_t, val_port_st_t, uint32_t>*> (cfg_node);

    val_port_st_t *val_port_ctr = cfgnode_ctr->get_val_structure();
    if (NULL == val_port_ctr) {
      pfc_log_error("val_port_ctr is NULL");
      return UNC_DRV_RC_ERR_GENERIC;
    }

    std::string port_id_ctr = reinterpret_cast<const char*>
        (val_port_ctr->logical_port_id);

    pfc_log_debug("Switch id is %s , Port id is %s",
                  switch_id.c_str(), port_id_ctr.c_str());
    unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
    pfc_bool_t exist_in_cache =
        ctr_ptr->physical_port_cache->compare_is_physical_node_found(
            cfgnode_ctr, cfgnode_cache);

    if (PFC_FALSE == exist_in_cache) {
      pfc_log_trace("Port %s not exist in cache", port_id_ctr.c_str());
      UncRespCode  ret_val = add_event(ctr_ptr, cfgnode_ctr, port_list);
      if (ret_val != UNC_RC_SUCCESS) {
        pfc_log_error("Error occured in adding to cache");
        return ret_val;
      }
    } else {
      pfc_log_trace("Port %s exist in cache", port_id_ctr.c_str());
      unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, val_port_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, val_port_st_t, uint32_t>*> (cfgnode_cache);

      key_port_t *key_port = cfgptr_cache->get_key_structure();
      val_port_st_t *val_port_cache = cfgptr_cache->get_val_structure();

      if ((NULL == key_port) || (NULL == val_port_cache)) {
        pfc_log_error("key_port/val_port_cache is NULL");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      pfc_bool_t sw_update = is_port_modified(val_port_ctr, val_port_cache);
      if (PFC_TRUE == sw_update) {
        pfc_log_trace("Port is %s, its parameters are updated",
                      port_id_ctr.c_str());
        UncRespCode  ret_val =
            update_event(ctr_ptr, cfgnode_ctr, val_port_cache, port_list);
        if (UNC_RC_SUCCESS != ret_val) {
          pfc_log_error("error in updating the cache");
          return ret_val;
        }
      } else {
        pfc_log_trace("Port %s exist and value not updated",
                      port_id_ctr.c_str());
        update_list(key_port, port_list);
      }
      delete_config_node(cfg_node);
    }
  }
  // Check for delete
  UncRespCode  ret_val = delete_event(ctr_ptr, switch_id, port_list);
  return ret_val;
}


// Check whether any attributes for existing port is modified
pfc_bool_t OdcPort::is_port_modified(val_port_st_t *val_port_ctr,
                                     val_port_st_t *val_port_cache) {
  ODC_FUNC_TRACE;
  std::string logical_port_id_ctr = reinterpret_cast<const char*>
      (val_port_ctr->logical_port_id);
  std::string logical_port_id_cache = reinterpret_cast<const char*>
      (val_port_cache->logical_port_id);
  std::string description_ctr = reinterpret_cast<const char*>
      (val_port_ctr->port.description);
  std::string description_cache = reinterpret_cast<const char*>
      (val_port_cache->port.description);

  if ((logical_port_id_ctr.compare(logical_port_id_cache) != 0) ||
      (val_port_ctr->port.admin_status != val_port_cache->port.admin_status) ||
      (val_port_ctr->oper_status != val_port_cache->oper_status) ||
      (val_port_ctr->port.port_number != val_port_cache->port.port_number) ||
      (val_port_ctr->speed != val_port_cache->speed) ||
      (description_ctr.compare(description_cache) != 0)) {
    return PFC_TRUE;
  } else {
    return PFC_FALSE;
  }
}

// parsing function for converting controller response to driver format
UncRespCode OdcPort::parse_port_response(
    unc::driver::controller *ctr_ptr, char *data,
    std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_type error");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  uint32_t array_length =0;
  json_object *json_obj_node_conn_prop = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(jobj,
                                                     "nodeConnectorProperties",
                                                     -1,
                                                     json_obj_node_conn_prop);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_conn_prop, json_type_null))) {
    json_object_put(jobj);
    pfc_log_error("Parsing Error json_obj_node_conn_prop is null");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if (json_object_is_type(json_obj_node_conn_prop, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(
        jobj,
        "nodeConnectorProperties");
  }
  if (0 == array_length) {
    pfc_log_debug("No nodes port present");
    json_object_put(jobj);
    return UNC_RC_SUCCESS;
  }
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    UncRespCode ret_val = fill_config_node_vector(ctr_ptr,
                                                  json_obj_node_conn_prop,
                                                  arr_idx,
                                                  cfgnode_vector);
    if (UNC_RC_SUCCESS != ret_val) {
      json_object_put(jobj);
      pfc_log_error("Error return from parse_node_append_vector failure");
      return ret_val;
    }
  }
  json_object_put(jobj);
  return UNC_RC_SUCCESS;
}


UncRespCode OdcPort::read_cmd(unc::driver::controller *ctr_ptr,
                              unc::vtnreadutil::driver_read_util* read_util) {
  ODC_FUNC_TRACE;
  PFC_VERIFY(ctr_ptr != NULL);
  std::map<std::string, std::string> port_map;
  key_port_t port_key;
  uint32_t option2 = read_util->get_option2();
  std::string port_id_key = "";
  std::string switch_id = "";
  pfc_log_debug("option2 = %d", option2);

  if (option2 == UNC_OPT2_NONE) {
    unc::vtnreadutil::driver_read_util_io<key_port_t>::read_key_val
        (&port_key, PFC_TRUE, read_util);
  } else if (option2 == UNC_OPT2_SIBLING_ALL) {
    unc::vtnreadutil::driver_read_util_io<key_port_t>::read_key_val
        (&port_key, PFC_TRUE, read_util, PFC_FALSE);
  }
  switch_id = reinterpret_cast<const char*>
      (port_key.sw_key.switch_id);
  if (switch_id.empty()) {
    pfc_log_error("Switch id is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  //  Form a url and send the request to get port names of s/w
  //  and populate in map <port-name, port-id>

  std::string name_value = "";
  uint state_value = 0;
  uint config_value = 0;
  unsigned long long speed = 0;
  std::string url = "";
  url.append(BASE_SW_URL);
  url.append(CONTAINER_NAME);
  url.append(NODE);
  url.append(NODE_OF);
  url.append(SLASH);
  url.append(switch_id);

  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                                        ctr_ptr->get_user_name(),
                                        ctr_ptr->get_pass_word());

  unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(
          url, restjson::HTTP_METHOD_GET, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  int resp_code = response->code;
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("Response code is not OK , resp : %d", resp_code);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data_sw = response->write_data->memory;
      pfc_log_trace("All ports of a sw : %s", data_sw);
      json_object *jobj = restjson::JsonBuildParse::get_json_object(data_sw);
      if (json_object_is_type(jobj, json_type_null)) {
        pfc_log_error("json_object_is_type error");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      uint32_t array_length =0;
      json_object *json_obj_node_conn_prop = NULL;
      uint32_t ret_val = restjson::JsonBuildParse::parse(
          jobj,
          "nodeConnectorProperties",
          -1,
          json_obj_node_conn_prop);
      if ((restjson::REST_OP_SUCCESS != ret_val) ||
          (json_object_is_type(json_obj_node_conn_prop, json_type_null))) {
        json_object_put(jobj);
        pfc_log_error("Parsing Error json_obj_node_conn_prop is null");
        return UNC_DRV_RC_ERR_GENERIC;
      }

      if (json_object_is_type(json_obj_node_conn_prop, json_type_array)) {
        array_length = restjson::JsonBuildParse::get_array_length(
            jobj,
            "nodeConnectorProperties");
      }
      if (0 == array_length) {
        pfc_log_debug("No nodes port present");
        json_object_put(jobj);
        return UNC_RC_SUCCESS;
      }
      for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
        json_object *json_obj_node_connector = NULL;
        ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn_prop,
                                                  "nodeconnector",
                                                  arr_idx,
                                                  json_obj_node_connector);

        if ((restjson::REST_OP_SUCCESS != ret_val) ||
            (json_object_is_type(json_obj_node_connector, json_type_null))) {
          json_object_put(jobj);
          pfc_log_error("Parsing Error json_obj_node_cconnector is null");
          return UNC_DRV_RC_ERR_GENERIC;
        }

        std::string port_id = "";
        ret_val = restjson::JsonBuildParse::parse(json_obj_node_connector,
                                                  "id",
                                                  -1,
                                                  port_id);
        pfc_log_debug("port_id = %s ", port_id.c_str());
        if ((restjson::REST_OP_SUCCESS != ret_val) ||
            (port_id.empty())) {
          pfc_log_error(" Error while parsing port_id");
          return UNC_DRV_RC_ERR_GENERIC;
        }

        ret_val = parse_port_properties_value(arr_idx,
                                              json_obj_node_conn_prop,
                                              name_value,
                                              state_value,
                                              config_value,
                                              speed);
        pfc_log_debug("name_value = %s ", name_value.c_str());

        port_map.insert(std::pair<string, string>(name_value, port_id));
      }
      for (std::map<string, string>::iterator it = port_map.begin();
           it != port_map.end(); ++it)
        pfc_log_debug("first: %s  second: %s",
                      it->first.c_str() ,
                      it->second.c_str());
    }
  }

  // Form a url and send request to get port statisctics
  std::string port_id = "";
  url = "";
  url.append(BASE_PORT_URL);
  url.append(CONTAINER_NAME);
  url.append(PORT);
  url.append(NODE);
  url.append(NODE_OF);
  url.append(SLASH);
  url.append(switch_id);

  response = rest_util_obj.send_http_request(
      url, restjson::HTTP_METHOD_GET, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  resp_code = response->code;
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("Response code is not OK , resp : %d", resp_code);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data_port = response->write_data->memory;
      pfc_log_trace("All ports : %s", data_port);
      UncRespCode ret_val = parse_port_stat_response(ctr_ptr, read_util,
                                                     data_port,
                                                     port_map,
                                                     switch_id);
      if (ret_val != UNC_RC_SUCCESS) {
        pfc_log_error("Error while port stats and ret_val is:" "%d", ret_val);
        return ret_val;
      }
    }
  }
  return UNC_RC_SUCCESS;
}

UncRespCode OdcPort::parse_port_stat_response(
    unc::driver::controller *ctr_ptr,
    unc::vtnreadutil::driver_read_util* read_util, char *data_port,
    std::map<string, string> port_map, string switch_id) {

  ODC_FUNC_TRACE;
  uint32_t option2 = read_util->get_option2();
  json_object* jobj = restjson::JsonBuildParse::get_json_object(data_port);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_type error");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  uint32_t array_length =0;
  json_object *json_obj_port_stat = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(jobj,
                                                     "portStatistic",
                                                     -1,
                                                     json_obj_port_stat);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_port_stat, json_type_null))) {
    json_object_put(jobj);
    pfc_log_error("Parsing Error json_obj_port_stat is null");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if (json_object_is_type(json_obj_port_stat, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj,
                                                              "portStatistic");
  }

  if (0 == array_length) {
    pfc_log_debug("No port present");
    json_object_put(jobj);
    return UNC_RC_SUCCESS;
  }

  // Loop and fill all valid fileds for val_port_stats
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    key_port_t key_port;
    val_port_stats_t val_port;
    memset(&key_port, 0, sizeof(key_port_t));
    memset(&val_port, 0, sizeof(val_port_stats_t));

    val_port.valid[kIdxPortStatSt] = UNC_VF_INVALID;

    unsigned long long rx_pkts = 0;
    uint32_t ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                                       "receivePackets",
                                                       arr_idx,
                                                       rx_pkts);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing rx_pkts");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (rx_pkts != UINT64_MAX) {
        val_port.valid[kIdxPortStatRxPackets] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatRxPackets] = UNC_VF_INVALID;
      }
    }

    unsigned long long tx_pkts = 0;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "transmitPackets",
                                              arr_idx,
                                              tx_pkts);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing tx_pkts");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (tx_pkts != UINT64_MAX) {
        val_port.valid[kIdxPortStatTxPackets] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatTxPackets] = UNC_VF_INVALID;
      }
    }

    unsigned long long rx_bytes = 0;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "receiveBytes",
                                              arr_idx,
                                              rx_bytes);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing rx_bytes");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (rx_bytes != UINT64_MAX) {
        val_port.valid[kIdxPortStatRxBytes] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatRxBytes] = UNC_VF_INVALID;
      }
    }

    unsigned long long tx_bytes = 0;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "transmitBytes",
                                              arr_idx,
                                              tx_bytes);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing tx_bytes");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (tx_bytes != UINT64_MAX) {
        val_port.valid[kIdxPortStatTxBytes] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatTxBytes] = UNC_VF_INVALID;
      }
    }

    unsigned long long rx_drops = 0;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "receiveDrops",
                                              arr_idx,
                                              rx_drops);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing rx_drops");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (rx_drops != UINT64_MAX) {
        val_port.valid[kIdxPortStatRxDropped] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatRxDropped] = UNC_VF_INVALID;
      }
    }

    unsigned long long tx_drops = 0;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "transmitDrops",
                                              arr_idx,
                                              tx_drops);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing tx_drops");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (tx_drops != UINT64_MAX) {
        val_port.valid[kIdxPortStatTxDropped] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatTxDropped] = UNC_VF_INVALID;
      }
    }

    unsigned long long rx_err = 0;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "receiveErrors",
                                              arr_idx,
                                              rx_err);

    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing rx_err");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (rx_err != UINT64_MAX) {
        val_port.valid[kIdxPortStatRxErrors] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatRxErrors] = UNC_VF_INVALID;
      }
    }

    unsigned long long tx_err = 0;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "transmitErrors",
                                              arr_idx,
                                              tx_err);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing tx_err");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (tx_err != UINT64_MAX) {
        val_port.valid[kIdxPortStatTxErrors] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatTxErrors] = UNC_VF_INVALID;
      }
    }

    unsigned long long rx_Frame_err = 0;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "receiveFrameError",
                                              arr_idx,
                                              rx_Frame_err);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing rx_Frame_err");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (rx_Frame_err != UINT64_MAX) {
        val_port.valid[kIdxPortStatRxFrameErr] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatRxFrameErr] = UNC_VF_INVALID;
      }
    }

    unsigned long long rx_OverRun_err = 0;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "receiveOverRunError",
                                              arr_idx,
                                              rx_OverRun_err);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing rx_OverRun_err");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (rx_OverRun_err != UINT64_MAX) {
        val_port.valid[kIdxPortStatRxOverErr] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatRxOverErr] = UNC_VF_INVALID;
      }
    }

    unsigned long long rx_Crc_err = 0;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "receiveCrcError",
                                              arr_idx,
                                              rx_Crc_err);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing rx_Crc_err");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (rx_Crc_err != UINT64_MAX) {
        val_port.valid[kIdxPortStatRxCrcErr] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatRxCrcErr] = UNC_VF_INVALID;
      }
    }

    unsigned long long Collision_count = 0;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "collisionCount",
                                              arr_idx,
                                              Collision_count);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error while parsing Collision_count");
      return UNC_DRV_RC_ERR_GENERIC;
    } else {
      if (Collision_count != UINT64_MAX) {
        val_port.valid[kIdxPortStatCollisions] = UNC_VF_VALID;
      } else {
        val_port.valid[kIdxPortStatCollisions] = UNC_VF_INVALID;
      }
    }
    json_object *json_obj_node_conn = NULL;
    ret_val = restjson::JsonBuildParse::parse(json_obj_port_stat,
                                              "nodeConnector",
                                              arr_idx,
                                              json_obj_node_conn);
    if ((restjson::REST_OP_SUCCESS != ret_val) ||
        (json_object_is_type(json_obj_node_conn, json_type_null))) {
      pfc_log_error(" Error while parsing node Connector");
      return UNC_DRV_RC_ERR_GENERIC;
    }

    std::string port_id = "";
    ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn,
                                              "id",
                                              -1,
                                              port_id);
    if ((restjson::REST_OP_SUCCESS != ret_val) || (port_id.empty())) {
      pfc_log_error(" Error while parsing port_id");
      return UNC_DRV_RC_ERR_GENERIC;
    }

    // Fill key and val Struct
    val_port.rx_packets = rx_pkts;
    val_port.tx_packets = tx_pkts;
    val_port.rx_bytes = rx_bytes;
    val_port.tx_bytes = tx_bytes;
    val_port.rx_dropped = rx_drops;
    val_port.tx_dropped = tx_drops;
    val_port.rx_errors = rx_err;
    val_port.tx_errors = tx_err;
    val_port.rx_frame_err = rx_Frame_err;
    val_port.rx_over_err = rx_OverRun_err;
    val_port.rx_crc_err = rx_Crc_err;
    val_port.collisions = Collision_count;

    if (option2 == UNC_OPT2_SIBLING_ALL) {
      std::string ctr_name = ctr_ptr->get_controller_id();
      strncpy(reinterpret_cast<char*> (key_port.sw_key.ctr_key.controller_name),
              ctr_name.c_str(), strlen(ctr_name.c_str()));
      for (std::map<string, string>::iterator it = port_map.begin();
          it != port_map.end(); ++it) {
        if (it->second.compare(port_id) == 0) {
          pfc_log_debug("Port_id matched map: %s", it->second.c_str());
          strncpy(reinterpret_cast<char*> (key_port.port_id), it->first.c_str(),
                  strlen(it->first.c_str()));
          break;
        }
      }
      strncpy(reinterpret_cast<char*> (key_port.sw_key.switch_id),
              switch_id.c_str(), strlen(switch_id.c_str()));
      unc::vtnreadutil::driver_read_util_io<key_port_t>::
          add_read_value(&key_port, read_util);
    } else if (option2 == UNC_OPT2_NONE) {
      unc::vtnreadutil::driver_read_util_io<key_port_t>::read_key_val
          (&key_port, PFC_TRUE, read_util);
      std::string port_id_key = reinterpret_cast<const char*>
          (key_port.port_id);
      if (port_id_key.empty()) {
        pfc_log_error("Port_id_key is empty");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      if (port_id.compare(port_id_key) == 0) {
        pfc_log_debug("Matched port_id = %s",  port_id.c_str());
        unc::vtnreadutil::driver_read_util_io<val_port_stats_t>::
            add_read_value(&val_port, read_util);
        return UNC_RC_SUCCESS;
      }
    }
    unc::vtnreadutil::driver_read_util_io<val_port_stats_t>::
        add_read_value(&val_port, read_util);

    //  Wrting empty key_val struct to cache as a separator
    //  for reading in platform layer
    memset(&key_port, 0, sizeof(key_port_t));
    memset(&val_port, 0, sizeof(val_port_stats_t));
    unc::vtnreadutil::driver_read_util_io<key_port_t>::
        add_read_value(&key_port, read_util);
    unc::vtnreadutil::driver_read_util_io<val_port_stats_t>::
        add_read_value(&val_port, read_util);
  }
  return UNC_RC_SUCCESS;
}

}  // namespace odcdriver
}  // namespace unc
