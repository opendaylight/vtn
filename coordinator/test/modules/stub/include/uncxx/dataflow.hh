/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_DATAFLOW_UTIL_HH_
#define UNC_DATAFLOW_UTIL_HH_

#include <cxx/pfcxx/ipc_server.hh>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <sstream>
#include <cxx/pfcxx/ipc_client.hh>
#include <unc/keytype.h>
#include <set>
using std::vector;
using std::map;
using std::stack;
using std::string;
using std::stringstream;
using std::endl;
using pfc::core::ipc::ServerSession;
using pfc::core::ipc::ClientSession;

#include "unc_dataflow.h"

#define kMaxLenCtrlrDomId  31

namespace unc {
namespace dataflow {

enum IpctStructNum {
  kidx_val_df_data_flow_cmn = 0,
  kidx_val_vtn_dataflow_cmn
};

struct key_vtn_ctrlr_dataflow {
  key_vtn  vtn_key;
  uint8_t    ctrlr[32];
  uint8_t    domain[32];
  uint16_t   vlanid;
  uint8_t    src_mac_address[6];
  key_vtn_ctrlr_dataflow(key_vtn_dataflow_t *key_vtn_df, uint8_t *in_ctrlr,
                         uint8_t *in_dom) {
    vtn_key = key_vtn_df->vtn_key;
    vlanid = key_vtn_df->vlanid;
    memcpy(ctrlr, in_ctrlr, sizeof(ctrlr));
    memcpy(domain, in_dom, sizeof(domain));
    memcpy(src_mac_address, key_vtn_df->src_mac_address, sizeof(src_mac_address));

  }
};


typedef struct actions_vect_st {
  UncDataflowFlowActionType action_type;
  void *action_st_ptr;
} val_actions_vect_st;

class AddlData {
 public:
  AddlData();
  ~AddlData();
  uint32_t  reason;
  uint32_t  controller_count;
  uint32_t  max_dom_traversal_count;
  uint32_t  current_traversal_count;
  map<string, uint32_t> traversed_controllers;
  string nonpfc_ingr_bdry_id;
};

class DataflowDummy : public pfc::core::Module {
 public:

  pfc_bool_t init() {
    return true;
  }
  pfc_bool_t fini() {
    return true;
  }
 private:
  static DataflowDummy* dataflow_dummy_;
};

class DataflowCmn;

class DataflowDetail {
 public:
  DataflowDetail(IpctStructNum df_type, unc_keytype_ctrtype_t ctr_type = UNC_CT_PFC) {
  }
  ~DataflowDetail() {
  }
  // Read the DataflowCmn details from session as mentioned in FD API doc
  int sessReadDataflow(ClientSession& sess, uint32_t& getres_pos) {
    return 1;
  }
  bool is_flow_drop;
  bool is_flow_redirect;
  val_df_data_flow_cmn_t *df_common;
  val_vtn_dataflow_cmn_t *vtn_df_common;
  map<UncDataflowFlowMatchType, void *> matches;
  vector<val_actions_vect_st *> actions;
  vector<val_df_data_flow_path_info_t *> path_infos;
  vector<val_vtn_dataflow_path_info_t *> vtn_path_infos;
  void *ckv_egress;
  uint32_t flow_traversed;
  IpctStructNum st_num_;
};

class DataflowCmn {
 public:
  DataflowCmn(bool isHead, DataflowDetail *df_segm) {
  }
  ~DataflowCmn() {
  }
  DataflowDetail *df_segment;
  bool is_head;
  map<UncDataflowFlowMatchType, void *> output_matches;
  vector<DataflowCmn *> next;
  DataflowCmn *head;
  AddlData *addl_data;
  uint32_t total_flow_count;
  bool is_vlan_src_mac_changed_;
  DataflowCmn *parent_node;
  bool action_applied;
  uint8_t pfc_vtn_name[32];

  std::string ToStr() {
    return "aa";

  }
  UncDataflowReason deleteflow(DataflowCmn* nextCtrlrFlow) {
      return UNC_DF_RES_SUCCESS;
    }


  // Write the DataflowCmn details into session as mentioned in FD API doc
  int sessOutDataflow(ServerSession& sess, int& putresp_pos) {
    return 1;
  }
  // Append given nextCtrlrFlow to 'next' vector
  UncDataflowReason appendFlow(DataflowCmn* nextCtrlrFlow
                  , map<string, uint32_t> & ctrlr_dom_count_map) {
    return UNC_DF_RES_SUCCESS;
  }

  // check given 'output_matches' against 'matches'
  bool check_match_condition(map <UncDataflowFlowMatchType
                            , void *> output_matches) {
    return true;
  }
  // Apply 'actions' to 'matches' and fill 'output_matches'
  void apply_action(){}
  bool CompareDataflow(DataflowCmn *otherflow) { return true; }

  static string get_string(const AddlData *ptr) { return "dummy"; }
  static string get_string(const key_dataflow_t &k) { return "dummy" ; }
  static string get_string(const key_ctr_dataflow_t &k) { return "dummy" ; }

  static string get_string(const val_df_data_flow_cmn_t &val_obj) { return "dummy" ;}

  static string get_string(const val_df_flow_match_t &val_obj) { return "dummy" ; }
  static string get_string(const val_df_flow_match_in_port_t &val_obj) { return "dummy"; }
  static string get_string(const val_df_flow_match_dl_addr_t &val_obj) { return "dummy" ; }
  static string get_string(const val_df_flow_match_dl_type_t &val_obj) { return "dummy"; }
  static string get_string(const val_df_flow_match_vlan_id_t &val_obj) { return "dummy"; }
  static string get_string(const val_df_flow_match_vlan_pcp_t &val_obj) { return "dummy"; }
  static string get_string(const val_df_flow_match_ip_tos_t &val_obj) { return "dummy" ; }
  static string get_string(const val_df_flow_match_ip_proto_t &val_obj) { return "dummy"; }
  static string get_string(const val_df_flow_match_ipv4_addr_t &val_obj) { return "dummy"; }
  static string get_string(const val_df_flow_match_tp_port_t &val_obj) { return "dummy"; }
  static string get_string(const val_df_flow_match_ipv6_addr_t &val_obj){ return "dummy"; }

  static string get_string(const val_df_flow_action_t &val_obj){ return "dummy"; }
  static string get_string(const val_df_flow_action_output_port_t &val_obj){ return "dummy"; }
  static string get_string(const val_df_flow_action_enqueue_port_t &val_obj){ return "dummy"; }
  static string get_string(const val_df_flow_action_set_dl_addr_t &val_obj){ return "dummy"; }
  static string get_string(const val_df_flow_action_set_vlan_id_t &val_obj){ return "dummy"; }
  static string get_string(const val_df_flow_action_set_vlan_pcp_t &val_obj) { return "dummy"; }
  static string get_string(const val_df_flow_action_set_ipv4_t &val_obj){ return "dummy"; }
  static string get_string(const val_df_flow_action_set_ip_tos_t &val_obj){ return "dummy"; }
  static string get_string(const val_df_flow_action_set_tp_port_t &val_obj) { return "dummy"; }
  static string get_string(const val_df_flow_action_set_ipv6_t &val_obj) { return "dummy"; }
  static string get_string(key_dataflow_v2_t& key_obj) {return "dummy";}
  static string get_string(val_dataflow_v2_t &val_obj) { return "dummy"; }
  static string get_string(const val_df_data_flow_path_info_t &val_obj) { return "dummy"; }
  static bool Compare(const key_dataflow_t& lhs, const key_dataflow_t& rhs) { return true; }
  void deep_copy() {}
 /* VTN */
  static string get_string(const key_vtn_dataflow_t &k) { return "dummy"; }
  static string get_string(const val_vtn_dataflow_path_info_t &val_obj) { return "dummy"; }
  static string get_string(const val_vtn_dataflow_cmn_t &val_obj) { return "dummy"; }
  static bool Compare(const key_vtn_ctrlr_dataflow& lhs, const key_vtn_ctrlr_dataflow& rhs) { return false;}
  bool CompareVtnDataflow(DataflowCmn *otherflow) { return true; }

};

struct KeyDataflowCmp {
  bool operator()(const key_dataflow_t& lhs, const key_dataflow_t& rhs) {
    return false;
  }
};



struct KeyVtnDataflowCmp {
  bool operator()(const key_vtn_ctrlr_dataflow &lhs, const key_vtn_ctrlr_dataflow &rhs) {
    return false;
  }
};

class DataflowUtil {
 public:
  ~DataflowUtil() { }
  // Write the DataflowCmn details into NB session as mentioned in FD API doc
  int sessOutDataflows(ServerSession& sess) { return 1; }

  // Method for PFCDriver module. Write the DataflowCmn details
  // into physical session as mentioned in FD API doc
  int sessOutDataflowsFromDriver(ServerSession& sess) { return 1; }
  uint32_t storeFlowDetails(key_dataflow_t, vector<DataflowDetail*> )  { return 1; }

  uint32_t get_total_flow_count() { return 1;}
  // Append given firstCtrlrFlow to 'firstCtrlrFlows' vector
  uint32_t appendFlow(DataflowCmn* firstCtrlrFlow) { return 1;}

  static string get_string(const val_df_data_flow_t &val_obj) { return "dummy"; }
  static string get_string(const val_df_data_flow_st_t &val_obj) { return "dummy"; }
  static string get_string(const val_vtn_dataflow_t &val_obj) { return "dummy"; }
  static bool checkMacAddress(uint8_t macaddr[6], uint8_t macaddr_mask[6]
                             , uint8_t checkmacaddr[6]) { return true; }
  static bool checkMacAddress(uint8_t macaddr[6], uint8_t macaddr_mask[6]
                             , uint8_t checkmacaddr[6]
                             , uint8_t checkmacaddr_mask[6]) { return true; }
  static bool checkIPv4Address(in_addr ipaddr, in_addr ip_mask
                             , in_addr checkipaddr) { return true; }
  static bool checkIPv4Address(uint32_t ipaddr, uint32_t ip_mask
                             , uint32_t checkipaddr) { return true; }
  static bool checkByte(uint8_t ipaddr, uint8_t ip_mask
                             , uint8_t checkipaddr) { return true; }
  static bool checkByte(uint8_t ipaddr, uint8_t ip_mask
                             , uint8_t checkipaddr, uint8_t chk_ip_mask) { return true; }
  static bool checkIPv4Address(in_addr ipaddr, in_addr ip_mask
                             , in_addr checkipaddr, in_addr chk_ip_mask) { return true; }
  static bool checkIPv4Address(uint32_t ipaddr, uint32_t ip_mask
                             , uint32_t checkipaddr, uint32_t chk_ip_mask) { return true; }
  static bool checkIPv6Address(in6_addr ipv6addr, in6_addr ipv6_mask
                             , in6_addr checkipv6addr) { return true; }
  static bool checkIPv6Address(in6_addr ipv6addr, in6_addr ipv6_mask
                             , in6_addr checkipv6addr, in6_addr chk_ipv6_mask) { return true; }
  static string getipstring(in_addr ipnaddr, int radix) { return "dummy"; }
  static string getipstring(uint32_t ipaddr, int radix) { return "dummy"; }
  static string getbytestring(uint8_t ipaddr, int radix) { return "dummy"; }

  vector<DataflowCmn*>* get_firstCtrlrFlows() {
    return &firstCtrlrFlows;
  };
  std::map<std::string, uint32_t>* get_ctrlr_dom_count_map() {
    return &ctrlr_dom_count_map;
  };
  std::map<key_dataflow_t, vector<DataflowDetail *>, KeyDataflowCmp > pfc_flows;
  std::map<key_vtn_ctrlr_dataflow, vector<DataflowDetail *>, KeyVtnDataflowCmp  > upll_pfc_flows;
  std::map<std::string, uint32_t> ctrlr_dom_count_map;
  std::set<std::string> bypass_dom_set;
  std::map<std::string, void* > vext_info_map;
  std::map<std::string, std::string> vnode_rename_map;

 private:
  vector<DataflowCmn* > firstCtrlrFlows;
};

}  // namespace dataflow
}  // namespace unc

#endif  // UNC_DATAFLOW_UTIL_HH_
