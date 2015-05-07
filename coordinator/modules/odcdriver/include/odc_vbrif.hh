/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_VBRIF_HH
#define _ODC_VBRIF_HH

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <odc_driver_common_defs.hh>
#include <vtn_conf_data_element_op.hh>
#include <odc_controller.hh>
#include <tclib_module.hh>
#include <string>
#include <vector>
#include <sstream>

namespace unc {
namespace odcdriver {

class OdcVbrIfCommand: public unc::driver::vtn_driver_command
                       <key_vbr_if_t, pfcdrv_val_vbr_if_t> {
 public:
  /**
   * @brief     - Parametrised Constructor
   * @param[in] - conf file values
   */
  explicit OdcVbrIfCommand(unc::restjson::ConfFileValues_t conf_values);

  /**
   * @brief Default Destructor
   */
  ~OdcVbrIfCommand();

  /**
   * @brief      - Creates VBRIf/ PortMap
   * @param[in]  - key structure of VBRIf
   * @param[in]  - value structure of VBRIf
   * @param[in]  - Controller connection information
   * @retval     - returns DRVAPI_RESPONSE SUCCESS on creation of vbrif successfully
   *               /returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode create_cmd(key_vbr_if_t& key,
                         pfcdrv_val_vbr_if_t& val,
                         unc::driver::controller *conn);

  /**
   * @brief                      - Updates VBRIf
   * @param[in] key              - key structure of VBRIf
   * @param[in] val              - value structure of VBRIf
   * @param[in] conn             - Controller connection information
   * @retval UncRespCode         - returns UNC_RC_SUCCESS on updation of VBRIf
   *                               successfully/returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode update_cmd(key_vbr_if_t& key,
                         pfcdrv_val_vbr_if_t& val,
                         unc::driver::controller *conn);

  /**
   * @brief                    - Deletes VBRIf
   * @param[in] key            - key structure of VBRIf
   * @param[in] val            - value structure of VBRIf
   * @param[in] conn           - Controller connection information
   * @return UncRespCode       - returns UNC_RC_SUCCESS on deletion of
   *                             VBRIf successfully /returns
   *                             UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode delete_cmd(key_vbr_if_t& key,
                         pfcdrv_val_vbr_if_t& val,
                         unc::driver::controller *conn);

  /**
   * @brief                           - get all the vbr child
   * @param[in] vtn_name              - vtn name
   * @param[in] vbr_name              - vbr name
   * @param[in] ctr                   - controller pointer
   * @param[out] cfgnode_vector       - config node vector
   * @return UncRespCode              - returns UNC_RC_SUCCESS on successfully retieving a vbr
   *                                    child /returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode get_vbrif_list(
      std::string vtn_name,
      std::string vbr_name,
      unc::driver::controller* ctr,
      std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

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
   * @brief                           - parse the vbr if data
   * @param[in] vtn_name              - vtn name
   * @param[in] vbr_name              - vbr name
   * @param[in] url                   - url to send the request
   * @param[in] ctr                   - controller pointer
   * @param[in] data                  - data from which parse should happen
   * @param[out] cfgnode_vector       - config node vector
   * @return UncRespCode              - returns UNC_RC_SUCCESS on parsing the
   *                                    response of vbrif/returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode parse_vbrif_response(
      std::string vtn_name,
      std::string vbr_name,
      std::string url,
      unc::driver::controller* ctr,
      char *data,
      std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /**
   * @brief                       - parse vbr if and append it to vector
   * @param[in] vtn_name          - vtn name
   * @param[in] vbr_name          - vbr name
   * @param[in] json_obj          - json object
   * @param[in] arr_idx           - array index
   * @param[in] url               - url to send request
   * @param[in] ctr               - controller pointer
   * @param[out] cfgnode_vector   - config node vector
   * @return UncRespCode          - returns UNC_RC_SUCCESS on parsing vbrif and appending
   *                                vector/ returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode fill_config_node_vector(
      std::string vtn_name,
      std::string vbr_name,
      json_object *json_obj,
      uint32_t arr_idx,
      std::string url,
      unc::driver::controller* ctr,
      std::vector< unc::vtndrvcache::ConfigNode *>&cfgnode_vector);

  /**
   * @brief                   - read portmap
   * @param[in] - ctr         - controller pointer
   * @param[in] - url         - url in string
   * @param[out] - resp_code  - response code
   * return json_object       - json object
   */
  json_object*  read_portmap(unc::driver::controller* ctr,
                             std::string url,
                             int &resp_code);

    /**
   * @brief                  - Constructs url for vbrif
   * @param[in] vbrif_key    - key structure of vbrif key structure
   * @return                 - returns the url of VBRIf
   */
  std::string get_vbrif_url(key_vbr_if_t& vbrif_key);

  /**
   * @brief                    - Creates the Request Body for portmap
   * @param[in]  vbrif_val     - VTN value structure val_vtn_t
   * @param[in]  logical port  - logical port id
   * @return  const char*      - returns the request body formed
   */
  json_object* create_request_body_port_map(pfcdrv_val_vbr_if_t& vbrif_val,
                                            const std::string &logical_port_id);

  /**
   * @brief                - Creates the Request Body
   * @param[in] val_vtn    - VTN value structure val_vtn_t
   * @return const char*   - returns the request body formed
   */
  json_object* create_request_body(pfcdrv_val_vbr_if_t& val_vtn);

  /*
   * @brief       - validated the format of logical port id
   * @param[in]   - logical_port_id which needs to be validated
   * @return      - returns ODC_DRV_SUCCESS/ ODC_DRV_FAILURE
   */
  odc_drv_resp_code_t validate_logical_port_id(
      const std::string& logical_port_id);

  /**
   * @brief                     - checks the logical port id format
   * @param[in][out]            - logical_port_id
   * @return                    - odc_drv_resp_code_t
   */
  odc_drv_resp_code_t check_logical_port_id_format(
      std::string& logical_port_id);

 private:
  unc::restjson::ConfFileValues_t conf_file_values_;
};
}  // namespace odcdriver
}  // namespace unc
#endif
