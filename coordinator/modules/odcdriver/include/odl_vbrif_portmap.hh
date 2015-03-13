#ifndef __ODL_VBRIF_PORTMAP_HH__
#define __ODL_VBRIF_PORTMAP_HH__
#include <unc/upll_ipc_enum.h>
#include <unc/pfcdriver_ipc_enum.h>
#include <odc_rest.hh>
#include <odl_virt_parser_vbrif_portmap.hh>
namespace unc {
namespace odcdriver{

class VbrifportmapParser {
public:
VbrifportmapParser() {}
~VbrifportmapParser() {}

UncRespCode create_vbr_if_portmap_request(json_object *out, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
pfc_log_info("entering in to create_vbr_if_portmap_request");
int retval = 1;
retval = set_node(out, key, val);
if (retval != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
retval = set_port(out, key, val);
if (retval != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
retval = set_vlan(out, key, val);
if (retval != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
pfc_log_info("leaving from create_vbr_if_portmap_request");
}

UncRespCode parse_vbr_if_portmap_response(json_object *in, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val, std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector ) {
pfc_log_info("entering in to parse_vbr_if_portmap_response");
int retval = 1;
uint32_t array_length = 0;
pfc_log_debug("arr_obj:%s", unc::restjson::JsonBuildParse::get_json_string(in));
json_object *json_obj_portmap = NULL;
uint32_t ret_val = unc::restjson::JsonBuildParse::parse(in, "portmap", -1, json_obj_portmap );
if(restjson::REST_OP_SUCCESS != ret_val)
{
pfc_log_error("Error occured in parsing %s" ,PFC_FUNCNAME);
return UNC_DRV_RC_ERR_GENERIC;
}
if (json_object_is_type(json_obj_portmap, json_type_array))
{
array_length = unc::restjson::JsonBuildParse::get_array_length(in, "portmap");
}
for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++)
{
key_vbr_if_t key;
pfcdrv_val_vbr_if_t val;
memset(&key, 0 , sizeof(key_vbr_if_t));
memset(&val, 0 , sizeof(pfcdrv_val_vbr_if_t));
json_object *json_obj_portmap_idx = NULL;
get_array_idx(&json_obj_portmap_idx, json_obj_portmap, arr_idx);
pfc_log_debug("arr_idx:%s", unc::restjson::JsonBuildParse::get_json_string(json_obj_portmap_idx));
retval = get_node(json_obj_portmap_idx, key, val);
if (retval != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
retval = get_port(json_obj_portmap_idx, key, val);
if (retval != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
retval = get_vlan(json_obj_portmap_idx, key, val);
if (retval != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
}
pfc_log_info("leaving from parse_vbr_if_portmap_response");
return UNC_RC_SUCCESS;
}

virtual UncRespCode set_type(json_object *out, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
VirtParserVbrifPortmapCmd obj;
int ret_val = obj.set_type(out, key ,val);
if (ret_val != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

virtual UncRespCode set_id(json_object *out, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
VirtParserVbrifPortmapCmd obj;
int ret_val = obj.set_id(out, key ,val);
if (ret_val != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

UncRespCode set_node(json_object *out, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
json_object *node = unc::restjson::JsonBuildParse::create_json_obj();
int ret_val = 0;
ret_val = set_type(node, key, val);
if (UNC_RC_SUCCESS != ret_val)
return UNC_DRV_RC_ERR_GENERIC;
ret_val = set_id(node, key, val);
if (UNC_RC_SUCCESS != ret_val)
return UNC_DRV_RC_ERR_GENERIC;
ret_val = unc::restjson::JsonBuildParse::build("node", node, out);
if (restjson::REST_OP_SUCCESS != ret_val) {
pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
return UNC_DRV_RC_ERR_GENERIC;
}
return UNC_RC_SUCCESS;
}

virtual UncRespCode get_type(json_object *in, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
VirtParserVbrifPortmapCmd obj;
int ret_val = obj.get_type(in, key ,val);
if (ret_val != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

virtual UncRespCode get_id(json_object *in, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
VirtParserVbrifPortmapCmd obj;
int ret_val = obj.get_id(in, key ,val);
if (ret_val != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

UncRespCode get_node(json_object *in, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
json_object *node = NULL;
int ret_val = unc::restjson::JsonBuildParse::parse(in, "node", -1, node);
ret_val = get_type(node, key, val);
if (UNC_RC_SUCCESS != ret_val)
return UNC_DRV_RC_ERR_GENERIC;
ret_val = get_id(node, key, val);
if (UNC_RC_SUCCESS != ret_val)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

virtual UncRespCode set_name(json_object *out, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
VirtParserVbrifPortmapCmd obj;
int ret_val = obj.set_name(out, key ,val);
if (ret_val != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

UncRespCode set_port(json_object *out, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
json_object *port = unc::restjson::JsonBuildParse::create_json_obj();
int ret_val = 0;
ret_val = set_name(port, key, val);
if (UNC_RC_SUCCESS != ret_val)
return UNC_DRV_RC_ERR_GENERIC;
ret_val = unc::restjson::JsonBuildParse::build("port", port, out);
if (restjson::REST_OP_SUCCESS != ret_val) {
pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
return UNC_DRV_RC_ERR_GENERIC;
}
return UNC_RC_SUCCESS;
}

virtual UncRespCode get_name(json_object *in, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
VirtParserVbrifPortmapCmd obj;
int ret_val = obj.get_name(in, key ,val);
if (ret_val != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

UncRespCode get_port(json_object *in, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
json_object *port = NULL;
int ret_val = unc::restjson::JsonBuildParse::parse(in, "port", -1, port);
ret_val = get_name(port, key, val);
if (UNC_RC_SUCCESS != ret_val)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

virtual UncRespCode set_vlan(json_object *out, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
VirtParserVbrifPortmapCmd obj;
int ret_val = obj.set_vlan(out, key ,val);
if (ret_val != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

virtual UncRespCode get_vlan(json_object *in, key_vbr_if_t &key, pfcdrv_val_vbr_if_t &val) {
VirtParserVbrifPortmapCmd obj;
int ret_val = obj.get_vlan(in, key ,val);
if (ret_val != UNC_RC_SUCCESS)
return UNC_DRV_RC_ERR_GENERIC;
return UNC_RC_SUCCESS;
}

void get_array_idx (json_object **out, json_object *in, uint32_t  arr_idx) {
*out = json_object_array_get_idx(in, arr_idx);
 pfc_log_debug("arr_idx_one:%s", unc::restjson::JsonBuildParse::get_json_string(*out));
}

int  send_httprequest(unc::driver::controller* ctr, std::string url, unc::restjson::ConfFileValues_t conf_values_ , unc::restjson::HttpMethod http_method, json_object *jobj_req_body)
{
unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(), ctr->get_user_name(), ctr->get_pass_word());
unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request( url, http_method, unc::restjson::JsonBuildParse::get_json_string(jobj_req_body), conf_values_);
if (NULL == response)
 {
 pfc_log_error("Error Occured while getting httpresponse");
return  UNC_DRV_RC_ERR_GENERIC;
}
int resp_code = response -> code;
 return resp_code;
}

};
} // namespace odcdriver
} // namespace unc
#endif
