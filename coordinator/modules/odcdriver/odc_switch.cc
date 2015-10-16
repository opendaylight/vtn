/*
 * Copyright (c) 2014-2015 NEC Corporation
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
UncRespCode OdcSwitch::fetch_config(unc::driver::controller *ctr_ptr,
                                        pfc_bool_t &cache_empty) {
  ODC_FUNC_TRACE;
  PFC_VERIFY(ctr_ptr != NULL);
  std::vector<unc::vtndrvcache::ConfigNode *> cfgnode_vector;

  vtn_nodes_request *req_obj = new vtn_nodes_request(ctr_ptr);
  std::string url =  req_obj->get_url();
  pfc_log_info("URL:%s",url.c_str());
  vtn_nodes_parser *parse_obj = new vtn_nodes_parser();
  UncRespCode ret_val = req_obj->get_response(parse_obj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Get response Error");
    delete req_obj;
    delete parse_obj;
    return UNC_DRV_RC_ERR_GENERIC;
}
  ret_val = parse_obj->set_vtn_node(parse_obj->jobj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("set-node error");
    delete req_obj;
    delete parse_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ret_val =  parse_node_response(ctr_ptr, parse_obj->vtn_node_, cfgnode_vector);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing");
    delete req_obj;
    delete parse_obj;
    return ret_val;
  }
  ret_val = compare_with_cache(ctr_ptr, cfgnode_vector, cache_empty);
  pfc_log_debug("Response from compare with cache is %d", ret_val);
  delete req_obj;
  delete parse_obj;
  return ret_val;
}

// parse each SWITCH append t  cache
UncRespCode OdcSwitch::fill_config_node_vector(
    unc::driver::controller *ctr_ptr,
    std::string id,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_switch_t key_switch;
  val_switch_st_t val_switch;
  memset(&key_switch, 0, sizeof(key_switch_t));
  memset(&val_switch, 0, sizeof(val_switch_st_t));
 

  std::string ctr_name = ctr_ptr->get_controller_id();
  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_switch.switch_id), id.c_str(),
          strlen(id.c_str()));
  strncpy(reinterpret_cast<char*> (key_switch.ctr_key.controller_name),
          ctr_name.c_str(), strlen(ctr_name.c_str()));

  // Fill Value Structure
  val_switch.valid[VAL_SWITCH_ATTR] = UNC_VF_VALID;

  strncpy(reinterpret_cast<char*> (val_switch.switch_val.domain_name),
          DOM_NAME.c_str(), strlen(DOM_NAME.c_str()));
  val_switch.switch_val.valid[VAL_DOMAINID_ATTR] = UNC_VF_VALID;
  std::string desc ="OF-SWITCH";
  strncpy(reinterpret_cast<char*> (val_switch.switch_val.description),
          desc.c_str(), strlen(desc.c_str()));
  val_switch.switch_val.valid[VAL_DESCRIPTION] = UNC_VF_VALID;

  strncpy(reinterpret_cast<char*> (val_switch.switch_val.model),
          switchmodel.c_str(), strlen(switchmodel.c_str()));
  val_switch.switch_val.valid[VAL_MODEL] = UNC_VF_VALID;

  val_switch.oper_status = UPPL_SWITCH_OPER_UP;
  val_switch.valid[VAL_OPER_STAT_ATTR] = UNC_VF_VALID;

  unc::vtndrvcache::ConfigNode *cfgptr = new unc::vtndrvcache::CacheElementUtil
      <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>
      (&key_switch, &val_switch, &val_switch, uint32_t(UNC_OP_READ));

  PFC_VERIFY(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  return UNC_RC_SUCCESS;
}

//  Compare whether cache empty or not
UncRespCode OdcSwitch::compare_with_cache(
    unc::driver::controller *ctr_ptr,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
    pfc_bool_t &cache_empty) {
  ODC_FUNC_TRACE;
  if (NULL == ctr_ptr->physical_port_cache) {
    pfc_log_error("cache pointer is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::list<std::string> switch_list;
  UncRespCode ret_val = UNC_DRV_RC_ERR_GENERIC;
  if (0 == ctr_ptr->physical_port_cache->cfg_list_count()) {
    cache_empty = PFC_TRUE;
    for (std::vector<unc::vtndrvcache::ConfigNode *>::iterator it =
         cfgnode_vector.begin(); it != cfgnode_vector.end(); ++it) {
      unc::vtndrvcache::ConfigNode *cfgnode_cache = *it;
      if (NULL == cfgnode_cache) {
        pfc_log_error("cfgnode_cache is NULL");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      ret_val = add_event(ctr_ptr, cfgnode_cache, switch_list);
      if (ret_val != UNC_RC_SUCCESS) {
        pfc_log_error("Error in adding to cache");
        return ret_val;
      }
    }
  } else {
    cache_empty = PFC_FALSE;
    ret_val = verify_in_cache(ctr_ptr, cfgnode_vector, switch_list);
    return ret_val;
  }
  return UNC_RC_SUCCESS;
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
  std::list <std::string>::iterator result = std::find(
      switch_list.begin(), switch_list.end(), switch_id);

  // Push into list only if the entry not exist already
  if (result == switch_list.end()) {
    switch_list.push_front(switch_id);
  }
}

//  Add event for Switch
UncRespCode OdcSwitch::add_event(unc::driver::controller *ctr_ptr,
                                     unc::vtndrvcache::ConfigNode *cfg_node,
                                     std::list<std::string> &switch_list) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::CacheElementUtil<key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>*> (cfg_node);
  key_switch_t *key_switch = cfgptr_cache->get_key_structure();
  val_switch_st_t *val_switch = cfgptr_cache->get_val_structure();

  if ((NULL == key_switch) || (NULL == val_switch)) {
    pfc_log_error("key_switch/val_switch is NULL");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  //  Send Notification to UPPL
  notify_physical(unc::driver::VTN_SWITCH_CREATE,
                  key_switch, val_switch, NULL);

  //  Append to cache
  UncRespCode  ret_val =
      ctr_ptr->physical_port_cache->append_Physical_attribute_node(cfg_node);
  if (ret_val != UNC_RC_SUCCESS) {
    delete_config_node(cfg_node);
    pfc_log_error(" Error in adding to cache");
    return ret_val;
  }
  update_list(key_switch, switch_list);
  return ret_val;
}

// Update event for Switch
UncRespCode OdcSwitch::update_event(unc::driver::controller *ctr_ptr,
                                        unc::vtndrvcache::ConfigNode *cfg_node,
                                        val_switch_st_t *val_new_switch,
                                        std::list<std::string> &switch_list) {
  ODC_FUNC_TRACE;
  unc::vtndrvcache::CacheElementUtil<key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>
      *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
      <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>*> (cfg_node);

  key_switch_t *key_switch = cfgptr_cache->get_key_structure();
  val_switch_st_t *val_switch = cfgptr_cache->get_val_structure();

  if ((NULL == key_switch) || (NULL == val_switch) ||
      (NULL == val_new_switch)) {
    pfc_log_error("key_switch/val_switch is NULL");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  // Send notification to UPPL
  notify_physical(unc::driver::VTN_SWITCH_UPDATE, key_switch,
                  val_switch, val_new_switch);
  // Append to cache
  UncRespCode  ret_val =
      ctr_ptr->physical_port_cache->update_physical_attribute_node(cfg_node);
  if (ret_val != UNC_RC_SUCCESS) {
    delete_config_node(cfg_node);
    pfc_log_error(" Error in updating to cache");
    return ret_val;
  }
  update_list(key_switch, switch_list);
  return ret_val;
}

// Delete event for Switch
UncRespCode OdcSwitch::delete_event(unc::driver::controller *ctr,
                                        std::list<std::string> &switch_list) {
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
    pfc_log_debug("key_type is %d", key_type);
    if (UNC_KT_SWITCH == key_type) {
      unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>*> (cfgnode_cache);

      key_switch_t *key_switch_cache = cfgptr_cache->get_key_structure();
      if (NULL == key_switch_cache) {
        pfc_log_error("Key sswitch cache is empty");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      std::string switch_id = reinterpret_cast<const char*>
          (key_switch_cache->switch_id);
      std::list <std::string>::iterator result = std::find(switch_list.begin(),
                                                 switch_list.end(), switch_id);
      if (result == switch_list.end()) {
        pfc_log_trace("Switch %s to be deleted", switch_id.c_str());
        cfg_node_delete_map[switch_id] = cfgnode_cache;
      }
    }
    if (UNC_KT_PORT == key_type) {
      unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, val_port_st_t, uint32_t>
          *cfgptr_cache_port = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, val_port_st_t, uint32_t>*> (cfgnode_cache);

      key_port_t *key_port_cache = cfgptr_cache_port->get_key_structure();
      if (NULL == key_port_cache) {
        pfc_log_error("key_port_cache is NULL");
        return UNC_DRV_RC_ERR_GENERIC;
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
        pfc_log_trace("Switch PORT %s to be deleted", switch_port_id.c_str());
        cfg_node_delete_map[logical_switch_map] = cfgnode_cache;
      }
    }
  }

  UncRespCode  ret_val =
        delete_logical_port(ctr, cfg_node_delete_map);
  if (ret_val != UNC_RC_SUCCESS) {
    pfc_log_error("Error in deleting logical port");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ret_val = delete_switch(ctr, cfg_node_delete_map);
  return ret_val;
}

// Send Delete notification to logical port
UncRespCode OdcSwitch::delete_logical_port(
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
    std::string switch_id = iter->first;
    if (switch_id.compare(0, 3, "LP-") == 0) {
      unc::vtndrvcache::CacheElementUtil<key_port_t, val_port_st_t, val_port_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_port_t, val_port_st_t, val_port_st_t, uint32_t>*> (cfg_node);

      key_port_t *key_port = cfgptr_cache->get_key_structure();
      val_port_st_t *val_port = cfgptr_cache->get_val_structure();
      if ((NULL == key_port) || (NULL == val_port)) {
        pfc_log_error("key_port/val_port is NULL");
        return UNC_DRV_RC_ERR_GENERIC;
      }

      OdcPort odc_port_obj(conf_file_values_);
      // Send notification to UPPL
      odc_port_obj.notify_logical_port_physical(unc::driver::VTN_LP_DELETE,
                                                key_port, val_port, NULL);
    }
  }
  return UNC_RC_SUCCESS;
}

// Send Delete notification to Switch
UncRespCode OdcSwitch::delete_switch(
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
    std::string switch_id = iter->first;
    if (switch_id.compare(0, 3, "LP-") != 0) {
      unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> *cfgptr_cache_sw =
          static_cast<unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>*> (cfg_node);

      key_switch_t *key_switch = cfgptr_cache_sw->get_key_structure();
      val_switch_st_t *val_switch = cfgptr_cache_sw->get_val_structure();
      if ((NULL == key_switch) || (NULL == val_switch)) {
        pfc_log_error("key_switch/val_switch is NULL");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      // Send notification to UPPL
      notify_physical(unc::driver::VTN_SWITCH_DELETE, key_switch,
                      val_switch, NULL);

      // Delete from cache
      UncRespCode  ret_val =
          ctr->physical_port_cache->delete_physical_attribute_node(cfg_node);
      if (ret_val != UNC_RC_SUCCESS) {
        pfc_log_error(" Error in deleting in cache");
        return ret_val;
      }
    }
  }
  return UNC_RC_SUCCESS;
}

//  Verify in cache whether switch aleady exists or not
UncRespCode OdcSwitch::verify_in_cache(
    unc::driver::controller *ctr_ptr,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
    std::list<std::string> &switch_list) {
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

    unc::vtndrvcache::CacheElementUtil<key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>
        *cfgnode_ctr = static_cast<unc::vtndrvcache::CacheElementUtil
        <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>*> (cfg_node);

    val_switch_st_t *val_switch_ctr = cfgnode_ctr->get_val_structure();
    key_switch_t *key_switch = cfgnode_ctr->get_key_structure();

    std::string switch_id = reinterpret_cast<const char*>
                                 (key_switch->switch_id);
    pfc_log_debug("Switch id is %s", switch_id.c_str());
    unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
    pfc_bool_t exist_in_cache =
        ctr_ptr->physical_port_cache->compare_is_physical_node_found(
            cfgnode_ctr, cfgnode_cache);
    if (PFC_FALSE == exist_in_cache) {
      pfc_log_trace("Switch %s not exists in cache", switch_id.c_str());
      UncRespCode  ret_val = add_event(ctr_ptr, cfgnode_ctr, switch_list);
      if (ret_val != UNC_RC_SUCCESS) {
        pfc_log_error("Error occured in adding to cache");
        return ret_val;
      }
    } else {
      pfc_log_trace("Switch  %s exists in cache", switch_id.c_str());
      unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>
          *cfgptr_cache = static_cast<unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t>*> (cfgnode_cache);

      key_switch_t *key_switch = cfgptr_cache->get_key_structure();
      val_switch_st_t *val_switch_cache = cfgptr_cache->get_val_structure();

      pfc_bool_t sw_update = is_switch_modified(val_switch_ctr,
                                                val_switch_cache);
      if (PFC_TRUE == sw_update) {
        pfc_log_trace("Switch is %s,  parameters are updated",
                                     switch_id.c_str());
        UncRespCode  ret_val =
            update_event(ctr_ptr, cfgnode_ctr, val_switch_cache, switch_list);
        if (ret_val != UNC_RC_SUCCESS) {
          pfc_log_error("error in updating the cache");
          return ret_val;
        }
      } else {
        pfc_log_trace("Switch %s ,Exists in cache not updated",
                                         switch_id.c_str());
        update_list(key_switch, switch_list);
      }
      delete_config_node(cfg_node);
    }
  }
  // Check for delete
  UncRespCode  ret_val = delete_event(ctr_ptr, switch_list);
  return ret_val;
}

// Deletes config node
void OdcSwitch::delete_config_node(unc::vtndrvcache::ConfigNode *cfg_node) {
  ODC_FUNC_TRACE;
  if (NULL != cfg_node) {
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
UncRespCode OdcSwitch::parse_node_response(
    unc::driver::controller *ctr_ptr,
    std::list<vtn_node> &node_detail,
    std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  std::list<vtn_node>::iterator it;
  for (it = node_detail.begin(); it != node_detail.end(); it++) {
    std::string id = it->id;
    UncRespCode ret_val = fill_config_node_vector(
      ctr_ptr, id, cfgnode_vector);

    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Error return from fill map failure");
      return ret_val;
    }
  }
  return UNC_RC_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
