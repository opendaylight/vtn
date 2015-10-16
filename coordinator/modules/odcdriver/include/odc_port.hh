/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_PORT_HH_
#define _ODC_PORT_HH_

#include <rest_util.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <unc/uppl_common.h>
#include <vtn_drv_module.hh>
#include <unc/unc_events.h>
#include <vtndrvintf_defs.h>
#include <driver/driver_command.hh>
#include <port.hh>
#include <string>
#include <list>
#include <vector>
#include <map>

namespace unc {
namespace odcdriver {

class OdcPort : public unc::driver::vtn_driver_read_command {
 public:
  /**
   * @brief Default Constructor
   */
  explicit OdcPort(unc::restjson::ConfFileValues_t conf_values);

  /**
   * @brief Default Destructor
   */
  ~OdcPort();

  /**
   * @brief                          - get all the vtns from the VTN Manager
   * @param[in] ctr                  - Controller pointer
   * @param[in] parent_key           - parent key is key_switch
   * @param[in] cache_empty          - PFC_TRUE/ PFC_FALSE
   * @return UncRespCode             - returns UNC_RC_SUCCESS on
   *                                   success of read all operation/returns
   *                                   UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode fetch_config(unc::driver::controller* ctr,
                               key_switch_t *parent_key,
                               const pfc_bool_t cache_empty);

  /**
   * @brief                           - notify logical port to physical
   * @param[in] type                  - operation type
   * @param[in] key_port              - key port structure pointer
   * @param[in] val_port              - val port struture pointer
   * @param[in] val_old_port          - val port old structure pointer
   */
  void notify_logical_port_physical(unc::driver::oper_type type,
                                    key_port_t *key_port,
                                    val_port_st_t *val_port,
                                    val_port_st_t *val_old_port);

  // For reading PORT statistics
  UncRespCode read_cmd(unc::driver::controller *ctr,
                        unc::vtnreadutil::driver_read_util*);
 /**
   * @brief                          - retruns the map
   * @param[out] link_map_           - link_map_which contains
   *                                   switch id, portname, state
   *                                   value and config value
   */
  std::map <std::string, std::string> get_linkmap( ) {
        return link_map_;
      }

 private:
  /**
   * @brief                          - parse port and append to the vector
   * @param[in]                      - ctr controller pointer
   * @param[in]                      - json_obj_node_prop - json object which is to be
   *                                   parsed
   * @param[in] arr_idx              - array index - in int specifies the array
   *                                   index  -1 denotes no array
   * @param[out] cfg_node_vector     - vector to which config node needs to be
   *                                   pushed
   * @return UncRespCode             - returns UNC_RC_SUCCESS on
   *                                   parsing port and appending to vector
   *                                   successfully/returns
   *                                   UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode fill_config_node_vector(
      unc::driver::controller *ctr_ptr,
      std::string cost,
      std::string id,
      std::string name,
      uint enabled,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector);

  /**
   * @brief                          - parses the port properties values from
   *                                   the json object
   * @param[in] json_obj_node_conn   - json object from which values to be
   *                                   parsed
   * @param[out] name_value          - name value is parsed and its out param
   * @param[out] state_value         - state_value is parsed and its out param
   * @param[out] config_value        - config_value is parsed and its out param
   * @param[out] speed               - speed value is parsed and its out param
   * @return UncRespCode             - returns
   *                                    UNC_RC_SUCCESS/UNC_DRV_RC_ERR_GENERIC
   */

  UncRespCode parse_port_properties_value(
      int arr_idx,
      json_object *json_obj_node_conn,
      std::string &name_value,
      uint &state_value,
      uint &config_value,
      unsigned long long &speed);
  /**
   * @brief                          - Compare whether cache empty or not
   * @param[in]                      - ctr controller pointer
   * @param[in]                      - filled config node vector
   * @param[in] switch id            - switch id in string
   * @param[in] cache_empty          - cache_empty pfc_bool_t
   * @return UncRespCode             - returns UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */

  UncRespCode compare_with_cache(
      unc::driver::controller *ctr_ptr,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
      const std::string &switch_id, const pfc_bool_t cache_empty);

  /**
   * @brief                          - Notify physical about the port event
   * @param[in] type                 - operation type
   * @param[in] key_port             - key structure of port
   * @param[in] val_port             - value structure of port
   * @param[in] val_old_port         - old value structure used only in case of UPDATE
   */
  void notify_physical(unc::driver::oper_type type,
                       key_port_t *key_port,
                       val_port_st_t *val_port,
                       val_port_st_t *val_old_port);
  /**
   * @brief                          - update the local list maintained
   * @param[in] key_port             - key structure of port
   * @param[out] port_list           - local list maintained for delete contains port id
   */
  void update_list(key_port_t *key_port, std::list<std::string> &port_list);

  /**
   * @brief                          - add  event to be sent to UPPL and cache
   * @param[in] ctr_ptr              - controller pointer
   * @param[in] cfg_node             - Config Node pointer
   * @param[out] port_list           - local list maintained for delete contains port id
   * @return UncRespCode             - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */

  UncRespCode add_event(unc::driver::controller *ctr_ptr,
                            unc::vtndrvcache::ConfigNode *cfg_node,
                            std::list<std::string> &port_list);

  /**
   * @brief                          - update event to be sent to UPPL and cache
   * @param[in] ctr_ptr              - controller pointer
   * @param[in] cfg_node             - Config Node pointer
   * @param[in] val_old_port         - old port value structure
   * @param[out] port_list           - local list maintained for delete contains port id
   * @return UncRespCode             - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode update_event(unc::driver::controller *ctr_ptr,
                               unc::vtndrvcache::ConfigNode *cfg_node,
                               val_port_st_t *val_old_port,
                               std::list<std::string> &port_list);

  /**
   * @brief                          - delete event to be sent to UPPL and cache
   * @param[in] ctr_ptr              - controller pointer
   * @param[in] switch id            - switch id in string
   * @param[in] port_list            - local list maintained for delete contains port id
   * @return UncRespCode             - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode delete_event(unc::driver::controller *ctr_ptr,
                               const std::string &switch_id,
                               std::list<std::string> &port_list);

  /**
   * @brief                          - Verify in cache whether port aleady exists or not
   * @param[in] ctr_ptr              - controller pointer
   * @param[in] cfgnode_vector       - Config Node vector
   * @param[in] switch id            - switch id in string
   * @param[out] port_list           - local list maintained for delete contains port id
   * @return UncRespCode             - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode verify_in_cache(
      unc::driver::controller *ctr_ptr,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
      const std::string &switch_id,
      std::list<std::string> &port_list);

  /**
   * @brief                          - Check whether any attributes for existing port is modified
   * @param[in] val_port_ctr         - value structure of port contains SW in controller
   * @param[in] val_port_cache       - value structure of port contains SW in cache
   * return pfc_bool_t               - PFC_TRUE/ PFC_FALSE
   */
  pfc_bool_t is_port_modified(val_port_st_t *val_port_ctr,
                              val_port_st_t *val_port_cache);

  /**
   * @brief                           - parse the response from controller
   * @param[in] ctr_ptr               - Controller pointer
   * @param[in] data                  - data to be parsed
   * @param[out] cfgnode_vector       - to be filled with the response
   * return UncRespCode               - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode parse_port_response(
      unc::driver::controller *ctr_ptr,
      std::list<vtn_port> &port_detail,
      std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /**
   * @brief                           - parse the response from controller
   * @param[in] ctr_ptr               - Controller pointer
   * @param[in] read_util             - readutil for reading key
   * @param[in] data                  - data to be parsed
   * @param[in] port_map              - Map contains, port-id and port-name
   * @param[in] switch-id             - switch-id
   * return UncRespCode               - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode parse_port_stat_response(
      unc::driver::controller *ctr_ptr,
      unc::vtnreadutil::driver_read_util*, char *data_port,
      std::map<string, string>port_map, string switch_id);


  /**
   * @brief                           - fills logical port structure
   * @param[in] key_port              - key port structure pointer
   * @param[in] val_port              - val port struture pointer
   * @param[out] key_logical_port     - key_logical_port struture
   * @param[out] val_logical_port     - val_logical_port struture
   */
  void fill_logical_port_structure(key_port_t *key_port,
                                   val_port_st_t *val_port,
                                   key_logical_port_t &key_logical_port,
                                   val_logical_port_st_t &val_logical_port);

  /**
   * @brief                           - fills logical port val structure
   * @param[in] key_port              - key port structure pointer
   * @param[in] val_port              - val port struture pointer
   * @param[out] val_logical_port     - val_logical_port struture
   */
  void fill_logical_port_val_structure(
      key_port_t *key_port,
      val_port_st_t *val_port,
      val_logical_port_st_t &val_logical_port);

  /**
   * @brief                           - deletes port
   * @param[in] ctr                   - Controller pointer
   * @param[in]                       - cfg_node_delete_map map which contains
   *                                    config node pointer to be deleted
   * @return UncRespCode              - return UNC_RC_SUCCESS/
   *                                    UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode delete_port(
      unc::driver::controller *ctr,
      const std::map<std::string,
      unc::vtndrvcache::ConfigNode *> &cfg_node_delete_map);

  /**
   * @brief                          - deletes config node
   * @param[in] cfg_node             - Config Node
   */
  void delete_config_node(unc::vtndrvcache::ConfigNode *cfg_node);

 private:
  unc::restjson::ConfFileValues_t conf_file_values_;
  std::map <std::string, std::string> port_id_val_;
  std::map <std::string, std::string> link_map_;
  std::string parent_switch_;
};
}  // namespace odcdriver
}  // namespace unc
#endif
