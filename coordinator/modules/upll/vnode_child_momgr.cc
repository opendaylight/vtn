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
#include "upll_log.hh"
#include "vbr_if_momgr.hh"
#include "vtn_momgr.hh"
#include "vlink_momgr.hh"

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
  result_code = GetParentConfigKey(pckv, ikey);
  if (result_code != UPLL_RC_SUCCESS && pckv == NULL) {
    UPLL_LOG_DEBUG("GetParentConfigKey failed err_code is %d\n",
                    result_code);
    return result_code;
  }
  result_code = GetControllerDomainId(pckv, UPLL_DT_AUDIT,
                                      &ctrlr_dom, dmi);
  if ((result_code != UPLL_RC_SUCCESS) || (cntrl_dom.ctrlr == NULL)
      || (cntrl_dom.domain == NULL)) {
    UPLL_LOG_INFO("GetControllerDomainId failed err_code %d\n", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR_DOMAIN(ikey,ctrlr_dom);
  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("UpdateConfigDB failed for creating vnode child err_code %d\n",
                   result_code);
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n",result_code); 
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
    UPLL_LOG_INFO("Parent doesn't exist in CANDIDATE DB. Error code : %d",
                  result_code);
    return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  }

  result_code = GetControllerDomainId(parent_ck_vnode, req->datatype,
                                      &cntrl_dom, dmi);
  if ((result_code != UPLL_RC_SUCCESS) || (cntrl_dom.ctrlr == NULL)
      || (cntrl_dom.domain == NULL)) {
    UPLL_LOG_INFO("Illegal params\n");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR_DOMAIN(ikey,cntrl_dom);
  // check for key support on controller and max count
  if (UPLL_DT_CANDIDATE == req->datatype) {
    ConfigKeyVal *dup_ikey = NULL;
    result_code = GetChildConfigKey(dup_ikey,ikey);
    if (!dup_ikey || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      return result_code;
    }
    result_code = GetInstanceCount(dup_ikey, reinterpret_cast<char*>
           (cntrl_dom.ctrlr), req->datatype, &cur_instance_count, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("GetInstanceCount error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    if (dup_ikey) delete dup_ikey;
    dup_ikey = NULL;
  }
// TODO(vinoth): capa validation
#if 0
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    // cout << "ValidateCapability failed \n";
    return result_code;
  }
#endif
  // Existence check in CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi, MAINTBL);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS
      || result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    return result_code;
  }
  SET_USER_DATA_CTRLR_DOMAIN(ikey, cntrl_dom);
  result_code = RestoreVnode(ikey, req, dmi);
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
         UPLL_LOG_DEBUG("Returning error %d\n", result_code);
        return result_code;
      }
      return result_code;
    }
  }
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE 
            || UPLL_DT_IMPORT == req->datatype) {
    UPLL_LOG_TRACE("Given Create Record doesn't exists in Running \n");
    result_code = DupConfigKeyVal(dup_ikey, ikey);
    if (result_code != UPLL_RC_SUCCESS || dup_ikey == NULL) {
      UPLL_LOG_DEBUG("Returning %d\n", result_code);
      return result_code;
    }
    result_code = ValidateAttribute(dup_ikey, dmi, req);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("ValidateAttribute semantic check returns error %d\n",
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
  }
  if (dup_ikey->get_key_type() == UNC_KT_VBR_IF) {
    val_drv_vbr_if_t *if_val = reinterpret_cast<val_drv_vbr_if *>
                                               (GetVal(dup_ikey));
    if (if_val != NULL &&
        if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
      ConverttoDriverPortMap(dup_ikey);
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
    UPLL_LOG_DEBUG("Returning error %d\n",result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, 
                dmi, MAINTBL);
  while (okey) {
    uint8_t if_flag = 0;
    GET_USER_DATA_FLAGS(okey,if_flag);
    if_flag &= VIF_TYPE;
    if (if_flag) {
       UPLL_LOG_DEBUG("Part of a vlink %d\n",if_flag);
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
     result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;

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
    result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
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
  if (flag == UNC_RENAME_KEY)
       result_code = mgr->GetRenamedUncKey(ck_parent, dt_type, dmi,
                                           ctrlr_dom->ctrlr);
  else
       result_code = mgr->GetRenamedControllerKey(ck_parent, dt_type, dmi,
                                                  ctrlr_dom);
  if (result_code == UPLL_RC_SUCCESS) {
    SET_USER_DATA(ikey, ck_parent)
    GET_USER_DATA_CTRLR_DOMAIN(ikey,*ctrlr_dom);
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
    UPLL_LOG_DEBUG("read request for %d,option 1 %d option2 %d\n",ktype,
                     header->option1,header->option2);
    if (header->option2 == UNC_OPT2_NEIGHBOR) {
      result_code = ValidateMessage(header, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ValidateMessage failed result_code %d\n", result_code);
        return result_code;
      }
      result_code = PopulateValVtnNeighbor(ikey, dmi);
      break;
    }
    /* fall through intended */
  default:
    result_code = MoMgrImpl::ReadMo(header, ikey, dmi);
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
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
    UPLL_LOG_INFO("ReadConfigDB failed for pckv err_code %d\n", result_code);
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
    UPLL_LOG_DEBUG("Invalid Mgr\n");
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
      UPLL_LOG_DEBUG("Invalid param\n");
      if (vlink_ckv) delete vlink_ckv;
      return UPLL_RC_ERR_GENERIC;
    }
    if ((iftype == kVlinkBoundaryNode1) || (iftype == kVlinkInternalNode1))
     got_left_side = true;
    val_vlink_t *vlink_val = reinterpret_cast<val_vlink *>
                       (GetVal(vlink_ckv));
    val_vtn_neighbor_t *val_vtn_neighbor =
     reinterpret_cast<val_vtn_neighbor_t *>(malloc(sizeof(val_vtn_neighbor_t)));
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
    UPLL_LOG_DEBUG("Invalid param\n");
    return UPLL_RC_ERR_GENERIC;
  }
  T1 *vnif_st = reinterpret_cast<T1 *>(vnif_db_st);
  vnif_db_st->down_count = 0;
  if ((valid_admin == UNC_VF_VALID) ||
     (valid_admin == UNC_VF_VALID_NO_VALUE)) {
    if (admin_status == UPLL_ADMIN_DISABLE) {
      vnif_st->oper_status = UPLL_OPER_STATUS_DOWN;
    } else {
      if_type vnif_type = kUnboundInterface;
      GetInterfaceType(ikey, valid_pm, vnif_type);
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
    }
  } else {
      vnif_st->oper_status = UPLL_OPER_STATUS_UNKNOWN;
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
    UPLL_LOG_DEBUG("Invalid param\n");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t flags = 0;
  GET_USER_DATA_FLAGS(ck_vnif,flags);
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
  return UPLL_RC_SUCCESS;
}


upll_rc_t VnodeChildMoMgr::UpdateOperStatus(ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi,
                                       state_notification notification,
                                       bool skip) {
  upll_rc_t result_code;
  UPLL_FUNC_TRACE;
  if (!skip) {
    DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
    result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                               MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Error in reading: %d", result_code);
      return result_code;
    }
  }
  ConfigKeyVal *tkey = ikey;
  unc_key_type_t ktype  = ikey->get_key_type();
  while (tkey != NULL) {
    bool oper_status_change;
    if (ktype == UNC_KT_VBR_IF) {
      oper_status_change = SetOperStatus
         <val_vbr_if_st, val_db_vbr_if_st>(tkey, notification, dmi);
    } else if (ktype == UNC_KT_VRT_IF) {
      oper_status_change = SetOperStatus
         <val_vrt_if_st, val_db_vrt_if_st>(tkey, notification, dmi);
    } else if (ktype == UNC_KT_VTEP_IF) {
      oper_status_change = SetOperStatus
         <val_vtep_if_st, val_db_vtep_if_st>(tkey, notification, dmi);
    } else if (ktype == UNC_KT_VTUNNEL_IF) {
      oper_status_change = SetOperStatus
         <val_vtunnel_if_st, val_db_vtunnel_if_st>(tkey, notification, dmi);
    } else {
      UPLL_LOG_DEBUG("oper status attribute not supported\n");
      return UPLL_RC_SUCCESS;
    }
    if (oper_status_change) {
      EnqueOperStatusNotification(tkey);
    }
    ConfigKeyVal *ck_parent = NULL;
    result_code = GetParentConfigKey(ck_parent, tkey);
    if (!ck_parent || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>
         (const_cast<MoManager *>(GetMoManager(ck_parent->get_key_type())));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr\n");
      delete ck_parent;
      return UPLL_RC_ERR_GENERIC;
    }
    oper_status_change = mgr->UpdateOperStatus(ck_parent, dmi, notification);
    if (oper_status_change) {
      EnqueOperStatusNotification(tkey);
    }
    delete ck_parent;
    tkey = tkey->get_next_cfg_key_val();
  }
  return result_code;
}

template<typename T1, typename T2>
bool VnodeChildMoMgr::SetOperStatus(ConfigKeyVal *ikey,
                                    state_notification notification,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  bool result = false;
  /* update corresponding interface operstatus */
  ConfigVal *tmp = (ikey->get_cfg_val()) ?
                    ikey->get_cfg_val()->get_next_cfg_val() : NULL;
  T2 *vn_valst = reinterpret_cast<T2 *>((tmp != NULL) ? tmp->get_val() : NULL);
  if (vn_valst == NULL) return UPLL_RC_ERR_GENERIC;
  T1 *vn_val = reinterpret_cast<T1 *>(vn_valst);

    /* Update oper status based on notification */
  vn_val->valid[0] = UNC_VF_VALID;
  switch (notification) {
  case kCtrlrReconnect:
    return false;
  case kCtrlrDisconnect:
    vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    break;
  case kPortFault:
  case kPathFault:
  case kBoundaryFault:
    vn_valst->down_count = (vn_valst->down_count + 1);
    if (vn_valst->down_count == 1) {
      vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      // generate alarm
      result = true;
    }
    break;
  case kPortFaultReset:
  case kPathFaultReset:
  case kBoundaryFaultReset:
    vn_valst->down_count = (vn_valst->down_count > 0) ?
                            (vn_valst->down_count - 1) : 0;
    if (vn_valst->down_count == 0) {
      vn_val->oper_status = UPLL_OPER_STATUS_UP;
      // reset alarm
      result = true;
    }
    break;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  upll_rc_t result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE,
                           dmi, &dbop, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
  }
  return result;
}

#if 0
template<typename T1,typename T2>
upll_rc_t VnodeChildMoMgr::GetUninitOperState(ConfigKeyVal *&ck_vn,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *cval = NULL;
  /* Allocate Memory for vnode st */
  result_code = AllocVal(cval,UPLL_DT_STATE,MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d",result_code);
    return result_code;
  }
  /* initialize vnode st */
  T2 *vnif_st = reinterpret_cast<T2 *>
                               (cval->get_next_cfg_val()->get_val());
  if (!vnif_st) {
   delete cval;
   UPLL_LOG_DEBUG("Invalid param");
   return UPLL_RC_ERR_GENERIC;
  }
  T1 *vnif = reinterpret_cast<T1 *>(vnif_st);
  vnif->valid[UPLL_IDX_OPER_STATUS_VBRIS] = UNC_VF_VALID;
 //  vnif->oper_status = UPLL_OPER_STATUS_UNINIT;
  /* Create Vnode If child */
  result_code = GetChildConfigKey (ck_vn, NULL); 
  if (UPLL_RC_SUCCESS != result_code)  {
    free(vnif_st);
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  ck_vn->AppendCfgVal(cval); 

  /* Reading the Vnode Table and Check the Operstatus is unknown 
   * for any one of the vnode if */
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag |
                           kOpInOutCtrlr | kOpInOutDomain };
  result_code = ReadConfigDB(ck_vn, UPLL_DT_STATE, UNC_OP_READ,
                                  dbop,dmi,MAINTBL); 
  if (UPLL_RC_SUCCESS != result_code) {
     result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                 UPLL_RC_SUCCESS : result_code;
     UPLL_LOG_DEBUG("Returning %d",result_code);
     if (ck_vn) delete ck_vn;
  }
  return result_code;
}
#endif
                       
upll_rc_t VnodeChildMoMgr::TxUpdateDtState(unc_key_type_t ktype,
                                           uint32_t session_id,
                                           uint32_t config_id,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_vn = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  switch (ktype) {
  case UNC_KT_VRT_IF:
  case UNC_KT_VUNK_IF:
    UPLL_LOG_DEBUG("Returning success\n");
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
    result_code = GetUninitOperState<val_vtunnel_if_st_t, val_db_vtunnel_if_st_t>
                                    (ck_vn, dmi);
    break;
  default:
    UPLL_LOG_DEBUG("Oper status not supported\n");
    return UPLL_RC_ERR_GENERIC;
  }
  if (!ck_vn || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateDtState failed %d\n",result_code);
    return result_code;
  }
  /* Read port status from physical */
  ConfigKeyVal *tkey = ck_vn;
  controller_domain_t ctrlr_dom;
  while (tkey) {
    uint8_t valid_pm;
    val_port_map_t *pm=NULL;
    GetPortMap(tkey,valid_pm,pm);
    UPLL_LOG_DEBUG("valid_pm %d\n",valid_pm); 
    if (valid_pm == UNC_VF_VALID) {
      val_oper_status port_oper_status;
      GET_USER_DATA_CTRLR_DOMAIN(tkey,ctrlr_dom);
      result_code = GetPortStatusFromPhysical(pm, ctrlr_dom,port_oper_status,
                                              session_id,config_id);     
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error retrieving port status from physical %d\n",
                        result_code);
        return result_code;
      }
      result_code = UpdateOperStatus(tkey, dmi, kPortFault, true);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error updating oper status %d\n",result_code);
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
    }
    tkey= tkey->get_next_cfg_key_val();
  }
  if (ck_vn)
    delete ck_vn;
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
                        (malloc(sizeof(key_logical_port)));
  memset(phy_logPort_Key, 0, sizeof(key_logical_port));
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
  if ((result_code != UPLL_RC_SUCCESS) || (! ipc_resp.ckv_data)) {
    delete log_Port_CK;
    log_Port_CK = NULL;
    logical_port_operStatus = UPLL_OPER_STATUS_UNKNOWN;
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("Invalid Logical Port Id\n");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    UPLL_LOG_DEBUG("Error in retrieving LogicalPortId data from Physical\n");
    return UPLL_RC_ERR_GENERIC;
  }
  log_Port_CK->ResetWith(ipc_resp.ckv_data);
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
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
