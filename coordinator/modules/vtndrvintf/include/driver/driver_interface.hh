/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made
 * available under the  terms of the Eclipse Public License v1.0 which
 * accompanies this  distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __CDF_DRIVER_INTERFACE_HH__
#define __CDF_DRIVER_INTERFACE_HH__

#include <driver/controller_interface.hh>
#include <driver/driver_command.hh>
#include <pfc/ipc_struct.h>
#include <unc/keytype.h>
#include <uncxx/tclib/tclib_defs.hh>

namespace unc {
namespace driver {


class driver {
 public:
  // 2-phase commit support from CDF needed?
  virtual pfc_bool_t is_2ph_commit_support_needed () =0;
  // Can driver collect running config by itself?
  virtual pfc_bool_t is_audit_collection_needed () =0;
  // add controller of this type
  virtual controller* add_controller(const key_ctr_t& key_ctr,
                                     const val_ctr_t& val_ctr)=0;
  // update controller of this type
  virtual controller* update_controller(const key_ctr_t& key_ctr,
                                        const val_ctr_t& val_ctr,
                                        controller* ctrl_inst)=0;
  //REturn Type of Controller supported by driver
  virtual unc_keytype_ctrtype_t get_controller_type() =0;
  // delete controller of this type
  virtual pfc_bool_t delete_controller(controller* delete_inst)=0;
  // driver command for particular KT
  virtual driver_command* get_driver_command(unc_key_type_t key_type)=0;
  // transaction command, if no transaction needed, return NULL
  virtual unc::tclib::TcCommonRet HandleVote(controller*)=0;
  virtual unc::tclib::TcCommonRet HandleCommit(controller*)=0;
  virtual unc::tclib::TcCommonRet HandleAbort(controller*)=0;

  virtual ~driver() {}; 
};
}  // driver
}  // unc
#endif
