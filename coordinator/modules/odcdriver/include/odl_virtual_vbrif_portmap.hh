#ifndef _ODC_VIRTUAL_VTN_HH_
#define _ODC_VIRTUAL_VTN_HH_

namespace unc {
namespace odcdriver{

class VirtualVbrifPortmapCommand {
public:
UncRespCode OdcVbrIfCommand::set_id(json_object *in, key_vbr_if_t& key, pfcdrv_val_vbr_if_t& val) {
  std::string logical_port_id =
            reinterpret_cast<char*>(val.val_vbrif.portmap.logical_port_id);
  odc_drv_resp_code_t logical_port_retval = check_logical_port_id_format(
                                                                logical_port_id);
  if (logical_port_retval != ODC_DRV_SUCCESS) {
      pfc_log_error("logical port id is Invalid");
          return UNC_DRV_RC_ERR_GENERIC;
        }

  std::string switch_id = logical_port_id.substr(3, 23);
  pfc_log_debug("switch_id:%s",switch_id.c_str());
  if (!(switch_id.empty())) {
     int ret_val = unc::restjson::JsonBuildParse::build<std::string>("id", switch_id, in);
     if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Failed in framing json request body for id");
          return UNC_DRV_RC_ERR_GENERIC;
        }
      }
     return UNC_RC_SUCCESS;
    }

UncRespCode OdcVbrIfCommand::set_name(json_object *in, key_vbr_if_t& key, pfcdrv_val_vbr_if_t& val) {
  std::string logical_port_id =
            reinterpret_cast<char*>(val.val_vbrif.portmap.logical_port_id);
  odc_drv_resp_code_t logical_port_retval = check_logical_port_id_format(
                                                                logical_port_id);
  if (logical_port_retval != ODC_DRV_SUCCESS) {
      pfc_log_error("logical port id is Invalid");
          return UNC_DRV_RC_ERR_GENERIC;
        }
  std::string port_name = logical_port_id.substr(27);
  pfc_log_debug("portname:%s",port_name.c_str());

  if (!(port_name.empty())) {
     int ret_val = unc::restjson::JsonBuildParse::build<std::string>("name", port_name, in);
     if (restjson::REST_OP_SUCCESS != ret_val) {
          pfc_log_error("Failed in framing json request body for name");
          return UNC_DRV_RC_ERR_GENERIC;
        }
      }
     unc::restjson::JsonBuildParse::build("name", port_name, in);
     return UNC_RC_SUCCESS;
    }

UncRespCode OdcVbrIfCommand::set_type(json_object *in, key_vbr_if_t& key, pfcdrv_val_vbr_if_t& val) {
  std::string type = "OF";
  int  ret_val = unc::restjson::JsonBuildParse::build
          ("type",type , in);
  pfc_log_debug("type:%d",ret_val);
  if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("Error in framing req body of type");
       return UNC_DRV_RC_ERR_GENERIC;
     }
  unc::restjson::JsonBuildParse::build("type", type, in);
    return UNC_RC_SUCCESS;
 }

UncRespCode OdcVbrIfCommand::set_vlan(json_object *in, key_vbr_if_t& key, pfcdrv_val_vbr_if_t& val_vbrif) {
       //int vlanid(0);
  int vlanid = val_vbrif.val_vbrif.portmap.vlan_id;
  pfc_log_debug("vlanid:%d",vlanid);
         //if (restjson::REST_OP_SUCCESS != ret_val) {
          //json_object_put(in);
          //pfc_log_debug("vlan parse error");
          //return UNC_DRV_RC_ERR_GENERIC;
        // }

 unc::restjson::JsonBuildParse::build("vlan", vlanid, in);
     return UNC_RC_SUCCESS;
 }
};
}
}
#endif
