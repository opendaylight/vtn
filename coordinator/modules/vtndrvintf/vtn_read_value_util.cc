 /*
  * Copyright (c) 2013-2015 NEC Corporation
  * All rights reserved.
  *
  * This program and the accompanying materials are made available under the
  * terms of the Eclipse Public License v1.0 which accompanies this
  * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
  */
#include <driver/driver_interface.hh>
#include <handler.hh>

namespace unc {
namespace vtnreadutil {
  driver_read_util::driver_read_util(pfc::core::ipc::ServerSession& sess) :
             df_util_(NULL), df_cmn_(NULL), sess_(sess),
             alternate_flow(PFC_FALSE)  {}

  driver_read_util::~driver_read_util() {
    if ( df_util_ )
      delete df_util_;
     for (std::list<unc::vtnreadutil::read_cache_element*>::iterator
         it = drv_read_results_.begin();
         it!= drv_read_results_.end();
         it++) {
       delete *it;
     }
  }


  void driver_read_util::write_response_to_session() {
      ODC_FUNC_TRACE;
      for (std::list<unc::vtnreadutil::read_cache_element*>::iterator
          it = drv_read_results_.begin();
          it!= drv_read_results_.end();
          it++) {
            pfc_log_info("Iterations of writing read results to sess");
            (*it)->write_to_sess(sess_);
      }
      if ((df_util_) && (alternate_flow == PFC_FALSE) ) {
        df_util_->sessOutDataflowsFromDriver(sess_);
      } else if ((df_cmn_) && (alternate_flow == PFC_TRUE) ) {
        int putresp_pos = 13;
        pfc_log_info("df_cmn sessOutDataflow read results to sess");
        df_cmn_->sessOutDataflow(sess_, putresp_pos);
      }
  }


  uint32_t driver_read_util::get_arg_count() {
      ODC_FUNC_TRACE;
      return sess_.getArgCount();
  }

  uint32_t driver_read_util::get_option1() {
      ODC_FUNC_TRACE;
      uint32_t option(0);
      if (sess_.getArgument(unc::driver::IPC_OPTION1_INDEX, option))
        pfc_log_error("Error Reading Option1");
      return option;
  }

  uint32_t driver_read_util::get_option2() {
      ODC_FUNC_TRACE;
      uint32_t option(0);
      if (sess_.getArgument(unc::driver::IPC_OPTION2_INDEX, option))
        pfc_log_error("Error Reading Option2");
      return option;
  }

  const char* driver_read_util::get_ctr_id() {
      ODC_FUNC_TRACE;
      const char* option;
      if (sess_.getArgument(unc::driver::IPC_CONTROLLER_ID_INDEX, option))
        pfc_log_error("Error Reading Option2");
      return option;
  }

}  // namespace vtnreadutil
}  // namespace unc



