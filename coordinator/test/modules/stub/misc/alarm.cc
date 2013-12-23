/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/un.h>
#include <math.h>
#include <string>
#include "cxx/pfcxx/module.hh"
#include "uncxx/alarm.hh"

namespace pfc {
namespace alarm {

class AlarmModule
  : public pfc::core::Module {
 public:
  explicit AlarmModule(const pfc_modattr_t *mattr) : pfc::core::Module(mattr) {}
  ~AlarmModule(void) {}

  pfc_bool_t init(void) { return PFC_TRUE; }
  pfc_bool_t fini(void) { return PFC_TRUE; }
};

/*
 * Stub for alarm API
 */

alarm_return_code_t
pfc_alarm_initialize(int32_t *fd) {
  *fd = -1;
  return ALM_OK;
}

alarm_return_code_t
pfc_alarm_send(const std::string& VTN_name, const std::string& alm_msg,
               const std::string& alm_msg_summary,
               alarm_info_t *data, int32_t fd) {
  return ALM_OK;
}

alarm_return_code_t
pfc_alarm_send_with_key(const std::string& VTN_name, const std::string& alm_msg,
                        const std::string& alm_msg_summary,
                        alarm_info_with_key_t* data, int32_t fd) {
  return ALM_OK;
}

alarm_return_code_t
pfc_alarm_send_with_key2(const std::string& VTN_name,
                         const std::string& alm_msg,
                         const std::string& alm_msg_summary,
                         alarm_info_with_key_t* data, int32_t fd,
                         struct timeval* tv) {
  return ALM_OK;
}

alarm_return_code_t
pfc_alarm_clear(uint8_t apl_No) {
  return ALM_OK;
}

alarm_return_code_t
pfc_alarm_close(int32_t fd) {
  return ALM_OK;
}

alarm_return_code_t
pfc_alarm_view_start(void) {
  return ALM_OK;
}
};  //  namespace alarm
};  //  namespace pfc

/* Declare C++ module. */
// PFC_MODULE_DECL(pfc::alarm::AlarmModule);
