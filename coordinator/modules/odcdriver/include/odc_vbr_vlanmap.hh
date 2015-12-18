/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_VBR_VLANMAP_HH
#define _ODC_VBR_VLANMAP_HH

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <odc_driver_common_defs.hh>
#include <vtn_conf_data_element_op.hh>
#include <odc_controller.hh>
#include <tclib_module.hh>
#include <vlanmap.hh>
#include <string>
#include <vector>
#include <sstream>

namespace unc {
namespace odcdriver {

class OdcVbrVlanMapCommand: public unc::driver::vtn_driver_command
                            <key_vlan_map_t, pfcdrv_val_vlan_map_t> {
 public:
  /**
   * @brief                     - Parametrised Constructor
   * @param[in]                 - conf file values
   */
  explicit OdcVbrVlanMapCommand(unc::restjson::ConfFileValues_t conf_values);

  /**
   * @brief Default Destructor
   */
  ~OdcVbrVlanMapCommand();

  /**
   * @brief                      - Forms VBR_VLANMAP command and send it to
   *                               restclient interface to create vbrvlanmap
   * @param[in] key              - key structure of VBR_VLANMAP
   * @param[in] val              - value structure of VBR_VLANMAP
   * @param[in] conn             - Controller pointer
   * @retval UncRespCode         - returns DRVAPI_RESPONSE SUCCESS on creation
   *                             - of VBR_VLANMAP successfully
   *                               /returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode create_cmd(key_vlan_map_t& key,
                         pfcdrv_val_vlan_map_t& val,
                         unc::driver::controller *conn);

  /**
   * @brief                      - Forms VBR_VLANMAP command and send it to
   *                               restclient interface to update vbrvlanmap
   * @param[in] key              - key structure of VBR_VLANMAP
   * @param[in] val              - Old value structure of VBR_VLANMAP
   * @param[in] val              - New value structure of VBR_VLANMAP
   * @param[in] conn             - Controller pointer
   * @retval UncRespCode         - returns UNC_RC_SUCCESS on
   *                             - updation of VBR_VLANMAP successfully
   *                             - returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode update_cmd(key_vlan_map_t& key,
                         pfcdrv_val_vlan_map_t& val_old,
                         pfcdrv_val_vlan_map_t& val_new,
                         unc::driver::controller *conn);

  /**
   * @brief                      - Deletes VBR_VLANMAP and send it to restclient
   *                               interface to delete vbrvlanmap
   * @param[in] key              - key structure of VBR_VLANMAP
   * @param[in] val              - value structure of VBR_VLANMAP
   * @param[in] conn             - Controller pointer
   * @return UncRespCode         - returns UNC_RC_SUCCESS on deletion of
   *                             - VBRIf successfully /returns
   *                             - UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode delete_cmd(key_vlan_map_t& key,
                         pfcdrv_val_vlan_map_t& val,
                         unc::driver::controller *conn);

  /**
   * @brief                      - Method to fetch child configurations for the parent kt
   * @param[in] ctr              - controller pointer
   * @param[in] parent_key       - parent key type pointer
   * @param[out] cfgnode_vector  - config node vector
   * @retval                     - UNC_RC_SUCCESS / UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode fetch_config(
      unc::driver::controller* ctr,
      void* parent_key,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

 private:
  /**
   * @brief                      - parse the VBR_VLANMAP data
   * @param[in] parent_key       - parent type key pointer
   * @param[in] ctr              - controller pointer
   * @param[in] data             - data from which parse should happen
   * @param[out] cfgnode_vector  - config node vector
   * @return UncRespCode         - returns UNC_RC_SUCCESS on parsing the
   *                              response of vbrif/returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode parse_vbrvlanmap_response(
      void *parent_key,
      unc::driver::controller* ctr_ptr,
      std::list<vlan_conf> &vlan_detail,
      std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /**
   * @brief                      - parse VBR_VLANMAP and append it to config
   *                               node vector
   * @param[in] parent_key       - parent type key pointer
   * @param[in] ctr              - controller pointer
   * @param[in] json_obj         - json_object pointer contains the controller response
   * @param[in] arr_idx          - array index of vlanmap
   * @param[out] cfgnode_vector  - config node vector
   * @return UncRespCode         - returns UNC_RC_SUCCESS on success
   *                               / returns UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode fill_config_node_vector(
      void *parent_key,
      unc::driver::controller* ctr_ptr,
      std::string mapid, uint16_t vlanid,
      std::vector< unc::vtndrvcache::ConfigNode *>&cfgnode_vector);

  /**
   * @brief                      - Constructs url for VBR_VLANMAP
   * @param[in] vbrif_key        - key structure of VBR_VLANMAP
   * @return string              - returns the url of VBR_VLANMAP
   */
  std::string get_vbrvlanmap_url(key_vlan_map_t& vbrif_key);

  /**
   * @brief                      - Delete Request Body
   * @param[in] vlanmap_key      - key structure VBRVLANMAP
   * @param[in] vlanmap_val      - val structureof VBRVLANMAP
   * @param[in] logical port id  - validated logical port id
   * @return json_object         - returns the request body formed in
   *                               json_object pointer
   */
  void delete_request_body(key_vlan_map_t& vlanmap_key,
                                   pfcdrv_val_vlan_map_t& vlanmap_val,
                                   ip_vlan_config&  ip_vlan_config_st);

  /**
   * @brief                      - Creates the Request Body
   * @param[in] vlanmap_key      - key structure VBRVLANMAP
   * @param[in] vlanmap_val      - val structureof VBRVLANMAP
   * @param[in] logical port id  - validated logical port id
   * @return json_object         - returns the request body formed in
   *                               json_object pointer
   */
  void create_request_body(key_vlan_map_t& vlanmap_key,
                                   pfcdrv_val_vlan_map_t& vlanmap_val,
                                   ip_vlan_config&  ip_vlan_config_st,
                                   const std::string &logical_port_id);

  /*
   * @brief                      - validates the format of logical port id
   * @param[in] logical_port_id  - logical_port_id which needs to be validated
   * @return                     - returns ODC_DRV_SUCCESS/ ODC_DRV_FAILURE
   */
  odc_drv_resp_code_t validate_logical_port_id(
      const std::string& logical_port_id);

  /**
   * @brief                      - Generate vlanmap id in the format
   *                               [Switchid.vlanid or ANY.vlanid]
   * @param[in]                  - key structure of VBR_VLANMAP
   * @param[in]                  - vlan id as string
   * @param[in]                  - validated logical port id
   * @return string              - vlanmap id string
   */
  std::string generate_vlanmap_id(key_vlan_map_t& vlanmap_key,
                                  std::string str_vlanid,
                                  const std::string &logical_port);

  /**
   * @brief                      - create/update vlan and send to restclient
   * @param[in] vlanmap_key      - key structure of VBR_VLANMAP
   * @param[in] vlanmap_val      - value structure of VBR_VLANMAP
   * @param[in] ctr_ptr          - controller pointer
   * @retval UncRespCode         - returns UNC_RC_SUCCESS/
   *                             - UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode create_update_cmd(key_vlan_map_t& vlanmap_key,
                                pfcdrv_val_vlan_map_t& vlanmap_val,
                                unc::driver::controller* ctr_ptr);
  /**
   * @brief                      - Delete vlan-map configuration from controller by
   *                               sending request to rest client interface
   * @param[in] vlanmap_key      - key structure of VBR_VLANMAP
   * @param[in] ctr_ptr          - controller pointer
   * @param[in]  port_id         - port id
   * @retval UncRespCode         - returns UNC_RC_SUCCESS/
   *                             - UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode del_existing_vlanmap(
      key_vlan_map_t& vlanmap_key,
      pfcdrv_val_vlan_map_t& vlanmap_val,
      unc::driver::controller* ctr_ptr,
      const std::string &port_id);

  /**
   * @brief                      - generates vlan id from the vlan-map vector
   * @param[in] ctr_ptr          - controller pointer
   * @param[in] vlanmap_key      - key structure of vbrvlanmap
   * @param[in] logical port     - validated logical port id
   * @retval                     - vlan id in the form of string
   */
  std::string generate_vlanid_from_vector(unc::driver::controller* ctr_ptr,
                                          key_vlan_map_t &vlanmap_key,
                                          std::string logical_port);

  /**
   * @brief                      - check switch id exists in controller
   * @param[in] key_vlan_map     - VBRVLANMAP key structure
   * @param[in] val_vlan_map     - VBRVLANMAP val structure
   * @param[in] logical port     - validated logical port
   * @param[in] ctr_ptr          - controller pointer
   * @param[out]is_switch_exist  - pfc_bool_t variable set to PFC_TRUE if switch
   *                               already exists in controller
   * @param[out] port_id         - existing switch id in controller
   * @return UncRespCode         - returns
   *                               UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode check_switch_already_exists(key_vlan_map_t &key_vlan_map,
                                          pfcdrv_val_vlan_map_t &val_vlan_map,
                                          const std::string &logical_port,
                                          unc::driver::controller *ctr_ptr,
                                          pfc_bool_t &is_switch_exist,
                                          std::string &port_id);

  /*
   * @brief                      - Check  "ANY" vlan-id (vlan-id not associated
   *                               with any switch) exists in controller
   * @param[in] key_vlan_map     - VBRVLANMAP key structure
   * @param[in] val_vlan_map     - VBRVLANMAP val structure
   * @param[in] ctr_ptr          - controller pointer
   * @param[out]is_switch_exist  - pfc_bool_t variable set to PFC_TRUE if "ANY"
   *                               already exists in controller
   * @param[out] port_id         - existing "ANY" vlan-id stored in controller
   * @return UncRespCode         - returns
   *                               UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode check_ANY_already_exists(key_vlan_map_t &key_vlan_map,
                                       pfcdrv_val_vlan_map_t &val_vlan_map,
                                       unc::driver::controller *ctr,
                                       pfc_bool_t &is_switch_exist,
                                       std::string &port_id);

  /*
   * @brief                      - Validates VLAN exists or not
   * @param[in] key_vlan_map     - VBRVLANMAP key structure
   * @param[in] val_vlan_map     - VBRVLANMAP val structure
   * @param[in] logical port     - validated logical port
   * @param[in] ctr_ptr          - controller pointer
   * @param[out]is_switch_exist  - pfc_bool_t variable set to PFC_TRUE if
   *                               "ANY"/switch id already exists in controller
   * @param[out] port_id         - existing "ANY"/SW id stored in the controller
   * @return UncRespCode         - returns
   *                               UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode validate_vlan_exist(key_vlan_map_t &key_vlan_map,
                                  pfcdrv_val_vlan_map_t &val_vlan_map,
                                  const std::string &logical_port,
                                  unc::driver::controller *ctr,
                                  pfc_bool_t &is_switch_exist,
                                  std::string &port_id);

  /**
   * @brief                      - Deletes particular entry from vlan-map vector
   * @param[in] ctr_             - controller pointer
   * @param[in] vtn_vbr_vlan     - this entry needs to be deleted
   */
  void delete_from_vector(unc::driver::controller *ctr,
                          std::string vtn_vbr_vlan);

  /**
   * @brief                      - Updates particular entry in vector
   * @param[in] ctr              - controller pointer
   * @param[in] vtn_vbr_vlan     - this entry needs to be updated into vector
   */
  void update_vector(unc::driver::controller *ctr,
                     std::string vtn_vbr_vlan);

  /**
   * @brief                      - parses the vector and gets vtn, vbr, sw id as out param
   * @param[in] vtn_vbr_vlan_data- data to be parsed
   * @param[out] vtn_name_ctr    - vtn name in string
   * @param[out] vbr_name_ctr    - vbr name in string
   * @param[out] switch_id_ctr   - switch id in string
   */
  void parse_data_from_vector(const std::string &vtn_vbr_vlan_data,
                              std::string &vtn_name_ctr,
                              std::string &vbr_name_ctr,
                              std::string &switch_id_ctr);

  /**
   * @brief                     - generates string of format vtn.vbr.swid
   * @param[in] vtn_name        - vtn name
   * @param[in] vbr_name        - vbr name
   * @param[in] vlan_id         - sw id.vlan id
   * @return string             - generated string format
   */
  std::string generate_string_for_vector(const std::string &vtn_name,
                                         const std::string &vbr_name,
                                         const std::string &vlan_id,
                                         unc::driver::controller *ctr);

  /**
   * @brief                     - converts switch id to hexadecimal format
   * @param[in] node_id         - vtn manager node id format
   * @return string             - generated string format
   */
  std::string frame_switchid_hex(std::string &node_id);

 private:
  unc::restjson::ConfFileValues_t conf_file_values_;
};
}  // namespace odcdriver
}  // namespace unc
#endif
