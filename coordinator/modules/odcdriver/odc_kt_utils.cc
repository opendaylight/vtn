/*
 * Copyright (c) 2014-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_kt_utils.hh>
#include <string>
#include <set>

namespace unc {
namespace odcdriver {

class vtn_read_resp_parser
    : public unc::restjson::json_array_object_parse_util {
 public:
  std::set <std::string> *vtnnames_;
  vtn_read_resp_parser(json_object* jobj,
                       std::set <std::string> *vtnset) :
      json_array_object_parse_util(jobj), vtnnames_(vtnset) {}
  int start_array_read(int length_of_array,
                       json_object* json_instance) {
    ODC_FUNC_TRACE;
    return unc::restjson::REST_OP_SUCCESS;
  }

  int read_array_iteration(uint32_t index,
                           json_object* json_instance) {
    ODC_FUNC_TRACE;
    std::string vtn_name("");
    std::string name_search("name");
    int ret(unc::restjson::json_object_parse_util::
             read_string(json_instance,
                          name_search,
                          vtn_name));
    if (ret == unc::restjson::REST_OP_SUCCESS) {
      pfc_log_info("vtn name %s", vtn_name.c_str());
      vtnnames_->insert(vtn_name);
      pfc_log_info("Read instance success ");
      return unc::restjson::REST_OP_SUCCESS;
    }
    return unc::restjson::REST_OP_FAILURE;
  }

  int end_array_read(uint32_t index,
                     json_object* json_instance) {
    ODC_FUNC_TRACE;
    return unc::restjson::REST_OP_SUCCESS;
  }
};

class port_info_resp_parser
: public unc::restjson::json_array_object_parse_util {
 public:
  std::string match_id_;
  std::string port_name_;

  port_info_resp_parser(json_object *array) :
      json_array_object_parse_util(array),
      match_id_(""), port_name_("") {}

  std::string get_port_name() {
    return port_name_;
  }

  void set_match_id(std::string match) {
    match_id_= match;
  }

  int start_array_read(int length_of_array,
                       json_object* json_instance) {
    ODC_FUNC_TRACE;
    return unc::restjson::REST_OP_SUCCESS;
  }

  int read_array_iteration(uint32_t index,
                           json_object* json_instance) {
    ODC_FUNC_TRACE;
    std::string nodeconn_name("nodeconnector");
    json_object *node_conn_obj(NULL);
    int nodecon_ret(unc::restjson::json_object_parse_util:: extract_json_object
                     (json_instance,
                      nodeconn_name,
                      &node_conn_obj));

    if (nodecon_ret != unc::restjson::REST_OP_SUCCESS)
      return nodecon_ret;

    // GET ID and chcek if it matches with match_id

    std::string id_search("id");
    std::string id_from_resp("");
    int id_ret(unc::restjson::json_object_parse_util::
                read_string(node_conn_obj,
                             id_search,
                             id_from_resp));

    if (id_ret != unc::restjson::REST_OP_SUCCESS)
      return id_ret;

    pfc_log_info("ID read is %s", id_from_resp.c_str());

    if ( id_from_resp == match_id_ ) {
      std::string properties_section("properties");
      json_object*  port_properties_obj(NULL);
      int port_prop_ret(unc::restjson::json_object_parse_util::
                         extract_json_object(json_instance,
                                              properties_section,
                                              &port_properties_obj));

      if (port_prop_ret != unc::restjson::REST_OP_SUCCESS)
        return port_prop_ret;

      std::string name_section("name");
      json_object*  port_name_obj(NULL);
      int portname_ret(unc::restjson::json_object_parse_util::
                        extract_json_object(port_properties_obj,
                                             name_section,
                                             &port_name_obj));

      if (portname_ret != unc::restjson::REST_OP_SUCCESS)
        return portname_ret;

      std::string name_value("value");
      int name_val_ret(unc::restjson::json_object_parse_util::
                        read_string(port_name_obj,
                                     name_value,
                                     port_name_));
      pfc_log_info("PORT NAME is %s", port_name_.c_str());

      if ( name_val_ret != unc::restjson::REST_OP_SUCCESS)
        return name_val_ret;
    }
    return unc::restjson::REST_OP_SUCCESS;
  }

  int end_array_read(uint32_t index,
                     json_object* json_instance) {
    ODC_FUNC_TRACE;
    return unc::restjson::REST_OP_SUCCESS;
  }

  UncRespCode operator()(std::string match,
                           std::string &port_name) {
    set_match_id(match);
    int ret(extract_values());

    if (ret != unc::restjson::REST_OP_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;

    port_name.assign(get_port_name());

    if ( port_name == "" )
      return UNC_DRV_RC_ERR_GENERIC;
    return UNC_RC_SUCCESS;
  }
};

class port_name_read_request : public odl_http_rest_intf {
 public:
  std::string switch_id_;
  std::string port_id_;
  std::string port_name_;

  port_name_read_request():switch_id_(""), port_name_("") {}
  void set_switch_id(std::string id) {
    switch_id_= id;
  }
  void set_port_id(std::string id) {
    port_id_= id;
  }

  std::string get_port_name() {
    return port_name_;
  }

  pfc_bool_t is_multiple_requests(unc::odcdriver::OdcDriverOps Op) {
    ODC_FUNC_TRACE;
    return PFC_FALSE;
  }

  UncRespCode get_multi_request_indicator(unc::odcdriver::OdcDriverOps Op,
                                           std::set<std::string> *arg_list) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }

  UncRespCode construct_url(unc::odcdriver::OdcDriverOps Op,
                            std::string &request_indicator,
                            std::string &url) {
    ODC_FUNC_TRACE;

    url.append(BASE_SW_URL);
    url.append(CONTAINER_NAME);
    url.append(NODE);
    url.append(NODE_OF);
    url.append(SLASH);
    url.append(switch_id_);
    return UNC_RC_SUCCESS;
  }

  UncRespCode construct_request_body(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     json_object *object) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }

  restjson::HttpMethod get_http_method(
      unc::odcdriver::OdcDriverOps Op,
      std::string &request_indicator) {
    ODC_FUNC_TRACE;
    return restjson::HTTP_METHOD_GET;
  }

  UncRespCode validate_response_code(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     int resp_code) {
    if ( resp_code == HTTP_200_RESP_OK )
      return UNC_RC_SUCCESS;

    return UNC_DRV_RC_ERR_GENERIC;
  }

  UncRespCode handle_response(unc::odcdriver::OdcDriverOps Op,
                              std::string &request_indicator,
                              char* data) {
    ODC_FUNC_TRACE;

    json_object* node_json_data(NULL);

    int obj_ret(unc::restjson::json_object_parse_util:: extract_json_object
                 (data,
                  &node_json_data));

    if (obj_ret != unc::restjson::REST_OP_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;


    unc::restjson::json_obj_destroy_util delete_node_obj(node_json_data);
    json_object* node_array_data(NULL);
    std::string node_search("nodeConnectorProperties");

    int node_ret(unc::restjson::json_object_parse_util:: extract_json_object
                  (node_json_data,
                   node_search,
                   &node_array_data));

    if (node_ret != unc::restjson::REST_OP_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;

    unc::restjson::json_obj_destroy_util delete_node_array_obj(node_array_data);

    port_info_resp_parser port_parse(node_array_data);
    if (port_parse(port_id_, port_name_) != UNC_RC_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;

    return UNC_RC_SUCCESS;
  }

  UncRespCode operator()(unc::driver::controller *ctr_ptr,
                           unc::restjson::ConfFileValues_t conf_values,
                           std::string switch_id,
                           std::string port_id,
                           std::string &port_name) {
    set_switch_id(switch_id);
    set_port_id(port_id);
    odl_http_request port_req;

    UncRespCode query_ret(port_req.handle_request(ctr_ptr,
                                                  CONFIG_READ,
                                                  this,
                                                  conf_values));

    if (query_ret != UNC_RC_SUCCESS)
      return query_ret;

    port_name.assign(get_port_name());

    if (port_name == "" )
      return UNC_DRV_RC_ERR_GENERIC;

    return UNC_RC_SUCCESS;
  }
};

class vtn_read_request : public odl_http_rest_intf {
 public:
  std::set<std::string> *vtnnames_;

  vtn_read_request(std::set<std::string> *vtns):
      vtnnames_(vtns) {}

  pfc_bool_t is_multiple_requests(unc::odcdriver::OdcDriverOps Op) {
    ODC_FUNC_TRACE;
    return PFC_FALSE;
  }


  UncRespCode get_multi_request_indicator(unc::odcdriver::OdcDriverOps Op,
                                           std::set<std::string> *arg_list) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }


  UncRespCode construct_url(unc::odcdriver::OdcDriverOps Op,
                            std::string &request_indicator,
                            std::string &url) {
    ODC_FUNC_TRACE;
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    return UNC_RC_SUCCESS;
  }

  UncRespCode construct_request_body(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     json_object *object) {
    return UNC_RC_SUCCESS;
  }

  restjson::HttpMethod get_http_method(
      unc::odcdriver::OdcDriverOps Op,
      std::string &request_indicator) {
    return restjson::HTTP_METHOD_GET;
  }

  UncRespCode validate_response_code(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     int resp_code) {
    if ( resp_code == HTTP_200_RESP_OK )
      return UNC_RC_SUCCESS;

    return UNC_DRV_RC_ERR_GENERIC;
  }

  UncRespCode handle_response(unc::odcdriver::OdcDriverOps Op,
                              std::string &request_indicator,
                              char* data) {
    json_object* vtn_data_json(NULL);
    json_object* vtn_array_json(NULL);
    std::string vtn_content("vtn");

    pfc_log_info("Data received for parsing %s", data);

    int ret(unc::restjson::json_object_parse_util:: extract_json_object
             (data,
              &vtn_data_json));

    if (ret != unc::restjson::REST_OP_SUCCESS) {
      pfc_log_info("Failed in parsing buffer");
      return UNC_DRV_RC_ERR_GENERIC;
    }

    unc::restjson::json_obj_destroy_util delete_vtn_obj(vtn_data_json);
    if (json_object_is_type(vtn_data_json, json_type_null)) {
      pfc_log_info("JSON is NULL");
            return UNC_DRV_RC_ERR_GENERIC;
    }

    int vtn_ret(unc::restjson::json_object_parse_util:: extract_json_object
                 (vtn_data_json,
                  vtn_content,
                  &vtn_array_json));

    if (vtn_ret != unc::restjson::REST_OP_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;
    unc::restjson::json_obj_destroy_util delete_vtn_date_obj(vtn_array_json);

    vtn_read_resp_parser vtn_collect(vtn_array_json, vtnnames_);
    int resp(vtn_collect.extract_values());

    if (resp != unc::restjson::REST_OP_SUCCESS) {
      pfc_log_info("Failed in collecting vtn names");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    pfc_log_info("VTN Collection Completed");
    return UNC_RC_SUCCESS;
  }
};

class vbr_read_request : public odl_http_rest_intf {
 public:
  std::set<std::string> *vbrnames_;
  std::string vtn_name_;

  vbr_read_request(std::set<std::string> *vbrs, std::string vtn_name):
      vbrnames_(vbrs), vtn_name_(vtn_name) {}
  pfc_bool_t is_multiple_requests(unc::odcdriver::OdcDriverOps Op) {
    ODC_FUNC_TRACE;
    return PFC_FALSE;
  }


  UncRespCode get_multi_request_indicator(unc::odcdriver::OdcDriverOps Op,
                                           std::set<std::string> *arg_list) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }

  UncRespCode construct_url(unc::odcdriver::OdcDriverOps Op,
                            std::string &request_indicator,
                            std::string &url) {
    ODC_FUNC_TRACE;

    if ( vtn_name_ == "" )
      return UNC_DRV_RC_ERR_GENERIC;
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    url.append("/");
    url.append(vtn_name_);
    url.append("/vbridges");
    return UNC_RC_SUCCESS;
  }

  UncRespCode construct_request_body(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     json_object *object) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }

  restjson::HttpMethod get_http_method(
      unc::odcdriver::OdcDriverOps Op,
      std::string &request_indicator) {
    ODC_FUNC_TRACE;
    return restjson::HTTP_METHOD_GET;
  }

  UncRespCode validate_response_code(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     int resp_code) {
    ODC_FUNC_TRACE;
    if ( resp_code == HTTP_200_RESP_OK )
      return UNC_RC_SUCCESS;

    return UNC_DRV_RC_ERR_GENERIC;
  }

  UncRespCode handle_response(unc::odcdriver::OdcDriverOps Op,
                              std::string &request_indicator,
                              char* data) {
    ODC_FUNC_TRACE;
    json_object* vtn_data_json(NULL);
    json_object* vtn_array_json(NULL);
    std::string vtn_content("vbridge");
    pfc_log_info("Data VBR received %s", data);
    int ret(unc::restjson::json_object_parse_util::extract_json_object
             (data,
              &vtn_data_json));

    if ( ret != unc::restjson::REST_OP_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;

    if ( vtn_data_json == NULL )
      return UNC_DRV_RC_ERR_GENERIC;

    unc::restjson::json_obj_destroy_util delete_vtn_obj(vtn_data_json);

    int vtn_ret(unc::restjson::json_object_parse_util::extract_json_object
                 (vtn_data_json,
                  vtn_content,
                  &vtn_array_json));

    if (vtn_ret != unc::restjson::REST_OP_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;

    unc::restjson::json_obj_destroy_util delete_vtn_array_obj(vtn_array_json);
    vtn_read_resp_parser vtn_collect(vtn_array_json, vbrnames_);
    int vtn_collect_ret(vtn_collect.extract_values());
    if ( vtn_collect_ret != unc::restjson::REST_OP_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;

    pfc_log_info("VBR Collection Completed");
    return UNC_RC_SUCCESS;
  }
};

class vterm_read_request : public odl_http_rest_intf {
 public:
  std::set<std::string> *vbrnames_;
  std::string vtn_name_;

  vterm_read_request(std::set<std::string> *vbrs, std::string vtn_name):
      vbrnames_(vbrs), vtn_name_(vtn_name) {}
  pfc_bool_t is_multiple_requests(unc::odcdriver::OdcDriverOps Op) {
    ODC_FUNC_TRACE;
    return PFC_FALSE;
  }

  UncRespCode get_multi_request_indicator(unc::odcdriver::OdcDriverOps Op,
                                           std::set<std::string> *arg_list) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }

  UncRespCode construct_url(unc::odcdriver::OdcDriverOps Op,
                            std::string &request_indicator,
                            std::string &url) {
    ODC_FUNC_TRACE;

    if ( vtn_name_ == "" )
      return UNC_DRV_RC_ERR_GENERIC;
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append(VTNS);
    url.append("/");
    url.append(vtn_name_);
    url.append("/vterminals");
    return UNC_RC_SUCCESS;
  }

  UncRespCode construct_request_body(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     json_object *object) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }

  restjson::HttpMethod get_http_method(
      unc::odcdriver::OdcDriverOps Op,
      std::string &request_indicator) {
    ODC_FUNC_TRACE;
    return restjson::HTTP_METHOD_GET;
  }

  UncRespCode validate_response_code(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     int resp_code) {
    ODC_FUNC_TRACE;
    if ( resp_code == HTTP_200_RESP_OK )
      return UNC_RC_SUCCESS;

    return UNC_DRV_RC_ERR_GENERIC;
  }

  UncRespCode handle_response(unc::odcdriver::OdcDriverOps Op,
                              std::string &request_indicator,
                              char* data) {
    ODC_FUNC_TRACE;
    json_object* vtn_data_json(NULL);
    json_object* vtn_array_json(NULL);
    std::string vtn_content("vterminal");
    pfc_log_info("Data VBR received %s", data);
    int ret(unc::restjson::json_object_parse_util::extract_json_object
             (data,
              &vtn_data_json));

    if ( ret != unc::restjson::REST_OP_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;

    if ( vtn_data_json == NULL )
      return UNC_DRV_RC_ERR_GENERIC;

    unc::restjson::json_obj_destroy_util delete_vterm_obj(vtn_data_json);

    int vtn_ret(unc::restjson::json_object_parse_util::extract_json_object
                 (vtn_data_json,
                  vtn_content,
                  &vtn_array_json));

    if (vtn_ret != unc::restjson::REST_OP_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;

    unc::restjson::json_obj_destroy_util delete_vterm_array_obj(vtn_array_json);
    vtn_read_resp_parser vtn_collect(vtn_array_json, vbrnames_);
    int vtn_collect_ret(vtn_collect.extract_values());
    if ( vtn_collect_ret != unc::restjson::REST_OP_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;

    pfc_log_info("VTERM Collection Completed");
    return UNC_RC_SUCCESS;
  }
};

UncRespCode port_info_parser::get_vlan_id(json_object* instance,
                                           int &vlanid ) {
  ODC_FUNC_TRACE;
  std::string search_vlan("vlan");
  int parse_ret(unc::restjson::json_object_parse_util::
                read_int_value(instance,
                               search_vlan,
                               vlanid));
  if ( parse_ret != unc::restjson::REST_OP_SUCCESS)
    return UNC_DRV_RC_ERR_GENERIC;

  return UNC_RC_SUCCESS;
}

//  Node!!

UncRespCode port_info_parser::get_switch_details(json_object* instance,
                                                  std::string &switch_id) {
  ODC_FUNC_TRACE;
  std::string search_node("node");
  json_object* switch_obj(NULL);
  int parse_ret(unc::restjson::json_object_parse_util::
                extract_json_object(instance, search_node, &switch_obj));

  if (parse_ret != unc::restjson::REST_OP_SUCCESS)
    return UNC_DRV_RC_ERR_GENERIC;

  std::string search_switch_name("id");
  int sw_parse_ret(unc::restjson::json_object_parse_util::
                   read_string(switch_obj,
                                search_switch_name,
                                switch_id));
  if ( sw_parse_ret != unc::restjson::REST_OP_SUCCESS)
    return UNC_DRV_RC_ERR_GENERIC;

  return UNC_RC_SUCCESS;
}

// port!!
UncRespCode port_info_parser::get_port_details(json_object* instance,
                                                std::string &portname) {
  std::string search_port("port");
  json_object* port_obj(NULL);

  int parse_ret(unc::restjson::json_object_parse_util::
                extract_json_object(instance, search_port, &port_obj));

  if ( parse_ret != unc::restjson::REST_OP_SUCCESS)
    return UNC_DRV_RC_ERR_GENERIC;

  std::string search_port_name("id");
  int sw_parse_ret(unc::restjson::json_object_parse_util::
                   read_string(port_obj,
                                search_port_name,
                                portname));
  if ( sw_parse_ret != unc::restjson::REST_OP_SUCCESS)
    return UNC_DRV_RC_ERR_GENERIC;

  return UNC_RC_SUCCESS;
}

UncRespCode odlutils::get_vtn_names(
    unc::driver::controller *ctr_ptr,
    unc::restjson::ConfFileValues_t conf_values,
    std::set <std::string> *vtns) {
  ODC_FUNC_TRACE;

  vtn_read_request vtn_read(vtns);
  odl_http_request vtn_req;
  return vtn_req.handle_request(ctr_ptr,
                                CONFIG_READ,
                                &vtn_read,
                                conf_values);
}

UncRespCode odlutils::get_tenant_names(unc::driver::controller *ctr,
                                   std::set <std::string> *vtns){

  vtn_class *req_obj = new vtn_class(ctr);
  std::string url = req_obj->get_url();
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
    pfc_log_error("set_vtn_conf error");
    delete req_obj;
    delete parser_obj;
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::list<vtn_conf>::iterator it = parser_obj->vtn_conf_.begin();
  while (it != parser_obj->vtn_conf_.end()) {
    vtns->insert(it->vtn_name);
    it++;
  }
  return UNC_RC_SUCCESS;
}

UncRespCode odlutils::get_vbridge_names(
    unc::driver::controller *ctr_ptr,
    unc::restjson::ConfFileValues_t conf_values,
    std::string vtn_name,
    std::set <std::string> *vbridges) {
  ODC_FUNC_TRACE;
  vbr_read_request vbr_read(vbridges, vtn_name);
  odl_http_request vbr_req;
  return vbr_req.handle_request(ctr_ptr,
      CONFIG_READ,
      &vbr_read,
      conf_values);
}

UncRespCode odlutils::get_bridge_names(unc::driver::controller *ctr,
                                       std::string vtn_name,
                                  std::set <std::string> *vbridges){

  vbr_class *req_obj = new vbr_class(ctr,vtn_name);
  std::string url = req_obj->get_url();
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
  std::list<vbridge_conf>::iterator it = parser_obj->vbridge_conf_.begin();
  while (it != parser_obj->vbridge_conf_.end()) {
    vbridges->insert(it->name);
    it++;
  }
  return UNC_RC_SUCCESS;
}

UncRespCode odlutils::get_terminal_names(unc::driver::controller *ctr,
    std::string vtn_name,
    std::set <std::string> *vterminals){

  vterm_class *req_obj = new vterm_class(ctr,vtn_name);
  std::string url = req_obj->get_url();
  vterm_parser *parser_obj = new vterm_parser();
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
  std::list<vterminal_conf>::iterator it = parser_obj->vterminal_conf_.begin();
  while (it != parser_obj->vterminal_conf_.end()) {
    vterminals->insert(it->name);
    it++;
  }
  return UNC_RC_SUCCESS;
}

UncRespCode odlutils::get_vterm_names (
    unc::driver::controller *ctr_ptr,
    unc::restjson::ConfFileValues_t conf_values,
    std::string vtn_name,
    std::set <std::string> *vbridges) {
  ODC_FUNC_TRACE;
  vterm_read_request vbr_read(vbridges, vtn_name);
  odl_http_request vbr_req;
  return vbr_req.handle_request(ctr_ptr,
      CONFIG_READ,
      &vbr_read,
      conf_values);
}

UncRespCode odlutils::get_portname(unc::driver::controller *ctr_ptr,
    unc::restjson::ConfFileValues_t conf_values,
    std::string switch_id,
    std::string port_id,
    std::string &port_name) {
  ODC_FUNC_TRACE;
  port_name_read_request port_req;
  return port_req(ctr_ptr, conf_values, switch_id, port_id, port_name);
}

}  // namespace odcdriver
}  // namespace  unc
