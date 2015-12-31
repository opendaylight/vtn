/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_FLOWLIST_HH_
#define _ODC_FLOWLIST_HH_

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <flowlistentry.hh>
#include <odc_util.hh>
#include <odc_rest.hh>
#include <unc/upll_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <flowlist.hh>
#include <tclib_module.hh>
#include <vector>
#include <string>
#include <cstdlib>

namespace unc {
namespace odcdriver {
using namespace unc::restjson;
class OdcFlowListCommand: public
  unc::driver::vtn_driver_command <key_flowlist, val_flowlist>

{
public:
  /**
   * @brief                          - Parametrised Constructor
   * @param[in]                      - conf file values
   */
  explicit OdcFlowListCommand(unc::restjson::ConfFileValues_t conf_values):
    conf_file_values_(conf_values) {}

  /**
   * @brief Default Destructor
   */
  ~OdcFlowListCommand() {}

  UncRespCode create_cmd(key_flowlist &key_in, val_flowlist &val_in,
                         unc::driver::controller *ctr_ptr) ;

  UncRespCode delete_cmd(key_flowlist &key_in, val_flowlist &val_in,
                         unc::driver::controller *ctr_ptr);

  /**
   * method to construct request body
   */
  void create_request_body (key_flowlist &key_in, val_flowlist &val_in, flowlist& flowlist_st);

  UncRespCode update_cmd(key_flowlist &key_in, val_flowlist &val_old_in,
                         val_flowlist &val_new_in,
                         unc::driver::controller *ctr) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }
  /**
   * @brief      - Method to fetch child configurations for the parent kt
   * @param[in]  - controller pointer
   * @param[in]  - parent key type pointer
   * @param[out] - list of configurations
   * @retval     - UNC_RC_SUCCESS / UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

UncRespCode r_copy(std::list<flowlist> &flowlist_detail,
                           std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  void copy (ip_flowlist&  ip_flowlist_st,
             key_flowlist& in_key,
             val_flowlist& in_val);


void
delete_request_body(key_flowlist &key_in,
                                        val_flowlist &val_in,
                                        ip_flowlist&  ip_flowlist_st);
private:
  unc::restjson::ConfFileValues_t conf_file_values_;
};

class OdcFlowListEntryCommand: public
  unc::driver::vtn_driver_command <key_flowlist_entry,
  val_flowlist_entry>

{
public:
  /**
   * @brief                          - Parametrised Constructor
   * @param[in]                      - conf file values
   */
  explicit OdcFlowListEntryCommand(unc::restjson::ConfFileValues_t conf_values):
    conf_file_values_(conf_values) {}

  /**
   * @brief Default Destructor
   */
  ~OdcFlowListEntryCommand() {}

  UncRespCode create_cmd(key_flowlist_entry &key_in,
                         val_flowlist_entry &val_in,
                         unc::driver::controller *ctr);

  UncRespCode delete_cmd(key_flowlist_entry &key_in,
                         val_flowlist_entry &val_in,
                         unc::driver::controller *ctr);


  UncRespCode update_cmd(key_flowlist_entry &key_in,
                         val_flowlist_entry &val_old_in,
                         val_flowlist_entry &val_new_in,
                         unc::driver::controller *ctr) ;
  /**
   * @brief      - Method to fetch child configurations for the parent kt
   * @param[in]  - controller pointer
   * @param[in]  - parent key type pointer
   * @param[out] - list of configurations
   * @retval     - UNC_RC_SUCCESS / UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    ODC_FUNC_TRACE;

    return UNC_RC_SUCCESS;
  }

  void copy (ip_flowlistentry&  ip_flowlistentry_st,
             key_flowlist_entry& in_key,
             val_flowlist_entry& in_val);

  void copy (ip_flowlistentry&  ip_flowlistentry_st,
             key_flowlist_entry& in_key,
             val_flowlist_entry& old_val,
             val_flowlist_entry& new_val);
void
delete_request_body(key_flowlist_entry &key_in,
                                        val_flowlist_entry &val_in,
                                     ip_flowlistentry&  ip_flowlistentry_st);

private:
  unc::restjson::ConfFileValues_t conf_file_values_;
};

}  // namespace odcdriver
}  // namespace unc
#endif
