/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef CAPA_INTF_HH_
#define CAPA_INTF_HH_

#include <string>

#include "unc/config.h"
#include "unc/keytype.h"

namespace unc {
namespace capa {

class CapaIntf {
 public:
  virtual ~CapaIntf() {}
  virtual bool  GetInstanceCount(unc_keytype_ctrtype_t ctrlr_type,
                                 const std::string &version,
                                 unc_key_type_t keytype,
                                 uint32_t &instance_count) = 0;
  virtual bool GetCreateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                   const std::string &version,
                                   unc_key_type_t keytype,
                                   uint32_t *instance_count,
                                   uint32_t *num_attrs,
                                   const uint8_t **attrs) = 0;
  virtual bool GetUpdateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                   const std::string &version,
                                   unc_key_type_t keytype,
                                   uint32_t *num_attrs,
                                   const uint8_t **attrs) = 0;

  virtual bool GetReadCapability(unc_keytype_ctrtype_t ctrlr_type,
                                 const std::string &version,
                                 unc_key_type_t keytype,
                                 uint32_t *num_attrs,
                                 const uint8_t **attrs) = 0;

  virtual bool GetStateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                  const std::string &version,
                                  unc_key_type_t keytype,
                                  uint32_t *num_attrs,
                                  const uint8_t **attrs) = 0;

  static const uint32_t kNumberOfAvailability = 4;
};
                                                                       // NOLINT
} /* namespace capctrl */
} /* namespace unc */

#endif  // CAPA_INTF_HH_
