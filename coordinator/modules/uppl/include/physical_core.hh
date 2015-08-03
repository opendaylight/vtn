/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    Physical Core
 * @file     physicalcore.hh
 */

#ifndef _UNC_PHYSICAL_CORE_HH_
#define _UNC_PHYSICAL_CORE_HH_

#include <unc/keytype.h>
#include <uncxx/tclib/tclib_interface.hh>
#include <pfcxx/module.hh>
#include <pfcxx/synch.hh>
#include <string>
#include <map>
#include <vector>
#include <list>
#include "physical_common_def.hh"
#include "physical_itc.hh"
#include "unc_state_handler.hh"
#include "capa_intf.hh"

using std::vector;
using std::map;
using std::list;
using unc::uppl::UncStateHandler;
using unc::tclib::TcCommonRet;
using unc::tclib::TcLibInterface;
using unc::tclib::TcTransEndResult;
using unc::tclib::TcAuditOpAbortPhase;
using unc::tclib::TcControllerList;

#define IPC_PARAMS_BLK           "ipc_params"

namespace unc {
namespace uppl {
/**
 * Struct consisting the capability detail to be updated in map
 */
struct cap_key_struct {
    uint32_t key_type;
    bool operator < (const cap_key_struct &var) const {
      return (this->key_type < var.key_type);
    }
};

/**
 * Struct consisting the capability detail to be updated in map
 */
struct attribute_struct {
    string attr_name;
};

struct cap_value_struct {
    uint32_t scalability_num;
    vector<attribute_struct> attrs;
};

/**
 * Physical Core Class
 */

class PhysicalCore : public TcLibInterface, public UncStateHandler {
  public:
    /**
     * @Description : The function returns singleton instance of
     * PhysicalCore class
     */

    static PhysicalCore* get_physical_core();

    /**
     * @Description : This function initializes physical core members and
     *                reads config, sends event subscription to driver
     * @param[in] :
     */

    UncRespCode InitializePhysical();

    /**
     * @Description : This function finalizes physical core members
     * @param[in] :
     */

    UncRespCode FinalizePhysical();

    /**
     * @Description : This function reads config from file
     * @param[in] :
     */

    UncRespCode ReadConfigFile();

    /**
     * @Description : This function sends event subscription request to Driver
     *                through IPCConnectionManager
     * @param[in] :
     */

    UncRespCode SendEventSubscripToDriver();

    /**
     * @Description : This function sends cancel event subscription request to
     *                Driver through IPCConnectionManager
     * @param[in] :
     */

    UncRespCode CancelEventSubscripInDriver();

    /**
     * @Description : This function validates the config ID by sending
     *                IPC msg to TC library
     * @param[in] : session_id - Session ID from the request
     * @param[in] : config_id - Configuration ID
     */

    UncRespCode ValidateConfigId(uint32_t session_id, uint32_t config_id);

    /**
     * @Description : This function validates if the operation is valid
     *                in STANDBY state
     * @param[in] : operation_type - type of operation requested
     */

    UncRespCode ValidateStandbyRequests(uint32_t operation_type);

    /**
     * @Description : This function gets the config mode from TC by sending
     *                IPC msg to TC library
     * @param[in] : session_id - Session ID from the request
     * @param[in] : config_id - Configuration ID
     * @param[out] : config_mode - config modes which are TC_CONFIG_GLOBAL or
     *              TC_CONFIG_REAL or TC_CONFIG_VIRTUAL or TC_CONFIG_VTN
     * @param[out] : vtn_name - VTN Name
     * */
    UncRespCode GetConfigMode(uint32_t session_id,
                                        uint32_t config_id,
                                        TcConfigMode &config_mode,
                                        std::string vtn_name);

    /**
     * @Description : This inline function gives the instance of
     *                InternalTransactionCoordinator class
     * @param[in] :
     */

    inline InternalTransactionCoordinator* get_internal_transaction_coordinator(
    ) {
      return internal_transaction_coordinator_;
    }

    /**
     * @Description : This function sends CONROLLER_DISCONNECT alarm to
     *                node manager
     *                This function will be called from Kt_Controller class
     *                when it receives CONTROLLER notification from driver
     *                with oper_status as down
     *
     * @param[in] : controller_id - Controller identification
     */

    UncRespCode SendControllerDisconnectAlarm(string controller_id);

    /**
     * @Description : This function sends CONROLLER_CONNECT alarm to
     *                node manager
     *                This is a clearance alarm for CONTROLLER_DISCONNECT alarm
     *                This function will be called from Kt_Controller class
     *                when it receives CONTROLLER notification from driver
     *                with oper_status as up
     *
     * @param[in] : controller_id - Controller identification
     */

    UncRespCode SendControllerConnectAlarm(string controller_id);

     /**
     *@Description : This function sends EVENT_HANDLING_SUCCESS alarm to
     *               node manager
     *               This is a clearance alarm for EVENT_HANDLING FAILURE alarm
     *
     *
     *@param[in] : controller_id - Controller identification
     * event_failed - Additional details about failed event handling
     *
     **/
    UncRespCode SendEventHandlingSuccessAlarm(string controller_id,
                                                 string event_details);

     /**
     *@Description : This function sends EVENT_HANDLING_FAILURE alarm to
     *               node manager
     *@param[in] : controller_id - Controller identification
     * event_failed - Additional details about failed event handling
     *
     **/
    UncRespCode SendEventHandlingFailureAlarm(string controller_id,
                                                string event_details);
    UncRespCode SendEventPostFailureNotice(string controller_id,
                                               string event_details);
    UncRespCode SendDuplicateControllerIdAlarm(string dup_ctr_id,
                         string actual_id, string orig_controller_id);

    UncRespCode SendControllerAuditFailureAlarm(string controller_id);

    UncRespCode SendControllerAuditSuccessAlarm(string controller_id);

    /**
     * @Description : This function gives the corresponding for controller type
     *
     *
     * @param[in] : controller_type - Type of Controller
     * @param[in] : driver_name - driver name is filled in this variable
     *
     */
    UncRespCode GetDriverName(unc_keytype_ctrtype_t controller_type,
                                 string &driver_name);

    /**
     * @Description : This function will be called back when TC sends user
     *                commit-transaction start request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleCommitTransactionStart(uint32_t session_id,
                                             uint32_t config_id,
                                             TcConfigMode config_mode,
                                             std::string vtn_name);

    /**
     * @Description : This function will be called back when TC sends user
     *                commit-transaction end request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleCommitTransactionEnd(uint32_t session_id,
                                           uint32_t config_id,
                                           TcConfigMode config_mode,
                                           std::string vtn_name,
                                           TcTransEndResult);

    /**
     * @Description : This function will be called back when TC sends user
     *                commit vote request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleCommitVoteRequest(uint32_t session_id,
                                        uint32_t config_id,
                                        TcConfigMode config_mode,
                                        std::string vtn_name,
                                        TcDriverInfoMap& driver_info);

    /**
     * @Description : This is a dummy function. TcLibInterface has pure virtual
     *                functions. All functions has to have an implementation to
     *                avoid runtime conflicts
     */

    TcCommonRet HandleCommitVoteRequest(uint32_t session_id, uint32_t config_id,
                                        TcControllerList controllers) {
      // To check
      return unc::tclib::TC_SUCCESS;
    }

    /**
     * @Description : This function will be called back when TC sends user
     *                Global commit request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleCommitGlobalCommit(uint32_t session_id,
                                         uint32_t config_id,
                                         TcConfigMode config_mode,
                                         std::string vtn_name,
                                         TcDriverInfoMap& driver_info);

    /**
     * @Description : This is a dummy function. TcLibInterface has pure virtual
     *                functions. All functions has to have an implementation to
     *                avoid runtime conflicts
     */

    TcCommonRet HandleCommitGlobalCommit(uint32_t session_id,
                                         uint32_t config_id,
                                         TcControllerList controllers) {
      return unc::tclib::TC_SUCCESS;
    }

    /**
     * @Description : This function will be called back when TC sends user
     *                Driver Result to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleCommitDriverResult(uint32_t session_id,
                                         uint32_t config_id,
                                         TcConfigMode config_mode,
                                         std::string vtn_name,
                                         TcCommitPhaseType commitphase,
                                         TcCommitPhaseResult driver_result);
    /**
     * @Description : This is a dummy function. TcLibInterface has pure virtual
     *                functions. All functions has to have an implementation to
     *                avoid runtime conflicts
     */

    TcCommonRet HandleCommitGlobalAbort(uint32_t session_id, uint32_t config_id,
                                        TcConfigMode config_mode,
                                        std::string vtn_name,
                                        TcCommitOpAbortPhase fail_phase);

    /**
     * @Description : This function will be called back when TC sends user
     *                Audit Start request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAuditStart(uint32_t session_id,
                                 unc_keytype_ctrtype_t driver_id,
                                 string controller_id,
                                 TcAuditType audit_type,
                                 uint64_t  commit_number,
                                 uint64_t  commit_date,
                                 std::string commit_application);
    /**
     * @Description : This is a dummy function. TcLibInterface has pure virtual
     *                functions. All functions has to have an implementation to
     *                avoid runtime conflicts
     */
    TcCommonRet HandleAuditStart(uint32_t session_id,
                                 unc_keytype_ctrtype_t ctr_type,
                                 string controller_id,
                                 pfc_bool_t force_reconnect,
                                 TcAuditType audit_type) {
      /*this should not called by UPPL*/
      return unc::tclib::TC_FAILURE;
    }
    /**
     * @Description : This function will be called back when TC sends user
     *                Audit End request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAuditEnd(uint32_t session_id,
                               unc_keytype_ctrtype_t driver_id,
                               string controller_id,
                               TcAuditResult audit_result);
    /**
     * @Description : This function will be called back when TC sends user
     *                Audit-transaction start request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAuditTransactionStart(uint32_t session_id,
                                            unc_keytype_ctrtype_t driver_id,
                                            string controller_id);


    /**
     * @Description : This function will be called back when TC sends user
     *                Audit-transaction End request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAuditTransactionEnd(uint32_t session_id,
                                          unc_keytype_ctrtype_t driver_id,
                                          string controller_id,
                                          TcTransEndResult);

    /**
     * @Description : This function will be called back when TC sends user
     *                Audit vote request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAuditVoteRequest(uint32_t session_id,
                                       uint32_t driver_id,
                                       string controller_id,
                                       TcDriverInfoMap &driver_info);
    /**
     * @Description : This is a dummy function. TcLibInterface has pure virtual
     *                functions. All functions has to have an implementation to
     *                avoid runtime conflicts
     */
    TcCommonRet HandleAuditVoteRequest(uint32_t session_id,
                                       string controller_id,
                                       TcControllerList controllers) {
      // to check
      return unc::tclib::TC_SUCCESS;
    }

    /**
     * @Description : This function will be called back when TC sends user
     *                Audit Global Commit request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAuditGlobalCommit(uint32_t session_id,
                                        uint32_t driver_id,
                                        string controller_id,
                                        TcDriverInfoMap& driver_info,
                                        TcAuditResult& audit_result);
    /**
     * @Description : This is a dummy function. TcLibInterface has pure virtual
     *                functions. All functions has to have an implementation to
     *                avoid runtime conflicts
     */

    TcCommonRet HandleAuditGlobalCommit(uint32_t session_id,
                                        string controller_id,
                                        TcControllerList controllers) {
      // to check
      return unc::tclib::TC_SUCCESS;
    }

    /**
     * @Description : This function will be called back when TC sends user
     *                Driver Result to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAuditDriverResult(uint32_t session_id,
                                        string controller_id,
                                        TcCommitPhaseType commitphase,
                                        TcCommitPhaseResult driver_result,
                                        TcAuditResult& audit_result);
    /**
     * @Description : This function will be called back when TC sends
     *                abort during audit transaction phase
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAuditGlobalAbort(uint32_t session_id,
                                       unc_keytype_ctrtype_t driver_id,
                                       string controller_id,
                                       TcAuditOpAbortPhase operation_phase);

    /**
     * @Description : This function will be called back when TC sends
     *                cancel audit when commit/abort request arrives 
     *                with cancel flag enable
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAuditCancel(uint32_t session_id,
                                  unc_keytype_ctrtype_t driver_id,
                                  string controller_id);
    /**
     * @Description : This function will be called back when TC sends user
     *                save configuration request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleSaveConfiguration(uint32_t session_id,
                                        uint64_t save_version);
    /**
     * @Description : This function will be called back when TC sends user
     *                Abort Candidate request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAbortCandidate(uint32_t session_id, uint32_t config_id,
                                     TcConfigMode config_mode,
                                     std::string vtn_name,
                                     uint64_t abort_version);

    /**
     * @Description : This function will be called back when TC sends user
     *                clear startup request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleClearStartup(uint32_t session_id);

    TcCommonRet SendControllerInfoToUPLL(OdbcmConnectionHandler *db_conn,
                                         uint32_t dt_type);

    /**
     * @Description : This function will be called back when TC sends
     *                db recovery requests during failover
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAuditConfig(unc_keytype_datatype_t db_target,
                                  TcServiceType fail_oper,
                                  TcConfigMode config_mode,
                                  std::string vtn_name, uint64_t version);

    /**
     * @Description : This function will be called back when TC sends
     *                load startup configuration request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleSetup();

    /**
     * @Description : This function will be called back when TC sends
     *                load startup configuration request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleSetupComplete();


    /**
     * @Description : This function returns the type of controller
     *                This is an implementation of virtual function in TCLib
     */

    unc_keytype_ctrtype_t HandleGetControllerType(string controller_id);

    /**
     * @Description : This is a dummy function. TcLibInterface has pure virtual
     *                functions. All functions has to have an implementation to
     *                avoid runtime conflicts
     */

    unc_keytype_ctrtype_t HandleGetControllerType() {
      return UNC_CT_UNKNOWN;
    }

    uint32_t getAuditNotfnTimeOut() {
      return audit_notfn_timeout_;
    }
    uint32_t getUnknownControllerCount() {
      return unknown_controller_count_;
    }
    //  getter method to access unc_mode_
    UncMode getunc_mode() {
      return unc_mode_;
    }

    vector<string> event_handling_controller_alarm_;
    UncRespCode RaiseEventHandlingAlarm(string controller_name);
    UncRespCode ClearEventHandlingAlarm(string controller_name);

    /**
    *  @Description : This is a map to keep the alarms which has been sent to
    *  node manager detail
    */
    std::map<std::string, bool> alarm_status_map_;
    UncRespCode remove_ctr_from_alarm_status_map(
         std::string , std::string);
    static unc::capa::CapaIntf *GetCapaInterface();
    /* Variable to hold max dataflow traversal count to verify during dataflow
     * traversal, dataflow traversal is limited by this count value
     *  */
    uint32_t uppl_max_dataflowtraversal_;
    /*variable to hold the max. allowed RO db connections */
    uint32_t uppl_max_ro_db_connections_;
    /*this flag enables to send the user initiated operations 
    *          recovery alarm to nodemgr */
    bool system_transit_state_;
    /*Auto save changes*/
    uint16_t getStartupValidStatus();

  private:
    /* PhysicalCore Instance */
    static PhysicalCore* physical_core_;

    /* This list contains the ids of the IPC services provided by south bound */
    list<uint8_t> sb_ipc_service_ids_;

    /* This list contains the ids of the IPC services
     *  provided by Logical Layer */
    list<uint8_t> logical_ipc_service_ids_;

    /* This list contains the ids of the IPC services
     *  provided by Physical Layer*/
    list<uint8_t> uppl_ipc_service_ids_;

    /* This variable contains the IPC channel name provided by UPPL */
    string uppl_ipc_channel_name_;

    /* This variable contains the IPC service name provided by UPPL */
    string uppl_ipc_service_name_;

    /* Map to store driver names */
    map<unc_keytype_ctrtype_t, string> drv_name_map_;

    /* Variable to hold time out to process notifications from driver
     * after audit end
     */
    uint32_t audit_notfn_timeout_;

    /* Variable to hold unknown controller count to verify during creation
     * number of unknown controllers should be allowed
     */
    uint32_t unknown_controller_count_;
    /* This variable contains the InternalTransactionCoordinator singleton
     *  object
     */
    InternalTransactionCoordinator* internal_transaction_coordinator_;
    /* This variable contains the unc_mode value, separate or coexist*/
    UncMode unc_mode_;

    /* Constructor */
    PhysicalCore() :
      uppl_max_dataflowtraversal_(0),
      uppl_max_ro_db_connections_(0),
      system_transit_state_(false),
      audit_notfn_timeout_(0),
      unknown_controller_count_(0),
      internal_transaction_coordinator_(NULL),
      unc_mode_(UNC_SEPARATE_MODE) {
    }
    /* Destructor */
    ~PhysicalCore() {
      if (!alarm_status_map_.empty())
        alarm_status_map_.clear();
    }
};
}  // namespace uppl
}  // namespace unc

#endif
