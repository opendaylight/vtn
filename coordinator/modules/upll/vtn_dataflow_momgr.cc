/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vbr_if_momgr.hh"
#include "vtn_momgr.hh"
#include "vnode_child_momgr.hh"
#include "vterminal_momgr.hh"
#include "vbr_momgr.hh"
#include "vterm_if_momgr.hh"
#include "ipc_client_handler.hh"
#include "domain_check_util.hh"

#include "vtn_dataflow_momgr.hh"

using unc::upll::ipc_util::IpcUtil;
using unc::upll::ipc_util::IpcClientHandler;

namespace unc {
namespace upll {
namespace kt_momgr {

#define INGRESS 0
#define EGRESS  1
#define VEXT_INFO_LENGTH  (kMaxLenCtrlrId + kMaxLenDomainId + \
                           kMaxLenVtnName + kMaxLenVnodeName + 10)
upll_rc_t VtnDataflowMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                            ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!req || !ikey || !(ikey->get_key())) {
    UPLL_LOG_INFO("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (req->operation != UNC_OP_READ) {
     UPLL_LOG_INFO("Unsupported Operation for VTN Dataflow - %d",
                    req->operation);
     return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  if (req->datatype != UPLL_DT_STATE) {
     UPLL_LOG_INFO("Unsupported datatype for VTN Dataflow - %d",
                    req->datatype);
     return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  if (req->option1 != UNC_OPT1_NORMAL) {
    UPLL_LOG_INFO("Invalid option1 for VTN Dataflow - %d", req->option1);
    return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if (req->option2 != UNC_OPT2_NONE && req->option2 != UNC_OPT2_NO_TRAVERSING) {
    UPLL_LOG_INFO("Invalid option2 for VTN Dataflow - %d", req->option2);
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
  if (UNC_KT_VTN_DATAFLOW != ikey->get_key_type()) {
    UPLL_LOG_INFO("Invalid KeyType in the request (%d)",
                  ikey->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtnDataflow) {
    UPLL_LOG_INFO("Invalid Key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_cfg_val()) {
    UPLL_LOG_INFO("Value structure is not NULL for VTN Dataflow");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vtn_dataflow *vtn_df_key = reinterpret_cast<key_vtn_dataflow *>
                                 (ikey->get_key());
  return(ValidateVtnDataflowKey(vtn_df_key));
}

upll_rc_t VtnDataflowMoMgr::ValidateVtnDataflowKey(
    key_vtn_dataflow *vtn_df_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(reinterpret_cast<char *>(vtn_df_key->vtn_key.vtn_name),
                         kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("vtn name syntax check failed for Vtn Dataflow."
                  "Received vtn name - %s",
                  vtn_df_key->vtn_key.vtn_name);
    return ret_val;
  }
  ret_val = ValidateKey(reinterpret_cast<char *>(vtn_df_key->vnode_id),
                         kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("vnode name syntax check failed for Vtn Dataflow."
                  "Received vnode name - %s",
                  vtn_df_key->vnode_id);
    return ret_val;
  }
  if ((vtn_df_key->vlanid != 0xFFFF) &&
          !ValidateNumericRange(vtn_df_key->vlanid,
                                kMinVlanId, kMaxVlanId,
                                true, true)) {
    UPLL_LOG_INFO("Vlan Id Number check failed for Vtn Dataflow."
                  "Received vlan_id - %d",
                  vtn_df_key->vlanid);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  // TODO(rev): What kind of MAC validation need to be done for VTN Dataflow?
  if (!ValidateMacAddr(vtn_df_key->src_mac_address)) {
     UPLL_LOG_INFO("Mac Address validation failure for Vtn Dataflow."
                    " Received  mac_address is - %s",
                    vtn_df_key->src_mac_address);
      return UPLL_RC_ERR_CFG_SYNTAX;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnDataflowMoMgr::ValidateControllerCapability(
    const char *ctrlr_name, bool is_first_ctrlr,
    unc_keytype_ctrtype_t *ctrlr_type) {
  UPLL_FUNC_TRACE;

  if (!is_first_ctrlr) {
    if (!uuc::CtrlrMgr::GetInstance()->GetCtrlrType(
            ctrlr_name, UPLL_DT_RUNNING, ctrlr_type)) {
      UPLL_LOG_INFO("GetCtrlrType failed for ctrlr %s", ctrlr_name);
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_TRACE("Controller type is  %d", *ctrlr_type);
    if ((*ctrlr_type != UNC_CT_PFC) && (*ctrlr_type != UNC_CT_ODC)) {
      return UPLL_RC_SUCCESS;
    }
  }

  uint32_t max_attrs = 0;
  const uint8_t *attrs = NULL;
  if (!GetReadCapability(ctrlr_name, UNC_KT_VTN_DATAFLOW,
    &max_attrs, &attrs, UPLL_DT_RUNNING)) {
    UPLL_LOG_DEBUG("Read vtn_dataflow is not supported by controller %s",
                   ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnDataflowMoMgr::FillCtrlrDomCountMap(uint8_t *vtn_name,
                                       uint32_t  &ctrlr_dom_count,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_ckv = NULL;
  VtnMoMgr *vtnmgr = static_cast<VtnMoMgr *>((const_cast<MoManager *>
                        (GetMoManager(UNC_KT_VTN))));
  result_code = vtnmgr->GetChildConfigKey(tmp_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey failed result_code %d", result_code);
    return result_code;
  }
  uuu::upll_strncpy(reinterpret_cast<key_vtn *>(
          tmp_ckv->get_key())->vtn_name,
                    vtn_name, (kMaxLenVtnName + 1));
  result_code = vtnmgr->GetInstanceCount(tmp_ckv, NULL,
                UPLL_DT_RUNNING, &ctrlr_dom_count, dmi, CTRLRTBL);
  if (result_code != UPLL_RC_SUCCESS) {
     delete tmp_ckv;
     return result_code;
  }
  delete tmp_ckv;
  return result_code;
}

upll_rc_t VtnDataflowMoMgr::ConvertVexternaltoVbr(DataflowCmn *df_cmn,
                                                  const uint8_t *vtn_name,
                                                  uint8_t *vex_name,
                                                  uint8_t *vex_if_name,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *vbrif_mgr = static_cast<MoMgrImpl *>((const_cast<MoManager*>
                         (GetMoManager(UNC_KT_VBR_IF))));
  if (!vbrif_mgr) {
    UPLL_LOG_DEBUG("Instance is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ckv_if = NULL;
  result_code = vbrif_mgr->GetChildConfigKey(ckv_if, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed");
    return result_code;
  }
  key_vbr_if *if_key = reinterpret_cast<key_vbr_if *>(ckv_if->get_key());
  val_drv_vbr_if *if_val = reinterpret_cast<val_drv_vbr_if *>(ConfigKeyVal::
                           Malloc(sizeof(val_drv_vbr_if)));
  if_val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
  ckv_if->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, if_val));
  uuu::upll_strncpy(if_key->vbr_key.vtn_key.vtn_name, vtn_name,
                            (kMaxLenVtnName + 1));
  uuu::upll_strncpy(if_val->vex_name, vex_name, (kMaxLenVnodeName+1));
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain ,
    kOpInOutFlag };
  SET_USER_DATA_CTRLR(ckv_if, df_cmn->df_segment->vtn_df_common->controller_id);
  SET_USER_DATA_DOMAIN(ckv_if,
                       df_cmn->df_segment->vtn_df_common->ingress_domain);
  result_code = vbrif_mgr->ReadConfigDB(ckv_if, UPLL_DT_RUNNING, UNC_OP_READ,
                                        dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
     delete ckv_if;
     return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
     uuu::upll_strncpy(vex_name, if_key->vbr_key.vbridge_name,
                      (kMaxLenVnodeName+1));
     uuu::upll_strncpy(vex_if_name, if_key->if_name, (kMaxLenInterfaceName+1));
  }
  delete ckv_if;
  return result_code;
}

upll_rc_t VtnDataflowMoMgr::MapCtrlrNameToUncName(
                                   const uint8_t *vtn_name,
                                   DataflowCmn *df_cmn,
                                   DataflowUtil *df_util,
                                   DalDmlIntf *dmi,
                                   unc_keytype_ctrtype_t ctrlr_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  for (unsigned int iter = 0;
         iter < (df_cmn->df_segment->vtn_path_infos.size()); iter++) {
    val_vtn_dataflow_path_info *path_info =
                             df_cmn->df_segment->vtn_path_infos[iter];
    if (!path_info) {
      return UPLL_RC_ERR_GENERIC;
    }
    VnodeMoMgr *vnode_momgr = static_cast<VnodeMoMgr *>((const_cast<MoManager *>
                                        (GetMoManager(UNC_KT_VBRIDGE))));

    for (int iter = 0 ; iter < 2 ; iter++) {
      uint32_t indx = (iter == 0)?UPLL_IDX_IN_VNODE_VVDPI:
                                UPLL_IDX_OUT_VNODE_VVDPI;
      uint32_t if_indx = (iter == 0)?UPLL_IDX_IN_VIF_VVDPI:
                                  UPLL_IDX_OUT_VIF_VVDPI;
      uint8_t *node_name =(iter == 0)?path_info->in_vnode:path_info->out_vnode;
      uint8_t *node_if_name =(iter == 0)?path_info->in_vif:path_info->out_vif;

      if (path_info->valid[indx] == UNC_VF_INVALID ||
          path_info->valid[if_indx] == UNC_VF_INVALID) {
         UPLL_LOG_DEBUG("Vnode or Vnode interface is invalid in path"
                     "info %d", iter);
         continue;
      }
      if (ctrlr_type == UNC_CT_PFC) {
        /*
         * Converting redirect vexternal to vbr and interface
         */
        result_code = ConvertVexternaltoVbr(df_cmn, vtn_name,
                                            node_name,
                                            node_if_name, dmi);
        if (result_code == UPLL_RC_SUCCESS) {
          continue;
        } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          return result_code;
        }
      }

      ConfigKeyVal *ckv_vn = NULL;
      result_code = vnode_momgr->GetChildConfigKey(ckv_vn, NULL);
      if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed");
      return result_code;
      }
      key_vnode_t *vnode_key
          = reinterpret_cast<key_vnode_t*>(ckv_vn->get_key());

      uuu::upll_strncpy(vnode_key->vtn_key.vtn_name,
                  vtn_name, (kMaxLenVtnName + 1));
      val_rename_vnode_t* rename_val = reinterpret_cast<val_rename_vnode_t *>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
      uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                        node_name, (kMaxLenVnodeName + 1));
      rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
      ckv_vn->AppendCfgVal(IpctSt::kIpcInvalidStNum, rename_val);

      /* Gets unc vnode name based on the received controller
       * vnode name*/
      SET_USER_DATA_CTRLR(ckv_vn,
                          df_cmn->df_segment->vtn_df_common->controller_id);
      if (
        df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_DOMAIN_VVDC]
         == UNC_VF_VALID) {
        SET_USER_DATA_DOMAIN(ckv_vn,
                             df_cmn->df_segment->vtn_df_common->ingress_domain);
      }
      DbSubOp rename_dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                              kOpInOutNone };
      result_code = vnode_momgr->ReadConfigDB(ckv_vn, UPLL_DT_RUNNING,
                                              UNC_OP_READ, rename_dbop,
                                              dmi, RENAMETBL);
      if (result_code == UPLL_RC_SUCCESS) {
      // Store the pfc_name and renamed unc_name in map.
        std::string str_key(reinterpret_cast<char *>(node_name));
        std::string str_value(
             reinterpret_cast<char *>
             (reinterpret_cast<key_vnode_t*>(ckv_vn->get_key())->vnode_name));
        df_util->vnode_rename_map[str_key] = str_value;
        uuu::upll_strncpy(node_name,
             reinterpret_cast<key_vnode_t *>(ckv_vn->get_key())->vnode_name,
            (kMaxLenVnodeName + 1));
      } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        result_code = UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_INFO("ReadConfigdB Failed %d", result_code);
        delete ckv_vn;
        return result_code;
      }
      delete ckv_vn;
    }
  }
  return result_code;
}

upll_rc_t VtnDataflowMoMgr::MapCtrlrNameToUncName(
                                   const uint8_t *pfc_vtn_name,
                                   val_vtn_dataflow_path_info *path_info,
                                   uint8_t *ctrlr_id,
                                   DataflowUtil *df_util,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!path_info) {
    return UPLL_RC_ERR_GENERIC;
  }
  for (int iter = 0 ; iter < 2 ; iter++) {
    uint32_t indx = (iter == 0)?UPLL_IDX_IN_VNODE_VVDPI:
                                UPLL_IDX_OUT_VNODE_VVDPI;
    uint32_t if_indx = (iter == 0)?UPLL_IDX_IN_VIF_VVDPI:
                                  UPLL_IDX_OUT_VIF_VVDPI;
    uint8_t *node_name =(iter == 0)?path_info->in_vnode:path_info->out_vnode;
    if (path_info->valid[indx] == UNC_VF_INVALID ||
        path_info->valid[if_indx] == UNC_VF_INVALID) {
      UPLL_LOG_DEBUG("Vnode or Vnode interface is invalid in path"
                     "info %d", iter);
      continue;
    }
    result_code = RenamePathinfoNodes(node_name, pfc_vtn_name, ctrlr_id,
                                      df_util, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_INFO(" RenamePathinfoNodes failed err code %d", result_code);
     return result_code;
    }
  }
  return result_code;
}

upll_rc_t VtnDataflowMoMgr::RenamePathinfoNodes(uint8_t *node_name,
                                                const uint8_t *pfc_vtn_name,
                                                uint8_t *ctrlr_id,
                                                DataflowUtil *df_util,
                                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_vn = NULL;
  MoMgrImpl *vbr_mgr = static_cast<MoMgrImpl *>((const_cast<MoManager *>
                                        (GetMoManager(UNC_KT_VBRIDGE))));
  std::string node_key(reinterpret_cast<char *>(node_name));
  std::map<std::string, std::string>::iterator it =
      (df_util->vnode_rename_map).find(node_key);
  if (it !=  (df_util->vnode_rename_map).end()) {
        uuu::upll_strncpy(node_name, it->second.c_str(),
                          (kMaxLenVnodeName + 1));
  } else {
    result_code = vbr_mgr->GetChildConfigKey(ckv_vn, NULL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed");
      return result_code;
    }
    key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t*>(ckv_vn->get_key());

    uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                      pfc_vtn_name, (kMaxLenVtnName + 1));
    uuu::upll_strncpy(vbr_key->vbridge_name,
                            node_name, (kMaxLenVnodeName + 1));
    result_code = vbr_mgr->GetRenamedUncKey(ckv_vn, UPLL_DT_RUNNING, dmi,
                                           ctrlr_id);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_SUCCESS;
    } else if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetRenamedUncKey Failed %d", result_code);
      delete ckv_vn;
      return result_code;
    } else {
      std::string str_key(reinterpret_cast<char *>(node_name));
      std::string str_value(reinterpret_cast<char *>(vbr_key->vbridge_name));
      df_util->vnode_rename_map[str_key] = str_value;
      uuu::upll_strncpy(node_name, vbr_key->vbridge_name,
                          (kMaxLenVnodeName + 1));
    }
    delete ckv_vn;
  }
  return result_code;
}

upll_rc_t VtnDataflowMoMgr::UpdatePathInfoInterfaces(
                                                     DataflowCmn *df_cmn,
                                                     const uint8_t *vtn_name,
                                                     DataflowUtil *df_util,
                                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  unc_keytype_ctrtype_t ctrlr_type = UNC_CT_UNKNOWN;
  if (!uuc::CtrlrMgr::GetInstance()->GetCtrlrType(
          (const char *)df_cmn->df_segment->vtn_df_common->controller_id,
           UPLL_DT_RUNNING, &ctrlr_type)) {
    UPLL_LOG_INFO("GetCtrlrType failed for ctrlr %s",
                   df_cmn->df_segment->vtn_df_common->controller_id);
    return UPLL_RC_ERR_GENERIC;
  }

  uint32_t path_info_count;
// NULL check for df_cmn/df_segment done in calling functino
  if ((path_info_count =
           df_cmn->df_segment->vtn_df_common->path_info_count) == 0) {
    UPLL_LOG_TRACE("Path count is zero");
    return UPLL_RC_SUCCESS;
  }
  if (!vtn_name) {
    UPLL_LOG_INFO("Invalid vtn name \n");
    return UPLL_RC_ERR_GENERIC;
  }
  // Convert ingress and egress vex/vexif if vlink_flag is reset (redirected).
  val_vtn_dataflow_path_info *path_info =
                         df_cmn->df_segment->vtn_path_infos[0];
  uint32_t path_node_indx = UPLL_IDX_IN_VNODE_VVDPI;
  uint32_t path_nodeif_indx = UPLL_IDX_IN_VIF_VVDPI;
  int iter = 0;
  // convert/rename first and last path_info
  while (path_info && (ctrlr_type == UNC_CT_PFC)) {
    UPLL_LOG_TRACE("node:%s",
           DataflowCmn::get_string(*path_info).c_str());
    uint8_t *df_node_name =(iter == INGRESS)?
                               df_cmn->df_segment->vtn_df_common->ingress_vnode:
                                df_cmn->df_segment->vtn_df_common->egress_vnode;
    uint8_t *df_node_if_name =(iter == INGRESS)?
                          df_cmn->df_segment->vtn_df_common->ingress_vinterface:
                           df_cmn->df_segment->vtn_df_common->egress_vinterface;
    if ((path_info->valid[path_node_indx] == UNC_VF_VALID) &&
        (path_info->valid[path_nodeif_indx] == UNC_VF_VALID) &&
        (path_info->vlink_flag == UPLL_DATAFLOW_PATH_VLINK_EXISTS)) {
      if (!iter || (df_cmn->df_segment->is_flow_drop == false)) {
        if ((iter == INGRESS) &&
            (df_cmn->df_segment->vtn_df_common->valid[
                   UPLL_IDX_INGRESS_VNODE_VVDC] == UNC_VF_VALID) &&
            (df_cmn->df_segment->vtn_df_common->valid[
                   UPLL_IDX_INGRESS_VINTERFACE_VVDC] == UNC_VF_VALID)) {
          path_info->valid[path_node_indx] = UNC_VF_INVALID;
          path_info->valid[path_nodeif_indx] = UNC_VF_INVALID;
          uuu::upll_strncpy(path_info->out_vnode, df_node_name,
                         (kMaxLenVnodeName+1));
          uuu::upll_strncpy(path_info->out_vif, df_node_if_name,
                         (kMaxLenInterfaceName+1));
        } else if ((iter == EGRESS) &&
                  (df_cmn->df_segment->vtn_df_common->valid[
                     UPLL_IDX_EGRESS_VNODE_VVDC] == UNC_VF_VALID) &&
                  (df_cmn->df_segment->vtn_df_common->valid[
                     UPLL_IDX_EGRESS_VINTERFACE_VVDC] == UNC_VF_VALID)) {
          path_info->valid[path_node_indx] = UNC_VF_INVALID;
          path_info->valid[path_nodeif_indx] = UNC_VF_INVALID;
          uuu::upll_strncpy(path_info->in_vnode, df_node_name,
                           (kMaxLenVnodeName+1));
          uuu::upll_strncpy(path_info->in_vif, df_node_if_name,
                           (kMaxLenInterfaceName+1));
        }
      }
    }
    if ((path_node_indx == UPLL_IDX_OUT_VNODE_VVDPI) ||
       (path_info_count == 1)) {
      path_info = NULL;
    } else {
      path_node_indx = UPLL_IDX_OUT_VNODE_VVDPI;
      path_nodeif_indx = UPLL_IDX_OUT_VIF_VVDPI;
      path_info =  df_cmn->df_segment->vtn_path_infos[path_info_count - 1];
    }
  iter++;
  }
  if ((df_cmn->df_segment->is_flow_redirect == false) &&
      (ctrlr_type == UNC_CT_PFC)) {
    // rename intermediate nodes (they can be vexternal too)
    for (unsigned int iter = 1;
         iter < (df_cmn->df_segment->vtn_path_infos.size()-1); iter++) {
       path_info = df_cmn->df_segment->vtn_path_infos[iter];
       result_code = MapCtrlrNameToUncName(df_cmn->pfc_vtn_name,
                              path_info,
                              df_cmn->df_segment->vtn_df_common->controller_id,
                              df_util, dmi);
       if (UPLL_RC_SUCCESS != result_code) {
         UPLL_LOG_DEBUG("MapCtrlrNameToUncName failed %d", result_code);
         return result_code;
       }
    }
  } else {
    result_code = MapCtrlrNameToUncName(vtn_name, df_cmn, df_util, dmi,
                                        ctrlr_type);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("MapCtrlrNameToUncName failed %d", result_code);
      return result_code;
    }
    // Invoke RedirectCheck() to optimise the path information
    // to remove Intermediate vexternal information.
    RedirectCheck(df_cmn);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnDataflowMoMgr::MapVexternalToVbridge(
                                     const ConfigKeyVal *ckv_df,
                                     DataflowCmn *df_cmn,
                                     DataflowUtil *df_util,
                                     bool *is_vnode_match,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  key_vtn_dataflow_t *vtn_df_key = reinterpret_cast<key_vtn_dataflow_t *>
                                         (ckv_df->get_key());
  if (!vtn_df_key || !df_cmn) {
    UPLL_LOG_DEBUG("Input key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  controller_domain ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ckv_df, ctrlr_dom);
  unc_keytype_ctrtype_t ctrlr_type = UNC_CT_UNKNOWN;
  if (!uuc::CtrlrMgr::GetInstance()->GetCtrlrType(
          (const char *)ctrlr_dom.ctrlr, UPLL_DT_RUNNING, &ctrlr_type)) {
    UPLL_LOG_INFO("GetCtrlrType failed for ctrlr %s", ctrlr_dom.ctrlr);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("In and Out Domain %s", ctrlr_dom.domain);
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VbrIfMoMgr *vbrif_mgr = static_cast<VbrIfMoMgr *>((const_cast<MoManager *>
                        (GetMoManager(UNC_KT_VBR_IF))));
  if (!vbrif_mgr) {
    UPLL_LOG_DEBUG("Instance is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (df_cmn->df_segment->flow_traversed == 0) {
    if (ctrlr_dom.domain &&
        strlen(reinterpret_cast<const char*>(ctrlr_dom.domain))) {
       uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->ingress_domain,
                         ctrlr_dom.domain, kMaxLenDomainId+1);
       df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_DOMAIN_VVDC] =
                                                                  UNC_VF_VALID;
       if (df_cmn->df_segment->vtn_df_common->
           valid[UPLL_IDX_EGRESS_VINTERFACE_VVDC] == UNC_VF_VALID) {
         uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->egress_domain,
                       ctrlr_dom.domain, kMaxLenDomainId+1);
         df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_DOMAIN_VVDC] =
                                                                  UNC_VF_VALID;
       }
    }
    // Variables to store the original ingress/egress node info of a flow
    uint8_t in_vnode[kMaxLenVnodeName+1], in_vif[kMaxLenInterfaceName+1],
            out_vnode[kMaxLenVnodeName+1], out_vif[kMaxLenInterfaceName+1];
    uint8_t in_vnode_valid =
      df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_VNODE_VVDC];
    uint8_t in_vif_valid =
     df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_VINTERFACE_VVDC];
    uint8_t out_vnode_valid =
      df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_VNODE_VVDC];
    uint8_t out_vif_valid =
      df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_VINTERFACE_VVDC];

    const uint8_t *vtn_name = vtn_df_key->vtn_key.vtn_name;
    UPLL_LOG_TRACE("node:%s",
                   DataflowCmn::get_string
                   (*df_cmn->df_segment->vtn_df_common).c_str());
    ConfigKeyVal *ckv_vbrif[2] = {NULL, NULL};
    for (int iter = 0; iter < 2 ; ++iter) {
      ckv_vbrif[0] = ckv_vbrif[1] = NULL;
      uint8_t *vnode = NULL, *vnode_if = NULL;
      uint8_t *ctrlr_id = NULL, *domain_id = NULL;
      uint8_t valid[2];
      key_vbr_if_t *key_vbrif;
      if (iter == INGRESS) {
        valid[0] =
            df_cmn->df_segment->vtn_df_common->
            valid[UPLL_IDX_INGRESS_VNODE_VVDC];
        valid[1] = df_cmn->df_segment->
                 vtn_df_common->valid[UPLL_IDX_INGRESS_VINTERFACE_VVDC];
        vnode = df_cmn->df_segment->vtn_df_common->ingress_vnode;
        vnode_if = df_cmn->df_segment->vtn_df_common->ingress_vinterface;
        domain_id = df_cmn->df_segment->vtn_df_common->ingress_domain;
      } else {
        valid[0] =
            df_cmn->df_segment->vtn_df_common->
            valid[UPLL_IDX_EGRESS_VNODE_VVDC];
        valid[1] = df_cmn->df_segment->
                   vtn_df_common->valid[UPLL_IDX_EGRESS_VINTERFACE_VVDC];
        vnode = df_cmn->df_segment->vtn_df_common->egress_vnode;
        vnode_if = df_cmn->df_segment->vtn_df_common->egress_vinterface;
        domain_id = df_cmn->df_segment->vtn_df_common->egress_domain;
      }
      if (valid[0] != UNC_VF_VALID && valid[1] != UNC_VF_VALID) {
        UPLL_LOG_INFO("Ingress/Egress vNode/vInterface is not valid");
        result_code =
            (iter == INGRESS) ? UPLL_RC_SUCCESS : UPLL_RC_ERR_NO_SUCH_INSTANCE;
        continue;
      }
      ctrlr_id = df_cmn->df_segment->vtn_df_common->controller_id;
       char tmp_key[VEXT_INFO_LENGTH];
        snprintf(tmp_key, sizeof(tmp_key), "%d:%s:%s:%s:%s", iter, ctrlr_id,
                 domain_id,
                 reinterpret_cast<char *>(vtn_df_key->vtn_key.vtn_name),
                 vnode);
        UPLL_LOG_DEBUG("vexternal information in find map is %s", tmp_key);
        std::map<std::string, void* >::iterator it =
            (df_util->vext_info_map).find(tmp_key);
        if (it != (df_util->vext_info_map).end()) {
          if (iter == INGRESS) {
            uuu::upll_strncpy(in_vnode, vnode, (kMaxLenVnodeName + 1));
            uuu::upll_strncpy(in_vif, vnode_if, (kMaxLenInterfaceName + 1));
          } else if (iter == EGRESS) {
            uuu::upll_strncpy(out_vnode, vnode, (kMaxLenVnodeName + 1));
            uuu::upll_strncpy(out_vif, vnode_if, (kMaxLenInterfaceName + 1));
          }
          ConfigKeyVal *ckv_tmp  = reinterpret_cast<ConfigKeyVal *>(it->second);
          result_code = vbrif_mgr->DupConfigKeyVal(ckv_vbrif[iter], ckv_tmp);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("DupConfigKeyVal Failed");
            return result_code;
          }
          key_vbrif =
              reinterpret_cast<key_vbr_if_t *>(ckv_vbrif[iter]->get_key());
          uuu::upll_strncpy(vnode, key_vbrif->vbr_key.vbridge_name,
                          (kMaxLenVnodeName + 1));
          uuu::upll_strncpy(vnode_if, key_vbrif->if_name,
                        (kMaxLenInterfaceName + 1));
          DELETE_IF_NOT_NULL(ckv_vbrif[0]);
          if (iter == 1)
           df_cmn->df_segment->ckv_egress = ckv_vbrif[iter];
        } else {
          result_code = vbrif_mgr->GetChildConfigKey(ckv_vbrif[iter], NULL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("GetChildConfigKey Failed");
            return result_code;
          }
          key_vbrif = static_cast<key_vbr_if_t *>(ckv_vbrif[iter]->get_key());

           if (ctrlr_type == UNC_CT_ODC) {
             uuu::upll_strncpy(key_vbrif->vbr_key.vtn_key.vtn_name,
                          vtn_df_key->vtn_key.vtn_name, (kMaxLenVtnName + 1));
             SET_USER_DATA_CTRLR_DOMAIN(ckv_vbrif[iter], ctrlr_dom);
             VbrMoMgr *vbr_mgr = reinterpret_cast<VbrMoMgr *>
              (const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));
             ConfigKeyVal *vbr_ckv = NULL;
             result_code = vbrif_mgr->GetParentConfigKey(vbr_ckv,
                                                           ckv_vbrif[iter]);
             if (UPLL_RC_SUCCESS != result_code) {
              DELETE_IF_NOT_NULL(ckv_vbrif[0]);
              DELETE_IF_NOT_NULL(ckv_vbrif[1]);
              UPLL_LOG_INFO("GetChildConfigKey Failed %d", result_code);
              return result_code;
            }  // Magudees: please correct the indentations.
            val_rename_vnode_t* rename_val =
                reinterpret_cast<val_rename_vnode_t *>
                (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
            uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                            vnode, (kMaxLenVnodeName + 1));
            rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
            vbr_ckv->AppendCfgVal(IpctSt::kIpcInvalidStNum, rename_val);

            /* Gets unc vterminal name based on the received controller
             * vterminal name*/
            DbSubOp rename_dbop = { kOpReadSingle, kOpMatchCtrlr |
                                    kOpMatchDomain, kOpInOutNone };
            result_code = vbr_mgr->ReadConfigDB(
                vbr_ckv, UPLL_DT_RUNNING,
                UNC_OP_READ, rename_dbop, dmi, RENAMETBL);
            if (result_code == UPLL_RC_SUCCESS) {
              // Store pfc_vterm_name and renamed unc_vterm_name in map.
              std::string str_key(reinterpret_cast<char *>(vnode));
              std::string str_value(reinterpret_cast<char *>
                                   (reinterpret_cast<key_vbr_t *>
                                   (vbr_ckv->get_key())->vbridge_name));
              df_util->vnode_rename_map[str_key] = str_value;
              uuu::upll_strncpy(vnode, reinterpret_cast<key_vbr_t *>
                                (vbr_ckv->get_key())->vbridge_name,
                                (kMaxLenVnodeName + 1));
            } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
              DELETE_IF_NOT_NULL(ckv_vbrif[0]);
              DELETE_IF_NOT_NULL(ckv_vbrif[1]);
              DELETE_IF_NOT_NULL(vbr_ckv);
              UPLL_LOG_INFO("ReadConfigDB Failed %d", result_code);
              return result_code;
            }
            DELETE_IF_NOT_NULL(vbr_ckv);
            uuu::upll_strncpy(key_vbrif->vbr_key.vbridge_name,
                            vnode, (kMaxLenVnodeName + 1));
            uuu::upll_strncpy(key_vbrif->if_name,
                            vnode_if, (kMaxLenInterfaceName + 1));
            UPLL_LOG_TRACE("VBR_IF ckv %s",
                              ckv_vbrif[iter]->ToStrAll().c_str());
          } else {
            val_drv_vbr_if_t *drv_val_vbrif = static_cast<val_drv_vbr_if_t *>
                              (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if_t)));
            ckv_vbrif[iter]->SetCfgVal(
                new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, drv_val_vbrif));
            uuu::upll_strncpy(key_vbrif->vbr_key.vtn_key.vtn_name,
                       vtn_df_key->vtn_key.vtn_name, (kMaxLenVtnName + 1));
            uuu::upll_strncpy(drv_val_vbrif->vex_name,
                         vnode, (kMaxLenVnodeName + 1));
            uuu::upll_strncpy(drv_val_vbrif->vex_if_name,
                        vnode_if, (kMaxLenInterfaceName + 1));
            drv_val_vbrif->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
            drv_val_vbrif->valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] = UNC_VF_VALID;
          }
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                           kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain };
          result_code = vbrif_mgr->ReadConfigDB(ckv_vbrif[iter],
                            UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi, MAINTBL);
          // Magudees: keep the ReadConfigDB here if possible
          if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
           ConfigKeyVal *vtermif_ckv = NULL;
           /* Populates VTERM_IF key structure */
           VtermIfMoMgr *vtermif_mgr = reinterpret_cast<VtermIfMoMgr *>
            (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));
           if (!vtermif_mgr) {
             UPLL_LOG_INFO("Instance is NULL");
             DELETE_IF_NOT_NULL(ckv_vbrif[0]);
             DELETE_IF_NOT_NULL(ckv_vbrif[1]);
             return UPLL_RC_ERR_GENERIC;
           }
           result_code = vtermif_mgr->GetChildConfigKey(vtermif_ckv, NULL);
           if (UPLL_RC_SUCCESS != result_code) {
             DELETE_IF_NOT_NULL(ckv_vbrif[0]);
             DELETE_IF_NOT_NULL(ckv_vbrif[1]);
             UPLL_LOG_INFO("GetChildConfigKey Failed %d", result_code);
             return result_code;
           }
           key_vterm_if *key_vtermif = reinterpret_cast<key_vterm_if_t *>
                                               (vtermif_ckv->get_key());
           uuu::upll_strncpy(key_vtermif->vterm_key.vtn_key.vtn_name,
                          vtn_df_key->vtn_key.vtn_name, (kMaxLenVtnName + 1));
           SET_USER_DATA_CTRLR_DOMAIN(vtermif_ckv, ctrlr_dom);
           VterminalMoMgr *vterm_mgr = reinterpret_cast<VterminalMoMgr *>
            (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERMINAL)));
           ConfigKeyVal *vterm_ckv = NULL;
           result_code = vtermif_mgr->GetParentConfigKey(vterm_ckv,
                                                         vtermif_ckv);
           if (UPLL_RC_SUCCESS != result_code) {
            DELETE_IF_NOT_NULL(ckv_vbrif[0]);
            DELETE_IF_NOT_NULL(ckv_vbrif[1]);
            DELETE_IF_NOT_NULL(vtermif_ckv);
            UPLL_LOG_INFO("GetChildConfigKey Failed %d", result_code);
            return result_code;
          }
          val_rename_vnode_t* rename_val =
              reinterpret_cast<val_rename_vnode_t *>
              (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
          uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                          vnode, (kMaxLenVnodeName + 1));
          rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
          vterm_ckv->AppendCfgVal(IpctSt::kIpcInvalidStNum, rename_val);

          /* Gets unc vterminal name based on the received controller
           * vterminal name*/
          DbSubOp rename_dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                                  kOpInOutNone };
          result_code = vterm_mgr->ReadConfigDB(
              vterm_ckv, UPLL_DT_RUNNING,
              UNC_OP_READ, rename_dbop, dmi, RENAMETBL);
          if (result_code == UPLL_RC_SUCCESS) {
            // Store pfc_vterm_name and renamed unc_vterm_name in map.
            std::string str_key(reinterpret_cast<char *>(vnode));
            std::string str_value(reinterpret_cast<char *>
                                 (reinterpret_cast<key_vterm_t *>
                                 (vterm_ckv->get_key())->vterminal_name));
            df_util->vnode_rename_map[str_key] = str_value;
            uuu::upll_strncpy(vnode, reinterpret_cast<key_vterm_t *>
                              (vterm_ckv->get_key())->vterminal_name,
                              (kMaxLenVnodeName + 1));
          } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            DELETE_IF_NOT_NULL(ckv_vbrif[0]);
            DELETE_IF_NOT_NULL(ckv_vbrif[1]);
            DELETE_IF_NOT_NULL(vterm_ckv);
            DELETE_IF_NOT_NULL(vtermif_ckv);
            UPLL_LOG_INFO("ReadConfigDB Failed %d", result_code);
            return result_code;
          }
          DELETE_IF_NOT_NULL(vterm_ckv);
          uuu::upll_strncpy(key_vtermif->vterm_key.vterminal_name,
                          vnode, (kMaxLenVnodeName + 1));
          uuu::upll_strncpy(key_vtermif->if_name,
                          vnode_if, (kMaxLenInterfaceName + 1));
          UPLL_LOG_TRACE("VTERM_IF Ckv %s", vtermif_ckv->ToStrAll().c_str());
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
            kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain };
          result_code = vtermif_mgr->ReadConfigDB(
              vtermif_ckv, UPLL_DT_RUNNING,
              UNC_OP_READ, dbop, dmi, MAINTBL);
          DELETE_IF_NOT_NULL(ckv_vbrif[0]);
          DELETE_IF_NOT_NULL(vtermif_ckv);
          key_vtermif = NULL;
          key_vbrif = NULL;
          if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            result_code = UPLL_RC_SUCCESS;
          } else if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("VTERM_IF ReadConfigDB Failed result_code - %d",
                         result_code);
            DELETE_IF_NOT_NULL(ckv_vbrif[1]);
            return result_code;
          }
        } else if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("vbrif ReadConfigDB Failed err code - %d",
                    result_code);
          DELETE_IF_NOT_NULL(ckv_vbrif[0]);
          DELETE_IF_NOT_NULL(ckv_vbrif[1]);
          return result_code;
        } else {
          if (ctrlr_type == UNC_CT_PFC) {
            char tmp_key[VEXT_INFO_LENGTH];
            snprintf(tmp_key, sizeof(tmp_key), "%d:%s:%s:%s:%s", iter, ctrlr_id,
                     domain_id,
                     reinterpret_cast<char *>(vtn_df_key->vtn_key.vtn_name),
                     vnode);
            UPLL_LOG_DEBUG("vexternal info in insert map is %s", tmp_key);
            df_util->vext_info_map[string(tmp_key)] = ckv_vbrif[iter];
            key_vbrif =
                reinterpret_cast<key_vbr_if_t *>(ckv_vbrif[iter]->get_key());
            if (iter == INGRESS) {
              uuu::upll_strncpy(in_vnode, vnode, (kMaxLenVnodeName + 1));
              uuu::upll_strncpy(in_vif, vnode_if, (kMaxLenInterfaceName + 1));
            } else if (iter == EGRESS) {
              uuu::upll_strncpy(out_vnode, vnode, (kMaxLenVnodeName + 1));
              uuu::upll_strncpy(out_vif, vnode_if, (kMaxLenInterfaceName + 1));
            }
            uuu::upll_strncpy(vnode, key_vbrif->vbr_key.vbridge_name,
                            (kMaxLenVnodeName + 1));
            uuu::upll_strncpy(vnode_if, key_vbrif->if_name,
                          (kMaxLenInterfaceName + 1));
          }
        }
        if (iter == 1)
          df_cmn->df_segment->ckv_egress = ckv_vbrif[iter];
      }
    }

    if (df_cmn->df_segment->vtn_df_common->path_info_count !=
        df_cmn->df_segment->vtn_path_infos.size()) {
      UPLL_LOG_INFO("Path info size not consistent vtn_cmn path count = %d"
                     "and path_info size = %" PFC_PFMT_SIZE_T "",
                     df_cmn->df_segment->vtn_df_common->path_info_count,
                     df_cmn->df_segment->vtn_path_infos.size());
      return UPLL_RC_ERR_GENERIC;
    }

    if ((df_cmn->df_segment->is_flow_drop == false) &&
         df_cmn->df_segment->vtn_df_common->path_info_count) {
      // Dynamic Interface Check when Ingress info is INVALID
      if (df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_VNODE_VVDC]
          == UNC_VF_INVALID) {
        val_vtn_dataflow_path_info *path_info =
        df_cmn->df_segment->vtn_path_infos[0];
        upll_rc_t res_code = DynamicInterfaceCheck(df_cmn, df_util, path_info,
                                                   vtn_name, INGRESS, dmi);
        if (res_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("DynamicInterfaceCheck failed, result_code: %d",
                           res_code);
          DELETE_IF_NOT_NULL(ckv_vbrif[0]);
          return res_code;
        }
      }
      // Dynamic Interface Check when Egress info is INVALID
      if (df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_VNODE_VVDC]
          == UNC_VF_INVALID) {
        val_vtn_dataflow_path_info *last_path_info =
               (df_cmn->df_segment->vtn_path_infos).back();
        upll_rc_t res_code = DynamicInterfaceCheck(df_cmn, df_util,
                                                   last_path_info, vtn_name,
                                                   EGRESS, dmi);
        if (res_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("DynamicInterfaceCheck failed, result_code: %d",
                           res_code);
          DELETE_IF_NOT_NULL(ckv_vbrif[0]);
          return res_code;
        }
      }
    }
    key_vtn_dataflow *key_df =
        reinterpret_cast<key_vtn_dataflow*>(ckv_df->get_key());
    *is_vnode_match = (0 == strcmp(reinterpret_cast<const char *>
                      (df_cmn->df_segment->vtn_df_common->ingress_vnode),
                       reinterpret_cast<const char *>(key_df->vnode_id)));
    // If the given dataflow ingress vnode does not match the given vnode,
    // we need to filter the dataflow.
    UPLL_LOG_TRACE(" The Status of is_vnode_match %d", *is_vnode_match);
    if (!(*is_vnode_match)) {
       // Before filtering the flow, replace its ingress/egress node info
       // to actual ingress/egress node info received from deriver.
      if (in_vnode_valid == UNC_VF_VALID) {
        uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->ingress_vnode,
                          in_vnode, (kMaxLenVnodeName+1));
      }
      if (in_vif_valid == UNC_VF_VALID) {
        uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->ingress_vinterface,
                          in_vif, (kMaxLenInterfaceName+1));
      }
      if (out_vnode_valid == UNC_VF_VALID) {
        uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->egress_vnode,
                          out_vnode, (kMaxLenVnodeName+1));
      }
      if (out_vif_valid == UNC_VF_VALID) {
        uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->egress_vinterface,
                          out_vif, (kMaxLenInterfaceName+1));
      }
      DELETE_IF_NOT_NULL(ckv_vbrif[0]);
      return UPLL_RC_SUCCESS;
    }
    upll_rc_t res_code = UpdatePathInfoInterfaces(df_cmn, vtn_name,
                                                  df_util, dmi);
    if (UPLL_RC_SUCCESS != res_code) {
      UPLL_LOG_TRACE("UpdatePathInfoInterface failed %d", res_code);
      DELETE_IF_NOT_NULL(ckv_vbrif[0]);
      return res_code;
    }
    if (df_cmn->df_segment->vtn_df_common->path_info_count == 0) {
      UPLL_LOG_DEBUG("Path count is zero");
    }
    df_cmn->df_segment->flow_traversed++;
  }
  return result_code;
}

upll_rc_t VtnDataflowMoMgr::PopulateVnpOrVbypassBoundaryInfo(
                           ConfigKeyVal *&ckv_inif,
                           ConfigKeyVal *&ckv_egress,
                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
      const_cast<MoManager *>(GetMoManager(ckv_inif->get_key_type())));
  if (!mgr) {
    UPLL_LOG_INFO("Invalid mgr param");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(ckv_egress, ckv_inif);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Error in retrieving the Child ConfigKeyVal");
    return result_code;
  }
  uint8_t *ifname = reinterpret_cast <key_vnode_if *>
                    (ckv_egress->get_key())->vnode_if_name;
  memset(ifname, 0, kMaxLenInterfaceName);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr |
                                   kOpInOutDomain | kOpInOutFlag};
  /* Get the list interfaces for given parent keytype */
  result_code = mgr->ReadConfigDB(ckv_egress, UPLL_DT_RUNNING, UNC_OP_READ,
                                  dbop, dmi, MAINTBL);
  /* Any other DB error */
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(ckv_egress);
    return result_code;
  }
  return result_code;
}


upll_rc_t
VtnDataflowMoMgr::ReadMo(IpcReqRespHeader *header,
                         ConfigKeyVal *ckv_in,
                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  DataflowUtil df_util;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = ValidateMessage(header, ckv_in);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ValidateMessage failed result_code %d",
                  result_code);
    return result_code;
  }
  pfc::core::ipc::ServerSession *sess = reinterpret_cast
          <pfc::core::ipc::ServerSession *>(ckv_in->get_user_data());
  if (!sess) {
    UPLL_LOG_INFO("Empty session");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ckv_req = NULL;
  result_code = GetChildConfigKey(ckv_req, ckv_in);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey failed result_code %d",
                  result_code);
    return result_code;
  }
  key_vtn_dataflow_t *vtn_df_key = reinterpret_cast<key_vtn_dataflow_t *>
                                     (ckv_req->get_key());
  ConfigKeyVal *ckv_vnode = NULL;
  VnodeMoMgr *vnmgr = static_cast<VnodeMoMgr *>(
            (const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE))));
  result_code = vnmgr->GetChildConfigKey(ckv_vnode, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey failed result_code %d", result_code);
    delete ckv_req;
    return result_code;
  }
  key_vbr *vbr_key = reinterpret_cast<key_vbr *>(ckv_vnode->get_key());
  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                   vtn_df_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
  uuu::upll_strncpy(vbr_key->vbridge_name, vtn_df_key->vnode_id,
                         (kMaxLenVnodeName+1));
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr
                            |kOpInOutDomain|kOpInOutFlag };
     /* Get the controller domain using this read operation */
  result_code = vnmgr->ReadConfigDB(ckv_vnode, UPLL_DT_RUNNING, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_INFO("VBR_IF ReadConfigDB failed %d", result_code);
    delete ckv_vnode;
    delete ckv_req;
    return result_code;
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    DELETE_IF_NOT_NULL(ckv_vnode);
    VterminalMoMgr *vterm_mgr = static_cast<VterminalMoMgr *>(
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERMINAL))));
    result_code = vterm_mgr->GetChildConfigKey(ckv_vnode, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("GetChildConfigKey failed result_code %d", result_code);
      delete ckv_req;
      return result_code;
    }
    key_vterm *vterm_key = reinterpret_cast<key_vterm *>(ckv_vnode->get_key());
    uuu::upll_strncpy(vterm_key->vtn_key.vtn_name,
        vtn_df_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
    uuu::upll_strncpy(vterm_key->vterminal_name, vtn_df_key->vnode_id,
        (kMaxLenVnodeName+1));
    /* Get the controller domain using this read operation */
    result_code = vterm_mgr->ReadConfigDB(ckv_vnode, UPLL_DT_RUNNING,
                                          UNC_OP_READ, dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("VTERM_IF ReadConfigDB failed %d", result_code);
      delete ckv_vnode;
      delete ckv_req;
      return result_code;
    }
  }
    /* Set the controller and domain name in ckv*/
  SET_USER_DATA(ckv_req, ckv_vnode);
  delete ckv_vnode;
  unc_keytype_ctrtype_t ctrlr_type = UNC_CT_UNKNOWN;
  uint8_t *ctrlr_id = NULL;
  GET_USER_DATA_CTRLR(ckv_req, ctrlr_id);
  result_code = ValidateControllerCapability((const char *)(ctrlr_id), true,
                                             &ctrlr_type);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ValidateControllerCapability failed result_code %d",
                  result_code);
    delete ckv_req;
    return result_code;
  }

  // Check whether the given Controller domain is Spine/Leaf or Normal
  if (ctrlr_type == UNC_CT_PFC) {
    uint8_t *domain_id = NULL;
    GET_USER_DATA_DOMAIN(ckv_req, domain_id);
    UpplDomainType dom_type = UPPL_DOMAIN_TYPE_NORMAL;
    unc::upll::domain_util::DomainUtil domain_util_obj;
    result_code = domain_util_obj.GetDomainTypeFromPhysical(
                                      (const char *)(ctrlr_id),
                                      (const char *)(domain_id), &dom_type);
    if ((result_code != UPLL_RC_SUCCESS) &&
        (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_ERROR("UPPL returns error for READ:%d", result_code);
      delete ckv_req;
      return result_code;
    }
    // Spine/Leaf domain check
    if ((dom_type == UPPL_DOMAIN_TYPE_PF_LEAF) ||
        (dom_type == UPPL_DOMAIN_TYPE_PF_SPINE)) {
      UPLL_LOG_ERROR("VTN dataflow is not supported for Spine/Leaf domain");
      delete ckv_req;
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }

  uint32_t ctrlr_dom_count = 0;
  result_code = FillCtrlrDomCountMap(vtn_df_key->vtn_key.vtn_name,
                                     ctrlr_dom_count, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Unable to get max controller domain count\n");
    delete ckv_req;
    return result_code;
  }
  df_util.ctrlr_dom_count_map["nvtnctrlrdom"] = ctrlr_dom_count;
  result_code = TraversePFCController(ckv_req, header, NULL, NULL,
                                      &df_util, dmi, true);
  delete ckv_req;
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("TraversePFCController failed %d", result_code);
    } else {
      UPLL_LOG_INFO("TraversePFCController failed %d", result_code);
    }
    return result_code;
  } else {
    if (!IpcUtil::WriteKtResponse(sess, *header, ckv_in)) {
      UPLL_LOG_INFO("Failed to send response to key tree request");
      return UPLL_RC_ERR_GENERIC;
    }
    df_util.sessOutDataflows(*sess);
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t
VtnDataflowMoMgr::TraversePFCController(ConfigKeyVal *ckv_df,
                                        IpcReqRespHeader *header,
                                        DataflowCmn *currentnode,
                                        DataflowCmn *lastPfcNode,
                                        DataflowUtil *df_util,
                                        DalDmlIntf *dmi,
                                        bool is_first_ctrlr)   {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn_dataflow_t *vtn_df_key = reinterpret_cast<key_vtn_dataflow_t *>
                                   (ckv_df->get_key());
  uint8_t pfc_vtn_name[32] = {};
  if (!is_first_ctrlr) {
  /**
   * Reset the VlanID and Source Mac Address from the output matches
   * in the key structure while
   * retreiving flow segments
   **/

    map <UncDataflowFlowMatchType, void *>::iterator output_matches_iter;
    output_matches_iter = lastPfcNode->output_matches.find(UNC_MATCH_VLAN_ID);
    if (output_matches_iter != lastPfcNode->output_matches.end()) {
      val_df_flow_match_vlan_id_t *prev =
      reinterpret_cast<val_df_flow_match_vlan_id_t *>
                                     ((*output_matches_iter).second);
      vtn_df_key->vlanid =  prev->vlan_id;
    }
    output_matches_iter = lastPfcNode->output_matches.find(UNC_MATCH_DL_SRC);
    if (output_matches_iter != lastPfcNode->output_matches.end()) {
      val_df_flow_match_dl_addr_t *prev =
      reinterpret_cast<val_df_flow_match_dl_addr_t *>
      ((*output_matches_iter).second);
      memcpy(vtn_df_key->src_mac_address, prev->dl_addr,
                           sizeof(vtn_df_key->src_mac_address));
    }
  }
  controller_domain ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ckv_df, ctrlr_dom);
  if (!ctrlr_dom.ctrlr || !ctrlr_dom.domain) {
    UPLL_LOG_INFO("ctrlr_dom controller or domain is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtn_ctrlr_dataflow vtn_ctrlr_df_key(vtn_df_key,
                                          ctrlr_dom.ctrlr, ctrlr_dom.domain);
  vector<DataflowDetail *> pfc_flows;
  std::map<key_vtn_ctrlr_dataflow, vector<DataflowDetail *> >::iterator iter =
                           df_util->upll_pfc_flows.begin();
  for (; iter != df_util->upll_pfc_flows.end(); iter ++) {
      if (DataflowCmn::Compare((*iter).first, vtn_ctrlr_df_key)) {
       UPLL_LOG_DEBUG("Maching the key");
       break;
      }
  }
  if (iter == df_util->upll_pfc_flows.end()) {
    // Check whether the given Controller domain is Spine/Leaf or Normal
    unc_keytype_ctrtype_t ctrlr_type = UNC_CT_UNKNOWN;
    if (!uuc::CtrlrMgr::GetInstance()->GetCtrlrType(
            (const char *)ctrlr_dom.ctrlr, UPLL_DT_RUNNING, &ctrlr_type)) {
      UPLL_LOG_INFO("GetCtrlrType failed for ctrlr %s", ctrlr_dom.ctrlr);
      return UPLL_RC_ERR_GENERIC;
    }
    if (ctrlr_type == UNC_CT_PFC) {
      UpplDomainType dom_type = UPPL_DOMAIN_TYPE_NORMAL;
      unc::upll::domain_util::DomainUtil domain_util_obj;
      result_code = domain_util_obj.GetDomainTypeFromPhysical(
                              (const char*)(ctrlr_dom.ctrlr),
                              (const char*)(ctrlr_dom.domain), &dom_type);
      if ((result_code != UPLL_RC_SUCCESS) &&
          (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
        UPLL_LOG_ERROR("UPPL returns error for READ:%d", result_code);
        currentnode->addl_data->reason = UNC_DF_RES_SYSTEM_ERROR;
        return UPLL_RC_SUCCESS;
      }
      // Spine/Leaf domain check
      if ((dom_type == UPPL_DOMAIN_TYPE_PF_LEAF) ||
          (dom_type == UPPL_DOMAIN_TYPE_PF_SPINE)) {
        UPLL_LOG_ERROR("VTN dataflow is not supported for Spine/Leaf domain");
        currentnode->addl_data->reason = UNC_DF_RES_DOMAIN_NOT_SUPPORTED;
        return UPLL_RC_SUCCESS;
      }
    }
    ConfigKeyVal *ckv_dupdf = NULL;
    result_code = GetChildConfigKey(ckv_dupdf, ckv_df);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("GetChildConfigKey failed");
      return result_code;
    }
    uint8_t rename_flag = 0;
    GET_USER_DATA_FLAGS(ckv_df, rename_flag);
    if (rename_flag & VTN_RENAME) {
      ConfigKeyVal *ckv_vtn = NULL;
      MoMgrImpl *vtn_mgr = reinterpret_cast<MoMgrImpl *>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN)));
      if (!vtn_mgr) {
        UPLL_LOG_INFO("Invalid Momgr");
        delete ckv_dupdf;
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = vtn_mgr->GetChildConfigKey(ckv_vtn, ckv_df);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_INFO("GetChildConfigKey failed");
        delete ckv_dupdf;
        return result_code;
      }
      result_code = vtn_mgr->GetRenamedControllerKey(ckv_vtn,
                                UPLL_DT_RUNNING, dmi, &ctrlr_dom);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_INFO("GetRenamedControllerKey Failed %d",
                       result_code);

        delete ckv_vtn;
        delete ckv_dupdf;
        return result_code;
      }
      key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t*>
                           (ckv_vtn->get_key());
      // Store pfc_vtn_name in temporary string.
      // used later to update pfc_vtn_name in df_cmn object
      uuu::upll_strncpy(pfc_vtn_name,
             reinterpret_cast<char *>(vtn_key->vtn_name), (kMaxLenVtnName+1));
      key_vtn_dataflow *dup_dfkey =
          reinterpret_cast<key_vtn_dataflow_t *>(ckv_dupdf->get_key());
      uuu::upll_strncpy(dup_dfkey->vtn_key.vtn_name,
                        vtn_key->vtn_name, (kMaxLenVtnName+1));
      delete ckv_vtn;
    }
    IpcClientHandler  ipc_client;
    IpcRequest  ipc_req;
    memset(&ipc_req, 0, sizeof(ipc_req));
    memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
    ipc_req.ckv_data = ckv_dupdf;
    IpcResponse *ipc_resp = &(ipc_client.ipc_resp);
    unc_keytype_option2_t option2 = ipc_req.header.option2;
    if (ipc_req.header.option2 == UNC_OPT2_NO_TRAVERSING) {
           ipc_req.header.option2 = UNC_OPT2_NONE;
    }
    if (!ipc_client.SendReqToDriver(reinterpret_cast<const char *>
            (ctrlr_dom.ctrlr),
            (reinterpret_cast<char *>(ctrlr_dom.domain)),
            &ipc_req)) {
       UPLL_LOG_INFO("SendReqToDriver failed");
       if (option2 == UNC_OPT2_NO_TRAVERSING)
         ipc_resp->header.option2 = UNC_OPT2_NONE;
       delete ckv_dupdf;
       return ipc_resp->header.result_code;
    }
    if (option2 == UNC_OPT2_NO_TRAVERSING)
      ipc_resp->header.option2 = UNC_OPT2_NONE;
    if (ipc_resp->header.result_code != UPLL_RC_SUCCESS) {
       if (ipc_resp->header.result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         UPLL_LOG_DEBUG("Read from driver failed err code %d",
                                 ipc_resp->header.result_code);
       } else {
         UPLL_LOG_INFO("Read from driver failed err code %d",
                                 ipc_resp->header.result_code);
       }
       delete ckv_dupdf;
       return ipc_resp->header.result_code;
    }
    uint32_t    arg = ipc_client.arg;
    UPLL_LOG_TRACE(" The Argument is %d", arg);
    pfc::core::ipc::ClientSession *cl_sess = ipc_client.cl_sess;
    uint32_t tot_flow_count = 0;
    int err = 0;
    if (0 != (err = cl_sess->getResponse(arg++, tot_flow_count))) {
      UPLL_LOG_TRACE("Failed to get total flow count field #%u."
            " Err=%d", arg, err);
      if (is_first_ctrlr) {
        UPLL_LOG_TRACE("Inside if (is_head_node) and returning");
        delete ckv_dupdf;
        return UPLL_RC_ERR_GENERIC;
      } else {
        currentnode->addl_data->reason = UNC_DF_RES_SYSTEM_ERROR;
        UPLL_LOG_TRACE("Inside else and returning UNC_RC_SUCCESS");
        delete ckv_dupdf;
        return UPLL_RC_SUCCESS;
      }
    }
    UPLL_LOG_TRACE("Total flow count is %d", tot_flow_count);
    for (uint32_t i = 0; i < tot_flow_count; i++) {
      DataflowDetail *df_segm = new DataflowDetail(kidx_val_vtn_dataflow_cmn,
                                                   ctrlr_type);
      df_segm->sessReadDataflow(*cl_sess, arg);
      pfc_flows.push_back(df_segm);
    }
    df_util->upll_pfc_flows.insert(std::pair<key_vtn_ctrlr_dataflow,
                     vector<DataflowDetail *> > (vtn_ctrlr_df_key, pfc_flows));
    UPLL_LOG_DEBUG(
        "Got upll_pfc_flows from driver. flows.size=%" PFC_PFMT_SIZE_T
        "", pfc_flows.size());
    delete ckv_dupdf;
  } else {
    pfc_flows = iter->second;
    UPLL_LOG_DEBUG("Got pfc_flows from map. flows.size=%" PFC_PFMT_SIZE_T "",
                 pfc_flows.size());
  }
  for (uint32_t i = 0; i < pfc_flows.size(); i++) {
    DataflowDetail *df_segm = pfc_flows[i];
    bool is_vnode_match = false;
    DataflowCmn *df_cmn = new DataflowCmn(is_first_ctrlr, df_segm);
    if (!is_first_ctrlr) {
      bool match_result = df_cmn->check_match_condition
                                      (lastPfcNode->output_matches);
      if (!match_result) {
          UPLL_LOG_DEBUG("2nd flow (id=%" PFC_PFMT_u64
            ") is not matching with 1st flow (id=%" PFC_PFMT_u64
            ") so ignoring", df_cmn->df_segment->vtn_df_common->flow_id,
            currentnode->df_segment->vtn_df_common->flow_id);
        delete df_cmn;
        continue;
      }
    }
    // Store pfc_vtn_name in df_cmn object.
    uuu::upll_strncpy(df_cmn->pfc_vtn_name, pfc_vtn_name,
                     (kMaxLenVtnName+1));

    result_code = MapVexternalToVbridge(ckv_df,
                                        df_cmn, df_util, &is_vnode_match, dmi);
    if (result_code == UPLL_RC_SUCCESS && !is_vnode_match) {
      UPLL_LOG_INFO("Ingress vnode does not match with filter vnode");
      delete df_cmn;
      continue;
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_INFO("Either ingress/Egress is a dynamic interface");
        UpdateReason(df_cmn, result_code);
    } else if (UPLL_RC_SUCCESS != result_code) {
        delete df_cmn;
        return result_code;
    }
    if (is_first_ctrlr) {
      df_cmn->apply_action();
      uint32_t ret = df_util->appendFlow(df_cmn);
      if (ret != 0) {
        delete df_cmn;
        UPLL_LOG_INFO("appendFlow failed");
        return UPLL_RC_ERR_GENERIC;
      }
    } else {
        UPLL_LOG_DEBUG("2nd flow (id=%" PFC_PFMT_u64
                      ") is matching with 1st flow (id=%" PFC_PFMT_u64  ")",
           df_cmn->df_segment->vtn_df_common->flow_id,
           currentnode->df_segment->vtn_df_common->flow_id);
        df_cmn->apply_action();
        currentnode->appendFlow(df_cmn, *(df_util->get_ctrlr_dom_count_map()));
        if (currentnode->addl_data->reason == UNC_DF_RES_EXCEEDS_HOP_LIMIT) {
          UPLL_LOG_DEBUG("flow reached max hop limit");
          delete df_cmn;
          continue;
        }
      }
  }
  vector<DataflowCmn* >* firstCtrlrFlows = df_util->get_firstCtrlrFlows();
  if (is_first_ctrlr) {
    if (firstCtrlrFlows->size() == 0) {
        return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  } else {
      if (currentnode->next.size() == 0 && currentnode->addl_data->reason ==
                                UNC_DF_RES_SUCCESS) {  // Preserving old reason
        if ((currentnode->df_segment->vtn_df_common->controller_type ==
             UNC_CT_PFC) ||
           (currentnode->df_segment->vtn_df_common->controller_type ==
            UNC_CT_ODC)) {                        //  if parentnode is PFC type
          currentnode->addl_data->reason = UNC_DF_RES_FLOW_NOT_FOUND;
        } else {
          currentnode->addl_data->reason = UNC_DF_RES_DST_NOT_REACHED;
        }
        return UPLL_RC_SUCCESS;
      }
  }
  if (header->option2 == UNC_OPT2_NO_TRAVERSING) {
     UPLL_LOG_TRACE(" Traversing stopped with the input first controller");
     return UPLL_RC_SUCCESS;
  }
  if (is_first_ctrlr) {
      vector<DataflowCmn *>::iterator iter_flow = firstCtrlrFlows->begin();
      while (iter_flow != firstCtrlrFlows->end()) {
        // Checking the particular flow is traversed
        DataflowCmn *traverse_flow_cmn =
                              reinterpret_cast<DataflowCmn *>(*iter_flow);
        UPLL_LOG_TRACE("node:%s",
           DataflowCmn::get_string(*traverse_flow_cmn->
                              df_segment->vtn_df_common).c_str());
          if (traverse_flow_cmn->addl_data->reason !=
             UNC_DF_RES_EXCEEDS_FLOW_LIMIT) {
            result_code = CheckBoundaryAndTraverse(ckv_df, header,
                          traverse_flow_cmn, traverse_flow_cmn, df_util, dmi);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_TRACE("CheckBoundaryAndTraverse Failed %d\n",
                              result_code);
              return result_code;
            }
            vector<DataflowCmn *>::iterator match_flow = iter_flow + 1;
            unsigned int no_of_dataflow = 1;
            while (match_flow != firstCtrlrFlows->end()) {
              DataflowCmn *traverse_match_flow_cmn =
                  reinterpret_cast<DataflowCmn *>(*match_flow);
              if ((traverse_flow_cmn->next.size() > 0) &&
                  (traverse_match_flow_cmn->addl_data->reason !=
                   UNC_DF_RES_EXCEEDS_FLOW_LIMIT)) {
                UPLL_LOG_DEBUG("Inside first ctrlr, if traversed == false");
                if (traverse_match_flow_cmn->
                    CompareVtnDataflow(traverse_flow_cmn) == true)  {
                   no_of_dataflow++;
                   UPLL_LOG_DEBUG("CompareVtnDataflow returns true, no of df ="
                                  "%d max_dataflow_traverse_count %d",
                                  no_of_dataflow, upll_max_dataflow_traversal_);
                   if (no_of_dataflow > upll_max_dataflow_traversal_) {
                     UPLL_LOG_DEBUG("Setting flow limit to %p",
                                    traverse_match_flow_cmn);
                     traverse_match_flow_cmn->addl_data->reason =
                         UNC_DF_RES_EXCEEDS_FLOW_LIMIT;
                     traverse_match_flow_cmn->addl_data->controller_count = 1;
                   }
                 }
              }
              match_flow++;
            }
          }
          iter_flow++;
          (df_util->bypass_dom_set).clear();
       }
    } else {
      vector<DataflowCmn *>::iterator iter_flow = currentnode->next.begin();
      while (iter_flow != currentnode->next.end()) {
        // Checking the particular flow is traversed
        DataflowCmn *traverse_flow_cmn =
                              reinterpret_cast<DataflowCmn *>(*iter_flow);
        UPLL_LOG_TRACE("node:%s",
           DataflowCmn::get_string(*traverse_flow_cmn->
                               df_segment->vtn_df_common).c_str());
        if (traverse_flow_cmn->addl_data->reason !=
            UNC_DF_RES_EXCEEDS_FLOW_LIMIT) {
          result_code = CheckBoundaryAndTraverse(ckv_df, header,
                        *iter_flow, *iter_flow, df_util, dmi);
          #if 0
          if (result_code == UPLL_RC_SUCCESS) {
            if ((*iter_flow)->addl_data->reason != UNC_DF_RES_SUCCESS) {
              currentnode->deleteflow(*iter_flow);
              delete *iter_flow;
            }
          } else {
            UpdateReason(currentnode, result_code);
            return UPLL_RC_SUCCESS;
          }
          #endif
          UPLL_LOG_TRACE("CheckBoundaryAndTraverse nohead returned %d\n",
                         result_code);
          vector<DataflowCmn *>::iterator match_flow = iter_flow + 1;
          unsigned int no_of_dataflow = 1;
          while (match_flow != currentnode->next.end()) {
            DataflowCmn *traverse_match_flow_cmn =
                reinterpret_cast<DataflowCmn *>(*match_flow);
            if ((traverse_flow_cmn->next.size() > 0) &&
                (traverse_match_flow_cmn->addl_data->reason !=
                   UNC_DF_RES_EXCEEDS_FLOW_LIMIT)) {
              UPLL_LOG_DEBUG("Inside if traversed = false if headnode\n");
              if (traverse_match_flow_cmn->CompareVtnDataflow(traverse_flow_cmn)
                                                                     == true) {
                no_of_dataflow++;
                UPLL_LOG_DEBUG("CompareVtndataflow returns true, node max_df"
                            "%d:%d\n", no_of_dataflow,
                            upll_max_dataflow_traversal_);
                if (no_of_dataflow > upll_max_dataflow_traversal_) {
                  traverse_match_flow_cmn->addl_data->reason =
                      UNC_DF_RES_EXCEEDS_FLOW_LIMIT;
                }
              }
            }
            match_flow++;
          }
        }
        iter_flow++;
       }
    }
    return UPLL_RC_SUCCESS;
}

upll_rc_t VtnDataflowMoMgr::UpdateReason(DataflowCmn *source_node,
                                          upll_rc_t result_code) {
  UPLL_FUNC_TRACE;
  if (result_code != UPLL_RC_SUCCESS) {
    switch (result_code) {
      case UPLL_RC_ERR_NO_SUCH_INSTANCE:
        if ((source_node->df_segment->vtn_df_common->controller_type ==
             UNC_CT_PFC) ||
            (source_node->df_segment->vtn_df_common->controller_type ==
             UNC_CT_ODC))
          source_node->addl_data->reason = UNC_DF_RES_FLOW_NOT_FOUND;
        else
          source_node->addl_data->reason = UNC_DF_RES_DST_NOT_REACHED;
        break;
      case UPLL_RC_ERR_RESOURCE_DISCONNECTED:
      case UPLL_RC_ERR_CTR_DISCONNECTED:
        source_node->addl_data->reason = UNC_DF_RES_CTRLR_DISCONNECTED;
        break;
      case UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR:
        source_node->addl_data->reason = UNC_DF_RES_OPERATION_NOT_SUPPORTED;
        break;
      default:
        source_node->addl_data->reason = UNC_DF_RES_SYSTEM_ERROR;
    }
  }
  return result_code;
}

upll_rc_t
VtnDataflowMoMgr::CheckBoundaryAndTraverse(ConfigKeyVal *ckv_df,
                                           IpcReqRespHeader *header,
                                           DataflowCmn *source_node,
                                           DataflowCmn *lastPfcNode,
                                           DataflowUtil *df_util,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_remif = NULL;
  if_type vnif_type;
  if (!source_node || !lastPfcNode) {
    UPLL_LOG_DEBUG("DataflowCmn is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtn_dataflow *vtn_df_key = reinterpret_cast<key_vtn_dataflow *>
                                 (ckv_df->get_key());
  ConfigKeyVal *ckv_ingress =
       reinterpret_cast<ConfigKeyVal *>(source_node->df_segment->ckv_egress);
  if (!ckv_ingress) {
//    source_node->addl_data->reason = UNC_DF_RES_SUCCESS;
    UPLL_LOG_DEBUG("Egress interface not specified");
    return  UPLL_RC_SUCCESS;
  }
  VnodeChildMoMgr *vnif_mgr = reinterpret_cast<VnodeChildMoMgr *>(
        const_cast<MoManager *>(GetMoManager(ckv_ingress->get_key_type())));
  if (!vnif_mgr) {
    UPLL_LOG_ERROR("Invalid mgr\n");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = vnif_mgr->GetInterfaceType(ckv_ingress,
                                           UNC_VF_INVALID, vnif_type);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  if (vnif_type == kBoundaryInterface) {
    VlinkMoMgr *vlink_mgr = reinterpret_cast<VlinkMoMgr *>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
    if (!vlink_mgr) {
      UPLL_LOG_ERROR("Invalid mgr\n");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = vlink_mgr->GetRemoteIf(ckv_ingress, ckv_remif, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UpdateReason(source_node, UPLL_RC_ERR_GENERIC);
      DELETE_IF_NOT_NULL(ckv_remif);
      return  UPLL_RC_SUCCESS;
    }
  } else {
    source_node->addl_data->reason = UNC_DF_RES_SUCCESS;
    UPLL_LOG_DEBUG("Egress interface is not boundary mapped");
    return  UPLL_RC_SUCCESS;
  }

  uint8_t *ctrlr_id = NULL;
  unc_keytype_ctrtype_t ctrlr_type = UNC_CT_UNKNOWN;
  GET_USER_DATA_CTRLR(ckv_remif, ctrlr_id);
  if (UNC_KT_VUNK_IF != ckv_remif->get_key_type()) {
    if (ctrlr_id && IsUnifiedVbr(ctrlr_id)) {
      source_node->addl_data->reason = UNC_DF_RES_DOMAIN_NOT_SUPPORTED;
      DELETE_IF_NOT_NULL(ckv_remif);
      return UPLL_RC_SUCCESS;
    }
    result_code = ValidateControllerCapability((const char *)(ctrlr_id),
                                             false, &ctrlr_type);
    if (result_code != UPLL_RC_SUCCESS) {
      UpdateReason(source_node, result_code);
      delete ckv_remif;
      return UPLL_RC_SUCCESS;
    }
  }
  if ((ctrlr_type == UNC_CT_PFC) || (ctrlr_type == UNC_CT_ODC)) {
    uuu::upll_strncpy(vtn_df_key->vnode_id,
      reinterpret_cast<key_vbr_if_t *>(ckv_remif->get_key())->
      vbr_key.vbridge_name, (kMaxLenVnodeName + 1));
    if (GetVal(ckv_remif) != NULL && reinterpret_cast<val_vbr_if *>
       (GetVal(ckv_remif))->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
       UNC_VF_VALID) {
       #if 0
       val_df_flow_match_vlan_id_t *vlan_id = NULL;
       vlan_id = new val_df_flow_match_vlan_id_t;
       vlan_id->vlan_id  = reinterpret_cast<val_vbr_if *>
                           (GetVal(ckv_remif))->portmap.vlan_id;
       source_node->output_matches[UNC_MATCH_VLAN_ID] = vlan_id;
       #endif
       map <UncDataflowFlowMatchType, void *>::iterator output_matches_iter;
       output_matches_iter =
           lastPfcNode->output_matches.find(UNC_MATCH_VLAN_ID);
       if (output_matches_iter != lastPfcNode->output_matches.end()) {
        (reinterpret_cast<val_df_flow_match_vlan_id_t *>
         ((*output_matches_iter).second))->vlan_id =
            reinterpret_cast<val_vbr_if *> (GetVal(ckv_remif))->portmap.vlan_id;
       }
    }
    SET_USER_DATA(ckv_df, ckv_remif);
    delete ckv_remif;
    result_code = TraversePFCController(ckv_df, header, source_node,
                                        lastPfcNode, df_util, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UpdateReason(source_node, result_code);
      return UPLL_RC_SUCCESS;
    }
  } else {
      // Get the egress interfaces of the boundary(ies)
      // leading out of the vnp/vbypass domain
      // into the next neighboring controller domain.
      // The List of vnp/vbypass boundary information
      // available in the ckv_egress.
      // the Ingress for the PFC to VNP/Vbypass availbe in
      // ckv_remif first iteration.
      ConfigKeyVal *ckv_egress = NULL;
      bool found_inif = false;
      if_type vnif_type = kUnboundInterface;
      result_code = PopulateVnpOrVbypassBoundaryInfo(ckv_remif,
                                               ckv_egress, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Retrieval of boundary info failed\n %d\n", result_code);
        UpdateReason(source_node, UPLL_RC_ERR_GENERIC);
        return UPLL_RC_SUCCESS;
      }
      ConfigKeyVal *nxt_ckv = NULL;
      uint8_t *bypass_domain[2] = {NULL, NULL};
      std::pair<std::set<std::string>::iterator, bool> ret;
      GET_USER_DATA_DOMAIN(ckv_remif, bypass_domain[0]);
      ConfigKeyVal *ckv_tmp_nxt_ckv = NULL;
      nxt_ckv = ckv_egress;
      #if 1
      if (ctrlr_type == UNC_CT_UNKNOWN) {
      //  std::pair<std::string, std::string> bypass_dom_if;
         key_vnode_if_t *key = reinterpret_cast<key_vnode_if_t *>(
                            ckv_remif->get_key());
      #if 0
      bypass_dom_if = std::make_pair(
                   (string(reinterpret_cast<char *>(bypass_domain[0]))),
                   (string(reinterpret_cast<char *>(key->vnode_if_name))));
      #endif
        char bypass_dom_if[66];
        snprintf(bypass_dom_if, sizeof(bypass_dom_if), "%s:%s",
                bypass_domain[0], key->vnode_if_name);
        ret = (df_util->bypass_dom_set).insert(bypass_dom_if);
        UPLL_LOG_DEBUG("bypass ingress domain %s", bypass_domain[0]);
        if (ret.second == false) {
          UPLL_LOG_INFO("bypass egress domain in loop %s", bypass_domain[0]);
          source_node->addl_data->reason = UNC_DF_RES_EXCEEDS_HOP_LIMIT;
          DELETE_IF_NOT_NULL(ckv_remif);
          DELETE_IF_NOT_NULL(ckv_egress);
          return result_code;
        }
      }
      #endif
      //  bool total_success = false;
      uint16_t bypass_cnt = 0;
      while (nxt_ckv) {
         ckv_tmp_nxt_ckv =  nxt_ckv->get_next_cfg_key_val();
         nxt_ckv->set_next_cfg_key_val(NULL);
         if (!found_inif && !strncmp(reinterpret_cast<char *>
          (reinterpret_cast<key_vnode_if_t *>
          (ckv_remif->get_key())->vnode_if_name),
          reinterpret_cast<char *>(reinterpret_cast<key_vnode_if_t *>
            (nxt_ckv->get_key())->vnode_if_name), kMaxLenInterfaceName + 1)) {
            found_inif = true;
            delete nxt_ckv;
            nxt_ckv = ckv_tmp_nxt_ckv;
            continue;
         }
         #if 0
         if (ctrlr_type == UNC_CT_UNKNOWN) {
           ret = bypass_dom_set.insert(
                   (string(reinterpret_cast<char *>(bypass_domain[1]))));
           UPLL_LOG_DEBUG("bypass egress domain %s", bypass_domain[1]);
           if (ret.second == false) {
             UPLL_LOG_INFO("bypass egress domain in loop %s", bypass_domain[1]);
             source_node->addl_data->reason = UNC_DF_RES_EXCEEDS_HOP_LIMIT;
             delete nxt_ckv;
             delete ckv_tmp_nxt_ckv;
             break;
           }
         }
         #endif
         #if 1
         result_code =  vnif_mgr->GetInterfaceType(nxt_ckv,
                                    UNC_VF_INVALID, vnif_type);
         if (vnif_type != kBoundaryInterface) {
           delete nxt_ckv;
           nxt_ckv = ckv_tmp_nxt_ckv;
           continue;
         }
         #endif
         //  total_success = true;
         bypass_cnt++;
         GET_USER_DATA_DOMAIN(nxt_ckv, bypass_domain[1]);
         DataflowDetail *df_segment =
                      new DataflowDetail(kidx_val_vtn_dataflow_cmn,
                                         ctrlr_type);
         DataflowCmn *df_cmn = new DataflowCmn(false, df_segment);
         uuu::upll_strncpy(
                df_cmn->df_segment->vtn_df_common->ingress_domain,
                bypass_domain[0], (kMaxLenDomainId + 1));
         uuu::upll_strncpy(
                df_cmn->df_segment->vtn_df_common->egress_domain,
                bypass_domain[1], (kMaxLenDomainId + 1));
         uuu::upll_strncpy(
                df_cmn->df_segment->vtn_df_common->controller_id,
                ctrlr_id, (kMaxLenCtrlrId + 1));
         df_cmn->df_segment->vtn_df_common->controller_type = ctrlr_type;
         uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->ingress_vnode,
              reinterpret_cast<key_vnode_if_t *>
              (ckv_remif->get_key())->vnode_key.vnode_name,
              (kMaxLenVnodeName + 1));
         uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->
          ingress_vinterface, reinterpret_cast<key_vnode_if_t *>
          (ckv_remif->get_key())->vnode_if_name, (kMaxLenInterfaceName + 1));
         uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->egress_vnode,
            reinterpret_cast<key_vnode_if_t *>
          (nxt_ckv->get_key())->vnode_key.vnode_name, (kMaxLenVnodeName + 1));
         uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->egress_vinterface,
              reinterpret_cast<key_vnode_if_t *>
              (nxt_ckv->get_key())->vnode_if_name, (kMaxLenInterfaceName + 1));
         source_node->appendFlow(df_cmn, *(df_util->get_ctrlr_dom_count_map()));
         #if 0
         result_code =  vnif_mgr->GetInterfaceType(nxt_ckv,
                                    UNC_VF_INVALID, vnif_type);
         if (vnif_type != kBoundaryInterface) {
           delete nxt_ckv;
           nxt_ckv = ckv_tmp_nxt_ckv;
           continue;
         }
         #endif
         df_cmn->df_segment->ckv_egress = nxt_ckv;
         SET_USER_DATA(ckv_df, nxt_ckv);
         // Traverse the VNP/Vbypass boundary nodes.
         result_code = CheckBoundaryAndTraverse(ckv_df, header,
                                        df_cmn, lastPfcNode, df_util, dmi);
         if (result_code == UPLL_RC_SUCCESS) {
           if (df_cmn->addl_data->reason != UNC_DF_RES_SUCCESS) {
             //  total_success = false;
             bypass_cnt--;
             source_node->deleteflow(df_cmn);
             delete df_cmn;
           }
         } else {  //  UPLL_RC_ERR_GENERIC
           UpdateReason(source_node, result_code);
           DELETE_IF_NOT_NULL(ckv_remif);
           DELETE_IF_NOT_NULL(ckv_tmp_nxt_ckv);
           return UPLL_RC_SUCCESS;
         }
         nxt_ckv = ckv_tmp_nxt_ckv;
      }
      if (bypass_cnt == 0) {
         DataflowDetail *df_segment =
                        new DataflowDetail(kidx_val_vtn_dataflow_cmn,
                                           ctrlr_type);
         DataflowCmn *df_cmn = new DataflowCmn(false, df_segment);
         uuu::upll_strncpy(
                df_cmn->df_segment->vtn_df_common->ingress_domain,
                bypass_domain[0], (kMaxLenDomainId + 1));
         uuu::upll_strncpy(
                df_cmn->df_segment->vtn_df_common->controller_id,
                ctrlr_id, (kMaxLenCtrlrId + 1));
         df_cmn->df_segment->vtn_df_common->controller_type = ctrlr_type;
         uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->ingress_vnode,
              reinterpret_cast<key_vnode_if_t *>
              (ckv_remif->get_key())->vnode_key.vnode_name,
              (kMaxLenVnodeName + 1));
         uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->
          ingress_vinterface, reinterpret_cast<key_vnode_if_t *>
          (ckv_remif->get_key())->vnode_if_name, (kMaxLenInterfaceName + 1));
         source_node->appendFlow(df_cmn, *(df_util->get_ctrlr_dom_count_map()));
         UpdateReason(df_cmn, UPLL_RC_ERR_NO_SUCH_INSTANCE);
      }
      delete ckv_remif;
  }
  return result_code;
}

upll_rc_t VtnDataflowMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn_dataflow *vtn_dfkey = NULL;
  if (okey && okey->get_key()) {
    vtn_dfkey = reinterpret_cast<key_vtn_dataflow *>(
                    okey->get_key());
  } else {
    vtn_dfkey = reinterpret_cast<key_vtn_dataflow *>(
      ConfigKeyVal::Malloc(sizeof(key_vtn_dataflow)));
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_VTN_DATAFLOW,
              IpctSt::kIpcStKeyVtnDataflow, vtn_dfkey, NULL);
    else if (okey->get_key() != vtn_dfkey)
      okey->SetKey(IpctSt::kIpcStKeyVtnDataflow, vtn_dfkey);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      ConfigKeyVal::Free(vtn_dfkey);
    return UPLL_RC_ERR_GENERIC;
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_ROOT:
      break;
    case UNC_KT_VTN_DATAFLOW:
      memcpy(vtn_dfkey, reinterpret_cast<key_vtn_dataflow *>(pkey),
             sizeof(key_vtn_dataflow));
      break;
    case UNC_KT_VTN:
      uuu::upll_strncpy(vtn_dfkey->vtn_key.vtn_name,
          reinterpret_cast<key_vtn *>(pkey)->vtn_name,
          (kMaxLenVtnName+1));
      break;
    default:
      if (!okey || !(okey->get_key())) {
        ConfigKeyVal::Free(vtn_dfkey);
      }
      return UPLL_RC_ERR_GENERIC;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VTN_DATAFLOW,
                IpctSt::kIpcStKeyVtnDataflow, vtn_dfkey, NULL);
  else if (okey->get_key() != vtn_dfkey)
    okey->SetKey(IpctSt::kIpcStKeyVtnDataflow, vtn_dfkey);
  SET_USER_DATA(okey, parent_key);
  return result_code;
}



// Dynamic interface check when Ingress or Egress info is INVALID.
upll_rc_t VtnDataflowMoMgr::DynamicInterfaceCheck(
    DataflowCmn *df_cmn, DataflowUtil *df_util,
    val_vtn_dataflow_path_info *path_info,
    const uint8_t *vtn_name, int direction, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  uint32_t path_node_indx = (direction == INGRESS) ? UPLL_IDX_IN_VNODE_VVDPI :
                                                     UPLL_IDX_OUT_VNODE_VVDPI;
  uint32_t path_nodeif_indx = (direction == INGRESS) ? UPLL_IDX_IN_VIF_VVDPI :
                                                       UPLL_IDX_OUT_VIF_VVDPI;
  uint8_t *node_name =
      (direction == INGRESS) ? path_info->in_vnode : path_info->out_vnode;
  uint8_t *node_if_name =
      (direction == INGRESS) ? path_info->in_vif : path_info->out_vif;

  // For dynamic interface Driver will send INVALID in path_info
  if ((path_info->valid[path_node_indx] == UNC_VF_INVALID) &&
      (path_info->valid[path_nodeif_indx] == UNC_VF_INVALID)) {
    if ((direction == INGRESS) &&
        (UNC_VF_VALID == path_info->valid[UPLL_IDX_OUT_VNODE_VVDPI])) {
      result_code = RenamePathinfoNodes(
          path_info->out_vnode, df_cmn->pfc_vtn_name,
          df_cmn->df_segment->vtn_df_common->controller_id, df_util, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("RenamePathinfoNodes failed, %d", result_code);
        return result_code;
      }
      UPLL_LOG_DEBUG("Ingress interface is dynamic");
      df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_INGRESS_VNODE_VVDC]
                               = UNC_VF_VALID;
      uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->ingress_vnode,
                        path_info->out_vnode, kMaxLenVnodeName+1);
    } else if ((direction == EGRESS) &&
         (UNC_VF_VALID == path_info->valid[UPLL_IDX_IN_VNODE_VVDPI])) {
      result_code = RenamePathinfoNodes(
          path_info->in_vnode, df_cmn->pfc_vtn_name,
          df_cmn->df_segment->vtn_df_common->controller_id, df_util, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("RenamePathinfoNodes failed, %d", result_code);
        return result_code;
      }
      UPLL_LOG_DEBUG("Egress interface is dynamic");
      df_cmn->df_segment->vtn_df_common->valid[UPLL_IDX_EGRESS_VNODE_VVDC]
                               = UNC_VF_VALID;
      uuu::upll_strncpy(df_cmn->df_segment->vtn_df_common->egress_vnode,
                        path_info->in_vnode, kMaxLenVnodeName+1);
    }
     /* If it is not a dynamic interface, then check for vexternal
      *     If it is a vexternal then set it as INVALID
      *     else Check for renamed or not.
      */
  } else if ((path_info->valid[path_node_indx] == UNC_VF_VALID) &&
             (path_info->valid[path_nodeif_indx] == UNC_VF_VALID) &&
             (df_cmn->df_segment->is_flow_redirect == false)) {
    result_code = ConvertVexternaltoVbr(df_cmn, vtn_name, node_name,
                                        node_if_name, dmi);
    if ((result_code != UPLL_RC_SUCCESS) &&
        (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_DEBUG("ConvertVexternaltoVbr failed %d", result_code);
      return result_code;
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      // when it is not a vExternal, check if Renamed or not
      result_code = RenamePathinfoNodes(node_name, df_cmn->pfc_vtn_name,
                 df_cmn->df_segment->vtn_df_common->controller_id,
                 df_util, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("RenamePathinfoNodes failed result_code: %d",
                      result_code);
        return result_code;
      }
    } else {
      // Setting to invalid If it is a vExternal.
      path_info->valid[path_node_indx] = UNC_VF_INVALID;
      path_info->valid[path_nodeif_indx] = UNC_VF_INVALID;
    }
  }
  return UPLL_RC_SUCCESS;
}

/*
 * To compare path_infos in case of redirect.
 * if prev_path_info's out_vnode, out_vif and current_path_info's
 *  in_vnode, in_vif are same then check whether it is vexternal or not
 *     If it is a vexternal then flow need to be skipped
 */

void VtnDataflowMoMgr::RedirectCheck(DataflowCmn *df_cmn) {
  UPLL_FUNC_TRACE;
  for (unsigned int idx = 0;
       idx < ((df_cmn->df_segment->vtn_df_common->path_info_count)-1);
       idx++) {
    val_vtn_dataflow_path_info *path_info_prev =
                           df_cmn->df_segment->vtn_path_infos[idx];
    val_vtn_dataflow_path_info_t *path_info =
                               df_cmn->df_segment->vtn_path_infos[idx+1];
    /*
     * Check previous path_info's in_vnode/in_vif , out_vnode/out_vif and
     * current path_info's in_vnode/in_vif are VALID or not
     *    If all are VALID then check all three vnodes and interfaces same or
     *    not
     *      If all are same, then copy current path_info's out_vnode/out_vif,
     *      vlink-flag and action to previous path_info's out_vnode/out_vif,
     *      vlink-flag and action.
     */
    if ((path_info_prev->valid[UPLL_IDX_IN_VNODE_VVDPI] == UNC_VF_VALID) &&
        (path_info_prev->valid[UPLL_IDX_IN_VIF_VVDPI] == UNC_VF_VALID) &&
        (path_info_prev->valid[UPLL_IDX_OUT_VNODE_VVDPI] == UNC_VF_VALID) &&
        (path_info_prev->valid[UPLL_IDX_OUT_VIF_VVDPI] == UNC_VF_VALID) &&
        (path_info->valid[UPLL_IDX_IN_VNODE_VVDPI] == UNC_VF_VALID) &&
        (path_info->valid[UPLL_IDX_IN_VIF_VVDPI] == UNC_VF_VALID)) {
      UPLL_LOG_TRACE("previous path info %s",
                     DataflowCmn::get_string(*path_info_prev).c_str());
      UPLL_LOG_TRACE("current path info %s",
                     DataflowCmn::get_string(*path_info).c_str());
      if (!(strncmp(reinterpret_cast<const char *>(path_info->in_vnode),
                    reinterpret_cast<const char *>(path_info_prev->out_vnode),
                    sizeof(path_info->in_vnode) + 1)) &&
          !(strncmp(reinterpret_cast<const char *>(path_info->in_vif),
                    reinterpret_cast<const char *>(path_info_prev->out_vif),
                    sizeof(path_info->in_vif) + 1)) &&
          !(strncmp(reinterpret_cast<const char *>(path_info->in_vnode),
                    reinterpret_cast<const char *>(path_info_prev->in_vnode),
                    sizeof(path_info->in_vnode) + 1)) &&
          !(strncmp(reinterpret_cast<const char *>(path_info->in_vif),
                    reinterpret_cast<const char *>(path_info_prev->in_vif),
                    sizeof(path_info->in_vif) + 1))) {
        UPLL_LOG_DEBUG("Skipping path info %s",
                      DataflowCmn::get_string(*path_info).c_str());
        uuu::upll_strncpy(reinterpret_cast<char *>(path_info_prev->out_vnode),
                          (const char *)path_info->out_vnode,
                          (kMaxLenVnodeName+1));
        uuu::upll_strncpy(reinterpret_cast<char *>(path_info_prev->out_vif),
                          (const char *)path_info->out_vif,
                          (kMaxLenInterfaceName + 1));
        path_info_prev->vlink_flag = path_info->vlink_flag;
        path_info_prev->status = path_info->status;
        df_cmn->df_segment->vtn_df_common->path_info_count--;
        df_cmn->df_segment->vtn_path_infos.erase(
                df_cmn->df_segment->vtn_path_infos.begin()+(idx+1));
        idx--;
        delete path_info;
      }
    }
  }
}


}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
