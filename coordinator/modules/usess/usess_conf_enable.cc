/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "usess_conf_enable.hh"

namespace unc {
namespace usess {

#define CLASS_NAME "UsessConfEnable"

// -------------------------------------------------------------
//  static member definitions.
// -------------------------------------------------------------
// configuration file data block name.
const char* UsessConfEnable::kConfBlockName_ = "usess_conf_enable";

// configuration data default value.
const usess_conf_enable_t UsessConfEnable::kDefaultConf_ = {
        HASH_TYPE_MD5,            // .hash_type
        72,                       // .passwd_length
        "[[:alnum:][:graph:]]+"   // .passwd_regular
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
UsessConfEnable::UsessConfEnable(void)
{
  data_ = kDefaultConf_;
}


/*
 * @brief   Destructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
UsessConfEnable::~UsessConfEnable(void)
{
}


/*
 * @brief   Configuration file data load.
 * @param   nothing.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e UsessConfEnable::LoadConf(void)
{
  pfc::core::ModuleConfBlock conf_block(kConfBlockName_);


  L_FUNCTION_START();

  // initialize configuration data.
  data_ = kDefaultConf_;

  if (conf_block.getBlock() == PFC_CFBLK_INVALID) {
    // Invalid block.
    return USESS_E_NG;
  }

  // get configuration file enable block.
  data_.hash_type = static_cast<hash_type_e>(conf_block.getInt32("hash",
                        kDefaultConf_.hash_type));
  data_.passwd_length = conf_block.getUint32("passwd_length",
                        kDefaultConf_.passwd_length);
  data_.passwd_regular = conf_block.getString("passwd_regular",
                        kDefaultConf_.passwd_regular.c_str());

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   configuration data access.
 * @param   nothing.
 * @return  Processing result.
 * @note    
 */
const usess_conf_enable_t& UsessConfEnable::data(void) const
{
  return data_;
}

}  // namespace usess
}  // namespace unc
