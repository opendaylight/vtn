/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *@brief   PhysicalLayer
 *@file    physical_layer.cc
 *
 * Desc:
 * This file contains the definition of PhysicalLayer class
 * which is the base class for all other classes
 */


#include "physicallayer.hh"

using unc::uppl::PhysicalLayer;

// It is a static variable
PhysicalLayer* PhysicalLayer::physical_layer_ = NULL;

// Static variable for mutex obj use in physical layer class
Mutex PhysicalLayer::physical_layer_mutex_;

// Static variable for mutex obj use in physical core class
Mutex PhysicalLayer::physical_core_mutex_;

// Static variable for mutex obj use in ipc_client_configuration_handler class
Mutex PhysicalLayer::ipc_client_config_hdlr_mutex_;

// Static variable for mutex obj use in ipc_client_logical_handler class
Mutex PhysicalLayer::ipc_client_logical_hdlr_mutex_;

// Static variable for mutex obj use in ipc_server_handler class
Mutex PhysicalLayer::ipc_server_hdlr_mutex_;

// Static variable for mutex obj use in notification_manager class
Mutex PhysicalLayer::notification_manager_mutex_;

// Static variable for mutex obj use in ODBCManager class
Mutex PhysicalLayer::ODBCManager_mutex_;

// Static variable for mutex obj use in InternalTransactionCoordinator class
Mutex PhysicalLayer::phyitc_mutex_;

// Static variable for mutex obj use in ODBC connection pool access
Mutex PhysicalLayer::db_conpool_mutex_;

// Static variable for mutex obj use to protect the ctr_oprn_status_ map object
Mutex PhysicalLayer::ctr_oprn_mutex_;

// Static variable for mutex obj use to protect the is_fatal_done flag
Mutex PhysicalLayer::fatal_mutex_;

ReadWriteLock PhysicalLayer::phy_fini_db_lock_;
ReadWriteLock PhysicalLayer::phy_fini_phycore_lock_;
ReadWriteLock PhysicalLayer::phy_fini_event_lock_;
ReadWriteLock PhysicalLayer::phy_dbsbcxn_lock_;
ReadWriteLock PhysicalLayer::events_done_lock_;
ReadWriteLock PhysicalLayer::timer_lock_;
ReadWriteLock PhysicalLayer::phy_sqlexec_lock_;
uint8_t PhysicalLayer::phyFiniFlag = 0;
bool PhysicalLayer::is_fatal_done = false;
std::map<string, CtrOprnStatus> PhysicalLayer::ctr_oprn_status_;

/**
 *@Description : This function will be automatically called by the PFC module
                framework.This is the module initialization function which
                invokes IntializePhysicalSubModules.
 *@param[in] : None
 *@return    : PFC_TRUE is returned if this module is initialized successfully
 *             otherwise PFC_FALSE is returned to denote error
 **/
pfc_bool_t PhysicalLayer::init() {
  pfc_log_info("Physical Layer init() called");
  uint8_t init_status = InitializePhysicalSubModules();
  if (init_status != UNC_RC_SUCCESS) {
    pfc_log_warn("Init failed with %d", init_status);
    return PFC_FALSE;
  }
  pfc_log_debug("Physical Layer init() is Successful");
  return PFC_TRUE;
}


/**
 *@Description : The fini() function will be called automatically by PFC core
                  framework when UPPL module is to be terminated.
 *@param[in] : None
 *@return    : PFC_TRUE is returned if this module is finalized successfully
 *             otherwise PFC_FALSE is returned to denote error
 **/
pfc_bool_t PhysicalLayer::fini() {
  pfc_log_info("Physical Layer's fini called");
  phyFiniFlag = 1;
  {
    ScopedReadWriteLock ipcFiniLock(PhysicalLayer::get_phy_fini_phycore_lock_(),
                                    PFC_TRUE);
  }
  UncRespCode fini_status = FinalizePhysicalSubModules();
  if (fini_status != UNC_RC_SUCCESS) {
    pfc_log_warn("Fini failed with %d", fini_status);
    return PFC_FALSE;
  }
  if (physical_layer_ != NULL) delete physical_layer_;
  pfc_log_debug("Fini is successful");
  return PFC_TRUE;
}


/**
 *@Description : This function instantiates PhysicalLayer singleton object.
 *@param[in] : None
 *@return : Pointer to PhysicalLayer
 **/
PhysicalLayer* PhysicalLayer::get_instance() {
  physical_layer_mutex_.lock();
  if (physical_layer_ == NULL) {
    pfc_log_debug(
        "Creating a new physical layer object for the singleton class");
    physical_layer_ = static_cast<PhysicalLayer *>
    (getInstance(UPPL_IPC_SVC_NAME));
    if (physical_layer_ == NULL) {
      UPPL_LOG_FATAL("Memory not allocated for physical_layer_");
    }
  }
  physical_layer_mutex_.unlock();
  return physical_layer_;
}


/**
 *@Description : This function instantiates objects for the classes
 *               PhysicalCore, IPCConnectionManager and ODBCManager.
 *@param[in] : None
 *@return    : UNC_RC_SUCCESS is returned if all sub modules are initialized
 *             successfully
 *             otherwise UNC_UPPL_RC_ERR* is returned to denote error

 **/
UncRespCode PhysicalLayer::InitializePhysicalSubModules() {
  UncRespCode ret = UNC_RC_SUCCESS;
  UncRespCode resp = UNC_RC_SUCCESS;
  pfc_log_debug("Initialising IPCConnectionManager submodule");
  ipc_connection_manager_= new IPCConnectionManager();
  if (ipc_connection_manager_ == NULL) {
    pfc_log_error("Memory not allocated for ipc_connection_manager_");
    return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  ipc_connection_manager_->InitializeIpcServerClient();
  pfc_log_info("IPCConnectionManager submodule initialised");
  physical_core_ = PhysicalCore::get_physical_core();
  if (physical_core_ == NULL) {
    pfc_log_error("Memory not allocated for physical_core_");
    return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  pfc_log_debug("Initialising PhysicalCore submodule");
  ret = physical_core_->InitializePhysical();
  if (ret != UNC_RC_SUCCESS) {
    pfc_log_error("Physical core initialise failed");
    resp = FinalizePhysicalSubModules();
    if (resp != UNC_RC_SUCCESS) return resp;
    return ret;
  }
  pfc_log_info("Physical Core initialised");
  pfc_log_debug("Initialising ODBCManger submodule");
  odbc_manager_= ODBCManager::get_ODBCManager();
  if (odbc_manager_ == NULL) {
    pfc_log_error("Memory not allocated for odbc_manager_");
    return UNC_UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  ODBCM_RC_STATUS odbc_ret = odbc_manager_->ODBCM_Initialize();
  if (odbc_ret != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCManager initialise failed");
    resp = FinalizePhysicalSubModules();
    if (resp != UNC_RC_SUCCESS) return resp;
    ret = UNC_UPPL_RC_FAILURE;
    return ret;  // Returns success code
  }
  pfc_log_info("ODBCManager initialised");
  pfc_log_info("All physical layer submodules initialised");
  return ret;  // Returns success code
}

/**
 *@Description : This function releases the  objects instantiated for the
                    classes PhysicalCore, IPCConnectionManager and
                    ODBCManager
 *@param[in] : None
 *@return    : UNC_RC_SUCCESS is returned if all sub modules are finalized
 *             successfully
 *             otherwise UNC_UPPL_RC_ERR* is returned to denote error
 **/
UncRespCode PhysicalLayer::FinalizePhysicalSubModules() {
  UncRespCode response = UNC_RC_SUCCESS;
  UncRespCode ret = UNC_RC_SUCCESS;
  {
    ScopedReadWriteLock eventFiniLock(PhysicalLayer::get_phy_fini_event_lock_(),
                                      PFC_TRUE);
    if (ipc_connection_manager_ != NULL) {
      ret = ipc_connection_manager_->Finalize();
      delete ipc_connection_manager_;
      ipc_connection_manager_ = NULL;
    }
  }
  if (ret != UNC_RC_SUCCESS) {
    pfc_log_error("IPCConnectionManager's Finalize failed");
    response = ret;
  }

  {
    ScopedReadWriteLock pcFiniLock(PhysicalLayer::get_phy_fini_phycore_lock_(),
                                   PFC_TRUE);
    ret = physical_core_->FinalizePhysical();
    if (ret != UNC_RC_SUCCESS) {
      pfc_log_error("Physical Core's Finalize failed");
      response = ret;
    }
    physical_core_ = NULL;
  }

  {
    ScopedReadWriteLock dbFiniLock(PhysicalLayer::get_phy_fini_db_lock(),
                                   PFC_TRUE);
    if (NULL != (odbc_manager_ = ODBCManager::get_ODBCManager())) {
      delete odbc_manager_;
      odbc_manager_ = NULL;
    } else {
      pfc_log_error("odbc_manager_ is already freed or NULL");
    }
  }
  // Deleting the CtrlOperationStatus map
  PhysicalLayer::ctr_oprn_mutex_.lock();
  map<string, CtrOprnStatus> :: iterator it;
  for (it = PhysicalLayer::ctr_oprn_status_.begin();
       it !=PhysicalLayer::ctr_oprn_status_.end(); it++) {
    if (it->second.rwlock_oper_st != NULL) delete (it->second).rwlock_oper_st;
  }
  PhysicalLayer::ctr_oprn_status_.clear();
  PhysicalLayer::ctr_oprn_mutex_.unlock();
  // Instance of physical core is already deleted FinalizePhysical()
  pfc_log_info("All the physical layer submodules finalised");
  return response;
}


/**
 *@Description : This is a call back function provided in IPC framework.
                This function will be called by IPC library when IPC Server
                receives a message corresponding to the registered services.
 * @param[in]   : session    - Object of ServerSession where the request
 *                argument present
 *                service_id - service id to classify the type of request.
 *                UPPL expects 1, 2 or 3.
 * @return      : Response code back to the caller.
 *   The system/generic level errors or common errors are generally
 *   returned in this function otherwise SUCCESS(0) will be returned.
 *   When an error code is being returned in this function, the caller
 *   cannot expect more specific error in response.
 *   When SUCCESS(0) is returned, the caller should further check
 *   the operation result_code in the response for more specific error.

 **/
pfc_ipcresp_t PhysicalLayer::ipcService(ServerSession &session,
                                        pfc_ipcid_t service_id) {
  pfc_log_debug("PhysicalLayer::ipcService is called with service id %d",
               service_id);
  PHY_FINI_IPC_LOCK(UNC_UPPL_RC_ERR_SHUTTING_DOWN);
  return ipc_connection_manager_->get_ipc_server_handler()->
      IpcService(session, service_id);
}


/**
 *@Description : Returns the singleton PhysicalCore object pointer
 *@param[in] : session, service_id
 *@return : Pointer to PhysicalCore
 **/
PhysicalCore* PhysicalLayer::get_physical_core() {
  return physical_core_;
}


/**
 *@Description : Returns the singleton IPCConnectionManager object pointer
 *@param[in] : None
 *@return : Pointer to IPCConnectionManager
 **/
IPCConnectionManager* PhysicalLayer::get_ipc_connection_manager() {
  return ipc_connection_manager_;
}


/**
 *@Description : Returns the singleton ODBCManager object pointer
 *@param[in] : None
 *@return : Pointer to ODBCManager
 **/
ODBCManager* PhysicalLayer::get_odbc_manager() {
  return odbc_manager_;
}

PFC_MODULE_IPC_DECL(unc::uppl::PhysicalLayer, 3);  // This macro
// registers the service handler

