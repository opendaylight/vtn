/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vterminal.hh>

namespace unc {
namespace odcdriver {

// Constructor
OdcVterminalCommand::OdcVterminalCommand(unc::restjson::ConfFileValues_t
                                         conf_values)
: conf_file_values_(conf_values) {
      ODC_FUNC_TRACE;
    }

// Destructor
OdcVterminalCommand::~OdcVterminalCommand() {
}

// Constructs command to create Vtn and send it to rest interface to send to
// controller
UncRespCode OdcVterminalCommand::create_cmd(key_vterm_t& key_vterm,
                                            val_vterm_t& val_vterm,
                                            unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);
  char* vtnname = NULL;
  vtnname = reinterpret_cast<char*>(key_vterm.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }
  vterm_class *req_obj = new vterm_class(ctr_ptr, vtnname);
  ip_vterminal st_obj;
  create_request_body(val_vterm, key_vterm, st_obj);
  vterm_parser *parser_obj = new vterm_parser();
  json_object *jobj = parser_obj->create_req(st_obj);
  if(jobj == NULL){
    pfc_log_error("Error in create request");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
}
  if(req_obj->set_post(jobj) != UNC_RC_SUCCESS) {
    pfc_log_error("Vterm Create Failed");
    pfc_log_debug("check if vtn is stand-alone");
    //  check if vtn is stand-alone
    UncRespCode ret_code = UNC_DRV_RC_ERR_GENERIC;
    std::vector<unc::vtndrvcache::ConfigNode*> child_list;
    child_list.clear();
    std::string vtn_name = reinterpret_cast<const char*>
        (key_vterm.vtn_key.vtn_name);
    pfc_log_debug("VTN name str:%s", vtn_name.c_str());
    ret_code = req_obj->get_response(parser_obj);
    if (UNC_RC_SUCCESS != ret_code) {
        pfc_log_error("Get response error");
        delete req_obj;
        delete parser_obj;
        return UNC_DRV_RC_ERR_GENERIC;
    }

    ret_code = parser_obj->set_vterminal_conf(parser_obj->jobj);
    if (UNC_RC_SUCCESS != ret_code) {
      pfc_log_error("set_vterm_conf error");
      delete req_obj;
      delete parser_obj;
      return ret_code;
    }

    if (ret_code == UNC_RC_SUCCESS) {
        pfc_log_debug("delete stand-alone vtn");
        //  delete stand-alone vtn
        key_vtn_t key_vtn;
        val_vtn_t val_vtn;
        memset(&key_vtn, 0, sizeof(key_vtn_t));
        memset(&val_vtn, 0, sizeof(val_vtn_t));
        memcpy(key_vtn.vtn_name, key_vterm.vtn_key.vtn_name,
               sizeof(key_vterm.vtn_key.vtn_name));
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

// Create Request body
void OdcVterminalCommand::create_request_body(const val_vterm_t&
                                val_vterm, key_vterm_t& key_vterm,
                                    ip_vterminal& ip_vterminal_st) {
  ODC_FUNC_TRACE;
  ip_vterminal_st.input_vterm_.valid = true;
  ip_vterminal_st.input_vterm_.update_mode =
                                   reinterpret_cast<const char*>("CREATE");
  ip_vterminal_st.input_vterm_.operation = reinterpret_cast<const char*>("SET");
  ip_vterminal_st.input_vterm_.tenant_name =
                    reinterpret_cast<const char*>(key_vterm.vtn_key.vtn_name);

  ip_vterminal_st.input_vterm_.vterminal_name =
                        reinterpret_cast<const char*>(key_vterm.vterminal_name);

  ip_vterminal_st.input_vterm_.description =
                    reinterpret_cast<const char*>(val_vterm.vterm_description);
  pfc_log_info("vterm des : %s",
                             ip_vterminal_st.input_vterm_.description.c_str());
  if (UNC_VF_VALID == val_vterm.valid[UPLL_IDX_DESC_VBR]) {
    if (!ip_vterminal_st.input_vterm_.description.empty()) {
       pfc_log_info("vterm des not empty : %s",
                            ip_vterminal_st.input_vterm_.description.c_str());
       ip_vterminal_st.input_vterm_.description.assign(
                                   ip_vterminal_st.input_vterm_.description);
    }
  }

}


// Delete Request body
void OdcVterminalCommand::delete_request_body(const val_vterm_t&
                                val_vterm, key_vterm_t& key_vterm,
                                    ip_vterminal& ip_vterminal_st) {
  ODC_FUNC_TRACE;
  ip_vterminal_st.input_vterm_.valid = true;
  ip_vterminal_st.input_vterm_.tenant_name =
                reinterpret_cast<const char*>(key_vterm.vtn_key.vtn_name);

  ip_vterminal_st.input_vterm_.vterminal_name =
                       reinterpret_cast<const char*>(key_vterm.vterminal_name);
}

// Update Vterminal
UncRespCode OdcVterminalCommand::update_cmd(key_vterm_t& key, val_vterm_t& val_old,
                                            val_vterm_t& val_new,
                                            unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  return UNC_RC_SUCCESS;
}

// Delete Vterminal
UncRespCode OdcVterminalCommand::delete_cmd(key_vterm_t& key_vterm,
                                            val_vterm_t& val_vterm,
                       unc::driver::controller *ctr_ptr) {
  ODC_FUNC_TRACE;
  PFC_ASSERT(ctr_ptr != NULL);

  char* vtnname = NULL;
  vtnname = reinterpret_cast<char*>(key_vterm.vtn_key.vtn_name);
  if (0 == strlen(vtnname)) {
    pfc_log_error("Empty vtn name in %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

  vterm_class *req_obj = new vterm_class(ctr_ptr, vtnname);
  ip_vterminal st_obj;
  delete_request_body(val_vterm,key_vterm,st_obj);
  vterm_parser *parser_obj = new vterm_parser();
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


// fetch all child configuration for the parent key type(root)
UncRespCode OdcVterminalCommand::fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector) {
  ODC_FUNC_TRACE;
  pfc_log_debug("fetch_config for OdcVterminalCommand");
  key_vtn_t* parent_vtn = reinterpret_cast<key_vtn_t*> (parent_key);
  std::string vtn_name = reinterpret_cast<const char*>
      (parent_vtn->vtn_name);
  pfc_log_debug("%s:vtn_name", vtn_name.c_str());

  vterm_class *req_obj =  new vterm_class(ctr_ptr, vtn_name);
  std::string url = req_obj->get_url();
  pfc_log_info("URL:%s",url.c_str());
  vterm_parser *parser_obj =  new vterm_parser();
  UncRespCode ret_val = req_obj->get_response(parser_obj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Get response error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = parser_obj->set_vterminal_conf(parser_obj->jobj);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("set_vterminal_conf error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }

  ret_val = parse_vterminal_response(parser_obj->vterminal_conf_, vtn_name,
                                       ctr_ptr, cfg_node_vector);
  if (UNC_RC_SUCCESS != ret_val) {
    pfc_log_error("Error occured while parsing");
    delete req_obj;
    delete parser_obj;
    return ret_val;
  }

 return ret_val;

}

// parsing function for converting controller response to driver format
UncRespCode OdcVterminalCommand::parse_vterminal_response(
    std::list<vterminal_conf> &vterm_detail,
    std::string vtn_name,
    unc::driver::controller* ctr_ptr,
    std::vector< unc::vtndrvcache::ConfigNode *> &cfg_node_vector) {
  ODC_FUNC_TRACE;
  key_vterm_t key_vterm;
  val_vterm_t val_vterm;
  memset(&key_vterm, 0, sizeof(key_vterm_t));
  memset(&val_vterm, 0, sizeof(val_vterm_t));

  std::list<vterminal_conf>::iterator it;
  for (it = vterm_detail.begin(); it != vterm_detail.end(); it++) {
    std::string vterm_name = it->name;
    pfc_log_debug("printing vtermname %s", vterm_name.c_str());
    std::string description = it->vterminal_config_.vterm_description;
    pfc_log_debug("printing vterm description %s", description.c_str());
    UncRespCode ret_val = fill_config_node_vector(ctr_ptr, vterm_name,
                                                 vtn_name, description,
                                                 cfg_node_vector);
    if (UNC_RC_SUCCESS != ret_val) {
      pfc_log_error("Error return from fill config failure");
      return ret_val;
    }
  }
  return UNC_RC_SUCCESS;

}

UncRespCode OdcVterminalCommand::fill_config_node_vector(
    unc::driver::controller* ctr_ptr, std::string vterm_name,
    std::string vtn_name, std::string description,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfg_node_vector) {
  ODC_FUNC_TRACE;
  key_vterm_t key_vterm;
  val_vterm_t val_vterm;
  memset(&key_vterm, 0, sizeof(key_vterm_t));
  memset(&val_vterm, 0, sizeof(val_vterm_t));

  //  Fills the vterm KEY structure
  strncpy(reinterpret_cast<char*> (key_vterm.vtn_key.vtn_name),
          vtn_name.c_str(),
          sizeof(key_vterm.vtn_key.vtn_name) - 1);
  pfc_log_debug("vtn :%s", reinterpret_cast<char*>
                (key_vterm.vtn_key.vtn_name));

  strncpy(reinterpret_cast<char*> (key_vterm.vterminal_name),
          vterm_name.c_str(),
          sizeof(key_vterm.vterminal_name) - 1);
  pfc_log_debug(" vterminal :%s", reinterpret_cast<char*>
                (key_vterm.vterminal_name));

  //  Fills the vterminal VAL structure
  if (0 == strlen(description.c_str())) {
    val_vterm.valid[UPLL_IDX_DESC_VTERM] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vterm.vterm_description),
            description.c_str(),
            sizeof(val_vterm.vterm_description) - 1);
    val_vterm.valid[UPLL_IDX_DESC_VTERM] = UNC_VF_VALID;
  }
  pfc_log_debug("desc :%s", reinterpret_cast<char*>
                (val_vterm.vterm_description));

  std::string controller_name = ctr_ptr->get_controller_id();
  if (0 == strlen(controller_name.c_str())) {
    val_vterm.valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_INVALID;
  } else {
    strncpy(reinterpret_cast<char*>(val_vterm.controller_id),
            controller_name.c_str(), sizeof(val_vterm.controller_id) - 1);
    pfc_log_debug(" %s controller id",
                  reinterpret_cast<char*>(val_vterm.controller_id));
    val_vterm.valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_VALID;
  }
  strncpy(reinterpret_cast<char*> (val_vterm.domain_id), DOM_NAME.c_str(),
          sizeof(val_vterm.domain_id) - 1);
  val_vterm.valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_VALID;

  unc::vtndrvcache::ConfigNode *cfgptr =
      new unc::vtndrvcache::CacheElementUtil<key_vterm_t, val_vterm_t,
          val_vterm_t,
          uint32_t> (
          &key_vterm,
          &val_vterm, &val_vterm,
          uint32_t(UNC_OP_READ));
  PFC_ASSERT(cfgptr != NULL);
  cfg_node_vector.push_back(cfgptr);
  pfc_log_debug("parse_vterminal_append_vector Exi. cfg_node_vector size: %d",
                static_cast<int>(cfg_node_vector.size()));
  return UNC_RC_SUCCESS;
}
}  // namespace odcdriver
}  // namespace unc
