#ifndef __ODL_VIRT_PARSER_VTERMIF_PORTMAP_HH__
#define __ODL_VIRT_PARSER_VTERMIF_PORTMAP_HH__

#include <odc_driver_common_defs.hh> 

namespace unc {
namespace odcdriver {

class VirtParserVtermifPortmapCmd {

private:
  //std::string port_name;
public:
 static std::string logical_port;

odc_drv_resp_code_t validate_logical_port_id(
    const std::string& logical_port_id) {
  ODC_FUNC_TRACE;
  pfc_log_debug("logical port received : %s", logical_port_id.c_str());
  //  First 3 characters should be PP-
  if ((logical_port_id.compare(0, 3, PP_PREFIX) != 0) &&
      (logical_port_id.size() < 28)) {
    pfc_log_error("Logical port id is not with PP- prefix");
    return ODC_DRV_FAILURE;
  }

  //  Split Switch id and prefix
  std::string switch_port = logical_port_id.substr(3);
  size_t hyphen_occurence = switch_port.find("-");
  if (hyphen_occurence == std::string::npos) {
    pfc_log_error("Error in Validating the port");
    return ODC_DRV_FAILURE;
  }
  //  Split switch id alone without port name
  std::string switch_id = switch_port.substr(0, hyphen_occurence);
  pfc_log_debug("Switch id in port map %s", switch_id.c_str());
  char *switch_id_char = const_cast<char *> (switch_id.c_str());
  char *save_ptr;
  char *colon_tokener = strtok_r(switch_id_char, ":", &save_ptr);
  int parse_occurence = 0;
  while (colon_tokener != NULL) {
    //  Split string with token ":" and length of splitted sw is 2
    // 11:11:22:22:33:33:44:44 length of splitted by ":" is 2
    if (2 != strlen(colon_tokener)) {
      pfc_log_error("Invalid switch id format supported by Vtn Manager");
      return ODC_DRV_FAILURE;
    }
    colon_tokener = strtok_r(NULL, ":", &save_ptr);
    parse_occurence++;
  }
  //  After splitting the switch id with token ":"  the occurence should be 8
  //  Eg : 11:11:22:22:33:33:44:44 by splitting with ":" the value is 8
  if (parse_occurence != 8) {
    pfc_log_error("Invalid format not supported by Vtn Manager");
    return ODC_DRV_FAILURE;
  }
  pfc_log_debug("Valid logical_port id");
  return ODC_DRV_SUCCESS;
}

//  Check the format of logical port id. If it is invalid converts to proper
//  format
odc_drv_resp_code_t check_logical_port_id_format(
    std::string& logical_port_id) {
  ODC_FUNC_TRACE;

  // If First SIX digits are in the format PP-OF: change to PP-
  if (logical_port_id.compare(0, 6, PP_OF_PREFIX) == 0) {
     logical_port_id.replace(logical_port_id.find(PP_OF_PREFIX),
                             PP_OF_PREFIX.length(), PP_PREFIX);
  }
  odc_drv_resp_code_t logical_port_retval = validate_logical_port_id(
      logical_port_id);
  if (logical_port_retval != ODC_DRV_SUCCESS) {
    pfc_log_debug("logical port needs to be converted to proper format");
    logical_port_retval = convert_logical_port(logical_port_id);
    if (logical_port_retval != ODC_DRV_SUCCESS) {
      pfc_log_error("Failed during conversion of logical port id %s",
                    logical_port_id.c_str());
      return ODC_DRV_FAILURE;
    }
    logical_port_retval = validate_logical_port_id(logical_port_id);
    if (logical_port_retval != ODC_DRV_SUCCESS) {
      pfc_log_error("Failed during validation of logical port id %s",
                    logical_port_id.c_str());
      return ODC_DRV_FAILURE;
    }
  }
  return logical_port_retval;
}

//  Converts the logical port id from PP-1111-2222-3333-4444-name to
//  PP-11:11:22:22:33:33:44:44-name format
odc_drv_resp_code_t convert_logical_port(
    std::string &logical_port_id) {
  ODC_FUNC_TRACE;
  if ((logical_port_id.compare(0, 3, PP_PREFIX) != 0) &&
      (logical_port_id.size() < 24)) {
    pfc_log_error("%s: Logical_port_id doesn't have PP- prefix and"
                  "less than required length", PFC_FUNCNAME);
    return ODC_DRV_FAILURE;
  }
  //  Validate the format before conversion
  std::string switch_id = logical_port_id.substr(3);
  char *switch_id_validation = const_cast<char *> (switch_id.c_str());
  char *save_ptr;
  //  Split string with token "-"
  char *string_token = strtok_r(switch_id_validation , "-", &save_ptr);
  int occurence = 0;

  while (string_token != NULL) {
    //  Switch id token should be of length 4
    if ((occurence < 4) && (4 != strlen(string_token))) {
      pfc_log_error("Invalid Switch id format");
      return ODC_DRV_FAILURE;
    }
    occurence++;
    string_token = strtok_r(NULL, "-", &save_ptr);
  }
  //  Parsing string token by "-" length of parsed SW minimun 5
  //  Eg : 1111-2222-3333-4444-portname length of SW parsed by token "-"
  if (occurence < 5) {
    pfc_log_error("Invalid Switch id format");
    return ODC_DRV_FAILURE;
  }
  switch_id = logical_port_id.substr(3, 19);
  pfc_log_debug("Switch id in port map %s", switch_id.c_str());
  std::string port_name = logical_port_id.substr(23);

  //  Converts 1111-2222-3333-4444 to 11:11:22:22:33:33:44:44
  //  First occurence of colon is 2 and next occuerence of colon is obtained if
  //  is incremented by 3
  for (uint position = 2; position < switch_id.length(); position += 3) {
    if (switch_id.at(position) == '-') {
      switch_id.replace(position, 1 , COLON);
    } else {
      switch_id.insert(position, COLON);
    }
  }
  logical_port_id = PP_PREFIX;
  logical_port_id.append(switch_id);
  logical_port_id.append(HYPHEN);
  logical_port_id.append(port_name);
  return ODC_DRV_SUCCESS;
}

 UncRespCode set_vlan(json_object *out, key_vterm_if_t &key, val_vterm_if_t &vterm_if_val) { 
  int vlanid(0);
  if ((UNC_VF_VALID == vterm_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM])
      &&
      (UNC_VF_VALID == vterm_if_val.portmap.valid[UPLL_IDX_TAGGED_PM])) {
    if (vterm_if_val.portmap.tagged == UPLL_VLAN_TAGGED) {
      vlanid = vterm_if_val.portmap.vlan_id;
    }
  }
  if (vlanid >= 0) {
    uint32_t ret_val = unc::restjson::JsonBuildParse::build<int>
        ("vlan", vlanid, out);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
   return UNC_RC_SUCCESS;
}

 UncRespCode set_type(json_object *out, key_vterm_if_t &key, val_vterm_if_t &vterm_if_val) {
  uint32_t ret_val = unc::restjson::JsonBuildParse::build<std::string>
      ("type", "OF", out);
  if (restjson::REST_OP_SUCCESS != ret_val) {
    pfc_log_error("Error in framing req body of type");
    return UNC_DRV_RC_ERR_GENERIC;
  }
   return UNC_RC_SUCCESS;
}

 UncRespCode set_id(json_object *out, key_vterm_if_t &key, val_vterm_if_t &vterm_if_val) {
  std::string switch_id = "";
  std::string logical_port_id =
        reinterpret_cast<char*>(vterm_if_val.portmap.logical_port_id);
  odc_drv_resp_code_t logical_port_retval = check_logical_port_id_format(
       logical_port_id);
  if (logical_port_retval != ODC_DRV_SUCCESS) {
    pfc_log_error("logical port id is invalid");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  switch_id = logical_port_id.substr(3, 23);
  if (!(switch_id.empty())) {
    int ret_val = unc::restjson::JsonBuildParse::build<std::string>
        ("id", switch_id, out);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("Failed in framing json request body for id");
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
   return UNC_RC_SUCCESS;
}

 UncRespCode set_name(json_object *out, key_vterm_if_t &key, val_vterm_if_t &vterm_if_val) {
  std::string port_name = "";
  std::string logical_port_id =
        reinterpret_cast<char*>(vterm_if_val.portmap.logical_port_id);
  odc_drv_resp_code_t logical_port_retval = check_logical_port_id_format(
       logical_port_id);
  if (logical_port_retval != ODC_DRV_SUCCESS) {
    pfc_log_error("logical port id is invalid");
    return UNC_DRV_RC_ERR_GENERIC;
  }

  port_name = logical_port_id.substr(27);
  
  if (!(port_name.empty())) {
    int ret_val = unc::restjson::JsonBuildParse::build<std::string>
        ("name", port_name, out);
    if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_error("Failed in framing json request body for name");
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
   return UNC_RC_SUCCESS;
 }

 UncRespCode get_vlan(json_object *in, key_vterm_if_t &key, val_vterm_if_t &vterm_if_val) { 
   vterm_if_val.valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;
   int vlanid(0);
   int ret_val = unc::restjson::JsonBuildParse::parse(in, "vlan", -1,
                                                     vlanid);
   if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_debug("vlan parse error");
        return UNC_DRV_RC_ERR_GENERIC;
      }
      pfc_log_debug("vlan id in portmap read %d", vlanid);
      if (0 == vlanid) {
        vterm_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] =
                                                   UNC_VF_INVALID;
        pfc_log_debug("untagged");
        vterm_if_val.portmap.tagged = UPLL_VLAN_UNTAGGED;
        vterm_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      } else {
        pfc_log_debug("vlan id valid");
        vterm_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
        vterm_if_val.portmap.vlan_id = vlanid;
        pfc_log_debug("%d  vlan id ", vlanid);
        pfc_log_debug("vlan id tagged");
        vterm_if_val.portmap.tagged = UPLL_VLAN_TAGGED;
        vterm_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      }
   return UNC_RC_SUCCESS;
 }

 UncRespCode get_type(json_object *in, key_vterm_if_t &key, val_vterm_if_t &vterm_if_val) {
   return UNC_RC_SUCCESS;
 }
 
 UncRespCode get_name(json_object *in, key_vterm_if_t &key, val_vterm_if_t &vterm_if_val) {
   std::string port_name = "";
   int ret_val = unc::restjson::JsonBuildParse::parse(in, "name",
                                                     -1, port_name);
   pfc_log_debug("ret_val:%d",ret_val);
   pfc_log_debug("port_name:%s",port_name.c_str());
   if (restjson::REST_OP_SUCCESS != ret_val) {

      pfc_log_debug("node_id parse error");
      return UNC_DRV_RC_ERR_GENERIC;
   }
   pfc_log_debug("logical_port:%s",logical_port.c_str());

   logical_port.append(port_name);
   pfc_log_debug("logical_port:%s",logical_port.c_str());
   pfc_log_debug("port_name:%s",port_name.c_str());
   strncpy(reinterpret_cast<char*>
              (vterm_if_val.portmap.logical_port_id),
              logical_port.c_str(),
              sizeof(vterm_if_val.portmap.logical_port_id) - 1);
   pfc_log_debug("%s logical port id in readportmap " ,
                    reinterpret_cast<char*>
                    (vterm_if_val.portmap.logical_port_id));
   vterm_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
   return UNC_RC_SUCCESS;
 }

 UncRespCode get_id(json_object *in, key_vterm_if_t &key, val_vterm_if_t &vterm_if_val) {
   std::string node_id = "";
   std::string logical_port_id = "PP-OF:";
   int ret_val = unc::restjson::JsonBuildParse::parse(in, "id", -1, node_id);
   if (restjson::REST_OP_SUCCESS != ret_val) {
      pfc_log_debug("get_id parse error");
      return UNC_DRV_RC_ERR_GENERIC;
   }
  pfc_log_debug("node_id:%s",node_id.c_str());
  pfc_log_debug("logical_port_id:%s",logical_port_id.c_str());
  pfc_log_debug("logical_port.append:%s",logical_port.c_str());
  logical_port.append(logical_port_id);
   pfc_log_debug("logical_port.append:%s",logical_port.c_str());

  logical_port.append(node_id);
   pfc_log_debug("logical_port.append:%s",logical_port.c_str());
  logical_port.append(HYPHEN);
  pfc_log_trace("get_id:%s", logical_port.c_str());
  return UNC_RC_SUCCESS;
 }
 /*   logical_port_id.append(node_id);
    logical_port_id.append(HYPHEN);
    logical_port_id.append(port_name);
    pfc_log_trace("get_id:%s", logical_port_id.c_str());

    strncpy(reinterpret_cast<char*>
               (vterm_if_val.portmap.logical_port_id),
               logical_port_id.c_str(),
               sizeof(vterm_if_val.portmap.logical_port_id) - 1);
    pfc_log_debug("%s logical port id in readportmap " ,
                     reinterpret_cast<char*>
                     (vterm_if_val.portmap.logical_port_id));
    vterm_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
    pfc_log_trace("get_id:%d", vterm_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]);
    pfc_log_trace("UNC_VF_VALID:%d", UNC_VF_VALID);
    return UNC_RC_SUCCESS;
  }*/

};

std::string VirtParserVtermifPortmapCmd::logical_port = "";
}
}
#endif
