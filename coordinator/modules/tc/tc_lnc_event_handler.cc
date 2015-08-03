/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "tc_lnc_event_handler.hh"


namespace unc {
namespace tc {

/*
 *  @brief Get Count of channels for the daemon
 *  @param[OUT] ch_names container to collect channel names
 *  @param[IN] unc_daemon_type  Type  of UNC Process
 */
pfc_bool_t TcLncApiHandler::get_dmlist(channel_names* ch_names,
                                     lnc_proctype_t unc_daemon_type ) {
  lnc_dmlist_t  *listp;
  uint32_t  count, iter;
  int   err;
  err = lncapi_dmlist_create(unc_daemon_type, &listp, NULL);
  if (PFC_EXPECT_FALSE(err != 0)) {
    pfc_log_error("Failed to get daemon list:%u:%s",
                  unc_daemon_type, strerror(err));
    return PFC_FALSE;
  }

  count = lncapi_dmlist_getsize(listp);

  for (iter = 0; iter < count; iter++) {
    lnc_dminfo_t  info;
    err = lncapi_dmlist_get(listp, iter, &info);
    if (PFC_EXPECT_FALSE(err  != 0)) {
      lncapi_dmlist_destroy(listp);
      pfc_log_error("Failed to get daemon list:%u:%s",
                    unc_daemon_type, strerror(err));
      return PFC_FALSE;
    }
    ch_names->push_back(std::string(info.dmi_channel));
  }
  lncapi_dmlist_destroy(listp);
  return PFC_TRUE;
}

/*
 * @brief Collect UPLL channel  Names
 */
pfc_bool_t TcLncApiHandler::collect_upll_name() {
  channel_names upll_channel_name;
  if (PFC_EXPECT_FALSE(get_dmlist(&upll_channel_name, LNC_PROCTYPE_LOGICAL))
                       == PFC_FALSE) {
    return PFC_FALSE;
  }
  unc_daemon_map_.insert(std::pair<TcDaemonName, std::string>(TC_UPLL,
                                                    upll_channel_name.at(0)));
  pfc_log_info("upll=%s", upll_channel_name.at(0).c_str());
  return PFC_TRUE;
}

/*
 * @brief Collect UPPL channel  Names
 */
pfc_bool_t TcLncApiHandler::collect_uppl_name() {
  channel_names uppl_channel_name;
  if (PFC_EXPECT_FALSE(get_dmlist(&uppl_channel_name, LNC_PROCTYPE_PHYSICAL))
                       == PFC_FALSE) {
    return PFC_FALSE;
  }
  unc_daemon_map_.insert(std::pair<TcDaemonName, std::string>(TC_UPPL,
                                                   uppl_channel_name.at(0)));

  pfc_log_info("uppl=%s", uppl_channel_name.at(0).c_str());
  return PFC_TRUE;
}

/*
 * @brief Convert driver types to local TC driver type
 * @param[IN] unc_keytype_ctrtype_t Type  of Driver
 */
TcDaemonName TcConvertDriverType(unc_keytype_ctrtype_t drv_type) {
  switch ( drv_type ) {
    case UNC_CT_PFC:
      return TC_DRV_OPENFLOW;

    case UNC_CT_VNP:
      return TC_DRV_OVERLAY;

    /*case UNC_CT_LEGACY:
      return TC_DRV_LEGACY;*/

    case UNC_CT_POLC:
      return TC_DRV_POLC;

    case UNC_CT_VAN:
      return TC_DRV_VAN;

    case UNC_CT_ODC:
      return TC_DRV_ODC;

    default:
       break;
  }
  return TC_NONE;
}

/*
 * @brief Collect Driver Channel Names
 */
pfc_bool_t TcLncApiHandler::collect_driver_names() {
  channel_names driver_channel_name;
  if (PFC_EXPECT_FALSE(get_dmlist(&driver_channel_name, LNC_PROCTYPE_DRIVER)
                       == PFC_FALSE)) {
    return PFC_FALSE;
  }
  size_t count(driver_channel_name.size());
  if (PFC_EXPECT_FALSE(count == 0)) {
    pfc_log_info("No Drivers");
    return PFC_TRUE;
  }
  pfc_log_info("count of drivers=%" PFC_PFMT_SIZE_T, count);
  for (channel_names::iterator it(driver_channel_name.begin());
        it != driver_channel_name.end(); it++) {
    unc_keytype_ctrtype_t drv_type;
    /* Collect Controller type from Driver tclib*/
    TcOperRet ret(TcMsg::GetControllerType(*it, &drv_type));
    if (PFC_EXPECT_FALSE(ret != TCOPER_RET_SUCCESS)) {
      pfc_log_error("Unable to get driver info %s", it->c_str());
      return PFC_FALSE;
    }
    TcDaemonName unc_dmn_name;
    unc_dmn_name = TcConvertDriverType(drv_type);
    if ( PFC_EXPECT_FALSE(unc_dmn_name == 0) ) {
      pfc_log_error("Unknown Driver");
      return PFC_FALSE;
    }
    pfc_log_info("driver %d", unc_dmn_name);
    pfc_log_info("driver channel_name %s", it->c_str());
    unc_daemon_map_.insert(
        std::pair<TcDaemonName, std::string>(unc_dmn_name, *it));
  }
  return PFC_TRUE;
}

/*
 * @brief wrapper to collect all channel names
 */
pfc_bool_t TcLncApiHandler::collect_unc_daemon_names() {
  if ( collect_upll_name() == PFC_TRUE ) {
    if (collect_uppl_name() == PFC_TRUE) {
      if (collect_driver_names() == PFC_TRUE) {
        pfc_log_info("All Daemon Names Collected Successfully");
        return PFC_TRUE;
      }
    }
  }
  return PFC_FALSE;
}

/*
 * @brief Return the Result Map
 */

TcChannelNameMap TcLncApiHandler::get_tc_channels() {
  return unc_daemon_map_;
}


}  // namespace tc
}  // namespace unc
