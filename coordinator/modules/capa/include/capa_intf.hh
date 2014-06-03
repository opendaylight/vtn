/*
 * Copyright (c) 2012-2014 NEC Corporation
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

  /**
   * @brief  Return instance count of specified key type.
   *
   * @param[in] ctrlr_type  controller type.
   * @param[in] version     Controller version.
   * @param[in] keytype     key type
   * @param[out] instance_count  Instance count for specified keytype.
   *
   * @retval true   Successful
   * @retval false  controller/keytype is not found
   */
  virtual bool  GetInstanceCount(unc_keytype_ctrtype_t ctrlr_type,
                                 const std::string &version,
                                 unc_key_type_t keytype,
                                 uint32_t &instance_count) = 0;

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  ctrlr_type controller type.
   * @param[in]  version    controller version
   * @param[in]  keytype    Key type.
   * @param[out] instance_count  Instance count for specified keytype.
   * @param[out] num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * 
   * @retval true   Successful
   * @retval false  controller or keytype is not found
   */
  virtual bool GetCreateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                   const std::string &version,
                                   unc_key_type_t keytype,
                                   uint32_t *instance_count,
                                   uint32_t *num_attrs,
                                   const uint8_t **attrs) = 0;

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  ctrlr_type controller type.
   * @param[in]  version    controller version
   * @param[in]  keytype    Key type.
   * @param[out] num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * 
   * @retval true   Successful
   * @retval false  controler or keytype is not found
   */
  virtual bool GetUpdateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                   const std::string &version,
                                   unc_key_type_t keytype,
                                   uint32_t *num_attrs,
                                   const uint8_t **attrs) = 0;

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  ctrlr_type controller type.
   * @param[in]  version    controller version
   * @param[in]  keytype    Key type.
   * @param[out] num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * 
   * @retval true   Successful
   * @retval false  controller or keytype is not found
   */
  virtual bool GetReadCapability(unc_keytype_ctrtype_t ctrlr_type,
                                 const std::string &version,
                                 unc_key_type_t keytype,
                                 uint32_t *num_attrs,
                                 const uint8_t **attrs) = 0;

  /**
   * @brief  Return Attribute SUPPORTED or NOT_SUPPORTED of specified key type.
   * 
   * @param[in]  ctrlr_type controller type.
   * @param[in]  version    controller version
   * @param[in]  keytype    Key type.
   * @param[out] num_attrs  Maximum attribute for specified key type
   * @param[out] attrs      Array of SUPPORTED and NOT_SUPPORTED information
   * 
   * @retval true   Successful
   * @retval false  controller or keytype is not found
   */
  virtual bool GetStateCapability(unc_keytype_ctrtype_t ctrlr_type,
                                  const std::string &version,
                                  unc_key_type_t keytype,
                                  uint32_t *num_attrs,
                                  const uint8_t **attrs) = 0;

  /**
   * @brief  Size of availability array.
   */
  static const uint32_t kNumberOfAvailability = 4;
};
                                                                       // NOLINT
} /* namespace capctrl */
} /* namespace unc */

#endif  // CAPA_INTF_HH_
