/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_LINK_HH_
#define _ODC_LINK_HH_

#include <rest_util.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/uppl_common.h>
#include <vtndrvintf_defs.h>
#include <vtn_drv_module.hh>
#include <topo.hh>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <sstream>

namespace unc {
namespace odcdriver {

class OdcLink {
 public:
  /**
   * @brief Parametrised Constructor
   * @param[in] - config file values
   */
  explicit OdcLink(unc::restjson::ConfFileValues_t conf_values);

  /**
   * @brief Default Destructor
   */
  ~OdcLink();

  /**
   * @brief                          - get all the vtns from the VTN Manager
   * @param[in] ctr                  - Controller pointer
   * @param[in] cache_empty          - If cache is empty it is PFC_TRUE else
   *                                   PFC_FALSE
   * @return UncRespCode             - returns UNC_RC_SUCCESS on
   *                                   success of read all operation/returns
   *                                   UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode fetch_config(unc::driver::controller* ctr,
                           const pfc_bool_t &cache_empty);
  /**
   * @brief                          - method to set the map
   * @param[in] link_prop            - map that contains link properties
   */
  void set_map(std::map<std::string, std::string> &link_prop) {
    link_map_ = link_prop;
  }

 private:
  /**
   * @brief                         - fills the map with edge values
   * @param[in] json_obj_node_prop  - json object
   * @param[in[ arr_idx             - array index - in int specifies the array
   *                                  index  -1 denotes no array
   * @param[in]                     - ctr controller pointer
   * @param[out] cfg_node_vector    - vector to which config node needs to be
   *                                   pushed
   * @return UncRespCode            - returns
   *                                  UNC_RC_SUCCESS/
   *                                  UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode fill_edge_value_map(
      std::string source,
      std::string dst,
      unc::driver::controller *ctr_ptr,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector);

  /**
   * @brief                          - parse link and append to the vector
   * @param[in]                      - ctr controller pointer
   * @param[in]                      - head_node_conn string which contains edge prperties
   * @param[in]                      - tail_node_conn string contains head
   *                                   connection properties
   * @param[out] cfg_node_vector     - vector to which config node needs to be
   *                                   pushed
   * @return UncRespCode             - returns UNC_RC_SUCCESS on
   *                                   parsing link and appending to vector
   *                                   successfully/returns
   *                                   UNC_DRV_RC_ERR_GENERIC on failure
   */
  UncRespCode fill_config_node_vector(
      unc::driver::controller *ctr_ptr,
      std::string head_node_conn,
      std::string tail_node_conn,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector);

  /**
   * @brief                          - Compare whether cache empty or not
   * @param[in]                      - ctr controller pointer
   * @param[in]                      - Cache empty or not PFC_TRUE/PFC_FALSE
   * @param[out]                     - filled config node vector
   * @return UncRespCode             - returns UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode compare_with_cache(
      unc::driver::controller *ctr_ptr,
      const pfc_bool_t &cache_empty,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  /**
   * @brief                          - Notify physical about the link event
   * @param[in] type                 - operation type
   * @param[in] key_link             - key structure of link
   * @param[in] val_link             - value structure of link
   * @param[in] val_old_link         - old value structure used only in case of UPDATE
   */
  void notify_physical(unc::driver::oper_type type,
                       key_link_t *key_link,
                       val_link_st_t *val_link,
                       val_link_st_t *val_old_link);

  /**
   * @brief                          - update the local list maintained
   * @param[in] key_link             - key structure of link
   * @param[out] link_list           - local list maintained for delete contains
   *                                   sw1_id , port1_id, sw2_id, port2_id
   */
  void update_list(key_link_t *key_link,
                   std::list<std::string> &link_list);

  /**
   * @brief                          - add  event to be sent to UPPL and cache
   * @param[in] ctr_ptr              - controller pointer
   * @param[in] cfg_node             - Config Node pointer
   * @param[out] link_list           - local list maintained for delete contains link id
   * @return UncRespCode             - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode add_event(unc::driver::controller *ctr_ptr,
                        unc::vtndrvcache::ConfigNode *cfg_node,
                        std::list<std::string> &link_list);

  /**
   * @brief                          - update event to be sent to UPPL and cache
   * @param[in] ctr_ptr              - controller pointer
   * @param[in] cfg_node             - Config Node pointer
   * @param[in] val_old_link         - old link value structure
   * @param[out] link_list           - local list maintained for delete contains link id
   * @return UncRespCode             - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode update_event(unc::driver::controller *ctr_ptr,
                           unc::vtndrvcache::ConfigNode *cfg_node,
                           val_link_st_t *val_old_link,
                           std::list<std::string> &link_list);

  /**
   * @brief                           - delete event to be sent to UPPL and cache
   * @param[in] ctr_ptr               - controller pointer
   * @param[in] link_list             - local list maintained for delete contains link id
   * @return UncRespCode              - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode delete_event(unc::driver::controller *ctr_ptr,
                           std::list<std::string> &link_list);

   /**
    * @brief                          - Verify in cache whether link aleady exists or not
    * @param[in] ctr_ptr              - controller pointer
    * @param[in] cfgnode_vector       - Config Node vector
    * @param[out] link_list           - local list maintained for delete contains link id
    * @return UncRespCode             - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
    */
  UncRespCode verify_in_cache(
      unc::driver::controller *ctr_ptr,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector,
      std::list<std::string> &link_list);

  /**
   * @brief                           - Check whether any attributes for existing link is modified
   * @param[in] val_link_ctr          - value structure of link in controller
   * @param[in] val_link_cache        - value structure of link in cache
   * return pfc_bool_t                - PFC_TRUE/ PFC_FALSE
   */
  pfc_bool_t is_link_modified(val_link_st_t *val_link_ctr,
                              val_link_st_t *val_link_cache);

  /**
   * @brief                           - parse the response from controller
   * @param[in] ctr_ptr               - Controller pointer
   * @param[in] data                  - data to be parsed
   * @param[out] cfgnode_vector       - to be filled with the response
   * return UncRespCode               - return UNC_RC_SUCCESS/ UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode parse_link_response(
      unc::driver::controller *ctr_ptr,
      std::list<vtn_link> &link_detail,
      std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector);
  /**
   * @brief                           -  parse the link details from
                                         connection string
   * @param[in] conn                  - Connection string contains link values
   * @param[out] switch_id            - to be filled with the connection string
   * @param[out] port                 - to be filled with the connection string
   * @param[out] state                - to be filled with the connection string
   * return UncRespCode               - None
   */
  void parse_properties(std::string conn, std::string &switch_id,
                          std::string &port, std::string &state);
  /**
   * @brief                          - delete_Link
   * @param[in] ctr                  - Controller pointer
   * @param[in]                      - cfg_node_delete_map
   * @return UncRespCode             - return UNC_RC_SUCCESS/
   *                                   UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode delete_link(
      unc::driver::controller *ctr,
      const std::map<std::string,
      unc::vtndrvcache::ConfigNode *> &cfg_node_delete_map);

  /**
   * @brief                          - deletes config node after update success
   * @param[in] cfg_node             - Config Node
   */
  void delete_config_node(unc::vtndrvcache::ConfigNode *cfg_node);
 private:
  unc::restjson::ConfFileValues_t conf_file_values_;
  std::map<std::string, std::string> link_map_;
};
}  // namespace odcdriver
}  // namespace unc
#endif
