/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * @brief    ITC SystemStateChange Request
 * @file     itc_state_change.hh
 *
 */

#ifndef _PHYSICALLAYER_ITCSTATECHANGE_HH_
#define _PHYSICALLAYER_ITCSTATECHANGE_HH_

#include <list>
#include <string>
#include <vector>
#include "physical_itc_req.hh"

namespace unc {
namespace uppl {
class SystemStateChangeRequest:public ITCReq  {
  public:
  SystemStateChangeRequest();
  ~SystemStateChangeRequest();
  UpplReturnCode GetControllerListFromDb(uint32_t,
                                         vector<string> &vec_controller_name);
  UpplReturnCode SystemStateChangeToStandBy();
  UpplReturnCode SystemStateChangeToActive();
};
}  // namespace uppl
}  // namespace unc
#endif

