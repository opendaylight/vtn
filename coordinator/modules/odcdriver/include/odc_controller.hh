/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_CONTROLLER_HH_
#define _ODC_CONTROLLER_HH_

#include <pfc/ipc_struct.h>
#include <arpa/inet.h>
#include <driver/controller_interface.hh>
#include <odc_driver_common_defs.hh>
#include <rest_util.hh>
#include <string>
#include <vector>
#include <sstream>

namespace unc {
namespace odcdriver {

class OdcController: public unc::driver::controller {
 public:
  /**
   * @brief     - Parametrised Constructor
   * @param[in] - key_ctr (key structure for controller)
   * @param[in] - val_ctr (value structure for controller)
   */
  OdcController(const key_ctr_t& key_ctr, const val_ctr_t& val_ctr,
                unc::restjson::ConfFileValues_t conf_values);

  /**
   * @brief - Destructor
   */
  ~OdcController();

  /**
   * @brief   - Gets the controller type
   * @return  - unc_keytype_ctrtype_t (controller type)
   */
  unc_keytype_ctrtype_t get_controller_type();

  /**
   *  @brief   - Gets the contoller id
   *  @return  - std::string(controller id)
   */
  std::string get_controller_id();

  /**
   * @brief   - Gets the audit status
   * @return  - pfc_bool_t(audit status)
   */
  pfc_bool_t get_audit_status();

  /**
   * @brief    - reset connect set or not
   * @return   - PFC_TRUE on reset connection, PFC_FALSE on failure
   */
  pfc_bool_t reset_connect();

  /**
   * @brief  - get the host address
   * @return - std::string(host address)
   */
  std::string get_host_address();

  /**
   * @brief  - gets the user name
   * @return - std::string (username)
   */
  std::string get_user_name();

  /**
   * @brief  - Gets the password
   * @return - std::string(password)
   */
  std::string get_pass_word();

  /**
   * @brief  - Gets the conf value struct
   * @return - conf_file_values
   */
  unc::restjson::ConfFileValues_t get_conf_value();

  /**
   * @brief     - Updates the controller parameters
   *              This method is called when KT_CTR update is received from UPPL
   * @param[in] - key_ctr (key structure for controller)
   * @param[in] - val_ctr (value structure for controller)
   * @return    - PFC_TRUE on updation
   */
  pfc_bool_t update_ctr(const key_ctr_t& key_ctr, const val_ctr_t& val_ctr);

  /**
   * @brief                     - converts switch id to openflow format
   * @param[in] node_id         - vtn manager node id format
   * @return string             - generated string format
   */
  std::string frame_openflow_switchid(std::string &node_id);

  /**
   * @brief     - Vector to hold vlan-ids for verification purpose
   */
  std::vector<std::string> vlan_vector;

 private:
  std::string ip_addr_;
  std::string controller_name_;
  std::string version_;
  std::string description_;
  std::string user_name_;
  std::string pass_word_;
  pfc_bool_t audit_;
  unc::restjson::ConfFileValues_t conf_file_values_;
};
}  // namespace odcdriver
}  // namespace unc
#endif
