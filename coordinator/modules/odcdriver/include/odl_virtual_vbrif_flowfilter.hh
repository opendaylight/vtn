#ifndef __ODL_VBR_IF_VIRTUAL_PARSER__
#define __ODL_VBR_IF_VIRTUAL_PARSER__

namespace unc {
namespace odcdriver {

class VirtParserVbrIfFlowfilterEntry {
  public:
    VirtParserVbrIfFlowfilterEntry () {}
    ~VirtParserVbrIfFlowfilterEntry() {}

  UncRespCode get_pass(json_object *in, key_vbr_if_flowfilter_entry &key, pfcdrv_val_flowfilter_entry &val) {
   json_object *obj (JsonBuildParse::create_json_obj());
   int ret_val = JsonBuildParse::parse(in, "pass", obj);
   if (UNC_RC_SUCCESS != ret_val)
     pfc_log_error("parse failed %s", PFC_FUNCNAME);
   else {
     val.val_ff_entry.action=UPLL_FLOWFILTER_ACT_PASS;
     val.valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
   }
   return UNC_RC_SUCCESS;
 } 
    
 UncRespCode get_drop(json_object *in, key_vbr_if_flowfilter_entry &key, pfcdrv_val_flowfilter_entry &val) {
   json_object *obj (JsonBuildParse::create_json_obj());
   int ret_val = JsonBuildParse::parse(in, "drop", obj);
   if (UNC_RC_SUCCESS != ret_val)
     pfc_log_error("parse failed %s", PFC_FUNCNAME);
   else {
     val.val_ff_entry.action=UPLL_FLOWFILTER_ACT_DROP;
     val.valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
   }
   return UNC_RC_SUCCESS;
 } 

UncRespCode set_filter(std::string parent_name, std::string parent_name1,std::string parent_name2, std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
key_vbr_if_flowfilter key_filter;
pfcdrv_val_vbrif_vextif val_filter;
memset ( &key_filter, 0, sizeof(key_vbr_if_flowfilter));
memset ( &val_filter, 0, sizeof(pfcdrv_val_vbrif_vextif));
strncpy(reinterpret_cast<char*> (key_filter.if_key.vbr_key.vbridge_name),
 parent_name1.c_str(), sizeof(key_filter.if_key.vbr_key.vbridge_name) - 1);
strncpy(reinterpret_cast<char*> (key_filter.if_key.vbr_key.vtn_key.vtn_name),
      parent_name.c_str(), sizeof(key_filter.if_key.vbr_key.vtn_key.vtn_name) - 1);
strncpy(reinterpret_cast<char*> (key_filter.if_key.if_name),
      parent_name2.c_str(), sizeof(key_filter.if_key.if_name) - 1);
key_filter.direction=UPLL_FLOWFILTER_DIR_IN;

val_filter.valid[PFCDRV_IDX_INTERFACE_TYPE]=UNC_VF_VALID;
val_filter.interface_type=PFCDRV_IF_TYPE_VBRIF;

 //Add to Cache
unc::vtndrvcache::ConfigNode *filter_cfgptr =
       new unc::vtndrvcache::CacheElementUtil<key_vbr_if_flowfilter, pfcdrv_val_vbrif_vextif, uint32_t>
      (&key_filter,&val_filter,uint32_t(UNC_OP_READ));

cfgnode_vector.push_back(filter_cfgptr);
return UNC_RC_SUCCESS;
}
 
 UncRespCode get_redirect(json_object *in, key_vbr_if_flowfilter_entry &key, pfcdrv_val_flowfilter_entry &val) {
   return UNC_RC_SUCCESS;
 }

 UncRespCode set_dlsrc_address(json_object *out, key_vbr_if_flowfilter_entry &key, pfcdrv_val_flowfilter_entry &val) {
 OdcUtil util;
 if (val.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID) {
 pfc_log_debug("set_dlsrc_src set valid");
 //val_flowfilter_entry val;
 std::string src = util.macaddress_to_string(&val.val_ff_entry.modify_srcmac[0]);
 int ret_val = unc::restjson::JsonBuildParse::build("address", src, out);
 if (restjson::REST_OP_SUCCESS != ret_val) {
 pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
 return UNC_DRV_RC_ERR_GENERIC;
  }
  }
   return UNC_RC_SUCCESS;
 }
 
  UncRespCode set_dldst_address(json_object *out, key_vbr_if_flowfilter_entry &key, pfcdrv_val_flowfilter_entry &val) {
   return UNC_RC_SUCCESS;
 }

  UncRespCode get_dlsrc_address(json_object *in, key_vbr_if_flowfilter_entry &key, pfcdrv_val_flowfilter_entry &val) {
   return UNC_RC_SUCCESS;
 }
  UncRespCode get_dldst_address(json_object *in, key_vbr_if_flowfilter_entry &key, pfcdrv_val_flowfilter_entry &val) {
   return UNC_RC_SUCCESS;
 }

};
}
}
#endif
