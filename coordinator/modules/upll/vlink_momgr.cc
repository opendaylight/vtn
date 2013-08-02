/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include "vbr_if_momgr.hh"
#include "vlink_momgr.hh"
#include "ctrlr_capa_defines.hh"
#include "capa_intf.hh"
#include "upll_validation.hh"
#include "upll_log.hh"
#include "vtn_momgr.hh"
#include "unc/uppl_common.h"

#define NUM_KEY_RENAME_TBL_ 4
#define NUM_KEY_MAIN_TBL_ 5

#define VN1_RENAME 0x04
#define VN2_RENAME 0x08

namespace dbl = unc::upll::dal;
namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VlinkMoMgr::vlink_bind_info[] = {
    { uudst::vlink::kDbiVtnName, CFG_KEY, offsetof(key_vlink, vtn_key),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiVlinkName, CFG_KEY, offsetof(key_vlink, vlink_name),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiAdminStatus, CFG_VAL, offsetof(val_vlink, admin_status),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiVnode1Name, CFG_VAL, offsetof(val_vlink, vnode1_name),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiVnode1Ifname, CFG_VAL, offsetof(val_vlink,
                                                        vnode1_ifname),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiVnode2Name, CFG_VAL, offsetof(val_vlink, vnode2_name),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiVnode2Ifname, CFG_VAL, offsetof(val_vlink,
                                                        vnode2_ifname),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiBoundaryName, CFG_VAL, offsetof(val_vlink,
                                                        boundary_name),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiVlanid, CFG_VAL, offsetof(val_vlink, vlan_id),
      uud::kDalUint16, 1 },
    { uudst::vlink::kDbiDesc, CFG_VAL, offsetof(val_vlink, description),
      uud::kDalChar, 128 },
    { uudst::vlink::kDbiOperStatus, ST_VAL,
                                          offsetof(val_vlink_st, oper_status),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiDownCount, ST_VAL,
                                        offsetof(val_db_vlink_st, down_count),
      uud::kDalUint32, 1 },
    { uudst::vlink::kDbiCtrlr1Name, CK_VAL, offsetof(key_user_data, ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiDomain1Id, CK_VAL, offsetof(key_user_data, domain_id),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiCtrlr2Name, CK_VAL2, offsetof(key_user_data, ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiDomain2Id, CK_VAL2, offsetof(key_user_data, domain_id),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiValidAdminStatus, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_ADMIN_STATUS_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidVnode1Name, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_VNODE1_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidVnode1Ifname, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_VNODE1_IF_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidVnode2Name, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_VNODE2_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidVnode2Ifname, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_VNODE2_IF_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidBoundaryName, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_BOUNDARY_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidVlanid, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_VLAN_ID_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidDesc, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_DESCRIPTION_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidOperStatus, ST_META_VAL, offsetof(
        val_vlink_st, valid[UPLL_IDX_OPER_STATUS_VLNKS]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsAdminStatus, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_ADMIN_STATUS_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsVnode1Name, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_VNODE1_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsVnode1Ifname, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_VNODE1_IF_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsVnode2Name, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_VNODE2_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsVnode2Ifname, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_VNODE2_IF_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsBoundaryName, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_BOUNDARY_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsVlanid, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_VLAN_ID_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsDesc, CFG_META_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_DESCRIPTION_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsRowstatus, CS_VAL, offsetof(val_vlink, cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiFlags, CK_VAL, offsetof(key_user_data, flags),
      uud::kDalUint8, 1 } };

BindInfo VlinkMoMgr::vlink_rename_bind_info[] = {
    { uudst::vnode_rename::kDbiUncVtnName, CFG_KEY, offsetof(key_vlink_t,
                                                          vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_KEY, offsetof(key_vlink_t,
                                                            vlink_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrName, CK_VAL, offsetof(key_user_data,
                                                           ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiDomainId, CK_VAL, offsetof(key_user_data,
                                                          domain_id),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrVtnName, CFG_VAL, offsetof(
        val_rename_vnode, ctrlr_vtn_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrVnodeName, CFG_VAL, offsetof(
        val_rename_vnode, ctrlr_vnode_name),
      uud::kDalChar, 32 } };

BindInfo VlinkMoMgr::key_vlink_maintbl_bind_info[] = {
    { uudst::vlink::kDbiVtnName, CFG_MATCH_KEY, offsetof(key_vlink_t,
                                                         vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vlink::kDbiVlinkName, CFG_MATCH_KEY, offsetof(key_vlink,
                                                           vlink_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vlink::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vlink::kDbiVlinkName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vlink::kDbiFlags, CK_VAL, offsetof(key_user_data_t, flags),
      uud::kDalUint8, 1 } };

BindInfo VlinkMoMgr::key_vlink_renametbl_update_bind_info[] = {
    { uudst::vnode_rename::kDbiUncVtnName, CFG_MATCH_KEY, offsetof(
        key_vlink_t, vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_MATCH_KEY, offsetof(
        key_vlink_t, vlink_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vnode_rename::kDbiUncVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 } };

VlinkMoMgr::VlinkMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];
  table[MAINTBL] = new Table(uudst::kDbiVlinkTbl, UNC_KT_VLINK,
                             vlink_bind_info, IpctSt::kIpcStKeyVlink,
                             IpctSt::kIpcStValVlink,
                             uudst::vlink::kDbiVlinkNumCols);
  table[RENAMETBL] = new Table(uudst::kDbiVlinkRenameTbl, UNC_KT_VLINK,
                  vlink_rename_bind_info, IpctSt::kIpcInvalidStNum,
                  IpctSt::kIpcInvalidStNum,
                  uudst::vlink_rename::kDbiVlinkRenameNumCols);
  table[CTRLRTBL] = NULL;
  nchild = 0;
  child = NULL;
  ck_boundary = NULL;
//    SetMoManager(UNC_KT_VLINK,(MoMgr *)this);
}

upll_rc_t VlinkMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || req == NULL || dmi == NULL) {
    UPLL_LOG_INFO("Cannot perform create operation"
                   "due to insufficient parameters\n");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom[2] = { { NULL, NULL }, { NULL, NULL } };
  UPLL_LOG_TRACE("VLINK CREATE INPUT %s", (ikey->ToStrAll()).c_str());
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ValidateMessage Failed : %d", result_code);
    return result_code;
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_INFO("ValidateAttribute returning %d\n", result_code);
     return result_code;
  }
  // Vnode Existence check in CANDIDATE DB
  result_code = VnodeChecks(ikey, req->datatype, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Vnode with the same name already exists %d", result_code);
    return result_code;
  }
  UPLL_LOG_TRACE("VnodeCheck Over, The Result code is %d", result_code);
  // Parent VTN check
  result_code = GetParentConfigKey(parent_ck_vtn, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in retrieving the parent VTN ConfigKeyVal");
    return result_code;
  }
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VTN)));
  result_code = mgr->UpdateConfigDB(parent_ck_vtn, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE
      || result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("VTN doesn't exist in CANDIDATE DB");
    return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  }
  // vbrIf checks are done and respective Vnodes ContollerIds are filled
  result_code = UpdateVlinkIf(req->datatype, ikey, dmi, ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS || ctrlr_dom[0].ctrlr == NULL
      || ctrlr_dom[1].ctrlr == NULL) {
    UPLL_LOG_INFO("Error in checking for Vlink Interfaces");
    return result_code;
  }
  UPLL_LOG_TRACE(" The Controller Name is %s", ctrlr_dom[0].ctrlr);
  if (UPLL_DT_CANDIDATE == req->datatype) {
    UPLL_LOG_TRACE(" The Controller Name is %s", ctrlr_dom[0].ctrlr);
    result_code = GetInstanceCount(ikey,
                                 reinterpret_cast<char *>(ctrlr_dom[0].ctrlr),
                                 req->datatype, &cur_instance_count, dmi,
                                 MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" returns error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  }
#if 0
  result_code = ValidateCapability(req, ikey,reinterpret_cast<const char *>
                                   (ctrlr_dom[0].ctrlr));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
    return result_code;
  }
  if (strcmp((const char *)ctrlr_dom[0].ctrlr,
                                      (const char *)ctrlr_dom[1].ctrlr)) {
    result_code = GetInstanceCount(ikey, reinterpret_cast<char *>
      (ctrlr_dom[1].ctrlr), req->datatype, &cur_instance_count, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR(" returns error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = ValidateCapability(req, ikey,
                     reinterpret_cast<const char *>(ctrlr_dom[1].ctrlr));
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
      return result_code;
    }

  }
#endif
  result_code = RestoreVnode(ikey, req, dmi, ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS)
    UPLL_LOG_DEBUG("Returning %d\n", result_code);
  return result_code;
}

upll_rc_t VlinkMoMgr::DeleteMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = MoMgrImpl::DeleteMo(req, ikey, dmi);
  return result_code;
}

upll_rc_t VlinkMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req) {
  upll_rc_t result_code;
  if (!ikey || !ikey->get_cfg_val()) {
    UPLL_LOG_DEBUG("Invalid parameter\n");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vlink_t *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ikey));
  if (vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) {
    result_code = ValidateBoundary(vlink_val->boundary_name, req);
    if (result_code != UPLL_RC_SUCCESS) return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  return UPLL_RC_SUCCESS;
}

#if 0
upll_rc_t VlinkMoMgr::RestoreVnode(ConfigKeyVal *ikey,
                                   IpcReqRespHeader *req,
                                   DalDmlIntf *dmi,
                                   controller_domain_t *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dmi == NULL) {
    UPLL_LOG_DEBUG("Create error due to insufficient parameters\n");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  /* check if vnode exists in RUNNING DB
   ** if exists : restore to CANDIDATE DB
   ** else : validate the attributes
   */
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
                               MAINTBL);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    result_code = RestoreChildren(ikey, req->datatype, UPLL_DT_RUNNING, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Restoring children failed. Error code : %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_ERROR("Problem in reading RUNNING DB\n");
    return result_code;
  }

  /* If parent VTN is renamed, set the renamed flag in Vnode
   ** and create an entry in vnode rename table if VTN is renamed
   */
  result_code = SetVnodeRenameFlag(ikey, req->datatype, ctrlr_dom, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
#if 0
  /* set the controller domain in the ikey */
  SET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  if (ikey->get_key_type() == UNC_KT_VLINK) {
    if (!ikey->get_cfg_val()) return UPLL_RC_ERR_GENERIC;
    SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
  }
#endif
  // create a record in CANDIDATE DB
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutDomain
                       | kOpInOutCtrlr };
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi, &dbop,
                               MAINTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Record Creation fialed in CANDIDATE DB\n");
    return result_code;
  }
  return result_code;
}
#endif



upll_rc_t VlinkMoMgr::GetNodeType(void *key, bool vnode,
                                  unc_key_type_t &keytype,
                                  ConfigKeyVal *&ck_val, DalDmlIntf *dmi) {
  unc_key_type_t *ktype, if_ktype[] = { UNC_KT_VBR_IF, UNC_KT_VRT_IF,
                                        UNC_KT_VTEP_IF, UNC_KT_VTUNNEL_IF };
  unc_key_type_t vnode_ktype[] = { UNC_KT_VBRIDGE, UNC_KT_VROUTER, UNC_KT_VTEP,
                                   UNC_KT_VTUNNEL };
  int numnodes;

  if (vnode) {
    ktype = vnode_ktype;
    numnodes = sizeof(vnode_ktype) / sizeof(unc_key_type_t);
  } else {
    ktype = if_ktype;
    numnodes = sizeof(if_ktype) / sizeof(unc_key_type_t);
  }
  for (int i = 0; i < numnodes; i++) {
    keytype = ktype[i];
    MoMgrImpl *mgr =
        (reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            keytype))));
    mgr->GetChildConfigKey(ck_val, NULL);
    if (ck_val == NULL) {
      UPLL_LOG_ERROR("Invalid param\n");
      return UPLL_RC_ERR_GENERIC;
    }
    switch (keytype) {
      case UNC_KT_VBRIDGE:
      case UNC_KT_VROUTER:
      case UNC_KT_VTUNNEL:
      case UNC_KT_VTEP: {
        const pfc_ipcstdef_t *key_stdef = IpctSt::GetIpcStdef(
            ck_val->get_st_num());
        if (!key_stdef) {
          UPLL_LOG_DEBUG("Invalid param\n");
          return UPLL_RC_ERR_GENERIC;
        }
        if (sizeof(reinterpret_cast<key_vnode_t *>(key)) != key_stdef->ist_size)
          return UPLL_RC_ERR_GENERIC;
        memcpy(ck_val->get_key(), key, sizeof(key_vnode_t));
        break;
      }
      case UNC_KT_VBR_IF:
      case UNC_KT_VRT_IF:
      case UNC_KT_VTUNNEL_IF:
      case UNC_KT_VTEP_IF: {
        const pfc_ipcstdef_t *key_stdef = IpctSt::GetIpcStdef(
            ck_val->get_st_num());
        if (!key_stdef) {
          UPLL_LOG_DEBUG("Invalid param\n");
          return UPLL_RC_ERR_GENERIC;
        }
        if (sizeof(reinterpret_cast<key_vbr_if_t *>(key))
                         != key_stdef->ist_size)
          return UPLL_RC_ERR_GENERIC;
        memcpy(ck_val->get_key(), key, sizeof(key_vbr_if_t));
        break;
      }
      default:
        UPLL_LOG_TRACE("Invalid Keytype \n");
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
    }
    DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutFlag
                          | kOpInOutDomain };
    upll_rc_t result_code = mgr->ReadConfigDB(ck_val, UPLL_DT_STATE,
                                              UNC_OP_READ, dbop1, dmi, MAINTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      continue;
    } else if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Error in reading %d", result_code);
      return result_code;
    } else {
      return UPLL_RC_SUCCESS;
    }
  }
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

upll_rc_t VlinkMoMgr::CreateVnodeConfigKey(ConfigKeyVal *ikey,
                                           ConfigKeyVal *&okey) {
  if (ikey == NULL) return UPLL_RC_ERR_GENERIC;

  key_vlink * vlink_key = reinterpret_cast<key_vlink *>
                                  (malloc(sizeof(key_vlink)));
  uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
         reinterpret_cast<key_vlink *>(ikey->get_key())->vtn_key.vtn_name,
          (kMaxLenVtnName+1));
  uuu::upll_strncpy(vlink_key->vlink_name,
         reinterpret_cast<key_vlink*>(ikey->get_key())->vlink_name,
           (kMaxLenVlinkName+1));

  ConfigKeyVal *ck_vlink = new ConfigKeyVal(UNC_KT_VLINK,
                                            IpctSt::kIpcStKeyVlink,
                           reinterpret_cast<void *>(vlink_key), NULL);
  if (ck_vlink == NULL) {
    UPLL_LOG_ERROR("Memory Allocation failed for ck_vlink");
    FREE_IF_NOT_NULL(vlink_key);
    return UPLL_RC_ERR_GENERIC;
  }
  okey = ck_vlink;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::SwapVnodes(ConfigKeyVal *&ck_vlink) {
  val_vlink * vlink_val = reinterpret_cast<val_vlink *>(GetVal(ck_vlink));
  uint8_t temp_swap[32];
  uuu::upll_strncpy(temp_swap, vlink_val->vnode1_name, (kMaxLenVnodeName+1));
  uuu::upll_strncpy(vlink_val->vnode1_name, vlink_val->vnode2_name,
                   (kMaxLenVnodeName+1));
  uuu::upll_strncpy(vlink_val->vnode2_name,  temp_swap, (kMaxLenVnodeName+1));
  uuu::upll_strncpy(temp_swap, vlink_val->vnode1_ifname,
                   (kMaxLenInterfaceName+1));
  uuu::upll_strncpy(vlink_val->vnode1_ifname, vlink_val->vnode2_ifname,
                    (kMaxLenInterfaceName+1));
  uuu::upll_strncpy(vlink_val->vnode2_ifname, temp_swap,
                   (kMaxLenInterfaceName+1));
#if 0
  int rename_flag = 0;
  int temp_flag = 0;
  GET_USER_DATA_FLAGS(ck_vlink, rename_flag);
#if 0
  if ((rename_flag & 0x30) == 0x30) temp_flag = 0x30;
  else if ((rename_flag & 0xc0) == 0x20) temp_flag = 0x20;
  else if ((rename_flag & 0xc0) == 0x10) temp_flag = 0x10;
  rename_flag |= temp_flag;
#else
  /* Swap the rename fields if vnodes renamed */
  if (rename_flag & VNODE1_RENAME) temp_flag = VNODE1_RENAME;
  if (rename_flag & VNODE2_RENAME) temp_flag |= VNODE2_RENAME;
  rename_flag &= VNODE_RENAME;
  rename_flag |= temp_flag;
#endif
  SET_USER_DATA_FLAGS(ck_vlink, rename_flag);
#endif
  return UPLL_RC_SUCCESS;
}




upll_rc_t VlinkMoMgr::UpdateVlinkMemIfFlag(upll_keytype_datatype_t dt_type,
                                           ConfigKeyVal *ckv_if,
                                           DalDmlIntf *dmi,
                                           vnode_if_type &vnif_type,
                                           MoMgrImpl *mgr,
                                           unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (!ckv_if || !mgr) {
    UPLL_LOG_DEBUG("Invalid param\n");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *dup_ckvif = NULL;
  upll_rc_t result_code = mgr->DupConfigKeyVal(dup_ckvif,ckv_if,MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Dupkey failed\n");
    return result_code;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
  void *if_val = GetVal(dup_ckvif);
  if (!if_val) {
     UPLL_LOG_DEBUG("Invalid param\n");
     if (dup_ckvif) delete dup_ckvif;
     return UPLL_RC_ERR_GENERIC;
  }
     
  switch (ckv_if->get_key_type()) {
  case UNC_KT_VBR_IF: {
      MoMgrImpl *pom_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
                                       GetMoManager(UNC_KT_VBRIF_FLOWFILTER)));
      MoMgrImpl *vbr_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
                                       GetMoManager(UNC_KT_VBR_IF)));
      if (!pom_mgr || !vbr_mgr) {
        UPLL_LOG_DEBUG("Instance is NULL");
        if (dup_ckvif) delete dup_ckvif;
        return UPLL_RC_ERR_GENERIC;
     }
     result_code = vbr_mgr->GetChildConfigKey (okey, ckv_if);
     if (UPLL_RC_SUCCESS != result_code) {
       UPLL_LOG_DEBUG("GetChilConfigKey Failed");
       if (dup_ckvif) delete dup_ckvif;
       return UPLL_RC_ERR_GENERIC;
     }
     if (UNC_OP_CREATE == op)
       result_code = pom_mgr->SetVlinkPortmapConfiguration(okey, dt_type, dmi, kVlinkConfigured);
     else if (UNC_OP_DELETE == op)
       result_code = pom_mgr->SetVlinkPortmapConfiguration(okey, dt_type, dmi, kVlinkPortMapNotConfigured);
     if (okey) delete okey;
     if (UPLL_RC_SUCCESS != result_code && 
           UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
           UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
           if (dup_ckvif) delete dup_ckvif;
           return result_code;
     }
     ResetValid(val_vbr_if, &reinterpret_cast<val_drv_vbr_if *>
                              (if_val)->vbr_if_val)
     vnif_type = kVbrIf;
     break;
     }
  case UNC_KT_VRT_IF:
     {
      MoMgrImpl *pom_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
                                       GetMoManager(UNC_KT_VRTIF_FLOWFILTER)));
      MoMgrImpl *vrt_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
                                       GetMoManager(UNC_KT_VRT_IF)));

       if (!pom_mgr || !vrt_mgr) {
         UPLL_LOG_DEBUG("Invalid Instance");
         if (dup_ckvif) delete dup_ckvif;
         return UPLL_RC_ERR_GENERIC;
       }
       result_code = vrt_mgr->GetChildConfigKey (okey, ckv_if);
       if (UPLL_RC_SUCCESS != result_code) {
         UPLL_LOG_DEBUG("GetChilConfigKey Failed");
         if (dup_ckvif) delete dup_ckvif;
         return UPLL_RC_ERR_GENERIC;
       }
       if (UNC_OP_CREATE == op) { 
         result_code = pom_mgr->SetVlinkPortmapConfiguration(okey, dt_type, dmi, kVlinkConfigured);
       }
       else if (UNC_OP_DELETE == op) {
         result_code = pom_mgr->SetVlinkPortmapConfiguration(okey, dt_type, dmi, kVlinkPortMapNotConfigured);
       }
       if (okey) delete okey;
       if (UPLL_RC_SUCCESS != result_code && 
           UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
           UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
           if (dup_ckvif) delete dup_ckvif;
           return result_code;
       }
     ResetValid(val_vrt_if, if_val);
     vnif_type = kVrtIf;
     break;
   }
  case UNC_KT_VTEP_IF:
     ResetValid(val_vtep_if, if_val);
     vnif_type = kVtepIf;
     break;
  case UNC_KT_VTUNNEL_IF:
     ResetValid(val_vtunnel_if, if_val);
     vnif_type = kVtunnelIf;
     break;
  case UNC_KT_VUNK_IF:
     ResetValid(val_vunk_if, if_val);
     vnif_type = kVunkIf;
     break;
  default:
     if (dup_ckvif) delete dup_ckvif;
     UPLL_LOG_DEBUG("Unsupported keytype %d\n", ckv_if->get_key_type());
     return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("Updating the Flag value in Interface Table %d",
                           ckv_if->get_key_type());
  result_code = mgr->UpdateConfigDB(ckv_if, dt_type,
                      UNC_OP_UPDATE, dmi, &dbop, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS)
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
  if (dup_ckvif) delete dup_ckvif;
  return result_code;
}

upll_rc_t VlinkMoMgr::UpdateVlinkIf(upll_keytype_datatype_t dt_type,
                                      ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi,
                                      controller_domain *ctrlr_dom) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool bound_vlink = false;
  unc_key_type_t key_type;
  if (!ikey || (ikey->get_key_type() != UNC_KT_VLINK)) {
    UPLL_LOG_INFO("Invalid ConfigKeyVal parameter\n");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t valid_boundary =
      reinterpret_cast<val_vlink *>(GetVal(ikey))->
                            valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
  uint8_t rename_flag = 0;
  VlinkNodePosition vnodeif_number = kVlinkVnode1;
  unc_key_type_t if1_type = UNC_KT_ROOT, if2_type = UNC_KT_ROOT;
  ConfigKeyVal *ck_if[2] = { NULL, NULL };
  ConfigKeyVal *ck_drv_vbr_if = NULL;
  ConfigVal *cv_link;
  int i = 0;
  VbrIfMoMgr *mgr =
      static_cast<VbrIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VBR_IF)));
  // val is mandatory in vlink structure
  cv_link = ikey->get_cfg_val();
  if (!cv_link) return UPLL_RC_ERR_GENERIC;
  do {
    GET_USER_DATA_FLAGS(cv_link, rename_flag);
    rename_flag |= (uint8_t)(vnodeif_number & VIF_TYPE);
    SET_USER_DATA_FLAGS(cv_link, rename_flag);
    result_code = mgr->GetChildConfigKey(ck_drv_vbr_if, ikey);
    rename_flag &= ~VIF_TYPE;
    SET_USER_DATA_FLAGS(cv_link, rename_flag);
    if (!ck_drv_vbr_if || (result_code != UPLL_RC_SUCCESS))
      return UPLL_RC_ERR_GENERIC;
    void *if_key = ck_drv_vbr_if->get_key();
    if (!if_key) return UPLL_RC_ERR_GENERIC;
    result_code = GetVnodeType(if_key, false, key_type, ck_if[i], dmi, dt_type);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Invalid Vlink if\n");
      return (result_code);
    }
    if (vnodeif_number == kVlinkVnode1) if1_type = key_type;
    else if (vnodeif_number == kVlinkVnode2) if2_type = key_type;
    GET_USER_DATA_CTRLR_DOMAIN(ck_if[i], ctrlr_dom[i]);
    UPLL_LOG_TRACE(" The Controller and Domain Name is %s & %s",
                     ctrlr_dom[i].ctrlr, ctrlr_dom[i].domain);
    GET_USER_DATA_FLAGS(ck_if[i], rename_flag);
    if (rename_flag & VIF_TYPE) {
      UPLL_LOG_DEBUG("Interface is already part of another vlink\n");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    delete ck_drv_vbr_if;
    ck_drv_vbr_if = NULL;
    vnodeif_number = (vnodeif_number == kVlinkVnode1) ?
                       kVlinkVnode2 : kVlinkVnode1;
    i++;
  } while (vnodeif_number != kVlinkVnode1);
  if (!ctrlr_dom[0].ctrlr || !ctrlr_dom[1].ctrlr) {
    UPLL_LOG_DEBUG("Invalid ctrlr");
    return UPLL_RC_ERR_GENERIC;
  }
  if ((memcmp(ctrlr_dom[0].ctrlr, ctrlr_dom[1].ctrlr, kMaxLenCtrlrId)) ||
     (memcmp(ctrlr_dom[0].domain, ctrlr_dom[1].domain, kMaxLenDomainId))) {
    UPLL_LOG_DEBUG("Boundary vlink\n");
    bound_vlink = true;
    if ((if1_type == UNC_KT_VRT_IF) || (if2_type == UNC_KT_VRT_IF)) {
      UPLL_LOG_DEBUG("vrt link is not supported on a boundary\n");
      for (int i =0; i <2; i++)
        if (ck_if[i]) delete ck_if[i];
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
    }
  } else {
    if ((if1_type == UNC_KT_VBR_IF) && (if2_type == UNC_KT_VBR_IF)) {
      UPLL_LOG_DEBUG("Internal link between 2 vbridges not allowed\n");
      for (int i =0; i <2; i++)
        if (ck_if[i]) delete ck_if[i];
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
    }
  }
  if ((if1_type != UNC_KT_VBR_IF) && (if2_type == UNC_KT_VBR_IF)) {
    result_code = SwapVnodes(ikey);
    ConfigKeyVal *tmp = ck_if[0];
    ck_if[0] = ck_if[1];
    ck_if[1] = tmp;
    GET_USER_DATA_CTRLR_DOMAIN(ck_if[0], ctrlr_dom[0]);
    GET_USER_DATA_CTRLR_DOMAIN(ck_if[1], ctrlr_dom[1]);
  }
  vnode_if_type vnif_type[2];
  for (int i = 0; i < 2; i++) {
  // Set the flag - 2 MSB for boundary and next 2 for internal
    GET_USER_DATA_FLAGS(ck_if[i], rename_flag);
    if (bound_vlink) {
      rename_flag |= (i == 0) ?
        kVlinkBoundaryNode1 : kVlinkBoundaryNode2;
      SET_USER_DATA_FLAGS(ck_if[i], rename_flag);
    } else {
      rename_flag |= (i == 0) ? kVlinkInternalNode1 :
              kVlinkInternalNode2;
    }
    SET_USER_DATA_FLAGS(ck_if[i], rename_flag);
    key_type = ck_if[i]->get_key_type();
    MoMgrImpl *if_mgr = reinterpret_cast<MoMgrImpl *>
                 (const_cast<MoManager *>(GetMoManager(key_type)));

    result_code = UpdateVlinkMemIfFlag(dt_type, ck_if[i],
                                        dmi, vnif_type[i], if_mgr,UNC_OP_CREATE);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return result_code;
    }
  }
  SET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
  /* Reset the pointers to ikey as the current stored pointers
   * point to ck_if which is going to get deleted.
   */
  GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  GET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
  /* Set the vnode if types in first 6 bits of flag */
  uint8_t flag = 0;
  GET_USER_DATA_FLAGS(ikey, flag);
  /* clear the iftype bits and preserve the rename bits */
  flag &= ~VLINK_FLAG_NODE_TYPE;
  flag |= (vnif_type[0] << kVlinkVnodeIf1Type);
  flag |= (vnif_type[1] << kVlinkVnodeIf2Type);
  SET_USER_DATA_FLAGS(ikey, flag);
  UPLL_LOG_DEBUG("Flags for vnode1if type %d vnode2if type %d value %d\n",
                  vnif_type[0], vnif_type[1], flag);
  if (valid_boundary) {
    UPLL_LOG_TRACE("Valid boundary\n");
    result_code = UpdateVbrIfExternal(dt_type, ikey, ck_if, dmi) ;
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    }
  }
  for (int i = 0; i <2; i++)
    if (ck_if[i]) delete ck_if[i];
  return result_code;
}


/* Pure Virtual functions from MoMgrImpl */
upll_rc_t VlinkMoMgr::GetControllerDomainId(ConfigKeyVal *ikey,
                                          controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (!ikey || !ctrlr_dom) {
    UPLL_LOG_INFO("Illegal parameter\n");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_user_data()) {
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
    if ((ctrlr_dom->ctrlr && 
         !strlen(reinterpret_cast<const char *>(ctrlr_dom->ctrlr))) ||
        (ctrlr_dom->domain && 
        !strlen(reinterpret_cast<const char *>(ctrlr_dom->domain)))) {
      UPLL_LOG_DEBUG("Ctrlr domain null\n");
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_DEBUG("ctrlr_dom %s %s", ctrlr_dom->ctrlr, ctrlr_dom->domain);
  }
  if (ikey->get_cfg_val() && ikey->get_cfg_val()->get_user_data()) {
    GET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
    if ((ctrlr_dom[1].ctrlr && 
         !strlen(reinterpret_cast<const char *>(ctrlr_dom[1].ctrlr))) ||
        (ctrlr_dom[1].domain && 
        !strlen(reinterpret_cast<const char *>(ctrlr_dom[1].domain)))) {
      UPLL_LOG_DEBUG("Ctrlr domain null\n");
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_DEBUG("boundary 2nd ctrlr_dom %s %s",
                    ctrlr_dom[1].ctrlr, ctrlr_dom[1].domain);
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VlinkMoMgr::UpdateVbrIfExternal(upll_keytype_datatype_t dt_type,
                      ConfigKeyVal *ikey, ConfigKeyVal **vbrif,
                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dt_type == UPLL_DT_INVALID || dmi == NULL)
    return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t valid_boundary =
      reinterpret_cast<val_vlink *>(GetVal(ikey))->
                                valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
  if (valid_boundary == UNC_VF_VALID) {
    for (int i = 0; i < 2; i++) {
      result_code = CheckPortmapValidandUpdateVbrIf(ikey, vbrif[i],
                                                      dmi, dt_type);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Updation of VbrIf Portmap failed %d\n", result_code);
        return result_code;
      }
    } 
  } else if (valid_boundary == UNC_VF_VALID_NO_VALUE) {
    VbrIfMoMgr *mgr = reinterpret_cast<VbrIfMoMgr *>
              (const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_IF)));
    for (int i = 0; i < 2; i++) {
      val_drv_vbr_if *drv_ifval = reinterpret_cast<val_drv_vbr_if *>
                                GetVal(vbrif[i]);
      drv_ifval->vbr_if_val.valid[UPLL_IDX_PM_DRV_VBRI] = UNC_VF_VALID_NO_VALUE;
      result_code = reinterpret_cast<VbrIfMoMgr *>(mgr)->
                         UpdateConfigVal(vbrif[i], dt_type,dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Clear Portmap failed %d\n", result_code);
        return result_code;
      }
    }
  } else {
      UPLL_LOG_DEBUG("Internal vlink\n");
  }
  if (ck_boundary)
    delete ck_boundary;
  ck_boundary= NULL;
  return result_code;
}

upll_rc_t VlinkMoMgr::CheckPortmapValidandUpdateVbrIf(
    ConfigKeyVal *ikey, ConfigKeyVal *ck_drv_vbr_if, DalDmlIntf *dmi,
                                     upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || ck_boundary == NULL || dmi == NULL
      || ck_drv_vbr_if == NULL || dt_type == UPLL_DT_INVALID)
    return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VbrIfMoMgr *mgr = reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager*>
                                              (GetMoManager(UNC_KT_VBR_IF)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid param\n");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t valid_port = reinterpret_cast<val_drv_vbr_if *>
               (GetVal(ck_drv_vbr_if))->vbr_if_val.valid[UPLL_IDX_PM_DRV_VBRI];
  if (valid_port == UNC_VF_INVALID) {
    result_code = ConverttoDriverPortMap(ck_drv_vbr_if, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return result_code;
    }
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
    result_code = mgr->UpdateConfigDB(ck_drv_vbr_if, dt_type, UNC_OP_UPDATE,
                         dmi, &dbop, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Updation failed %d\n", result_code);
        return result_code;
    }
  } else if (valid_port == UNC_VF_VALID) {
#if 0
    if (CompareVbrIfWithPhysicalData(ck_drv_vbr_if, ck_boundary, ikey)) {
        UPLL_LOG_DEBUG("VbrIf is already configured \n");
        return UPLL_RC_ERR_GENERIC;
    }
#endif
  }
  return result_code;
}

bool VlinkMoMgr::CompareVbrIfWithPhysicalData(ConfigKeyVal *ck_drv_vbr_if,
                                              ConfigKeyVal *ck_boundary_data,
                                              ConfigKeyVal *ikey) {
  if (ikey == NULL || ck_drv_vbr_if == NULL || ck_boundary_data == NULL)
    return UPLL_RC_ERR_GENERIC;
  val_boundary *boundary_val_data = reinterpret_cast<val_boundary *>
                                            ((GetVal(ck_boundary_data)));
  if (boundary_val_data == NULL) {
    UPLL_LOG_DEBUG("Boundary invalid\n");
    return UPLL_RC_ERR_GENERIC;
  }
#if 0
  val_drv_port_map vbr_port_data = reinterpret_cast<val_drv_vbr_if *>(
                      (GetVal(ck_drv_vbr_if)))->portmap;
  if (*(reinterpret_cast<int*>
         (ikey->get_cfg_val()->get_user_data())) & kVnode1) {
    if (boundary_val_data->valid[kIdxValBoundarySwitchid1] != UNC_VF_VALID &&
     vbr_port_data.valid[UPLL_IDX_SWITCH_ID_DRV_PM] != UNC_VF_VALID)
    return false;
    else if (!strcmp(reinterpret_cast<char*>(&boundary_val_data->switch_id_1),
              reinterpret_cast<char*>&vbr_port_data.switch_id))
    return false;
    if (boundary_val_data->valid[kIdxValBoundaryPortname1] != UNC_VF_VALID &&
        vbr_port_data.valid[UPLL_IDX_PORT_NAME_DRV_PM] != UNC_VF_VALID)
    return false;
    else if (!strcmp(reinterpret_cast<char*>(&boundary_val_data->port_name_1),
           reinterpret_cast<char*>)(&vbr_port_data.port_name)))
    return false;

  } else if (*(reinterpret_cast<int*>(ikey->get_cfg_val()->get_user_data()))
            & kVnode2) {
    if (boundary_val_data->valid[kIdxValBoundarySwitchid2] != UNC_VF_VALID
        && vbr_port_data.valid[UPLL_IDX_SWITCH_ID_PM] != UNC_VF_VALID)
    return false;
    else if (!strcmp(reinterpret_cast<char*>(boundary_val_data->switch_id_2),
          reinterpret_cast<char*>(&vbr_port_data.switch_id)))
    return false;
    if (boundary_val_data->valid[kIdxValBoundaryPortname2] != UNC_VF_VALID
       && vbr_port_data.valid[UPLL_IDX_PORT_NAME_DRV_PM] != UNC_VF_VALID)
    return false;
    else if (!strcmp(reinterpret_cast<char*>(&boundary_val_data->port_name_2),
      reinterpret_cast<char*>(&vbr_port_data.port_name)))
    return false;
  }
  if (vlan_id != NULL && vbr_port_data.valid[UPLL_IDX_VLAN_ID_DRV_PM]
         == UNC_VF_VALID )
  if (vlan_id != vbr_port_data.vlan_id)
    return false;
#endif
  return true;
}

upll_rc_t VlinkMoMgr::ConverttoDriverPortMap(ConfigKeyVal *iokey,
                                         ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  val_boundary *boundary_val_data = reinterpret_cast<val_boundary *>
                                              (GetVal(ck_boundary));
  if (boundary_val_data == NULL) {
    UPLL_LOG_DEBUG("Invalid parameter\n");
    return UPLL_RC_ERR_GENERIC;
  }
  std::string if_name = reinterpret_cast<const char *>
                  (reinterpret_cast<key_vbr_if*>(iokey->get_key())->if_name);
  if (strlen(if_name.c_str()) >= 18)
  if_name.assign(if_name.c_str(), 18);
  std::string vex_name = "vx_" + if_name + static_cast<std::ostringstream*>(
                                &(std::ostringstream() << time(NULL)) )->str();
  std::string vex_if_name = "vi_" + if_name + static_cast<std::ostringstream*>(
                                &(std::ostringstream() << time(NULL)) )->str();
  std::string vex_link_name = "vl_" + if_name + static_cast<std::ostringstream*>(
                                &(std::ostringstream() << time(NULL)) )->str();

  val_drv_vbr_if *drv_ifval = reinterpret_cast<val_drv_vbr_if *>
                               (GetVal(iokey));
  if (!drv_ifval) {
    UPLL_LOG_DEBUG("Invalid param\n");
    return UPLL_RC_ERR_GENERIC;
  }
  val_port_map *port_map_val = &drv_ifval->vbr_if_val.portmap;
  uint8_t rename = 0;
  GET_USER_DATA_FLAGS(iokey, rename);
  char *port_id = NULL;
  uppl_val_boundary_index index;
  if (rename & kVlinkBoundaryNode1) {
    port_id = reinterpret_cast<char *>(boundary_val_data->logical_port_id1);
    index = kIdxBoundaryLogicalPortId1;
  } else {
    port_id = reinterpret_cast<char *>(boundary_val_data->logical_port_id2);
    index = kIdxBoundaryLogicalPortId2;
  }
  if (boundary_val_data->valid[index] == UNC_VF_VALID) {
      port_map_val->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
      uuu::upll_strncpy(port_map_val->logical_port_id,
              port_id, kMaxLenLogicalPortId+1);
  } else if (boundary_val_data->valid[index] == UNC_VF_VALID_NO_VALUE) {
      port_map_val->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
      uuu::upll_strncpy(port_map_val->logical_port_id, "\0", 1);
  } else {
      port_map_val->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
  }
  drv_ifval->vbr_if_val.valid[UPLL_IDX_PM_DRV_VBRI] = UNC_VF_VALID;
  drv_ifval->valid[UPLL_IDX_VEXT_DRV_PM] = UNC_VF_VALID;
  uuu::upll_strncpy(drv_ifval->vex_name, vex_name.c_str(), kMaxLenVnodeName+1);

  drv_ifval->valid[UPLL_IDX_VEXT_IF_DRV_PM] = UNC_VF_VALID;
  uuu::upll_strncpy(reinterpret_cast<char *>(drv_ifval->vex_if_name),
                    vex_if_name.c_str(), kMaxLenInterfaceName+1);

  drv_ifval->valid[UPLL_IDX_VEXT_LINK_DRV_PM] = UNC_VF_VALID;
  uuu::upll_strncpy(reinterpret_cast<char *>(drv_ifval->vex_link_name),
                    vex_link_name.c_str(), kMaxLenVlinkName+1);

  val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ikey));
  port_map_val->valid[UPLL_IDX_VLAN_ID_PM] =
                            vlink_val->valid[UPLL_IDX_VLAN_ID_VLNK];
  switch (vlink_val->valid[UPLL_IDX_VLAN_ID_VLNK]) {
  case UNC_VF_VALID:
    port_map_val->vlan_id = reinterpret_cast<val_vlink *>
                                ((ikey->get_cfg_val())->get_val())->vlan_id;
    break;
  case UNC_VF_VALID_NO_VALUE:
    port_map_val->vlan_id = 0;
    break;
  default:
    break;
  }
  port_map_val->tagged = UPLL_VLAN_TAGGED;
  port_map_val->valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::CheckIfMemberOfVlink(ConfigKeyVal *ck_vnif,
                                           upll_keytype_datatype_t dt_type,
                                           ConfigKeyVal *&ck_vlink,
                                           DalDmlIntf *dmi,
                                           vn_if_type &interface_type) {
  UPLL_FUNC_TRACE;
  uint8_t if_flag = 0;
  if (!ck_vnif) {
    UPLL_LOG_DEBUG("Invalid ck_vnif\n");
    return UPLL_RC_ERR_GENERIC;
  }
  interface_type = kUnlinkedInterface;
  GET_USER_DATA_FLAGS(ck_vnif, if_flag);
  SET_USER_DATA_FLAGS(ck_vnif, kVlinkVnode1);
  upll_rc_t result_code = GetVlinkKeyVal(ck_vnif, dt_type, ck_vlink, dmi);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    // check if the vBrIf is the second VnodeIf in VLINK TBL
    SET_USER_DATA_FLAGS(ck_vnif, kVlinkVnode2);
    if (ck_vlink) {
      delete ck_vlink;
      ck_vlink = NULL;
    }
    result_code = GetVlinkKeyVal(ck_vnif, dt_type, ck_vlink, dmi);
  } else {
    interface_type = kVlinkInternalNode1;
  }
  /* the interface is not part of a boundary vlink */
  if (result_code != UPLL_RC_SUCCESS) {
      if (ck_vlink) delete ck_vlink;
      ck_vlink = NULL;
      interface_type = kUnlinkedInterface;
  } else {
    if (!ck_vlink) {
       UPLL_LOG_DEBUG("Invalid param\n");
       return UPLL_RC_ERR_GENERIC;
    }
    uint8_t valid_boundary = reinterpret_cast<val_vlink *>(GetVal(ck_vlink))->
                            valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
    if (interface_type == kUnlinkedInterface) {
      interface_type = (valid_boundary)?
                       kVlinkBoundaryNode2:kVlinkInternalNode2;
    } else if (interface_type == kVlinkInternalNode1) {
      interface_type = (valid_boundary)? kVlinkBoundaryNode1: interface_type;
    } else {
      UPLL_LOG_DEBUG("Invalid value of interface_type %d\n", interface_type);
    }
  }
  SET_USER_DATA_FLAGS(ck_vnif, if_flag);
  return result_code;
}


upll_rc_t VlinkMoMgr::GetVlinkKeyVal(ConfigKeyVal *keyVal,
                          upll_keytype_datatype_t dt_type,
                                    ConfigKeyVal *&ck_vlink,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  int mem_vlink = 0;
  if (!keyVal) {
    UPLL_LOG_DEBUG("Invalid keyVal\n");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vlink *link_val = reinterpret_cast<val_vlink *>
                                  (malloc(sizeof(val_vlink)));
  if (!link_val) {
    UPLL_LOG_ERROR("Invalid linkval\n");
    return UPLL_RC_ERR_GENERIC;
  }
  memset(link_val, 0, sizeof(val_vlink));
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  key_vbr_if *vbr_key_if = reinterpret_cast<key_vbr_if *>(keyVal->get_key());
  if (!vbr_key_if) {
    UPLL_LOG_ERROR("Invalid key\n");
    free(link_val);
    return UPLL_RC_ERR_GENERIC;
  }
  GET_USER_DATA_FLAGS(keyVal, mem_vlink);
  mem_vlink &= VIF_TYPE;
  if (mem_vlink & kVlinkVnode2) {
    uuu::upll_strncpy(link_val->vnode2_name,
                     vbr_key_if->vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
    uuu::upll_strncpy(link_val->vnode2_ifname,
                     vbr_key_if->if_name, (kMaxLenVnodeName + 1));
    link_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_VALID;
    link_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_VALID;
    link_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
    link_val->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_INVALID;
  } else  {
    uuu::upll_strncpy(link_val->vnode1_name,
                      vbr_key_if->vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
    uuu::upll_strncpy(link_val->vnode1_ifname,
                      vbr_key_if->if_name, (kMaxLenVnodeName + 1));
    link_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_VALID;
    link_val->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_VALID;
    link_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_INVALID;
    link_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };

  /* get vlink key from if key */
  upll_rc_t result_code = GetChildConfigKey(ck_vlink, keyVal);
  if (!ck_vlink || result_code != UPLL_RC_SUCCESS) {
    if (link_val) free(link_val);
    UPLL_LOG_DEBUG("Returning error %d \n", result_code);
    return result_code;
  }
  ck_vlink->AppendCfgVal(IpctSt::kIpcStValVlink, link_val);
  res_code = ReadConfigDB(ck_vlink, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                       dbop, dmi, MAINTBL);
  UPLL_LOG_TRACE(" Vlink ReadConfigDB After %d", res_code);
  return res_code;
}

/*
 *  Based on the key type the bind info will pass
 */

bool VlinkMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                                      int &nattr, MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_vlink_maintbl_bind_info;
  } else if (RENAMETBL == tbl ) {
    nattr = NUM_KEY_RENAME_TBL_;
    binfo = key_vlink_renametbl_update_bind_info;
  }
  return PFC_TRUE;
}

upll_rc_t VlinkMoMgr::GetValid(void *val,
                               uint64_t indx,
                               uint8_t *&valid,
                               upll_keytype_datatype_t dt_type,
                               MoMgrTables tbl) {
  if (val == NULL) return UPLL_RC_ERR_GENERIC;
  if (tbl == MAINTBL) {
    switch (indx) {
      case uudst::vlink::kDbiOperStatus:
        valid = &(reinterpret_cast<val_vlink *>(val))->
                            valid[UPLL_IDX_OPER_STATUS_VLNKS];
          break;
      case uudst::vlink::kDbiDownCount:
        valid = NULL;
          break;
      case uudst::vlink::kDbiAdminStatus:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_ADMIN_STATUS_VLNK];
        break;
      case uudst::vlink::kDbiVnode1Name:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE1_NAME_VLNK];
        break;
      case uudst::vlink::kDbiVnode1Ifname:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE1_IF_NAME_VLNK];
        break;
      case uudst::vlink::kDbiVnode2Name:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE2_NAME_VLNK];
        break;
      case uudst::vlink::kDbiVnode2Ifname:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE2_IF_NAME_VLNK];
        break;
      case uudst::vlink::kDbiBoundaryName:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
        break;
      case uudst::vlink::kDbiVlanid:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_VLAN_ID_VLNK];
        break;
      case uudst::vlink::kDbiDesc:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_DESCRIPTION_VLNK];
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  } else if (tbl == RENAMETBL) {
    switch(indx) {
       case uudst::vlink_rename::kDbiCtrlrVtnName:
         valid = &(reinterpret_cast<val_rename_vnode *>(val))->
                            valid[UPLL_CTRLR_VTN_NAME_VALID];
         break;
       case uudst::vlink_rename::kDbiCtrlrVlinkName:
         valid = &(reinterpret_cast<val_rename_vnode *>(val))->
                            valid[UPLL_CTRLR_VNODE_NAME_VALID];

         break;
       default:
         break;
     }
  }
  return UPLL_RC_SUCCESS;
}

bool VlinkMoMgr::IsValidKey(void *key, uint64_t index) {
  UPLL_FUNC_TRACE;
  key_vlink *vlink_key = reinterpret_cast<key_vlink *>(key);
  bool ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vlink::kDbiVtnName:
    case uudst::vnode_rename::kDbiUncVtnName:
      ret_val = ValidateKey(
                          reinterpret_cast<char *>(vlink_key->vtn_key.vtn_name),
                          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vlink::kDbiVlinkName:
    case uudst::vnode_rename::kDbiUncvnodeName:
      ret_val = ValidateKey(reinterpret_cast<char *>(vlink_key->vlink_name),
                            kMinLenVlinkName, kMaxLenVlinkName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VLink Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_TRACE("Invalid Key Index");
      return false;
      break;
  }
  UPLL_LOG_TRACE("Leaving IsValidKey");
  return true;
}

upll_rc_t VlinkMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *parent_key) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vlink *vlink_key = reinterpret_cast<key_vlink *>
                                    (malloc(sizeof(key_vlink)));
  memset(vlink_key, 0, sizeof(key_vlink));
  unc_key_type_t ktype;

  void *pkey;
  if (parent_key == NULL) {
    okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink, vlink_key,
                            NULL);
    return (okey)?UPLL_RC_SUCCESS:UPLL_RC_ERR_GENERIC;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    free(vlink_key);
    return UPLL_RC_ERR_GENERIC;
  }
  ktype = parent_key->get_key_type();
  switch (ktype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name,
             (kMaxLenVtnName+1) );
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VBR_IF:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vbr_if *>(pkey)->vbr_key.vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VROUTER:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vrt *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VRT_IF:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vrt_if *>(pkey)->vrt_key.vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VLINK:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      uuu::upll_strncpy(vlink_key->vlink_name,
             reinterpret_cast<key_vlink *>(pkey)->vlink_name,
             (kMaxLenVlinkName+1));
      break;
    case UNC_KT_VTEP:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtep *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTEP_IF:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtep_if *>(pkey)->vtep_key.vtn_key.vtn_name,
              (kMaxLenVtnName+1));
      break;
    case UNC_KT_VUNKNOWN:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vunknown *>(pkey)->vtn_key.vtn_name,
              (kMaxLenVtnName+1));
      break;
    case UNC_KT_VUNK_IF:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vunk_if *>(pkey)->vunk_key.vtn_key.vtn_name,
              (kMaxLenVtnName+1));
      break;
    default:
      free(vlink_key);
      return UPLL_RC_ERR_GENERIC; /* OPERATION_NOT_SUPPORTED ? */
  }
  okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink, vlink_key,
                          NULL);
  if (okey == NULL) {
    free(vlink_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
     SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

/* Pure Virtual functions from MoMgrImpl */
upll_rc_t VlinkMoMgr::GetVnodeName(ConfigKeyVal *ikey, uint8_t *&vtn_name,
                                   uint8_t *&vnode_name) {
  UPLL_FUNC_TRACE;
  key_vlink *vlink_key = reinterpret_cast<key_vlink *>(ikey->get_key());
  if (vlink_key == NULL) return UPLL_RC_ERR_GENERIC;
  vtn_name = vlink_key->vtn_key.vtn_name;
  vnode_name = vlink_key->vlink_name;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                         ConfigKeyVal *ikey) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_VLINK) return UPLL_RC_ERR_GENERIC;
  void *pkey = (ikey) ? ikey->get_key() : NULL;
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>(malloc(sizeof(key_vtn)));
  memset(vtn_key, 0, sizeof(key_vtn));
  uuu::upll_strncpy(vtn_key->vtn_name,
         reinterpret_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
          (kMaxLenVtnName+1));
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  if (okey == NULL) result_code = UPLL_RC_ERR_GENERIC;
  return result_code;
}

upll_rc_t VlinkMoMgr::AllocVal(ConfigVal *&ck_val,
                               upll_keytype_datatype_t dt_type,
                               MoMgrTables tbl) {
  void *val;
  // ConfigVal *ck_nxtval;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>(malloc(sizeof(val_vlink)));

      if (!val) {
        UPLL_LOG_ERROR("Invalid memory allocation\n");
        return UPLL_RC_ERR_GENERIC;
      }
      memset(val, 0, sizeof(val_vlink));
      ck_val = new ConfigVal(IpctSt::kIpcStValVlink, val);
      if (!ck_val) {
        UPLL_LOG_ERROR("Invalid memory allocation\n");
        return UPLL_RC_ERR_GENERIC;
      }
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>(malloc(sizeof(val_db_vlink_st)));
        if (!val) {
          UPLL_LOG_ERROR("Invalid memory allocation\n");
          return UPLL_RC_ERR_GENERIC;
        }
        memset(val, 0, sizeof(val_db_vlink_st));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVlinkSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    case RENAMETBL:
      val = reinterpret_cast<void *>(malloc(sizeof(val_db_rename_vlink)));
      memset(val, 0, sizeof(val_db_rename_vlink));
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                                      MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if ((req == NULL) || (okey != NULL)) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VLINK) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_vlink *ival = reinterpret_cast<val_vlink *>(GetVal(req));
      if (!ival) {
        UPLL_LOG_TRACE("Empty Val structure\n");
        return UPLL_RC_ERR_GENERIC;
      }
      val_vlink *vlink_val = reinterpret_cast<val_vlink *>
                                        (malloc(sizeof(val_vlink)));
      memcpy(vlink_val, ival, sizeof(val_vlink));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVlink, vlink_val);
    } else if (tbl == RENAMETBL) {
      void *rename_val;
      ConfigVal *ck_v = req->get_cfg_val();
      if (ck_v->get_st_num() == IpctSt::kIpcInvalidStNum) {
        val_db_rename_vlink *ival = reinterpret_cast<val_db_rename_vlink *>
                                                             (GetVal(req));
        rename_val = reinterpret_cast<void *>
                                (malloc(sizeof(val_db_rename_vlink)));
        memcpy(rename_val, ival, sizeof(val_db_rename_vlink));
        tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum, rename_val);
      } else {
        val_rename_vlink *ival = reinterpret_cast<val_rename_vlink *>
                                                     (GetVal(req));
        rename_val = reinterpret_cast<void *>(malloc(sizeof(val_rename_vlink)));
        memcpy(rename_val, ival, sizeof(val_rename_vlink));
        tmp1 = new ConfigVal(IpctSt::kIpcStValRenameVlink, rename_val);
      }
    }
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL) {
      val_vlink_st *ival = reinterpret_cast<val_vlink_st *>(tmp->get_val());
      val_vlink_st *val_vlink = reinterpret_cast<val_vlink_st *>
                                        (malloc(sizeof(val_vlink_st)));
      memcpy(val_vlink, ival, sizeof(val_vlink_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVlinkSt, val_vlink);
      tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vlink *ikey = reinterpret_cast<key_vlink *>(tkey);
  key_vlink *vlink_key = reinterpret_cast<key_vlink *>
                                      (malloc(sizeof(key_vlink)));
  memcpy(vlink_key, ikey, sizeof(key_vlink));
  okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink, vlink_key,
                          tmp1);
  if (okey) {
    SET_USER_DATA(okey, req);
    SET_USER_DATA(okey->get_cfg_val(), req->get_cfg_val());
  } else {
    delete tmp1;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
                                       upll_keytype_datatype_t dt_type,
                                       DalDmlIntf *dmi, uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (ctrlr_id == NULL) return UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  val_db_rename_vlink *rename_vlink = reinterpret_cast<val_db_rename_vlink *>
                                  (malloc(sizeof(val_db_rename_vlink)));
  if (rename_vlink == NULL) return UPLL_RC_ERR_GENERIC;
  memset(rename_vlink, 0, sizeof(val_db_rename_vlink));
  key_vlink *ctrlr_key = reinterpret_cast<key_vlink *>(ikey->get_key());
  uuu::upll_strncpy(rename_vlink->ctrlr_vtn_name,
          ctrlr_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
  uuu::upll_strncpy(rename_vlink->ctrlr_vlink_name,
         ctrlr_key->vlink_name, (kMaxLenVtnName+1));

  result_code = GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed result_code %d\n", result_code);
    return result_code;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  rename_vlink->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
  rename_vlink->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

  unc_key->AppendCfgVal(IpctSt::kIpcInvalidStNum, rename_vlink);
  result_code = ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                       RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_vlink *vlink_key = reinterpret_cast<key_vlink *>(unc_key->get_key());
    if (strcmp(reinterpret_cast<const char *>(ctrlr_key->vtn_key.vtn_name),
              reinterpret_cast<const char *>(vlink_key->vtn_key.vtn_name))) {
      uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name,
             vlink_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
//      rename |= VTN_RENAME;
    }
    if (strcmp(reinterpret_cast<const char *>(ctrlr_key->vlink_name),
               reinterpret_cast<const char *>(vlink_key->vlink_name))) {
      uuu::upll_strncpy(reinterpret_cast<char *>(ctrlr_key->vlink_name),
             reinterpret_cast<const char *>(vlink_key->vlink_name),
              (kMaxLenVtnName+1));
//      rename |= VN_RENAME;
    }
    SET_USER_DATA(ikey, unc_key);
  }
  delete unc_key;
  return result_code;
}

upll_rc_t VlinkMoMgr::GetRenamedControllerKey(ConfigKeyVal *ikey,
                                              upll_keytype_datatype_t dt_type,
                                              DalDmlIntf *dmi,
                                              controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t rename = 1;
  ConfigKeyVal *okey = NULL;
  result_code = IsRenamed(ikey, dt_type, dmi, rename);
  if ((result_code != UPLL_RC_SUCCESS) || !ikey->get_cfg_val()) {
    UPLL_LOG_DEBUG("Invalid param %d\n", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  if (!ctrlr_dom->ctrlr ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->ctrlr)) ||
      !ctrlr_dom->domain ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->domain))) {
    GET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), *ctrlr_dom);
  }
  if (rename == 0) return UPLL_RC_SUCCESS;

  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;
  if ((ctrlr_dom->ctrlr == NULL) || (ctrlr_dom->domain == NULL)) {
    UPLL_LOG_INFO("Invalid ctrlr/domain\n");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  DbSubOp dbop = { kOpReadSingle, kOpMatchFlag | kOpMatchCtrlr, kOpInOutFlag
                       | kOpInOutCtrlr };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi, RENAMETBL);
  if (result_code != UPLL_RC_SUCCESS) return result_code;
  val_db_rename_vlink *rename_val = reinterpret_cast<val_db_rename_vlink *>
                                                           (GetVal(okey));
  if (!rename_val) return UPLL_RC_ERR_GENERIC;
  key_vlink *ctrlr_key = reinterpret_cast<key_vlink *>(ikey->get_key());
  if (!ctrlr_key) return UPLL_RC_ERR_GENERIC;
  memset(ctrlr_key, 0, sizeof(key_vlink));
  if (rename & 0x01) { /* vtn renamed */
    uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, rename_val->ctrlr_vtn_name,
                     (kMaxLenVtnName+1));
  }
  if (rename & 0x10) { /* vnode renamed */
    uuu::upll_strncpy(ctrlr_key->vlink_name, rename_val->ctrlr_vlink_name,
                     (kMaxLenVlinkName+1));
  }
  SET_USER_DATA_FLAGS(ikey, rename);
  delete okey;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::UpdateConfigStatus(ConfigKeyVal *vlink_key,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vlink *vlink_val;
  val_db_vlink_st *val_vlinkst;

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  vlink_val = reinterpret_cast<val_vlink *>(GetVal(vlink_key));
  if (vlink_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    val_vlinkst = reinterpret_cast<val_db_vlink_st *>
                           (malloc(sizeof(val_db_vlink_st)));
    memset(val_vlinkst, 0, sizeof(val_db_vlink_st));
    val_vlinkst->vlink_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
    val_vlinkst->down_count = 0;
    val_vlinkst->vlink_val_st.valid[UPLL_IDX_OPER_STATUS_VLNKS]
                                            = UNC_VF_INVALID;
    vlink_key->AppendCfgVal(IpctSt::kIpcStValVlinkSt, val_vlinkst);
    vlink_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    void *updval = reinterpret_cast<val_vlink *> (GetVal(upd_key));
    CompareValidValue(updval, vlink_val, true);
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  for ( unsigned int loop = 0;
        loop < sizeof(vlink_val->valid)/sizeof(vlink_val->valid[0]);
                                                         ++loop ) {
    // Setting CS to the not supported attributes
    if (UNC_VF_NOT_SOPPORTED == vlink_val->valid[loop]) {
      vlink_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
      continue;
    }
    if ( (UNC_VF_VALID == vlink_val->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == vlink_val->valid[loop]))
        vlink_val->cs_attr[loop] = cs_status;
  }
  SetConsolidatedStatus(vlink_key, op, cs_status, dmi);
  return result_code;
}

#if 0
upll_rc_t VlinkMoMgr::GetControllerKey(ConfigKeyVal *ikey,
    ConfigKeyVal *&okey, unc_keytype_datatype_t dt_type, char *ctrlr_name ) {
  okey = NULL;
  char rename = (uint64_t)(ikey->get_user_data());
  key_vlink_t *key = reinterpret_cast<key_vlink_t *>(ikey->get_key());
  if (!rename) return UPLL_RC_SUCCESS;
  key_vlink_t *ctrlr_key = new key_vlink_t;

  /* vtn renamed */
  if (rename & 0x01) {
    key_vtn_t *vtn_key = new key_vtn_t;
    uuu::upll_strncpy(vtn_key->vtn_name, (key->vtn_key.vtn_name));
    ConfigKeyVal *ck_vtn = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                            vtn_key, NULL);
    //   MoMgrImpl *mgr = (MoMgrImpl *)GetMoManager(UNC_KT_VTN);
//    mgr->ReadConfigDB(ck_vtn, dt_type, UNC_OP_READ, RENAMETBL);
    val_rename_vtn* rename_val = reinterpret_cast<val_rename_vtn *>
                                                  (GetVal(ck_vtn));
    uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, rename_val->new_name);
    delete ck_vtn;
  }
  /* vnode renamed */
  if (rename & 0x10) {
    GetChildConfigKey(okey, ikey);
    //  mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, RENAMETBL);
    val_rename_vbr *rename_val = reinterpret_cast<val_rename_vbr *>
                                                          (GetVal(okey));
    uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, rename_val->new_name);
    delete okey;
  }
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVlink,
                          ctrlr_key, NULL);
  return UPLL_RC_SUCCESS;
}
#endif

upll_rc_t VlinkMoMgr::SwapKeyVal(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                                 DalDmlIntf *dmi, uint8_t *ctrlr,
                                 bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  okey = NULL;
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  if (ikey->get_key_type() != UNC_KT_VLINK) return UPLL_RC_ERR_BAD_REQUEST;
  val_rename_vlink_t *tval = reinterpret_cast<val_rename_vlink_t *>
                             (GetVal(ikey));
  if (!tval)
    return UPLL_RC_ERR_GENERIC;

  /* The PFC Name and New Name should not be equal */
  if (!strcmp(reinterpret_cast<char *>(tval->new_name),
         reinterpret_cast<char *>(reinterpret_cast<key_vlink_t *>
         (ikey->get_key())->vlink_name)))
    return UPLL_RC_ERR_GENERIC;
  key_vlink_t * key_vlink = reinterpret_cast<key_vlink_t *>
                            (ConfigKeyVal::Malloc(sizeof(key_vlink_t)));

  if (tval->valid[UPLL_IDX_NEW_NAME_RVLNK] == UNC_VF_VALID_NO_VALUE) {
    uuu::upll_strncpy(key_vlink->vlink_name,
            static_cast<key_vlink_t *>(ikey->get_key())->vlink_name,
            (kMaxLenVlinkName+1));
    no_rename = true;
  } else {
    if ((reinterpret_cast<val_rename_vlink_t *>(tval))->
            valid[UPLL_IDX_NEW_NAME_RVLNK] == UNC_VF_VALID) {
      /* checking the string is empty or not */
      if (!strlen(reinterpret_cast<char *>
         (static_cast<val_rename_vlink_t *>(tval)->new_name))) {
        free(key_vlink);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(key_vlink->vlink_name,
         static_cast<val_rename_vlink_t *>(GetVal(ikey))->new_name,
         (kMaxLenVlinkName+1));
    } else {
      free(key_vlink);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  /* Checking the vlink parent is renamed get the UNC name */
  ConfigKeyVal *pkey = NULL;
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    free(key_vlink);
    return result_code;
  }
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                              (GetMoManager(UNC_KT_VTN)));
  result_code = mgr->GetRenamedUncKey(pkey, UPLL_DT_IMPORT, dmi, ctrlr);
  if (UPLL_RC_SUCCESS != result_code
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    free(key_vlink);
    return result_code;
  }
  /* use the UNC VTN name if PFC VTN name is renamed; */
  if (strlen(reinterpret_cast<char *>(reinterpret_cast<key_vtn_t *>
                                              (pkey->get_key())->vtn_name)))
    uuu::upll_strncpy(key_vlink->vtn_key.vtn_name,
            reinterpret_cast<key_vtn_t *>(pkey->get_key())->vtn_name,
            (kMaxLenVtnName+1));
    delete pkey;
    pkey = NULL;
  okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink, key_vlink,
                          NULL);
  if (NULL == okey) {
    free(key_vlink);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>
                                                      (ikey->get_key());
  key_vlink_t * key_vlink = reinterpret_cast<key_vlink_t *>(
                              ConfigKeyVal::Malloc(sizeof(key_vlink_t)));
    UPLL_LOG_TRACE("Table is MainTable "); 
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
      FREE_IF_NOT_NULL(key_vlink);
      return UPLL_RC_ERR_GENERIC;
     }
    
    uuu::upll_strncpy(key_vlink->vtn_key.vtn_name,
                key_rename->old_unc_vtn_name, (kMaxLenVtnName+1));

     if (ikey->get_key_type() == table[MAINTBL]->get_key_type()) {
       UPLL_LOG_TRACE("Current Rename Key type is %d", ikey->get_key_type());
       if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
         FREE_IF_NOT_NULL(key_vlink);
         return UPLL_RC_ERR_GENERIC;
       }
       uuu::upll_strncpy(key_vlink->vlink_name,
          key_rename->old_unc_vnode_name, (kMaxLenVlinkName+1));
     } 
  okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink, key_vlink,
                          NULL);
  if (!okey) {
    free(key_vlink);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VlinkMoMgr::GetRenameInfo(ConfigKeyVal *ikey,
                                    ConfigKeyVal *okey,
                                    ConfigKeyVal *&rename_info,
                                    DalDmlIntf *dmi,
                                    const char *ctrlr_id,
                                    bool &renamed) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!okey || !ikey) {
    UPLL_LOG_DEBUG("Empty Input");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vlink_t * vlink_key_ = NULL;

  vlink_key_ = reinterpret_cast<key_vlink_t *>(ikey->get_key());
  if (!vlink_key_) {
    UPLL_LOG_DEBUG("Input Key Is Empty");
    return UPLL_RC_ERR_GENERIC;
  }
  key_rename_vnode_info_t *vlink_rename_info =
                        reinterpret_cast<key_rename_vnode_info_t *>
                        (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info_t)));
  if (renamed) {
    if (!strlen(reinterpret_cast<char *>(reinterpret_cast<val_rename_vnode *>
                                       (GetVal(ikey))->ctrlr_vtn_name))) {
      free(vlink_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vlink_rename_info->ctrlr_vtn_name,
          reinterpret_cast<val_rename_vnode *>(GetVal(ikey))->ctrlr_vtn_name,
          (kMaxLenVtnName+1));
    if (!strlen(reinterpret_cast<char *>(reinterpret_cast<val_rename_vnode *>
                                         (GetVal(ikey))->ctrlr_vnode_name))) {
      free(vlink_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vlink_rename_info->ctrlr_vnode_name,
          reinterpret_cast<val_rename_vnode *>(GetVal(ikey))->ctrlr_vnode_name,
          (kMaxLenVlinkName+1));
  } else {
    if (!strlen(reinterpret_cast<char *>(vlink_key_->vlink_name))) {
       free(vlink_rename_info);
       return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vlink_rename_info->ctrlr_vnode_name,
           vlink_key_->vlink_name, (kMaxLenVlinkName+1));
    if (!strlen(reinterpret_cast<char *>(vlink_key_->vtn_key.vtn_name))) {
      free(vlink_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vlink_rename_info->ctrlr_vtn_name,
           vlink_key_->vtn_key.vtn_name, (kMaxLenVtnName+1));
  }
  if (!strlen(reinterpret_cast<char *>(vlink_key_->vlink_name))) {
       free(vlink_rename_info);
       return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vlink_rename_info->old_unc_vnode_name,
         vlink_key_->vlink_name, (kMaxLenVlinkName+1));
  if (!strlen(reinterpret_cast<char *>(vlink_key_->vtn_key.vtn_name))) {
    free(vlink_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vlink_rename_info->new_unc_vtn_name,
         vlink_key_->vtn_key.vtn_name, (kMaxLenVtnName+1));
  uuu::upll_strncpy(vlink_rename_info->old_unc_vtn_name,
         vlink_key_->vtn_key.vtn_name, (kMaxLenVtnName+1));

  if (!(okey->get_key())) {
    free(vlink_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vlink_t *>
                                      (okey->get_key())->vlink_name))) {
    free(vlink_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vlink_rename_info->new_unc_vnode_name,
       reinterpret_cast<key_vlink_t *>(okey->get_key())->vlink_name,
       (kMaxLenVlinkName+1));

  rename_info = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcInvalidStNum,
                                 vlink_rename_info, NULL);
  if (!rename_info) {
    free(vlink_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }

  if (!renamed) {
    val_rename_vnode_t *vnode = reinterpret_cast<val_rename_vnode_t*>
        (ConfigKeyVal::Malloc(sizeof(val_rename_vnode_t)));
    ConfigKeyVal *tmp_key = NULL;
    result_code = GetChildConfigKey(tmp_key, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
       UPLL_LOG_DEBUG("GetChildConfigKey Failed ");
       return result_code;
    }
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};
    result_code = ReadConfigDB(tmp_key, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                                                 MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB Failed");
      return result_code;
    }
    controller_domain ctrlr_dom;
    result_code = GetControllerDomainId(tmp_key, &ctrlr_dom);
    if (UPLL_RC_SUCCESS != result_code)
       return result_code;
    SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);

    vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
    vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

    uuu::upll_strncpy(vnode->ctrlr_vtn_name,
           reinterpret_cast<key_vlink_t *>(ikey->get_key())->vtn_key.vtn_name,
            (kMaxLenVtnName+1));
    uuu::upll_strncpy(vnode->ctrlr_vnode_name,
           reinterpret_cast<key_vlink_t *>(ikey->get_key())->vlink_name,
          (kMaxLenVlinkName+1));
    ConfigVal *rename_val_ = new ConfigVal(IpctSt::kIpcInvalidStNum, vnode);
    okey->SetCfgVal(rename_val_);
    dbop.readop = kOpNotRead;
    result_code = UpdateConfigDB(okey, UPLL_DT_IMPORT, UNC_OP_CREATE, dmi,
                                 RENAMETBL);
    if (tmp_key)
      delete tmp_key;
  }
  return result_code;
}

upll_rc_t VlinkMoMgr::UpdateVnodeVal(ConfigKeyVal *rename_info,
                                     DalDmlIntf *dmi,
                                     upll_keytype_datatype_t data_type,
                                     bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!rename_info) return UPLL_RC_ERR_GENERIC;
  key_rename_vnode_info *rename_info_ =
                          reinterpret_cast<key_rename_vnode_info *>(
                          rename_info->get_key());
  if (!rename_info_){
    UPLL_LOG_DEBUG("Rename Info is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  uint8_t rename = 0;
  key_vlink_t *key_vlink_ =  NULL;
  val_vlink_t *val_vlink_ =  reinterpret_cast<val_vlink_t *>
      (ConfigKeyVal::Malloc(sizeof(val_vlink_t)));
  ConfigKeyVal *okey = NULL;
  UPLL_LOG_TRACE("Rename NoRename Falg = %d", no_rename);
  GetChildConfigKey(okey, NULL);

  if (!okey || !(okey->get_key())) { 
    UPLL_LOG_TRACE("GetChildConfigKey Failed ");
    free(val_vlink_);
    return UPLL_RC_ERR_GENERIC;
  }
  okey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValVlink, val_vlink_));

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag|kOpInOutCtrlr|kOpInOutDomain };

  key_vlink_ = reinterpret_cast<key_vlink_t *>(okey->get_key());

  uuu::upll_strncpy(key_vlink_->vtn_key.vtn_name,
         rename_info_->new_unc_vtn_name, (kMaxLenVtnName+1));

  uuu::upll_strncpy(val_vlink_->vnode1_name,
         rename_info_->old_unc_vnode_name, (kMaxLenVnodeName+1));

  val_vlink_->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_VALID;

  result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi, MAINTBL);

  if (UPLL_RC_SUCCESS == result_code) {
    while (okey) {
      val_vlink_ =  reinterpret_cast<val_vlink_t *>(GetVal(okey));
      UPLL_LOG_TRACE("After Read ConfigDB in Vlink %s", (okey->ToStrAll()).c_str());

      GET_USER_DATA_FLAGS(okey, rename);
      UPLL_LOG_TRACE("Rename No Rename %d", no_rename);

      UPLL_LOG_TRACE("Before Update Rename flag =%d", rename);
      if (!no_rename)
        rename = rename | VN1_RENAME;
      else
        rename = rename & NO_VN1_RENAME;

      UPLL_LOG_TRACE("After Update Rename flag =%d", rename);
      SET_USER_DATA_FLAGS(okey, rename);

      uuu::upll_strncpy(val_vlink_->vnode1_name,
             rename_info_->new_unc_vnode_name, (kMaxLenVnodeName+1));

      UPLL_LOG_TRACE("The New vnode name %s", val_vlink_->vnode1_name);
      UPLL_LOG_TRACE("Before Update ConfigDB in Vlink %s", (okey->ToStrAll()).c_str());
      dbop.readop = kOpNotRead;
      dbop.inoutop =  kOpInOutFlag ; 

      result_code = UpdateConfigDB(okey, data_type, UNC_OP_UPDATE, dmi,
                                   &dbop, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) return result_code;
      okey = okey->get_next_cfg_key_val();
    }
    if (okey) {
      delete okey;
    }
    return result_code;
  }

  val_vlink_ =  reinterpret_cast<val_vlink_t *>(GetVal(okey));
  val_vlink_->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
  uuu::upll_strncpy(val_vlink_->vnode2_name,
         rename_info_->old_unc_vnode_name, (kMaxLenVnodeName+1));
  val_vlink_->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_VALID;

  result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS == result_code) {
    while (okey) {
      val_vlink_ =  reinterpret_cast<val_vlink_t *>(GetVal(okey));

      GET_USER_DATA_FLAGS(okey, rename);
      UPLL_LOG_TRACE("Rename No Rename %d", no_rename);
      UPLL_LOG_TRACE("Before Update Rename flag =%d", rename);

      if (!no_rename)
          rename = rename | VN2_RENAME;
      else
          rename = rename & NO_VN2_RENAME;

      UPLL_LOG_TRACE("After Update Rename flag =%d", rename);

      SET_USER_DATA_FLAGS(okey, rename);

      uuu::upll_strncpy(val_vlink_->vnode2_name,
              rename_info_->new_unc_vnode_name, (kMaxLenVnodeName+1));
      dbop.readop = kOpNotRead;
      dbop.inoutop =  kOpInOutFlag ; 
      result_code = UpdateConfigDB(okey, data_type, UNC_OP_UPDATE, dmi,
                                   &dbop, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) return result_code;
      okey = okey->get_next_cfg_key_val();
    }
  }
  if (okey) {
    delete okey;
  }
  return result_code;
}


upll_rc_t VlinkMoMgr::ValidateBoundary(uint8_t *boundary_name,
                                       IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  IpcResponse ipc_resp;

  key_boundary *bndrykey = static_cast<key_boundary *>
                                      (malloc(sizeof(key_boundary)));

  uuu::upll_strncpy(bndrykey->boundary_id, boundary_name,
                   (kMaxLenBoundaryName+1));
  ck_boundary = new ConfigKeyVal(UNC_KT_BOUNDARY, IpctSt::kIpcStKeyBoundary,
                                 bndrykey, NULL);

  result_code = SendIpcReq(req->clnt_sess_id, req->config_id, UNC_OP_READ,
                   UPLL_DT_CANDIDATE, ck_boundary, NULL, &ipc_resp);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in retrieving boundary data %d\n", result_code);
    delete ck_boundary;
    ck_boundary = NULL;
    return UPLL_RC_ERR_GENERIC;
  }
  /* store the boundary data returned by physical in temp. class variable*/
  if (ipc_resp.ckv_data)
    ck_boundary->ResetWith(ipc_resp.ckv_data);
  else
    UPLL_LOG_DEBUG("Boundary information not obtained from physical\n");
  return result_code;
}

upll_rc_t VlinkMoMgr::IsKeyInUse(upll_keytype_datatype_t dt_type,
                                 const ConfigKeyVal *ckv, bool *in_use,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_link = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  key_vlink_t *vlink_key = reinterpret_cast<key_vlink_t *>
                                    (malloc(sizeof(key_vlink)));
  if (!vlink_key) {
    UPLL_LOG_DEBUG("Memory Allocation Failed");
    return UPLL_RC_ERR_GENERIC;
  }
  memset(vlink_key, 0, sizeof(key_vlink));
  val_vlink_t *linkval = static_cast<val_vlink_t *>
                         (malloc(sizeof(val_vlink_t)));
  if (!linkval) {
    UPLL_LOG_DEBUG("Invalid memory allocation\n");
    free(vlink_key);
    return UPLL_RC_ERR_GENERIC;
  }
  memset(linkval, 0, sizeof(val_vlink_t));
  key_boundary *bndrykey = reinterpret_cast<key_boundary *>(ckv->get_key());
  if (!strlen(reinterpret_cast<const char *>(bndrykey->boundary_id))) {
    free(linkval);
    free(vlink_key);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(linkval->boundary_name, bndrykey->boundary_id,
                    (kMaxLenBoundaryName+1));
  linkval->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] = UNC_VF_VALID;
  ck_link = new ConfigKeyVal(UNC_KT_LINK, IpctSt::kIpcStKeyVlink, vlink_key,
                            new ConfigVal(IpctSt::kIpcStValVlink, linkval));
  result_code = ReadConfigDB(ck_link, dt_type, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  delete ck_link;
  if (UPLL_RC_SUCCESS != result_code
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failure %d", result_code);
    return result_code;
  }
  *in_use = (UPLL_RC_SUCCESS == result_code) ? true : false;
  UPLL_LOG_TRACE("IsKeyInUse : %d", *in_use);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status, uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vlink_t *val;
  val = (ckv_running != NULL) ? reinterpret_cast<val_vlink_t *>
                                     (GetVal(ckv_running)) : NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase) val->cs_row_status = cs_status;
  for (unsigned int loop = 0; loop < sizeof(val->valid) / sizeof(uint8_t);
      ++loop) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) ||
         cs_status == UNC_CS_APPLIED)
           val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

bool VlinkMoMgr::FilterAttributes(void *&val1, void *val2, bool copy_to_running,
                                  unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  val_vlink_t *val_vlink1 = reinterpret_cast<val_vlink_t *>(val1);
  val_vlink1->valid[UPLL_IDX_DESCRIPTION_VLNK] = UNC_VF_INVALID;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool VlinkMoMgr::CompareValidValue(void *&val1, void *val2, bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vlink_t *val_vlink1 = reinterpret_cast<val_vlink_t *>(val1);
  val_vlink_t *val_vlink2 = reinterpret_cast<val_vlink_t *>(val2);
  for (unsigned int loop = 0;
      loop < sizeof(val_vlink1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vlink1->valid[loop]
        && UNC_VF_VALID == val_vlink2->valid[loop])
      val_vlink1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  val_vlink1->valid[UPLL_IDX_DESCRIPTION_VLNK] = UNC_VF_INVALID;
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_ADMIN_STATUS_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_ADMIN_STATUS_VLNK]) {
    if (val_vlink1->admin_status == val_vlink2->admin_status)
      val_vlink1->valid[UPLL_IDX_ADMIN_STATUS_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_VNODE1_NAME_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_VNODE1_NAME_VLNK]) {
    if (!strcmp(reinterpret_cast<char*>(val_vlink1->vnode1_name),
                reinterpret_cast<char*>(val_vlink2->vnode1_name)))
      val_vlink1->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK]) {
    if (!strcmp(reinterpret_cast<char*>(val_vlink1->vnode1_ifname),
                reinterpret_cast<char*>(val_vlink2->vnode1_ifname)))
      val_vlink1->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_VNODE2_NAME_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_VNODE2_NAME_VLNK]) {
    if (!strcmp(reinterpret_cast<char*>(val_vlink1->vnode2_name),
                reinterpret_cast<char*>(val_vlink2->vnode2_name)))
      val_vlink1->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK]) {
    if (!strcmp(reinterpret_cast<char*>(val_vlink1->vnode2_ifname),
                reinterpret_cast<char*>(val_vlink2->vnode2_ifname)))
      val_vlink1->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_BOUNDARY_NAME_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_BOUNDARY_NAME_VLNK]) {
    if (!strcmp(reinterpret_cast<char*>(val_vlink1->boundary_name),
                reinterpret_cast<char*>(val_vlink2->boundary_name)))
      val_vlink1->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_VLAN_ID_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_VLAN_ID_VLNK]) {
    if (val_vlink1->vlan_id != val_vlink2->vlan_id)
      val_vlink1->valid[UPLL_IDX_VLAN_ID_VLNK] = UNC_VF_INVALID;
  }
  for (unsigned int loop = 0;
        loop < sizeof(val_vlink1->valid)/sizeof(val_vlink1->valid[0]);
                                                         ++loop ) {
    if ((UNC_VF_VALID == val_vlink1->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == val_vlink1->valid[loop]))
        invalid_attr = false;
  }
  return invalid_attr;
}

upll_rc_t VlinkMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                      ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VLINK != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVlink) {
    UPLL_LOG_DEBUG(
       "Invalid structure received.Expected struct-kIpcStKeyVlink, "
       "received struct -%d ",
       ((ikey->get_st_num())));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vlink_t *key_vlink = reinterpret_cast<key_vlink_t *>(ikey->get_key());
  if (key_vlink == NULL) {
    UPLL_LOG_DEBUG("key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ret_val = ValidateVlinkKey(key_vlink, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key struct Validation failed KT_VLINK");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  val_vlink_t *val_vlink = NULL;
  val_rename_vlink_t *val_rename_vlink = NULL;
  if ((ikey->get_cfg_val())
     && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVlink)) {
    val_vlink =
         reinterpret_cast<val_vlink_t *>(ikey->get_cfg_val()->get_val());
  } else if ((ikey->get_cfg_val()) &&
       ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValRenameVlink)) {
      val_rename_vlink =
        reinterpret_cast<val_rename_vlink_t *>(ikey->get_cfg_val()->get_val());
  }
  if ((operation == UNC_OP_CREATE) &&
      (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_IMPORT)) {
    if (val_vlink == NULL) {
      UPLL_LOG_DEBUG("Value structure is mandatory for CREATE op");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    UPLL_LOG_TRACE("The Input Key is %s", (ikey->ToStrAll()).c_str());
    ret_val = ValidateVlinkValue(val_vlink, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("val struct Validation failed for CREATE op ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_UPDATE) &&
             (dt_type == UPLL_DT_CANDIDATE)) {
    if (val_vlink == NULL) {
      UPLL_LOG_DEBUG("Value structure is mandatory for UPDATE op");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    ret_val = ValidateVlinkValue(val_vlink, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("val struct validation failed for UPDATE op");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_RENAME) && (dt_type == UPLL_DT_IMPORT)) {
    if (val_rename_vlink == NULL) {
      UPLL_LOG_DEBUG("Value rename struct is mandatory for RENAME op");
       return UPLL_RC_ERR_BAD_REQUEST;
    }
    ret_val = ValidateRenameVlinkValue(val_rename_vlink);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("val rename struct validation failed for Rename op");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING ||
              operation == UNC_OP_READ_SIBLING_BEGIN) &&
             (dt_type == UPLL_DT_IMPORT)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG("option2 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    if (val_rename_vlink == NULL) {
      UPLL_LOG_DEBUG("val rename structure is an optional for READ op");
       return UPLL_RC_SUCCESS;
    }
    ret_val = ValidateRenameVlinkValue(val_rename_vlink);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("val rename struct validation failed for READ op");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING ||
              operation == UNC_OP_READ_SIBLING_BEGIN) &&
         (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
          dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG("option2 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    if (val_vlink == NULL) {
      UPLL_LOG_DEBUG("Value struct is an optional for READ op");
      return UPLL_RC_SUCCESS;
    }
    ret_val = ValidateVlinkValue(val_vlink, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("val struct validation failed for READ op");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_DELETE) ||
             (operation == UNC_OP_READ_SIBLING_COUNT) ||
             (operation == UNC_OP_READ_NEXT) ||
             (operation == UNC_OP_READ_BULK)) {
    UPLL_LOG_DEBUG("Value structure validation is none for this operation :%d"
                  , operation);
    return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_DEBUG("Invalid datatype(%d) or operation(%d)", dt_type,
                  operation);
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
}

upll_rc_t VlinkMoMgr::ValidateVlinkValue(val_vlink_t *val_vlink,
                                         unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE("Operation type :(%d)", operation);
  
  if ((operation == UNC_OP_CREATE) &&
     ((val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] == UNC_VF_INVALID) ||
      (val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] == UNC_VF_VALID_NO_VALUE)||
      (val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] == UNC_VF_INVALID) ||
      (val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] ==
                                                     UNC_VF_VALID_NO_VALUE) ||
      (val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] == UNC_VF_INVALID) ||
      (val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] == UNC_VF_VALID_NO_VALUE) ||
      (val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] == UNC_VF_INVALID) ||
      (val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] ==
                                                     UNC_VF_VALID_NO_VALUE))) {
    UPLL_LOG_DEBUG("Vlink node name/If name is mandatory "
                                               " for Create operation\n");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation == UNC_OP_UPDATE) &&
     ((val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] != UNC_VF_INVALID) ||
     (val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] != UNC_VF_INVALID) ||
     (val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] != UNC_VF_INVALID) ||
     (val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK]  != UNC_VF_INVALID))) {
    UPLL_LOG_DEBUG("Vlink node name/If name must be invalid "
                                               " for Update operation\n");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_vlink->admin_status,
                              (uint8_t)(UPLL_ADMIN_ENABLE),
                              (uint8_t)(UPLL_ADMIN_DISABLE), true, true)) {
      UPLL_LOG_DEBUG("Syntax check failed. admin_status - %d",
                     val_vlink->admin_status);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK]
      == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vlink->admin_status = UPLL_ADMIN_ENABLE;
  }
  if ((val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] == UNC_VF_VALID) &&
      (!READ_OP(operation))) {
  if ((val_vlink->vnode1_name[0] == '\0')
      || (val_vlink->vnode1_ifname[0] == '\0')
      || (val_vlink->vnode2_name[0] == '\0')
      || (val_vlink->vnode2_ifname[0] == '\0')) {
    UPLL_LOG_DEBUG("Vlink node name/If name is empty!!");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  }
  if (val_vlink->valid[UPLL_IDX_DESCRIPTION_VLNK] == UNC_VF_VALID) {
    ret_val = ValidateDesc(reinterpret_cast<char *>(val_vlink->description),
                         kMinLenDescription, kMaxLenDescription);
   if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. description-  (%s)",
                    val_vlink->description);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vlink->valid[UPLL_IDX_DESCRIPTION_VLNK] ==
              UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vlink->description[0] = ' ';
    val_vlink->description[1] = '\0';
  }
  if (val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(val_vlink->vnode1_name),
                          kMinLenVnodeName, kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. vnode1_name- (%s)",
                    val_vlink->vnode1_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } 
  if (val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(val_vlink->vnode1_ifname),
                          kMinLenInterfaceName, kMaxLenInterfaceName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. vnode1_ifname- %s",
                    val_vlink->vnode1_ifname);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(val_vlink->vnode2_name),
                          kMinLenVnodeName, kMaxLenVnodeName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. vnode2_name- (%s)",
                    val_vlink->vnode2_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if (val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char*>(val_vlink->vnode2_ifname),
                          kMinLenInterfaceName, kMaxLenInterfaceName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed."
                    "vnode2_ifname- (%s)",
                    val_vlink->vnode2_ifname);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if (val_vlink->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(val_vlink->boundary_name),
                          kMinLenBoundaryName, kMaxLenBoundaryName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. "
                    "boundary_name- (%s)",
                    val_vlink->boundary_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
#if 0
    // Boundary validation
    ret_val = ValidateBoundary((val_vlink->boundary_name));
    if (ret_val != UPLL_RC_SUCCESS) return UPLL_RC_ERR_CFG_SEMANTIC;
#endif
  } else if ((val_vlink->valid[UPLL_IDX_BOUNDARY_NAME_VLNK]
      == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vlink->boundary_name[0] = ' ';
    val_vlink->boundary_name[1] = '\0';
  }

  if (val_vlink->valid[UPLL_IDX_VLAN_ID_VLNK] == UNC_VF_VALID) {
    if ((val_vlink->vlan_id != 0xFFFF) &&
         !ValidateNumericRange(val_vlink->vlan_id,
         (uint16_t)(kMinVlanId), (uint16_t)
         (kMaxVlanId), true, true)) {
      UPLL_LOG_DEBUG("Syntax check failed. vlan_id- (%d)", val_vlink->vlan_id);
     return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vlink->valid[UPLL_IDX_VLAN_ID_VLNK] == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
     val_vlink->vlan_id = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::ValidateRenameVlinkValue(
    val_rename_vlink_t *val_rename_vlink) {
  UPLL_FUNC_TRACE;
  bool ret_val = UPLL_RC_SUCCESS;
  if (val_rename_vlink->valid[UPLL_IDX_NEW_NAME_RVLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char*>(val_rename_vlink->new_name),
                          kMinLenVlinkName, kMaxLenVlinkName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed."
                    "new_name- (%s)",
                    val_rename_vlink->new_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::ValidateVlinkKey(key_vlink_t *key_vlink,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  int ret_val = UPLL_RC_SUCCESS;
  VtnMoMgr *objvtnmgr =
      reinterpret_cast<VtnMoMgr*>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VTN)));
  if (NULL == objvtnmgr) {
    UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }
  ret_val = objvtnmgr->ValidateVtnKey(&(key_vlink->vtn_key));

  if (UPLL_RC_SUCCESS != ret_val) {
    UPLL_LOG_DEBUG("Vtn_name syntax validation failed.Err Code- %d", ret_val);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    ret_val = ValidateKey(reinterpret_cast<char *>(key_vlink->vlink_name),
        kMinLenVlinkName, kMaxLenVlinkName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. vlink_name (%s)",
          key_vlink->vlink_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
      StringReset(key_vlink->vlink_name);
  }
  UPLL_LOG_TRACE("key structure validation successful for VLINK keytype");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::ValVlinkAttributeSupportCheck(const char * crtlr_name,
                                                    ConfigKeyVal *ikey,
                                                    uint32_t operation) {
  UPLL_FUNC_TRACE;
  bool result_code = false;
  uint32_t instance_count = 0;
  uint32_t num_attrs = 0;
  const uint8_t *attrs = 0;
  if ((ikey->get_cfg_val())->get_st_num() != IpctSt::kIpcStValVlink) {
    UPLL_LOG_DEBUG(
        " Invalid structure received.Expected struct-kIpcStValVlink, "
        "received struct - %s ",
        reinterpret_cast<const char *>(IpctSt::GetIpcStdef(
            (ikey->get_cfg_val())->get_st_num())));
    return UPLL_RC_ERR_GENERIC;
  }
  val_vlink_t *val_vlink = NULL;
  if ((ikey->get_cfg_val())
      && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVlink)) {
    val_vlink = reinterpret_cast<val_vlink_t *>(ikey->get_cfg_val()->get_val());
  }
  if (val_vlink != NULL) {
    switch (operation) {
      case UNC_OP_CREATE:
        UPLL_LOG_TRACE(" The Controller Name is %s", crtlr_name);
        result_code = GetCreateCapability(crtlr_name, ikey->get_key_type(),
                                          &instance_count, &num_attrs, &attrs);
        break;

      case UNC_OP_UPDATE:
        result_code = GetUpdateCapability(crtlr_name, ikey->get_key_type(),
                                          &num_attrs, &attrs);
        break;

      case UNC_OP_READ:
      case UNC_OP_READ_SIBLING:
      case UNC_OP_READ_SIBLING_BEGIN:
        result_code = GetReadCapability(crtlr_name, ikey->get_key_type(),
                                        &num_attrs, &attrs);
        break;

      default:
        UPLL_LOG_DEBUG("Invalid operation");
        break;
    }
    if (!result_code) {
      UPLL_LOG_DEBUG("key_type - %d is not supported by controller - %s",
                    ikey->get_key_type(), crtlr_name);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
    }
    if ((val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK] == UNC_VF_VALID)
        || (val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vlink::kCapAdminStatus] == 0) {
        val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_DEBUG(
            "UPLL_IDX_ADMIN_STATUS_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] == UNC_VF_VALID)
        || (val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] ==
         UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vlink::kCapVnode1Name] == 0) {
        val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_DEBUG(
            "UPLL_IDX_VNODE1_NAME_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] == UNC_VF_VALID)
        || (val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vlink::kCapVnode1IfName] == 0) {
        val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_DEBUG(
            "UPLL_IDX_VNODE1_IF_NAME_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] == UNC_VF_VALID)
        || (val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] ==
         UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vlink::kCapVnode2Name] == 0) {
        val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_DEBUG(
            "UPLL_IDX_VNODE2_NAME_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] == UNC_VF_VALID)
        || (val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vlink::kCapVnode2IfName] == 0) {
        val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_DEBUG(
            "UPLL_IDX_VNODE2_IF_NAME_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_vlink->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID)
        || (val_vlink->valid[UPLL_IDX_BOUNDARY_NAME_VLNK]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vlink::kCapBoundaryName] == 0) {
        val_vlink->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_DEBUG(
            "UPLL_IDX_BOUNDARY_NAME_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }

    if ((val_vlink->valid[UPLL_IDX_VLAN_ID_VLNK] == UNC_VF_VALID)
        || (val_vlink->valid[UPLL_IDX_VLAN_ID_VLNK] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vlink::kCapVlanId] == 0) {
        val_vlink->valid[UPLL_IDX_VLAN_ID_VLNK] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_DEBUG("UPLL_IDX_VLAN_ID_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  } else {
    UPLL_LOG_DEBUG("value structure is empty");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("ValVlink attributes are supported in PFC controller");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey,
                                         const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  /* controller name */
  if (!ctrlr_name)
    ctrlr_name = reinterpret_cast<char *>(ikey->get_user_data());

  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;
  UPLL_LOG_TRACE("dt_type   : (%d), "
                "operation : (%d), "
                "option1   : (%d), "
                "option2   : (%d)",
                dt_type, operation, option1, option2);
  if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE))
      && (dt_type == UPLL_DT_CANDIDATE)) {
    if ((GetVal(ikey)) != NULL) {
      ret_val = ValVlinkAttributeSupportCheck(ctrlr_name, ikey, operation);
      if (ret_val == UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("Attribute validation success for "
                                                     "VLINK value structure");
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Attribute validation failure for "
                                                    " VLINK value structure");
        return UPLL_RC_ERR_GENERIC;
      }
    } else {
      UPLL_LOG_INFO("Value structure mandatory for the CREATE operation");
      return UPLL_RC_ERR_GENERIC;
    }

  } else if (((operation == UNC_OP_READ) || (operation == UNC_OP_READ_SIBLING)
      || (operation == UNC_OP_READ_SIBLING_BEGIN))
      && ((dt_type == UPLL_DT_CANDIDATE) || (dt_type == UPLL_DT_RUNNING)
          || (dt_type == UPLL_DT_STARTUP))) {
    if (option1 == UNC_OPT1_NORMAL) {
      if (option2 == UNC_OPT2_NONE) {
        if ((GetVal(ikey)) != NULL) {
          ret_val = ValVlinkAttributeSupportCheck(ctrlr_name, ikey, operation);
          if (ret_val == UPLL_RC_SUCCESS) {
            UPLL_LOG_TRACE("Attribute check success for VLINK value structure");
            return UPLL_RC_SUCCESS;
          } else {
            UPLL_LOG_INFO("Attribute check failed for VLINK value structure");
            return UPLL_RC_ERR_GENERIC;
          }
        } else {
          UPLL_LOG_TRACE("Value structure is optional");
          return UPLL_RC_SUCCESS;
        }
      } else {
        UPLL_LOG_DEBUG("option2 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
    } else {
      UPLL_LOG_DEBUG("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
  } else if ((operation == UNC_OP_DELETE)
      || (operation == UNC_OP_READ_SIBLING_COUNT)
      || (operation == UNC_OP_READ_NEXT) || (operation == UNC_OP_READ_BULK)) {
    UPLL_LOG_TRACE("Value structure is none for operation type:%d", operation);
    return UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_DEBUG("Invalid datatype(%d) and operation(%d)", dt_type,
                                                             operation);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::CheckVnodeInfo(ConfigKeyVal *ikey,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  ConfigVal *val = NULL;
  if (!ikey) return UPLL_RC_ERR_GENERIC;

  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed result code %d\n", result_code);
    return result_code;
  }

  val_vlink_t *vlink_val = reinterpret_cast<val_vlink_t *>
                               (malloc(sizeof(val_vlink_t)));
  if (!vlink_val) return UPLL_RC_ERR_GENERIC;
  memset(vlink_val, 0, sizeof(val_vlink_t));
  vlink_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_VALID;
  vlink_val->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_VALID;
  vlink_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_INVALID;
  vlink_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;

  switch (ikey->get_key_type()) {
    case UNC_KT_VBRIDGE:
    case UNC_KT_VBR_IF:
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vbr_if_t*>
                                     (ikey->get_key())->vbr_key.vbridge_name)))
        return UPLL_RC_ERR_GENERIC;
      uuu::upll_strncpy(vlink_val->vnode1_name,
             reinterpret_cast<key_vbr_if_t*>(ikey->get_key())->
             vbr_key.vbridge_name, (kMaxLenVnodeName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vbr_if_t*>
                                       (ikey->get_key())->if_name)))
        return UPLL_RC_ERR_GENERIC;
      uuu::upll_strncpy(vlink_val->vnode1_ifname,
             reinterpret_cast<key_vbr_if_t*>(ikey->get_key())->if_name,
              (kMaxLenInterfaceName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vbr_if_t*>
                                      (ikey->get_key())->vbr_key.vbridge_name)))
        return UPLL_RC_ERR_GENERIC;
      uuu::upll_strncpy(vlink_val->vnode2_name, reinterpret_cast<key_vbr_if_t*>
                                      (ikey->get_key())->vbr_key.vbridge_name,
                   (kMaxLenVnodeName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vbr_if_t*>
                                      (ikey->get_key())->if_name)))
        return UPLL_RC_ERR_GENERIC;
      uuu::upll_strncpy(vlink_val->vnode2_ifname,
             reinterpret_cast<key_vbr_if_t*>(ikey->get_key())->if_name,
              (kMaxLenInterfaceName+1));

      break;
    case UNC_KT_VROUTER:
    case UNC_KT_VRT_IF:
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vrt_if_t*>
                                      (ikey->get_key())->vrt_key.vrouter_name)))
        return UPLL_RC_ERR_GENERIC;
      uuu::upll_strncpy(vlink_val->vnode1_name,
        reinterpret_cast<key_vrt_if_t*>(ikey->get_key())->vrt_key.vrouter_name,
         (kMaxLenVnodeName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vrt_if_t*>
                                                   (ikey->get_key())->if_name)))
        return UPLL_RC_ERR_GENERIC;
      uuu::upll_strncpy(vlink_val->vnode1_ifname,
             reinterpret_cast<key_vrt_if_t*>(ikey->get_key())->if_name,
             (kMaxLenInterfaceName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vrt_if_t*>
                                      (ikey->get_key())->vrt_key.vrouter_name)))
        return UPLL_RC_ERR_GENERIC;
      uuu::upll_strncpy(vlink_val->vnode2_name,
        reinterpret_cast<key_vrt_if_t*>(ikey->get_key())->vrt_key.vrouter_name,
        (kMaxLenVnodeName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vrt_if_t*>
                                                  (ikey->get_key())->if_name)))
        return UPLL_RC_ERR_GENERIC;
      uuu::upll_strncpy(vlink_val->vnode2_ifname,
             reinterpret_cast<key_vrt_if_t*>(ikey->get_key())->if_name,
             (kMaxLenInterfaceName+1));
      break;
    default:
      break;
  }
  val = new ConfigVal(IpctSt::kIpcStValVlink, vlink_val);
  if (!val) {
    free(vlink_val);
    return UPLL_RC_ERR_GENERIC;
  }
  okey->SetCfgVal(val);
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS == result_code) {
    return result_code;
  }
  vlink_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
  vlink_val->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_INVALID;
  vlink_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_VALID;
  vlink_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_VALID;

  DbSubOp dbop1 = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop1, dmi, MAINTBL);
  delete okey;
  return result_code;
}

upll_rc_t VlinkMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t dt_type,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (!ikey) return UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *ck_vlink = NULL;
  upll_rc_t result_code = GetChildConfigKey(ck_vlink, ikey);
  if (!ck_vlink || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Invalid param %d\n", result_code);
    return result_code;
  }
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutFlag};
  result_code = ReadConfigDB(ck_vlink, dt_type, UNC_OP_READ, dbop1,
                                  dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Invalid param %d\n", result_code);
    return result_code;
  }
  // uint8_t iftype, if1_type, if2_type;
  uint8_t if_flag = 0;
  /* reset the bits of the interfaces constituiing the vlink */
  SET_USER_DATA_FLAGS(ck_vlink->get_cfg_val(), kVlinkVnode1);
  for (int i = 0 ; i < 2; i++) {
    unc_key_type_t ktype;
    ktype = GetVlinkVnodeIfKeyType(ck_vlink, i);
    UPLL_LOG_DEBUG("Vlink Interface %d type : if1 %d \n", i, ktype);
    if (ktype == UNC_KT_ROOT) {
      UPLL_LOG_DEBUG("Invalid param\n");
      return UPLL_RC_ERR_GENERIC;
    }
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                        (GetMoManager(ktype)));
    ConfigKeyVal *ck_vnif = NULL;
    result_code = mgr->GetChildConfigKey(ck_vnif, ck_vlink);
    if (!ck_vnif || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    }
    result_code = mgr->ReadConfigDB(ck_vnif, dt_type, UNC_OP_READ,
                                    dbop1, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returing error %d\n", result_code);
      delete ck_vnif;
      return result_code;
    }
    GET_USER_DATA_FLAGS(ck_vnif, if_flag);
    if_flag &= ~VIF_TYPE;
    SET_USER_DATA_FLAGS(ck_vnif, if_flag);
    vnode_if_type vnif_type;
    result_code = UpdateVlinkMemIfFlag(dt_type, ck_vnif, dmi,
                                           vnif_type, mgr,UNC_OP_DELETE);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      delete ck_vnif;
      return result_code;
    }
    UPLL_LOG_DEBUG("Reset bit for iftype %d\n", vnif_type);
    if (ktype == UNC_KT_VBR_IF) {
      val_drv_vbr_if *val_drv_vbr = reinterpret_cast<val_drv_vbr_if *>
                                  (GetVal(ck_vnif));
      if (val_drv_vbr == NULL) {
        UPLL_LOG_DEBUG("Invalid param\n");
        return UPLL_RC_ERR_GENERIC;
      }
      if (val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
        val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID_NO_VALUE;        
        result_code = reinterpret_cast<VbrIfMoMgr *>(mgr)->
                         UpdateConfigVal(ck_vnif, dt_type,dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning %d\n",result_code);
          return result_code;
        }
        DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
        result_code = mgr->UpdateConfigDB(ck_vnif, dt_type, UNC_OP_UPDATE,
                               dmi, &dbop, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning %d\n",result_code);
          return result_code;
        }
      }
    }
    SET_USER_DATA_FLAGS(ck_vlink->get_cfg_val(), kVlinkVnode2);
    delete ck_vnif;
  }
  delete ck_vlink;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::SetConsolidatedStatus(ConfigKeyVal *vlink,
                                      unc_keytype_operation_t op,
                            unc_keytype_configstatus_t cs_status,
                                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(vlink));
  bool bound_vlink = false;
  if (!vlink_val) {
    UPLL_LOG_DEBUG("Invalid param\n");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t *ctrlr1 = NULL, *ctrlr2 = NULL;
  GET_USER_DATA_CTRLR(vlink,ctrlr1);
  GET_USER_DATA_CTRLR(vlink->get_cfg_val(),ctrlr2);
  if (!ctrlr1 || !ctrlr2) {
    UPLL_LOG_DEBUG("Invalid param\n");
    return UPLL_RC_ERR_GENERIC;
  }
  if (memcmp(ctrlr1,ctrlr2,kMaxLenCtrlrId+1))
     bound_vlink = true;
  ConfigKeyVal *vnif[2] = {NULL, NULL};
  unc_keytype_configstatus_t if_cstatus[2];
  VlinkNodePosition vnode_number = kVlinkVnode1;
  int i = 0;
  uint8_t rename_flag = 0;
  GET_USER_DATA_FLAGS(vlink->get_cfg_val(), rename_flag);
  do {
    unc_key_type_t ktype = GetVlinkVnodeIfKeyType(vlink, i);
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                      (GetMoManager(ktype)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr\n");
      return UPLL_RC_ERR_GENERIC;
    }
    SET_USER_DATA_FLAGS(vlink->get_cfg_val(), vnode_number);
    mgr->GetChildConfigKey(vnif[i], vlink);
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCs };
    result_code = mgr->ReadConfigDB(vnif[i], UPLL_DT_STATE, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    i++;
    vnode_number = (vnode_number == kVlinkVnode1)?kVlinkVnode2:kVlinkVnode1;
  } while (vnode_number != kVlinkVnode1);
  SET_USER_DATA_FLAGS(vlink->get_cfg_val(), rename_flag);
  if (!GetVal(vnif[0]) ||!GetVal(vnif[1])) {
    UPLL_LOG_DEBUG("Returning error\n");
    return UPLL_RC_ERR_GENERIC;
  }
  if (op == UNC_OP_CREATE) {
    /* update consolidated oper status */
    val_db_vbr_if_st *vnif1_st = reinterpret_cast<val_db_vbr_if_st *>
                     (GetStateVal(vnif[0]));
    val_db_vbr_if_st *vnif2_st = reinterpret_cast<val_db_vbr_if_st *>
                     (GetStateVal(vnif[1]));
    val_db_vlink_st *val_vlink_st = reinterpret_cast<val_db_vlink_st *>
                     (GetStateVal(vlink));
    val_oper_status oper_status[2];

    if ((vnif1_st->vbr_if_val_st.valid[UPLL_IDX_OPER_STATUS_VBRIS] !=
          UNC_VF_VALID) ||
        (vnif2_st->vbr_if_val_st.valid[UPLL_IDX_OPER_STATUS_VBRIS] !=
          UNC_VF_VALID)) {
       val_vlink_st->vlink_val_st.valid[UPLL_IDX_OPER_STATUS_VLNKS] =
          UNC_VF_INVALID;
       UPLL_LOG_DEBUG("Oper status not initlaized correctly\n");
    } else {
      oper_status[0] = (val_oper_status)vnif1_st->vbr_if_val_st.oper_status;
      oper_status[1] = (val_oper_status)vnif2_st->vbr_if_val_st.oper_status;
      if ((oper_status[0] == oper_status[1]) == UPLL_OPER_STATUS_UP)
        val_vlink_st->vlink_val_st.oper_status = UPLL_OPER_STATUS_UP;
      else if ((oper_status[0] == UPLL_OPER_STATUS_UNKNOWN) ||
             (oper_status[1] == UPLL_OPER_STATUS_UNKNOWN))
        val_vlink_st->vlink_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
      else if ((oper_status[0] == UPLL_OPER_STATUS_DOWN) ||
               (oper_status[1] == UPLL_OPER_STATUS_DOWN))  
        val_vlink_st->vlink_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
      val_vlink_st->vlink_val_st.valid[UPLL_IDX_OPER_STATUS_VLNKS] =
                                                             UNC_VF_VALID;
    }

    /* update consolidated config status if boundary vlink */
    if (bound_vlink) {
      if_cstatus[0] = (unc_keytype_configstatus_t)
                      (reinterpret_cast<val_vbr_if *>
                      (GetVal(vnif[0]))->cs_row_status);
      if_cstatus[1] = (unc_keytype_configstatus_t)
                      (reinterpret_cast<val_vbr_if *>
                       (GetVal(vnif[1]))->cs_row_status);
      if ((if_cstatus[0] == UNC_CS_APPLIED) &&
        (if_cstatus[1] == UNC_CS_APPLIED))
        vlink_val->cs_row_status = UNC_CS_APPLIED;
      else if ((if_cstatus[0] == UNC_CS_INVALID) ||
           (if_cstatus[1] == UNC_CS_INVALID))
        vlink_val->cs_row_status = UNC_CS_INVALID;
      else if ((if_cstatus[0] == UNC_CS_NOT_APPLIED) &&
           (if_cstatus[1] == UNC_CS_NOT_APPLIED))
        vlink_val->cs_row_status = UNC_CS_NOT_APPLIED;
      else
        vlink_val->cs_row_status = UNC_CS_PARTAILLY_APPLIED;
    }
  }
  if (bound_vlink) {
    val_port_map *pm1 = &(reinterpret_cast<val_drv_vbr_if *>
                       (GetVal(vnif[0]))->vbr_if_val.portmap);
    val_port_map *pm2 = &(reinterpret_cast<val_drv_vbr_if *>
                       (GetVal(vnif[1]))->vbr_if_val.portmap);

    if ((pm1->valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) ||
        (pm1->valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID_NO_VALUE)) {
      if_cstatus[0] = (unc_keytype_configstatus_t)
                     (pm1->cs_attr[UPLL_IDX_VLAN_ID_PM]);
      if_cstatus[1] = (unc_keytype_configstatus_t)
                      (pm2->cs_attr[UPLL_IDX_VLAN_ID_PM]);
      if ((if_cstatus[0] == UNC_CS_APPLIED) &&
          (if_cstatus[1] == UNC_CS_APPLIED))
        vlink_val->cs_attr[UPLL_IDX_VLAN_ID_VLNK] = UNC_CS_APPLIED;
      else if ((if_cstatus[0] == UNC_CS_INVALID) ||
               (if_cstatus[1] == UNC_CS_INVALID))
        vlink_val->cs_attr[UPLL_IDX_VLAN_ID_VLNK] = UNC_CS_INVALID;
      else if ((if_cstatus[0] == UNC_CS_NOT_APPLIED) &&
               (if_cstatus[1] == UNC_CS_NOT_APPLIED))
        vlink_val->cs_attr[UPLL_IDX_VLAN_ID_VLNK]  = UNC_CS_NOT_APPLIED;
      else
        vlink_val->cs_attr[UPLL_IDX_VLAN_ID_VLNK] = UNC_CS_PARTAILLY_APPLIED;
    }
  }
  if (vnif[0]) delete vnif[0];
  if (vnif[1]) delete vnif[1];
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::SetOperStatus(ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                    int notification, bool skip) {
  UPLL_FUNC_TRACE;
  if (!ikey) return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code;
  if (!skip) {
    DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
    result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                               MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Error in reading: %d", result_code);
      return result_code;
    }
  }
  ConfigKeyVal *tkey = ikey;
  while (tkey != NULL) {
    ConfigVal *tmp =
        (ikey->get_cfg_val()) ? ikey->get_cfg_val()->get_next_cfg_val() : NULL;
    val_vlink_st_t *vlink_valst = (tmp != NULL) ?
                      reinterpret_cast<val_vlink_st_t *>(tmp->get_val()) : NULL;
    if (vlink_valst == NULL) return UPLL_RC_ERR_GENERIC;

    /* Update oper status based on notification */
    vlink_valst->valid[UPLL_IDX_OPER_STATUS_VLNKS] = UNC_VF_VALID;
#if TODO
    switch (notification) {
      case CONNECTION_FAULT:
      vlink_valst->oper_status = UPLL_OPER_STATUS_UNKNOWN
      break;
      case PATHFAULT:
      vlink_valst->down_count = (notification == UPLL_ALARM_RAISE)?
      (vlink_valst->down_count + 1):(vlink_valst->down_count > 0)?
      (vlink_valst->down_count - 1):0;
      if (notification == UPLL_ALARM_RAISE &&
          (vlink_valst->down_count == 1)) {
        vlink_valst->oper_status = UPLL_OPER_STATUS_DOWN
        /* generate alarm */
        /* GenerateAlarm(notification); */
      } else if (notification == UPLL_ALARM_CLEAR &&
          (vlink_valst->down_count == 0)) {
        vlink_valst->oper_status = UPLL_OPER_STATUS_UP;
        /* reset alarm  */
        /*ResetAlarm(notification);*/
      }
      break;
      default:
      break;
    }
#endif
    result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                                 MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Error in update oper status");
      return result_code;
    }
    tkey = tkey->get_next_cfg_key_val();
  }
  if (ikey->get_cfg_val()) delete ikey->get_cfg_val();
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::MergeValidate(unc_key_type_t keytype,
                                    const char *ctrlr_id, ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp = NULL;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG(" Input key is NULL");
    return UPLL_RC_ERR_GENERIC; 
  }
  /*
   * Here getting FULL Key (VTN & Vlink Name )
   */
  result_code = ReadConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) return result_code;
  /* checks the vnode name present in the running vnode under the
   * same vtn
   */
  while (ikey) {
    /* Same Name should not present in the vnodes in running*/
    result_code = VnodeChecks(ikey, UPLL_DT_RUNNING, dmi);

    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
      UPLL_LOG_DEBUG("Vlink Name Conflict %d", result_code);
      return UPLL_RC_ERR_MERGE_CONFLICT;
    }
    /* Any other DB error */
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" VnodeChecks Failed %d", result_code);
      return result_code;
    }
    tmp = ikey;
    ikey = ikey->get_next_cfg_key_val();
    if (tmp) {
     delete tmp;
    }
  }
  return result_code;
}

upll_rc_t VlinkMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  // To adapt the value structure with VTNService
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey\n");
    return UPLL_RC_ERR_GENERIC;
  }
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Config Val is Null\n");
      return UPLL_RC_ERR_GENERIC;
    }
    while (cval ) {
      if (IpctSt::kIpcStValVlinkSt == cval->get_st_num()) {
        val_vlink_st *vlink_val_st = reinterpret_cast<val_vlink_st *>
                         (malloc(sizeof(val_vlink_st)));
        if (!vlink_val_st) {
          UPLL_LOG_ERROR("Memory Allocation failed");
          return UPLL_RC_ERR_GENERIC;
        }
        val_db_vlink_st *db_vlink_val_st = reinterpret_cast<val_db_vlink_st *>
                                     (cval->get_val());
        memcpy(vlink_val_st, &(db_vlink_val_st->vlink_val_st),
               sizeof(val_vlink_st));
        cval->SetVal(IpctSt::kIpcStValVlinkSt, vlink_val_st);
      } else if (IpctSt::kIpcStValRenameVlink == cval->get_st_num()) {
        void *key = NULL;
        key = ikey->get_key();
        ConfigVal *val = ikey->get_cfg_val();
        val_rename_vlink_t *val_rename_vlink = reinterpret_cast
           <val_rename_vlink_t *> (malloc(sizeof(val_rename_vlink_t)));
         // Getting valid
         val_rename_vlink->valid[UPLL_IDX_NEW_NAME_RVLNK] = UNC_VF_VALID;
      // Getting new_name
         uuu::upll_strncpy(val_rename_vlink->new_name,
             (reinterpret_cast<key_vlink_t*>(key))->vlink_name,
             (kMaxLenVlinkName+1));
         val->SetVal(IpctSt::kIpcStValRenameVlink, val_rename_vlink);
         ikey->SetCfgVal(val);
      }
      cval = cval->get_next_cfg_val();
    }
    ikey = ikey->get_next_cfg_key_val();
  }
  UPLL_LOG_DEBUG("Exiting VbrMoMgr::AdaptValToVtnService");
  return UPLL_RC_SUCCESS;
}


upll_rc_t VlinkMoMgr::UpdateMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutFlag
                          | kOpInOutDomain };
  if (NULL == ikey || NULL == req || !(ikey->get_key())) {
     UPLL_LOG_ERROR("Given Input is Empty");
     return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_ERROR("UpdateMo for %d", ikey->get_key_type());
  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Validation Message is Failed ");
      return result_code;
  }
  ConfigKeyVal *dup_ckvlink = NULL;
  result_code = GetChildConfigKey(dup_ckvlink, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(dup_ckvlink, req->datatype, UNC_OP_READ, dbop1,
                             dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
      if (dup_ckvlink) delete dup_ckvlink;
      UPLL_LOG_ERROR("Record does Not Exists");
      return result_code;
  }
#if 0
  result_code = DupConfigKeyVal(okey, ikey, MAINTBL);
   if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" DupConfigKeyVal Failed %d", result_code);
    return result_code;
  }
#endif

  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS  != result_code) {
      if (dup_ckvlink) delete dup_ckvlink;
      UPLL_LOG_ERROR("Validate Attribute is Failed");
      return result_code;
  }
  val_vlink_t *tmp_val = reinterpret_cast<val_vlink *>(GetVal(ikey));
  if (!tmp_val) {
    UPLL_LOG_DEBUG("Invalid val");
    if (dup_ckvlink) delete dup_ckvlink;
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t valid_boundary = tmp_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
  if (valid_boundary) {
    MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VBR_IF)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid Mgr");
      if (dup_ckvlink) delete dup_ckvlink;
      return UPLL_RC_ERR_GENERIC;
    }
    ConfigKeyVal *ck_vbrif[2] = {NULL, NULL};
    SET_USER_DATA_FLAGS(dup_ckvlink->get_cfg_val(), kVlinkVnode1);
    int i = 0;
    do {
      result_code = mgr->GetChildConfigKey(ck_vbrif[i], dup_ckvlink);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning %d", result_code);
        if (dup_ckvlink) delete dup_ckvlink;
        return result_code;
      }
      result_code = mgr->ReadConfigDB(ck_vbrif[i], req->datatype,
                                              UNC_OP_READ, dbop1, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in reading %d", result_code);
        if (dup_ckvlink) delete dup_ckvlink;
        if (ck_vbrif[0]) delete ck_vbrif[0];
        if (ck_vbrif[1]) delete ck_vbrif[1];
        ck_vbrif[0] = ck_vbrif[1] = NULL;
        return result_code;
      }
      SET_USER_DATA_FLAGS(dup_ckvlink->get_cfg_val(), kVlinkVnode2);
      i++;
    } while (i < 2);
    result_code = UpdateVbrIfExternal(req->datatype, ikey, ck_vbrif, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
    }
    for (int i = 0; i < 2 ; i++)
      if (ck_vbrif[i]) delete ck_vbrif[i];
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  UPLL_LOG_DEBUG("The ikey Structue before update  %s", (ikey->ToStrAll()).c_str());
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE,
                               dmi, &dbop, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (dup_ckvlink) delete dup_ckvlink;
    UPLL_LOG_ERROR("Updation Failure in DB : %d", result_code);
    return result_code;
  }
  if (dup_ckvlink) delete dup_ckvlink;
  return result_code;
}

/* This function update the vnode operstatus 
 * while doing commit
 */
upll_rc_t VlinkMoMgr::TxUpdateDtState(unc_key_type_t ktype,
                                      uint32_t session_id,
                                      uint32_t config_id,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vlink = NULL;
 
  result_code = GetUninitOperState<val_vlink_st_t,val_db_vlink_st_t>
                                   (ck_vlink, dmi); 
  if (!ck_vlink || UPLL_RC_SUCCESS != result_code) {
     return result_code;
  }
  ConfigKeyVal *tkey = ck_vlink;
  // Get consitutent interface status - ignore if unknown
  while (tkey) {
    uint8_t oper_status[2]; 
    ConfigKeyVal *ck_vnif[2];
    result_code = GetVnodeIfFromVlink(tkey, ck_vnif, dmi); 
    if (!ck_vnif[0] || !ck_vnif[1] || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      return result_code;
    }
    for (int i = 0; i < 2; i++) {
      oper_status[i] = UPLL_OPER_STATUS_DOWN;
      switch (ck_vnif[i]->get_key_type()) {
      case UNC_KT_VBR_IF: 
      {
        val_db_vbr_if_st *vnif_st = reinterpret_cast<val_db_vbr_if_st *>
                                        (GetStateVal(ck_vnif[i]));
        if (vnif_st && vnif_st->vbr_if_val_st.
               valid[UPLL_IDX_OPER_STATUS_VBRIS] == UNC_VF_VALID) {
          oper_status[i] = vnif_st->vbr_if_val_st.oper_status;
        } 
        break;
      }
      case UNC_KT_VRT_IF: 
      {
        val_db_vrt_if_st *vnif_st = reinterpret_cast<val_db_vrt_if_st *>
                                        (GetStateVal(ck_vnif[i]));
        if (vnif_st && vnif_st->vrt_if_val_st.
              valid[UPLL_IDX_OPER_STATUS_VRTIS] == UNC_VF_VALID) {
          oper_status[i] = vnif_st->vrt_if_val_st.oper_status;
        } 
        break;
      }
      case UNC_KT_VTEP_IF: 
      {
        val_db_vtep_if_st *vnif_st = reinterpret_cast<val_db_vtep_if_st *>
                                        (GetStateVal(ck_vnif[i]));
        if (vnif_st && vnif_st->vtep_if_val_st.
                  valid[UPLL_IDX_IF_OPER_STATUS_VTEPIS] == UNC_VF_VALID) {
          oper_status[i] = vnif_st->vtep_if_val_st.oper_status;
        } 
        break;
      }
      case UNC_KT_VTUNNEL_IF: 
      {
        val_db_vtunnel_if_st *vnif_st = reinterpret_cast<val_db_vtunnel_if_st *>
                                        (GetStateVal(ck_vnif[i]));
        if (vnif_st && vnif_st->vtunnel_if_val_st.
                     valid[UPLL_IDX_IF_OPER_STATUS_VTNLI] == UNC_VF_VALID) {
          oper_status[i] = vnif_st->vtunnel_if_val_st.oper_status;
        } 
        break;
      }
      case UNC_KT_VUNK_IF:
        oper_status[i] = UPLL_OPER_STATUS_UP;
        continue;
      default:
        UPLL_LOG_DEBUG("Unsupported operation on key %d\n",
                                         ck_vnif[i]->get_key_type()); 
        return UPLL_RC_ERR_GENERIC;
      }
      if (ck_vnif[i]) delete ck_vnif[i];
      ck_vnif[i] = NULL;
    }
    val_db_vlink_st *val_vlink_st = reinterpret_cast<val_db_vlink_st *>
                                    (GetStateVal(tkey));
    if (!val_vlink_st) {
      UPLL_LOG_DEBUG("Invalid vlink st\n");
      return UPLL_RC_ERR_GENERIC;
    }
    if ((oper_status[0] == oper_status[1]) == UPLL_OPER_STATUS_UP)
      val_vlink_st->vlink_val_st.oper_status = UPLL_OPER_STATUS_UP;
    else if ((oper_status[0] == UPLL_OPER_STATUS_UNKNOWN) ||
             (oper_status[1] == UPLL_OPER_STATUS_UNKNOWN))
        val_vlink_st->vlink_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
    else
        val_vlink_st->vlink_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
    val_vlink_st->vlink_val_st.valid[UPLL_IDX_OPER_STATUS_VLNKS] =
                                                             UNC_VF_VALID;
    tkey= tkey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::GetVnodeIfFromVlink(ConfigKeyVal *vlink, 
                                          ConfigKeyVal **vnif,
                                          DalDmlIntf   *dmi) {
  if (!vlink) {
    UPLL_LOG_TRACE("Input vlink key null\n");
    return UPLL_RC_ERR_GENERIC;
  }
  VlinkNodePosition vnode_number = kVlinkVnode1;
  int i = 0;
  uint8_t rename_flag = 0;
  GET_USER_DATA_FLAGS(vlink->get_cfg_val(), rename_flag);
  do {
    unc_key_type_t ktype = GetVlinkVnodeIfKeyType(vlink, i);
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                      (GetMoManager(ktype)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr for keytype %d\n",ktype);
      return UPLL_RC_ERR_GENERIC;
    }
    SET_USER_DATA_FLAGS(vlink->get_cfg_val(), vnode_number);
    mgr->GetChildConfigKey(vnif[i], vlink);
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCs };
    upll_rc_t result_code = mgr->ReadConfigDB(vnif[i], UPLL_DT_STATE, 
                              UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    i++;
    vnode_number = (vnode_number == kVlinkVnode1)?kVlinkVnode2:kVlinkVnode1;
  } while (vnode_number != kVlinkVnode1);
  SET_USER_DATA_FLAGS(vlink->get_cfg_val(), rename_flag);
  if (!GetVal(vnif[0]) ||!GetVal(vnif[1])) {
    UPLL_LOG_DEBUG("Returning error\n");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc

