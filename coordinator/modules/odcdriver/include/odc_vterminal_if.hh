/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_VTERMINAL_IF_HH
#define _ODC_VTERMINAL_IF_HH

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <odc_driver_common_defs.hh>
#include <vtn_conf_data_element_op.hh>
#include <odc_controller.hh>
#include <tclib_module.hh>
#include <vtermif.hh>
#include <vtermif_portmap.hh>
#include <string>
#include <vector>
#include <sstream>

namespace unc {
namespace odcdriver {

class OdcVtermIfCommand: public unc::driver::vtn_driver_command
                       <key_vterm_if_t, val_vterm_if_t> {
 public:
  /**
   * @brief     - Parametrised Constructor
   * @param[in] - conf file values
   */
  explicit OdcVtermIfCommand(unc::restjson::ConfFileValues_t conf_values);

  /**
   * @brief Default Destructor
   */
  ~OdcVtermIfCommand();


  /**
   * @brief      - Creates VBRIf/ PortMap
   * @param[in]  - key structure of VBRIf
   * @param[in]  - value structure of VBRIf
   * @param[in]  - Controller connection information
   * @retval     - returns DRVAPI_RESPONSE SUCCESS on creation of vterm_if successfully
   *               /returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode create_cmd(key_vterm_if_t& vterm_if_key,
                         val_vterm_if_t& vterm_if_val,
                         unc::driver::controller *ctr_ptr);

  /**
   * @brief                      - Updates VBRIf
   * @param[in] key              - key structure of VBRIf
   * @param[in] val              - Old value structure of VBRIf
   * @param[in] val              - New value structure of VBRIf
   * @param[in] conn             - Controller connection information
   * @retval UncRespCode         - returns UNC_RC_SUCCESS on updation of VBRIf
   *                               successfully/returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode update_cmd(key_vterm_if_t& key,
                         val_vterm_if_t& val_old,
                         val_vterm_if_t& val_new,
                         unc::driver::controller *ctr_ptr);

  /**
   * @brief                    - Deletes VBRIf
   * @param[in] key            - key structure of VBRIf
   * @param[in] val            - value structure of VBRIf
   * @param[in] conn           - Controller connection information
   * @return UncRespCode       - returns UNC_RC_SUCCESS on deletion of
   *                             VBRIf successfully /returns
   *                             UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode delete_cmd(key_vterm_if_t& key,
                         val_vterm_if_t& val,
                         unc::driver::controller *ctr_ptr);

  /**
   * @brief      - Method to fetch child configurations for the parent kt
   * @param[in]  - controller pointer
   * @param[in]  - parent key type pointer
   * @param[out] - list of configurations
   * @retval     - UNC_RC_SUCCESS / UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode fetch_config(
      unc::driver::controller* ctr_ptr,
      void* parent_key,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

 private:
  /**
   * @brief                           - parse the vterminal if data
   * @param[in] vtn_name              - vtn name
   * @param[in] vterminal_name              - vterminal name
   * @param[in] url                   - url to send the request
   * @param[in] ctr                   - controller pointer
   * @param[in] data                  - data from which parse should happen
   * @param[out] cfgnode_vector       - config node vector
   * @return UncRespCode              - returns UNC_RC_SUCCESS on parsing the
   *                                    response of vterm_if/returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode parse_vterm_if_response(
      std::list<vterm_if_conf> &vtermif_detail,
      std::string vtn_name,
      std::string vterminal_name,
      unc::driver::controller* ctr_ptr,
      std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /**
   * @brief                       - parse vterminal if and append it to vector
   * @param[in] vtn_name          - vtn name
   * @param[in] vterminal_name          - vterminal name
   * @param[in] json_obj          - json object
   * @param[in] arr_idx           - array index
   * @param[in] url               - url to send request
   * @param[in] ctr               - controller pointer
   * @param[out] cfgnode_vector   - config node vector
   * @return UncRespCode          - returns UNC_RC_SUCCESS on parsing vterm_if and appending
   *                                vector/ returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode fill_config_node_vector(
      unc::driver::controller* ctr,
      std::string name,
      std::string vtn_name,
      std::string vterminal_name,
      bool enabled,
      std::string description,
      std::vector< unc::vtndrvcache::ConfigNode *>&cfgnode_vector);


  /**
   * @brief                   - read portmap
   * @param[in] - ctr         - controller pointer
   * @param[in] - url         - url in string
   * @param[out] - resp_code  - response code
   * return json_object       - json object
   */
  /*json_object*  read_portmap(unc::driver::controller* ctr,
                             std::string url,
                             int &resp_code);*/
  UncRespCode read_portmap(
      unc::driver::controller* ctr_ptr,
      key_vterm_if_t& vterm_if_key,
      std::list<vterm_if> &vtermif_port_detail,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /**
   * @brief                  - Constructs url for vterm_if
   * @param[in] term_iff_key    - key structure of vterm_if key structure
   * @return                 - returns the url of VBRIf
   */
  std::string get_vterm_if_url(key_vterm_if_t& vterm_if_key);

  /**
   * @brief                    - Creates the Request Body for portmap
   * @param[in]  vterm_if_val     - VTN value structure val_vtn_t
   * @param[in]  logical port  - logical port id
   * @return  const char*      - returns the request body formed
   */
  void create_request_body_port_map(val_vterm_if_t& vterm_if_val,
                                    key_vterm_if_t& vterm_if_key,
                                    const std::string &logical_port_id,
                                    ip_vtermif_port&  ip_vtermif_port_st);

  /**
   * @brief                - Creates the Request Body
   * @param[in] val_vtn    - VTN value structure val_vtn_t
   * @return const char*   - returns the request body formed
   */
  void create_request_body(val_vterm_if_t& val_vtn,
                             key_vterm_if_t& vterm_if_key,
                             ip_vterminal_config&  ip_vterminal_config_st);


  void update_request_body(val_vterm_if_t& val_vterm_if,
                                              key_vterm_if_t& vterm_if_key,
                                 ip_vterminal_config&  ip_vterminal_config_st);
  void delete_request_body(val_vterm_if_t& val_vterm_if,
                                              key_vterm_if_t& vterm_if_key,
                                  ip_vterminal_config&  ip_vterminal_config_st);
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

  /**
   * @brief                     - converts the format of logical port id
   * @param[in][out]            - the converted logical port id
   * @return odc_drv_resp_code_t- ODC_DRV_SUCCESS/ODC_DRV_FAILURE
   */
  odc_drv_resp_code_t convert_logical_port(std::string &logical_port_id);

 private:
   unc::restjson::ConfFileValues_t conf_file_values_;
};
}  // namespace odcdriver
}  // namespace unc
#endif

