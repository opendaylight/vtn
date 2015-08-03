/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    ipct_util declaration
 * @file     ipct_util.hh
 */

#ifndef IPCT_UTIL_HH
#define IPCT_UTIL_HH

#include <pfcxx/ipc_server.hh>
#include <pfcxx/module.hh>
#include <sstream>
#include <vector>
#include <string>
#include "physical_common_def.hh"
#include "unc/uppl_common.h"
#include "itc_kt_controller.hh"

using std::vector;
using std::string;
using std::stringstream;
using std::endl;
using pfc::core::ipc::ServerSession;
using pfc::core::ipc::ClientSession;
using pfc::core::ipc::ServerEvent;
using unc::uppl::ODBCManager;
using unc::uppl::DBTableSchema;

class IpctUtil  {
  public:
    static string get_string(const key_root_t &k);

    static string get_string(const key_ctr_t &k);
    static string get_string(const val_ctr_t &v);
    static string get_string(const val_ctr_st_t &v);
    static string get_string(const val_ctr_commit_ver_t &v);

    static string get_string(const key_boundary_t &key_obj);
    static string get_string(const val_boundary_t &val_obj);
    static string get_string(const val_boundary_st_t &val_obj);

    static string get_string(const key_ctr_domain &k);
    static string get_string(const val_ctr_domain &v);
    static string get_string(const val_ctr_domain_st &v);

    static string get_string(const key_link_t &k);
    static string get_string(const val_link_t &v);
    static string get_string(const val_link_st_t &v);

    static string get_string(const key_logical_member_port_t &k);
    static string get_string(const val_lm_port_st_neighbor &v);

    static string get_string(const key_logical_port_t &k);
    static string get_string(const val_logical_port_st_t &v);
    static string get_string(const val_logical_port_boundary_t &v);
    static string get_string(const val_logical_port_t &v);

    static string get_string(const key_port_t &k);
    static string get_string(const val_port_t &v);
    static string get_string(const val_port_st_t &v);
    static string get_string(const val_port_stats_t &v);
    static string get_string(const val_port_st_neighbor &v);

    static string get_string(const key_switch_t &k);
    static string get_string(const val_switch_t &v);
    static string get_string(const val_switch_st_t &v);
    static string get_string(const val_switch_st_detail_t &v);
    static std::string get_string(const val_df_data_flow_t &val_obj);
    static std::string get_string(const val_df_data_flow_st_t &val_obj);

    static string get_string(const val_path_fault_alarm_t  &v);
    static string get_string(const val_phys_path_fault_alarm_t  &v);
    static string get_string(const val_flow_entry_full_alarm_t  &v);
    static string get_string(const val_ofs_lack_features_alarm_t  &v);

    static string get_string(const val_port_alarm_t  &v);
};
#endif


