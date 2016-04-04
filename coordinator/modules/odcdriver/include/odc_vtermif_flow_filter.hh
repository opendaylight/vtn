/*
 * Copyright (c) 2014-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __VTERMIF_FLOWFILTER_HH__
#define __VTERMIF_FLOWFILTER_HH__

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <odc_util.hh>
#include <odc_rest.hh>
#include <odc_kt_utils.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <tclib_module.hh>
#include <vtermif_flowfilter_entry.hh>
#include <vtermif_flow_filters.hh>
#include <string>
#include <list>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <vector>


namespace unc {
namespace odcdriver {

class OdcVTermIfFlowFilterCmd : public unc::driver::vtn_driver_command
  <key_vterm_if_flowfilter,val_flowfilter>

{
private:
  std::string parent_vtn_name_;
  std::string parent_vterm_name_;
  std::string parent_vtermif_name_;
  pfc_bool_t is_out;

public:
  OdcVTermIfFlowFilterCmd (unc::restjson::ConfFileValues_t conf_values):
    parent_vtn_name_(""),
    parent_vterm_name_(""),
    parent_vtermif_name_(""),
    is_out(PFC_FALSE) {}
  UncRespCode
  create_cmd(key_vterm_if_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  update_cmd(key_vterm_if_flowfilter& key, val_flowfilter& val_old,
          val_flowfilter& val_new,
             unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }
//add the delete
  UncRespCode
  delete_cmd(key_vterm_if_flowfilter& key, val_flowfilter& val,
             unc::driver::controller *ctr_ptr);

  UncRespCode fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);


   UncRespCode portmap_check(unc::driver::controller *ctr,
                             std::string vtn_name,
                             std::string vterm_name,
                             std::string vtermif_name,
            std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

   UncRespCode r_copy(std::list<vtermif_flow_filter> &filter_detail,
                      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
            std::string vtn_name, std::string vterm_name, std::string vtermif_name);
};

class OdcVTermIfFlowFilterEntryCmd : public unc::driver::vtn_driver_command
  <key_vterm_if_flowfilter_entry,val_flowfilter_entry>
{
private:
  unc::restjson::ConfFileValues_t conf_values_;
  std::set <std::string> bridges_;
  std::set <std::string> terminals_;

public:
  OdcVTermIfFlowFilterEntryCmd(unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values) {}


  UncRespCode
  create_cmd(key_vterm_if_flowfilter_entry& key, val_flowfilter_entry& val,
              unc::driver::controller *ctr_ptr);


  UncRespCode
  update_cmd(key_vterm_if_flowfilter_entry& key, val_flowfilter_entry& val_old,
             val_flowfilter_entry& val_new,unc::driver::controller *ctr_ptr);


  UncRespCode
  delete_cmd(key_vterm_if_flowfilter_entry& key, val_flowfilter_entry& val,
             unc::driver::controller *ctr_ptr);


  void delete_request_body(ip_vterm_if_flowfilter& ip_vterm_if_flowfilter_st,
                           key_vterm_if_flowfilter_entry& key,
                           val_flowfilter_entry& val);

  UncRespCode fetch_config(
       unc::driver::controller* ctr_ptr,
       void* parent_key,
       std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector){

    return UNC_RC_SUCCESS;
   }

  void copy(ip_vterm_if_flowfilter& ip_vterm_if_flowfilter_st, key_vterm_if_flowfilter_entry &key_in,
             val_flowfilter_entry &value_in);

  //  Method to  handle two value structures during update operation
  void copy(ip_vterm_if_flowfilter& ip_vterm_if_flowfilter_st, key_vterm_if_flowfilter_entry &key_in,
            val_flowfilter_entry &value_old_in,
             val_flowfilter_entry &value_new_in);

};
}
}

#endif
