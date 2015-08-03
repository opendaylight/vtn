/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vtn_dataflow.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcVtnDataFlowCommand::OdcVtnDataFlowCommand(
    unc::restjson::ConfFileValues_t conf_values)
    : age_interval_(DEFAULT_AGE_INTERVAL),
    conf_file_values_(conf_values) {
      ODC_FUNC_TRACE;
    }

// Destructor
OdcVtnDataFlowCommand::~OdcVtnDataFlowCommand() {
}


UncRespCode OdcVtnDataFlowCommand::read_cmd(
    unc::driver::controller *ctr,
    unc::vtnreadutil::driver_read_util* df_util) {
  ODC_FUNC_TRACE;

  key_vtn_dataflow_t key_vtn_dataflow;

  UncRespCode key_read_resp =
      unc::vtnreadutil::driver_read_util_io<key_vtn_dataflow_t>::read_key_val
      (&key_vtn_dataflow, PFC_TRUE, df_util);

  if ( key_read_resp != UNC_RC_SUCCESS )
    pfc_log_info("Error reading KEy");

  //UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;
  //std::vector<string> vtn_names;
  /*resp_code_ = fetch_config_vtn(ctr, NULL, vtn_names);
  if (resp_code_ != UNC_RC_SUCCESS) {
    pfc_log_error("%s:failed to get VTN-List .rt, %u",
                  PFC_FUNCNAME, resp_code_);
    return resp_code_;
  }
  int check_count = 0;
  std::vector<string> ::iterator it_vtn_name = vtn_names.begin();
  for (; it_vtn_name != vtn_names.end(); ++it_vtn_name) */
    pfc_log_trace("Vtn Name in dataflow  %s",
                  key_vtn_dataflow.vtn_key.vtn_name);
    std::string vtn_name = reinterpret_cast<char*> (key_vtn_dataflow.vtn_key.vtn_name);
    std::string url = get_dataflow_url(vtn_name,
                                       key_vtn_dataflow);
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
        UncRespCode ret_val =  parse_flow_response(key_vtn_dataflow,
                                                   df_util,
                                                   data);
        if (ret_val != UNC_RC_SUCCESS) {
          pfc_log_error("Error occured while parsing");
          //vtn_names.clear();
          return ret_val;
        }
      }
    }

  return UNC_RC_SUCCESS;
}


// fetch child configurations for the parent kt
UncRespCode OdcVtnDataFlowCommand::fetch_config_vtn(
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
    unc::vtndrvcache::CacheElementUtil<key_vtn_t, val_vtn_t,val_vtn_t, uint32_t>
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
std::string OdcVtnDataFlowCommand::get_dataflow_url(std::string vtn_name,
                                                 key_vtn_dataflow_t key) {
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
 /* char* switch_id = reinterpret_cast<char*> (key.switch_id);
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
   */
  // vlan-id check
  unsigned int vlan_id = 0;
  if (key.vlanid) {
    if (key.vlanid != 0xffff) {
      vlan_id = key.vlanid;
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
  //url.append("?node=");
  //url.append(node);
  //url.append("&portName=");
  //url.append(port_name);
  url.append("?srcMac=");
  url.append(srcMac);
  url.append("&srcVlan=");
  url.append(vlan_str);

  pfc_log_debug("%s: final URL:%s", PFC_FUNCNAME, url.c_str());
  return url;
}

std::string OdcVtnDataFlowCommand::switch_to_odc_type(char* switch_id) {
  std::string sw_id= switch_id;
  std::replace(sw_id.begin(), sw_id.end(), '-', ':');
  sw_id.insert(2, ":");
  sw_id.insert(8, ":");
  sw_id.insert(14, ":");
  sw_id.insert(20, ":");

  return sw_id;
}

UncRespCode OdcVtnDataFlowCommand::parse_flow_response(
    key_vtn_dataflow_t& key_vtn_dataflow,
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
    resp_code = parse_flow_response_values(key_vtn_dataflow,
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

UncRespCode OdcVtnDataFlowCommand::parse_flow_response_values(
    key_vtn_dataflow_t& key_vtn_dataflow,
    unc::vtnreadutil::driver_read_util* df_util,
    json_object *jobj_data,
    uint32_t arr_idx) {
  ODC_FUNC_TRACE;
  uint32_t flowid = 0;
  uint32_t creationTime = 0;
  uint32_t hardTimeout = 0;
  uint32_t idleTimeout = 0;
  uint32_t status = 0;
  uint32_t vlink_flag = 0;

  // Populate values to dataflow
  unc::dataflow::DataflowDetail *df_segm =
      new unc::dataflow::DataflowDetail(
          unc::dataflow::kidx_val_vtn_dataflow_cmn);
  unc::dataflow::DataflowCmn *df_cmn =
      new unc::dataflow::DataflowCmn(false, df_segm);
  df_cmn->df_segment->df_common = NULL;
  /*
     if (df_cmn->df_segment->vtn_df_common != NULL) {
     pfc_log_info("memset vtn_df_common");
     memset(df_cmn->df_segment->vtn_df_common, 0, sizeof(val_df_data_flow_cmn_t));
     }
     */

  uint32_t ret_val = restjson::JsonBuildParse::parse(jobj_data, "id",
                                                     arr_idx, flowid);
  pfc_log_info("flowid 1 -- %d", flowid);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing flowid");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = restjson::JsonBuildParse::parse(jobj_data, "creationTime",
                                            arr_idx, creationTime);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing cretiontime");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("Creation Time -- %d", creationTime);

  ret_val = restjson::JsonBuildParse::parse(jobj_data, "idleTimeout",
                                            arr_idx, idleTimeout);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing idle timeout");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("IdleTimeout Time -- %d", idleTimeout);

  ret_val = restjson::JsonBuildParse::parse(jobj_data, "hardTimeout",
                                            arr_idx, hardTimeout);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing hardtimeout");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("hardTimeout Time -- %d", hardTimeout);

  //ingress_vbridge
  json_object  *jobj_ingressnode = NULL;
  ret_val = restjson::JsonBuildParse::parse(jobj_data, "ingressNode",
                                                     arr_idx, jobj_ingressnode);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing jobj_ingressnode");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string in_vbridge = "";
  ret_val = restjson::JsonBuildParse::parse(jobj_ingressnode, "bridge",
                                                     -1, in_vbridge);

  if (restjson::REST_OP_SUCCESS != ret_val ||
      in_vbridge.empty()) {
    pfc_log_error("Error occured while parsing ingress_vbridge");
    //return UNC_DRV_RC_ERR_GENERIC;
  } else {
    pfc_log_trace("ingress_vbridge %s", in_vbridge.c_str());
    if(df_cmn->df_segment->vtn_df_common == NULL) {
       pfc_log_trace("df_cmn->df_segment->vtn_df_common is NULL");
    }
    strncpy(reinterpret_cast<char*>(df_cmn->df_segment->vtn_df_common->ingress_vnode),
            in_vbridge.c_str(),
            strlen(in_vbridge.c_str()));
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_VNODE_VVDC] = UNC_VF_VALID;
    pfc_log_trace("ingress_vbridge %s", df_cmn->df_segment->vtn_df_common->ingress_vnode);
  }

  std::string in_vterm = "";
  ret_val = restjson::JsonBuildParse::parse(jobj_ingressnode, "terminal",
                                                     -1, in_vterm);

  if (restjson::REST_OP_SUCCESS != ret_val ||
      in_vterm.empty()) {
    pfc_log_error("Error occured while parsing ingress_terminal");
    //return UNC_DRV_RC_ERR_GENERIC;
  } else {
    pfc_log_trace("ingress_terminal %s", in_vterm.c_str());
    if(df_cmn->df_segment->vtn_df_common == NULL) {
       pfc_log_trace("df_cmn->df_segment->vtn_df_common is NULL");
    }
    strncpy(reinterpret_cast<char*>(df_cmn->df_segment->vtn_df_common->ingress_vnode),
            in_vterm.c_str(),
            strlen(in_vterm.c_str()));
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_VNODE_VVDC] = UNC_VF_VALID;
    pfc_log_trace("ingress_terminal %s", df_cmn->df_segment->vtn_df_common->ingress_vnode);
  }
 //ingress_vinterface
 std::string in_vinterface = "";
 ret_val = restjson::JsonBuildParse::parse(jobj_ingressnode, "interface",
                                                        -1, in_vinterface);
 if (restjson::REST_OP_SUCCESS != ret_val) {
   pfc_log_error("Error occured while parsing in_vinterface");
   return UNC_DRV_RC_ERR_GENERIC;
 }
 if (!in_vinterface.empty()) {
  strncpy(reinterpret_cast<char*>(df_cmn->df_segment->vtn_df_common->ingress_vinterface),
          in_vinterface.c_str(),
          strlen(in_vinterface.c_str()));
     df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_VINTERFACE_VVDC] = UNC_VF_VALID;
   } else {
     df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_VINTERFACE_VVDC] = UNC_VF_INVALID;
   }
   pfc_log_trace("ingress_vinterface %s", df_cmn->df_segment->vtn_df_common->ingress_vinterface);


 //egress_vbridge
 json_object *jobj_egressnode = NULL;

 std::string out_vbridge = "";
 std::string out_vinterface = "";
 std::string out_vterm = "";
 ret_val = restjson::JsonBuildParse::parse(jobj_data, "egressNode",
                                                    arr_idx, jobj_egressnode);
 if (restjson::REST_OP_SUCCESS != ret_val ||
      (json_object_is_type(jobj_egressnode, json_type_null))) {
   pfc_log_info("Error occured while parsing egress_node is NULL");
   //return UNC_DRV_RC_ERR_GENERIC;
 } else {
 ret_val = restjson::JsonBuildParse::parse(jobj_egressnode, "bridge",
                                                  -1, out_vbridge);
 if(restjson::REST_OP_SUCCESS != ret_val) {
   pfc_log_error("Error occured while parsing egress_vbridge");
   return UNC_DRV_RC_ERR_GENERIC;
 }
 if(!out_vbridge.empty()) {
   strncpy(reinterpret_cast<char*>(df_cmn->df_segment->vtn_df_common->egress_vnode), 
           out_vbridge.c_str(),
          strlen(out_vbridge.c_str()));
     df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_VNODE_VVDC] = UNC_VF_VALID;
    } else {
     df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_VNODE_VVDC] = UNC_VF_INVALID;
   }
  pfc_log_trace("egress_vnode %s", df_cmn->df_segment->vtn_df_common->egress_vnode);

  ret_val = restjson::JsonBuildParse::parse(jobj_egressnode, "terminal",
                                                     -1, out_vterm);

  if (restjson::REST_OP_SUCCESS != ret_val ||
      out_vterm.empty()) {
    pfc_log_error("Error occured while parsing egress_terminal");
    //return UNC_DRV_RC_ERR_GENERIC;
  } else {
    pfc_log_trace("ingress_terminal %s", out_vterm.c_str());
    if(df_cmn->df_segment->vtn_df_common == NULL) {
       pfc_log_trace("df_cmn->df_segment->vtn_df_common is NULL");
    }
    strncpy(reinterpret_cast<char*>(df_cmn->df_segment->vtn_df_common->egress_vnode),
            out_vterm.c_str(),
            strlen(out_vterm.c_str()));
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_VNODE_VVDC] = UNC_VF_VALID;
    pfc_log_trace("ingress_terminal %s", df_cmn->df_segment->vtn_df_common->egress_vnode);
  }

 //egress_vinterface
 ret_val = restjson::JsonBuildParse::parse(jobj_egressnode, "interface",
                                                      -1, out_vinterface);
 if(restjson::REST_OP_SUCCESS != ret_val) {
   pfc_log_error("Error occured while parsing out_vinterface");
   return UNC_DRV_RC_ERR_GENERIC;
 }
 if(!out_vinterface.empty()) {
   strncpy(reinterpret_cast<char*>(df_cmn->df_segment->vtn_df_common->egress_vinterface),
                                   out_vinterface.c_str(),
                                   strlen(out_vinterface.c_str()));
     df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_VINTERFACE_VVDC] = UNC_VF_VALID;
   } else {
     df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_VINTERFACE_VVDC] = UNC_VF_INVALID;
   }
   pfc_log_trace("egress_vinterface %s", df_cmn->df_segment->vtn_df_common->egress_vinterface);
 }
 json_object *jobj_ingressport = NULL;
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
      df_cmn->df_segment->vtn_df_common->action_count = action_array_len;
      df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_ACTION_COUNT_VVDC] =
          UNC_VF_VALID;
      pfc_log_info("action_count %u",
                   df_cmn->df_segment->vtn_df_common->action_count);
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

    /*  json_object *jobj_drop = NULL;
      uint32_t drop_array_len =
          restjson::JsonBuildParse::get_array_length(jobj_drop);
      pfc_log_info("pop_array_len = %d and array_index=%d ",
                   drop_array_len, arr_idx);
      ret_val = restjson::JsonBuildParse::parse(jobj_actions, "drop",
                                                drop_array_len, jobj_drop);
      if (restjson::REST_OP_SUCCESS == ret_val && jobj_drop != NULL) {
        pfc_log_debug("drop success");
        status = 1;
      } else {
        pfc_log_debug("drop failure");
        status = 0;
    }*/
    } else {
      df_cmn->df_segment->vtn_df_common->valid[kidxDfDataFlowActionCount] = UNC_VF_INVALID;
}
}
  json_object *jobj_vnoderoute = NULL;
  ret_val = restjson::JsonBuildParse::parse(jobj_data, "vnoderoute",
                                            arr_idx, jobj_vnoderoute);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error(" Error occured while parsing ");
    return UNC_DRV_RC_ERR_GENERIC;
  }
uint32_t node_array_len = 0;
if (json_object_is_type(jobj_vnoderoute, json_type_array)) {
  node_array_len =
      restjson::JsonBuildParse::get_array_length(jobj_vnoderoute);
  pfc_log_info("--------------path-info arraylength --- %d", node_array_len);
  //struct path_info pathinfo_record[node_array_len];
  for (uint32_t node_arr_idx = 0; node_arr_idx < node_array_len;
       node_arr_idx++) {
    std::string in_reason = "";
    ret_val = restjson::JsonBuildParse::parse(jobj_vnoderoute, "reason",
                                              node_arr_idx, in_reason);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error(" Error occured while parsing ");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    std::string in_reason_info1 = "FORWARDED";
    std::string in_reason_info2 = "REDIRECTED";
    std::string in_reason_info3 = "PORTMAPPED";
    if(strcmp(in_reason.c_str(),in_reason_info1.c_str()) == 0) {
      pfc_log_debug("Pass Success");
      status = 0;
      vlink_flag = 1;
    } else if(strcmp(in_reason.c_str(),in_reason_info2.c_str()) == 0) {
      pfc_log_debug("Redirect Success");
      status = 0;
      vlink_flag = 0;
    } else if(strcmp(in_reason.c_str(),in_reason_info3.c_str()) == 0) {
      pfc_log_debug("portmapped");
    } else {
      pfc_log_debug("drop Success");
      status = 1;
      vlink_flag = 0;
    }

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
    df_cmn->df_segment->vtn_df_common->match_count = 1;
    pfc_log_debug("match count = %u",
                  df_cmn->df_segment->vtn_df_common->match_count);

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

  //match - vlanpri
  uint32_t vlan_pcp = 0;
  ret_val = restjson::JsonBuildParse::parse(jobj_ethernet, "vlanpri",
                                           -1, vlan_pcp);
  pfc_log_info("match-ethernetMatch-vlan_pcp -- %d", vlan_pcp);
  if(restjson::REST_OP_SUCCESS == ret_val) {
    pfc_log_info("match-ethernetMatch-vlan_pcp. success");
    val_df_flow_match_vlan_pcp_t* val_vlan_pcp_obj =
        new val_df_flow_match_vlan_pcp_t;
    memset(val_vlan_pcp_obj, 0, sizeof(val_df_flow_match_vlan_pcp_t));
    val_vlan_pcp_obj->vlan_pcp = vlan_pcp;
    df_cmn->df_segment->matches.insert(
        std::pair<UncDataflowFlowMatchType, void *>(UNC_MATCH_VLAN_PCP,
                                                    val_vlan_pcp_obj));
  } else {
     pfc_log_error(" Error occured while parsing match-ethernetMatch-vlan_pcp. continue");
  }

  if (match_status == PFC_TRUE) {
    pfc_log_error("match_status is true. match found");
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_MATCH_COUNT_VVDC] =
        UNC_VF_VALID;
  } else {
    pfc_log_error("match_status is false. match NOT found");
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_MATCH_COUNT_VVDC] =
        UNC_VF_INVALID;
  }

  // match - END


  /*struct path_info {
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
    */

   //val_vtn_dataflow_path_info
   val_vtn_dataflow_path_info_t *ptr = new val_vtn_dataflow_path_info;
   memset(ptr, 0, sizeof(val_vtn_dataflow_path_info));
   uint32_t in_flag = 0;
   uint32_t out_flag = 0;
   if(!in_vbridge.empty()) {
     memcpy(ptr->in_vnode, in_vbridge.c_str(), strlen(in_vbridge.c_str())+1);
     ptr->valid[UPLL_IDX_IN_VNODE_VVDPI] = UNC_VF_VALID;
     in_flag = 1;
     pfc_log_debug("in_vbridge: %s", ptr->in_vnode);
   } else {
     ptr->valid[UPLL_IDX_IN_VNODE_VVDPI] = UNC_VF_INVALID;
   }
    if(in_flag == 0) {
     if(!in_vterm.empty()) {
     memcpy(ptr->in_vnode, in_vterm.c_str(), strlen(in_vterm.c_str())+1);
     ptr->valid[UPLL_IDX_IN_VNODE_VVDPI] = UNC_VF_VALID;
     pfc_log_debug("in vterm: %s", ptr->in_vnode);
   } else {
     ptr->valid[UPLL_IDX_IN_VNODE_VVDPI] = UNC_VF_INVALID;
   }
  }
   if(!in_vinterface.empty()) {
     memcpy(ptr->in_vif, in_vinterface.c_str(), strlen(in_vinterface.c_str())+1);
     ptr->valid[UPLL_IDX_IN_VIF_VVDPI] = UNC_VF_VALID;
     pfc_log_debug("in_vinterface: %s", ptr->in_vif);
   } else {
     ptr->valid[UPLL_IDX_IN_VIF_VVDPI] = UNC_VF_INVALID;
   }

   if(!out_vbridge.empty()) {
     memcpy(ptr->out_vnode, out_vbridge.c_str(), strlen(out_vbridge.c_str())+1);
     ptr->valid[UPLL_IDX_OUT_VNODE_VVDPI] = UNC_VF_VALID;
     out_flag = 1;
     pfc_log_debug("out_vbridge: %s", ptr->out_vnode);
   } else {
     ptr->valid[UPLL_IDX_OUT_VNODE_VVDPI] = UNC_VF_INVALID;
   }
     if(out_flag == 0) {
     if(!out_vterm.empty()) {
     memcpy(ptr->out_vnode, out_vterm.c_str(), strlen(out_vterm.c_str())+1);
     ptr->valid[UPLL_IDX_OUT_VNODE_VVDPI] = UNC_VF_VALID;
     pfc_log_debug("out_vterm: %s", ptr->out_vnode);
   } else {
     ptr->valid[UPLL_IDX_OUT_VNODE_VVDPI] = UNC_VF_INVALID;
   }
   }

   if(!out_vinterface.empty()) {
     memcpy(ptr->out_vif, out_vinterface.c_str(), strlen(out_vinterface.c_str())+1);
     ptr->valid[UPLL_IDX_OUT_VIF_VVDPI] = UNC_VF_VALID;
     pfc_log_debug("out_vinterface: %s", ptr->out_vif);
   } else {
     ptr->valid[UPLL_IDX_OUT_VIF_VVDPI] = UNC_VF_INVALID;
   }
      ptr->vlink_flag = vlink_flag;
      ptr->valid[UPLL_IDX_VLINK_FLAG_VVDPI] = UNC_VF_VALID;

      ptr->status = status;
      ptr->valid[UPLL_IDX_STATUS_VVDPI] = UNC_VF_VALID;
   df_cmn->df_segment->vtn_path_infos.push_back(ptr);

   const char* ctr_name = df_util->get_ctr_id();
    pfc_log_info("controller name %s", ctr_name);
    /* copy controller name to dataflow common struct */
    memcpy(df_cmn->df_segment->vtn_df_common->controller_id,
           ctr_name,
           strlen(ctr_name)+1);
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_CONTROLLER_ID_VVDC] =
        UNC_VF_VALID;
    /* copy controller type to dataflow common struct */
    df_cmn->df_segment->vtn_df_common->controller_type = UNC_CT_ODC;
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_CONTROLLER_TYPE_VVDC] =
        UNC_VF_VALID;

    /* copy flow-id to dataflow common struct */
    df_cmn->df_segment->vtn_df_common->flow_id = flowid;
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_FLOW_ID_VVDC] = UNC_VF_VALID;
    pfc_log_info(" flow-id : %" PFC_PFMT_u64,
                 df_cmn->df_segment->vtn_df_common->flow_id);

    df_cmn->df_segment->vtn_df_common->created_time = creationTime;
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_CREATED_TIME_VVDC] = UNC_VF_VALID;
    pfc_log_trace(" created time : %" PFC_PFMT_u64, df_cmn->df_segment->vtn_df_common->created_time);

    df_cmn->df_segment->vtn_df_common->idle_timeout = idleTimeout;
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_IDLE_TIMEOUT_VVDC] = UNC_VF_VALID;
    pfc_log_trace(" idle time out : %u", df_cmn->df_segment->vtn_df_common->idle_timeout);

    df_cmn->df_segment->vtn_df_common->hard_timeout = hardTimeout;
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_HARD_TIMEOUT_VVDC] = UNC_VF_VALID;
    pfc_log_trace(" hard time out : %u", df_cmn->df_segment->vtn_df_common->hard_timeout);

    
    /* copy Status to dataflow common struct */
    //df_cmn->df_segment->vtn_df_common->valid[kidxDfDataFlowStatus] =
    //    UNC_VF_INVALID;
    /* copy flow_type to dataflow common struct */
    //df_cmn->df_segment->vtn_df_common->valid[kidxDfDataFlowFlowType] =
    //    UNC_VF_INVALID;
    /* copy PolicyIndex to dataflow common struct */
    //df_cmn->df_segment->vtn_df_common->valid[kidxDfDataFlowPolicyIndex] =
    //    UNC_VF_INVALID;
    /* copy VtnId to dataflow common struct */
    //df_cmn->df_segment->vtn_df_common->valid[kidxDfDataFlowVtnId] =
    //    UNC_VF_INVALID;
    /* copy Ingress SwitchId to dataflow common struct */
    
    strncpy(reinterpret_cast<char*> (
            df_cmn->df_segment->vtn_df_common->ingress_switch_id),
            switch_id.c_str(),
            strlen(switch_id.c_str()));
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_SWITCH_ID_VVDC] =
        UNC_VF_VALID;
    pfc_log_info("switch_id : %s",
                 df_cmn->df_segment->vtn_df_common->ingress_switch_id);

    /* copy Ingress Port to dataflow common struct */
    strncpy(reinterpret_cast<char*>(df_cmn->df_segment->vtn_df_common->ingress_port_id),
            in_portname.c_str(),
            strlen(in_portname.c_str()));
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_PORT_ID_VVDC] = UNC_VF_VALID;
    pfc_log_info("Ingress InPortName : %s",
                 df_cmn->df_segment->vtn_df_common->ingress_port_id);

    /* copy Ingress StationId to dataflow common struct */
    /*df_cmn->df_segment->vtn_df_common->in_station_id = atoi(station_id.c_str());
    df_cmn->df_segment->vtn_df_common->valid[kidxDfDataFlowInStationId] =
        UNC_VF_VALID;
    pfc_log_trace("station_id : %" PFC_PFMT_u64,
                  df_cmn->df_segment->vtn_df_common->in_station_id);
                  */
    
  //ingress_logical_port_id
  //Converts the logical port id from 11:11:22:22:33:33:44:44-name to
  //PP-1111-2222-3333-4444-name format
  memset(df_cmn->df_segment->vtn_df_common->ingress_logical_port_id,'\0',
                            sizeof(df_cmn->df_segment->vtn_df_common->ingress_logical_port_id));
  std::string in_logicalport_id = "PP-";
  std::string COLON = "-";
  uint32_t flag = 0;
  for(uint32_t position = 0; position < switch_id.length(); position ++){
    if (switch_id.at(position) == ':') {
      if((flag%2) == 0) {
        switch_id.erase(position, 1);
        position--;
      } else {
        switch_id.erase(position, 1);
        switch_id.insert(position, COLON);
      }
    }
     flag++;
  }
  in_logicalport_id.append(switch_id);
  in_logicalport_id.append(HYPHEN);
  in_logicalport_id.append(in_portname);
  df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_LOGICAL_PORT_ID_VVDC] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char*>(df_cmn->df_segment->vtn_df_common->ingress_logical_port_id),
          in_logicalport_id.c_str(),
          strlen(in_logicalport_id.c_str()));
  pfc_log_trace("ingress_logical_port_id %s", df_cmn->df_segment->vtn_df_common->ingress_logical_port_id);


    /* copy In-Domain DEFAULT to dataflow common struct */
    strncpy(reinterpret_cast<char*> (df_cmn->df_segment->vtn_df_common->ingress_domain),
            DOM_NAME.c_str(), strlen(DOM_NAME.c_str()));
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_DOMAIN_VVDC] =
        UNC_VF_VALID;
    pfc_log_trace("ingress_domain %s",
                  df_cmn->df_segment->vtn_df_common->ingress_domain);

    /* copy Egress SwitchId to dataflow common struct */
    strncpy(reinterpret_cast<char*> (
            df_cmn->df_segment->vtn_df_common->egress_switch_id),
        egress_switch_id.c_str(),
            strlen(egress_switch_id.c_str()));
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_SWITCH_ID_VVDC] =
        UNC_VF_VALID;
    pfc_log_trace("Egress SwitchId : %s",
                  df_cmn->df_segment->vtn_df_common->egress_switch_id);

    /* copy Egress OutPort to dataflow common struct */
    strncpy(reinterpret_cast<char*> (
            df_cmn->df_segment->vtn_df_common->egress_port_id), out_portname.c_str(),
            strlen(out_portname.c_str()));
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_PORT_ID_VVDC] = UNC_VF_VALID;
    pfc_log_info("Egress Out-port_name %s",
                 df_cmn->df_segment->vtn_df_common->egress_port_id);

    /* copy Egress OutStationId to dataflow common struct */
    /*df_cmn->df_segment->vtn_df_common->out_station_id =
        atoi(out_station_id.c_str());
    df_cmn->df_segment->vtn_df_common->valid[kidxDfDataFlowOutStationId] =
        UNC_VF_VALID;
    pfc_log_trace("Out-station_id : %" PFC_PFMT_u64,
                  df_cmn->df_segment->vtn_df_common->out_station_id);
                  */
    
    //egress_logical_port_id
  memset(df_cmn->df_segment->vtn_df_common->egress_logical_port_id,'\0',
                            sizeof(df_cmn->df_segment->vtn_df_common->egress_logical_port_id));
  std::string out_logicalport_id = "PP-";
  flag = 0;
  for(uint32_t position = 0; position < egress_switch_id.length(); position ++){
    if (egress_switch_id.at(position) == ':') {
      if((flag%2) == 0) {
        egress_switch_id.erase(position, 1);
        position--;
      } else {
        egress_switch_id.erase(position, 1);
        egress_switch_id.insert(position, COLON);
      }
    }
    flag++;
  }
  out_logicalport_id.append(egress_switch_id);
  out_logicalport_id.append(HYPHEN);
  out_logicalport_id.append(out_portname);
  df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_LOGICAL_PORT_ID_VVDC] = UNC_VF_VALID;
  strncpy(reinterpret_cast<char*>(df_cmn->df_segment->vtn_df_common->egress_logical_port_id),
          out_logicalport_id.c_str(),
          strlen(out_logicalport_id.c_str()));
  pfc_log_trace("egress_logical_port_id %s", df_cmn->df_segment->vtn_df_common->egress_logical_port_id);


    /* copy Out-Domain DEFAULT to dataflow common struct */
    strncpy(reinterpret_cast<char*> (
            df_cmn->df_segment->vtn_df_common->egress_domain),
            DOM_NAME.c_str(), strlen(DOM_NAME.c_str()));
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_DOMAIN_VVDC] =
        UNC_VF_VALID;
    pfc_log_trace("egress_Out-domain %s",
                  df_cmn->df_segment->vtn_df_common->egress_domain);

    df_cmn->df_segment->vtn_df_common->path_info_count = 1;
    df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_PATH_INFO_COUNT_VVDC] = UNC_VF_VALID;
    pfc_log_trace("path_count %u", df_cmn->df_segment->vtn_df_common->path_info_count);

    /* copy Path-info to dataflow common struct */
    /*
    if (node_array_len > 0) {
      df_cmn->df_segment->vtn_df_common->path_info_count = node_array_len;
      df_cmn->df_segment->vtn_df_common->valid[20] =
          UNC_VF_VALID;
      for (uint32_t count = 0; count < node_array_len; count++) {
        val_df_data_flow_path_info *ptr = new val_df_data_flow_path_info;
        memset(ptr, 0, sizeof(val_df_data_flow_path_info_t));*/
        /* copy Path-info SwitchId to dataflow common struct */
      /*  strncpy(reinterpret_cast<char*> (ptr->switch_id),
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
      df_cmn->df_segment->vtn_df_common->valid[kidxDfDataFlowPathInfoCount] =
          UNC_VF_INVALID;
      pfc_log_trace("There is no Physical path related Information");
    }

  } else {
    pfc_log_info("no element for noderoute array");
  }*/
  unc::vtnreadutil::driver_dataflow_read_util::add_read_value
      (df_cmn, df_util);

  // df_util.appendFlow(df_cmn);
  return UNC_RC_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
