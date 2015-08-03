/*
 * Copyright (c) 2014-2015 NEC Corporation
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
#include <odc_flow_conditions.hh>
#include <odc_util.hh>
#include <odc_rest.hh>
#include <unc/upll_ipc_enum.h>
#include <vtn_conf_data_element_op.hh>
#include <tclib_module.hh>
#include <vector>
#include <string>
#include <cstdlib>

namespace unc {
namespace odcdriver {


class odl_flowlist_http_request: public odl_http_rest_intf {

public:
  odl_flowlist_http_request(flowcondition *value, std::string url,
                            flowConditions *read_value, pfc_bool_t entry):
    url_(url),
    flow_condition_(value),
    flow_conditions_(read_value),
    is_entry_(entry) {}

  // Is multiple requests need to be sent to handle the request?
  pfc_bool_t is_multiple_requests(unc::odcdriver::OdcDriverOps Op) {
    return PFC_FALSE;
  }

// Provide a list of request indicators for multiple request scenario.
  UncRespCode get_multi_request_indicator(
    unc::odcdriver::OdcDriverOps Op,
    std::set<std::string> *arg_list) {
    ODC_FUNC_TRACE;
    return UNC_RC_SUCCESS;
  }

// Construct the URL for the operation
// The request indicator will ne SINGLE as default
  UncRespCode construct_url(unc::odcdriver::OdcDriverOps Op,
                            std::string &request_indicator,
                            std::string &url) {
    ODC_FUNC_TRACE;
    url=url_;
    return UNC_RC_SUCCESS;
  }

// Construct Request body for the operation
  UncRespCode construct_request_body(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     json_object *object) {
    ODC_FUNC_TRACE;
      UncRespCode ret = UNC_RC_SUCCESS;
    if ( Op != unc::odcdriver::CONFIG_READ &&
        Op != unc::odcdriver::CONFIG_DELETE ) {
      unc::odcdriver::flowConditionsUtil util_;
      if ( is_entry_ == PFC_FALSE ) {
        ret=util_.SetValue(object,flow_condition_);
      } else {
        std::list <match*>::iterator iter_;
        iter_ = flow_condition_->match_.begin();
        if ( iter_ != flow_condition_->match_.end() && (*iter_ == NULL) ) {
          pfc_log_error("No Contents in match");
        } else {
          UncRespCode ret(util_.SetValue(object,*iter_));
          if ( ret != UNC_RC_SUCCESS ) {
            pfc_log_error("Failed to Copy from flowcondtion to json!!");
            return ret;
          }
        }
      }
    } else {
      pfc_log_info("READ or delete");
    }
    return ret;
  }
// Return the HTTP operation intended
  restjson::HttpMethod get_http_method(
    unc::odcdriver::OdcDriverOps Op,
    std::string &request_indicator) {

    ODC_FUNC_TRACE;
    if ( Op == unc::odcdriver::CONFIG_READ )
      return unc::restjson::HTTP_METHOD_GET;
    if ( Op == unc::odcdriver::CONFIG_CREATE )
      return unc::restjson::HTTP_METHOD_POST;
    if ( Op == unc::odcdriver::CONFIG_DELETE)
      return unc::restjson::HTTP_METHOD_DELETE;
    if ( Op == unc::odcdriver::CONFIG_UPDATE)
      return unc::restjson::HTTP_METHOD_PUT;
    return unc::restjson::HTTP_METHOD_GET;
  }

// Validate the response code handed
  UncRespCode validate_response_code(unc::odcdriver::OdcDriverOps Op,
                                     std::string &request_indicator,
                                     int resp_code) {
    ODC_FUNC_TRACE;

    pfc_log_info("the Response code is %d",resp_code);
    if (HTTP_200_RESP_OK != resp_code &&
        HTTP_201_RESP_CREATED != resp_code &&
        HTTP_204_NO_CONTENT != resp_code) {
      return UNC_DRV_RC_ERR_GENERIC;
    }
    return UNC_RC_SUCCESS;
  }

// Read the response of the http request from data
  UncRespCode handle_response(unc::odcdriver::OdcDriverOps Op,
                              std::string &request_indicator,
                              char* data) {
    ODC_FUNC_TRACE;
    if ( Op == unc::odcdriver::CONFIG_READ ) {
      unc::odcdriver::flowConditionsUtil util_;
      json_object *parse(unc::restjson::JsonBuildParse::get_json_object(data));
      if ( parse != NULL ) {
        unc::restjson::json_obj_destroy_util delete_obj(parse);
        UncRespCode ret(util_.GetValue( parse, flow_conditions_));
        if ( ret != UNC_RC_SUCCESS )
          return ret;
      } else {
        return UNC_DRV_RC_ERR_GENERIC;
      }
    }
    return UNC_RC_SUCCESS;
  }

private:
  std::string url_;
  flowcondition* flow_condition_;
  flowConditions* flow_conditions_;
  pfc_bool_t is_entry_;
};

template <typename key,typename value>
class OdcFlowConditionCmd {

private:
  unc::restjson::ConfFileValues_t conf_values_;
  pfc_bool_t is_entry_;

public:
  OdcFlowConditionCmd(unc::restjson::ConfFileValues_t conf_values,
                      pfc_bool_t is_entry):
    conf_values_(conf_values),
    is_entry_(is_entry) {}

  ~OdcFlowConditionCmd() {}

  virtual std::string get_url_tail(key &key_in,value &val_in)=0;

  // Copy from Key Value Structure to FlowConditions Structure
  virtual void copy(flowcondition *out, key &key_in, value &value_val_in) = 0;
  virtual void copy(flowcondition* out, key &key_in,
                           value &value_old_in, value &value_new_in)=0;

  // Copy from FlowConditions List and send to platform
  virtual UncRespCode r_copy(flowConditions *in,
                             std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) =0;

  UncRespCode run_command(key& key_in,
                          value& val_in,
                          unc::driver::controller *ctr,
                          unc::odcdriver::OdcDriverOps Op) {
    ODC_FUNC_TRACE;
    flowcondition command_;
    //Copy Contents from key and val
    copy(&command_,key_in,val_in);

    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append("/");
    url.append("flowconditions");
    url.append("/");
    url.append(get_url_tail(key_in,val_in));

    pfc_log_info("The Flow list URL: %s",url.c_str());

    odl_flowlist_http_request flow_cond_request(&command_,url, NULL, is_entry_);

    odl_http_request odl_fc_create;
    return odl_fc_create.handle_request(ctr,
                                        Op,
                                        &flow_cond_request,
                                        conf_values_);
  }
  UncRespCode run_command(key& key_in,
                          value& val_old_in,
                          value& val_new_in,
                          unc::driver::controller *ctr,
                          unc::odcdriver::OdcDriverOps Op) {
    ODC_FUNC_TRACE;
    flowcondition command_;
    //Copy Contents from key and val
    copy(&command_,key_in,val_old_in, val_new_in);

    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append("/");
    url.append("flowconditions");
    url.append("/");
    url.append(get_url_tail(key_in,val_new_in));
    pfc_log_info("The Flow list URL: %s",url.c_str());

    odl_flowlist_http_request flow_cond_request(&command_,url, NULL, is_entry_);


    odl_http_request odl_fc_create;
    return odl_fc_create.handle_request(ctr,
                                        Op,
                                        &flow_cond_request,
                                        conf_values_);
  }
  UncRespCode odl_flow_condition_read_all( unc::driver::controller *ctr,
      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    ODC_FUNC_TRACE;
    flowConditions read_all_;
    std::string url = "";
    url.append(BASE_URL);
    url.append(CONTAINER_NAME);
    url.append("/");
    url.append("flowconditions");
    pfc_log_info("The Flow list URL: %s",url.c_str());

    odl_flowlist_http_request flow_cond_request(NULL,url,&read_all_,is_entry_);
    odl_http_request odl_fc_create;
    UncRespCode ret (odl_fc_create.handle_request(ctr,
                     unc::odcdriver::CONFIG_READ,
                     &flow_cond_request,
                     conf_values_));
    if ( ret != UNC_RC_SUCCESS )
      return ret;

    return r_copy(&read_all_,cfgnode_vector);
  }

};


class OdcFlowListCommand: public
  unc::driver::vtn_driver_command <key_flowlist, val_flowlist>,
  unc::odcdriver::OdcFlowConditionCmd<key_flowlist,val_flowlist>

{
public:
  /**
   * @brief                          - Parametrised Constructor
   * @param[in]                      - conf file values
   */
  explicit OdcFlowListCommand(unc::restjson::ConfFileValues_t conf_values):
    OdcFlowConditionCmd<key_flowlist,val_flowlist>(conf_values,PFC_FALSE),
    conf_file_values_(conf_values) {}

  /**
   * @brief Default Destructor
   */
  ~OdcFlowListCommand() {}

  UncRespCode create_cmd(key_flowlist &key_in, val_flowlist &val_in,
                         unc::driver::controller *ctr) {
    ODC_FUNC_TRACE;
    return run_command(key_in,
                       val_in,
                       ctr,
                       unc::odcdriver::CONFIG_UPDATE);
    return UNC_RC_SUCCESS;
  }

  UncRespCode delete_cmd(key_flowlist &key_in, val_flowlist &val_in,
                         unc::driver::controller *ctr) {
    ODC_FUNC_TRACE;
    return run_command(key_in,
                       val_in,
                       ctr,
                       unc::odcdriver::CONFIG_DELETE);
  }


  UncRespCode update_cmd(key_flowlist &key_in, val_flowlist &val_old_in,
                         val_flowlist &val_new_in,
                         unc::driver::controller *ctr) {
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
    return odl_flow_condition_read_all(ctr,cfgnode_vector);
  }

  void copy (flowcondition* out,
             key_flowlist& in_key,
             val_flowlist& in_val);

  void copy (flowcondition* out,
             key_flowlist& in_key,
             val_flowlist& old_val,
             val_flowlist& new_val);
  UncRespCode r_copy (flowConditions *in,
                      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector);

  std::string get_url_tail(key_flowlist& in_key,
                           val_flowlist& in_val);

private:
  unc::restjson::ConfFileValues_t conf_file_values_;
};

class OdcFlowListEntryCommand: public
  unc::driver::vtn_driver_command <key_flowlist_entry,
  val_flowlist_entry>,
  unc::odcdriver::OdcFlowConditionCmd<key_flowlist_entry,
  val_flowlist_entry>

{
public:
  /**
   * @brief                          - Parametrised Constructor
   * @param[in]                      - conf file values
   */
  explicit OdcFlowListEntryCommand(unc::restjson::ConfFileValues_t conf_values):
    OdcFlowConditionCmd<key_flowlist_entry, val_flowlist_entry>(conf_values, PFC_TRUE),
    conf_file_values_(conf_values) {}

  /**
   * @brief Default Destructor
   */
  ~OdcFlowListEntryCommand() {}

  UncRespCode create_cmd(key_flowlist_entry &key_in,
                         val_flowlist_entry &val_in,
                         unc::driver::controller *ctr) {
    ODC_FUNC_TRACE;
    return run_command(key_in,
                       val_in,
                       ctr,
                       unc::odcdriver::CONFIG_UPDATE);
  }

  UncRespCode delete_cmd(key_flowlist_entry &key_in,
                         val_flowlist_entry &val_in,
                         unc::driver::controller *ctr) {
    ODC_FUNC_TRACE;
    return run_command(key_in,
                       val_in,
                       ctr,
                       unc::odcdriver::CONFIG_DELETE);
  }


  UncRespCode update_cmd(key_flowlist_entry &key_in,
                         val_flowlist_entry &val_old_in,
                         val_flowlist_entry &val_new_in,
                         unc::driver::controller *ctr) {
    ODC_FUNC_TRACE;
    return run_command(key_in,
                       val_old_in,
                       val_new_in,
                       ctr,
                       unc::odcdriver::CONFIG_UPDATE);
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

    return UNC_RC_SUCCESS;
  }

  void copy (flowcondition* out,
             key_flowlist_entry& in_key,
             val_flowlist_entry& in_val);

  void copy (flowcondition* out,
             key_flowlist_entry& in_key,
             val_flowlist_entry& old_val,
             val_flowlist_entry& new_val);
  UncRespCode r_copy (flowConditions *in,
                      std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    return UNC_RC_SUCCESS;
  }

  std::string get_url_tail(key_flowlist_entry& in_key,
                           val_flowlist_entry& in_val);

private:
  unc::restjson::ConfFileValues_t conf_file_values_;
};

}  // namespace odcdriver
}  // namespace unc
#endif
