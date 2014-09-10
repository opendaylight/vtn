/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_util.hh>
#include <cstdlib>

namespace unc {
namespace odcdriver {

pfc_bool_t OdcUtil::convert_ip_to_inaddr(
    std::string ip_addr,
    struct in_addr *addr_ctrinfoIP) {
  const char * ctr_ip = ip_addr.c_str();
  if (inet_aton(ctr_ip, addr_ctrinfoIP) != 0) {
    return PFC_TRUE;
  } else {
    return PFC_FALSE;
  }
}

void OdcUtil::convert_macstring_to_uint8(std::string mac_address,
                                         uint8_t *macOut) {
  size_t search_pos(0);
  search_pos = mac_address.find_first_of(":");
  if (search_pos != std::string::npos) {
    sscanf(mac_address.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x",
           (unsigned int *)&macOut[0],
           (unsigned int *)&macOut[1],
           (unsigned int *)&macOut[2],
           (unsigned int *)&macOut[3],
           (unsigned int *)&macOut[4],
           (unsigned int *)&macOut[5]);
  }
}

void OdcUtil::convert_uint8_to_number(uint8_t macOut[6], uint64_t *uniq_no) {
  char temp[100];
  sprintf(temp, "%d%d%d%d%d%d",
               (unsigned char) macOut[0],
               (unsigned char) macOut[1],
               (unsigned char) macOut[2],
               (unsigned char) macOut[3],
               (unsigned char) macOut[4],
               (unsigned char) macOut[5]);
  pfc_log_info("temp string is %s", temp);
  *uniq_no = atol(temp);
}


char * OdcUtil::my_ntoa(unsigned long ip_addr) {
  struct in_addr addr;
  addr.s_addr = htonl(ip_addr);
  return inet_ntoa(addr);
}



void OdcUtil::calculate_broadcast(std::string ip_address,
                                  std::string netmask) {
  const char *ipadd = ip_address.c_str();
  const char *netmask_value  = netmask.c_str();
  struct in_addr address, netmask_val;
  unsigned long network, hostmask, broadcast;
  inet_aton(ipadd, &address);
  inet_aton(netmask_value, &netmask_val);
  pfc_log_debug("IP address   %s\n", inet_ntoa(address));
  pfc_log_debug("Netmask      %s\n", inet_ntoa(netmask_val));

  network = ntohl(address.s_addr) & ntohl(netmask_val.s_addr);
  hostmask = ~ntohl(netmask_val.s_addr);
  broadcast = network | hostmask;
  pfc_log_debug("Broadcast    %s\n", my_ntoa(broadcast));
}

std::string OdcUtil::macaddress_to_string(unsigned char macIn1[6]) {
  char mac[32] = { 0 };
  sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
          macIn1[0],
          macIn1[1],
          macIn1[2],
          macIn1[3],
          macIn1[4],
          macIn1[5]);
  std::string mac_address_str = mac;
  return mac_address_str;
}


}  // namespace odcdriver
}  // namespace unc


