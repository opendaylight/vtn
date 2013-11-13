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

class RestClient {
 public:
  RestClient(std::string& ipaddress,
             std::string& url,
             uint32_t port,
             HttpMethod method)
      : ip_address_(ipaddress),
      url_(url),
      port_(port),
      method_(method) {
        response_ = new HttpResponse_t;
        response_->code = 0;
        response_->write_data = new HttpContent_t;
      }

  ~RestClient() {
    if (response_ !=  NULL) {
     if (response_->write_data != NULL) {
       delete response_->write_data;
       response_->write_data = NULL;
      }
      delete response_;
      response_ =  NULL;
    }
  }

  HttpResponse_t* send_http_request(std::string username,
                                    std::string password,
                                    uint32_t connect_time_out,
                                    uint32_t req_time_out,
                                    const char* request_body) {
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


    if (ip_address_.compare(NULL_RESPONSE) ==  0) {
      return NULL;
    } else if (ip_address_.compare(INVALID_RESPONSE) ==  0) {
      response_->code = 1;
      return response_;
    }  else if (ip_address_.compare(CREATE_201) == 0) {
      response_->code = 201;
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
      std::cout << "-------444------" << url_;
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
  uint32_t port_;
  HttpMethod method_;
};
}  // namespace restjson
}  // namespace unc
#endif  // RESTJSON_REST_CLIENT_H_
