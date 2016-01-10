/*
 * Copyright (c) 2014-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __VBRIF_FLOWFILTER_HH__
#define __VBRIF_FLOWFILTER_HH__

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
#include <vbrif_flowfilter_entry.hh>
#include <vbrif_flow_filters.hh>
#include <algorithm>
#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>
#include <list>


namespace unc {
namespace odcdriver {

class OdcVbrIfFlowFilterCmd : public unc::driver::vtn_driver_command
  <key_vbr_if_flowfilter,pfcdrv_val_vbrif_vextif>

{
private:
  std::string parent_vtn_name_;
  std::string parent_vbr_name_;
  std::string parent_vbrif_name_;
  pfc_bool_t is_out;

public:
  OdcVbrIfFlowFilterCmd (unc::restjson::ConfFileValues_t conf_values):
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
  update_cmd(key_vbr_if_flowfilter& key, pfcdrv_val_vbrif_vextif& val_old,
             pfcdrv_val_vbrif_vextif& val_new,
             unc::driver::controller *ctr) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode
  delete_cmd(key_vbr_if_flowfilter& key, pfcdrv_val_vbrif_vextif& val,
             unc::driver::controller *ctr_ptr);

  UncRespCode fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

UncRespCode portmap_chcek( unc::driver::controller* ctr,
                                 std::string vtn_name, std::string vbr_name,
                                  std::string if_name,
              std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);


  UncRespCode r_copy(std::list<vbrif_flow_filter> &filter_detail,
                     std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);


};

class OdcVbrIfFlowFilterEntryCmd : public unc::driver::vtn_driver_command
  <key_vbr_if_flowfilter_entry,pfcdrv_val_flowfilter_entry>
{
private:
  unc::restjson::ConfFileValues_t conf_values_;
  std::set <std::string> bridges_;
  std::set <std::string> terminals_;

public:
  OdcVbrIfFlowFilterEntryCmd(unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values) {}


  UncRespCode
  create_cmd(key_vbr_if_flowfilter_entry& key, pfcdrv_val_flowfilter_entry& val,
             unc::driver::controller *ctr_ptr);

  UncRespCode
  update_cmd(key_vbr_if_flowfilter_entry& key, pfcdrv_val_flowfilter_entry& val_old,
             pfcdrv_val_flowfilter_entry& val_new,
             unc::driver::controller *ctr_ptr);
  UncRespCode
  delete_cmd(key_vbr_if_flowfilter_entry& key, pfcdrv_val_flowfilter_entry& val,
             unc::driver::controller *ctr_ptr);


 void delete_request_body(ip_vbr_if_flowfilter&  ip_vbr_if_flowfilter_st,
                     key_vbr_if_flowfilter_entry& key, pfcdrv_val_flowfilter_entry& val);

  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    return UNC_RC_SUCCESS;
  }

  void copy(ip_vbr_if_flowfilter&  ip_vbr_if_flowfilter_st, key_vbr_if_flowfilter_entry &key_in,
            pfcdrv_val_flowfilter_entry &value_in);
  //  Method to  handle two value structures during update operation
  void copy(ip_vbr_if_flowfilter&  ip_vbr_if_flowfilter_st, key_vbr_if_flowfilter_entry &key_in,
            pfcdrv_val_flowfilter_entry &value_old_in,
            pfcdrv_val_flowfilter_entry &value_new_in);

};

}
}

#endif
