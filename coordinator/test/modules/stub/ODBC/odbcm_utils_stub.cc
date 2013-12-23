/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


/*
 *  @brief   ODBC Manager
 *  @file    odbcm_utils.cc
 */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <physical_common_def.hh>
#include <odbcm_common.hh>
#include "odbcm_utils_stub.hh"

namespace unc {
namespace uppl {

/*
 * Mapping between SQL State string and odbcm return code
 */

/**
 * @Description : ODBCMUtils constructor
 * @param[in]   : None
 * @return      : None
 **/
ODBCMUtils::ODBCMUtils() {
  /** Empty constructor */
}

/**
 * @Description : ODBCMUtils destructor
 * @param[in]   : None
 * @return      : None
 **/
ODBCMUtils::~ODBCMUtils() {
  /*
   * Clear the structure and map
   */
}

/**
 * @Description : return the ip address version 4
 * @param[in]   : ip_address - ip address type i.e. IPV4
 * @return      : string - The conversion of ip adress to asci format
 **/
std::string ODBCMUtils::get_ip_string(uint32_t ip_address) {
  struct sockaddr_in ip;
  ip.sin_addr.s_addr = ip_address;
  /** Conver uint32_t ip adress to asci readable format */
  return inet_ntoa(ip.sin_addr);
}

/**
 * @Description : return the ipv6 address string
 * @param[in]   : ipv6_address - ip address type i.e. IPV6
 * @return      : string  - The conversion of ip adress to asci format
 **/
std::string ODBCMUtils::get_ipv6_string(uint8_t *ipv6_address) {
  sockaddr_in6 addr;
  memset(&addr, 0, sizeof(sockaddr_in6));
  char str[INET6_ADDRSTRLEN];
  memset(&str, '\0', INET6_ADDRSTRLEN);
  memset(&addr.sin6_addr, 0, sizeof addr.sin6_addr);
  //  store this IP address in addr
  memcpy(&addr.sin6_addr.s6_addr, ipv6_address,
               sizeof addr.sin6_addr.s6_addr);
  inet_ntop(AF_INET6, &(addr.sin6_addr), str, INET6_ADDRSTRLEN);
  return std::string(str);
}

}  // namespace uppl
}  // namespace unc
