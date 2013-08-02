/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef DAL_MODULE_HH_
#define DAL_MODULE_HH_

#include "pfcxx/module.hh"

namespace unc {
namespace upll {
namespace dal {

class DalModule : public pfc::core::Module {
 public:
  /**
   * @brief  Constructor.
   */
  explicit DalModule(const pfc_modattr_t *attr);

  /**
   * @brief  Destructor.
   */
  ~DalModule();

  /**
   *  @brief  Initialize capa module.
   *  @retval PFC_TRUE   Successful completion.
   *  @retval PFC_FALSE  Failure occurred.
   */
  pfc_bool_t init();

  /**
   * @brief  Finalize capa module.
   * @retval  This function always returns PFC_TRUE.
   */
  pfc_bool_t fini(void);
};

}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // DAL_MODULE_HH_

