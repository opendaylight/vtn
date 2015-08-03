/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "usess_conf_common.hh"

namespace unc {
namespace usess {

#define CLASS_NAME "UsessConfCommon"

// -------------------------------------------------------------
//  static member definitions.
// -------------------------------------------------------------
const char* UsessConfCommon::kConfBlockName_ = "usess_conf_common";
const usess_conf_common_t UsessConfCommon::kDefaultConf_ = {
                           { -1,     // .lock_timeout.tv_sec
                             -1},    // .lock_timeout.tv_nsec
                              0      // .auth_retry_count
};


// -------------------------------------------------------------
//  Class method definitions.
// -------------------------------------------------------------
/*
 * @brief   Constructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
UsessConfCommon::UsessConfCommon(void)
{
  data_ = kDefaultConf_;
}

/*
 * @brief   Destructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
UsessConfCommon::~UsessConfCommon(void)
{
}

/*
 * @brief   Configuration file data load.
 * @param   nothing.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e UsessConfCommon::LoadConf(void)
{
  pfc::core::ModuleConfBlock conf_block(kConfBlockName_);


  L_FUNCTION_START();

  // initialize configuration data.
  data_ = kDefaultConf_;

  if (conf_block.getBlock() == PFC_CFBLK_INVALID) {
    // Invalid block.
    return USESS_E_NG;
  }

  // get configuration file common block.
  data_.lock_timeout.tv_sec = conf_block.getInt64(
        "lock_timeout_sec", kDefaultConf_.lock_timeout.tv_sec);
  data_.lock_timeout.tv_nsec = conf_block.getInt64(
        "lock_timeout_nsec", kDefaultConf_.lock_timeout.tv_nsec);
//  data_.auth_retry_count = conf_block.getUint32(
//        "auth_retry_count", kDefaultConf_.auth_retry_count);
  data_.auth_retry_count = 0;

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}

/*
 * @brief   configuration data access.
 * @param   nothing.
 * @return  Processing result.
 * @note    
 */
const usess_conf_common_t& UsessConfCommon::data(void) const
{
  return data_;
}

}  // namespace usess
}  // namespace unc
