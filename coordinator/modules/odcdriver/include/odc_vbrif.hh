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
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <string>
#include <sstream>

namespace unc {
namespace odcdriver {
class ODCVBRIfCommand: public unc::driver::vbrif_driver_command {
 public:
  /**
   * @brief Default Constructor
   */
  ODCVBRIfCommand();

  /**
   * @brief Default Destructor
   */
  ~ODCVBRIfCommand();

  /**
   * @brief      - Creates VBRIf/ PortMap
   * @param[in]  - key structure of VBRIf
   * @param[in]  - value structure of VBRIf
   * @param[in]  - Controller connection information
   * @retval     - returns DRVAPI_RESPONSE SUCCESS on creation of vbrif successfully
   *               /returns DRVAPI_RESPONSE_FAILURE on failure
   */
  drv_resp_code_t create_cmd(key_vbr_if_t& key, pfcdrv_val_vbr_if_t& val,
                             unc::driver::controller *conn);

  /**
   * @brief                      - Updates VBRIf
   * @param[in] key              - key structure of VBRIf
   * @param[in] val              - value structure of VBRIf
   * @param[in] conn             - Controller connection information
   * @retval drv_resp_code_t     - returns DRVAPI_RESPONSE_SUCCESS on updation of VBRIf
   *                               successfully/returns DRVAPI_RESPONSE_FAILURE on failure
   */
  drv_resp_code_t update_cmd(key_vbr_if_t& key, pfcdrv_val_vbr_if_t& val,
                             unc::driver::controller *conn);

  /**
   * @brief                       - Updates  PortMap
   * @param[in] vbrif_key         - key structure of VBRIf
   * @param[in] vbrif_val         - value structure of VBRIf
   * @param[in] conn              - Controller connection information
   * @return uint32_t             - returns DRVAPI_RESPONSE_SUCCESS on updation of port_map
   *                                successfully/returns DRVAPI_RESPONSE_FAILURE on failure
   */
  uint32_t update_cmd_port_map(key_vbr_if_t& vbrif_key,
                               pfcdrv_val_vbr_if_t& vbrif_val,
                               unc::driver::controller *conn);

  /**
   * @brief                    - Deletes VBRIf
   * @param[in] key            - key structure of VBRIf
   * @param[in] val            - value structure of VBRIf
   * @param[in] conn           - Controller connection information
   * @return drv_resp_code_t   - returns DRVAPI_RESPONSE_SUCCESS on deletion of
   *                             VBRIf successfully /returns
   *                             DRVAPI_RESPONSE_FAILURE on failure
   */
  drv_resp_code_t delete_cmd(key_vbr_if_t& key, pfcdrv_val_vbr_if_t& val,
                             unc::driver::controller *conn);

  /**
   * @brief                - Deletes VBRIf/ PortMap
   * @param[in] vbrif_key  - key structure of VBRIf
   * @param[in] vbrif_val  - value structure of VBRIf
   * @param[in] conn       - Controller connection information
   * @retval uint32_t      - returns DRVAPI_RESPONSE_SUCCESS on deletion of
   *                         PortMap /returns
   *                         DRVAPI_RESPONSE_FAILURE on failure
   */
  uint32_t delete_cmd_port_map(key_vbr_if_t& vbrif_key,
                               pfcdrv_val_vbr_if_t& vbrif_val,
                               unc::driver::controller *conn);

  /**
   * @brief                  - Validates the operation
   * @param[in] - key        - VBRIF Key Structure key_vbr_if
   * @param[in] - val        - VBRIF value structure pfcdrv_val_vbr_if_t
   * @param[in] - ctr        - Controller pointer
   * @param[in] - op         - operation
   * @return drv_resp_code_t - returns DRVAPI_RESPONSE_SUCCESS on suceess of
   *                           validation operation of VBRIf/returns
   *                           DRVAPI_RESPONSE_FAILURE on failure
   */
  drv_resp_code_t validate_op(key_vbr_if_t& key,
                              pfcdrv_val_vbr_if_t& val,
                              unc::driver::controller* ctr,
                              uint32_t op);

 private:
  /**
   * @brief                    - get_controller_response and checks the res code
   * @param[in] url            - url to be set to
   * @param[in] ctr            - controller pointer
   * @param[in] method         - HttpMethod enum
   * @param[in] request_body   - request body
   * @return uint32_t          - returns response code from the controller
   */
  uint32_t get_controller_response_code(std::string url,
                                        unc::driver::controller* ctr,
                                        unc::restjson::HttpMethod method,
                                        const char* request_body);
  /**
   * @brief                  - Constructs url for vbrif
   * @param[in] vbrif_key    - key structure of vbrif key structure
   * @return                 - returns the url of VBRIf
   */
  std::string get_vbrif_url(key_vbr_if_t& vbrif_key);

  /**
   * @brief                    - Creates the Request Body for portmap
   * @param[in]  vbrif_val     - VTN value structure val_vtn_t
   * @return  const char*      - returns the request body formed
   */
  const char* create_request_body_port_map(pfcdrv_val_vbr_if_t& vbrif_val);

  /**
   * @brief                    - Validates Create vbrif
   * @param[in] key_vbr_if     - VBRIF Key Structure key_vbr_if
   * @param[in] ctr            - Controller pointer
   * @return drv_resp_code_t   - returns DRVAPI_RESPONSE_SUCCESS on success of
   *                             validate create of VBRIf / returns
   *                             DRVAPI_RESPONSE_FAILURE on failure
   */
  drv_resp_code_t validate_create_vbrif(key_vbr_if_t& key_vbr_if,
                                        unc::driver::controller* ctr);

  /**
   * @brief                  - Validates Delete vbrif
   * @param[in] key_vbr_if   - VBRIF Key Structure key_vbr_if
   * @param[in] ctr          - Controller pointer
   * @return drv_resp_code_t - returns DRVAPI_RESPONSE_SUCCESS on success of
   *                           validate delete of VBRIf/returns
   *                           DRVAPI_RESPONSE_FAILURE on failure
   */
  drv_resp_code_t validate_delete_vbrif(key_vbr_if_t& key_vbr_if,
                                        unc::driver::controller* ctr);

  /**
   * @brief                    - Validates Update vbrif
   * @param[in] key_vbr_if     - VBRIF Key Structure key_vbr_if
   * @param[in] ctr            - Controller pointer
   * @return drv_resp_code_t   - returns DRVAPI_RESPONSE_SUCCESS on success of
   *                             validate update of VBRIf/returns
   *                             DRVAPI_RESPONSE_FAILURE on failure
   */
  drv_resp_code_t validate_update_vbrIf(key_vbr_if_t& key_vbr_if,
                                        unc::driver::controller* ctr);

  /**
   * @brief                       - Checks is_vtn_exists_in_controller
   * @param[in] key_vbr_if        - VBRIF Key Structure key_vbr_if
   * @param[in] ctr               - Controller pointer
   * @return unint32_t            - returns response code from the controller
   *                                on checking whether the vtn exists in
   *                                controller
   */
  uint32_t is_vtn_exists_in_controller(key_vbr_if_t& key_vbr_if,
                                       unc::driver::controller* ctr);

  /**
   * @brief                      - Checks is_vbr_exists_in_controller
   * @param[in] key_vbr_if       - VBRIF Key Structure key_vbr_if
   * @param[in] ctr              - Controller pointer
   * @return  unint32_t          - returns response code from the controller on
   *                               checking whether the VBRIf exists in the
   *                               controller
   */
  uint32_t is_vbrif_exists_in_controller(key_vbr_if_t& key_vbr_if,
                                         unc::driver::controller* ctr);

  /**
   * @brief                      - Checks is_vbrif_exists_in_controller
   * @param[in] key_vbr_if       - VBRIF Key Structure key_vbr_if
   * @param[in] ctr              - Controller pointer
   * @return uint32_t            - returns response code from the controller on
   *                               checking whether the vbr exists in ctl
   */
  uint32_t is_vbr_exists_in_controller(key_vbr_if_t& key_vbr_if,
                                       unc::driver::controller* ctr);

  /**
   * @brief                - get_controller_response and checks the resp code
   * @param[in] url        - url to be set to
   * @param[in] ctr        - controller pointer
   * @return               - returns response code from the controller
   */
  uint32_t get_controller_response(std::string url,
                                   unc::driver::controller* ctr);

  /**
   * @brief                - Creates the Request Body
   * @param[in] - val_vtn  - VTN value structure val_vtn_t
   * @return - const char* - returns the request body formed
   */
  const char* create_request_body(pfcdrv_val_vbr_if_t& val_vtn);
};
}  // namespace odcdriver
}  // namespace unc
#endif
