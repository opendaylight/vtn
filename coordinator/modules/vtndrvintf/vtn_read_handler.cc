 /*
  * Copyright (c) 2013-2015 NEC Corporation
  * All rights reserved.
  *
  * This program and the accompanying materials are made available under the
  * terms of the Eclipse Public License v1.0 which accompanies this
  * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
  */

#include <read_handler.hh>


namespace unc {
namespace driver {

UncRespCode
  KtReadRequestHandler::parse_request(
          pfc::core::ipc::ServerSession &sess,
          unc::driver::odl_drv_request_header_t &request_header) {
      return UNC_RC_SUCCESS;
  }


  /**
   * @brief    - This method executes the Create,Delete,Update requests of
   *             Keytypes
   * @param[in]- ServerSession, odl_drv_request_header_t, ControllerFramework*
   *             UncRespCode,key,val
   * @retval   - UncRespCode
   **/
UncRespCode
KtReadRequestHandler::execute(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    unc::driver::ControllerFramework* ctrl_int,
    unc::vtnreadutil::driver_read_util *read_util) {
  ODC_FUNC_TRACE;

  std::string ctrl_name = std::string(request_header.controller_name);

  unc::driver::driver*  drv_ptr     = NULL;
  unc::driver::controller* ctrl_ptr = NULL;
  UncRespCode resp_code_            = UNC_DRV_RC_ERR_GENERIC;

  controller_operation util_obj(ctrl_int, WRITE_TO_CONTROLLER, ctrl_name);
  if (util_obj.get_controller_status() == PFC_FALSE) {
    pfc_log_debug("%s Controller status is down, send disconnected",
                                 PFC_FUNCNAME);
    return UNC_RC_CTR_DISCONNECTED;
  }
  ctrl_ptr = util_obj.get_controller_handle();
  drv_ptr = util_obj.get_driver_handle();

  PFC_ASSERT(drv_ptr != NULL);
  PFC_ASSERT(ctrl_ptr !=NULL);

  pfc_log_debug("getting conn status..ctr nt NULL");
  if (ctrl_ptr->get_connection_status() == CONNECTION_DOWN) {
    //  check controller connection status, if down send disconnected
    pfc_log_debug("%s Controller status is down, send disconnected",
                                 PFC_FUNCNAME);
    return UNC_RC_CTR_DISCONNECTED;
  }
  if (ctrl_ptr->get_audit_result() == PFC_FALSE) {
    //  check controller audit status, if audit not completed
    //  send disconnected
    pfc_log_debug("%s Audit is not completed, send disconnected",
                                 PFC_FUNCNAME);
    return UNC_RC_CTR_DISCONNECTED;
  }
  unc::driver::vtn_driver_read_command * drv_command_ptr =
      drv_ptr->create_driver_read_command(request_header.key_type);
  if (drv_command_ptr == NULL) {
    pfc_log_error("Driver Does not support read for keytype: %d",
                        request_header.key_type);

    return UNC_DRV_RC_INVALID_OPERATION;
  }

  if (request_header.header.operation == UNC_OP_READ) {
    pfc_log_debug("%s: Execute Read  Command string", PFC_FUNCNAME);
      pfc_log_debug("Dataflow Execute Read  Command string");
      resp_code_ = drv_command_ptr->read_cmd(ctrl_ptr, read_util);
    } else {
      pfc_log_debug("NOT A Read Operation!!!");
    }
  delete drv_command_ptr;
  return resp_code_;
}


  /**
   * @brief    - This method handles read request received from platform layer
   * @param[in]- ServerSession, odl_drv_request_header_t,ControllerFramework*
   * @retval   - UncRespCode
   **/
UncRespCode
KtReadRequestHandler::handle_request(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    unc::driver::ControllerFramework* ctrl_int) {
  ODC_FUNC_TRACE;

  UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;
  unc::vtnreadutil::driver_read_util *read_util = NULL;

  read_util = new unc::vtnreadutil::driver_read_util(sess);

  PFC_ASSERT(ctrl_int != NULL);
  PFC_ASSERT(read_util != NULL);

  resp_code_ = parse_request(sess, request_header);

  if (resp_code_ == UNC_RC_SUCCESS) {
      pfc_log_debug("read execute");
      resp_code_ = execute(sess, request_header, ctrl_int,
                            read_util);
    if (resp_code_ != UNC_RC_SUCCESS) {
      pfc_log_error("%s: read execute return err with resp_code(err = %u)",
                    PFC_FUNCNAME, resp_code_);
    }
  }

  resp_code_ = handle_response(sess, request_header, ctrl_int,
                        read_util, resp_code_);
  if (resp_code_ != UNC_RC_SUCCESS) {
    pfc_log_error("%s:. Failed to send response(err = %u)",
                  PFC_FUNCNAME, resp_code_);
  }

  if (read_util != NULL)
    delete read_util;

  return resp_code_;
}


  /**
   * @brief     - This method handles response from controller,
   *              doesnot support audit
   * @param[in] - ServerSession, odl_drv_request_header_t,ControllerFramework*,
   *              key,val,UncRespCode
   * @retval    - UncRespCode
   **/
UncRespCode
KtReadRequestHandler::handle_response(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_request_header_t &request_header,
    unc::driver::ControllerFramework* ctrl_int,
    unc::vtnreadutil::driver_read_util *read_util,
    UncRespCode &controller_resp_code_ ) {
  ODC_FUNC_TRACE;

  unc::driver::odl_drv_response_header_t resp_hdr;
  uint32_t err_= 0;
  create_response_header(request_header, resp_hdr, controller_resp_code_);
  UncRespCode resp_code_ = UNC_DRV_RC_ERR_GENERIC;
  resp_code_ = populate_response_header(sess, resp_hdr);
  if (resp_code_ != UNC_RC_SUCCESS) {
    pfc_log_error("%s: populate_response_header failed with ret_code ,%u",
                  PFC_FUNCNAME, resp_code_);
    return resp_code_;
  }

  err_ = sess.addOutput(resp_hdr.result);
  if (err_ != 0) {
    pfc_log_error("%s: Failed to send resp code:(err = %d)",
                  PFC_FUNCNAME, resp_code_);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  err_ = sess.addOutput((uint32_t) resp_hdr.key_type);
  if (err_ != 0) {
    pfc_log_error("%s: Failed to send driver data key type: (err = %d)",
                  PFC_FUNCNAME, resp_code_);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  // Add Read data to session
      pfc_log_trace("add read valuesto session");
      read_util->write_response_to_session();

  return UNC_RC_SUCCESS;
}


 /**
   * @brief    - This method creates the Response Header
   * @param[in]- odl_drv_response_header_t,
   *             odl_drv_request_header_t,UncRespCode
   * @retval   - void
   */
void
KtReadRequestHandler::create_response_header(
    unc::driver::odl_drv_request_header_t &reqhdr,
    unc::driver::odl_drv_response_header_t &resphdr,
    UncRespCode &resp_code_) {
  memset(&resphdr, 0, sizeof(resphdr));

  /* Copy parameters from the request header to the response header. */
  memcpy(reinterpret_cast<void *>(&resphdr.header),
         reinterpret_cast<void *>(&reqhdr.header),
         sizeof(resphdr.header));

  char * resphdr_domain_id = reinterpret_cast<char*>
                             (&resphdr.header.domain_id);

  char * reqhdr_domain_id = reinterpret_cast<char*>
                             (&reqhdr.header.domain_id);

  pfc_log_debug("%s: resphdr.header.domain_id %s", PFC_FUNCNAME,
                resphdr_domain_id);

  pfc_log_debug("%s: reqhdr.header.domain_id %s", PFC_FUNCNAME,
                reqhdr_domain_id);

  /* Set the result code. */
  pfc_log_trace("%s: Result code: %u", PFC_FUNCNAME, resp_code_);

  resphdr.result = resp_code_;

  memcpy(resphdr.controller_name, reqhdr.controller_name,
         sizeof(resphdr.controller_name));
  pfc_log_trace("REQ  controller_name : %s", reqhdr.controller_name);

  resphdr.key_type = reqhdr.key_type;
}


/**
   * @brief    - This method populates the Response Header
   * @param[in]- ServerSession, odl_drv_response_header_t
   * @retval   - UncRespCode
   **/
UncRespCode
KtReadRequestHandler::populate_response_header(
    pfc::core::ipc::ServerSession &sess,
    unc::driver::odl_drv_response_header_t &resp_hdr) {
  ODC_FUNC_TRACE;

  uint32_t err = UNC_DRV_RC_ERR_GENERIC;

  const char* ctr_name =
      reinterpret_cast<const char*> (resp_hdr.controller_name);

  pfc_log_debug("%s: Request received from "
                "platfotm layer. controller_name = %s \n"
                "session_id = %u \n"
                "config_id = %u \n"
                "operation = %u \n"
                "option1 = %u \n"
                "option2 = %u \n"
                "data_type = %u \n"
                "result = %u \n", PFC_FUNCNAME, ctr_name,
                resp_hdr.header.session_id,
                resp_hdr.header.config_id, resp_hdr.header.operation,
                resp_hdr.header.option1, resp_hdr.header.option2,
                resp_hdr.header.data_type, resp_hdr.result);

  /* Sends session_id */
  err = sess.addOutput(resp_hdr.header.session_id);
  if (err) {
    pfc_log_fatal("%s: Failed to send client session id: (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends config_id */
  err = sess.addOutput(resp_hdr.header.config_id);
  if (err) {
    pfc_log_fatal("%s: Failed to send configurationid: (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends controller_name */
  const char* controller_name =
      reinterpret_cast<const char *> (resp_hdr.controller_name);
  err = sess.addOutput(controller_name);
  if (err) {
    pfc_log_fatal("%s: Failed to send controller id: (err = %d)", PFC_FUNCNAME,
                  err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends domain-id*/
  const char* domain_id =
      reinterpret_cast<const char *> (resp_hdr.header.domain_id);
  err = sess.addOutput(domain_id);
  if (err) {
    pfc_log_fatal("%s: Failed to send domain id:(err = %d)", PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends operation */
  err = sess.addOutput((uint32_t) resp_hdr.header.operation);
  if (err) {
    pfc_log_fatal("%s: Failed to send driver data operation (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends max-rep-count */
  err = sess.addOutput(resp_hdr.header.max_rep_count);
  if (err) {
    pfc_log_fatal("%s: Failed to send driver data max_rep_count: (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends option1 */
  err = sess.addOutput(resp_hdr.header.option1);
  if (err) {
    pfc_log_fatal("%s: Failed to send driver data option1 : (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends option2 */
  err = sess.addOutput(resp_hdr.header.option2);
  if (err) {
    pfc_log_fatal("%s: Failed to send driver data option2 : (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  /* Sends data type */
  err = sess.addOutput((uint32_t) resp_hdr.header.data_type);
  if (err) {
    pfc_log_fatal("%s: Failed to send driver data data_type: (err = %d)",
                  PFC_FUNCNAME, err);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  return UNC_RC_SUCCESS;
}

}  // namespace driver
}  // namespace unc

