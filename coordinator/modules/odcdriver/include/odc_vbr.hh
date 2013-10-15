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

#ifndef _ODC_VBR_HH_
#define _ODC_VBR_HH_

#include <driver/driver_command.hh>
#include <json_build_parse.hh>
#include <vtn_conf_data_element_op.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <rest_client.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <string>
#include <vector>

namespace unc {
namespace odcdriver {

class ODCVBRCommand: public unc::driver::vbr_driver_command {
 public:
  /*
   * @brief Default Constructor
   */
  ODCVBRCommand();

  /*
   * @bried Default Destructor
   */
  ~ODCVBRCommand();

  /**
   * @brief - Creates VBR
   * @param[in] key_vbr_t - key structure of VBR
   * @param[in] val_vbr_t - value structure of VBR
   * @param[in] ControllerConnection - Controller connection information
   * @retval - SUCCESS / FAILURE
   */

  drv_resp_code_t create_cmd(key_vbr_t& key_vbr, val_vbr_t& val_vbr,
      unc::driver::controller *conn);

  /**
   * @brief - Updates VBR
   * @param[in] key_vbr_t - key structure of VBR
   * @param[in] val_vbr_t - value structure of VBR
   * @param[in] ControllerConnection - Controller connection information
   * @retval - SUCCESS / FAILURE
   */

  drv_resp_code_t update_cmd(key_vbr_t& key_vbr, val_vbr_t& val_vbr,
      unc::driver::controller* conn);

  /**
   * @brief - Deletes VBR
   * @param[in] key_vbr_t - key structure of VBR
   * @param[in] val_vbr_t - value structure of VBR
   * @param[in] ControllerConnection - Controller connection information
   * @retval - SUCCESS / FAILURE
   */
  drv_resp_code_t delete_cmd(key_vbr_t& key_vbr, val_vbr_t& val_vbr,
      unc::driver::controller *conn);

  /**
   * @brief - Validates the operation
   * @param[in] - key_vbr - VBR Key Structure key_vbr_t
   * @param[in] - val_vbr - VBR value structure val_vbr_t
   * @param[in] - ctr - Controller Connection
   * @param[in] - op - operation
   */
  drv_resp_code_t validate_op(key_vbr_t& key_vbr, val_vbr_t& val_vbr,
      unc::driver::controller* ctr, uint32_t op);
  /*
   * @brief - get all the vbr child
   * @param[in] - vtn_name - vtn name
   * @param[in] - vbr_name - vbr name
   * @param[in] - ctr - controller pointer
   * @param[out] - cfgnode_vector - config node vector
   */
  drv_resp_code_t get_vbr_child(string vtn_name, string vbr_name,
           unc::driver::controller* ctr,
           vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);


  /*
   * @brief - parse the vbr if data
   * @param[in] - url to send the request
   * @param[in] - ctr - controller pointer
   * @param[in] - data from which parse should happen
   * @param[out] - cfgnode_vector - config node vector
   * @param[out] - drv_resp_code_t
   */
  drv_resp_code_t parse_vbrif_resp_data(string vtn_name, string vbr_name,
                      string url, unc::driver::controller* ctr, char *data,
                      vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

 private:
  /*
   * @brief - gets the vbr url
   * @param[in] - key_vbr_t - vbr key structure
   */
  std::string get_vbr_url(key_vbr_t& key_vbr);
  /*
   * @brief  - Creates the Request Body
   * @param[in] - val - VTN value structure val_vtn_t
   * @retval - const char*  - request body formed
   */
  const char* create_request_body(const val_vbr_t& val_vtn);

  /*
   * @brief  - Validates Create Vbr
   * @param[in] - key_vbr - VBR Key Structure key_vbr_t
   * @param[in] - val_vbr - VBR value structure val_vbr_t
   * @param[in] - ctr - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_create_vbr(key_vbr_t& key_vbr,
      unc::driver::controller* ctr);

  /*
   * @brief  - Validates Delete Vbr
   * @param[in] - key_vbr - VBR Key Structure key_vbr_t
   * @param[in] - val_vbr - VBR value structure val_vbr_t
   * @param[in] - ctr - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_delete_vbr(key_vbr_t& key_vbr,
      unc::driver::controller* ctr);

  /*
   * @brief  - Validates Update Vbr
   * @param[in] - key_vbr - VBR Key Structure key_vbr_t
   * @param[in] - val_vbr - VBR value structure val_vbr_t
   * @param[in] - ctr - Controller Connection
   * @retval - drv_resp_code_t - DRVAPI_RESPONSE_SUCCESS
   *            / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t validate_update_vbr(key_vbr_t& key_vbr,
      unc::driver::controller* ctr);

  /*
   * @brief  - Checks is_vtn_exists_in_controller
   * @param[in] - key_vbr - VBR Key Structure key_vbr_t
   * @param[in] - val_vbr - VBR value structure val_vbr_t
   * @param[in] - ctr - Controller Connection
   * @uint32_t - response code from the controller
   */
  uint32_t is_vtn_exists_in_controller(const key_vbr_t& key_vbr,
      unc::driver::controller* ctr);

  /*
   * @brief  - Checks is_vtn_exists_in_controller
   * @param[in] - key_vbr - VBR Key Structure key_vbr_t
   * @param[in] - val_vbr - VBR value structure val_vbr_t
   * @param[in] - ctr - Controller Connection
   * @uint32_t - response code from the controller
   */
  uint32_t is_vbr_exists_in_controller(key_vbr_t& key_vbr,
      unc::driver::controller* ctr);

  /*
   * @brief  - GetControllerResponse and checks the resp code
   * @param[in] - url to be set to
   * @uint32_t - response code from the controller
   */
  uint32_t get_controller_response_code(std::string url,
                               unc::driver::controller* ctr,
                               unc::restjson::HttpMethod method,
                               const char* request_body);
  /*
   * @brief - parse vbr if and append it to vector
   * @param[in] - vtn , vbr name
   * @param[in] - json object
   * @param[in] - array index
   * @param[in] - url to send request
   * @param[in] - controller pointer
   * @param[out] - config node vector
   */
  drv_resp_code_t parse_vbrif_append_vector(string vtn_name, string vbr_name,
                      json_object *json_obj, int arr_idx, std::string url,
                      unc::driver::controller* ctr,
                      vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /*
   * @brief - read port map
   * @param[in] - ctr - controller pointer
   * @param[in] - url - to send the request
   * @param[out] - json_object pointer - response from controller
   */
  json_object* read_portmap(unc::driver::controller* ctr, std::string url);

 private:
  std::string age_interval_;
};
}  // namespace odcdriver
}  // namespace unc
#endif
