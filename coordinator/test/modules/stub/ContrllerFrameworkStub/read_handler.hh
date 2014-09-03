/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _READ_HANDLER_H_
#define _READ_HANDLER_H_

#include <kt_handler.hh>
#include <controller_fw.hh>
#include <driver/driver_interface.hh>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <memory>
using namespace unc::dataflow;
namespace unc {
namespace driver {

typedef std::map <unc_key_type_t, KtHandler*> kt_handler_map;

/**
 * @brief KtReadRequestHandler provides function for handling read request and response
 *
 **/
class KtReadRequestHandler : public KtHandler {
 public:
  /*
   * @brief - Default Constructor
   */
  explicit KtReadRequestHandler() {}

  /*
   * @brief - Default Destructor
   */
  ~KtReadRequestHandler() {}

  /**
   * @brief    - This method handles request received from platform layer
   * @param[in]- ServerSession, odl_drv_request_header_t,ControllerFramework*
   * @retval   - UncRespCode
   **/
  UncRespCode
      handle_request(pfc::core::ipc::ServerSession &sess,
                     unc::driver::odl_drv_request_header_t &request_header,
                     unc::driver::ControllerFramework* ctrl_int);

 private:
  /**
   * @brief     - This method retrieves the key and val structures
   * @param[in] - ServerSession, odl_drv_request_header_t,key,val
   * @retval    -  UncRespCode
   **/
  UncRespCode
      parse_request(pfc::core::ipc::ServerSession &sess,
                    unc::driver::odl_drv_request_header_t &request_header);

  /**
   * @brief    - This method executes the Create,Delete,Update and Read requests of
   *             Keytypes
   * @param[in]- ServerSession, odl_drv_request_header_t, ControllerFramework*
   *             UncRespCode,key,val
   * @retval   - UncRespCode
   **/
  UncRespCode
      execute(pfc::core::ipc::ServerSession &sess,
              unc::driver::odl_drv_request_header_t &request_header,
              unc::driver::ControllerFramework* ctrl_int,
              unc::vtnreadutil::driver_read_util *read_util);

  /**
   * @brief     - This method handles response from controller
   *              and send response to platform layer
   * @param[in] - ServerSession, odl_drv_request_header_t,ControllerFramework*,
   *              key,val,UncRespCode
   * @retval    - UncRespCode
   **/
  UncRespCode
  handle_response(pfc::core::ipc::ServerSession &sess,
                      unc::driver::odl_drv_request_header_t &request_header,
                      unc::driver::ControllerFramework* ctrl_int,
                      unc::vtnreadutil::driver_read_util *read_util,
                      UncRespCode &resp_code_);

  /**
   * @brief    - This method creates the Response Header
   * @param[in]- odl_drv_response_header_t,
   *             odl_drv_request_header_t,UncRespCode
   * @retval   - void
   */
  void
  create_response_header(unc::driver::odl_drv_request_header_t &reqhdr,
                             unc::driver::odl_drv_response_header_t &resphdr,
                             UncRespCode &resp_code_);

  /**
   * @brief    - This method populates the Response Header
   * @param[in]- ServerSession, odl_drv_response_header_t
   * @retval   - UncRespCode
   **/
  UncRespCode
  populate_response_header(pfc::core::ipc::ServerSession &sess,
                         unc::driver::odl_drv_response_header_t &resp_hdr);

};

}  // namespace driver
}  // namespace unc
#endif
