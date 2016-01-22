/*
 * Copyright (c) 2013-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vbrif.hh>

namespace unc {
namespace odcdriver {
// Constructor
OdcVbrIfCommand::OdcVbrIfCommand(unc::restjson::ConfFileValues_t conf_values)
: conf_file_values_(conf_values) {
  ODC_FUNC_TRACE;
}

// Destructor
OdcVbrIfCommand::~OdcVbrIfCommand() {
}

odc_drv_resp_code_t OdcVbrIfCommand::validate_logical_port_id(
    const std::string& logical_port_id) {
  ODC_FUNC_TRACE;
  pfc_log_debug("logical port received : %s", logical_port_id.c_str());
  //  First 3 characters should be PP-
  if ((logical_port_id.compare(0, 3, PP_PREFIX) != 0) &&
      (logical_port_id.size() < 28)) {
    pfc_log_error("Logical port id is not with PP- prefix");
    return ODC_DRV_FAILURE;
  }

  //  Split Switch id and prefix
  std::string switch_port = logical_port_id.substr(3);
  size_t hyphen_occurence = switch_port.find("-");
  if (hyphen_occurence == std::string::npos) {
    pfc_log_error("Error in Validating the port");
    return ODC_DRV_FAILURE;
  }
  //  Split switch id alone without port name
  std::string switch_id = switch_port.substr(0, hyphen_occurence);
  size_t base_occurence = switch_id.find(":");
  std::string switch_base = switch_id.substr(0, base_occurence);
  pfc_log_info("switch_base:%s", switch_base.c_str());
  if ((switch_base.compare("openflow"))) {
      pfc_log_error("Invalid switch id format supported by Vtn Manager");
      return ODC_DRV_FAILURE;
      }
  pfc_log_debug("Switch id in port map %s", switch_id.c_str());
  pfc_log_debug("Valid logical_port id");
  return ODC_DRV_SUCCESS;
}

//  Check the format of logical port id. If it is invalid converts to proper
//  format
odc_drv_resp_code_t OdcVbrIfCommand::check_logical_port_id_format(
    std::string& logical_port_id) {
  ODC_FUNC_TRACE;

  // If First SIX digits are in the format PP-OF: change to PP-
  if (logical_port_id.compare(0, 6, PP_OF_PREFIX) == 0) {
     logical_port_id.replace(logical_port_id.find(PP_OF_PREFIX),
                             PP_OF_PREFIX.length(), PP_PREFIX);
  }
  odc_drv_resp_code_t logical_port_retval = validate_logical_port_id(
      logical_port_id);
    if (logical_port_retval != ODC_DRV_SUCCESS) {
      pfc_log_error("Failed during validation of logical port id %s",
                    logical_port_id.c_str());
      return ODC_DRV_FAILURE;
    }
  return logical_port_retval;
}

// Creates Request Body for Port Map
UncRespCode OdcVbrIfCommand::create_request_body_port_map(
                                           pfcdrv_val_vbr_if_t& vbrif_val,
                                           key_vbr_if_t& vbrif_key,
                                           const std::string &logical_port_id,
                                      ip_vbrif_port&  ip_vbrif_port_st) {
  ODC_FUNC_TRACE;
  std::string switch_id = "";
  std::string port_name = "";

 ip_vbrif_port_st.input_vbrifport_.valid = true;
 ip_vbrif_port_st.input_vbrifport_.tenant_name =
              reinterpret_cast<const char*>(vbrif_key.vbr_key.vtn_key.vtn_name);
 if (ip_vbrif_port_st.input_vbrifport_.tenant_name.empty()) {
   pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
   return UNC_DRV_RC_ERR_GENERIC;
 }
 ip_vbrif_port_st.input_vbrifport_.bridge_name =
             reinterpret_cast<const char*>(vbrif_key.vbr_key.vbridge_name);
 if (ip_vbrif_port_st.input_vbrifport_.bridge_name.empty()) {
   pfc_log_error("Empty vbr name in %s", PFC_FUNCNAME);
   return UNC_DRV_RC_ERR_GENERIC;
 }
 ip_vbrif_port_st.input_vbrifport_.interface_name =
                     reinterpret_cast<const char*>(vbrif_key.if_name);
 if (ip_vbrif_port_st.input_vbrifport_.interface_name.empty()) {
   pfc_log_error("Empty interface name in %s", PFC_FUNCNAME);
   return UNC_DRV_RC_ERR_GENERIC;
 }

  pfc_log_debug("VALUE RECEIVED for LOGICAL PORT %u" ,
              vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]);

  pfc_log_debug("logical_port_id is %s", logical_port_id.c_str());
  std::string switch_port = logical_port_id.substr(3);
  size_t hyphen_occurence = switch_port.find("-");
  std::string of_switch_id = switch_port.substr(0, hyphen_occurence);
  port_name = switch_port.substr(hyphen_occurence+1);
  // convert switch id from unsigned decimal to hex string
  int switch_val = atoi(of_switch_id.substr(9).c_str());
  std::stringstream stream;
  stream << std::hex << switch_val;
  switch_id = stream.str();
  switch_id = SWITCH_BASE + switch_id ;
  pfc_log_debug("port name : %s", port_name.c_str());
  pfc_log_debug("switch id : %s", switch_id.c_str());
  //ip_vbrif_port_st.input_vbrifport_.vlan = 0;
  if ((UNC_VF_VALID == vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM])
      &&
      (UNC_VF_VALID == vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM])) {
    if (vbrif_val.val_vbrif.portmap.tagged == UPLL_VLAN_TAGGED) {
      ip_vbrif_port_st.input_vbrifport_.ip_vlan =
                                         vbrif_val.val_vbrif.portmap.vlan_id;
    }
  }
  if (!(switch_id.empty())) {
    ip_vbrif_port_st.input_vbrifport_.node = switch_id;
  }

  if (!(port_name.empty())) {
    ip_vbrif_port_st.input_vbrifport_.port_name = port_name;
  }
  return UNC_DRV_RC_ERR_GENERIC;
}

// Create Command for vbrif
UncRespCode OdcVbrIfCommand::create_cmd(key_vbr_if_t& vbrif_key,
                                            pfcdrv_val_vbr_if_t& vbrif_val,
                                            unc::driver::controller*
                                            ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  char *vtnname = NULL;
  char *vbrname = NULL;
  char *ifname = NULL;
  vtnname = reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
     pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
     return UNC_DRV_RC_ERR_GENERIC;
  }
  vbrname = reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name);
  if (0 == strlen(vbrname)) {
    pfc_log_error("Empty vbr name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ifname = reinterpret_cast<char*>(vbrif_key.if_name);
  if (0 == strlen(ifname)) {
    pfc_log_error("Empty if name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vbrif_class *req_obj = new vbrif_class(ctr_ptr, vtnname, vbrname);
  ip_vbridge_config st_obj;
  create_request_body(vbrif_val, vbrif_key, st_obj);
  vbrif_parser *parser_obj = new vbrif_parser();
  json_object *jobj = parser_obj->create_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
}
  if(req_obj->set_post(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vbr_if Create Failed");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }


  std::string logical_port_id ="";
  if (vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    if (UNC_VF_VALID !=
      vbrif_val.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]) {
      pfc_log_error("portmap - logical port valid flag is not set");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    logical_port_id =
        reinterpret_cast<char*>(vbrif_val.val_vbrif.portmap.logical_port_id);
    odc_drv_resp_code_t logical_port_retval = check_logical_port_id_format(
                                                            logical_port_id);
    if (logical_port_retval != ODC_DRV_SUCCESS) {
      pfc_log_error("logical port id is Invalid");
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  // VBR_IF successful...Check for PortMap
  if (vbrif_val.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
  vbrifport_class *req_ob = new vbrifport_class(ctr_ptr, vtnname, vbrname,
                                                    ifname);
  ip_vbrif_port st_ob;
  create_request_body_port_map(vbrif_val, vbrif_key, logical_port_id, st_ob);
  vbrifport_parser *parser_ob = new vbrifport_parser();
  json_object *job = parser_ob->create_req(st_ob);
  if(job == NULL){
    pfc_log_error("Error in create request");
    delete req_ob;
    delete parser_ob;
    return UNC_DRV_RC_ERR_GENERIC;
}
  if(req_ob->set_put(job) != UNC_RC_SUCCESS) {
    pfc_log_error("Vbr_if_portmap Create Failed");
    delete req_ob;
    delete parser_ob;
    return UNC_DRV_RC_ERR_GENERIC;
 }
}
  delete req_obj;
  delete parser_obj;
	pfc_log_error("vbrif post is success");
  return UNC_RC_SUCCESS;
}

// Update command for vbrif command
UncRespCode OdcVbrIfCommand::update_cmd(key_vbr_if_t& vbrif_key,
                                        pfcdrv_val_vbr_if_t& val_old,
                                        pfcdrv_val_vbr_if_t& val_new,
                                        unc::driver::controller
                                        *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  char *vtnname = NULL;
  char *vbrname = NULL;
  char *ifname = NULL;
  vtnname = reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
     pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
     return UNC_DRV_RC_ERR_GENERIC;
  }
  vbrname = reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name);
  if (0 == strlen(vbrname)) {
    pfc_log_error("Empty vbr name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ifname = reinterpret_cast<char*>(vbrif_key.if_name);
  if (0 == strlen(ifname)) {
    pfc_log_error("Empty if name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vbrif_class *req_obj = new vbrif_class(ctr_ptr, vtnname, vbrname);
  ip_vbridge_config st_obj;
  update_request_body(val_new, vbrif_key, st_obj);
  vbrif_parser *parser_obj = new vbrif_parser();
  json_object *jobj = parser_obj->create_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
}
  if(req_obj->set_put(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vbr_if Update Failed");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }



  std::string logical_port_id ="";
  if (val_new.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    if (UNC_VF_INVALID ==
        val_new.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]) {
      pfc_log_error("portmap - logical port valid flag is not set");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    logical_port_id =
        reinterpret_cast<char*>(val_new.val_vbrif.portmap.logical_port_id);
    odc_drv_resp_code_t logical_port_retval = check_logical_port_id_format(
        logical_port_id);
    if (logical_port_retval != ODC_DRV_SUCCESS) {
      pfc_log_error("logical port id is invalid");
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  if ((val_new.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID)
      && (val_new.valid[PFCDRV_IDX_VAL_VBRIF] == UNC_VF_VALID)) {
    vbrifport_class *req_ob = new vbrifport_class(ctr_ptr, vtnname, vbrname,
                                                      ifname);
    ip_vbrif_port st_ob;
    create_request_body_port_map(val_new, vbrif_key, logical_port_id, st_ob);
    vbrifport_parser *parser_ob = new vbrifport_parser();
    json_object *job = parser_ob->create_req(st_ob);
    if(jobj == NULL){
      pfc_log_error("Error in create request");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    if(req_ob->set_put(job) != UNC_RC_SUCCESS) {
      pfc_log_error("Vbr_if_portmap Update Failed");
      delete req_ob;
      delete parser_ob;
      return UNC_DRV_RC_ERR_GENERIC;
    }
  } else if((val_new.val_vbrif.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE)
           && (val_new.valid[PFCDRV_IDX_VAL_VBRIF] == UNC_VF_VALID)) {
    vbrifport_class *req_ob = new vbrifport_class(ctr_ptr, vtnname, vbrname,
                                                               ifname);
    if(req_ob->set_delete(jobj) != UNC_RC_SUCCESS) {
      pfc_log_error("Vbr_if_portmap Update Failed");
      delete req_ob;
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
}

// Delete Command for vbr if
UncRespCode OdcVbrIfCommand::delete_cmd(key_vbr_if_t& vbrif_key,
                                            pfcdrv_val_vbr_if_t& val,
                                  unc::driver::controller* ctr_ptr) {
  ODC_FUNC_TRACE;
  pfc_log_debug("%s Enter delete_cmd", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  char *vtnname = NULL;
  char *vbrname = NULL;
  vtnname = reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
     pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
     return UNC_DRV_RC_ERR_GENERIC;
  }
  vbrname = reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name);
  if (0 == strlen(vbrname)) {
    pfc_log_error("Empty vbr name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vbrif_class *req_obj = new vbrif_class(ctr_ptr, vtnname, vbrname);
  ip_vbridge_config st_obj;
  delete_request_body(val, vbrif_key, st_obj);
  vbrif_parser *parser_obj = new vbrif_parser();
  json_object *jobj = parser_obj->del_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in delete request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if(req_obj->set_delete(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vbr_if Delete Failed");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
}

UncRespCode OdcVbrIfCommand::create_request_body(pfcdrv_val_vbr_if_t& val_vbrif,
                                          key_vbr_if_t& vbrif_key,
                                      ip_vbridge_config&  ip_vbridge_config_st){
  ODC_FUNC_TRACE;
  ip_vbridge_config_st.input_vbrif_.valid = true;
  ip_vbridge_config_st.input_vbrif_.update_mode =
                                       reinterpret_cast<const char*>("CREATE");
  ip_vbridge_config_st.input_vbrif_.operation =
                                          reinterpret_cast<const char*>("SET");
  ip_vbridge_config_st.input_vbrif_.tenant_name =
              reinterpret_cast<const char*>(vbrif_key.vbr_key.vtn_key.vtn_name);
  if (ip_vbridge_config_st.input_vbrif_.tenant_name.empty()) {
    pfc_log_error("Empty vtn name in %s",PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ip_vbridge_config_st.input_vbrif_.bridge_name =
               reinterpret_cast<const char*>(vbrif_key.vbr_key.vbridge_name);
  if (ip_vbridge_config_st.input_vbrif_.bridge_name.empty()) {
    pfc_log_error("Empty vbr name in %s",PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ip_vbridge_config_st.input_vbrif_.interface_name =
                     reinterpret_cast<const char*>(vbrif_key.if_name);
  if (ip_vbridge_config_st.input_vbrif_.interface_name.empty()) {
    pfc_log_error("empty vinterface name in %s",PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ip_vbridge_config_st.input_vbrif_.input_description =
                 reinterpret_cast<const char*>(val_vbrif.val_vbrif.description);
  pfc_log_info("vbr des : %s",
                 ip_vbridge_config_st.input_vbrif_.input_description.c_str());
  if (UNC_VF_VALID == val_vbrif.val_vbrif.valid[UPLL_IDX_DESC_VBRI]) {
    if (!ip_vbridge_config_st.input_vbrif_.input_description.empty()) {
      ip_vbridge_config_st.input_vbrif_.input_description.assign(
                        ip_vbridge_config_st.input_vbrif_.input_description);
    }
  }
  if (UNC_VF_VALID == val_vbrif.val_vbrif.valid[UPLL_IDX_ADMIN_STATUS_VBRI]) {
    if (UPLL_ADMIN_ENABLE == val_vbrif.val_vbrif.admin_status) {
        ip_vbridge_config_st.input_vbrif_.input_enabled = true;
    } else if (UPLL_ADMIN_DISABLE == val_vbrif.val_vbrif.admin_status) {
        ip_vbridge_config_st.input_vbrif_.input_enabled = false;
      }
    pfc_log_info("vbr enabled : %d",
                         ip_vbridge_config_st.input_vbrif_.input_enabled);
  } else {
     ip_vbridge_config_st.input_vbrif_.input_enabled = true;
     pfc_log_debug("vbrif default admin_Status as true");
   }
  return UNC_RC_SUCCESS;
}

// Update Request body
UncRespCode OdcVbrIfCommand::update_request_body(pfcdrv_val_vbr_if_t& val_vbrif,
                                          key_vbr_if_t& vbrif_key,
                                   ip_vbridge_config&  ip_vbridge_config_st ) {
  ODC_FUNC_TRACE;
  ip_vbridge_config_st.input_vbrif_.valid = true;
  ip_vbridge_config_st.input_vbrif_.update_mode =
                                       reinterpret_cast<const char*>("UPDATE");
  ip_vbridge_config_st.input_vbrif_.operation =
                                          reinterpret_cast<const char*>("ADD");
  ip_vbridge_config_st.input_vbrif_.tenant_name =
              reinterpret_cast<const char*>(vbrif_key.vbr_key.vtn_key.vtn_name);
  if (ip_vbridge_config_st.input_vbrif_.tenant_name.empty()) {
    pfc_log_error("Empty vtn name in %s",PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ip_vbridge_config_st.input_vbrif_.bridge_name =
               reinterpret_cast<const char*>(vbrif_key.vbr_key.vbridge_name);
  if (ip_vbridge_config_st.input_vbrif_.bridge_name.empty()) {
    pfc_log_error("Empty vbr name in %s",PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ip_vbridge_config_st.input_vbrif_.interface_name =
                     reinterpret_cast<const char*>(vbrif_key.if_name);
  if (ip_vbridge_config_st.input_vbrif_.interface_name.empty()) {
    pfc_log_error("Empty vinterface name in %s",PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ip_vbridge_config_st.input_vbrif_.input_description =
                 reinterpret_cast<const char*>(val_vbrif.val_vbrif.description);
  pfc_log_info("vbr des : %s",
                  ip_vbridge_config_st.input_vbrif_.input_description.c_str());
  if (UNC_VF_VALID == val_vbrif.val_vbrif.valid[UPLL_IDX_DESC_VBRI]) {
    if (!ip_vbridge_config_st.input_vbrif_.input_description.empty()) {
      ip_vbridge_config_st.input_vbrif_.input_description.assign(
                        ip_vbridge_config_st.input_vbrif_.input_description);
    }
  }
  if (UNC_VF_VALID == val_vbrif.val_vbrif.valid[UPLL_IDX_ADMIN_STATUS_VBRI]) {
    if (UPLL_ADMIN_ENABLE == val_vbrif.val_vbrif.admin_status) {
        ip_vbridge_config_st.input_vbrif_.input_enabled = true;
    } else if (UPLL_ADMIN_DISABLE == val_vbrif.val_vbrif.admin_status) {
        ip_vbridge_config_st.input_vbrif_.input_enabled = false;
      }
        pfc_log_info("vbr enabled : %d",
                ip_vbridge_config_st.input_vbrif_.input_enabled);
  } else {
     ip_vbridge_config_st.input_vbrif_.input_enabled = true;
     pfc_log_debug("settting vbrif admi_status default value as true");
    }
  return UNC_RC_SUCCESS;
}

//Delete request body
UncRespCode OdcVbrIfCommand::delete_request_body(pfcdrv_val_vbr_if_t& val_vbrif,
                                          key_vbr_if_t& vbrif_key,
                                    ip_vbridge_config&  ip_vbridge_config_st) {
  ODC_FUNC_TRACE;
  ip_vbridge_config_st.input_vbrif_.valid = true;
  ip_vbridge_config_st.input_vbrif_.tenant_name =
              reinterpret_cast<const char*>(vbrif_key.vbr_key.vtn_key.vtn_name);
  if (ip_vbridge_config_st.input_vbrif_.tenant_name.empty()) {
    pfc_log_error("Empty vtn name in %s",PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ip_vbridge_config_st.input_vbrif_.bridge_name =
               reinterpret_cast<const char*>(vbrif_key.vbr_key.vbridge_name);
  if (ip_vbridge_config_st.input_vbrif_.bridge_name.empty()) {
    pfc_log_error("Empty vbr name in %s",PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ip_vbridge_config_st.input_vbrif_.interface_name =
                     reinterpret_cast<const char*>(vbrif_key.if_name);
  if (ip_vbridge_config_st.input_vbrif_.interface_name.empty()) {
    pfc_log_error("Empty vinterface name in %s",PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}

//  fetch child configurations for the parent kt(vbr)
UncRespCode OdcVbrIfCommand::fetch_config(unc::driver::controller* ctr_ptr,
                                     void* parent_key,
                                     std::vector<unc::vtndrvcache::ConfigNode *>
                                     &cfgnode_vector) {
  ODC_FUNC_TRACE;
  pfc_log_debug("fetch  OdcVbrIfCommand config");

  key_vbr_t* parent_vbr = reinterpret_cast<key_vbr_t*> (parent_key);
  std::string parent_vtn_name =
      reinterpret_cast<const char*> (parent_vbr->vtn_key.vtn_name);
  std::string parent_vbr_name =
      reinterpret_cast<const char*> (parent_vbr->vbridge_name);

  pfc_log_debug("parent_vtn_name:%s, parent_vbr_name:%s",
                parent_vtn_name.c_str(), parent_vbr_name.c_str());

  vbrif_class *req_obj = new vbrif_class(ctr_ptr, parent_vtn_name,
                                                      parent_vbr_name);
  std::string url = req_obj->get_url();
  pfc_log_info("URL:%s",url.c_str());
  vbrif_parser *parser_obj = new vbrif_parser();
  UncRespCode ret_val = req_obj->get_response(parser_obj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("VBRIF Get response error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = parser_obj->set_vbr_if_conf(parser_obj->jobj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("set_vbridge-interface_conf error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = parse_vbrif_response(parent_vtn_name, parent_vbr_name, ctr_ptr,
                                 parser_obj->vbr_if_conf_, cfgnode_vector);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing");
    delete req_obj;
    delete parser_obj;
    return ret_val;
  }
  return ret_val;

}
UncRespCode OdcVbrIfCommand::parse_vbrif_response(std::string vtn_name,
                                     std::string vbr_name,
                                  unc::driver::controller* ctr_ptr,
                                std::list<vbr_if_conf> &vbrif_detail,
            std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;

  std::list<vbr_if_conf>::iterator it;
  for (it = vbrif_detail.begin(); it != vbrif_detail.end(); it++) {
    std::string if_name = it->name;
    std::string description = it->vbr_vinterface_config_.description;
    bool admin_status = it->vbr_vinterface_config_.enabled;
    UncRespCode ret_val = fill_config_node_vector(vtn_name,vbr_name,
                      if_name,description,admin_status,ctr_ptr,cfgnode_vector);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Error return from fill config failure");
      return ret_val;
    }
  }
  return UNC_RC_SUCCESS;
}

// Reading port-map from active controller
UncRespCode  OdcVbrIfCommand::read_portmap(unc::driver::controller* ctr_ptr,
                                               key_vbr_if_t& vbrif_key,
                                 std::list<portmap_interface>  &vbrif_port_detail,
                                    std::vector<unc::vtndrvcache::ConfigNode *>
                                          &cfgnode_vector) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  char *vtnname = NULL;
  char *vbrname = NULL;
  char *ifname = NULL;
  vtnname = reinterpret_cast<char*>(vbrif_key.vbr_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
     pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
     return UNC_DRV_RC_ERR_GENERIC;
  }
  vbrname = reinterpret_cast<char*>(vbrif_key.vbr_key.vbridge_name);
  if (0 == strlen(vbrname)) {
    pfc_log_error("Empty vbr name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ifname = reinterpret_cast<char*>(vbrif_key.if_name);
  if (0 == strlen(ifname)) {
    pfc_log_error("Empty if name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vbrifport_class *req_obj = new vbrifport_class(ctr_ptr, vtnname,
                                              vbrname, ifname);
  std::string url = req_obj->get_url();
  pfc_log_info("URL:%s",url.c_str());
  vbrifport_parser *parser_obj = new vbrifport_parser();
  UncRespCode ret_val = req_obj->get_response(parser_obj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Get response error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = parser_obj->set_portmap_interface(parser_obj->jobj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("set vinterface portmap_conf error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vbrif_port_detail = parser_obj->portmap_interface_;
  return ret_val;

}

UncRespCode OdcVbrIfCommand::fill_config_node_vector(std::string vtn_name,
                                  std::string vbr_name, std::string if_name,
       std::string description, bool admin_status,unc::driver::controller* ctr,
                std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_vbr_if_t key_vbr_if;
  pfcdrv_val_vbr_if_t val_vbr_if;
  memset(&key_vbr_if, 0, sizeof(key_vbr_if_t));
  memset(&val_vbr_if, 0, sizeof(pfcdrv_val_vbr_if_t));
  //  Fills the vbrif KEY structure
  strncpy(reinterpret_cast<char*> (key_vbr_if.vbr_key.vtn_key.vtn_name),
          vtn_name.c_str(), sizeof(key_vbr_if.vbr_key.vtn_key.vtn_name) - 1);
  pfc_log_debug(" vtn name in vbrif:%s", reinterpret_cast<char*>
                (key_vbr_if.vbr_key.vtn_key.vtn_name));
  strncpy(reinterpret_cast<char*> (key_vbr_if.vbr_key.vbridge_name),
          vbr_name.c_str(), sizeof(key_vbr_if.vbr_key.vbridge_name) - 1);
  pfc_log_error(" vbr name in vbrif:%s",
                reinterpret_cast<char*> (key_vbr_if.vbr_key.vbridge_name));

  strncpy(reinterpret_cast<char*> (key_vbr_if.if_name), if_name.c_str(),
          sizeof(key_vbr_if.if_name) - 1);
  pfc_log_debug(" vbrif name:%s",
                reinterpret_cast<char*> (key_vbr_if.if_name));

  //  Fills vbrif VAL structure
  val_vbr_if.valid[PFCDRV_IDX_VAL_VBRIF] = UNC_VF_VALID;
  val_vbr_if.valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_INVALID;
  val_vbr_if.valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] = UNC_VF_INVALID;
  val_vbr_if.valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_INVALID;

  if (0 == strlen(description.c_str())) {
    val_vbr_if.val_vbrif.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
  } else {
    val_vbr_if.val_vbrif.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_VALID;
    strncpy(reinterpret_cast<char*> (val_vbr_if.val_vbrif.description),
            description.c_str(), sizeof(val_vbr_if.val_vbrif.description) - 1);
  }
  val_vbr_if.val_vbrif.valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
  if (admin_status == true ) {
    val_vbr_if.val_vbrif.admin_status = UPLL_ADMIN_ENABLE;
  } else if (admin_status == false ) {
    val_vbr_if.val_vbrif.admin_status = UPLL_ADMIN_DISABLE;
  }

      std::list<portmap_interface> vbrif_port_detail;
      UncRespCode ret_val = read_portmap(ctr,key_vbr_if,
                                    vbrif_port_detail, cfgnode_vector);
      if (ret_val != UNC_RC_SUCCESS){
        return ret_val;
      }
       uint32_t vlan = 0;
       std::string node = "";
       std::string port_name = "";
       std::list<portmap_interface>::iterator it;
       for (it = vbrif_port_detail.begin();it != vbrif_port_detail.end();it++){
         vlan = it->portmap_config_.vlan;
      pfc_log_debug("the obtained vlan:%d", it->portmap_config_.vlan);
         node = it->portmap_config_.node_id;
      pfc_log_debug("the obtained node id:%s", (it->portmap_config_.node_id).c_str());
         port_name = it->portmap_config_.port;
      pfc_log_debug("the obtained port_name:%s", port_name.c_str());
       }

      val_vbr_if.val_vbrif.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
      pfc_log_debug("vlan id in portmap read %d", vlan);
      if (0 == vlan) {
        val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] =
                                                   UNC_VF_INVALID;
        pfc_log_debug("untagged");
        val_vbr_if.val_vbrif.portmap.tagged = UPLL_VLAN_UNTAGGED;
        val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      } else {
        pfc_log_debug("vlan id valid");
        val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
        val_vbr_if.val_vbrif.portmap.vlan_id = vlan;
        pfc_log_debug("%d  vlan id ", vlan);
        pfc_log_debug("vlan id tagged");
        val_vbr_if.val_vbrif.portmap.tagged = UPLL_VLAN_TAGGED;
        val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      }

      std::string logical_port = "PP-OF:";
      unc::odcdriver::OdcController *odc_ctr =
                 reinterpret_cast<unc::odcdriver::OdcController *>(ctr);
      PFC_ASSERT(odc_ctr != NULL);
      std::string switch_val = odc_ctr->frame_openflow_switchid(node);
      if (switch_val.empty()) {
        pfc_log_error("%s:switch id empty", PFC_FUNCNAME);
        return UNC_DRV_RC_ERR_GENERIC;
      }
      pfc_log_debug("the obtained node id:%s", node.c_str());
      logical_port.append(node);
      logical_port.append(HYPHEN);
      logical_port.append(port_name);
      pfc_log_debug("logical port id %s", logical_port.c_str());
      strncpy(reinterpret_cast<char*>
              (val_vbr_if.val_vbrif.portmap.logical_port_id),
              logical_port.c_str(),
              sizeof(val_vbr_if.val_vbrif.portmap.logical_port_id) - 1);
      pfc_log_debug("%s logical port id in readportmap " ,
                    reinterpret_cast<char*>
                    (val_vbr_if.val_vbrif.portmap.logical_port_id));
      val_vbr_if.val_vbrif.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
          UNC_VF_VALID;


  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vbr_if_t,
                                             pfcdrv_val_vbr_if_t,
                                              pfcdrv_val_vbr_if_t,
                                             uint32_t> (&key_vbr_if,
                                             &val_vbr_if, &val_vbr_if,
                                             uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  return UNC_RC_SUCCESS;
}

}  // namespace odcdriver
}  // namespace unc

