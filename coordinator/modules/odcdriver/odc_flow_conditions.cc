/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_flow_conditions.hh>

namespace unc {
namespace odcdriver {
UncRespCode
flowConditionsUtil::GetValue(json_object *in,dst* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string fromkey ("from");
  int from_ret (JsonBuildParse::parse(in,fromkey,out->from_));
  if ( from_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", fromkey.c_str());
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string tokey ("to");
  int to_ret (JsonBuildParse::parse(in,tokey,out->to_));
  if ( to_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", tokey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,dst* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string fromkey ("from");
  if ( in->from_ < 0 || in->from_ > 65535 ) {
    return UNC_DRV_RC_ERR_GENERIC;
  } else {
    int from_ret (JsonBuildParse::build(fromkey,in->from_,out));
    if ( from_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", fromkey.c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  std::string tokey ("to");
  if ( in->to_ < 0 || in->to_ > 65535 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int to_ret (JsonBuildParse::build(tokey,in->to_,out));
    if ( to_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", tokey.c_str());
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::GetValue(json_object *in,ethernet* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string typekey ("type");
  int type_ret (JsonBuildParse::parse(in,typekey,out->type_));
  if ( type_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", typekey.c_str());
  }
  std::string vlankey ("vlan");
  int vlan_ret (JsonBuildParse::parse(in,vlankey,out->vlan_));
  if ( vlan_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", vlankey.c_str());
  }
  std::string vlanprikey ("vlanpri");
  int vlanpri_ret (JsonBuildParse::parse(in,vlanprikey,out->vlanpri_));
  if ( vlanpri_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", vlanprikey.c_str());
  }
  std::string srckey ("src");
  int src_ret (JsonBuildParse::parse(in,srckey,out->src_));
  if ( src_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", srckey.c_str());
  }
  std::string dstkey ("dst");
  int dst_ret (JsonBuildParse::parse(in,dstkey,out->dst_));
  if ( dst_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", dstkey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,ethernet* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string typekey ("type");
  if ( in->type_ < 0 || in->type_ > 65535 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int type_ret (JsonBuildParse::build(typekey,in->type_,out));
    if ( type_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", typekey.c_str());
    }
  }
  std::string vlankey ("vlan");
  if ( in->vlan_ < 0 || in->vlan_ > 4095 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int vlan_ret (JsonBuildParse::build(vlankey,in->vlan_,out));
    if ( vlan_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", vlankey.c_str());
    }
  }
  std::string vlanprikey ("vlanpri");
  if ( in->vlanpri_ < 0 || in->vlanpri_ > 7 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int vlanpri_ret (JsonBuildParse::build(vlanprikey,in->vlanpri_,out));
    if ( vlanpri_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", vlanprikey.c_str());
    }
  }
  std::string srckey ("src");
  if ( in->src_ == "" ) {
    pfc_log_info("string src empty");
  } else {
    int src_ret (JsonBuildParse::build(srckey,in->src_,out));
    if ( src_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", srckey.c_str());
    }
  }
  std::string dstkey ("dst");
  if ( in->dst_ == "" ) {
    pfc_log_info("string dst empty");
  } else {
    int dst_ret (JsonBuildParse::build(dstkey,in->dst_,out));
    if ( dst_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", dstkey.c_str());
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::GetValue(json_object *in,flowcondition* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string namekey ("name");
  int name_ret (JsonBuildParse::parse(in,namekey,out->name_));
  if ( name_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", namekey.c_str());
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string matchkey ("match");
  int array_length (JsonBuildParse::get_array_length(in,matchkey));
  if ( array_length == 0 ) {
    pfc_log_error("parse failed %s", matchkey.c_str());
  } else {
    json_object* array_obj(NULL);
    int match_ret(JsonBuildParse::parse(in,matchkey,array_obj));
    if ( match_ret != REST_OP_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;
    for( int count=0; count < array_length ; count ++ ) {
      json_object *array_entry(json_object_array_get_idx(array_obj,count));
      if ( array_entry == NULL )
        return UNC_DRV_RC_ERR_GENERIC;
      match* elem_=new match();
      UncRespCode match_getret(GetValue(array_entry, elem_));
      if ( match_getret != UNC_RC_SUCCESS )
        return match_getret;
      out->match_.push_back(elem_);
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,flowcondition* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string namekey ("name");
  if ( in->name_ == "" ) {
    return UNC_DRV_RC_ERR_GENERIC;
  } else {
    int name_ret (JsonBuildParse::build(namekey,in->name_,out));
    if ( name_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", namekey.c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  std::string matchkey ("match");
  json_object* match_array(JsonBuildParse::create_json_array_obj());
  std::list <match*>::iterator iter=in->match_.begin();
  while ( iter != in->match_.end() ) {
    if ( *iter == NULL ) {
      pfc_log_error("No Contents in match");
    } else {
       json_object* match_obj = JsonBuildParse::create_json_obj();
       UncRespCode match_ret(SetValue(match_obj,*iter));
       if ( match_ret != UNC_RC_SUCCESS ) {
         json_object_put(match_obj);
         json_object_put(match_array);
        return match_ret;
      }
       JsonBuildParse::add_to_array(match_array,match_obj);
      iter++;
    }
  }
  int match_setret (JsonBuildParse::build(matchkey,match_array,out));
  if ( match_setret != REST_OP_SUCCESS )
    return UNC_DRV_RC_ERR_GENERIC;
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::GetValue(json_object *in,icmp* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string typekey ("type");
  int type_ret (JsonBuildParse::parse(in,typekey,out->type_));
  if ( type_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", typekey.c_str());
  }
  std::string codekey ("code");
  int code_ret (JsonBuildParse::parse(in,codekey,out->code_));
  if ( code_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", codekey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,icmp* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string typekey ("type");
  if ( in->type_ < 0 || in->type_ > 255 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int type_ret (JsonBuildParse::build(typekey,in->type_,out));
    if ( type_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", typekey.c_str());
    }
  }
  std::string codekey ("code");
  if ( in->code_ < 0 || in->code_ > 255 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int code_ret (JsonBuildParse::build(codekey,in->code_,out));
    if ( code_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", codekey.c_str());
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::GetValue(json_object *in,inet4* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string srcsuffixkey ("srcsuffix");
  int srcsuffix_ret (JsonBuildParse::parse(in,srcsuffixkey,out->srcsuffix_));
  if ( srcsuffix_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", srcsuffixkey.c_str());
  }
  std::string dstsuffixkey ("dstsuffix");
  int dstsuffix_ret (JsonBuildParse::parse(in,dstsuffixkey,out->dstsuffix_));
  if ( dstsuffix_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", dstsuffixkey.c_str());
  }
  std::string protocolkey ("protocol");
  int protocol_ret (JsonBuildParse::parse(in,protocolkey,out->protocol_));
  if ( protocol_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", protocolkey.c_str());
  }
  std::string dscpkey ("dscp");
  int dscp_ret (JsonBuildParse::parse(in,dscpkey,out->dscp_));
  if ( dscp_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", dscpkey.c_str());
  }
  std::string srckey ("src");
  int src_ret (JsonBuildParse::parse(in,srckey,out->src_));
  if ( src_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", srckey.c_str());
  }
  std::string dstkey ("dst");
  int dst_ret (JsonBuildParse::parse(in,dstkey,out->dst_));
  if ( dst_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", dstkey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,inet4* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string srcsuffixkey ("srcsuffix");
  if ( in->srcsuffix_ < 1 || in->srcsuffix_ > 31 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int srcsuffix_ret (JsonBuildParse::build(srcsuffixkey,in->srcsuffix_,out));
    if ( srcsuffix_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", srcsuffixkey.c_str());
    }
  }
  std::string dstsuffixkey ("dstsuffix");
  if ( in->dstsuffix_ < 1 || in->dstsuffix_ > 31 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int dstsuffix_ret (JsonBuildParse::build(dstsuffixkey,in->dstsuffix_,out));
    if ( dstsuffix_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", dstsuffixkey.c_str());
    }
  }
  std::string protocolkey ("protocol");
  if ( in->protocol_ < 0 || in->protocol_ > 255 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int protocol_ret (JsonBuildParse::build(protocolkey,in->protocol_,out));
    if ( protocol_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", protocolkey.c_str());
    }
  }
  std::string dscpkey ("dscp");
  if ( in->dscp_ < 0 || in->dscp_ > 63 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int dscp_ret (JsonBuildParse::build(dscpkey,in->dscp_,out));
    if ( dscp_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", dscpkey.c_str());
    }
  }
  std::string srckey ("src");
  if ( in->src_ == "" ) {
    pfc_log_info("string src empty");
  } else {
    int src_ret (JsonBuildParse::build(srckey,in->src_,out));
    if ( src_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", srckey.c_str());
    }
  }
  std::string dstkey ("dst");
  if ( in->dst_ == "" ) {
    pfc_log_info("string dst empty");
  } else {
    int dst_ret (JsonBuildParse::build(dstkey,in->dst_,out));
    if ( dst_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", dstkey.c_str());
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::GetValue(json_object *in,inetMatch* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string inet4key ("inet4");
  json_object *inet4_obj (JsonBuildParse::create_json_obj());
  // Clear memory when variable(inet4_obj) is out of scope
  unc::restjson::json_obj_destroy_util inet4_obj_delete_obj(inet4_obj);
  int inet4_ret (JsonBuildParse::parse(in,inet4key,inet4_obj));
  if ( inet4_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", inet4key.c_str());
  } else {
    out->inet4_=new inet4();
    UncRespCode inet4_getret ( GetValue ( inet4_obj,out->inet4_));
    if ( inet4_getret != UNC_RC_SUCCESS )
      return inet4_getret;
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,inetMatch* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string inet4key ("inet4");
  if ( in->inet4_ == NULL ) {
    pfc_log_error("No Contents in inet4");
  } else {
    json_object *inet4_obj (JsonBuildParse::create_json_obj());
    UncRespCode inet4_ret (SetValue(inet4_obj,in->inet4_));
    if ( inet4_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", inet4key.c_str());
      json_object_put(inet4_obj);
    } else {
      int inet4_setret(JsonBuildParse::build(inet4key,inet4_obj,out));
      if ( inet4_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", inet4key.c_str());
      }
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::GetValue(json_object *in,l4Match* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string tcpkey ("tcp");
  json_object *tcp_obj (JsonBuildParse::create_json_obj());
  // Clear memory when variable(tcp_obj) is out of scope
  unc::restjson::json_obj_destroy_util tcp_obj_delete_obj(tcp_obj);
  int tcp_ret (JsonBuildParse::parse(in,tcpkey,tcp_obj));
  if ( tcp_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", tcpkey.c_str());
  } else {
    out->tcp_=new tcp();
    UncRespCode tcp_getret ( GetValue ( tcp_obj,out->tcp_));
    if ( tcp_getret != UNC_RC_SUCCESS )
      return tcp_getret;
  }
  std::string udpkey ("udp");
  json_object *udp_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util udp_obj_delete_obj(udp_obj);
  int udp_ret (JsonBuildParse::parse(in,udpkey,udp_obj));
  if ( udp_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", udpkey.c_str());
  } else {
    out->udp_=new udp();
    UncRespCode udp_getret ( GetValue ( udp_obj,out->udp_));
    if ( udp_getret != UNC_RC_SUCCESS )
      return udp_getret;
  }
  std::string icmpkey ("icmp");
  json_object *icmp_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util icmp_obj_delete_obj(icmp_obj);
  int icmp_ret (JsonBuildParse::parse(in,icmpkey,icmp_obj));
  if ( icmp_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", icmpkey.c_str());
  } else {
    out->icmp_=new icmp();
    UncRespCode icmp_getret ( GetValue ( icmp_obj,out->icmp_));
    if ( icmp_getret != UNC_RC_SUCCESS )
      return icmp_getret;
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,l4Match* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string tcpkey ("tcp");
  if ( in->tcp_ == NULL ) {
    pfc_log_error("No Contents in tcp");
  } else {
    json_object *tcp_obj (JsonBuildParse::create_json_obj());
    UncRespCode tcp_ret (SetValue(tcp_obj,in->tcp_));
    if ( tcp_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", tcpkey.c_str());
      json_object_put(tcp_obj);
    } else {
      int tcp_setret(JsonBuildParse::build(tcpkey,tcp_obj,out));
      if ( tcp_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", tcpkey.c_str());
      }
    }
  }
  std::string udpkey ("udp");
  if ( in->udp_ == NULL ) {
    pfc_log_error("No Contents in udp");
  } else {
    json_object *udp_obj (JsonBuildParse::create_json_obj());
    UncRespCode udp_ret (SetValue(udp_obj,in->udp_));
    if ( udp_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", udpkey.c_str());
      json_object_put(udp_obj);
    } else {
      int udp_setret(JsonBuildParse::build(udpkey,udp_obj,out));
      if ( udp_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", udpkey.c_str());
      }
    }
  }
  std::string icmpkey ("icmp");
  if ( in->icmp_ == NULL ) {
    pfc_log_error("No Contents in icmp");
  } else {
    json_object *icmp_obj (JsonBuildParse::create_json_obj());
    UncRespCode icmp_ret (SetValue(icmp_obj,in->icmp_));
    if ( icmp_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", icmpkey.c_str());
      json_object_put(icmp_obj);
    } else {
      int icmp_setret(JsonBuildParse::build(icmpkey,icmp_obj,out));
      if ( icmp_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", icmpkey.c_str());
      }
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::GetValue(json_object *in,match* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string indexkey ("index");
  int index_ret (JsonBuildParse::parse(in,indexkey,out->index_));
  if ( index_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", indexkey.c_str());
  }
  std::string ethernetkey ("ethernet");
  json_object *ethernet_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util ethernet_obj_delete_obj(ethernet_obj);
  int ethernet_ret (JsonBuildParse::parse(in,ethernetkey,ethernet_obj));
  if ( ethernet_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", ethernetkey.c_str());
  } else {
    out->ethernet_=new ethernet();
    UncRespCode ethernet_getret ( GetValue ( ethernet_obj,out->ethernet_));
    if ( ethernet_getret != UNC_RC_SUCCESS )
      return ethernet_getret;
  }
  std::string inetMatchkey ("inetMatch");
  json_object *inetMatch_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util inetMatch_obj_delete_obj(inetMatch_obj);
  int inetMatch_ret (JsonBuildParse::parse(in,inetMatchkey,inetMatch_obj));
  if ( inetMatch_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", inetMatchkey.c_str());
  } else {
    out->inetMatch_=new inetMatch();
    UncRespCode inetMatch_getret ( GetValue ( inetMatch_obj,out->inetMatch_));
    if ( inetMatch_getret != UNC_RC_SUCCESS )
      return inetMatch_getret;
  }
  std::string l4Matchkey ("l4Match");
  json_object *l4Match_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util l4Match_obj_delete_obj(l4Match_obj);
  int l4Match_ret (JsonBuildParse::parse(in,l4Matchkey,l4Match_obj));
  if ( l4Match_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", l4Matchkey.c_str());
  } else {
    out->l4Match_=new l4Match();
    UncRespCode l4Match_getret ( GetValue ( l4Match_obj,out->l4Match_));
    if ( l4Match_getret != UNC_RC_SUCCESS )
      return l4Match_getret;
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,match* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string indexkey ("index");
  if ( in->index_ < 1 || in->index_ > 65535 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int index_ret (JsonBuildParse::build(indexkey,in->index_,out));
    if ( index_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", indexkey.c_str());
    }
  }
  std::string ethernetkey ("ethernet");
  if ( in->ethernet_ == NULL ) {
    pfc_log_error("No Contents in ethernet");
  } else {
    json_object *ethernet_obj (JsonBuildParse::create_json_obj());
    UncRespCode ethernet_ret (SetValue(ethernet_obj,in->ethernet_));
    if ( ethernet_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", ethernetkey.c_str());
      json_object_put(ethernet_obj);
    } else {
      int ethernet_setret(JsonBuildParse::build(ethernetkey,ethernet_obj,out));
      if ( ethernet_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", ethernetkey.c_str());
      }
    }
  }
  std::string inetMatchkey ("inetMatch");
  if ( in->inetMatch_ == NULL ) {
    pfc_log_error("No Contents in inetMatch");
  } else {
    json_object *inetMatch_obj (JsonBuildParse::create_json_obj());
    UncRespCode inetMatch_ret (SetValue(inetMatch_obj,in->inetMatch_));
    if ( inetMatch_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", inetMatchkey.c_str());
      json_object_put(inetMatch_obj);
    } else {
      int inetMatch_setret(JsonBuildParse::build(inetMatchkey,inetMatch_obj,out));
      if ( inetMatch_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", inetMatchkey.c_str());
      }
    }
  }
  std::string l4Matchkey ("l4Match");
  if ( in->l4Match_ == NULL ) {
    pfc_log_error("No Contents in l4Match");
  } else {
    json_object *l4Match_obj (JsonBuildParse::create_json_obj());
    UncRespCode l4Match_ret (SetValue(l4Match_obj,in->l4Match_));
    if ( l4Match_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", l4Matchkey.c_str());
      json_object_put(l4Match_obj);
    } else {
      int l4Match_setret(JsonBuildParse::build(l4Matchkey,l4Match_obj,out));
      if ( l4Match_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", l4Matchkey.c_str());
      }
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::GetValue(json_object *in,src* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string fromkey ("from");
  int from_ret (JsonBuildParse::parse(in,fromkey,out->from_));
  if ( from_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", fromkey.c_str());
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string tokey ("to");
  int to_ret (JsonBuildParse::parse(in,tokey,out->to_));
  if ( to_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", tokey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,src* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string fromkey ("from");
  if ( in->from_ < 0 || in->from_ > 65535 ) {
    return UNC_DRV_RC_ERR_GENERIC;
  } else {
    int from_ret (JsonBuildParse::build(fromkey,in->from_,out));
    if ( from_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", fromkey.c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  std::string tokey ("to");
  if ( in->to_ < 0 || in->to_ > 65535 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int to_ret (JsonBuildParse::build(tokey,in->to_,out));
    if ( to_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", tokey.c_str());
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::GetValue(json_object *in,tcp* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string srckey ("src");
  json_object *src_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util src_obj_delete_obj(src_obj);
  int src_ret (JsonBuildParse::parse(in,srckey,src_obj));
  if ( src_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", srckey.c_str());
  } else {
    out->src_=new src();
    UncRespCode src_getret ( GetValue ( src_obj,out->src_));
    if ( src_getret != UNC_RC_SUCCESS )
      return src_getret;
  }
  std::string dstkey ("dst");
  json_object *dst_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util dst_obj_delete_obj(dst_obj);
  int dst_ret (JsonBuildParse::parse(in,dstkey,dst_obj));
  if ( dst_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", dstkey.c_str());
  } else {
    out->dst_=new dst();
    UncRespCode dst_getret ( GetValue ( dst_obj,out->dst_));
    if ( dst_getret != UNC_RC_SUCCESS )
      return dst_getret;
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,tcp* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string srckey ("src");
  if ( in->src_ == NULL ) {
    pfc_log_error("No Contents in src");
  } else {
    json_object *src_obj (JsonBuildParse::create_json_obj());
    UncRespCode src_ret (SetValue(src_obj,in->src_));
    if ( src_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", srckey.c_str());
      json_object_put(src_obj);
    } else {
      int src_setret(JsonBuildParse::build(srckey,src_obj,out));
      if ( src_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", srckey.c_str());
      }
    }
  }
  std::string dstkey ("dst");
  if ( in->dst_ == NULL ) {
    pfc_log_error("No Contents in dst");
  } else {
    json_object *dst_obj (JsonBuildParse::create_json_obj());
    UncRespCode dst_ret (SetValue(dst_obj,in->dst_));
    if ( dst_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", dstkey.c_str());
      json_object_put(dst_obj);
    } else {
      int dst_setret(JsonBuildParse::build(dstkey,dst_obj,out));
      if ( dst_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", dstkey.c_str());
      }
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::GetValue(json_object *in,udp* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string srckey ("src");
  json_object *src_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util src_obj_delete_obj(src_obj);
  int src_ret (JsonBuildParse::parse(in,srckey,src_obj));
  if ( src_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", srckey.c_str());
  } else {
    out->src_=new src();
    UncRespCode src_getret ( GetValue ( src_obj,out->src_));
    if ( src_getret != UNC_RC_SUCCESS )
      return src_getret;
  }
  std::string dstkey ("dst");
  json_object *dst_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util dst_obj_delete_obj(dst_obj);
  int dst_ret (JsonBuildParse::parse(in,dstkey,dst_obj));
  if ( dst_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", dstkey.c_str());
  } else {
    out->dst_=new dst();
    UncRespCode dst_getret ( GetValue ( dst_obj,out->dst_));
    if ( dst_getret != UNC_RC_SUCCESS )
      return dst_getret;
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,udp* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string srckey ("src");
  if ( in->src_ == NULL ) {
    pfc_log_error("No Contents in src");
  } else {
    json_object *src_obj (JsonBuildParse::create_json_obj());
    UncRespCode src_ret (SetValue(src_obj,in->src_));
    if ( src_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", srckey.c_str());
      json_object_put(src_obj);
    } else {
      int src_setret(JsonBuildParse::build(srckey,src_obj,out));
      if ( src_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", srckey.c_str());
      }
    }
  }
  std::string dstkey ("dst");
  if ( in->dst_ == NULL ) {
    pfc_log_error("No Contents in dst");
  } else {
    json_object *dst_obj (JsonBuildParse::create_json_obj());
    UncRespCode dst_ret (SetValue(dst_obj,in->dst_));
    if ( dst_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", dstkey.c_str());
      json_object_put(dst_obj);
    } else {
      int dst_setret(JsonBuildParse::build(dstkey,dst_obj,out));
      if ( dst_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", dstkey.c_str());
      }
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::GetValue(json_object *in,flowConditions* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string flowconditionkey ("flowcondition");
  int array_length (JsonBuildParse::get_array_length(in,flowconditionkey));
  if ( array_length == 0 ) {
    pfc_log_error("parse failed %s", flowconditionkey.c_str());
  } else {
    json_object* array_obj(NULL);
    int flowcondition_ret(JsonBuildParse::parse(in,flowconditionkey,array_obj));
    if ( flowcondition_ret != REST_OP_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;
    for( int count=0; count < array_length ; count ++ ) {
      json_object *array_entry(json_object_array_get_idx(array_obj,count));
      if ( array_entry == NULL )
        return UNC_DRV_RC_ERR_GENERIC;
      flowcondition* elem_=new flowcondition();
      UncRespCode flowcondition_getret(GetValue(array_entry, elem_));
      if ( flowcondition_getret != UNC_RC_SUCCESS )
        return flowcondition_getret;
      out->flowcondition_.push_back(elem_);
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowConditionsUtil::SetValue(json_object *out,flowConditions* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string flowconditionkey ("flowcondition");
  json_object* flowcondition_array(JsonBuildParse::create_json_array_obj());
  std::list <flowcondition*>::iterator iter=in->flowcondition_.begin();
  while ( iter != in->flowcondition_.end() ) {
    if ( *iter == NULL ) {
      pfc_log_error("No Contents in flowcondition");
    } else {
      json_object* flowcondition_obj = JsonBuildParse::create_json_obj();
      UncRespCode flowcondition_ret(SetValue(flowcondition_obj,*iter));
      if ( flowcondition_ret != UNC_RC_SUCCESS ) {
        json_object_put(flowcondition_obj);
        json_object_put(flowcondition_array);
        return flowcondition_ret;
      }
      JsonBuildParse::add_to_array(flowcondition_array,flowcondition_obj);
      iter++;
    }
  }
  int flowcondition_setret (JsonBuildParse::build(flowconditionkey,flowcondition_array,out));
  if ( flowcondition_setret != REST_OP_SUCCESS )
    return UNC_DRV_RC_ERR_GENERIC;
  return UNC_RC_SUCCESS;
}


}
}
