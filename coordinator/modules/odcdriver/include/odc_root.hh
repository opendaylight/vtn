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

#ifndef _ODC_ROOT_HH_
#define _ODC_ROOT_HH_

#include <driver/driver_command.hh>
#include <vtn_conf_data_element_op.hh>
#include <odc_vtn.hh>
#include <odc_vbr.hh>
#include <vector>
#include <string>

namespace unc {
namespace odcdriver {

class ODCROOTCommand : public unc::driver::root_driver_command {
 public:
  /*
   * Constructor
   */
  ODCROOTCommand() {}

  /*
   * Destructor
   */
  ~ODCROOTCommand() {}

  /*
   * @brief - Creates cmd
   * @param[in] - key_root - key structure of key_root
   * @param[in] - val value structure of val_root
   * @param[in] - conn - controller pointer
   * @param[out] - drv_resp_code enum
   */
  drv_resp_code_t create_cmd(key_root_t& key,
                             unc::driver::val_root_t& val,
                             unc::driver::controller *conn) {
    return DRVAPI_RESPONSE_SUCCESS;
  }

  /*
   * @brief - Update Cmd
   * @param[in] - key_root - key structure of key_root
   * @param[in] - val value structure of val_root
   * @param[in] - conn - controller pointer
   * @param[out] - drv_resp_code enum
   */
  drv_resp_code_t update_cmd(key_root_t& key,
                             unc::driver::val_root_t& val,
                             unc::driver::controller *conn) {
    return DRVAPI_RESPONSE_SUCCESS;
  }

  /*
   * @brief - Update Cmd
   * @param[in] - key_root - key structure of key_root
   * @param[in] - val value structure of val_root
   * @param[in] - conn - controller pointer
   * @param[out] - drv_resp_code enum
   */
  drv_resp_code_t delete_cmd(key_root_t& key,
                             unc::driver::val_root_t& val,
                             unc::driver::controller *conn) {
    return DRVAPI_RESPONSE_SUCCESS;
  }

  /*
   * validates the operation
   * @param[in] - key_root - key structure of key_root
   * @param[in] - val value structure of val_root
   * @param[in] - conn - controller pointer
   * @param[in] - operation
   * @param[out] - drv_resp_code enum
   */
  drv_resp_code_t validate_op(key_root_t& key,
                              unc::driver::val_root_t& val,
                              unc::driver::controller* ctr,
                              uint32_t op) {
    return DRVAPI_RESPONSE_SUCCESS;
  }

  /*
   * @brief - reads the root child
   * @param[in] - cfg_ptr - Config Node pointer
   * @param[in] - ctl_ptr - Controller pointer
   * @param[out] - drv_resp_code_t enum
   */
  drv_resp_code_t
      read_root_child(std::vector<unc::vtndrvcache::ConfigNode*> &cfg_ptr,
                      unc::driver::controller* ctl_ptr);

  /*
   * @brief - reads all child info
   * @param[in] - cfg_ptr - Config Node pointer
   * @param[out] - vector of Config Node ptr
   * @param[in] - ctr - controller pointer
   * @param[out] - drv_resp_code_t enum
   */
  drv_resp_code_t
      read_all_child(unc::vtndrvcache::ConfigNode* cfg_ptr,
                     std::vector<unc::vtndrvcache::ConfigNode*> & cfg,
                     unc::driver::controller* ctl_ptr);
};
}  // namespace odcdriver
}  // // namespace unc
#endif
