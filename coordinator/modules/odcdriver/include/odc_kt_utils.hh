/*
 * Copyright (c) 2014-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef __ODC_KT_UTIL_HH__
#define __ODC_KT_UTIL_HH__

#include <odc_rest.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <vbr.hh>
#include <vterminal.hh>
#include <string>
#include <set>


namespace unc {
namespace odcdriver {

class odlutils {
 public:
  static UncRespCode get_vtn_names(unc::driver::controller *ctr_ptr,
                                    unc::restjson::ConfFileValues_t conf_values,
                                    std::set <std::string> *vtns);

  static UncRespCode get_vbridge_names(
      unc::driver::controller *ctr_ptr,
      unc::restjson::ConfFileValues_t conf_values,
      std::string vtn_name,
      std::set <std::string> *vbridges);

   static UncRespCode get_bridge_names(unc::driver::controller *ctr,
                                       std::string vtn_name,
                               std::set <std::string> *vbridges);

  static UncRespCode get_terminal_names(unc::driver::controller *ctr,
                                       std::string vtn_name,
                               std::set <std::string> *vterminals);

  static UncRespCode get_vterm_names(
      unc::driver::controller *ctr_ptr,
      unc::restjson::ConfFileValues_t conf_values,
      std::string vtn_name,
      std::set <std::string> *vbridges);

  static UncRespCode get_portname(
      unc::driver::controller *ctr_ptr,
      unc::restjson::ConfFileValues_t conf_values,
      std::string switch_id,
      std::string port_id,
      std::string &port_name);
};


class port_info_parser {
 public:
  static UncRespCode get_vlan_id(json_object* instance,
                                 int &vlanid);

  static UncRespCode get_switch_details(json_object* instance,
                                        std::string &switch_id);

  static UncRespCode get_port_details(json_object* instance,
                                      std::string &portname);
};


}  // namespace odcdriver
}  // namespace unc

#endif
