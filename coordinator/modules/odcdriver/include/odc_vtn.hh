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

/**
 * @brief   Handles VTN Command Formation"
 * @file    odc_vtn.hh
 *
 **/

#ifndef _ODC_VTN_HH_
#define _ODC_VTN_HH_

#include <rest_client.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <string>

namespace unc {
namespace odcdriver {

/**
 * @brief ODLVTN Command provides function to send Request and Get Response
 *
 **/

class ODCVTNCommand: public unc::driver::vtn_driver_command {
 public:
  /*
   * @brief Default Constructor
   */
  ODCVTNCommand();

  /*
   * @bried Default Destructor
   */
  ~ODCVTNCommand();

  /**
   * @brief - Creates VTN
   * @param[in] key_vtn_t - key structure of VTN
   * @param[in] val_vtn_t - value structure of VTN
   * @param[in] ControllerConnection - Controller connection information
   * @retval - ODLDRVSUCCESS / ODLDRVFAILURE
   */
  drv_resp_code_t create_cmd(key_vtn_t& key, val_vtn_t& val,
      unc::driver::controller *conn);

  /**
   * @brief - Updates VTN
   * @param[in] key_vtn_t - key structure of VTN
   * @param[in] val_vtn_t - value structure of VTN
   * @param[in] ControllerConnection - Controller connection information
   * @retval - SUCCESS / FAILURE
   */
  drv_resp_code_t update_cmd(key_vtn_t& key, val_vtn_t& val,
      unc::driver::controller *conn);

  /**
   * @brief - Creates VTN
   * @param[in] key_vtn_t - key structure of VTN
   * @param[in] val_vtn_t - value structure of VTN
   * @param[in] ControllerConnection - Controller connection information
   * @retval - ODLDRVSUCCESS / ODLDRVFAILURE
   */
  drv_resp_code_t delete_cmd(key_vtn_t& key, val_vtn_t& val,
      unc::driver::controller *conn);

  /**
   * @brief - Validates the operation
   * @param[in] - key - VTN Key Structure key_vtn_t
   * @param[in] - conn - Controller Connection
   * @param[in] - op - operation
   */
  drv_resp_code_t validate_op(key_vtn_t& key, val_vtn_t& val,
      unc::driver::controller *conn, uint32_t op);

 private:
  /*
   * @brief  - Validates Create Vtn
   * @param[in] - key - VTN Key Structure key_vtn_t
   * @param[in] - conn - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_create_vtn(key_vtn_t& key_vtn,
      unc::driver::controller* ctr);

  /*
   * @brief  - Validates Delete Vtn
   * @param[in] - key - VTN Key Structure key_vtn_t
   * @param[in] - conn - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_delete_vtn(key_vtn_t& key_vtn,
      unc::driver::controller* ctr);

  /*
   * @brief  - Validates Update Vtn
   * @param[in] - key - VTN Key Structure key_vtn_t
   * @param[in] - conn - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_update_vtn(key_vtn_t& key_vtn,
      unc::driver::controller* ctr);

  /*
   * @brief  - Checks is_vtn_exists_in_controller
   * @param[in] - key - VTN Key Structure key_vtn_t
   * @param[in] - conn - Controller Connection
   * @uint32_t - response code from the controller
   */
  uint32_t is_vtn_exists_in_controller(key_vtn_t& key_vtn,
      unc::driver::controller* ctr);

  /*
   * @brief  - get_controller_response and checks the resp code
   * @param[in] - url to be set to
   * @uint32_t - response code from the controller
   */
  uint32_t get_controller_response(std::string url,
      unc::driver::controller* ctr);

  /*
   * @brief  - Creates the Request Body
   * @param[in] - val - VTN value structure val_vtn_t
   * @retval - char*  - request body formed
   */
  const char* create_request_body(val_vtn_t& val_vtn);

 private:
  std::string idle_timeout_;
  std::string hard_timeout_;
};
}
}
#endif
