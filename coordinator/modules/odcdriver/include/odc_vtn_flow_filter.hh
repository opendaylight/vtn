/*
 * Copyright (c) 2013-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __VTN_FLOWFILTER_HH__
#define __VTN_FLOWFILTER_HH__

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <json_build_parse.hh>
#include <vtn_conf_data_element_op.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <vtn_flowfilter_entry.hh>
#include <vtn_flow_filters.hh>
#include <tclib_module.hh>
#include <string>
#include <vector>
#include <sstream>
#include <list>


namespace unc {
namespace odcdriver {

class OdcVtnFlowFilterCmd : public unc::driver::vtn_driver_command
  <key_vtn_flowfilter,val_flowfilter>

{
private:
  std::string parent_vtn_name_;

public:
  OdcVtnFlowFilterCmd (unc::restjson::ConfFileValues_t conf_values):
    parent_vtn_name_("") {}
  UncRespCode
  create_cmd(key_vtn_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr);

  UncRespCode
  update_cmd(key_vtn_flowfilter& key, val_flowfilter& val_old,
             val_flowfilter& val_new,
             unc::driver::controller *ctr);

  UncRespCode
  delete_cmd(key_vtn_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr_ptr);

  UncRespCode fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  UncRespCode r_copy(std::list<par_flow_filter> &flow_detail,
                     std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

};

class OdcVtnFlowFilterEntryCmd : public unc::driver::vtn_driver_command
  <key_vtn_flowfilter_entry,val_vtn_flowfilter_entry>
{
private:
  std::set <std::string> vtns_;
  unc::restjson::ConfFileValues_t conf_values_;

public:
  OdcVtnFlowFilterEntryCmd(unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values) {}


  UncRespCode
  create_cmd(key_vtn_flowfilter_entry& key, val_vtn_flowfilter_entry& val,
             unc::driver::controller *ctr_ptr);

  UncRespCode
  update_cmd(key_vtn_flowfilter_entry& key, val_vtn_flowfilter_entry& val_old,
              val_vtn_flowfilter_entry& val_new,
             unc::driver::controller *ctr_ptr);

  UncRespCode
  delete_cmd(key_vtn_flowfilter_entry& key, val_vtn_flowfilter_entry& val,
             unc::driver::controller *ctr_ptr);


  UncRespCode fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    return UNC_RC_SUCCESS;
  }
   void delete_request_body(ip_vtn_flowfilter& ip_vtn_flowfilter_st,
                                                      key_vtn_flowfilter_entry& key,
                                                      val_vtn_flowfilter_entry& val);

  void copy(ip_vtn_flowfilter& ip_vtn_flowfilter_st, key_vtn_flowfilter_entry &key_in,
            val_vtn_flowfilter_entry &value_in);
 //  Method to  handle two value structures during update operation
 void copy(ip_vtn_flowfilter& ip_vtn_flowfilter_st, key_vtn_flowfilter_entry &key_in,
                      val_vtn_flowfilter_entry &value_old_in,
                      val_vtn_flowfilter_entry &value_new_in);

};

}
}

#endif
