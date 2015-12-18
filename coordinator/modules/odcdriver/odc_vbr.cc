/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vbr.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcVbrCommand::OdcVbrCommand(unc::restjson::ConfFileValues_t conf_values)
: age_interval_(DEFAULT_AGE_INTERVAL),
  conf_file_values_(conf_values) {
  ODC_FUNC_TRACE;
}

// Destructor
OdcVbrCommand::~OdcVbrCommand() {
}

// Create Command and send Request to Controller
UncRespCode OdcVbrCommand::create_cmd(key_vbr_t& key_vbr,
                                          val_vbr_t& val_vbr,
                                          unc::driver::controller
                                          *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  char* vtnname = NULL;
  vtnname = reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vbr_class *req_obj = new vbr_class(ctr_ptr, vtnname);
  ip_vbridge st_obj;
  UncRespCode ret_val = UNC_RC_SUCCESS;
  ret_val = create_request_body(val_vbr, key_vbr, st_obj);
  if (ret_val != UNC_RC_SUCCESS){
    pfc_log_error("Vbr Create Request body failed");
    delete req_obj;
    return ret_val;
  }
  vbr_parser *parser_obj = new vbr_parser();
  json_object *jobj = parser_obj->create_req(st_obj);
  if(jobj == NULL) {
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if(req_obj->set_post(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vbr Create Failed");
    pfc_log_debug("check if vtn is stand-alone");
    //  check if vtn is stand-alone
    UncRespCode ret_code = UNC_DRV_RC_ERR_GENERIC;
    std::vector<unc::vtndrvcache::ConfigNode*> child_list;
    child_list.clear();
    std::string vtn_name = reinterpret_cast<const char*>
        (key_vbr.vtn_key.vtn_name);
    pfc_log_debug("VTN name str:%s", vtn_name.c_str());
    ret_code = req_obj->get_response(parser_obj);
    if (UNC_RC_SUCCESS != ret_code) {
      pfc_log_error("Get response error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
    }

    ret_code = parser_obj->set_vbridge_conf(parser_obj->jobj);
    if (UNC_RC_SUCCESS != ret_code) {
      pfc_log_error("set_vbridge_conf error");
      delete req_obj;
      delete parser_obj;
      return UNC_DRV_RC_ERR_GENERIC;
  }
    if (ret_code == UNC_RC_SUCCESS) {
        pfc_log_debug("delete stand-alone vtn");
        //  delete stand-alone vtn
        key_vtn_t key_vtn;
        val_vtn_t val_vtn;
        memset(&key_vtn, 0, sizeof(key_vtn_t));
        memset(&val_vtn, 0, sizeof(val_vtn_t));
        memcpy(key_vtn.vtn_name, key_vbr.vtn_key.vtn_name,
               sizeof(key_vbr.vtn_key.vtn_name));
        pfc_log_debug("VTN name:%s", key_vtn.vtn_name);
        OdcVtnCommand vtn_obj(conf_file_values_);
        vtn_obj.delete_cmd(key_vtn, val_vtn, ctr_ptr);
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
}

//  Command to update vtn  and Send request to Controller
UncRespCode OdcVbrCommand::update_cmd(key_vbr_t& key_vbr,
                                 val_vbr_t& val_old_vbr,
                                 val_vbr_t& val_new_vbr,
                                 unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  char* vtnname = NULL;
  vtnname = reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vbr_class *req_obj = new vbr_class(ctr_ptr, vtnname);
  ip_vbridge st_obj;
  UncRespCode ret_val = UNC_RC_SUCCESS;
  ret_val = update_request_body(val_new_vbr, key_vbr, st_obj);
  if (ret_val != UNC_RC_SUCCESS){
    pfc_log_error("Vbr Update Request body failed");
    delete req_obj;
    return ret_val;
  }

  vbr_parser *parser_obj = new vbr_parser();
  json_object *jobj = parser_obj->create_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if (req_obj->set_put(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vbr Update Failed");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
}

// Delete Request send to the Controller
UncRespCode OdcVbrCommand::delete_cmd(key_vbr_t& key_vbr,
                                          val_vbr_t& val_vbr,
                                          unc::driver::controller
                                          *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  char* vtnname = NULL;
  vtnname = reinterpret_cast<char*>(key_vbr.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vbr_class *req_obj = new vbr_class(ctr_ptr, vtnname);
  ip_vbridge st_obj;
  UncRespCode ret_val = UNC_RC_SUCCESS;
  ret_val = delete_request_body(val_vbr, key_vbr, st_obj);
    if (ret_val != UNC_RC_SUCCESS){
    pfc_log_error("Vbr Create Request body failed");
    delete req_obj;
    return ret_val;
  }
  vbr_parser *parser_obj = new vbr_parser();
  json_object *jobj = parser_obj->del_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
}

  if(req_obj->set_delete(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vbr Delete Failed");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
}

// Creates Request Body
UncRespCode OdcVbrCommand::create_request_body(const val_vbr_t& val_vbr,
                                                key_vbr_t& key_vbr,
                                                ip_vbridge&  ip_vbridge_st) {
  ODC_FUNC_TRACE;
  ip_vbridge_st.vbr_input_.valid = true;
  ip_vbridge_st.vbr_input_.update_mode =
                                   reinterpret_cast<const char*>("CREATE");
  ip_vbridge_st.vbr_input_.operation = reinterpret_cast<const char*>("SET");
  ip_vbridge_st.vbr_input_.tenant_name =
                        reinterpret_cast<const char*>(key_vbr.vtn_key.vtn_name);
  if (ip_vbridge_st.vbr_input_.tenant_name.empty()) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ip_vbridge_st.vbr_input_.bridge_name =
                                  reinterpret_cast<char*>(key_vbr.vbridge_name);
  if (ip_vbridge_st.vbr_input_.bridge_name.empty()) {
    pfc_log_error("Empty vbr name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  pfc_log_info("vtn name : %s",ip_vbridge_st.vbr_input_.tenant_name.c_str());
  pfc_log_info("vbr name : %s", ip_vbridge_st.vbr_input_.bridge_name.c_str());
  ip_vbridge_st.vbr_input_.ip_description =
                         reinterpret_cast<const char*>(val_vbr.vbr_description);
  if (UNC_VF_VALID == val_vbr.valid[UPLL_IDX_DESC_VBR]) {
    std::string description =
                         reinterpret_cast<const char*>(val_vbr.vbr_description);
    if (!description.empty()) {
       pfc_log_info("vbr des : %s", description.c_str());
       pfc_log_info("vbr des not empty : %s", description.c_str());
       ip_vbridge_st.vbr_input_.ip_description.assign(description);
    }
  }
  std::ostringstream age_interval_str_format;
  age_interval_str_format << age_interval_;
  ip_vbridge_st.vbr_input_.ageinterval.assign(age_interval_str_format.str());

 return UNC_RC_SUCCESS;
}

// update Request Body
UncRespCode OdcVbrCommand::update_request_body(const val_vbr_t& val_vbr,
                                                key_vbr_t& key_vbr,
                                                ip_vbridge&  ip_vbridge_st) {
  ODC_FUNC_TRACE;
  ip_vbridge_st.vbr_input_.valid = true;
  ip_vbridge_st.vbr_input_.update_mode =
                                    reinterpret_cast<const char*>("UPDATE");
  ip_vbridge_st.vbr_input_.operation = reinterpret_cast<const char*>("ADD");
  ip_vbridge_st.vbr_input_.tenant_name =
                        reinterpret_cast<const char*>(key_vbr.vtn_key.vtn_name);
   if (ip_vbridge_st.vbr_input_.tenant_name.empty()) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  ip_vbridge_st.vbr_input_.bridge_name =
                                 reinterpret_cast<char*>(key_vbr.vbridge_name);
  if (ip_vbridge_st.vbr_input_.bridge_name.empty()) {
    pfc_log_error("Empty vbr name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  pfc_log_info("vtn name : %s", ip_vbridge_st.vbr_input_.tenant_name.c_str());
  pfc_log_info("vbr name : %s", ip_vbridge_st.vbr_input_.bridge_name.c_str());
  pfc_log_info("vbr des : %s", ip_vbridge_st.vbr_input_.ip_description.c_str());
  if (UNC_VF_VALID == val_vbr.valid[UPLL_IDX_DESC_VBR]) {
    if (!ip_vbridge_st.vbr_input_.ip_description.empty()) {
       pfc_log_info("vbr des not empty : %s",
                            ip_vbridge_st.vbr_input_.ip_description.c_str());
       ip_vbridge_st.vbr_input_.ip_description.assign(
                        reinterpret_cast<const char*>(val_vbr.vbr_description));
    }
  }

  std::ostringstream age_interval_str_format;
  age_interval_str_format << age_interval_;
  ip_vbridge_st.vbr_input_.ageinterval.assign(
                                             age_interval_str_format.str());

 return UNC_RC_SUCCESS;
}

UncRespCode OdcVbrCommand::delete_request_body(const val_vbr_t& val_vbr,
                                                key_vbr_t& key_vbr,
                                                ip_vbridge&  ip_vbridge_st) {
  ODC_FUNC_TRACE;
  ip_vbridge_st.vbr_input_.valid = true;
  ip_vbridge_st.vbr_input_.tenant_name =
                        reinterpret_cast<const char*>(key_vbr.vtn_key.vtn_name);
  if (ip_vbridge_st.vbr_input_.tenant_name.empty()) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ip_vbridge_st.vbr_input_.bridge_name =
                                 reinterpret_cast<char*>(key_vbr.vbridge_name);
  if (ip_vbridge_st.vbr_input_.bridge_name.empty()) {
    pfc_log_error("Empty vbr name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  pfc_log_info("vtn name : %s",ip_vbridge_st.vbr_input_.tenant_name.c_str());
  pfc_log_info("vbr name : %s", ip_vbridge_st.vbr_input_.bridge_name.c_str());

return UNC_RC_SUCCESS;
}


// fetch child configurations for the parent kt(vtn)
UncRespCode OdcVbrCommand::fetch_config(unc::driver::controller* ctr_ptr,
                                    void* parent_key,
                                    std::vector<unc::vtndrvcache::ConfigNode *>
                                    &cfgnode_vector) {
  ODC_FUNC_TRACE;
  pfc_log_debug("fetch_config for OdcVbrCommand");

  key_vtn_t* parent_vtn = reinterpret_cast<key_vtn_t*> (parent_key);
  std::string vtn_name = reinterpret_cast<const char*>
                   (parent_vtn->vtn_name);
  pfc_log_debug("%s:vtn_name", vtn_name.c_str());

  vbr_class *req_obj = new vbr_class(ctr_ptr, vtn_name);
  std::string url = req_obj->get_url();
  pfc_log_info("URL:%s",url.c_str());
  vbr_parser *parser_obj = new vbr_parser();
  UncRespCode ret_val = req_obj->get_response(parser_obj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Get response error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = parser_obj->set_vbridge_conf(parser_obj->jobj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("set_vbridge_conf error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = parse_vbr_response(parser_obj->vbridge_conf_,
                           vtn_name, ctr_ptr, cfgnode_vector);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing");
    delete req_obj;
    delete parser_obj;
    return ret_val;
  }

  return ret_val;
}

// Prsing fuction for vbridge after getting vbridge from controller
UncRespCode OdcVbrCommand::parse_vbr_response(
                                    std::list<vbridge_conf> &vbr_detail,
                                         std::string vtn_name,
                                  unc::driver::controller* ctr_ptr,
  std::vector< unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_vbr, 0, sizeof(key_vbr_t));
  memset(&val_vbr, 0, sizeof(val_vbr_t));

  std::list<vbridge_conf>::iterator it;
  for (it = vbr_detail.begin(); it != vbr_detail.end(); it++) {
    std::string vbrname = it->name;
    pfc_log_debug("printing vbrname %s", vbrname.c_str());
    std::string description = it->vbridge_config_.description;
    pfc_log_debug("printing vbr description %s", description.c_str());
    UncRespCode ret_val = fill_config_node_vector(ctr_ptr,vbrname, description,
                                                     vtn_name, cfgnode_vector);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Error return from fill config failure");
      return ret_val;
    }

  }
  return UNC_RC_SUCCESS;
}

// each vbridge node append to cache
UncRespCode OdcVbrCommand::fill_config_node_vector(
    unc::driver::controller* ctr, std::string vbrname,
    std::string description, std::string vtn_name,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_vbr_t key_vbr;
  val_vbr_t val_vbr;
  memset(&key_vbr, 0, sizeof(key_vbr_t));
  memset(&val_vbr, 0, sizeof(val_vbr_t));

  //  Fills the vbr KEY structure
  strncpy(reinterpret_cast<char*> (key_vbr.vtn_key.vtn_name), vtn_name.c_str(),
          sizeof(key_vbr.vtn_key.vtn_name) - 1);
  pfc_log_debug("vtn :%s", reinterpret_cast<char*> (key_vbr.vtn_key.vtn_name));

  strncpy(reinterpret_cast<char*> (key_vbr.vbridge_name), vbrname.c_str(),
          sizeof(key_vbr.vbridge_name) - 1);
  pfc_log_debug(" vbr :%s", reinterpret_cast<char*> (key_vbr.vbridge_name));

  //  Fills the vbr VAL structure
  if (0 == strlen(description.c_str())) {
    val_vbr.valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vbr.vbr_description),
            description.c_str(), sizeof(val_vbr.vbr_description) - 1);
    val_vbr.valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID;
  }
  pfc_log_debug("desc :%s", reinterpret_cast<char*> (val_vbr.vbr_description));

  std::string controller_name = ctr->get_controller_id();
  if (0 == strlen(controller_name.c_str())) {
    val_vbr.valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vbr.controller_id),
            controller_name.c_str(), sizeof(val_vbr.controller_id) - 1);
    pfc_log_debug(" %s controller id",
                  reinterpret_cast<char*>(val_vbr.controller_id));
    val_vbr.valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
  }
  strncpy(reinterpret_cast<char*> (val_vbr.domain_id), DOM_NAME.c_str(),
          sizeof(val_vbr.domain_id) - 1);
  val_vbr.valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;

  unc::vtndrvcache::ConfigNode *cfgptr =
                        new unc::vtndrvcache::CacheElementUtil<key_vbr_t,
                         val_vbr_t, val_vbr_t, uint32_t> (
                         &key_vbr, &val_vbr, &val_vbr, uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  pfc_log_debug("parse_vbr_append_vector Exiting. cfgnode_vector size: %d",
                static_cast<int>(cfgnode_vector.size()));
  return UNC_RC_SUCCESS;
}

}  // namespace odcdriver
}  // namespace unc
