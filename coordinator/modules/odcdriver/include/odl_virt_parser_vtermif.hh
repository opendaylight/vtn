#ifndef __ODL_VTERMIF_VIRT_PARSER_HH__
#define __ODL_VTERMIF_VIRT_PARSER_HH__

#include <odl_vterm_if.hh>
#include <odl_vtermif_portmap.hh>
#include <odc_rest.hh>

namespace unc {
namespace odcdriver {

class VirtParserOdcVtermIfCmd {

public:
UncRespCode get_fill_portmap(std::string url, key_vterm_if_t &key, val_vterm_if_t &vterm_if_val, unc::driver::controller* ctr,  unc::restjson::ConfFileValues_t conf_values_, std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {

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
           pfc_log_debug("vterm portmap present : %s", data);
         }
        }

  json_object* jobj = unc::restjson::JsonBuildParse::get_json_object(data);
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json_object_is_null");
    return UNC_DRV_RC_ERR_GENERIC;
 }

VtermifportmapParser obj;
pfc_log_debug("portmap");
pfc_log_debug("entering vterm_if_parse_portmap_response");
pfc_log_debug("jobj_portmap:%s",unc::restjson::JsonBuildParse::get_json_string(jobj));
int ret_val = obj.parse_vterm_if_portmap_response(jobj,key,vterm_if_val, cfgnode_vector);
pfc_log_debug("leaving vterm_if_parse_portmap_response");
if(ret_val != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

  UncRespCode set_vterm_if_enabled(json_object *out, key_vterm_if_t &key, val_vterm_if_t &vterm_if_val) { 
    int ret_val = 0;
    if (UNC_VF_VALID == vterm_if_val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI]) {
      if (UPLL_ADMIN_ENABLE == vterm_if_val.admin_status){
        pfc_log_trace("set_vterm_if_enabled entering");
        ret_val = unc::restjson::JsonBuildParse::build("enabled", "true", out);}
      else if (UPLL_ADMIN_DISABLE == vterm_if_val.admin_status){
        pfc_log_trace("set_vterm_if_disabled entering");
        ret_val = unc::restjson::JsonBuildParse::build("enabled", "false", out);}
        
      if (restjson::REST_OP_SUCCESS != ret_val) 
        return UNC_DRV_RC_ERR_GENERIC; 
    }
   pfc_log_trace("set_vterm_if_enabled:%s", unc::restjson::JsonBuildParse::get_json_string(out));
   return UNC_RC_SUCCESS;
 }
  
  UncRespCode get_vterm_if_enabled(json_object *out, key_vterm_if_t &key, val_vterm_if_t &vterm_if_val) { 
   
    //vterm_if_val.valid[UPLL_IDX_VAL_VTERMIF] = UNC_VF_VALID;
    //vterm_if_val.valid[UPLL_IDX_VEXT_NAME_VTERMIF] = UNC_VF_INVALID;
    //vterm_if_val.valid[UPLL_IDX_VEXT_IF_NAME_VTERMIF] = UNC_VF_INVALID;
    //vterm_if_val.valid[UPLL_IDX_VLINK_NAME_VTERMIF] = UNC_VF_INVALID;

   std::string admin_status = "";
   int ret_val = unc::restjson::JsonBuildParse::parse(out,
                                                 "enabled",
                                                 -1, admin_status);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error in parsing function %s", PFC_FUNCNAME);
    return UNC_DRV_RC_ERR_GENERIC;
  }

    vterm_if_val.valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_VALID;
    if (admin_status.compare("true") == 0) {
      pfc_log_trace("admin_status.compare true");
      vterm_if_val.admin_status = UPLL_ADMIN_ENABLE;}
    else if (admin_status.compare("false") == 0)  {
      pfc_log_trace("admin_status.compare false");
      vterm_if_val.admin_status = UPLL_ADMIN_DISABLE;}
    return UNC_RC_SUCCESS;
  }
};
}
}

#endif
