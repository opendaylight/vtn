/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_VTN_HH_
#define _ODC_VTN_HH_

#include <rest_client.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <unc/upll_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <tclib_module.hh>
#include <vector>
#include <string>
#include <sstream>

namespace unc {
namespace odcdriver {

class OdcVtnCommand: public unc::driver::vtn_driver_command {
 public:
  /**
   * @brief Default Constructor
   */
  OdcVtnCommand();

  /**
   * @brief Default Destructor
   */
  ~OdcVtnCommand();

  /**
   * @brief                          - Frames VTN Create command and uses rest
   *                                   client interface to send it to VTN Manager
   * @param[in] key_vtn_t            - key structure of VTN
   * @param[in] val_vtn_t            - value structure of VTN
   * @param[in] ctr                  - Controller pointer
   * @return                         - returns DRVAPI_RESPONSE_SUCCESS on
   *                                   creation of vtn/ returns
   *                                   DRVAPI_RESPONSE_FAILURE on failure
   */
  drv_resp_code_t create_cmd(key_vtn_t& key, val_vtn_t& val,
                             unc::driver::controller *ctr);

  /**
   * @brief                          - Frames VTN update command and uses rest client
   *                                   interface to send it to VTN Manager
   * @param[in] key_vtn_t            - key structure of VTN
   * @param[in] val_vtn_t            - value structure of VTN
   * @param[in] ctr                  - Controller pointer
   * @return                         - returns DRVAPI_RESPONSE_SUCCESS on
   *                                   updation of vtn /returns
   *                                   DRVAPI_RESPONSE_FAILURE on failure
   */
  drv_resp_code_t update_cmd(key_vtn_t& key, val_vtn_t& val,
                             unc::driver::controller *ctr);

  /**
   * @brief                          - Frames VTN delete command and uses rest
   *                                   client interface to send it to VTN Manager
   * @param[in] key_vtn_t            - key structure of VTN
   * @param[in] val_vtn_t            - value structure of VTN
   * @param[in] ctr                  - Controller pointer
   * @return                         - returns DRVAPI_RESPONSE_SUCCESS on
   *                                   deletion/returns DRVAPI_RESPONSE_FAILURE
   *                                   on failure
   */
  drv_resp_code_t delete_cmd(key_vtn_t& key, val_vtn_t& val,
                             unc::driver::controller *ctr);
  /**
   * @brief                          - get all the vtns from the VTN Manager
   * @param[in] ctr                  - Controller pointer
   * @param[out] cfg_node_vector      - cfg_node_vector - out parameter contains
   *                                   list of vtns present in controller
   * @return drv_resp_code_t         - returns DRVAPI_RESPONSE_SUCCESS on
   *                                   success of read all operation/returns
   *                                   DRVAPI_RESPONSE_FAILURE on failure
   */
  drv_resp_code_t get_vtn_list(unc::driver::controller* ctr,
                 std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector);

  /**
   * @brief      - Method to fetch child configurations for the parent kt
   * @param[in]  - controller pointer
   * @param[in]  - parent key type pointer
   * @param[out] - list of configurations
   * @retval     - DRVAPI_RESPONSE_SUCCESS / DRVAPI_RESPONSE_FAILURE
   */
  drv_resp_code_t fetch_config(unc::driver::controller* ctr,
                                     void* parent_key,
                                     std::vector<unc::vtndrvcache::ConfigNode *>
                                       &cfgnode_vector);

 private:
  /**
   * @brief                          - parse vtn and append to the vector
   * @param[in]                      - json_obj_vtn - json object which is to be
   *                                   parsed
   * @param[in] arr_idx              - array index - in int specifies the array
   *                                   index  -1 denotes no array
   * @param[out] cfg_node_vector      - vector to which config node needs to be
   *                                   pushed
   * @return drv_resp_code_t         - returns DRVAPI_RESPONSE_SUCCESS on
   *                                   parsing vtn and appending to vector
   *                                   successfully/returns
   *                                   DRVAPI_RESPONSE_FAILURE on failure
   */
  drv_resp_code_t fill_config_node_vector(json_object *json_obj_vtn,
                                   int arr_idx,
                                   std::vector<unc::vtndrvcache::ConfigNode *>
                                   &cfg_node_vector);
  /**
   * @brief                 - Creates the Request Body
   * @param[in] val         - VTN value structure val_vtn_t
   * @retval - json_object  - returns the request body in json_object
   */
  json_object* create_request_body(const val_vtn_t& val_vtn);

  /**
   * @brief                     - parse the vtn response data
   * @param[in]                 - data which is the response from the controller
   * @param[out] cfg_node_vector - vector to which the resp to be pushed
   * @return drv_resp_code_t    - returns DRVAPI_RESPONSE_SUCCESS on parsing the
   *                              response data/returns DRVAPI_RESPONSE_FAILURE
   *                              on failure
   */
  drv_resp_code_t parse_vtn_response(char *data,
                std::vector< unc::vtndrvcache::ConfigNode *> &cfg_node_vector);


  /**
   * @brief             - gets username password from controller or conf file
   * @param[in]         - controller pointer
   * @param[out]        - username in string
   * @param[out]        - password in string
   */
  void get_username_password(unc::driver::controller* ctr_ptr,
                             std::string &user_name, std::string &password);

  /**
   * @brief               - reads username password from conf file or default
   *                        values
   * @param[out] username - in string
   * @param[out] password - in string
   */
  void read_user_name_password(std::string &user_name,
                               std::string &password);

  /**
   * @brief - reads configuration file
   * @return - void
   */
  void read_conf_file(uint32_t &odc_port,
                      uint32_t &connection_time_out,
                      uint32_t &request_time_out);

 private:
  uint32_t idle_timeout_;
  uint32_t hard_timeout_;
};
}  // namespace odcdriver
}  // namespace unc
#endif
