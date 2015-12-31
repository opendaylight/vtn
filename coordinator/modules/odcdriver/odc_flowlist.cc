/*
 * Copyright (c) 2014-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_flowlist.hh>

namespace unc {
namespace odcdriver {
void
OdcFlowListCommand::copy(ip_flowlist&  ip_flowlist_st,
                         key_flowlist& key,
                         val_flowlist& val) {
  ODC_FUNC_TRACE;

  pfc_log_error("Entering FLowlsit copy command");
  ip_flowlist_st.input_flowlist_.valid = true;
  ip_flowlist_st.input_flowlist_.ip_name =
               reinterpret_cast <char *>(key.flowlist_name);
  ip_flowlist_st.input_flowlist_.operation =
                       reinterpret_cast<const char*>("SET");
  ip_flowlist_st.input_flowlist_.present = false;
  pfc_log_error("Leaving FLowlsit copy command");

}

UncRespCode
OdcFlowListCommand::create_cmd(key_flowlist &key_in, val_flowlist &val_in,
                               unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr_ptr != NULL);
    flowlist_class *req_obj = new flowlist_class(ctr_ptr);
    ip_flowlist st_obj;
    copy(st_obj, key_in, val_in);
    flowlist_parser *parser_obj = new flowlist_parser();
    json_object *jobj = parser_obj->create_req(st_obj);
    if (jobj == NULL) {
      pfc_log_error("Error in create request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    if(req_obj->set_post(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("Flowlist Create Failed");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
   delete req_obj;
   delete parser_obj;
   return UNC_RC_SUCCESS;
}

UncRespCode
OdcFlowListCommand::delete_cmd(key_flowlist &key_in, val_flowlist &val_in,
                               unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr_ptr != NULL);
    flowlist_class *req_obj = new flowlist_class(ctr_ptr);
    ip_flowlist st_obj;
    delete_request_body(key_in,val_in,st_obj);
    flowlist_parser *parser_obj = new flowlist_parser();
    json_object *jobj = parser_obj->del_req(st_obj);
     if (jobj == NULL) {
      pfc_log_error("Error in delete request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }

    if(req_obj->set_delete(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("Flowlist Delete Failed");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
}

void
OdcFlowListCommand::create_request_body(key_flowlist &key_in,
                                        val_flowlist &val_in,
                                         flowlist& flowlist_st) {
ODC_FUNC_TRACE;
char *flowlist_name = reinterpret_cast <char *>(key_in.flowlist_name);
if (0 == strlen(flowlist_name)) {
  pfc_log_error("Empty flowlist_name in %s", PFC_FUNCNAME);
}
flowlist_st.name.assign(flowlist_name);
}

void
OdcFlowListCommand::delete_request_body(key_flowlist &key_in,
                                        val_flowlist &val_in,
                                         ip_flowlist&  ip_flowlist_st) {
  ODC_FUNC_TRACE;

  ip_flowlist_st.input_flowlist_.valid = true;
  ip_flowlist_st.input_flowlist_.ip_name =
                        reinterpret_cast <char *>(key_in.flowlist_name);


}

UncRespCode OdcFlowListCommand::fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

    flowlist_class *req_obj = new flowlist_class(ctr_ptr);
    std::string url = req_obj->get_url();
    pfc_log_info("URL:%s",url.c_str());
    flowlist_parser *parser_obj = new flowlist_parser();
    UncRespCode ret_val = req_obj->get_response(parser_obj);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Get response error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    ret_val = parser_obj->set_flowlist(parser_obj->jobj);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("set_flowlist_conf error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    ret_val = r_copy(parser_obj->flowlist_, cfgnode_vector);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Error occured while parsing");
      delete req_obj;
      delete parser_obj;
      return ret_val;
  }

  return ret_val;

}

UncRespCode
OdcFlowListCommand::r_copy(std::list<flowlist> &flowlist_detail,
                           std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;

  unc::odcdriver::OdcUtil util;

  std::list<flowlist>::iterator list_iter = flowlist_detail.begin();

  while ( list_iter != flowlist_detail.end() ) {
    PFC_ASSERT(*list_iter != NULL);
    key_flowlist key_fl;
    val_flowlist val_fl;

    memset(&key_fl,0,sizeof(key_flowlist));
    memset(&val_fl,0,sizeof(val_flowlist));

    // Copy Condition NAme to key!!
    strcpy(reinterpret_cast<char*>(key_fl.flowlist_name),(list_iter)->name.c_str());

    // Create Val Structure
    val_fl.ip_type=UPLL_FLOWLIST_TYPE_IP;
    val_fl.valid[UPLL_IDX_IP_TYPE_FL] = UNC_VF_VALID;

    //Addto Cache
    unc::vtndrvcache::ConfigNode *cfgptr(
      new unc::vtndrvcache::CacheElementUtil<key_flowlist, val_flowlist, val_flowlist, uint32_t>
      (&key_fl,&val_fl, &val_fl, uint32_t(UNC_OP_READ)));

    PFC_ASSERT(cfgptr != NULL );
    cfgnode_vector.push_back(cfgptr);
    //Check if Match list has entried to create flowlist entry structures

    std::list<vtn_flow_match>::iterator match_iter =
                                 (list_iter)->vtn_flow_match_.begin();

    while ( match_iter != (list_iter)->vtn_flow_match_.end () ) {
      PFC_ASSERT(*match_iter != NULL);

      // Create flowlist entry key
      key_flowlist_entry key_fl_entry;
      val_flowlist_entry val_fl_entry;

      memset(&key_fl_entry,0,sizeof(key_flowlist_entry));
      memset(&val_fl_entry,0,sizeof(val_flowlist_entry));

      memcpy(&key_fl_entry.flowlist_key, &key_fl,sizeof(key_flowlist));
      key_fl_entry.sequence_num=(match_iter)->flow_index;
        if ( match_iter->vtn_ether_match_.source_address != "" ) {
          util.convert_macstring_to_uint8(match_iter->vtn_ether_match_.source_address,
                                          &val_fl_entry.mac_src[0]);
          val_fl_entry.valid[UPLL_IDX_MAC_SRC_FLE] = UNC_VF_VALID;
        }
        if ( match_iter->vtn_ether_match_.destination_address != "" ) {
          util.convert_macstring_to_uint8(match_iter->vtn_ether_match_.destination_address,
                                          &val_fl_entry.mac_dst[0]);
          val_fl_entry.valid[UPLL_IDX_MAC_DST_FLE] = UNC_VF_VALID;
        }
        if ( match_iter->vtn_ether_match_.ether_type != -1 ) {
          val_fl_entry.mac_eth_type=match_iter->vtn_ether_match_.ether_type;
          val_fl_entry.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] = UNC_VF_VALID;
        }
        if (  match_iter->vtn_ether_match_.vlanpri != -1 ) {
          val_fl_entry.vlan_priority=match_iter->vtn_ether_match_.vlanpri;
          val_fl_entry.valid[UPLL_IDX_VLAN_PRIORITY_FLE] = UNC_VF_VALID;
        }

          if ( match_iter->vtn_inet_match_.source_network != "" ) {
            util.convert_ip_to_inaddr(match_iter->vtn_inet_match_.source_network,
                                      &val_fl_entry.src_ip);
            val_fl_entry.valid[UPLL_IDX_SRC_IP_FLE] = UNC_VF_VALID;
          }
          if ( match_iter->vtn_inet_match_.destination_network != "" ) {
            util.convert_ip_to_inaddr(match_iter->vtn_inet_match_.destination_network,
                                      &val_fl_entry.dst_ip);
            val_fl_entry.valid[UPLL_IDX_DST_IP_FLE] = UNC_VF_VALID;
          }
          if ( match_iter->vtn_inet_match_.protocol != -1 ) {
            val_fl_entry.ip_proto= match_iter->vtn_inet_match_.protocol;
            val_fl_entry.valid[UPLL_IDX_IP_PROTOCOL_FLE]=UNC_VF_VALID;
          }
          if ( match_iter->vtn_inet_match_.dscp != -1 ) {
            val_fl_entry.ip_dscp=match_iter->vtn_inet_match_.dscp;
            val_fl_entry.valid[UPLL_IDX_IP_DSCP_FLE]=UNC_VF_VALID;
          }

          if ( match_iter->icmp_type != -1 ) {
            val_fl_entry.icmp_type = match_iter->icmp_type;
            val_fl_entry.valid[UPLL_IDX_ICMP_TYPE_FLE]=UNC_VF_VALID;
          }
          if ( match_iter->icmp_code != -1 ) {
            val_fl_entry.icmp_type = match_iter->icmp_code;
            val_fl_entry.valid[UPLL_IDX_ICMP_CODE_FLE]=UNC_VF_VALID;
          }
            if ( match_iter->tcp_source_range_.src_port_from != -1 ) {
              val_fl_entry.l4_src_port= match_iter->tcp_source_range_.src_port_from;
              val_fl_entry.valid[UPLL_IDX_L4_SRC_PORT_FLE]=UNC_VF_VALID;
            }
            if ( match_iter->tcp_source_range_.src_port_to != -1 ) {
              val_fl_entry.l4_src_port_endpt= match_iter->tcp_source_range_.src_port_from;
              val_fl_entry.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE]=UNC_VF_VALID;
            }
            if ( match_iter->tcp_destination_range_.dst_port_from != -1 ) {
              val_fl_entry.l4_dst_port= match_iter->tcp_destination_range_.dst_port_from;
              val_fl_entry.valid[UPLL_IDX_L4_DST_PORT_FLE]=UNC_VF_VALID;
            }
            if ( match_iter->tcp_destination_range_.dst_port_to!= -1 ) {
              val_fl_entry.l4_dst_port_endpt= match_iter->tcp_destination_range_.dst_port_to;
              val_fl_entry.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]=UNC_VF_VALID;
            }

      // Add to Cache
      unc::vtndrvcache::ConfigNode *entry_cfgptr(
        new unc::vtndrvcache::CacheElementUtil<key_flowlist_entry, val_flowlist_entry, val_flowlist_entry, uint32_t>
        (&key_fl_entry,&val_fl_entry, &val_fl_entry, uint32_t(UNC_OP_READ)));

      PFC_ASSERT(entry_cfgptr != NULL );
      cfgnode_vector.push_back(entry_cfgptr);
      match_iter++; // Iterate to next match entry
    } // Match Iteration
    list_iter++; // Iterate to next flow condition entry
  } // Flow Conditions Iteration
  return UNC_RC_SUCCESS;
}


UncRespCode OdcFlowListEntryCommand::create_cmd(key_flowlist_entry &key_in,
                                                val_flowlist_entry &val_in,
                                                unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr_ptr != NULL);
    char *cond_name = reinterpret_cast <char *>(key_in.flowlist_key.flowlist_name);
    flowlistentry_class *req_obj = new flowlistentry_class(ctr_ptr,cond_name);
    ip_flowlistentry st_obj;
    copy(st_obj, key_in, val_in);
    flowlistentry_parser *parser_obj = new flowlistentry_parser();
    json_object *jobj = parser_obj->create_req(st_obj);
    if(jobj == NULL) {
      pfc_log_error("Error in create request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    if(req_obj->set_post(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("Flowlistentry Create Failed");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
   delete req_obj;
   delete parser_obj;
   return UNC_RC_SUCCESS;
}

UncRespCode OdcFlowListEntryCommand::update_cmd(key_flowlist_entry &key_in,
                                            val_flowlist_entry &val_old_in,
                                            val_flowlist_entry &val_new_in,
                                            unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr_ptr != NULL);
    char *cond_name = reinterpret_cast <char *>(key_in.flowlist_key.flowlist_name);
    flowlistentry_class *req_obj = new flowlistentry_class(ctr_ptr,cond_name);
    ip_flowlistentry st_obj;
    copy(st_obj, key_in, val_old_in, val_new_in);
    flowlistentry_parser *parser_obj = new flowlistentry_parser();
    json_object *jobj = parser_obj->create_req(st_obj);
    if(jobj == NULL) {
      pfc_log_error("Error in create request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    if(req_obj->set_put(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("Flowlistentry update Failed");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
   delete req_obj;
   delete parser_obj;
   return UNC_RC_SUCCESS;
}

UncRespCode
OdcFlowListEntryCommand::delete_cmd(key_flowlist_entry &key_in,
                                    val_flowlist_entry &val_in,
                                    unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr_ptr != NULL);
    char *cond_name = reinterpret_cast <char *>(key_in.flowlist_key.flowlist_name);
    char sequence_no[10];
    sprintf(sequence_no,"%d",key_in.sequence_num);
    flowlistentry_class *req_obj = new flowlistentry_class(ctr_ptr,cond_name);
    ip_flowlistentry st_obj;
    delete_request_body(key_in,val_in,st_obj);
    flowlistentry_parser *parser_obj = new flowlistentry_parser();
    json_object *jobj = parser_obj->del_req(st_obj);
    if(jobj == NULL) {
      pfc_log_error("Error in delete request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }

    if(req_obj->set_delete(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("Flowlistentry Create Failed");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
   delete req_obj;
   delete parser_obj;
   return UNC_RC_SUCCESS;
}

void
OdcFlowListEntryCommand::delete_request_body(key_flowlist_entry &key_in,
                                    val_flowlist_entry &val_in,
                            ip_flowlistentry&  ip_flowlistentry_st){

   ip_flowlistentry_st.in_flowcond_.valid = true;
   ip_flowlistentry_st.in_flowcond_.name = reinterpret_cast <char *>(
                                      key_in.flowlist_key.flowlist_name);
}


void
OdcFlowListEntryCommand::copy(ip_flowlistentry&  ip_flowlistentry_st,
                              key_flowlist_entry& key,
                              val_flowlist_entry& val) {
  ODC_FUNC_TRACE;
  unc::odcdriver::OdcUtil util;


  ip_flowlistentry_st.in_flowcond_.valid = true;
  ip_flowlistentry_st.in_flowcond_.name = reinterpret_cast <char *>(
                                         key.flowlist_key.flowlist_name);
  match match_;
  match_.index = key.sequence_num;
  // Ethernet Match Values
  if ( val.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID ) {

    pfc_log_info("Filling Ethernet Match Details");
    match_.ether_match_.valid = true;

    if ( val.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID )
      match_.ether_match_.dest_addr =
                                           util.macaddress_to_string(&val.mac_dst[0]);

    if ( val.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID )
      match_.ether_match_.src_addr =
                                         util.macaddress_to_string(&val.mac_src[0]);

    if ( val.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID )
      match_.ether_match_.ether_type =
                                                                val.mac_eth_type;

    if ( val.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID )
      match_.ether_match_.vlanpri = val.vlan_priority;

  }

  if ( val.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_DST_IP_PREFIX_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_SRC_IP_PREFIX_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID ) {

    pfc_log_info("Filling Inet Match Details");
    match_.inet_match_.valid = true;

    if( val.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID && (val.valid[UPLL_IDX_DST_IP_PREFIX_FLE] == UNC_VF_VALID)){
      std::stringstream dst_ip;
      dst_ip << inet_ntoa(val.dst_ip);
      dst_ip << "/";
      int ip = val.dst_ip_prefixlen;
      dst_ip << ip;
      match_.inet_match_.dest_network = dst_ip.str();

    }
    if( (val.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID)  && (val.valid[UPLL_IDX_SRC_IP_PREFIX_FLE] == UNC_VF_VALID)) {
      std::stringstream src_ip;
      src_ip << inet_ntoa(val.src_ip);
      src_ip << "/";
      int ip = val.src_ip_prefixlen;
      src_ip << ip;
      match_.inet_match_.src_network = src_ip.str();
    }
    if (val.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID)
      match_.inet_match_.protocol = val.ip_proto;

    if (val.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID)
      match_.inet_match_.dscp = val.ip_dscp;
  }

  if ( val.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) {

    pfc_log_info("Filling L4 Match Details");

      if ( val.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID )
        match_.icmp_type = val.icmp_type;

      if ( val.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID )
        match_.icmp_code =val.icmp_code;

      pfc_log_info("Filling L4 tcp Match Details");

      if ( val.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID) {
          match_.tcp_dest_range_.valid = true;
        match_.tcp_dest_range_.dst_port_from = val.l4_dst_port;
        match_.tcp_dest_range_.dst_port_to = val.l4_dst_port_endpt;
      if ( val.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID) {
          match_.tcp_src_range_.valid = true;
          match_.tcp_src_range_.src_port_from= val.l4_src_port;
      }
      if ( val.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] == UNC_VF_VALID) {
        match_.tcp_src_range_.src_port_to = val.l4_src_port_endpt;
      }
    }
  }
  ip_flowlistentry_st.in_flowcond_.match_.push_back(match_);
}

//  Method to  handle two value structures during update operation
void
OdcFlowListEntryCommand::copy(ip_flowlistentry&  ip_flowlistentry_st,
                              key_flowlist_entry& key,
                              val_flowlist_entry& val_old,
                              val_flowlist_entry& val_new) {
  ODC_FUNC_TRACE;
  unc::odcdriver::OdcUtil util;

  ip_flowlistentry_st.in_flowcond_.valid = true;
  ip_flowlistentry_st.in_flowcond_.name = reinterpret_cast <char *>(
                                         key.flowlist_key.flowlist_name);
  match match_;
  match_.index = key.sequence_num;

  // Ethernet Match Values
  if ( val_old.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID ) {
    pfc_log_info("Filling Ethernet Match Details");
    match_.ether_match_.valid = true;

    if(val_new.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID) {
      match_.ether_match_.dest_addr =
                         util.macaddress_to_string(&val_new.mac_dst[0]);
    } else if ( val_new.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_INVALID &&
          val_old.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID) {
      match_.ether_match_.dest_addr =
                                 util.macaddress_to_string(&val_old.mac_dst[0]);
    }
    if ( val_new.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID) {
      match_.ether_match_.src_addr =
                                  util.macaddress_to_string(&val_new.mac_src[0]);
    } else if ( val_new.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_INVALID &&
        val_old.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID ) {
      match_.ether_match_.src_addr = util.macaddress_to_string(
                                                   &val_old.mac_src[0]);
    }
    if ( val_new.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID) {
      match_.ether_match_.ether_type = val_new.mac_eth_type;
    } else if ( val_new.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_INVALID &&
        val_old.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID ){
      match_.ether_match_.ether_type = val_old.mac_eth_type;
    }
    if ( val_new.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID) {
      match_.ether_match_.vlanpri = val_new.vlan_priority;
    } else if ( val_new.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_INVALID &&
        val_old.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID ){
      match_.ether_match_.vlanpri= val_old.vlan_priority;
    }
  }

  //Inet Match
  if ( val_new.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_DST_IP_PREFIX_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_DST_IP_PREFIX_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_SRC_IP_PREFIX_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_SRC_IP_PREFIX_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID ) {

    pfc_log_info("Filling Inet Match Details");
    match_.inet_match_.valid = true;

    if (val_new.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID ||
        val_new.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALUE_NOT_MODIFIED) {
      match_.inet_match_.dest_network.assign(
                                        inet_ntoa(val_new.dst_ip));
      pfc_log_info("dst ip:%s",match_.inet_match_.dest_network.c_str());
    } else if ( val_new.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_INVALID &&
       val_old.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID) {
      match_.inet_match_.dest_network.assign(
                                                       inet_ntoa(val_old.dst_ip));
      pfc_log_info("dst ip:%s",match_.inet_match_.dest_network.c_str());
    }

    if (val_new.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID ||
        val_new.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALUE_NOT_MODIFIED) {
      match_.inet_match_.src_network.assign(inet_ntoa(val_new.src_ip));
      pfc_log_info("src ip:%s",match_.inet_match_.src_network.c_str());
    } else if (val_new.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_INVALID &&
        val_old.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID) {
      match_.inet_match_.src_network.assign(inet_ntoa(val_old.src_ip));
      pfc_log_info("src ip:%s",match_.inet_match_.src_network.c_str());
    }
    if (val_new.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID) {
      match_.inet_match_.protocol = val_new.ip_proto;
    } else if (val_new.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_INVALID &&
         val_old.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID) {
      match_.inet_match_.protocol= val_old.ip_proto;
    }

    if (val_new.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID) {
      match_.inet_match_.dscp = val_new.ip_dscp;
    } else if (val_new.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_INVALID &&
         val_old.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID) {
      match_.inet_match_.dscp = val_old.ip_dscp;
    }
  }

  // L4 Match
  if (val_new.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID ) {

    pfc_log_info("Filling L4 Match Details");

    if( val_new.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID ||
        val_old.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID ||
        val_new.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID ||
        val_old.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID ) {


      if ( val_new.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID) {
        match_.icmp_type = val_new.icmp_type;
      } else if ( val_new.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_INVALID &&
          val_old.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID) {
        match_.icmp_type = val_old.icmp_type;
      }

      if ( val_new.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) {
        match_.icmp_code = val_new.icmp_code;
      }  else if ( val_new.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_INVALID &&
          val_old.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) {
        match_.icmp_code = val_old.icmp_code;
      }
    } else {
      pfc_log_info("Filling L4 tcp Match Details");

      if ( val_new.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID) {
        match_.tcp_dest_range_.valid = true;
        match_.tcp_dest_range_.dst_port_from = val_new.l4_dst_port;
      } else if ( val_new.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_INVALID &&
          val_old.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID) {
        match_.tcp_dest_range_.valid = true;
        match_.tcp_dest_range_.dst_port_from = val_old.l4_dst_port;
      }

      if ( val_new.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] == UNC_VF_VALID) {
        match_.tcp_dest_range_.valid = true;
        match_.tcp_dest_range_.dst_port_to = val_new.l4_dst_port_endpt;
      } else if ( val_new.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] ==
           UNC_VF_INVALID && val_old.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]
                                 == UNC_VF_VALID) {
        match_.tcp_dest_range_.valid = true;
        match_.tcp_dest_range_.dst_port_to  = val_old.l4_dst_port_endpt;
      }

      if ( val_new.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID) {
        match_.tcp_src_range_.valid = true;
        match_.tcp_src_range_.src_port_from  = val_new.l4_src_port;
      } else if ( val_new.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_INVALID &&
          val_old.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID) {
        match_.tcp_src_range_.valid = true;
        match_.tcp_src_range_.src_port_from = val_old.l4_src_port;
      }

      if ( val_new.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] == UNC_VF_VALID) {
        match_.tcp_src_range_.valid = true;
        match_.tcp_src_range_.src_port_to = val_new.l4_src_port_endpt;
      } else if ( val_new.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] ==
           UNC_VF_INVALID && val_old.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE]
                                                         == UNC_VF_VALID) {
        match_.tcp_src_range_.valid = true;
        match_.tcp_src_range_.src_port_to = val_old.l4_src_port_endpt;
      }
    }
  }
  ip_flowlistentry_st.in_flowcond_.match_.push_back(match_);
}
}
}
