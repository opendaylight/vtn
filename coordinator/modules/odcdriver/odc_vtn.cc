/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vtn.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcVtnCommand::OdcVtnCommand(unc::restjson::ConfFileValues_t conf_values)
: conf_file_values_(conf_values) {
  ODC_FUNC_TRACE;
}

// Destructor
OdcVtnCommand::~OdcVtnCommand() {
}

// Constructs command to create Vtn and send it to rest interface to send to
// controller
UncRespCode OdcVtnCommand::create_cmd(key_vtn_t& key_vtn,
                                          val_vtn_t& val_vtn,
                                          unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  vtn_class *req_obj = new vtn_class(ctr_ptr);
  ip_vtn st_obj;
  UncRespCode ret_val = UNC_RC_SUCCESS;
  ret_val = create_request_body(val_vtn, key_vtn, st_obj);
  if (ret_val != UNC_RC_SUCCESS) {
    pfc_log_error("Error in create request body");
    delete req_obj;
    return ret_val;
  }
  vtn_parser *parser_obj = new vtn_parser();
  json_object *jobj = parser_obj->create_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if(req_obj->set_post(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vtn Create Failed");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
}

// Create Request Body
UncRespCode OdcVtnCommand::create_request_body(const val_vtn_t& val_vtn,
                                        key_vtn_t& key_vtn,
                                        ip_vtn& ip_vtn_st ) {
  ODC_FUNC_TRACE;
  ip_vtn_st.input_.update_mode = reinterpret_cast<const char*>("CREATE");
  ip_vtn_st.input_.operation = reinterpret_cast<const char*>("SET");
  ip_vtn_st.input_.tenant_name =
                      reinterpret_cast<const char*>(key_vtn.vtn_name);
  if (ip_vtn_st.input_.tenant_name.empty()) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  pfc_log_info("vtn name : %s", ip_vtn_st.input_.tenant_name.c_str());
  std::string description = reinterpret_cast<const char*>(val_vtn.description);
  pfc_log_info("vtn des : %s", description.c_str());
  ip_vtn_st.input_.valid = true;
  if (UNC_VF_VALID == val_vtn.valid[UPLL_IDX_DESC_VTN]) {
    if (!description.empty()) {
      ip_vtn_st.input_.ip_description.assign(description);
      }
    }
  return UNC_RC_SUCCESS;
}

// Update Request Body
UncRespCode OdcVtnCommand::update_request_body(const val_vtn_t& val_vtn,
                                        key_vtn_t& key_vtn,
                                        ip_vtn& ip_vtn_st) {
  ODC_FUNC_TRACE;
  ip_vtn_st.input_.valid = true;
  ip_vtn_st.input_.update_mode = reinterpret_cast<const char*>("ADD");
  ip_vtn_st.input_.operation = reinterpret_cast<const char*>("UPDATE");
  ip_vtn_st.input_.tenant_name =
                       reinterpret_cast<const char*>(key_vtn.vtn_name);

  if (ip_vtn_st.input_.tenant_name.empty()) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  pfc_log_info("vtn name : %s", ip_vtn_st.input_.tenant_name.c_str());
  std::string description = reinterpret_cast<const char*>(val_vtn.description);
  pfc_log_info("vtn des : %s", description.c_str());
  if (UNC_VF_VALID == val_vtn.valid[UPLL_IDX_DESC_VTN]) {
    if (!description.empty()) {
      ip_vtn_st.input_.ip_description.assign(description);
      }
    }
  return UNC_RC_SUCCESS;
  }

UncRespCode OdcVtnCommand::delete_request_body(const val_vtn_t& val_vtn,
                                        key_vtn_t& key_vtn,
                                        ip_vtn& ip_vtn_st) {
  ODC_FUNC_TRACE;
  ip_vtn_st.input_.valid = true;
  ip_vtn_st.input_.tenant_name =
                    reinterpret_cast<const char*>(key_vtn.vtn_name);
  if (ip_vtn_st.input_.tenant_name.empty()) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  pfc_log_info("vtn name : %s", ip_vtn_st.input_.tenant_name.c_str());
  return UNC_RC_SUCCESS;
}


//  Command to update vtn  and Send request to Controller
UncRespCode OdcVtnCommand::update_cmd(key_vtn_t& key_vtn,
                                          val_vtn_t& val_old_vtn,
                                          val_vtn_t& val_new_vtn,
                                          unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  vtn_class *req_obj = new vtn_class(ctr_ptr);
  ip_vtn st_obj;
  UncRespCode ret_val = UNC_RC_SUCCESS;
  ret_val = update_request_body(val_new_vtn, key_vtn, st_obj);
  if (ret_val != UNC_RC_SUCCESS){
    pfc_log_error("Error in update request body");
    delete req_obj;
    return ret_val;
  }
  vtn_parser *parser_obj = new vtn_parser();
  json_object *jobj = parser_obj->create_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in update");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  if(req_obj->set_put(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vtn Update Failed");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
}

//  fetch all child configuration for the parent key type(root)
UncRespCode OdcVtnCommand::fetch_config(unc::driver::controller* ctr_ptr,
                                   void* parent_key,
                                   std::vector<unc::vtndrvcache::ConfigNode *>
                                   &cfgnode_vector) {
  ODC_FUNC_TRACE;

  vtn_class *req_obj = new vtn_class(ctr_ptr);
  std::string url = req_obj->get_url();
  pfc_log_info("URL:%s",url.c_str());
  vtn_parser *parser_obj = new vtn_parser();
  UncRespCode ret_val = req_obj->get_response(parser_obj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Get response error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = parser_obj->set_vtn_conf(parser_obj->jobj);
   if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("set vtn-conf error");
    delete req_obj;
    delete parser_obj;
    return ret_val;
  }

  ret_val = parse_vtn_response(parser_obj->vtn_conf_ , cfgnode_vector);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing");
    delete req_obj;
    delete parser_obj;
    return ret_val;
  }

 return ret_val;
}

// parse each vtn node append to cache
UncRespCode OdcVtnCommand::fill_config_node_vector(
     std::string name,
     std::string descp,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
  ODC_FUNC_TRACE;
  key_vtn_t key_vtn;
  val_vtn_t val_vtn;
  memset(&key_vtn, 0, sizeof(key_vtn_t));
  memset(&val_vtn, 0, sizeof(val_vtn_t));

  //  Fills Key Structure
  strncpy(reinterpret_cast<char*> (key_vtn.vtn_name), name.c_str(),
          sizeof(key_vtn.vtn_name) - 1);

  // Fill Value Structure
  if (0 == strlen(descp.c_str())) {
    val_vtn.valid[UPLL_IDX_DESC_VTN] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vtn.description),
            descp.c_str(), sizeof(val_vtn.description) - 1);
    val_vtn.valid[UPLL_IDX_DESC_VTN] = UNC_VF_VALID;
  }
  pfc_log_debug("key_vtn.vtn_name: %s",
                reinterpret_cast<char*> (key_vtn.vtn_name));
  pfc_log_debug("val_vtn.description: %s",
                reinterpret_cast<char*> (val_vtn.description));

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vtn_t, val_vtn_t, val_vtn_t, uint32_t>
      (&key_vtn, &val_vtn, &val_vtn, uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfgnode_vector.push_back(cfgptr);
  return UNC_RC_SUCCESS;
}

// parsing function for converting controller response to driver format
UncRespCode OdcVtnCommand::parse_vtn_response(std::list<vtn_conf> &vtn_detail,
                                               std::vector< unc::vtndrvcache
                                               ::ConfigNode *>
                                               &cfgnode_vector) {
  ODC_FUNC_TRACE;
  UncRespCode ret_val = UNC_RC_SUCCESS;
  std::list<vtn_conf>::iterator it;
  for (it = vtn_detail.begin(); it != vtn_detail.end(); it++) {
  std::string vtnname = it->vtn_name;
  pfc_log_error("printing vtnname %s", vtnname.c_str());
  std::string descp = it->vtenant_config_.description;
  pfc_log_error("printing desccp %s", descp.c_str());
  ret_val = fill_config_node_vector(vtnname, descp, cfgnode_vector);
   if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Error return from fill config failure");
      return ret_val;
    }
 }
 return ret_val;
}

// Delete Request send to the Contoller
UncRespCode OdcVtnCommand::delete_cmd(key_vtn_t& key_vtn,
                                          val_vtn_t& val_vtn,
                                          unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  vtn_class *req_obj = new vtn_class(ctr_ptr);
  ip_vtn st_obj;
  UncRespCode ret_val = UNC_RC_SUCCESS;
  ret_val = delete_request_body(val_vtn, key_vtn, st_obj);
  if (ret_val != UNC_RC_SUCCESS) {
    pfc_log_error("Error in delete request body");
    delete req_obj;
    return ret_val;
  }
  vtn_parser *parser_obj = new vtn_parser();
  json_object *jobj = parser_obj->del_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in delete request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if(req_obj->set_delete(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vtn Create Failed");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  delete req_obj;
  delete parser_obj;
  return UNC_RC_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
