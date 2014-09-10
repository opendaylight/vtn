/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <odc_rest.hh>

namespace unc {
namespace odcdriver {

UncRespCode odl_http_request::handle_request_internal(
    unc::driver::controller *ctr_ptr,
    unc::odcdriver::OdcDriverOps Op,
    unc::odcdriver::odl_http_rest_intf *handler,
    unc::restjson::ConfFileValues_t conf_values,
    std::string &request_indicator) {

  ODC_FUNC_TRACE;
  unc::restjson::RestUtil rest_util_obj(ctr_ptr->get_host_address(),
                                        ctr_ptr->get_user_name(),
                                        ctr_ptr->get_pass_word());

  std::string url("");
  UncRespCode resp_url(handler->construct_url(Op, request_indicator, url));

  pfc_log_debug("the URL is %s", url.c_str());

  if ( resp_url != UNC_RC_SUCCESS )
    return resp_url;

  json_object* jobj_req_body(unc::restjson::JsonBuildParse::create_json_obj());
  UncRespCode resp_reqbody((handler->construct_request_body(Op,
                                                            request_indicator,
                                                            jobj_req_body)));

  if ( resp_reqbody != UNC_RC_SUCCESS )
    return resp_reqbody;


  if ((json_object_is_type(jobj_req_body, json_type_null))) {
    pfc_log_debug("No request body");
  }
  unc::restjson::json_obj_destroy_util delete_obj(jobj_req_body);


  restjson::HttpMethod method(handler->get_http_method(Op,
                                                       request_indicator));

  unc::restjson::HttpResponse_t* response =
      rest_util_obj.send_http_request(
          url,
          method,
          restjson::JsonBuildParse::get_json_string(jobj_req_body),
          conf_values);
  UncRespCode resp_respcode(handler->validate_response_code(Op,
                                                            request_indicator,
                                                            response->code));

  if ( resp_respcode != UNC_RC_SUCCESS )
    return resp_respcode;


  if (NULL != response->write_data) {
    if (NULL != response->write_data->memory) {
      pfc_log_debug("Data Exists");
      // char *data=response->write_data->memory;
      pfc_log_info("Data REceived : %s", response->write_data->memory);
      UncRespCode resp_parse(handler->handle_response(
              Op,
              request_indicator,
              response->write_data->memory));
      if ( resp_parse != UNC_RC_SUCCESS )
        return resp_parse;
    }
  }

  pfc_log_info("Handle REquest Internal Success");
  return UNC_RC_SUCCESS;
}

UncRespCode odl_http_request::handle_request(
    unc::driver::controller *ctr_ptr,
    unc::odcdriver::OdcDriverOps Op,
    unc::odcdriver::odl_http_rest_intf *handler,
    unc::restjson::ConfFileValues_t conf_values) {
  ODC_FUNC_TRACE;
  std::string def_indicator("SINGLE");
  if (!handler) {
    return UNC_DRV_RC_ERR_GENERIC;
  }

  if (handler->is_multiple_requests(Op) != PFC_TRUE) {
    return handle_request_internal(ctr_ptr,
                                   Op,
                                   handler,
                                   conf_values,
                                   def_indicator);
  }

  std::set<std::string> indicators;
  UncRespCode resp(handler->get_multi_request_indicator(Op, &indicators));

  if ( resp != UNC_RC_SUCCESS )
    return resp;

  std::string list_entry("");
  for ( std::set<std::string>::iterator iter = indicators.begin() ;
       iter != indicators.end() ; iter++ ) {
    list_entry=*iter;
    UncRespCode resp(handle_request_internal(ctr_ptr,
                                             Op, handler,
                                             conf_values,
                                             list_entry));
    if (resp != UNC_RC_SUCCESS) {
      pfc_log_info("HandleREquest Failed");
      return resp;
    }
  }

  pfc_log_info("HandleREquest Suuceess");
  return UNC_RC_SUCCESS;
}


}  //  namespace odcdriver
}  //  namespace unc

