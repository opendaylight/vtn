/* Copyright (c) 2009-2012 NEC Corporation                 */
/* NEC CONFIDENTIAL AND PROPRIETARY                        */
/* All rights reserved by NEC Corporation.                 */
/* This program must be used solely for the purpose for    */
/* which it was furnished by NEC Corporation. No part      */
/* of this program may be reproduced or disclosed to       */
/* others, in any form, without the prior written          */
/* permission of NEC Corporation. Use of copyright         */
/* notice does not evidence publication of the program.    */

/**
 * @file    pfc_alarm_api.h
 * @brief   alarm API header file
 * @version V1.0
 * @date    2012-12-26
 * @author  ari\@NCOS
 */

#ifndef LIB_ALM_API_H
#define LIB_ALM_API_H

#include <stdint.h>
#include <string>
#include <sys/time.h>

namespace pfc{
namespace alarm{

/*
 * alarm API
 */

#define ALARM_KEY_MAX  64  /* アラームキーの最大長(64Byte) */

/* アラーム番号 */
typedef struct alarm_info {
        uint8_t     alarm_class;
        uint8_t     apl_No;
        uint16_t    alarm_category;
        uint32_t    alarm_id ;
        uint8_t     alarm_kind;
} alarm_info_t;


/* アラーム通知構造体（alarm_key対応版）*/
typedef struct alarm_info_with_key {
        uint8_t  alarm_class;
        uint8_t  apl_No;
        uint16_t alarm_category;
        uint8_t  alarm_key_size;
        uint8_t* alarm_key;
        uint8_t  alarm_kind;
} alarm_info_with_key_t;


/* アラームクラス */
typedef enum{
    ALM_EMERG=0,
    ALM_ALERT,
    ALM_CRITICAL,
    ALM_ERROR,
    ALM_WARNING,
    ALM_NOTICE,
    ALM_INFO,
    ALM_DEBUG
}alarm_level_t;
    
/* alarm APIリターン値 */
typedef enum {
    ALM_OK = 0, /* 成功 */
    ALM_EAGAIN, /* 失敗：リトライ可 */
    ALM_ERR     /* 失敗：リトライ不可 */
} alarm_return_code_t;


static alarm_return_code_t intialize_;
static alarm_return_code_t alarm_send_key_;

/* API 関数 */
alarm_return_code_t 
pfc_alarm_initialize(int32_t *fd);

void set_pfc_alarm_initialize(alarm_return_code_t ret);

alarm_return_code_t
pfc_alarm_send(const std::string& VTN_name, const std::string& alm_msg,
const std::string& alm_msg_summary,  alarm_info_t *data,  int32_t fd);

alarm_return_code_t 
pfc_alarm_send_with_key(const std::string& VTN_name, const std::string& alm_msg,
const std::string& alm_msg_summary, alarm_info_with_key_t* data, int32_t fd);

void set_pfc_alarm_send_with_key(alarm_return_code_t ret);

alarm_return_code_t 
pfc_alarm_send_with_key2(const std::string& VTN_name,
        const std::string& alm_msg, const std::string& alm_msg_summary,
        alarm_info_with_key_t* data, int32_t fd, struct timeval* tv);

alarm_return_code_t
pfc_alarm_clear (uint8_t apl_No);

alarm_return_code_t
pfc_alarm_close(int32_t fd);

alarm_return_code_t pfc_alarm_view_start(void);

};
};

#endif /*! end of LIB_NOMG_CMD_H */

