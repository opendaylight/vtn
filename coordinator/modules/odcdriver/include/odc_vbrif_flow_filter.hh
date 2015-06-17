/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __VBRIF_FLOWFILTER_HH__
#define __VBRIF_FLOWFILTER_HH__

#include <odc_flowfilter_template.hh>
#include <string>
#include <list>
#include <algorithm>
#include <odc_kt_utils.hh>

namespace unc {
namespace odcdriver {

class OdcVbrIfFlowFilterCmd : public unc::driver::vtn_driver_command
  <key_vbr_if_flowfilter,pfcdrv_val_vbrif_vextif>,
public OdcFlowFilterCmd<key_vbr_if_flowfilter,
  pfcdrv_val_vbrif_vextif>

{
private:
  std::string parent_vtn_name_;
  std::string parent_vbr_name_;
  std::string parent_vbrif_name_;
  pfc_bool_t is_out;

public:
  OdcVbrIfFlowFilterCmd (unc::restjson::ConfFileValues_t conf_values):
    OdcFlowFilterCmd<key_vbr_if_flowfilter,pfcdrv_val_vbrif_vextif>(conf_values),
    parent_vtn_name_(""),
    parent_vbr_name_(""),
    parent_vbrif_name_(""),
    is_out(PFC_FALSE) {}
  UncRespCode
  create_cmd(key_vbr_if_flowfilter& key, pfcdrv_val_vbrif_vextif& val,
             unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  update_cmd(key_vbr_if_flowfilter& key, pfcdrv_val_vbrif_vextif& val,
             unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  delete_cmd(key_vbr_if_flowfilter& key, pfcdrv_val_vbrif_vextif& val,
             unc::driver::controller *ctr) {
    return run_command(key,val,ctr,
                       unc::odcdriver::CONFIG_DELETE);
  }

  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

    key_vbr_if_t *parent_vbr = reinterpret_cast<key_vbr_if_t *> (parent_key);
    std::string url("");
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    url.append("/");
    url.append(reinterpret_cast<char*>(parent_vbr->vbr_key.vtn_key.vtn_name));
    url.append("/");
    url.append("vbridges");
    url.append("/");
    url.append(reinterpret_cast<char*>(parent_vbr->vbr_key.vbridge_name));
    url.append("/");
    url.append("interfaces");
    url.append("/");
    url.append(reinterpret_cast<char*>(parent_vbr->if_name));
    url.append("/");
    url.append("flowfilters");
    url.append("/");
    url.append("in");
    parent_vtn_name_.assign(reinterpret_cast<char*>(parent_vbr->vbr_key.vtn_key.vtn_name));
    parent_vbr_name_.assign(reinterpret_cast<char*>(parent_vbr->vbr_key.vbridge_name));
    parent_vbrif_name_.assign(reinterpret_cast<char*>(parent_vbr->if_name));
    if (odl_flow_filter_read_all(ctr,cfgnode_vector,url) != UNC_RC_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;
    url.clear();
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    url.append("/");
    url.append(reinterpret_cast<char*>(parent_vbr->vbr_key.vtn_key.vtn_name));
    url.append("/");
    url.append("vbridges");
    url.append("/");
    url.append(reinterpret_cast<char*>(parent_vbr->vbr_key.vbridge_name));
    url.append("/");
    url.append("interfaces");
    url.append("/");
    url.append(reinterpret_cast<char*>(parent_vbr->if_name));
    url.append("/");
    url.append("flowfilters");
    url.append("/");
    url.append("out");
    is_out=PFC_TRUE;
    if (odl_flow_filter_read_all(ctr,cfgnode_vector,url) != UNC_RC_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;
    return UNC_RC_SUCCESS;
  }

  std::string get_url_tail(key_vbr_if_flowfilter &key_in,
                           pfcdrv_val_vbrif_vextif &val_in) {
    std::string vtn_name(reinterpret_cast<char*>(key_in.if_key.vbr_key.vtn_key.vtn_name));
    std::string vbr_name (reinterpret_cast<char*>(key_in.if_key.vbr_key.vbridge_name));
    std::string vbrif_name (reinterpret_cast<char*>(key_in.if_key.if_name));

    std::string url(vtn_name);
    url.append("/");
    url.append("vbridges");
    url.append("/");
    url.append(vbr_name);
    url.append("/");
    url.append("interfaces");
    url.append("/");
    url.append(vbrif_name);
    url.append("/");
    url.append("flowfilters");
    if ( key_in.direction == UPLL_FLOWFILTER_DIR_OUT)
      url.append("/out");
    else
      url.append("/in");
    return url;
  }


  void copy(flowfilter *out, key_vbr_if_flowfilter &key_in,
            pfcdrv_val_vbrif_vextif &value_in) {
    // Do Nothing as No Request Body is REquired
  }

  UncRespCode r_copy(flowfilterlist* in,
                     std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

    ODC_FUNC_TRACE;
    //Create both and FlowFilter and Entries and add to cache

    if ( in == NULL )
      return UNC_DRV_RC_ERR_GENERIC;

    if ( in->flowfilter_.size() == 0 ) {
      pfc_log_info("No Flow Entries");
      return UNC_RC_SUCCESS;
    }

    key_vbr_if_flowfilter key_filter;
    pfcdrv_val_vbrif_vextif val_filter;
    memset ( &key_filter, 0, sizeof(key_vbr_if_flowfilter));
    memset ( &val_filter, 0, sizeof(pfcdrv_val_vbrif_vextif));
    strncpy(reinterpret_cast<char*> (key_filter.if_key.vbr_key.vbridge_name),
            parent_vbr_name_.c_str(), sizeof(key_filter.if_key.vbr_key.vbridge_name) - 1);
    strncpy(reinterpret_cast<char*> (key_filter.if_key.vbr_key.vtn_key.vtn_name),
            parent_vtn_name_.c_str(), sizeof(key_filter.if_key.vbr_key.vtn_key.vtn_name) - 1);
    strncpy(reinterpret_cast<char*> (key_filter.if_key.if_name),
            parent_vbrif_name_.c_str(), sizeof(key_filter.if_key.if_name) - 1);
    if (is_out == PFC_FALSE)
      key_filter.direction=UPLL_FLOWFILTER_DIR_IN;
    else
      key_filter.direction=UPLL_FLOWFILTER_DIR_OUT;

    val_filter.valid[PFCDRV_IDX_INTERFACE_TYPE]=UNC_VF_VALID;
    val_filter.interface_type=PFCDRV_IF_TYPE_VBRIF;

    //Add to Cache
    unc::vtndrvcache::ConfigNode *filter_cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vbr_if_flowfilter, pfcdrv_val_vbrif_vextif, uint32_t>
    (&key_filter,&val_filter,uint32_t(UNC_OP_READ));

    cfgnode_vector.push_back(filter_cfgptr);

    std::list<flowfilter*>::iterator entry_iter = in->flowfilter_.begin();

    while ( entry_iter != in->flowfilter_.end() ) {
      flowfilter *entry (*entry_iter);
      PFC_ASSERT( entry != NULL );
      key_vbr_if_flowfilter_entry key_entry;
      pfcdrv_val_flowfilter_entry val_entry;

      memset(&key_entry,0,sizeof(key_vbr_if_flowfilter_entry));
      memset(&val_entry,0,sizeof(pfcdrv_val_flowfilter_entry));

      // Key VTN Flow Filter Entry
      key_entry.sequence_num=entry->index_;
      memcpy(&key_entry.flowfilter_key,&key_filter,
             sizeof(key_vbr_if_flowfilter));

      // VAL VTN Flow Filter Entry
      strncpy(reinterpret_cast<char*> (val_entry.val_ff_entry.flowlist_name),
              entry->condition_.c_str(),sizeof(val_entry.val_ff_entry.flowlist_name) - 1);
      val_entry.val_ff_entry.valid[UPLL_IDX_SEQ_NUM_FFES] = UNC_VF_VALID;

      if ( entry->filterType_ == NULL ) {
        pfc_log_error("Filter Type is Empty!!");
        return UNC_DRV_RC_ERR_GENERIC;
      }

      if ( entry->filterType_->pass_ ) {
        val_entry.val_ff_entry.action=UPLL_FLOWFILTER_ACT_PASS;
        val_entry.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
      } else if ( entry->filterType_->drop_ ) {
        val_entry.val_ff_entry.action=UPLL_FLOWFILTER_ACT_DROP;
        val_entry.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;
      } else if ( entry->filterType_->redirect_) {
        val_entry.val_ff_entry.action=UPLL_FLOWFILTER_ACT_REDIRECT;
        val_entry.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] = UNC_VF_VALID;

        if ( entry->filterType_->redirect_->destination_->bridge_ != "" ) {
          strncpy(reinterpret_cast<char*> (val_entry.val_ff_entry.redirect_node),
                  entry->filterType_->redirect_->destination_->bridge_.c_str(),
                  sizeof(val_entry.val_ff_entry.redirect_node) - 1);
          val_entry.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE]=UNC_VF_VALID;
        }
        if ( entry->filterType_->redirect_->destination_->terminal_ != "" ) {
          strncpy(reinterpret_cast<char*> (val_entry.val_ff_entry.redirect_node),
                  entry->filterType_->redirect_->destination_->terminal_.c_str(),
                  sizeof(val_entry.val_ff_entry.redirect_node) - 1);
          val_entry.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE]=UNC_VF_VALID;
        }
        if ( entry->filterType_->redirect_->destination_->interface_ != "" ) {
          strncpy(reinterpret_cast<char*> (val_entry.val_ff_entry.redirect_port),
                  entry->filterType_->redirect_->destination_->interface_.c_str(),
                  sizeof(val_entry.val_ff_entry.redirect_port) - 1);
          val_entry.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE]=UNC_VF_VALID;
        }

        if ( entry->filterType_->redirect_->output_ == false )
          val_entry.val_ff_entry.redirect_direction=UPLL_FLOWFILTER_DIR_OUT;
        else
          val_entry.val_ff_entry.redirect_direction=UPLL_FLOWFILTER_DIR_IN;
        val_entry.val_ff_entry.valid[UPLL_IDX_REDIRECT_DIRECTION_FFE]=UNC_VF_VALID;

      }

      std::list<action*>::iterator action_iter = entry->action_.begin();
      unc::odcdriver::OdcUtil util;

      while ( action_iter != entry->action_.end() ) {

        //For Every action, check if dscp or vlanpcp

        action *act_entry(*action_iter);
        if ( act_entry->dscp_ ) {
          if ( act_entry->dscp_->dscp_ != -1 ) {
            val_entry.val_ff_entry.dscp=act_entry->dscp_->dscp_;
            val_entry.val_ff_entry.valid[UPLL_IDX_DSCP_FFE]=UNC_VF_VALID;
          }
        } else if ( act_entry->vlanpcp_ ) {
          if ( act_entry->vlanpcp_->priority_ != -1 ) {
            val_entry.val_ff_entry.priority=act_entry->vlanpcp_->priority_;
            val_entry.val_ff_entry.valid[UPLL_IDX_PRIORITY_FFE]=UNC_VF_VALID;
          }
        } else if ( act_entry->dlsrc_ ) {
          if ( act_entry->dlsrc_->address_ != "" ) {
            util.convert_macstring_to_uint8(act_entry->dlsrc_->address_,
                                            &val_entry.val_ff_entry.modify_srcmac[0]);
            val_entry.val_ff_entry.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] = UNC_VF_VALID;
          }
        } else if ( act_entry->dldst_ ) {
          if ( act_entry->dldst_->address_ != "" ) {
            util.convert_macstring_to_uint8(act_entry->dldst_->address_,
                                            &val_entry.val_ff_entry.modify_dstmac[0]);
            val_entry.val_ff_entry.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] = UNC_VF_VALID;
          }
        }
        action_iter++;
      }
      unc::vtndrvcache::ConfigNode *entry_cfgptr=
        new unc::vtndrvcache::CacheElementUtil<key_vbr_if_flowfilter_entry, pfcdrv_val_flowfilter_entry, uint32_t>
      (&key_entry,&val_entry,uint32_t(UNC_OP_READ));
      cfgnode_vector.push_back(entry_cfgptr);
      entry_iter++;
    }
    return UNC_RC_SUCCESS;
  }

};

class OdcVbrIfFlowFilterEntryCmd : public unc::driver::vtn_driver_command
  <key_vbr_if_flowfilter_entry,pfcdrv_val_flowfilter_entry>,
public OdcFlowFilterCmd<key_vbr_if_flowfilter_entry,
  pfcdrv_val_flowfilter_entry>
{
private:
  unc::restjson::ConfFileValues_t conf_values_;
  std::set <std::string> bridges_;
  std::set <std::string> terminals_;

public:
  OdcVbrIfFlowFilterEntryCmd(unc::restjson::ConfFileValues_t conf_values):
    OdcFlowFilterCmd<key_vbr_if_flowfilter_entry,pfcdrv_val_flowfilter_entry>(conf_values),
    conf_values_(conf_values) {}


  UncRespCode
  create_cmd(key_vbr_if_flowfilter_entry& key, pfcdrv_val_flowfilter_entry& val,
             unc::driver::controller *ctr) {
    std::string vtn_name(reinterpret_cast<char*>(key.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
    unc::odcdriver::odlutils::get_vbridge_names(ctr,
        conf_values_,
        vtn_name,
        &bridges_);

    unc::odcdriver::odlutils::get_vterm_names(ctr,
        conf_values_,
        vtn_name,
        &terminals_);

    return run_command(key,val,ctr,unc::odcdriver::CONFIG_UPDATE);
  }

  UncRespCode
  update_cmd(key_vbr_if_flowfilter_entry& key, pfcdrv_val_flowfilter_entry& val,
             unc::driver::controller *ctr) {
    std::string vtn_name(reinterpret_cast<char*>(key.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
    unc::odcdriver::odlutils::get_vbridge_names(ctr,
        conf_values_,
        vtn_name,
        &bridges_);

    unc::odcdriver::odlutils::get_vterm_names(ctr,
        conf_values_,
        vtn_name,
        &terminals_);
    return run_command(key,val,ctr,unc::odcdriver::CONFIG_UPDATE);
  }

  UncRespCode
  delete_cmd(key_vbr_if_flowfilter_entry& key, pfcdrv_val_flowfilter_entry& val,
             unc::driver::controller *ctr) {
    return run_command(key,val,ctr,unc::odcdriver::CONFIG_DELETE);
  }

  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    return UNC_RC_SUCCESS;
  }

  std::string get_url_tail(key_vbr_if_flowfilter_entry &key_in,
                           pfcdrv_val_flowfilter_entry &val_in) {
    char index[10];
    std::string vtn_name(reinterpret_cast<char*>(key_in.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
    std::string vbridge_name(reinterpret_cast<char*>(key_in.flowfilter_key.if_key.vbr_key.vbridge_name));
    std::string vbrif_name(reinterpret_cast<char*>(key_in.flowfilter_key.if_key.if_name));
    std::string url(vtn_name);
    url.append("/");
    url.append("vbridges");
    url.append("/");
    url.append(vbridge_name);
    url.append("/");
    url.append("interfaces");
    url.append("/");
    url.append(vbrif_name);
    url.append("/");
    url.append("flowfilters");
    url.append("/");
    if ( key_in.flowfilter_key.direction == UPLL_FLOWFILTER_DIR_OUT)
      url.append("out");
    else
      url.append("in");
    url.append("/");
    sprintf(index,"%d",key_in.sequence_num);
    url.append(index);
    return url;
  }

  void copy(flowfilter *out, key_vbr_if_flowfilter_entry &key_in,
            pfcdrv_val_flowfilter_entry &value_in) {

    ODC_FUNC_TRACE;
    PFC_ASSERT(out != NULL);

    out->index_=key_in.sequence_num;

    out->condition_.assign(reinterpret_cast<char*>(value_in.val_ff_entry.flowlist_name));

    if ( value_in.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] == UNC_VF_VALID ) {
      out->filterType_=new filterType();
      if ( value_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_PASS ) {
        out->filterType_->pass_=new pass();
      } else if ( value_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_DROP ) {
        out->filterType_->drop_=new drop();
      } else if ( value_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_REDIRECT ) {
        out->filterType_->redirect_=new redirect();
        if ( value_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE] == UNC_VF_VALID ) {
          out->filterType_->redirect_->destination_ = new destination();
          std::string redirect_node(reinterpret_cast <char *>(value_in.val_ff_entry.redirect_node));
          std::set <std::string>::iterator find_iter;
          find_iter = std::find (bridges_.begin(),bridges_.end(),redirect_node);

          if ( find_iter != bridges_.end()) {
            out->filterType_->redirect_->destination_->bridge_.assign(
              reinterpret_cast <char *>(value_in.val_ff_entry.redirect_node));
          } else {
            find_iter = std::find (terminals_.begin(),terminals_.end(),redirect_node);
            if ( find_iter != terminals_.end() )
              out->filterType_->redirect_->destination_->terminal_.assign(
                reinterpret_cast <char *>(value_in.val_ff_entry.redirect_node));
          }
          out->filterType_->redirect_->destination_->tenant_.assign(
            reinterpret_cast<char*>(key_in.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
        }
        if ( value_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE] == UNC_VF_VALID ) {
          out->filterType_->redirect_->destination_->interface_.assign(
            reinterpret_cast <char *>(value_in.val_ff_entry.redirect_port));
        }
        if ( value_in.val_ff_entry.redirect_direction == UPLL_FLOWFILTER_DIR_IN )
          out->filterType_->redirect_->output_=false;
        else
          out->filterType_->redirect_->output_=true;
      }
    } else {
      //Make FilterType as pass by default
      out->filterType_=new filterType();
      out->filterType_->pass_=new pass();
    }

    if ( value_in.val_ff_entry.valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID ) {
      action *new_action = new action();
      new_action->dscp_ = new dscp();
      new_action->dscp_->dscp_=value_in.val_ff_entry.dscp;
      out->action_.push_back(new_action);
    }
    if ( value_in.val_ff_entry.valid[UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID ) {
      action *new_action = new action();
      new_action->vlanpcp_ = new vlanpcp();
      new_action->vlanpcp_->priority_ = value_in.val_ff_entry.priority;
      out->action_.push_back(new_action);
    }
    if ( value_in.val_ff_entry.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID ) {
      action *new_action = new action();
      new_action->dldst_ = new dldst();
      unc::odcdriver::OdcUtil util_;
      new_action->dldst_->address_=util_.macaddress_to_string(&value_in.val_ff_entry.modify_dstmac[0]);
      out->action_.push_back(new_action);
    }
    if ( value_in.val_ff_entry.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID ) {
      action *new_action = new action();
      new_action->dlsrc_ = new dlsrc();
      unc::odcdriver::OdcUtil util_;
      new_action->dlsrc_->address_=util_.macaddress_to_string(&value_in.val_ff_entry.modify_srcmac[0]);
      out->action_.push_back(new_action);
    }
  }
  //  Method to  handle two value structures during update operation
  void copy(flowfilter *out, key_vbr_if_flowfilter_entry &key_in,
            pfcdrv_val_flowfilter_entry &value_old_in,
            pfcdrv_val_flowfilter_entry &value_new_in) {

    ODC_FUNC_TRACE;
    PFC_ASSERT(out != NULL);

    out->index_=key_in.sequence_num;

    if (value_new_in.val_ff_entry.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    out->condition_.assign(reinterpret_cast<char*>(value_new_in.val_ff_entry.flowlist_name));
    } else if (value_new_in.val_ff_entry.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_INVALID
           || value_old_in.val_ff_entry.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    out->condition_.assign(reinterpret_cast<char*>(value_old_in.val_ff_entry.flowlist_name));
    }

    if ( value_new_in.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] ==
                                                    UNC_VF_VALID ) {
      out->filterType_=new filterType();
      if ( value_new_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_PASS ) {
        out->filterType_->pass_=new pass();
      } else if ( value_new_in.val_ff_entry.action ==
                                        UPLL_FLOWFILTER_ACT_DROP ) {
        out->filterType_->drop_=new drop();
      } else if ( value_new_in.val_ff_entry.action ==
                                            UPLL_FLOWFILTER_ACT_REDIRECT ) {
        out->filterType_->redirect_=new redirect();
        if (value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
                                           UNC_VF_VALID ||
                value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
                           UNC_VF_VALUE_NOT_MODIFIED) {
          out->filterType_->redirect_->destination_ = new destination();
          std::string redirect_node(reinterpret_cast <char *>(
                                    value_new_in.val_ff_entry.redirect_node));
          std::set <std::string>::iterator find_iter;
          find_iter = std::find (bridges_.begin(),bridges_.end(),redirect_node);
          if ( find_iter != bridges_.end()) {
            out->filterType_->redirect_->destination_->bridge_.assign(
             reinterpret_cast <char *>(value_new_in.val_ff_entry.redirect_node));
          } else {
            find_iter = std::find (terminals_.begin(),terminals_.end(),
                                                        redirect_node);
            if ( find_iter != terminals_.end() )
              out->filterType_->redirect_->destination_->terminal_.assign(
                reinterpret_cast <char *>(
                                      value_new_in.val_ff_entry.redirect_node));
          }
          out->filterType_->redirect_->destination_->tenant_.assign(
            reinterpret_cast<char*>(
                     key_in.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
        }
        if (value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
                                                               UNC_VF_VALID ||
                 value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
                                   UNC_VF_VALUE_NOT_MODIFIED) {
          out->filterType_->redirect_->destination_->interface_.assign(
            reinterpret_cast <char *>(value_new_in.val_ff_entry.redirect_port));
        }
        if ( value_new_in.val_ff_entry.redirect_direction ==
                                                      UPLL_FLOWFILTER_DIR_IN )
          out->filterType_->redirect_->output_=false;
        else
          out->filterType_->redirect_->output_=true;
      }
    } else if( value_old_in.val_ff_entry.valid[UPLL_IDX_ACTION_FFE] ==
                                                       UNC_VF_VALID ) {
      out->filterType_=new filterType();
      if ( value_old_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_PASS ) {
        out->filterType_->pass_=new pass();
      } else if ( value_old_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_DROP ) {
        out->filterType_->drop_=new drop();
      } else if ( value_old_in.val_ff_entry.action == UPLL_FLOWFILTER_ACT_REDIRECT ) {
        out->filterType_->redirect_=new redirect();
        if ( value_old_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
                                                    UNC_VF_VALID  ||
                  value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
                                   UNC_VF_VALUE_NOT_MODIFIED) {
          out->filterType_->redirect_->destination_ = new destination();
          std::string redirect_node(reinterpret_cast <char *>(
                                            value_new_in.val_ff_entry.redirect_node));
          std::set <std::string>::iterator find_iter;
          find_iter = std::find (bridges_.begin(),bridges_.end(),redirect_node);
          if ( find_iter != bridges_.end()) {
            out->filterType_->redirect_->destination_->bridge_.assign(
              reinterpret_cast <char *>(value_new_in.val_ff_entry.redirect_node));
          } else {
            find_iter = std::find (terminals_.begin(),terminals_.end(),redirect_node);
            if ( find_iter != terminals_.end() )
              out->filterType_->redirect_->destination_->terminal_.assign(
                reinterpret_cast <char *>(value_new_in.val_ff_entry.redirect_node));
          }
          out->filterType_->redirect_->destination_->tenant_.assign(
            reinterpret_cast<char*>(
                        key_in.flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
        }
        if (( value_old_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
                                                        UNC_VF_VALID )||
                value_new_in.val_ff_entry.valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
                                    UNC_VF_VALUE_NOT_MODIFIED ) {
          out->filterType_->redirect_->destination_->interface_.assign(
            reinterpret_cast <char *>(value_new_in.val_ff_entry.redirect_port));
        }
        if ( value_old_in.val_ff_entry.redirect_direction == UPLL_FLOWFILTER_DIR_IN )
          out->filterType_->redirect_->output_=false;
        else
          out->filterType_->redirect_->output_=true;
      }
    }  else {
      //Make FilterType as pass by default
      out->filterType_=new filterType();
      out->filterType_->pass_=new pass();
    }
    if ( value_new_in.val_ff_entry.valid[UPLL_IDX_DSCP_FFE] == UNC_VF_VALID ){
      action *new_action = new action();
      new_action->dscp_ = new dscp();
      new_action->dscp_->dscp_=value_new_in.val_ff_entry.dscp;
      out->action_.push_back(new_action);
    } else if (value_new_in.val_ff_entry.valid[UPLL_IDX_DSCP_FFE] ==
           UNC_VF_INVALID && value_old_in.val_ff_entry.valid[UPLL_IDX_DSCP_FFE]
                                                             == UNC_VF_VALID ) {
        action *new_action = new action();
        new_action->dscp_ = new dscp();
        new_action->dscp_->dscp_=value_old_in.val_ff_entry.dscp;
        out->action_.push_back(new_action);
      } else {
    pfc_log_info("INVALID for new and old value structures of dscp attribute");
    }
    if ( value_new_in.val_ff_entry.valid[UPLL_IDX_PRIORITY_FFE] ==  UNC_VF_VALID) {
      action *new_action = new action();
      new_action->vlanpcp_ = new vlanpcp();
      new_action->vlanpcp_->priority_=value_new_in.val_ff_entry.priority;
      out->action_.push_back(new_action);
    } else if (value_new_in.val_ff_entry.valid[UPLL_IDX_PRIORITY_FFE] ==
                  UNC_VF_INVALID && value_old_in.val_ff_entry.valid[
                                    UPLL_IDX_PRIORITY_FFE] == UNC_VF_VALID ) {
        action *new_action = new action();
        new_action->vlanpcp_ = new vlanpcp();
        new_action->vlanpcp_->priority_=value_old_in.val_ff_entry.priority;
        out->action_.push_back(new_action);
      } else {
    pfc_log_info("INVALID for new and old value structures of PRIORITY ");
    }

    if (value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID ||
                value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] ==
                                        UNC_VF_VALUE_NOT_MODIFIED) {
      action *new_action = new action();
      new_action->dldst_ = new dldst();
      unc::odcdriver::OdcUtil util_;
      new_action->dldst_->address_= util_.macaddress_to_string(
                               &value_new_in.val_ff_entry.modify_dstmac[0]);
      out->action_.push_back(new_action);
    } else if (value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_DST_MAC_FFE] ==
          UNC_VF_INVALID && value_old_in.val_ff_entry.valid[
                                UPLL_IDX_MODIFY_DST_MAC_FFE] == UNC_VF_VALID ) {
      action *new_action = new action();
      new_action->dldst_ = new dldst();
      unc::odcdriver::OdcUtil util_;
      new_action->dldst_->address_=util_.macaddress_to_string(
                                    &value_old_in.val_ff_entry.modify_dstmac[0]);
      out->action_.push_back(new_action);
     } else {
      pfc_log_info("INVALID for new and old va struct of DSTMACADDR attribute");
    }

   if (value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID
                  || value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] ==
                          UNC_VF_VALUE_NOT_MODIFIED) {
      action *new_action = new action();
      new_action->dlsrc_ = new dlsrc();
      unc::odcdriver::OdcUtil util_;
      new_action->dlsrc_->address_=util_.macaddress_to_string(
                                   &value_new_in.val_ff_entry.modify_srcmac[0]);
      out->action_.push_back(new_action);
    } else if (value_new_in.val_ff_entry.valid[UPLL_IDX_MODIFY_SRC_MAC_FFE] ==
                       UNC_VF_INVALID && value_old_in.val_ff_entry.valid[
                                UPLL_IDX_MODIFY_SRC_MAC_FFE] == UNC_VF_VALID ) {
      action *new_action = new action();
      new_action->dlsrc_ = new dlsrc();
      unc::odcdriver::OdcUtil util_;
      new_action->dlsrc_->address_=util_.macaddress_to_string(
                  &value_old_in.val_ff_entry.modify_srcmac[0]);
      out->action_.push_back(new_action);
    } else {
      pfc_log_info("INVALID for new and old valstruct of SRCMACADDR attribute");
    }
  }

  UncRespCode r_copy(flowfilterlist* in,
                     std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    //Never Invoked as FectConfig is not implemented!!
    return UNC_RC_SUCCESS;
  }
};

}
}

#endif
