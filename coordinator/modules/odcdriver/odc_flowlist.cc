/*
 * Copyright (c) 2014 NEC Corporation
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
OdcFlowListCommand::copy(flowcondition* flow,
                         key_flowlist& key,
                         val_flowlist& val) {
  ODC_FUNC_TRACE;
  char *flowlist_name = reinterpret_cast <char *>(key.flowlist_name);
  flow->name_.assign(flowlist_name);
}
void
OdcFlowListCommand::copy(flowcondition* flow,
                         key_flowlist& key,
                         val_flowlist& val_old,
                          val_flowlist& val_new) {
  ODC_FUNC_TRACE;
  char *flowlist_name = reinterpret_cast <char *>(key.flowlist_name);
  flow->name_.assign(flowlist_name);
}
std::string
OdcFlowListCommand::get_url_tail(key_flowlist& key,
                                 val_flowlist& val) {
  ODC_FUNC_TRACE;
  char *flowlist_name = reinterpret_cast <char *>(key.flowlist_name);
  std::string url_string(flowlist_name);
  return url_string;
}

UncRespCode
OdcFlowListCommand::r_copy(flowConditions* in,
                           std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;

  unc::odcdriver::OdcUtil util;
  if ( in == NULL )
    return UNC_DRV_RC_ERR_GENERIC;

  std::list<flowcondition*>::iterator list_iter = in->flowcondition_.begin();

  while ( list_iter != in->flowcondition_.end() ) {
    PFC_ASSERT(*list_iter != NULL);
    key_flowlist key_fl;
    val_flowlist val_fl;

    memset(&key_fl,0,sizeof(key_flowlist));
    memset(&val_fl,0,sizeof(val_flowlist));

    // Copy Condition NAme to key!!
    strcpy(reinterpret_cast<char*>(key_fl.flowlist_name),(*list_iter)->name_.c_str());

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

    std::list<match*>::iterator match_iter = (*list_iter)->match_.begin();

    while ( match_iter != (*list_iter)->match_.end () ) {
      PFC_ASSERT(*match_iter != NULL);

      match *match_entry(*match_iter);
      // Create flowlist entry key
      key_flowlist_entry key_fl_entry;
      val_flowlist_entry val_fl_entry;

      memset(&key_fl_entry,0,sizeof(key_flowlist_entry));
      memset(&val_fl_entry,0,sizeof(val_flowlist_entry));

      memcpy(&key_fl_entry.flowlist_key, &key_fl,sizeof(key_flowlist));
      key_fl_entry.sequence_num=(*match_iter)->index_;
      if ( match_entry->ethernet_) {
        if ( match_entry->ethernet_->src_ != "" ) {
          util.convert_macstring_to_uint8(match_entry->ethernet_->src_,
                                          &val_fl_entry.mac_src[0]);
          val_fl_entry.valid[UPLL_IDX_MAC_SRC_FLE] = UNC_VF_VALID;
        }
        if ( match_entry->ethernet_->dst_ != "" ) {
          util.convert_macstring_to_uint8(match_entry->ethernet_->dst_,
                                          &val_fl_entry.mac_dst[0]);
          val_fl_entry.valid[UPLL_IDX_MAC_DST_FLE] = UNC_VF_VALID;
        }
        if ( match_entry->ethernet_->type_ != -1 ) {
          val_fl_entry.mac_eth_type=match_entry->ethernet_->type_;
          val_fl_entry.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] = UNC_VF_VALID;
        }
        if (  match_entry->ethernet_->vlanpri_ != -1 ) {
          val_fl_entry.vlan_priority=match_entry->ethernet_->vlanpri_;
          val_fl_entry.valid[UPLL_IDX_VLAN_PRIORITY_FLE] = UNC_VF_VALID;
        }
      }

      if (  match_entry->inetMatch_ ) {
        if ( match_entry->inetMatch_->inet4_) {
          if ( match_entry->inetMatch_->inet4_->src_ != "" ) {
            util.convert_ip_to_inaddr(match_entry->inetMatch_->inet4_->src_,
                                      &val_fl_entry.src_ip);
            val_fl_entry.valid[UPLL_IDX_SRC_IP_FLE] = UNC_VF_VALID;
            val_fl_entry.src_ip_prefixlen= 32;
          }
          if ( match_entry->inetMatch_->inet4_->dst_ != "" ) {
            util.convert_ip_to_inaddr(match_entry->inetMatch_->inet4_->dst_,
                                      &val_fl_entry.dst_ip);
            val_fl_entry.valid[UPLL_IDX_DST_IP_FLE] = UNC_VF_VALID;
            val_fl_entry.valid[UPLL_IDX_SRC_IP_PREFIX_FLE]=UNC_VF_VALID;
            val_fl_entry.dst_ip_prefixlen= 32;
          }
          if ( match_entry->inetMatch_->inet4_->protocol_ != -1 ) {
            val_fl_entry.ip_proto=match_entry->inetMatch_->inet4_->protocol_;
            val_fl_entry.valid[UPLL_IDX_IP_PROTOCOL_FLE]=UNC_VF_VALID;
          }
          if ( match_entry->inetMatch_->inet4_->dscp_ != -1 ) {
            val_fl_entry.ip_dscp=match_entry->inetMatch_->inet4_->dscp_;
            val_fl_entry.valid[UPLL_IDX_IP_DSCP_FLE]=UNC_VF_VALID;
          }

        }
      }

      if ( match_entry->l4Match_ ) {
        if ( match_entry->l4Match_->icmp_ ) {
          if ( match_entry->l4Match_->icmp_->type_ != -1 ) {
            val_fl_entry.icmp_type = match_entry->l4Match_->icmp_->type_;
            val_fl_entry.valid[UPLL_IDX_ICMP_TYPE_FLE]=UNC_VF_VALID;
          }
          if ( match_entry->l4Match_->icmp_->code_ != -1 ) {
            val_fl_entry.icmp_type = match_entry->l4Match_->icmp_->code_;
            val_fl_entry.valid[UPLL_IDX_ICMP_CODE_FLE]=UNC_VF_VALID;
          }
        }
        if ( match_entry->l4Match_->tcp_) {
          if ( match_entry->l4Match_->tcp_->dst_ ) {
            if ( match_entry->l4Match_->tcp_->dst_->from_ != -1 ) {
              val_fl_entry.l4_dst_port= match_entry->l4Match_->tcp_->dst_->from_;
              val_fl_entry.valid[UPLL_IDX_L4_DST_PORT_FLE]=UNC_VF_VALID;
            }
            if ( match_entry->l4Match_->tcp_->dst_->to_ != -1 ) {
              val_fl_entry.l4_dst_port_endpt= match_entry->l4Match_->tcp_->dst_->to_;
              val_fl_entry.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]=UNC_VF_VALID;
            }
          }
          if ( match_entry->l4Match_->tcp_->src_ ) {
            if ( match_entry->l4Match_->tcp_->src_->from_ != -1 ) {
              val_fl_entry.l4_src_port= match_entry->l4Match_->tcp_->src_->from_;
              val_fl_entry.valid[UPLL_IDX_L4_SRC_PORT_FLE]=UNC_VF_VALID;
            }
            if ( match_entry->l4Match_->tcp_->src_->to_ != -1 ) {
              val_fl_entry.l4_src_port_endpt= match_entry->l4Match_->tcp_->src_->to_;
              val_fl_entry.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE]=UNC_VF_VALID;
            }
          }

        }
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




void
OdcFlowListEntryCommand::copy(flowcondition* flow,
                              key_flowlist_entry& key,
                              val_flowlist_entry& val) {
  ODC_FUNC_TRACE;
  PFC_ASSERT ( flow != NULL );
  char *flowlist_name=reinterpret_cast <char *>(key.flowlist_key.flowlist_name);
  flow->name_.assign(flowlist_name);
  unc::odcdriver::OdcUtil util;


  match* flow_match=new match();
  flow_match->index_=key.sequence_num;
  // Ethernet Match Values
  if ( val.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID ) {

    pfc_log_info("Filling Ethernet Match Details");
    flow_match->ethernet_=new ethernet();

    if ( val.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID )
      flow_match->ethernet_->dst_=util.macaddress_to_string(&val.mac_dst[0]);

    if ( val.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID )
      flow_match->ethernet_->src_=util.macaddress_to_string(&val.mac_src[0]);

    if ( val.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID )
      flow_match->ethernet_->type_=val.mac_eth_type;

    if ( val.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID )
      flow_match->ethernet_->vlanpri_=val.vlan_priority;

  }

  if ( val.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_DST_IP_PREFIX_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_SRC_IP_PREFIX_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID ) {

    pfc_log_info("Filling Inet Match Details");
    flow_match->inetMatch_=new inetMatch();
    flow_match->inetMatch_->inet4_=new inet4();

    if( val.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID )
      flow_match->inetMatch_->inet4_->dst_.assign(inet_ntoa(val.dst_ip));

    if( val.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID )
      flow_match->inetMatch_->inet4_->src_.assign(inet_ntoa(val.src_ip));

    if (val.valid[UPLL_IDX_DST_IP_PREFIX_FLE] == UNC_VF_VALID)
      flow_match->inetMatch_->inet4_->dstsuffix_=val.dst_ip_prefixlen;

    if (val.valid[UPLL_IDX_SRC_IP_PREFIX_FLE] == UNC_VF_VALID)
      flow_match->inetMatch_->inet4_->srcsuffix_=val.src_ip_prefixlen;

    if (val.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID)
      flow_match->inetMatch_->inet4_->protocol_=val.ip_proto;

    if (val.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID)
      flow_match->inetMatch_->inet4_->dscp_=val.ip_dscp;
  }

  if ( val.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID ||
       val.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) {

    pfc_log_info("Filling L4 Match Details");
    flow_match->l4Match_ =new l4Match();

    if( val.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID ||
        val.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) {

      flow_match->l4Match_->icmp_ =new icmp();

      if ( val.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID )
        flow_match->l4Match_->icmp_->type_=val.icmp_type;

      if ( val.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID )
        flow_match->l4Match_->icmp_->code_=val.icmp_code;

    } else {
      flow_match->l4Match_->tcp_=new tcp();
      pfc_log_info("Filling L4 tcp Match Details");

      if ( val.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->dst_ == NULL )
          flow_match->l4Match_->tcp_->dst_=new dst();
        flow_match->l4Match_->tcp_->dst_->from_=val.l4_dst_port;
      }
      if ( val.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->dst_ == NULL )
          flow_match->l4Match_->tcp_->dst_=new dst();
        flow_match->l4Match_->tcp_->dst_->to_=val.l4_dst_port_endpt;
      }
      if ( val.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->src_ == NULL )
          flow_match->l4Match_->tcp_->src_=new src();
        flow_match->l4Match_->tcp_->src_->from_=val.l4_src_port;
      }
      if ( val.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->src_ == NULL )
          flow_match->l4Match_->tcp_->src_=new src();
        flow_match->l4Match_->tcp_->src_->to_=val.l4_src_port_endpt;
      }
    }
  }
  flow->match_.push_back(flow_match);
}
//  Method to  handle two value structures during update operation
void
OdcFlowListEntryCommand::copy(flowcondition* flow,
                              key_flowlist_entry& key,
                              val_flowlist_entry& val_old,
                              val_flowlist_entry& val_new) {
  ODC_FUNC_TRACE;
  PFC_ASSERT( flow != NULL );
  char *flowlist_name = reinterpret_cast <char *>(
                            key.flowlist_key.flowlist_name);
  flow->name_.assign(flowlist_name);
  unc::odcdriver::OdcUtil util;


  match* flow_match = new match();
  flow_match->index_= key.sequence_num;
  // Ethernet Match Values
  if ( val_new.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID ||
       val_new.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID ||
       val_old.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID ) {
    pfc_log_info("Filling Ethernet Match Details");
    flow_match->ethernet_=new ethernet();

    if(val_new.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID) {
      flow_match->ethernet_->dst_ = util.macaddress_to_string(
                                           &val_new.mac_dst[0]);
    } else if ( val_new.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_INVALID &&
          val_old.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID) {
      flow_match->ethernet_->dst_ = util.macaddress_to_string(
                                                 &val_old.mac_dst[0]);
    }
    if ( val_new.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID) {
      flow_match->ethernet_->src_ = util.macaddress_to_string(
                                                  &val_new.mac_src[0]);
    } else if ( val_new.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_INVALID &&
        val_old.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID ) {
      flow_match->ethernet_->src_ = util.macaddress_to_string(
                                                   &val_old.mac_src[0]);
    }
    if ( val_new.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID) {
      flow_match->ethernet_->type_ = val_new.mac_eth_type;
    } else if ( val_new.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_INVALID &&
        val_old.valid[UPLL_IDX_MAC_ETH_TYPE_FLE] == UNC_VF_VALID ){
      flow_match->ethernet_->type_ = val_old.mac_eth_type;
    }
    if ( val_new.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID) {
      flow_match->ethernet_->vlanpri_ = val_new.vlan_priority;
    } else if ( val_new.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_INVALID &&
        val_old.valid[UPLL_IDX_VLAN_PRIORITY_FLE] == UNC_VF_VALID ){
      flow_match->ethernet_->vlanpri_ = val_old.vlan_priority;
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
    flow_match->inetMatch_ = new inetMatch();
    flow_match->inetMatch_->inet4_ = new inet4();

    if ((val_new.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID) ||
        (val_new.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALUE_NOT_MODIFIED)) {
      flow_match->inetMatch_->inet4_->dst_.assign(inet_ntoa(val_new.dst_ip));
      pfc_log_info("dst ip:%s",flow_match->inetMatch_->inet4_->dst_.c_str());
    } else if ( val_new.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_INVALID &&
       val_old.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID) {
      flow_match->inetMatch_->inet4_->dst_.assign(inet_ntoa(val_old.dst_ip));
      pfc_log_info("dst ip:%s",flow_match->inetMatch_->inet4_->dst_.c_str());
    }

    if ((val_new.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID) ||
        (val_new.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALUE_NOT_MODIFIED)) {
      flow_match->inetMatch_->inet4_->src_.assign(inet_ntoa(val_new.src_ip));
      pfc_log_info("src ip:%s",flow_match->inetMatch_->inet4_->src_.c_str());
    } else if (val_new.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_INVALID &&
        val_old.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID) {
      flow_match->inetMatch_->inet4_->src_.assign(inet_ntoa(val_old.src_ip));
      pfc_log_info("src ip:%s",flow_match->inetMatch_->inet4_->src_.c_str());
    }

    if ((val_new.valid[UPLL_IDX_DST_IP_PREFIX_FLE] == UNC_VF_VALID) ||
       (val_new.valid[UPLL_IDX_DST_IP_PREFIX_FLE] == UNC_VF_VALUE_NOT_MODIFIED)){
      flow_match->inetMatch_->inet4_->dstsuffix_ = val_new.dst_ip_prefixlen;
      pfc_log_info("prefixlen :%d",val_new.dst_ip_prefixlen);
    } else if (val_new.valid[UPLL_IDX_DST_IP_PREFIX_FLE] == UNC_VF_INVALID &&
        val_old.valid[UPLL_IDX_DST_IP_PREFIX_FLE] == UNC_VF_VALID) {
      flow_match->inetMatch_->inet4_->dstsuffix_ = val_old.dst_ip_prefixlen;
      pfc_log_info("Filling Inet Match Details dst len old struct");
    }

    if ((val_new.valid[UPLL_IDX_SRC_IP_PREFIX_FLE] == UNC_VF_VALID) ||
        (val_new.valid[UPLL_IDX_SRC_IP_PREFIX_FLE] ==
                                             UNC_VF_VALUE_NOT_MODIFIED)) {
      flow_match->inetMatch_->inet4_->srcsuffix_ = val_new.src_ip_prefixlen;
      pfc_log_info("prefixlen :%d",val_new.src_ip_prefixlen);
    } else if (val_new.valid[UPLL_IDX_SRC_IP_PREFIX_FLE] == UNC_VF_INVALID &&
          val_old.valid[UPLL_IDX_SRC_IP_PREFIX_FLE] == UNC_VF_VALID) {
      flow_match->inetMatch_->inet4_->srcsuffix_ = val_old.src_ip_prefixlen;
    }

    if (val_new.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID) {
      flow_match->inetMatch_->inet4_->protocol_ = val_new.ip_proto;
    } else if (val_new.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_INVALID &&
         val_old.valid[UPLL_IDX_IP_PROTOCOL_FLE] == UNC_VF_VALID) {
      flow_match->inetMatch_->inet4_->protocol_ = val_old.ip_proto;
    }

    if (val_new.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID) {
      flow_match->inetMatch_->inet4_->dscp_ = val_new.ip_dscp;
    } else if (val_new.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_INVALID &&
         val_old.valid[UPLL_IDX_IP_DSCP_FLE] == UNC_VF_VALID) {
      flow_match->inetMatch_->inet4_->dscp_ = val_old.ip_dscp;
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
       val_old.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) {

    pfc_log_info("Filling L4 Match Details");
    flow_match->l4Match_ =new l4Match();

    if( val_new.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID ||
        val_old.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID ||
        val_new.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID ||
        val_old.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) {

      flow_match->l4Match_->icmp_ =new icmp();

      if ( val_new.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID) {
        flow_match->l4Match_->icmp_->type_ = val_new.icmp_type;
      } else if ( val_new.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_INVALID &&
          val_old.valid[UPLL_IDX_ICMP_TYPE_FLE] == UNC_VF_VALID) {
        flow_match->l4Match_->icmp_->type_ = val_old.icmp_type;
      }

      if ( val_new.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) {
        flow_match->l4Match_->icmp_->code_ = val_new.icmp_code;
      }  else if ( val_new.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_INVALID &&
          val_old.valid[UPLL_IDX_ICMP_CODE_FLE] == UNC_VF_VALID) {
        flow_match->l4Match_->icmp_->code_ = val_old.icmp_code;
      }
    } else {
      flow_match->l4Match_->tcp_ = new tcp();
      pfc_log_info("Filling L4 tcp Match Details");

      if ( val_new.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->dst_ == NULL)
          flow_match->l4Match_->tcp_->dst_ = new dst();
        flow_match->l4Match_->tcp_->dst_->from_ = val_new.l4_dst_port;
      } else if ( val_new.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_INVALID &&
          val_old.valid[UPLL_IDX_L4_DST_PORT_FLE] == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->dst_ == NULL )
          flow_match->l4Match_->tcp_->dst_ = new dst();
        flow_match->l4Match_->tcp_->dst_->from_ = val_old.l4_dst_port;
      }

      if ( val_new.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->dst_ == NULL )
          flow_match->l4Match_->tcp_->dst_ = new dst();
        flow_match->l4Match_->tcp_->dst_->to_=val_new.l4_dst_port_endpt;
      } else if ( val_new.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE] ==
           UNC_VF_INVALID && val_old.valid[UPLL_IDX_L4_DST_PORT_ENDPT_FLE]
                                 == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->dst_ == NULL )
          flow_match->l4Match_->tcp_->dst_ = new dst();
        flow_match->l4Match_->tcp_->dst_->to_ = val_old.l4_dst_port_endpt;
      }

      if ( val_new.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->src_ == NULL )
          flow_match->l4Match_->tcp_->src_ = new src();
        flow_match->l4Match_->tcp_->src_->from_ = val_new.l4_src_port;
      } else if ( val_new.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_INVALID &&
          val_old.valid[UPLL_IDX_L4_SRC_PORT_FLE] == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->src_ == NULL )
          flow_match->l4Match_->tcp_->src_ = new src();
        flow_match->l4Match_->tcp_->src_->from_ = val_old.l4_src_port;
      }

      if ( val_new.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->src_ == NULL )
          flow_match->l4Match_->tcp_->src_ = new src();
        flow_match->l4Match_->tcp_->src_->to_ = val_new.l4_src_port_endpt;
      } else if ( val_new.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE] ==
           UNC_VF_INVALID && val_old.valid[UPLL_IDX_L4_SRC_PORT_ENDPT_FLE]
                                                         == UNC_VF_VALID) {
        if ( flow_match->l4Match_->tcp_->src_ == NULL )
          flow_match->l4Match_->tcp_->src_ = new src();
        flow_match->l4Match_->tcp_->src_->to_ = val_old.l4_src_port_endpt;
      }
    }
  }
  flow->match_.push_back(flow_match);
}
std::string
OdcFlowListEntryCommand::get_url_tail(key_flowlist_entry& key,
                                      val_flowlist_entry& val) {
  ODC_FUNC_TRACE;
  char *flowlist_name=reinterpret_cast <char *>(key.flowlist_key.flowlist_name);
  std::string url_string ("");
  url_string.append(flowlist_name);
  url_string.append("/");
  char sequence_no[10];
  sprintf(sequence_no,"%d",key.sequence_num);
  url_string.append(sequence_no);
  return url_string;
}


}
}
