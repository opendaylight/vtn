/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef MODULES_UPLL_UPLL_VALIDATE_HH_
#define MODULES_UPLL_UPLL_VALIDATE_HH_

#include<string.h>
#include<ctype.h>
#include"pfc/log.h"

namespace unc {
namespace upll {
namespace kt_momgr {


#define READ_SUPPORTED_OPERATION (operation == UNC_OP_READ) ||\
  (operation == UNC_OP_READ_SIBLING) ||\
  (operation == UNC_OP_READ_SIBLING_COUNT)\
  || (operation == UNC_OP_READ_SIBLING_BEGIN)

#define OPEARTION_WITH_VAL_STRUCT_NONE (operation == UNC_OP_READ_NEXT)\
  || (operation == UNC_OP_READ_BULK) ||\
  (operation == UNC_OP_DELETE)

#define READ_SUPPORTED_DATATYPE (dt_type == UPLL_DT_CANDIDATE) ||\
  (dt_type == UPLL_DT_RUNNING) || (dt_type == UPLL_DT_STARTUP) ||\
  (dt_type == UPLL_DT_STATE)


// Min and Max values for validation

const uint8_t kMinLenCtrlrId = 1;
const uint8_t kMaxLenCtrlrId = 31;

const uint8_t kMinLenDomainId = 1;
const uint8_t kMaxLenDomainId = 31;
static const char* kDefaultDomainId = "(DEFAULT)";

const uint8_t kMinLenVtnName = 1;
const uint8_t kMaxLenVtnName = 31;

const uint8_t kMinLenUnifiedNwName = 1;
const uint8_t kMaxLenUnifiedNwName = 31;

const uint8_t kMinLenUnwLabelName = 1;
const uint8_t kMaxLenUnwLabelName = 31;

const uint8_t kMinLenUnwSpineID = 1;
const uint8_t kMaxLenUnwSpineID = 31;

const uint32_t kUnwLabelMaxCountMinValue = 1;
const uint32_t kUnwLabelMaxCountMaxValue = 4000;

const uint32_t kUnwLabelMinRange = 1;
const uint32_t kUnwLabelMaxRange = 4000;

const uint32_t kDefFallingThresholdRange = 1;
const uint32_t kDefRaisingThresholdRange = 4000;

const uint8_t kMinLenVlinkName = 1;
const uint8_t kMaxLenVlinkName = 31;

const uint8_t kMinLenVnodeName = 1;
const uint8_t kMaxLenVnodeName = 31;

const uint8_t kMinLenPortMapName = 1;
const uint8_t kMaxLenPortMapName = 31;

const uint8_t kMinLenUnwName = 1;
const uint8_t kMaxLenUnwName = 31;

const uint8_t kMinLenSpineName = 1;
const uint8_t kMaxLenSpineName = 31;

const uint8_t kMinLenInterfaceName = 1;
const uint8_t kMaxLenInterfaceName = 31;

const uint8_t kMinLenVbrPortMapId = 1;
const uint8_t kMaxLenVbrPortMapId = 31;

const uint8_t kMinLenDescription = 1;
const uint8_t kMaxLenDescription = 127;

const uint8_t kMinLenSwitchId = 1;
const uint8_t kMaxLenSwitchId = 255;

const uint16_t kMinLenLogicalPortId = 1;
const uint16_t kMaxLenLogicalPortId = 319;

const uint16_t kMinVlanId = 1;
const uint16_t kMaxVlanId = 4095;

const uint8_t kMinIpRoutePrefix = 0;
const uint8_t kMaxIpRoutePrefix = 32;

const uint8_t kMinVnodeIpv4Prefix = 1;
const uint8_t kMaxVnodeIpv4Prefix = 30;

const uint8_t kMinIpv4Prefix = 1;
const uint8_t kMaxIpv4Prefix = 32;

const uint8_t kMinIpv6Prefix = 1;
const uint8_t kMaxIpv6Prefix = 128;

const uint16_t kMinPingPacketLen = 1;
const uint16_t kMaxPingPacketLen = 65467;

const uint32_t kMinPingCount = 1;
const uint32_t kMaxPingCount = 655350;

const uint8_t kMinPingInterval = 1;
const uint8_t kMaxPingInterval = 60;

const uint8_t kMinPingTimeout = 1;
const uint8_t kMaxPingTimeout = 60;

const uint8_t kMinLenPortName = 1;
const uint8_t kMaxLenPortName = 32;

const uint64_t kMinStationId = 1;
const uint64_t kMaxStationId = 524287;

const uint32_t kMinIpAddressCount = 1;
const uint32_t kMaxIpAddressCount = 0xFFFFFFFF;

const uint16_t kMinLenGroupMetric = 1;
const uint16_t kMaxLenGroupMetric = 65535;

const uint8_t kMinLenBoundaryName = 1;
const uint8_t kMaxLenBoundaryName = 31;

const uint8_t kMinLenNwmName = 1;
const uint8_t kMaxLenNwmName = 31;

const uint16_t kMinNWMHHealthInterval = 5;
const uint16_t kMaxNWMHHealthInterval = 600;

const uint16_t kMinNWMHRecoveryInterval = 5;
const uint16_t kMaxNWMHRecoveryInterval = 600;

const uint8_t kMinNWMHFailureCount = 1;
const uint8_t kMaxNWMHFailureCount = 10;

const uint8_t kMinNWMHRecoveryCount = 1;
const uint8_t kMaxNWMHRecoveryCount = 10;

const uint8_t kMinNWMHWaitTime = 1;
const uint8_t kMaxNWMHWaitTime = 60;

/*KT_FLOWLIST, KT_FLOWLIST_ENTRY*/
const uint8_t kMinLenFlowListName = 1;
const uint8_t kMaxLenFlowListName = 32;

const uint16_t kMinEthType = 0x0000;
const uint16_t kMaxEthType = 0xffff;

const uint8_t kMinIPDscp = 0;
const uint8_t kMaxIPDscp = 63;

const uint8_t kMinVlanPriority = 0;
const uint8_t kMaxVlanPriority = 7;

const uint8_t kMinIPProto = 1;
const uint8_t kMaxIPProto = 255;

const uint16_t kMinL4Port = 0;
const uint16_t kMaxL4Port = 65535;

const uint8_t kMinIcmpValue = 0;
const uint8_t kMaxIcmpValue = 255;

/*KT_POLICING_PROFILE, KT_POLICING_PROFILE_ENTRY */
const uint8_t kMinLenPolicingProfileName = 1;
const uint8_t kMaxLenPolicingProfileName = 32;

const uint8_t kMinPolicingProfileSeqNum = 1;
const uint8_t kMaxPolicingProfileSeqNum = 255;

const uint32_t kMinRateType = 0;
const uint32_t kMaxRateType = 4294967295u;

const uint32_t kMinBurstSize = 0;
const uint32_t kMaxBurstSize = 4294967295u;

const uint8_t kMinPrecedence = 1;
const uint8_t kMaxPrecedence = 3;

const uint16_t kMinFlowFilterSeqNum = 1;
const uint16_t kMaxFlowFilterSeqNum = 65535;

const uint8_t kMinLenConvertVnodeName = 1;
const uint8_t kMaxLenConvertVnodeName = 39;

const uint8_t kMinGVtnIdRows = 1;
const uint8_t kMaxGVtnIdRows = 126;

const uint8_t kMinVbidIdRows = 1;
const uint8_t kMaxVbidIdRows = 125;

// template function to validate the boundary conditions for the input value

template<class ValType>
bool ValidateNumericRange(const ValType &num_val, const ValType &min_value,
                          const ValType &max_value, const bool &include_min,
                          const bool &include_max) {
  bool ret_val = false;

  if (include_min && include_max) {   // include min and max value
    ret_val = ((num_val >= min_value) && (num_val <= max_value));
  } else if (!include_min && !include_max) {  // exclude min and max value
    ret_val = ((num_val > min_value) && (num_val < max_value));
  } else if (include_min && !include_max) {  // include min, exclude max value
    ret_val = ((num_val >= min_value) && (num_val < max_value));
  } else if (!include_min && include_max) {  // exclude min, include max value
    ret_val = ((num_val > min_value) && (num_val <= max_value));
  }

  return ret_val;
}

// function to validate the boundary conditions for the input value

inline bool ValidateStringRange(const char *str_val,
                                const unsigned int &min_len,
                                const unsigned int &max_len) {
  // check if input string is NULL
  if (NULL == str_val) {
    return false;
  }

  bool ret_val = ((strlen(str_val) >= min_len) && (strlen(str_val) <= max_len));

  return ret_val;
}

// validation function UNC Key Type identifiers
inline bool ValidateStrId(const char *str_val) {
  // check if input string is NULL
  if (NULL == str_val) {
    return false;
  }

  // check if first character in the input string is '_'
  if ('_' == str_val[0]) {
    return false;
  }

  // check if input string contains only alphanumeric and '_' characters
  while (*str_val) {
    if (!(isalnum(*str_val) || ('_' == *str_val))) return false;
    str_val++;
  }

  return true;
}

// validation function for printable strings
inline bool ValidateStrPrint(const char *str_val) {
  // check if input string is NULL
  if (NULL == str_val) {
    return false;
  }

  // check if input string contains printable characters
  while (*str_val) {
    if (!isprint(*str_val)) return false;
    str_val++;
  }

  return true;
}

// wrapper function for validating default strings
inline bool ValidateDefaultStr(uint8_t *str,
                               unsigned int min,
                               unsigned int max) {
  if (NULL == str) {
    UPLL_LOG_DEBUG(" Input string is NULL ");
    return false;
  } else if (!ValidateStringRange(reinterpret_cast<char *>(str), min, max)) {
    UPLL_LOG_DEBUG(" Invalid string length %d",
        (unsigned int) strlen(reinterpret_cast<char *>(str)));
    return false;
  } else if ((str[0] == '(') &&
      (0 == strncmp(reinterpret_cast<char *>(str),
                    kDefaultDomainId, strlen(kDefaultDomainId)))) {
    return true;
  } else if (!ValidateStrId(reinterpret_cast<char *>(str))) {
    UPLL_LOG_DEBUG("Invalid string format %s", str);
    return false;
  }

  return true;
}

// wrapper function for validating key strings
inline upll_rc_t ValidateKey(char *str, unsigned int min, unsigned int max) {
  bool ret_val = false;

  if (NULL == str) {
    UPLL_LOG_DEBUG(" Input string is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!(ret_val = ValidateStringRange(str, min, max))) {
    UPLL_LOG_DEBUG(" Invalid string length %d", (unsigned int) strlen(str));

  } else {
    if (!(ret_val = ValidateStrId(str))) {
      UPLL_LOG_DEBUG(" Invalid string format %s", str);
    }
  }

  if (!ret_val) {
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  return UPLL_RC_SUCCESS;
}

inline bool ValidateString(uint8_t *str, unsigned int min, unsigned int max) {
  upll_rc_t result_code = ValidateKey(reinterpret_cast<char *>(str), min, max);
  if (result_code != UPLL_RC_SUCCESS) {
    return false;
  }
  return true;
}


// wrapper function for validating description strings
inline bool ValidateDesc(uint8_t *str, unsigned int min, unsigned int max) {
  bool ret_val = false;

  if (NULL == str) {
    UPLL_LOG_DEBUG("Input string is NULL ");
    return false;
  }

  if (!(ret_val = ValidateStringRange(reinterpret_cast<char *>(str),
                                      min, max))) {
    UPLL_LOG_DEBUG("Invalid string length %d",
                  (unsigned int) strlen(reinterpret_cast<char *>(str)));
  } else {
    if (!(ret_val = ValidateStrPrint(reinterpret_cast<char *>(str)))) {
      UPLL_LOG_DEBUG("Invalid string format %s", str);
    }
  }

  if (!ret_val) {
    return ret_val;
  }
  return true;
}

// wrapper function for validating logical port id
inline bool ValidateLogicalPortId(char *str, unsigned int min,
                                       unsigned int max) {
  UPLL_LOG_DEBUG("Inside ValidateLogicalPortId");

  if (NULL == str) {
    UPLL_LOG_DEBUG(" Input string is NULL ");
    return false;
  }

  UPLL_LOG_DEBUG("Input string expected min (%d) max (%d)", min, max);
  UPLL_LOG_DEBUG("String length %d", (unsigned int) strlen(str));
  if (!(ValidateStringRange(str, min, max))) {
    UPLL_LOG_DEBUG("Invalid string length %d", (unsigned int) strlen(str));
    return false;
  }

/*
  while(*str) {
    if (!(isalnum(*str) || ('-' == *str) || ('.' == *str) || (':' == *str))) {
      UPLL_LOG_DEBUG("UPLL_RC_ERR_CFG_SYNTAX %c ",*str);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    str++;
  }
*/
  return true;
}

inline bool ValidateMacAddr(uint8_t *mac_addr) {
  uint8_t temp_mac1[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  uint8_t temp_mac2[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  /** check range */
  if ((memcmp(mac_addr, temp_mac1, 6) == 0)
      || (memcmp(mac_addr, temp_mac2, 6) == 0)) {
    UPLL_LOG_DEBUG("Invalid Mac address: %x %x %x %x %x %x", mac_addr[0],
                  mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
                  mac_addr[5]);
    return false;
  }
  /* Multicast MAC address check*/
  if (mac_addr[0] == 0x01 && mac_addr[1] == 0x00 && mac_addr[2] == 0x5e) {
    if ((mac_addr[3] & 0x80) == 0) {
      UPLL_LOG_DEBUG("Invalid Mac address:%x %x %x %x %x %x", mac_addr[0],
                    mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
                    mac_addr[5]);
      return false;
    }
  }
  UPLL_LOG_DEBUG("Valid Mac_Address received");
  return true;
}

// IPv4 Address Validation functions

inline bool chk_ip_mask(uint32_t mask) {
  int i;
  uint32_t h_mask;
  bool zero_flg = false;

  h_mask = ntohl(mask);

  for (i = 0; i < 32; i++) {
    if (h_mask & 0x80000000) {
      if (zero_flg == true) {
        return false;
      }
    } else {
      zero_flg = true;
    }
    h_mask = h_mask << 1;
  }

  return true;
}


inline bool chk_ip_mask_allow(uint32_t mask) {
  uint32_t h_mask;
  h_mask = ntohl(mask);

  if (h_mask == 0x00000000
    || h_mask == 0xfffffffe
    || h_mask == 0xffffffff) {
    return false;
  }
  return true;
}


inline bool chk_ip_host_id(uint32_t ipaddr, uint32_t mask) {
  uint32_t h_ip_address;
  uint32_t h_subnet_mask;
  uint32_t h_host_id;
  uint32_t h_host_id_all_1;
  h_ip_address = ntohl(ipaddr);
  h_subnet_mask = ntohl(mask);

  h_host_id = h_ip_address&(~h_subnet_mask);
  h_host_id_all_1 = 0xffffffff&(~h_subnet_mask);

  if (!h_host_id ||h_host_id_all_1 == h_host_id) {
    return false;
  }
  return true;
}

inline bool chk_ip_address_muticast(uint32_t ip_adr) {
  const uint32_t IPADDER_CLASS_MASK = 0x000000f0;
  const uint32_t IPADDER_CLASS_D =    0x000000e0;

  if ((ip_adr & IPADDER_CLASS_MASK) == IPADDER_CLASS_D) {
    return false;
  } else {
    return true;
  }
}

inline bool ValidateIpv4Addr(uint32_t host_addr, uint8_t prefix_len) {
  int32_t i;
  uint32_t mask = 0;

  for (i = 31; i > (int32_t) (31 - prefix_len); i--) {
    mask |= (1 << i);
  }
  uint32_t ip_mask = htonl(mask);

  if (!chk_ip_mask(ip_mask)) {
    UPLL_LOG_DEBUG(" Invalid Input subnet mask - 0x%08X", ip_mask);
    return false;
  }

  if (!chk_ip_mask_allow(ip_mask)) {
    UPLL_LOG_DEBUG(" Invalid Input subnet mask - 0x%08X", ip_mask);
    return false;
  }

  if (host_addr == 0xffffffff || host_addr == 0x00000000) {
    UPLL_LOG_DEBUG(" Invalid Input host address - 0x%08X", host_addr);
    return false;
  }

  if (!chk_ip_address_muticast(host_addr)) {
    UPLL_LOG_DEBUG(" Invalid Input host address - 0x%08X", host_addr);
    return false;
  }

  if (!chk_ip_host_id(host_addr, ip_mask)) {
    UPLL_LOG_DEBUG(" Invalid Input host id - 0x%08X", host_addr);
    return false;
  }
  return true;
}

inline bool bc_check(const uint32_t &ip_adr) {
  const uint32_t IPADDER_CLASS_A_MASK = 0x00000080;
  const uint32_t IPADDER_CLASS_A      = 0x00000000;
  const uint32_t IPADDER_CLASS_A_BC   = 0xffffff00;
  const uint32_t IPADDER_CLASS_B_MASK = 0x000000c0;
  const uint32_t IPADDER_CLASS_B      = 0x00000080;
  const uint32_t IPADDER_CLASS_B_BC   = 0xffff0000;
  const uint32_t IPADDER_CLASS_C_MASK = 0x000000e0;
  const uint32_t IPADDER_CLASS_C      = 0x000000c0;
  const uint32_t IPADDER_CLASS_C_BC   = 0xff000000;

  if ((ip_adr & IPADDER_CLASS_A_MASK) == IPADDER_CLASS_A) {
    if (((ip_adr & IPADDER_CLASS_A_BC) == IPADDER_CLASS_A_BC) ||
        ((ip_adr & IPADDER_CLASS_A_BC) == 0)) {
      return false;
    } else {
      return true;
    }
  } else if ((ip_adr & IPADDER_CLASS_B_MASK) == IPADDER_CLASS_B) {
    if (((ip_adr & IPADDER_CLASS_B_BC) == IPADDER_CLASS_B_BC) ||
        ((ip_adr & IPADDER_CLASS_B_BC) == 0)) {
      return false;
    } else {
      return true;
    }
  } else if ((ip_adr & IPADDER_CLASS_C_MASK) == IPADDER_CLASS_C) {
    if (((ip_adr & IPADDER_CLASS_C_BC) == IPADDER_CLASS_C_BC) ||
        ((ip_adr & IPADDER_CLASS_C_BC) == 0)) {
      return false;
    } else {
      return true;
    }
  }
  return false;
}

inline bool mc_check(const uint32_t &ip_adr) {
  const uint32_t IPADDER_CLASS_MASK = 0x000000f0;
  const uint32_t IPADDER_CLASS_D =    0x000000e0;
  const uint32_t IPADDER_CLASS_E =    0x000000f0;

  if (((ip_adr & IPADDER_CLASS_MASK) == IPADDER_CLASS_D) ||
      ((ip_adr & IPADDER_CLASS_MASK) == IPADDER_CLASS_E)) {
    return false;
  } else {
    return true;
  }
}

inline void StringReset(uint8_t *key) {
  PFC_ASSERT(key != NULL);
  key[0] = '\0';
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc

#endif  // UPLL_UPLL_VALIDATE_HH_
