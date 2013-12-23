/*
 * Copyright (c) 2013 NEC Corporation
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

    } else if (ip_address_.compare (VLAN_MAP_VLAN_INCORRECT_RESP) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vlanmap\":[{ \"id\":\"23}]}");
      return response_;
    } else if (ip_address_.compare (VLAN_MAP_RESP) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vlanmap\":[{ \"id\":\"OF-00:00:00:00:00:00:00:03.0\", \"vlan\":\"0\",\"node\": {\"type\":\"OF\", \"id\": \"00:00:00:00:00:00:00:03\"}}, {\"id\":\"ANY.7\", \"vlan\":\"7\"}]}");
      return response_;
    } else if (ip_address_.compare (VLAN_MAP_RESP_ANY_0) == 0) {
      response_->code = 200;
      response_->write_data->memory = const_cast<char *>("{\"vlanmap\":[{ \"id\":\"OF-00:00:00:00:00:00:00:03.10\", \"vlan\":\"10\",\"node\": {\"type\":\"OF\", \"id\": \"00:00:00:00:00:00:00:03\"}}, {\"id\":\"ANY.0\", \"vlan\":\"0\"}]}");
      return response_;
    } else if(ip_address_.compare(NULL_RESP_DATA) == 0) {
      response_->code = 200;
      delete response_->write_data;
      response_->write_data = NULL;
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
