/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_UTIL_HH_
#define _ODC_UTIL_HH_

#include <unc/upll_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <pfc/ipc_struct.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>

namespace unc {
namespace odcdriver {

class OdcUtil {
 public:
  /**
   * @brief     - This method converts ip address string to struct in_addr format
   * @param[in] - ip_addr (ip address)
   * @param[in] - addr_ctrinfoIP (struct in_addr)
   */

  pfc_bool_t convert_ip_to_inaddr(std::string ip_addr,
                                  struct in_addr *addr_ctrinfoIP);

  /**
   * @brief      - This method converts mac address string to uint8_t format
   * @param[in]  - mac_address (mac address)
   * @param[out] - macOut (mac address in uint8_t)
   */
  void convert_macstring_to_uint8(std::string mac_address,
                                  uint8_t *macOut);

  void convert_uint8_to_macstring(uint8_t macOut[6], std::string &mac);
  void convert_uint8_to_number(uint8_t macOut[6], uint64_t *);
  /**
   * @brief      - This method converts struct in_addr to  string format
   * @param[in]  - ip_addr (struct in_addr)
   */
  char *my_ntoa(unsigned long ip_addr);

  /**
   * @brief      - This method calculates the broadcast address
   * @param[in]  - ip_address (ip address)
   * @param[out] - netmask (netmask)
   */
  void calculate_broadcast(std::string ip_address,
                           std::string netmask);

  /**
   *@brief      - This method converts mac address from unsigned char array to string
   *@param[in]  - macIn1 (mac address tokens)
   */
  std::string macaddress_to_string(unsigned char macIn1[6]);
};

}  // namespace odcdriver
}  // namespace unc

#endif
