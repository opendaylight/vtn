/*
 * Copyright (c) 2015-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include<odc_vbr_flow_filter.hh>

namespace unc {
namespace odcdriver {
using namespace unc::restjson;
  UncRespCode
  OdcVbrFlowFilterCmd::create_cmd(key_vbr_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  OdcVbrFlowFilterCmd::update_cmd(key_vbr_flowfilter& key,
                  val_flowfilter& val_old,
                  val_flowfilter& val_new,
                  unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  OdcVbrFlowFilterCmd::delete_cmd(key_vbr_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr_ptr) {

    return UNC_RC_SUCCESS;
}

UncRespCode
OdcVbrFlowFilterCmd::fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

   key_vbr_t* parent_vbr = reinterpret_cast<key_vbr_t*> (parent_key);
   std::string vtn_name = reinterpret_cast<char*>(parent_vbr->vtn_key.vtn_name);
   std::string vbr_name = reinterpret_cast<char*>(parent_vbr->vbridge_name);

   vbrflowfilter_class *req_obj = new vbrflowfilter_class(
                                           ctr_ptr, vtn_name, vbr_name);
   std::string url = req_obj->get_url();
   pfc_log_info("URL:%s",url.c_str());
   vbrflowfilter_parser *parser_obj = new vbrflowfilter_parser();
   UncRespCode ret_val = req_obj->get_response(parser_obj);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Get response error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }

   ret_val = parser_obj->set_flow_filter(parser_obj->jobj);
   if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("set_flow_filter_conf error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }

   ret_val = r_copy(parser_obj->flow_filter_, cfgnode_vector);
   if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Error occured while parsing");
      delete req_obj;
      delete parser_obj;
      return ret_val;
  }
  return ret_val;
}

UncRespCode
OdcVbrFlowFilterCmd::r_copy(std::list<flow_filter> &filter_detail,
                 std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

    ODC_FUNC_TRACE;
    //Create both and FlowFilter and Entries and add to cache

    key_vbr_flowfilter key_filter;
    val_flowfilter val_filter;
    //val_flowfilter_entry val_entry;
    memset ( &key_filter, 0, sizeof(key_vbr_flowfilter));
    memset ( &val_filter, 0, sizeof(val_flowfilter));
    strncpy(reinterpret_cast<char*> (key_filter.vbr_key.vbridge_name),
                        parent_vbr_name_.c_str(),
                               sizeof(key_filter.vbr_key.vbridge_name) - 1);
    strncpy(reinterpret_cast<char*> (key_filter.vbr_key.vtn_key.vtn_name),
                          parent_vtn_name_.c_str(),
                       sizeof(key_filter.vbr_key.vtn_key.vtn_name) - 1);

      key_filter.direction=UPLL_FLOWFILTER_DIR_IN;

    //Add to Cache
    unc::vtndrvcache::ConfigNode *filter_cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vbr_flowfilter,
                   val_flowfilter,val_flowfilter, uint32_t>
    (&key_filter, &val_filter, &val_filter, uint32_t(UNC_OP_READ));

    cfgnode_vector.push_back(filter_cfgptr);

    std::list<flow_filter>::iterator iter = filter_detail.begin();

    while ( iter != filter_detail.end() ) {
      key_vbr_flowfilter_entry key_entry;
      val_flowfilter_entry val_entry;

      memset(&key_entry,0,sizeof(key_vbr_flowfilter_entry));
      memset(&val_entry,0,sizeof(val_flowfilter_entry));

      // Key VTN Flow Filter Entry
      key_entry.sequence_num=iter->index;
      memcpy(&key_entry.flowfilter_key,&key_filter,
             sizeof(key_vbr_flowfilter));

      // VAL VTN Flow Filter Entry
      strncpy(reinterpret_cast<char*> (val_entry.flowlist_name),
              iter->condition.c_str(),sizeof(val_entry.flowlist_name) - 1);

      val_entry.valid[UPLL_IDX_SEQ_NUM_FFES] = UNC_VF_VALID;


      if ( iter->vbr_pass_filter_.valid == true) {
        pfc_log_info("ACTION Pass");
        val_entry.action=UPLL_FLOWFILTER_ACT_PASS;
        val_entry.valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
      } else if ( iter->vbr_drop_filter_.valid == true) {
        pfc_log_info("ACTION Drop");
        val_entry.action=UPLL_FLOWFILTER_ACT_DROP;
        val_entry.valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
      } else if ( iter->vbr_redirect_filter_.valid == true) {
        pfc_log_info("ACTION Redirect");
        val_entry.action=UPLL_FLOWFILTER_ACT_REDIRECT;
        val_entry.valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
        if ( iter->vbr_redirect_filter_.vbr_redirect_destination_.re_bridge_name != "" ) {
          pfc_log_info("ACTION Redirect Bridge");
          strncpy(reinterpret_cast<char*> (val_entry.redirect_node),
              iter->vbr_redirect_filter_.vbr_redirect_destination_.re_bridge_name.c_str(),
                  sizeof(val_entry.redirect_node) - 1);
          val_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE]=UNC_VF_VALID;
        }
        if (iter->vbr_redirect_filter_.vbr_redirect_destination_.re_terminal_name != "" ) {
          pfc_log_info("ACTION Redirect Terminal");
          strncpy(reinterpret_cast<char*> (val_entry.redirect_node),
                iter->vbr_redirect_filter_.vbr_redirect_destination_.re_terminal_name.c_str(),
                  sizeof(val_entry.redirect_node) - 1);
          val_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE]=UNC_VF_VALID;
        }
        if (iter->vbr_redirect_filter_.vbr_redirect_destination_.re_interface_name != "" ) {
          pfc_log_info("ACTION Redirect Interface");
          strncpy(reinterpret_cast<char*> (val_entry.redirect_port),
            iter->vbr_redirect_filter_.vbr_redirect_destination_.re_interface_name.c_str(),
                  sizeof(val_entry.redirect_port) - 1);
          val_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE]=UNC_VF_VALID;
        }

        if ( (iter->vbr_redirect_filter_.vbr_redirect_destination_.re_bridge_name != "") &&
            (iter->vbr_redirect_filter_.vbr_redirect_destination_.re_interface_name != "")) {
          pfc_log_info("ACTION Redirect node IN");
          val_entry.redirect_direction=UPLL_FLOWFILTER_DIR_IN;
        }
        else
        {
          pfc_log_info("ACTION Redirect node OUT");
          val_entry.redirect_direction=UPLL_FLOWFILTER_DIR_OUT;
        }
        val_entry.valid[UPLL_IDX_REDIRECT_DIRECTION_FFE]=UNC_VF_VALID;
        }


      std::list<flow_action>::iterator action_iter = iter->flow_action_.begin();
      unc::odcdriver::OdcUtil util;

      while ( action_iter != iter->flow_action_.end() ) {

        //For Every action, check if dscp or vlanpcp

        if ( action_iter->vbr_dscp_.valid == true) {
          if (action_iter->vbr_dscp_.dscp_value != -1 ) {
            val_entry.dscp=action_iter->vbr_dscp_.dscp_value;
            val_entry.valid[UPLL_IDX_DSCP_FFE]=UNC_VF_VALID;
          }
        } else if ( action_iter->vbr_vlanpcp_.valid == true) {
          if ( action_iter->vbr_vlanpcp_.vlan_pcp != -1 ) {
            val_entry.priority=action_iter->vbr_vlanpcp_.vlan_pcp;
            val_entry.valid[UPLL_IDX_PRIORITY_FFE]=UNC_VF_VALID;
          }
        } else if ( action_iter->vbr_dlsrc_.valid == true) {
          if ( action_iter->vbr_dlsrc_.dlsrc_address != "" ) {
            util.convert_macstring_to_uint8(action_iter->vbr_dlsrc_.dlsrc_address,
                                            &val_entry.modify_srcmac[0]);
            val_entry.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] = UNC_VF_VALID;
          }
        } else if ( action_iter->vbr_dldst_.valid == true) {
          if ( !action_iter->vbr_dldst_.dlsdt_address.empty() ) {
            util.convert_macstring_to_uint8(action_iter->vbr_dldst_.dlsdt_address,
                                            &val_entry.modify_dstmac[0]);
            val_entry.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = UNC_VF_VALID;
          }
        }
        action_iter++;
      }
      unc::vtndrvcache::ConfigNode *entry_cfgptr=
        new unc::vtndrvcache::CacheElementUtil<key_vbr_flowfilter_entry,
                    val_flowfilter_entry, val_flowfilter_entry, uint32_t>
      (&key_entry, &val_entry, &val_entry, uint32_t(UNC_OP_READ));
      cfgnode_vector.push_back(entry_cfgptr);
      iter++;
    }
    return UNC_RC_SUCCESS;
  }

UncRespCode
OdcVbrFlowFilterEntryCmd::create_cmd(key_vbr_flowfilter_entry& key,
                                            val_flowfilter_entry& val,
                                       unc::driver::controller *ctr_ptr) {

    ODC_FUNC_TRACE;

    std::string vtn_name(reinterpret_cast<char*>(
                key.flowfilter_key.vbr_key.vtn_key.vtn_name));
    std::string vbr_name(reinterpret_cast<char*>(
                      key.flowfilter_key.vbr_key.vbridge_name));

    vbrflowfilter_entry_class *req_obj = new vbrflowfilter_entry_class(ctr_ptr,
                                     vtn_name, vbr_name);
    ip_vbr_flowfilter st_obj;
    copy(st_obj, key, val);
    vbrflowfilter_entry_parser *parser_obj = new vbrflowfilter_entry_parser();
    json_object *jobj = parser_obj->create_req(st_obj);
    if (jobj == NULL){
      pfc_log_error("Error in create_request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
      }

    if(req_obj->set_post(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("VBR FlowFilterEntry Create Failed");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }

  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
  }

  UncRespCode
  OdcVbrFlowFilterEntryCmd::update_cmd(key_vbr_flowfilter_entry& key,
                                       val_flowfilter_entry& val_old,
                                      val_flowfilter_entry& val_new,
                                      unc::driver::controller *ctr_ptr) {

    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr_ptr != NULL);

    std::string vtn_name(reinterpret_cast<char*>(
                             key.flowfilter_key.vbr_key.vtn_key.vtn_name));
    std::string vbr_name(reinterpret_cast<char*>(
                                   key.flowfilter_key.vbr_key.vbridge_name));

    vbrflowfilter_entry_class *req_obj = new vbrflowfilter_entry_class (ctr_ptr,
                                     vtn_name, vbr_name);
    ip_vbr_flowfilter st_obj;
    copy(st_obj, key,val_old, val_new);
    vbrflowfilter_entry_parser *parser_obj = new vbrflowfilter_entry_parser();
    json_object *jobj = parser_obj->create_req(st_obj);
    if (jobj == NULL){
      pfc_log_error("Error in create_request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
      }

    if(req_obj->set_post(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("VBR FlowFilterEntry UPdate Failed");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
  }

  UncRespCode
  OdcVbrFlowFilterEntryCmd::delete_cmd(key_vbr_flowfilter_entry& key,
                                          val_flowfilter_entry& val,
                                           unc::driver::controller *ctr_ptr) {

    std::string vtn_name(reinterpret_cast<char*>(
                                  key.flowfilter_key.vbr_key.vtn_key.vtn_name));
    std::string vbr_name(reinterpret_cast<char*>(
                                      key.flowfilter_key.vbr_key.vbridge_name));

    vbrflowfilter_entry_class *req_obj = new vbrflowfilter_entry_class (ctr_ptr,
                                     vtn_name, vbr_name);
    ip_vbr_flowfilter st_obj;
    delete_request_body(st_obj, key,val);
    vbrflowfilter_entry_parser *parser_obj = new vbrflowfilter_entry_parser();
    json_object *jobj = parser_obj->del_req(st_obj);
    if (jobj == NULL){
      pfc_log_error("Error in delete_request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
      }

    if(req_obj->set_delete(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("VBR FlowFilterEntry DELETE Failed");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
  }

void OdcVbrFlowFilterEntryCmd::delete_request_body(
                                      ip_vbr_flowfilter&  ip_vbr_flowfilter_st,
                                                 key_vbr_flowfilter_entry& key,
                                                 val_flowfilter_entry& val){

    ip_vbr_flowfilter_st.input_flow_filter_.valid = true;
    ip_vbr_flowfilter_st.input_flow_filter_.tenant_name =
        (reinterpret_cast<char*>(key.flowfilter_key.vbr_key.vtn_key.vtn_name));
    ip_vbr_flowfilter_st.input_flow_filter_.bridge_name =
          (reinterpret_cast<char*>(key.flowfilter_key.vbr_key.vbridge_name));
}


void OdcVbrFlowFilterEntryCmd::copy(ip_vbr_flowfilter&  ip_vbr_flowfilter_st,
                                   key_vbr_flowfilter_entry &key_in,
                                   val_flowfilter_entry &value_in) {
    pfc_log_error("Checking vbr_flowfilter copy function ");

    ODC_FUNC_TRACE;

    ip_vbr_flowfilter_st.input_flow_filter_.valid = true;
    if (key_in.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_OUT)
      ip_vbr_flowfilter_st.input_flow_filter_.output = true;
    else if (key_in.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_IN)
      ip_vbr_flowfilter_st.input_flow_filter_.output = false;
    ip_vbr_flowfilter_st.input_flow_filter_.tenant_name = (reinterpret_cast<
                       char*>(key_in.flowfilter_key.vbr_key.vtn_key.vtn_name));
    ip_vbr_flowfilter_st.input_flow_filter_.bridge_name = (reinterpret_cast<
                           char*>(key_in.flowfilter_key.vbr_key.vbridge_name));

    vtn_flow_filter vtn_flow_filter_;
    vtn_flow_filter_.index = key_in.sequence_num;
    pfc_log_error("Checking vbr_flowfilter seq_num ");

    vtn_flow_filter_.condition.assign(reinterpret_cast<char*>(
                                              value_in.flowlist_name));
    pfc_log_error("Checking vbr_flowfilter condition ");

    if ( value_in.valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID ) {
      if ( value_in.action == UPLL_FLOWFILTER_ACT_PASS ) {
        pfc_log_error("Checking ACTION value %d", value_in.action);
        pfc_log_error("Checking vbr PASS ");
        vtn_flow_filter_.vtn_pass_filter_.valid = true;
        pfc_log_error("Checking vbr PASS value %d ",
                                   vtn_flow_filter_.vtn_pass_filter_.valid);
      } else if ( value_in.action == UPLL_FLOWFILTER_ACT_DROP ) {
        pfc_log_error("Checking vbr DROP ");
        vtn_flow_filter_.vtn_drop_filter_.valid = true;
        pfc_log_error("Checking vbr DROP value %d ",
                                      vtn_flow_filter_.vtn_drop_filter_.valid);
      } else if ( value_in.action == UPLL_FLOWFILTER_ACT_REDIRECT ) {
        vtn_flow_filter_.vtn_redirect_filter_.valid = true;
        if ( value_in.valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID ) {
          pfc_log_error("Checking vbr REDIRECT ");
          vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.valid =
                                                                      true;
          std::string redirect_node(reinterpret_cast <char *>(
                                                         value_in.redirect_node));
          std::set <std::string>::iterator find_iter;
          find_iter = std::find (bridges_.begin(),bridges_.end(),redirect_node);

          if ( find_iter != bridges_.end()) {
             pfc_log_error("Checking vbr REDIRECT BRIDGES ");
            vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.re_bridge_name.assign(
              reinterpret_cast <char *>(value_in.redirect_node));
             pfc_log_error("Checking vbr REDIRECT AFTER BRIDGES ");
          } else {
            find_iter = std::find (terminals_.begin(),terminals_.end(),redirect_node);
            if ( find_iter != terminals_.end() )
              vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.re_terminal_name.assign(
                reinterpret_cast <char *>(value_in.redirect_node));
          }
        }
        if ( value_in.valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID ) {
          vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.re_interface_name.assign(
            reinterpret_cast <char *>(value_in.redirect_port));
        }
        if ( value_in.redirect_direction == UPLL_FLOWFILTER_DIR_IN )
          vtn_flow_filter_.vtn_redirect_filter_.redirect_output = false;
        else
          vtn_flow_filter_.vtn_redirect_filter_.redirect_output = true;
      }
    }

    pfc_log_info("check DSCP Valid %d", value_in.valid[UPLL_IDX_DSCP_FFE]);
    pfc_log_info("check Priority Valid %d", value_in.valid[UPLL_IDX_PRIORITY_FFE]);

    uint8_t order = 1;
    if ( value_in.valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID ) {
      pfc_log_info("New DSCP Action Created");
      vtn_flow_action action_st;
      action_st.ip_dscp_.valid = true;
      action_st.ip_dscp_.dscp_value  = value_in.dscp;
      action_st.order = order;
      order ++;
      vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
    }
    if ( value_in.valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID ) {
      vtn_flow_action action_st;
      pfc_log_info("New Priority Action Created");
      action_st.ip_vlanpcp_.valid = true;
      action_st.ip_vlanpcp_.vlan_pcp = value_in.priority;
      action_st.order = order;
      order ++;
      vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
    }
    if ( value_in.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID ) {
      vtn_flow_action action_st;
      unc::odcdriver::OdcUtil util_;
      action_st.ip_dldst_.valid = true;
      action_st.ip_dldst_.dlsdt_address = util_.macaddress_to_string
                                            (&value_in.modify_dstmac[0]);
      action_st.order = order;
      order ++;
      vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
    }
    if ( value_in.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID ) {
      vtn_flow_action action_st;
      unc::odcdriver::OdcUtil util_;
      action_st.ip_dlsrc_.valid = true;
      action_st.ip_dlsrc_.dlsrc_address = util_.macaddress_to_string
                                           (&value_in.modify_srcmac[0]);
      action_st.order = order;
      order ++;
      vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
    }

    ip_vbr_flowfilter_st.input_flow_filter_.vtn_flow_filter_.push_back(vtn_flow_filter_);
  }
   //  Method to  handle two value structures during update operation
void OdcVbrFlowFilterEntryCmd::copy(ip_vbr_flowfilter&  ip_vbr_flowfilter_st,
                                       key_vbr_flowfilter_entry &key_in,
                                       val_flowfilter_entry &value_old_in,
                                          val_flowfilter_entry &value_new_in) {


    ip_vbr_flowfilter_st.input_flow_filter_.valid = true;
    if (key_in.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_OUT)
      ip_vbr_flowfilter_st.input_flow_filter_.output = true;
    else if (key_in.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_IN)
      ip_vbr_flowfilter_st.input_flow_filter_.output = false;
    ip_vbr_flowfilter_st.input_flow_filter_.tenant_name =
      (reinterpret_cast<char*>(key_in.flowfilter_key.vbr_key.vtn_key.vtn_name));
    ip_vbr_flowfilter_st.input_flow_filter_.bridge_name =
          (reinterpret_cast<char*>(key_in.flowfilter_key.vbr_key.vbridge_name));

    vtn_flow_filter vtn_flow_filter_;
    vtn_flow_filter_.index = key_in.sequence_num;
    pfc_log_error("Checking vbr_flowfilter seq_num ");

    if (value_new_in.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    pfc_log_error("Checking vbr_flowfilter condition ");
    vtn_flow_filter_.condition.assign(reinterpret_cast<char*>(value_new_in.flowlist_name));
    } else if (value_new_in.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_INVALID
           ||  value_old_in.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    pfc_log_error("Checking vbr_flowfilter condition ");
    vtn_flow_filter_.condition.assign(reinterpret_cast<char*>(value_old_in.flowlist_name));
    }

    if (value_new_in.valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID) {
      if ( value_new_in.action == UPLL_FLOWFILTER_ACT_PASS ) {
        vtn_flow_filter_.vtn_pass_filter_.valid = true;
      } else if ( value_new_in.action == UPLL_FLOWFILTER_ACT_DROP ) {
        vtn_flow_filter_.vtn_drop_filter_.valid = true;
      } else if ( value_new_in.action == UPLL_FLOWFILTER_ACT_REDIRECT ) {
        vtn_flow_filter_.vtn_redirect_filter_.valid = true;
        if ( value_new_in.valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID ||
            value_new_in.valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
                                                 UNC_VF_VALUE_NOT_MODIFIED) {
          vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.valid = true;
          std::string redirect_node(reinterpret_cast <char *>(
                                 value_new_in.redirect_node));
          std::set <std::string>::iterator find_iter;
          find_iter = std::find (bridges_.begin(),bridges_.end(),redirect_node);
          if ( find_iter != bridges_.end()) {
            vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.re_bridge_name.assign(
              reinterpret_cast <char *>(value_new_in.redirect_node));
          } else {
            find_iter = std::find (terminals_.begin(),terminals_.end(),
                                                               redirect_node);
            if ( find_iter != terminals_.end() )
              vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.re_terminal_name.assign(
                reinterpret_cast <char *>(value_new_in.redirect_node));
          }
        }
        if ( value_new_in.valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID ||
             value_new_in.valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
                                              UNC_VF_VALUE_NOT_MODIFIED) {
          vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.re_interface_name.assign(
            reinterpret_cast <char *>(value_new_in.redirect_port));
        }
        if ( value_new_in.redirect_direction == UPLL_FLOWFILTER_DIR_IN )
          vtn_flow_filter_.vtn_redirect_filter_.redirect_output = false;
        else
          vtn_flow_filter_.vtn_redirect_filter_.redirect_output =true;
      }
    } else if( value_old_in.valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID ) {
      if ( value_old_in.action == UPLL_FLOWFILTER_ACT_PASS ) {
        vtn_flow_filter_.vtn_pass_filter_.valid = true;
      } else if (value_old_in.action == UPLL_FLOWFILTER_ACT_DROP ) {
        vtn_flow_filter_.vtn_drop_filter_.valid = true;
      } else if ( value_old_in.action == UPLL_FLOWFILTER_ACT_REDIRECT ) {
        vtn_flow_filter_.vtn_redirect_filter_.valid = true;
        if ( value_old_in.valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID ||
                  value_new_in.valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
                                             UNC_VF_VALUE_NOT_MODIFIED) {
          vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.valid =true ;
          std::string redirect_node(reinterpret_cast <char *>
                                               (value_new_in.redirect_node));
          std::set <std::string>::iterator find_iter;
          find_iter = std::find (bridges_.begin(),bridges_.end(),redirect_node);
          if ( find_iter != bridges_.end()) {
            vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.re_bridge_name.assign(
              reinterpret_cast <char *>(value_new_in.redirect_node));
          } else {
            find_iter = std::find (terminals_.begin(),terminals_.end(),
                                                             redirect_node);
            if ( find_iter != terminals_.end() )
              vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.re_terminal_name.assign(
                reinterpret_cast <char *>(value_new_in.redirect_node));
          }
        }
        if ( value_old_in.valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID ||
                  value_new_in.valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
                                                 UNC_VF_VALUE_NOT_MODIFIED) {
          vtn_flow_filter_.vtn_redirect_filter_.ip_redirect_destination_.re_interface_name.assign(
            reinterpret_cast <char *>(value_new_in.redirect_port));
        }
        if ( value_old_in.redirect_direction == UPLL_FLOWFILTER_DIR_IN )
          vtn_flow_filter_.vtn_redirect_filter_.redirect_output = false;
        else
          vtn_flow_filter_.vtn_redirect_filter_.redirect_output = true;
      }
    }
    uint8_t order = 1;
    if (value_new_in.valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID) {
      vtn_flow_action action_st;
      action_st.ip_dscp_.valid = true;
      action_st.ip_dscp_.dscp_value = value_new_in.dscp;
      action_st.order = order;
      order ++;
      pfc_log_info("New value structure of dscp attribute %d", value_new_in.dscp);
      vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
      pfc_log_info("New value structure of dscp attribute PUSHED ");
    } else if (value_new_in.valid[UPLL_IDX_DSCP_FFE] == UNC_VF_INVALID &&
                 value_old_in.valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID ) {
         vtn_flow_action action_st;
         action_st.ip_dscp_.valid = true;
         action_st.ip_dscp_.dscp_value = value_old_in.dscp;
         action_st.order = order;
         order ++;
         vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
      } else {
    pfc_log_info("INVALID for new and old value structures of dscp attribute ");
    }
    if ( value_new_in.valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID ){
      vtn_flow_action action_st;
      action_st.ip_vlanpcp_.valid =true ;
      action_st.ip_vlanpcp_.vlan_pcp = value_new_in.priority;
      action_st.order = order;
      order ++;
      vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
    } else if (value_new_in.valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_INVALID &&
                 value_old_in.valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID ) {
        vtn_flow_action action_st;
        action_st.ip_vlanpcp_.valid =true ;
        action_st.ip_vlanpcp_.vlan_pcp = value_old_in.priority;
        action_st.order = order;
        order ++;
        vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
      } else {
        pfc_log_info("INVALID for new and old value structures of PRIORITY ");
    }
   if ((value_new_in.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID &&
              value_old_in.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID) ||
                  value_new_in.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] ==
                         UNC_VF_VALUE_NOT_MODIFIED ){
      vtn_flow_action action_st;
      action_st.ip_dldst_.valid = true;
      unc::odcdriver::OdcUtil util_;
      action_st.ip_dldst_.dlsdt_address = util_.macaddress_to_string(
                                    &value_new_in.modify_dstmac[0]);
      action_st.order = order;
      order ++;
      vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
    } else if(value_new_in.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_INVALID
         && value_old_in.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID ) {
        vtn_flow_action action_st;
        action_st.ip_dldst_.valid = true;
        unc::odcdriver::OdcUtil util_;
        action_st.ip_dldst_.dlsdt_address = util_.macaddress_to_string(
                                &value_old_in.modify_dstmac[0]);
        action_st.order = order;
        order ++;
        vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
     }else {
      pfc_log_info("INVALID for new and old valstruct of DSTMACADDR attribute");
    }
   if ((value_new_in.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID) ||
         value_new_in.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] ==
                                                UNC_VF_VALUE_NOT_MODIFIED) {
      vtn_flow_action action_st;
      action_st.ip_dlsrc_.valid = true;
      unc::odcdriver::OdcUtil util_;
      action_st.ip_dlsrc_.dlsrc_address  = util_.macaddress_to_string(
                                     &value_new_in.modify_srcmac[0]);
      action_st.order = order;
      order ++;
      vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
    } else if(value_new_in.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_INVALID
          && value_old_in.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID ) {
      vtn_flow_action action_st;
      action_st.ip_dlsrc_.valid = true;
      unc::odcdriver::OdcUtil util_;
      action_st.ip_dlsrc_.dlsrc_address = util_.macaddress_to_string(
                               &value_old_in.modify_srcmac[0]);
      action_st.order = order;
      order ++;
      vtn_flow_filter_.vtn_flow_action_.push_back(action_st);
    } else {
      pfc_log_info("INVALID for new and old val structs of SRCMAC ADDR attribute");
    }
    ip_vbr_flowfilter_st.input_flow_filter_.vtn_flow_filter_.push_back(vtn_flow_filter_);
  }

}
}
