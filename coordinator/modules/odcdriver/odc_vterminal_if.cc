/*
 * Copyright (c) 2014-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vterminal_if.hh>

namespace unc {
namespace odcdriver {
// Constructor
OdcVtermIfCommand::OdcVtermIfCommand
    (unc::restjson::ConfFileValues_t conf_values)
    : conf_file_values_(conf_values) {
      ODC_FUNC_TRACE;
    }

// Destructor
OdcVtermIfCommand::~OdcVtermIfCommand() {
}

odc_drv_resp_code_t OdcVtermIfCommand::validate_logical_port_id(
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
odc_drv_resp_code_t OdcVtermIfCommand::check_logical_port_id_format(
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
void OdcVtermIfCommand::create_request_body_port_map(
                                              val_vterm_if_t& vterm_if_val,
                                              key_vterm_if_t& vterm_if_key,
                                          const std::string &logical_port_id,
                                         ip_vtermif_port&  ip_vtermif_port_st){

  ODC_FUNC_TRACE;
  ip_vtermif_port_st.input_vtermif_port_.valid = true;
  ip_vtermif_port_st.input_vtermif_port_.tenant_name =
        reinterpret_cast<const char*>(vterm_if_key.vterm_key.vtn_key.vtn_name);
  ip_vtermif_port_st.input_vtermif_port_.terminal_name =
          reinterpret_cast<const char*>(vterm_if_key.vterm_key.vterminal_name);
  ip_vtermif_port_st.input_vtermif_port_.interface_name =
                  reinterpret_cast<const char*>(vterm_if_key.if_name);
  std::string switch_id = "";
  std::string port_name = "";
  pfc_log_debug("VALUE RECEIVED for LOGICAL PORT %u" ,
                vterm_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]);

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
  switch_id = SWITCH_BASE + switch_id;
  pfc_log_debug("port name : %s", port_name.c_str());
  pfc_log_debug("switch_id : %s", switch_id.c_str());
  if ((UNC_VF_VALID == vterm_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM])
      &&
      (UNC_VF_VALID == vterm_if_val.portmap.valid[UPLL_IDX_TAGGED_PM])) {
    if (vterm_if_val.portmap.tagged == UPLL_VLAN_TAGGED) {
      ip_vtermif_port_st.input_vtermif_port_.vlan=vterm_if_val.portmap.vlan_id;
    }
  }
  if (!(switch_id.empty())) {
    ip_vtermif_port_st.input_vtermif_port_.node  = switch_id;
    }
  if (!(port_name.empty())) {
    ip_vtermif_port_st.input_vtermif_port_.port_name_ip = port_name;
    }
}

// delete request Body for Port Map
void OdcVtermIfCommand::delete_request_body_port_map(
                                              val_vterm_if_t& vterm_if_val,
                                              key_vterm_if_t& vterm_if_key,
                                         ip_vtermif_port&  ip_vtermif_port_st){

  ODC_FUNC_TRACE;
  ip_vtermif_port_st.input_vtermif_port_.valid = true;
  ip_vtermif_port_st.input_vtermif_port_.tenant_name =
        reinterpret_cast<const char*>(vterm_if_key.vterm_key.vtn_key.vtn_name);
  ip_vtermif_port_st.input_vtermif_port_.terminal_name =
          reinterpret_cast<const char*>(vterm_if_key.vterm_key.vterminal_name);
  ip_vtermif_port_st.input_vtermif_port_.interface_name =
                  reinterpret_cast<const char*>(vterm_if_key.if_name);
}

void OdcVtermIfCommand::create_request_body(val_vterm_if_t& val_vterm_if,
                                            key_vterm_if_t& vterm_if_key,
                                ip_vterminal_config&  ip_vterminal_config_st) {
  ODC_FUNC_TRACE;
  ip_vterminal_config_st.input_vtermif_.valid = true;
  ip_vterminal_config_st.input_vtermif_.update_mode =
                                    reinterpret_cast<const char*>("CREATE");
  ip_vterminal_config_st.input_vtermif_.operation =
                                    reinterpret_cast<const char*>("SET");
  ip_vterminal_config_st.input_vtermif_.tenant_name =
        reinterpret_cast<const char*>(vterm_if_key.vterm_key.vtn_key.vtn_name);
  ip_vterminal_config_st.input_vtermif_.terminal_name =
          reinterpret_cast<const char*>(vterm_if_key.vterm_key.vterminal_name);
  ip_vterminal_config_st.input_vtermif_.interface_name =
                  reinterpret_cast<const char*>(vterm_if_key.if_name);
  ip_vterminal_config_st.input_vtermif_.vtermif_description =
                        reinterpret_cast<const char*>(val_vterm_if.description);
  pfc_log_info("vterm des : %s",
             ip_vterminal_config_st.input_vtermif_.vtermif_description.c_str());
  if (UNC_VF_VALID == val_vterm_if.valid[UPLL_IDX_DESC_VTERMI]) {
    if (!ip_vterminal_config_st.input_vtermif_.vtermif_description.empty()) {
       ip_vterminal_config_st.input_vtermif_.vtermif_description.assign(
                   ip_vterminal_config_st.input_vtermif_.vtermif_description);
   }
 }
 bool admin_status = 0;
 if (UNC_VF_VALID == val_vterm_if.valid[UPLL_IDX_ADMIN_STATUS_VTERMI]) {
    if (UPLL_ADMIN_ENABLE == val_vterm_if.admin_status) {
      if(admin_status != 0){
        bool enabled = true;
        ip_vterminal_config_st.input_vtermif_.vtermif_enabled = enabled;
        pfc_log_info("vterm  enabled : %d", enabled);
      }
    } else if (UPLL_ADMIN_DISABLE == val_vterm_if.admin_status) {
      if(admin_status == 0){
        bool enabled = false;
        ip_vterminal_config_st.input_vtermif_.vtermif_enabled = enabled;
        pfc_log_info("vterm enabled : %d", enabled);
    }
  }
 }
}

void OdcVtermIfCommand::update_request_body(val_vterm_if_t& val_vterm_if,
                                            key_vterm_if_t& vterm_if_key,
                                ip_vterminal_config&  ip_vterminal_config_st) {
  ODC_FUNC_TRACE;
  ip_vterminal_config_st.input_vtermif_.valid = true;
  ip_vterminal_config_st.input_vtermif_.update_mode =
                                    reinterpret_cast<const char*>("UPDATE");
  ip_vterminal_config_st.input_vtermif_.operation =
                                    reinterpret_cast<const char*>("ADD");
  ip_vterminal_config_st.input_vtermif_.tenant_name =
        reinterpret_cast<const char*>(vterm_if_key.vterm_key.vtn_key.vtn_name);
  ip_vterminal_config_st.input_vtermif_.terminal_name =
          reinterpret_cast<const char*>(vterm_if_key.vterm_key.vterminal_name);
  ip_vterminal_config_st.input_vtermif_.interface_name =
                  reinterpret_cast<const char*>(vterm_if_key.if_name);
  ip_vterminal_config_st.input_vtermif_.vtermif_description =
                        reinterpret_cast<const char*>(val_vterm_if.description);
  pfc_log_info("vterm des : %s",
            ip_vterminal_config_st.input_vtermif_.vtermif_description.c_str());
  if (UNC_VF_VALID == val_vterm_if.valid[UPLL_IDX_DESC_VTERMI]) {
    if (!ip_vterminal_config_st.input_vtermif_.vtermif_description.empty()) {
      ip_vterminal_config_st.input_vtermif_.vtermif_description.assign(
                   ip_vterminal_config_st.input_vtermif_.vtermif_description);
   }
 }
 bool admin_status = 0;
 if (UNC_VF_VALID == val_vterm_if.valid[UPLL_IDX_ADMIN_STATUS_VTERMI]) {
    if (UPLL_ADMIN_ENABLE == val_vterm_if.admin_status) {
      if(admin_status != 0){
        bool enabled = true;
        ip_vterminal_config_st.input_vtermif_.vtermif_enabled = enabled;
        pfc_log_info("vterm  enabled : %d", enabled);
      }
    } else if (UPLL_ADMIN_DISABLE == val_vterm_if.admin_status) {
      if(admin_status == 0){
        bool enabled = false;
        ip_vterminal_config_st.input_vtermif_.vtermif_enabled = enabled;
        pfc_log_info("vterm enabled : %d", enabled);
    }
  }
 }
}

void OdcVtermIfCommand::delete_request_body(val_vterm_if_t& val_vterm_if,
                                            key_vterm_if_t& vterm_if_key,
                             ip_vterminal_config&  ip_vterminal_config_st) {
  ODC_FUNC_TRACE;
  ip_vterminal_config_st.input_vtermif_.valid = true;
  ip_vterminal_config_st.input_vtermif_.tenant_name =
       reinterpret_cast<const char*>(vterm_if_key.vterm_key.vtn_key.vtn_name);
  ip_vterminal_config_st.input_vtermif_.terminal_name =
         reinterpret_cast<const char*>(vterm_if_key.vterm_key.vterminal_name);
  ip_vterminal_config_st.input_vtermif_.interface_name =
                  reinterpret_cast<const char*>(vterm_if_key.if_name);
}


// Create Command for vterm_if
UncRespCode OdcVtermIfCommand::create_cmd(key_vterm_if_t& vterm_if_key,
                                        val_vterm_if_t& vterm_if_val,
                                        unc::driver::controller*
                                        ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  char *vtnname = NULL;
  char *vterminalname = NULL;
  char *intfname = NULL;
  vtnname = reinterpret_cast<char*>(vterm_if_key.vterm_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("vtn name is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vterminalname = reinterpret_cast<char*>(vterm_if_key.vterm_key.vterminal_name);
  if(0 == strlen(vterminalname)) {
    pfc_log_error("vterm name is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  intfname = reinterpret_cast<char*>(vterm_if_key.if_name);
  if(0 == strlen(intfname)) {
    pfc_log_error("vterm if is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string logical_port_id = "";
  if (vterm_if_val.valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) {
    if (UNC_VF_INVALID ==
        vterm_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]) {
      pfc_log_error("portmap - logical port valid flag is not set");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    logical_port_id =
        reinterpret_cast<char*>(vterm_if_val.portmap.logical_port_id);
    odc_drv_resp_code_t logical_port_retval = check_logical_port_id_format(
        logical_port_id);
    if (logical_port_retval != ODC_DRV_SUCCESS) {
      pfc_log_error("logical port id is Invalid");
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  vtermif_class *req_obj = new vtermif_class(ctr_ptr,vtnname,vterminalname);
  ip_vterminal_config  st_obj;
  create_request_body(vterm_if_val, vterm_if_key, st_obj);
  vtermif_odl *parser_obj = new vtermif_odl();
  json_object *jobj = parser_obj->create_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
}
  if(req_obj->set_post(jobj) != UNC_RC_SUCCESS){
     pfc_log_error("Vtermif Create Failed");
     delete req_obj;
     delete parser_obj;
     return UNC_DRV_RC_ERR_GENERIC;
    }
  if (vterm_if_val.valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) {
    vtermifport_class *req_ob = new vtermifport_class(
                                   ctr_ptr,vtnname,vterminalname,intfname);
    ip_vtermif_port  st_ob;
    create_request_body_port_map(vterm_if_val,vterm_if_key,
                                                   logical_port_id, st_ob);
    vtermif_portmap  *parser_ob = new  vtermif_portmap();
    json_object *job = parser_ob->create_req(st_ob);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
    }
  if(req_ob->set_put(job) != UNC_RC_SUCCESS) {
    pfc_log_error("Vtermifport Create Failed");
    delete req_ob;
    delete parser_ob;
    return UNC_DRV_RC_ERR_GENERIC;
 }
}

  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
}


// Update Cmd for Vterm If
UncRespCode OdcVtermIfCommand::update_cmd(key_vterm_if_t& vterm_if_key,
                       val_vterm_if_t& vterm_if_val_old,
                       val_vterm_if_t& vterm_if_val_new,
                       unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  char *vtnname = NULL;
  char *vterminalname = NULL;
  char *intfname = NULL;
  vtnname = reinterpret_cast<char*>(vterm_if_key.vterm_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("vtn name is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vterminalname = reinterpret_cast<char*>(
                                   vterm_if_key.vterm_key.vterminal_name);
  if(0 == strlen(vterminalname)) {
    pfc_log_error("vterm name is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  intfname = reinterpret_cast<char*>(vterm_if_key.if_name);
  if(0 == strlen(intfname)) {
    pfc_log_error("vterm if is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
 std::string logical_port_id = "";
  if (vterm_if_val_new.valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) {
    if (UNC_VF_INVALID ==
        vterm_if_val_new.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]) {
      pfc_log_error("portmap - logical port valid flag is not set");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    logical_port_id =
        reinterpret_cast<char*>(vterm_if_val_new.portmap.logical_port_id);
    odc_drv_resp_code_t logical_port_retval = check_logical_port_id_format(
        logical_port_id);
    if (logical_port_retval != ODC_DRV_SUCCESS) {
      pfc_log_error("logical port id is Invalid");
      return UNC_DRV_RC_ERR_GENERIC;
    }
}
  vtermif_class *req_obj = new vtermif_class(ctr_ptr,vtnname,vterminalname);
  ip_vterminal_config  st_obj;
  update_request_body(vterm_if_val_new, vterm_if_key, st_obj);
  vtermif_odl *parser_obj = new vtermif_odl();
  json_object *jobj = parser_obj->create_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if(req_obj->set_put(jobj) != UNC_RC_SUCCESS){
     pfc_log_error("vtermif Update Failed");
     delete req_obj;
     delete parser_obj;
     return UNC_DRV_RC_ERR_GENERIC;
  }
  if (vterm_if_val_new.valid[UPLL_IDX_PM_VTERMI] == UNC_VF_VALID) {
    vtermifport_class *req_ob = new vtermifport_class(
                                     ctr_ptr,vtnname,vterminalname,intfname);
    ip_vtermif_port  st_ob;
    create_request_body_port_map(vterm_if_val_new,vterm_if_key,
                                                     logical_port_id, st_ob);
    vtermif_portmap  *parser_ob = new  vtermif_portmap();
    json_object *job = parser_ob->create_req(st_ob);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
   }
   if(req_ob->set_put(job) != UNC_RC_SUCCESS) {
     pfc_log_error("Vtermifport Update Failed");
     delete req_ob;
     delete parser_ob;
     return UNC_DRV_RC_ERR_GENERIC;
   }
  } else if ((vterm_if_val_new.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE)
          && (vterm_if_val_new.valid[PFCDRV_IDX_VAL_VBRIF] == UNC_VF_VALID)){
    vtermifport_class *req_ob = new vtermifport_class(ctr_ptr,
                                                vtnname, vterminalname, intfname);
    ip_vtermif_port  st_ob;
    delete_request_body_port_map(vterm_if_val_new,vterm_if_key,
                                                     st_ob);
    vtermif_portmap  *parser_ob = new  vtermif_portmap();
    json_object *job = parser_ob->del_req(st_ob);
    if(jobj == NULL){
      pfc_log_error("Error in create request");
      delete req_ob;
      delete parser_ob;
      return UNC_DRV_RC_ERR_GENERIC;
    }
    if(req_ob->set_delete(job) != UNC_RC_SUCCESS) {
      pfc_log_error("vtermifportmap delete Failed");
      delete req_ob;
      delete parser_ob;
      return UNC_DRV_RC_ERR_GENERIC;
    }
   }
     delete req_obj;
     delete parser_obj;
     return UNC_RC_SUCCESS;
}

UncRespCode OdcVtermIfCommand::delete_cmd(key_vterm_if_t& vterm_if_key,
                       val_vterm_if_t& vterm_if_val,
                       unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  pfc_log_debug("%s Enter delete_cmd", PFC_FUNCNAME);
  PFC_ASSERT(ctr_ptr != NULL);
  char *vtnname = NULL;
  char *vterminalname = NULL;
  char *intfname = NULL;
  vtnname = reinterpret_cast<char*>(vterm_if_key.vterm_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("vtn name is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vterminalname = reinterpret_cast<char*>(
                                      vterm_if_key.vterm_key.vterminal_name);
  if(0 == strlen(vterminalname)) {
    pfc_log_error("vterm name is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  intfname = reinterpret_cast<char*>(vterm_if_key.if_name);
  if(0 == strlen(intfname)) {
    pfc_log_error("vterm if is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vtermif_class *req_obj = new vtermif_class(ctr_ptr, vtnname, vterminalname);
  ip_vterminal_config  st_obj;
  delete_request_body(vterm_if_val, vterm_if_key, st_obj);
  vtermif_odl *parser_obj = new vtermif_odl();
  json_object *jobj = parser_obj->del_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in delete request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if(req_obj->set_delete(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("vtermif delete Failed");
    delete req_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  delete req_obj;
  return UNC_RC_SUCCESS;
}

UncRespCode OdcVtermIfCommand::fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  pfc_log_debug("fetch  OdcVbrIfCommand config");

  key_vterm_t* parent_vterminal = reinterpret_cast<key_vterm_t*> (parent_key);
  std::string parent_vtn_name =
      reinterpret_cast<const char*> (parent_vterminal->vtn_key.vtn_name);
  std::string parent_vterminal_name =
      reinterpret_cast<const char*> (parent_vterminal->vterminal_name);

  pfc_log_debug("parent_vtn_name:%s, parent_vterminal_name:%s",
                parent_vtn_name.c_str(), parent_vterminal_name.c_str());

  vtermif_class *req_obj = new vtermif_class(ctr_ptr, parent_vtn_name,
                                                parent_vterminal_name);
  vtermif_odl *parse_obj = new vtermif_odl();
  UncRespCode ret_val = req_obj->get_response(parse_obj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("get_response error");
    delete req_obj;
    delete parse_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ret_val = parse_obj->set_vterm_if_conf(parse_obj->jobj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing set_vterm_if_conf");
    delete req_obj;
    delete parse_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ret_val = parse_vterm_if_response(parse_obj->vterm_if_conf_, parent_vtn_name,
                           parent_vterminal_name, ctr_ptr, cfgnode_vector);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing");
    delete req_obj;
    delete parse_obj;
    return ret_val;
  }
  return ret_val;
}

UncRespCode OdcVtermIfCommand::parse_vterm_if_response(
    std::list<vterm_if_conf> &vtermif_detail,
    std::string vtn_name,
    std::string vterminal_name,
    unc::driver::controller* ctr_ptr,
    std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  std::list<vterm_if_conf>::iterator it;
  for(it=vtermif_detail.begin();it!=vtermif_detail.end();it++) {
    std::string name = it->name;
    bool enabled = it->vterm_vinterface_config_.enabled;
    std::string description = it->vterm_vinterface_config_.description;
    pfc_log_debug("Vtermif Name: %s",name.c_str());
    pfc_log_debug("Vtermif enabled: %d",enabled);
    pfc_log_debug("Vtermif description:%s",description.c_str());
    UncRespCode ret_val = fill_config_node_vector(ctr_ptr, name, vtn_name,
                          vterminal_name, enabled, description, cfgnode_vector);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Error return from fill config failure");
      return ret_val;
    }
  }
  return UNC_RC_SUCCESS;
}

UncRespCode OdcVtermIfCommand::fill_config_node_vector(
    unc::driver::controller* ctr,
    std::string name,
    std::string vtn_name,
    std::string vterminal_name,
    bool enabled,
    std::string description,
    std::vector< unc::vtndrvcache::ConfigNode *>&cfgnode_vector) {

  ODC_FUNC_TRACE;
  key_vterm_if_t key_vterm_if;
  val_vterm_if_t val_vterm_if;
  memset(&key_vterm_if, 0, sizeof(key_vterm_if_t));
  memset(&val_vterm_if, 0, sizeof(val_vterm_if_t));

  //  Fills the vtermif KEY structure
  strncpy(reinterpret_cast<char*> (key_vterm_if.vterm_key.vtn_key.vtn_name),
       vtn_name.c_str(), sizeof(key_vterm_if.vterm_key.vtn_key.vtn_name) - 1);
  pfc_log_debug(" vtn name in vtermif:%s", reinterpret_cast<char*>
                (key_vterm_if.vterm_key.vtn_key.vtn_name));
  strncpy(reinterpret_cast<char*> (key_vterm_if.vterm_key.vterminal_name),
    vterminal_name.c_str(), sizeof(key_vterm_if.vterm_key.vterminal_name) - 1);
  pfc_log_error(" vterminal name in vtermif:%s",
            reinterpret_cast<char*> (key_vterm_if.vterm_key.vterminal_name));

  strncpy(reinterpret_cast<char*> (key_vterm_if.if_name), name.c_str(),
          sizeof(key_vterm_if.if_name) - 1);
  pfc_log_debug(" vtermif name:%s",
           reinterpret_cast<char*> (key_vterm_if.if_name));

  if (0 == strlen(description.c_str())) {
    val_vterm_if.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_INVALID;
  } else {
    val_vterm_if.valid[UPLL_IDX_DESC_VTERMI] = UNC_VF_VALID;
    strncpy(reinterpret_cast<char*> (val_vterm_if.description),
            description.c_str(), sizeof(val_vterm_if.description) - 1);
  }
  val_vterm_if.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
   if (enabled == true) {
     val_vterm_if.admin_status = UPLL_ADMIN_ENABLE;
  } else if (enabled == false) {
      val_vterm_if.admin_status = UPLL_ADMIN_DISABLE;
  }
     std::list<vterm_if_portmap> vtermif_port_detail;
     UncRespCode ret_val = read_portmap(ctr,key_vterm_if,
                                vtermif_port_detail, cfgnode_vector);
     if (ret_val != UNC_RC_SUCCESS){
       return ret_val;
     }
     uint32_t vlan = 0;
     std::string node = "";
     std::string port_name = "";
     std::list<vterm_if_portmap>::iterator it;
     for (it = vtermif_port_detail.begin();it!= vtermif_port_detail.end();it++){
       vlan = it->vterm_if_.vlan_id;
       node = it->vterm_if_.node_id;
       port_name = it->vterm_if_.port;
     }
      pfc_log_debug("the parsed node id:%s", node.c_str());
      val_vterm_if.valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
      pfc_log_debug("vlan id in portmap read %d", vlan);
      if (0 == vlan) {
        val_vterm_if.portmap.valid[UPLL_IDX_VLAN_ID_PM] =
            UNC_VF_INVALID;
        pfc_log_debug("untagged");
        val_vterm_if.portmap.tagged = UPLL_VLAN_UNTAGGED;
        val_vterm_if.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      } else {
         pfc_log_debug("vlan id valid");
         val_vterm_if.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
         val_vterm_if.portmap.vlan_id = vlan;
         pfc_log_debug("%d  vlan id ", vlan);
         pfc_log_debug("vlan id tagged");
         val_vterm_if.portmap.tagged = UPLL_VLAN_TAGGED;
         val_vterm_if.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      }
      std::string logical_port = "PP-OF:";
      pfc_log_debug("the obtained node id:%s", node.c_str());
      logical_port.append(node);
      logical_port.append(HYPHEN);
      logical_port.append(port_name);
      pfc_log_debug("logical port id %s", logical_port.c_str());
      strncpy(reinterpret_cast<char*>
              (val_vterm_if.portmap.logical_port_id),
              logical_port.c_str(),
              sizeof(val_vterm_if.portmap.logical_port_id) - 1);
      pfc_log_debug("%s logical port id in readportmap " ,
                    reinterpret_cast<char*>
                    (val_vterm_if.portmap.logical_port_id));
      val_vterm_if.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
          UNC_VF_VALID;

  unc::vtndrvcache::ConfigNode *cfgptr =
          new unc::vtndrvcache::CacheElementUtil<key_vterm_if_t,
                                                 val_vterm_if_t,
                                                 val_vterm_if_t,
                                                 uint32_t> (&key_vterm_if,
                                                 &val_vterm_if, &val_vterm_if,
                                                 uint32_t(UNC_OP_READ));
      PFC_ASSERT(cfgptr != NULL);
      cfgnode_vector.push_back(cfgptr);
      return UNC_RC_SUCCESS;
}

UncRespCode OdcVtermIfCommand::read_portmap(unc::driver::controller* ctr_ptr,
                                          key_vterm_if_t& vterm_if_key,
                                  std::list<vterm_if_portmap> &vtermif_port_detail,
                                  std::vector<unc::vtndrvcache::ConfigNode *>
                                             &cfgnode_vector) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  char *vtnname = NULL;
  char *vterminalname = NULL;
  char *intfname = NULL;
  vtnname = reinterpret_cast<char*>(vterm_if_key.vterm_key.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("vtn name is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vterminalname = reinterpret_cast<char*>(vterm_if_key.vterm_key.vterminal_name);
  if(0 == strlen(vterminalname)) {
    pfc_log_error("vterm name is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }
  intfname = reinterpret_cast<char*>(vterm_if_key.if_name);
  if(0 == strlen(intfname)) {
    pfc_log_error("vterm if is empty");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vtermifport_class *req_obj = new vtermifport_class(ctr_ptr,
                                       vtnname, vterminalname, intfname);
  std::string url = req_obj->get_url();
  pfc_log_info("URL:%s",url.c_str());
  vtermif_portmap *parse_obj = new vtermif_portmap();
  UncRespCode ret_val = req_obj->get_response(parse_obj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Get response error");
    delete req_obj;
    delete parse_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = parse_obj->set_vterm_if_portmap(parse_obj->jobj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("set vinterface portmap_conf error");
    delete req_obj;
    delete parse_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vtermif_port_detail = parse_obj->vterm_if_portmap_;
  return ret_val;
}

}  // namespace odcdriver
}  // namespace unc

