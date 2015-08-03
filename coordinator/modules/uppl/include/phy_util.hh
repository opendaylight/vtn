/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "odbcm_connection.hh"

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
using unc::uppl::ODBCMTableColumns;
using unc::uppl::OdbcmConnectionHandler;

const uint16_t UPPL_NO_VAL_STRUCT = -1;

class PhyUtil {
  public:
    static void printReqHeader(const physical_request_header& rqh);
    static void printRespHeader(const physical_response_header& rsh);
    static void getRespHeaderFromReqHeader(const physical_request_header& rqh,
                                           physical_response_header& rsh);
    static int sessOutRespHeader(ServerSession& sess,
                                 const physical_response_header& rsh);
    static int sessGetReqHeader(ServerSession& sess,
                                physical_request_header& rqh);
    static int sessOutReqHeader(ClientSession& sess,
                                const physical_request_header& rqh);
    static int sessGetRespHeader(ClientSession& sess,
                                 physical_response_header& rsh);
    static void printDriverReqHeader(const driver_request_header& rqh);
    static void printDriverRespHeader(const driver_response_header& rsh);
    static int sessOutDriverReqHeader(ClientSession& sess,
                                      const driver_request_header& rqh);
    static int sessGetDriverRespHeader(ClientSession& sess,
                                       driver_response_header& rsh);
    static int sessGetDriverEventHeader(ClientSession& sess,
                                        driver_event_header& rsh);
    static int sessGetDriverAlarmHeader(ClientSession& sess,
                                        driver_alarm_header& rsh);
    static int sessOutNBEventHeader(ServerEvent& sess,
                                    const northbound_event_header& rqh);
    static int sessOutNBAlarmHeader(ServerEvent& sess,
                                    const northbound_alarm_header& rqh);
    static string uint8tostr(const uint8_t &c);
    static string uint16tostr(const uint16_t &c);
    static string uint32tostr(const uint32_t &c);
    static string uint64tostr(const uint64_t &c);
    static int uint8touint(const uint8_t &c);
    static unsigned int strtouint(const string &str);
    static uint64_t strtouint64(const string &str);
    static void FillDbSchema(ODBCMTableColumns attr_name,
                             string attr_value,
                             unsigned int attr_length,
                             AttributeDataType attr_type,
                             vector<TableAttrSchema> &vect_attr);
    static void FillDbSchema(ODBCMTableColumns attr_name,
                             uint8_t* attr_value,
                             unsigned int attr_length,
                             AttributeDataType attr_type,
                             vector<TableAttrSchema> &vect_attr);
    static void FillDbSchema(ODBCMTableColumns attr_name,
                             string attr_name_str,
                             string attr_value,
                             unsigned int attr_length,
                             AttributeDataType attr_type,
                             uint32_t operation_type,
                             uint16_t in_valid_val,
                             uint16_t prev_db_valid_val,
                             vector<TableAttrSchema> &vect_attr,
                             vector<string> &vect_prim_keys,
                             stringstream &out_valid_value);
    static void FillDbSchema(ODBCMTableColumns attr_name,
                             string attr_name_str,
                             uint8_t* attr_value,
                             unsigned int attr_length,
                             AttributeDataType attr_type,
                             uint32_t operation_type,
                             uint16_t in_valid_val,
                             uint16_t prev_db_valid_val,
                             vector<TableAttrSchema> &vect_attr,
                             vector<string> &vect_prim_keys,
                             stringstream &out_valid_value);
    static void GetValueFromDbSchema(const TableAttrSchema& table_attr_schema,
                                     string &attr_value,
                                     AttributeDataType attr_type);
    static void GetValueFromDbSchemaStr(
        const TableAttrSchema& table_attr_schema,
        uint8_t *attr_value,
        AttributeDataType attr_type);
    static UncRespCode get_controller_type(
        OdbcmConnectionHandler *db_conn,
        string controller_name,
        unc_keytype_ctrtype_t& controller_type,
        unc_keytype_datatype_t datatype);
    static UncRespCode ConvertToControllerName(OdbcmConnectionHandler *db_conn,
                                  string actual_controller_id, string &ctr_name);
    static void reorder_col_attrs(
        vector<string> vect_prim_keys,
        vector<TableAttrSchema> &vect_table_attr_schema);

    static bool IsValidValue(uint32_t operation_type,
                      unsigned int valid);

    static bool IsFilteringOperation(uint32_t operation_type,
                              unsigned int valid);
    static std::string getEventDetailsString(pfc_ipcevtype_t);
};

#endif
