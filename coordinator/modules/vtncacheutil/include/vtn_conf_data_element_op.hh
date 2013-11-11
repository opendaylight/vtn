/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#ifndef __VTN_CONF_DATA_ELEMENT_OP_HH__
#define __VTN_CONF_DATA_ELEMENT_OP_HH__

#include <string>
#include "confignode.hh"
#include "vtn_conf_utility.hh"

namespace unc {
namespace vtndrvcache {
template<typename key, typename value, typename op>
class CacheElementUtil: public  ConfigNode {
 private:
  key* key_;
  value* value_;
  op operation_;

 public:
  /**
   * @brief  : Constructor to set the key struct, value structure & operation_
   */
  CacheElementUtil(key* key_ty, value* value_ty, op opet):
      operation_(opet) {
        ODC_FUNC_TRACE;
        key_ = new key();
        PFC_ASSERT(key_ != NULL);
        value_ = new value();
        PFC_ASSERT(value_ != NULL);

        memcpy(key_, key_ty, sizeof(key));
        memcpy(value_, value_ty, sizeof(value));
      }

  /**
   * @brief : Destructor to free the key struct, value structure
   */
  ~CacheElementUtil() {
    ODC_FUNC_TRACE;
    if (key_ != NULL)
      delete key_;

    if (value_ != NULL)
      delete value_;
  }

  /**
   * @brief  : This method returns the Keytype given the key struct
   * @retval : key_type
   */
  unc_key_type_t  get_type_name() {
    ODC_FUNC_TRACE;
    return ConfUtil::get_key_type(*key_);
  }

  /**
   * @brief   : This method returns the search Key given the key struct
   * @retval  : string
   */
  std::string  get_key_generate() {
    ODC_FUNC_TRACE;
    if (key_ != NULL) {
      return ConfUtil::get_search_key(*key_);
    }
    return "";
  }

  /**
   * @brief   : This method returns the Key name given the key struct
   * @retval  : string
   */
  std::string get_key_name() {
    ODC_FUNC_TRACE;
    if (key_ != NULL) {
      return ConfUtil::get_key(*key_);
    }
    return "";
  }

  /**
   * @brief   : This method returns the parent Key given the key struct
   * @retval  : string
   */
  std::string  get_parent_key_name() {
    ODC_FUNC_TRACE;
    if (key_ != NULL) {
      return ConfUtil::get_parent_key(*key_);
    }
    return "";
  }

  /**
   * @brief   : This method returns the key struct
   * @retval  : key*
   */
  key* get_key_structure() {
    ODC_FUNC_TRACE;
    return key_;
  }

  /**
   * @brief   : This method returns the value struct
   * @retval  : key*
   */
  value* get_val_structure() {
    ODC_FUNC_TRACE;
    return value_;
  }

  /**
   * @brief  : This method returns the operation_
   * @retval : operation_
   */
  op get_operation() {
    ODC_FUNC_TRACE;
    return operation_;
  }
};
}  // namespace vtndrvcache
}  // namespace unc
#endif
