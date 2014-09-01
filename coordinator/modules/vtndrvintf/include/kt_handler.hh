/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef KT_HANDLER_HH_
#define KT_HANDLER_HH_

#include <handler.hh>
#include <confignode.hh>
#include <vtn_conf_data_element_op.hh>

namespace unc {
namespace driver {

class KtHandler {
 public:
  /**
   * @brief - Destructor
   */
  virtual ~KtHandler() { }

  /**
  * @brief      - Handling Request received from platform
  * @param[in]  - ControllerFramework pointer
  * @param[in]  - ServerSession,
  * @param[in]  - key interface request header
  * @retval     - UncRespCode
  */
  virtual UncRespCode handle_request(pfc::core::ipc::ServerSession &sess,
                                odl_drv_request_header_t &request_header,
                                ControllerFramework*) = 0;

   /**
   * @brief     - Converting ConfigNode pointer to execute Command
   * @param[in] - ConfigNode pointer
   * @param[in] - controller pointer
   * @param[in] - driver pointer
   * @retval    - UncRespCode
   */
  virtual UncRespCode  execute_cmd(unc::vtndrvcache::ConfigNode *cfgptr,
                                      unc::driver::controller* ctl_ptr,
                                      unc::driver::driver* drv_ptr) {
    return UNC_RC_SUCCESS;
  }

  /**
  * @brief     - Retrieve Key struct
  * @param[in] - ConfigNode pointer
  * @retval    - void * to key
  */
  virtual void* get_key_struct(unc::vtndrvcache::ConfigNode *cfgptr) {
    return NULL;
  }

  /**
  * @brief     - Retrieve Val struct
  * @param[in] - ConfigNode pointer
  * @retval    - void * to val
  */
  virtual void* get_val_struct(unc::vtndrvcache::ConfigNode *cfgptr) {
    return NULL;
  }
};
}  // namespace driver
}  // namespace unc
#endif
