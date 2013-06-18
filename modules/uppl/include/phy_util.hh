/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   Physical Utility header file
 * @file    phy_util.hh
 *
 */

#ifndef _PHY_UTIL_HH_
#define _PHY_UTIL_HH_

#include <pfcxx/ipc_server.hh>
#include <pfcxx/module.hh>
#include <sstream>
#include <vector>
#include <string>
#include "physical_common_def.hh"
#include "unc/uppl_common.h"
#include "odbcm_mgr.hh"

using std::vector;
using std::string;
using std::stringstream;
using std::endl;
using pfc::core::ipc::ServerSession;
using pfc::core::ipc::ClientSession;
using pfc::core::ipc::ServerEvent;
using unc::uppl::ODBCManager;
using unc::uppl::DBTableSchema;
using unc::uppl::ColumnAttrValue;
using unc::uppl::TableAttrSchema;

const uint16_t UPPL_NO_VAL_STRUCT = -1;

class PhyUtil {
  public:
    static void printReqHeader(physical_request_header rqh);
    static void printRespHeader(physical_response_header rsh);
    static void getRespHeaderFromReqHeader(physical_request_header rqh,
                                           physical_response_header& rsh);
    static int sessOutRespHeader(ServerSession& sess,
                                 physical_response_header& rsh);
    static int sessGetReqHeader(ServerSession& sess,
                                physical_request_header& rqh);
    static int sessOutReqHeader(ClientSession& sess,
                                physical_request_header rqh);
    static int sessGetRespHeader(ClientSession& sess,
                                 physical_response_header& rsh);
    static void printDriverReqHeader(driver_request_header rqh);
    static void printDriverRespHeader(driver_response_header rsh);
    static int sessOutDriverReqHeader(ClientSession& sess,
                                      driver_request_header rqh);
    static int sessGetDriverRespHeader(ClientSession& sess,
                                       driver_response_header& rsh);
    static int sessGetDriverEventHeader(ClientSession& sess,
                                        driver_event_header& rsh);
    static int sessGetDriverAlarmHeader(ClientSession& sess,
                                        driver_alarm_header& rsh);
    static int sessOutNBEventHeader(ServerEvent& sess,
                                    northbound_event_header& rqh);
    static int sessOutNBAlarmHeader(ServerEvent& sess,
                                    northbound_alarm_header& rqh);
    static string uint8tostr(uint8_t c);
    static string uint16tostr(uint16_t c);
    static string uint64tostr(uint64_t c);
    static int uint8touint(uint8_t c);
    static unsigned int strtouint(string str);
    static uint64_t strtouint64(string str);
    static void FillDbSchema(string attr_name,
                             string attr_value,
                             unsigned int attr_length,
                             AttributeDataType attr_type,
                             vector<TableAttrSchema> &vect_attr);
    static void FillDbSchema(string attr_name,
                             string attr_value,
                             unsigned int attr_length,
                             AttributeDataType attr_type,
                             uint32_t operation_type,
                             uint16_t in_valid_val,
                             uint16_t prev_db_valid_val,
                             vector<TableAttrSchema> &vect_attr,
                             vector<string> &vect_prim_keys,
                             stringstream &out_valid_value);
    static void GetValueFromDbSchema(TableAttrSchema table_attr_schema,
                                     string &attr_value,
                                     AttributeDataType attr_type);
    static UpplReturnCode get_controller_type(
        string controller_name,
        unc_keytype_ctrtype_t& controller_type,
        unc_keytype_datatype_t datatype);
    static UpplReturnCode GetControllerStatus(unc_keytype_datatype_t,
                                              string controller_id,
                                              uint8_t& oper_status);
    static UpplReturnCode GetOperStatus(unc_keytype_datatype_t,
                                        string controller_id,
                                        uint16_t& oper_status);
    static void reorder_col_attrs(
        vector<string> vect_prim_keys,
        vector<TableAttrSchema> &vect_table_attr_schema);

    static bool IsValidValue(uint32_t operation_type,
                      unsigned int valid);

    static bool IsFilteringOperation(uint32_t operation_type,
                              unsigned int valid);
};

#define FN_START_TIME(fn_name, kt) \
pfc_timespec_t start; \
pfc_clock_gettime(&start); \
pfc_log_info("Start Time for %s in function %s is %" PFC_PFMT_u64, \
         fn_name, kt, static_cast<uint64_t> (start.tv_nsec)); \

#define FN_END_TIME(fn_name, kt) \
pfc_timespec_t end; \
pfc_clock_gettime(&end); \
pfc_log_info("End Time for %s in function %s is %" PFC_PFMT_u64, \
           fn_name, kt, static_cast<uint64_t> (end.tv_nsec)); \
pfc_timespec_sub(&end, &start); \
uint64_t elapsed(pfc_clock_time2msec(&end)); \
pfc_log_info("Time taken for %s in function %s is %" PFC_PFMT_u64, \
     fn_name, kt, elapsed); \

#endif
