/*
 * Copyright (c) 2012-2013 NEC Corporation
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

#include "physical_core.hh"
using std::istringstream;

namespace unc {
namespace uppl {

/**
 * @Description : ControllerVersion class constructor implementation
 */

ControllerVersion::ControllerVersion(string version) {
  version_ = version;
  product_version_part1_ = 0;
  product_version_part2_ = 0;
  product_version_part3_ = 0;
  UpplReturnCode return_code = ParseControllerVersion(version_);
  if (return_code != UPPL_RC_SUCCESS) {
    pfc_log_error("Controller version parsing error");
  }
}
/**
 * @Description : This function parses the given controller version string and
 *                fills the class data members
 */

UpplReturnCode ControllerVersion::ParseControllerVersion(string version) {
  // String parsing of controller version
  vector<string> split_version;
  istringstream strversion(version);
  string temp_version;
  while (getline(strversion, temp_version, '.')) {
    split_version.push_back(temp_version);
  }
  if (split_version.empty()) {
    // unable to get version
    return UPPL_RC_FAILURE;
  }
  for (unsigned int index = 0 ; index < split_version.size(); ++index) {
    uint16_t version_part =
        static_cast<uint16_t>(atoi(split_version[index].c_str()));
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
  return UPPL_RC_SUCCESS;
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
