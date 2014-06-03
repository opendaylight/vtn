/*
 * Copyright (c) 2012-2014 NEC Corporation
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
#include "controller_version.hh"
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

typedef map<ControllerVersion, map<cap_key_struct,
    cap_value_struct > >::iterator cap_iter;

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
     * @Description : This function reads controller capability config
     * and fills the capability map
     * @param[in] :
     */

    UncRespCode ReadCtrlrStaticCapability();

    /**
     * @Description : This function validates key type controller capability map
     * @param[in] : version - Controller Version
     * @param[in] : key_type - Key type from NB
     */

    UncRespCode ValidateKeyTypeInCtrlrCap(string version,
                                             uint32_t key_type);

    /**
     * @Description : This function checks the attribute name in associated key
     *                type is available in controller capability map
     * @param[in] : version - Controller Version
     * @param[in] : key_type - Key type from NB
     * @param[in] : attribute_name - name of the attribute associated
     */

    UncRespCode ValidateAttribInCtrlrCap(string version,
                                            uint32_t key_type,
                                            string attribute_name);

    /**
     * @Description : This function gets the scalability number of associated
     *                key type
     * @param[in] : version - Controller Version
     * @param[in] : key_type - Key type from NB
     */

    UncRespCode GetScalabilityNumber(string version,
                                        uint32_t key_type,
                                        uint32_t &scalability_num);

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
     * @Description : This inline function gives the instance of
     *                InternalTransactionCoordinator class
     * @param[in] :
     */

    inline InternalTransactionCoordinator* get_internal_transaction_coordinator(
    ) {
      return internal_transaction_coordinator_;
    }

    /**
     * @Description : This function returns the list of supported controller
     *                version
     *                Controller version will be available in static
     *                capability file
     *                This function will be called from ITC
     * @param[in] :
     */

    list<string> GetControllerVersionList();

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
                                             uint32_t config_id);

    /**
     * @Description : This function will be called back when TC sends user
     *                commit-transaction end request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleCommitTransactionEnd(uint32_t session_id,
                                           uint32_t config_id,
                                           TcTransEndResult);

    /**
     * @Description : This function will be called back when TC sends user
     *                commit vote request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleCommitVoteRequest(uint32_t session_id,
                                        uint32_t config_id,
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
                                         TcCommitPhaseType commitphase,
                                         TcCommitPhaseResult driver_result);
    /**
     * @Description : This is a dummy function. TcLibInterface has pure virtual
     *                functions. All functions has to have an implementation to
     *                avoid runtime conflicts
     */

    TcCommonRet HandleCommitGlobalAbort(uint32_t session_id, uint32_t config_id,
                                        TcCommitOpAbortPhase fail_phase);

    /**
     * @Description : This function will be called back when TC sends user
     *                Audit Start request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAuditStart(uint32_t session_id,
                                 unc_keytype_ctrtype_t driver_id,
                                 string controller_id);
    /**
     * @Description : This is a dummy function. TcLibInterface has pure virtual
     *                functions. All functions has to have an implementation to
     *                avoid runtime conflicts
     */
    TcCommonRet HandleAuditStart(uint32_t session_id,
                                 unc_keytype_ctrtype_t driver_id,
                                 string controller_id, 
                                 pfc_bool_t force_reconnect) {
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
     * @Description : This function will be called back when TC sends user
     *                save configuration request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleSaveConfiguration(uint32_t session_id);
    /**
     * @Description : This function will be called back when TC sends user
     *                Abort Candidate request to UPPL
     *                This is a virtual function in TCLib
     */

    TcCommonRet HandleAbortCandidate(uint32_t session_id, uint32_t config_id);

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
                                  TcServiceType fail_oper);

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

  private:
    /* PhysicalCore Instance */
    static PhysicalCore* physical_core_;

    /* For each controller version, this map the Key types and the attributes
     * under each Key Type that would be supported by controller.
     * For each attribute the properties are stored in a map
     */
    map<ControllerVersion, map<cap_key_struct, cap_value_struct > >
    ctr_cap_map_;

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

    /* Constructor */
    PhysicalCore() :
      uppl_max_dataflowtraversal_(0),
      uppl_max_ro_db_connections_(0),
      system_transit_state_(false),
      audit_notfn_timeout_(0),
      unknown_controller_count_(0),
      internal_transaction_coordinator_(NULL) {
    }
    /* Destructor */
    ~PhysicalCore() {
      if (!alarm_status_map_.empty())
        alarm_status_map_.clear();
    }
    cap_iter GetVersionIterator(ControllerVersion version_in);
};
}  // namespace uppl
}  // namespace unc

#endif
