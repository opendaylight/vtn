/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_VTN_HH_
#define _ODC_VTN_HH_

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <unc/upll_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <tclib_module.hh>
#include <vtn.hh>
#include <vector>
#include <string>
#include <sstream>

namespace unc {
namespace odcdriver {

class OdcVtnCommand: public unc::driver::vtn_driver_command
                     <key_vtn_t, val_vtn_t> {
 public:
  /**
   * @brief                          - Parametrised Constructor
   * @param[in]                      - conf file values
   */
  explicit OdcVtnCommand(unc::restjson::ConfFileValues_t conf_values);

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
   * @return                         - returns UNC_RC_SUCCESS on
   *                                   creation of vtn/ returns
   *                                   UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode create_cmd(key_vtn_t& key, val_vtn_t& val,
                         unc::driver::controller *ctr_ptr);

  /**
   * @brief                          - Frames VTN update command and uses rest client
   *                                   interface to send it to VTN Manager
   * @param[in] key_vtn_t            - key structure of VTN
   * @param[in] val_vtn_t            - Old value structure of VTN
   * @param[in] val_vtn_t            - New value structure of VTN
   * @param[in] ctr                  - Controller pointer
   * @return                         - returns UNC_RC_SUCCESS on
   *                                   updation of vtn /returns
   *                                   UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode update_cmd(key_vtn_t& key, val_vtn_t& val_old,
                         val_vtn_t& val_new,
                         unc::driver::controller *ctr_ptr);

  /**
   * @brief                          - Frames VTN delete command and uses rest
   *                                   client interface to send it to VTN Manager
   * @param[in] key_vtn_t            - key structure of VTN
   * @param[in] val_vtn_t            - value structure of VTN
   * @param[in] ctr                  - Controller pointer
   * @return                         - returns UNC_RC_SUCCESS on
   *                                   deletion/returns UNC_DRV_RC_ERR_GENERIC
   *                                   on failure
   */
  UncRespCode delete_cmd(key_vtn_t& key, val_vtn_t& val,
                         unc::driver::controller *ctr_ptr);

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
  /**
   * @brief                          - parse vtn and append to the vector
   * @param[in]                      - json_obj_vtn - json object which is to be
   *                                   parsed
   * @param[in] arr_idx              - array index - in int specifies the array
   *                                   index  -1 denotes no array
   * @param[out] cfg_node_vector     - vector to which config node needs to be
   *                                   pushed
   * @return UncRespCode             - returns UNC_RC_SUCCESS on
   *                                   parsing vtn and appending to vector
   *                                   successfully/returns
   *                                   UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode fill_config_node_vector(
      std::string name,
      std::string descp,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector);

  /**
   * @brief                 - Creates the Request Body
   * @param[in] val         - VTN value structure val_vtn_t
   * @retval - json_object  - returns the request body in json_object
   */
  UncRespCode create_request_body(const val_vtn_t& val_vtn,
                           key_vtn_t& key_vtn,
                           ip_vtn& ip_vtn_st);

  /**
   * @brief                 - Update Request Body
   * @param[in] val         - VTN value structure val_vtn_t
   * @retval - json_object  - returns the request body in json_object
   */
  UncRespCode update_request_body(const val_vtn_t& val_vtn,
                           key_vtn_t& key_vtn,
                           ip_vtn& ip_vtn_st);
  /**
   * @brief                 - delete Request Body
   * @param[in] val         - VTN value structure val_vtn_t
   * @retval - json_object  - returns the request body in json_object
   */
  UncRespCode delete_request_body(const val_vtn_t& val_vtn,
                           key_vtn_t& key_vtn,
                           ip_vtn& ip_vtn_st);


  /**
   * @brief                      - parse the vtn response data
   * @param[in]                  - data which is the response from the controller
   * @param[out] cfg_node_vector - vector to which the resp to be pushed
   * @return UncRespCode         - returns UNC_RC_SUCCESS on parsing the
   *                              response data/returns UNC_DRV_RC_ERR_GENERIC
   *                              on failure
   */
  UncRespCode parse_vtn_response(
      std::list<vtn_conf> &vtn_detail,
      std::vector< unc::vtndrvcache::ConfigNode *> &cfg_node_vector);

 private:
  unc::restjson::ConfFileValues_t conf_file_values_;
};
}  // namespace odcdriver
} //namespace unc
#endif
