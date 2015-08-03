/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @file    dataflow.cc
 *
 */

#include "uncxx/dataflow.hh"
#include <unc/unc_base.h>
#include <arpa/inet.h>
#include <sstream>
#include <bitset>
#include "unc/keytype.h"
#include "ipc_util.hh"

#define DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_xx_t, \
                                   match_iterator, \
                                   session, \
                                   putresp_position, \
                                   err_code) \
  val_df_flow_match_xx_t *ptr_t = \
     reinterpret_cast<val_df_flow_match_xx_t *>((match_iterator).second); \
  pfc_log_debug("%d. %s", (putresp_position), get_string(*ptr_t).c_str()); \
  (putresp_position)++; \
  (err_code) |= (session).addOutput(*ptr_t); \
  if ((err_code) != UNC_RC_SUCCESS) return (err_code);

#define DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_xx_t, \
                                  actions_vect_st, \
                                  session, \
                                  putresp_position, \
                                  err_code) \
  val_df_flow_action_set_xx_t *ptr_t = \
    reinterpret_cast<val_df_flow_action_set_xx_t *>( \
      (actions_vect_st)->action_st_ptr); \
  pfc_log_debug("%d. %s", (putresp_position), get_string(*ptr_t).c_str()); \
  (putresp_position)++; \
  (err_code) |= (session).addOutput(*ptr_t); \
  if ((err_code) != UNC_RC_SUCCESS) return (err_code);

#define DATAFLOW_MATCHES_GETRESP(val_df_flow_match_xx_t, \
                                 mmatches, \
                                 UncDataflowFlowMatchType_val, \
                                 session, \
                                 getres_position, \
                                 err_code) \
  val_df_flow_match_xx_t *ptr_t =  new val_df_flow_match_xx_t; \
  memset(ptr_t, 0, sizeof(val_df_flow_match_xx_t)); \
  (err_code) |= (session).getResponse((getres_position)++, *ptr_t); \
  if ((err_code) != UNC_RC_SUCCESS) { \
    pfc_log_debug("getResponse failed in DATAFLOW_MATCHES_GETRESP macro");\
    delete ptr_t; \
    return (err_code); \
  } \
  (mmatches)[(UncDataflowFlowMatchType_val)] = ptr_t;

#define DATAFLOW_ACTION_GETRESP(val_df_flow_action_xx_t, \
                                getres_position, \
                                session, \
                                actions_vect_st, \
                                err_code) \
  val_df_flow_action_xx_t *ptr_t =  new val_df_flow_action_xx_t; \
  memset(ptr_t, 0, sizeof(val_df_flow_action_xx_t)); \
  (err_code) |= (session).getResponse((getres_position)++, *ptr_t); \
  if ((err_code) != UNC_RC_SUCCESS) { \
    pfc_log_debug("getResponse failed in DATAFLOW_ACTION_GETRESP macro");\
    delete ptr_t; \
    delete actions_vect_st; \
    return (err_code); \
  } \
  (actions_vect_st)->action_st_ptr = ptr_t;

#define DATAFLOW_MATCHES_GETSTRING(sstr, match_iter, val_df_flow_match_xx_t) \
  val_df_flow_match_xx_t *ptr_t = \
    reinterpret_cast<val_df_flow_match_xx_t*>((match_iter).second); \
  (sstr) << "  " << ((match_iter).first) << "..." << get_string(*ptr_t); \

#define DATAFLOW_ACTIONS_GETSTRING(sstr, ptr, val_df_flow_action_xx_t) \
  val_df_flow_action_xx_t *ptr_t = \
    reinterpret_cast<val_df_flow_action_xx_t *>(ptr->action_st_ptr); \
  (sstr) << "  " << (ptr)->action_type << "..." << get_string(*ptr_t); \

#define DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_xx_t, \
                                           op_matches, \
                                           UncDataflowFlowMatchType_val, \
                                           iterator) \
  val_df_flow_match_xx_t *val_mat = \
    reinterpret_cast<val_df_flow_match_xx_t*>((iterator).second); \
  val_df_flow_match_xx_t *val_out_mat = new(val_df_flow_match_xx_t); \
  memcpy(val_out_mat, val_mat, sizeof(val_df_flow_match_xx_t)); \
  (op_matches)[(UncDataflowFlowMatchType_val)] = val_out_mat;

using std::stringstream;
using std::bitset;
using namespace unc::dataflow;
using unc::upll::ipc_util::ConfigKeyVal;

string uint8tostr(const uint8_t& c) {
  char str[20];
  memset(&str, '\0', 20);
  snprintf(str, sizeof(str), "%d", c);
  string str1(str);
  return str1;
}
string uint16tostr(const uint16_t& c) {
  char str[20];
  snprintf(str, sizeof(str), "%d", c);
  string str1(str);
  return str1;
}
string uint64tostr(const uint64_t& c) {
  char str[25];
  snprintf(str, sizeof(str), "%" PFC_PFMT_u64, c);
  string str1(str);
  return str1;
}




DataflowDummy* DataflowDummy::dataflow_dummy_ = NULL;

DataflowDummy::DataflowDummy(const pfc_modattr_t *attr)
:pfc::core::Module(attr) {
}

DataflowDummy* DataflowDummy::get_dataflow_dummy() {
  /** No Need to check NULL and create capabilit object.*/
  return dataflow_dummy_;
}

pfc_bool_t DataflowDummy::init() {
  dataflow_dummy_ = this;
  return PFC_TRUE;
}

pfc_bool_t DataflowDummy::fini() {
  return PFC_TRUE;
}

AddlData::AddlData()
    : reason(0),
      controller_count(0),
      max_dom_traversal_count(0),
      current_traversal_count(0) {
}

AddlData::~AddlData() {
  traversed_controllers.clear();
}

DataflowDetail::DataflowDetail(IpctStructNum df_type,
                               unc_keytype_ctrtype_t ctr_type)
    : df_common(NULL), vtn_df_common(NULL), ckv_egress(NULL), 
      flow_traversed(0), is_flow_redirect(false), is_flow_drop(false), st_num_(df_type) {
  if (st_num_ == kidx_val_df_data_flow_cmn) {
    df_common = new val_df_data_flow_cmn_t;
    memset(df_common, 0, sizeof(val_df_data_flow_cmn_t));
    df_common->controller_type = ctr_type;
    
    if (df_common->controller_type == UNC_CT_VNP ||
        df_common->controller_type == UNC_CT_POLC) {
      df_common->valid[kidxDfDataFlowControllerName] = 1;
      df_common->valid[kidxDfDataFlowControllerType] = 1;
      df_common->valid[kidxDfDataFlowIngressSwitchId] = 1;
      df_common->valid[kidxDfDataFlowInPort] = 1;
      df_common->valid[kidxDfDataFlowInDomain] = 1;
      df_common->valid[kidxDfDataFlowEgressSwitchId] = 1;
      df_common->valid[kidxDfDataFlowOutPort] = 1;
      df_common->valid[kidxDfDataFlowOutDomain] = 1;
    } else if (df_common->controller_type == UNC_CT_UNKNOWN) {
      df_common->valid[kidxDfDataFlowControllerName] = 1;
      df_common->valid[kidxDfDataFlowControllerType] = 1;
      df_common->valid[kidxDfDataFlowInDomain] = 1;
      df_common->valid[kidxDfDataFlowOutDomain] = 1;
    }
    
    
  } else {
    // LOGICAL
    flow_traversed = 0;
    vtn_df_common = new val_vtn_dataflow_cmn_t;
    memset(vtn_df_common, 0, sizeof(val_vtn_dataflow_cmn_t));
    vtn_df_common->controller_type = ctr_type;
    if ((vtn_df_common->controller_type == UNC_CT_VNP) ||
	  (vtn_df_common->controller_type == UNC_CT_POLC) ||
	  (vtn_df_common->controller_type == UNC_CT_UNKNOWN)) {
      if (vtn_df_common->controller_type != UNC_CT_UNKNOWN)
       vtn_df_common->valid[UPLL_IDX_CONTROLLER_ID_VVDC]     = 1;  // Controller Name
    	vtn_df_common->valid[UPLL_IDX_CONTROLLER_TYPE_VVDC]    = 1;  // Controller Type
    	vtn_df_common->valid[UPLL_IDX_INGRESS_VNODE_VVDC]      = 1;  // ingress vnode ****
    	vtn_df_common->valid[UPLL_IDX_INGRESS_VINTERFACE_VVDC] = 1;  // ingress vInterface Name****
    	vtn_df_common->valid[UPLL_IDX_INGRESS_DOMAIN_VVDC]     = 1; // ingress domain ****
     	vtn_df_common->valid[UPLL_IDX_EGRESS_VNODE_VVDC]       = 1; // egress vnode
    	vtn_df_common->valid[UPLL_IDX_EGRESS_VINTERFACE_VVDC]  = 1; // egress vInterface Name
    	vtn_df_common->valid[UPLL_IDX_EGRESS_DOMAIN_VVDC]      = 1; // egress domain
    }
  }
}


/** Constructor
 * * @Description : This constructor initializes member variables
 * * @param[in]   : isHead, DataflowDetail pointer
 * * @return      : None
 **/
DataflowCmn::DataflowCmn(bool isHead, DataflowDetail *df_segm)
    : df_segment(df_segm),
      is_head(isHead),
      head(NULL),
      total_flow_count(0),
      is_vlan_src_mac_changed_(false),
      parent_node(NULL),
      action_applied(false) {
  addl_data = new AddlData();
}


/** Destructor
 * * @Description : Destructor deletes all allocated memories
 * * @param[in]   : None
 * * @return      : None
 **/
DataflowCmn::~DataflowCmn() {
  pfc_log_trace("DataflowCmn -Destructor call");
  if (df_segment != NULL) {
    if (df_segment->st_num_ == kidx_val_df_data_flow_cmn) {
      if (df_segment->df_common->controller_type != UNC_CT_PFC &&
          df_segment->df_common->controller_type != UNC_CT_ODC) {
        pfc_log_debug("~DataflowCmn deleting %d df_segment %p",
          df_segment->df_common->controller_type, df_segment);
        delete df_segment;
        df_segment = NULL;
      }
    } else {
      // LOGICAL
      if (df_segment->vtn_df_common->controller_type != UNC_CT_PFC &&
          df_segment->vtn_df_common->controller_type != UNC_CT_ODC) {
        pfc_log_debug("~DataflowCmn deleting %d df_segment %p",
           df_segment->vtn_df_common->controller_type, df_segment);
        delete df_segment;
        df_segment = NULL;
      }
    }
  }
  if (output_matches.size() > 0) {
  map<UncDataflowFlowMatchType, void *>::iterator op_matches_iter;
  for (op_matches_iter = output_matches.begin();
       op_matches_iter != output_matches.end();
       op_matches_iter++) {
    ::operator delete((*op_matches_iter).second);
    pfc_log_trace("out_matches map entry is deleted");
  }
  output_matches.clear();
  }
  if (next.size() > 0) {
  vector<DataflowCmn *>::iterator next_iter;
  for (next_iter = next.begin();
      next_iter != next.end();
      next_iter++) {
    delete (*next_iter);
    pfc_log_trace("next vector entry is deleted");
  }
  next.clear();
  }
  if (addl_data != NULL) {
    delete addl_data;
    addl_data = NULL;
  }
  pfc_log_trace("DataflowCmn - Destructor call end");
}


/** sessReadDataflow
 * @Description : This function Reads dataflow val structures
 * using getresponse function
 * @param[in]   : sess - ipc client session where the response has to be added
 * getres_pos - read position
 * @return      : UncRespCode is returned 
 **/
int DataflowDetail::sessReadDataflow(ClientSession& sess,
                                     uint32_t& getres_pos) {
  int err = 0;

  pfc_log_debug("inside sessReadDataflow fn with getres_pos as %d",
                                         getres_pos);
  uint32_t match_count = 0;
  uint32_t action_count = 0;
  if (st_num_ == kidx_val_df_data_flow_cmn) {
    memset(df_common, 0, sizeof(val_df_data_flow_cmn_t));
    err |= sess.getResponse(getres_pos++, *df_common);
    pfc_log_debug("%s", DataflowCmn::get_string(*df_common).c_str());
    match_count = df_common->match_count;
    action_count = df_common->action_count;
  } else {
    memset(vtn_df_common, 0, sizeof(val_vtn_dataflow_cmn_t));
    err |= sess.getResponse(getres_pos++, *vtn_df_common);
    pfc_log_debug("%s", DataflowCmn::get_string(*vtn_df_common).c_str());
    match_count = vtn_df_common->match_count;
    action_count = vtn_df_common->action_count;
  }
  if (err != UNC_RC_SUCCESS) {
    return err;
  }
  val_df_flow_match_t val_flow_match_obj;
  memset(&val_flow_match_obj, 0, sizeof(val_flow_match_obj));
  for (uint32_t i = 0; i < match_count; i++) {
    err |= sess.getResponse(getres_pos++, val_flow_match_obj);
    if (err != UNC_RC_SUCCESS) {
      pfc_log_error("getresponse returned %d, for %d flow_match strt", err, i);
      return err;
    }

    pfc_log_debug("%s", DataflowCmn::get_string(val_flow_match_obj).c_str());
    switch (val_flow_match_obj.match_type) {
      case UNC_MATCH_IN_PORT:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_in_port_t, matches,
                                 UNC_MATCH_IN_PORT, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_DL_DST:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_dl_addr_t, matches,
                                 UNC_MATCH_DL_DST, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_DL_SRC:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_dl_addr_t, matches,
                                 UNC_MATCH_DL_SRC, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_DL_TYPE:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_dl_type_t, matches,
                                 UNC_MATCH_DL_TYPE, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_VLAN_ID:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_vlan_id_t, matches,
                                 UNC_MATCH_VLAN_ID, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_VLAN_PCP:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_vlan_pcp_t, matches,
                                 UNC_MATCH_VLAN_PCP, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_IP_TOS:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_ip_tos_t, matches,
                                 UNC_MATCH_IP_TOS, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_IP_PROTO:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_ip_proto_t, matches,
                                 UNC_MATCH_IP_PROTO, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_IPV4_SRC:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_ipv4_addr_t, matches,
                                 UNC_MATCH_IPV4_SRC, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_IPV4_DST:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_ipv4_addr_t, matches,
                                 UNC_MATCH_IPV4_DST, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_IPV6_SRC:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_ipv6_addr_t, matches,
                                 UNC_MATCH_IPV6_SRC, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_IPV6_DST:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_ipv6_addr_t, matches,
                                 UNC_MATCH_IPV6_DST, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_TP_SRC:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_tp_port_t, matches,
                                 UNC_MATCH_TP_SRC, sess, getres_pos, err);
        break;
      }
      case UNC_MATCH_TP_DST:
      {
        DATAFLOW_MATCHES_GETRESP(val_df_flow_match_tp_port_t, matches,
                                 UNC_MATCH_TP_DST, sess, getres_pos, err);
        break;
      }
      default:
      {
        pfc_log_warn("Invalid Match Type received %d"
            , val_flow_match_obj.match_type);
        return EBADF;
        break;
      }
    }
  }

  for (uint32_t i = 0; i < action_count; i++) {
    val_df_flow_action_t val_flow_action_obj;
    memset(&val_flow_action_obj, 0, sizeof(val_df_flow_action_t));
    err |= sess.getResponse(getres_pos++, val_flow_action_obj);
    if (err != UNC_RC_SUCCESS) {
      pfc_log_warn("getresponse returned %d, for %d flow_action strt", err, i);
      return err;
    }
    pfc_log_debug("%s", DataflowCmn::get_string(val_flow_action_obj).c_str());
    val_actions_vect_st *obj_actions_vect_st = new val_actions_vect_st;
    memset(obj_actions_vect_st, 0, sizeof(val_actions_vect_st));
    obj_actions_vect_st->action_type =
        (UncDataflowFlowActionType)val_flow_action_obj.action_type;
    switch (val_flow_action_obj.action_type) {
      case UNC_ACTION_OUTPUT:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_output_port_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_SET_ENQUEUE:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_enqueue_port_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_SET_DL_SRC:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_set_dl_addr_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_SET_DL_DST:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_set_dl_addr_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_SET_VLAN_ID:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_set_vlan_id_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_SET_VLAN_PCP:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_set_vlan_pcp_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_STRIP_VLAN:
      {
        getres_pos++;
        break;
      }
      case UNC_ACTION_SET_IPV4_SRC:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_set_ipv4_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_SET_IPV4_DST:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_set_ipv4_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_SET_IP_TOS:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_set_ip_tos_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_SET_TP_SRC:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_set_tp_port_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_SET_TP_DST:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_set_tp_port_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_SET_IPV6_SRC:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_set_ipv6_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      case UNC_ACTION_SET_IPV6_DST:
      {
        DATAFLOW_ACTION_GETRESP(val_df_flow_action_set_ipv6_t,
                                getres_pos, sess, obj_actions_vect_st, err);
        break;
      }
      default:
      {
        pfc_log_warn("Invalid Action Type received %d"
            , val_flow_action_obj.action_type);
        return EBADF;
        break;
      }
    }
    actions.push_back(obj_actions_vect_st);
  }

  pfc_log_trace("inside sessReadDataflow fn getting path info");
  if (st_num_ == kidx_val_df_data_flow_cmn) {
    for (uint32_t i = 0; i < df_common->path_info_count; i++) {
      val_df_data_flow_path_info_t *val_flow_path_obj
          = new val_df_data_flow_path_info_t;
      memset(val_flow_path_obj, 0, sizeof(val_df_data_flow_path_info_t));
      err |= sess.getResponse(getres_pos++, *val_flow_path_obj);
      if (err != UNC_RC_SUCCESS) {
        pfc_log_warn("getresponse returned %d, for %d path_info strt", err, i);
        return err;
      }
      pfc_log_debug("%d.%s", i,
        DataflowCmn::get_string(*val_flow_path_obj).c_str());
      path_infos.push_back(val_flow_path_obj);
    }
  } else {
    uint32_t orig_path_count = vtn_df_common->path_info_count;
    pfc_log_debug("original vtn path count from pfc is %d", orig_path_count);
    for (uint32_t i = 0; i < orig_path_count; i++) {
      val_vtn_dataflow_path_info_t *val_flow_path_obj
        = new val_vtn_dataflow_path_info_t;
      memset(val_flow_path_obj, 0, sizeof(val_vtn_dataflow_path_info_t));
      err |= sess.getResponse(getres_pos++, *val_flow_path_obj);
      if (err != 0)
        return err;
      pfc_log_debug("%d.%s", i,
        DataflowCmn::get_string(*val_flow_path_obj).c_str());
     if (val_flow_path_obj->vlink_flag ==  UPLL_DATAFLOW_PATH_VLINK_NOT_EXISTS_)
        is_flow_redirect  =  true;
     if (val_flow_path_obj->status ==  UPLL_DATAFLOW_PATH_STATUS_DROP)
        is_flow_drop  =  true;
     vtn_path_infos.push_back(val_flow_path_obj);
    }
  }
  pfc_log_debug("Returned value is %d", err);
  return err;
}

/**  CompareDataflow
 * * @Description : This function compare dataflows
 * * @param[in]   : vector 
 * * @return      : true or false
*/ 
bool DataflowCmn::CompareDataflow(DataflowCmn *otherflow) {
  if ((strcmp((const char*)otherflow->df_segment->df_common->controller_name
          , (const char*)this->df_segment->df_common->controller_name) != 0)
    || (strcmp((const char*)otherflow->df_segment->df_common->egress_switch_id
          , (const char*)this->df_segment->df_common->egress_switch_id) != 0)
    || (strcmp((const char*)otherflow->df_segment->df_common->out_port
          , (const char*)this->df_segment->df_common->out_port) != 0)) {
    pfc_log_info("Controller Name, switch id, Port id are not matched");
    return false;
  }
  return true;
}
/**  Compare 
 * * @Description : This function compare dataflows
 * * @param[in]   : vector 
 * * @return      : true or false
*/ 
bool DataflowCmn::Compare(const key_dataflow_t& lhs,
                                   const key_dataflow_t& rhs) {
  int ret = strcmp((const char*)lhs.controller_name, (const char*)
                                             rhs.controller_name);
  if ( ret != 0 ) return false;
  ret = strcmp((const char*)lhs.switch_id, (const char*)rhs.switch_id);
  if ( ret != 0 ) return false;
  ret = strcmp((const char*)lhs.port_id, (const char*)rhs.port_id);
  if ( ret != 0 ) return false;
  if (lhs.vlan_id != rhs.vlan_id) return false;
  ret = memcmp(lhs.src_mac_address, rhs.src_mac_address,
                                             sizeof(lhs.src_mac_address));
  if ( ret != 0 ) return false;
  return true;
}
/**  CompareVtnDataflow 
 * * @Description : This function compare dataflows
 * * @param[in]   : vector 
 * * @return      : true or false
*/ 
bool DataflowCmn::CompareVtnDataflow(DataflowCmn *otherflow) {
  if ((strcmp((const char*)otherflow->df_segment->vtn_df_common->controller_id
          , (const char*)this->df_segment->vtn_df_common->controller_id) != 0)
    || (strcmp((const char*)otherflow->df_segment->vtn_df_common->
                                                            egress_switch_id
          , (const char*)this->df_segment->vtn_df_common->egress_switch_id)
                                                            != 0)
    || (strcmp((const char*)otherflow->df_segment->vtn_df_common->
                                                            egress_vinterface
          , (const char*)this->df_segment->vtn_df_common->egress_vinterface)
                                                            != 0)
    || (strcmp((const char*)otherflow->df_segment->vtn_df_common->
                                                            egress_port_id
          , (const char*)this->df_segment->vtn_df_common->egress_port_id)
                                                            != 0)) {
    pfc_log_info("Controller Name, switch id, Port id are not matched");
    return false;
  }
  return true;
}


/** operator 
 * * @Description : This function compare dataflows
 * * @param[in]   : vector 
 * * @return      : true or false
*/ 
bool DataflowCmn::Compare(const key_vtn_ctrlr_dataflow& lhs, const key_vtn_ctrlr_dataflow& rhs) {
  int ret = strcmp((const char*)lhs.ctrlr, (const char*)rhs.ctrlr);
  if( ret != 0 ) return false;
  ret = strcmp((const char*)lhs.domain, (const char*)rhs.domain);
  if( ret != 0 ) return false;
  ret = strcmp((const char*)lhs.vtn_key.vtn_name, (const char*)rhs.vtn_key.vtn_name);
  if( ret != 0 ) return false;
  if(lhs.vlanid != rhs.vlanid) return false;
  ret = memcmp(lhs.src_mac_address, rhs.src_mac_address, sizeof(lhs.src_mac_address));
  if( ret != 0 ) return false;
  return true;
}

bool KeyDataflowCmp::operator()(const key_dataflow_t& lhs, const key_dataflow_t& rhs) {
  pfc_log_debug("KeyDataflowCmp called \n%s\n%s", DataflowCmn::get_string(lhs).c_str(), DataflowCmn::get_string(rhs).c_str());
  int ret = strcmp((const char*)lhs.controller_name, (const char*)rhs.controller_name);
  if ( ret < 0 ) return true;
  ret = strcmp((const char*)lhs.switch_id, (const char*)rhs.switch_id);
  if ( ret < 0 ) return true;
  ret = strcmp((const char*)lhs.port_id, (const char*)rhs.port_id);
  if ( ret < 0 ) return true;
  if (lhs.vlan_id < rhs.vlan_id) return true;
  ret = memcmp(lhs.src_mac_address, rhs.src_mac_address, sizeof(lhs.src_mac_address));
  if ( ret < 0 ) return true;
  return false;
}

/**  CompareDataflow 
 * * @Description : This function compare dataflows
 * * @param[in]   : vector 
 * * @return      : true or false
*/ 
bool KeyVtnDataflowCmp::operator()(const key_vtn_ctrlr_dataflow &lhs, const key_vtn_ctrlr_dataflow &rhs) {
  int ret =  strcmp((const char *)lhs.vtn_key.vtn_name, (const char*)rhs.vtn_key.vtn_name);
  if (ret < 0) return true; 
  ret =  strcmp((const char *)lhs.ctrlr, (const char *)rhs.ctrlr);
  if (ret < 0) return true; 
  ret =  strcmp((const char *)lhs.domain, (const char *)rhs.domain);
  if (ret < 0) return true; 
  if ( lhs.vlanid < rhs.vlanid) return true;
  ret = memcmp(lhs.src_mac_address, rhs.src_mac_address, sizeof(lhs.src_mac_address));
  if ( ret < 0 ) return true;
  return false;
}


/** sessOutDataflow
 * @Description : This function writes dataflow val structures
 * using addoutput function
 * @param[in]   : sess - ipc server session where the response has to be added
 * putres_pos - write position
 * @return      : UncRespCode is returned 
 **/
int DataflowCmn::sessOutDataflow(ServerSession& sess, int& putresp_pos) {
  if (df_segment == NULL) {
    pfc_log_debug("df_segment is NULL in DataflowCmn::sessReadDataflow");
    return -1;
  }
  int err = 0;
  map<UncDataflowFlowMatchType, void *>::iterator match_iter;

  pfc_log_trace("inside sessOutDataflow fn with getres_pos as %d", putresp_pos);

  if (df_segment->st_num_ == kidx_val_df_data_flow_cmn) {
    df_segment->df_common->path_info_count = df_segment->path_infos.size();
    df_segment->df_common->match_count = df_segment->matches.size();
    df_segment->df_common->action_count = df_segment->actions.size();
  } else {
    df_segment->vtn_df_common->path_info_count = df_segment->vtn_path_infos.size();
    df_segment->vtn_df_common->match_count = df_segment->matches.size();
    df_segment->vtn_df_common->action_count = df_segment->actions.size();
  }

  if (df_segment->st_num_ == kidx_val_df_data_flow_cmn) {
    pfc_log_debug("%d. %s", putresp_pos, get_string(*df_segment->df_common).c_str());
    putresp_pos++;
    err |= sess.addOutput(*df_segment->df_common);
  } else {
    // LOGICAL
    pfc_log_debug("%d. %s", putresp_pos, get_string(*df_segment->vtn_df_common).c_str());
    putresp_pos++;
    err |= sess.addOutput(*df_segment->vtn_df_common);
  }

  for (match_iter = df_segment->matches.begin(); match_iter != df_segment->matches.end(); match_iter++) {
    val_df_flow_match_t val_match;
    val_match.match_type = (UncDataflowFlowMatchType)(*match_iter).first;
    pfc_log_debug("%d. %s", putresp_pos, get_string(val_match).c_str());
    putresp_pos++;
    err |= sess.addOutput(val_match);
    if (err != UNC_RC_SUCCESS) {
      pfc_log_trace("addoutput status %d, failed to sent %d flow_match strt ",
          err, putresp_pos);
      return err;
    }

    switch (val_match.match_type) {
      case UNC_MATCH_IN_PORT:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_in_port_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_DL_SRC:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_dl_addr_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_DL_DST:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_dl_addr_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_DL_TYPE:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_dl_type_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_VLAN_ID:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_vlan_id_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_VLAN_PCP:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_vlan_pcp_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_IP_TOS:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_ip_tos_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_IP_PROTO:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_ip_proto_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_IPV4_SRC:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_ipv4_addr_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_IPV4_DST:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_ipv4_addr_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_IPV6_SRC:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_ipv6_addr_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_IPV6_DST:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_ipv6_addr_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_TP_SRC:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_tp_port_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      case UNC_MATCH_TP_DST:
      {
        DATAFLOW_MATCHES_ADDOUTPUT(val_df_flow_match_tp_port_t,
                                   *match_iter, sess, putresp_pos, err);
        break;
      }
      default:
      {
        pfc_log_debug("Invalid Match Type received %d", val_match.match_type);
        break;
      }
    }
  }

  pfc_log_trace("inside sessOutDataflow fn sending actions after match struct sessout.");
  for (uint32_t i = 0; i < df_segment->actions.size(); i++) {
    val_actions_vect_st *obj_actions_vect_st = reinterpret_cast<val_actions_vect_st *>(df_segment->actions[i]);
    val_df_flow_action_t val_action;
    // memset(&val_action, 0, sizeof(val_df_flow_action_t));
    val_action.action_type = (UncDataflowFlowActionType)df_segment->actions[i]->action_type;
    pfc_log_debug("%d. %s", putresp_pos, get_string(val_action).c_str());
    putresp_pos++;
    err |= sess.addOutput(val_action);
    if (err != UNC_RC_SUCCESS) {
      pfc_log_trace("addoutput status %d, failed to sent %d flow_action strt",
          err, putresp_pos);
      return err;
    }

    switch (val_action.action_type) {
      case UNC_ACTION_OUTPUT:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_output_port_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_SET_ENQUEUE:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_enqueue_port_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_SET_DL_SRC:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_dl_addr_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_SET_DL_DST:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_dl_addr_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_SET_VLAN_ID:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_vlan_id_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_SET_VLAN_PCP:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_vlan_pcp_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_STRIP_VLAN:
      {
        putresp_pos++;
        err |= sess.addOutput();  // Send NULL struct
        break;
      }
      case UNC_ACTION_SET_IPV4_SRC:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_ipv4_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_SET_IPV4_DST:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_ipv4_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_SET_IP_TOS:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_ip_tos_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_SET_TP_SRC:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_tp_port_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_SET_TP_DST:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_tp_port_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_SET_IPV6_SRC:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_ipv6_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      case UNC_ACTION_SET_IPV6_DST:
      {
        DATAFLOW_ACTION_ADDOUTPUT(val_df_flow_action_set_ipv6_t,
                                  obj_actions_vect_st, sess, putresp_pos, err);
        break;
      }
      default:
      {
        pfc_log_debug("Invalid action Type in action map %d",
            val_action.action_type);
        break;
      }
    }
  }
  if (df_segment->st_num_ == kidx_val_df_data_flow_cmn) {
    for (uint32_t i = 0; i < df_segment->path_infos.size(); i++) {
      pfc_log_debug("%d. %s", putresp_pos, get_string(*df_segment->path_infos[i]).c_str());
      putresp_pos++;
      err |= sess.addOutput(*df_segment->path_infos[i]);
      if (err != UNC_RC_SUCCESS) {
        pfc_log_trace("addoutput status %d, failed to sent %d path_info struct",
            err, putresp_pos);
        return err;
      }
    }
  } else {
    // LOGICAL
    for (uint32_t i = 0; i < df_segment->vtn_path_infos.size(); i++) {
      pfc_log_debug("%d. %s", putresp_pos, get_string(*df_segment->vtn_path_infos[i]).c_str());
      putresp_pos++;
      err |= sess.addOutput(*df_segment->vtn_path_infos[i]);
      if (err != UNC_RC_SUCCESS) {
        pfc_log_trace("addoutput status %d, failed to sent %d vtn_path_info struct",
            err, putresp_pos);
        return err;
      }
    }
  }

  pfc_log_debug("Return value %d", err);
  return err;
}

/** check_match_condition
 * * @Description : This function check the matches and output_matches 
 * * @param[in]   : prev_output_matches - output_matches
 * * @return      : bool 
 **/
bool DataflowCmn::check_match_condition(map<UncDataflowFlowMatchType,
                                 void *> prev_output_matches) {
  bool ret_value = true;
  map<UncDataflowFlowMatchType, void *>::iterator iter_out_match =
        prev_output_matches.begin();
  pfc_log_debug("Size of prev_output_matches:%" PFC_PFMT_SIZE_T,
               prev_output_matches.size());
  pfc_log_debug("Size of curr matches:%" PFC_PFMT_SIZE_T, df_segment->matches.size());
  while (iter_out_match != prev_output_matches.end()) {
    switch (iter_out_match->first) {
      case UNC_MATCH_IN_PORT:
        break;
      case UNC_MATCH_DL_SRC:
        break;
      case UNC_MATCH_DL_DST:
      {
        pfc_log_debug("check match for mac src/dst %d", iter_out_match->first);
        map<UncDataflowFlowMatchType, void *>::iterator iter_mch = df_segment->matches.find(iter_out_match->first);
        if (iter_mch !=  df_segment->matches.end()) {
          val_df_flow_match_dl_addr_t *curr =
              reinterpret_cast<val_df_flow_match_dl_addr_t *>(iter_mch->second);
          val_df_flow_match_dl_addr_t *prev = reinterpret_cast<val_df_flow_match_dl_addr_t *>((*iter_out_match).second);
          if (prev == NULL) {
            //(strict or masked) and ANY
            break;
          }
          if (curr->v_mask == UNC_MATCH_MASK_VALID) {
            if (prev->v_mask == UNC_MATCH_MASK_VALID) {
              // 9.RANGE - RANGE
              if (!DataflowUtil::checkMacAddress(curr->dl_addr, curr->dl_addr_mask, prev->dl_addr, prev->dl_addr_mask)) {
                pfc_log_debug("check match failed for mac addr range1 range2");
                ret_value &= false;
              }
            } else {
              // 3.STRICT - RANGE
              if (!DataflowUtil::checkMacAddress(curr->dl_addr, curr->dl_addr_mask, prev->dl_addr)) {
                pfc_log_debug("check match failed for mac addr strict1 range2");
                ret_value &= false;
              }
            }
          } else {
            if (prev->v_mask == UNC_MATCH_MASK_VALID) {
              // 7.RANGE - STRICT
              if (!DataflowUtil::checkMacAddress(prev->dl_addr, prev->dl_addr_mask, curr->dl_addr)) {
                pfc_log_debug("check match failed for mac addr range1 strict2");
                ret_value &= false;
              }
            } else {
              // 1.STRICT - STRICT
              if (memcmp(curr->dl_addr, prev->dl_addr, sizeof(prev->dl_addr)) != 0) {
                pfc_log_debug("check match failed for mac addr strict1 strict2");
                ret_value &= false;
              }
            }
          }
        } // else {
          // 2.STRICT - ANY //8.RANGE - ANY // This is OK. Do nothing.
        // }
        break;
      }
      case UNC_MATCH_DL_TYPE:
      {
        map<UncDataflowFlowMatchType, void *>::iterator iter_mch = df_segment->matches.find(iter_out_match->first);
        if (iter_mch !=  df_segment->matches.end()) {
          val_df_flow_match_dl_type_t *curr =
              reinterpret_cast<val_df_flow_match_dl_type_t *>(iter_mch->second);
          val_df_flow_match_dl_type_t *prev = reinterpret_cast<val_df_flow_match_dl_type_t *>((*iter_out_match).second);
          if (prev == NULL) {
            //(strict or masked) and ANY
            break;
          }
          // 1.STRICT - STRICT
          if (curr->dl_type != prev->dl_type) {
            pfc_log_info("check match failed for dl_type");
            ret_value &= false;
          }
        }
        break;
      }
      case UNC_MATCH_VLAN_ID:
        break;
      case UNC_MATCH_VLAN_PCP:
      {
        map<UncDataflowFlowMatchType, void *>::iterator iter_mch = df_segment->matches.find(iter_out_match->first);
        if (iter_mch !=  df_segment->matches.end()) {
          val_df_flow_match_vlan_pcp_t *curr =
              reinterpret_cast<val_df_flow_match_vlan_pcp_t *>(iter_mch->second);
          val_df_flow_match_vlan_pcp_t *prev = reinterpret_cast<val_df_flow_match_vlan_pcp_t *>((*iter_out_match).second);
          if (prev == NULL) {
            //(strict or masked) and ANY
            break;
          }
          // 1.STRICT - STRICT
          if (curr->vlan_pcp != prev->vlan_pcp) {
            pfc_log_info("check match failed for vlan pcp");
            ret_value &= false;
          }
        }
        break;
      }
      case UNC_MATCH_IP_TOS:
      {
        map<UncDataflowFlowMatchType, void *>::iterator iter_mch = df_segment->matches.find(iter_out_match->first);
        if (iter_mch !=  df_segment->matches.end()) {
          val_df_flow_match_ip_tos_t *curr =
              reinterpret_cast<val_df_flow_match_ip_tos_t *>(iter_mch->second);
          val_df_flow_match_ip_tos_t *prev = reinterpret_cast<val_df_flow_match_ip_tos_t *>((*iter_out_match).second);
          if (prev == NULL) {
            //(strict or masked) and ANY
            break;
          }
          // 1.STRICT - STRICT
          if (curr->ip_tos != prev->ip_tos) {
            pfc_log_info("check match failed for ip tos");
            ret_value &= false;
          }
        }
        break;
      }
      case UNC_MATCH_IP_PROTO:
      {
        map<UncDataflowFlowMatchType, void *>::iterator iter_mch = df_segment->matches.find(iter_out_match->first);
        if (iter_mch !=  df_segment->matches.end()) {
          val_df_flow_match_ip_proto_t *curr =
              reinterpret_cast<val_df_flow_match_ip_proto_t *>(iter_mch->second);
          val_df_flow_match_ip_proto_t *prev = reinterpret_cast<val_df_flow_match_ip_proto_t *>((*iter_out_match).second);
          if (prev == NULL) {
            //(strict or masked) and ANY
            break;
          }
          // 1.STRICT - STRICT
          if (curr->ip_proto != prev->ip_proto) {
            pfc_log_info("check match failed for ip_proto");
            ret_value &= false;
          }
        }
        break;
      }
      case UNC_MATCH_IPV4_SRC:
      case UNC_MATCH_IPV4_DST:
      {
        pfc_log_debug("check match for ipv4 src/dst %d", iter_out_match->first);
        map<UncDataflowFlowMatchType, void *>::iterator iter_mch = df_segment->matches.find(iter_out_match->first);
        if (iter_mch !=  df_segment->matches.end()) {
          val_df_flow_match_ipv4_addr_t *curr =
              reinterpret_cast<val_df_flow_match_ipv4_addr_t *>(iter_mch->second);
          val_df_flow_match_ipv4_addr_t *prev = reinterpret_cast<val_df_flow_match_ipv4_addr_t *>((*iter_out_match).second);
          if (prev == NULL) {
            //(strict or masked) and ANY
            break;
          }
          if (curr->v_mask == UNC_MATCH_MASK_VALID) {
            if (prev->v_mask == UNC_MATCH_MASK_VALID) {
              // 9.RANGE - RANGE
              if (!DataflowUtil::checkIPv4Address(curr->ipv4_addr, curr->ipv4_addr_mask, prev->ipv4_addr, prev->ipv4_addr_mask)) {
                pfc_log_debug("check match failed for ipv4 addr range1 range2");
                ret_value &= false;
              }
            } else {
              // 3.STRICT - RANGE
              if (!DataflowUtil::checkIPv4Address(curr->ipv4_addr, curr->ipv4_addr_mask, prev->ipv4_addr)) {
                pfc_log_debug("check match failed for ipv4 addr strict1 range2");
                ret_value &= false;
              }
            }
          } else {
            if (prev->v_mask == UNC_MATCH_MASK_VALID) {
              // 7.RANGE - STRICT
              if (!DataflowUtil::checkIPv4Address(prev->ipv4_addr, prev->ipv4_addr_mask, curr->ipv4_addr)) {
                pfc_log_debug("check match failed for ipv4 addr range1 strict2");
                ret_value &= false;
              }
            } else {
              // 1.STRICT - STRICT
              if (curr->ipv4_addr.s_addr != prev->ipv4_addr.s_addr) {
                pfc_log_debug("check match failed for ipv4 addr strict1 strict2");
                ret_value &= false;
              }
            }
          }
        } // else {
          // 2.STRICT - ANY  //8.RANGE - ANY  // This is OK. Do nothing.
        // }
        break;
      }
      case UNC_MATCH_IPV6_SRC:
      case UNC_MATCH_IPV6_DST:
      {
        pfc_log_debug("check match for ipv6 src/dst %d", iter_out_match->first);
        map<UncDataflowFlowMatchType, void *>::iterator iter_mch = df_segment->matches.find(iter_out_match->first);
        if (iter_mch !=  df_segment->matches.end()) {
          val_df_flow_match_ipv6_addr_t *curr =
              reinterpret_cast<val_df_flow_match_ipv6_addr_t *>(iter_mch->second);
          val_df_flow_match_ipv6_addr_t *prev = reinterpret_cast<val_df_flow_match_ipv6_addr_t *>((*iter_out_match).second);
          if (prev == NULL) {
            //(strict or masked) and ANY
            break;
          }
          if (curr->v_mask == UNC_MATCH_MASK_VALID) {
            if (prev->v_mask == UNC_MATCH_MASK_VALID) {
              // 9.RANGE - RANGE
              if (!DataflowUtil::checkIPv6Address(curr->ipv6_addr, curr->ipv6_addr_mask, prev->ipv6_addr, prev->ipv6_addr_mask)) {
                pfc_log_debug("check match failed for ipv6 addr range1 range2");
                ret_value &= false;
              }
            } else {
              // 3.STRICT - RANGE
              if (!DataflowUtil::checkIPv6Address(curr->ipv6_addr, curr->ipv6_addr_mask, prev->ipv6_addr)) {
                pfc_log_debug("check match failed for ipv6 addr strict1 range2");
                ret_value &= false;
              }
            }
          } else {
            if (prev->v_mask == UNC_MATCH_MASK_VALID) {
              // 7.RANGE - STRICT
              if (!DataflowUtil::checkIPv6Address(prev->ipv6_addr, prev->ipv6_addr_mask, curr->ipv6_addr)) {
                pfc_log_debug("check match failed for ipv6 addr range1 strict2");
                ret_value &= false;
              }
            } else {
              // 1.STRICT - STRICT
              if (memcmp(curr->ipv6_addr.s6_addr, prev->ipv6_addr.s6_addr, sizeof(prev->ipv6_addr.s6_addr)) != 0) {
                pfc_log_debug("check match failed for ipv6 addr strict1 strict2");
                ret_value &= false;
              }
            }
          }
        } // else {
          // 2.STRICT - ANY  //8.RANGE - ANY  // This is OK. Do nothing.
        // }
        break;
      }
      case UNC_MATCH_TP_SRC:
      case UNC_MATCH_TP_DST:
      {
        pfc_log_debug("check match for tp port src/dst %d", iter_out_match->first);
        map<UncDataflowFlowMatchType, void *>::iterator iter_mch = df_segment->matches.find(iter_out_match->first);
        if (iter_mch !=  df_segment->matches.end()) {
          val_df_flow_match_tp_port_t *curr =
              reinterpret_cast<val_df_flow_match_tp_port_t *>(iter_mch->second);
          val_df_flow_match_tp_port_t *prev = reinterpret_cast<val_df_flow_match_tp_port_t *>((*iter_out_match).second);
          if (prev == NULL) {
            //(strict or masked) and ANY
            break;
          }
          if (curr->v_mask == UNC_MATCH_MASK_VALID) {
            if (prev->v_mask == UNC_MATCH_MASK_VALID) {
              // 9.RANGE - RANGE
              if (!DataflowUtil::checkIPv4Address(curr->tp_port, curr->tp_port_mask, prev->tp_port, prev->tp_port_mask)) {
                pfc_log_debug("check match failed for tp port range1 range2");
                ret_value &= false;
              }
            } else {
              // 3.STRICT - RANGE
              if (!DataflowUtil::checkIPv4Address(curr->tp_port, curr->tp_port_mask, prev->tp_port)) {
                pfc_log_debug("check match failed for tp port strict1 range2");
                ret_value &= false;
              }
            }
          } else {
            if (prev->v_mask == UNC_MATCH_MASK_VALID) {
              // 7.RANGE - STRICT
              if (!DataflowUtil::checkIPv4Address(prev->tp_port, prev->tp_port_mask, curr->tp_port)) {
                pfc_log_debug("check match failed for tp port range1 strict2");
                ret_value &= false;
              }
            } else {
              // 1.STRICT - STRICT
              if (curr->tp_port != prev->tp_port) {
                pfc_log_debug("check match failed for tp port strict1 strict2");
                ret_value &= false;
              }
            }
          }
        } // else {
          // 2.STRICT - ANY  //8.RANGE - ANY  // This is OK. Do nothing.
        // }
        break;
      }
      default:
        pfc_log_warn("check_match_condition Ignoring %d ", iter_out_match->first);
	break;
    }
    if (ret_value == false)
      break;
    iter_out_match++;
  }
  pfc_log_debug("check_match_condition returns %d", ret_value);
  return ret_value;
}

/** apply_action 
 * * @Description : This function Apply 'actions' to 'matches'
 * * and fill 'output_matches'
 * * @param[in]   : None
 * * @return      : None 
 **/
void DataflowCmn::apply_action() {
  if (action_applied == true)
    return;
  action_applied = true;
  // DEEP COPY matches to output_matches;
  deep_copy();
  if(df_segment->actions.size() > 0) {
  map<UncDataflowFlowMatchType, void *>::iterator match_iter;
  vector<val_actions_vect_st *>::iterator iter_action = df_segment->actions.begin();
  for (; iter_action != df_segment->actions.end(); iter_action++) {
    val_actions_vect_st *obj_action_vect = (*iter_action);
    pfc_log_debug("apply_action action_type=%d", obj_action_vect->action_type);
    switch (obj_action_vect->action_type) {
      case UNC_ACTION_OUTPUT:
      case UNC_ACTION_SET_ENQUEUE:
        break;
      case UNC_ACTION_SET_DL_SRC:
      {
        val_df_flow_action_set_dl_addr_t *act_st = reinterpret_cast<val_df_flow_action_set_dl_addr_t *>(obj_action_vect->action_st_ptr);
        match_iter = output_matches.find(UNC_MATCH_DL_SRC);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_dl_addr_t *out_match_st = reinterpret_cast<val_df_flow_match_dl_addr_t *>((*match_iter).second);
          if (memcmp((const char*)out_match_st->dl_addr, (const char*)act_st->dl_addr,
                     sizeof(act_st->dl_addr)) != 0) {
            is_vlan_src_mac_changed_ = true;
          }
          memcpy(out_match_st->dl_addr, act_st->dl_addr, sizeof(act_st->dl_addr));
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
        } else {
          val_df_flow_match_dl_addr_t *out_match_st = new val_df_flow_match_dl_addr_t;
          memset(out_match_st, '\0', sizeof(val_df_flow_match_dl_addr_t));
          memcpy(out_match_st->dl_addr, act_st->dl_addr, sizeof(act_st->dl_addr));
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
          output_matches[UNC_MATCH_DL_SRC] = out_match_st;
        }
        break;
      }
      case UNC_ACTION_SET_DL_DST:
      {
        val_df_flow_action_set_dl_addr_t *act_st = reinterpret_cast<val_df_flow_action_set_dl_addr_t *>(obj_action_vect->action_st_ptr);
        match_iter = output_matches.find(UNC_MATCH_DL_DST);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_dl_addr_t *out_match_st = reinterpret_cast<val_df_flow_match_dl_addr_t *>((*match_iter).second);
          memcpy(out_match_st->dl_addr, act_st->dl_addr, sizeof(act_st->dl_addr));
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
        } else {
          val_df_flow_match_dl_addr_t *out_match_st = new val_df_flow_match_dl_addr_t;
          memset(out_match_st, '\0', sizeof(val_df_flow_match_dl_addr_t));
          memcpy(out_match_st->dl_addr, act_st->dl_addr, sizeof(act_st->dl_addr));
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
          output_matches[UNC_MATCH_DL_DST] = out_match_st;
        }
        break;
      }
      case UNC_ACTION_SET_VLAN_ID:
      {
        val_df_flow_action_set_vlan_id_t *act_st = reinterpret_cast<val_df_flow_action_set_vlan_id_t *>(obj_action_vect->action_st_ptr);
        match_iter = output_matches.find(UNC_MATCH_VLAN_ID);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_vlan_id_t *out_match_st = reinterpret_cast<val_df_flow_match_vlan_id_t *>((*match_iter).second);
          if (out_match_st->vlan_id != act_st->vlan_id) {
            is_vlan_src_mac_changed_ = true;
          }
          out_match_st->vlan_id = act_st->vlan_id;
        } else {
          val_df_flow_match_vlan_id_t *out_match_st = new val_df_flow_match_vlan_id_t;
          out_match_st->vlan_id = act_st->vlan_id;
          output_matches[UNC_MATCH_VLAN_ID] = out_match_st;
        }
        break;
      }
      case UNC_ACTION_SET_VLAN_PCP:
      {
        val_df_flow_action_set_vlan_pcp_t *act_st = reinterpret_cast<val_df_flow_action_set_vlan_pcp_t *>(obj_action_vect->action_st_ptr);
        match_iter = output_matches.find(UNC_MATCH_VLAN_PCP);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_vlan_pcp_t *out_match_st = reinterpret_cast<val_df_flow_match_vlan_pcp_t *>((*match_iter).second);
          out_match_st->vlan_pcp = act_st->vlan_pcp;
        } else {
          val_df_flow_match_vlan_pcp_t *out_match_st = new val_df_flow_match_vlan_pcp_t;
          out_match_st->vlan_pcp = act_st->vlan_pcp;
          output_matches[UNC_MATCH_VLAN_PCP] = out_match_st;
        }
        break;
      }
      case UNC_ACTION_STRIP_VLAN:
      {
        match_iter = output_matches.find(UNC_MATCH_VLAN_ID);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_vlan_id_t *out_match_st = reinterpret_cast<val_df_flow_match_vlan_id_t *>((*match_iter).second);
          if (out_match_st->vlan_id != 0xFFFF)
            is_vlan_src_mac_changed_ = true;
          out_match_st->vlan_id = 0xFFFF;
        } else {
          val_df_flow_match_vlan_id_t *out_match_st = new val_df_flow_match_vlan_id_t;
          out_match_st->vlan_id = 0xFFFF;
          output_matches[UNC_MATCH_VLAN_ID] = out_match_st;
        }
        break;
      }
      case UNC_ACTION_SET_IPV4_SRC:
      {
        val_df_flow_action_set_ipv4_t *act_st = reinterpret_cast<val_df_flow_action_set_ipv4_t *>(obj_action_vect->action_st_ptr);
        match_iter = output_matches.find(UNC_MATCH_IPV4_SRC);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_ipv4_addr_t *out_match_st = reinterpret_cast<val_df_flow_match_ipv4_addr_t *>((*match_iter).second);
          out_match_st->ipv4_addr.s_addr = act_st->ipv4_addr.s_addr;
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
        } else {
          val_df_flow_match_ipv4_addr_t *out_match_st = new val_df_flow_match_ipv4_addr_t;
          memset(out_match_st, '\0', sizeof(val_df_flow_match_ipv4_addr_t));
          out_match_st->ipv4_addr.s_addr = act_st->ipv4_addr.s_addr;
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
          output_matches[UNC_MATCH_IPV4_SRC] = out_match_st;
        }
        break;
      }
      case UNC_ACTION_SET_IPV4_DST:
      {
        val_df_flow_action_set_ipv4_t *act_st = reinterpret_cast<val_df_flow_action_set_ipv4_t *>(obj_action_vect->action_st_ptr);
        match_iter = output_matches.find(UNC_MATCH_IPV4_DST);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_ipv4_addr_t *out_match_st = reinterpret_cast<val_df_flow_match_ipv4_addr_t *>((*match_iter).second);
          out_match_st->ipv4_addr.s_addr = act_st->ipv4_addr.s_addr;
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
        } else {
          val_df_flow_match_ipv4_addr_t *out_match_st = new val_df_flow_match_ipv4_addr_t;
          memset(out_match_st, '\0', sizeof(val_df_flow_match_ipv4_addr_t));
          out_match_st->ipv4_addr.s_addr = act_st->ipv4_addr.s_addr;
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
          output_matches[UNC_MATCH_IPV4_DST] = out_match_st;
        }
        break;
      }
      case UNC_ACTION_SET_IP_TOS:
      {
        val_df_flow_action_set_ip_tos_t *act_st = reinterpret_cast<val_df_flow_action_set_ip_tos_t *>(obj_action_vect->action_st_ptr);
        match_iter = output_matches.find(UNC_MATCH_IP_TOS);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_ip_tos_t *out_match_st = reinterpret_cast<val_df_flow_match_ip_tos_t *>((*match_iter).second);
          out_match_st->ip_tos = act_st->ip_tos;
        } else {
          val_df_flow_match_ip_tos_t *out_match_st = new val_df_flow_match_ip_tos_t;
          out_match_st->ip_tos = act_st->ip_tos;
          output_matches[UNC_MATCH_IP_TOS] = out_match_st;
        }
        break;
      }
      case UNC_ACTION_SET_TP_SRC:
      {
        val_df_flow_action_set_tp_port_t *act_st = reinterpret_cast<val_df_flow_action_set_tp_port_t *>(obj_action_vect->action_st_ptr);
        match_iter = output_matches.find(UNC_MATCH_TP_SRC);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_tp_port_t *out_match_st = reinterpret_cast<val_df_flow_match_tp_port_t *>((*match_iter).second);
          out_match_st->tp_port = act_st->tp_port;
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
        } else {
          val_df_flow_match_tp_port_t *out_match_st = new val_df_flow_match_tp_port_t;
          out_match_st->tp_port = act_st->tp_port;
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
          output_matches[UNC_MATCH_TP_SRC] = out_match_st;
        }
        break;
      }
      case UNC_ACTION_SET_TP_DST:
      {
        val_df_flow_action_set_tp_port_t *act_st = reinterpret_cast<val_df_flow_action_set_tp_port_t *>(obj_action_vect->action_st_ptr);
        match_iter = output_matches.find(UNC_MATCH_TP_DST);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_tp_port_t *out_match_st = reinterpret_cast<val_df_flow_match_tp_port_t *>((*match_iter).second);
          out_match_st->tp_port = act_st->tp_port;
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
        } else {
          val_df_flow_match_tp_port_t *out_match_st = new val_df_flow_match_tp_port_t;
          out_match_st->tp_port = act_st->tp_port;
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
          output_matches[UNC_MATCH_TP_DST] = out_match_st;
        }
        break;
      }
      case UNC_ACTION_SET_IPV6_SRC:
      {
        val_df_flow_action_set_ipv6_t *act_st = reinterpret_cast<val_df_flow_action_set_ipv6_t *>(obj_action_vect->action_st_ptr);
        match_iter = output_matches.find(UNC_MATCH_IPV6_SRC);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_ipv6_addr_t *out_match_st = reinterpret_cast<val_df_flow_match_ipv6_addr_t *>((*match_iter).second);
          memcpy(out_match_st->ipv6_addr.s6_addr, act_st->ipv6_addr.s6_addr, sizeof(act_st->ipv6_addr.s6_addr));
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
        } else {
          val_df_flow_match_ipv6_addr_t *out_match_st = new val_df_flow_match_ipv6_addr_t;
          memset(out_match_st, '\0', sizeof(val_df_flow_match_ipv6_addr_t));
          memcpy(out_match_st->ipv6_addr.s6_addr, act_st->ipv6_addr.s6_addr, sizeof(act_st->ipv6_addr.s6_addr));
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
          output_matches[UNC_MATCH_IPV6_SRC] = out_match_st;
        }
        break;
      }
      case UNC_ACTION_SET_IPV6_DST:
      {
        val_df_flow_action_set_ipv6_t *act_st = reinterpret_cast<val_df_flow_action_set_ipv6_t *>(obj_action_vect->action_st_ptr);
        match_iter = output_matches.find(UNC_MATCH_IPV6_DST);
        if (match_iter != output_matches.end()) {
          val_df_flow_match_ipv6_addr_t *out_match_st = reinterpret_cast<val_df_flow_match_ipv6_addr_t *>((*match_iter).second);
          memcpy(out_match_st->ipv6_addr.s6_addr, act_st->ipv6_addr.s6_addr, sizeof(act_st->ipv6_addr.s6_addr));
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
        } else {
          val_df_flow_match_ipv6_addr_t *out_match_st = new val_df_flow_match_ipv6_addr_t;
          memset(out_match_st, '\0', sizeof(val_df_flow_match_ipv6_addr_t));
          memcpy(out_match_st->ipv6_addr.s6_addr, act_st->ipv6_addr.s6_addr, sizeof(act_st->ipv6_addr.s6_addr));
          out_match_st->v_mask = UNC_MATCH_MASK_INVALID;
          output_matches[UNC_MATCH_IPV6_DST] = out_match_st;
        }
        break;
      }
      default:
        break;
    }
  }
  }
  pfc_log_trace("Exiting the for loop in apply_action");
}

UncDataflowReason DataflowCmn::deleteflow(DataflowCmn* nextCtrlrFlow) {
  pfc_log_debug("before is_head=%d next.size()=%" PFC_PFMT_SIZE_T
                " head->total_flow_count=%d", is_head, next.size(),
                head->total_flow_count); 
  if (next.size() > 0 && (head->total_flow_count > 1)) {
    head->total_flow_count--;
  }
  next.pop_back();
  pfc_log_debug("after is_head=%d next.size()=%" PFC_PFMT_SIZE_T
                " head->total_flow_count=%d", is_head, next.size(),
                head->total_flow_count);
  return UNC_DF_RES_SUCCESS;
}

UncDataflowReason DataflowCmn::appendFlow(DataflowCmn* nextCtrlrFlow, map<string, uint32_t>& ctrlr_dom_count_map) {
  pfc_log_debug("ctrlr_dom_count_map.size=%" PFC_PFMT_SIZE_T " is_head=%d",
                ctrlr_dom_count_map.size(), is_head);
  pfc_log_debug("Head is %p", head);
  nextCtrlrFlow->head = head;
  addl_data->reason = 0;
  addl_data->controller_count = 0;
  string cname = "";
  if (kidx_val_df_data_flow_cmn == nextCtrlrFlow->df_segment->st_num_) {
    cname = (const char*)nextCtrlrFlow->df_segment->df_common->controller_name;
    map<string, uint32_t >::iterator dciter = addl_data->traversed_controllers.find(cname);
    if (dciter == addl_data->traversed_controllers.end()) {
      pfc_log_debug("%s is not present in traversed controllers map", cname.c_str());
      uint32_t dom_count = 0;
      map<string, uint32_t>::iterator cdciter = ctrlr_dom_count_map.find(cname);
      if (cdciter != ctrlr_dom_count_map.end()) {
        dom_count = cdciter->second;
      }
      if (dom_count == 0)
        dom_count = 2; //Assume as 2 - double the value. 
      addl_data->max_dom_traversal_count += dom_count;
      pfc_log_debug("ctr_name %s  dom count %d is added, max_dom_traversal_count :%d"
          , cname.c_str(), dom_count, addl_data->max_dom_traversal_count);
      addl_data->traversed_controllers.insert(std::pair<string, uint32_t>(cname, dom_count));
    } else {
      pfc_log_debug("%s is present in traversed controllers map", cname.c_str());
    }
  } else {
    addl_data->max_dom_traversal_count = ctrlr_dom_count_map["nvtnctrlrdom"] * 2;
  } 
  if (addl_data->current_traversal_count+1 > addl_data->max_dom_traversal_count) {
    pfc_log_debug("current_traversal_count %d is exceeding max_dom_traversal_count %d", addl_data->current_traversal_count+1, addl_data->max_dom_traversal_count);
    addl_data->reason = UNC_DF_RES_EXCEEDS_HOP_LIMIT;
    return UNC_DF_RES_EXCEEDS_HOP_LIMIT;
  } else {
    nextCtrlrFlow->addl_data->max_dom_traversal_count = addl_data->max_dom_traversal_count;
    if ((kidx_val_df_data_flow_cmn == nextCtrlrFlow->df_segment->st_num_) ||
       (kidx_val_df_data_flow_cmn != nextCtrlrFlow->df_segment->st_num_ &&
        UNC_CT_UNKNOWN != nextCtrlrFlow->df_segment->vtn_df_common->controller_type)) {
        nextCtrlrFlow->addl_data->current_traversal_count = addl_data->current_traversal_count+1;
    }
    nextCtrlrFlow->addl_data->traversed_controllers = addl_data->traversed_controllers;
    pfc_log_debug("current_traversal_count %d is not exceeding max_dom_traversal_count %d", addl_data->current_traversal_count, addl_data->max_dom_traversal_count);
  }

  pfc_log_debug("before is_head=%d next.size()=%" PFC_PFMT_SIZE_T
                " head->total_flow_count=%d", is_head, next.size(),
                head->total_flow_count); 
  if (next.size() >= 1) {
    // increament total_flow_count
    head->total_flow_count++;
  }
  pfc_log_debug("after is_head=%d next.size()=%" PFC_PFMT_SIZE_T
                " head->total_flow_count=%d", is_head, next.size(),
                head->total_flow_count);
  next.push_back(nextCtrlrFlow);
  return UNC_DF_RES_SUCCESS;
}

/** Destructor
 * * @Description : Destructor deletes all allocated memories
 * * @param[in]   : None
 * * @return      : None
 **/
DataflowUtil::~DataflowUtil() {
  pfc_log_debug("~DataflowUtil ctrlr_dom_count_map.size=%d firstCtrlrFlows.size=%d pfc_flows.size=%d", 
                (int)ctrlr_dom_count_map.size(), (int)firstCtrlrFlows.size(), (int)pfc_flows.size());
  // Clear the allocated memory
  ctrlr_dom_count_map.clear();

  if (firstCtrlrFlows.size() > 0) {
  vector<DataflowCmn *>::iterator firstCtrlrFlows_iter;
  for (firstCtrlrFlows_iter = firstCtrlrFlows.begin();
      firstCtrlrFlows_iter != firstCtrlrFlows.end();
      firstCtrlrFlows_iter++) {
    delete (*firstCtrlrFlows_iter);
  }
  firstCtrlrFlows.clear();
  }
  
  if (pfc_flows.size() > 0) {
  map<key_dataflow_t, vector<DataflowDetail*> >::iterator miter = pfc_flows.begin();
  while(miter != pfc_flows.end()) {
    vector<DataflowDetail*> flowDetails = miter->second;
    pfc_log_debug("~DataflowUtil deleting %d df_segments for key %s", (int)flowDetails.size(), DataflowCmn::get_string(miter->first).c_str());
    if (flowDetails.size() > 0) {
    vector<DataflowDetail*>::iterator it = flowDetails.begin();
    while(it != flowDetails.end()) {
      DataflowDetail *df_segm = (DataflowDetail*)*it;
      pfc_log_debug("~DataflowUtil deleting %d df_segment %p", df_segm->df_common->controller_type, df_segm);
      delete df_segm;
      it++;
    }
    flowDetails.clear();
    }
    miter++;
  }
  pfc_flows.clear();
  }
  
  if (upll_pfc_flows.size() > 0) {
  map<key_vtn_ctrlr_dataflow, vector<DataflowDetail*> >::iterator miter = upll_pfc_flows.begin();
  while(miter != upll_pfc_flows.end()) {
    vector<DataflowDetail*> flowDetails = miter->second;
    if (flowDetails.size() > 0) {
    vector<DataflowDetail*>::iterator it = flowDetails.begin();
    while(it != flowDetails.end()) {
      DataflowDetail *df_segm = (DataflowDetail*)*it;
      delete df_segm;
      it++;
    }
    flowDetails.clear();
    }
    miter++;
  }
  upll_pfc_flows.clear();
  }
  if (vext_info_map.size() > 0) {
    std::map<std::string, void* >::iterator it;
    std::string tmp_key;
    it = vext_info_map.begin();
    for ( ; it != vext_info_map.end(); ++it) {
     tmp_key = it->first;
     if ((tmp_key.compare(0, 1, "0")) == 0) {
      UPLL_LOG_DEBUG("vexternal information ingress is deleted");
      if (it->second)
       delete reinterpret_cast<ConfigKeyVal *>(it->second);
     }
    }
    vext_info_map.clear();
  }
  if (vnode_rename_map.size() > 0)
    vnode_rename_map.clear();
  if (bypass_dom_set.size() > 0)
    bypass_dom_set.clear();
}


uint32_t DataflowUtil::get_total_flow_count() {
    uint32_t tot_flow_count = 0;
    vector<DataflowCmn *>::iterator iter_1st_ctrlr_flow =
          firstCtrlrFlows.begin();
    while (iter_1st_ctrlr_flow != firstCtrlrFlows.end()) {
      DataflowCmn *aFlow = (DataflowCmn *)(*iter_1st_ctrlr_flow);
      tot_flow_count += aFlow->total_flow_count;
      pfc_log_debug("Tot=%d, curr=%d", tot_flow_count, aFlow->total_flow_count);
      ++iter_1st_ctrlr_flow;
    }
    return tot_flow_count;
}


uint32_t DataflowUtil::appendFlow(DataflowCmn* firstCtrlrFlow) {
  pfc_log_debug("Inside appendFlow of DataflowUtil");
  if (firstCtrlrFlow != NULL) {
    firstCtrlrFlow->head = firstCtrlrFlow;
    firstCtrlrFlow->total_flow_count = 1;
    firstCtrlrFlow->addl_data->reason = 0;
    firstCtrlrFlow->addl_data->controller_count = 1;
    firstCtrlrFlow->addl_data->current_traversal_count = 1;
    uint32_t dom_count = 0;
    string cname = "";
    if (kidx_val_vtn_dataflow_cmn == firstCtrlrFlow->df_segment->st_num_) {
     cname = (const char*)firstCtrlrFlow->df_segment->vtn_df_common->
                                                    controller_id;
    } else {
     cname = (const char*)firstCtrlrFlow->df_segment->df_common->
                                                    controller_name;
    }
    map<string, uint32_t>::iterator cdciter = ctrlr_dom_count_map.find(cname);
    if (cdciter != ctrlr_dom_count_map.end()) {
      dom_count = cdciter->second;
    }
    if (dom_count == 0)
      dom_count = 2;  // Assume as 2 - double the value.
    firstCtrlrFlow->addl_data->max_dom_traversal_count = dom_count;
    pfc_log_debug("Initialising max_dom_traversal_count to %d for %s", dom_count, cname.c_str());
    firstCtrlrFlow->addl_data->traversed_controllers.insert(std::pair<string,
                                                  uint32_t>(cname, dom_count));
    firstCtrlrFlows.push_back(firstCtrlrFlow);
  }
  return 0;
}

int DataflowUtil::sessOutDataflowsFromDriver(ServerSession& sess) {
  int err = 0;
  int putresp_pos = 12;
  uint32_t sz = firstCtrlrFlows.size();
  pfc_log_info("%d. flow_count=%d", putresp_pos, sz);
  putresp_pos++;
  sess.addOutput(sz);
  vector<DataflowCmn *>::iterator iter_1st_ctrlr_flow =
      firstCtrlrFlows.begin();
  while (iter_1st_ctrlr_flow != firstCtrlrFlows.end()) {
    DataflowCmn *aFlow = reinterpret_cast<DataflowCmn *>(*iter_1st_ctrlr_flow);
    err |= aFlow->sessOutDataflow(sess, putresp_pos);
    ++iter_1st_ctrlr_flow;
    DataflowDetail *df_seg = aFlow->df_segment;
    delete aFlow;
    pfc_log_debug("sessOutDataflowsFromDriver befor df_segment delete");
    delete df_seg;
  }
  firstCtrlrFlows.clear();
  return err;
}

int DataflowUtil::sessOutDataflows(ServerSession& sess) {
  pfc_log_debug("Inside sessOutDataflows");
  uint32_t tot_flow_count = DataflowUtil::get_total_flow_count();
  int putresp_pos = 10;
  int err = 0;
  pfc_log_info("%d. flow_count=%d", putresp_pos, tot_flow_count);
  putresp_pos++;
  err |= sess.addOutput(tot_flow_count);

  while (firstCtrlrFlows.size()>0) {
  pfc_log_debug("sessOutDataflows flows.size:%"
                                  PFC_PFMT_SIZE_T, firstCtrlrFlows.size());

  vector<DataflowCmn *>::iterator iter_1st_ctrlr_flow =
                                  firstCtrlrFlows.begin();
    while (iter_1st_ctrlr_flow != firstCtrlrFlows.end()) {
      pfc_log_debug("One head flow is being processed");
      DataflowCmn *headFlow =  reinterpret_cast<DataflowCmn *>
                                                      (*iter_1st_ctrlr_flow);

      DataflowCmn *aFlow = headFlow;
      uint32_t ctrlr_cnt = 1;
      uint32_t reason = headFlow->addl_data->reason;
      while (aFlow->next.size() != 0) {
        pfc_log_debug("Inside while before controller_count=%d reason=%d",
                                    ctrlr_cnt, reason);
        ctrlr_cnt++;
        aFlow = *(aFlow->next.begin());
        if (aFlow != NULL && aFlow->addl_data != NULL) {
          pfc_log_debug("aFlow ir aFlow->addl_data is null");
          reason = aFlow->addl_data->reason;
        } else {
          break;
        }
        pfc_log_debug("Inside while after controller_count=%d reason=%d",
                                    ctrlr_cnt, reason);
      }
      pfc_log_debug("Final controller_count=%d reason=%d df_type=%d", ctrlr_cnt, reason, headFlow->df_segment->st_num_);
      if (kidx_val_vtn_dataflow_cmn == headFlow->df_segment->st_num_) {
        val_vtn_dataflow_t obj_val_vtn_dataflow;
        memset(&obj_val_vtn_dataflow, '\0', sizeof(val_vtn_dataflow_t));
        obj_val_vtn_dataflow.reason = reason;
        obj_val_vtn_dataflow.ctrlr_domain_count = ctrlr_cnt;
        memset(obj_val_vtn_dataflow.valid, UNC_VF_VALID,
                                     sizeof(obj_val_vtn_dataflow.valid));
        err |= sess.addOutput(obj_val_vtn_dataflow);
        if (err != 0) {
          pfc_log_warn("Adding to session failed with err %d", err);
          return err;
        }
      } else {
        val_df_data_flow_t obj_val_df_data_flow;
        //memset(&obj_val_df_data_flow, '\0', sizeof(val_df_data_flow));
        obj_val_df_data_flow.reason = reason;
        obj_val_df_data_flow.controller_count = ctrlr_cnt;
        memset(obj_val_df_data_flow.valid, UNC_VF_VALID, sizeof(obj_val_df_data_flow.valid));
        pfc_log_debug("%d. %s", putresp_pos, get_string(obj_val_df_data_flow).c_str());
        putresp_pos++;
        err |= sess.addOutput(obj_val_df_data_flow);
        if (err != 0) {
          pfc_log_warn("Adding to session failed with err %d", err);
          return err;
        }
      }

      aFlow = headFlow;
      stack <DataflowCmn *> lastBranchStack;

      err |= aFlow->sessOutDataflow(sess, putresp_pos);
      if (err != 0) {
        pfc_log_warn("Adding to session failed with err %d", err);
        return err;
      }

      while (aFlow->next.size() != 0) {
          if (aFlow->next.size() > 1) {
            while (!lastBranchStack.empty()) {
              lastBranchStack.pop();
            };
            pfc_log_debug("Cleared all stack entries. Size=%" PFC_PFMT_SIZE_T,
                         lastBranchStack.size());
          }

         if (kidx_val_vtn_dataflow_cmn == headFlow->df_segment->st_num_) {
          pfc_log_debug("Adding to stack flowid=%" PFC_PFMT_u64,
                        aFlow->df_segment->vtn_df_common->flow_id);
          } else {
          pfc_log_debug("Adding to stack flowid=%" PFC_PFMT_u64,
                        aFlow->df_segment->df_common->flow_id);
          }
          lastBranchStack.push(aFlow);
          aFlow = *(aFlow->next.begin());
          err |= aFlow->sessOutDataflow(sess, putresp_pos);
          if (err != 0) {
            pfc_log_warn("Adding to session failed with err %d", err);
            return err;
          }
      }
      pfc_log_debug("Added one complete path to session. Stack.size=%"
                         PFC_PFMT_SIZE_T, lastBranchStack.size());
      DataflowCmn *iter_fl = headFlow;
      while (!lastBranchStack.empty()) {
        pfc_log_debug("stack Size=%" PFC_PFMT_SIZE_T, lastBranchStack.size());
        iter_fl = lastBranchStack.top();
        lastBranchStack.pop();
        pfc_log_debug("after pop stack Size=%" PFC_PFMT_SIZE_T " iter_fl=%p",
                          lastBranchStack.size(), iter_fl);
        vector<DataflowCmn *>::iterator it = iter_fl->next.begin();
        if (it != iter_fl->next.end()) {
          pfc_log_debug("Found tree branch node to be deleted");
          delete *it;
          iter_fl->next.erase(it);  // remove the first element from the vector
        }
      }
      pfc_log_debug("One branch deleted iter_fl=%p headFlow=%p",
                            iter_fl, headFlow);

      if (iter_fl == headFlow) {
        if (iter_fl->next.size() == 0) {
          pfc_log_debug("Reached head node, so delete it");
          delete  reinterpret_cast<DataflowCmn *>(*iter_1st_ctrlr_flow);
          firstCtrlrFlows.erase(iter_1st_ctrlr_flow);
        }
        break;
      } else {
        break;
      }
    }
  }
  return err;
}

bool DataflowUtil::checkMacAddress(uint8_t macaddr[6], uint8_t macaddr_mask[6],
                       uint8_t checkmacaddr[6]) {
  bool retval = false;
  for (int i = 0; i < 6; i++) {
    retval = checkByte(macaddr[i], macaddr_mask[i], checkmacaddr[i]);
    if (retval == false)
      break;
  }
  return retval;
}

bool DataflowUtil::checkMacAddress(uint8_t macaddr[6], uint8_t macaddr_mask[6],
                       uint8_t checkmacaddr[6], uint8_t checkmacaddr_mask[6]) {
  bool retval = false;
  for (int i = 0; i < 6; i++) {
    retval = checkByte(macaddr[i], macaddr_mask[i], checkmacaddr[i],
                       checkmacaddr_mask[i]);
    if (retval == false)
      break;
  }
  return retval;
}

bool DataflowUtil::checkIPv4Address(in_addr ipaddr, in_addr ip_mask,
                        in_addr checkipaddr) {
  return checkIPv4Address(ipaddr.s_addr, ip_mask.s_addr, checkipaddr.s_addr);
}

bool DataflowUtil::checkByte(uint8_t ipaddr, uint8_t ip_mask,
                         uint8_t checkipaddr) {
  // stringstream ss;
  // ss << "1FRIP-" << getipstring(ipaddr, 2) << " " << getipstring(ipaddr, 16) <<endl;
  // ss << "1MASK:" << getipstring(ip_mask, 2) << " " << getipstring(ip_mask, 16) <<endl;
  // ss << "1TOIP:" << getipstring(checkipaddr, 2) << " " << getipstring(checkipaddr, 16) <<endl;
  uint8_t anded1 = ipaddr & ip_mask;
  // ss << "1AND :" << getipstring(anded1, 2) << " " << getipstring(anded1, 16) <<endl;
  uint8_t anded2 = checkipaddr & ip_mask;
  // ss << "2AND :" << getipstring(anded2, 2) << " " << getipstring(anded2, 16) <<endl;
  // pfc_log_debug("\n%s", ss.str().c_str());
  if (anded1 == anded2) return true;
  return false;
}
bool DataflowUtil::checkByte(uint8_t ipaddr, uint8_t ip_mask,
                           uint8_t checkipaddr, uint8_t chk_ip_mask) {
  // stringstream ss;
  // ss << "1MASK:" << getipstring(ip_mask, 2) << " " << getipstring(ip_mask, 16) <<endl;
  // ss << "2MASK:" << getipstring(chk_ip_mask, 2) << " " << getipstring(chk_ip_mask, 16) <<endl;
  uint8_t and_mask = ip_mask & chk_ip_mask;
  // ss << "&MASK:" << getipstring(and_mask, 2) << " " << getipstring(and_mask, 16) <<endl;
  // pfc_log_debug("\n%s", ss.str().c_str());
  return checkByte(ipaddr, and_mask, checkipaddr);
}

bool DataflowUtil::checkIPv4Address(uint32_t ipaddr, uint32_t ip_mask,
                            uint32_t checkipaddr) {
  // stringstream ss;
  // ss << "1FRIP-" << getipstring(ipaddr, 2) << " " << getipstring(ipaddr, 16) <<endl;
  // ss << "1MASK:" << getipstring(ip_mask, 2) << " " << getipstring(ip_mask, 16) <<endl;
  // ss << "2TOIP:" << getipstring(checkipaddr, 2) << " " << getipstring(checkipaddr, 16) <<endl;
  uint32_t anded1 = ipaddr & ip_mask;
  // ss << "1AND :" << getipstring(anded1, 2) << " " << getipstring(anded1, 16) <<endl;
  uint32_t anded2 = checkipaddr & ip_mask;
  // ss << "2AND :" << getipstring(anded2, 2) << " " << getipstring(anded2, 16) <<endl;
  // pfc_log_debug("\n%s", ss.str().c_str());
  if (anded1 == anded2) return true;
  return false;
}



bool DataflowUtil::checkIPv6Address(in6_addr ipv6addr, in6_addr ipv6_mask,
                             in6_addr checkipv6addr) {
  bool ret = false;
  for (int index = 0; index < 4; index ++) {
    uint32_t ipv4_addr = ipv6addr.s6_addr32[index];
    uint32_t ipv4_mask = ipv6_mask.s6_addr32[index];
    uint32_t checkipv4_addr = checkipv6addr.s6_addr32[index];
    ret = checkIPv4Address(ipv4_addr, ipv4_mask, checkipv4_addr);
    if (ret == false)
      break;
  }
  return ret;
}


bool DataflowUtil::checkIPv6Address(in6_addr ipv6addr, in6_addr ipv6_mask,
                            in6_addr checkipv6addr, in6_addr chk_ipv6_mask) {
  bool ret = false;
  for (int index = 0; index < 4; index ++) {
    in_addr ipv4_addr, ipv4_mask, checkipv4_addr, checkipv4_mask;
    ipv4_addr.s_addr =  ipv6addr.s6_addr32[index];
    ipv4_mask.s_addr = ipv6_mask.s6_addr32[index];
    checkipv4_addr.s_addr = checkipv6addr.s6_addr32[index];
    checkipv4_mask.s_addr = chk_ipv6_mask.s6_addr32[index];
    ret = checkIPv4Address(ipv4_addr, ipv4_mask, checkipv4_addr,
                             checkipv4_mask);
    if (ret == false)
      break;
  }
  return ret;
}



bool DataflowUtil::checkIPv4Address(in_addr ipaddr, in_addr ip_mask,
                             in_addr checkipaddr, in_addr chk_ip_mask) {
  return checkIPv4Address(ipaddr.s_addr, ip_mask.s_addr, checkipaddr.s_addr,
                             chk_ip_mask.s_addr);
}

bool DataflowUtil::checkIPv4Address(uint32_t ipaddr, uint32_t ip_mask,
                             uint32_t checkipaddr, uint32_t chk_ip_mask) {
  // stringstream ss;
  // ss << "1MASK:" << getipstring(ip_mask, 2) << " " << getipstring(ip_mask, 16)
  //                            <<endl;
  // ss << "2MASK:" << getipstring(chk_ip_mask, 2) << " "
  //                            << getipstring(chk_ip_mask, 16) <<endl;
  uint32_t and_mask = ip_mask & chk_ip_mask;
  // ss << "&MASK:" << getipstring(and_mask, 2) << " " << getipstring(and_mask, 16)
  //                            <<endl;
  // pfc_log_info("\n%s", ss.str().c_str());
  return checkIPv4Address(ipaddr, and_mask, checkipaddr);
}

string DataflowUtil::getipstring(in_addr ipnaddr, int radix) {
  uint32_t ipaddr = ipnaddr.s_addr;
  return getipstring(ipaddr, radix);
}

string DataflowUtil::getipstring(uint32_t ipaddr, int radix) {
  if (radix == 2) {
    bitset<32> A = ipaddr;
    stringstream ss;
    for (int i = 31; i >= 0; i--) {
      if ((i+1)%8 == 0) ss << ".";
      ss << A[i];
    };
    return ss.str();
  } else if (radix == 8) {
    char b_ipaddr[16];
    memset(&b_ipaddr, '\0', 16);
    snprintf(b_ipaddr, sizeof(b_ipaddr), "%03d.%03d.%03d.%03d",
           ipaddr>>24&0xFF, ipaddr>>16&0xFF, ipaddr>>8&0xFF, ipaddr&0xFF);
    return b_ipaddr;
  } else if (radix == 16) {
    char b_ipaddr[12];
    memset(&b_ipaddr, '\0', 12);
    snprintf(b_ipaddr, sizeof(b_ipaddr), "%02x.%02x.%02x.%02x",
           ipaddr>>24&0xFF, ipaddr>>16&0xFF, ipaddr>>8&0xFF, ipaddr&0xFF);
    return b_ipaddr;
  } else if (radix == 32) {
    char b_ipaddr[12];
    memset(&b_ipaddr, '\0', 12);
    snprintf(b_ipaddr, sizeof(b_ipaddr), "%d", ipaddr);
    return b_ipaddr;
  }
  return "";
}

string DataflowUtil::getbytestring(uint8_t ipaddr, int radix) {
  if (radix == 2) {
    bitset<8> A = ipaddr;
    stringstream ss;
    for (int i = 7; i >= 0; i--) {
      if ((i+1)%8 == 0) ss << ".";
      ss << A[i];
    };
    return ss.str();
  } else if (radix == 8) {
    char b_ipaddr[4];
    memset(&b_ipaddr, '\0', 4);
    snprintf(b_ipaddr, sizeof(b_ipaddr), "%03d",
           ipaddr&0xFF);
    return b_ipaddr;
  } else if (radix == 16) {
    char b_ipaddr[3];
    memset(&b_ipaddr, '\0', 3);
    snprintf(b_ipaddr, sizeof(b_ipaddr), "%02x",
           ipaddr&0xFF);
    return b_ipaddr;
  }
  return "";
}

/** get_string
 * @Description : This function returns the values from
 *                the key structure
 * @param[in]   : k - structure variable of type key_dataflow
 * @return      : attributes in key structure of key_dataflow_t are returned
 * returned
 **/
string DataflowCmn::get_string(const key_dataflow_t &k) {
  char macaddr[18];
  memset(&macaddr, '\0', 18);
  snprintf(macaddr, sizeof(macaddr), "%02x%02x.%02x%02x.%02x%02x",
           k.src_mac_address[0], k.src_mac_address[1], k.src_mac_address[2],
           k.src_mac_address[3], k.src_mac_address[4], k.src_mac_address[5]);

  stringstream ss;
  ss << "KT_DATAFLOW:[KEY: "
      << "controller_name:" << k.controller_name
      << ", switch_id:" << k.switch_id
      << ", port_id:" << k.port_id
      << ", vlan_id:" << uint16tostr(k.vlan_id)
      << ", src_mac_address:" << macaddr
      << "]";
  return ss.str();
}
/** 
 * @Description : This function returns the values from
 *                the key structure
 * @param[in]   : k - structure variable of type key_vtn_dataflow
 * @return      : attributes in key structure of key_vtn_dataflow_t are returned
 * returned
 **/
string DataflowCmn::get_string(const key_vtn_dataflow_t &k) {
  char macaddr[18];
  memset(&macaddr, '\0', 18);
  snprintf(macaddr, sizeof(macaddr), "%02x%02x.%02x%02x.%02x%02x",
           k.src_mac_address[0], k.src_mac_address[1], k.src_mac_address[2],
           k.src_mac_address[3], k.src_mac_address[4], k.src_mac_address[5]);

  stringstream ss;
  ss << "KT_VTN_DATAFLOW:[KEY: "
      << "vtn_name:" << k.vtn_key.vtn_name
      << ", vnode_id:" << k.vnode_id
      << ", vlanid:" << uint16tostr(k.vlanid)
      << ", src_mac_address:" << macaddr
      << "]";
  return ss.str();
}


/** get_string
 * @Description : This function returns controller name and flow_id from
 *                the key structure
 * @param[in]   : k - structure variable of type key_ctr_dataflow
 * @return      : attributes in key structure of key_ctr_dataflow_t are returned
 * returned
 **/
string DataflowCmn::get_string(const key_ctr_dataflow_t &k) {
  stringstream ss;
  ss << "KT_CTR_DATAFLOW:[KEY: "
      << "controller_name:" << k.ctr_key.controller_name
      << ", flow_id:" << uint64tostr(k.flow_id)
      << "]";
  return ss.str();
}

/** get_string
 * @Description : This function returns controller name and flow_id from
 *                the key structure
 * @param[in]   : k - structure variable of type key_ctr_dataflow
 * @return      : attributes in key structure of key_ctr_dataflow_t are returned
 * returned
 **/
string DataflowCmn::get_string(const key_dataflow_v2_t &k) {
  stringstream ss;
  ss << "KT_DATAFLOW_V2:[KEY: "
      << "controller_name:" << k.controller_name
      << "]";
  return ss.str();
}

/** get_string
 * @Description : This function returns controller name and flow_id from
 *                the key structure
 * @param[in]   : k - structure variable of type key_ctr_dataflow
 * @return      : attributes in key structure of key_ctr_dataflow_t are returned
 * returned
 **/
string DataflowCmn::get_string(const val_dataflow_v2_t &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 1; ++i) {
    valid << uint8tostr(v.valid[i]);
  }
  ss << "VAL_DATAFLOW_V2:[VAL: "
      << "flow_id:" << uint64tostr(v.flow_id)
      << " valid:" << valid.str()
      << "]";
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_data_flow_cmn_t
 * @return      : attributes in value structure of val_df_data_flow_cmn_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_data_flow_cmn_t &val_obj) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 18 ; ++i) {
    valid << uint8tostr(val_obj.valid[i]);
  }
  ss << "[VAL_FLOW_CMN:"
      << " controller_name: " << val_obj.controller_name
      << ", controller_type: " << uint8tostr(val_obj.controller_type)
      << "\n"
      << " flow_id: " << uint64tostr(val_obj.flow_id)
      << ", status: " << val_obj.status
      << ", flow_type: " << val_obj.flow_type
      << ", policy_index: " << val_obj.policy_index
      << " vtn_id: " << val_obj.vtn_id
      << "\n"
      << " ingress_switch_id: " << val_obj.ingress_switch_id
      << ", in_port: " << val_obj.in_port
      << ", in_station_id: " << uint64tostr(val_obj.in_station_id)
      << ", in_domain: " << val_obj.in_domain
      << "\n"
      << " egress_switch_id: " << val_obj.egress_switch_id
      << ", out_port: " << val_obj.out_port
      << ", out_station_id: " << uint64tostr(val_obj.out_station_id)
      << ", out_domain: " << val_obj.out_domain
      << "\n"
      << " path_info_count: " << val_obj.path_info_count
      << ", match_count: " << val_obj.match_count
      << ", action_count: " << val_obj.action_count
      << "\n"
      << " valid: " << valid.str()
      << " ]"
      << endl;
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_vtn_dataflow_cmn_t
 * @return      : attributes in value structure of val_vtn_dataflow_cmn_t are
 * returned
 **/
string DataflowCmn::get_string(const val_vtn_dataflow_cmn_t &val_obj) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 21 ; ++i) {
    valid << uint8tostr(val_obj.valid[i]);
  }
  ss << "[VAL_FLOW_CMN:"
      << " valid: " << valid.str()
      << " ]"
      << "\n"
      << " controller_name: " << val_obj.controller_id
      << "\n"
      << " controller_type: " << uint8tostr(val_obj.controller_type)
      << "\n"
      << " flow_id: " << uint64tostr(val_obj.flow_id)
      << "\n"
      << " created_time: " << uint64tostr(val_obj.created_time)
      << "\n"
      << " idle_timeout: " << val_obj.idle_timeout
      << "\n"
      << " hard_timeout:" << val_obj.hard_timeout
      << "\n"
      << " ingress_vnode: " << val_obj.ingress_vnode
      << "\n"
      << " ingress_vinterface: " << val_obj.ingress_vinterface
      << "\n"
      << " ingress_switch_id: " << val_obj.ingress_switch_id
      << "\n"
      << " ingress_port_id: " << val_obj.ingress_port_id
      << "\n"
      << " ingress_logical_port_id: " << val_obj.ingress_logical_port_id
      << "\n"
      << " ingress_domain: " << val_obj.ingress_domain
      << "\n"
      << " egress_vnode: " << val_obj.egress_vnode
      << "\n"
      << " egress_vinterface: " << val_obj.egress_vinterface
      << "\n"
      << " egress_switch_id: " << val_obj.egress_switch_id
      << "\n"
      << " egress_port_id: " << val_obj.egress_port_id
      << "\n"
      << " egress_logical_port_id: " << val_obj.egress_logical_port_id
      << "\n"
      << " egress_domain: " << val_obj.egress_domain
      << "\n"
      << " match_count: " << val_obj.match_count
      << "\n"
      << " action_count: " << val_obj.action_count
      << "\n"
      << " path_info_count: " << val_obj.path_info_count
      << "\n"
      << endl;
  return ss.str();
}



/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_match_t
 * @return      : attributes in value structure of val_df_flow_match_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_match_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_MATCH: "
      << "match_type: " << val_obj.match_type
      << " ]"
      << endl;
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_match_in_port_t
 * @return      : attributes in value structure of val_df_flow_match_in_port_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_match_in_port_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_MATCH_IN_PORT: "
      << "in_port: " << val_obj.in_port
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_match_dl_addr_t
 * @return      : attributes in value structure of val_df_flow_match_dl_addr_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_match_dl_addr_t &val_obj) {
  stringstream ss;
  char macaddr[18];
  memset(&macaddr, '\0', 18);
  snprintf(macaddr, sizeof(macaddr), "%02x%02x.%02x%02x.%02x%02x",
           val_obj.dl_addr[0], val_obj.dl_addr[1], val_obj.dl_addr[2],
           val_obj.dl_addr[3], val_obj.dl_addr[4], val_obj.dl_addr[5]);
  char macaddr_mask[18];
  memset(&macaddr_mask, '\0', 18);
  snprintf(macaddr_mask, sizeof(macaddr_mask), "%02x%02x.%02x%02x.%02x%02x",
           val_obj.dl_addr_mask[0], val_obj.dl_addr_mask[1],
           val_obj.dl_addr_mask[2],
           val_obj.dl_addr_mask[3], val_obj.dl_addr_mask[4],
           val_obj.dl_addr_mask[5]);

  ss << "[VAL_FLOW_MATCH_DL_ADDR: "
      <<"dl_addr: " << macaddr
      <<", v_mask: " << uint8tostr(val_obj.v_mask)
      <<", dl_addr_mask: " << macaddr_mask
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_match_dl_type_t 
 * @return      : attributes in value structure of val_df_flow_match_dl_type_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_match_dl_type_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_MATCH_DL_TYPE: "
      << "dl_type: " << uint8tostr(val_obj.dl_type)
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_match_vlan_id_t
 * @return      : attributes in value structure of val_df_flow_match_vlan_id_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_match_vlan_id_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_MATCH_VLAN_ID: "
      << "_vlan_id: " << uint16tostr(val_obj.vlan_id)
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_match_vlan_pcp_t 
 * @return      : attributes in value structure of val_df_flow_match_vlan_pcp_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_match_vlan_pcp_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_MATCH_VLAN_PCP: "
      << "vlan_pcp: " << uint8tostr(val_obj.vlan_pcp)
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_match_ip_tos_t
 * @return      : attributes in value structure of val_df_flow_match_ip_tos_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_match_ip_tos_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_MATCH_IP_TOS: "
      << "ip_tos: " << uint8tostr(val_obj.ip_tos)
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_match_ip_proto_t
 * @return      : attributes in value structure of val_df_flow_match_ip_proto_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_match_ip_proto_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_MATCH_ip_proto: "
      << "ip_proto: " << uint8tostr(val_obj.ip_proto)
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_match_ipv4_addr_t
 * @return      : attributes in value structure of val_df_flow_match_ipv4_addr_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_match_ipv4_addr_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_MATCH_ipv4_addr: "
      << "ipv4_addr: " << val_obj.ipv4_addr.s_addr
      << ", v_mask: " << uint8tostr(val_obj.v_mask)
      << ", ipv4_addr_mask: " << val_obj.ipv4_addr_mask.s_addr
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_match_tp_port_t
 * @return      : attributes in value structure of val_df_flow_match_tp_port_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_match_tp_port_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_MATCH_tp_port: "
      << "tp_port: " << uint16tostr(val_obj.tp_port)
      << ", v_mask: " << uint8tostr(val_obj.v_mask)
      << ", tp_port_mask: " << uint16tostr(val_obj.tp_port_mask)
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_match_ipv6_addr_t
 * @return      : attributes in value structure of val_df_flow_match_ipv6_addr_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_match_ipv6_addr_t &val_obj) {
  stringstream ss;
  char ipv6_addr[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, &val_obj.ipv6_addr.s6_addr, ipv6_addr, INET6_ADDRSTRLEN);
  char ipv6_addr_mask[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, &val_obj.ipv6_addr_mask.s6_addr, ipv6_addr_mask,
                                 INET6_ADDRSTRLEN);
  ss << "[VAL_FLOW_MATCH_ipv6_addr: "
      << "ipv6_addr: " << ipv6_addr
      << ", v_mask: " << uint8tostr(val_obj.v_mask)
      << ", ipv6_addr_mask: " << ipv6_addr_mask
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_action_t
 * @return      : attributes in value structure of val_df_flow_action_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_action_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_ACTION_SET: "
      << "action_type: " << val_obj.action_type
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_action_output_port_t
 * @return      : attributes in value structure of val_df_flow_action_output_port_t are
 * returned
 **/
string DataflowCmn::get_string
                      (const val_df_flow_action_output_port_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_ACTION_SET_output_port: "
      << "output_port: " << val_obj.output_port
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_action_enqueue_port_t
 * @return      : attributes in value structure of val_df_flow_action_enqueue_port_t are
 * returned
 **/
string DataflowCmn::get_string
                    (const val_df_flow_action_enqueue_port_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_ACTION_SET_enqueue_port: "
      << "output_port: " << val_obj.output_port
      << ", enqueue_id: " << uint16tostr(val_obj.enqueue_id)
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_action_set_dl_addr_t
 * @return      : attributes in value structure of val_df_flow_action_set_dl_addr_t are
 * returned
 **/
string DataflowCmn::get_string
                    (const val_df_flow_action_set_dl_addr_t &val_obj) {
  char macaddr[18];
  memset(&macaddr, '\0', 18);
  snprintf(macaddr, sizeof(macaddr), "%02x%02x.%02x%02x.%02x%02x",
           val_obj.dl_addr[0], val_obj.dl_addr[1], val_obj.dl_addr[2],
           val_obj.dl_addr[3], val_obj.dl_addr[4], val_obj.dl_addr[5]);

  stringstream ss;
  ss << "[VAL_FLOW_ACTION_SET_set_dl_addr: "
      << "dl_addr: " << macaddr
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_action_set_vlan_id_t
 * @return      : attributes in value structure of val_df_flow_action_set_vlan_id_t are
 * returned
 **/
string DataflowCmn::get_string
                     (const val_df_flow_action_set_vlan_id_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_ACTION_SET_vlan_id: "
      << "vlan_id: " << uint16tostr(val_obj.vlan_id)
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_action_set_vlan_pcp_t
 * @return      : attributes in value structure of val_df_flow_action_set_vlan_pcp_t are
 * returned
 **/
string DataflowCmn::get_string
                      (const val_df_flow_action_set_vlan_pcp_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_ACTION_SET_vlan_pcp: "
      << "vlan_pcp: " << uint8tostr(val_obj.vlan_pcp)
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_action_set_ipv4_t
 * @return      : attributes in value structure of val_df_flow_action_set_ipv4_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_action_set_ipv4_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_ACTION_SET_ipv4: "
      << "ipv4_addr: " << val_obj.ipv4_addr.s_addr
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_action_set_ip_tos_t
 * @return      : attributes in value structure of val_df_flow_action_set_ip_tos_t are
 * returned
 **/
string DataflowCmn::get_string
                           (const val_df_flow_action_set_ip_tos_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_ACTION_SET_ip_tos: "
      << "ip_tos: " << uint8tostr(val_obj.ip_tos)
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_action_set_tp_port_t 
 * @return      : attributes in value structure of val_df_flow_action_set_tp_port_t are
 * returned
 **/
string DataflowCmn::get_string
                           (const val_df_flow_action_set_tp_port_t &val_obj) {
  stringstream ss;
  ss << "[VAL_FLOW_ACTION_SET_tp_port: "
      << "tp_port: " << uint16tostr(val_obj.tp_port)
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_flow_action_set_ipv6_t 
 * @return      : attributes in value structure of val_df_flow_action_set_ipv6_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_flow_action_set_ipv6_t &val_obj) {
  stringstream ss;
  char ipv6_addr[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, &val_obj.ipv6_addr.s6_addr, ipv6_addr, INET6_ADDRSTRLEN);
  ss << "[VAL_FLOW_ACTION_SET_ipv6: "
      << "ipv6_addr: " << ipv6_addr
      << " ]"
      << endl;
  return ss.str();
}

string DataflowCmn::get_string(const val_vtn_dataflow_path_info_t &val_obj) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 6 ; ++i) {
    valid << uint8tostr(val_obj.valid[i]);
  }
  ss << "[VAL_FLOW_PATH_INFO: "
      << " valid: " << valid.str()
      << " ]"
      << "\n"
      << " in_vnode: " << val_obj.in_vnode
      << "\n"
      << " in_vif: " << val_obj.in_vif
      << "\n"
      << " out_vnode: " << val_obj.out_vnode
      << "\n"
      << " out_vif: " << val_obj.out_vif
      << "\n"
      << " vlink_flag: " << uint8tostr(val_obj.vlink_flag)
      << "\n"
      << " status: " << uint8tostr(val_obj.status)
      << endl;
  return ss.str();
}


/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_data_flow_path_info_t 
 * @return      : attributes in value structure of val_df_data_flow_path_info_t are
 * returned
 **/
string DataflowCmn::get_string(const val_df_data_flow_path_info_t &val_obj) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 3 ; ++i) {
    valid << uint8tostr(val_obj.valid[i]);
  }
  ss << "[VAL_FLOW_PATH_INFO: "
      << "switch_id: " << val_obj.switch_id
      << ", in_port: " << val_obj.in_port
      << ", out_port: " << val_obj.out_port
      << ", valid: " << valid.str()
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from AddlData structure
 * @param[in]   : ptr - structure variable of type AddlData
 * @return      : attributes in AddlData structure of are
 * returned
 **/
string DataflowCmn::get_string(const AddlData *ptr) {
  if (ptr == NULL) {
    return "[AddlData:NULL]\n";
  }
  stringstream ss;
  ss << "[AddlData: "
      << "reason: " << ptr->reason
      << ", controller_count: " << ptr->controller_count
      << ", max_dom_traversal_count: " << ptr->max_dom_traversal_count
      << ", current_traversal_count: " << ptr->current_traversal_count
      << ", traversed_controllers.size: " << ptr->traversed_controllers.size()
      << " ]"
      << endl;
  return ss.str();
}

/** get_string
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_data_flow_t
 * @return      : attributes in value structure of val_df_data_flow_t are
 * returned
 **/
string DataflowUtil::get_string(const val_df_data_flow_t &val_obj) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 2 ; ++i) {
    valid << uint8tostr(val_obj.valid[i]);
  }
  ss << "[VAL_FLOW: "
      << " reason: " << val_obj.reason
      << " controller_count: " << val_obj.controller_count
      << " valid: " << valid.str()
      << " ]"
      << endl;
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_vtn_dataflow_t
 * @return      : attributes in value structure of val_vtn_dataflow_t are
 * returned
 **/
string DataflowUtil::get_string(const val_vtn_dataflow_t &val_obj) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 2 ; ++i) {
    valid << uint8tostr(val_obj.valid[i]);
  }
  ss << "[VAL_FLOW: "
      << " valid: " << valid.str()
      << " ]"
      << "\n"
      << " reason: " << val_obj.reason
      << "\n"
      << " controller_domain_count: " << val_obj.ctrlr_domain_count
      << endl;
  return ss.str();
}


/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_data_flow_t
 * @return      : attributes in value structure of val_df_data_flow_t are
 * returned
 **/
string DataflowUtil::get_string(const val_df_data_flow_st_t &val_obj) {
  stringstream ss;
  stringstream valid_st;
  for (unsigned int i = 0; i < 3 ; ++i) {
    valid_st << uint8tostr(val_obj.valid[i]);
  }
  ss << "[VAL_FLOW_ST: "
      << " packets: " << uint64tostr(val_obj.packets)
      << ", octets: " << uint64tostr(val_obj.octets)
      << ", duration: " << val_obj.duration
      << " valid_st: " << valid_st.str()
      << " ]"
      << endl;
  return ss.str();
}

/** 
 * @Description : This function returns the values from the dataflow
 * value structure
 * @param[in]   : None
 * @return      : attributes in value structure of dataflow val_* are
 * returned
 **/
std::string DataflowCmn::ToStr() {
  stringstream ss;
  ss << get_string(*df_segment->df_common);
  map<UncDataflowFlowMatchType, void *>::iterator matches_iter;
  for (matches_iter = df_segment->matches.begin(); matches_iter !=
                                  df_segment->matches.end(); matches_iter++) {
    switch (matches_iter->first) {
      case UNC_MATCH_IN_PORT:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_in_port_t);
        break;
      }
      case UNC_MATCH_DL_DST:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_dl_addr_t);
        break;
      }
      case UNC_MATCH_DL_SRC:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_dl_addr_t);
        break;
      }
      case UNC_MATCH_DL_TYPE:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_dl_type_t);
        break;
      }
      case UNC_MATCH_VLAN_ID:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_vlan_id_t);
        break;
      }
      case UNC_MATCH_VLAN_PCP:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_vlan_pcp_t);
        break;
      }
      case UNC_MATCH_IP_TOS:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_ip_tos_t);
        break;
      }
      case UNC_MATCH_IP_PROTO:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_ip_proto_t);
        break;
      }
      case UNC_MATCH_IPV4_SRC:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_ipv4_addr_t);
        break;
      }
      case UNC_MATCH_IPV4_DST:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_ipv4_addr_t);
        break;
      }
      case UNC_MATCH_IPV6_SRC:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_ipv6_addr_t);
        break;
      }
      case UNC_MATCH_IPV6_DST:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_ipv6_addr_t);
        break;
      }
      case UNC_MATCH_TP_SRC:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_tp_port_t);
        break;
      }
      case UNC_MATCH_TP_DST:
      {
        DATAFLOW_MATCHES_GETSTRING(ss, *matches_iter,
                                   val_df_flow_match_tp_port_t);
        break;
      }
      default:
        break;
    }
  }

  vector<val_actions_vect_st *>::iterator actions_iter;
  for (actions_iter = df_segment->actions.begin(); actions_iter !=
                                   df_segment->actions.end(); actions_iter++) {
    val_actions_vect_st *ptr =  reinterpret_cast<val_actions_vect_st *>
                                                               (*actions_iter);
    switch (ptr->action_type) {
      case UNC_ACTION_OUTPUT:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_output_port_t);
        break;
      }
      case UNC_ACTION_SET_ENQUEUE:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_enqueue_port_t);
        break;
      }
      case UNC_ACTION_SET_DL_SRC:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_set_dl_addr_t);
        break;
      }
      case UNC_ACTION_SET_DL_DST:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_set_dl_addr_t);
        break;
      }
      case UNC_ACTION_SET_VLAN_ID:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_set_vlan_id_t);
        break;
      }
      case UNC_ACTION_SET_VLAN_PCP:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_set_vlan_pcp_t);
        break;
      }
      case UNC_ACTION_STRIP_VLAN:
      {
        ss << "  6...VAL_FLOW_ACTION_STRIP_VLAN" << endl;
        break;
      }
      case UNC_ACTION_SET_IPV4_SRC:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_set_ipv4_t);
        break;
      }
      case UNC_ACTION_SET_IPV4_DST:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_set_ipv4_t);
        break;
      }
      case UNC_ACTION_SET_IP_TOS:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_set_ip_tos_t);
        break;
      }
      case UNC_ACTION_SET_TP_SRC:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_set_tp_port_t);
        break;
      }
      case UNC_ACTION_SET_TP_DST:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_set_tp_port_t);
        break;
      }
      case UNC_ACTION_SET_IPV6_SRC:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_set_ipv6_t);
        break;
      }
      case UNC_ACTION_SET_IPV6_DST:
      {
        DATAFLOW_ACTIONS_GETSTRING(ss, ptr, val_df_flow_action_set_ipv6_t);
        break;
      }
      default:
        break;
    }
  }
  for (uint32_t i = 0; i < df_segment->path_infos.size(); i++) {
    ss << "  " << get_string(*df_segment->path_infos[i]);
  }
  ss << get_string(addl_data);
  if (head == NULL)
    ss << "head=NULL";
  else
    ss << "head=ptr";

  ss << " next.size=" << next.size()
     << " total_flow_count=" << total_flow_count << endl;

  return ss.str();
}

/** deep_copy
 * @Description : This function copies matches to output_matches
 * @param[in]   : None
 * @return      : None
 **/
void DataflowCmn::deep_copy() {
  map<UncDataflowFlowMatchType, void *>::iterator iter;
  for (iter = df_segment->matches.begin(); iter != df_segment->matches.end();
                                                iter++) {
    switch (iter->first) {
      case UNC_MATCH_IN_PORT:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_in_port_t,
                                           output_matches,
                                           UNC_MATCH_IN_PORT, *iter);
        break;
      }
      case UNC_MATCH_DL_DST:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_dl_addr_t,
                                           output_matches,
                                           UNC_MATCH_DL_DST, *iter);
        break;
      }
      case UNC_MATCH_DL_SRC:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_dl_addr_t,
                                           output_matches,
                                           UNC_MATCH_DL_SRC, *iter);
        break;
      }
      case UNC_MATCH_DL_TYPE:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_dl_type_t,
                                           output_matches,
                                           UNC_MATCH_DL_TYPE, *iter);
        break;
      }
      case UNC_MATCH_VLAN_ID:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_vlan_id_t,
                                           output_matches,
                                           UNC_MATCH_VLAN_ID, *iter);
        break;
      }
      case UNC_MATCH_VLAN_PCP:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_vlan_pcp_t,
                                           output_matches,
                                           UNC_MATCH_VLAN_PCP, *iter);
        break;
      }
      case UNC_MATCH_IP_TOS:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_ip_tos_t,
                                           output_matches,
                                           UNC_MATCH_IP_TOS, *iter);
        break;
      }
      case UNC_MATCH_IP_PROTO:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_ip_proto_t,
                                           output_matches,
                                           UNC_MATCH_IP_PROTO, *iter);
        break;
      }
      case UNC_MATCH_IPV4_SRC:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_ipv4_addr_t,
                                           output_matches,
                                           UNC_MATCH_IPV4_SRC, *iter);
        break;
      }
      case UNC_MATCH_IPV4_DST:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_ipv4_addr_t,
                                           output_matches,
                                           UNC_MATCH_IPV4_DST, *iter);
        break;
      }
      case UNC_MATCH_IPV6_SRC:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_ipv6_addr_t,
                                           output_matches,
                                           UNC_MATCH_IPV6_SRC, *iter);
        break;
      }
      case UNC_MATCH_IPV6_DST:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_ipv6_addr_t,
                                           output_matches,
                                           UNC_MATCH_IPV6_DST, *iter);
        break;
      }
      case UNC_MATCH_TP_SRC:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_tp_port_t,
                                           output_matches,
                                           UNC_MATCH_TP_SRC, *iter);
        break;
      }
      case UNC_MATCH_TP_DST:
      {
        DEEP_COPY_MATCHES_TO_OUTPUTMATCHES(val_df_flow_match_tp_port_t,
                                           output_matches,
                                           UNC_MATCH_TP_DST, *iter);
        break;
      }
    }
  }
}

/** Destructor
 * * @Description : Destructor deletes all allocated memories
 * * @param[in]   : None
 * * @return      : None
 **/
DataflowDetail::~DataflowDetail() {
  pfc_log_trace("DataflowDetail -Destructor call");

  if (df_common != NULL) {
    delete df_common;
    df_common = NULL;
  }
  if (vtn_df_common != NULL) {
    delete vtn_df_common;
    vtn_df_common = NULL;
  }
  if (ckv_egress)
    delete reinterpret_cast<ConfigKeyVal *>(ckv_egress);
  ckv_egress = NULL;
  if (matches.size() > 0) {
  map<UncDataflowFlowMatchType, void *>::iterator matches_iter;
  for (matches_iter = matches.begin();
       matches_iter != matches.end();
       matches_iter++) {
    ::operator delete((*matches_iter).second);
    pfc_log_trace("out_matches map entry is deleted");
  }
  matches.clear();
  }
  if (actions.size() > 0) {
  vector<val_actions_vect_st *>::iterator action_vect_iter;
  for (action_vect_iter = actions.begin();
      action_vect_iter != actions.end();
      action_vect_iter++) {
    val_actions_vect_st *ptr = reinterpret_cast<val_actions_vect_st*>
                                         ((*action_vect_iter));
    ::operator delete(ptr->action_st_ptr);
    delete ptr;
    ptr = NULL;
    pfc_log_trace("action vector entry is deleted");
  }
  actions.clear();
  }
  if (path_infos.size() > 0) {
  vector<val_df_data_flow_path_info *>::iterator path_infos_iter;
  for (path_infos_iter = path_infos.begin();
      path_infos_iter != path_infos.end();
      path_infos_iter++) {
    ::operator delete((*path_infos_iter));
    pfc_log_trace("path_infos vector entry is deleted");
  }
  path_infos.clear();
  }
  if (vtn_path_infos.size() > 0) {
  vector<val_vtn_dataflow_path_info *>::iterator vtn_path_infos_iter;
  for (vtn_path_infos_iter = vtn_path_infos.begin();
      vtn_path_infos_iter != vtn_path_infos.end();
      vtn_path_infos_iter++) {
    ::operator delete((*vtn_path_infos_iter));
    pfc_log_trace("vtn_path_infos vector entry is deleted");
  }
  vtn_path_infos.clear();
  }
}

PFC_MODULE_DECL(unc::dataflow::DataflowDummy);
