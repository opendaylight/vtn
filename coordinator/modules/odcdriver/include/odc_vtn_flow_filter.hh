/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __VTN_FLOWFILTER_HH__
#define __VTN_FLOWFILTER_HH__

#include <odc_flowfilter_template.hh>
#include <string>
#include <list>

namespace unc {
namespace odcdriver {

class OdcVtnFlowFilterCmd : public unc::driver::vtn_driver_command
  <key_vtn_flowfilter,val_flowfilter>,
public OdcFlowFilterCmd<key_vtn_flowfilter,
  val_flowfilter>

{
private:
  std::string parent_vtn_name_;

public:
  OdcVtnFlowFilterCmd (unc::restjson::ConfFileValues_t conf_values):
    OdcFlowFilterCmd<key_vtn_flowfilter,val_flowfilter>(conf_values),
    parent_vtn_name_("") {}
  UncRespCode
  create_cmd(key_vtn_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr) {
    if ( key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
      return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  update_cmd(key_vtn_flowfilter& key, val_flowfilter& val_old,
             val_flowfilter& val_new,
             unc::driver::controller *ctr) {
    if ( key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
      return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  delete_cmd(key_vtn_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr) {
    if ( key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
      return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    return run_command(key,val,ctr,
                       unc::odcdriver::CONFIG_DELETE);
  }

  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

    key_vtn_t* parent_vtn = reinterpret_cast<key_vtn_t*> (parent_key);
    std::string url("");
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    url.append("/");
    url.append(reinterpret_cast<char*>(parent_vtn->vtn_name));
    url.append("/");
    url.append("flowfilters");
    parent_vtn_name_.assign(reinterpret_cast<char*>(parent_vtn->vtn_name));
    return odl_flow_filter_read_all(ctr,cfgnode_vector,url);
  }

  std::string get_url_tail(key_vtn_flowfilter &key_in,
                           val_flowfilter &val_in) {
    std::string url(reinterpret_cast<char*>(key_in.vtn_key.vtn_name));
    url.append("/");
    url.append("flowfilters");
    return url;
  }


  void copy(flowfilter *out, key_vtn_flowfilter &key_in,
            val_flowfilter &value_in) {
    // Do Nothing as No Request Body is REquired
  }

  void copy(flowfilter *out, key_vtn_flowfilter &key_in,
            val_flowfilter &value_old_in,
           val_flowfilter &value_new_in) {
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

    std::list<flowfilter*>::iterator entry_iter = in->flowfilter_.begin();

    while ( entry_iter != in->flowfilter_.end() ) {
      flowfilter *entry (*entry_iter);
      PFC_ASSERT( entry != NULL );
      key_vtn_flowfilter_entry key_entry;
      val_vtn_flowfilter_entry val_entry;

      memset(&key_entry,0,sizeof(key_vtn_flowfilter_entry));
      memset(&val_entry,0,sizeof(val_vtn_flowfilter_entry));

      // Key VTN Flow Filter Entry
      key_entry.sequence_num=entry->index_;
      memcpy(&key_entry.flowfilter_key,&key_filter,
             sizeof(key_vtn_flowfilter));

      // VAL VTN Flow Filter Entry
      strncpy(reinterpret_cast<char*> (val_entry.flowlist_name),
              entry->condition_.c_str(),sizeof(val_entry.flowlist_name) - 1);
      val_entry.valid[UPLL_IDX_SEQ_NUM_FFES] = UNC_VF_VALID;

      if ( entry->filterType_ == NULL )
        return UNC_DRV_RC_ERR_GENERIC;

      if ( entry->filterType_->pass_ ) {
        val_entry.action=UPLL_FLOWFILTER_ACT_PASS;
        val_entry.valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
      } else if ( entry->filterType_->drop_ ) {
        val_entry.action=UPLL_FLOWFILTER_ACT_DROP;
        val_entry.valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
      } else if ( entry->filterType_->redirect_) {
        val_entry.action=UPLL_FLOWFILTER_ACT_REDIRECT;
        val_entry.valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
      }

      std::list<action*>::iterator action_iter = entry->action_.begin();

      while ( action_iter != entry->action_.end() ) {

        //For Every action, check if dscp or vlanpcp

        action *act_entry(*action_iter);
        if ( act_entry->dscp_ ) {
          if ( act_entry->dscp_->dscp_ != -1 ) {
            val_entry.dscp=act_entry->dscp_->dscp_;
            val_entry.valid[UPLL_IDX_DSCP_VFFE]=UNC_VF_VALID;
          }
        } else if ( act_entry->vlanpcp_ ) {
          if ( act_entry->vlanpcp_->priority_ != -1 ) {
            val_entry.priority=act_entry->vlanpcp_->priority_;
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
      entry_iter++;
    }
    return UNC_RC_SUCCESS;
  }

};

class OdcVtnFlowFilterEntryCmd : public unc::driver::vtn_driver_command
  <key_vtn_flowfilter_entry,val_vtn_flowfilter_entry>,
public OdcFlowFilterCmd<key_vtn_flowfilter_entry,
  val_vtn_flowfilter_entry>
{
private:
  unc::restjson::ConfFileValues_t conf_values_;

public:
  OdcVtnFlowFilterEntryCmd(unc::restjson::ConfFileValues_t conf_values):
    OdcFlowFilterCmd<key_vtn_flowfilter_entry,val_vtn_flowfilter_entry>(conf_values),
    conf_values_(conf_values) {}


  UncRespCode
  create_cmd(key_vtn_flowfilter_entry& key, val_vtn_flowfilter_entry& val,
             unc::driver::controller *ctr) {
    if ( key.flowfilter_key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
      return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    return run_command(key,val,ctr,unc::odcdriver::CONFIG_UPDATE);
  }

  UncRespCode
  update_cmd(key_vtn_flowfilter_entry& key, val_vtn_flowfilter_entry& val_old,
              val_vtn_flowfilter_entry& val_new,
             unc::driver::controller *ctr) {
    if ( key.flowfilter_key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
      return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    return run_command(key,val_old,val_new,ctr,unc::odcdriver::CONFIG_UPDATE);
  }

  UncRespCode
  delete_cmd(key_vtn_flowfilter_entry& key, val_vtn_flowfilter_entry& val,
             unc::driver::controller *ctr) {
    if ( key.flowfilter_key.input_direction == UPLL_FLOWFILTER_DIR_OUT)
      return UNC_DRV_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    return run_command(key,val,ctr,unc::odcdriver::CONFIG_DELETE);
  }

  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    return UNC_RC_SUCCESS;
  }

  std::string get_url_tail(key_vtn_flowfilter_entry &key_in,
                           val_vtn_flowfilter_entry &val_in) {
    char index[10];
    std::string url(reinterpret_cast<char*>(key_in.flowfilter_key.vtn_key.vtn_name));
    url.append("/");
    url.append("flowfilters");
    url.append("/");
    sprintf(index,"%d",key_in.sequence_num);
    url.append(index);
    return url;
  }

  void copy(flowfilter *out, key_vtn_flowfilter_entry &key_in,
            val_vtn_flowfilter_entry &value_in) {

    ODC_FUNC_TRACE;
    PFC_ASSERT(out != NULL);

    out->index_=key_in.sequence_num;

    out->condition_.assign(reinterpret_cast<char*>(value_in.flowlist_name));

    if ( value_in.valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID ) {
      out->filterType_=new filterType();
      if ( value_in.action == UPLL_FLOWFILTER_ACT_PASS ) {
        out->filterType_->pass_=new pass();
      } else if ( value_in.action == UPLL_FLOWFILTER_ACT_DROP ) {
        out->filterType_->drop_=new drop();
      }
    } else {
      //Make FilterType as pass by default
      out->filterType_=new filterType();
      out->filterType_->pass_=new pass();
    }


    if ( value_in.valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID ) {
      action *new_action = new action();
      new_action->dscp_ = new dscp();
      new_action->dscp_->dscp_=value_in.dscp;
      out->action_.push_back(new_action);
    }
    if ( value_in.valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID ) {
      action *new_action = new action();
      new_action->vlanpcp_ = new vlanpcp();
      new_action->vlanpcp_->priority_ = value_in.priority;
      out->action_.push_back(new_action);
    }
  }
 //  Method to  handle two value structures during update operation
 void copy(flowfilter *out, key_vtn_flowfilter_entry &key_in,
                      val_vtn_flowfilter_entry &value_old_in,
                      val_vtn_flowfilter_entry &value_new_in) {

    ODC_FUNC_TRACE;
    PFC_ASSERT(out != NULL);

    out->index_=key_in.sequence_num;

    if (value_new_in.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    out->condition_.assign(reinterpret_cast<char*>(value_new_in.flowlist_name));
    } else if (value_new_in.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_INVALID 
           ||  value_old_in.valid[UPLL_IDX_FLOWLIST_NAME_FFE] == UNC_VF_VALID) {
    out->condition_.assign(reinterpret_cast<char*>(value_old_in.flowlist_name));
    }
    if ( value_new_in.valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID ) {
      out->filterType_=new filterType();
      if ( value_new_in.action == UPLL_FLOWFILTER_ACT_PASS ) {
        out->filterType_->pass_=new pass();
      } else if ( value_new_in.action == UPLL_FLOWFILTER_ACT_DROP ) {
        out->filterType_->drop_=new drop();
      }
    } else if ( value_old_in.valid[UPLL_IDX_ACTION_VFFE] == UNC_VF_VALID ) {
      out->filterType_=new filterType();
      if ( value_old_in.action == UPLL_FLOWFILTER_ACT_PASS ) {
        out->filterType_->pass_=new pass();
      } else if ( value_old_in.action == UPLL_FLOWFILTER_ACT_DROP ) {
        out->filterType_->drop_=new drop();
      }
    }  else {
      //Make FilterType as pass by default
      out->filterType_=new filterType();
      out->filterType_->pass_=new pass();
    }

   if ( value_new_in.valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID) {
      action *new_action = new action();
      new_action->dscp_ = new dscp();
      new_action->dscp_->dscp_=value_new_in.dscp;
      out->action_.push_back(new_action);
    } else if (value_new_in.valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_INVALID &&
                 value_old_in.valid[UPLL_IDX_DSCP_VFFE] == UNC_VF_VALID ) {
      action *new_action = new action();
      new_action->dscp_ = new dscp();
      new_action->dscp_->dscp_=value_old_in.dscp;
      out->action_.push_back(new_action);
      } else {
      pfc_log_info("INVALID for new and old val structures of dscp attribute");
    }
    if ( value_new_in.valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID) {
      action *new_action = new action();
      new_action->vlanpcp_ = new vlanpcp();
      new_action->vlanpcp_->priority_=value_new_in.priority;
      out->action_.push_back(new_action);
    } else if (value_new_in.valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_INVALID &&
                 value_old_in.valid[UPLL_IDX_PRIORITY_VFFE] == UNC_VF_VALID ) {
      action *new_action = new action();
      new_action->vlanpcp_ = new vlanpcp();
      new_action->vlanpcp_->priority_=value_old_in.priority;
      out->action_.push_back(new_action);
      } else {
      pfc_log_info("INVALID for new and old value structures of PRIORITY");
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
