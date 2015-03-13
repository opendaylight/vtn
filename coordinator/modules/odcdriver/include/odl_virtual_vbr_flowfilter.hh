#ifndef __ODL_VBR_VIRTUAL_PARSER__
#define __ODL_VBR_VIRTUAL_PARSER__

namespace unc {
namespace odcdriver {

class VirtParserVbrFlowfilterEntry {
  public:
    VirtParserVbrFlowfilterEntry () {}
    ~VirtParserVbrFlowfilterEntry() {}

  UncRespCode get_pass(json_object *in, key_vbr_flowfilter_entry &key, val_flowfilter_entry &val) {
   json_object *obj (JsonBuildParse::create_json_obj());
   int ret_val = JsonBuildParse::parse(in, "pass", obj);
   if (UNC_RC_SUCCESS != ret_val)
     pfc_log_error("parse failed %s", PFC_FUNCNAME);
   else {
     val.action=UPLL_FLOWFILTER_ACT_PASS;
     val.valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
   }
   return UNC_RC_SUCCESS;
 } 
    
 UncRespCode get_drop(json_object *in, key_vbr_flowfilter_entry &key, val_flowfilter_entry &val) {
   json_object *obj (JsonBuildParse::create_json_obj());
   int ret_val = JsonBuildParse::parse(in, "drop", obj);
   if (UNC_RC_SUCCESS != ret_val)
     pfc_log_error("parse failed %s", PFC_FUNCNAME);
   else {
     val.action=UPLL_FLOWFILTER_ACT_DROP;
     val.valid[UPLL_IDX_ACTION_VFFE] = UNC_VF_VALID;
   }
   return UNC_RC_SUCCESS;
 } 

 
 UncRespCode get_redirect(json_object *in, key_vbr_flowfilter_entry &key, val_flowfilter_entry &val) {
   return UNC_RC_SUCCESS;
 }

 UncRespCode set_dlsrc_address(json_object *out, key_vbr_flowfilter_entry &key, val_flowfilter_entry &val) {
   return UNC_RC_SUCCESS;
 }
 
  UncRespCode set_dldst_address(json_object *out, key_vbr_flowfilter_entry &key, val_flowfilter_entry &val) {
   return UNC_RC_SUCCESS;
 }

  UncRespCode get_dlsrc_address(json_object *in, key_vbr_flowfilter_entry &key, val_flowfilter_entry &val) {
   return UNC_RC_SUCCESS;
 }
  UncRespCode get_dldst_address(json_object *in, key_vbr_flowfilter_entry &key, val_flowfilter_entry &val) {
   return UNC_RC_SUCCESS;
 }
UncRespCode set_filter(std::string parent_name, std::string parent_name1,std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    key_vbr_flowfilter key_filter;
    val_flowfilter val_filter;
    memset (&key_filter, 0, sizeof(key_vbr_flowfilter));
    memset (&val_filter, 0, sizeof(val_flowfilter));
    strncpy(reinterpret_cast<char*> (key_filter.vbr_key.vtn_key.vtn_name), parent_name.c_str(), sizeof(key_filter.vbr_key.vtn_key.vtn_name)-1);
    strncpy(reinterpret_cast<char*> (key_filter.vbr_key.vbridge_name), parent_name1.c_str(), sizeof(key_filter.vbr_key.vbridge_name)-1);
    pfc_log_debug("parent_vtn_name is :%s",(char *)key_filter.vbr_key.vtn_key.vtn_name);
    key_filter.direction=UPLL_FLOWFILTER_DIR_IN;
    unc::vtndrvcache::ConfigNode *filter_cfgptr = new unc::vtndrvcache::CacheElementUtil<key_vbr_flowfilter, val_flowfilter, uint32_t>(&key_filter, &val_filter, uint32_t(UNC_OP_READ));
    PFC_ASSERT(cfgptr != NULL);
    cfgnode_vector.push_back(filter_cfgptr);
    return UNC_RC_SUCCESS;
    }

};
}
}
#endif
