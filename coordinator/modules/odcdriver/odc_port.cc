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
}


// Get logical port information in the controller
drv_resp_code_t OdcPort::fetch_config(
             unc::driver::controller *ctr_ptr, key_switch_t *parent_switch,
             const pfc_bool_t cache_empty) {
  ODC_FUNC_TRACE;

  PFC_VERIFY(ctr_ptr != NULL);

  if (NULL == parent_switch) {
    pfc_log_error("parent_switch is NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::vector<unc::vtndrvcache::ConfigNode *> cfgnode_vector;
  std::string switch_id = reinterpret_cast<const char*>
                (parent_switch->switch_id);

  if (switch_id.empty()) {
    pfc_log_error("Switch id is empty");
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::string url = "";
  url.append(BASE_SW_URL);
  url.append(CONTAINER_NAME);
  url.append(NODE);
  url.append(NODE_OF);
  url.append(SLASH);
  url.append(switch_id);

  pfc_log_debug("Url for port %s", url.c_str());
  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                                        ctr_ptr->get_user_name(),
                                        ctr_ptr->get_pass_word());

  unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(
          url, restjson::HTTP_METHOD_GET, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return DRVAPI_RESPONSE_FAILURE;
  }

  int resp_code = response->code;
  pfc_log_debug("response code returned in get all nodes is %d", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("Response code is not OK , resp : %d", resp_code);
    return DRVAPI_RESPONSE_FAILURE;
  }

  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      char *data = response->write_data->memory;
      pfc_log_debug("All nodes : %s", data);
      drv_resp_code_t ret_val =
          parse_port_response(ctr_ptr, data, cfgnode_vector);
      pfc_log_debug("Number of Ports in Switch %s is, %d", switch_id.c_str(),
                    static_cast<int>(cfgnode_vector.size()));
      if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
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
  return DRVAPI_RESPONSE_FAILURE;
}

// parse each port and append to cache
drv_resp_code_t OdcPort::fill_config_node_vector(
                                 unc::driver::controller *ctr_ptr,
                     json_object *json_obj_node_conn_prop, int arr_idx,
         std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_port_t key_port;
  val_port_st_t val_port;
  memset(&key_port, 0, sizeof(key_port_t));
  memset(&val_port, 0, sizeof(val_port_st_t));

  json_object *json_obj_node_conn;
  uint32_t ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn_prop,
                                 "nodeconnector", arr_idx, json_obj_node_conn);
  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
  (json_object_is_type(json_obj_node_conn, json_type_null))) {
    pfc_log_debug(" Error while parsing node");
    return DRVAPI_RESPONSE_FAILURE;
  }

  std::string node_conn_id = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn, "id",
                                            -1, node_conn_id);
  pfc_log_debug("node_conn_id is %s", node_conn_id.c_str());
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    pfc_log_debug(" Error while parsing node_conn_id");
    return DRVAPI_RESPONSE_FAILURE;
  }

  json_object *json_obj_node = NULL;
  std::string node_type = "";
  std::string node_id   = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn, "node",
                                            -1, json_obj_node);
  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node, json_type_null))) {
    pfc_log_debug(" Error while parsing port or json obj is NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn, "type",
                                            -1, node_type);
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    pfc_log_debug(" Error while parsing node_type");
    return DRVAPI_RESPONSE_FAILURE;
  }
  if (node_type.compare("SW") == 0) {
    pfc_log_debug("Type is SW ");
    return DRVAPI_RESPONSE_SUCCESS;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_node, "id",
                                            -1, node_id);
  pfc_log_debug(" node id %s", node_id.c_str());
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    pfc_log_debug(" Error while parsing node id");
    return DRVAPI_RESPONSE_FAILURE;
  }

  std::string name_value = "";
  uint state_value = 0;
  uint config_value = 0;
  unsigned long long speed = 0;
  ret_val = parse_port_properties_value(arr_idx, json_obj_node_conn_prop,
                             name_value, state_value, config_value, speed);
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    pfc_log_error("Error in  parsing node conn properties");
    return DRVAPI_RESPONSE_FAILURE;
  }

  pfc_log_debug("Name of port %s", name_value.c_str());
  pfc_log_debug("Port id %s", node_id.c_str());

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
  pfc_log_debug("key_port.id: %s",
                reinterpret_cast<char*> (key_port.port_id));

  std::string port_id = "PP-OF:";
  port_id.append(node_id);
  port_id.append(HYPHEN);
  port_id.append(name_value);
  pfc_log_debug("Port id formed in logical port : %s", port_id.c_str());

  strncpy(reinterpret_cast<char*> (val_port.logical_port_id), port_id.c_str(),
          strlen(port_id.c_str()));
  val_port.valid[kIdxPortLogicalPortId] = UNC_VF_VALID;

  if (ADMIN_UP == config_value) {
    val_port.port.admin_status = UPPL_SWITCH_ADMIN_UP;
    val_port.port.valid[VAL_PORT_EVENT_ATTR3] = UNC_VF_VALID;
  } else if (ADMIN_DOWN == config_value)  {
    val_port.port.admin_status = UPPL_SWITCH_ADMIN_DOWN;
    val_port.port.valid[VAL_PORT_EVENT_ATTR3] = UNC_VF_VALID;
  }

  val_port.port.port_number = atoi(node_conn_id.c_str());
  val_port.port.valid[VAL_PORT_EVENT_ATTR1] = UNC_VF_VALID;

  if (unc::driver::CONNECTION_UP == ctr_ptr->get_connection_status()) {
    if ((ADMIN_UP == config_value) &&
        (EDGE_UP == state_value)) {
      pfc_log_debug("oper status of port is UP");
      val_port.oper_status = UPPL_PORT_OPER_UP;
      val_port.valid[VAL_PORT_EVENT_ATTR2] = UNC_VF_VALID;
    } else {
      pfc_log_debug("oper status of port is DOWN");
      val_port.oper_status = UPPL_PORT_OPER_DOWN;
      val_port.valid[VAL_PORT_EVENT_ATTR2] = UNC_VF_VALID;
    }
  } else {
    pfc_log_debug("oper status of port is UNKNOWN");
    val_port.oper_status = UPPL_SWITCH_OPER_UNKNOWN;
    val_port.valid[VAL_PORT_EVENT_ATTR2] = UNC_VF_VALID;
  }

  val_port.speed = speed;
  val_port.valid[VAL_PORT_EVENT_ATTR6] = UNC_VF_VALID;

  unc::vtndrvcache::ConfigNode *cfgptr =
    new unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, uint32_t>
    (&key_port, &val_port, uint32_t(UNC_OP_READ));
  PFC_VERIFY(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  return DRVAPI_RESPONSE_SUCCESS;
}

// parse the port properties vale
drv_resp_code_t OdcPort::parse_port_properties_value(int arr_idx,
                   json_object *json_obj_node_conn, std::string &name_value,
         uint &state_value, uint &config_value, unsigned long long &speed) {
  ODC_FUNC_TRACE;
  json_object *json_obj_node_prop = NULL;
  int ret_val = restjson::JsonBuildParse::parse(json_obj_node_conn,
                           "properties", arr_idx, json_obj_node_prop);
  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_prop, json_type_null))) {
    pfc_log_error(" Error while parsing node properties");
    return DRVAPI_RESPONSE_FAILURE;
  }
  json_object *json_obj_node_prop_name = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop, "name",
                                          -1, json_obj_node_prop_name);
  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_prop_name, json_type_null))) {
    pfc_log_error(" Error while parsing node name");
    return DRVAPI_RESPONSE_FAILURE;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop_name, "value",
                                            -1, name_value);
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    pfc_log_debug(" Error while parsing node name value");
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("name value %s", name_value.c_str());

  json_object *json_obj_node_prop_state = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop, "state",
                                            -1, json_obj_node_prop_state);
  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_prop_state, json_type_null))) {
    pfc_log_error(" Error while parsing node state properties");
    return DRVAPI_RESPONSE_FAILURE;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop_state, "value",
                                            -1, state_value);
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    pfc_log_debug(" Error while parsing node state value properties");
    return DRVAPI_RESPONSE_FAILURE;
  }

  json_object *json_obj_node_prop_config = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop, "config",
                                            -1, json_obj_node_prop_config);
  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_prop_config, json_type_null))) {
    pfc_log_error(" Error while parsing node config properties");
    return DRVAPI_RESPONSE_FAILURE;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop_config, "value",
                                            -1, config_value);
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    pfc_log_error(" Error while parsing node config properties");
    return DRVAPI_RESPONSE_FAILURE;
  }

  json_object *json_obj_node_prop_speed = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop, "bandwidth",
                                            -1, json_obj_node_prop_speed);
  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_prop_speed, json_type_null))) {
    pfc_log_error(" Error while parsing node bandwidth properties");
    return DRVAPI_RESPONSE_FAILURE;
  }

  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop_speed, "value",
                                            -1, speed);
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    pfc_log_error(" Error while parsing node bandwidth properties");
    return DRVAPI_RESPONSE_FAILURE;
  }

  return DRVAPI_RESPONSE_SUCCESS;
}

//  Compare whether cache empty or not
drv_resp_code_t OdcPort::compare_with_cache(unc::driver::controller *ctr_ptr,
                  std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
                 const std::string &switch_id, const pfc_bool_t cache_empty) {
  ODC_FUNC_TRACE;
  if (ctr_ptr->physical_port_cache == NULL) {
    pfc_log_error("Cache is empty");
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::list<std::string> port_list;
  drv_resp_code_t ret_val = DRVAPI_RESPONSE_FAILURE;

  if (cache_empty == PFC_TRUE) {
    for (std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
         cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it) {
      unc::vtndrvcache::ConfigNode *cfgnode_cache = *it;
      if (NULL == cfgnode_cache) {
        pfc_log_error("cfgnode_cache is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }
      ret_val = add_event(ctr_ptr, cfgnode_cache, port_list);
      if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
        pfc_log_error("Error in adding to cache");
        return ret_val;
      }
    }
  } else {
    ret_val = verify_in_cache(ctr_ptr, cfgnode_vector, switch_id, port_list);
    pfc_log_debug("verify in cache ret val %d", ret_val);
    return ret_val;
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

//  Notify physical about the port event
void OdcPort::notify_physical(unc::driver::oper_type type, key_port_t *key_port,
                        val_port_st_t *val_port, val_port_st_t *val_old_port) {
  ODC_FUNC_TRACE;
  unc::driver::VtnDrvIntf* disp_inst = static_cast<unc::driver::VtnDrvIntf*>
      (pfc::core::Module::getInstance("vtndrvintf"));
  PFC_VERIFY(disp_inst != NULL);
  std::string port_id = reinterpret_cast<const char*> (key_port->port_id);
  pfc_log_debug("Port id received %s", port_id.c_str());

  if (type == unc::driver::VTN_PORT_UPDATE) {
    pfc_log_debug("Port event received is UPDATE");
    disp_inst->port_event(unc::driver::VTN_PORT_UPDATE, *key_port,
                          *val_port, *val_old_port);
  } else {
    pfc_log_debug("Port event received %d", type);
    disp_inst->port_event(type, *key_port, *val_port);

    pfc_log_debug("logical port event to be sent");
    notify_logical_port_physical(type, key_port, val_port, val_old_port);
  }
}

// Notify Logical port to physical
void OdcPort::notify_logical_port_physical(unc::driver::oper_type type,
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
                              key_logical_port, val_logical_port);

  // If PORT update request received fill old logical port structure
  if (type == unc::driver::VTN_PORT_CREATE) {
    disp_inst->logicalport_event(unc::driver::VTN_LP_CREATE, key_logical_port,
                                 val_logical_port);
  } else if ((type == unc::driver::VTN_PORT_DELETE) ||
             (type == unc::driver::VTN_LP_DELETE)) {
    disp_inst->logicalport_event(unc::driver::VTN_LP_DELETE, key_logical_port,
                                 val_logical_port);
  }
}

// fills logical port structure for CREATE/ DELETE
void OdcPort::fill_logical_port_structure(key_port_t *key_port,
         val_port_st_t *val_port, key_logical_port_t &key_logical_port,
         val_logical_port_st_t &val_logical_port) {
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
void OdcPort::fill_logical_port_val_structure(key_port_t *key_port,
            val_port_st_t *val_port, val_logical_port_st_t &val_logical_port) {
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
  std::string switch_port_id = reinterpret_cast<const char*>
      (key_port->sw_key.switch_id);
  std::string port_id = reinterpret_cast<const char*> (key_port->port_id);
  switch_port_id.append(HYPHEN);
  switch_port_id.append(port_id);
  pfc_log_debug("port id to be updated in list is %s", switch_port_id.c_str());

  std::list <std::string>::iterator result = std::find(port_list.begin(),
                                           port_list.end(), switch_port_id);

  // Push into list only if the entry not exist already
  if (result == port_list.end()) {
    port_list.push_front(switch_port_id);
  }
}

//  Add event for Port
drv_resp_code_t OdcPort::add_event(unc::driver::controller *ctr_ptr,
                                   unc::vtndrvcache::ConfigNode *cfg_node,
                                   std::list<std::string> &port_list) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_port_t, val_port_st_t, uint32_t>*> (cfg_node);
  key_port_t *key_port = cfgptr_cache->get_key_structure();
  val_port_st_t *val_port = cfgptr_cache->get_val_structure();

  if ((NULL == key_port) || (NULL == val_port)) {
    pfc_log_debug("key_port/val_port is NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }
  //  Send Notification to UPPL
  notify_physical(unc::driver::VTN_PORT_CREATE, key_port, val_port, NULL);

  std::string port_id = reinterpret_cast<const char*> (key_port->port_id);
  //  Append to cache
  drv_resp_code_t  ret_val =
      ctr_ptr->physical_port_cache->append_Physical_attribute_node(cfg_node);
  if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
    pfc_log_error(" Error in adding to cache");
    return ret_val;
  }
  update_list(key_port, port_list);
  return ret_val;
}

// Update event for Port
drv_resp_code_t OdcPort::update_event(unc::driver::controller *ctr_ptr,
                                      unc::vtndrvcache::ConfigNode *cfg_node,
                                      val_port_st_t *val_old_port,
                                      std::list<std::string> &port_list) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_port_t, val_port_st_t, uint32_t>*> (cfg_node);
  key_port_t *key_port = cfgptr_cache->get_key_structure();
  val_port_st_t *val_port = cfgptr_cache->get_val_structure();

  if ((NULL == key_port) || (NULL == val_port) || (NULL == val_old_port)) {
    pfc_log_error("key_port/val_port is NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }
  // Send notification to UPPL
  notify_physical(unc::driver::VTN_PORT_UPDATE, key_port,
                                  val_port, val_old_port);

  // Append to cache
  drv_resp_code_t  ret_val =
      ctr_ptr->physical_port_cache->update_physical_attribute_node(cfg_node);
  if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
    pfc_log_error(" Error in updating to cache");
    return ret_val;
  }

  update_list(key_port, port_list);
  return ret_val;
}

// Check Delete event for Port
drv_resp_code_t OdcPort::delete_event(unc::driver::controller *ctr_ptr,
                                      const std::string &switch_id,
                                      std::list<std::string> &port_list) {
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  std::map<std::string, unc::vtndrvcache::ConfigNode *> cfg_node_delete_map;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr_ptr->physical_port_cache->create_iterator());

  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    if (cfgnode_cache == NULL) {
      pfc_log_debug("cfgnode is NULL before get_type");
      return DRVAPI_RESPONSE_FAILURE;
    }
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_PORT == key_type) {
      unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, uint32_t>*> (cfgnode_cache);
      if (NULL == cfgptr_cache) {
        pfc_log_error("cfgptr_cache pointer is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }

      key_port_t *key_port_cache = cfgptr_cache->get_key_structure();
      if (NULL == key_port_cache) {
        pfc_log_error("key_port_cache is NULL");
        return DRVAPI_RESPONSE_FAILURE;
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
        pfc_log_debug("Search %s in list", sw_port_id.c_str());
        std::list <std::string>::iterator result = std::find(port_list.begin(),
                                                 port_list.end(), sw_port_id);
        if (result == port_list.end()) {
          pfc_log_debug("Port %s to be deleted", switch_id.c_str());
          cfg_node_delete_map[sw_port_id] = cfgnode_cache;
        }
      }
    }
  }
  return delete_port(ctr_ptr, cfg_node_delete_map);
}

drv_resp_code_t OdcPort::delete_port(unc::driver::controller *ctr_ptr,
                                     const std::map<std::string,
                unc::vtndrvcache::ConfigNode *> &cfg_node_delete_map) {
  int cfg_size = cfg_node_delete_map.size();
  pfc_log_debug("Port delete_map size %d",  cfg_size);

  std::map<std::string, unc::vtndrvcache::ConfigNode *>::
      const_iterator iterator;
  for (iterator = cfg_node_delete_map.begin();
       iterator != cfg_node_delete_map.end(); ++iterator) {
    unc::vtndrvcache::ConfigNode *cfg_node = iterator->second;
    if (cfg_node == NULL) {
      pfc_log_debug("cfgnode is NULL before get_type");
      return DRVAPI_RESPONSE_FAILURE;
    }

    unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, uint32_t>
        *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
        <key_port_t, val_port_st_t, uint32_t>*> (cfg_node);

    key_port_t *key_port = cfgptr_cache->get_key_structure();
    val_port_st_t *val_port = cfgptr_cache->get_val_structure();

    if ((NULL == key_port) || (NULL == val_port)) {
      pfc_log_error("key_port/val_port is NULL");
      return DRVAPI_RESPONSE_FAILURE;
    }
    // Send notification to UPPL
    notify_physical(unc::driver::VTN_PORT_DELETE, key_port, val_port, NULL);

    // Delete from cache
    drv_resp_code_t  ret_val =
        ctr_ptr->physical_port_cache->delete_physical_attribute_node(cfg_node);
    if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
      pfc_log_error(" Error in deleting in cache");
      return ret_val;
    }
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

// Deletes config node
void OdcPort::delete_config_node(unc::vtndrvcache::ConfigNode *cfg_node) {
  ODC_FUNC_TRACE;
  if (cfg_node != NULL) {
    delete cfg_node;
    cfg_node = NULL;
  }
}

//  Verify in cache whether port aleady exists or not
drv_resp_code_t OdcPort::verify_in_cache(unc::driver::controller *ctr_ptr,
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
    if (cfg_node == NULL) {
      pfc_log_error("cfg_node is NULL");
      return DRVAPI_RESPONSE_FAILURE;
    }
    unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, uint32_t>
        *cfgnode_ctr = static_cast<unc::vtndrvcache::CacheElementUtil
        <key_port_t, val_port_st_t, uint32_t>*> (cfg_node);

    val_port_st_t *val_port_ctr = cfgnode_ctr->get_val_structure();
    if (NULL == val_port_ctr) {
      pfc_log_error("val_port_ctr is NULL");
      return DRVAPI_RESPONSE_FAILURE;
    }

    unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
    pfc_bool_t exist_in_cache =
        ctr_ptr->physical_port_cache->compare_is_physical_node_found(
            cfgnode_ctr, cfgnode_cache);

    if (PFC_FALSE == exist_in_cache) {
      pfc_log_debug("Port  not exist in cache");
      drv_resp_code_t  ret_val = add_event(ctr_ptr, cfgnode_ctr, port_list);
      if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
        pfc_log_error("Error occured in adding to cache");
        return ret_val;
      }
    } else {
      pfc_log_debug("Already port exist in cache");
      unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, uint32_t>*> (cfgnode_cache);

      key_port_t *key_port = cfgptr_cache->get_key_structure();
      val_port_st_t *val_port_cache = cfgptr_cache->get_val_structure();

      if ((NULL == key_port) || (NULL == val_port_cache)) {
        pfc_log_error("key_port/val_port_cache is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }
      pfc_bool_t sw_update = is_port_modified(val_port_ctr, val_port_cache);
      if (sw_update == PFC_TRUE) {
        pfc_log_debug("Port is modifed");
        drv_resp_code_t  ret_val =
            update_event(ctr_ptr, cfgnode_ctr, val_port_cache, port_list);
        if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
          pfc_log_error("error in updating the cache");
          return ret_val;
        }
      } else {
        pfc_log_debug("Already port exist and value not modified");
        update_list(key_port, port_list);
      }
      delete_config_node(cfg_node);
    }
  }
  // Check for delete
  drv_resp_code_t  ret_val = delete_event(ctr_ptr, switch_id, port_list);
  pfc_log_debug("Value returned in delete event %d", ret_val);
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
drv_resp_code_t OdcPort::parse_port_response(
    unc::driver::controller *ctr_ptr, char *data,
    std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_type error");
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t array_length =0;
  json_object *json_obj_node_conn_prop = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(jobj,
                       "nodeConnectorProperties", -1, json_obj_node_conn_prop);

  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_conn_prop, json_type_null))) {
    json_object_put(jobj);
    pfc_log_error("Parsing Error json_obj_node_conn_prop is null");
    return DRVAPI_RESPONSE_FAILURE;
  }

  if (json_object_is_type(json_obj_node_conn_prop, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj,
                                                  "nodeConnectorProperties");
  }
  if (0 == array_length) {
    pfc_log_debug("No nodes port present");
    json_object_put(jobj);
    return DRVAPI_RESPONSE_SUCCESS;
  }
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    drv_resp_code_t ret_val = fill_config_node_vector(ctr_ptr,
                          json_obj_node_conn_prop, arr_idx, cfgnode_vector);
    if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
      json_object_put(jobj);
      pfc_log_error("Error return from parse_node_append_vector failure");
      return ret_val;
    }
  }
  json_object_put(jobj);
  return DRVAPI_RESPONSE_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
