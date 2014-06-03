/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "usess_conf_session.hh"

namespace unc {
namespace usess {

#define CLASS_NAME "UsessConfSession"

// -------------------------------------------------------------
//  static member definitions.
// -------------------------------------------------------------
// configuration file data block name.
const char* UsessConfSession::kConfBlockName_ = "usess_session_info";

// configuration file data map name, key.
const char* UsessConfSession::kConfMapName_ = "usess_session_parameter";
const char* UsessConfSession::kConfMapKey_[ID_NUM] =
    {"cli", "web_api", "web_ui", "fixed"};

// configuration data default value.
const usess_conf_session_t UsessConfSession::UsessConfSession::kDefaultConf_ =
{
  {true, 64},                   // .global
  {{{   1,   127}, {true, 16}},  // .local[0] CLI connect.
  {{1024, 65535}, {true, 64}},  // .local[1] WEB API connect.
  {{ 256,   511}, {true, 24}},  // .local[2] WEB UI connect.
  {{ 128,   255}, {false, 0}}}   // .local[3] connect to fixed session id.
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
UsessConfSession::UsessConfSession(void)
{
  data_ = kDefaultConf_;
}


/*
 * @brief   Destructor.
 * @param   nothing.
 * @return  nothing.
 * @note    
 */
UsessConfSession::~UsessConfSession(void)
{
}


/*
 * @brief   Configuration file data load.
 * @param   nothing.
 * @return  Processing result.
 * @note    
 */
usess_ipc_err_e UsessConfSession::LoadConf(void)
{
  pfc::core::ModuleConfBlock global(kConfBlockName_);

  pfc::core::ModuleConfBlock cli(kConfMapName_, kConfMapKey_[ID_CLI]);
  pfc::core::ModuleConfBlock web_api(kConfMapName_, kConfMapKey_[ID_WEB_API]);
  pfc::core::ModuleConfBlock web_ui(kConfMapName_, kConfMapKey_[ID_WEB_UI]);
  pfc::core::ModuleConfBlock fixed(kConfMapName_, kConfMapKey_[ID_FIXED]);

  pfc::core::ModuleConfBlock* local[ID_NUM];
  uint32_t range_num = 0;


  local[ID_CLI] = &cli;
  local[ID_WEB_API] = &web_api;
  local[ID_WEB_UI] = &web_ui;
  local[ID_FIXED] = &fixed;


  L_FUNCTION_START();

  // initialize configuration data.
  data_ = kDefaultConf_;

  if (global.getBlock() == PFC_CFBLK_INVALID ||
      cli.getBlock() == PFC_CFBLK_INVALID ||
      web_api.getBlock() == PFC_CFBLK_INVALID ||
      web_ui.getBlock() == PFC_CFBLK_INVALID ||
      fixed.getBlock() == PFC_CFBLK_INVALID) {
    // Invalid block or map.
    data_ = kDefaultConf_;
    return USESS_E_NG;
  }

  // get configuration file session block.
  data_.global.limited = global.getBool("limited",
          kDefaultConf_.global.limited);
  data_.global.max_session = global.getUint32("max_session",
          kDefaultConf_.global.max_session);

  // get configuration file session map.
  for (int id = 0; id < ID_NUM; ++id) {
    data_.local[id].range.start = local[id]->getUint32("start_id",
          kDefaultConf_.local[id].range.start);
    data_.local[id].range.end = local[id]->getUint32("end_id",
          kDefaultConf_.local[id].range.end);
    data_.local[id].connect.limited = local[id]->getBool("limited",
          kDefaultConf_.local[id].connect.limited);
    data_.local[id].connect.max_session = local[id]->getUint32("max_session",
          kDefaultConf_.local[id].connect.max_session);

    // Correction of session max connection.
    range_num = data_.local[id].range.end - data_.local[id].range.start + 1;
    if (range_num < data_.local[id].connect.max_session) {
      data_.local[id].connect.max_session = range_num;
    }
  }

  L_FUNCTION_COMPLETE();
  return USESS_E_OK;
}


/*
 * @brief   configuration data access.
 * @param   nothing.
 * @return  Processing result.
 * @note    
 */
const usess_conf_session_t& UsessConfSession::data(void) const
{
  return data_;
}

}  // namespace usess
}  // namespace unc
