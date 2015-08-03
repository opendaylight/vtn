/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef _UNC_TC_MODULE_HH_
#define _UNC_TC_MODULE_HH_

#include <pfcxx/module.hh>
#include <pfcxx/synch.hh>
#include <tc_lock.hh>
#include <tc_operations.hh>
#include <unc/tc/external/tc_services.h>
#include <tc_lnc_event_handler.hh>
#include <unc_state_handler.hh>
#include <tc_db_handler.hh>
#include <tc_module_data.hh>
#include <string>



namespace unc {
namespace tc {

typedef enum {
  TC_API_COMMON_SUCCESS = 0,
  TC_INVALID_PARAM,
  TC_INVALID_UNC_STATE,
  TC_NO_CONFIG_SESSION,
  TC_API_COMMON_FAILURE
}TcApiRet;

class TcModule : public pfc::core::Module, public UncStateHandler {
 public:
  /* Constructor*/
  explicit TcModule(const pfc_modattr_t *mattr);

  /* Destructor*/
  ~TcModule();

  /* To initiate the tc module*/
  pfc_bool_t init();

  /* To stop the tc module*/
  pfc_bool_t fini();

  /**
   *  This function is used to receive messages to TC.
   *  @param[in]  sess  - Session Id.
   *  @param[in]  service - Represents the request sent from TC.
   */
  pfc_ipcresp_t ipcService(pfc::core::ipc::ServerSession& sess,
                           pfc_ipcid_t service);

  /* API's for Session Manager*/

  /* 
   * @brief Get the details of the config session
   * @param[in]  session_id - Session Id .
   * @param[out]  config_id - Represents the request sent from TC.  
   * @param[out]  tc_mode - Represents the request sent from TC.
   * @param[out]  vtn_name - Represents the request sent from TC.
*/
  TcApiRet TcGetConfigSession(uint32_t session_id,
                              uint32_t& config_id,
                              TcConfigMode& tc_mode,
                              std::string& vtn_name);

  /* API's for Session Manager*/

  /* 
   * @brief Tc Module API
   * API that is used to notify TC to release the session.
   */
  TcApiRet TcReleaseSession(uint32_t session_id);

  /* Methods to handle state transition notifications */
  pfc_bool_t HandleStart();
  pfc_bool_t HandleStop();
  pfc_bool_t HandleAct(pfc_bool_t is_switch);
  pfc_bool_t check_tc_data();

  /* Method to collect conf parameters */
  void collect_db_params();

  /* Methods to create Operation objects */
  pfc_bool_t validate_tc_db(TcDbHandler* db_handler);
  TcTaskqUtil* create_tc_taskq(uint32_t concurrency);

  /* Methods to handle service requests */
  TcOperStatus HandleConfigRequests(pfc::core::ipc::ServerSession* sess);
  TcOperStatus HandleCandidateRequests(pfc::core::ipc::ServerSession* sess);
  TcOperStatus HandleStartUpRequests(pfc::core::ipc::ServerSession* sess);
  TcOperStatus HandleReadRequests(pfc::core::ipc::ServerSession* sess);
  TcOperStatus HandleReadStatusRequests(pfc::core::ipc::ServerSession* sess);
  TcOperStatus HandleAutoSaveRequests(pfc::core::ipc::ServerSession* sess);
  TcOperStatus HandleAuditRequests(pfc::core::ipc::ServerSession* sess);
  TcOperStatus ReleaseConfigSession(uint32_t session_id);
  TcOperStatus ReleaseConfigSession();


 private:
  /* TcLock Instance for Exclusion */
  TcLock tc_lock_;
  /* Channel Names in UNC */
  TcChannelNameMap tc_channels_;
  /* Read Task Queue to handle timeout */
  TcTaskqUtil* read_q_;
  /* Audit Task Queue to handle driver audit requests */
  TcTaskqUtil* audit_q_;
  /* DB DSN NAME */
  std::string dsn_name;
  /* DB DSN name used in ACT */
  std::string act_dsn_name;
  /* DB DSN name used in SBY */
  std::string sby_dsn_name;
  /*Max Failover instance*/
  uint32_t max_failover_instance_;
  /* Seconday wait time for cancelled audit in seconds */
  uint32_t secondary_wait_time_for_cancelled_audit_;
};
}  //  namespace tc
}  //  namespace unc

#endif /* _UNC_TC_MODULE_HH_ */
