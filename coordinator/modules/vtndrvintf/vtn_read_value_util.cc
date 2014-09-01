 /*
  * Copyright (c) 2013-2014 NEC Corporation
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
             df_util_(NULL), sess_(sess) {}


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
      if (df_util_)
        df_util_->sessOutDataflowsFromDriver(sess_);
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


}  // namespace vtnreadutil
}  // namespace unc



