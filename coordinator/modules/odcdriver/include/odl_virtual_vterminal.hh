#ifndef _ODC_VIRTUAL_VTERMINAL_HH_
#define _ODC_VIRTUAL_VTERMINAL_HH_

namespace unc {
namespace odcdriver{

class VirtualVtermCommand {
public:
  VirtualVtermCommand() {}
  ~VirtualVtermCommand() {}
 UncRespCode get_controller_name( unc::driver::controller* ctr, key_vterm_t& key, val_vterm_t& val) {
 std::string controller_name = ctr->get_controller_id();
 if (0 == strlen(controller_name.c_str())) {
 val.valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_INVALID;
 }
 else {
        strncpy(reinterpret_cast<char*>(val.controller_id),
                controller_name.c_str(), sizeof(val.controller_id) - 1);
        pfc_log_trace("controllerid is done");
        pfc_log_debug(" %s controller id",
                      reinterpret_cast<char*>(val.controller_id));
        val.valid[UPLL_IDX_CONTROLLER_ID_VTERM] = UNC_VF_VALID;
      }

   return UNC_RC_SUCCESS;
 }
 UncRespCode get_domain_name( unc::driver::controller* ctr, key_vterm_t& key, val_vterm_t& val) {
   std::string dom_name("(DEFAULT)");
// strncpy(reinterpret_cast<char*> (val.domain_id), DOM_NAME.c_str(),
  //            sizeof(val.domain_id) - 1);
 strncpy(reinterpret_cast<char*> (val.domain_id), dom_name.c_str(),
              sizeof(val.domain_id) - 1);
 pfc_log_debug("domain_id:%s",reinterpret_cast<char*>(val.domain_id));
 pfc_log_trace("domainid is done");
      val.valid[UPLL_IDX_DOMAIN_ID_VTERM] = UNC_VF_VALID;

      return UNC_RC_SUCCESS;
}
/* 
UncRespCode set_vbridge_faults(json_object *in, key_vbr_t& key, val_vbr_t& val) {
   int faults_ = 0;
   int ret_val = unc::restjson::JsonBuildParse::build("faults", faults_, in);
   if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
      json_object_put(in);
      return UNC_DRV_RC_ERR_GENERIC;
   }
   return UNC_RC_SUCCESS;
 }
UncRespCode set_vbridge_state(json_object *in, key_vbr_t& key, val_vbr_t& val) {
   int state_ = 0;
   int ret_val = unc::restjson::JsonBuildParse::build("state", state_, in);
   if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
      json_object_put(in);
      return UNC_DRV_RC_ERR_GENERIC;
   }
   return UNC_RC_SUCCESS;
 }*/
};
}
}
#endif
