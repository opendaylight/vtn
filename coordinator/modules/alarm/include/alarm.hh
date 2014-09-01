/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef ALARM_ALARM_HH
#define ALARM_ALARM_HH

#include <sys/time.h>
#include <stdint.h>
#include <string>

namespace pfc {
namespace alarm {

/*
 * alarm API
 */

#define ALARM_KEY_MAX  128

typedef struct alarm_info {
        uint8_t     alarm_class;
        uint8_t     apl_No;
        uint16_t    alarm_category;
        uint32_t    alarm_id;
        uint8_t     alarm_kind;
} alarm_info_t;

typedef struct alarm_info_with_key {
        uint8_t  alarm_class;
        uint8_t  apl_No;
        uint16_t alarm_category;
        uint8_t  alarm_key_size;
        uint8_t* alarm_key;
        uint8_t  alarm_kind;
} alarm_info_with_key_t;

typedef enum {
    ALM_EMERG = 0,
    ALM_ALERT,
    ALM_CRITICAL,
    ALM_ERROR,
    ALM_WARNING,
    ALM_NOTICE,
    ALM_INFO,
    ALM_DEBUG
}alarm_level_t;

typedef enum {
    ALM_OK = 0,
    ALM_EAGAIN,
    ALM_ERR
} alarm_return_code_t;

alarm_return_code_t
pfc_alarm_initialize(int32_t *fd);

alarm_return_code_t
pfc_alarm_send(const std::string& VTN_name, const std::string& alm_msg,
const std::string& alm_msg_summary,  alarm_info_t *data,  int32_t fd);

alarm_return_code_t
pfc_alarm_send_with_key(const std::string& VTN_name, const std::string& alm_msg,
const std::string& alm_msg_summary, alarm_info_with_key_t* data, int32_t fd);

alarm_return_code_t
pfc_alarm_send_with_key2(const std::string& VTN_name,
        const std::string& alm_msg, const std::string& alm_msg_summary,
        alarm_info_with_key_t* data, int32_t fd, struct timeval* tv);

alarm_return_code_t
pfc_alarm_clear(uint8_t apl_No);

alarm_return_code_t
pfc_alarm_close(int32_t fd);

alarm_return_code_t pfc_alarm_view_start(void);

};  // namespace alarm
};  // namespace pfc

#endif /* !ALARM_ALARM_HH */
