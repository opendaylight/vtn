/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_link.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcLink::OdcLink(unc::restjson::ConfFileValues_t conf_values)
: conf_file_values_(conf_values) {
  ODC_FUNC_TRACE;
}

// Destructor
OdcLink::~OdcLink() {
}


// Get link information in the controller
drv_resp_code_t OdcLink::fetch_config(
    unc::driver::controller *ctr_ptr, const pfc_bool_t &cache_empty) {
  ODC_FUNC_TRACE;
  PFC_VERIFY(ctr_ptr != NULL);
  std::vector<unc::vtndrvcache::ConfigNode *> cfgnode_vector;

  std::string url = "";
  url.append(BASE_TOPO_URL);
  url.append(CONTAINER_NAME);
  url.append(SLASH);

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
      pfc_log_debug("All Linkes : %s", data);
      drv_resp_code_t ret_val =  parse_link_response(
                              ctr_ptr, data, cfgnode_vector);
      pfc_log_debug("Number of LINK present, %d",
                    static_cast<int>(cfgnode_vector.size()));
      if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
        pfc_log_error("Error occured while parsing");
        return ret_val;
      }
      ret_val = compare_with_cache(ctr_ptr, cache_empty, cfgnode_vector);
      pfc_log_debug("Response from compare_with_cache is %d", ret_val);
      return ret_val;
    }
  }
  pfc_log_error("Response data is NULL");
  return DRVAPI_RESPONSE_FAILURE;;
}

// parse each LINK append to map maintained
drv_resp_code_t OdcLink::fill_edge_value_map(
    json_object *json_obj_node_prop,
    int arr_idx, std::map<std::string, std::string> &edge_prop_map,
    std::map<std::string, std::string> &head_conn_map) {
  ODC_FUNC_TRACE;

  json_object *json_obj_edge = NULL;
  uint32_t ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop, "edge",
                                                     arr_idx, json_obj_edge);
  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_edge, json_type_null))) {
    pfc_log_debug(" Error while parsing edge or json type NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  json_object *json_obj_tail_conn = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_edge, "tailNodeConnector",
                                            -1, json_obj_tail_conn);
  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_tail_conn, json_type_null))) {
    pfc_log_debug(" Error while parsing tail connector or json type NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  json_object *json_obj_tail_node = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_tail_conn, "node",
                                            -1, json_obj_tail_node);
  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_tail_node, json_type_null))) {
    pfc_log_debug(" Error while parsing node or json_node NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  std::string tail_node_id = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_tail_node, "id",
                                            -1, tail_node_id);

  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error while parsing node_id");
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("tail_node_id %s", tail_node_id.c_str());
  std::string tail_id   = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_tail_conn, "id",
                                            -1, tail_id);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_debug(" Error while parsing type id");
    return DRVAPI_RESPONSE_FAILURE;
  }

  json_object *json_obj_head_conn = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_edge, "headNodeConnector",
                                                -1, json_obj_head_conn);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_head_conn, json_type_null))) {
    pfc_log_debug(" Error while parsing head connector or json type NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  std::string head_id = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_head_conn, "id",
                                                    -1, head_id);

  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error while parsing head conn id");
    return DRVAPI_RESPONSE_FAILURE;
  }
  json_object *json_obj_head_node = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_head_conn, "node",
                                                        -1, json_obj_head_node);
  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_head_node, json_type_null))) {
    pfc_log_debug(" Error while parsing head connector node or json type NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  std::string head_node_id = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_head_node, "id",
                                            -1, head_node_id);

  pfc_log_debug("head_node_id %s", head_node_id.c_str());
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error while parsing head conn id");
    return DRVAPI_RESPONSE_FAILURE;
  }

  json_object *json_obj_prop = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop, "properties",
                                            arr_idx, json_obj_prop);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_prop, json_type_null))) {
    pfc_log_debug(" Error while parsing json_obj_prop or json type NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  json_object *json_obj_prop_name = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_prop, "name",
                                            -1, json_obj_prop_name);
  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_prop_name, json_type_null))) {
    pfc_log_debug(" Error while parsing json_obj_prop_name or json type NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  std::string prop_name = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_prop_name, "value",
                                            -1, prop_name);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error while parsing prop name value");
    return DRVAPI_RESPONSE_FAILURE;
  }

  json_object *json_obj_prop_state = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_prop, "state",
                                            -1, json_obj_prop_state);
  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_prop_state, json_type_null))) {
    pfc_log_debug(" Error while parsing json_obj_prop_state or json type NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  uint state_value = 0;
  ret_val = restjson::JsonBuildParse::parse(json_obj_prop_state, "value",
                                            -1, state_value);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error while parsing prop state value");
    return DRVAPI_RESPONSE_FAILURE;
  }

  pfc_log_debug("state value %d", state_value);
  json_object *json_obj_prop_config = NULL;
  ret_val = restjson::JsonBuildParse::parse(json_obj_prop, "config",
                                            -1, json_obj_prop_config);
  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (json_object_is_type(json_obj_prop_config, json_type_null))) {
    pfc_log_debug(" Error while parsing json_obj_prop_config"
                  "or json type NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  uint config_value = 0;
  ret_val = restjson::JsonBuildParse::parse(json_obj_prop_config, "value",
                                            -1, config_value);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error while parsing prop config value");
    return DRVAPI_RESPONSE_FAILURE;
  }
  pfc_log_debug("config value %d", config_value);

  std::string tail_conn_prop = "";
  tail_conn_prop.append(tail_node_id);
  tail_conn_prop.append(PIPE_SEPARATOR);
  tail_conn_prop.append(tail_id);

  std::string head_conn_prop = "";
  head_conn_prop.append(head_node_id);
  head_conn_prop.append(PIPE_SEPARATOR);
  head_conn_prop.append(head_id);

  // head_conn_map contains details in the format sample
  // head_conn_map["00:00:00:00:00:00:00:03|3"]="s3-eth3" used to map the port
  // of tail node connector key is the combination of headnodeid, head id. Value
  // is the combination of name value
  head_conn_map[head_conn_prop] = prop_name;

  pfc_log_debug("Entry in head_conn_map is key : %s, val : %s",
                head_conn_prop.c_str(), prop_name.c_str());

  head_conn_prop.append(PIPE_SEPARATOR);
  head_conn_prop.append(prop_name);
  head_conn_prop.append(PIPE_SEPARATOR);
  std::ostringstream str_state_val;
  str_state_val << state_value;
  head_conn_prop.append(str_state_val.str());
  head_conn_prop.append(PIPE_SEPARATOR);

  std::ostringstream str_conf_val;
  str_conf_val << config_value;
  head_conn_prop.append(str_conf_val.str());

  // edge_prop_map contains details in the format
  // edge_prop_map["00:00:00:00:00:00:00:01|2"]=
  // "00:00:00:00:00:00:00:03|3|s3-eth3|1|1"
  // key is the combination of tailnodeid and id, value is the combinationation
  // of headnodeid, head id, namevalue, state value, conf value
  edge_prop_map[tail_conn_prop] = head_conn_prop;
  pfc_log_debug("Entry in edge prop map is key : %s, val : %s",
                tail_conn_prop.c_str(), head_conn_prop.c_str());
  return DRVAPI_RESPONSE_SUCCESS;
}

//  Compare whether cache empty or not
drv_resp_code_t OdcLink::compare_with_cache(unc::driver::controller *ctr_ptr,
                                               const pfc_bool_t &cache_empty,
               std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  if (ctr_ptr->physical_port_cache == NULL) {
    pfc_log_error("cache pointer is empty");
    return DRVAPI_RESPONSE_FAILURE;
  }
  std::list<std::string> link_list;
  drv_resp_code_t ret_val = DRVAPI_RESPONSE_FAILURE;
  if (PFC_TRUE == cache_empty) {
    for (std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
         cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it) {
      unc::vtndrvcache::ConfigNode *cfgnode_cache = *it;
      if (cfgnode_cache == NULL) {
        pfc_log_error("cfgnode_cache is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }
      ret_val = add_event(ctr_ptr, cfgnode_cache, link_list);
      if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
        pfc_log_error("Error in adding to cache");
        return ret_val;
      }
    }
  } else {
    ret_val = verify_in_cache(ctr_ptr, cfgnode_vector, link_list);
    pfc_log_debug("Response for verify_in_cache is %d", ret_val);
    return ret_val;
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

//  Notify physical about the link event
void OdcLink::notify_physical(unc::driver::oper_type type,
                                key_link_t *key_link,
                                val_link_st_t *val_link,
                                val_link_st_t *val_old_link) {
  ODC_FUNC_TRACE;
  unc::driver::VtnDrvIntf* disp_inst = static_cast<unc::driver::VtnDrvIntf*>
      (pfc::core::Module::getInstance("vtndrvintf"));
  PFC_VERIFY(disp_inst != NULL);
  if (type == unc::driver::VTN_LINK_UPDATE) {
    disp_inst->link_event(unc::driver::VTN_LINK_UPDATE, *key_link,
                            *val_link, *val_old_link);
  } else {
    disp_inst->link_event(type, *key_link, *val_link);
  }
}

//  Update the local list maintained for delete
void OdcLink::update_list(key_link_t *key_link,
                            std::list<std::string> &link_list) {
  ODC_FUNC_TRACE;
  std::string switch_id1 = reinterpret_cast<const char*> (key_link->switch_id1);
  std::string switch_id2 = reinterpret_cast<const char*> (key_link->switch_id2);
  std::string port_id1 = reinterpret_cast<const char*> (key_link->port_id1);
  std::string port_id2 = reinterpret_cast<const char*> (key_link->port_id2);

  std::string entry_in_list = "";
  entry_in_list.append(switch_id1);
  entry_in_list.append(PIPE_SEPARATOR);
  entry_in_list.append(port_id1);
  entry_in_list.append(PIPE_SEPARATOR);
  entry_in_list.append(switch_id2);
  entry_in_list.append(PIPE_SEPARATOR);
  entry_in_list.append(port_id2);

  pfc_log_debug("link updated in list %s", entry_in_list.c_str());
  std::list <std::string>::iterator result = std::find(
      link_list.begin(), link_list.end(), entry_in_list);

  // Push into list only if the entry not exist already
  // Entry is in the form of "switch_id1|port_id1switch_id2|port_id2"
  if (result == link_list.end()) {
    link_list.push_front(entry_in_list);
  }
}

//  Add event for Link
drv_resp_code_t OdcLink::add_event(unc::driver::controller *ctr_ptr,
                                     unc::vtndrvcache::ConfigNode *cfg_node,
                                     std::list<std::string> &link_list) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::CacheElementUtil<key_link_t, val_link_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_link_t, val_link_st_t, uint32_t>*> (cfg_node);
  key_link_t *key_link = cfgptr_cache->get_key_structure();
  val_link_st_t *val_link = cfgptr_cache->get_val_structure();

  if ((NULL == key_link) || (NULL == val_link)) {
    pfc_log_error("key_link/val_link is NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }
  //  Send Notification to UPPL
  notify_physical(unc::driver::VTN_LINK_CREATE,
                  key_link, val_link, NULL);

  //  Append to cache
  drv_resp_code_t  ret_val =
      ctr_ptr->physical_port_cache->append_Physical_attribute_node(cfg_node);
  if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
    pfc_log_error(" Error in adding to cache");
    delete_config_node(cfg_node);
    return ret_val;
  }
  pfc_log_debug("Update the list maintained");
  update_list(key_link, link_list);
  return ret_val;
}

// Update event for Link
drv_resp_code_t OdcLink::update_event(unc::driver::controller *ctr_ptr,
                                        unc::vtndrvcache::ConfigNode *cfg_node,
                                        val_link_st_t *val_old_link,
                                        std::list<std::string> &link_list) {
  unc::vtndrvcache::CacheElementUtil<key_link_t, val_link_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_link_t, val_link_st_t, uint32_t>*> (cfg_node);
  key_link_t *key_link = cfgptr_cache->get_key_structure();
  val_link_st_t *val_link = cfgptr_cache->get_val_structure();

  if ((NULL == key_link) || (NULL == val_link) ||
      (NULL == val_old_link)) {
    pfc_log_error("key_link/val_link is NULL");
    return DRVAPI_RESPONSE_FAILURE;
  }

  // Send notification to UPPL
  notify_physical(unc::driver::VTN_LINK_UPDATE, key_link,
                 val_link, val_old_link);

  // Append to cache
  drv_resp_code_t  ret_val =
      ctr_ptr->physical_port_cache->update_physical_attribute_node(cfg_node);
  if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
    pfc_log_error(" Error in updating to cache");
    delete_config_node(cfg_node);
    return ret_val;
  }
  update_list(key_link, link_list);
  return ret_val;
}

// Delete event for Link
drv_resp_code_t OdcLink::delete_event(unc::driver::controller *ctr,
                                        std::list<std::string> &link_list) {
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
    if (UNC_KT_LINK == key_type) {
      pfc_log_debug("Received is Link");
      unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, uint32_t>*> (cfgnode_cache);
      key_link_t *key_link_cache = cfgptr_cache->get_key_structure();
      if (key_link_cache == NULL) {
        pfc_log_error("Key slink cache is empty");
        return DRVAPI_RESPONSE_FAILURE;
      }
      std::string switch_id1 =
          reinterpret_cast<const char*>(key_link_cache->switch_id1);
      std::string switch_id2 =
          reinterpret_cast<const char*>(key_link_cache->switch_id2);
      std::string port_id1 =
          reinterpret_cast<const char*>(key_link_cache->port_id1);
      std::string port_id2 =
          reinterpret_cast<const char*>(key_link_cache->port_id2);

      std::string link_to_search = "";
      link_to_search.append(switch_id1);
      link_to_search.append(PIPE_SEPARATOR);
      link_to_search.append(port_id1);
      link_to_search.append(PIPE_SEPARATOR);
      link_to_search.append(switch_id2);
      link_to_search.append(PIPE_SEPARATOR);
      link_to_search.append(port_id2);

      pfc_log_debug("link to search %s", link_to_search.c_str());
      std::list <std::string>::iterator result = std::find(link_list.begin(),
                                               link_list.end(), link_to_search);
      if (result == link_list.end()) {
        pfc_log_debug("Link %s to be deleted", link_to_search.c_str());
        cfg_node_delete_map[link_to_search] = cfgnode_cache;
      }
    }
  }
  drv_resp_code_t ret_val = delete_link(ctr, cfg_node_delete_map);
  return ret_val;
}

// Send Delete notification to Link
drv_resp_code_t OdcLink::delete_link(unc::driver::controller *ctr,
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
    std::string link_id = iterator->first;
      unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, uint32_t> *cfgptr_cache =
          static_cast<unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, uint32_t>*> (cfg_node);

      key_link_t *key_link = cfgptr_cache->get_key_structure();
      val_link_st_t *val_link = cfgptr_cache->get_val_structure();
      if ((NULL == key_link) || (NULL == val_link)) {
        pfc_log_error("key_link/val_link is NULL");
        return DRVAPI_RESPONSE_FAILURE;
      }
      // Send notification to UPPL
      notify_physical(unc::driver::VTN_LINK_DELETE, key_link,
                      val_link, NULL);

      // Delete from cache
      drv_resp_code_t  ret_val =
          ctr->physical_port_cache->delete_physical_attribute_node(cfg_node);
      if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
        pfc_log_error(" Error in deleting in cache");
        return ret_val;
      }
    }
  return DRVAPI_RESPONSE_SUCCESS;
}

//  Verify in cache whether link aleady exists or not
drv_resp_code_t OdcLink::verify_in_cache(unc::driver::controller *ctr_ptr,
                std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
                                      std::list<std::string> &link_list) {
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

    unc::vtndrvcache::CacheElementUtil<key_link_t, val_link_st_t, uint32_t>
        *cfgnode_ctr = static_cast<unc::vtndrvcache::CacheElementUtil
        <key_link_t, val_link_st_t, uint32_t>*> (cfg_node);

    val_link_st_t *val_link_ctr = cfgnode_ctr->get_val_structure();

    unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
    pfc_bool_t exist_in_cache =
        ctr_ptr->physical_port_cache->compare_is_physical_node_found(
            cfgnode_ctr, cfgnode_cache);
    if (PFC_FALSE == exist_in_cache) {
      pfc_log_debug("value not exists in cache");
      drv_resp_code_t  ret_val = add_event(ctr_ptr, cfgnode_ctr, link_list);
      if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
        pfc_log_error("Error occured in adding to cache");
        return ret_val;
      }
    } else {
      pfc_log_debug("value already exists in cache");
      unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, uint32_t>*> (cfgnode_cache);

      key_link_t *key_link = cfgptr_cache->get_key_structure();
      val_link_st_t *val_link_cache = cfgptr_cache->get_val_structure();

      pfc_bool_t link_update = is_link_modified(val_link_ctr,
                                                val_link_cache);
      if (link_update == PFC_TRUE) {
        pfc_log_debug("Link is modifed");
        drv_resp_code_t  ret_val =
            update_event(ctr_ptr, cfgnode_ctr, val_link_cache, link_list);
        if (ret_val != DRVAPI_RESPONSE_SUCCESS) {
          pfc_log_error("error in updating the cache");
          return ret_val;
        }
      } else {
        pfc_log_debug("Already exists in cache not modified");
        update_list(key_link, link_list);
      }
      delete_config_node(cfg_node);
    }
  }
  // Check for delete
  drv_resp_code_t  ret_val = delete_event(ctr_ptr, link_list);
  pfc_log_debug("Response returned for delete event is %d", ret_val);
  return ret_val;
}

// Deletes config node
void OdcLink::delete_config_node(unc::vtndrvcache::ConfigNode *cfg_node) {
  ODC_FUNC_TRACE;
  if (cfg_node != NULL) {
    delete cfg_node;
    cfg_node = NULL;
  }
}

// Check whether any attributes for existing link is modified
pfc_bool_t OdcLink::is_link_modified(val_link_st_t *val_link_ctr,
                                         val_link_st_t *val_link_cache) {
  ODC_FUNC_TRACE;

  if (val_link_ctr->oper_status != val_link_cache->oper_status) {
    return PFC_TRUE;
  } else {
    return PFC_FALSE;
  }
}

// Fills config node vector with link
drv_resp_code_t OdcLink::fill_config_node_vector(
              unc::driver::controller *ctr_ptr,
              const std::map<std::string, std::string> &edge_prop_map,
              const std::map<std::string, std::string> &head_conn_map,
              std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  for (std::map<std::string, std::string>::const_iterator it
      = edge_prop_map.begin(); it != edge_prop_map.end(); ++it) {
    std::string tail_node_conn = it->first;
    std::string head_node_conn = it->second;
    pfc_log_debug("Values tail %s head %s", tail_node_conn.c_str(),
                  head_node_conn.c_str());
    std::map<std::string, std::string>::const_iterator it_head_node;
    it_head_node = head_conn_map.find(tail_node_conn);

    std::string port_id1 = "";
    if (it_head_node != head_conn_map.end()) {
      port_id1 = it_head_node->second;
    } else {
      pfc_log_error("No corresponding port available . Error occured");
      return DRVAPI_RESPONSE_FAILURE;
    }
    size_t pos_sw1 = tail_node_conn.find(PIPE_SEPARATOR);
    if (pos_sw1 == std::string::npos) {
      pfc_log_error("Error in tail_node_conn value");
      return DRVAPI_RESPONSE_FAILURE;
    }
    std::string switch_id1 = tail_node_conn.substr(0, pos_sw1);
    std::string switch_id2 = "";
    std::string port_id2 = "";
    std::string state_value = "";
    std::string conf_value = "";
    char *save_ptr;
    char *head_node_char = const_cast<char *> (head_node_conn.c_str());
    char *tokener = strtok_r(head_node_char, "|", &save_ptr);
    int parse_occurence = 0;
    while (tokener != NULL) {
      switch (parse_occurence) {
        case 0:
          switch_id2 = tokener;
          break;
        case 2:
          port_id2 = tokener;
          break;
        case 3:
          state_value = tokener;
          break;
        case 4:
          conf_value = tokener;
          break;
        default:
          pfc_log_debug("Value is not relevant %d", parse_occurence);
          break;
      }
      parse_occurence++;
      tokener = strtok_r(NULL, "|", &save_ptr);
    }
    if ((switch_id1.empty()) || (switch_id2.empty()) ||
        (port_id1.empty()) || (port_id2.empty())) {
      pfc_log_error("switch id or port id is empty");
      return DRVAPI_RESPONSE_FAILURE;
    }
    pfc_log_debug("SW1 : %s ,PORT1: %s, SW2: %s,PORT2: %s", switch_id1.c_str(),
                  port_id1.c_str(), switch_id2.c_str(), port_id2.c_str());
    key_link_t key_link;
    val_link_st_t val_link;
    memset(&key_link, 0, sizeof(key_link_t));
    memset(&val_link, 0, sizeof(val_link_st_t));

    // Fills Key structure
    strncpy(reinterpret_cast<char*>
        (key_link.switch_id1), switch_id1.c_str(), strlen(switch_id1.c_str()));
    strncpy(reinterpret_cast<char*>
        (key_link.switch_id2), switch_id2.c_str(), strlen(switch_id2.c_str()));
    strncpy(reinterpret_cast<char*>
        (key_link.port_id1), port_id1.c_str(), strlen(port_id1.c_str()));
    strncpy(reinterpret_cast<char*>
        (key_link.port_id2), port_id2.c_str(), strlen(port_id2.c_str()));

    std::string ctr_name = ctr_ptr->get_controller_id();
    strncpy(reinterpret_cast<char*> (key_link.ctr_key.controller_name),
                                ctr_name.c_str(), strlen(ctr_name.c_str()));

    // Fills Val Structure
    uint state_value_int = atoi(state_value.c_str());
    uint conf_value_int = atoi(conf_value.c_str());
    val_link.valid[VAL_LINK_STRUCT_ATTR] = UNC_VF_VALID;
    if (unc::driver::CONNECTION_UP == ctr_ptr->get_connection_status()) {
      if ((ADMIN_UP == conf_value_int) &&
          (EDGE_UP == state_value_int)) {
        pfc_log_debug("oper status of port is UP");
        val_link.oper_status = UPPL_LINK_OPER_UP;
        val_link.valid[VAL_OPERSTATUS_ATTR] = UNC_VF_VALID;
      } else {
        pfc_log_debug("oper status of port is DOWN");
        val_link.oper_status = UPPL_LINK_OPER_DOWN;
        val_link.valid[VAL_OPERSTATUS_ATTR] = UNC_VF_VALID;
      }
    } else {
      pfc_log_debug("oper status of port is UNKNOWN");
      val_link.oper_status = UPPL_LINK_OPER_UNKNOWN;
      val_link.valid[VAL_OPERSTATUS_ATTR] = UNC_VF_VALID;
    }

    unc::vtndrvcache::ConfigNode *cfgptr =
    new unc::vtndrvcache::CacheElementUtil<key_link_t, val_link_st_t, uint32_t>
        (&key_link, &val_link, uint32_t(UNC_OP_READ));
    PFC_VERIFY(cfgptr != NULL);
    cfgnode_vector.push_back(cfgptr);
  }
  return DRVAPI_RESPONSE_SUCCESS;
}

// parsing function for converting controller response to driver format
drv_resp_code_t OdcLink::parse_link_response(unc::driver::controller *ctr_ptr,
     char *data, std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_type error");
    return DRVAPI_RESPONSE_FAILURE;
  }
  uint32_t array_length =0;
  json_object *json_obj_edge_prop = NULL;
  int parse_ret_val = restjson::JsonBuildParse::parse(jobj, "edgeProperties",
                                                      -1, json_obj_edge_prop);

  if ((restjson::REST_OP_SUCCESS != parse_ret_val) ||
      (json_object_is_type(json_obj_edge_prop, json_type_null))) {
    json_object_put(jobj);
    pfc_log_error("Parsing Error json_obj_edge_prop is null");
    return DRVAPI_RESPONSE_FAILURE;
  }

  if (json_object_is_type(json_obj_edge_prop, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj,
                                                              "edgeProperties");
  }
  pfc_log_debug("edges array length : %d", array_length);
  if (0 == array_length) {
    pfc_log_debug("No EDGE present");
    json_object_put(jobj);
    return DRVAPI_RESPONSE_SUCCESS;
  }
  drv_resp_code_t ret_val = DRVAPI_RESPONSE_FAILURE;
  std::map<std::string, std::string> edge_prop_map;
  std::map<std::string, std::string> head_conn_map;
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    ret_val = fill_edge_value_map(
        json_obj_edge_prop, arr_idx, edge_prop_map, head_conn_map);
    if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
      json_object_put(jobj);
      pfc_log_error("Error return from fill map failure");
      return ret_val;
    }
  }

  ret_val = fill_config_node_vector
      (ctr_ptr, edge_prop_map, head_conn_map, cfgnode_vector);
  if (DRVAPI_RESPONSE_SUCCESS != ret_val) {
    json_object_put(jobj);
    pfc_log_error("Error return from fill config node_vector failure");
    return ret_val;
  }
  json_object_put(jobj);
  return DRVAPI_RESPONSE_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
