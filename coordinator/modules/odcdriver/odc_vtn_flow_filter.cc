/*
 * Copyright (c) 2015-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vtn_flow_filter.hh>

namespace unc {
namespace odcdriver {
using namespace unc::restjson;
  UncRespCode
  OdcVtnFlowFilterCmd::create_cmd(key_vtn_flowfilter& key,
                                val_flowfilter& val,
                              unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  OdcVtnFlowFilterCmd::update_cmd(key_vtn_flowfilter& key,
                            val_flowfilter& val_old,
                              val_flowfilter& val_new,
                             unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  OdcVtnFlowFilterCmd::delete_cmd(key_vtn_flowfilter& key,
                                     val_flowfilter& val,
                           unc::driver::controller *ctr_ptr) {
      return UNC_RC_SUCCESS;
  }

UncRespCode OdcVtnFlowFilterCmd::fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

    key_vtn_t* parent_vtn = reinterpret_cast<key_vtn_t*> (parent_key);
    std::string vtn_name = reinterpret_cast<char*>(parent_vtn->vtn_name);
    vtnflowfilter_request *req_obj = new vtnflowfilter_request(ctr_ptr,
                                                           vtn_name);
    std::string url = req_obj->get_url();
    pfc_log_info("URL:%s",url.c_str());
    vtnflowfilter_parser *parser_obj = new vtnflowfilter_parser();
    UncRespCode ret_val = req_obj->get_response(parser_obj);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Get response error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }

    ret_val = parser_obj->set_par_flow_filter(parser_obj->jobj);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("set_par_vtn_flowfilter_conf error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }

    ret_val = r_copy(parser_obj->par_flow_filter_ ,cfgnode_vector);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Error occured while parsing");
      delete req_obj;
      delete parser_obj;
      return ret_val;
    }
    return ret_val;
}

UncRespCode
OdcVtnFlowFilterCmd::r_copy(std::list<par_flow_filter> &flow_detail,
               std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

    ODC_FUNC_TRACE;

    key_vtn_flowfilter key_filter;
    val_flowfilter val_filter;
    memset ( &key_filter, 0, sizeof(key_vtn_flowfilter));
    memset ( &val_filter, 0, sizeof(val_flowfilter));
    strncpy(reinterpret_cast<char*> (key_filter.vtn_key.vtn_name),
            parent_vtn_name_.c_str(), sizeof(key_filter.vtn_key.vtn_name)-1);
    key_filter.input_direction=UPLL_FLOWFILTER_DIR_IN;

    //Add to Cache
    unc::vtndrvcache::ConfigNode *filter_cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vtn_flowfilter, val_flowfilter,
                                   val_flowfilter, uint32_t>
    (&key_filter,&val_filter,&val_filter, uint32_t(UNC_OP_READ));

    cfgnode_vector.push_back(filter_cfgptr);

    std::list<par_flow_filter>::iterator iter = flow_detail.begin();

    while ( iter != flow_detail.end() ) {
      key_vtn_flowfilter_entry key_entry;
      val_vtn_flowfilter_entry val_entry;

      memset(&key_entry,0,sizeof(key_vtn_flowfilter_entry));
      memset(&val_entry,0,sizeof(val_vtn_flowfilter_entry));

      // Key VTN Flow Filter Entry
      key_entry.sequence_num=iter->index;
      memcpy(&key_entry.flowfilter_key,&key_filter,
             sizeof(key_vtn_flowfilter));

      // VAL VTN Flow Filter Entry
      strncpy(reinterpret_cast<char*> (val_entry.flowlist_name),
              iter->condition.c_str(),sizeof(val_entry.flowlist_name) - 1);
      val_entry.valid[UPLL_IDX_SEQ_NUM_FFES] = UNC_VF_VALID;


      if ( iter->par_vtn_pass_filter_.valid == true ) {
        val_entry.action=UPLL_FLOWFILTER_ACT_PASS;
        val_entry.valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
      } else if ( iter->par_vtn_drop_filter_.valid == true ) {
        val_entry.action=UPLL_FLOWFILTER_ACT_DROP;
        val_entry.valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
      }

      std::list<par_flow_action>::iterator action_iter =
                                           iter->par_flow_action_.begin();

      while ( action_iter != iter->par_flow_action_.end() ) {

        //For Every action, check if dscp or vlanpcp

        if ( action_iter->par_dscp_.valid == true) {
          if ( action_iter->par_dscp_.dscp_value != -1 ) {
            val_entry.dscp=action_iter->par_dscp_.dscp_value;
            val_entry.valid[UPLL_IDX_DSCP_VFFE]=UNC_VF_VALID;
          }
        } else if ( action_iter->par_vlanpcp_.valid == true) {
          if ( action_iter->par_vlanpcp_.vlan_pcp != -1 ) {
            val_entry.priority=action_iter->par_vlanpcp_.vlan_pcp;
            val_entry.valid[UPLL_IDX_PRIORITY_VFFE]=UNC_VF_VALID;
          }
        }
        action_iter++;
      }
      unc::vtndrvcache::ConfigNode *entry_cfgptr=
        new unc::vtndrvcache::CacheElementUtil<key_vtn_flowfilter_entry,
              val_vtn_flowfilter_entry, val_vtn_flowfilter_entry, uint32_t>
      (&key_entry, &val_entry, &val_entry, uint32_t(UNC_OP_READ));
      cfgnode_vector.push_back(entry_cfgptr);
      iter++;
    }
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  OdcVtnFlowFilterEntryCmd::create_cmd(key_vtn_flowfilter_entry& key,
                                          val_vtn_flowfilter_entry& val,
             unc::driver::controller *ctr_ptr) {
      std::string vtn_name(reinterpret_cast<char*>(
                          key.flowfilter_key.vtn_key.vtn_name));

      vtnflowfilter_entry_class *req_obj = new
                              vtnflowfilter_entry_class(ctr_ptr,vtn_name);
      ip_vtn_flowfilter st_obj;
      copy(st_obj, key, val);
      vtnflowfilter_entry_parser *parser_obj = new vtnflowfilter_entry_parser();
      json_object *jobj = parser_obj->create_req(st_obj);
      if (jobj == NULL){
        pfc_log_error("Error in create_request");
        delete req_obj;
        delete parser_obj;
        return UNC_DRV_RC_ERR_GENERIC;
      }
      if(req_obj->set_post(jobj) != UNC_RC_SUCCESS) {
        pfc_log_error("VTN FlowFilterEntry Create Failed");
        delete req_obj;
        delete parser_obj;
        return UNC_DRV_RC_ERR_GENERIC;
      }
      delete req_obj;
      delete parser_obj;
      return UNC_RC_SUCCESS;
  }

  UncRespCode
  OdcVtnFlowFilterEntryCmd::update_cmd(key_vtn_flowfilter_entry& key,
                                 val_vtn_flowfilter_entry& val_old,
                                  val_vtn_flowfilter_entry& val_new,
                                      unc::driver::controller *ctr_ptr) {
      std::string vtn_name(reinterpret_cast<char*>(
                                      key.flowfilter_key.vtn_key.vtn_name));

      vtnflowfilter_entry_class *req_obj = new
                                  vtnflowfilter_entry_class(ctr_ptr,vtn_name);
      ip_vtn_flowfilter st_obj;
      copy(st_obj, key, val_old,val_new);
      vtnflowfilter_entry_parser *parser_obj = new vtnflowfilter_entry_parser();
      json_object *jobj = parser_obj->create_req(st_obj);
      if (jobj == NULL){
        pfc_log_error("Error in create_request");
        delete req_obj;
        delete parser_obj;
        return UNC_DRV_RC_ERR_GENERIC;
      }

      if(req_obj->set_post(jobj) != UNC_RC_SUCCESS) {
        pfc_log_error("VTN FlowFilterEntry UPDATE Failed");
        delete req_obj;
        delete parser_obj;
        return UNC_DRV_RC_ERR_GENERIC;
      }
      delete req_obj;
      delete parser_obj;
      return UNC_RC_SUCCESS;
  }

  UncRespCode
  OdcVtnFlowFilterEntryCmd::delete_cmd(key_vtn_flowfilter_entry& key,
                                         val_vtn_flowfilter_entry& val,
                                     unc::driver::controller *ctr_ptr) {
      std::string vtn_name(reinterpret_cast<char*>(
                                        key.flowfilter_key.vtn_key.vtn_name));

      vtnflowfilter_entry_class *req_obj = new
                                  vtnflowfilter_entry_class(ctr_ptr,vtn_name);
      ip_vtn_flowfilter st_obj;
      delete_request_body(st_obj, key, val);
      vtnflowfilter_entry_parser *parser_obj = new vtnflowfilter_entry_parser();
      json_object *jobj = parser_obj->del_req(st_obj);
      if (jobj == NULL){
        pfc_log_error("Error in delete_request");
        delete req_obj;
        delete parser_obj;
        return UNC_DRV_RC_ERR_GENERIC;
      }
      if(req_obj->set_delete(jobj) != UNC_RC_SUCCESS) {
        pfc_log_error("VTN FlowFilterEntry DELETE Failed");
        delete req_obj;
        delete parser_obj;
        return UNC_DRV_RC_ERR_GENERIC;
      }
      delete req_obj;
      delete parser_obj;
      return UNC_RC_SUCCESS;
  }

void OdcVtnFlowFilterEntryCmd::delete_request_body(
                                      ip_vtn_flowfilter& ip_vtn_flowfilter_st,
                                                 key_vtn_flowfilter_entry& key,
                                                val_vtn_flowfilter_entry& val){
  ip_vtn_flowfilter_st.input_vtn_flowfilter_.valid = true;
  ip_vtn_flowfilter_st.input_vtn_flowfilter_.tenant_name =
              (reinterpret_cast<char*>(key.flowfilter_key.vtn_key.vtn_name));
}
void OdcVtnFlowFilterEntryCmd::
  copy(ip_vtn_flowfilter& ip_vtn_flowfilter_st, key_vtn_flowfilter_entry &key_in,
            val_vtn_flowfilter_entry &value_in) {

    ODC_FUNC_TRACE;

    ip_vtn_flowfilter_st.input_vtn_flowfilter_.valid = true;
    ip_vtn_flowfilter_st.input_vtn_flowfilter_.output = false;
    ip_vtn_flowfilter_st.input_vtn_flowfilter_.tenant_name =
           (reinterpret_cast<char*>(key_in.flowfilter_key.vtn_key.vtn_name));

    vt_flow_filter vtn_flow_filter_;
    vtn_flow_filter_.index = key_in.sequence_num;
    pfc_log_debug("Checking for vtn_flowfilter sequence num");

    vtn_flow_filter_.condition.assign(reinterpret_cast<char*>(
                                                    value_in.flowlist_name));
    pfc_log_debug("checking for vtn_flowfilter condition");

    if ( value_in.valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID ) {
      if ( value_in.action == UPLL_FLOWFILTER_ACT_PASS ) {
        vtn_flow_filter_.pass_filter_.valid = true;
      } else if ( value_in.action == UPLL_FLOWFILTER_ACT_DROP ) {
        vtn_flow_filter_.drop_filter_.valid = true;
      }
    }

    uint8_t order = 1;
    if ( value_in.valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID ) {
      pfc_log_info("New DSCP Action Created");
      vt_flow_action action_st;
      action_st.vtn_dscp_.valid = true;
      action_st.vtn_dscp_.dscp_value =value_in.dscp;
      action_st.order = order;
      order ++;
      vtn_flow_filter_.vt_flow_action_.push_back(action_st);
    }
    if ( value_in.valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID ) {
      vt_flow_action action_st;
      pfc_log_info("New PRIORITY Action Created");
      action_st.vtn_vlanpcp_.valid = true;
      action_st.vtn_vlanpcp_.vlan_pcp = value_in.priority;
      action_st.order = order;
      order ++;
      vtn_flow_filter_.vt_flow_action_.push_back(action_st);
    }
    ip_vtn_flowfilter_st.input_vtn_flowfilter_.vt_flow_filter_.push_back(vtn_flow_filter_);
  }
 //  Method to  handle two value structures during update operation
 void OdcVtnFlowFilterEntryCmd::copy(ip_vtn_flowfilter& ip_vtn_flowfilter_st,
                                       key_vtn_flowfilter_entry &key_in,
                                  val_vtn_flowfilter_entry &value_old_in,
                                     val_vtn_flowfilter_entry &value_new_in) {

    ODC_FUNC_TRACE;

    ip_vtn_flowfilter_st.input_vtn_flowfilter_.valid = true;
    ip_vtn_flowfilter_st.input_vtn_flowfilter_.output = false;
    ip_vtn_flowfilter_st.input_vtn_flowfilter_.tenant_name =
             (reinterpret_cast<char*>(key_in.flowfilter_key.vtn_key.vtn_name));

    vt_flow_filter vtn_flow_filter_;
    vtn_flow_filter_.index =key_in.sequence_num;
    pfc_log_debug("Checking for vtn_flowfilter sequence_num");

    if (value_new_in.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    vtn_flow_filter_.condition.assign(reinterpret_cast<char*>(
                                                value_new_in.flowlist_name));
    } else if (value_new_in.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_INVALID
           ||  value_old_in.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    vtn_flow_filter_.condition.assign(reinterpret_cast<char*>(
                                             value_old_in.flowlist_name));
    }
    if ( value_new_in.valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID ) {
      if ( value_new_in.action == UPLL_FLOWFILTER_ACT_PASS ) {
        vtn_flow_filter_.pass_filter_.valid = true;
      } else if ( value_new_in.action == UPLL_FLOWFILTER_ACT_DROP ) {
        vtn_flow_filter_.drop_filter_.valid = true;
      }
    } else if ( value_old_in.valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID ) {
      if ( value_old_in.action == UPLL_FLOWFILTER_ACT_PASS ) {
        vtn_flow_filter_.pass_filter_.valid = true;
      } else if ( value_old_in.action == UPLL_FLOWFILTER_ACT_DROP ) {
        vtn_flow_filter_.pass_filter_.valid = true;
      }
    }

   if ( value_new_in.valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID) {
      vt_flow_action  action_st;
      action_st.vtn_dscp_.valid = true;
      action_st.vtn_dscp_.dscp_value = value_new_in.dscp;
      pfc_log_error("New value structure of dscp attribute %d", value_new_in.dscp);
      vtn_flow_filter_.vt_flow_action_.push_back(action_st);
    } else if (value_new_in.valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_INVALID &&
                 value_old_in.valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID ) {
        vt_flow_action  action_st;
        action_st.vtn_dscp_.valid = true;
        action_st.vtn_dscp_.dscp_value = value_old_in.dscp;
        pfc_log_error("New value structure of dscp attribute %d", value_old_in.dscp);
        vtn_flow_filter_.vt_flow_action_.push_back(action_st);
      } else {
      pfc_log_info("INVALID for new and old val structures of dscp attribute");
    }
    if ( value_new_in.valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID) {
      vt_flow_action  action_st;
      action_st.vtn_vlanpcp_.valid =true ;
      action_st.vtn_vlanpcp_.vlan_pcp = value_new_in.priority;
      vtn_flow_filter_.vt_flow_action_.push_back(action_st);
    } else if (value_new_in.valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_INVALID &&
                 value_old_in.valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID ) {
        vt_flow_action  action_st;
        action_st.vtn_vlanpcp_.valid =true ;
        action_st.vtn_vlanpcp_.vlan_pcp = value_old_in.priority;
        vtn_flow_filter_.vt_flow_action_.push_back(action_st);
      } else {
      pfc_log_info("INVALID for new and old value structures of PRIORITY");
    }
    ip_vtn_flowfilter_st.input_vtn_flowfilter_.vt_flow_filter_.push_back(vtn_flow_filter_);
  }
}
}

