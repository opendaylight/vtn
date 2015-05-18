/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_VBR_HH_
#define _ODC_VBR_HH_

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <json_build_parse.hh>
#include <vtn_conf_data_element_op.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <odc_vtn.hh>
#include <tclib_module.hh>
#include <string>
#include <vector>
#include <sstream>

namespace unc {
namespace odcdriver {

class OdcVbrCommand: public unc::driver::vtn_driver_command
                     <key_vbr_t, val_vbr_t> {
 public:
  /**
   * @brief                         - Parametrised Constructor
   * @param[in]                     - conf file values
   */
  explicit OdcVbrCommand(unc::restjson::ConfFileValues_t conf_values);

  /**
   * @brief Default Destructor
   */
  ~OdcVbrCommand();

  /**
   * @brief                          - Constructs VBR command and send it to
   *                                   rest interface
   * @param[in] key_vbr              - key structure of VBR
   * @param[in] val_vbr              - value structure of VBR
   * @param[in] ctr                  - Controller pointrt
   * @return drv_resp_t              - returns UNC_RC_SUCCESS on creating vbr successfully
   *                                   /returns UNC_DRV_RC_ERR_GENERIC on failure
   */

  UncRespCode create_cmd(key_vbr_t& key_vbr,
                         val_vbr_t& val_vbr,
                         unc::driver::controller *ctr);

  /**
   * @brief                           - Constructs VBR update command and send
   *                                    it to rest interface
   * @param[in] key_vbr               - key structure of VBR
   * @param[in] val_vbr               - old value structure of VBR
   * @param[in] val_vbr               - New value structure of VBR
   * @param[in] ctr                   - Controller pointer
   * @return UncRespCode              - returns UNC_RC_SUCCESS on updating vbr successfully
   *                                    /returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode update_cmd(key_vbr_t& key_vbr,
                         val_vbr_t& val_old_vbr,
                         val_vbr_t& val_new_vbr,
                         unc::driver::controller* ctr);

  /**
   * @brief                           - Constructs VBR Delete command and send
   *                                    it to rest interface
   * @param[in] key_vbr               - key structure of VBR
   * @param[in] val_vbr               - value structure of VBR
   * @param[in] ctr                   - Controller pointer
   * @return  UncRespCode             - returns UNC_RC_SUCCESS on deleting a vbr
   *                                    / returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode delete_cmd(key_vbr_t& key_vbr,
                         val_vbr_t& val_vbr,
                         unc::driver::controller *ctr);

  /**
   * @brief                          - get vbr list - gets all the vbridge
   *                                   under particular vtn
   * @param[in]                      - vtn name
   * @param[in] ctr                  - Controller pointer
   * @param[out] cfg_node_vector     - cfg_node_vector out parameter contains
   *                                   list of vbridge present for specified vtn
   *                                   in controller
   * @return UncRespCode             - returns UNC_RC_SUCCESS on
   *                                   retrieving the vtn child successfully/
   *                                   returns UNC_DRV_RC_ERR_GENERIC on fail
   */
  UncRespCode get_vbr_list(
      std::string vtnname,
      unc::driver::controller* ctr,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector);

  private:
  /**
   * @brief               - gets the vbr url
   * @param[in] key_vbr   - vbr key structure
   * @return std::string  - returns the url string of vbr
   */
  std::string get_vbr_url(key_vbr_t& key_vbr);

  /**
   * @brief               - Creates the Request Body
   * @param[in] val_vtn   - VTN value structure val_vtn_t
   * @return const char*  - returns the request body formed
   */
  json_object* create_request_body(const val_vbr_t& val_vtn);

  /**
   * @brief                      - parse the vbr response data
   * @param[in] data             - data which is the response from controller
   * @param[in] vtn_name         - vtn name
   * @param[out] cfg_node_vector - vector to which the resp to be pushed
   * @return UncRespCode         - returns UNC_RC_SUCCESS on parsing vbr
   *                               reponse data successfully/returns
   *                               UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode parse_vbr_response(
      char *data,
      std::string vtn_name,
      unc::driver::controller* ctr,
      std::vector< unc::vtndrvcache::ConfigNode *> &cfg_node_vector);

  /**
   * @brief                          - parse vbr information and append to the vector
   * @param[in] ctr                  - controller pointer
   * @param[in] json_obj_vbr         - json object which is to be parsed
   * @param[in] arr_idx              - array index in int specifies the array
   *                                   index  -1 denotes no array
   * @param[out] cfg_node_vector     - vector to which config node needs
   *                                   to be pushed
   * @return UncRespCode             - returns UNC_RC_SUCCESS on
   *                                   parsing vbr and appending to vector
   *                                   successfully/returns
   *                                   UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode fill_config_node_vector(
      unc::driver::controller* ctr,
      json_object *json_obj_vbr,
      std::string vtn_name,
      uint32_t arr_idx,
      std::vector< unc::vtndrvcache::ConfigNode *> &cfg_node_vector);

  /**
   * @brief                      - read port map
   * @param[in] ctr              - controller pointer
   * @param[in] url              - url to send the request
   * @return json_object pointer - returns the response from controller
   */
  json_object* read_portmap(unc::driver::controller* ctr, std::string url);

  /**
   * @brief      - Method to fetch child configurations for the parent kt
   * @param[in]  - controller pointer
   * @param[in]  - parent key type pointer
   * @param[out] - list of configurations
   * @retval     - UNC_RC_SUCCESS / UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode fetch_config(
      unc::driver::controller* ctr,
      void* parent_key,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  private:
  uint32_t age_interval_;
  unc::restjson::ConfFileValues_t conf_file_values_;
};
}  // namespace odcdriver
}  // namespace unc
#endif
