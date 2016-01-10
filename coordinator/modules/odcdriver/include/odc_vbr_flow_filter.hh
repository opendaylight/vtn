/*
 * Copyright (c) 2014-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VBR_FLOWFILTER_HH__
#define __VBR_FLOWFILTER_HH__

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <json_build_parse.hh>
#include <vtn_conf_data_element_op.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <vbr_flow_filters.hh>
#include <vbr_flowfilter_entry.hh>
#include <string>
#include <list>
#include <algorithm>
#include <odc_kt_utils.hh>
#include <odc_util.hh>

namespace unc {
namespace odcdriver {

class OdcVbrFlowFilterCmd : public unc::driver::vtn_driver_command
  <key_vbr_flowfilter,val_flowfilter>

{
private:
  std::string parent_vtn_name_;
  std::string parent_vbr_name_;
  pfc_bool_t is_out;

public:
    OdcVbrFlowFilterCmd (unc::restjson::ConfFileValues_t conf_values):
    parent_vtn_name_(""),
    parent_vbr_name_(""),
    is_out(PFC_FALSE) {}
  UncRespCode
  create_cmd(key_vbr_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr);


  UncRespCode
  update_cmd(key_vbr_flowfilter& key, val_flowfilter& val_old,
             val_flowfilter& val_new,
             unc::driver::controller *ctr);


  UncRespCode
  delete_cmd(key_vbr_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr_ptr);


  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  void copy(flow_filter& flow_filter_st, key_vbr_flowfilter &key_in,
            val_flowfilter &value_in) {
    // Do Nothing as No Request Body is REquired
  }

  void copy(flow_filter& flow_filter_st, key_vbr_flowfilter &key_in,
            val_flowfilter &value_old_in,
             val_flowfilter &value_new_in) {
    // Do Nothing as No Request Body is REquired
  }

UncRespCode r_copy(std::list<flow_filter> &filter_detail,
                     std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);
};

class OdcVbrFlowFilterEntryCmd : public unc::driver::vtn_driver_command
  <key_vbr_flowfilter_entry,val_flowfilter_entry>
{
private:
  std::set <std::string> bridges_;
  std::set <std::string> terminals_;
  unc::restjson::ConfFileValues_t conf_values_;

public:
  OdcVbrFlowFilterEntryCmd(unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values) {}


  UncRespCode
  create_cmd(key_vbr_flowfilter_entry& key, val_flowfilter_entry& val,
             unc::driver::controller *ctr_ptr);

  UncRespCode
  update_cmd(key_vbr_flowfilter_entry& key, val_flowfilter_entry& val_old,
                 val_flowfilter_entry& val_new,
             unc::driver::controller *ctr_ptr);

  UncRespCode
  delete_cmd(key_vbr_flowfilter_entry& key, val_flowfilter_entry& val,
             unc::driver::controller *ctr_ptr);

void delete_request_body(ip_vbr_flowfilter&  ip_vbr_flowfilter_st,
                                                 key_vbr_flowfilter_entry& key,
                                                 val_flowfilter_entry& val);


  UncRespCode fetch_config(
         unc::driver::controller* ctr,
         void* parent_key,
         std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
         return UNC_RC_SUCCESS;
  }



  void copy(ip_vbr_flowfilter&  ip_vbr_flowfilter_st, key_vbr_flowfilter_entry &key_in,
            val_flowfilter_entry &value_in);

   //  Method to  handle two value structures during update operation
   void copy(ip_vbr_flowfilter&  ip_vbr_flowfilter_st, key_vbr_flowfilter_entry &key_in,
       val_flowfilter_entry &value_old_in, val_flowfilter_entry &value_new_in);

};

}
}

#endif
