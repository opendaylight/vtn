/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <uncxx/pfc_alarm_api.h>
#include <string>

namespace pfc {
namespace alarm {


alarm_return_code_t
pfc_alarm_initialize(int32_t *fd) {
    return intialize_;
}

void set_pfc_alarm_initialize(alarm_return_code_t ret) {
  intialize_ = ret;
}


alarm_return_code_t
pfc_alarm_send(const std::string& VTN_name, const std::string& alm_msg,
const std::string& alm_msg_summary, alarm_info_t *data, int32_t fd) {
    return ALM_ERR;
}

alarm_return_code_t
pfc_alarm_send_with_key(const std::string& VTN_name, const std::string& alm_msg,
const std::string& alm_msg_summary, alarm_info_with_key_t* data, int32_t fd) {
  uint8_t* alarm_key(reinterpret_cast<uint8_t*>(malloc(sizeof(uint8_t))));
  data->alarm_key = alarm_key;
    return alarm_send_key_;
}

void set_pfc_alarm_send_with_key(alarm_return_code_t ret) {
  alarm_send_key_ = ret;
}

alarm_return_code_t
pfc_alarm_send_with_key2(const std::string& VTN_name,
    const std::string& alm_msg, const std::string& alm_msg_summary,
    alarm_info_with_key_t* data, int32_t fd, struct timeval* tv) {
    return ALM_ERR;
}

alarm_return_code_t
pfc_alarm_clear(uint8_t apl_No) {
    return ALM_ERR;
}

alarm_return_code_t
pfc_alarm_close(int32_t fd) {
    return ALM_ERR;
}

alarm_return_code_t pfc_alarm_view_start(void) {
    return ALM_ERR;
}
}  // namespace alarm
}  // namespace pfc
