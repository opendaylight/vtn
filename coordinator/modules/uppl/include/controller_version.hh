/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    Controller Version handler
 * @file     controller_version.hh
 *
 */

#ifndef _UNC_CONTROLLER_VERSION_HH_
#define _UNC_CONTROLLER_VERSION_HH_

#include <string>

namespace unc {
namespace uppl {
/**
 * Controller Version class holds version info
 */
class ControllerVersion {
 public:
  explicit ControllerVersion(string version,
                             UncRespCode &return_code);

  ~ControllerVersion() {}

  UncRespCode ParseControllerVersion(string version);

  bool operator<(const ControllerVersion &val) const;

  string get_version() {
    return version_;
  }

  string version_;
  uint16_t product_version_part1_;
  uint16_t product_version_part2_;
  uint16_t product_version_part3_;
};
}  // namespace uppl
}  // namespace unc

#endif
