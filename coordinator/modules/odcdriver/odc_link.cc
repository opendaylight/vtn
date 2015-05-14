/*
 * Copyright (c) 2014-2015 NEC Corporation
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
  pfc_log_info("odclink link_map cleared");
  link_map_.clear();
}


// Get link information in the controller
UncRespCode OdcLink::fetch_config(
    unc::driver::controller *ctr_ptr,
    const pfc_bool_t &cache_empty) {
  ODC_FUNC_TRACE;
  PFC_VERIFY(ctr_ptr != NULL);
  std::vector<unc::vtndrvcache::ConfigNode *> cfgnode_vector;

  std::string url = "";
  url.append(RESTCONF_BASE);
  url.append(VTN_TOPO_URL);

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
      pfc_log_trace("All Linkes : %s", data);
      UncRespCode ret_val =  parse_link_response(
                              ctr_ptr, data, cfgnode_vector);
      pfc_log_debug("Number of LINK present, %d",
                    static_cast<int>(cfgnode_vector.size()));
      if (UNC_RC_SUCCESS != ret_val) {
        pfc_log_error("Error occured while parsing");
        return ret_val;
      }
      ret_val = compare_with_cache(ctr_ptr, cache_empty, cfgnode_vector);
      pfc_log_debug("Response from compare_with_cache is %d", ret_val);
      return ret_val;
    }
  }
  pfc_log_error("Response data is NULL");
  return UNC_DRV_RC_ERR_GENERIC;
}

// parse each LINK append to map maintained
UncRespCode OdcLink::fill_edge_value_map(
    json_object *json_obj_node_prop,
    int arr_idx,
    unc::driver::controller *ctr_ptr,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector) {
  ODC_FUNC_TRACE;

  std::string source_node_id = "";
  uint32_t ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop,
                                                     "source",
                                                     arr_idx,
                                                     source_node_id);
  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (source_node_id.empty())) {
    pfc_log_error(" Error while parsing source node id");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  std::string dst_node_id = "";
  ret_val = restjson::JsonBuildParse::parse(json_obj_node_prop,
                                            "destination",
                                            arr_idx,
                                            dst_node_id);

  if ((restjson::REST_OP_SUCCESS != ret_val) ||
      (dst_node_id.empty())) {
    pfc_log_error("Error while parsing destination node_id");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string dst_prob = "";
  std::string source_prob = "";
  std::map<std::string, std::string>::iterator iter;
  pfc_log_info("source_node_id %s", source_node_id.c_str());
  iter = link_map_.find(source_node_id);
  if (iter != link_map_.end()) {
      source_prob = iter->second;
  } else {
    pfc_log_error("Error while parsing link map source node");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  pfc_log_info("dst_node_id %s", dst_node_id.c_str());
  iter = link_map_.find(dst_node_id);
  if (iter != link_map_.end()) {
      dst_prob = iter->second;
  } else {
    pfc_log_error("Error while parsing link map dst node");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("Entry in link map source is  val : %s", source_prob.c_str());
  pfc_log_info("Entry in link map dst is  val : %s", dst_prob.c_str());
  ret_val = fill_config_node_vector
      (ctr_ptr, source_prob, dst_prob, cfg_node_vector);

  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error return from fill config node_vector failure");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}

//  Compare whether cache empty or not
UncRespCode OdcLink::compare_with_cache(
    unc::driver::controller *ctr_ptr,
    const pfc_bool_t &cache_empty,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  if (NULL == ctr_ptr->physical_port_cache) {
    pfc_log_error("cache pointer is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::list<std::string> link_list;
  UncRespCode ret_val = UNC_DRV_RC_ERR_GENERIC;
  if (PFC_TRUE == cache_empty) {
    for (std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
         cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it) {
      unc::vtndrvcache::ConfigNode *cfgnode_cache = *it;
      if (NULL == cfgnode_cache) {
        pfc_log_error("cfgnode_cache is NULL");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      ret_val = add_event(ctr_ptr, cfgnode_cache, link_list);
      if (UNC_RC_SUCCESS != ret_val) {
        pfc_log_error("Error in adding to cache");
        return ret_val;
      }
    }
  } else {
    ret_val = verify_in_cache(ctr_ptr, cfgnode_vector, link_list);
    return ret_val;
  }
  return UNC_RC_SUCCESS;
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
    disp_inst->link_event(unc::driver::VTN_LINK_UPDATE,
                          *key_link,
                          *val_link,
                          *val_old_link);
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

  pfc_log_trace("link updated in list %s", entry_in_list.c_str());
  std::list <std::string>::iterator result = std::find(
      link_list.begin(), link_list.end(), entry_in_list);

  // Push into list only if the entry not exist already
  // Entry is in the form of "switch_id1|port_id1switch_id2|port_id2"
  if (result == link_list.end()) {
    link_list.push_front(entry_in_list);
  }
}

//  Add event for Link
UncRespCode OdcLink::add_event(unc::driver::controller *ctr_ptr,
                                   unc::vtndrvcache::ConfigNode *cfg_node,
                                   std::list<std::string> &link_list) {
  ODC_FUNC_TRACE;

  unc::vtndrvcache::CacheElementUtil<key_link_t, val_link_st_t, val_link_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_link_t, val_link_st_t, val_link_st_t, uint32_t>*> (cfg_node);

  key_link_t *key_link = cfgptr_cache->get_key_structure();
  val_link_st_t *val_link = cfgptr_cache->get_val_structure();

  if ((NULL == key_link) || (NULL == val_link)) {
    pfc_log_error("key_link/val_link is NULL");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  //  Send Notification to UPPL
  notify_physical(unc::driver::VTN_LINK_CREATE,
                  key_link,
                  val_link,
                  NULL);

  //  Append to cache
  UncRespCode  ret_val =
      ctr_ptr->physical_port_cache->append_Physical_attribute_node(cfg_node);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error(" Error in adding to cache");
    delete_config_node(cfg_node);
    return ret_val;
  }
  update_list(key_link, link_list);
  return ret_val;
}

// Update event for Link
UncRespCode OdcLink::update_event(unc::driver::controller *ctr_ptr,
                                      unc::vtndrvcache::ConfigNode *cfg_node,
                                      val_link_st_t *val_new_link,
                                      std::list<std::string> &link_list) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::CacheElementUtil<key_link_t, val_link_st_t, val_link_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_link_t, val_link_st_t, val_link_st_t, uint32_t>*> (cfg_node);

  key_link_t *key_link = cfgptr_cache->get_key_structure();
  val_link_st_t *val_link = cfgptr_cache->get_val_structure();

  if ((NULL == key_link) || (NULL == val_link) ||
      (NULL == val_new_link)) {
    pfc_log_error("key_link/val_link is NULL");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  // Send notification to UPPL
  notify_physical(unc::driver::VTN_LINK_UPDATE,
                  key_link,
                  val_link,
                  val_new_link);

  // Append to cache
  UncRespCode  ret_val =
      ctr_ptr->physical_port_cache->update_physical_attribute_node(cfg_node);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error(" Error in updating to cache");
    delete_config_node(cfg_node);
    return ret_val;
  }
  update_list(key_link, link_list);
  return ret_val;
}

// Delete event for Link
UncRespCode OdcLink::delete_event(unc::driver::controller *ctr,
                                      std::list<std::string> &link_list) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  std::map<std::string, unc::vtndrvcache::ConfigNode *> cfg_node_delete_map;
  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr->physical_port_cache->create_iterator());

  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    if (NULL == cfgnode_cache) {
      pfc_log_error("cfgnode is NULL before get_type");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    if (UNC_KT_LINK == key_type) {
      unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, val_link_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, val_link_st_t, uint32_t>*> (cfgnode_cache);

      key_link_t *key_link_cache = cfgptr_cache->get_key_structure();
      if (NULL == key_link_cache) {
        pfc_log_error("Key slink cache is empty");
        return UNC_DRV_RC_ERR_GENERIC;
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

      pfc_log_trace("link to search %s", link_to_search.c_str());
      std::list <std::string>::iterator result = std::find(link_list.begin(),
                                               link_list.end(), link_to_search);
      if (result == link_list.end()) {
        pfc_log_debug("Link %s to be deleted", link_to_search.c_str());
        cfg_node_delete_map[link_to_search] = cfgnode_cache;
      }
    }
  }
  UncRespCode ret_val = delete_link(ctr, cfg_node_delete_map);
  return ret_val;
}

// Send Delete notification to Link
UncRespCode OdcLink::delete_link(
    unc::driver::controller *ctr,
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
    std::string link_id = iter->first;
      unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, val_link_st_t, uint32_t> *cfgptr_cache =
          static_cast<unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, val_link_st_t, uint32_t>*> (cfg_node);

      key_link_t *key_link = cfgptr_cache->get_key_structure();
      val_link_st_t *val_link = cfgptr_cache->get_val_structure();

      if ((NULL == key_link) || (NULL == val_link)) {
        pfc_log_error("key_link/val_link is NULL");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      // Send notification to UPPL
      notify_physical(unc::driver::VTN_LINK_DELETE,
                      key_link,
                      val_link,
                      NULL);

      // Delete from cache
      UncRespCode  ret_val =
          ctr->physical_port_cache->delete_physical_attribute_node(cfg_node);
      if (ret_val != UNC_RC_SUCCESS) {
        pfc_log_error(" Error in deleting in cache");
        return ret_val;
      }
    }
  return UNC_RC_SUCCESS;
}

//  Verify in cache whether link exists or not
UncRespCode OdcLink::verify_in_cache(
    unc::driver::controller *ctr_ptr,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
    std::list<std::string> &link_list) {
  ODC_FUNC_TRACE;
  for (
      std::vector<unc::vtndrvcache::ConfigNode *>::iterator ctr_iterator =
      cfgnode_vector.begin();
      ctr_iterator != cfgnode_vector.end();
      ctr_iterator++) {
    unc::vtndrvcache::ConfigNode *cfg_node = *ctr_iterator;
    if (NULL == cfg_node) {
      pfc_log_error("cfgnode is NULL before get_type");
      return UNC_DRV_RC_ERR_GENERIC;
    }

    unc::vtndrvcache::CacheElementUtil<key_link_t, val_link_st_t, val_link_st_t, uint32_t>
        *cfgnode_ctr = static_cast<unc::vtndrvcache::CacheElementUtil
        <key_link_t, val_link_st_t, val_link_st_t, uint32_t>*> (cfg_node);

    val_link_st_t *val_link_ctr = cfgnode_ctr->get_val_structure();
    key_link_t *key_link = cfgnode_ctr->get_key_structure();

    if ((NULL == key_link) || (NULL == val_link_ctr)) {
      pfc_log_error("key_link / val_link is NULL");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    std::string switch_id1 =
        reinterpret_cast<const char*>(key_link->switch_id1);
    std::string switch_id2 =
        reinterpret_cast<const char*>(key_link->switch_id2);
    std::string port_id1 =
        reinterpret_cast<const char*>(key_link->port_id1);
    std::string port_id2 =
        reinterpret_cast<const char*>(key_link->port_id2);

    pfc_log_debug("Link - SW1 : %s ,PORT1: %s, SW2: %s,PORT2: %s",
      switch_id1.c_str(), port_id1.c_str(),
      switch_id2.c_str(), port_id2.c_str());

    unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
    pfc_bool_t exist_in_cache =
        ctr_ptr->physical_port_cache->compare_is_physical_node_found(
                                          cfgnode_ctr, cfgnode_cache);
    if (PFC_FALSE == exist_in_cache) {
      pfc_log_trace("Link not exist in cache");
      UncRespCode  ret_val = add_event(ctr_ptr, cfgnode_ctr, link_list);
      if (ret_val != UNC_RC_SUCCESS) {
        pfc_log_error("Error occured in adding to cache");
        return ret_val;
      }
    } else {
      pfc_log_trace("Link exist in cache");
      unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, val_link_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_link_t, val_link_st_t, val_link_st_t, uint32_t>*> (cfgnode_cache);

      key_link_t *key_link = cfgptr_cache->get_key_structure();
      val_link_st_t *val_link_cache = cfgptr_cache->get_val_structure();

      pfc_bool_t link_update = is_link_modified(val_link_ctr,
                                                val_link_cache);
      if (PFC_TRUE == link_update) {
        pfc_log_trace("Link parameters are updated");
        UncRespCode  ret_val =
            update_event(ctr_ptr, cfgnode_ctr, val_link_cache, link_list);
        if (UNC_RC_SUCCESS != ret_val) {
          pfc_log_error("error in updating the cache");
          return ret_val;
        }
      } else {
        pfc_log_trace("Link Exists in cache not updated");
        update_list(key_link, link_list);
      }
      delete_config_node(cfg_node);
    }
  }
  // Check for delete
  UncRespCode  ret_val = delete_event(ctr_ptr, link_list);
  return ret_val;
}

// Deletes config node
void OdcLink::delete_config_node(unc::vtndrvcache::ConfigNode *cfg_node) {
  ODC_FUNC_TRACE;
  if (NULL != cfg_node) {
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
UncRespCode OdcLink::fill_config_node_vector(
    unc::driver::controller *ctr_ptr,
    std::string head_node_conn,
    std::string tail_node_conn,
    std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  std::string switch_id1 = "";
  std::string switch_id2 = "";
  std::string port_id1 = "";
  std::string port_id2 = "";
  std::string state_value1 = "";
  std::string state_value2 = "";
  pfc_log_info("head_node_conn entry:%s", head_node_conn.c_str());
  pfc_log_info("tail_node_conn entry:%s", tail_node_conn.c_str());
  parse_properties(head_node_conn, switch_id1, port_id1, state_value1);
  parse_properties(tail_node_conn, switch_id2, port_id2, state_value2);
  pfc_log_trace("SW1 : %s ,PORT1: %s, SW2: %s,PORT2: %s", switch_id1.c_str(),
                port_id1.c_str(), switch_id2.c_str(), port_id2.c_str());
  if ((switch_id1.empty()) || (switch_id2.empty()) ||
      (port_id1.empty()) || (port_id2.empty())) {
    pfc_log_error("switch id or port id is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  key_link_t key_link;
  val_link_st_t val_link;
  memset(&key_link, 0, sizeof(key_link_t));
  memset(&val_link, 0, sizeof(val_link_st_t));

  // Fills Key structure
  strncpy(reinterpret_cast<char*>(key_link.switch_id1), switch_id1.c_str(),
                         strlen(switch_id1.c_str()));
  strncpy(reinterpret_cast<char*>(key_link.switch_id2), switch_id2.c_str(),
                         strlen(switch_id2.c_str()));
  strncpy(reinterpret_cast<char*>(key_link.port_id1), port_id1.c_str(),
                         strlen(port_id1.c_str()));
  strncpy(reinterpret_cast<char*>(key_link.port_id2), port_id2.c_str(),
                         strlen(port_id2.c_str()));

  std::string ctr_name = ctr_ptr->get_controller_id();
  strncpy(reinterpret_cast<char*> (key_link.ctr_key.controller_name),
          ctr_name.c_str(), strlen(ctr_name.c_str()));

  // Fills Val Structure
  uint state_value_int1 = atoi(state_value1.c_str());
  uint state_value_int2 = atoi(state_value2.c_str());
  val_link.valid[VAL_LINK_STRUCT_ATTR] = UNC_VF_VALID;
  val_link.link.valid[VAL_DESCRPTION_ATTR] = UNC_VF_VALID;
  if (unc::driver::CONNECTION_UP == ctr_ptr->get_connection_status()) {
    if ((EDGE_UP == state_value_int1) &&
        (EDGE_UP == state_value_int2)) {
      val_link.oper_status = UPPL_LINK_OPER_UP;
      val_link.valid[VAL_OPERSTATUS_ATTR] = UNC_VF_VALID;
    } else {
      val_link.oper_status = UPPL_LINK_OPER_DOWN;
      val_link.valid[VAL_OPERSTATUS_ATTR] = UNC_VF_VALID;
    }
  } else {
    val_link.oper_status = UPPL_LINK_OPER_UNKNOWN;
    val_link.valid[VAL_OPERSTATUS_ATTR] = UNC_VF_VALID;
  }
  pfc_log_debug("oper status of link is %d", val_link.oper_status);

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_link_t, val_link_st_t,
     val_link_st_t,
     uint32_t>(&key_link, &val_link, &val_link, uint32_t(UNC_OP_READ));
  PFC_VERIFY(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  return UNC_RC_SUCCESS;
}

// method to parse the link details from connection string
void OdcLink::parse_properties(std::string conn, std::string &switch_id,
                               std::string &port, std::string &state) {
  std::size_t pos = conn.find(PIPE_SEPARATOR);
  port = conn.substr(0, pos);
  std::string state_sub = conn.substr(pos+1);
  pos = state_sub.find("|");
  state = state_sub.substr(0, pos);
  switch_id = state_sub.substr(pos+1);
}

// parsing function for converting controller response to driver format
UncRespCode OdcLink::parse_link_response(
    unc::driver::controller *ctr_ptr,
    char *data,
    std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_type error");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  uint32_t array_length =0;
  json_object *json_obj_vtn_topo = NULL;
  int parse_ret_val = restjson::JsonBuildParse::parse(jobj,
                                                      "vtn-topology",
                                                      -1,
                                                      json_obj_vtn_topo);

  if ((restjson::REST_OP_SUCCESS != parse_ret_val) ||
      (json_object_is_type(json_obj_vtn_topo, json_type_null))) {
    json_object_put(jobj);
    pfc_log_error("Parsing Error json_obj_vtn_topo is null");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  json_object *json_obj_vtn_links = NULL;
  parse_ret_val = restjson::JsonBuildParse::parse(json_obj_vtn_topo,
                                                      "vtn-link",
                                                      -1,
                                                      json_obj_vtn_links);
  if ((restjson::REST_OP_SUCCESS != parse_ret_val) ||
      (json_object_is_type(json_obj_vtn_links, json_type_null))) {
    json_object_put(jobj);
    pfc_log_error("Parsing Error json_obj_vtn_vlinks is null");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if (json_object_is_type(json_obj_vtn_links, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(json_obj_vtn_topo,
                                                              "vtn-link");
  }
  if (0 == array_length) {
    pfc_log_debug("No LINKS present");
    json_object_put(jobj);
    return UNC_RC_SUCCESS;
  }
  UncRespCode ret_val = UNC_DRV_RC_ERR_GENERIC;
  std::map<std::string, std::string> edge_prop_map;
  std::map<std::string, std::string> head_conn_map;
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    ret_val = fill_edge_value_map(
        json_obj_vtn_links, arr_idx, ctr_ptr, cfgnode_vector);
    if (UNC_RC_SUCCESS != ret_val) {
      json_object_put(jobj);
      pfc_log_error("Error return from fill map failure");
      return ret_val;
    }
  }
  json_object_put(jobj);
  return UNC_RC_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
