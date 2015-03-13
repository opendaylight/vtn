#ifndef _ODC_VIRTUAL_VTN_HH_
#define _ODC_VIRTUAL_VTN_HH_

namespace unc {
namespace odcdriver{

class VirtualVtnCommand {
public:
  VirtualVtnCommand() {}
  ~VirtualVtnCommand() {}
  UncRespCode set_vtn_idletime(json_object *&in, key_vtn_t& key, val_vtn_t& val) {
    int idle_timeout_ = 0;
    int ret_val = unc::restjson::JsonBuildParse::build("idleTimeout", idle_timeout_, in);
    if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
        json_object_put(in);
        return UNC_DRV_RC_ERR_GENERIC;
      }
    return UNC_RC_SUCCESS;
  }

  UncRespCode set_vtn_hardtime(json_object *&in, key_vtn_t& key, val_vtn_t& val) {
    int hard_timeout_ = 600;
    int ret_val = unc::restjson::JsonBuildParse::build("hardTimeout", hard_timeout_, in);
    if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
        json_object_put(in);
        return UNC_DRV_RC_ERR_GENERIC;
      }
    return UNC_RC_SUCCESS;
  }
};
}
}
#endif
