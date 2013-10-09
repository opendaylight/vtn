/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the
 * terms of the Eclipse Public License v1.0 which
 * accompanies this
 * distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_MOD_HH__
#define _ODC_MOD_HH__

#include <odc_vtn.hh>
#include <odc_vbr.hh>
#include <odc_vbrif.hh>
#include <odc_driver_common_defs.hh>
#include <vtn_drv_module.hh>

namespace unc {
namespace odcdriver {

class ODCModule: public pfc::core::Module, public unc::driver::driver {
 public:
  /*
   * Paramaretrised Constructor
   */
  explicit ODCModule(const pfc_modattr_t*& obj) :
      Module(obj) {
  }
  /*
   *@brief - Gets the controller type
   *@param[in] -  unc_keytype_ctrtype_t enum
   */
  unc_keytype_ctrtype_t get_controller_type();

  /*
   * init method - Get Instance of vtndrvinntf & Register into RegisterDriver
   * @retnal - PFC_TRUE/ PFC_FALSE
   */
  pfc_bool_t init();
  /*
   * Fini
   * @retval  PFC_TRUE/ PFC_FALSE
   */
  pfc_bool_t fini();

  /*
   * Is 2phase commit supported or not
   * @retval - PFC_TRUE/ PFC_FALSE
   */
  pfc_bool_t is_2ph_commit_support_needed();

  /*
   * Is audit collection needed or not
   * @retval - PFC_TRUE/ PFC_FALSE
   */
  pfc_bool_t is_audit_collection_needed();

  /*
   * Creatyes Controller pointer with specific values and return
   * @param[in]- key_ctr_t - Controller key structure
   * @param[in] - val_ctr- Controller value structute
   * @param[out]- Controller pointer
   */
  unc::driver::controller* add_controller(const key_ctr_t& key_ctr,
      const val_ctr_t& val_ctr);

  /*
   * Updates Controller pointer with specific values and return
   * @param[in]- key_ctr_t - Controller key structure
   * @param[in] - val_ctr- Controller value structute
   * @param[out]- Controller pointer
   */
  unc::driver::controller* update_controller(const key_ctr_t& key_ctr,
      const val_ctr_t& val_ctr, unc::driver::controller* ctrl_inst);

  /*
   * Deletes Controller pointer with specific values
   * @param[in]- Controller pointer
   * @retval - PFC_TRUE/ PFC_FALSE
   */
  pfc_bool_t delete_controller(unc::driver::controller* delete_inst);

  /*
   * DGets the driver command as per the keytype
   * @param[in]- unc_key_type_t- Keytype
   * @retval - driver_command - correspoding driver_command
   */
  unc::driver::driver_command* get_driver_command(unc_key_type_t key_type);

  /*
   * HandleVote
   * @param[in] - Controller pointer
   * @partam[out]-   unc::tclib::TcCommonRet - enum value
   */
  unc::tclib::TcCommonRet HandleVote(unc::driver::controller*);

  /*
   * Handlethecommit
   * @param[in] - Controller pointer
   * @partam[ou]-   unc::tclib::TcCommonRet - enum value
   */
  unc::tclib::TcCommonRet HandleCommit(unc::driver::controller*);

  /*
   * Handle the abort
   * @param[in] - Controller pointer
   * @partam[out]-   unc::tclib::TcCommonRet - enum value
   */
  unc::tclib::TcCommonRet HandleAbort(unc::driver::controller*);
};
}
}
#endif
