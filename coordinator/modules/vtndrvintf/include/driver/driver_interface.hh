/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
  /**
   * @brief  - Method to check if two phase commit support is needed
   * @retval - PFC_TRUE/PFC_FALSE
   */
  virtual pfc_bool_t is_2ph_commit_support_needed() =0;

  /**
   * @brief  - Method to check if needs driver needs audit support
   * @retval - PFC_TRUE/PFC_FALSE
   */
  virtual pfc_bool_t is_audit_collection_needed() =0;

  /**
   * @brief     - Method to add controller
   * @param[in] - key_ctr_t,val_ctr_t
   * @retval    - controller* - Instance of controller
   */
  virtual controller* add_controller(const key_ctr_t& key_ctr,
                                     const val_ctr_t& val_ctr)=0;
  /**
   * @brief     - Method to update controller
   * @param[in] - key_ctr_t,val_ctr_t,controller*
   * @retval    - pfc_bool_t - PFC_TRUE
   */
  virtual pfc_bool_t update_controller(const key_ctr_t& key_ctr,
                                        const val_ctr_t& val_ctr,
                                        controller* ctrl_inst)=0;

  /**
   * @brief  - Method to retrieve the type of Controller supported by driver
   * @retval - unc_keytype_ctrtype_t - Type of controller
   */
  virtual unc_keytype_ctrtype_t get_controller_type() =0;

  /**
   * @brief     - Method to delete controller
   * @param[in] - controller* - Instance of controller
   * @retval    - PFC_TRUE/PFC_FALSE
   */
  virtual pfc_bool_t delete_controller(controller* delete_inst)=0;

  /**
   * @brief     - Method to get the driver command ptr for particular KT
   * @param[in] - key_type
   * @retval    - driver_command* - command pointer of Keytype
   */
  virtual driver_command* create_driver_command(unc_key_type_t key_type)=0;
  virtual vtn_driver_read_command* create_driver_read_command(
                                             unc_key_type_t key_type)=0;

  /**
   * @brief     - Method to handle vote request of transaction
   * @param[in] - controller* -Instance of controller
   * @retval    - TcCommonRet - TC return code
   */
  virtual unc::tclib::TcCommonRet HandleVote(controller* ctlptr)=0;
  /**
   * @brief     - Method to handle global commit request of transaction
   * @param[in] - controller* -Instance of controller
   * @retval    - TcCommonRet - TC return code
   */
  virtual unc::tclib::TcCommonRet HandleCommit(controller* ctlptr)=0;

  /**
   * @brief     - Method to get the abort request of transaction
   * @param[in] - controller* -Instance of controller
   * @retval    - TcCommonRet - TC return code
   */
  virtual unc::tclib::TcCommonRet HandleAbort(controller* ctlptr)=0;

  /**
   * @brief    - Method to check if ping is needed by the controller
   * @retval   - PFC_TRUE/PFC_FALSE
   */
  virtual pfc_bool_t is_ping_needed() = 0;

  /**
   * @brief    - Method to check if physical configuration needed
   *             by the controller
   * @retval   - PFC_TRUE/PFC_FALSE
   */
  virtual pfc_bool_t is_physicalconfig_needed() = 0;

  /**
   * @brief    - Method to retrive the ping Interval
   * @retval   - uint32_t - ping Interval in secs
   */
  virtual uint32_t  get_ping_interval() = 0;

  /**
   * @brief    - Method to retrive the Ping Retry Count
   * @retval   - uint32_t - ping retry count
   */
  virtual uint32_t get_ping_fail_retry_count() = 0;

  /**
   * @brief     - Method to ping the controller
   * @param[in] - controller* -Instance of controller
   * @retval    - PFC_TRUE/PFC_FALSE
   */
  virtual pfc_bool_t  ping_controller(unc::driver::controller*) = 0;
  /**
   * @brief     - Method to get physical port derails from controller
   * @param[in] - controller* -Instance of controller
   * @retval    - PFC_TRUE/PFC_FALSE
   */

  virtual pfc_bool_t  get_physical_port_details(unc::driver::controller*) = 0;

  /**
   * @brief    - Virtual destructort
   */
  virtual ~driver() {}
};
}  // namespace driver
}  // namespace unc
#endif
