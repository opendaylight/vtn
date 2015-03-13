#ifndef __ODL_VIRTUAL_FLOWLIST_ENTRY_HH__
#define __ODL_VIRTUAL_FLOWLIST_ENTRY_HH__

#include <odc_util.hh>
#include <odl_flowlist_entry.hh>

namespace unc {
namespace odcdriver {

class VirtParserFlowlistEntry {
  
public:
  VirtParserFlowlistEntry() {}
  ~VirtParserFlowlistEntry() {}
 
  UncRespCode set_ethernet_dst(json_object *out, key_flowlist_entry_t &key, val_flowlist_entry_t &val) {
    OdcUtil util;
    if (val.valid[UPLL_IDX_MAC_DST_FLE] == UNC_VF_VALID) {
      pfc_log_debug("set_ethernet_dst set valid");
      std::string dst = util.macaddress_to_string(&val.mac_dst[0]);
      int ret_val = unc::restjson::JsonBuildParse::build("dst", dst, out);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
        return UNC_DRV_RC_ERR_GENERIC;
      }
    }
    return UNC_RC_SUCCESS;
  }
  
  UncRespCode set_ethernet_src(json_object *out, key_flowlist_entry_t &key, val_flowlist_entry_t &val) {
    OdcUtil util;
    if (val.valid[UPLL_IDX_MAC_SRC_FLE] == UNC_VF_VALID) {
      pfc_log_debug("set_ethernet_src set valid");
      std::string src = util.macaddress_to_string(&val.mac_src[0]);
      int ret_val = unc::restjson::JsonBuildParse::build("src", src, out);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
        return UNC_DRV_RC_ERR_GENERIC;
      }
    }
    return UNC_RC_SUCCESS;
  }

  UncRespCode set_inet4_dst(json_object *out, key_flowlist_entry_t &key, val_flowlist_entry_t &val) {
    if( val.valid[UPLL_IDX_DST_IP_FLE] == UNC_VF_VALID ) {
      pfc_log_debug("set_inet_dst set valid");
      std::string dst = inet_ntoa(val.dst_ip);
      int ret_val = unc::restjson::JsonBuildParse::build("dst", dst, out);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
        return UNC_DRV_RC_ERR_GENERIC;
      }
    }
    return UNC_RC_SUCCESS;
  }
  
  UncRespCode set_inet4_src(json_object *out, key_flowlist_entry_t &key, val_flowlist_entry_t &val) {
    if( val.valid[UPLL_IDX_SRC_IP_FLE] == UNC_VF_VALID ) {
      pfc_log_debug("set_inet4_src set valid");
      std::string src = inet_ntoa(val.src_ip);
      int ret_val = unc::restjson::JsonBuildParse::build("src", src, out);
      if (restjson::REST_OP_SUCCESS != ret_val) {
        pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);
        return UNC_DRV_RC_ERR_GENERIC;
      }
    }
    return UNC_RC_SUCCESS;
  }

    
  UncRespCode get_ethernet_dst(json_object *in, key_flowlist_entry_t &key, val_flowlist_entry_t &val) {
    OdcUtil util;
    std::string dst = "";
    int ret_val = restjson::JsonBuildParse::parse(in, "dst", -1, dst);
    if (restjson::REST_OP_SUCCESS != ret_val)
      pfc_log_error("parse failed %s", PFC_FUNCNAME);
    if (dst != "") {
      util.convert_macstring_to_uint8(dst, &val.mac_dst[0]);
      val.valid[UPLL_IDX_MAC_DST_FLE] = UNC_VF_VALID;
    }
    return UNC_RC_SUCCESS;
  }
  
  UncRespCode get_ethernet_src(json_object *in, key_flowlist_entry_t &key, val_flowlist_entry_t &val) {
    OdcUtil util;
    std::string src = "";
    int ret_val = restjson::JsonBuildParse::parse(in, "src", -1, src);
    if (restjson::REST_OP_SUCCESS != ret_val)
      pfc_log_error("parse failed %s", PFC_FUNCNAME);
    if (src != "") {
      util.convert_macstring_to_uint8(src, &val.mac_src[0]);
      val.valid[UPLL_IDX_MAC_SRC_FLE] = UNC_VF_VALID;
    }
    return UNC_RC_SUCCESS;
  }
    
  UncRespCode get_inet4_dst(json_object *in, key_flowlist_entry_t &key, val_flowlist_entry_t &val) {
    OdcUtil util;
    std::string dst = "";
    int ret_val = restjson::JsonBuildParse::parse(in, "dst", -1, dst);
    if (restjson::REST_OP_SUCCESS != ret_val)
      pfc_log_error("parse failed %s", PFC_FUNCNAME);
    if (dst != "") {
      util.convert_ip_to_inaddr(dst, &val.dst_ip);
      val.valid[UPLL_IDX_DST_IP_FLE] = UNC_VF_VALID;
    }
    return UNC_RC_SUCCESS;
  }
  
  UncRespCode get_inet4_src(json_object *in, key_flowlist_entry_t &key, val_flowlist_entry_t &val) {
    OdcUtil util;
    std::string src = "";
    int ret_val = restjson::JsonBuildParse::parse(in, "src", -1, src);
    if (restjson::REST_OP_SUCCESS != ret_val)
      pfc_log_error("parse failed %s", PFC_FUNCNAME);
    if (src != "") {
      util.convert_ip_to_inaddr(src, &val.src_ip);
      val.valid[UPLL_IDX_SRC_IP_FLE] = UNC_VF_VALID;
    }
    return UNC_RC_SUCCESS;
 }
/*UncRespCode set_filter(std::string parent_name, std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {
    key_flowlist_t key;
    val_flowlist_t val;
    memset (&key, 0, sizeof(key_flowlist));
    memset (&val, 0, sizeof(val_flowlist));
    strncpy(reinterpret_cast<char*> (key.flowlist_name), parent_name.c_str(), sizeof(key.flowlist_name)-1);
    pfc_log_debug("parent_flowlist_name is :%s",(char *)key.flowlist_name);
    //key_filter.input_direction=UPLL_FLOWFILTER_DIR_IN;
    val.ip_type=UPLL_FLOWLIST_TYPE_IP;
    val.valid[UPLL_IDX_IP_TYPE_FL] = UNC_VF_VALID;

    unc::vtndrvcache::ConfigNode *filter_cfgptr = new unc::vtndrvcache::CacheElementUtil<key_flowlist, val_flowlist, uint32_t>(&key, &val, uint32_t(UNC_OP_READ));
    PFC_ASSERT(cfgptr != NULL);
    cfgnode_vector.push_back(filter_cfgptr);
    return UNC_RC_SUCCESS;
    }*/

};
}
}
#endif
