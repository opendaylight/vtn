/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    Controller version
 * @file     controller_version.cc
 *
 */

#include <cctype>
#include "physical_core.hh"

using std::istringstream;

namespace unc {
namespace uppl {

/**
 * @Description : ControllerVersion class constructor implementation
 */
ControllerVersion::ControllerVersion(string version,
                                     UncRespCode &return_code):
                                     version_(version),
                                     product_version_part1_(0),
                                     product_version_part2_(0),
                                     product_version_part3_(0) {
  return_code = ParseControllerVersion(version_);
  if (return_code != UNC_RC_SUCCESS) {
    pfc_log_error("Controller version parsing error");
  }
}

/**
 * @Description : This function parses the given controller version string and
 *                fills the class data members
 *@param[in]    : version - specifies the controller version
 *@return       : UNC_RC_SUCCESS or any associated erro code
 */

UncRespCode ControllerVersion::ParseControllerVersion(string version) {
  // String parsing of controller version
  vector<string> split_version;
  istringstream strversion(version);
  string temp_version = "";
  while (getline(strversion, temp_version, '.')) {
    split_version.push_back(temp_version);
  }
  if (split_version.empty()) {
    // unable to get version
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  for (unsigned int index = 0 ; index < split_version.size(); ++index) {
    string part_version = split_version[index];
    uint16_t version_part =
        static_cast<uint16_t>(atoi(part_version.c_str()));
    if (version_part == 0 && isdigit(part_version[0]) == 0) {
      pfc_log_error("Unable to parse the version part: %s",
                    part_version.c_str());
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    switch (index) {
      case 0 : {
        product_version_part1_ = version_part;
        break;
      }
      case 1 : {
        product_version_part2_ = version_part;
        break;
      }
      case 2 : {
        product_version_part3_ = version_part;
        break;
      }
    }
  }
  return UNC_RC_SUCCESS;
}

bool ControllerVersion::operator<(const ControllerVersion &val) const {
  if (product_version_part1_ < val.product_version_part1_) {
    return true;
  } else if (product_version_part1_ > val.product_version_part1_) {
    return false;
  }

  if (product_version_part2_ < val.product_version_part2_) {
    return true;
  } else if (product_version_part2_ > val.product_version_part2_) {
    return false;
  }

  if (product_version_part3_ < val.product_version_part3_) {
    return true;
  } else if (product_version_part3_ > val.product_version_part3_) {
    return false;
  }
  return false;
}
}  // namespace uppl
}  // namespace unc
