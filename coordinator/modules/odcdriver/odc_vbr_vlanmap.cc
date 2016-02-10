/*
 * Copyright (c) 2013-2015 NEC Corporation
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
OdcVbrVlanMapCommand::OdcVbrVlanMapCommand(
    unc::restjson::ConfFileValues_t conf_values)
:conf_file_values_(conf_values) {
  ODC_FUNC_TRACE;
}

// Destructor
OdcVbrVlanMapCommand::~OdcVbrVlanMapCommand() {
  ODC_FUNC_TRACE;
}

// fetch child configurations for the parent kt(vbr)
UncRespCode OdcVbrVlanMapCommand::fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector <unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;

  key_vbr_t* parent_vbr = reinterpret_cast<key_vbr_t*> (parent_key);
  PFC_ASSERT(parent_vbr != NULL);
  std::string vtn_name =
      reinterpret_cast<const char*> (parent_vbr->vtn_key.vtn_name);
  std::string vbr_name =
      reinterpret_cast<const char*> (parent_vbr->vbridge_name);

  PFC_ASSERT(ctr_ptr != NULL);
  if ((0 == strlen(vtn_name.c_str())) || (0 == strlen(vbr_name.c_str()))) {
    pfc_log_error("%s: Empty VTN/VBR name", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vlan_class *req_obj = new vlan_class(ctr_ptr, vtn_name, vbr_name);
  std::string url = req_obj->get_url();
  pfc_log_info("URL:%s",url.c_str());
  vlan_parser *parser_obj = new vlan_parser();
  UncRespCode ret_val = req_obj->get_response(parser_obj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Get response error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  pfc_log_info("entering set_vlan_conf");
  ret_val = parser_obj->set_vlan_conf(parser_obj->jobj);
  pfc_log_info("exiting set_vlan_conf");
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("set_vbridge_vlan_conf error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = parse_vbrvlanmap_response(parent_key, ctr_ptr,
                                        parser_obj->vlan_conf_,
                                       cfgnode_vector);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing");
    delete req_obj;
    delete parser_obj;
    return ret_val;
  }

 return UNC_RC_SUCCESS;
}

// Validates VLAN exists or not
UncRespCode OdcVbrVlanMapCommand::validate_vlan_exist(
  key_vlan_map_t &key_vlan_map,
  pfcdrv_val_vlan_map_t &val_vlan_map,
  const std::string &logical_port_id,
  unc::driver::controller *ctr,
  pfc_bool_t &is_switch_exist,
  std::string &port_id) {
  ODC_FUNC_TRACE;
  if (key_vlan_map.logical_port_id_valid != 0) {
    // Validate request received with SwitchID
    UncRespCode ret_val =
        check_switch_already_exists(key_vlan_map,
                 val_vlan_map, logical_port_id, ctr, is_switch_exist, port_id);
    return ret_val;
  } else {
    // Validate request received without SwitchID
    UncRespCode  ret_val = check_ANY_already_exists
        (key_vlan_map, val_vlan_map, ctr, is_switch_exist, port_id);
      return ret_val;
  }
}

// Check  "ANY" vlan-id (vlan-id not associated with any switch)
// exists in controller
UncRespCode
OdcVbrVlanMapCommand::check_ANY_already_exists(
    key_vlan_map_t &key_vlan_map,
    pfcdrv_val_vlan_map_t  &val_vlan_map,
    unc::driver::controller *ctr,
    pfc_bool_t &is_switch_exist,
    std::string &port_id) {
  ODC_FUNC_TRACE;
  uint vlan_id_req = val_vlan_map.vm.vlan_id;
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
      return UNC_DRV_RC_ERR_GENERIC;
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
      return UNC_DRV_RC_ERR_GENERIC;
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
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  return UNC_RC_SUCCESS;
}

// Check if switch id exists in controller
UncRespCode OdcVbrVlanMapCommand::check_switch_already_exists(
    key_vlan_map_t &key_vlan_map,
    pfcdrv_val_vlan_map_t &val_vlan_map,
    const std::string &switch_id_req,
    unc::driver::controller *ctr_ptr,
    pfc_bool_t &is_switch_exist,
    std::string &port_id) {
  ODC_FUNC_TRACE;
  uint vlan_id_req = val_vlan_map.vm.vlan_id;
  std::string vtn_name_req = reinterpret_cast<char*>
      (key_vlan_map.vbr_key.vtn_key.vtn_name);
  std::string vbr_name_req = reinterpret_cast<char*>
      (key_vlan_map.vbr_key.vbridge_name);

  // If vlan-id is received as 0xFFFF from UPLL,its translated as 0
  // to check with controller
  if (vlan_id_req == 0xFFFF) {
    vlan_id_req = 0;
  }

  std::string logical_port_req = NODE_TYPE_OF;
  pfc_log_debug(" Logical port id received %s", switch_id_req.c_str());
  // logical_port_id received in key structure should be prefixed with SW-
  if (switch_id_req.compare(0, 3, SW_PREFIX) == 0) {
    logical_port_req.append(switch_id_req.substr(3));
  } else {
    pfc_log_error("%s: Request logical port is not in the format of SW-",
                  PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
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
      return UNC_DRV_RC_ERR_GENERIC;
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
      return UNC_DRV_RC_ERR_GENERIC;
    }

    if (vlan_id_req == vlan_id_ctr) {
      if (vtn_name_req.compare(vtn_name_ctr) == 0) {
        if (vbr_name_req.compare(vbr_name_ctr) == 0) {
          if (switch_id_ctr == NODE_TYPE_ANY) {
            pfc_log_error("%s:VLAN ID Conflict for SWID in Request & "
                              "ANY in Controller %s",
                              PFC_FUNCNAME, switch_id_ctr.c_str());
            return UNC_DRV_RC_ERR_GENERIC;
          } else if (logical_port_req.compare(switch_id_ctr) == 0) {
            pfc_log_error("%s: VLANID Conflict for SWID in Request & "
                          "Controller (%s)!!!",
                          PFC_FUNCNAME, switch_id_ctr.c_str());
            return UNC_DRV_RC_ERR_GENERIC;
          }
        } else {
          pfc_log_error("%s:VLAN id Conflict in different vbridges "
                        "under same VTN", PFC_FUNCNAME);
          return UNC_DRV_RC_ERR_GENERIC;
        }
      } else {
        pfc_log_error("%s:VLAN id Conflict in different "
                      "vbridges under different VTN", PFC_FUNCNAME);
        return UNC_DRV_RC_ERR_GENERIC;
      }
    } else {
      if ((vtn_name_req.compare(vtn_name_ctr) == 0) &&
          (vbr_name_req.compare(vbr_name_ctr) == 0) &&
          (logical_port_req.compare(switch_id_ctr) == 0)) {
        pfc_log_debug("%s:VLAN id is different for same SWID "
                      "in same vbridges/VTN", PFC_FUNCNAME);
        is_switch_exist = PFC_TRUE;
        int convert_id = atoi(switch_id_ctr.substr(12).c_str());
        std::stringstream stream;
        stream << std::hex << convert_id;
        std::string switch_dec = stream.str();
        std::string switch_id = frame_switchid_hex(switch_dec);
        port_id.append(NODE_TYPE_OF);
        port_id.append(switch_id);
        port_id.append(PERIOD);
        std::ostringstream val_id_str_format;
        val_id_str_format << vlan_id_ctr;
        port_id.append(val_id_str_format.str());
      }
    }
  }
  return UNC_RC_SUCCESS;
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
// Parse the VBR_VLANMAP data
UncRespCode OdcVbrVlanMapCommand::parse_vbrvlanmap_response(void *parent_key,
                                                unc::driver::controller* ctr_ptr,
                                             std::list<vlan_conf> &vlan_detail,
               std::vector < unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;

  std::list<vlan_conf>::iterator it;
  for (it = vlan_detail.begin(); it != vlan_detail.end(); it++) {
    std::string mapid = it->map_id;
    uint32_t vlanid = it->vlanmap_config_.vlanid;
    pfc_log_info("vlanid:%d",vlanid);
    UncRespCode ret_val = fill_config_node_vector(parent_key, ctr_ptr, mapid,
                                       vlanid, cfgnode_vector);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Error return from fill config failure");
      return ret_val;
    }
  }
  return UNC_RC_SUCCESS;
}
// Parse VBR_VLANMAP and append it to confignode vector
UncRespCode OdcVbrVlanMapCommand::fill_config_node_vector(void *parent_key,
                                            unc::driver::controller* ctr_ptr,
                         std::string mapid, uint16_t vlanid,
                std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_vlan_map_t key_vlan_map;
  pfcdrv_val_vlan_map_t val_vlan_map;
  memset(&key_vlan_map, 0, sizeof(key_vlan_map_t));
  memset(&val_vlan_map, 0, sizeof(pfcdrv_val_vlan_map_t));

  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast<unc::odcdriver::OdcController *>(ctr_ptr);
  PFC_ASSERT(odc_ctr != NULL);

  //  convertnode  id to decimal format
    std::string id = "";
    if (mapid.compare(0, 3, NODE_TYPE_ANY) == 0) {
    pfc_log_debug("switch id not present map id framed in parse %s:",
                mapid.c_str());
    std::stringstream vlan_str;
    vlan_str << vlanid;
    std::string vlan = vlan_str.str();
    id.append(vlan);
    pfc_log_debug("Printing mapid %s:", mapid.c_str());

    }  else {
    std::string switch_hex_val = mapid.substr(3, 23);
    std::string switch_dec_val  =
        odc_ctr->frame_openflow_switchid(switch_hex_val);
    if (switch_hex_val.empty()) {
        pfc_log_error("%s: switch id is empty", PFC_FUNCNAME);
        return UNC_DRV_RC_ERR_GENERIC;
    }
    mapid.append(NODE_TYPE_OF);
    mapid.append(switch_dec_val);
    mapid.append(PERIOD);
    std::stringstream vlan_str;
    vlan_str << vlanid;
    std::string vlan = vlan_str.str();
    mapid.append(vlan);
    pfc_log_debug("switch id present map id framed in parse %s:",
                  mapid.c_str());
    std::string switch_id = SW_PREFIX;
    std::string openflow_id = odc_ctr->frame_openflow_switchid(mapid);
    if (openflow_id.empty()) {
        pfc_log_error("%s: switch id is empty", PFC_FUNCNAME);
        return UNC_DRV_RC_ERR_GENERIC;
    }
    switch_id.append(openflow_id);
    strncpy(reinterpret_cast<char*>
            (key_vlan_map.logical_port_id),
            switch_id.c_str(),
            strlen(switch_id.c_str()));
    key_vlan_map.logical_port_id_valid = 1;

  }

  val_vlan_map.valid[PFCDRV_IDX_VAL_VLAN_MAP] = UNC_VF_VALID;
  val_vlan_map.vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  val_vlan_map.vm.vlan_id = vlanid;
  if (val_vlan_map.vm.vlan_id == 0) {
    val_vlan_map.vm.vlan_id = 0xFFFF;
    pfc_log_debug("%s: Vlan id untagged set as %d", PFC_FUNCNAME,
                  val_vlan_map.vm.vlan_id);
  }
  // filling key structure
  key_vbr_t* parent_vbr = reinterpret_cast<key_vbr_t*> (parent_key);
  PFC_ASSERT(parent_vbr != NULL);
  std::string parent_vtn_name =
      reinterpret_cast<const char*> (parent_vbr->vtn_key.vtn_name);
  std::string parent_vbr_name =
      reinterpret_cast<const char*> (parent_vbr->vbridge_name);

  if ((parent_vtn_name.empty()) || (parent_vbr_name.empty())) {
    pfc_log_error("%s: VTN/VBR name is empty", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  strncpy(reinterpret_cast<char*> (key_vlan_map.vbr_key.vtn_key.vtn_name),
          parent_vtn_name.c_str(), strlen(parent_vtn_name.c_str()));

  strncpy(reinterpret_cast<char*> (key_vlan_map.vbr_key.vbridge_name),
          parent_vbr_name.c_str(), strlen(parent_vbr_name.c_str()));

  // Cache the parsed vlanmap configurations
  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil
      <key_vlan_map_t, pfcdrv_val_vlan_map_t, pfcdrv_val_vlan_map_t, uint32_t>
      (&key_vlan_map, &val_vlan_map,  &val_vlan_map, uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  pfc_log_debug("THE MAPID is %s", mapid.c_str());
  std::string vtn_vbr_vlan = generate_string_for_vector(parent_vtn_name,
                                                        parent_vbr_name, mapid,
                                                        ctr_ptr);

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
  return UNC_RC_SUCCESS;
}

std::string OdcVbrVlanMapCommand::generate_string_for_vector(
    const std::string &vtn_name,
    const std::string &vbr_name,
    const std::string &vlan_id,
    unc::driver::controller *ctr) {
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
  size_t pos = vlan_id.find(PERIOD);
  std::string switch_id = vlan_id.substr(3, pos-3);
  if ((!switch_id.compare(NODE_TYPE_ANY))
         &&(!switch_id.compare(0, 9, SWITCH_BASE))) {
      pfc_log_debug("VTN manager switch format received:%s ",
                                                  switch_id.c_str());
      std::string map_id = switch_id;
      vtn_vbr_vlan.append(NODE_TYPE_OF);
      vtn_vbr_vlan.append(map_id);
      vtn_vbr_vlan.append(PERIOD);
      vtn_vbr_vlan.append(vlan_id.substr(pos));
      pfc_log_debug("generated ID:%s ", vtn_vbr_vlan.c_str());
  } else {
    vtn_vbr_vlan.append(vlan_id);
  }
  return vtn_vbr_vlan;
}

// Generate Mapping ID
std::string OdcVbrVlanMapCommand::generate_vlanmap_id(
    key_vlan_map_t& vlanmap_key,
    std::string str_vlanid, const std::string &logical_port_id) {
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
    pfc_log_debug(" Logical port id received %s", logical_port_id.c_str());
    std::string switch_id = "";
    if (0 != strlen(logical_port_id.c_str())) {
      switch_id = logical_port_id.substr(3);
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
UncRespCode OdcVbrVlanMapCommand::create_cmd(
    key_vlan_map_t& vlanmap_key,
    pfcdrv_val_vlan_map_t& vlanmap_val,
    unc::driver::controller* ctr_ptr) {

  ODC_FUNC_TRACE;
  return create_update_cmd(vlanmap_key, vlanmap_val, ctr_ptr);
}

// Update Command for vbr vlanmap
UncRespCode OdcVbrVlanMapCommand::update_cmd(
    key_vlan_map_t& vlanmap_key,
    pfcdrv_val_vlan_map_t& vlanmap_val_old,
    pfcdrv_val_vlan_map_t& vlanmap_val_new,
    unc::driver::controller* ctr_ptr) {

  ODC_FUNC_TRACE;
  return create_update_cmd(vlanmap_key, vlanmap_val_new,
                                   ctr_ptr);
}

// Delete vlan-map from controller
UncRespCode OdcVbrVlanMapCommand::del_existing_vlanmap(
    key_vlan_map_t& vlanmap_key,
     pfcdrv_val_vlan_map_t& vlanmap_val,
    unc::driver::controller*
    ctr_ptr, const std::string &str_mapping_id) {
  ODC_FUNC_TRACE;
  std::string vlanid = "";

  char* vtnname = NULL;
  char* vbrname = NULL;
  vtnname = reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vbrname = reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name);
  if (0 == strlen(vbrname)) {
    pfc_log_error("Empty vbr name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vlan_class *req_obj = new vlan_class(ctr_ptr, vtnname, vbrname);
  ip_vlan_config st_obj;
  delete_request_body(vlanmap_key,vlanmap_val,st_obj);
  vlan_parser *parser_obj = new vlan_parser();
  json_object *jobj = parser_obj->del_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
}

  if (str_mapping_id.empty()) {
    pfc_log_error("%s: MapID received is empty", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  UncRespCode ret_val = (req_obj->set_delete(jobj));
  if (ret_val != UNC_RC_SUCCESS) {
    pfc_log_error("delete existing vlanmap failed");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string mapid = "";
  if (str_mapping_id.compare(0, 3, NODE_TYPE_ANY) == 0) {
    mapid = str_mapping_id;
    pfc_log_debug("switch id not present map id framed in parse %s:",
                  mapid.c_str());
  } else {
    std::string convert_mapid =str_mapping_id.substr(3, 23);
    unc::odcdriver::OdcController *odc_ctr =
        reinterpret_cast<unc::odcdriver::OdcController *>(ctr_ptr);
    std::string switch_dec_val =
         odc_ctr->frame_openflow_switchid(convert_mapid);
    mapid = str_mapping_id;
    mapid.replace(3, 23, switch_dec_val);
    pfc_log_debug("final map id to delete from vector:%s", mapid.c_str());
  }
  std::string vtn_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name);
  std::string vbr_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name);
  std::string vtn_vbr_vlan_delete = generate_string_for_vector(vtn_name_req,
                                                vbr_name_req, mapid,
                                                ctr_ptr);
  if (vtn_vbr_vlan_delete.empty()) {
    pfc_log_debug("vtn/vbr/id is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }


  // Entry from search vector is removed
  delete_from_vector(ctr_ptr, vtn_vbr_vlan_delete);

  return UNC_RC_SUCCESS;
}

// Create or Update Command for vbr vlanmap
UncRespCode OdcVbrVlanMapCommand::create_update_cmd(
    key_vlan_map_t& vlanmap_key,
    pfcdrv_val_vlan_map_t& vlanmap_val,
    unc::driver::controller* ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  pfc_bool_t vlan_map_exists = PFC_FALSE;
  std::string strmapid = "";
  std::string logical_port_id = "";
  if (vlanmap_key.logical_port_id_valid != 0) {
    pfc_log_debug("%s: Logical_port_id is valid", PFC_FUNCNAME);
    logical_port_id = reinterpret_cast<char*>
        (vlanmap_key.logical_port_id);
    odc_drv_resp_code_t ret_val = validate_logical_port_id(logical_port_id);
    if (ret_val != ODC_DRV_SUCCESS) {
      pfc_log_error("%s: Validation for logical_port[%s] failed ",
                    PFC_FUNCNAME, logical_port_id.c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
    pfc_log_debug(" Logical port id received %s", logical_port_id.c_str());
  }
  if (vlanmap_val.vm.valid[UPLL_IDX_VLAN_ID_VM] != UNC_VF_VALID) {
    pfc_log_error("%s: Received invalid Vlan_id",
                    PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  // Validate if vlanid already exists in controller
  UncRespCode validate_resp = validate_vlan_exist(vlanmap_key,
                                                  vlanmap_val,
                                                  logical_port_id,
                                                  ctr_ptr,
                                                  vlan_map_exists,
                                                  strmapid);

  if (UNC_RC_SUCCESS != validate_resp) {
    pfc_log_error("%s Validation of vlanmap failed, validate_resp(%u)",
                  PFC_FUNCNAME, validate_resp);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  // If vlanid exists for same SwitchID/ANY delete the existing vlanmap
  // and create a new vlanmap
  if (PFC_TRUE == vlan_map_exists) {
    if (del_existing_vlanmap(vlanmap_key, vlanmap_val, ctr_ptr, strmapid) !=
        UNC_RC_SUCCESS) {
      pfc_log_error("%s Delete of vlanmap failed", PFC_FUNCNAME);
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }

  char* vtnname = NULL;
  char* vbrname = NULL;
  vtnname = reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vbrname = reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name);
  if (0 == strlen(vbrname)) {
    pfc_log_error("Empty vbr name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vlan_class *req_obj = new vlan_class(ctr_ptr, vtnname, vbrname);
  ip_vlan_config st_obj;
  create_request_body(vlanmap_key, vlanmap_val, st_obj, logical_port_id);
  vlan_parser *parser_obj = new vlan_parser();
  json_object *jobj = parser_obj->create_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
}
  if(req_obj->set_post(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vlan create/update Failed");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string vtn_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name);
  std::string vbr_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name);

  uint vlanid =  vlanmap_val.vm.vlan_id;
  std::ostringstream convert_vlanid;
  convert_vlanid << vlanid;
  std::string map_id = generate_vlanmap_id(vlanmap_key, convert_vlanid.str(),
                                           logical_port_id);
  if (map_id.empty()) {
    pfc_log_error("Error occured in generating vlan map id");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string vtn_vbr_vlan_update = generate_string_for_vector(vtn_name_req,
                          vbr_name_req, map_id, ctr_ptr);
  pfc_log_info("mapid generate for update:%s", vtn_vbr_vlan_update.c_str());
  if (vtn_vbr_vlan_update.empty()) {
    pfc_log_error("vtn/vbr/switch id is empty in %s", PFC_FUNCNAME);
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  update_vector(ctr_ptr,  vtn_vbr_vlan_update);
  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
}

// Deletes particular entry from vlan-map vector
void OdcVbrVlanMapCommand::delete_from_vector(unc::driver::controller *ctr ,
                                              std::string vtn_vbr_vlan) {
  ODC_FUNC_TRACE;
  unc::odcdriver::OdcController *odc_ctr =
      reinterpret_cast< unc::odcdriver::OdcController *>(ctr);
  pfc_log_debug("mapid to delete:%s", vtn_vbr_vlan.c_str());
  PFC_ASSERT(odc_ctr != NULL);
  for (uint iterator = 0; iterator < odc_ctr->vlan_vector.size(); iterator++) {
    if (odc_ctr->vlan_vector.at(iterator).compare(vtn_vbr_vlan) == 0) {
      pfc_log_debug("mapid matched in vector:%s", vtn_vbr_vlan.c_str());
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
                                             key_vlan_map_t &vlanmap_key,
                                             std::string logical_portid_req) {
  ODC_FUNC_TRACE;
  std::string vtn_name_req = reinterpret_cast<char*>
      (vlanmap_key.vbr_key.vtn_key.vtn_name);
  std::string vbr_name_req = reinterpret_cast<char*>
      (vlanmap_key.vbr_key.vbridge_name);

  pfc_log_error(" Logical port id received %s", logical_portid_req.c_str());
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
      pfc_log_debug("Printing the vector data %s", vtn_vbr_vlan_ctr.c_str());
      return vtn_vbr_vlan_data.substr(occurence+1);
    }
  }
  pfc_log_error("No Values in vector");
  return "";
}

// Delete Command for vbr vlan-map
UncRespCode OdcVbrVlanMapCommand::delete_cmd(
    key_vlan_map_t& vlanmap_key,
    pfcdrv_val_vlan_map_t& vlanmap_val,
    unc::driver::controller* ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  key_vbr_t parent_vbr;
  std::string del_vlanid;

  memcpy(parent_vbr.vtn_key.vtn_name, vlanmap_key.vbr_key.vtn_key.vtn_name,
         sizeof(vlanmap_key.vbr_key.vtn_key.vtn_name));
  memcpy(parent_vbr.vbridge_name, vlanmap_key.vbr_key.vbridge_name,
         sizeof(vlanmap_key.vbr_key.vbridge_name));

  std::string str_mapping_id = "";
  std::string logical_portid_req = "";
  if (vlanmap_key.logical_port_id_valid != 0) {
    logical_portid_req = reinterpret_cast<char*>
        (vlanmap_key.logical_port_id);
    pfc_log_error("Validation for logical_port in Key structure %s ",
                                             logical_portid_req.c_str());
    odc_drv_resp_code_t ret_val = validate_logical_port_id(
                                                logical_portid_req);
    if (ret_val != ODC_DRV_SUCCESS) {
      pfc_log_error("%s: Validation for logical_port[%s] failed ",
                    PFC_FUNCNAME, logical_portid_req.c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
    pfc_log_error("Validation for logical_port[%s]", logical_portid_req.c_str());
    std::string switch_id = logical_portid_req;
    str_mapping_id.append("OF-");

    // convert the switch id from openflow:2 to
    // 00:00:00:00:00:00:00:02 format
    int convert_id = atoi(switch_id.substr(12).c_str());
      pfc_log_error(" Printing convert_id %d",convert_id );
    std::stringstream stream;
    stream << std::hex << convert_id;
    std::string switch_dec = stream.str();
    pfc_log_error("PRINTING Converted switch if %s ", switch_dec.c_str());
    std::string switch_val = frame_switchid_hex(switch_dec);
    if (switch_val.empty()) {
      pfc_log_error("%s: Empty switch id returned", PFC_FUNCNAME);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    str_mapping_id.append(switch_val);
  } else {
    str_mapping_id.append(NODE_TYPE_ANY);
  }
  std::string str_vlan_id = generate_vlanid_from_vector(ctr_ptr,
                                      vlanmap_key, logical_portid_req);
  if (str_vlan_id.empty()) {
    pfc_log_error("%s: Empty vlan id returned", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  str_mapping_id.append(PERIOD);
  str_mapping_id.append(str_vlan_id);
  pfc_log_debug("%s: Mapping id formed in delete %s", PFC_FUNCNAME,
                str_mapping_id.c_str());

  // Delete URL to be sent to controller will be of the form "/controller/nb/v2/
  // vtn/default/vtns/{VTNName}/vbridges/{VBRName}/vlanmaps/{mapId}"

  char* vtnname = NULL;
  char* vbrname = NULL;
  vtnname = reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vbrname = reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name);
  if (0 == strlen(vbrname)) {
    pfc_log_error("Empty vbr name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vlan_class *req_obj = new vlan_class(ctr_ptr, vtnname, vbrname);
  ip_vlan_config st_obj;
  delete_request_body(vlanmap_key, vlanmap_val, st_obj);
  vlan_parser *parser_obj = new vlan_parser();
  json_object *jobj = parser_obj->del_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
}


  if ((req_obj->get_cu_url()).empty()) {
    pfc_log_error("%s: vlanmap url is empty", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if(req_obj->set_delete(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vlan delete Failed");
  }
  std::string vtn_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vtn_key.vtn_name);
  std::string vbr_name_req =
      reinterpret_cast<char*>(vlanmap_key.vbr_key.vbridge_name);

  std::string mapid = "";
  if (str_mapping_id.compare(0, 3, NODE_TYPE_ANY) == 0) {
    mapid = str_mapping_id;
    pfc_log_debug("switch id not present map id framed in parse %s:",
                  mapid.c_str());
  } else {
    std::string convert_mapid =str_mapping_id.substr(3, 23);
    unc::odcdriver::OdcController *odc_ctr =
                reinterpret_cast<unc::odcdriver::OdcController *>(ctr_ptr);
    std::string switch_dec_val  = odc_ctr->frame_openflow_switchid(convert_mapid);
    mapid = str_mapping_id;
    mapid.replace(3, 23, switch_dec_val);
    pfc_log_info("final map id to delete from vector:%s", mapid.c_str());
  }
  std::string vtn_vbr_vlan_delete = generate_string_for_vector(vtn_name_req,
                                               vbr_name_req, mapid,
                                               ctr_ptr);
  if (vtn_vbr_vlan_delete.empty()) {
    pfc_log_error("vtn/vbr/switch id is empty in %s", PFC_FUNCNAME);
    delete req_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  delete_from_vector(ctr_ptr, vtn_vbr_vlan_delete);
  delete req_obj;
  return UNC_RC_SUCCESS;
}

// Creates request body for vbr vlanmap
void OdcVbrVlanMapCommand::create_request_body(
    key_vlan_map_t& vlanmap_key,
    pfcdrv_val_vlan_map_t& vlanmap_val,
    ip_vlan_config&  ip_vlan_config_st,
    const std::string &logical_port_id) {
  ODC_FUNC_TRACE;
  std::string vlanid;

  ip_vlan_config_st.input_vlan_.valid = true;
  ip_vlan_config_st.input_vlan_.tenant_name =
             reinterpret_cast<const char*>(vlanmap_key.vbr_key.vtn_key.vtn_name);
  ip_vlan_config_st.input_vlan_.bridge_name =
             reinterpret_cast<const char*>(vlanmap_key.vbr_key.vbridge_name);

  if (vlanmap_val.vm.vlan_id == 0xFFFF) {
    pfc_log_debug("%s: Vlan Untagged ", PFC_FUNCNAME);
    vlanid = "0";
  } else {
    pfc_log_debug("%s: Vlan Tagged ", PFC_FUNCNAME);
    std::ostringstream convert_vlanid;
    convert_vlanid << vlanmap_val.vm.vlan_id;
    vlanid.append(convert_vlanid.str());
  }
  pfc_log_debug("%s: Vlanid: %s", PFC_FUNCNAME, vlanid.c_str());
  if (!vlanid.empty()) {
    pfc_log_info("vlan not empty : %s", vlanid.c_str());
    ip_vlan_config_st.input_vlan_.vlan_id.assign(vlanid);
  }
    pfc_log_debug(" Logical port id received %s", logical_port_id.c_str());
    std::string of_switch_id = "";
    if (0 != strlen(logical_port_id.c_str())) {
      of_switch_id = logical_port_id.substr(3);
      int switch_val = atoi(of_switch_id.substr(9).c_str());
      std::stringstream stream;
      stream << std::hex << switch_val;
      std::string switch_id = stream.str();
      pfc_log_debug("%s: Logical_port_id(%s) ", PFC_FUNCNAME,
                    logical_port_id.c_str());
      pfc_log_debug("%s: Switch id(%s) ", PFC_FUNCNAME,
                    of_switch_id.c_str());
      ip_vlan_config_st.input_vlan_.node.assign(of_switch_id);

    }
}

//Delete request body
void OdcVbrVlanMapCommand::delete_request_body(
    key_vlan_map_t& vlanmap_key,
    pfcdrv_val_vlan_map_t& vlanmap_val,
    ip_vlan_config&  ip_vlan_config_st) {

  ip_vlan_config_st.input_vlan_.valid = true;
  ip_vlan_config_st.input_vlan_.tenant_name =
             reinterpret_cast<const char*>(vlanmap_key.vbr_key.vtn_key.vtn_name);
  ip_vlan_config_st.input_vlan_.bridge_name =
             reinterpret_cast<const char*>(vlanmap_key.vbr_key.vbridge_name);

}

// Validates the format of logical port id received from UPLL
odc_drv_resp_code_t OdcVbrVlanMapCommand::validate_logical_port_id(
    const std::string& logical_port_id) {
  ODC_FUNC_TRACE;
  pfc_log_debug("%s: Logical port received : %s", PFC_FUNCNAME,
                logical_port_id.c_str());
  if ((logical_port_id.compare(0, 3, SW_PREFIX) != 0) &&
      (logical_port_id.size() != 26)) {
    pfc_log_error("%s: Logical_port_id doesn't have SW- prefix "
                  "or not in proper length", PFC_FUNCNAME);
    return ODC_DRV_FAILURE;
  }
  std::string switch_id = logical_port_id.substr(3);
  //  Split Switch id and prefix
  std::string switch_base = switch_id.substr(0, 8);
  pfc_log_info("switch_base:%s", switch_base.c_str());
  if ((switch_base.compare("openflow"))) {
    pfc_log_error("Invalid switch id format supported by Vtn Manager");
    return ODC_DRV_FAILURE;
  }
  pfc_log_debug("Switch id in vlan map %s", switch_id.c_str());
  pfc_log_debug("Valid logical_port id");
  return ODC_DRV_SUCCESS;
}
// convert switch id to vtn manager supported format
std::string OdcVbrVlanMapCommand::frame_switchid_hex(
    std::string &node_id) {
  pfc_log_info("switch val recived:%s", node_id.c_str());
  std::string switch_id_hex = SWITCH_HEX;
  int pos = switch_id_hex.length() - node_id.length();
  if (pos < 0) {
    pfc_log_error("Invalid switch id format");
    return "";
  }
  switch_id_hex.replace(pos, node_id.length(), node_id);
  for (uint position = 2; position < switch_id_hex.length(); position +=3) {
    if (switch_id_hex.at(position) != ':') {
      switch_id_hex.insert(position, ":");
    }
  }
  return switch_id_hex;
}
}  // namespace odcdriver
}  // namespace unc
