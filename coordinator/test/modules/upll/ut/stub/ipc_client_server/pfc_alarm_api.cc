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
 * @file    pfc_alarm_api.cc
 * @brief   alarm API
 * @version S1.5
 * @date    2009-12-18
 * @author  ari\@NCOS
 */
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>

#include <sys/un.h>
#include <math.h>
#include <string>


//#include "pfcbase.h"
#include "uncxx/pfc_alarm_api.h"
//#include "unc/pfc_alarm_api_c.h"

//typedef alarm_info_t alarm_info_t_c;
//typedef alarm_info_with_key_t alarm_info_with_key_c;
namespace pfc{
namespace alarm{


alarm_return_code_t 
pfc_alarm_initialize(int32_t *fd)
{

    return intialize_;
}

void set_pfc_alarm_initialize(alarm_return_code_t ret)
{
	intialize_=ret;
}


alarm_return_code_t
pfc_alarm_send(const std::string& VTN_name, const std::string& alm_msg,
const std::string& alm_msg_summary, alarm_info_t *data, int32_t fd)
{

    return ALM_ERR;
}

alarm_return_code_t 
pfc_alarm_send_with_key(const std::string& VTN_name, const std::string& alm_msg,
const std::string& alm_msg_summary, alarm_info_with_key_t* data, int32_t fd)
{
	uint8_t* alarm_key((uint8_t*)malloc(sizeof(uint8_t)));
	data->alarm_key=alarm_key;
    return alarm_send_key_;
}

void set_pfc_alarm_send_with_key(alarm_return_code_t ret)
{
	alarm_send_key_ = ret;
}

// 日時指定のアラーム通知(key指定)
alarm_return_code_t 
pfc_alarm_send_with_key2(const std::string& VTN_name,
    const std::string& alm_msg, const std::string& alm_msg_summary,
    alarm_info_with_key_t* data, int32_t fd, struct timeval* tv)
{

    return ALM_ERR;
}

alarm_return_code_t
pfc_alarm_clear (uint8_t apl_No)
{

    return ALM_ERR;
}

alarm_return_code_t
pfc_alarm_close(int32_t fd)
{
    return ALM_ERR;
}

alarm_return_code_t pfc_alarm_view_start(void)
{
    return ALM_ERR;
}

};
};
