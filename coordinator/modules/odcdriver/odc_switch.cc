/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_switch.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcSwitch::OdcSwitch(unc::restjson::ConfFileValues_t conf_values)
: conf_file_values_(conf_values) {
  ODC_FUNC_TRACE;
}

// Destructor
OdcSwitch::~OdcSwitch() {
}


// Get switch information in the controller
drv_resp_code_t OdcSwitch::fetch_config(
    unc::driver::controller *ctr_ptr, pfc_bool_t &cache_empty) {
  ODC_FUNC_TRACE;
  PFC_VERIFY(ctr_ptr != NULL);
  std::vector<unc::vtndrvcache::ConfigNode *> cfgnode_vector;

  std::string url = "";
  url.append(BASE_SW_URL);
  url.append(CONTAINER_NAME);
  url.append(NODES);

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
      pfc_log_debug("All Switches : %s", data);
      drv_resp_code_t ret_val =  parse_node_response(
                              ctr_ptr, data, cfgnode_vector);
      pfc_log_debug("Number of SWITCH present, %d",
                    static_cast<int>(cfgnode_vector.size()));
      if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
        pfc_log_error("Error occured while parsing");
        return ret_val;
      }
      ret_val = compare_with_cache(ctr_ptr, cfgnode_vector, cache_empty);
      pfc_log_debug("Response from compare_with_cache is %d", ret_val);
      return ret_val;
    }
  }
  pfc_log_error("Response data is NULL");
  return DRVAPI_RESPONSE_FAILURE;;
}

// parse each SWITCH append t  cache
drv_resp_code_t OdcSwitch::fill_config_node_vector(
    unc::driver::controller *ctr_ptr, json_object *json_obj_node_prop,
    int arr_idx, std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_switch_t key_switch;
  val_switch_st_t val_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  memset(&val_switch, 0, sizeof(val_switch_st_t));

  json_object *json_obj_node = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop, "node",
                                                     arr_idx, json_obj_node);
  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node, json_type_null))) {
    pfc_log_debug(" Error while parsing node or json type NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  std::string node_id   = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_node, "id",
                                            -1, node_id);
  pfc_log_debug(" node id %s:", node_id.c_str());
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    pfc_log_debug(" Error while parsing id");
    return DRVAPI_RESPONSE_FAILURE;
  }

  json_object *json_prop = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop, "properties",
                                            arr_idx, json_prop);
  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
      (json_object_is_type(json_prop, json_type_null))) {
    pfc_log_debug(" Error while parsing description or json_prop NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }
  json_object *json_obj_description = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_prop, "description",
                                            -1, json_obj_description);
  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_description, json_type_null))) {
    pfc_log_debug(" Error while parsing description or json type NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::string desc   = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_description, "value",
                                            -1, desc);
  pfc_log_debug("desc %s:", desc.c_str());
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    pfc_log_debug(" Error while parsing desc");
    return DRVAPI_RESPONSE_FAILURE;
  }

  std::string ctr_name = ctr_ptr->get_controller_id();
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), node_id.c_str(),
          strlen(node_id.c_str()));
  strncpy(reinterpret_cast<char*> (key_switch.ctr_key.controller_name),
          ctr_name.c_str(), strlen(ctr_name.c_str()));

  // Fill Value Structure
  val_switch.valid[VAL_SWITCH_ATTR] = UNC_VF_VALID;
  pfc_log_debug("key_switch.id: %s",
                reinterpret_cast<char*> (key_switch.switch_id));

  strncpy(reinterpret_cast<char*> (val_switch.switch_val.domain_name),
          DOM_NAME.c_str(), strlen(DOM_NAME.c_str()));
  val_switch.switch_val.valid[VAL_DOMAINID_ATTR] = UNC_VF_VALID;

  strncpy(reinterpret_cast<char*> (val_switch.switch_val.description),
          desc.c_str(), strlen(desc.c_str()));
  val_switch.switch_val.valid[VAL_DESCRIPTION] = UNC_VF_VALID;

  strncpy(reinterpret_cast<char*> (val_switch.switch_val.model),
          switchmodel.c_str(), strlen(switchmodel.c_str()));
  val_switch.switch_val.valid[VAL_MODEL] = UNC_VF_VALID;

  val_switch.oper_status = UPPL_SWITCH_OPER_UP;
  val_switch.valid[VAL_OPER_STAT_ATTR] = UNC_VF_VALID;

  unc::vtndrvcache::ConfigNode *cfgptr = new unc::vtndrvcache::CacheElementUtil
      <key_switch_t, val_switch_st_t, uint32_t>
      (&key_switch, &val_switch, uint32_t(UNC_OP_READ));

  PFC_VERIFY(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  return DRVAPI_RESPONSE_SUCCESS;
}

//  Compare whether cache empty or not
drv_resp_code_t OdcSwitch::compare_with_cache(unc::driver::controller *ctr_ptr,
                   std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
                                                     pfc_bool_t &cache_empty) {
  ODC_FUNC_TRACE;
  if (ctr_ptr->physical_port_cache == NULL) {
    pfc_log_error("cache pointer is empty");
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::list<std::string> switch_list;
  drv_resp_code_t ret_val = DRVAPI_RESPONSE_FAILURE;
  if (0 == ctr_ptr->physical_port_cache->cfg_list_count()) {
    cache_empty = PFC_TRUE;
    for (std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
         cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it) {
      unc::vtndrvcache::ConfigNode *cfgnode_cache = *it;
      if (cfgnode_cache == NULL) {
        pfc_log_error("cfgnode_cache is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }
      ret_val = add_event(ctr_ptr, cfgnode_cache, switch_list);
      if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
        pfc_log_error("Error in adding to cache");
        return ret_val;
      }
    }
  } else {
    cache_empty = PFC_FALSE;
    ret_val = verify_in_cache(ctr_ptr, cfgnode_vector, switch_list);
    pfc_log_debug("Response for verify_in_cache is %d", ret_val);
    return ret_val;
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

//  Notify physical about the switch event
void OdcSwitch::notify_physical(unc::driver::oper_type type,
                                key_switch_t *key_switch,
                                val_switch_st_t *val_switch,
                                val_switch_st_t *val_old_switch) {
  ODC_FUNC_TRACE;
  unc::driver::VtnDrvIntf* disp_inst = static_cast<unc::driver::VtnDrvIntf*>
      (pfc::core::Module::getInstance("vtndrvintf"));
  PFC_VERIFY(disp_inst != NULL);
  if (type == unc::driver::VTN_SWITCH_UPDATE) {
    disp_inst->switch_event(unc::driver::VTN_SWITCH_UPDATE, *key_switch,
                            *val_switch, *val_old_switch);
  } else {
    disp_inst->switch_event(type, *key_switch, *val_switch);
  }
}

//  Update the local list maintained for delete
void OdcSwitch::update_list(key_switch_t *key_switch,
                            std::list<std::string> &switch_list) {
  ODC_FUNC_TRACE;
  std::string switch_id = reinterpret_cast<const char*> (key_switch->switch_id);
  pfc_log_debug("SW updated in list %s", switch_id.c_str());
  std::list <std::string>::iterator result = std::find(
      switch_list.begin(), switch_list.end(), switch_id);

  // Push into list only if the entry not exist already
  if (result == switch_list.end()) {
    switch_list.push_front(switch_id);
  }
}

//  Add event for Switch
drv_resp_code_t OdcSwitch::add_event(unc::driver::controller *ctr_ptr,
                                     unc::vtndrvcache::ConfigNode *cfg_node,
                                     std::list<std::string> &switch_list) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::CacheElementUtil<key_switch_t, val_switch_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_switch_t, val_switch_st_t, uint32_t>*> (cfg_node);
  key_switch_t *key_switch = cfgptr_cache->get_key_structure();
  val_switch_st_t *val_switch = cfgptr_cache->get_val_structure();

  if ((NULL == key_switch) || (NULL == val_switch)) {
    pfc_log_error("key_switch/val_switch is NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }
  //  Send Notification to UPPL
  notify_physical(unc::driver::VTN_SWITCH_CREATE,
                  key_switch, val_switch, NULL);

  //  Append to cache
  drv_resp_code_t  ret_val =
      ctr_ptr->physical_port_cache->append_Physical_attribute_node(cfg_node);
  if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
    pfc_log_error(" Error in adding to cache");
    return ret_val;
  }
  pfc_log_debug("Update the list maintained");
  update_list(key_switch, switch_list);
  return ret_val;
}

// Update event for Switch
drv_resp_code_t OdcSwitch::update_event(unc::driver::controller *ctr_ptr,
                                        unc::vtndrvcache::ConfigNode *cfg_node,
                                        val_switch_st_t *val_old_switch,
                                        std::list<std::string> &switch_list) {
  unc::vtndrvcache::CacheElementUtil<key_switch_t, val_switch_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_switch_t, val_switch_st_t, uint32_t>*> (cfg_node);
  key_switch_t *key_switch = cfgptr_cache->get_key_structure();
  val_switch_st_t *val_switch = cfgptr_cache->get_val_structure();

  if ((NULL == key_switch) || (NULL == val_switch) ||
      (NULL == val_old_switch)) {
    pfc_log_error("key_switch/val_switch is NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  // Send notification to UPPL
  notify_physical(unc::driver::VTN_SWITCH_UPDATE, key_switch,
                  val_switch, val_old_switch);

  // Append to cache
  drv_resp_code_t  ret_val =
      ctr_ptr->physical_port_cache->update_physical_attribute_node(cfg_node);
  if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
    pfc_log_error(" Error in updating to cache");
    return ret_val;
  }
  update_list(key_switch, switch_list);
  return ret_val;
}

// Delete event for Switch
drv_resp_code_t OdcSwitch::delete_event(unc::driver::controller *ctr,
                                        std::list<std::string> &switch_list) {
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  std::map<std::string, unc::vtndrvcache::ConfigNode *> cfg_node_delete_map;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());

  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    if (cfgnode_cache == NULL) {
      pfc_log_debug("cfgnode is NULL before get_type");
      return DRVAPI_RESPONSE_FAILURE;
    }
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    pfc_log_debug("key_type is %d", key_type);
    if (UNC_KT_SWITCH == key_type) {
      pfc_log_debug("Received is SW");
      unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, uint32_t>*> (cfgnode_cache);
      if (NULL == cfgptr_cache) {
        pfc_log_error("cfgptr_cache is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }
      key_switch_t *key_switch_cache = cfgptr_cache->get_key_structure();
      if (key_switch_cache == NULL) {
        pfc_log_error("Key sswitch cache is empty");
        return DRVAPI_RESPONSE_FAILURE;
      }
      std::string switch_id = reinterpret_cast<const char*>
          (key_switch_cache->switch_id);
      pfc_log_debug("SW id %s", switch_id.c_str());
      std::list <std::string>::iterator result = std::find(switch_list.begin(),
                                                 switch_list.end(), switch_id);
      if (result == switch_list.end()) {
        pfc_log_debug("Switch %s to be deleted", switch_id.c_str());
        cfg_node_delete_map[switch_id] = cfgnode_cache;
      }
    }
    if (UNC_KT_PORT == key_type) {
      pfc_log_debug("Received is PORT");
      unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, uint32_t>
          *cfgptr_cache_port = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, uint32_t>*> (cfgnode_cache);
      if (NULL == cfgptr_cache_port) {
        pfc_log_error("cfgptr_cache_port is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }
      key_port_t *key_port_cache = cfgptr_cache_port->get_key_structure();
      if (NULL == key_port_cache) {
        pfc_log_error("key_port_cache is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }
      std::string switch_port_id = reinterpret_cast<const char*>
                                  (key_port_cache->sw_key.switch_id);
      std::string port_id = reinterpret_cast<const char*>
                                  (key_port_cache->port_id);
      if (cfg_node_delete_map.find(switch_port_id) !=
          cfg_node_delete_map.end() ) {
        std::string logical_switch_map = "LP-";
        logical_switch_map.append(switch_port_id);
        logical_switch_map.append(port_id);
        pfc_log_debug("Switch PORT %s to be deleted", switch_port_id.c_str());
        cfg_node_delete_map[logical_switch_map] = cfgnode_cache;
      }
    }
  }

  drv_resp_code_t  ret_val =
        delete_logical_port(ctr, cfg_node_delete_map);
  if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
    pfc_log_error("Error in deleting logical port");
    return DRVAPI_RESPONSE_FAILURE;
  }
  ret_val = delete_switch(ctr, cfg_node_delete_map);
  return ret_val;
}

// Send Delete notification to logical port
drv_resp_code_t OdcSwitch::delete_logical_port(unc::driver::controller *ctr,
       const std::map<std::string,
       unc::vtndrvcache::ConfigNode *> &cfg_node_delete_map) {
  int cfg_size = cfg_node_delete_map.size();
  pfc_log_debug("SIZE cfg_node_delete_map %d",  cfg_size);
  std::map<std::string, unc::vtndrvcache::ConfigNode *>::
                                  const_iterator iterator;

  for (iterator = cfg_node_delete_map.begin();
      iterator != cfg_node_delete_map.end(); ++iterator) {
    unc::vtndrvcache::ConfigNode *cfg_node = iterator->second;
    if (cfg_node == NULL) {
      pfc_log_error("cfgnode is NULL before get_type");
      return DRVAPI_RESPONSE_FAILURE;
    }
    std::string switch_id = iterator->first;
    pfc_log_debug("To be deleted %s", switch_id.c_str());
    if (switch_id.compare(0, 3, "LP-") == 0) {
      unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, uint32_t>*> (cfg_node);
      if (NULL == cfgptr_cache) {
        pfc_log_error("cfgptr_cache is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }
      key_port_t *key_port = cfgptr_cache->get_key_structure();
      val_port_st_t *val_port = cfgptr_cache->get_val_structure();
      if ((NULL == key_port) || (NULL == val_port)) {
        pfc_log_error("key_port/val_port is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }

      OdcPort odc_port_obj(conf_file_values_);
      // Send notification to UPPL
      odc_port_obj.notify_logical_port_physical(unc::driver::VTN_LP_DELETE,
                                                key_port, val_port, NULL);
    }
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

// Send Delete notification to Switch
drv_resp_code_t OdcSwitch::delete_switch(unc::driver::controller *ctr,
                                         const std::map<std::string,
                    unc::vtndrvcache::ConfigNode *> &cfg_node_delete_map) {
  std::map<std::string, unc::vtndrvcache::ConfigNode *>::
      const_iterator iterator;

  for (iterator = cfg_node_delete_map.begin();
       iterator != cfg_node_delete_map.end(); ++iterator) {
    unc::vtndrvcache::ConfigNode *cfg_node = iterator->second;
    if (cfg_node == NULL) {
      pfc_log_error("cfgnode is NULL before get_type");
      return DRVAPI_RESPONSE_FAILURE;
    }
    std::string switch_id = iterator->first;
    if (switch_id.compare(0, 3, "LP-") != 0) {
      unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, uint32_t> *cfgptr_cache_sw =
          static_cast<unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, uint32_t>*> (cfg_node);
      if (NULL == cfgptr_cache_sw) {
        pfc_log_error("cfgptr_cache_sw is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }
      key_switch_t *key_switch = cfgptr_cache_sw->get_key_structure();
      val_switch_st_t *val_switch = cfgptr_cache_sw->get_val_structure();
      if ((NULL == key_switch) || (NULL == val_switch)) {
        pfc_log_error("key_switch/val_switch is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }
      // Send notification to UPPL
      notify_physical(unc::driver::VTN_SWITCH_DELETE, key_switch,
                      val_switch, NULL);

      // Delete from cache
      drv_resp_code_t  ret_val =
          ctr->physical_port_cache->delete_physical_attribute_node(cfg_node);
      if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
        pfc_log_error(" Error in deleting in cache");
        return ret_val;
      }
    }
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

//  Verify in cache whether switch aleady exists or not
drv_resp_code_t OdcSwitch::verify_in_cache(unc::driver::controller *ctr_ptr,
                std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
                                      std::list<std::string> &switch_list) {
  ODC_FUNC_TRACE;
  for (
      std::vector<unc::vtndrvcache::ConfigNode *>::iterator ctr_iterator =
      cfgnode_vector.begin();
      ctr_iterator != cfgnode_vector.end();
      ctr_iterator++) {
    unc::vtndrvcache::ConfigNode *cfg_node = *ctr_iterator;
    if (cfg_node == NULL) {
      pfc_log_error("cfgnode is NULL before get_type");
      return DRVAPI_RESPONSE_FAILURE;
    }

    unc::vtndrvcache::CacheElementUtil<key_switch_t, val_switch_st_t, uint32_t>
        *cfgnode_ctr = static_cast<unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, uint32_t>*> (cfg_node);

    val_switch_st_t *val_switch_ctr = cfgnode_ctr->get_val_structure();

    unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
    pfc_bool_t exist_in_cache =
        ctr_ptr->physical_port_cache->compare_is_physical_node_found(
            cfgnode_ctr, cfgnode_cache);
    if (PFC_FALSE == exist_in_cache) {
      pfc_log_debug("value not exists in cache");
      drv_resp_code_t  ret_val = add_event(ctr_ptr, cfgnode_ctr, switch_list);
      if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
        pfc_log_error("Error occured in adding to cache");
        return ret_val;
      }
    } else {
      pfc_log_debug("value already exists in cache");
      unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, uint32_t>*> (cfgnode_cache);

      key_switch_t *key_switch = cfgptr_cache->get_key_structure();
      val_switch_st_t *val_switch_cache = cfgptr_cache->get_val_structure();

      pfc_bool_t sw_update = is_switch_modified(val_switch_ctr,
                                                val_switch_cache);
      if (sw_update == PFC_TRUE) {
        pfc_log_debug("Switch is modifed");
        drv_resp_code_t  ret_val =
            update_event(ctr_ptr, cfgnode_ctr, val_switch_cache, switch_list);
        if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
          pfc_log_error("error in updating the cache");
          return ret_val;
        }
      } else {
        pfc_log_debug("Already exists in cache not modified");
        update_list(key_switch, switch_list);
      }
      delete_config_node(cfg_node);
    }
  }
  // Check for delete
  drv_resp_code_t  ret_val = delete_event(ctr_ptr, switch_list);
  pfc_log_debug("Response returned for delete event is %d", ret_val);
  return ret_val;
}

// Deletes config node
void OdcSwitch::delete_config_node(unc::vtndrvcache::ConfigNode *cfg_node) {
  ODC_FUNC_TRACE;
  if (cfg_node != NULL) {
    delete cfg_node;
    cfg_node = NULL;
  }
}

// Check whether any attributes for existing switch is modified
pfc_bool_t OdcSwitch::is_switch_modified(val_switch_st_t *val_switch_ctr,
                                         val_switch_st_t *val_switch_cache) {
  ODC_FUNC_TRACE;
  std::string desc_ctr = reinterpret_cast<const char*>
                                 (val_switch_ctr->switch_val.description);
  std::string desc_cache = reinterpret_cast<const char*>
                                 (val_switch_cache->switch_val.description);
  std::string domain_ctr = reinterpret_cast<const char*>
                                 (val_switch_ctr->switch_val.domain_name);
  std::string domain_cache = reinterpret_cast<const char*>
                                 (val_switch_cache->switch_val.domain_name);

  if ((desc_ctr.compare(desc_cache) != 0) ||
      (domain_ctr.compare(domain_cache) != 0)) {
    return PFC_TRUE;
  } else {
    return PFC_FALSE;
  }
}

// parsing function for converting controller response to driver format
drv_resp_code_t OdcSwitch::parse_node_response(unc::driver::controller *ctr_ptr,
     char *data, std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_type error");
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t array_length =0;
  json_object *json_obj_node_prop = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(jobj, "nodeProperties", -1,
                                                     json_obj_node_prop);

  if ((DRVAPI_RESPONSE_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_node_prop, json_type_null))) {
    json_object_put(jobj);
    pfc_log_error("Parsing Error json_obj_node_prop is null");
    return DRVAPI_RESPONSE_FAILURE;
  }

  if (json_object_is_type(json_obj_node_prop, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj,
                                                  "nodeProperties");
  }
  pfc_log_debug("nodes array length : %d", array_length);
  if (0 == array_length) {
    pfc_log_debug("No SWITCH present");
    json_object_put(jobj);
    return DRVAPI_RESPONSE_SUCCESS;
  }
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    drv_resp_code_t ret_val = fill_config_node_vector(ctr_ptr,
                             json_obj_node_prop, arr_idx, cfgnode_vector);
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
