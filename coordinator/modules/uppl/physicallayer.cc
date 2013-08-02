/*
 * Copyright (c) 2012-2013 NEC Corporation
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
 * This header file contains the definition of PhysicalLayer class
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

ReadWriteLock PhysicalLayer::phy_fini_db_lock_;
ReadWriteLock PhysicalLayer::phy_fini_phycore_lock_;
ReadWriteLock PhysicalLayer::phy_fini_event_lock_;

uint8_t PhysicalLayer::phyFiniFlag = 0;

/**
 *@Description : This function will be automatically called by the PFC module
                framework.This is the module initialization function which
                invokes IntializePhysicalSubModules.
 *@param[in] : None
 *@return    : PFC_TRUE or PFC_FALSE
 **/
pfc_bool_t PhysicalLayer::init() {
  pfc_log_debug("Physical Layer's init called");
  uint8_t init_status = InitializePhysicalSubModules();
  pfc_log_info("Initialising physical sub mobules");
  if (init_status != UPPL_RC_SUCCESS) {
    pfc_log_debug("Init failed");
    return PFC_FALSE;
  }
  pfc_log_info("Init() is Successful");
  return PFC_TRUE;
}


/**
 *@Description : The fini() function will be called automatically by PFC core
                  framework when UPPL module is to be terminated.
 *@param[in] : None
 *@return    : PFC_TRUE or PFC_FALSE
 **/
pfc_bool_t PhysicalLayer::fini() {
  pfc_log_info("Physical Layer's fini called");
  phyFiniFlag = 1;
  {
    ScopedReadWriteLock ipcFiniLock(PhysicalLayer::get_phy_fini_phycore_lock_(),
                                    PFC_TRUE);
  }
  UpplReturnCode fini_status = FinalizePhysicalSubModules();
  pfc_log_debug("Finalising all physical sub modules");
  if (fini_status != UPPL_RC_SUCCESS) {
    pfc_log_debug("Finish failed");
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
      pfc_log_fatal("Memory not allocated for physical_layer_");
    }
  }
  physical_layer_mutex_.unlock();
  return physical_layer_;
}


/**
 *@Description : This function instantiates objects for the classes PhysicalCore,
                    IPCConnectionManager, ODBCManager and LogManager.
 *@param[in] : None
 *@return : UPPL_RC_SUCCESS or any associated error code
 **/
UpplReturnCode PhysicalLayer::InitializePhysicalSubModules() {
  UpplReturnCode ret = UPPL_RC_SUCCESS;
  UpplReturnCode resp = UPPL_RC_SUCCESS;
  pfc_log_info("Initialising IPCConnectionManager submodule");
  ipc_connection_manager_= new IPCConnectionManager();
  if (ipc_connection_manager_ == NULL) {
    pfc_log_error("Memory not allocated for ipc_connection_manager_");
    return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  ipc_connection_manager_->InitializeIpcServerClient();
  pfc_log_info("IPCConnectionManager submodule initialised");
  physical_core_ = PhysicalCore::get_physical_core();
  if (physical_core_ == NULL) {
    pfc_log_error("Memory not allocated for physical_core_");
    return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  pfc_log_info("Initialising PhysicalCore submodule");
  ret = physical_core_->InitializePhysical();
  if (ret != UPPL_RC_SUCCESS) {
    pfc_log_error("Physical core initialise failed");
    resp = FinalizePhysicalSubModules();
    if (resp != UPPL_RC_SUCCESS) return resp;
    return ret;
  }
  pfc_log_info("Physical Core initialised");
  pfc_log_info("Initialising ODBCManger submodule");
  odbc_manager_= ODBCManager::get_ODBCManager();
  if (odbc_manager_ == NULL) {
    pfc_log_error("Memory not allocated for odbc_manager_");
    return UPPL_RC_ERR_FATAL_RESOURCE_ALLOCATION;
  }
  ODBCM_RC_STATUS odbc_ret = odbc_manager_->ODBCM_Initialize();
  if (odbc_ret != ODBCM_RC_SUCCESS) {
    pfc_log_error("ODBCManager initialise failed");
    resp = FinalizePhysicalSubModules();
    if (resp != UPPL_RC_SUCCESS) return resp;
    ret = UPPL_RC_FAILURE;
    return ret;  // Returns success code
  }
  pfc_log_info("ODBCManager initialised");
  pfc_log_info("All physical layer submodules initialised");
  return ret;  // Returns success code
}

/**
 *@Description : This function releases the  objects instantiated for the
                    classes PhysicalCore,IPCConnectionManager,
                    ODBCManager and LogManager
 *@param[in] : None
 *@return : UPPL_RC_SUCCESS or any associated error code
 **/
UpplReturnCode PhysicalLayer::FinalizePhysicalSubModules() {
  UpplReturnCode response = UPPL_RC_SUCCESS;
  UpplReturnCode ret = UPPL_RC_SUCCESS;
  {
    ScopedReadWriteLock eventFiniLock(PhysicalLayer::get_phy_fini_event_lock_(),
                                      PFC_TRUE);
    if (ipc_connection_manager_ != NULL) {
      ret = ipc_connection_manager_->Finalize();
      delete ipc_connection_manager_;
      ipc_connection_manager_ = NULL;
    }
  }
  if (ret != UPPL_RC_SUCCESS) {
    pfc_log_error("IPCConnectionManager's Finalize failed");
    response = ret;
  }

  {
    ScopedReadWriteLock pcFiniLock(PhysicalLayer::get_phy_fini_phycore_lock_(),
                                   PFC_TRUE);
    ret = physical_core_->FinalizePhysical();
    if (ret != UPPL_RC_SUCCESS) {
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
      pfc_log_info("odbc_manager_ is freed");
    } else {
      pfc_log_error("odbc_manager_ is already freed or NULL");
    }
  }
  // Instance of physical core is already deleted FinalizePhysical()
  pfc_log_info("All the physical layer submodules finalised");
  return response;
}


/**
 *@Description : This is a call back function provided in IPC framework.
                This function will be called by IPC library when IPC Server
                receives a message corresponding to the registered services.
 *@param[in] : session, service_id
 *@return : ipc response
 **/
pfc_ipcresp_t PhysicalLayer::ipcService(ServerSession &session,
                                        pfc_ipcid_t service_id) {
  pfc_log_info("PhysicalLayer::ipcService is called with service id %d",
               service_id);
  PHY_FINI_IPC_LOCK(UPPL_RC_ERR_SHUTTING_DOWN);
  pfc_ipcresp_t resp = 0;
  resp = ipc_connection_manager_->get_ipc_server_handler()->
      IpcService(session, service_id);
  return resp;
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

PFC_MODULE_IPC_DECL(unc::uppl::PhysicalLayer, 5);  // This macro
// registers the service handler

