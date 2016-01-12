/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_vtnstation.hh>
#include <json_read_util.hh>
#include <set>
#include <string>


namespace unc {
namespace odcdriver {

class odl_vtn_station_filter {
 private:
  std::set <std::string> ip_address_filter_list_;
  pfc_bool_t ip_filter_;
  std::string switch_id_;
  pfc_bool_t switch_id_filter_;
  std::string mac_address_;
  pfc_bool_t mac_address_filter_;
  std::string port_name_;
  pfc_bool_t port_name_filter_;
  std::string vtn_name_;
  pfc_bool_t vtn_name_filter_;
  std::string vbr_name_;
  pfc_bool_t vbr_name_filter_;
  std::string vbrif_name_;
  pfc_bool_t vbrif_name_filter_;
  uint32_t vlan_id_;
  pfc_bool_t vlan_id_filter_;
  pfc_bool_t count_oper_;
  uint32_t valid_count_;

 public:
  odl_vtn_station_filter():ip_filter_(PFC_FALSE),
    switch_id_(""),
    switch_id_filter_(PFC_FALSE),
    mac_address_(""),
    mac_address_filter_(PFC_FALSE),
    port_name_(""),
    port_name_filter_(PFC_FALSE),
    vtn_name_(""),
    vtn_name_filter_(PFC_FALSE),
    vbr_name_(""),
    vbr_name_filter_(PFC_FALSE),
    vbrif_name_(""),
    vbrif_name_filter_(PFC_FALSE),
    vlan_id_(0),
    vlan_id_filter_(PFC_FALSE) ,
    count_oper_(PFC_FALSE),
    valid_count_(0) {}


  void update_filter(val_vtnstation_controller_st* in_val) {
    if ( in_val->valid[UPLL_IDX_MAC_ADDR_VSCS] == UNC_VF_VALID ) {
      // TODO(ODC): copy mac adres from val structure
      mac_address_filter_= PFC_TRUE;
    }

    if ( in_val->valid[UPLL_IDX_DATAPATH_ID_VSCS] == UNC_VF_VALID ) {
      switch_id_filter_= PFC_TRUE;
      switch_id_.assign(reinterpret_cast<char*> (in_val->switch_id));
    }

    if ( in_val->valid[UPLL_IDX_PORT_NAME_VSCS] == UNC_VF_VALID ) {
      port_name_filter_= PFC_TRUE;
      port_name_.assign(reinterpret_cast<char*> (in_val->port_name));
    }

    if ( in_val->valid[UPLL_IDX_VLAN_ID_VSCS] == UNC_VF_VALID ) {
      vlan_id_filter_= PFC_TRUE;
      vlan_id_= in_val->vlan_id;
    }

    if ( in_val->valid[UPLL_IDX_VTN_NAME_VSCS] == UNC_VF_VALID ) {
      vtn_name_filter_= PFC_TRUE;
      vtn_name_.assign(reinterpret_cast<char*>(in_val->vtn_name));
    }

    if ( in_val->valid[UPLL_IDX_VNODE_NAME_VSCS] == UNC_VF_VALID ) {
      vbr_name_filter_= PFC_TRUE;
      vbr_name_.assign(reinterpret_cast<char*>(in_val->vnode_name));
    }

    if ( in_val->valid[UPLL_IDX_VNODE_IF_NAME_VSCS] == UNC_VF_VALID ) {
      vbrif_name_filter_= PFC_TRUE;
      vbrif_name_.assign(reinterpret_cast<char*>(in_val->vnode_if_name));
    }
  }

  void update_filter(std::string ip_address) {
    ip_address_filter_list_.insert(ip_address);
  }

  void update_portmap_filter(std::string switch_id,
                             std::string port_name) {
    switch_id_= switch_id;
    switch_id_filter_= PFC_TRUE;
    port_name_= port_name;
    port_name_filter_= PFC_TRUE;
  }

  void count_operation_set() {
    ODC_FUNC_TRACE;
    count_oper_ = PFC_TRUE;
  }

  pfc_bool_t is_count_set() {
    ODC_FUNC_TRACE;
    return count_oper_;
  }

  UncRespCode check_ip_filter_match(std::string ip_address) {
    std::set<std::string>::iterator it;
    it = ip_address_filter_list_.find(ip_address);
    if ( it != ip_address_filter_list_.end())
      return UNC_RC_SUCCESS;
    return UNC_DRV_RC_ERR_GENERIC;
  }


  UncRespCode check_switch_match(val_vtnstation_controller_st* in_val) {
    if ( switch_id_filter_ == PFC_FALSE )
      return UNC_RC_SUCCESS;

    std::string check_switch_id(reinterpret_cast<char*>(in_val->switch_id));

    // Check for match with filter contents
    if ( switch_id_ != check_switch_id ) {
      pfc_log_info("Switch ID Match Failure");
      pfc_log_info("filter value %s", switch_id_.c_str());
      pfc_log_info("REad Value %s", check_switch_id.c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
    return UNC_RC_SUCCESS;
  }

  UncRespCode check_port_name_match(val_vtnstation_controller_st* in_val) {
    if (port_name_filter_ == PFC_FALSE)
      return UNC_RC_SUCCESS;

    std::string check_port_name(reinterpret_cast<char*>(in_val->port_name));

    // Check for match with filter contents
    if (port_name_ != check_port_name) {
      pfc_log_info("PORT Name Match Failure");
      pfc_log_info("Filter Value %s", port_name_.c_str());
      pfc_log_info("Real Value %s", check_port_name.c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
    return UNC_RC_SUCCESS;
  }


  UncRespCode check_vlan_id_match(val_vtnstation_controller_st* in_val) {
    if ( vlan_id_filter_ == PFC_FALSE )
      return UNC_RC_SUCCESS;

    if (vlan_id_ != in_val->vlan_id) {
      pfc_log_info("Filter vlan id %d", vlan_id_);
      pfc_log_info("Value vlan id %d", in_val->vlan_id);
      return UNC_DRV_RC_ERR_GENERIC;
    }

    return UNC_RC_SUCCESS;
  }


  UncRespCode check_valid_update(val_vtnstation_controller_st* in_val) {
    UncRespCode swid_ret(check_switch_match(in_val));

    if (swid_ret != UNC_RC_SUCCESS)
      return swid_ret;

    UncRespCode portname_ret(check_port_name_match(in_val));

    if (portname_ret != UNC_RC_SUCCESS)
      return portname_ret;

    UncRespCode vlan_ret(check_vlan_id_match(in_val));

    if (vlan_ret != UNC_RC_SUCCESS)
      return vlan_ret;

    return UNC_RC_SUCCESS;
  }

  void add_count() {
    valid_count_++;
  }

  uint32_t get_count() {
    return valid_count_;
  }
};

class odl_vtn_station_urls {
 public:
  odl_vtn_station_filter *filter_;
  unc::driver::controller *ctr_;
  unc::restjson::ConfFileValues_t conf_values_;


  odl_vtn_station_urls(odl_vtn_station_filter *filter,
                       unc::driver::controller *ctr,
                       unc::restjson::ConfFileValues_t conf_values) :
      filter_(filter), ctr_(ctr), conf_values_(conf_values) {}

  UncRespCode get_vtns(std::set <std::string> *vtns) {
    return unc::odcdriver::odlutils::get_tenant_names(ctr_,  vtns);
  }

  UncRespCode get_vbrs(std::string vtn_name, std::set <std::string> *vtns) {
    pfc_log_info("Collect VBRS for vtn %s", vtn_name.c_str());
    return unc::odcdriver::odlutils::get_bridge_names(ctr_,
                                                      vtn_name,
                                                      vtns);
  }

  std::string create_vtnstation_url(std::string vtn_name,
                                    std::string vbr_name) {
    ODC_FUNC_TRACE;
    std::string url = "";
    url.append(RESTCONF_BASE);
    url.append("/");
    url.append("vtn-mac-table");
    url.append(":");
    url.append("mac-tables");
    url.append("/");
    url.append("tenant-mac-table");
    url.append("/");
    url.append(vtn_name);

    pfc_log_info("url constructed is %s", url.c_str());
    return url;
  }

  UncRespCode get_urls(std::set <std::string> *urls) {
    ODC_FUNC_TRACE;
    // Get complete list of vtns and vbrs for url
    // if ( filter_->is_count_set() == PFC_TRUE ) {
    std::set <std::string> vtns;
    UncRespCode vtn_ret(get_vtns(&vtns));

    if ( vtn_ret != UNC_RC_SUCCESS ) {
      pfc_log_info("VTN Collection Failed");
      return vtn_ret;
    }

    std::set <std::string>::iterator vtn_iter;

    pfc_log_info("VBR Collection BEGIN");
    for (vtn_iter = vtns.begin(); vtn_iter != vtns.end(); ++vtn_iter) {
      pfc_log_info("VTN name %s", (*vtn_iter).c_str());
      pfc_log_info("ITERATION");
      std::set <std::string> vbrs;
      UncRespCode vbr_ret(get_vbrs((*vtn_iter), &vbrs));

      if (vbr_ret != UNC_RC_SUCCESS)
        return vbr_ret;

      std::set <std::string>::iterator vbr_iter;

      for (vbr_iter = vbrs.begin(); vbr_iter != vbrs.end(); ++vbr_iter) {
        pfc_log_info("ITERATION 2");
        std::string url_distinct("");
        url_distinct.append((*vtn_iter));
        url_distinct.append("/");
        url_distinct.append((*vbr_iter));
        urls->insert(url_distinct);
      }
    }
    // }
    return UNC_RC_SUCCESS;
  }


  UncRespCode get_port_name(std::string switch_id,
                             std::string port_id,
                             std::string &port_name) {
    return unc::odcdriver::odlutils::get_portname(ctr_,
                                                  conf_values_,
                                                  switch_id,
                                                  port_id,
                                                  port_name);
  }
};

class ip_address_parser : public unc::restjson::json_array_object_parse_util {
 public:
  std::set <std::string> ip_address_list_;

  ip_address_parser(json_object* instance):
      unc::restjson::json_array_object_parse_util(instance) {}

  int start_array_read(int length_of_array,
                       json_object* json_instance) {
    return unc::restjson::REST_OP_SUCCESS;
  }

  int end_array_read(uint32_t index,
                     json_object* json_instance) {
    return unc::restjson::REST_OP_SUCCESS;
  }

  int read_array_iteration(uint32_t index,
                           json_object* json_instance) {
    std::string search_ip("");
    std::string ip_address;
    int parse_ret(unc::restjson::json_object_parse_util::
                  read_string(json_instance,
                               search_ip,
                               ip_address));
    if ( parse_ret != unc::restjson::REST_OP_SUCCESS)
      return parse_ret;

    std::string ip_station = json_object_get_string(json_instance);
    ip_address_list_.insert(ip_station);
    return unc::restjson::REST_OP_SUCCESS;
  }

  UncRespCode validate_ip_with_filter(odl_vtn_station_filter* filter) {
    std::set<std::string>::iterator iter = ip_address_list_.begin();
    for ( ; iter  != ip_address_list_.end(); iter++) {
      if (filter->check_ip_filter_match(*iter))
        return UNC_RC_SUCCESS;
    }
    return UNC_DRV_RC_ERR_GENERIC;
  }

  uint32_t get_ip_count() {
    return ip_address_list_.size();
  }

  UncRespCode write_to_cache(unc::vtnreadutil::driver_read_util* read_) {
    std::set <std::string>::iterator iter = ip_address_list_.begin();

    for ( ; iter != ip_address_list_.end(); iter++ ) {
      unc::odcdriver::OdcUtil ip_util;
      in_addr ip_address;
      ip_util.convert_ip_to_inaddr(*iter, &ip_address);

      //         pfc_log_info("converted ip %"PFC_PFMT_u64 , ip_address.s_addr);
      unc::vtnreadutil::driver_read_util_io<in_addr>::
          add_read_value(&ip_address, read_);
    }
    return UNC_RC_SUCCESS;
  }
};



class odl_vtn_station_entry_parser {
 private:
  UncRespCode get_mac_address(json_object* instance,
                               std::string &mac_address ) {
    std::string search_mac("mac-address");
    int parse_ret(unc::restjson::json_object_parse_util::
                  read_string(instance,
                               search_mac,
                               mac_address));
    if ( parse_ret != unc::restjson::REST_OP_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;
    return UNC_RC_SUCCESS;
  }

  UncRespCode write_val_vtn_station(
      int vlanid,
      std::string mac,
      std::string switch_id,
      std::string port_name,
      std::string vtn_name,
      std::string vbr_name,
      unc::odcdriver::ip_address_parser* ip_parse,
      odl_vtn_station_urls *url_,
      unc::vtnreadutil::driver_read_util* read_) {
    ODC_FUNC_TRACE;
    unc::odcdriver::OdcUtil util;
    val_vtnstation_controller_st val_station;
    memset(&val_station, 0, sizeof(val_vtnstation_controller_st));

    if ( vlanid != 0 )
      val_station.vlan_id = vlanid;
    else
      val_station.vlan_id = ZERO_VLANID;

    val_station.valid[UPLL_IDX_VLAN_ID_VSCS]=UNC_VF_VALID;

    util.convert_macstring_to_uint8(mac, &val_station.mac_addr[0]);
    val_station.valid[UPLL_IDX_MAC_ADDR_VSCS] = UNC_VF_VALID;

    pfc_log_info(
        "MAC ADDR written to val struct %02x:%02x:%02x:%02x:%02x:%02x " ,
        val_station.mac_addr[0],
        val_station.mac_addr[1],
        val_station.mac_addr[2],
        val_station.mac_addr[3],
        val_station.mac_addr[4],
        val_station.mac_addr[5]);

    strcpy((char *)val_station.switch_id, switch_id.c_str());
    val_station.valid[UPLL_IDX_DATAPATH_ID_VSCS] = UNC_VF_VALID;

    pfc_log_info("Switch ID written is %s", (char *)val_station.switch_id);

    strcpy((char *)val_station.port_name, port_name.c_str());
    val_station.valid[UPLL_IDX_PORT_NAME_VSCS] = UNC_VF_VALID;

    util.convert_uint8_to_number(val_station.mac_addr, &val_station.station_id);
    val_station.valid[UPLL_IDX_STATION_ID_VSCS] = UNC_VF_VALID;


    pfc_log_info("STATION ID written is %" PFC_PFMT_u64,
                 val_station.station_id);

    val_station.ipv4_count = ip_parse->get_ip_count();
    val_station.valid[UPLL_IDX_IPV4_COUNT_VSCS]=UNC_VF_VALID;


    strcpy((char *)val_station.vtn_name, vtn_name.c_str());
    val_station.valid[UPLL_IDX_VTN_NAME_VSCS]=UNC_VF_VALID;

    strcpy((char *)val_station.vnode_name, vbr_name.c_str());
    val_station.valid[UPLL_IDX_VNODE_NAME_VSCS]=UNC_VF_VALID;

    std::string default_domain("(DEFAULT)");
    strcpy((char *)val_station.domain_id, default_domain.c_str());
    val_station.valid[UPLL_IDX_DOMAIN_ID_VSCS] = UNC_VF_VALID;

    val_station.vnode_type = UPLL_VNODE_VBRIDGE;
    val_station.valid[UPLL_IDX_VNODE_TYPE_VSCS] = UNC_VF_VALID;


    if (url_->filter_->check_valid_update(&val_station) != UNC_RC_SUCCESS) {
      pfc_log_info("Skip Invalid REsult");
      return UNC_RC_SUCCESS;
    }

    url_->filter_->add_count();

    unc::vtnreadutil::driver_read_util_io<val_vtnstation_controller_st>::
        add_read_value(&val_station, read_);

    int option1(read_->get_option1());
    if ( option1 == UNC_OPT1_DETAIL ) {
      write_dummy_stats(read_);
    }
    ip_parse->write_to_cache(read_);
    return UNC_RC_SUCCESS;
  }

  UncRespCode write_dummy_stats(unc::vtnreadutil::driver_read_util* read_) {
    val_vtnstation_controller_stat dumy_stat;
    memset(&dumy_stat, 0, sizeof(val_vtnstation_controller_stat));

    unc::vtnreadutil::driver_read_util_io<val_vtnstation_controller_stat>::
        add_read_value(&dumy_stat, read_);
    return UNC_RC_SUCCESS;
  }



 public:
  UncRespCode operator() (json_object* response,
                          unc::vtnreadutil::driver_read_util* read_,
                          odl_vtn_station_urls *url_,
                          std::string vtn_name,
                          std::string vbr_name) {
    pfc_log_info("instance to operator:%s", json_object_get_string(response));
    int vlan_id(0);
    uint32_t ret_val = unc::restjson::JsonBuildParse::parse(response,
                                                            "vlan-id",
                                                            -1,
                                                            vlan_id);
    pfc_log_info("vlan id %d", vlan_id);

    std::string mac_address("");
    UncRespCode mac_ret(get_mac_address(response, mac_address));
    if ( mac_ret != UNC_RC_SUCCESS )
      return mac_ret;

    pfc_log_info("MAC Address %s", mac_address.c_str());

    std::string switch_id("");
    ret_val = unc::restjson::JsonBuildParse::parse(response,
                                                   "node",
                                                   -1,
                                                   switch_id);
    pfc_log_info("SWITCH ID %s", switch_id.c_str());


    std::string port_id("");

    ret_val = unc::restjson::JsonBuildParse::parse(response,
                                                   "port-id",
                                                   -1,
                                                   port_id);
    pfc_log_info("PORT ID %s", port_id.c_str());

    std::string port_name("");

    ret_val = unc::restjson::JsonBuildParse::parse(response,
                                                   "port-name",
                                                   -1,
                                                   port_name);
    pfc_log_debug("PORT NAME %s", port_name.c_str());

    json_object *ipblockv4_str(NULL);
    ret_val = unc::restjson::JsonBuildParse::parse(response,
                                                   "ip-addresses",
                                                   -1,
                                                   ipblockv4_str);
    if ((json_object_is_type(ipblockv4_str, json_type_null))  ||
        (restjson::REST_OP_SUCCESS != ret_val)) {
      pfc_log_error("%s: json ip address is null", PFC_FUNCNAME);
      json_object_put(response);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    pfc_log_debug("IP Address:%s",json_object_get_string(ipblockv4_str));

    ip_address_parser parse(ipblockv4_str);
    int ip_parse_ret(parse.extract_values());

    if ( ip_parse_ret != unc::restjson::REST_OP_SUCCESS ) {
      pfc_log_info("Extract IP Address Failed");
    }

    int option1= read_->get_option1();
    int option2= read_->get_option2();

    pfc_log_info("option1 and option2 %d and %d", option1, option2);
    if (option1 != UNC_OPT1_COUNT) {
      pfc_log_info("Count Not SET");
      pfc_log_info("MAC READ is %s", mac_address.c_str());
      pfc_log_info("SWITCHID READ is %s", switch_id.c_str());
      pfc_log_info("PORT NAME READ is %s", port_name.c_str());
      write_val_vtn_station(vlan_id,
                            mac_address,
                            switch_id,
                            port_name,
                            vtn_name,
                            vbr_name,
                            &parse,
                            url_,
                            read_);
    } else {
      pfc_log_info("COUNT is SET");
      url_->filter_->add_count();
    }
    return UNC_RC_SUCCESS;
  }
};

class vtn_station_array_parser
: public unc::restjson::json_array_object_parse_util {
 public:
  unc::vtnreadutil::driver_read_util* read_util_;
  odl_vtn_station_urls *url_;
  std::string vtn_name_;
  std::string vbr_name_;


  vtn_station_array_parser(json_object* json_instance,
                           unc::vtnreadutil::driver_read_util* readutil,
                           odl_vtn_station_urls *url) :
      unc::restjson::json_array_object_parse_util(json_instance),
      read_util_(readutil), url_(url) {}
  void set_vtn_name(std::string vtn_name) {
    vtn_name_= vtn_name;
  }

  void set_vbr_name(std::string vbr_name) {
    vbr_name_= vbr_name;
  }

  int start_array_read(int length_of_array,
                       json_object* json_instance) {
    ODC_FUNC_TRACE;
    return unc::restjson::REST_OP_SUCCESS;
  }

  int read_array_iteration(uint32_t index,
                           json_object* json_instance) {
    ODC_FUNC_TRACE;

    odl_vtn_station_entry_parser parse;
    pfc_log_info("Invoke the Entry processor");
    if (parse(json_instance, read_util_, url_, vtn_name_, vbr_name_) ==
        UNC_RC_SUCCESS) {
      pfc_log_info("REad and Inserted to cachec successfully");
    } else {
      pfc_log_info("PARSE FAILED...");
    }
  pfc_log_debug("Leaving into vtn_station_array_parser");
    return unc::restjson::REST_OP_SUCCESS;
  }

  int end_array_read(uint32_t index,
                     json_object* json_instance) {
    ODC_FUNC_TRACE;
    return unc::restjson::REST_OP_SUCCESS;
  }
};

class odl_vtn_station_command : public unc::odcdriver::odl_http_rest_intf {
 public:
  odl_vtn_station_urls *url_;
  unc::vtnreadutil::driver_read_util* read_util_;
  uint32_t station_count_;

  odl_vtn_station_command(odl_vtn_station_urls *url,
                          unc::vtnreadutil::driver_read_util* read_util):
      url_(url), read_util_(read_util),
      station_count_(0) {}


  UncRespCode split_request_indicator(std::string request,
                                       std::string &vtn_name,
                                       std::string &vbr_name) {
    std::string delim("/");
    size_t position = request.find(delim);
    vtn_name = request.substr(0, position);
    vbr_name = request.substr(position+1);
    pfc_log_info("vtn_name is %s\n", vtn_name.c_str());
    pfc_log_info("vbr_name is %s\n", vbr_name.c_str());

    return UNC_RC_SUCCESS;
  }


  pfc_bool_t is_multiple_requests(unc::odcdriver::OdcDriverOps Op) {
    ODC_FUNC_TRACE;
    //  For count multiple queries are to be sent
    return PFC_TRUE;
  }

  UncRespCode get_multi_request_indicator(unc::odcdriver::OdcDriverOps Op,
                                           std::set<std::string> *arg_list) {
    ODC_FUNC_TRACE;
    return url_->get_urls(arg_list);
  }


  UncRespCode construct_url(unc::odcdriver::OdcDriverOps Op,
                            std::string &request_indicator,
                            std::string &url) {
    ODC_FUNC_TRACE;

    pfc_log_info("Request Indicator is %s", request_indicator.c_str());
    std::string vtn("");
    std::string vbr("");
    split_request_indicator(request_indicator, vtn, vbr);
    url.assign(url_->create_vtnstation_url(vtn, vbr));

    pfc_log_info("url in progress %s", url.c_str());
    return UNC_RC_SUCCESS;
  }

  UncRespCode construct_request_body(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     json_object *object) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }

  unc::restjson::HttpMethod get_http_method(unc::odcdriver::OdcDriverOps Op,
                                             std::string &request_indicator) {
    ODC_FUNC_TRACE;
    return  unc::restjson::HTTP_METHOD_GET;
  }


  UncRespCode validate_response_code(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     int resp_code) {
    ODC_FUNC_TRACE;
    if (HTTP_200_RESP_OK != resp_code) {
      return UNC_DRV_RC_ERR_GENERIC;
    }
    return UNC_RC_SUCCESS;
  }


  UncRespCode handle_response(unc::odcdriver::OdcDriverOps Op,
                              std::string &request_indicator,
                              char* data) {
    ODC_FUNC_TRACE;
    json_object *mac_entries_complete(NULL);
    int get_resp(unc::restjson::json_object_parse_util::
                 extract_json_object(data, &mac_entries_complete));
    if ( get_resp != unc::restjson::REST_OP_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;

    unc::restjson::json_obj_destroy_util mac_delete_obj(mac_entries_complete);

    std::string mac_string("tenant-mac-table");
    json_object *mac_entries_array(NULL);
    int get_array_resp(unc::restjson::json_object_parse_util::
                       extract_json_object(mac_entries_complete, mac_string,
                                           &mac_entries_array));
    if ( get_array_resp != unc::restjson::REST_OP_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;
    json_object *jobj_fill = NULL;
    uint32_t ret_val = unc::restjson::JsonBuildParse::parse(mac_entries_array,
                                                            "name",
                                                             0,
                                                             jobj_fill);
    std::string vtn = json_object_get_string(jobj_fill);
    if ((json_object_is_type(jobj_fill, json_type_null)) ||
                (restjson::REST_OP_SUCCESS != ret_val)) {
              pfc_log_error("%s: json data is null", PFC_FUNCNAME);
              json_object_put(mac_entries_array);
              return UNC_DRV_RC_ERR_GENERIC;
            }

    json_object *jobj_data = NULL;
    unc::restjson::json_obj_destroy_util mac_entrydelete_obj(mac_entries_array);
    pfc_log_debug("TMAC Table Entry:%s",json_object_get_string(mac_entries_array));
    ret_val = unc::restjson::JsonBuildParse::parse(mac_entries_array,
                                                   "mac-address-table",
                                                   0,
                                                   jobj_data);
    if ((json_object_is_type(jobj_data, json_type_null)) ||
        (restjson::REST_OP_SUCCESS != ret_val)) {
      pfc_log_error("%s: json data is null", PFC_FUNCNAME);
      json_object_put(mac_entries_array);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    pfc_log_debug("JOBJ DATA:%s",json_object_get_string(jobj_data));
    std::string vbr("");
    uint32_t array_length = 0;
    if (json_object_is_type(jobj_data, json_type_array)) {
       array_length = restjson::JsonBuildParse::get_array_length(jobj_data);
       pfc_log_debug("mac-address table array length:%d",array_length);
    }
    if (0 == array_length) {
       pfc_log_debug("No mac-address-table present");
       json_object_put(mac_entries_array);
       return UNC_RC_SUCCESS;
    }
    for(uint32_t arr_ide = 0;arr_ide<array_length;arr_ide++) {
    json_object *jobj_parse = NULL;
    ret_val = unc::restjson::JsonBuildParse::parse(jobj_data,
                                                   "name",
                                                   arr_ide,
                                                   jobj_parse);
    vbr = json_object_get_string(jobj_parse);
    if ((json_object_is_type(jobj_parse, json_type_null)) ||
            (restjson::REST_OP_SUCCESS != ret_val)) {
          pfc_log_error("%s: json data is null", PFC_FUNCNAME);
          json_object_put(jobj_data);
          return UNC_DRV_RC_ERR_GENERIC;
        }
    pfc_log_debug("VBR DATA:%s",vbr.c_str());
    }
    array_length = 0;
    if (json_object_is_type(jobj_data, json_type_array)) {
      array_length = restjson::JsonBuildParse::get_array_length(jobj_data);
      pfc_log_debug("mac-array_length:%d",array_length);
    }
    if (0 == array_length) {
      pfc_log_debug("No mac-table-entry present");
      json_object_put(mac_entries_complete);
      return UNC_RC_SUCCESS;
    }
    for(uint32_t arr_idx = 0;arr_idx<array_length;arr_idx++) {
    json_object *jobj_table = NULL;
    ret_val = unc::restjson::JsonBuildParse::parse(jobj_data,
                                                   "mac-table-entry",
                                                   arr_idx,
                                                   jobj_table);
    if ((json_object_is_type(jobj_data, json_type_null)) ||
        (restjson::REST_OP_SUCCESS != ret_val)) {
      pfc_log_error("%s: json table is null", PFC_FUNCNAME);
      json_object_put(jobj_data);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    pfc_log_debug("mac-table-entry idx:%d",arr_idx);
    pfc_log_debug("Entering into vtn_station_array_parser");
    if(json_object_is_type(mac_entries_complete, json_type_null)) {
      pfc_log_error("NULL");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    vtn_station_array_parser resp_parse(jobj_table, read_util_, url_);
    resp_parse.set_vtn_name(vtn);
    resp_parse.set_vbr_name(vbr);
    int vtn_station_ret(resp_parse.extract_values());

    if ( vtn_station_ret != unc::restjson::REST_OP_FAILURE )
      pfc_log_info("Array Parse SUCCESS");

    station_count_+=resp_parse.get_array_length();
  }
  return UNC_RC_SUCCESS;
}
};

//  Constructor
OdcVtnStationCommand::OdcVtnStationCommand(
    unc::restjson::ConfFileValues_t conf_values)
: age_interval_(DEFAULT_AGE_INTERVAL),
    conf_file_values_(conf_values) {
      ODC_FUNC_TRACE;
    }

//  Destructor
OdcVtnStationCommand::~OdcVtnStationCommand() {
}

UncRespCode OdcVtnStationCommand::read_cmd(
    unc::driver::controller *ctr,
    unc::vtnreadutil::driver_read_util* read_util) {
  ODC_FUNC_TRACE;
  // TODO(ODC): Add implementation here
  // Please read key and val from the read_util like below
  // key_vtnstation_controller_t key_
  // read_util.read_key_val<key_vtnstation_controller_t>(&key_, PFC_TRUE);
  // If you want to read val set FALSE to the above call and use
  // val object
  // Use read_util.add_read_value<val_structure> (value)
  int count = read_util->get_arg_count();
  key_vtnstation_controller station_key;
  UncRespCode key_read_resp =
      unc::vtnreadutil::driver_read_util_io<
      key_vtnstation_controller>::read_key_val
      (&station_key, PFC_TRUE, read_util, PFC_FALSE);

  if ( key_read_resp != UNC_RC_SUCCESS )
    pfc_log_info("Error reading KEy");
  pfc_log_info("Arg Count received %d", count);

  pfc_log_info("Create Filer ");
  odl_vtn_station_filter filter_;

  val_vtnstation_controller_st in_val;
  UncRespCode val_read(unc::vtnreadutil::
                        driver_read_util_io<
                        val_vtnstation_controller_st>::read_key_val
                        (&in_val, PFC_FALSE, read_util, PFC_FALSE));

  if (val_read != UNC_DRV_RC_MISSING_VAL_STRUCT) {
    // update filters
    filter_.update_filter(&in_val);
  }
  int option2= read_util->get_option1();

  if (option2 == UNC_OPT1_COUNT) {
    pfc_log_info("Count SET");
    filter_.count_operation_set();
  }

  odl_vtn_station_urls url_parser(&filter_,
                                  ctr,
                                  conf_file_values_);
  odl_vtn_station_command cmd(&url_parser,
                              read_util);

  odl_http_request vtn_station_request;
  UncRespCode cmd_ret(vtn_station_request.handle_request(
          ctr,
          unc::odcdriver::CONFIG_READ,
          &cmd,
          conf_file_values_));
  if ( cmd_ret != UNC_RC_SUCCESS )
    return cmd_ret;

  uint32_t valid_count = filter_.get_count();

  unc::vtnreadutil::driver_read_util_io<uint32_t>::add_read_value_top(
      &valid_count,
      read_util);

  unc::vtnreadutil::driver_read_util_io<
      key_vtnstation_controller>::add_read_value_top(
      &station_key,
      read_util);

  if ( (valid_count == 0) && (option2 != UNC_OPT1_COUNT) )
    return UNC_RC_NO_SUCH_INSTANCE;

  return UNC_RC_SUCCESS;
}

//  Constructing URL for vbridge, inject request to controller
std::string OdcVtnStationCommand::get_vtnstation_url(
    key_vtnstation_controller_t& key_) {
  ODC_FUNC_TRACE;
  std::string url = "";
  return url;
}
}  //  namespace odcdriver
}  //  namespace unc
