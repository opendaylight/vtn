#ifndef __ODL_VBRIF_VIRT_PARSER_HH__
#define __ODL_VBRIF_VIRT_PARSER_HH__

#include <odl_vbrif.hh>
#include <odl_vbrif_portmap.hh>
#include <odc_rest.hh>

namespace unc {
namespace odcdriver {

class VirtParserOdcVbrIfCmd {

public:
UncRespCode get_fill_portmap(std::string url, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val, unc::driver::controller* ctr,  unc::restjson::ConfFileValues_t conf_values_, std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
 
  url.append("/");
  url.append((char *)key.if_name);
  url.append("/portmap");

  unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),
                                 ctr->get_user_name(), ctr->get_pass_word());
  unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(url, restjson::HTTP_METHOD_GET, NULL, conf_values_);
  if (NULL == response)
  {
    pfc_log_error("Error Occured while getting httpresponse:no portmap");
    return  UNC_DRV_RC_ERR_GENERIC;
  }
 char *data = NULL;
        if (NULL != response->write_data) {
         if (NULL != response->write_data->memory) {
           data = response->write_data->memory;
           pfc_log_debug("vtns present : %s", data);
         }
        }

  json_object* jobj = unc::restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_null");
    return UNC_DRV_RC_ERR_GENERIC;
 }

VbrifportmapParser obj;
pfc_log_debug("portmap");
pfc_log_debug("entering vbr_if_parse_portmap_response");
int ret_val = obj.parse_vbr_if_portmap_response(jobj,key,val, cfgnode_vector);
pfc_log_debug("leaving vbr_if_parse_portmap_response");
if(ret_val != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

  UncRespCode set_vbr_if_enabled(json_object *out, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) { 
    int ret_val = 0;
    if (UNC_VF_VALID == val.val_vbrif.valid[UPLL_IDX_ADMIN_STATUS_VBRI]) {
      pfc_log_trace("set_vbr_if_enabled entering");
      if (UPLL_ADMIN_ENABLE == val.val_vbrif.admin_status) 
        ret_val = unc::restjson::JsonBuildParse::build("enabled", "true", out);
      else if (UPLL_ADMIN_DISABLE == val.val_vbrif.admin_status) 
        ret_val = unc::restjson::JsonBuildParse::build("enabled", "false", out);
        
      if (restjson::REST_OP_SUCCESS != ret_val) 
        return UNC_DRV_RC_ERR_GENERIC; 
    }
   pfc_log_trace("set_vbr_if_enabled:%s", unc::restjson::JsonBuildParse::get_json_string(out));
   return UNC_RC_SUCCESS;
 }
  
  UncRespCode get_vbr_if_enabled(json_object *out, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) { 
   
    val.valid[PFCDRV_IDX_VAL_VBRIF] = UNC_VF_VALID;
    val.valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_INVALID;
    val.valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] = UNC_VF_INVALID;
    val.valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_INVALID;

   std::string admin_status = "";
   int ret_val = unc::restjson::JsonBuildParse::parse(out,
                                                 "enabled",
                                                 -1, admin_status);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error in parsing function %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

    val.val_vbrif.valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
    if (admin_status.compare("true") == 0) 
      val.val_vbrif.admin_status = UPLL_ADMIN_ENABLE;
    else if (admin_status.compare("false") == 0) 
      val.val_vbrif.admin_status = UPLL_ADMIN_DISABLE;
    return UNC_RC_SUCCESS;
  }
};
}
}

#endif
