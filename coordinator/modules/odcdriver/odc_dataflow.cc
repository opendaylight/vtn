/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_dataflow.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcDataFlowCommand::OdcDataFlowCommand(
    unc::restjson::ConfFileValues_t conf_values)
    : age_interval_(DEFAULT_AGE_INTERVAL),
    conf_file_values_(conf_values) {
      ODC_FUNC_TRACE;
    }

// Destructor
OdcDataFlowCommand::~OdcDataFlowCommand() {
}


UncRespCode OdcDataFlowCommand::read_cmd(
    unc::driver::controller *ctr,
    unc::vtnreadutil::driver_read_util* df_util) {
  ODC_FUNC_TRACE;

  key_dataflow_t key_dataflow;

  UncRespCode key_read_resp =
      unc::vtnreadutil::driver_read_util_io<key_dataflow_t>::read_key_val
      (&key_dataflow, PFC_TRUE, df_util);

  if ( key_read_resp != UNC_RC_SUCCESS )
    pfc_log_info("Error reading KEy");

  UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;
  std::vector<string> vtn_names;
  resp_code_ = fetch_config_vtn(ctr, NULL, vtn_names);
  if (resp_code_ != UNC_RC_SUCCESS) {
    pfc_log_error("%s:failed to get VTN-List .rt, %u",
                  PFC_FUNCNAME, resp_code_);
    return resp_code_;
  }
  int check_count = 0;
  std::vector<string> ::iterator it_vtn_name = vtn_names.begin();
  for (; it_vtn_name != vtn_names.end(); ++it_vtn_name) {
    pfc_log_trace("Vtns present in list  %s", (*it_vtn_name).c_str());
    std::string url = get_dataflow_url(*it_vtn_name, key_dataflow);
    if (url == "") {
      pfc_log_error("%s:Invalid Key", PFC_FUNCNAME);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                                          ctr->get_user_name(),
                                          ctr->get_pass_word());
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
        pfc_log_trace("All Flows : %s", data);
        UncRespCode ret_val =  parse_flow_response(key_dataflow,
                                                   df_util,
                                                   data);
        if (ret_val == UNC_RC_NO_SUCH_INSTANCE) {
          check_count = 1;
          pfc_log_trace("No such Inst. count : %d", check_count);
          continue;

        } else if (ret_val != UNC_RC_SUCCESS) {
          pfc_log_error("Error occured while parsing");
          vtn_names.clear();
          return ret_val;
        }
      }
      check_count = 0;
    }
  }
  if (check_count == 1) {
    vtn_names.clear();
    return  UNC_RC_NO_SUCH_INSTANCE;
  }

  return UNC_RC_SUCCESS;
}


// fetch child configurations for the parent kt
UncRespCode OdcDataFlowCommand::fetch_config_vtn(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<string> &vtn_name) {
  ODC_FUNC_TRACE;
  UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;
  std::vector<unc::vtndrvcache::ConfigNode*> cfg_list;
  // vtn class object....
  OdcVtnCommand *odc_vtn =  new OdcVtnCommand(conf_file_values_);
  resp_code_ = odc_vtn->fetch_config(ctr, NULL, cfg_list);
  if (resp_code_ != UNC_RC_SUCCESS) {
    pfc_log_error("%s:failed to get VTN-List .rt, %u",
                  PFC_FUNCNAME, resp_code_);
    delete odc_vtn;
    return resp_code_;
  }
  std::vector<unc::vtndrvcache::ConfigNode*>::iterator node_itr =
      cfg_list.begin();
  for (; node_itr != cfg_list.end(); ++node_itr) {
    unc::vtndrvcache::ConfigNode *cfg_node = *node_itr;
    unc::vtndrvcache::CacheElementUtil<key_vtn_t, val_vtn_t, val_vtn_t, uint32_t>
        *cfgnode_ctr = static_cast<unc::vtndrvcache::CacheElementUtil
        <key_vtn_t, val_vtn_t, val_vtn_t, uint32_t>*> (cfg_node);
    // std::string vtn_name =reinterpret_cast<char*>
    // (cfgnode_ctr->get_key_structure()->vtn_name);
    vtn_name.push_back(reinterpret_cast<char*>
                       (cfgnode_ctr->get_key_structure()->vtn_name));
  }
  // parse .. vtn name add

  return UNC_RC_SUCCESS;
}


// Constructing URL for vbridge, inject request to controller
std::string OdcDataFlowCommand::get_dataflow_url(std::string vtn_name,
                                                 key_dataflow key) {
  ODC_FUNC_TRACE;
  std::string url = "";
  url.append(BASE_URL);
  url.append(CONTAINER_NAME);
  url.append(VTNS);
  url.append(SLASH);
  url.append(vtn_name);
  url.append(FLOWS);
  url.append(DETAIL);
  pfc_log_debug("%s: URL before filter:%s", PFC_FUNCNAME, url.c_str());

  // src_mac check
  uint8_t mac_arr[VAL_MAC_ADDR_SIZE];
  memset(&mac_arr, 0, VAL_MAC_ADDR_SIZE);
  string srcMac = "";
  if (memcmp(key.src_mac_address, mac_arr, VAL_MAC_ADDR_SIZE) > 0) {
    pfc_log_debug("valid src_mac ");
    pfc_log_debug("key mac_address %02x:%02x:%02x:%02x:%02x:%02x ",
                  key.src_mac_address[0], key.src_mac_address[1],
                  key.src_mac_address[2], key.src_mac_address[3],
                  key.src_mac_address[4], key.src_mac_address[5]);

    // convert key.src_mac_address to string
    char mac[32] = { 0 };
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
            key.src_mac_address[0],
            key.src_mac_address[1],
            key.src_mac_address[2],
            key.src_mac_address[3],
            key.src_mac_address[4],
            key.src_mac_address[5]);
    srcMac = mac;
    pfc_log_debug("src_mac:%s", srcMac.c_str());
  } else {
    pfc_log_error("%s: src_mac is missing in key_struct", PFC_FUNCNAME);
    return "";
  }

  // switch_id check
  char* switch_id = reinterpret_cast<char*> (key.switch_id);
  pfc_log_debug("URL-switch_id:%s", switch_id);
  if (strlen(switch_id) == 0) {
    pfc_log_error("%s: switch_id is missing in key_struct", PFC_FUNCNAME);
    return "";
  }
  // convert switch to odc type id
  std::string node = switch_to_odc_type(switch_id);
  pfc_log_debug("URL-node:%s", node.c_str());


  // port_id check
  char* port_name = reinterpret_cast<char*> (key.port_id);
  pfc_log_debug("URL-port_id:%s", port_name);
  if (strlen(port_name) == 0) {
    pfc_log_error("%s: port_id is missing in key_struct", PFC_FUNCNAME);
    return "";
  }

  // vlan-id check
  unsigned int vlan_id = 0;
  if (key.vlan_id) {
    if (key.vlan_id != 0xffff) {
      vlan_id = key.vlan_id;
      pfc_log_info("vlan-id : %u", vlan_id);
    } else {
      pfc_log_info("vlan-id : not set.");
    }
  } else {
    pfc_log_error("%s: vlan_id is missing in key_struct", PFC_FUNCNAME);
    return "";
  }
  std::stringstream ss_vlan;
  ss_vlan << vlan_id;
  std::string vlan_str = ss_vlan.str();

  // append Node+portName+srcMac+srcVlan
  url.append("?node=");
  url.append(node);
  url.append("&portName=");
  url.append(port_name);
  url.append("&srcMac=");
  url.append(srcMac);
  url.append("&srcVlan=");
  url.append(vlan_str);

  pfc_log_debug("%s: final URL:%s", PFC_FUNCNAME, url.c_str());
  return url;
}

std::string OdcDataFlowCommand::switch_to_odc_type(char* switch_id) {
  std::string sw_id= switch_id;
  std::replace(sw_id.begin(), sw_id.end(), '-', ':');
  sw_id.insert(2, ":");
  sw_id.insert(8, ":");
  sw_id.insert(14, ":");
  sw_id.insert(20, ":");

  return sw_id;
}

UncRespCode OdcDataFlowCommand::parse_flow_response(
    key_dataflow_t& key_dataflow,
    unc::vtnreadutil::driver_read_util* df_util,
    char *data) {
  ODC_FUNC_TRACE;
  UncRespCode resp_code = UNC_DRV_RC_ERR_GENERIC;
  json_object* jobj = unc::restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("%s: json_object_is_null", PFC_FUNCNAME);
    return resp_code;
  }
  uint32_t array_length = 0;
  json_object* jobj_data = NULL;
  uint32_t ret_val = unc::restjson::JsonBuildParse::parse(jobj,
                                                          "dataflow",
                                                          -1,
                                                          jobj_data);
  if ((json_object_is_type(jobj_data, json_type_null))  ||
      (restjson::REST_OP_SUCCESS != ret_val)) {
    pfc_log_error("%s: json datai is null", PFC_FUNCNAME);
    json_object_put(jobj);
    return resp_code;
  }
  if (json_object_is_type(jobj_data, json_type_array)) {
    array_length = restjson::JsonBuildParse::get_array_length(jobj,
                                                              "dataflow");
  }
  if (array_length == 0) {
    pfc_log_info("%s: No Entries received from Controller", PFC_FUNCNAME);
    return UNC_RC_NO_SUCH_INSTANCE;
  }
  for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {
    resp_code = parse_flow_response_values(key_dataflow,
                                           df_util,
                                           jobj_data,
                                           arr_idx);
    if (UNC_DRV_RC_ERR_GENERIC == resp_code) {
      json_object_put(jobj);
      return resp_code;
    }
  }
  json_object_put(jobj);
  return UNC_RC_SUCCESS;
}

UncRespCode OdcDataFlowCommand::parse_flow_response_values(
    key_dataflow_t& key_dataflow,
    unc::vtnreadutil::driver_read_util* df_util,
    json_object *jobj_data,
    uint32_t arr_idx) {
  ODC_FUNC_TRACE;
  uint32_t flowid = 0;
  uint32_t creationTime = 0;
  uint32_t hardTimeout = 0;
  uint32_t idleTimeout = 0;

  // Populate values to dataflow
  unc::dataflow::DataflowDetail *df_segm =
      new unc::dataflow::DataflowDetail(
          unc::dataflow::kidx_val_df_data_flow_cmn);
  unc::dataflow::DataflowCmn *df_cmn =
      new unc::dataflow::DataflowCmn(false, df_segm);

  /*
     if (df_cmn->df_segment->df_common != NULL) {
     pfc_log_info("memset df_common");
     memset(df_cmn->df_segment->df_common, 0, sizeof(val_df_data_flow_cmn_t));
     }
     */

  uint32_t ret_val = restjson::JsonBuildParse::parse(jobj_data, "id",
                                                     arr_idx, flowid);
  pfc_log_info("flowid 1 -- %d", flowid);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ret_val = restjson::JsonBuildParse::parse(jobj_data, "creationTime",
                                            arr_idx, creationTime);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("Creation Time -- %d", creationTime);
  ret_val = restjson::JsonBuildParse::parse(jobj_data, "idleTimeout",
                                            arr_idx, idleTimeout);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("IdleTimeout Time -- %d", idleTimeout);
  ret_val = restjson::JsonBuildParse::parse(jobj_data, "hardTimeout",
                                            arr_idx, hardTimeout);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("hardTimeout Time -- %d", hardTimeout);
  json_object  *jobj_ingressport = NULL;
  json_object *jobj_node = NULL;

  ret_val = restjson::JsonBuildParse::parse(jobj_data, "ingressPort",
                                            arr_idx, jobj_ingressport);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string switch_id = "";
  ret_val = restjson::JsonBuildParse::parse(jobj_ingressport, "node",
                                            -1, jobj_node);
  if (json_object_is_type(jobj_node, json_type_null)) {
    json_object_put(jobj_data);
    pfc_log_error("%s jobj_vbr NULL", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ret_val = restjson::JsonBuildParse::parse(jobj_node, "id",
                                            -1, switch_id);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("ingress_switch_id -- %s", switch_id.c_str());

  json_object *jobj_port = NULL;
  ret_val = restjson::JsonBuildParse::parse(jobj_ingressport, "port",
                                            -1, jobj_port);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string station_id = "";
  ret_val = restjson::JsonBuildParse::parse(jobj_port, "id",
                                            -1, station_id);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("station_id_id -- %s", station_id.c_str());
  std::string in_portname = "";
  ret_val = restjson::JsonBuildParse::parse(jobj_port, "name",
                                            -1, in_portname);
  pfc_log_info("in_portname_name -- %s", in_portname.c_str());
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  // egressPort
  ret_val = restjson::JsonBuildParse::parse(jobj_data, "egressPort",
                                            arr_idx, jobj_ingressport);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing egressPort");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  std::string egress_switch_id = "";
  ret_val = restjson::JsonBuildParse::parse(jobj_ingressport, "node",
                                            -1, jobj_node);
  if (json_object_is_type(jobj_node, json_type_null)) {
    json_object_put(jobj_data);
    pfc_log_error("%s jobj_vbr NULL", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ret_val = restjson::JsonBuildParse::parse(jobj_node, "id",
                                            -1, egress_switch_id);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("egress_switch_id -- %s", egress_switch_id.c_str());

  jobj_port = NULL;
  ret_val = restjson::JsonBuildParse::parse(jobj_ingressport, "port",
                                            -1, jobj_port);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string out_station_id = "";
  ret_val = restjson::JsonBuildParse::parse(jobj_port, "id",
                                            -1, out_station_id);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("out_station_id_id -- %s", out_station_id.c_str());
  std::string out_portname = "";
  ret_val = restjson::JsonBuildParse::parse(jobj_port, "name",
                                            -1, out_portname);
  pfc_log_info("out_portname_name -- %s", out_portname.c_str());
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  // action
  json_object* jobj_actions = NULL;
  ret_val = restjson::JsonBuildParse::parse(jobj_data, "actions",
                                            arr_idx, jobj_actions);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  uint32_t action_array_len = 0;
  if (json_object_is_type(jobj_actions, json_type_array)) {
    // check for action-popvlan
    action_array_len = restjson::JsonBuildParse::get_array_length(
        jobj_actions);
    pfc_log_info("--------------action_count arraylength --- %d",
                 action_array_len);
    if (action_array_len > 0) {
      df_cmn->df_segment->df_common->action_count = action_array_len;
      df_cmn->df_segment->df_common->valid[kidxDfDataFlowActionCount] =
          UNC_VF_VALID;
      pfc_log_info("action_count %u",
                   df_cmn->df_segment->df_common->action_count);
      json_object *jobj_popvlan = NULL;
      uint32_t pop_array_len =
          restjson::JsonBuildParse::get_array_length(jobj_popvlan);
      pfc_log_info("pop_array_len = %d and array_index=%d ",
                   pop_array_len, arr_idx);
      ret_val = restjson::JsonBuildParse::parse(jobj_actions, "popvlan",
                                                pop_array_len, jobj_popvlan);
      if (restjson::REST_OP_SUCCESS == ret_val && jobj_popvlan != NULL) {
        pfc_log_debug("popvlan success");
        unc::dataflow::val_actions_vect_st *ptr =
            new unc::dataflow::val_actions_vect_st;
        memset(ptr, 0, sizeof(unc::dataflow::val_actions_vect_st));
        ptr->action_type = UNC_ACTION_STRIP_VLAN;
        pfc_log_trace("vln_action_type : %d vln_action_type_enum : %d",
                      ptr->action_type, (uint32_t)UNC_ACTION_STRIP_VLAN);
        df_cmn->df_segment->actions.push_back(ptr);
      } else {
        pfc_log_error(" Error occured while parsing popvlan.continue");
      }

      // check for action-vlanid
      json_object *jobj_vlanid = NULL;
      uint32_t vlanid_array_len =
          restjson::JsonBuildParse::get_array_length(jobj_vlanid);
      ret_val = restjson::JsonBuildParse::parse(jobj_actions, "vlanid",
                                                vlanid_array_len, jobj_vlanid);
      if (restjson::REST_OP_SUCCESS == ret_val && jobj_vlanid != NULL) {
        pfc_log_debug("parsing vlanid success");
        uint32_t action_vlanid = 0;
        ret_val = restjson::JsonBuildParse::parse(jobj_vlanid, "vlan",
                                                  -1, action_vlanid);
        if (restjson::REST_OP_SUCCESS == ret_val) {
          pfc_log_debug("parsing vlanid success inner loop");
          unc::dataflow::val_actions_vect_st *ptr =
              new unc::dataflow::val_actions_vect_st;
          memset(ptr, 0, sizeof(unc::dataflow::val_actions_vect_st));
          ptr->action_type = UNC_ACTION_SET_VLAN_ID;
          pfc_log_trace("vln_action_type : %d vln_action_type_enum : %d",
                        ptr->action_type, (uint32_t)UNC_ACTION_SET_VLAN_ID);
          val_df_flow_action_set_vlan_id *set_vlan_id =
              new val_df_flow_action_set_vlan_id;
          memset(set_vlan_id, 0, sizeof(val_df_flow_action_set_vlan_id_t));
          set_vlan_id->vlan_id = action_vlanid;
          ptr->action_st_ptr = (void *) set_vlan_id;
          df_cmn->df_segment->actions.push_back(ptr);
        } else {
          pfc_log_error(" Error occured while parsing vlanid.continue");
        }
      } else {
        pfc_log_error(" Error occured while parsing vlanid.continue");
      }
    } else {
      df_cmn->df_segment->df_common->valid[kidxDfDataFlowActionCount] =
          UNC_VF_INVALID;
    }
  }

  // match - START
  json_object *jobj_match = NULL;
  json_object *jobj_ethernet = NULL;
  json_object *jobj_inetMatch = NULL;
  json_object *jobj_inet4 = NULL;
  pfc_bool_t match_status = PFC_FALSE;
  ret_val = restjson::JsonBuildParse::parse(jobj_data, "match",
                                            arr_idx, jobj_match);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing match");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  // check match-ethernet
  ret_val = restjson::JsonBuildParse::parse(jobj_match, "ethernet",
                                            -1, jobj_ethernet);
  if (restjson::REST_OP_SUCCESS == ret_val) {
    pfc_log_debug(" match-ethernet success. fill val st ");
    df_cmn->df_segment->df_common->match_count = 1;
    pfc_log_debug("match count = %u",
                  df_cmn->df_segment->df_common->match_count);

    // match-ethernet-vlan
    uint32_t vlan = 0;
    ret_val = restjson::JsonBuildParse::parse(jobj_ethernet, "vlan",
                                              -1, vlan);
    pfc_log_info("vlan -- %d", vlan);
    if (restjson::REST_OP_SUCCESS == ret_val) {
      if (vlan !=0) {
        pfc_log_info("valid vlan. set and send");
        val_df_flow_match_vlan_id_t* val_vlan_id_obj =
            new val_df_flow_match_vlan_id_t;
        memset(val_vlan_id_obj, 0, sizeof(val_df_flow_match_vlan_id_t));
        val_vlan_id_obj->vlan_id = vlan;
        df_cmn->df_segment->matches.insert(
            std::pair<UncDataflowFlowMatchType, void *>
            (UNC_MATCH_VLAN_ID, val_vlan_id_obj));
      } else {
        pfc_log_info("vlan is 0 and not set to send");
      }
    } else {
      pfc_log_error(" Error occured while parsing.continue ");
    }

    // match-ethernet-src
    std::string src = "";
    ret_val = restjson::JsonBuildParse::parse(jobj_ethernet, "src",
                                              -1, src);
    pfc_log_info("src df -- %s", src.c_str());
    if (restjson::REST_OP_SUCCESS == ret_val) {
      pfc_log_info("ethernet-src. success");
      val_df_flow_match_dl_addr_t* val_dl_addr_obj =
          new val_df_flow_match_dl_addr_t;
      memset(val_dl_addr_obj, 0, sizeof(val_df_flow_match_dl_addr_t));
      OdcUtil util_obj;
      util_obj.convert_macstring_to_uint8(src, val_dl_addr_obj->dl_addr);
      pfc_log_debug(
          "match-ethernet-src mac address %02x:%02x:%02x:%02x:%02x:%02x ",
          val_dl_addr_obj->dl_addr[0],
          val_dl_addr_obj->dl_addr[1],
          val_dl_addr_obj->dl_addr[2],
          val_dl_addr_obj->dl_addr[3],
          val_dl_addr_obj->dl_addr[4],
          val_dl_addr_obj->dl_addr[5]);
      val_dl_addr_obj->v_mask = UNC_MATCH_MASK_INVALID;
      df_cmn->df_segment->matches.insert(
          std::pair<UncDataflowFlowMatchType, void *>
          (UNC_MATCH_DL_SRC, val_dl_addr_obj));
    } else {
      pfc_log_error(" Error occured while parsing ethernet-src.continue");
    }

    // match-ethernet-dst
    std::string dst = "";
    ret_val = restjson::JsonBuildParse::parse(jobj_ethernet, "dst",
                                              -1, dst);
    pfc_log_info("dst -- %s", dst.c_str());
    if (restjson::REST_OP_SUCCESS == ret_val) {
      pfc_log_info("ethernet-dst. success");
      val_df_flow_match_dl_addr_t* val_dl_addr_obj =
          new val_df_flow_match_dl_addr_t;
      memset(val_dl_addr_obj, 0, sizeof(val_df_flow_match_dl_addr_t));
      OdcUtil util_obj;
      util_obj.convert_macstring_to_uint8(dst, val_dl_addr_obj->dl_addr);
      pfc_log_debug(
          "match-ethernet-dst mac address %02x:%02x:%02x:%02x:%02x:%02x ",
          val_dl_addr_obj->dl_addr[0],
          val_dl_addr_obj->dl_addr[1],
          val_dl_addr_obj->dl_addr[2],
          val_dl_addr_obj->dl_addr[3],
          val_dl_addr_obj->dl_addr[4],
          val_dl_addr_obj->dl_addr[5]);
      val_dl_addr_obj->v_mask = UNC_MATCH_MASK_INVALID;
      df_cmn->df_segment->matches.insert(
          std::pair<UncDataflowFlowMatchType, void *>
          (UNC_MATCH_DL_DST, val_dl_addr_obj));
    } else {
      pfc_log_error(" Error occured while parsing ethernet-dst.continue");
    }
    match_status = PFC_TRUE;
    pfc_log_debug("match_status:%d", match_status);
  } else {
    pfc_log_error("Error occured while parsing ethernet."
                  "continue for inetMatch.");
  }

  // check match-inetMatch
  ret_val = restjson::JsonBuildParse::parse(jobj_match, "inetMatch",
                                            -1, jobj_inetMatch);
  if (restjson::REST_OP_SUCCESS == ret_val) {
    pfc_log_debug(" match-inetMatch success. Check for inet4 ");
    ret_val = restjson::JsonBuildParse::parse(jobj_match, "inet4",
                                              -1, jobj_inet4);

    if (restjson::REST_OP_SUCCESS == ret_val) {
      pfc_log_debug(" match-inetMatch-inet4 success. fill val st ");

      // match-inetMatch-inet4-src
      std::string src = "";
      ret_val = restjson::JsonBuildParse::parse(jobj_inet4, "src",
                                                -1, src);
      pfc_log_info("src df -- %s", src.c_str());
      if (restjson::REST_OP_SUCCESS == ret_val) {
        pfc_log_info("inetMatch-inet4-src. success");
        val_df_flow_match_ipv4_addr_t* val_ipv4_addr_obj =
            new val_df_flow_match_ipv4_addr_t;
        memset(val_ipv4_addr_obj, 0, sizeof(val_df_flow_match_ipv4_addr_t));
        memcpy(&val_ipv4_addr_obj->ipv4_addr, src.c_str(),
               sizeof(val_df_flow_match_ipv4_addr_t));
        val_ipv4_addr_obj->v_mask = UNC_MATCH_MASK_INVALID;
        df_cmn->df_segment->matches.insert(
            std::pair<UncDataflowFlowMatchType, void *>(UNC_MATCH_IPV4_SRC,
                                                        val_ipv4_addr_obj));
      } else {
        pfc_log_error(" Error occured while parsing"
                      "inetMatch-inet4-src.continue");
      }

      // match-inetMatch-inet4-dst
      std::string dst = "";
      ret_val = restjson::JsonBuildParse::parse(jobj_inet4, "dst",
                                                -1, dst);
      pfc_log_info("dst -- %s", dst.c_str());
      if (restjson::REST_OP_SUCCESS == ret_val) {
        pfc_log_info("match-inetMatch-inet4-dst. success");
        val_df_flow_match_ipv4_addr_t* val_ipv4_addr_obj =
            new val_df_flow_match_ipv4_addr_t;
        memset(val_ipv4_addr_obj, 0, sizeof(val_df_flow_match_ipv4_addr_t));
        memcpy(&val_ipv4_addr_obj->ipv4_addr, dst.c_str(),
               sizeof(val_df_flow_match_ipv4_addr_t));
        val_ipv4_addr_obj->v_mask = UNC_MATCH_MASK_INVALID;
        df_cmn->df_segment->matches.insert(
            std::pair<UncDataflowFlowMatchType, void *>(UNC_MATCH_IPV4_DST,
                                                        val_ipv4_addr_obj));
      } else {
        pfc_log_error(" Error occured while parsing"
                      "match-inetMatch-inet4-dst.continue");
      }

      // match-inetMatch-inet4-protocol
      uint32_t proto = 0;
      ret_val = restjson::JsonBuildParse::parse(jobj_inet4, "protocol",
                                                -1, proto);
      pfc_log_info("protocol -- %d", proto);
      if (restjson::REST_OP_SUCCESS == ret_val) {
        pfc_log_info("match-inetMatch-inet4-protocol. success");
        val_df_flow_match_ip_proto_t* val_ip_proto_obj =
            new val_df_flow_match_ip_proto_t;
        memset(val_ip_proto_obj, 0, sizeof(val_df_flow_match_ip_proto_t));
        val_ip_proto_obj->ip_proto = proto;
        df_cmn->df_segment->matches.insert(
            std::pair<UncDataflowFlowMatchType, void *>(UNC_MATCH_IP_PROTO,
                                                        val_ip_proto_obj));
      } else {
        pfc_log_error(" Error occured while parsing"
                      "match-inetMatch-inet4-protocol.continue");
      }
      match_status = PFC_TRUE;
      pfc_log_debug("match_status:%d", match_status);
    } else {
      pfc_log_error(" Error occured while parsing inetMatch-inet4. continue");
    }
  } else {
    pfc_log_error(" Error occured while parsing inetMatch. continue");
  }

  if (match_status == PFC_TRUE) {
    pfc_log_error("match_status is true. match found");
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowMatchCount] =
        UNC_VF_VALID;
  } else {
    pfc_log_error("match_status is false. match NOT found");
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowMatchCount] =
        UNC_VF_INVALID;
  }
  // match - END


  struct path_info {
    std::string switchid;
    std::string inport;
    std::string outport;
  };
  json_object* jobj_noderoute = NULL;
  ret_val = restjson::JsonBuildParse::parse(jobj_data, "noderoute",
                                            arr_idx, jobj_noderoute);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  uint32_t node_array_len = 0;
  if (json_object_is_type(jobj_noderoute, json_type_array)) {
    // node_array_len = restjson::JsonBuildParse::get_array_length(jobj_data,
    node_array_len =
        restjson::JsonBuildParse::get_array_length(jobj_noderoute);
    pfc_log_info("--------------path-info arraylength --- %d", node_array_len);
    struct path_info pathinfo_record[node_array_len];
    for (uint32_t node_arr_idx = 0; node_arr_idx < node_array_len;
         node_arr_idx++) {
      json_object* jobj_nrnode = NULL;
      ret_val = restjson::JsonBuildParse::parse(jobj_noderoute, "node",
                                                node_arr_idx, jobj_nrnode);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error(" Error occured while parsing ");
        return UNC_DRV_RC_ERR_GENERIC;
      }

      std::string pathinfo_switch_id = "";
      ret_val = restjson::JsonBuildParse::parse(jobj_nrnode, "id",
                                                -1, pathinfo_switch_id);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error(" Error occured while parsing ");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      pfc_log_info("pathinfo_switch_id -- %s", pathinfo_switch_id.c_str());
      pathinfo_record[node_arr_idx].switchid = pathinfo_switch_id;
      json_object* jobj_input = NULL;
      ret_val = restjson::JsonBuildParse::parse(jobj_noderoute, "input",
                                                node_arr_idx, jobj_input);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error(" Error occured while parsing ");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      std::string inport_name = "";
      ret_val = restjson::JsonBuildParse::parse(jobj_input, "name",
                                                -1, inport_name);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error(" Error occured while parsing ");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      pfc_log_info("pathinfo_inport_name -- %s", inport_name.c_str());
      pathinfo_record[node_arr_idx].inport =  inport_name;

      json_object* jobj_output = NULL;
      ret_val = restjson::JsonBuildParse::parse(jobj_noderoute, "output",
                                                node_arr_idx, jobj_output);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error(" Error occured while parsing ");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      std::string outport_name = "";
      ret_val = restjson::JsonBuildParse::parse(jobj_output, "name",
                                                -1, outport_name);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error(" Error occured while parsing ");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      pfc_log_info("pathinfo_outport_name -- %s", outport_name.c_str());
      pathinfo_record[node_arr_idx].outport = outport_name;
    }

    pfc_log_info("controller name %s", key_dataflow.controller_name);
    /* copy controller name to dataflow common struct */
    memcpy(df_cmn->df_segment->df_common->controller_name,
           key_dataflow.controller_name,
           sizeof(key_dataflow.controller_name));
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowControllerName] =
        UNC_VF_VALID;
    /* copy controller type to dataflow common struct */
    df_cmn->df_segment->df_common->controller_type = UNC_CT_ODC;
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowControllerType] =
        UNC_VF_VALID;

    /* copy flow-id to dataflow common struct */
    df_cmn->df_segment->df_common->flow_id = flowid;
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowFlowId] = UNC_VF_VALID;
    pfc_log_info(" flow-id : %" PFC_PFMT_u64,
                 df_cmn->df_segment->df_common->flow_id);
    /* copy Status to dataflow common struct */
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowStatus] =
        UNC_VF_INVALID;
    /* copy flow_type to dataflow common struct */
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowFlowType] =
        UNC_VF_INVALID;
    /* copy PolicyIndex to dataflow common struct */
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowPolicyIndex] =
        UNC_VF_INVALID;
    /* copy VtnId to dataflow common struct */
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowVtnId] =
        UNC_VF_INVALID;
    /* copy Ingress SwitchId to dataflow common struct */
    strncpy(reinterpret_cast<char*> (
            df_cmn->df_segment->df_common->ingress_switch_id),
            switch_id.c_str(),
            strlen(switch_id.c_str()));
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowIngressSwitchId] =
        UNC_VF_VALID;
    pfc_log_info("switch_id : %s",
                 df_cmn->df_segment->df_common->ingress_switch_id);

    /* copy Ingress Port to dataflow common struct */
    strncpy(reinterpret_cast<char*>(df_cmn->df_segment->df_common->in_port),
            in_portname.c_str(),
            strlen(in_portname.c_str()));
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowInPort] = UNC_VF_VALID;
    pfc_log_info("Ingress InPortName : %s",
                 df_cmn->df_segment->df_common->in_port);

    /* copy Ingress StationId to dataflow common struct */
    df_cmn->df_segment->df_common->in_station_id = atoi(station_id.c_str());
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowInStationId] =
        UNC_VF_VALID;
    pfc_log_trace("station_id : %" PFC_PFMT_u64,
                  df_cmn->df_segment->df_common->in_station_id);

    /* copy In-Domain DEFAULT to dataflow common struct */
    strncpy(reinterpret_cast<char*> (df_cmn->df_segment->df_common->in_domain),
            DOM_NAME.c_str(), strlen(DOM_NAME.c_str()));
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowInDomain] =
        UNC_VF_VALID;
    pfc_log_trace("ingress_domain %s",
                  df_cmn->df_segment->df_common->in_domain);

    /* copy Egress SwitchId to dataflow common struct */
    strncpy(reinterpret_cast<char*> (
            df_cmn->df_segment->df_common->egress_switch_id),
        egress_switch_id.c_str(),
            strlen(egress_switch_id.c_str()));
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowEgressSwitchId] =
        UNC_VF_VALID;
    pfc_log_trace("Egress SwitchId : %s",
                  df_cmn->df_segment->df_common->egress_switch_id);

    /* copy Egress OutPort to dataflow common struct */
    strncpy(reinterpret_cast<char*> (
            df_cmn->df_segment->df_common->out_port), out_portname.c_str(),
            strlen(out_portname.c_str()));
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowOutPort] = UNC_VF_VALID;
    pfc_log_info("Egress Out-port_name %s",
                 df_cmn->df_segment->df_common->out_port);

    /* copy Egress OutStationId to dataflow common struct */
    df_cmn->df_segment->df_common->out_station_id =
        atoi(out_station_id.c_str());
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowOutStationId] =
        UNC_VF_VALID;
    pfc_log_trace("Out-station_id : %" PFC_PFMT_u64,
                  df_cmn->df_segment->df_common->out_station_id);

    /* copy Out-Domain DEFAULT to dataflow common struct */
    strncpy(reinterpret_cast<char*> (
            df_cmn->df_segment->df_common->out_domain),
            DOM_NAME.c_str(), strlen(DOM_NAME.c_str()));
    df_cmn->df_segment->df_common->valid[kidxDfDataFlowOutDomain] =
        UNC_VF_VALID;
    pfc_log_trace("egress_Out-domain %s",
                  df_cmn->df_segment->df_common->out_domain);

    /* copy Path-info to dataflow common struct */
    if (node_array_len > 0) {
      df_cmn->df_segment->df_common->path_info_count = node_array_len;
      df_cmn->df_segment->df_common->valid[kidxDfDataFlowPathInfoCount] =
          UNC_VF_VALID;
      for (uint32_t count = 0; count < node_array_len; count++) {
        val_df_data_flow_path_info *ptr = new val_df_data_flow_path_info;
        memset(ptr, 0, sizeof(val_df_data_flow_path_info_t));
        /* copy Path-info SwitchId to dataflow common struct */
        strncpy(reinterpret_cast<char*> (ptr->switch_id),
                pathinfo_record[count].switchid.c_str(),
                strlen(pathinfo_record[count].switchid.c_str()));
        pfc_log_info("pathinfo_Switch_name1 -- %s",
                     pathinfo_record[count].switchid.c_str());
        ptr->valid[kidxDfDataFlowPathInfoSwitchId] = UNC_VF_VALID;
        strncpy(reinterpret_cast<char*> (ptr->in_port),
                pathinfo_record[count].inport.c_str(),
                strlen(pathinfo_record[count].inport.c_str()));
        pfc_log_info("pathinfo_Inport_name1 -- %s",
                     pathinfo_record[count].inport.c_str());
        ptr->valid[kidxDfDataFlowPathInfoInPort] = UNC_VF_VALID;
        strncpy(reinterpret_cast<char*> (ptr->out_port),
                pathinfo_record[count].outport.c_str(),
                strlen(pathinfo_record[count].outport.c_str()));
        pfc_log_info("pathinfo_Outport_name1 -- %s",
                     pathinfo_record[count].outport.c_str());
        ptr->valid[kidxDfDataFlowPathInfoOutPort] = UNC_VF_VALID;
        df_cmn->df_segment->path_infos.push_back(ptr);
      }
    } else {
      df_cmn->df_segment->df_common->valid[kidxDfDataFlowPathInfoCount] =
          UNC_VF_INVALID;
      pfc_log_trace("There is no Physical path related Information");
    }

  } else {
    pfc_log_info("no element for noderoute array");
  }
  unc::vtnreadutil::driver_dataflow_read_util::add_read_value
      (df_cmn, df_util);

  // df_util.appendFlow(df_cmn);
  return UNC_RC_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
