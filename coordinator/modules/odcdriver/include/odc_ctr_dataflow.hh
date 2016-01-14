/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_CTR_DATAFLOW_HH_
#define _ODC_CTR_DATAFLOW_HH_
#include <uncxx/dataflow.hh>
#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <json_build_parse.hh>
#include <vtn_conf_data_element_op.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <odc_vtn.hh>
#include <dataflow.hh>
#include <odc_util.hh>
#include <tclib_module.hh>
#include <string>
#include <vector>
#include <sstream>

#define VAL_MAC_ADDR_SIZE 6
#define PFCAPI_DATAFLOW_REQ_ALL     0xffffffff
namespace unc {
namespace odcdriver {

class OdcCtrDataFlowCommand: public unc::driver::vtn_driver_read_command {
 public:
  /**
   * @brief Parametrised Constructor
   * @param[in] - conf file values
   */
  explicit OdcCtrDataFlowCommand(unc::restjson::ConfFileValues_t conf_values);

  /**
   * @brief Default Destructor
   */
  ~OdcCtrDataFlowCommand();

  UncRespCode read_cmd(unc::driver::controller *ctr,
                       unc::vtnreadutil::driver_read_util*);

  //UncRespCode parse_flow_response(key_ctr_dataflow_t& key_ctr_dataflow,
    //                              unc::vtnreadutil::driver_read_util* df_util,
      //                            char *data);

  UncRespCode parse_flow_response_values(key_ctr_dataflow_t& key_ctr_dataflow,
                                  unc::vtnreadutil::driver_read_util* df_util,
                                  std::list<data_flow> &data_flow);

  private:
  /**
   * @brief                   - gets the dataflow url
   * @param[in] key_dataflow  - dataflow key structure
   * @param[in] vtn_name      - vtn_name
   * @return std::string      - returns the url string of dataflow
   */
  std::string get_dataflow_url(std::string vtn_name, key_ctr_dataflow key);

  /**
   * @brief      - Method to fetch child configurations for the parent kt
   * @param[in]  - controller pointer
   * @param[in]  - parent key type pointer
   * @param[out] - list of configurations
   * @retval     - UNC_RC_SUCCESS / UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode fetch_config_vtn(
      unc::driver::controller* ctr,
      void* parent_key,
      std::vector<string> &vtn_names);

  UncRespCode fetch_config(
      unc::driver::controller* ctr,
      void* parent_key,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector)
  {return UNC_RC_SUCCESS;}

  std::string switch_to_odc_type(char* switch_id);

  private:
  uint32_t age_interval_;
  unc::restjson::ConfFileValues_t conf_file_values_;
};
}  // namespace odcdriver
}  // namespace unc
#endif
