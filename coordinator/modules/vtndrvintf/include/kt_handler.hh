/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the  terms of the Eclipse Public License v1.0 which
 * accompanies this  distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
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
  * @brief -   Handling Request received from platform
  * @param[in] ServerSession, keyif_drv_request_header_t,ControllerFramework
  * @retval -  drv_resp_code_t
  **/ 
  virtual drv_resp_code_t handle_request(pfc::core::ipc::ServerSession &sess,
                                keyif_drv_request_header_t &request_header,
                                ControllerFramework* )=0;

   /**
   * @brief - Converting ConfigNode pointer to Command
   * @param[in] ConfigNode*, controller*, driver*
   * @retval - drv_resp_code_t
   **/
 
  virtual drv_resp_code_t  execute_cmd(unc::vtndrvcache::ConfigNode *cfgptr,
                                      unc::driver::controller* ctl_ptr,
                                      unc::driver::driver* drv_ptr)=0;

  /**
  * @brief - Retrieve Key struct
  * @param[in] ConfigNode*
  * @retval - drv_resp_code_t
  **/ 
  virtual void* get_key_struct(unc::vtndrvcache::ConfigNode *cfgptr)=0;

  /**
  * @brief - Retrieve Val struct
  * @param[in] ConfigNode*
  * @retval - drv_resp_code_t
  **/

  virtual void* get_val_struct(unc::vtndrvcache::ConfigNode *cfgptr)=0;
};
} //driver
} //unc
#endif
