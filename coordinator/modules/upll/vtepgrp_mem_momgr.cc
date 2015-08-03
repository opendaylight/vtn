/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vtepgrp_mem_momgr.hh"
namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VtepGrpMemMoMgr::vtepgrp_mem_bind_info[] = {
  { uudst::vtep_groupmember::kDbiVtnName, CFG_KEY, offsetof
    (key_vtep_grp_member, vtepgrp_key.vtn_key.vtn_name),
    uud::kDalChar, kMaxLenVtnName+1 },
  { uudst::vtep_groupmember::kDbiVtepgrpName, CFG_KEY, offsetof
    (key_vtep_grp_member, vtepgrp_key.vtepgrp_name),
    uud::kDalChar, kMaxLenVnodeName+1 },
  { uudst::vtep_groupmember::kDbiVtepgrpMemberName, CFG_KEY,
    offsetof(key_vtep_grp_member, vtepmember_name),
    uud::kDalChar, kMaxLenVnodeName+1 },
  { uudst::vtep_groupmember::kDbiCtrlrName,  CK_VAL, offsetof
    (key_user_data_t, ctrlr_id), uud::kDalChar, kMaxLenCtrlrId+1},
  { uudst::vtep_groupmember::kDbiDomainId,  CK_VAL, offsetof
    (key_user_data_t, domain_id), uud::kDalChar, kMaxLenDomainId+1},
  { uudst::vtep_groupmember::kDbiFlags,  CK_VAL, offsetof
    (key_user_data_t, flags), uud::kDalUint8, 1},
  { uudst::vtep_groupmember::kDbiCsRowstatus,  CS_VAL, offsetof(
          val_vtep_grp_member, cs_row_status), uud::kDalUint8, 1}
};


VtepGrpMemMoMgr::VtepGrpMemMoMgr() {
  UPLL_FUNC_TRACE;
  Table *tbl = new Table(uudst::kDbiVtepGrpMemTbl, UNC_KT_VTEP_GRP_MEMBER,
      vtepgrp_mem_bind_info, IpctSt::kIpcStKeyVtepGrpMember,
      IpctSt::kIpcStValVtepGrpMember,
      uudst::vtep_groupmember::kDbiVtepGrpMemNumCols);
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = tbl;
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;

  nchild = 0;
  child = NULL;
#ifdef _STANDALONE_
  SetMoManager(UNC_KT_VTEP_GRP_MEMBER, reinterpret_cast<MoMgr *>(this));
#endif
}

/*
 * Based on the key type the bind info will pass
 *
 bool VtepGrpMemMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
 BindInfo *&binfo, int &nattr,
 MoMgrTables tbl ) {
 if (MAINTBL == tbl) {

 nattr = NUM_KEY_MAIN_TBL_;
 binfo = key_vtepgrp_mem_maintbl_update_bind_info;
 }
 return PFC_TRUE;
 }*/

bool VtepGrpMemMoMgr::IsValidKey(void *key, uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_vtep_grp_member *vtep_grp_mem_key = reinterpret_cast
                                          <key_vtep_grp_member *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vtep_groupmember::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                (vtep_grp_mem_key->vtepgrp_key.vtn_key.vtn_name),
                kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vtep_groupmember::kDbiVtepgrpName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                    (vtep_grp_mem_key->vtepgrp_key.vtepgrp_name),
                    kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VtepGroup Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vtep_groupmember::kDbiVtepgrpMemberName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                    (vtep_grp_mem_key->vtepmember_name),
                    kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VtepGroup Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_TRACE("Invalid Key Index");
      return false;
  }
  return true;
}

upll_rc_t VtepGrpMemMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  void *pkey;
  key_vtep_grp_member *vtep_key = static_cast<key_vtep_grp_member *>
    (ConfigKeyVal::Malloc(sizeof(key_vtep_grp_member)));
  if (vtep_key == NULL) return UPLL_RC_ERR_GENERIC;
  if (parent_key == NULL) {
    okey = new ConfigKeyVal(UNC_KT_VTEP_GRP_MEMBER,
                            IpctSt::kIpcStKeyVtepGrpMember,
                            vtep_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    free(vtep_key);
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey && (okey->get_key())) {
    free(vtep_key);
    vtep_key = reinterpret_cast<key_vtep_grp_member *>(okey->get_key());
  } else {
    okey = new ConfigKeyVal(UNC_KT_VTEP_GRP_MEMBER,
                            IpctSt::kIpcStKeyVtepGrpMember,
                            vtep_key, NULL);
    if (okey == NULL) {
      free(vtep_key);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  /* presumes MoMgrs receive only supported keytypes */
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTEP_GRP_MEMBER:
      uuu::upll_strncpy(vtep_key->vtepmember_name,
             reinterpret_cast<key_vtep_grp_member *>(pkey)->vtepmember_name,
             (kMaxLenVnodeName+1));
      /* fall through intended */
    case UNC_KT_VTEP_GRP:
      uuu::upll_strncpy(vtep_key->vtepgrp_key.vtepgrp_name,
             reinterpret_cast<key_vtep_grp *>(pkey)->vtepgrp_name,
             (kMaxLenVnodeName+1));
      uuu::upll_strncpy(vtep_key->vtepgrp_key.vtn_key.vtn_name,
             reinterpret_cast<key_vtep_grp *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTN:
    default:
      uuu::upll_strncpy(vtep_key->vtepgrp_key.vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name, (kMaxLenVtnName+1));
  }
  SET_USER_DATA(okey, parent_key);
  return result_code;
}

upll_rc_t VtepGrpMemMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey ) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtep_grp_member *pkey = (ikey)?
    static_cast<key_vtep_grp_member *>(ikey->get_key()):NULL;
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  if (ikey->get_key_type() != UNC_KT_VTEP_GRP_MEMBER)
    return UPLL_RC_ERR_GENERIC;
  key_vtep_grp *vtepgrp_key = reinterpret_cast<key_vtep_grp *>
    (ConfigKeyVal::Malloc(sizeof(key_vtep_grp)));
  uuu::upll_strncpy(vtepgrp_key->vtn_key.vtn_name,
         reinterpret_cast<key_vtep_grp_member *>(pkey)->
         vtepgrp_key.vtn_key.vtn_name, (kMaxLenVtnName+1));
  uuu::upll_strncpy(vtepgrp_key->vtepgrp_name,
         reinterpret_cast<key_vtep_grp_member *>(pkey)->
         vtepgrp_key.vtepgrp_name, (kMaxLenVnodeName+1));
  DELETE_IF_NOT_NULL(okey);
  okey = new ConfigKeyVal(UNC_KT_VTEP_GRP, IpctSt::kIpcStKeyVtepGrp,
         vtepgrp_key, NULL);
  if (okey == NULL) {
    free(vtepgrp_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return result_code;
}


upll_rc_t VtepGrpMemMoMgr::AllocVal(ConfigVal *&ck_val,
    upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_vtep_grp_member)));
      if (!val) return UPLL_RC_ERR_GENERIC;
      ck_val = new ConfigVal(IpctSt::kIpcStValVtepGrpMember, val);
      if (!ck_val) {
        free(reinterpret_cast<val_vtep_grp_member *>(val));
        return UPLL_RC_ERR_GENERIC;
      }
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMemMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
    ConfigKeyVal *&req, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VTEP_GRP_MEMBER)
    return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  if (tmp) {
    if (tbl == MAINTBL) {
      val_vtep_grp_member *ival = reinterpret_cast
                                  <val_vtep_grp_member *>(GetVal(req));
      val_vtep_grp_member *vtep_val = reinterpret_cast<val_vtep_grp_member *>
          (ConfigKeyVal::Malloc(sizeof(val_vtep_grp_member)));
      if (!vtep_val) return UPLL_RC_ERR_GENERIC;
      memcpy(vtep_val, ival, sizeof(val_vtep_grp_member));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVtepGrpMember, vtep_val);
      if (!tmp1) {
        free(vtep_val);
        return UPLL_RC_ERR_GENERIC;
      }
    }
  };
  void *tkey = (req != NULL)?(req)->get_key():NULL;
  key_vtep_grp_member *ikey = reinterpret_cast<key_vtep_grp_member *>(tkey);
  key_vtep_grp_member *vtep_key = reinterpret_cast<key_vtep_grp_member *>
     (ConfigKeyVal::Malloc(sizeof(key_vtep_grp_member)));
  if (!vtep_key) {
    if (tmp1) delete tmp1;
    return UPLL_RC_ERR_GENERIC;
  }
  memcpy(vtep_key, ikey, sizeof(key_vtep_grp_member));
  okey = new ConfigKeyVal(UNC_KT_VTEP_GRP_MEMBER,
                          IpctSt::kIpcStKeyVtepGrpMember,
                          vtep_key, tmp1);
  if (okey) {
    SET_USER_DATA(okey, req);
  } else {
    if (tmp1) delete tmp1;
    free(vtep_key);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMemMoMgr::UpdateConfigStatus(ConfigKeyVal *vtepgrpmem_key,
                                              unc_keytype_operation_t op,
                                              uint32_t driver_result,
                                              ConfigKeyVal *upd_key,
                                              DalDmlIntf *dmi,
                                              ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_vtep_grp_member *vtep_grp_member_val;
  unc_keytype_configstatus_t cs_status =
           (driver_result == UPLL_RC_SUCCESS)?UNC_CS_APPLIED
                                             :UNC_CS_NOT_APPLIED;
  vtep_grp_member_val = reinterpret_cast<val_vtep_grp_member *>
                                        (GetVal(vtepgrpmem_key));
  if (vtep_grp_member_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    vtep_grp_member_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
  val_vtep_grp_member *grp_mem_val_run =
                        reinterpret_cast<val_vtep_grp_member *>
                                        (GetVal(upd_key));
  UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
  vtep_grp_member_val->cs_row_status =
                          grp_mem_val_run->cs_row_status;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMemMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  val_vtep_grp_member_t *vtepgrpmem_val = (ckv_running != NULL)?
    reinterpret_cast<val_vtep_grp_member_t *>
    (GetVal(ckv_running)):NULL;
  if (NULL == vtepgrpmem_val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
    vtepgrpmem_val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (vtepgrpmem_val->cs_row_status == UNC_CS_INVALID ||
            vtepgrpmem_val->cs_row_status == UNC_CS_NOT_APPLIED))
    vtepgrpmem_val->cs_row_status = cs_status;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMemMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VTEP_GRP_MEMBER != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  if (ikey->get_st_num() != IpctSt::kIpcStKeyVtepGrpMember) {
    UPLL_LOG_DEBUG("Invalid structure received.Expected struct-"
        "kIpcStKeyVtepGrpMember, received struct -%s ",
        reinterpret_cast<const char *>
        (IpctSt::GetIpcStdef(ikey->get_st_num())));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vtep_grp_member_t *key_vtep_grp_member =
    reinterpret_cast <key_vtep_grp_member_t *> (ikey->get_key());

  ret_val = ValidateVTepGrpMemberKey(key_vtep_grp_member, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Key struct Validation failed for VTEP_GRP_MEMBER");
    return UPLL_RC_ERR_CFG_SYNTAX;
  } else {
    if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE) ||
         (operation == UNC_OP_DELETE)) &&
         (dt_type == UPLL_DT_CANDIDATE)) {
      UPLL_LOG_TRACE("Value structure validation is none");
      return UPLL_RC_SUCCESS;
    } else if (((operation == UNC_OP_READ) ||
          (operation == UNC_OP_READ_SIBLING) ||
          (operation == UNC_OP_READ_SIBLING_BEGIN) ||
          (operation == UNC_OP_READ_SIBLING_COUNT) ||
          (operation == UNC_OP_READ_NEXT) ||
          (operation == UNC_OP_READ_BULK))
        && ((dt_type == UPLL_DT_CANDIDATE) ||
          (dt_type == UPLL_DT_RUNNING) ||
          (dt_type == UPLL_DT_STARTUP) ||
          (dt_type == UPLL_DT_STATE))) {
      if (option1 == UNC_OPT1_NORMAL) {
        if (option2 == UNC_OPT2_NONE) {
       UPLL_LOG_TRACE("Value structure validation is none");
       return UPLL_RC_SUCCESS;
     }  else {
       UPLL_LOG_DEBUG("option2 is not matching");
       return UPLL_RC_ERR_INVALID_OPTION2;
     }
     } else {
      UPLL_LOG_DEBUG("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
     }
    } else {
      UPLL_LOG_DEBUG("Invalid datatype(%d) and operation(%d)", dt_type,
                     operation);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMemMoMgr::ValidateAttribute(ConfigKeyVal *kval,
                            DalDmlIntf *dmi,
                            IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_vtep = NULL;
  upll_rc_t result_code = GetVtepConfigValData(kval, ck_vtep,
                                       UPLL_DT_CANDIDATE, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in fetching the Vtep data from DB %d",
                   result_code);
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                   UPLL_RC_ERR_CFG_SEMANTIC:result_code;
  }
  DELETE_IF_NOT_NULL(ck_vtep);
  return result_code;
}

upll_rc_t VtepGrpMemMoMgr::ValidateCapability(
    IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  bool result_code = false;
  uint32_t max_attrs = 0;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs = NULL;

  if (!ikey || !req) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_key_type() != UNC_KT_VTEP_GRP_MEMBER) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", ikey->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (!ctrlr_name) {
    ctrlr_name = reinterpret_cast<char*>((reinterpret_cast<key_user_data_t *>
                  (ikey->get_user_data()))->ctrlr_id);
    if (!ctrlr_name || !strlen(ctrlr_name)) {
      UPLL_LOG_DEBUG("Controller Name is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  UPLL_LOG_DEBUG("dt_type   : (%d)"
               "operation : (%d)"
               "option1   : (%d)"
               "option2   : (%d)",
               dt_type, operation, option1, option2);

  switch (operation) {
    case UNC_OP_CREATE: {
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count,
                                        &max_attrs, &attrs);
      break;
    }
    case UNC_OP_UPDATE: {
      result_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    }
    case UNC_OP_READ:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING_COUNT:

      result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      break;
    default:
      UPLL_LOG_DEBUG("Invalid Operation Code - (%d)", operation);
      return UPLL_RC_ERR_GENERIC;
  }

  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for opeartion(%d)",
                   ikey->get_key_type(), ctrlr_name, operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMemMoMgr::ValidateVTepGrpMemberKey(
    key_vtep_grp_member_t *key_vtep_grp_member,
    uint32_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(
  reinterpret_cast<char *>(key_vtep_grp_member->vtepgrp_key.vtn_key.vtn_name),
      kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Vtn Name syntax check failed."
                  "Received VTN Name - %s",
                  key_vtep_grp_member->vtepgrp_key.vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  ret_val = ValidateKey(
      reinterpret_cast<char *>(key_vtep_grp_member->vtepgrp_key.vtepgrp_name),
      kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("VtepGrpName syntax check failed."
                "Received VtepGrpName -%s",
                key_vtep_grp_member->vtepgrp_key.vtepgrp_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    ret_val = ValidateKey(reinterpret_cast<char *>
        (key_vtep_grp_member->vtepmember_name),
        kMinLenVnodeName, kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Syntax check failed. vtepmember_name-%s",
          key_vtep_grp_member->vtepmember_name);
      return ret_val;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(key_vtep_grp_member->vtepmember_name);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMemMoMgr::GetControllerDomainId(ConfigKeyVal *ikey,
                                       upll_keytype_datatype_t dt_type,
                                       controller_domain_t *ctrlr_dom,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_vtep = NULL;
  upll_rc_t result_code = GetVtepConfigValData(ikey, ck_vtep,
                                       UPLL_DT_CANDIDATE, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in fetching the Vtep data from DB %d",
                   result_code);
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                   UPLL_RC_ERR_CFG_SEMANTIC:result_code;
    return result_code;
  }
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VTEP)));
  if (!mgr) return UPLL_RC_ERR_GENERIC;
  result_code = mgr->GetControllerDomainId(ck_vtep, ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  // GET_USER_DATA_CTRLR_DOMAIN(grp_mem_key, *ctrlr_dom);
  if (!ctrlr_dom->ctrlr || !ctrlr_dom->domain) {
    DELETE_IF_NOT_NULL(ck_vtep);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(ikey, ck_vtep);
  UPLL_LOG_DEBUG("ctrlr %s domain %s", ctrlr_dom->ctrlr, ctrlr_dom->domain);
  DELETE_IF_NOT_NULL(ck_vtep);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtepGrpMemMoMgr::CompareControllers(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi) {
  ConfigKeyVal *parent_grp_key = NULL;
  controller_domain *ctrlr_dom[2] = { NULL, NULL };
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetParentConfigKey(parent_grp_key, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Error in fetching the parent key");
    return result_code;
  }
  result_code = GetControllerDomainId(parent_grp_key, dt_type,
                                      ctrlr_dom[0], dmi);
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom[0]->ctrlr == NULL)) {
    UPLL_LOG_INFO("Invalid ctrlr");
    return result_code;
  }

  ConfigKeyVal *ck_vtep = NULL;
  result_code = GetVtepConfigValData(ikey, ck_vtep, dt_type, dmi);
  if (result_code != UPLL_RC_SUCCESS || (!ck_vtep)) {
    UPLL_LOG_DEBUG("Error in fetching the Vtep val data from DB");
    return result_code;
  }
  GET_USER_DATA_CTRLR(ck_vtep, ctrlr_dom[1]->ctrlr);
  if (ctrlr_dom[1]->ctrlr == NULL)
    return UPLL_RC_ERR_GENERIC;
  if (!strcmp(reinterpret_cast<char *>(ctrlr_dom[0]->ctrlr),
              reinterpret_cast<char *>(ctrlr_dom[1]->ctrlr)))
    return UPLL_RC_ERR_CFG_SEMANTIC;
  return result_code;
}

upll_rc_t VtepGrpMemMoMgr::GetVtepConfigValData(ConfigKeyVal *ikey,
    ConfigKeyVal *&okey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VTEP)));
  if (!mgr) return UPLL_RC_ERR_GENERIC;
  key_vtep *vtep_key = reinterpret_cast<key_vtep *>
                       (ConfigKeyVal::Malloc(sizeof(key_vtep)));
  if (!vtep_key) return UPLL_RC_ERR_GENERIC;
  uuu::upll_strncpy(vtep_key->vtep_name,
         reinterpret_cast<key_vtep_grp_member *>
         (ikey->get_key())->vtepmember_name, (kMaxLenVnodeName+1));
  uuu::upll_strncpy(vtep_key->vtn_key.vtn_name,
          reinterpret_cast<key_vtep_grp_member *>
         (ikey->get_key())->vtepgrp_key.vtn_key.vtn_name,
         (kMaxLenVtnName+1));
  okey = new ConfigKeyVal(UNC_KT_VTEP, IpctSt::kIpcStKeyVtep, vtep_key, NULL);

  if (okey == NULL) {
    free(vtep_key);
    return UPLL_RC_ERR_GENERIC;
  } else {
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone,
                    kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
    result_code = mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB Return Failure = %d ", result_code);
      delete okey;
      return result_code;
    }
  }
  return result_code;
}
/*
   upll_rc_t VtepGrpMemMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
   ConfigKeyVal *ikey) {

   if ( !ikey || !(ikey->get_key()) )
   return UPLL_RC_ERR_GENERIC;

   upll_rc_t result_code = UPLL_RC_SUCCESS;
   key_rename_vnode_info *key_rename = (key_rename_vnode_info *)ikey->get_key();
   key_vtep_grp_member_t *vtep_key =
   (key_vtep_grp_member_t *) malloc ( sizeof (key_vtep_grp_member_t));
   if (!vtep_key)
   return UPLL_RC_ERR_GENERIC;
   if (!strlen ((char *)key_rename->old_unc_vtn_name))
   return UPLL_RC_ERR_GENERIC;
   strcpy ((char *)vtep_key ->vtepgrp_key.vtn_key.vtn_name,
   (char *)key_rename->old_unc_vtn_name);

   okey = new ConfigKeyVal (UNC_KT_VTEP_GRP_MEMBER,
   IpctSt::kIpcStKeyVtepGrpMember, vtep_key, NULL);
   if (!okey) {
   free(vtep_key);
   return UPLL_RC_ERR_GENERIC;
   }
   return result_code;
   }
   */

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
