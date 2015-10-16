/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <vtn_drv_module.hh>
#include <odc_mod.hh>

namespace unc {
namespace odcdriver {

// Get Intsance of vtndrvintf and Register Driver
pfc_bool_t ODCModule::init() {
  ODC_FUNC_TRACE;
  pfc_log_debug("Launching Odc Module");
  unc::driver::VtnDrvIntf* disp_inst =
      static_cast<unc::driver::VtnDrvIntf*> (getInstance("vtndrvintf"));
  PFC_ASSERT(disp_inst != NULL);
  uint32_t ret_val = disp_inst->register_driver(this);
  if (ret_val) {
    pfc_log_error("Register driver failed");
    return PFC_FALSE;
  }
  pfc_log_debug(" %s Read Configuration file" , PFC_FUNCNAME);
  read_conf_file();
  return PFC_TRUE;
}

// Fini
pfc_bool_t ODCModule::fini() {
  ODC_FUNC_TRACE;
  return PFC_TRUE;
}

// Gets Conttoller type
unc_keytype_ctrtype_t ODCModule::get_controller_type() {
  ODC_FUNC_TRACE;
  return UNC_CT_ODC;
}

// Is two phase commit needed or not
pfc_bool_t ODCModule::is_2ph_commit_support_needed() {
  ODC_FUNC_TRACE;
  return PFC_FALSE;
}

// Is Audit Collection needed or not
pfc_bool_t ODCModule::is_audit_collection_needed() {
  ODC_FUNC_TRACE;
  return PFC_FALSE;
}

// Return New Contrtoller Pointer
unc::driver::controller* ODCModule::add_controller(const key_ctr_t& key_ctr,
                                                   const val_ctr_t& val_ctr) {
  ODC_FUNC_TRACE;
  const char* ctr_name =
      reinterpret_cast<const char*>(key_ctr.controller_name);
  if (0 == strlen(ctr_name)) {
    return NULL;
  }
  return new OdcController(key_ctr, val_ctr, conf_file_values_);
}

// Start audit notification to TC
uint32_t ODCModule::notify_audit_start_to_tc(
    std::string controller_id) {
  ODC_FUNC_TRACE;
  // Send start audit notification to TC
  unc::tclib::TcLibModule* ptr_tclib = NULL;
  ptr_tclib  = static_cast<unc::tclib::TcLibModule*>
      (unc::tclib::TcLibModule::getInstance(TCLIB_MODULE_NAME));
  PFC_ASSERT(ptr_tclib != NULL);
  ptr_tclib->TcLibAuditControllerRequest(controller_id);
  return ODC_DRV_SUCCESS;
}

// Return Updated Contrtoller Pointer
pfc_bool_t ODCModule::update_controller(const key_ctr_t& key_ctr,
                                        const val_ctr_t& val_ctr,
                              unc::driver::controller* ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  pfc_bool_t old_audit_status = ctr_ptr->get_audit_status();
  ctrl_info_update_type_t change_type = compare_ctr_info(ctr_ptr,
                                                         val_ctr);
  pfc_log_debug(" change type, %d" , change_type);

  ctr_ptr->update_ctr(key_ctr, val_ctr);
 // If audit status is disable and IP address Added or updated,
 // then set audit result to PFC_FALSE
  if ((change_type == CTRLINFO_IP_ADDED || CTRLINFO_IP_CHANGED) &&
      (!val_ctr.enable_audit)) {
     pfc_log_debug(" %s IP added/changed set audit result" , PFC_FUNCNAME);
     ctr_ptr->set_audit_result(PFC_FALSE);
  }
  // If previous audit status is disable and current audit status is enable,
  // then trigger sudit
  if ((!old_audit_status) && (val_ctr.enable_audit)) {
    pfc_log_debug(" %s Audit status changed, triggering audit" , PFC_FUNCNAME);
    std::string controller_name = ctr_ptr->get_controller_id();
    notify_audit_start_to_tc(controller_name);
  }
  return PFC_TRUE;
}

// Deletes the Controller Pointer
pfc_bool_t ODCModule::delete_controller(unc::driver::controller* ctr_ptr) {
  ODC_FUNC_TRACE;
  if (NULL != ctr_ptr) {
    delete ctr_ptr;
    ctr_ptr = NULL;
    return PFC_TRUE;
  }
  return PFC_FALSE;
}

// Gets the driver command
unc::driver::driver_command* ODCModule::create_driver_command(
    unc_key_type_t key_type) {
  ODC_FUNC_TRACE;
  unc::driver::driver_command* driver_cmd_ptr = NULL;
  switch (key_type) {
    case UNC_KT_VTN: {
      pfc_log_debug("UNC_KT_VTN key type received");
      driver_cmd_ptr = new OdcVtnCommand(conf_file_values_);
      break;
    }
    case UNC_KT_VBRIDGE: {
      pfc_log_debug("UNC_KT_VBR key type received");
      driver_cmd_ptr = new OdcVbrCommand(conf_file_values_);
      break;
    }
    case UNC_KT_VBR_IF: {
      pfc_log_debug("UNC_KT_VBRIF key type received");
      driver_cmd_ptr = new OdcVbrIfCommand(conf_file_values_);
      break;
    }
    case UNC_KT_VBR_VLANMAP: {
      pfc_log_debug("UNC_KT_VBR_VLANMAP key type received");
      driver_cmd_ptr = new OdcVbrVlanMapCommand(conf_file_values_);
      break;
    }
    case UNC_KT_VTERMINAL: {
      pfc_log_debug("UNC_KT_VTERMINAL key type received");
      driver_cmd_ptr = new OdcVterminalCommand(conf_file_values_);
      break;
    }

    case UNC_KT_VTERM_IF: {
      pfc_log_debug("UNC_KT_VTERM_IF key type received");
      driver_cmd_ptr = new OdcVtermIfCommand(conf_file_values_);
      break;
    }

    case UNC_KT_FLOWLIST: {
      pfc_log_debug("UNC_KT_FLOWLIST key type received");
      driver_cmd_ptr = new OdcFlowListCommand(conf_file_values_);
      break;
    }

    case UNC_KT_FLOWLIST_ENTRY: {
      pfc_log_debug("UNC_KT_FLOWLIST_ENTRY key type received");
      driver_cmd_ptr = new OdcFlowListEntryCommand(conf_file_values_);
      break;
    }

    case UNC_KT_VTN_FLOWFILTER: {
      pfc_log_debug("UNC_VTN_FLOWFILTER key type received");
      driver_cmd_ptr = new OdcVtnFlowFilterCmd(conf_file_values_);
      break;
    }

    case UNC_KT_VTN_FLOWFILTER_ENTRY: {
      pfc_log_debug("UNC_VTN_FLOW_FILTER_ENTRY key type received");
      driver_cmd_ptr = new OdcVtnFlowFilterEntryCmd(conf_file_values_);
      break;
    }

    case UNC_KT_VBR_FLOWFILTER : {
      pfc_log_debug("UNC_VBR_FLOW_FILTER key type received");
      driver_cmd_ptr = new OdcVbrFlowFilterCmd(conf_file_values_);
      break;
    }

    case UNC_KT_VBR_FLOWFILTER_ENTRY : {
      pfc_log_debug("UNC_VBR_FLOW_FILTER_ENTRY key type received");
      driver_cmd_ptr = new OdcVbrFlowFilterEntryCmd(conf_file_values_);
      break;
    }

    case UNC_KT_VBRIF_FLOWFILTER: {
      pfc_log_debug("UNC_VBRIF_FLOW_FILTER key type received");
      driver_cmd_ptr = new OdcVbrIfFlowFilterCmd(conf_file_values_);
      break;
    }

    case UNC_KT_VBRIF_FLOWFILTER_ENTRY: {
      pfc_log_debug("UNC_VBRIF_FLOW_FILTER_ENTRY key type received");
      driver_cmd_ptr = new OdcVbrIfFlowFilterEntryCmd(conf_file_values_);
      break;
    }

    case UNC_KT_VTERMIF_FLOWFILTER: {
      pfc_log_debug("UNC_VTERMIF_FLOW_FILTER key type received");
      driver_cmd_ptr = new OdcVTermIfFlowFilterCmd(conf_file_values_);
      break;
    }

    case UNC_KT_VTERMIF_FLOWFILTER_ENTRY: {
      pfc_log_debug("UNC_VTERMIF_FLOW_FILTER_ENTRY key type received");
      driver_cmd_ptr = new OdcVTermIfFlowFilterEntryCmd(conf_file_values_);
      break;
    }

    default:
      pfc_log_debug("Unknown keytype received : %d", key_type);
      break;
  }
  return driver_cmd_ptr;
}


// Get the Driver Read Command for supported keytypes
unc::driver::vtn_driver_read_command* ODCModule::create_driver_read_command(
    unc_key_type_t key_type) {
  ODC_FUNC_TRACE;
  unc::driver::vtn_driver_read_command* driver_cmd_ptr = NULL;
  switch (key_type) {
    case UNC_KT_DATAFLOW: {
      pfc_log_debug("Read for Dataflow");
      driver_cmd_ptr = new OdcDataFlowCommand(conf_file_values_);
      break;
    }

    case UNC_KT_CTR_DATAFLOW: {
      pfc_log_debug("Read for Dataflow");
      driver_cmd_ptr = new OdcCtrDataFlowCommand(conf_file_values_);
      break;
    }

    case UNC_KT_VTN_DATAFLOW: {
      pfc_log_debug("Read for vtn Dataflow");
      driver_cmd_ptr = new OdcVtnDataFlowCommand(conf_file_values_);
      break;
    }
    case UNC_KT_PORT: {
      pfc_log_info("UNC_KT_PORT type received");
      driver_cmd_ptr = new OdcPort(conf_file_values_);
      break;
    }

    case UNC_KT_VTNSTATION_CONTROLLER: {
      pfc_log_debug("UNC_KT_PORT type received");
      driver_cmd_ptr = new OdcVtnStationCommand(conf_file_values_);
      break;
    }
    default:
      pfc_log_error("READ not supported");
      break;
  }
  return driver_cmd_ptr;
}

// Gets the ping interval
uint32_t ODCModule::get_ping_interval() {
  ODC_FUNC_TRACE;
  return ping_interval;
}

// Gets the ping fail retry count
uint32_t ODCModule::get_ping_fail_retry_count() {
  ODC_FUNC_TRACE;
  return PING_RETRY_COUNT;
}

// Is ping need or not
pfc_bool_t ODCModule::is_ping_needed() {
  ODC_FUNC_TRACE;
  return PFC_TRUE;
}

// Is physicalconfiguration  need or not
pfc_bool_t ODCModule::is_physicalconfig_needed() {
  ODC_FUNC_TRACE;
  return PFC_TRUE;
}

//  ping controller
pfc_bool_t ODCModule::ping_controller(unc::driver::controller* ctr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr != NULL);

  std::string url = "";
  url.append(BASE_URL);
  url.append(VERSION);

  unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                          ctr->get_user_name(), ctr->get_pass_word());
  unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(
          url, restjson::HTTP_METHOD_GET, NULL, conf_file_values_);

  if (NULL == response) {
    pfc_log_error("Error in getting response from controller");
    return PFC_FALSE;
  }
  int resp_code = response->code;
  pfc_log_debug("Response code for ping controller  :%d", resp_code);
  if (HTTP_200_RESP_OK != resp_code) {
    pfc_log_error("Response code is : %d", resp_code);
    return PFC_FALSE;
  }
  pfc_bool_t new_audit_status = ctr->get_audit_status();
  unc::driver::ConnectionStatus connection_status =
      ctr->get_connection_status();
  if ((connection_status) && (new_audit_status)) {
    pfc_log_info("Posting Controller Audit from ODC Driver");
    std::string controller_name = ctr->get_controller_id();
    notify_audit_start_to_tc(controller_name);
  }
  return PFC_TRUE;
}

// Gets the switch and its port details from Controller and notify UPPL
pfc_bool_t ODCModule::get_physical_port_details(
                          unc::driver::controller* ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  OdcSwitch odc_switch_obj(conf_file_values_);
  OdcPort odc_port_obj(conf_file_values_);
  pfc_bool_t cache_empty = PFC_TRUE;

  // If controller is down clear physical port cache
  if (unc::driver::CONNECTION_DOWN == ctr_ptr->get_connection_status()) {
    pfc_log_debug("Controller is Down, Clear Physical port cache");
    if (NULL != ctr_ptr->physical_port_cache) {
       delete ctr_ptr->physical_port_cache;
       ctr_ptr->physical_port_cache = NULL;
    }
    return PFC_FALSE;
  }
  if (NULL == ctr_ptr->physical_port_cache) {
    pfc_log_error("Physical port cache pointer is NULL");
    return PFC_FALSE;
  }
  // Gets the SWITCH details
  UncRespCode ret_val = odc_switch_obj.fetch_config(ctr_ptr, cache_empty);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error occured in getting switch details");
    return PFC_FALSE;
  }

  std::auto_ptr<unc::vtndrvcache::CommonIterator>
      itr_ptr(ctr_ptr->physical_port_cache->create_iterator());
  unc::vtndrvcache::ConfigNode *cfgnode_cache = NULL;
  for (cfgnode_cache = itr_ptr->PhysicalNodeFirstItem();
       itr_ptr->IsDone() == false;
       cfgnode_cache = itr_ptr->NextItem() ) {
    if (NULL == cfgnode_cache) {
      pfc_log_error("cfgnode is NULL before get_type");
      delete ctr_ptr->physical_port_cache;
      ctr_ptr->physical_port_cache = NULL;
      return PFC_FALSE;
    }

    unc_key_type_t key_type =  cfgnode_cache->get_type_name();
    pfc_log_debug("key_type in odc_mod %d", key_type);
    if (UNC_KT_SWITCH == key_type) {
      unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * cache_util_ptr =
          static_cast <unc::vtndrvcache::CacheElementUtil
          <key_switch_t, val_switch_st_t, val_switch_st_t, uint32_t> * > (cfgnode_cache);

      PFC_ASSERT(cache_util_ptr != NULL);
      key_switch_t *key_switch = cache_util_ptr->get_key_structure();
      if (NULL == key_switch) {
        pfc_log_error("key_switch is NULL");
        return PFC_FALSE;
      }
      //  Gets the port details of a particular SW
      ret_val = odc_port_obj.fetch_config(ctr_ptr, key_switch, cache_empty);
      if (UNC_RC_SUCCESS != ret_val) {
        pfc_log_error("Error occured in getting port details");
        return PFC_FALSE;
      }
    }
  }
  std::map<std::string, std::string> link_prop;
  link_prop = odc_port_obj.get_linkmap();
  OdcLink odc_link_obj(conf_file_values_);
  odc_link_obj.set_map(link_prop);
  ret_val = odc_link_obj.fetch_config(ctr_ptr, cache_empty);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error occured in getting link details");
    return PFC_FALSE;
  }

  return PFC_TRUE;
}
ctrl_info_update_type_t ODCModule::compare_ctr_info(unc::driver::controller* ctr_ptr,
                                                    const val_ctr_t& val_ctr) {

  ODC_FUNC_TRACE;
  ctrl_info_update_type_t  change_type = CTRLINFO_OTHER_UPDATE;
  std::string new_ip = (inet_ntoa(val_ctr.ip_address));
  std::string old_ip = ctr_ptr->get_host_address();

  pfc_log_debug("%s:newIP ..%s", PFC_FUNCNAME,new_ip.c_str());
  pfc_log_debug("%s:OdlIP ..%s", PFC_FUNCNAME,old_ip.c_str());
  if(new_ip.compare(DEFAULT_IP) != 0) {
    if(old_ip.compare(DEFAULT_IP) == 0) {
      pfc_log_debug("%s:IP is ADDED..%s", PFC_FUNCNAME,new_ip.c_str());
      change_type = CTRLINFO_IP_ADDED;
    } else if(new_ip.compare(old_ip) != 0) {
      pfc_log_debug("%s:IP is CHANGED..%s", PFC_FUNCNAME,new_ip.c_str());
      change_type = CTRLINFO_IP_CHANGED;
    }
  } else if (old_ip.compare(DEFAULT_IP) != 0) {
    pfc_log_debug("%s:IP is removed..%s", PFC_FUNCNAME,old_ip.c_str());
    change_type = CTRLINFO_IP_REMOVED;
  }
  return change_type;
}

unc::tclib::TcCommonRet ODCModule::HandleVote(unc::driver::controller*) {
  ODC_FUNC_TRACE;
  return unc::tclib::TC_FAILURE;
}

unc::tclib::TcCommonRet ODCModule::HandleCommit(unc::driver::controller*) {
  ODC_FUNC_TRACE;
  return unc::tclib::TC_FAILURE;
}

unc::tclib::TcCommonRet ODCModule::HandleAbort(unc::driver::controller*) {
  ODC_FUNC_TRACE;
  return unc::tclib::TC_FAILURE;
}

void ODCModule::read_conf_file() {
  ODC_FUNC_TRACE;
  pfc::core::ModuleConfBlock drv_block(DRV_CONF_BLK);
  if (drv_block.getBlock() != PFC_CFBLK_INVALID) {
    ping_interval = drv_block.getUint32(CONF_PING_INTERVAL,
                                        PING_DEFAULT_INTERVAL);
    conf_file_values_.odc_port  = drv_block.getUint32(CONF_ODC_PORT,
                                                      DEFAULT_ODC_PORT);
    conf_file_values_.request_time_out = drv_block.getUint32(
        CONF_REQ_TIME_OUT, DEFAULT_REQ_TIME_OUT);

    conf_file_values_.connection_time_out = drv_block.getUint32(
        CONF_CONNECT_TIME_OUT, DEFAULT_CONNECT_TIME_OUT);
    conf_file_values_.user_name = drv_block.getString(
        CONF_USER_NAME, DEFAULT_USER_NAME.c_str());

    conf_file_values_.password  = drv_block.getString(
        CONF_PASSWORD, DEFAULT_PASSWORD.c_str());
    pfc_log_debug("%s: Block Handle is Valid,Ping Timeout %d", PFC_FUNCNAME,
                  ping_interval);
  } else {
    ping_interval = PING_DEFAULT_INTERVAL;
    conf_file_values_.odc_port   =  DEFAULT_ODC_PORT;
    conf_file_values_.connection_time_out = DEFAULT_CONNECT_TIME_OUT;
    conf_file_values_.request_time_out = DEFAULT_REQ_TIME_OUT;
    conf_file_values_.user_name = DEFAULT_USER_NAME;
    conf_file_values_.password  = DEFAULT_PASSWORD;
    pfc_log_debug("%s: Block Handle is Invalid,set default Value %d",
                  PFC_FUNCNAME, ping_interval);
  }
  pfc_log_debug("%s: Exiting function", PFC_FUNCNAME);
}
}  // namespace odcdriver
}  // namespace unc
PFC_MODULE_IPC_DECL(unc::odcdriver::ODCModule, 0);
