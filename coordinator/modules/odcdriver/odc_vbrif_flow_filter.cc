/*
 * Copyright (c) 2015-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vbrif_flow_filter.hh>

namespace unc {
namespace odcdriver {
using namespace unc::restjson;

  UncRespCode
  OdcVbrIfFlowFilterCmd::delete_cmd(key_vbr_if_flowfilter& key,
                                    pfcdrv_val_vbrif_vextif& val,
                               unc::driver::controller *ctr_ptr) {

    return UNC_RC_SUCCESS;
  }

  UncRespCode OdcVbrIfFlowFilterCmd::fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

    key_vbr_if_t *parent_vbr = reinterpret_cast<key_vbr_if_t *> (parent_key);
    parent_vtn_name_ = reinterpret_cast<char *>(
                                  parent_vbr->vbr_key.vtn_key.vtn_name);
    parent_vbr_name_ = reinterpret_cast<char *>(
                                       parent_vbr->vbr_key.vbridge_name);
    parent_vbrif_name_ = reinterpret_cast<char *>(parent_vbr->if_name);

    UncRespCode ret_val = portmap_chcek(ctr_ptr,parent_vtn_name_,parent_vbr_name_,
                                                   parent_vbrif_name_,cfgnode_vector);
    if (ret_val != UNC_RC_SUCCESS ){
      pfc_log_error("Cannot fetch flowfilter without portmapping");
      return ret_val;
    }

   vbrif_flowfilter_class *req_obj = new vbrif_flowfilter_class(ctr_ptr,
                         parent_vtn_name_,parent_vbr_name_,parent_vbrif_name_);
   std::string url = req_obj->get_url();
   pfc_log_info("URL:%s",url.c_str());
   vbrif_flowfilter_parser *parser_obj = new vbrif_flowfilter_parser();
   ret_val = req_obj->get_response(parser_obj);
   if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Get response error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }

   ret_val = parser_obj->set_vbrif_flow_filter(parser_obj->jobj);
   if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("set_vbrif_flow_filter_conf error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
   ret_val = r_copy(parser_obj->vbrif_flow_filter_, cfgnode_vector);
   if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Error occured while parsing");
      delete req_obj;
      delete parser_obj;
      return ret_val;
   }
  return ret_val;
}

UncRespCode
OdcVbrIfFlowFilterCmd::portmap_chcek( unc::driver::controller* ctr,
                                 std::string vtn_name, std::string vbr_name,
                                  std::string if_name,
              std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

  std::string url = "";
  url.append(RESTCONF_BASE);
  url.append(VTNS);
  url.append("/");
  url.append(vtn_name);
  url.append("/");
  url.append("vbridge");
  url.append("/");
  url.append(vbr_name);
  url.append("/");
  url.append("vinterface");
  url.append("/");
  url.append(if_name);
  url.append("/port-map-config");

  pfc_log_debug("url:%s", url.c_str());

  unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                                        ctr->get_user_name(),
                                        ctr->get_pass_word());
  unc::odcdriver::OdcController *odc_ctr =
                      reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
      url, restjson::HTTP_METHOD_GET, NULL, odc_ctr->get_conf_value());
  if (NULL == response) {
    pfc_log_error("Error Occured while getting httpresponse");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code for GET vbrif portmap is : %d", resp_code);

  if (HTTP_200_RESP_OK != resp_code ) {
    pfc_log_error("get vbridge interface portmap failure, resp_code %d", resp_code);
    return UNC_DRV_RC_ERR_GENERIC;
  }

 return UNC_RC_SUCCESS;
}

UncRespCode
OdcVbrIfFlowFilterCmd::r_copy(std::list<vbrif_flow_filter>  &filter_detail,
                     std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

    ODC_FUNC_TRACE;
    //Create both and FlowFilter and Entries and add to cache

    key_vbr_if_flowfilter key_filter;
    pfcdrv_val_vbrif_vextif val_filter;
    memset ( &key_filter, 0, sizeof(key_vbr_if_flowfilter));
    memset ( &val_filter, 0, sizeof(pfcdrv_val_vbrif_vextif));
    strncpy(reinterpret_cast<char*> (key_filter.if_key.vbr_key.vbridge_name),
            parent_vbr_name_.c_str(), sizeof(
                               key_filter.if_key.vbr_key.vbridge_name) - 1);
    strncpy(reinterpret_cast<char*> (key_filter.if_key.vbr_key.vtn_key.vtn_name),
            parent_vtn_name_.c_str(), sizeof(
                                key_filter.if_key.vbr_key.vtn_key.vtn_name) - 1);
    strncpy(reinterpret_cast<char*> (key_filter.if_key.if_name),
            parent_vbrif_name_.c_str(), sizeof(key_filter.if_key.if_name) - 1);

    key_filter.direction=UPLL_FLOWFILTER_DIR_IN;

    //Add to Cache
    unc::vtndrvcache::ConfigNode *filter_cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vbr_if_flowfilter,
                   pfcdrv_val_vbrif_vextif, pfcdrv_val_vbrif_vextif, uint32_t>
    (&key_filter, &val_filter, &val_filter, uint32_t(UNC_OP_READ));

    cfgnode_vector.push_back(filter_cfgptr);

    std::list<vbrif_flow_filter>::iterator iter = filter_detail.begin();

    while ( iter != filter_detail.end() ) {
      key_vbr_if_flowfilter_entry key_entry;
      pfcdrv_val_flowfilter_entry val_entry;

      memset(&key_entry,0,sizeof(key_vbr_if_flowfilter_entry));
      memset(&val_entry,0,sizeof(pfcdrv_val_flowfilter_entry));

      // Key VTN Flow Filter Entry
      key_entry.sequence_num=iter->index;
      memcpy(&key_entry.flowfilter_key,&key_filter,
             sizeof(key_vbr_if_flowfilter));

      // VAL VTN Flow Filter Entry
      strncpy(reinterpret_cast<char*> (val_entry.val_ff_entry.flowlist_name),
                               iter->condition.c_str(),
                             sizeof(val_entry.val_ff_entry.flowlist_name) - 1);
      val_entry.val_ff_entry.valid[UPLL_IDX_SEQ_NUM_FFES] = UNC_VF_VALID;


      if ( iter->vbrif_vtn_pass_filter_.valid == true) {
        pfc_log_info("ACTION Pass");
        val_entry.val_ff_entry.action=UPLL_FLOWFILTER_ACT_PASS;
        val_entry.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
      } else if ( iter->vbrif_vtn_drop_filter_.valid == true) {
        pfc_log_info("ACTION Drop");
        val_entry.val_ff_entry.action=UPLL_FLOWFILTER_ACT_DROP;
        val_entry.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
      } else if ( iter->vbrif_vtn_pass_filter_.valid == true) {
        pfc_log_info("ACTION Redirect");
        val_entry.val_ff_entry.action=UPLL_FLOWFILTER_ACT_REDIRECT;
        val_entry.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;

        if ( iter->vbrif_vtn_redirect_filter_.vbrif_redirect_destination_.vbrif_bridge_name != "" ) {
          pfc_log_info("ACTION Redirect Bridge");
          strncpy(reinterpret_cast<char*> (val_entry.val_ff_entry.redirect_node),
                  iter->vbrif_vtn_redirect_filter_.vbrif_redirect_destination_.vbrif_bridge_name.c_str(),
                  sizeof(val_entry.val_ff_entry.redirect_node) - 1);
          val_entry.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE]=UNC_VF_VALID;
        }
        if ( iter->vbrif_vtn_redirect_filter_.vbrif_redirect_destination_.vbrif_terminal_name != "" ) {
          pfc_log_info("ACTION Redirect Terminal");
          strncpy(reinterpret_cast<char*> (val_entry.val_ff_entry.redirect_node),
                  iter->vbrif_vtn_redirect_filter_.vbrif_redirect_destination_.vbrif_terminal_name.c_str(),
                  sizeof(val_entry.val_ff_entry.redirect_node) - 1);
          val_entry.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE]=UNC_VF_VALID;
        }
        if ( iter->vbrif_vtn_redirect_filter_.vbrif_redirect_destination_.vbrif_interface_name != "" ) {
          pfc_log_info("ACTION Redirect Interface");
          strncpy(reinterpret_cast<char*> (val_entry.val_ff_entry.redirect_port),
                  iter->vbrif_vtn_redirect_filter_.vbrif_redirect_destination_.vbrif_interface_name.c_str(),
                  sizeof(val_entry.val_ff_entry.redirect_port) - 1);
          val_entry.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE]=UNC_VF_VALID;
        }

        if ( (iter->vbrif_vtn_redirect_filter_.vbrif_redirect_destination_.vbrif_bridge_name != "") &&
            (iter->vbrif_vtn_redirect_filter_.vbrif_redirect_destination_.vbrif_interface_name != "")) {
          pfc_log_info("ACTION Redirect node IN");
          val_entry.val_ff_entry.redirect_direction=UPLL_FLOWFILTER_DIR_IN;
        }
        else
        {
          pfc_log_info("ACTION Redirect node OUT");
          val_entry.val_ff_entry.redirect_direction=UPLL_FLOWFILTER_DIR_OUT;
        }
        val_entry.val_ff_entry.valid[UPLL_IDX_REDIRECT_DIRECTION_FFE]=UNC_VF_VALID;

      }

       std::list<vbrif_flow_action>::iterator action_iter = iter->vbrif_flow_action_.begin();
      unc::odcdriver::OdcUtil util;

      while ( action_iter != iter->vbrif_flow_action_.end() ) {

        //For Every action, check if dscp or vlanpcp

        if (( action_iter->vbrif_dscp_.valid == true) &&
           ( action_iter->vbrif_dscp_.dscp_value != -1 )) {
            val_entry.val_ff_entry.dscp=action_iter->vbrif_dscp_.dscp_value;
            val_entry.val_ff_entry.valid[UPLL_IDX_DSCP_FFE]=UNC_VF_VALID;
        }
        if (( action_iter->vbrif_vlanpcp_.valid == true) &&
           ( action_iter->vbrif_vlanpcp_.vlan_pcp != -1 )) {
            val_entry.val_ff_entry.priority = action_iter->vbrif_vlanpcp_.vlan_pcp;
            val_entry.val_ff_entry.valid[UPLL_IDX_PRIORITY_FFE]=UNC_VF_VALID;
        }
        if (( action_iter->vbrif_dlsrc_.valid == true) &&
           ( action_iter->vbrif_dlsrc_.dlsrc_address != "" )) {
            util.convert_macstring_to_uint8(action_iter->vbrif_dlsrc_.dlsrc_address,
                                            &val_entry.val_ff_entry.modify_srcmac[0]);
            val_entry.val_ff_entry.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] = UNC_VF_VALID;
        }
        if (( action_iter->vbrif_dldst_.valid == true) &&
           ( action_iter->vbrif_dldst_.dldst_address != "" )) {
            util.convert_macstring_to_uint8(action_iter->vbrif_dldst_.dldst_address,
                                            &val_entry.val_ff_entry.modify_dstmac[0]);
            val_entry.val_ff_entry.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = UNC_VF_VALID;
        }
        action_iter++;
      }
      unc::vtndrvcache::ConfigNode *entry_cfgptr=
        new unc::vtndrvcache::CacheElementUtil<key_vbr_if_flowfilter_entry,
                    pfcdrv_val_flowfilter_entry, pfcdrv_val_flowfilter_entry, uint32_t>
      (&key_entry, &val_entry, &val_entry, uint32_t(UNC_OP_READ));
      cfgnode_vector.push_back(entry_cfgptr);
      iter++;
    }
    return UNC_RC_SUCCESS;
  }

UncRespCode
OdcVbrIfFlowFilterEntryCmd::create_cmd(key_vbr_if_flowfilter_entry& key,
                                         pfcdrv_val_flowfilter_entry& val,
                                         unc::driver::controller *ctr_ptr) {
    std::string vtn_name(reinterpret_cast<char*>(
                        key.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
    std::string vbr_name(reinterpret_cast<char*>(
                             key.flowfilter_key.if_key.vbr_key.vbridge_name));
    std::string if_name (reinterpret_cast<char*>(
                                          key.flowfilter_key.if_key.if_name));

    unc::odcdriver::odlutils::get_bridge_names(ctr_ptr,
        vtn_name,
        &bridges_);

    unc::odcdriver::odlutils::get_terminal_names(ctr_ptr,
        vtn_name,
        &terminals_);

    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr_ptr != NULL);

    vbrif_flowfilter_entry_class *req_obj = new vbrif_flowfilter_entry_class(
                                   ctr_ptr,vtn_name,vbr_name,if_name);
    ip_vbr_if_flowfilter st_obj;
    copy(st_obj, key, val);
    vbrif_flowfilter_entry_parser *parser_obj = new vbrif_flowfilter_entry_parser();
    json_object *jobj = parser_obj->create_req(st_obj);
    if (jobj == NULL){
      pfc_log_error("Error in create_request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
      }

    if(req_obj->set_put(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("VBRIF FlowFilterEntry Create Failed");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    delete req_obj;
    delete parser_obj;
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  OdcVbrIfFlowFilterEntryCmd::update_cmd(key_vbr_if_flowfilter_entry& key,
                                         pfcdrv_val_flowfilter_entry& val_old,
                                         pfcdrv_val_flowfilter_entry& val_new,
                                         unc::driver::controller *ctr_ptr) {
    std::string vtn_name(reinterpret_cast<char*>(
                           key.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
    std::string vbr_name(reinterpret_cast<char*>(
                               key.flowfilter_key.if_key.vbr_key.vbridge_name));
    std::string if_name (reinterpret_cast<char*>(
                                           key.flowfilter_key.if_key.if_name));
    char index[10];
    sprintf(index,"%d",key.sequence_num);
    std::string filter_type;
    if ( key.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_OUT)
      filter_type = "out";
    else
      filter_type = "in";

    unc::odcdriver::odlutils::get_bridge_names(ctr_ptr,
        vtn_name,
        &bridges_);

    unc::odcdriver::odlutils::get_terminal_names(ctr_ptr,
        vtn_name,
        &terminals_);


    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr_ptr != NULL);

    vbrif_flowfilter_entry_class *req_obj = new vbrif_flowfilter_entry_class(
                                   ctr_ptr,vtn_name,vbr_name,if_name);
    ip_vbr_if_flowfilter st_obj;
    copy(st_obj, key, val_old, val_new);
    vbrif_flowfilter_entry_parser *parser_obj = new vbrif_flowfilter_entry_parser();
    json_object *jobj = parser_obj->create_req(st_obj);
    if (jobj == NULL){
      pfc_log_error("Error in create_request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
      }

    if(req_obj->set_put(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("VBRIF FlowFilterEntry UPdate Failed");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    delete req_obj;
    delete parser_obj;
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  OdcVbrIfFlowFilterEntryCmd::delete_cmd(key_vbr_if_flowfilter_entry& key,
                                         pfcdrv_val_flowfilter_entry& val,
                                         unc::driver::controller *ctr_ptr) {
    std::string vtn_name(reinterpret_cast<char*>(
                           key.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
    std::string vbr_name(reinterpret_cast<char*>(
                               key.flowfilter_key.if_key.vbr_key.vbridge_name));
    std::string if_name (reinterpret_cast<char*>(
                                       key.flowfilter_key.if_key.if_name));

    ODC_FUNC_TRACE;
    PFC_ASSERT(ctr_ptr != NULL);

    vbrif_flowfilter_entry_class *req_obj = new vbrif_flowfilter_entry_class(
                                   ctr_ptr,vtn_name,vbr_name,if_name);
    ip_vbr_if_flowfilter st_obj;
    delete_request_body(st_obj,key,val);
    vbrif_flowfilter_entry_parser *parser_obj = new vbrif_flowfilter_entry_parser();
    json_object *jobj = parser_obj->del_req(st_obj);
    if (jobj == NULL){
      pfc_log_error("Error in delete_request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }

    if(req_obj->set_delete(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("VBRIF FlowFilterEntry Delete Failed");
      delete req_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    delete req_obj;
    return UNC_RC_SUCCESS;
  }



void OdcVbrIfFlowFilterEntryCmd::delete_request_body(
                        ip_vbr_if_flowfilter& ip_vbr_if_flowfilter_st,
                                   key_vbr_if_flowfilter_entry& key,
                                   pfcdrv_val_flowfilter_entry& val){

   ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.valid = true;
   ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.tenant_name =
             reinterpret_cast<char*>(
                   key.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name);
   ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.bridge_name =
             reinterpret_cast<char*>(
                        key.flowfilter_key.if_key.vbr_key.vbridge_name);
   ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.interface_name =
                   reinterpret_cast<char*>(key.flowfilter_key.if_key.if_name);

   vbrin_flow_filter match_index_;
   match_index_.index = key.sequence_num;
   ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.vbrin_flow_filter_.push_back(match_index_);

}

void OdcVbrIfFlowFilterEntryCmd::copy(
                                 ip_vbr_if_flowfilter&  ip_vbr_if_flowfilter_st,
                                        key_vbr_if_flowfilter_entry &key_in,
                                        pfcdrv_val_flowfilter_entry &value_in) {

    ODC_FUNC_TRACE;

    ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.valid = true;

    ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.tenant_name =
       reinterpret_cast<char*>(key_in.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name);
    ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.bridge_name =
       reinterpret_cast<char*>(key_in.flowfilter_key.if_key.vbr_key.vbridge_name);
    ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.interface_name =
        reinterpret_cast<char*>(key_in.flowfilter_key.if_key.if_name);

    if (key_in.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_OUT)
      ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.output = true;
    else if (key_in.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_IN)
      ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.output = false;

    vbrin_flow_filter flowfilter_;

    flowfilter_.index =key_in.sequence_num;

    flowfilter_.condition.assign(reinterpret_cast<char*>(
                              value_in.val_ff_entry.flowlist_name));

    if ( value_in.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID ) {
      if ( value_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_PASS ) {
        flowfilter_.vbrin_pass_filter_.valid = true;
      } else if ( value_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_DROP ) {
          flowfilter_.vbrin_drop_filter_.valid = true;
      } else if ( value_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_REDIRECT ) {
          flowfilter_.vbrin_redirect_filter_.valid = true;
        if ( value_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID ) {
          flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.valid = true;
          pfc_log_info("BRDIGE REDIRECT ENTERted");
          std::string redirect_node(reinterpret_cast <char *>(
                                                  value_in.val_ff_entry.redirect_node));
          pfc_log_info("BRDIGE REDIRECT %s", value_in.val_ff_entry.redirect_node);
          std::set <std::string>::iterator find_iter;
          find_iter = std::find (bridges_.begin(),bridges_.end(),redirect_node);

          if ( find_iter != bridges_.end()) {
            pfc_log_info("BRDIGE REDIRECT Created %s", value_in.val_ff_entry.redirect_node);
            flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.vbrin_bridge_name.assign(
              reinterpret_cast <char *>(value_in.val_ff_entry.redirect_node));
          } else {
            find_iter = std::find (terminals_.begin(),terminals_.end(),redirect_node);
            if ( find_iter != terminals_.end() )
               flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.vbrin_terminal_name.assign(
                reinterpret_cast <char *>(value_in.val_ff_entry.redirect_node));
          }
        }
        if ( value_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID ) {
           flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.vbrin_interface_name.assign(
            reinterpret_cast <char *>(value_in.val_ff_entry.redirect_port));
        }
        if ( value_in.val_ff_entry.redirect_direction == UPLL_FLOWFILTER_DIR_IN )
          flowfilter_.vbrin_redirect_filter_.redirect_output = false;
        else
          flowfilter_.vbrin_redirect_filter_.redirect_output = true;
      }
    }

    uint8_t order = 1;
    if ( value_in.val_ff_entry.valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID ) {
      vbrin_flow_action action_st;
      pfc_log_info("DSCP Action Created");
      action_st.vbrin_dscp_.valid = true;
      action_st.vbrin_dscp_.dscp_value  = value_in.val_ff_entry.dscp;
      action_st.order = order;
      order ++;
      flowfilter_.vbrin_flow_action_.push_back(action_st);
    }
    if ( value_in.val_ff_entry.valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID ) {
      vbrin_flow_action action_st;
      pfc_log_info("Priority Action Created");
      action_st.vbrin_vlanpcp_.valid = true;
      action_st.vbrin_vlanpcp_.vlan_pcp = value_in.val_ff_entry.priority;
      action_st.order = order;
      order ++;
      flowfilter_.vbrin_flow_action_.push_back(action_st);
    }
    if ( value_in.val_ff_entry.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID ) {
      vbrin_flow_action action_st;
      pfc_log_info(" DST MAC Action Created");
      action_st.vbrin_dldst_.valid = true;
      unc::odcdriver::OdcUtil util_;
      action_st.vbrin_dldst_.dldst_address =
             util_.macaddress_to_string(&value_in.val_ff_entry.modify_dstmac[0]);
      action_st.order = order;
      order ++;
      flowfilter_.vbrin_flow_action_.push_back(action_st);
    }
    if ( value_in.val_ff_entry.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID ) {
      vbrin_flow_action action_st;
      pfc_log_info(" SRC MAC Action Created");
      action_st.vbrin_dlsrc_.valid = true;
      unc::odcdriver::OdcUtil util_;
      action_st.vbrin_dlsrc_.dlsrc_address =
              util_.macaddress_to_string(&value_in.val_ff_entry.modify_srcmac[0]);
      action_st.order = order;
      order ++;
      flowfilter_.vbrin_flow_action_.push_back(action_st);
    }
   ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.vbrin_flow_filter_.push_back(flowfilter_);
  }
  //  Method to  handle two value structures during update operation
  void OdcVbrIfFlowFilterEntryCmd::copy(
                              ip_vbr_if_flowfilter& ip_vbr_if_flowfilter_st,
                                        key_vbr_if_flowfilter_entry &key_in,
                                        pfcdrv_val_flowfilter_entry &value_old_in,
                                        pfcdrv_val_flowfilter_entry &value_new_in) {

    ODC_FUNC_TRACE;

    ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.valid = true;

    ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.tenant_name =
       reinterpret_cast<char*>(
                 key_in.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name);
    ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.bridge_name =
       reinterpret_cast<char*>(
                      key_in.flowfilter_key.if_key.vbr_key.vbridge_name);
    ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.interface_name =
                 reinterpret_cast<char*>(key_in.flowfilter_key.if_key.if_name);
    if (key_in.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_OUT)
      ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.output = true;
    else if (key_in.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_IN)
      ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.output = false;

    vbrin_flow_filter flowfilter_;

    flowfilter_.index =key_in.sequence_num;

    flowfilter_.condition.assign(reinterpret_cast<char*>(
                              value_new_in.val_ff_entry.flowlist_name));

    flowfilter_.index=key_in.sequence_num;

    if (value_new_in.val_ff_entry.valid[UPLL_IDX_FLOWLIST_NAME_FFE] ==
                                               UNC_VF_VALID) {
      flowfilter_.condition.assign(reinterpret_cast<char*>(
                                   value_new_in.val_ff_entry.flowlist_name));
    } else if (value_new_in.val_ff_entry.valid[UPLL_IDX_FLOWLIST_NAME_FFE] ==
                                 UNC_VF_INVALID ||
            value_old_in.val_ff_entry.valid[UPLL_IDX_FLOWLIST_NAME_FFE] ==
                                                       UNC_VF_VALID) {
      flowfilter_.condition.assign(reinterpret_cast<char*>(
                                   value_old_in.val_ff_entry.flowlist_name));
    }

    if ( value_new_in.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] ==
                                                    UNC_VF_VALID ) {
      if ( value_new_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_PASS ) {
        flowfilter_.vbrin_pass_filter_.valid = true;
      } else if ( value_new_in.val_ff_entry.action ==
                                        UPLL_FLOWFILTER_ACT_DROP ) {
        flowfilter_.vbrin_drop_filter_.valid = true;
      } else if ( value_new_in.val_ff_entry.action ==
                                            UPLL_FLOWFILTER_ACT_REDIRECT ) {
        flowfilter_.vbrin_redirect_filter_.valid = true;
        if (value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
                                           UNC_VF_VALID ||
                value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
                           UNC_VF_VALUE_NOT_MODIFIED) {
          flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.valid =
                                                true;
          std::string redirect_node(reinterpret_cast <char *>(
                                    value_new_in.val_ff_entry.redirect_node));
          std::set <std::string>::iterator find_iter;
          find_iter = std::find (bridges_.begin(),bridges_.end(),redirect_node);
          if ( find_iter != bridges_.end()) {
            flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.vbrin_bridge_name.assign(
             reinterpret_cast <char *>(value_new_in.val_ff_entry.redirect_node));
          } else {
            find_iter = std::find (terminals_.begin(),terminals_.end(),
                                                        redirect_node);
            if ( find_iter != terminals_.end() )
              flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.vbrin_terminal_name.assign(
                reinterpret_cast <char *>(
                                      value_new_in.val_ff_entry.redirect_node));
          }
        }
        if (value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
                                                               UNC_VF_VALID ||
                 value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
                                   UNC_VF_VALUE_NOT_MODIFIED) {
          flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.vbrin_interface_name.assign(
            reinterpret_cast <char *>(value_new_in.val_ff_entry.redirect_port));
        }
        if ( value_new_in.val_ff_entry.redirect_direction ==
                                                      UPLL_FLOWFILTER_DIR_IN )
          flowfilter_.vbrin_redirect_filter_.redirect_output = false;
        else
          flowfilter_.vbrin_redirect_filter_.redirect_output = true;
      }
    } else if( value_old_in.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] ==
                                                       UNC_VF_VALID ) {
      if ( value_old_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_PASS ) {
        flowfilter_.vbrin_pass_filter_.valid = true;
      } else if ( value_old_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_DROP ) {
        flowfilter_.vbrin_drop_filter_.valid = true;
      } else if ( value_old_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_REDIRECT ) {
        flowfilter_.vbrin_redirect_filter_.valid = true;
        if (( value_old_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
                                                    UNC_VF_VALID ) ||
                  value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
                                   UNC_VF_VALUE_NOT_MODIFIED) {
          flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.valid =true ;
          std::string redirect_node(reinterpret_cast <char *>(
                                            value_new_in.val_ff_entry.redirect_node));
          std::set <std::string>::iterator find_iter;
          find_iter = std::find (bridges_.begin(),bridges_.end(),redirect_node);
          if ( find_iter != bridges_.end()) {
            flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.vbrin_bridge_name.assign(
              reinterpret_cast <char *>(value_new_in.val_ff_entry.redirect_node));
          } else {
            find_iter = std::find (terminals_.begin(),terminals_.end(),redirect_node);
            if ( find_iter != terminals_.end() )
              flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.vbrin_terminal_name.assign(
                reinterpret_cast <char *>(value_new_in.val_ff_entry.redirect_node));
          }
        }
        if (( value_old_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
                                                        UNC_VF_VALID )||
                value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
                                    UNC_VF_VALUE_NOT_MODIFIED ) {
          flowfilter_.vbrin_redirect_filter_.vbrin_redirect_destination_.vbrin_interface_name.assign(
            reinterpret_cast <char *>(value_new_in.val_ff_entry.redirect_port));
        }
        if ( value_old_in.val_ff_entry.redirect_direction == UPLL_FLOWFILTER_DIR_IN )
          flowfilter_.vbrin_redirect_filter_.redirect_output = false;
        else
          flowfilter_.vbrin_redirect_filter_.redirect_output = true;
      }
    }
    uint8_t order = 1;
    if ( value_new_in.val_ff_entry.valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID) {
      vbrin_flow_action  action_st;
      action_st.vbrin_dscp_.valid = true;
      action_st.vbrin_dscp_.dscp_value = value_new_in.val_ff_entry.dscp;
      action_st.order = order;
      order ++;
      pfc_log_error("New value structure of dscp attribute %d", value_new_in.val_ff_entry.dscp);
      flowfilter_.vbrin_flow_action_.push_back(action_st);
    } else if (value_new_in.val_ff_entry.valid[UPLL_IDX_DSCP_FFE] ==
           UNC_VF_INVALID && value_old_in.val_ff_entry.valid[UPLL_IDX_DSCP_FFE]
                                                             == UNC_VF_VALID ) {
        vbrin_flow_action  action_st;
        action_st.vbrin_dscp_.valid = true;
        action_st.vbrin_dscp_.dscp_value = value_old_in.val_ff_entry.dscp;
        action_st.order = order;
        order ++;
        pfc_log_error("New value structure of dscp attribute %d", value_old_in.val_ff_entry.dscp);
        flowfilter_.vbrin_flow_action_.push_back(action_st);
      } else {
    pfc_log_info("INVALID for new and old value structures of dscp attribute");
    }
    if ( value_new_in.val_ff_entry.valid[UPLL_IDX_PRIORITY_FFE] ==
            UNC_VF_VALID ) {
      vbrin_flow_action  action_st;
      action_st.vbrin_vlanpcp_.valid = true ;
      action_st.vbrin_vlanpcp_.vlan_pcp = value_new_in.val_ff_entry.priority;
      action_st.order = order;
      order ++;
      flowfilter_.vbrin_flow_action_.push_back(action_st);
    } else if (value_new_in.val_ff_entry.valid[UPLL_IDX_PRIORITY_FFE] ==
                  UNC_VF_INVALID && value_old_in.val_ff_entry.valid[
                                    UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID ) {
        vbrin_flow_action  action_st;
        action_st.vbrin_vlanpcp_.valid = true ;
        action_st.vbrin_vlanpcp_.vlan_pcp = value_old_in.val_ff_entry.priority;
        action_st.order = order;
        order ++;
        flowfilter_.vbrin_flow_action_.push_back(action_st);
      } else {
    pfc_log_info("INVALID for new and old value structures of PRIORITY ");
    }

    if ((value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] ==
              UNC_VF_VALID) ||
                (value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] ==
                                        UNC_VF_VALUE_NOT_MODIFIED)) {
      vbrin_flow_action  action_st;
      unc::odcdriver::OdcUtil util_;
      action_st.vbrin_dldst_.valid = true;
      action_st.vbrin_dldst_.dldst_address = util_.macaddress_to_string(
                               &value_new_in.val_ff_entry.modify_dstmac[0]);
      action_st.order = order;
      order ++;
      flowfilter_.vbrin_flow_action_.push_back(action_st);
    } else if (value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] ==
          UNC_VF_INVALID && value_old_in.val_ff_entry.valid[
                                UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID ) {
        vbrin_flow_action  action_st;
        action_st.vbrin_dldst_.valid = true;
        unc::odcdriver::OdcUtil util_;
        action_st.vbrin_dldst_.dldst_address = util_.macaddress_to_string(
                                    &value_old_in.val_ff_entry.modify_dstmac[0]);
        action_st.order = order;
        order ++;
        flowfilter_.vbrin_flow_action_.push_back(action_st);
     } else {
      pfc_log_info("INVALID for new and old va struct of DSTMACADDR attribute");
    }

   if ((value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] ==
            UNC_VF_VALID)||
          (value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] ==
                          UNC_VF_VALUE_NOT_MODIFIED)) {
      vbrin_flow_action  action_st;
      action_st.vbrin_dlsrc_.valid = true;
      unc::odcdriver::OdcUtil util_;
      action_st.vbrin_dlsrc_.dlsrc_address = util_.macaddress_to_string(
                                   &value_new_in.val_ff_entry.modify_srcmac[0]);
      action_st.order = order;
      order ++;
      flowfilter_.vbrin_flow_action_.push_back(action_st);
    } else if (value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] ==
                       UNC_VF_INVALID && value_old_in.val_ff_entry.valid[
                                UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID ) {
      vbrin_flow_action  action_st;
      action_st.vbrin_dlsrc_.valid = true;
      unc::odcdriver::OdcUtil util_;
      action_st.vbrin_dlsrc_.dlsrc_address = util_.macaddress_to_string(
                  &value_old_in.val_ff_entry.modify_srcmac[0]);
      action_st.order = order;
      order ++;
      flowfilter_.vbrin_flow_action_.push_back(action_st);
    } else {
      pfc_log_info("INVALID for new and old valstruct of SRCMACADDR attribute");
    }

   ip_vbr_if_flowfilter_st.input_vbrif_flow_filter_.vbrin_flow_filter_.push_back(flowfilter_);
  }
}
}

