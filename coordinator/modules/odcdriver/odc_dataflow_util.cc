/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __READ_UTIL_DATAFLOW_HH__
#define __READ_UTIL_DATAFLOW_HH__
#include <driver/vtn_read_value_util.hh>

namespace unc {
namespace vtnreadutil {

void driver_dataflow_read_util::add_read_value(
    unc::dataflow::DataflowCmn* value,
    unc::vtnreadutil::driver_read_util *read_util) {
  if (!read_util->df_util_) {
    read_util->df_util_= new unc::dataflow::DataflowUtil();
    PFC_ASSERT(read_util->df_util_ != NULL);
    read_util->df_util_->appendFlow(value);
  } else {
    read_util->df_util_->appendFlow(value);
  }
  if (!read_util->df_cmn_) {
    read_util->df_cmn_ = value;
    PFC_ASSERT(read_util->df_cmn_ != NULL);
}
}


}  // namespace vtnreadutil
}  // namespace unc



#endif
