/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_MOD_HH__
#define _ODC_MOD_HH__

#include <odc_vtn.hh>
#include <odc_vbr.hh>
#include <odc_vbrif.hh>
#include <odc_switch.hh>
#include <odc_port.hh>
#include <odc_link.hh>
#include <odc_vbr_vlanmap.hh>
#include <odc_dataflow.hh>
#include <odc_ctr_dataflow.hh>
#include <odc_vtn_dataflow.hh>
#include <odc_vtnstation.hh>
#include <odc_vterminal.hh>
#include <odc_vterminal_if.hh>
#include <odc_flowlist.hh>
#include <odc_driver_common_defs.hh>
#include <odc_vtn_flow_filter.hh>
#include <odc_vbr_flow_filter.hh>
#include <odc_vbrif_flow_filter.hh>
#include <odc_vtermif_flow_filter.hh>
#include <unc/keytype.h>
#include <rest_util.hh>
#include <arpa/inet.h>
#include <unc/tc/external/tc_services.h>
#include <string>

namespace unc {
namespace odcdriver {

class ODCModule: public pfc::core::Module, public unc::driver::driver {
 public:
  /**
   * @brief     - Paramaretrised Constructor
   * @param[in] - pfc_modattr obj
   */
  explicit ODCModule(const pfc_modattr_t*& obj)
      : Module(obj),
        ping_interval(0),
        conf_file_values_() { }
  /**
   * @brief     - Gets the controller type
   * @param[in] - unc_keytype_ctrtype_t enum
   * @return    - returns Enum of type unc_keytype_ctrtype_t
   */
  unc_keytype_ctrtype_t get_controller_type();

  /**
   * @brief  - init method - Get Instance of vtndrvinntf & Register into RegisterDriver
   * @return - returns PFC_TRUE on success
   *         returns  PFC_FALSE on failure
   */
  pfc_bool_t init();

  /**
   * @brief  - Fini
   * @return - returns PFC_TRUE on success/
   *         returns PFC_FALSE on failure
   */
  pfc_bool_t fini();

  /**
   * @brief   - Is 2phase commit supported or not
   * @return  - returns PFC_TRUE if 2phase commit support is needed
   *            returns PFC_FALSE if 2 phase commit is not required.
   */
  pfc_bool_t is_2ph_commit_support_needed();

  /**
   * @brief  - Is audit collection needed or not
   * @return - returns PFC_TRUE if audit collection is required/returns PFC_FALSE
   *           if audit collection is not required.
   */
  pfc_bool_t is_audit_collection_needed();

  /**
   * @brief             - Creates Controller pointer with specific values and return
   * @param[in] key_ctr - Controller key structure
   * @param[in] val_ctr - Controller value structute
   * @return            - returns new added controller pointer
   */
  unc::driver::controller* add_controller(const key_ctr_t& key_ctr,
                                          const val_ctr_t& val_ctr);

  /**
   * @brief               - Updates Controller pointer with specific values and return
   * @param[in] key_ctr   - Controller key structure
   * @param[in] val_ctr   - Controller value structute
   * @param[out] ctrl_inst- Controller pointer
   * @return              - returns PFC_TRUE
   */
  pfc_bool_t update_controller(const key_ctr_t& key_ctr,
                               const val_ctr_t& val_ctr,
                               unc::driver::controller* ctrl_inst);

  /**
   * @brief                 - Deletes Controller pointer with specific values
   * @param[in] delete_inst - Controller pointer
   * @return                - returns PFC_TRUE if the controller is deleted/ returns PFC_FALSE
   *                          if controller is not deleted
   */
  pfc_bool_t delete_controller(unc::driver::controller* delete_inst);

  /**
   * @brief                    - Gets the driver command as per the keytype
   * @param[in] key_type       - unc Keytype
   * @return  driver_command*  - returns corresponding instance driver_command* of the
   *                             key type
   */
  unc::driver::driver_command* create_driver_command(unc_key_type_t key_type);
  unc::driver::vtn_driver_read_command* create_driver_read_command(
                                                   unc_key_type_t key_type);

  /**
   * @brief     -  HandleVote
   * @param[in] -  Controller pointer
   * @return    -  returns unc::tclib::TcCommonRet - enum value
   */
  unc::tclib::TcCommonRet HandleVote(unc::driver::controller*);

  /**
   * @brief     - Handle the commit
   * @param[in] - Controller pointer
   * @return    -  returns unc::tclib::TcCommonRet - enum value
   */
  unc::tclib::TcCommonRet HandleCommit(unc::driver::controller*);

  /**
   * @brief     - Handle the abort
   * @param[in] - Controller pointer
   * @return    - returns unc::tclib::TcCommonRet - enum value
   */
  unc::tclib::TcCommonRet HandleAbort(unc::driver::controller*);

  /**
   * @brief   - Ping needed for the ODC Conttoller or not
   * @return  - returns PFC_TRUE if ping is need / returns PFC_FALSE
   *            if ping is not needed.
   */
  pfc_bool_t is_ping_needed();

  /**
   * @brief   - Physicalconfiguration needed for the ODC Conttoller or not
   * @return  - returns PFC_TRUE if Physicalconfiguration is need /
   *            returns PFC_FALSE if Physicalconfiguration is not needed.
   */
  pfc_bool_t is_physicalconfig_needed();

  /**
   * @brief  - Gets the ping interval
   * @return - returns the ping interval
   */
  uint32_t get_ping_interval();

  /**
   * @brief  - Gets the ping interval count
   * @return - returns the ping interval count
   */
  uint32_t get_ping_fail_retry_count();

  /**
   * @brief     - ping controller available or not
   * @param[in] - Controller pointer
   * @return    - returns PFC_TRUE if controller is available/
   *              returns PFC_FALSE if controller is not available
   */
  pfc_bool_t ping_controller(unc::driver::controller*);

  /**
   * @brief      - gets the physical port details
   * @param[in]  - Controller pointer
   * @return     - returns PFC_TRUE/PFC_FALSE
   */
  pfc_bool_t get_physical_port_details(unc::driver::controller* ctr);

  /**
   * @brief      - Compare the val_ctr with controller obeject
   *               and return the type of change identified
   * @param[in]  - ,Controller pointer, val_ctr
   * @return     - ctrl_info_update_type_t
   */
  ctrl_info_update_type_t compare_ctr_info(unc::driver::controller* ctr,
                                           const val_ctr_t& val_ctr);
 private:
  /**
   * @brief     - Read configuration file of odcdriver
   * @param[in] - None
   * @return    - None
   */
  void read_conf_file();

  /**
   * @brief     - Notify Audit to TC
   * @param[in] - controller id
   * @return    - returns ODC_DRV_SUCCESS on sending start
   *              notification to tc
   */
  uint32_t notify_audit_start_to_tc(std::string controller_id);

 private:
  uint32_t ping_interval;  // in seconds
  unc::restjson::ConfFileValues_t conf_file_values_;
};
}  //  namespace odcdriver
}  //  namespace unc
#endif
