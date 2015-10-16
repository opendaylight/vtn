/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_SWITCH_HH_
#define _ODC_SWITCH_HH_

#include <rest_util.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <odc_port.hh>
#include <unc/upll_ipc_enum.h>
#include <vtndrvintf_defs.h>
#include <vtn_drv_module.hh>
#include <../../../dist/target/objs/modules/odcdriver/nodes.hh>
#include <string>
#include <list>
#include <vector>
#include <map>

namespace unc {
namespace odcdriver {

class OdcSwitch {
 public:
  /**
   * @brief Parametrised Constructor
   * @param[in] - config file values
   */
  explicit OdcSwitch(unc::restjson::ConfFileValues_t conf_values);

  /**
   * @brief Default Destructor
   */
  ~OdcSwitch();

  /**
   * @brief                          - get all the vtns from the VTN Manager
   * @param[in] ctr                  - Controller pointer
   * @param[out] cache_empty         - If cache is empty it is PFC_TRUE else
   *                                   PFC_FALSE
   * @return UncRespCode             - returns UNC_RC_SUCCESS on
   *                                   success of read all operation/returns
   *                                   UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode fetch_config(unc::driver::controller* ctr,
                               pfc_bool_t &cache_empty);

 private:
  /**
   * @brief                          - parse switch and append to the vector
   * @param[in]                      - ctr controller pointer
   * @param[in]                      - json_obj_node_prop - json object which is to be
   *                                   parsed
   * @param[in] arr_idx              - array index - in int specifies the array
   *                                   index  -1 denotes no array
   * @param[out] cfg_node_vector     - vector to which config node needs to be
   *                                   pushed
   * @return UncRespCode             - returns UNC_RC_SUCCESS on
   *                                   parsing switch and appending to vector
   *                                   successfully/returns
   *                                   UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode fill_config_node_vector(
      unc::driver::controller *ctr_ptr,
      std::string id,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector);

  /**
   * @brief                          - Compare whether cache empty or not
   * @param[in]                      - ctr controller pointer
   * @param[out]                     - filled config node vector
   * @param[out]                     - Cache empty or not PFC_TRUE/PFC_FALSE
   * @return UncRespCode             - returns UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode compare_with_cache(
      unc::driver::controller *ctr_ptr,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
      pfc_bool_t &cache_empty);

  /**
   * @brief                          - Notify physical about the switch event
   * @param[in] type                 - operation type
   * @param[in] key_switch           - key structure of switch
   * @param[in] val_switch           - value structure of switch
   * @param[in] val_old_switch       - old value structure used only in case of UPDATE
   */
  void notify_physical(unc::driver::oper_type type,
                       key_switch_t *key_switch,
                       val_switch_st_t *val_switch,
                       val_switch_st_t *val_old_switch);

  /**
   * @brief                          - update the local list maintained
   * @param[in] key_switch           - key structure of switch
   * @param[out] sw_list             - local list maintained for delete contains switch id
   */
  void update_list(key_switch_t *key_switch,
                   std::list<std::string> &sw_list);

  /**
   * @brief                          - add  event to be sent to UPPL and cache
   * @param[in] ctr_ptr              - controller pointer
   * @param[in] cfg_node             - Config Node pointer
   * @param[out] switch_list         - local list maintained for delete contains switch id
   * @return UncRespCode             - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */

  UncRespCode add_event(unc::driver::controller *ctr_ptr,
                            unc::vtndrvcache::ConfigNode *cfg_node,
                            std::list<std::string> &switch_list);

  /**
   * @brief                          - update event to be sent to UPPL and cache
   * @param[in] ctr_ptr              - controller pointer
   * @param[in] cfg_node             - Config Node pointer
   * @param[in] val_old_switch       - old switch value structure
   * @param[out] switch_list         - local list maintained for delete contains switch id
   * @return UncRespCode             - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode update_event(unc::driver::controller *ctr_ptr,
                               unc::vtndrvcache::ConfigNode *cfg_node,
                               val_switch_st_t *val_old_switch,
                               std::list<std::string> &switch_list);

  /**
   * @brief                           - delete event to be sent to UPPL and cache
   * @param[in] ctr_ptr               - controller pointer
   * @param[in] switch_list           - local list maintained for delete contains switch id
   * @return UncRespCode              - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode delete_event(unc::driver::controller *ctr_ptr,
                           std::list<std::string> &switch_list);

   /**
    * @brief                          - Verify in cache whether switch aleady exists or not
    * @param[in] ctr_ptr              - controller pointer
    * @param[in] cfgnode_vector       - Config Node vector
    * @param[out] switch_list         - local list maintained for delete contains switch id
    * @return UncRespCode             - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
    */
  UncRespCode verify_in_cache(
      unc::driver::controller *ctr_ptr,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
      std::list<std::string> &switch_list);

  /**
   * @brief                           - Check whether any attributes for existing switch is modified
   * @param[in] val_switch_ctr        - value structure of switch contains SW in controller
   * @param[in] val_switch_cache      - value structure of switch contains SW in cache
   * return pfc_bool_t                - PFC_TRUE/ PFC_FALSE
   */
  pfc_bool_t is_switch_modified(val_switch_st_t *val_switch_ctr,
                                val_switch_st_t *val_switch_cache);

  /**
   * @brief                           - parse the response from controller
   * @param[in] ctr_ptr               - Controller pointer
   * @param[in] list                  - data to be parsed
   * @param[out] cfgnode_vector       - to be filled with the response
   * return UncRespCode               - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode parse_node_response(
      unc::driver::controller *ctr_ptr,
      std::list<vtn_node> &node_deatil,
      std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /**
   * @brief                          - delete_logical_port
   * @param[in] ctr                  - Controller pointer
   * @param[in]                      - cfg_node_delete_map
   * @return UncRespCode             - return UNC_RC_SUCCESS/
   *                                   UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode delete_logical_port(
      unc::driver::controller *ctr,
      const std::map<std::string,
      unc::vtndrvcache::ConfigNode *> &cfg_node_delete_map);

  /**
   * @brief                          - delete_Switch
   * @param[in] ctr                  - Controller pointer
   * @param[in]                      - cfg_node_delete_map
   * @return UncRespCode             - return UNC_RC_SUCCESS/
   *                                   UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode delete_switch(
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
};
}  // namespace odcdriver
}  // namespace unc
#endif
