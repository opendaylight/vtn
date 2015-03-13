#ifndef __ODL_VTN_VIRTUAL_PARSER__
#define __ODL_VTN_VIRTUAL_PARSER__

namespace unc {
namespace odcdriver {

class VirtParserVtnFlowfilterEntry {
  public:
    VirtParserVtnFlowfilterEntry () {}
    ~VirtParserVtnFlowfilterEntry() {}

 UncRespCode get_pass(json_object *in, key_vtn_flowfilter_entry &key, val_vtn_flowfilter_entry &val) {
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
 
 UncRespCode get_drop(json_object *in, key_vtn_flowfilter_entry &key, val_vtn_flowfilter_entry &val) {
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

};
}
}
#endif

