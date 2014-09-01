/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


/*@brief   unclib
 *@file    unclib_module.hh
 *Desc:    This file contains the prototype of UncLibModule module
 */

#ifndef UNC_UNCMODULE_HH_
#define UNC_UNCMODULE_HH_

#include "pfcxx/module.hh"
#include <pfc/pfc.h>
#include <pfc/log.h>
#include <string>

namespace unc {
namespace unclib {

class UncLibModule : public pfc::core::Module {
 public:
  /**
   * @brief  Constructor.
   */
  explicit UncLibModule(const pfc_modattr_t *attr);

  /**
   * @brief  Destructor.
   */
  ~UncLibModule();

  /**
   *  @brief  Initialize module.
   *  @retval PFC_TRUE   Successful completion.
   *  @retval PFC_FALSE  Failure occurred.
   */
  pfc_bool_t init();

  /**
   * @brief  Finalize module.
   * @retval  This function always returns PFC_TRUE.
   */
  pfc_bool_t fini(void);
  static UncLibModule* get_unclib_module();

 private:
  static UncLibModule* unclib_module_;
};
}
}
#endif
