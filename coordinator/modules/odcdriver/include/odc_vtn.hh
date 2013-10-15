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
#include <unc/upll_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <vector>
#include <string>

namespace unc {
namespace odcdriver {

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

  /*
   * @brief - read all - reads all the vtns
   * @param[in] - ctr - controller pointer
   * @param[in] - cfgnode_vector to which values are pushed
   * @param[out] - drv_resp_code_t -  DRVAPI_RESPONSE_SUCCESS/
   *                                DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t read_all(unc::driver::controller* ctr,
                   vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /*
   * @brief - get vtn child - gets all the vbridge under particular vtn
   * @param[in] - vtn name
   * @param[in] - ctr - controller pointer
   * @param[in] - cfgnode_vector to which values are pushed
   * @param[out] - drv_resp_code_t -  DRVAPI_RESPONSE_SUCCESS/
   *                                  DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t get_vtn_child(std::string vtnname,
                     unc::driver::controller* ctr,
                     vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

 private:
  /*
   * parse vtn and append to the vector
   * @param[in] - json_obj_vtn - json object
   * @param[in] - arr_idx - array index
   * @param[in] - cfgnode_vector - vector to which config node needs to be
   *              pushed
   * @param[out] - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *               / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t parse_vtn_append_vector(json_object *json_obj_vtn,
          int arr_idx, vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /*
   * parse vbr and append to the vector
   * @param[in] - json_obj_vtn - json object
   * @param[in] - arr_idx - array index
   * @param[in] - cfgnode_vector - vector to which config node needs
   *              to be pushed
   * @param[out] - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *               / DRVAPI_RESPONSE_FAILURE
   */

  drv_resp_code_t parse_vbr_append_vector(unc::driver::controller* ctr,
          json_object *json_obj_vbr, std::string vtn_name,
          int arr_idx, vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /*
   * @brief  - Validates Create Vtn
   * @param[in] - key - VTN Key Structure key_vtn_t
   * @param[in] - conn - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_create_vtn(const key_vtn_t& key_vtn,
                                      unc::driver::controller* ctr);

  /*
   * @brief  - Validates Delete Vtn
   * @param[in] - key - VTN Key Structure key_vtn_t
   * @param[in] - conn - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_delete_vtn(const key_vtn_t& key_vtn,
                                      unc::driver::controller* ctr);

  /*
   * @brief  - Validates Update Vtn
   * @param[in] - key - VTN Key Structure key_vtn_t
   * @param[in] - conn - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_update_vtn(const key_vtn_t& key_vtn,
                                      unc::driver::controller* ctr);

  /*
   * @brief  - Checks is_vtn_exists_in_controller
   * @param[in] - key - VTN Key Structure key_vtn_t
   * @param[in] - conn - Controller Connection
   * @uint32_t - response code from the controller
   */
  uint32_t is_vtn_exists_in_controller(const key_vtn_t& key_vtn,
                                       unc::driver::controller* ctr);

  /*
   * @brief  - get_controller_response and checks the resp code
   * @param[in] - url to be set to
   * @uint32_t - response code from the controller
   */
  uint32_t get_controller_response_code(std::string url,
                               unc::driver::controller* ctr,
                               unc::restjson::HttpMethod method,
                               const char* request_body);

  /*
   * @brief  - Creates the Request Body
   * @param[in] - val - VTN value structure val_vtn_t
   * @retval - char*  - request body formed
   */
  const char* create_request_body(const val_vtn_t& val_vtn);

  /*
   * @brief - parse the response data
   * @param[in] - data which is the response
   * @param[in] - cfgnode_vector - to which the resp to be pushed
   * @param[out] - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *                                / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t parse_resp_data(char *data,
                vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /*
   * @brief - parse the vbr response data
   * @param[in] - data which is the response
   * @param[in] - cfgnode_vector - to which the resp to be pushed
   * @param[out] - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *                                / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t parse_vbr_resp_data(char *data, std::string vtn_name,
                    unc::driver::controller* ctr,
                    vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

 private:
  std::string idle_timeout_;
  std::string hard_timeout_;
};
}  // namespace odcdriver
}  // namespace unc
#endif
