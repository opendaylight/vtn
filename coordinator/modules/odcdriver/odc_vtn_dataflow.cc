/*
 * Copyright (c) 2015-2016 NEC Corporation
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

    pfc_log_trace("Vtn Name in dataflow  %s",
                  key_vtn_dataflow.vtn_key.vtn_name);
    std::string vtn_name = reinterpret_cast<char*> (key_vtn_dataflow.vtn_key.vtn_name);

    dataflow_class *req_obj = new dataflow_class(ctr,vtn_name);
    std::string url = req_obj->get_url();
    if (url == "") {
      pfc_log_error("%s:Invalid Key", PFC_FUNCNAME);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    dataflow_parser *parser_obj = new dataflow_parser();
    ip_dataflow in_obj;
    in_obj.valid = true;
    in_obj.input_data_flow_.valid = true;
    in_obj.input_data_flow_.data_flow_source_.valid = true;
    unsigned int vlan_id = 0;
    if (key_vtn_dataflow.vlanid) {
       if (key_vtn_dataflow.vlanid != 0xffff) {
       in_obj.input_data_flow_.data_flow_source_.ip_vlan = key_vtn_dataflow.vlanid;
      pfc_log_info("vlan-id : %u", vlan_id);
     } else {
       in_obj.input_data_flow_.data_flow_source_.ip_vlan = 0;
      pfc_log_info("vlan-id : not set.");
    }
    }
    in_obj.input_data_flow_.ip_tenant_name =
                    reinterpret_cast<char*> (key_vtn_dataflow.vtn_key.vtn_name);
   uint8_t mac_arr[VAL_MAC_ADDR_SIZE];
    memset(&mac_arr, 0, VAL_MAC_ADDR_SIZE);
    string srcMac = "";
    if (memcmp(key_vtn_dataflow.src_mac_address, mac_arr, VAL_MAC_ADDR_SIZE) >
                                                                       0) {
      pfc_log_debug("valid src_mac ");
      pfc_log_debug("key mac_address %02x:%02x:%02x:%02x:%02x:%02x ",
                  key_vtn_dataflow.src_mac_address[0],
                  key_vtn_dataflow.src_mac_address[1],
                  key_vtn_dataflow.src_mac_address[2],
                   key_vtn_dataflow.src_mac_address[3],
                  key_vtn_dataflow.src_mac_address[4],
                   key_vtn_dataflow.src_mac_address[5]);

    // convert key.src_mac_address to string
    char mac[32] = { 0 };
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
            key_vtn_dataflow.src_mac_address[0],
            key_vtn_dataflow.src_mac_address[1],
            key_vtn_dataflow.src_mac_address[2],
            key_vtn_dataflow.src_mac_address[3],
            key_vtn_dataflow.src_mac_address[4],
            key_vtn_dataflow.src_mac_address[5]);
    srcMac = mac;
    }
    in_obj.input_data_flow_.data_flow_source_.ip_mac_addr = srcMac;

    in_obj.input_data_flow_.mode = "DETAIL";

    parser_obj->get_req(in_obj);
    UncRespCode ret_val = req_obj->get_response(parser_obj);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Get response error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    ret_val = parser_obj->set_data_flow(parser_obj->jobj);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("set_dataflow_error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    ret_val = parse_flow_response_values(key_vtn_dataflow, df_util,
                                         parser_obj->data_flow_);
        if (ret_val != UNC_RC_SUCCESS) {
          pfc_log_error("Error occured while parsing");
          return ret_val;
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
    vtn_name.push_back(reinterpret_cast<char*>
                       (cfgnode_ctr->get_key_structure()->vtn_name));
  }
  // parse .. vtn name add

  return UNC_RC_SUCCESS;
}

UncRespCode OdcVtnDataFlowCommand::parse_flow_response_values(
    key_vtn_dataflow_t& key_vtn_dataflow,
    unc::vtnreadutil::driver_read_util* df_util,
    std::list<data_flow> &df_info) {
  ODC_FUNC_TRACE;

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

  std::list<data_flow>::iterator it;
  for (it = df_info.begin(); it != df_info.end(); it++) {
    uint32_t flowid = 0;
    uint32_t creationTime = 0;
    uint32_t hardTimeout = 0;
    uint32_t idleTimeout = 0;
    uint32_t status = 0;
    uint32_t vlink_flag = 0;

  //flow_id
    flowid = it->flow_id;
    creationTime = it->creation_time;
    hardTimeout = it->hard_timeout;
    idleTimeout = it->idle_timeout;

  //ingress_vbridge

  std::string in_vbridge = "";
  if (it->data_ingress_node_.valid == true){
    if (!it->data_ingress_node_.ig_bridge.empty()){
       in_vbridge = it->data_ingress_node_.ig_bridge;
  } else {
       pfc_log_error("vbridge is empty");
       //return UNC_DRV_RC_ERR_GENERIC;
  }
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
  if (it->data_ingress_node_.valid == true){
    if (!it->data_ingress_node_.ig_term_name.empty()){
       in_vterm = it->data_ingress_node_.ig_term_name;
   } else {
       pfc_log_error("vterminal is empty");
       //return UNC_DRV_RC_ERR_GENERIC;
     }
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
  if (!it->data_ingress_node_.ig_if_name.empty()){
    in_vinterface = it->data_ingress_node_.ig_if_name;
  } else {
      pfc_log_error("vinterface is empty");
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

  std::string out_vbridge = "";
  std::string out_vinterface = "";
  std::string out_vterm = "";
  if (it->data_egress_node_.valid == true){
    if (!it->data_egress_node_.eg_bridge.empty()){
       out_vbridge = it->data_egress_node_.eg_bridge;
   } else {
       pfc_log_error("vbridge is empty");
     }
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


  if (it->data_egress_node_.valid == true){
    if (!it->data_egress_node_.eg_terminal.empty()){
      out_vterm = it->data_egress_node_.eg_terminal;
    } else {
       pfc_log_error("vterminal is empty");
  }
  }

  if (out_vterm.empty()) {
    pfc_log_error("Error occured while parsing egress_terminal");
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
  if (it->data_egress_node_.valid == true){
    if (!it->data_egress_node_.eg_if.empty()){
      out_vinterface = it->data_egress_node_.eg_if;
  } else {
       pfc_log_error("vinterface is empty");
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

 //ingressport

   std::string switch_id = "";
   std::string station_id = "";
   std::string in_portname = "";
   if (it->data_ingress_port_.valid == true) {
   if(!it->data_ingress_port_.ig_node.empty()) {
     switch_id = it->data_ingress_port_.ig_node;
   } else {
      pfc_log_error("switch-id is null");
      return UNC_DRV_RC_ERR_GENERIC;
   }
   if(!it->data_ingress_port_.ig_port_name.empty())
     in_portname = it->data_ingress_port_.ig_port_name;
   else {
      pfc_log_error("port-name is empty");
      return UNC_DRV_RC_ERR_GENERIC;
   }
   if(!it->data_ingress_port_.ig_port_id.empty())
     station_id = it->data_ingress_port_.ig_port_id;
   else {
      pfc_log_error("port-id is empty");
      return UNC_DRV_RC_ERR_GENERIC;
   }
   }

  // egressPort
   std::string egress_switch_id = "";
   std::string out_station_id = "";
   std::string out_portname = "";
   if (it->data_egress_port_.valid == true) {
   if(!it->data_egress_port_.eg_node.empty())
      egress_switch_id = it->data_egress_port_.eg_node;
   else {
      pfc_log_error("node-id is empty");
      return UNC_DRV_RC_ERR_GENERIC;
   }
   if(!it->data_egress_port_.eg_port_id.empty())
     out_station_id = it->data_egress_port_.eg_port_id;
   else {
      pfc_log_error("port-id is empty");
      return UNC_DRV_RC_ERR_GENERIC;
   }
   if(!it->data_egress_port_.eg_port_name.empty())
     out_portname = it->data_egress_port_.eg_port_name;
   else {
      pfc_log_error("port-id is empty");
      return UNC_DRV_RC_ERR_GENERIC;
   }
   }

  // action
    df_cmn->df_segment->vtn_df_common->valid[kidxDfDataFlowActionCount] = UNC_VF_INVALID;
 //vnoderoute
   std::list<virtual_route>::iterator iter;
   uint32_t node_array_len = it->virtual_route_.size();

   pfc_log_info("--------------path-info arraylength --- %d", node_array_len);
   //struct path_info pathinfo_record[node_array_len];
   uint32_t order = 0;
   for (iter = it->virtual_route_.begin(),order = 0; iter !=
               it->virtual_route_.end(),order < node_array_len;iter++,order++) {
     std::string in_reason = "";
     if (!iter->reason.empty()){
       in_reason = iter->reason;
     } else {
        pfc_log_error("reason is empty");
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

  // match - START
  pfc_bool_t match_status = PFC_FALSE;
  if(it->data_flow_match_.valid == true){

  // check match-ethernet
    df_cmn->df_segment->vtn_df_common->match_count = 1;
    pfc_log_debug("match count = %u",
                  df_cmn->df_segment->vtn_df_common->match_count);

    // match-ethernet-vlan
    uint32_t vlan = 0;
    if (it->data_flow_match_.da_vtn_ether_match_.vlan != -1) {
      vlan = it->data_flow_match_.da_vtn_ether_match_.vlan;
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
    if(!it->data_flow_match_.da_vtn_ether_match_.src_arr.empty()) {
      src = it->data_flow_match_.da_vtn_ether_match_.src_arr;
      pfc_log_info("src df -- %s", src.c_str());
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
    if(!it->data_flow_match_.da_vtn_ether_match_.dst_arr.empty()) {
      dst = it->data_flow_match_.da_vtn_ether_match_.dst_arr;
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
  }
  // df_util.appendFlow(df_cmn);
  return UNC_RC_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
