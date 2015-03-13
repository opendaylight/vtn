/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _ODC_FLOWLIST_HH_
#define _ODC_FLOWLIST_HH_

#include <rest_util.hh>
#include <driver/driver_command.hh>
#include <odc_driver_common_defs.hh>
#include <odc_controller.hh>
#include <odc_util.hh>
#include <odc_rest.hh>
#include <odl_flowlist.hh>
#include <odl_flowlist_entry.hh>
#include <unc/upll_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <tclib_module.hh>
#include <vector>
#include <string>
#include <cstdlib>

namespace unc {
namespace odcdriver {

class OdcFlowListCommand: public
  unc::driver::vtn_driver_command <key_flowlist, val_flowlist>,
  public FlowlistParser 
{
public:
  /**
   * @brief                          - Parametrised Constructor
   * @param[in]                      - conf file values
   */
  OdcFlowListCommand(unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values) {
    pfc_log_trace("Flowlist Cons");
  }

  /**
   * @brief Default Destructor
   */
  ~OdcFlowListCommand() {}

  UncRespCode create_cmd(key_flowlist &key_in, val_flowlist &val_in,
                         unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    pfc_log_trace("In function create_cmd of flowlist");
    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append("/");
    url.append("flowconditions");
    url.append("/");
    url.append(get_url_tail(key_in, val_in));

    json_object *jobj_req_body = unc::restjson::JsonBuildParse::create_json_obj();
    pfc_log_debug("entering in to create flowlist request.....");
    int retval = create_flowlist_request(jobj_req_body, key_in, val_in);
    pfc_log_debug("leaving from create flowlist request////");
    if (retval != UNC_RC_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;

   pfc_log_trace("flowlist req_body:%s", unc::restjson::JsonBuildParse::get_json_string(jobj_req_body));
    
    /*unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());

    unc::restjson::HttpResponse_t* response =
         rest_util_obj.send_http_request(
         url, restjson::HTTP_METHOD_PUT,
         unc::restjson::JsonBuildParse::get_json_string(jobj_req_body),
         conf_values_);*/
   //FlowlistParser obj;
   int resp_code = send_httprequest(ctr_ptr,url,conf_values_,restjson::HTTP_METHOD_PUT,jobj_req_body);
    json_object_put(jobj_req_body);
    if (0 == resp_code) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    //int resp_code = response->code;
    pfc_log_debug("response code returned in create  is %d", resp_code);
    if (HTTP_201_RESP_CREATED != resp_code)
      return UNC_DRV_RC_ERR_GENERIC;
    return UNC_RC_SUCCESS;
  }

  UncRespCode delete_cmd(key_flowlist &key_in, val_flowlist &val_in,
                         unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append("/");
    url.append("flowconditions");
    url.append("/");
    url.append(get_url_tail(key_in, val_in));

    /*unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());

    unc::restjson::HttpResponse_t* response =
         rest_util_obj.send_http_request(
         url, restjson::HTTP_METHOD_DELETE, NULL,
         conf_values_);*/
   int resp_code = send_httprequest(ctr_ptr,url,conf_values_,restjson::HTTP_METHOD_DELETE,NULL);

    if (0 == resp_code) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    //int resp_code = response->code;
    pfc_log_debug("response code returned in create vtn is %d", resp_code);
    if (HTTP_200_RESP_OK != resp_code)
      return UNC_DRV_RC_ERR_GENERIC;
    return UNC_RC_SUCCESS;
  }


  UncRespCode update_cmd(key_flowlist &key_in, val_flowlist &val_in,
                         unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }
  /**
   * @brief      - Method to fetch child configurations for the parent kt
   * @param[in]  - controller pointer
   * @param[in]  - parent key type pointer
   * @param[out] - list of configurations
   * @retval     - UNC_RC_SUCCESS / UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode fetch_config(
    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    ODC_FUNC_TRACE;
    std::string parent_flowlist_name = "flowlist1";
    key_flowlist key_in;
    val_flowlist val_in;
    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append("/");
    url.append("flowconditions");
    pfc_log_info("The Flow list URL: %s",url.c_str());
 
    unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                          ctr->get_user_name(), ctr->get_pass_word());
    unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
                  url, restjson::HTTP_METHOD_GET, NULL,  conf_values_);

    if (NULL == response) {
      pfc_log_error("Error Occured while getting httpresponse -- ");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    int resp_code = response->code;
    if (HTTP_200_RESP_OK != resp_code) {
      pfc_log_error("%d error resp ", resp_code);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    if (NULL != response->write_data) {
      if (NULL != response->write_data->memory) {
        char *data = response->write_data->memory;
        pfc_log_debug("flowlist present : %s", data);
        json_object* jobj1 = restjson::JsonBuildParse::get_json_object(data);
        json_object* jobj2 = restjson::JsonBuildParse::get_json_object(data);
        pfc_log_debug("entering in to parse_flowlist_response");
        UncRespCode ret_val = parse_flowlist_response(jobj1,key_in,val_in, cfgnode_vector);
        pfc_log_debug("leaving from parse_flowlist_response");
        if (ret_val != UNC_RC_SUCCESS)
         {
           pfc_log_debug("error in parsing");
           return UNC_DRV_RC_ERR_GENERIC;
         }
        key_flowlist_entry key;
        val_flowlist_entry val;
        /*key_flowlist_entry_t * parent_flowlist_key = reinterpret_cast<key_flowlist_entry_t*> (parent_key);
        parent_flowlist_name.assign(reinterpret_cast<char*>(parent_flowlist_key->flowlist_key.flowlist_name));*/
        pfc_log_debug("entering in to parse_flowlist_entry_response");
        FlowlistEntryParser obj;
        pfc_log_debug("json_obj_after flowlist call:%s", unc::restjson::JsonBuildParse::get_json_string(jobj2));
        //FlowlistEntryParser obj(parent_flowlist_name);
        UncRespCode retval = obj.parse_flowlist_entry_response(jobj2,key,val,cfgnode_vector);
        pfc_log_debug("leaving from parse_flowlist_entry_response");
        if (retval != UNC_RC_SUCCESS)
        {
        pfc_log_debug("error in parsing");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      }
    }
    return UNC_RC_SUCCESS;
  }

std::string get_url_tail(key_flowlist& key,
                                 val_flowlist& val) {
  ODC_FUNC_TRACE;
  char *flowlist_name=reinterpret_cast <char *>(key.flowlist_name);
  std::string url_string (flowlist_name);
  return url_string;
}

private:
  unc::restjson::ConfFileValues_t conf_values_;
};

class OdcFlowListEntryCommand: public
  unc::driver::vtn_driver_command <key_flowlist_entry,
  val_flowlist_entry>, public FlowlistEntryParser
{
public:
  /**
   * @brief                          - Parametrised Constructor
   * @param[in]                      - conf file values
   */
  OdcFlowListEntryCommand(unc::restjson::ConfFileValues_t conf_values):
    conf_values_(conf_values) {
    pfc_log_trace("Flowlistentry Cons");
}

  /**
   * @brief Default Destructor
   */
  ~OdcFlowListEntryCommand() {}

  UncRespCode create_cmd(key_flowlist_entry &key_in,
                         val_flowlist_entry &val_in,
                         unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    pfc_log_trace("In flowlistentry create_cmd");
    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append("/");
    url.append("flowconditions");
    url.append("/");
    url.append(get_url_tail(key_in, val_in));
    
    json_object *jobj_req_body = unc::restjson::JsonBuildParse::create_json_obj();
    pfc_log_debug("create.........create");
    int retval = create_flowlist_entry_request(jobj_req_body, key_in, val_in);
    pfc_log_debug("create .....returned");
    if (retval != UNC_RC_SUCCESS)
      return UNC_DRV_RC_ERR_GENERIC;
    pfc_log_debug("url.append:%s",url.c_str());
   
    pfc_log_trace("flowlist req_body:%s", unc::restjson::JsonBuildParse::get_json_string(jobj_req_body));
    
    /*unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());

    unc::restjson::HttpResponse_t* response =
         rest_util_obj.send_http_request(
         url, restjson::HTTP_METHOD_PUT,
         unc::restjson::JsonBuildParse::get_json_string(jobj_req_body),
         conf_values_);*/
    int resp_code = send_httprequest(ctr_ptr,url,conf_values_,restjson::HTTP_METHOD_PUT, jobj_req_body);

    json_object_put(jobj_req_body);
    if (0 == resp_code) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    //int resp_code = response->code;
    pfc_log_debug("response code returned in create  is %d", resp_code);
    if (HTTP_201_RESP_CREATED!= resp_code)
      return UNC_DRV_RC_ERR_GENERIC;
    return UNC_RC_SUCCESS;
  }

  UncRespCode delete_cmd(key_flowlist_entry &key_in,
                         val_flowlist_entry &val_in,
                         unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append("/");
    url.append("flowconditions");
    url.append("/");
    url.append(get_url_tail(key_in, val_in));
    
    /*unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                  ctr_ptr->get_user_name(), ctr_ptr->get_pass_word());

    unc::restjson::HttpResponse_t* response =
         rest_util_obj.send_http_request(
         url, restjson::HTTP_METHOD_DELETE, NULL,
         conf_values_);*/
   int resp_code = send_httprequest(ctr_ptr,url,conf_values_,restjson::HTTP_METHOD_DELETE,NULL);

    if (0 == resp_code) {
      pfc_log_error("Error Occured while getting httpresponse");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    //int resp_code = response->code;
    pfc_log_debug("response code returned in delete is %d", resp_code);
    if (HTTP_200_RESP_OK != resp_code)
      return UNC_DRV_RC_ERR_GENERIC;
    return UNC_RC_SUCCESS;
  }


  UncRespCode update_cmd(key_flowlist_entry &key_in,
                         val_flowlist_entry &val_in,
                         unc::driver::controller *ctr_ptr) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }
  /**
   * @brief      - Method to fetch child configurations for the parent kt
   * @param[in]  - controller pointer
   * @param[in]  - parent key type pointer
   * @param[out] - list of configurations
   * @retval     - UNC_RC_SUCCESS / UNC_DRV_RC_ERR_GENERIC
   */
  UncRespCode fetch_config(
    unc::driver::controller* ctr_ptr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    ODC_FUNC_TRACE;
/*    unc::driver::controller* ctr,
    void* parent_key,
    std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    ODC_FUNC_TRACE;
    key_flowlist key_in;
    val_flowlist val_in;
    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append("/");
    url.append("flowconditions");
    url.append(get_url_tail);
    pfc_log_info("The Flow list URL: %s",url.c_str());
 
    unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                          ctr->get_user_name(), ctr->get_pass_word());
    unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(
                  url, restjson::HTTP_METHOD_GET, NULL, conf_values_);

    if (NULL == response) {
      pfc_log_error("Error Occured while getting httpresponse -- ");
      return UNC_DRV_RC_ERR_GENERIC;
    }
    int resp_code = response->code;
    if (HTTP_200_RESP_OK != resp_code) {
      pfc_log_error("%d error resp ", resp_code);
      return UNC_DRV_RC_ERR_GENERIC;
    }
    if (NULL != response->write_data) {
      if (NULL != response->write_data->memory) {
        char *data = response->write_data->memory;
        pfc_log_debug("flowlist present : %s", data);
        json_object* jobj = restjson::JsonBuildParse::get_json_object(data);
       pfc_log_debug("entering in to parse_flowlist_response");
        UncRespCode ret_val = parse_flowlist_response(jobj,key_in,val_in, cfgnode_vector);
        pfc_log_debug("leaving from parse_flowlist_response");
        if (ret_val != UNC_RC_SUCCESS)
         {
           pfc_log_debug("error in parsing");
           return UNC_DRV_RC_ERR_GENERIC;
     }
        key_flowlist_entry key;
        val_flowlist_entry val;
        pfc_log_debug("entering in to parse_flowlist_entry_response");
        FlowlistEntryParser obj;
        UncRespCode retval = obj.parse_flowlist_entry_response(jobj,key,val, cfgnode_vector);
        pfc_log_debug("leaving from parse_flowlist_entry_response");
        if (retval != UNC_RC_SUCCESS)
         {
          pfc_log_debug("error in parsing");
          return UNC_DRV_RC_ERR_GENERIC;
      }
      }
    } */
    return UNC_RC_SUCCESS;
  }


std::string get_url_tail(key_flowlist_entry& key,
                                      val_flowlist_entry& val) {
     ODC_FUNC_TRACE;
     char *flowlist_name=reinterpret_cast <char *>(key.flowlist_key.flowlist_name);
     std::string url_string ("");
     url_string.append(flowlist_name);
     url_string.append("/");
     char sequence_no[10];
     sprintf(sequence_no,"%d",key.sequence_num);
     url_string.append(sequence_no);
     return url_string;
}

private:
  unc::restjson::ConfFileValues_t conf_values_;
};

}  // namespace odcdriver
}  // namespace unc
#endif
