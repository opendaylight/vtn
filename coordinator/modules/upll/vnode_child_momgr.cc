/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vnode_child_momgr.hh"
#include "vnode_momgr.hh"
#include "uncxx/upll_log.hh"
#include "vbr_if_momgr.hh"
#include "vtn_momgr.hh"
#include "vlink_momgr.hh"
#include "config_mgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

upll_rc_t VnodeChildMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal *pckv = NULL;
  uint8_t *controller_id = reinterpret_cast<uint8_t *>(
                                 const_cast<char *>(ctrlr_id));

  /* check if object is renamed in the corresponding Rename Tbl
   * if "renamed"  create the object by the UNC name.
   * else - create using the controller name.
   */
  result_code = GetRenamedUncKey(ikey, UPLL_DT_RUNNING, dmi, controller_id);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("GetRenamedUncKey Failed err_code %d", result_code);
    return result_code;
  }
  if ((ikey->get_key_type() == UNC_KT_VTEP_GRP) ||
      (ikey->get_key_type() == UNC_KT_VTEP_GRP_MEMBER)) {
    result_code = GetControllerDomainId(ikey, UPLL_DT_AUDIT,
                                       &ctrlr_dom, dmi);
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  } else {
    result_code = GetParentConfigKey(pckv, ikey);
    if (result_code != UPLL_RC_SUCCESS && pckv == NULL) {
      UPLL_LOG_DEBUG("GetParentConfigKey failed err_code is %d",
                      result_code);
      return result_code;
    }
    result_code = GetControllerDomainId(pckv, UPLL_DT_AUDIT,
                                       &ctrlr_dom, dmi);
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(pckv, ctrlr_dom);
    SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    DELETE_IF_NOT_NULL(pckv);
  }
 
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    UPLL_LOG_INFO("GetControllerDomainId failed err_code %d", result_code);
    DELETE_IF_NOT_NULL(pckv);
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutDomain
                       | kOpInOutCtrlr };
  // SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE,
                               dmi, &dbop, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("UpdateConfigDB failed for creating vnode child err_code %d",
                   result_code);
  }
  DELETE_IF_NOT_NULL(pckv);
  return result_code;
}

upll_rc_t VnodeChildMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }

  // Parent check
  parent_ck_vnode = NULL;
  result_code = GetParentConfigKey(parent_ck_vnode, ikey);
  if (result_code != UPLL_RC_SUCCESS || parent_ck_vnode == NULL) {
    return result_code;
  }

  unc_key_type_t key_type = parent_ck_vnode->get_key_type();
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                        (GetMoManager(key_type)));
  result_code = mgr->UpdateConfigDB(parent_ck_vnode, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE
      || result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_INFO("Parent doesn't exist in CANDIDATE DB. Error code : %d",
                  result_code);
      DELETE_IF_NOT_NULL(parent_ck_vnode);
      return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }else {
      UPLL_LOG_DEBUG("UpdateConfigDB Failed %d", result_code);
      DELETE_IF_NOT_NULL(parent_ck_vnode);
      return result_code;
    }
  }
  if ((ikey->get_key_type() == UNC_KT_VTEP_GRP) ||
      (ikey->get_key_type() == UNC_KT_VTEP_GRP_MEMBER)) {
    result_code = GetControllerDomainId(ikey, req->datatype,
                                      &cntrl_dom, dmi);
    cntrl_dom.ctrlr = NULL;
    if (ikey->get_key_type() != UNC_KT_VTEP_GRP)
      cntrl_dom.domain = NULL;

    GET_USER_DATA_CTRLR_DOMAIN(ikey, cntrl_dom);
  } else {
    result_code = GetControllerDomainId(parent_ck_vnode, req->datatype,
                                        &cntrl_dom, dmi);
    cntrl_dom.ctrlr = NULL;
    cntrl_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(parent_ck_vnode, cntrl_dom);
    SET_USER_DATA_CTRLR_DOMAIN(ikey, cntrl_dom);
  }
  if ((result_code != UPLL_RC_SUCCESS) || (cntrl_dom.ctrlr == NULL)
      || (cntrl_dom.domain == NULL)) {
    UPLL_LOG_INFO("Illegal params");
    DELETE_IF_NOT_NULL(parent_ck_vnode);
    return result_code;
  }
  ConfigKeyVal *dup_ikey = NULL;
  if (UPLL_DT_IMPORT != req->datatype) {
      void *ifval = GetVal(ikey);
      if ((ikey->get_key_type() == UNC_KT_VBR_IF) && ifval) {
         result_code = GetChildConfigKey(dup_ikey, ikey);
         if (!dup_ikey || result_code != UPLL_RC_SUCCESS) {
           DELETE_IF_NOT_NULL(parent_ck_vnode);
           UPLL_LOG_DEBUG("Returning error %d", result_code);
           return result_code;
         }
         val_drv_vbr_if *valif = reinterpret_cast<val_drv_vbr_if *>
           (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
         memcpy(&(valif->vbr_if_val), ifval, sizeof(val_vbr_if));
         dup_ikey->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrIf, valif);
      } else {
      dup_ikey = ikey;
      }
    } else {
      dup_ikey = ikey;
    }
    UPLL_LOG_DEBUG("%s \n",dup_ikey->ToStrAll().c_str());

  // Existence check in DB
  result_code = UpdateConfigDB(dup_ikey, req->datatype, UNC_OP_READ, dmi, MAINTBL);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS
      || result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(parent_ck_vnode);
    if (UPLL_DT_IMPORT != req->datatype) {
        void *ifval = GetVal(ikey);
      if ((ikey->get_key_type() == UNC_KT_VBR_IF) && ifval) {
        DELETE_IF_NOT_NULL(dup_ikey);
      }    
    }
    return result_code;
  }
  ConfigKeyVal *inst_key = NULL;
  result_code = GetChildConfigKey(inst_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(parent_ck_vnode);
    DELETE_IF_NOT_NULL(dup_ikey);
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }
  result_code = GetInstanceCount(inst_key, reinterpret_cast<char*>
           (cntrl_dom.ctrlr), req->datatype, &cur_instance_count, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("GetInstanceCount error %d", result_code);
      DELETE_IF_NOT_NULL(dup_ikey);
      DELETE_IF_NOT_NULL(parent_ck_vnode);
      DELETE_IF_NOT_NULL(inst_key);
      return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(inst_key);
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("ValidateCapability failed");
    DELETE_IF_NOT_NULL(parent_ck_vnode);
    if (UPLL_DT_IMPORT != req->datatype) {
        void *ifval = GetVal(ikey);
      if ((ikey->get_key_type() == UNC_KT_VBR_IF) && ifval) {
        DELETE_IF_NOT_NULL(dup_ikey);
      }    
    }
    return result_code;
  }
  SET_USER_DATA_CTRLR_DOMAIN(dup_ikey, cntrl_dom);
  result_code = RestoreVnode(dup_ikey, req, dmi);
  DELETE_IF_NOT_NULL(parent_ck_vnode);
  
  if (UPLL_DT_IMPORT != req->datatype) {
      void *ifval = GetVal(ikey);
    if ((ikey->get_key_type() == UNC_KT_VBR_IF) && ifval) {
      DELETE_IF_NOT_NULL(dup_ikey);
    }    
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::RestoreVnode(ConfigKeyVal *ikey,
                                        IpcReqRespHeader *req,
                                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *dup_ikey = NULL;
  upll_keytype_datatype_t dt_type = req->datatype;

  if (UPLL_DT_CANDIDATE == req->datatype) {
    result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
                                 MAINTBL);
    if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      result_code = RestoreChildren(ikey, dt_type, UPLL_DT_RUNNING, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("Returning error %d", result_code);
        return result_code;
      }
      return result_code;
    }
  }
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE
            || UPLL_DT_IMPORT == req->datatype) {
    UPLL_LOG_TRACE("Given Create Record doesn't exists in Running");
    result_code = DupConfigKeyVal(dup_ikey, ikey);
    if (result_code != UPLL_RC_SUCCESS || dup_ikey == NULL) {
      UPLL_LOG_DEBUG("Returning %d", result_code);
      return result_code;
    }
    result_code = ValidateAttribute(dup_ikey, dmi, req);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("ValidateAttribute semantic check returns error %d",
                    result_code);
      return result_code;
    }
  } else {
//    std::cout << "Problem in reading RUNNING DB";
    return result_code;
  }
  unc_key_type_t ktype = parent_ck_vnode->get_key_type();
  if (UPLL_DT_CANDIDATE == req->datatype) {
    if (ktype == UNC_KT_VBRIDGE || ktype == UNC_KT_VROUTER) {
      result_code = SetRenameField(dup_ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Problem in setting rename field");
        return result_code;
      }
    }

    if (dup_ikey->get_key_type() == UNC_KT_VBR_IF) {
      val_drv_vbr_if_t *if_val = reinterpret_cast<val_drv_vbr_if *>
                                                 (GetVal(dup_ikey));
      if (if_val != NULL &&
        if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
        ConverttoDriverPortMap(dup_ikey);
      }
    }
  }
  result_code = UpdateConfigDB(dup_ikey, dt_type, UNC_OP_CREATE, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to create an entry in Candidate DB");
  }
  if (dup_ikey) delete dup_ikey;
  return result_code;
}


upll_rc_t VnodeChildMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t dt_type,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;

  switch (ikey->get_key_type()) {
  case UNC_KT_VBR_IF:
  case UNC_KT_VRT_IF:
  case UNC_KT_VTEP_IF:
  case UNC_KT_VTUNNEL_IF:
  case UNC_KT_VUNK_IF:
    break;
  default:
    return UPLL_RC_SUCCESS;
  }
  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS || okey == NULL) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop,
                dmi, MAINTBL);
  while (okey) {
    uint8_t if_flag = 0;
    GET_USER_DATA_FLAGS(okey, if_flag);
    if_flag &= VIF_TYPE;
    if (if_flag) {
       UPLL_LOG_DEBUG("Part of a vlink %d", if_flag);
       result_code = UPLL_RC_ERR_CFG_SEMANTIC;
       break;
    }
    okey = okey->get_next_cfg_key_val();
  }
  delete okey;
  return result_code;
}

upll_rc_t VnodeChildMoMgr::SetRenameField(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t rename = 0;
  GET_USER_DATA_FLAGS(parent_ck_vnode, rename);
  rename &= RENAME;
  SET_USER_DATA_FLAGS(ikey, rename);

  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
                                            upll_keytype_datatype_t dt_type,
                                            DalDmlIntf *dmi,
                                            uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t ktype = ikey->get_key_type();
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_id;
  ctrlr_dom.domain = NULL;
  if (!OVERLAY_KT(ktype))
       result_code = GetRenamedKey(ikey, dt_type, dmi,
                                   &ctrlr_dom,
                                   UNC_RENAME_KEY);
  else
     result_code = UPLL_RC_SUCCESS;

  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t ktype = ikey->get_key_type();
  if (!OVERLAY_KT(ktype)) {
    result_code = GetRenamedKey(ikey, dt_type, dmi,
                                ctrlr_dom, CTRLR_RENAME_KEY);
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom->ctrlr,
                 ctrlr_dom->domain);
  } else {
    result_code = UPLL_RC_SUCCESS;
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetRenamedKey(ConfigKeyVal *ikey,
                                         upll_keytype_datatype_t dt_type,
                                         DalDmlIntf *dmi,
                                         controller_domain *ctrlr_dom,
                                         rename_key flag) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_parent = NULL;
  if (!ikey) return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = GetParentConfigKey(ck_parent, ikey);
  if (!ck_parent || (result_code != UPLL_RC_SUCCESS)) return result_code;
  unc_key_type_t ktype = ck_parent->get_key_type();
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                  (GetMoManager(ktype)));
  if (flag == UNC_RENAME_KEY) {
    result_code = mgr->GetRenamedUncKey(ck_parent, dt_type, dmi,
                                         ctrlr_dom->ctrlr);
    if (result_code != UPLL_RC_SUCCESS &&
         result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("GetRenamedUncKey failed. Result : %d", result_code);
      DELETE_IF_NOT_NULL(ck_parent);
      return result_code;
    }
  } else {
       result_code = mgr->GetRenamedControllerKey(ck_parent, dt_type, dmi,
                                                  ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed. Result: %d", result_code);
      DELETE_IF_NOT_NULL(ck_parent);
      return result_code;
    }
  }
  if (result_code == UPLL_RC_SUCCESS) {
    result_code  = GetChildConfigKey(ikey, ck_parent);
    if (UPLL_RC_SUCCESS != result_code) {
       DELETE_IF_NOT_NULL (ck_parent);
       UPLL_LOG_DEBUG("GetChildConfigkey Failed");
       return result_code;
    }
    SET_USER_DATA(ikey, ck_parent)
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  }
  delete ck_parent;
  return result_code;
}

upll_rc_t VnodeChildMoMgr::ReadMo(IpcReqRespHeader *header, ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t ktype = ikey->get_key_type();
  switch (ktype) {
  case UNC_KT_VBR_IF:
  case UNC_KT_VRT_IF:
  case UNC_KT_VUNK_IF:
  case UNC_KT_VTEP_IF:
  case UNC_KT_VTUNNEL_IF:
    UPLL_LOG_DEBUG("read request for %d, option 1 %d option2 %d", ktype,
                     header->option1, header->option2);
    if (header->option2 == UNC_OPT2_NEIGHBOR) {
      result_code = ValidateMessage(header, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ValidateMessage failed result_code %d", result_code);
        return result_code;
      }
      result_code = PopulateValVtnNeighbor(ikey, dmi);
      break;
    }
    /* fall through intended */
  default:
    result_code = MoMgrImpl::ReadMo(header, ikey, dmi);
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetControllerDomainId(
    ConfigKeyVal *pckv, upll_keytype_datatype_t dt_type,
    controller_domain *ctrlr_dom, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag | kOpInOutCtrlr
                       | kOpInOutDomain };
  if (!pckv) return UPLL_RC_ERR_GENERIC;
  unc_key_type_t pktype = pckv->get_key_type();
  VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager*>
                                                  (GetMoManager(pktype)));
  result_code = mgr->ReadConfigDB(pckv, dt_type, UNC_OP_READ, dbop, dmi,
                                  MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ReadConfigDB failed for pckv err_code %d", result_code);
    return result_code;
  }
  return (mgr->GetControllerDomainId(pckv, ctrlr_dom));
}


upll_rc_t VnodeChildMoMgr::PopulateValVtnNeighbor(ConfigKeyVal *&in_ckv,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  // Read on the left side of the link
  bool got_left_side = false;
  ConfigKeyVal *vlink_ckv = NULL;
  VlinkMoMgr *vlink_momgr = reinterpret_cast<VlinkMoMgr *>
                           (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
  if (!vlink_momgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  vn_if_type iftype;
  upll_rc_t result_code = vlink_momgr->CheckIfMemberOfVlink(in_ckv,
                               UPLL_DT_RUNNING, vlink_ckv, dmi, iftype);
  if (!vlink_ckv || result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_TRACE("Not found in vlink table");
  } else if (result_code == UPLL_RC_SUCCESS) {
    key_vlink_t *vlink_key = reinterpret_cast<key_vlink_t *>
                                             (vlink_ckv->get_key());
    if (!vlink_key) {
      UPLL_LOG_DEBUG("Invalid param");
      if (vlink_ckv) delete vlink_ckv;
      return UPLL_RC_ERR_GENERIC;
    }
    if ((iftype == kVlinkBoundaryNode1) || (iftype == kVlinkInternalNode1))
     got_left_side = true;
    val_vlink_t *vlink_val = reinterpret_cast<val_vlink *>
                       (GetVal(vlink_ckv));
    val_vtn_neighbor_t *val_vtn_neighbor =
     reinterpret_cast<val_vtn_neighbor_t *>
      (ConfigKeyVal::Malloc(sizeof(val_vtn_neighbor_t)));
    memset(val_vtn_neighbor, 0, sizeof(val_vtn_neighbor_t));
    val_vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] = UNC_VF_VALID;
    val_vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN] = UNC_VF_VALID;
    val_vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] = UNC_VF_VALID;
    uuu::upll_strncpy(val_vtn_neighbor->connected_vnode_name,
                      ((got_left_side) ? vlink_val->vnode2_name :
                                         vlink_val->vnode1_name),
                      (kMaxLenVnodeName + 1));
    uuu::upll_strncpy(val_vtn_neighbor->connected_if_name,
                      ((got_left_side) ? vlink_val->vnode2_ifname :
                                         vlink_val->vnode1_ifname),
                      (kMaxLenInterfaceName + 1));
    uuu::upll_strncpy(val_vtn_neighbor->connected_vlink_name,
                      vlink_key->vlink_name, (kMaxLenVnodeName + 1));
    in_ckv->SetCfgVal(new ConfigVal(IpctSt::kIpcStValVtnNeighbor,
                                    val_vtn_neighbor));
  } else {
    UPLL_LOG_DEBUG("ReadConfigDB failed result_code - %d", result_code);
  }

  if (vlink_ckv) delete vlink_ckv;
  return result_code;
}

template<typename T1, typename T2>
upll_rc_t VnodeChildMoMgr::InitOperStatus(ConfigKeyVal *ikey,
                                          uint8_t valid_admin,
                                          uint8_t admin_status,
                                          uint8_t valid_pm,
                                          val_port_map_t *pm) {
  UPLL_FUNC_TRACE;
  T2 *vnif_db_st = reinterpret_cast<T2 *>(GetStateVal(ikey));
  if (!vnif_db_st) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  T1 *vnif_st = reinterpret_cast<T1 *>(vnif_db_st);
  // vnif_db_st->down_count = 0;
#if 0
  if ((valid_admin == UNC_VF_VALID) ||
     (valid_admin == UNC_VF_VALID_NO_VALUE)) 
#endif
    if (vnif_st->oper_status != UPLL_OPER_STATUS_UNINIT) {
      if (admin_status == UPLL_ADMIN_DISABLE) {
      vnif_st->oper_status = UPLL_OPER_STATUS_DOWN;
      } else if (admin_status == UPLL_ADMIN_ENABLE) {
        if_type vnif_type = kUnboundInterface;
        GetInterfaceType(ikey, valid_pm, vnif_type);
        UPLL_LOG_DEBUG("ktype %d vnif_type %d valid_pm %d\n", 
                    ikey->get_key_type(),vnif_type, valid_pm);
        switch (vnif_type) {
         case kBoundaryInterface:
         case kMappedInterface:
           if (valid_pm != UNC_VF_VALID)
              vnif_st->oper_status = UPLL_OPER_STATUS_DOWN;
           else
              vnif_st->oper_status = UPLL_OPER_STATUS_UNINIT;
           break;
         case kLinkedInterface:
           vnif_st->oper_status = UPLL_OPER_STATUS_UP;
           break;
         case kUnboundInterface:
           vnif_st->oper_status = UPLL_OPER_STATUS_DOWN;
           break;
        }
      }  else {
        vnif_st->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      }
  }
  vnif_st->valid[UPLL_IDX_OPER_STATUS_VRTS] = UNC_VF_VALID;
  return UPLL_RC_SUCCESS;
}

template upll_rc_t VnodeChildMoMgr::InitOperStatus<val_vrt_if_st,
                                          val_db_vrt_if_st>(
                                          ConfigKeyVal *ikey,
                                          uint8_t valid_admin,
                                          uint8_t admin_status,
                                          uint8_t valid_pm,
                                          val_port_map_t *pm);
template upll_rc_t VnodeChildMoMgr::InitOperStatus<val_vtep_if_st,
                                          val_db_vtep_if_st>(
                                          ConfigKeyVal *ikey,
                                          uint8_t valid_admin,
                                          uint8_t admin_status,
                                          uint8_t valid_pm,
                                          val_port_map_t *pm);
template upll_rc_t VnodeChildMoMgr::InitOperStatus<val_vtunnel_if_st,
                                          val_db_vtunnel_if_st>(
                                          ConfigKeyVal *ikey,
                                          uint8_t valid_admin,
                                          uint8_t admin_status,
                                          uint8_t valid_pm,
                                          val_port_map_t *pm);
template upll_rc_t VnodeChildMoMgr::InitOperStatus<val_vbr_if_st,
                                          val_db_vbr_if_st>(
                                          ConfigKeyVal *ikey,
                                          uint8_t valid_admin,
                                          uint8_t admin_status,
                                          uint8_t valid_pm,
                                          val_port_map_t *pm);

upll_rc_t VnodeChildMoMgr::GetInterfaceType(ConfigKeyVal *ck_vnif,
                                            uint8_t valid_pm,
                                            if_type &vnif_type ) {
  UPLL_FUNC_TRACE;
  if (!ck_vnif) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t flags = 0;
  GET_USER_DATA_FLAGS(ck_vnif, flags);
  flags = flags & VIF_TYPE;
  if (flags & VIF_TYPE_BOUNDARY) {
    vnif_type = kBoundaryInterface;
  } else if (flags & VIF_TYPE_LINKED) {
    vnif_type = kLinkedInterface;
  } else if (valid_pm == UNC_VF_VALID) {
    vnif_type = kMappedInterface;
  } else {
    vnif_type = kUnboundInterface;
  }
  UPLL_LOG_DEBUG("flags %d vnif_type %d", flags, vnif_type);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VnodeChildMoMgr::UpdateParentOperStatus(ConfigKeyVal *ikey, 
                                                  DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_ERROR("Returning error \n");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ktype = ikey->get_key_type();
  if ((ktype != UNC_KT_VBR_IF) && (ktype != UNC_KT_VRT_IF)) 
    return UPLL_RC_SUCCESS;
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  // value not bound during delete - read to get down count / oper status
  result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ,
                                  dbop, dmi, MAINTBL); 
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n",result_code);
    return result_code;
  }
  val_db_vbr_if_st *valst = reinterpret_cast<val_db_vbr_if_st *>(GetStateVal(ikey)); 
  if (!valst) {
    UPLL_LOG_DEBUG("Returning error\n");
    return UPLL_RC_ERR_GENERIC;
  }
  if (valst->vbr_if_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] == UNC_VF_VALID) {
    if (valst->vbr_if_val_st.oper_status !=  UPLL_OPER_STATUS_DOWN)
      return UPLL_RC_SUCCESS; 
    ConfigKeyVal *ck_vn = NULL;
    result_code = GetParentConfigKey(ck_vn,ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      return result_code;
    }
    VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
                            (GetMoManager(ck_vn->get_key_type())));
    if (!mgr) {
      UPLL_LOG_DEBUG("Returning error \n");
      delete ck_vn;
      return UPLL_RC_ERR_GENERIC;
    }
#if 0
    result_code = mgr->ReadConfigDB(ck_vn, UPLL_DT_STATE, UNC_OP_READ,
                                    dbop, dmi, MAINTBL); 
    if (result_code != UPLL_RC_SUCCESS) {
      if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      } else
        result_code = UPLL_RC_SUCCESS;
      DELETE_IF_NOT_NULL(ck_vn);
      return result_code;
    }
    val_db_vbr_if_st *vn_valst = reinterpret_cast<val_db_vbr_if_st *>
                             GetStateVal(ck_vn);
    if (valst->down_count > 0) { 
      vn_valst->down_count--;
    }
    result_code = mgr->GetCkvUninit(ck_vn,NULL,dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      DELETE_IF_NOT_NULL(ck_vn);
      return result_code;
    }
    result_code = mgr->UpdateConfigDB(ck_vn, UPLL_DT_STATE, UNC_OP_UPDATE,
                                 dmi, MAINTBL);
#else
    state_notification notification = (valst->down_count > 0)?kPortFaultReset:kAdminStatusEnabled; 
    result_code = mgr->UpdateOperStatus(ck_vn, dmi, notification, false, true);
#endif
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                     UPLL_RC_SUCCESS:result_code;
    }
    DELETE_IF_NOT_NULL(ck_vn);
  }
  return result_code; 
}

upll_rc_t VnodeChildMoMgr::UpdateOperStatus(ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi,
                                       state_notification notification,
                                       bool skip, bool upd_if, bool upd_remif,
                                       bool save_to_db) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!skip) {
    DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr | kOpMatchDomain,
                     kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag };
    if (notification == kCtrlrDisconnect || notification == kCtrlrReconnect) {
      dbop.matchop = kOpMatchCtrlr;
      dbop.inoutop = kOpInOutFlag;
    }
    result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                               MAINTBL);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in reading: %d", result_code);
      return result_code;
    }
  }
  ConfigKeyVal *tkey = ikey;
  unc_key_type_t ktype  = ikey->get_key_type(),pktype = UNC_KT_ROOT;
  while (tkey != NULL) {
   /* upd_if is set to true ->if operstatus
    * is updated as part of vlink processing.
    */
    if (!upd_if) {
      if_type vnif_type ; 
      result_code = GetInterfaceType(tkey,UNC_VF_INVALID,vnif_type);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d\n",result_code);
        return result_code;
      }
      if ((vnif_type == kBoundaryInterface) ||
          (vnif_type == kLinkedInterface)) {
        tkey = tkey->get_next_cfg_key_val();
        continue;
      } 
    }
    alarm_status oper_status_change = ALARM_NOT_SET;
    switch (ktype) {
    case UNC_KT_VBR_IF:
      pktype = UNC_KT_VBRIDGE;
      result_code = SetOperStatus
         <val_vbr_if_st, val_db_vbr_if_st>(tkey, notification, dmi,
                                           oper_status_change);
      if (tkey) {
        val_drv_vbr_if *vbr_if =
           reinterpret_cast<val_drv_vbr_if *>(GetVal(tkey));
        if (vbr_if)
         vbr_if->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] =
           UNC_VF_INVALID;
      }
      break;
    case UNC_KT_VRT_IF:
      pktype = UNC_KT_VROUTER;
      result_code = SetOperStatus
         <val_vrt_if_st, val_db_vrt_if_st>(tkey, notification, dmi,
                                           oper_status_change);
      if (tkey) {
        val_vrt_if *vrt_if =
           reinterpret_cast<val_vrt_if *>(GetVal(tkey));
        if (vrt_if)
         vrt_if->valid[UPLL_IDX_ADMIN_ST_VI] = UNC_VF_INVALID;
      }
      break;
    case UNC_KT_VTEP_IF:
      pktype = UNC_KT_VTEP;
      result_code = SetOperStatus
         <val_vtep_if_st, val_db_vtep_if_st>(tkey, notification, dmi,
                                             oper_status_change);
      if (tkey) {
        val_vtep_if *vtep_if =
           reinterpret_cast<val_vtep_if *>(GetVal(tkey));
       if (vtep_if)
         vtep_if->valid[UPLL_IDX_ADMIN_ST_VTEPI] = UNC_VF_INVALID;
      }
      break;
    case UNC_KT_VTUNNEL_IF:
      pktype = UNC_KT_VTUNNEL;
      result_code = SetOperStatus
         <val_vtunnel_if_st, val_db_vtunnel_if_st>(tkey, notification, dmi,
                                                   oper_status_change);
      if (tkey) {
        val_vtunnel_if *vtunnel_if =
           reinterpret_cast<val_vtunnel_if *>(GetVal(tkey));
        if (vtunnel_if)
          vtunnel_if->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] = UNC_VF_INVALID;
      }
      break;
    default:
      UPLL_LOG_DEBUG("oper status attribute not supported");
      return UPLL_RC_SUCCESS;
    }
    if (oper_status_change != ALARM_NOT_SET) {
      if (!EnqueOperStatusNotification(tkey, oper_status_change)) {
        UPLL_LOG_DEBUG("Alarm Notification failed");
        return UPLL_RC_ERR_GENERIC;
      }
    }
    if (notification != kCtrlrDisconnect &&
        notification != kCtrlrReconnect) {
      ConfigKeyVal *ck_parent = NULL;
      result_code = GetParentConfigKey(ck_parent, tkey);
      if (!ck_parent || result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
      VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>
           (const_cast<MoManager *>(GetMoManager(ck_parent->get_key_type())));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        delete ck_parent;
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->UpdateOperStatus(ck_parent, dmi, notification, false, save_to_db);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("UpdateOperStatus failed for %d key_type",
                        ck_parent->get_key_type());
        delete ck_parent;
        return result_code;
      }
      if (save_to_db)
        delete ck_parent;
    }
    if (upd_if && upd_remif &&
        notification != kCtrlrDisconnect && 
        notification != kCtrlrReconnect && 
        notification != kCtrlrReconnectIfUp && 
        notification != kCtrlrReconnectIfDown && 
        notification != kBoundaryFault && 
        notification != kBoundaryFaultReset) { 
      uint8_t vlink_flag = 0;
      uint8_t remote_if = 0;
      ConfigKeyVal *ck_vlink = NULL;
      /* interface is a member of a vlink */
      GET_USER_DATA_FLAGS(ikey, vlink_flag);
      switch (vlink_flag & VIF_TYPE) {
      case kVlinkBoundaryNode1:
      case kVlinkInternalNode1:
        SET_USER_DATA_FLAGS(ikey, kVlinkVnode1);
        remote_if = 1;
        break;
      case kVlinkBoundaryNode2:
      case kVlinkInternalNode2:
        SET_USER_DATA_FLAGS(ikey, kVlinkVnode2);
        remote_if = 0;
        break;
      default:
        if (skip) {
          UPLL_LOG_DEBUG("Not member of vlink %s",(ikey->ToStr()).c_str());
          goto flush_db ;
        }
        tkey = tkey->get_next_cfg_key_val();
        continue;
      }
      VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>
                    (const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->GetVlinkKeyVal(ikey, UPLL_DT_STATE, ck_vlink, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        return result_code;
      }
      SET_USER_DATA_FLAGS(ikey, vlink_flag);
      result_code = mgr->SetOperStatus(ck_vlink, dmi, notification, true);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("SetOperStatus failed %d", result_code);
        return result_code;
      }
      /* set the status of the remote interface */
      ConfigKeyVal *vnif = NULL;
      result_code = mgr->GetVnodeIfFromVlink(ck_vlink,&vnif,dmi,remote_if);
      if (result_code != UPLL_RC_SUCCESS &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("get remote interface failed %d", result_code);
        return result_code;
      }
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
        return UPLL_RC_SUCCESS;

      VnodeChildMoMgr *rem_mgr = reinterpret_cast<VnodeChildMoMgr *>
                  (const_cast<MoManager *>(GetMoManager(vnif->get_key_type())));
      if (!rem_mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = rem_mgr->UpdateOperStatus(vnif, dmi, notification, true,
                                          true,false);
      if (vnif) delete vnif;
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error updating rem interface oper status %d", 
                        result_code);
        return result_code;
      }
      if (ck_vlink) delete ck_vlink;
    }
    if (skip) break;
    tkey = tkey->get_next_cfg_key_val();
  }
  if (pktype == UNC_KT_ROOT)
   return UPLL_RC_SUCCESS;
flush_db:
  if (!save_to_db) {
    map<string ,ConfigKeyVal *>::iterator it;
    VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>
             (const_cast<MoManager *>(GetMoManager(pktype)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr");
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_DEBUG("Flushing vnode to db %d",ktype);
    for (it = mgr->vnode_oper_map.begin(); it != mgr->vnode_oper_map.end(); it++) {
      DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
      ConfigKeyVal *tckv = it->second;
      UPLL_LOG_TRACE("%s %p",(it->first).c_str(),it->second);
      UPLL_LOG_DEBUG("nextvnode %p",tckv);
      result_code = mgr->UpdateConfigDB(tckv, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                                   &dbop, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in update oper status %d", result_code);
      }
       if (tckv) delete tckv;
    }
    mgr->vnode_oper_map.clear();
  }
  return result_code;
}

template<typename T1, typename T2>
upll_rc_t VnodeChildMoMgr::SetOperStatus(ConfigKeyVal *ikey,
                                  state_notification &notification,
                                  DalDmlIntf *dmi, alarm_status &oper_change) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  /* update corresponding interface operstatus */
  ConfigVal *tmp = (ikey->get_cfg_val()) ?
                    ikey->get_cfg_val()->get_next_cfg_val() : NULL;
  T2 *vn_valst = reinterpret_cast<T2 *>((tmp != NULL) ? tmp->get_val() : NULL);
  if (vn_valst == NULL) return UPLL_RC_ERR_GENERIC;
  T1 *vn_val = reinterpret_cast<T1 *>(vn_valst);
  
  UPLL_LOG_DEBUG("notification %d down_count %d ",notification, vn_valst->down_count);

    /* Update oper status based on notification */
  vn_val->valid[0] = UNC_VF_VALID;
  switch (notification) {
  case kCtrlrReconnect: {
      uint8_t valid_pm,valid_admin,admin_status;
      val_port_map_t *pm = NULL;
      GetPortMap(ikey, valid_pm, pm, valid_admin, admin_status);
      if_type vnif_type;
      result_code = GetInterfaceType(ikey, valid_pm, vnif_type);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        return result_code;
      }
      UPLL_LOG_DEBUG("vnif_type is %d",vnif_type);
      if (ikey->get_key_type() == UNC_KT_VBR_IF) {
        val_vbr_if *vbr_if_val = reinterpret_cast<val_vbr_if*>(GetVal(ikey));
        if (!vbr_if_val) {
          UPLL_LOG_DEBUG("val vlink is NULL");
          return UPLL_RC_ERR_GENERIC;
        }
        admin_status = vbr_if_val->admin_status;
      } else if (ikey->get_key_type() == UNC_KT_VRT_IF) {
        val_vrt_if *vrt_if_val = reinterpret_cast<val_vrt_if*>(GetVal(ikey));
        if (!vrt_if_val) {
          UPLL_LOG_DEBUG("val vlink is NULL");
          return UPLL_RC_ERR_GENERIC;
        }
        admin_status = vrt_if_val->admin_status;
      }
      if (vnif_type == kLinkedInterface) {
        if (admin_status == UPLL_ADMIN_DISABLE) {
          vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
          notification = kCtrlrReconnectIfDown;
        } else {
          vn_val->oper_status = UPLL_OPER_STATUS_UP;
          notification = kCtrlrReconnectIfUp;
        } 
        VlinkMoMgr *vlink_mgr =
           reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager *>
           (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK))));
        result_code = vlink_mgr->RestoreVlinkOperStatus(ikey, dmi,
                                                    notification, false);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("restoring vlink operstatus failed");
          return result_code;
        }
      } else if (vnif_type == kUnboundInterface) {
          vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
          notification = kCtrlrReconnectIfDown;
      } else {
        return UPLL_RC_SUCCESS;
      } 
    }
    break;
  case kAdminStatusDisabled:
    vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    break;
  case kAdminStatusEnabled:
    if (vn_valst->down_count == 0) {
      if_type vnif_type;
      uint8_t valid_pm,valid_admin,admin_status;
      val_port_map_t *pm = NULL;
      GetPortMap(ikey, valid_pm, pm, valid_admin, admin_status);
      upll_rc_t result_code = GetInterfaceType(ikey,valid_pm,vnif_type);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d",result_code);
        return result_code;
      }
      if ((vnif_type == kLinkedInterface) || (vnif_type == kMappedInterface) ||
          (vnif_type == kBoundaryInterface))
        vn_val->oper_status = UPLL_OPER_STATUS_UP;
      else if (vnif_type == kUnboundInterface)
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    } else 
      vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    break;
  case kCtrlrDisconnect:
    vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    vn_valst->down_count = 0;
    break;
  case kPortFault:
  case kPathFault:
  case kBoundaryFault:
    if (vn_valst->down_count < 1) 
      vn_valst->down_count = (vn_valst->down_count + 1);
    if (vn_valst->down_count == 1) {
      vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      // generate alarm
     oper_change = ALARM_OPER_DOWN;
    }
    break;
  case kPortFaultReset:
  case kPathFaultReset:
  case kBoundaryFaultReset:
    if (vn_valst->down_count > 0) {
      --vn_valst->down_count;
    }
    if (vn_valst->down_count == 0) {
        uint8_t valid_pm,valid_admin,admin_status;
        val_port_map_t *pm = NULL;
        GetPortMap(ikey, valid_pm, pm, valid_admin, admin_status);
        if (admin_status == UPLL_ADMIN_ENABLE) {
          vn_val->oper_status = UPLL_OPER_STATUS_UP;
         // reset alarm
          oper_change = ALARM_OPER_UP;
        } else 
          vn_val->oper_status = UPLL_OPER_STATUS_UP;
    } else 
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    break;
  default:
      UPLL_LOG_DEBUG("unsupported notification for operstatus update");
      return UPLL_RC_ERR_GENERIC;
    break;  
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE,
                           dmi, &dbop, MAINTBL);
  UPLL_LOG_TRACE("VnodeChild SetOperstatus for VTN after Update is \n %s",
                    ikey->ToStrAll().c_str());
  return result_code;
}


upll_rc_t VnodeChildMoMgr::TxUpdateDtState(unc_key_type_t ktype,
                                           uint32_t session_id,
                                           uint32_t config_id,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_vn = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
#if 0
  switch (ktype) {
  case UNC_KT_VRT_IF:
    result_code = GetUninitOperState<val_vrt_if_st_t, val_db_vrt_if_st_t>
                                    (ck_vn, dmi);
    break;
  case UNC_KT_VUNK_IF:
    UPLL_LOG_DEBUG("Returning success");
    return UPLL_RC_SUCCESS;
  case UNC_KT_VBR_IF:
    result_code = GetUninitOperState<val_vbr_if_st_t, val_db_vbr_if_st_t>
                                    (ck_vn, dmi);
    break;
  case UNC_KT_VTEP_IF:
    result_code = GetUninitOperState<val_vtep_if_st_t, val_db_vtep_if_st_t>
                                    (ck_vn, dmi);
    break;
  case UNC_KT_VTUNNEL_IF:
    result_code = GetUninitOperState<val_vtunnel_if_st_t,
                                    val_db_vtunnel_if_st_t>(ck_vn, dmi);
    break;
  default:
    UPLL_LOG_DEBUG("Oper status not supported");
    return UPLL_RC_ERR_GENERIC;
  }
#endif
  result_code = GetUninitOperState(ck_vn, dmi);
  if (!ck_vn || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateDtState failed %d", result_code);
    return result_code;
  }
  /* Read port status from physical */
  ConfigKeyVal *tkey;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  while (ck_vn) {
    tkey = ck_vn;
    ck_vn = ck_vn->get_next_cfg_key_val();
    tkey->set_next_cfg_key_val(NULL);
    state_notification notification = kCtrlrReconnect; //noop 
    uint8_t valid_pm,valid_admin,admin_status;
    val_port_map_t *pm = NULL;
    result_code = GetPortMap(tkey, valid_pm, pm,valid_admin,admin_status);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      return result_code;
    }
    UPLL_LOG_DEBUG("valid_pm %d valid_admin %d admin_status %d", 
                    valid_pm, valid_admin,admin_status);
    if (valid_admin == UNC_VF_VALID &&
        admin_status == UPLL_ADMIN_DISABLE) 
       notification = kAdminStatusDisabled;
    else if (valid_pm == UNC_VF_VALID) {
        val_oper_status port_oper_status;
        GET_USER_DATA_CTRLR_DOMAIN(tkey, ctrlr_dom);
        result_code = GetPortStatusFromPhysical(pm, ctrlr_dom, port_oper_status,
                                              session_id, config_id);
          /* During import Physical don't have the Port status
           * in the running db so its return no such instance
           * so here comment this check */
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error retrieving port status from physical %d %d",
                        result_code, port_oper_status);
        // return result_code;
        }
        val_db_vbr_if_st *vbrif_valst = reinterpret_cast<val_db_vbr_if_st *>
                                         (GetStateVal(tkey));
        switch (port_oper_status) {
        case  UPLL_OPER_STATUS_DOWN:
          if (vbrif_valst->down_count == 0)
            notification = kPortFault;
          else
            notification = kAdminStatusEnabled;
          break;
        case  UPLL_OPER_STATUS_UNKNOWN:
          notification = kCtrlrDisconnect;
          break;
        case  UPLL_OPER_STATUS_UP:
        {
          if (vbrif_valst->down_count > 0)
            notification = kPortFaultReset;
          else
            notification = kAdminStatusEnabled;
          break;
        }
        default:
          if (vbrif_valst->down_count == 0)
            notification = kPortFault;
          else
            notification = kAdminStatusEnabled;
          break;
        }
    } 
    if (notification == kAdminStatusEnabled || admin_status == UPLL_ADMIN_ENABLE) {
      if_type vnif_type;
      result_code = GetInterfaceType(tkey,valid_pm,vnif_type);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d",result_code);
        return result_code;
      }
      if (vnif_type == kBoundaryInterface) {
        ConfigKeyVal *ck_remif = NULL;
        VlinkMoMgr *vlink_mgr = reinterpret_cast<VlinkMoMgr *>
                      (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
        if (!vlink_mgr) {
          UPLL_LOG_ERROR("Invalid mgr\n");
          return UPLL_RC_ERR_GENERIC;
        }
        result_code = vlink_mgr->GetRemoteIf(tkey,ck_remif, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Returning error %d\n",result_code);
          DELETE_IF_NOT_NULL(ck_remif);
          return result_code;
        }   
        result_code = GetPortMap(ck_remif, valid_pm, pm,
                                valid_admin,admin_status);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d\n",result_code);
          DELETE_IF_NOT_NULL(ck_remif);
          return result_code;
        }
        if (valid_admin == UNC_VF_VALID && 
                            admin_status == UPLL_ADMIN_DISABLE) 
          notification = kAdminStatusDisabled;
        else
          notification = kAdminStatusEnabled;
        DELETE_IF_NOT_NULL(ck_remif);
      } else if (vnif_type == kLinkedInterface) {
        notification = kAdminStatusEnabled;
      } else if (vnif_type == kUnboundInterface) {
        notification = kAdminStatusDisabled;
      } 
    }
    // vlinked interfaces should be handled as part of vlink dt state update
    result_code = UpdateOperStatus(tkey, dmi, notification, true, true, true);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error updating oper status %d", result_code);
      return result_code;
    }
#if 0
      DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutNone };
      result_code = UpdateConfigDB(tkey, UPLL_DT_STATE, UNC_OP_UPDATE,
                                      dmi, &dbop1, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_DEBUG("UpdateConfigDB Executed %d", result_code);
           break;
      }
#endif
     delete tkey;
  }
  return result_code;
}


upll_rc_t VnodeChildMoMgr::GetPortStatusFromPhysical(val_port_map_t *pm,
                            controller_domain_t ctr_domain,
                            val_oper_status &logical_port_operStatus,
                            uint32_t session_id,
                            uint32_t config_id) {
  UPLL_FUNC_TRACE;
  IpcResponse ipc_resp;
  ConfigKeyVal *log_Port_CK = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_logical_port *phy_logPort_Key = static_cast<key_logical_port *>
                        (ConfigKeyVal::Malloc(sizeof(key_logical_port)));
  uuu::upll_strncpy(phy_logPort_Key->domain_key.ctr_key.controller_name,
                    reinterpret_cast<const char *>(ctr_domain.ctrlr),
                    (kMaxLenCtrlrId+1));
  uuu::upll_strncpy(phy_logPort_Key->domain_key.domain_name,
                    reinterpret_cast<const char *>(ctr_domain.domain),
                    (kMaxLenDomainId+1));
  uuu::upll_strncpy(phy_logPort_Key->port_id,
                    reinterpret_cast<const char *>(pm->logical_port_id),
                    (kMaxLenLogicalPortId+1));
  log_Port_CK = new ConfigKeyVal(UNC_KT_LOGICAL_PORT,
                                 IpctSt::kIpcStKeyLogicalPort,
                                 phy_logPort_Key, NULL);
  result_code = SendIpcReq(session_id, config_id, UNC_OP_READ,
                   UPLL_DT_STATE, log_Port_CK, NULL, &ipc_resp);
  if ((result_code != UPLL_RC_SUCCESS) || (!ipc_resp.ckv_data)) {
    delete log_Port_CK;
    log_Port_CK = NULL;
    logical_port_operStatus = UPLL_OPER_STATUS_DOWN;
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      UPLL_LOG_DEBUG("Invalid Logical Port Id");
//      return UPLL_RC_ERR_CFG_SEMANTIC;
        return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("Error in retrieving LogicalPortId data from Physical");
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    return UPLL_RC_ERR_GENERIC;
  }
  log_Port_CK->ResetWith(ipc_resp.ckv_data);
  DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
  val_logical_port_st *phy_logPort_St = static_cast<val_logical_port_st *>
                                         (GetVal(log_Port_CK));
  if (phy_logPort_St)
    switch (phy_logPort_St->oper_status) {
      case UPPL_CONTROLLER_OPER_DOWN:
        logical_port_operStatus = UPLL_OPER_STATUS_DOWN;
        break;
      case UPPL_CONTROLLER_OPER_UP:
        logical_port_operStatus = UPLL_OPER_STATUS_UP;
        break;
      case UPPL_CONTROLLER_OPER_WAITING_AUDIT:
      case UPPL_CONTROLLER_OPER_AUDITING:
        logical_port_operStatus = UPLL_OPER_STATUS_UNKNOWN;
        break;
      default:
        break;
  }
  if (log_Port_CK)
    delete log_Port_CK;
  return result_code;
}

bool VnodeChildMoMgr::EnqueOperStatusNotification(ConfigKeyVal *ikey,
                                                   bool oper_status_change) {
  UPLL_FUNC_TRACE;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_DEBUG("Inputs are Invalid");
    return false;
  }
  uuc::UpllConfigMgr *cfg_instance = uuc::UpllConfigMgr::GetUpllConfigMgr();
  unc_key_type_t ktype  = ikey->get_key_type();
  uint8_t *vtn_name = NULL;
  uint8_t *vnode_name = NULL;
  uint8_t *vif_name = NULL;

  switch (ktype) {
    case UNC_KT_VBR_IF:
      vtn_name = reinterpret_cast<key_vbr_if*>
                   (ikey->get_key())->vbr_key.vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vbr_if*>
                     (ikey->get_key())->vbr_key.vbridge_name;
      vif_name = (reinterpret_cast<key_vbr_if*>(ikey->get_key()))->if_name;
    break;
    case UNC_KT_VRT_IF:
      vtn_name = reinterpret_cast<key_vrt_if*>
                     (ikey->get_key())->vrt_key.vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vrt_if*>
                     (ikey->get_key())->vrt_key.vrouter_name;
      vif_name = (reinterpret_cast<key_vrt_if*>(ikey->get_key()))->if_name;
    break;
    case UNC_KT_VTUNNEL_IF:
      vtn_name = reinterpret_cast<key_vtunnel_if*>
                     (ikey->get_key())->vtunnel_key.vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vtunnel_if*>
                     (ikey->get_key())->vtunnel_key.vtunnel_name;
      vif_name = reinterpret_cast<key_vtunnel_if*>(ikey->get_key())->if_name;
    break;
    case UNC_KT_VTEP_IF:
      vtn_name = reinterpret_cast<key_vtep_if*>
                     (ikey->get_key())->vtep_key.vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vtep_if*>
                     (ikey->get_key())->vtep_key.vtep_name;
      vif_name = (reinterpret_cast<key_vtep_if*>(ikey->get_key()))->if_name;
    break;
    default:
       UPLL_LOG_DEBUG("Invalid KeyType");
  }
  return cfg_instance->SendOperStatusAlarm(reinterpret_cast<char*>(vtn_name),
                                           reinterpret_cast<char*>(vnode_name),
                                           reinterpret_cast<char*>(vif_name),
                                           oper_status_change);
}

upll_rc_t VnodeChildMoMgr::SetLinkedIfOperStatusforPathFault(
                        const key_vnode_type_t &vnode_key,
                        state_notification notification,
                        DalDmlIntf *dmi) {
  ConfigKeyVal *ck_vnode_if = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetMappedInterfaces(vnode_key, dmi, ck_vnode_if);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Operstatus updation failed");
    return result_code;
  }
  ConfigKeyVal *tmp_vnode_if = ck_vnode_if;
  while (tmp_vnode_if != NULL) {
    result_code = UpdateOperStatus(tmp_vnode_if, dmi, notification, true, false, false);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Operstatus updation failed");
      return result_code;
    }
    tmp_vnode_if = tmp_vnode_if->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ck_vnode_if); 
  return result_code;
}

/*This function gives the interfaces mapped to a particular vbridge */
upll_rc_t VnodeChildMoMgr::GetMappedInterfaces(const key_vnode_type_t &vnode_key_type,
                                          DalDmlIntf *dmi,
                                          ConfigKeyVal *&iokey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  GetChildConfigKey(iokey, NULL);
  key_vnode *vnode_key = reinterpret_cast<key_vnode*>(iokey->get_key());
  memcpy(vnode_key, &(vnode_key_type.vnode_key), sizeof(key_vnode));
  
  /* Get all the vbridges under the VTN */
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(iokey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  return result_code;
}

upll_rc_t VnodeChildMoMgr::UpdateVnodeIf(ConfigKeyVal *ck_if,
                           DalDmlIntf *dmi,
                           state_notification notification) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
                  kOpInOutFlag};
  result_code = ReadConfigDB(ck_if, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                           MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB failed with result_code %d",
                    result_code);
    return result_code;
  }
  ConfigKeyVal *tmp = ck_if;
  while (tmp != NULL) {
    result_code = UpdateOperStatus(tmp, dmi, notification, true, true, 
                                   true, true);
    if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("Error updating oper status %d", result_code);
       return result_code;
    }
    tmp = tmp->get_next_cfg_key_val();
  }
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
