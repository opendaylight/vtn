/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UNCMODE_UTIL_HH_
#define UNC_UNCMODE_UTIL_HH_
#include "pfcxx/module.hh"
#define UNC_MODE_PARAMS_BLK "mode_params"
#define UNC_IMPORT_MODE_PARAMS_BLK "import_params"

namespace unc {
namespace unclib {

class UncModeUtil {
  public:
    explicit UncModeUtil(int &ret_code);
    ~UncModeUtil();
    uint8_t libunc_get_unc_mode();
    uint8_t libunc_get_import_mode();

  private:
    uint8_t ReadConfigFile();
    uint8_t unc_mode_;
    uint8_t import_mode_;
};
}
}
#endif 

