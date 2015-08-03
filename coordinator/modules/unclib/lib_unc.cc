/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/* @brief   unclib 
  *@file    lib_unc.cc
  *Desc:
  * This file contains the definition of UncModeUtil class of unclib
  *  */

#include <uncxx/lib_unc.hh>

namespace unc {
namespace unclib {
/**
 * @brief      UncModeUtil constructor
 * @param[in]  int &ret_code
 * @param[out]  int &ret_code
 */
UncModeUtil::UncModeUtil(int &ret_code) {
  if (ReadConfigFile() != 0) {
    pfc_log_warn("unclib.conf read failed. - default value taken");
    ret_code = -1;
  }
  ret_code = 0;
}
/**
 * @brief      UncModeUtil destructor
 * @param[in]  none
 */
UncModeUtil::~UncModeUtil() {
}

/**
 * @brief     ReadConfigFile() - to read the unclib.conf file param block. 
 * @param[in]  none
 */
uint8_t UncModeUtil::ReadConfigFile() {
  pfc::core::ModuleConfBlock uncmodeblock(UNC_MODE_PARAMS_BLK);
  pfc::core::ModuleConfBlock importmodeblock(UNC_IMPORT_MODE_PARAMS_BLK);
  unc_mode_ = (uint8_t)uncmodeblock.getByte("unc_mode", 0);
  // 0 -  UNC_SEPARATE_MODE, 1 - UNC_COEXISTS_MODE
  pfc_log_info("unc_mode from unclib.conf is %d", unc_mode_);
  import_mode_ = (uint8_t)importmodeblock.getByte("import_mode", 0);
  // 0 -  UNC_IMPORT_ERROR_MODE , 1 - UNC_IMPORT_IGNORE_MODE
  pfc_log_info("import_mode from unclib.conf is %d", import_mode_);
  return 0;
}

/**
 * @brief     libunc_get_unc_mode() - to get the unc_mode. 
 * @param[in]  none
 * @param[out] uint8_t unc_mode_
 */
uint8_t UncModeUtil::libunc_get_unc_mode() {
  pfc_log_debug("unc_mode at unclib is %d", unc_mode_);
  return (uint8_t)unc_mode_;
}
/**
 * @brief     libunc_get_import_mode() - to get the import_mode. 
 * @param[in]  none
 * @param[out] uint8_t import_mode_
 */
uint8_t UncModeUtil::libunc_get_import_mode() {
  pfc_log_debug("import_mode at unclib is %d", import_mode_);
  return (uint8_t)import_mode_;
}
}
}
