/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <odc_flow_filter.hh>
namespace unc {
namespace odcdriver {
UncRespCode
flowfilterlistUtil::GetValue(json_object *in,action* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string dlsrckey ("dlsrc");
  json_object *dlsrc_obj (JsonBuildParse::create_json_obj());
  // Clear memory when variable(dlsrc_obj) is out of scope
  unc::restjson::json_obj_destroy_util dlsrc_obj_delete_obj(dlsrc_obj);
  int dlsrc_ret (JsonBuildParse::parse(in,dlsrckey,dlsrc_obj));
  if ( dlsrc_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", dlsrckey.c_str());
  } else {
    out->dlsrc_=new dlsrc();
    UncRespCode dlsrc_getret ( GetValue ( dlsrc_obj,out->dlsrc_));
    if ( dlsrc_getret != UNC_RC_SUCCESS )
      return dlsrc_getret;
  }
  std::string dldstkey ("dldst");
  json_object *dldst_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util dldst_obj_delete_obj(dldst_obj);
  int dldst_ret (JsonBuildParse::parse(in,dldstkey,dldst_obj));
  if ( dldst_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", dldstkey.c_str());
  } else {
    out->dldst_=new dldst();
    UncRespCode dldst_getret ( GetValue ( dldst_obj,out->dldst_));
    if ( dldst_getret != UNC_RC_SUCCESS )
      return dldst_getret;
  }
  std::string vlanpcpkey ("vlanpcp");
  json_object *vlanpcp_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util vlanpcp_obj_delete_obj(vlanpcp_obj);
  int vlanpcp_ret (JsonBuildParse::parse(in,vlanpcpkey,vlanpcp_obj));
  if ( vlanpcp_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", vlanpcpkey.c_str());
  } else {
    out->vlanpcp_=new vlanpcp();
    UncRespCode vlanpcp_getret ( GetValue ( vlanpcp_obj,out->vlanpcp_));
    if ( vlanpcp_getret != UNC_RC_SUCCESS )
      return vlanpcp_getret;
  }
  std::string inet4srckey ("inet4src");
  json_object *inet4src_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util inet4src_obj_delete_obj(inet4src_obj);
  int inet4src_ret (JsonBuildParse::parse(in,inet4srckey,inet4src_obj));
  if ( inet4src_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", inet4srckey.c_str());
  } else {
    out->inet4src_=new inet4src();
    UncRespCode inet4src_getret ( GetValue ( inet4src_obj,out->inet4src_));
    if ( inet4src_getret != UNC_RC_SUCCESS )
      return inet4src_getret;
  }
  std::string inet4dstkey ("inet4dst");
  json_object *inet4dst_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util inet4dst_obj_delete_obj(inet4dst_obj);
  int inet4dst_ret (JsonBuildParse::parse(in,inet4dstkey,inet4dst_obj));
  if ( inet4dst_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", inet4dstkey.c_str());
  } else {
    out->inet4dst_=new inet4dst();
    UncRespCode inet4dst_getret ( GetValue ( inet4dst_obj,out->inet4dst_));
    if ( inet4dst_getret != UNC_RC_SUCCESS )
      return inet4dst_getret;
  }
  std::string dscpkey ("dscp");
  json_object *dscp_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util dscp_obj_delete_obj(dscp_obj);
  int dscp_ret (JsonBuildParse::parse(in,dscpkey,dscp_obj));
  if ( dscp_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", dscpkey.c_str());
  } else {
    out->dscp_=new dscp();
    UncRespCode dscp_getret ( GetValue ( dscp_obj,out->dscp_));
    if ( dscp_getret != UNC_RC_SUCCESS )
      return dscp_getret;
  }
  std::string tpsrckey ("tpsrc");
  json_object *tpsrc_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util tpsrc_obj_delete_obj(tpsrc_obj);
  int tpsrc_ret (JsonBuildParse::parse(in,tpsrckey,tpsrc_obj));
  if ( tpsrc_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", tpsrckey.c_str());
  } else {
    out->tpsrc_=new tpsrc();
    UncRespCode tpsrc_getret ( GetValue ( tpsrc_obj,out->tpsrc_));
    if ( tpsrc_getret != UNC_RC_SUCCESS )
      return tpsrc_getret;
  }
  std::string tpdstkey ("tpdst");
  json_object *tpdst_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util tpdst_obj_delete_obj(tpdst_obj);
  int tpdst_ret (JsonBuildParse::parse(in,tpdstkey,tpdst_obj));
  if ( tpdst_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", tpdstkey.c_str());
  } else {
    out->tpdst_=new tpdst();
    UncRespCode tpdst_getret ( GetValue ( tpdst_obj,out->tpdst_));
    if ( tpdst_getret != UNC_RC_SUCCESS )
      return tpdst_getret;
  }
  std::string icmpcodekey ("icmpcode");
  json_object *icmpcode_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util icmpcode_obj_delete_obj(icmpcode_obj);
  int icmpcode_ret (JsonBuildParse::parse(in,icmpcodekey,icmpcode_obj));
  if ( icmpcode_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", icmpcodekey.c_str());
  } else {
    out->icmpcode_=new icmpcode();
    UncRespCode icmpcode_getret ( GetValue ( icmpcode_obj,out->icmpcode_));
    if ( icmpcode_getret != UNC_RC_SUCCESS )
      return icmpcode_getret;
  }
  std::string icmptypekey ("icmptype");
  json_object *icmptype_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util icmptype_obj_delete_obj(icmptype_obj);
  int icmptype_ret (JsonBuildParse::parse(in,icmptypekey,icmptype_obj));
  if ( icmptype_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", icmptypekey.c_str());
  } else {
    out->icmptype_=new icmptype();
    UncRespCode icmptype_getret ( GetValue ( icmptype_obj,out->icmptype_));
    if ( icmptype_getret != UNC_RC_SUCCESS )
      return icmptype_getret;
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,action* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string dlsrckey ("dlsrc");
  if ( in->dlsrc_ == NULL ) {
    pfc_log_error("No Contents in dlsrc");
  } else {
    json_object *dlsrc_obj (JsonBuildParse::create_json_obj());
    UncRespCode dlsrc_ret (SetValue(dlsrc_obj,in->dlsrc_));
    if ( dlsrc_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", dlsrckey.c_str());
      json_object_put(dlsrc_obj);
    } else {
      int dlsrc_setret(JsonBuildParse::build(dlsrckey,dlsrc_obj,out));
      if ( dlsrc_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", dlsrckey.c_str());
      }
    }
  }
  std::string dldstkey ("dldst");
  if ( in->dldst_ == NULL ) {
    pfc_log_error("No Contents in dldst");
  } else {
    json_object *dldst_obj (JsonBuildParse::create_json_obj());
    UncRespCode dldst_ret (SetValue(dldst_obj,in->dldst_));
    if ( dldst_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", dldstkey.c_str());
      json_object_put(dldst_obj);
    } else {
      int dldst_setret(JsonBuildParse::build(dldstkey,dldst_obj,out));
      if ( dldst_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", dldstkey.c_str());
      }
    }
  }
  std::string vlanpcpkey ("vlanpcp");
  if ( in->vlanpcp_ == NULL ) {
    pfc_log_error("No Contents in vlanpcp");
  } else {
    json_object *vlanpcp_obj (JsonBuildParse::create_json_obj());
    UncRespCode vlanpcp_ret (SetValue(vlanpcp_obj,in->vlanpcp_));
    if ( vlanpcp_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", vlanpcpkey.c_str());
      json_object_put(vlanpcp_obj);
    } else {
      int vlanpcp_setret(JsonBuildParse::build(vlanpcpkey,vlanpcp_obj,out));
      if ( vlanpcp_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", vlanpcpkey.c_str());
      }
    }
  }
  std::string inet4srckey ("inet4src");
  if ( in->inet4src_ == NULL ) {
    pfc_log_error("No Contents in inet4src");
  } else {
    json_object *inet4src_obj (JsonBuildParse::create_json_obj());
    UncRespCode inet4src_ret (SetValue(inet4src_obj,in->inet4src_));
    if ( inet4src_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", inet4srckey.c_str());
      json_object_put(inet4src_obj);
    } else {
      int inet4src_setret(JsonBuildParse::build(inet4srckey,inet4src_obj,out));
      if ( inet4src_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", inet4srckey.c_str());
      }
    }
  }
  std::string inet4dstkey ("inet4dst");
  if ( in->inet4dst_ == NULL ) {
    pfc_log_error("No Contents in inet4dst");
  } else {
    json_object *inet4dst_obj (JsonBuildParse::create_json_obj());
    UncRespCode inet4dst_ret (SetValue(inet4dst_obj,in->inet4dst_));
    if ( inet4dst_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", inet4dstkey.c_str());
      json_object_put(inet4dst_obj);
    } else {
      int inet4dst_setret(JsonBuildParse::build(inet4dstkey,inet4dst_obj,out));
      if ( inet4dst_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", inet4dstkey.c_str());
      }
    }
  }
  std::string dscpkey ("dscp");
  if ( in->dscp_ == NULL ) {
    pfc_log_error("No Contents in dscp");
  } else {
    json_object *dscp_obj (JsonBuildParse::create_json_obj());
    UncRespCode dscp_ret (SetValue(dscp_obj,in->dscp_));
    if ( dscp_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", dscpkey.c_str());
      json_object_put(dscp_obj);
    } else {
      int dscp_setret(JsonBuildParse::build(dscpkey,dscp_obj,out));
      if ( dscp_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", dscpkey.c_str());
      }
    }
  }
  std::string tpsrckey ("tpsrc");
  if ( in->tpsrc_ == NULL ) {
    pfc_log_error("No Contents in tpsrc");
  } else {
    json_object *tpsrc_obj (JsonBuildParse::create_json_obj());
    UncRespCode tpsrc_ret (SetValue(tpsrc_obj,in->tpsrc_));
    if ( tpsrc_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", tpsrckey.c_str());
      json_object_put(tpsrc_obj);
    } else {
      int tpsrc_setret(JsonBuildParse::build(tpsrckey,tpsrc_obj,out));
      if ( tpsrc_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", tpsrckey.c_str());
      }
    }
  }
  std::string tpdstkey ("tpdst");
  if ( in->tpdst_ == NULL ) {
    pfc_log_error("No Contents in tpdst");
  } else {
    json_object *tpdst_obj (JsonBuildParse::create_json_obj());
    UncRespCode tpdst_ret (SetValue(tpdst_obj,in->tpdst_));
    if ( tpdst_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", tpdstkey.c_str());
      json_object_put(tpdst_obj);
    } else {
      int tpdst_setret(JsonBuildParse::build(tpdstkey,tpdst_obj,out));
      if ( tpdst_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", tpdstkey.c_str());
      }
    }
  }
  std::string icmpcodekey ("icmpcode");
  if ( in->icmpcode_ == NULL ) {
    pfc_log_error("No Contents in icmpcode");
  } else {
    json_object *icmpcode_obj (JsonBuildParse::create_json_obj());
    UncRespCode icmpcode_ret (SetValue(icmpcode_obj,in->icmpcode_));
    if ( icmpcode_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", icmpcodekey.c_str());
      json_object_put(icmpcode_obj);
    } else {
      int icmpcode_setret(JsonBuildParse::build(icmpcodekey,icmpcode_obj,out));
      if ( icmpcode_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", icmpcodekey.c_str());
      }
    }
  }
  std::string icmptypekey ("icmptype");
  if ( in->icmptype_ == NULL ) {
    pfc_log_error("No Contents in icmptype");
  } else {
    json_object *icmptype_obj (JsonBuildParse::create_json_obj());
    UncRespCode icmptype_ret (SetValue(icmptype_obj,in->icmptype_));
    if ( icmptype_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", icmptypekey.c_str());
      json_object_put(icmptype_obj);
    } else {
      int icmptype_setret(JsonBuildParse::build(icmptypekey,icmptype_obj,out));
      if ( icmptype_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", icmptypekey.c_str());
      }
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,destination* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string tenantkey ("tenant");
  int tenant_ret (JsonBuildParse::parse(in,tenantkey,out->tenant_));
  if ( tenant_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", tenantkey.c_str());
  }
  std::string bridgekey ("bridge");
  int bridge_ret (JsonBuildParse::parse(in,bridgekey,out->bridge_));
  if ( bridge_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", bridgekey.c_str());
  }
  std::string routerkey ("router");
  int router_ret (JsonBuildParse::parse(in,routerkey,out->router_));
  if ( router_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", routerkey.c_str());
  }
  std::string terminalkey ("terminal");
  int terminal_ret (JsonBuildParse::parse(in,terminalkey,out->terminal_));
  if ( terminal_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", terminalkey.c_str());
  }
  std::string interfacekey ("interface");
  int interface_ret (JsonBuildParse::parse(in,interfacekey,out->interface_));
  if ( interface_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", interfacekey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,destination* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string tenantkey ("tenant");
  if ( in->tenant_ == "" ) {
    pfc_log_info("string tenant empty");
  } else {
    int tenant_ret (JsonBuildParse::build(tenantkey,in->tenant_,out));
    if ( tenant_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", tenantkey.c_str());
    }
  }
  std::string bridgekey ("bridge");
  if ( in->bridge_ == "" ) {
    pfc_log_info("string bridge empty");
  } else {
    int bridge_ret (JsonBuildParse::build(bridgekey,in->bridge_,out));
    if ( bridge_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", bridgekey.c_str());
    }
  }
  std::string routerkey ("router");
  if ( in->router_ == "" ) {
    pfc_log_info("string router empty");
  } else {
    int router_ret (JsonBuildParse::build(routerkey,in->router_,out));
    if ( router_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", routerkey.c_str());
    }
  }
  std::string terminalkey ("terminal");
  if ( in->terminal_ == "" ) {
    pfc_log_info("string terminal empty");
  } else {
    int terminal_ret (JsonBuildParse::build(terminalkey,in->terminal_,out));
    if ( terminal_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", terminalkey.c_str());
    }
  }
  std::string interfacekey ("interface");
  if ( in->interface_ == "" ) {
    pfc_log_info("string interface empty");
  } else {
    int interface_ret (JsonBuildParse::build(interfacekey,in->interface_,out));
    if ( interface_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", interfacekey.c_str());
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,dldst* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string addresskey ("address");
  int address_ret (JsonBuildParse::parse(in,addresskey,out->address_));
  if ( address_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", addresskey.c_str());
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,dldst* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string addresskey ("address");
  if ( in->address_ == "" ) {
    return UNC_DRV_RC_ERR_GENERIC;
  } else {
    int address_ret (JsonBuildParse::build(addresskey,in->address_,out));
    if ( address_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", addresskey.c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,dlsrc* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string addresskey ("address");
  int address_ret (JsonBuildParse::parse(in,addresskey,out->address_));
  if ( address_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", addresskey.c_str());
    return UNC_DRV_RC_ERR_GENERIC;
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,dlsrc* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string addresskey ("address");
  if ( in->address_ == "" ) {
    return UNC_DRV_RC_ERR_GENERIC;
  } else {
    int address_ret (JsonBuildParse::build(addresskey,in->address_,out));
    if ( address_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", addresskey.c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,drop* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,drop* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string key ("");
  json_object *null_object(NULL);
  int ret (JsonBuildParse::build(key,null_object,out));
  if ( ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", key.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,dscp* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string dscpkey ("dscp");
  int dscp_ret (JsonBuildParse::parse(in,dscpkey,out->dscp_));
  if ( dscp_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", dscpkey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,dscp* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string dscpkey ("dscp");
  if ( in->dscp_ < 0 || in->dscp_ > 63 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int dscp_ret (JsonBuildParse::build(dscpkey,in->dscp_,out));
    if ( dscp_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", dscpkey.c_str());
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,filterType* out) {
  ODC_FUNC_TRACE;
  pfc_log_info("FLOWFILTER TYPE Parse");
  pfc_log_info("FLOWFILTER TYPE Parse:%s",restjson::JsonBuildParse::get_json_string(in));
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string passkey ("pass");
  std::string action_type("");
  json_object *pass_obj (JsonBuildParse::create_json_obj());
  // Clear memory when variable(pass_obj) is out of scope
  unc::restjson::json_obj_destroy_util pass_obj_delete_obj(pass_obj);
  //int pass_ret (JsonBuildParse::parse(in,passkey,pass_obj));
  int pass_ret (JsonBuildParse::parse(in,passkey,action_type));
  //action_type = restjson::JsonBuildParse::get_json_string(pass_obj);
  //pass_obj = restjson::JsonBuildParse::get_json_object(action_type.c_str());
  pfc_log_info("FLOWFILTER TYPE Parse pass action type %s",action_type.c_str());
  if ( pass_ret != REST_OP_SUCCESS || strlen(action_type.c_str()) == 0 ) {
    pfc_log_error(" PASS parse failed %s", passkey.c_str());
  } else {
     pfc_log_info("FLOWFILTER TYPE Parse Pass");
     pfc_log_info("pass_obj: %s",restjson::JsonBuildParse::get_json_string(pass_obj));
     out->pass_=new pass();
     UncRespCode pass_getret ( GetValue ( pass_obj,out->pass_));
     action_type = "";
    if ( pass_getret != UNC_RC_SUCCESS )
      return pass_getret;
  }
  std::string dropkey ("drop");
  json_object *drop_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util drop_obj_delete_obj(drop_obj);
  //int drop_ret (JsonBuildParse::parse(in,dropkey,drop_obj));
  //action_type = restjson::JsonBuildParse::get_json_string(drop_obj);
  int drop_ret (JsonBuildParse::parse(in,dropkey,action_type));
  pfc_log_info("FLOWFILTER TYPE Parse drop action type %s",action_type.c_str());
  if ( drop_ret != REST_OP_SUCCESS || strlen(action_type.c_str()) == 0) {
    pfc_log_error("DROP parse failed %s", dropkey.c_str());
  } else {
    pfc_log_info("FLOWFILTER TYPE Parse Drop");
    out->drop_=new drop();
    UncRespCode drop_getret ( GetValue ( drop_obj,out->drop_));
    action_type = "";
    if ( drop_getret != UNC_RC_SUCCESS )
      return drop_getret;
  }
  std::string redirectkey ("redirect");
  json_object *redirect_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util redirect_obj_delete_obj(redirect_obj);
  int redirect_ret (JsonBuildParse::parse(in,redirectkey,action_type));
  JsonBuildParse::parse(in,redirectkey,redirect_obj);
  //action_type = restjson::JsonBuildParse::get_json_string(drop_obj);
  pfc_log_info("FLOWFILTER TYPE Parse RD action type %s",action_type.c_str());
  if ( redirect_ret != REST_OP_SUCCESS || strlen(action_type.c_str()) == 0 ) {
    pfc_log_error("Redirect parse failed %s", redirectkey.c_str());
  } else {
    pfc_log_info("FLOWFILTER TYPE Parse Redirect");
    pfc_log_info("redirect_obj: %s",restjson::JsonBuildParse::get_json_string(redirect_obj));
    out->redirect_=new redirect();
    UncRespCode redirect_getret ( GetValue ( redirect_obj,out->redirect_));
    action_type = "";
    if ( redirect_getret != UNC_RC_SUCCESS )
      return redirect_getret;
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,filterType* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string passkey ("pass");
  if ( in->pass_ == NULL ) {
    pfc_log_error("No Contents in pass");
  } else {
    json_object *pass_obj (JsonBuildParse::create_json_obj());
    UncRespCode pass_ret (SetValue(pass_obj,in->pass_));
    if ( pass_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", passkey.c_str());
      json_object_put(pass_obj);
    } else {
      int pass_setret(JsonBuildParse::build(passkey,pass_obj,out));
      if ( pass_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", passkey.c_str());
      }
    }
  }
  std::string dropkey ("drop");
  if ( in->drop_ == NULL ) {
    pfc_log_error("No Contents in drop");
  } else {
    json_object *drop_obj (JsonBuildParse::create_json_obj());
    UncRespCode drop_ret (SetValue(drop_obj,in->drop_));
    if ( drop_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", dropkey.c_str());
      json_object_put(drop_obj);
    } else {
      int drop_setret(JsonBuildParse::build(dropkey,drop_obj,out));
      if ( drop_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", dropkey.c_str());
      }
    }
  }
  std::string redirectkey ("redirect");
  if ( in->redirect_ == NULL ) {
    pfc_log_error("No Contents in redirect");
  } else {
    json_object *redirect_obj (JsonBuildParse::create_json_obj());
    UncRespCode redirect_ret (SetValue(redirect_obj,in->redirect_));
    if ( redirect_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", redirectkey.c_str());
      json_object_put(redirect_obj);
    } else {
      int redirect_setret(JsonBuildParse::build(redirectkey,redirect_obj,out));
      if ( redirect_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", redirectkey.c_str());
      }
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,flowfilter* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string indexkey ("index");
  int index_ret (JsonBuildParse::parse(in,indexkey,out->index_));
  if ( index_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", indexkey.c_str());
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string conditionkey ("condition");
  int condition_ret (JsonBuildParse::parse(in,conditionkey,out->condition_));
  if ( condition_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", conditionkey.c_str());
    return UNC_DRV_RC_ERR_GENERIC;
  }
  std::string filterTypekey ("filterType");
  json_object *filterType_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util filterType_obj_delete_obj(filterType_obj);
  int filterType_ret (JsonBuildParse::parse(in,filterTypekey,filterType_obj));
  if ( filterType_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", filterTypekey.c_str());
  } else {
    out->filterType_=new filterType();
    UncRespCode filterType_getret ( GetValue ( filterType_obj,out->filterType_));
    if ( filterType_getret != UNC_RC_SUCCESS )
      return filterType_getret;
  }
  std::string actionkey ("actions");
  int array_length (JsonBuildParse::get_array_length(in,actionkey));
  if ( array_length == 0 ) {
    pfc_log_error("parse failed %s", actionkey.c_str());
  } else {
    json_object* array_obj(NULL);
    int action_ret(JsonBuildParse::parse(in,actionkey,array_obj));
    if ( action_ret != REST_OP_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;
    for( int count=0; count < array_length ; count ++ ) {
      json_object *array_entry(json_object_array_get_idx(array_obj,count));
      if ( array_entry == NULL )
        return UNC_DRV_RC_ERR_GENERIC;
      action* elem_=new action();
      UncRespCode action_getret(GetValue(array_entry, elem_));
      if ( action_getret != UNC_RC_SUCCESS )
        return action_getret;
      out->action_.push_back(elem_);
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,flowfilter* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string indexkey ("index");
  if ( in->index_ < 1 || in->index_ > 65535 ) {
    return UNC_DRV_RC_ERR_GENERIC;
  } else {
    int index_ret (JsonBuildParse::build(indexkey,in->index_,out));
    if ( index_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", indexkey.c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  std::string conditionkey ("condition");
  if ( in->condition_ == "" ) {
    return UNC_DRV_RC_ERR_GENERIC;
  } else {
    int condition_ret (JsonBuildParse::build(conditionkey,in->condition_,out));
    if ( condition_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", conditionkey.c_str());
      return UNC_DRV_RC_ERR_GENERIC;
    }
  }
  std::string filterTypekey ("filterType");
  if ( in->filterType_ == NULL ) {
    pfc_log_error("No Contents in filterType");
  } else {
    json_object *filterType_obj (JsonBuildParse::create_json_obj());
    UncRespCode filterType_ret (SetValue(filterType_obj,in->filterType_));
    if ( filterType_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", filterTypekey.c_str());
      json_object_put(filterType_obj);
    } else {
      int filterType_setret(JsonBuildParse::build(filterTypekey,filterType_obj,out));
      if ( filterType_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", filterTypekey.c_str());
      }
    }
  }
  std::string actionkey ("actions");
  json_object* action_array(JsonBuildParse::create_json_array_obj());
  std::list <action*>::iterator iter=in->action_.begin();
  while ( iter != in->action_.end() ) {
    if ( *iter == NULL ) {
      pfc_log_error("No Contents in action");
    } else {
      json_object* action_obj = JsonBuildParse::create_json_obj();
      UncRespCode action_ret(SetValue(action_obj,*iter));
      if ( action_ret != UNC_RC_SUCCESS ) {
        json_object_put(action_obj);
        json_object_put(action_array);
      return action_ret;
      }
      JsonBuildParse::add_to_array(action_array,action_obj);
      iter++;
    }
  }
  int action_setret (JsonBuildParse::build(actionkey,action_array,out));
  if ( action_setret != REST_OP_SUCCESS )
    return UNC_DRV_RC_ERR_GENERIC;
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,icmpcode* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string codekey ("code");
  int code_ret (JsonBuildParse::parse(in,codekey,out->code_));
  if ( code_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", codekey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,icmpcode* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
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
flowfilterlistUtil::GetValue(json_object *in,icmptype* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string typekey ("type");
  int type_ret (JsonBuildParse::parse(in,typekey,out->type_));
  if ( type_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", typekey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,icmptype* in) {
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
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,inet4dst* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string addresskey ("address");
  int address_ret (JsonBuildParse::parse(in,addresskey,out->address_));
  if ( address_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", addresskey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,inet4dst* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string addresskey ("address");
  if ( in->address_ == "" ) {
    pfc_log_info("string address empty");
  } else {
    int address_ret (JsonBuildParse::build(addresskey,in->address_,out));
    if ( address_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", addresskey.c_str());
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,inet4src* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string addresskey ("address");
  int address_ret (JsonBuildParse::parse(in,addresskey,out->address_));
  if ( address_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", addresskey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,inet4src* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string addresskey ("address");
  if ( in->address_ == "" ) {
    pfc_log_info("string address empty");
  } else {
    int address_ret (JsonBuildParse::build(addresskey,in->address_,out));
    if ( address_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", addresskey.c_str());
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,pass* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,pass* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string key ("");
  json_object *null_object(NULL);
  int ret (JsonBuildParse::build(key,null_object,out));
  if ( ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", key.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,redirect* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string destinationkey ("destination");
  json_object *destination_obj (JsonBuildParse::create_json_obj());
  unc::restjson::json_obj_destroy_util destination_obj_delete_obj(destination_obj);
  int destination_ret (JsonBuildParse::parse(in,destinationkey,destination_obj));
  if ( destination_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", destinationkey.c_str());
    return UNC_DRV_RC_ERR_GENERIC;
  } else {
    out->destination_=new destination();
    UncRespCode destination_getret ( GetValue ( destination_obj,out->destination_));
    if ( destination_getret != UNC_RC_SUCCESS )
      return destination_getret;
  }
  std::string outputkey ("output");
  int output_ret (JsonBuildParse::parse(in,outputkey,out->output_));
  if ( output_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", outputkey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,redirect* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string destinationkey ("destination");
  if ( in->destination_ == NULL ) {
    pfc_log_error("No Contents in destination");
    return UNC_DRV_RC_ERR_GENERIC;
  } else {
    json_object *destination_obj (JsonBuildParse::create_json_obj());
    UncRespCode destination_ret (SetValue(destination_obj,in->destination_));
    if ( destination_ret != UNC_RC_SUCCESS ) {
      pfc_log_error("build failed %s", destinationkey.c_str());
      json_object_put(destination_obj);
      return destination_ret;
    } else {
      int destination_setret(JsonBuildParse::build(destinationkey,destination_obj,out));
      if ( destination_setret != REST_OP_SUCCESS ) {
        pfc_log_error("build failed %s", destinationkey.c_str());
        return UNC_DRV_RC_ERR_GENERIC;
      }
    }
  }
  std::string outputkey ("output");
  int ret (JsonBuildParse::build(outputkey,in->output_,out));
  if ( ret != REST_OP_SUCCESS ) {
    pfc_log_error("build failed %s", outputkey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,tpdst* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string typekey ("type");
  int type_ret (JsonBuildParse::parse(in,typekey,out->type_));
  if ( type_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", typekey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,tpdst* in) {
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
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,tpsrc* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string typekey ("type");
  int type_ret (JsonBuildParse::parse(in,typekey,out->type_));
  if ( type_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", typekey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,tpsrc* in) {
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
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,vlanpcp* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string prioritykey ("priority");
  int priority_ret (JsonBuildParse::parse(in,prioritykey,out->priority_));
  if ( priority_ret != REST_OP_SUCCESS ) {
    pfc_log_error("parse failed %s", prioritykey.c_str());
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,vlanpcp* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string prioritykey ("priority");
  if ( in->priority_ < 0 || in->priority_ > 7 ) {
    pfc_log_info("Value in Wrong Range");
  } else {
    int priority_ret (JsonBuildParse::build(prioritykey,in->priority_,out));
    if ( priority_ret != REST_OP_SUCCESS ) {
      pfc_log_error("build failed %s", prioritykey.c_str());
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::GetValue(json_object *in,flowfilterlist* out) {
  ODC_FUNC_TRACE;
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string flowfilterkey ("flowfilter");
  int array_length (JsonBuildParse::get_array_length(in,flowfilterkey));
  if ( array_length == 0 ) {
    pfc_log_error("parse failed %s", flowfilterkey.c_str());
  } else {
    json_object* array_obj(NULL);
    int flowfilter_ret(JsonBuildParse::parse(in,flowfilterkey,array_obj));
    if ( flowfilter_ret != REST_OP_SUCCESS )
      return UNC_DRV_RC_ERR_GENERIC;
    for( int count=0; count < array_length ; count ++ ) {
      json_object *array_entry(json_object_array_get_idx(array_obj,count));
      if ( array_entry == NULL )
        return UNC_DRV_RC_ERR_GENERIC;
      flowfilter* elem_=new flowfilter();
      UncRespCode flowfilter_getret(GetValue(array_entry, elem_));
      if ( flowfilter_getret != UNC_RC_SUCCESS )
        return flowfilter_getret;
      out->flowfilter_.push_back(elem_);
    }
  }
  return UNC_RC_SUCCESS;
}


UncRespCode
flowfilterlistUtil::SetValue(json_object *out,flowfilterlist* in) {
  if ( in == NULL || out == NULL )
    return UNC_DRV_RC_ERR_GENERIC;
  std::string flowfilterkey ("flowfilter");
  json_object* flowfilter_array(JsonBuildParse::create_json_array_obj());
  std::list <flowfilter*>::iterator iter=in->flowfilter_.begin();
  while ( iter != in->flowfilter_.end() ) {
    if ( *iter == NULL ) {
      pfc_log_error("No Contents in flowfilter");
    } else {
      json_object* flowfilter_obj = JsonBuildParse::create_json_obj();
      UncRespCode flowfilter_ret(SetValue(flowfilter_obj,*iter));
       if ( flowfilter_ret != UNC_RC_SUCCESS ) {
          json_object_put(flowfilter_obj);
          json_object_put(flowfilter_array);
        return flowfilter_ret;
       }
       JsonBuildParse::add_to_array(flowfilter_array,flowfilter_obj);
       iter++;
    }
  }
  int flowfilter_setret (JsonBuildParse::build(flowfilterkey,flowfilter_array,out));
  if ( flowfilter_setret != REST_OP_SUCCESS )
    return UNC_DRV_RC_ERR_GENERIC;
  return UNC_RC_SUCCESS;
}


}
}
