/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_DRV_HH_
#define _ODC_DRV_HH_

#include <driver/driver_interface.hh>
#include <pfcxx/synch.hh>

namespace unc {
namespace driver {

class OdcDriver : public unc::driver::driver {
 public:
  explicit OdcDriver() : ping_result(PFC_TRUE) {}

  pfc_bool_t init() {
    return PFC_TRUE;
  }

  pfc_bool_t fini() {
    return PFC_TRUE;
  }

  pfc_bool_t is_2ph_commit_support_needed() {
    return PFC_FALSE;
  }

  pfc_bool_t is_audit_collection_needed() {
    return PFC_FALSE;
  }

  unc_keytype_ctrtype_t get_controller_type() {
    return UNC_CT_ODC;
  }

  pfc_bool_t delete_controller(unc::driver::controller* ctrl_inst) {
    return PFC_TRUE;
  }

  uint32_t get_ping_fail_retry_count() {
    return 2;
  }

  pfc_bool_t is_ping_needed() {
    return PFC_TRUE;
  }

 pfc_bool_t is_physicalconfig_needed() {
   return PFC_TRUE;
  }

  uint32_t get_ping_interval() {
    return 10;
  }


  pfc_bool_t  get_physical_port_details(unc::driver::controller*) {
    return true;
  }

  pfc_bool_t  ping_controller(unc::driver::controller*) {
    return ping_result;
  }

  pfc_bool_t set_ping_result(pfc_bool_t result) {
    ping_result = result;
    return ping_result;
  }

  unc::driver::driver_command* create_driver_command(unc_key_type_t key_type);
  unc::driver::vtn_driver_read_command* create_driver_read_command(
							unc_key_type_t key_type){ 
    return NULL;	
  }

  unc::tclib::TcCommonRet HandleVote(unc::driver::controller* ctlptr);

  unc::tclib::TcCommonRet HandleCommit(unc::driver::controller* ctlptr);

  unc::tclib::TcCommonRet HandleAbort(unc::driver::controller* ctlptr);

  unc::driver::controller* add_controller(const key_ctr_t& key_ctr,
                                          const val_ctr_t& val_ctr);
  pfc_bool_t update_controller(const key_ctr_t& key_ctr,
                                     const val_ctr_t& val_ctr,
                                    unc::driver::controller* ctrl_inst);

 private:
  pfc_bool_t  ping_result;
};
}  // namespace driver
}  // namespace unc
#endif
