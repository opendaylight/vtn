/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_drv.hh>
namespace unc {
namespace driver {

unc::driver::driver_command* OdcDriver::create_driver_command(
                                        unc_key_type_t key_type) {
  return NULL;
}

unc::tclib::TcCommonRet OdcDriver::HandleVote(
                                   unc::driver::controller* ctlptr) {
  return unc::tclib::TC_SUCCESS;
}

unc::tclib::TcCommonRet OdcDriver::HandleCommit(
                                   unc::driver::controller* ctlptr) {
  return unc::tclib::TC_SUCCESS;
}

unc::tclib::TcCommonRet OdcDriver::HandleAbort(
                                   unc::driver::controller* ctlptr) {
  return unc::tclib::TC_SUCCESS;
}

unc::driver::controller* OdcDriver::add_controller(const key_ctr_t& key_ctr,
                                                   const val_ctr_t& val_ctr) {
  return NULL;
}

pfc_bool_t OdcDriver::update_controller(const key_ctr_t& key_ctr,
                                          const val_ctr_t& val_ctr,
                                          unc::driver::controller* ctrl_inst) {
  return PFC_TRUE;
}
}  // namespace driver
}  // namespace unc
