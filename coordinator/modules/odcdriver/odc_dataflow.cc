/*
 * Copyright (c) 2014-2016 NEC Corporation
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

    dataflow_class *req_obj = new dataflow_class(ctr,*it_vtn_name);
    std::string url = req_obj->get_url();
    if (url == "") {
      pfc_log_error("%s:Invalid Key", PFC_FUNCNAME);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    dataflow_parser *parser_obj = new dataflow_parser();
    ip_dataflow in_obj;
    in_obj.valid = true;
    in_obj.input_data_flow_.valid = true;
    in_obj.input_data_flow_.ip_node =
                        reinterpret_cast<char*>(key_dataflow.switch_id);
    in_obj.input_data_flow_.data_flow_port_.valid = true;
    in_obj.input_data_flow_.data_flow_port_.ip_port_name =
                           reinterpret_cast<char*>(key_dataflow.port_id);
    in_obj.input_data_flow_.data_flow_source_.valid = true;
    in_obj.input_data_flow_.data_flow_source_.ip_vlan = key_dataflow.vlan_id;
    uint8_t mac_arr[VAL_MAC_ADDR_SIZE];
    memset(&mac_arr, 0, VAL_MAC_ADDR_SIZE);
    string srcMac = "";
    if (memcmp(key_dataflow.src_mac_address, mac_arr, VAL_MAC_ADDR_SIZE) > 0) {
      pfc_log_debug("valid src_mac ");
      pfc_log_debug("key mac_address %02x:%02x:%02x:%02x:%02x:%02x ",
                  key_dataflow.src_mac_address[0], key_dataflow.src_mac_address[1],
                  key_dataflow.src_mac_address[2], key_dataflow.src_mac_address[3],
                  key_dataflow.src_mac_address[4], key_dataflow.src_mac_address[5]);

    // convert key.src_mac_address to string
    char mac[32] = { 0 };
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
            key_dataflow.src_mac_address[0],
            key_dataflow.src_mac_address[1],
            key_dataflow.src_mac_address[2],
            key_dataflow.src_mac_address[3],
            key_dataflow.src_mac_address[4],
            key_dataflow.src_mac_address[5]);
    srcMac = mac;
    }
    in_obj.input_data_flow_.data_flow_source_.ip_mac_addr = srcMac;
    pfc_log_debug("SRCMAC,VLAN %d",key_dataflow.vlan_id);
    in_obj.input_data_flow_.ip_tenant_name = *it_vtn_name;
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
    ret_val = parse_flow_response_values(key_dataflow, df_util,
                                         parser_obj->data_flow_);

    if (ret_val == UNC_RC_NO_SUCH_INSTANCE) {
       check_count = 1;
       pfc_log_trace("No such Inst. count : %d", check_count);
       continue;

    } else if (ret_val != UNC_RC_SUCCESS) {
          pfc_log_error("Error occured while parsing");
          vtn_names.clear();
          return ret_val;
      }
      check_count = 0;
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

UncRespCode OdcDataFlowCommand::parse_flow_response_values(
    key_dataflow_t& key_dataflow,
    unc::vtnreadutil::driver_read_util* df_util,
    std::list<data_flow> &df_info) {
  ODC_FUNC_TRACE;

  // Populate values to dataflow
  unc::dataflow::DataflowDetail *df_segm =
      new unc::dataflow::DataflowDetail(
          unc::dataflow::kidx_val_df_data_flow_cmn);
  unc::dataflow::DataflowCmn *df_cmn =
      new unc::dataflow::DataflowCmn(false, df_segm);

  std::list<data_flow>::iterator it;
  for (it = df_info.begin(); it != df_info.end(); it++) {
    uint32_t flowid = 0;
    uint32_t creationTime = 0;
    uint32_t hardTimeout = 0;
    uint32_t idleTimeout = 0;

  //flow_id
    flowid = it->flow_id;
    creationTime = it->creation_time;
    hardTimeout = it->hard_timeout;
    idleTimeout = it->idle_timeout;
    pfc_log_debug("Creation time %d,%d,%d", creationTime, hardTimeout,
                                                       idleTimeout);

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
   df_cmn->df_segment->df_common->valid[kidxDfDataFlowActionCount] =
          UNC_VF_INVALID;

  // match - START
  pfc_bool_t match_status = PFC_FALSE;
  if (it->data_flow_match_.valid == true) {

  // check match-ethernet
    df_cmn->df_segment->df_common->match_count = 1;
    pfc_log_debug("match count = %u",
                  df_cmn->df_segment->df_common->match_count);
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
      pfc_log_info("dst -- %s", dst.c_str());
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
  std::list<physical_route>::iterator iter;
    uint32_t node_array_len = it->physical_route_.size();
    pfc_log_info("--------------path-info arraylength --- %d", node_array_len);
    struct path_info pathinfo_record[node_array_len];
    uint32_t order = 0;
    for (iter = it->physical_route_.begin(),order = 0; iter !=
                     it->physical_route_.end(),order < node_array_len;
                                                       iter++,order++) {
      std::string pathinfo_switch_id = "";
      if(!iter->node.empty()) {
        pathinfo_switch_id = iter->node;
      pfc_log_info("pathinfo_switch_id -- %s", pathinfo_switch_id.c_str());
      pathinfo_record[order].switchid = pathinfo_switch_id;
      } else {
          pfc_log_error("invalid pathinfo_switch_id ");
          return UNC_DRV_RC_ERR_GENERIC;
      }

      std::string inport_name = "";
      if (!iter->phy_ingr_port_.port_name_ing.empty()) {
        inport_name = iter->phy_ingr_port_.port_name_ing;
        pfc_log_info("pathinfo_inport_name -- %s", inport_name.c_str());
        pathinfo_record[order].inport =  inport_name;
      } else {
          pfc_log_error("invalid pathinfo_inport_name ");
          return UNC_DRV_RC_ERR_GENERIC;
      }

      std::string outport_name = "";
      if (!iter->phy_egr_port_.port_name_eg.empty()) {
        outport_name = iter->phy_egr_port_.port_name_eg;
        pfc_log_info("pathinfo_outport_name -- %s", outport_name.c_str());
        pathinfo_record[order].outport = outport_name;
      } else {
          pfc_log_error("invalid pathinfo_inport_name ");
          return UNC_DRV_RC_ERR_GENERIC;
      }
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

  // else {
   // pfc_log_info("no element for noderoute array");
  //}
  unc::vtnreadutil::driver_dataflow_read_util::add_read_value
      (df_cmn, df_util);
  }

  // df_util.appendFlow(df_cmn);
  return UNC_RC_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
