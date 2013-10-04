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

#ifndef _ODC_VBRIF_HH
#define _ODC_VBRIF_HH

#include <driver/driver_command.hh>
#include <rest_client.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <string>

namespace odc {
namespace driver {
class ODLVBRIfCommand: public unc::driver::vbrif_driver_command {
 public:
  /*
   * @brief Default Constructor
   */
  ODLVBRIfCommand();

  /*
   * @bried Default Destructor
   */
  ~ODLVBRIfCommand();

  /**
   * @brief - Creates VBRIf/ PortMap
   * @param[in] key_vbr_if - key structure of VBRIf
   * @param[in] val_vbr_if - value structure of VBRIf
   * @param[in] ControllerConnection - Controller connection information
   * @retval - SUCCESS / FAILURE
   */
  drv_resp_code_t create_cmd(key_vbr_if_t& key, pfcdrv_val_vbr_if_t& val,
      unc::driver::controller *conn);

  /**
   * @brief - Creates PortMap
   * @param[in] key_vbr_if - key structure of VBRIf
   * @param[in] val_vbr_if - value structure of VBRIf
   * @param[in] ControllerConnection - Controller connection information
   * @retval - SUCCESS / FAILURE
   */
  uint32_t create_cmd_port_map(key_vbr_if_t& vbrif_key,
      pfcdrv_val_vbr_if_t& vbrif_val, unc::driver::controller *conn);

  /**
   * @brief - Updates VBRIf/ PortMap
   * @param[in] key_vbr_if - key structure of VBRIf
   * @param[in] val_vbr_if - value structure of VBRIf
   * @param[in] ControllerConnection - Controller connection information
   * @retval - SUCCESS / FAILURE
   */

  drv_resp_code_t update_cmd(key_vbr_if_t& key, pfcdrv_val_vbr_if_t& val,
      unc::driver::controller *conn);

  /**
   * @brief - Updates  PortMap
   * @param[in] key_vbr_if - key structure of VBRIf
   * @param[in] val_vbr_if - value structure of VBRIf
   * @param[in] ControllerConnection - Controller connection information
   * @retval - SUCCESS / FAILURE
   */

  uint32_t update_cmd_port_map(key_vbr_if_t& vbrif_key,
      pfcdrv_val_vbr_if_t& vbrif_val, unc::driver::controller *conn);
  /**
   * @brief - Deletes VBRIf/ PortMap
   * @param[in] key_vbr_if - key structure of VBRIf
   * @param[in] val_vbr_if - value structure of VBRIf
   * @param[in] ControllerConnection - Controller connection information
   * @retval - SUCCESS / FAILURE
   */
  drv_resp_code_t delete_cmd(key_vbr_if_t& key, pfcdrv_val_vbr_if_t& val,
      unc::driver::controller *conn);

  /**
   * @brief - Deletes VBRIf/ PortMap
   * @param[in] key_vbr_if - key structure of VBRIf
   * @param[in] val_vbr_if - value structure of VBRIf
   * @param[in] ControllerConnection - Controller connection information
   * @retval - SUCCESS / FAILURE
   */

  uint32_t delete_cmd_port_map(key_vbr_if_t& vbrif_key,
      pfcdrv_val_vbr_if_t& vbrif_val, unc::driver::controller *conn);

  /**
   * @brief - Validates the operation
   * @param[in] - key - VBRIF Key Structure key_vbr_if
   * @param[in] - val - VBRIF value structure pfcdrv_val_vbr_if_t
   * @param[in] - conn - Controller Connection
   * @param[in] - op - operation
   */
  drv_resp_code_t validate_op(key_vbr_if_t& key, pfcdrv_val_vbr_if_t& val,
      unc::driver::controller* ctr, uint32_t op);

 private:
  /*
   * @brief - Constructs url for vbrif
   * @param[in] - key_vbr_if_t - key structure of vbrif key structure
   */
  std::string get_vbrif_url(key_vbr_if_t& vbrif_key);

  /*
   * @brief  - Creates the Request Body for portmap
   * @param[in] - val - VTN value structure val_vtn_t
   * @retval - const char*  - request body formed
   */
  const char* create_request_body_port_map(pfcdrv_val_vbr_if_t& vbrif_val);

  /*
   * @brief  - Validates Create vbrif
   * @param[in] - key_vbr_if - VBRIF Key Structure key_vbr_if
   * @param[in] - conn - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_create_vbrif(key_vbr_if_t& key_vbr_if,
      unc::driver::controller* ctr);

  /*
   * @brief  - Validates Delete vbrif
   * @param[in] - key_vbr_if - VBRIF Key Structure key_vbr_if
   * @param[in] - conn - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_delete_vbrif(key_vbr_if_t& key_vbr_if,
      unc::driver::controller* ctr);

  /*
   * @brief  - Validates Update vbrif
   * @param[in] - key_vbr_if - VBRIF Key Structure key_vbr_if
   * @param[in] - conn - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_update_vbrIf(key_vbr_if_t& key_vbr_if,
      unc::driver::controller* ctr);

  /*
   * @brief  - Checks is_vtn_exists_in_controller
   * @param[in] - key_vbr_if - VBRIF Key Structure key_vbr_if
   * @param[in] - conn - Controller Connection
   * @uint32_t - response code from the controller
   */
  uint32_t is_vtn_exists_in_controller(key_vbr_if_t& key_vbr_if,
      unc::driver::controller* ctr);

  /*
   * @brief  - Checks is_vbr_exists_in_controller
   * @param[in] - key_vbr_if - VBRIF Key Structure key_vbr_if
   * @param[in] - conn - Controller Connection
   * @uint32_t - response code from the controller
   */
  uint32_t is_vbrif_exists_in_controller(key_vbr_if_t& key_vbr_if,
      unc::driver::controller* ctr);

  /*
   * @brief  - Checks is_vbrif_exists_in_controller
   * @param[in] - key_vbr_if - VBRIF Key Structure key_vbr_if
   * @param[in] - conn - Controller Connection
   * @uint32_t - response code from the controller
   */
  uint32_t is_vbr_exists_in_controller(key_vbr_if_t& key_vbr_if,
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
   * @retval - const char*  - request body formed
   */
  const char* create_request_body(pfcdrv_val_vbr_if_t& val_vtn);
};
}
}
#endif
