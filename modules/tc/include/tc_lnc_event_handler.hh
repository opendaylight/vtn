/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_TC_LNC_HANDLER_HH_
#define UNC_TC_LNC_HANDLER_HH_

#include <launcher_api.h>
#include <tc_module_data.hh>
#include <tcmsg.hh>
#include <pfcxx/event.hh>
#include <pfcxx/synch.hh>
#include <string>
#include <map>
#include <vector>


namespace unc {
namespace tc {

typedef std::vector<std::string> channel_names;

class TcLncApiHandler {
 public:
  TcLncApiHandler() {}
  ~TcLncApiHandler() {}
  /* Method wrapping all methods to collect different
   * channel  names for all daemons
   */
  pfc_bool_t collect_unc_daemon_names();
  TcChannelNameMap get_tc_channels();


 private:
  /* Method to count number of channels */
  pfc_bool_t get_dmlist(channel_names* ch_names,
                      lnc_proctype_t unc_damon_type);
  /*  Methods to collect channel names for
   *  different  process types.
   */
  pfc_bool_t collect_upll_name();
  pfc_bool_t collect_uppl_name();
  pfc_bool_t collect_driver_names();
  /* map with <process_type,channel_name> */
  TcChannelNameMap unc_daemon_map_;
};

}  // namespace tc
}  // namespace unc
#endif
