/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef RESTJSON_REST_CLIENT_H_
#define RESTJSON_REST_CLIENT_H_

#include <stdio.h>
#include <rest_common_defs.hh>

#include <string>

namespace unc {
namespace restjson {

class RestUtil {
 public:
  RestUtil(const std::string& ipaddress,
           const std::string& username,
          const std::string& pass)
      : ip_address_(ipaddress) {
        response_ = new HttpResponse_t;
        response_->code = 0;
        response_->write_data = new HttpContent_t;
      }

  ~RestUtil() {
    if (response_ !=  NULL) {
     if (response_->write_data != NULL) {
       delete response_->write_data;
       response_->write_data = NULL;
      }
      delete response_;
      response_ =  NULL;
    }
  }

  HttpResponse_t* send_http_request(const std::string &url,
                                    const unc::restjson::HttpMethod method,
                                    const char* request,
                                    const ConfFileValues_t &conf_file_values_) {
    url_ = url;
    std::string NULL_RESPONSE = "172.16.0.0";
    std::string INVALID_RESPONSE = "172.0.0.0";
    std::string CREATE_201      = "172.16.0.1";
    std::string UPDATE_DELETE_200  = "172.16.0.2";
    std::string READ_VTN_NULL_DATA     = "172.16.0.3";
    std::string CREATE_VBRIF_PORTMAP = "172.16.0.4";
    std::string UPDATE_DELETE_VBRIF_PORTMAP = "172.16.0.5";

    std::string VBRIF_GET_RESP = "172.16.0.6";
    std::string VBRIF_GET_RESP_EMPTY = "172.16.0.7";
    std::string  VBRIF_GET_RESP_PORT_MAP = "172.16.0.10";
    std::string READ_VBR_VALID = "172.16.0.8";
    std::string READ_VBR_NULL_DATA = "172.16.0.9";

    std::string VBRIF_GET_RESP_PORT_MAP_NO_VLAN = "172.16.0.11";

    std::string NOT_FOUND_404               = "172.16.0.12";
    std::string SERVICE_UNAVAILABLE_503      = "172.16.0.13";
    std::string VLAN_MAP_EMPTY               = "172.16.0.14";
    std::string VLAN_MAP_RESP            = "172.16.0.15";
    std::string VLAN_MAP_INCORRECT_RESP               = "172.16.0.16";
    std::string VLAN_MAP_VLAN_INCORRECT_RESP               = "172.16.0.17";
    std::string VLAN_MAP_RESP_ANY_0 = "172.16.0.18";
    std::string NULL_RESP_DATA = "172.16.0.19";


    std::string SWITCH_RESP  = "172.16.0.20";
    std::string SWITCH_RESP_UPDATE  = "172.16.0.21";
    std::string SWITCH_RESP_DELETE = "172.16.0.22";

    std::string PORT_RESP  = "172.16.0.23";
    std::string PORT_RESP_UPDATE  = "172.16.0.24";
    std::string PORT_RESP_DELETE = "172.16.0.25";

    std::string PORT_RESP_EMPTY = "172.16.0.26";
    std::string SWITCH_RESP_EMPTY = "172.16.0.27";
    std::string SWITCH_RESP_ONE = "172.16.0.28";
    std::string PORT_RESP_ONE = "172.16.0.29";
    std::string PORT_RESP_TWO = "172.16.0.30";

    std::string LINK_RESP_ONE = "172.16.0.31";
    std::string LINK_RESP_ONE_LINK = "172.16.0.32";
    std::string LINK_RESP_ONE_WRONG = "172.16.0.33";
    std::string LINK_RESP_ADD = "172.16.0.34";
    std::string LINK_RESP_ADD_DYNAMICALLY = "172.16.0.35";
    std::string LINK_RESP_UPDATE = "172.16.0.36";
    std::string LINK_RESP_DELETE = "172.16.0.37";
    std::string LINK_EDGE_WRONG = "172.16.0.38";
    std::string LINK_TAIL_NODE_WRONG = "172.16.0.39";
    std::string LINK_NODE_WRONG = "172.16.0.40";
    std::string LINK_NODE_ID_WRONG = "172.16.0.41";
    std::string LINK_TAIL_ID_WRONG = "172.16.0.42";
    std::string LINK_HEAD_NODE_CONN_WRONG = "172.16.0.43";
    std::string LINK_HEAD_ID_WRONG = "172.16.0.44";
    std::string LINK_HEAD_NODE_WRONG = "172.16.0.45";
    std::string LINK_HEAD_NODE_ID_WRONG = "172.16.0.47";
    std::string LINK_HEAD_NODE_PROP_WRONG = "172.16.0.48";
    std::string LINK_HEAD_NODE_PROP_NAME_WRONG = "172.16.0.49";
    std::string LINK_HEAD_NODE_PROP_NAME_VALUE_WRONG = "172.16.0.50";
    std::string LINK_HEAD_NODE_PROP_STATE_WRONG = "172.16.0.51";
    std::string LINK_HEAD_NODE_PROP_STATE_VALUE_WRONG = "172.16.0.52";
    std::string LINK_HEAD_NODE_PROP_CONFIG_WRONG = "172.16.0.53";
    std::string LINK_HEAD_NODE_PROP_CONFIG_VALUE_WRONG = "172.16.0.54";
    std::string LINK_EDGE_PROP_WRONG = "172.16.0.55";
    std::string LINK_RESP_UPDATE_LINK = "172.16.0.56";

    std::string PORT_NODE_CONN_PROP_WRONG = "172.16.0.57";
    std::string PORT_NODE_CONN_WRONG = "172.16.0.58";
    std::string PORT_NODE_CONN_ID_WRONG = "172.16.0.59";
    std::string PORT_NODE_CONN_NODE_WRONG = "172.16.0.60";
    std::string PORT_NODE_CONN_TYPE_WRONG = "172.16.0.61";
    std::string PORT_NODE_ID_WRONG = "172.16.0.62";
    std::string PORT_NODE_ID_SW = "172.16.0.63";
    std::string PORT_NODE_PROP_WRONG = "172.16.0.64";
    std::string PORT_NODE_PROP_NAME_WRONG = "172.16.0.65";
    std::string PORT_NODE_PROP_NAME_VALUE_WRONG = "172.16.0.66";
    std::string PORT_NODE_PROP_STATE_WRONG = "172.16.0.67";
    std::string PORT_NODE_PROP_STATE_VALUE_WRONG = "172.16.0.68";
    std::string PORT_COST_PROP_WRONG = "172.16.0.69";
    std::string PORT_NODE_PROP_CONFIG_VALUE_WRONG = "172.16.0.70";
    std::string PORT_NODE_PROP_BANDWIDTH_WRONG = "172.16.0.71";

    std::string SWITCH_NODE_PROP_WRONG = "172.16.0.72";
    std::string SWITCH_NODE_PROP_NODE_WRONG = "172.16.0.73";
    std::string SWITCH_NODE_PROP_ID_WRONG = "172.16.0.74";
    std::string SWITCH_NODE_PROPERTIES_WRONG = "172.16.0.75";

    std::string port_map_url_create_valid = "";
    port_map_url_create_valid.append
        ("/controller/nb/v2/vtn/default/vtns/vtn1/vbridges/");
    port_map_url_create_valid.append
        ("vbr1/interfaces/if_valid_create/portmap");

    std::string port_map_url_update_valid = "";
    port_map_url_update_valid.append
        ("/controller/nb/v2/vtn/default/vtns/vtn1/vbridges/");
    port_map_url_update_valid.append
        ("vbr1/interfaces/if_valid_update/portmap");


    std::string port_map_url_invalid = "";
    port_map_url_invalid.append
        ("/controller/nb/v2/vtn/default/vtns/vtn1/vbridges/");
    port_map_url_invalid.append
        ("vbr1/interfaces/if_invalid/portmap");
    std::string port_map_null_resp = "";
    port_map_null_resp.append
        ("/controller/nb/v2/vtn/default/vtns/vtn1/vbridges/");
    port_map_null_resp.append
        ("vbr1/interfaces/if_invalid/portmap");

    if (ip_address_.compare(CREATE_201) == 0) {
      response_->code = 201;
      return response_;
    } else if (ip_address_.compare(NULL_RESPONSE) ==  0) {
      clear_http_response();
      return NULL;
    } else if (ip_address_.compare(INVALID_RESPONSE) ==  0) {
      response_->code = 1;
      return response_;
    } else if (ip_address_.compare(NOT_FOUND_404) ==  0) {
      response_->code = 404;
      return response_;
    } else if (ip_address_.compare(NOT_FOUND_404) ==  0) {
      response_->code = 404;
      return response_;
    } else if (ip_address_.compare(SERVICE_UNAVAILABLE_503) == 0) {
      response_->code = 503;
      return response_;
    } else if (ip_address_.compare(UPDATE_DELETE_200) == 0) {
      response_->write_data->memory =
          const_cast<char *>("{\"vtn\": [ { \"name\": \"vtn_1\",\"description\": \"1\" }, {\"name\": \"vtn_2\" } ] }");
      response_->code = 200;
      return response_;
    } else if (ip_address_.compare(READ_VTN_NULL_DATA) == 0) {
      response_->write_data->memory =  const_cast<char *>("{\"vtn\":[]}");
      response_->code = 200;
      return response_;
    } else if (ip_address_.compare(CREATE_VBRIF_PORTMAP) == 0) {
      if (url_.compare(port_map_url_update_valid) == 0) {
        response_->code = 200;
        return response_;
      } else if (url_.compare(port_map_null_resp) == 0) {
        return NULL;
      }
      response_->code = 201;
      return response_;
    } else if (ip_address_.compare(UPDATE_DELETE_VBRIF_PORTMAP) == 0) {
      if (url_.compare(port_map_url_update_valid) == 0) {
        response_->code = 200;
        return response_;
      } else if (url_.compare(port_map_url_invalid) == 0) {
        response_->code = 1;
        return response_;
      } else if (url_.compare(port_map_null_resp) == 0) {
        return NULL;
      }
      response_->code = 200;
      return response_;
    } else if (ip_address_.compare(VBRIF_GET_RESP) == 0) {
      if (url_.compare(port_map_url_update_valid) != 0) {
        response_->code = 200;
        response_->write_data->memory = const_cast<char *>("{\"interface\": [ { \"name\": \"if_valid_update\",\"description\": \"1\" } ] }");
        return response_;
      }
      response_->code = 204;
      return response_;
    }  else if (ip_address_.compare(VBRIF_GET_RESP_EMPTY) == 0) {
      if (url_.compare(port_map_url_update_valid) != 0) {
        response_->code = 200;
        response_->write_data->memory =
            const_cast<char *>("{\"interface\": [ { \"name\": \"if_valid_update\" } ] }");
        return response_;
      }
      response_->code = 200;
      response_->write_data = NULL;
      return response_;
    } else if (ip_address_.compare(VBRIF_GET_RESP_PORT_MAP) == 0) {
      if (url_.compare(port_map_url_update_valid) != 0) {
        response_->code = 200;
        response_->write_data->memory = const_cast<char *>("{\"interface\": [ { \"name\": \"if_valid_update\",\"description\": \"1\" } ] }");
        return response_;
      }
      response_->code = 200;
      response_->write_data->memory =   const_cast<char *>("{\"vlan\": \"100\",\"node\" : {\"type\": \"OF\",\"id\": \"00:00:00:00:00:00:00:03\" }, \"port\" : {\"name\": \"port\" } }");
      return response_;
    } else if (ip_address_.compare(VBRIF_GET_RESP_PORT_MAP_NO_VLAN) == 0) {
      if (url_.compare(port_map_url_update_valid) != 0) {
        response_->code = 200;
        response_->write_data->memory = const_cast<char *>("{\"interface\": [ { \"name\": \"if_valid_update\",\"description\": \"1\" } ] }");
        return response_;
      }
      response_->code = 200;
      response_->write_data->memory =   const_cast<char *>("{\"vlan\": \"0\",\"node\" : {\"type\": \"OF\",\"id\": \"00:00:00:00:00:00:00:03\" }, \"port\" : {\"name\": \"port\" } }");
      return response_;
    } else if (ip_address_.compare(READ_VBR_VALID) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vbridge\": [ { \"name\": \"vbridge_1\",\"description\": \"1\" }, {\"name\": \"vbridge_2\" } ] }");
      return response_;
    } else if (ip_address_.compare(READ_VBR_NULL_DATA) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vbridge\":[]}");
      return response_;
    } else if (ip_address_.compare(VLAN_MAP_EMPTY) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vlanmap\":[]}");
      return response_;
    } else if (ip_address_.compare(VLAN_MAP_INCORRECT_RESP) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vlanmap:[]}");
      return response_;
    } else if (ip_address_.compare(VLAN_MAP_VLAN_INCORRECT_RESP) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vlanmap\":[{ \"id\":\"23}]}");
      return response_;
    } else if (ip_address_.compare(VLAN_MAP_RESP) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vlanmap\":[{ \"id\":\"OF-00:00:00:00:00:00:00:03.0\", \"vlan\":\"0\",\"node\": {\"type\":\"OF\", \"id\": \"00:00:00:00:00:00:00:03\"}}, {\"id\":\"ANY.7\", \"vlan\":\"7\"}]}");
      return response_;
    } else if (ip_address_.compare(VLAN_MAP_RESP_ANY_0) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vlanmap\":[{ \"id\":\"OF-00:00:00:00:00:00:00:03.10\", \"vlan\":\"10\",\"node\": {\"type\":\"OF\", \"id\": \"00:00:00:00:00:00:00:03\"}}, {\"id\":\"ANY.0\", \"vlan\":\"0\"}]}");
      return response_;
    } else if (ip_address_.compare(NULL_RESP_DATA) == 0) {
      response_->code = 200;
      delete response_->write_data;
      response_->write_data = NULL;
      return response_;
    } else if (ip_address_.compare(SWITCH_RESP) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-nodes\":{\"vtn-node\":[{\"id\":\"openflow:1\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:1:2\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:1:2\",\"peer\":\"openflow:3:3\"},{\"link-id\":\"openflow:3:3\",\"peer\":\"openflow:3:3\"}],\"name\":\"s1-eth2\"},{\"id\":\"openflow:1:1\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:2:3\",\"peer\":\"openflow:2:3\"},{\"link-id\":\"openflow:1:1\",\"peer\":\"openflow:2:3\"}],\"name\":\"s1-eth1\"}]},{\"id\":\"openflow:2\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:2:2\",\"enabled\":true,\"cost\":1000,\"name\":\"s2-eth2\"},{\"id\":\"openflow:2:3\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:2:3\",\"peer\":\"openflow:1:1\"},{\"link-id\":\"openflow:1:1\",\"peer\":\"openflow:1:1\"}],\"name\":\"s2-eth3\"},{\"id\":\"openflow:2:1\",\"enabled\":true,\"cost\":1000,\"name\":\"s2-eth1\"}]},{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"enabled\":true,\"cost\":1000,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:1\",\"enabled\":true,\"cost\":1000,\"name\":\"s3-eth1\"},{\"id\":\"openflow:3:3\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:1:2\",\"peer\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"peer\":\"openflow:1:2\"}],\"name\":\"s3-eth3\"}]}]}}");
      return response_;
    } else if (ip_address_.compare(SWITCH_RESP_UPDATE) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-nodes\":{\"vtn-node\":[{\"id\":\"openflow:1\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:1:2\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:1:2\",\"peer\":\"openflow:3:3\"},{\"link-id\":\"openflow:3:3\",\"peer\":\"openflow:3:3\"}],\"name\":\"s1-eth2\"},{\"id\":\"openflow:1:1\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:2:3\",\"peer\":\"openflow:2:3\"},{\"link-id\":\"openflow:1:1\",\"peer\":\"openflow:2:3\"}],\"name\":\"s1-eth1\"}]},{\"id\":\"openflow:2\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:2:2\",\"enabled\":true,\"cost\":1000,\"name\":\"s2-eth2\"},{\"id\":\"openflow:2:3\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:2:3\",\"peer\":\"openflow:1:1\"},{\"link-id\":\"openflow:1:1\",\"peer\":\"openflow:1:1\"}],\"name\":\"s2-eth3\"},{\"id\":\"openflow:2:1\",\"enabled\":true,\"cost\":1000,\"name\":\"s2-eth1\"}]},{\"id\":\"openflow:4\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:4:2\",\"enabled\":true,\"cost\":1000,\"name\":\"s3-eth2\"},{\"id\":\"openflow:4:1\",\"enabled\":true,\"cost\":1000,\"name\":\"s3-eth1\"},{\"id\":\"openflow:4:3\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:1:2\",\"peer\":\"openflow:1:2\"},{\"link-id\":\"openflow:4:3\",\"peer\":\"openflow:1:2\"}],\"name\":\"s3-eth3\"}]}]}}");
      return response_;
    } else if (ip_address_.compare(SWITCH_RESP_DELETE) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-nodes\":{\"vtn-node\":[{\"id\":\"openflow:1\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:1:2\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:1:2\",\"peer\":\"openflow:3:3\"},{\"link-id\":\"openflow:3:3\",\"peer\":\"openflow:3:3\"}],\"name\":\"s1-eth2\"},{\"id\":\"openflow:1:1\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:2:3\",\"peer\":\"openflow:2:3\"},{\"link-id\":\"openflow:1:1\",\"peer\":\"openflow:2:3\"}],\"name\":\"s1-eth1\"}]},{\"id\":\"openflow:2\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:2:2\",\"enabled\":true,\"cost\":1000,\"name\":\"s2-eth2\"},{\"id\":\"openflow:2:3\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:2:3\",\"peer\":\"openflow:1:1\"},{\"link-id\":\"openflow:1:1\",\"peer\":\"openflow:1:1\"}],\"name\":\"s2-eth3\"},{\"id\":\"openflow:2:1\",\"enabled\":true,\"cost\":1000,\"name\":\"s2-eth1\"}]}]}");
      return response_;
      return response_;
    } else if (ip_address_.compare(SWITCH_RESP_ONE) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-nodes\":{\"vtn-node\":[{\"id\":\"openflow:1\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:1:2\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:1:2\",\"peer\":\"openflow:3:3\"},{\"link-id\":\"openflow:3:3\",\"peer\":\"openflow:3:3\"}],\"name\":\"s1-eth2\"},{\"id\":\"openflow:1:1\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:2:3\",\"peer\":\"openflow:2:3\"},{\"link-id\":\"openflow:1:1\",\"peer\":\"openflow:2:3\"}]}],\"name\":\"s1-eth1\"},{\"id\":\"openflow:2\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:2:2\",\"enabled\":true,\"cost\":1000,\"name\":\"s2-eth2\"},{\"id\":\"opoenflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"enabled\":true,\"cost\":1000,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:1\",\"enabled\":true,\"cost\":1000,\"name\":\"s3-eth1\"},{\"id\":\"openflow:3:3\",\"enabled\":true,\"cost\":1000,\"port-link\":[{\"link-id\":\"openflow:1:2\",\"peer\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"peer\":\"openflow:1:2\"}],\"name\":\"s3-eth3\"}]}]}");
      return response_;
    } else if (ip_address_.compare(PORT_RESP_ONE) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"nodeConnectorProperties\":[{\"nodeconnector\":{\"type\":\"SW\",\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:01\"},\"id\":\"0\"},\"properties\":{\"name\":{\"value\":\"s2\"},\"state\":{\"value\":0},\"config\":{\"value\":0}}},{\"nodeconnector\":{\"type\":\"OF\",\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:01\"},\"id\":\"1\"},\"properties\":{\"name\":{\"value\":\"s2-eth1\"},\"state\":{\"value\":1},\"config\":{\"value\":1},\"bandwidth\":{\"value\":10000000000}}}]}");
      return response_;
    } else if (ip_address_.compare(PORT_RESP_TWO) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"nodeConnectorProperties\":[{\"nodeconnector\":{\"type\":\"SW\",\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:01\"},\"id\":\"0\"},\"properties\":{\"name\":{\"value\":\"s2\"},\"state\":{\"value\":0},\"config\":{\"value\":0}}},{\"nodeconnector\":{\"type\":\"OF\",\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:01\"},\"id\":\"1\"},\"properties\":{\"name\":{\"value\":\"s1-eth2\"},\"state\":{\"value\":1},\"config\":{\"value\":1},\"bandwidth\":{\"value\":10000000000}}},{\"nodeconnector\":{\"type\":\"OF\",\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:01\"},\"id\":\"1\"},\"properties\":{\"name\":{\"value\":\"s1-eth3\"},\"state\":{\"value\":1},\"config\":{\"value\":1},\"bandwidth\":{\"value\":10000000000}}}]}");
      return response_;
    } else if (ip_address_.compare(SWITCH_RESP_EMPTY) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"nodeProperties\":[]}");
      return response_;
    } else if (ip_address_.compare(PORT_RESP) == 0) {
      response_->code = 200;
       response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"}]}]}");
      return response_;
    } else if (ip_address_.compare(PORT_RESP_EMPTY) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"nodeConnectorProperties\":[]}");
      return response_;
    } else if (ip_address_.compare(PORT_RESP_UPDATE) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth1\"}]}]}");
      return response_;
    } else if (ip_address_.compare(PORT_RESP_DELETE) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"nodeConnectorProperties\":[{\"nodeconnector\":{\"type\":\"SW\",\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:02\"},\"id\":\"0\"},\"properties\":{\"name\":{\"value\":\"s2\"},\"state\":{\"value\":0},\"config\":{\"value\":0}}},{\"nodeconnector\":{\"type\":\"OF\",\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:02\"},\"id\":\"1\"},\"properties\":{\"name\":{\"value\":\"s2-eth1\"},\"state\":{\"value\":1},\"config\":{\"value\":1},\"bandwidth\":{\"value\":10000000000}}}]}");
      return response_;
    } else if (ip_address_.compare(LINK_RESP_ONE) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:2:3\",\"source\":\"openflow:2:3\",\"destination\":\"openflow:1:1\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_RESP_ONE_LINK) == 0) {
      response_->code =  200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:2:3\",\"source\":\"openflow:2:3\",\"destination\":\"openflow:1:1\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_RESP_ONE_WRONG) == 0) {
      response_->code =  200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:2:3\",\"source\":\"openflow:2:3\",\"destination\":\"openflow:1:1\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"desnation\":\"openflow:1:2\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_RESP_ADD) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:2:3\",\"source\":\"openflow:2:3\",\"destination\":\"openflow:1:1\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:1:2\",\"source\":\"openflow:1:2\",\"destination\":\"openflow:3:3\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_RESP_ADD_DYNAMICALLY) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:1:2\",\"source\":\"openflow:1:2\",\"destination\":\"openflow:3:3\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_RESP_UPDATE) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:1:2\",\"source\":\"openflow:1:2\",\"destination\":\"openflow:3:3\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_RESP_UPDATE_LINK) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:1:2\",\"source\":\"openflow:1:2\",\"destination\":\"openflow:3:3\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_RESP_DELETE) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":[]}");
      return response_;
    } else if (ip_address_.compare(LINK_EDGE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-lin\":[{\"link-id\":\"openflow:2:3\",\"source\":\"openflow:2:3\",\"destination\":\"openflow:1:1\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_TAIL_NODE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:2:3\",\"source\":\"openflow:2:3\",\"destination\":\"openflow:2:3\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_NODE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-i\":\"openflow:2:3\",\"source\":\"openflow:2:3\",\"destination\":\"openflow:1:1\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_NODE_ID_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflo:2:3\",\"source\":\"openflow:2:3\",\"destination\":\"openflow:1:1\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_TAIL_ID_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:2:3\",\"source\":\"openflow:2:3\",\"destination\":\"openflo:1:1\"}]}}");
      return response_;
    } else if (ip_address_.compare(LINK_HEAD_NODE_CONN_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-i\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(LINK_HEAD_ID_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"sourc\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(LINK_HEAD_NODE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(LINK_HEAD_NODE_ID_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(LINK_HEAD_NODE_PROP_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(LINK_HEAD_NODE_PROP_NAME_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(LINK_HEAD_NODE_PROP_NAME_VALUE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(LINK_HEAD_NODE_PROP_STATE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(LINK_HEAD_NODE_PROP_STATE_VALUE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(LINK_HEAD_NODE_PROP_CONFIG_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(LINK_HEAD_NODE_PROP_CONFIG_VALUE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(LINK_EDGE_PROP_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-topology\":{\"vtn-link\":[{\"link-id\":\"openflow:3:4\",\"source\":\"openflow:3:4\",\"destination\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"source\":\"openflow:3:3\",\"destination\":\"openflow:1:2\"}]}}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_CONN_PROP_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"i\":\"openflow:3\",\"openflow-vers\":\"OF10\",\"vtn-port\":[{\"i\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_CONN_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-nod\":[{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_CONN_ID_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"i\":\"openflo:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_CONN_NODE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"i\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cot\":1000,\"enable\":tru,\"name\":\"s3-eth2\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_CONN_TYPE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflo:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth1\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_ID_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflow:i\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth1\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_ID_SW) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"i\":\"openflow:\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth1\"},{\"id\":\"openflow:3:3\",\"cost\":1000,\"enabled\":true,\"port-link\":[{\"link-id\":\"openflow:1:2\",\"peer\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"peer\":\"openflow:1:2\"}],\"name\":\"s3-eth3\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_PROP_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth1\"},{\"id\":\"openflow:3:3,\"cost\":1000,\"enabled\":true,\"port-link\":[{\"link-id\":\"openflow:1:2\",\"peer\":\"openflow:1:2\"},{\"link-id\":\"openflow:3:3\",\"peer\":\"openflow:1:2\"}],\"name\":\"s3-eth3\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_PROP_NAME_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"nam\":\"s3-eth1\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_PROP_NAME_VALUE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth1\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_PROP_STATE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"i\":\"openflow:3:2\",\"cot\":1000,\"enabled\":tr,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth1\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_PROP_STATE_VALUE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth1\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_COST_PROP_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"co\":1000,\"enabled\":true,\"name\":\"s3-eth2\"},{\"id\":\"openflow::2\",\"cost\":1000,\"enabled\":true,\"name\":\"s-eth1\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_PROP_CONFIG_VALUE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth1\"}]}]}");
      return response_;
  } else if (ip_address_.compare(PORT_NODE_PROP_BANDWIDTH_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vtn-node\":[{\"id\":\"openflow:3\",\"openflow-version\":\"OF10\",\"vtn-port\":[{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth2\"},{\"id\":\"openflow:3:2\",\"cost\":1000,\"enabled\":true,\"name\":\"s3-eth1\"}]}]}");
      return response_;
  } else if (ip_address_.compare(SWITCH_NODE_PROP_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"nodePropertie\":[{\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:02\"},\"properties\":{\"tables\":{\"value\":-1},\"forwarding\":{\"value\":0},\"timeStamp\":{\"value\":1388174650189,\"name\":\"connectedSince\"},\"buffers\":{\"value\":256},\"description\":{\"value\":\"None\"},\"capabilities\":{\"value\":199},\"macAddress\":{\"value\":\"00:00:00:00:00:02\"},\"supportedFlowActions\":{\"value\":\"[Controller, Drop, Enqueue, HwPath, Output, PopVlan, SetDlDst,SetDlSrc, SetNwDst, SetNwSrc, SetNwTos, SetTpDst, SetTpSrc, SetVlanId, SetVlanPcp, SwPath]\"}}},{\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:03\"},\"properties\":{\"tables\":{\"value\":-1},\"forwarding\":{\"value\":0},\"timeStamp\":{\"value\":1388174650225,\"name\":\"connectedSince\"},\"buffers\":{\"value\":256},\"description\":{\"value\":\"None\"},\"capabilities\":{\"value\":199},\"macAddress\":{\"value\":\"00:00:00:00:00:03\"},\"supportedFlowActions\":{\"value\":\"[Controller, Drop, Enqueue, HwPath, Output, PopVlan, SetDlDst, SetDlSrc, SetNwDst, SetNwSrc, SetNwTos, SetTpDst, SetTpSrc, SetVlanId, SetVlanPcp, SwPath]\"}}}]}");
      return response_;
  } else if (ip_address_.compare(SWITCH_NODE_PROP_NODE_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"nodeProperties\":[{\"nod\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:02\"},\"properties\":{\"tables\":{\"value\":-1},\"forwarding\":{\"value\":0},\"timeStamp\":{\"value\":1388174650189,\"name\":\"connectedSince\"},\"buffers\":{\"value\":256},\"description\":{\"value\":\"None\"},\"capabilities\":{\"value\":199},\"macAddress\":{\"value\":\"00:00:00:00:00:02\"},\"supportedFlowActions\":{\"value\":\"[Controller, Drop, Enqueue, HwPath, Output, PopVlan, SetDlDst,SetDlSrc, SetNwDst, SetNwSrc, SetNwTos, SetTpDst, SetTpSrc, SetVlanId, SetVlanPcp, SwPath]\"}}},{\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:03\"},\"properties\":{\"tables\":{\"value\":-1},\"forwarding\":{\"value\":0},\"timeStamp\":{\"value\":1388174650225,\"name\":\"connectedSince\"},\"buffers\":{\"value\":256},\"description\":{\"value\":\"None\"},\"capabilities\":{\"value\":199},\"macAddress\":{\"value\":\"00:00:00:00:00:03\"},\"supportedFlowActions\":{\"value\":\"[Controller, Drop, Enqueue, HwPath, Output, PopVlan, SetDlDst, SetDlSrc, SetNwDst, SetNwSrc, SetNwTos, SetTpDst, SetTpSrc, SetVlanId, SetVlanPcp, SwPath]\"}}}]}");
      return response_;
  } else if (ip_address_.compare(SWITCH_NODE_PROP_ID_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"nodeProperties\":[{\"node\":{\"type\":\"OF\",\"i\":\"00:00:00:00:00:00:00:02\"},\"properties\":{\"tables\":{\"value\":-1},\"forwarding\":{\"value\":0},\"timeStamp\":{\"value\":1388174650189,\"name\":\"connectedSince\"},\"buffers\":{\"value\":256},\"description\":{\"value\":\"None\"},\"capabilities\":{\"value\":199},\"macAddress\":{\"value\":\"00:00:00:00:00:02\"},\"supportedFlowActions\":{\"value\":\"[Controller, Drop, Enqueue, HwPath, Output, PopVlan, SetDlDst,SetDlSrc, SetNwDst, SetNwSrc, SetNwTos, SetTpDst, SetTpSrc, SetVlanId, SetVlanPcp, SwPath]\"}}},{\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:03\"},\"properties\":{\"tables\":{\"value\":-1},\"forwarding\":{\"value\":0},\"timeStamp\":{\"value\":1388174650225,\"name\":\"connectedSince\"},\"buffers\":{\"value\":256},\"description\":{\"value\":\"None\"},\"capabilities\":{\"value\":199},\"macAddress\":{\"value\":\"00:00:00:00:00:03\"},\"supportedFlowActions\":{\"value\":\"[Controller, Drop, Enqueue, HwPath, Output, PopVlan, SetDlDst, SetDlSrc, SetNwDst, SetNwSrc, SetNwTos, SetTpDst, SetTpSrc, SetVlanId, SetVlanPcp, SwPath]\"}}}]}");
      return response_;
  } else if (ip_address_.compare(SWITCH_NODE_PROPERTIES_WRONG) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"nodeProperties\":[{\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:02\"},\"propertie\":{\"tables\":{\"value\":-1},\"forwarding\":{\"value\":0},\"timeStamp\":{\"value\":1388174650189,\"name\":\"connectedSince\"},\"buffers\":{\"value\":256},\"description\":{\"value\":\"None\"},\"capabilities\":{\"value\":199},\"macAddress\":{\"value\":\"00:00:00:00:00:02\"},\"supportedFlowActions\":{\"value\":\"[Controller, Drop, Enqueue, HwPath, Output, PopVlan, SetDlDst,SetDlSrc, SetNwDst, SetNwSrc, SetNwTos, SetTpDst, SetTpSrc, SetVlanId, SetVlanPcp, SwPath]\"}}},{\"node\":{\"type\":\"OF\",\"id\":\"00:00:00:00:00:00:00:03\"},\"properties\":{\"tables\":{\"value\":-1},\"forwarding\":{\"value\":0},\"timeStamp\":{\"value\":1388174650225,\"name\":\"connectedSince\"},\"buffers\":{\"value\":256},\"description\":{\"value\":\"None\"},\"capabilities\":{\"value\":199},\"macAddress\":{\"value\":\"00:00:00:00:00:03\"},\"supportedFlowActions\":{\"value\":\"[Controller, Drop, Enqueue, HwPath, Output, PopVlan, SetDlDst, SetDlSrc, SetNwDst, SetNwSrc, SetNwTos, SetTpDst, SetTpSrc, SetVlanId, SetVlanPcp, SwPath]\"}}}]}");
      return response_;
  }
    return NULL;
  }

  void clear_http_response() {
    if (response_ != NULL) {
      if (response_->write_data != NULL) {
        delete response_->write_data;
        response_->write_data = NULL;
      }
      delete response_;
      response_ = NULL;
    }
  }

 private:
  restjson::HttpResponse_t* response_;
  std::string ip_address_;
  std::string url_;
};
}  // namespace restjson
}  // namespace unc
#endif  // RESTJSON_REST_CLIENT_H_
