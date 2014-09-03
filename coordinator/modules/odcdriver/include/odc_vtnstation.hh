/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_VTNSTATION_HH_
#define _ODC_VTNSTATION_HH_

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <json_read_util.hh>
#include <vtn_conf_data_element_op.hh>
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <odc_vtn.hh>
#include <odc_rest.hh>
#include <odc_util.hh>
#include <odc_kt_utils.hh>
#include <string>
#include <vector>
#include <sstream>
#define ZERO_VLANID 65535

namespace unc {
namespace odcdriver {

class OdcVtnStationCommand: public unc::driver::vtn_driver_read_command {
 public:
  /**
   * @brief Parametrised Constructor
   * @param[in] - conf file values
   */
  explicit OdcVtnStationCommand(unc::restjson::ConfFileValues_t conf_values);

  /**
   * @brief Default Destructor
   */
  ~OdcVtnStationCommand();

  /**
   * @brief                          - Constructs Dataflow command and send it to
   *                                   rest interface
   * @param[in] ctr                  - Controller pointrt
   */

  UncRespCode read_cmd(unc::driver::controller *ctr,
                        unc::vtnreadutil::driver_read_util*);

  private:
  /**
   * @brief                   - gets the dataflow url
   * @return std::string      - returns the url string of dataflow
   */
  std::string get_vtnstation_url(key_vtnstation_controller_t& key_);

  uint32_t age_interval_;
  unc::restjson::ConfFileValues_t conf_file_values_;
};


}  // namespace odcdriver
}  // namespace unc
#endif
