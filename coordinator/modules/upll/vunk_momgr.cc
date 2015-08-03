/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vunk_momgr.hh"
#include "uncxx/upll_log.hh"
#include "vtn_momgr.hh"

#define NUM_KEY_MAIN_TBL_ 4

#define VUNKNOWN_TYPE_BRIDGE 0
#define VUNKNOWN_TYPE_ROUTER 1

namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VunknownMoMgr::vunk_bind_info[] = {
    { uudst::vunknown::kDbiVtnName, CFG_KEY, offsetof(key_vunknown,
                                                      vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::vunknown::kDbiVunknownName, CFG_KEY, offsetof(key_vunknown,
                                                           vunknown_name),
      uud::kDalChar, 32 },
    { uudst::vunknown::kDbiDesc, CFG_VAL, offsetof(val_vunknown, description),
      uud::kDalChar, 128 },
    { uudst::vunknown::kDbiType, CFG_VAL, offsetof(val_vunknown, type),
      uud::kDalUint8, 1 },
    { uudst::vunknown::kDbiDomainId, CFG_VAL, offsetof(val_vunknown, domain_id),
      uud::kDalChar, 32 },
    { uudst::vunknown::kDbiDomainId, CK_VAL, offsetof(key_user_data, domain_id),
      uud::kDalChar, 32 },
    { uudst::vunknown::kDbiValidDesc, CFG_META_VAL, offsetof(
        val_vunknown, valid[UPLL_IDX_DESC_VUN]),
      uud::kDalUint8, 1 },
    { uudst::vunknown::kDbiValidType, CFG_META_VAL, offsetof(
        val_vunknown, valid[UPLL_IDX_TYPE_VUN]),
      uud::kDalUint8, 1 },
    { uudst::vunknown::kDbiValidDomainId, CFG_META_VAL, offsetof(
        val_vunknown, valid[UPLL_IDX_DOMAIN_ID_VUN]),
      uud::kDalUint8, 1 },
    { uudst::vunknown::kDbiCsDesc, CS_VAL, offsetof(val_vunknown,
                                                    cs_attr[UPLL_IDX_DESC_VUN]),
      uud::kDalUint8, 1 },
    { uudst::vunknown::kDbiCsType, CS_VAL, offsetof(val_vunknown,
                                                    cs_attr[UPLL_IDX_TYPE_VUN]),
      uud::kDalUint8, 1 },
    { uudst::vunknown::kDbiCsDomainId, CS_VAL, offsetof(
        val_vunknown, cs_attr[UPLL_IDX_DOMAIN_ID_VUN]),
      uud::kDalUint8, 1 },
    { uudst::vunknown::kDbiCsRowstatus, CS_VAL, offsetof(val_vunknown,
                                                         cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::vunknown::kDbiFlags, CK_VAL, offsetof(key_user_data_t, flags),
      uud::kDalUint8, 1 } };

BindInfo VunknownMoMgr::key_vunk_maintbl_update_bind_info[] = {
    { uudst::vunknown::kDbiVtnName, CFG_MATCH_KEY, offsetof(key_vunknown,
                                                            vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vunknown::kDbiVunknownName, CFG_MATCH_KEY, offsetof(key_vunknown,
                                                                 vunknown_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vunknown::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vunknown::kDbiFlags, CK_VAL, offsetof(key_user_data_t,
                                                          flags),
      uud::kDalUint8, 1 } };

unc_key_type_t VunknownMoMgr::vunk_child[] = { UNC_KT_VUNK_IF };

VunknownMoMgr::VunknownMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVunknownTbl, UNC_KT_VUNKNOWN,
                         vunk_bind_info, IpctSt::kIpcStKeyVunknown,
                         IpctSt::kIpcStValVunknown,
                         uudst::vunknown::kDbiVunknownNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;

  nchild = sizeof(vunk_child) / sizeof(*vunk_child);
  child = vunk_child;
}

/*
 * Based on the key type the bind info will pass
 **/

bool VunknownMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                         BindInfo *&binfo, int &nattr,
                                         MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_vunk_maintbl_update_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalide Table");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

bool VunknownMoMgr::IsValidKey(void *key, uint64_t index,
                               MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_vunknown *vunk_key = reinterpret_cast<key_vunknown *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vunknown::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vunk_key->vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vunknown::kDbiVunknownName:
      ret_val = ValidateKey(reinterpret_cast<char *>(vunk_key->vunknown_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("Vunknown name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_INFO("Invalid Key Index");
      break;
  }
  return true;
}

upll_rc_t VunknownMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                           ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vunknown *vunk_key = reinterpret_cast<key_vunknown *>
                                (malloc(sizeof(key_vunknown)));

  if (!vunk_key) {
    UPLL_LOG_ERROR("vunk_key Memory allocation failed");
    return UPLL_RC_ERR_GENERIC;
  }
  memset(vunk_key, 0, sizeof(key_vunknown));

  void *pkey;
  if (parent_key == NULL) {
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VUNKNOWN, IpctSt::kIpcStKeyVunknown,
                            vunk_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
     pkey = parent_key->get_key();
  }
  if (!pkey) {
    free(vunk_key);
    return UPLL_RC_ERR_GENERIC;
  }

  /* presumes MoMgrs receive only supported keytypes */
  switch (parent_key->get_key_type()) {
    case UNC_KT_VUNKNOWN:
      uuu::upll_strncpy(vunk_key->vunknown_name,
             reinterpret_cast<key_vunknown *>(pkey)->vunknown_name,
             (kMaxLenVnodeName+1));
      uuu::upll_strncpy(vunk_key->vtn_key.vtn_name,
            reinterpret_cast<key_vunknown *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTN:
    default:
      uuu::upll_strncpy(vunk_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name,
             (kMaxLenVtnName+1));
  }
  okey = new ConfigKeyVal(UNC_KT_VUNKNOWN, IpctSt::kIpcStKeyVunknown, vunk_key,
                          NULL);
  if (okey == NULL) {
    free(vunk_key);
    result_code = UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, parent_key);
  return result_code;
}

upll_rc_t VunknownMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                            ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) return UPLL_RC_ERR_GENERIC;
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_VUNKNOWN) return UPLL_RC_ERR_GENERIC;
  void *pkey =  ikey->get_key();
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>(malloc(sizeof(key_vtn)));
  if (!vtn_key) {
    UPLL_LOG_ERROR("vtn_key Memory allocation failed");
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(vtn_key->vtn_name,
         reinterpret_cast<key_vunknown *>(pkey)->vtn_key.vtn_name,
         (kMaxLenVtnName+1) );
  if (okey) delete okey;
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  if (okey == NULL) {
    free(vtn_key);
    result_code = UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VunknownMoMgr::AllocVal(ConfigVal *&ck_val,
                                  upll_keytype_datatype_t dt_type,
                                  MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>(malloc(sizeof(val_vunknown)));
      if (!val) return UPLL_RC_ERR_GENERIC;
      memset(val, 0, sizeof(val_vunknown));
      ck_val = new ConfigVal(IpctSt::kIpcStValVunknown, val);
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunknownMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                         ConfigKeyVal *&req, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VUNKNOWN) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  void *tkey = (req)->get_key();
  if (tkey == NULL) return UPLL_RC_ERR_GENERIC;

  if (tmp) {
    if (tbl == MAINTBL) {
      val_vunknown *ival = reinterpret_cast<val_vunknown *>(GetVal(req));
      if (!ival) {
        UPLL_LOG_DEBUG("Invalid vunknown value");
        return UPLL_RC_ERR_GENERIC;
      }
      val_vunknown *vunk_val = reinterpret_cast<val_vunknown *>
                                          (malloc(sizeof(val_vunknown)));
      if (!vunk_val) return UPLL_RC_ERR_GENERIC;
      memcpy(vunk_val, ival, sizeof(val_vunknown));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVunknown, vunk_val);
    }
  };
  key_vunknown *ikey = reinterpret_cast<key_vunknown *>(tkey);
  key_vunknown *vunk_key = reinterpret_cast<key_vunknown *>
      (ConfigKeyVal::Malloc(sizeof(key_vunknown)));
  memcpy(vunk_key, ikey, sizeof(key_vunknown));
  okey = new ConfigKeyVal(UNC_KT_VUNKNOWN, IpctSt::kIpcStKeyVunknown, vunk_key,
                          tmp1);
  SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunknownMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  return result_code;
}
upll_rc_t VunknownMoMgr::UpdateConfigStatus(ConfigKeyVal *vunk_key,
                                            unc_keytype_operation_t op,
                                            uint32_t driver_result,
                                            ConfigKeyVal *upd_key,
                                            DalDmlIntf *dmi,
                                            ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_vunknown *vunk_val;
  unc_keytype_configstatus_t cs_status = UNC_CS_APPLIED;
  vunk_val = reinterpret_cast<val_vunknown *>(GetVal(vunk_key));
  val_vunknown *vunk_val2 = reinterpret_cast<val_vunknown *>(GetVal(upd_key));
  UPLL_LOG_TRACE("Key in Candidate %s", (vunk_key->ToStrAll()).c_str());
  if (vunk_val == NULL || vunk_val2 == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    vunk_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    void *vunkval = reinterpret_cast<void *>(vunk_val);
    CompareValidValue(vunkval, GetVal(upd_key), true);
    UPLL_LOG_TRACE("Key in Running %s", (upd_key->ToStrAll()).c_str());
    vunk_val->cs_row_status = vunk_val2->cs_row_status;
  } else {
     return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
       loop < sizeof(vunk_val->valid) / sizeof(vunk_val->valid[0]); ++loop) {
    if ((UNC_VF_VALID == vunk_val->valid[loop])
          || (UNC_VF_VALID_NO_VALUE == vunk_val->valid[loop]))
      vunk_val->cs_attr[loop] =  cs_status;
    else if ((UNC_VF_INVALID == vunk_val->valid[loop]) &&
             (UNC_OP_CREATE == op))
      vunk_val->cs_attr[loop] = UNC_CS_APPLIED;
    else if ((UNC_VF_INVALID == vunk_val->valid[loop]) &&
             (UNC_OP_UPDATE == op)) {
      vunk_val->cs_attr[loop] = vunk_val2->cs_attr[loop];
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunknownMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                         ConfigKeyVal *ikey) {
  UPLL_LOG_INFO("Not supported for this keytype. Returning Generic Error");
  return UPLL_RC_ERR_GENERIC;
}
upll_rc_t VunknownMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVunknown) {
    UPLL_LOG_DEBUG(
        "Invalid structure received.Expected struct-kIpcStKeyVunknown"
        "received struct -%s ",
        reinterpret_cast<const char *>
        (IpctSt::GetIpcStdef(ikey->get_st_num())));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vunknown_t *key_vunknown =
    reinterpret_cast<key_vunknown_t *>(ikey->get_key());
  val_vunknown_t *val_vunknown = NULL;
  if ((ikey->get_cfg_val())
      && (ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVunknown) {
    val_vunknown =
      reinterpret_cast<val_vunknown_t *>(ikey->get_cfg_val()->get_val());
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VUNKNOWN != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (key_vunknown == NULL) {
    UPLL_LOG_DEBUG("KT_VUNKNOWN Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  ret_val = ValidateVunknownKey(key_vunknown, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Syntax check failed for KT_VUNKNOWN key structure");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE))
      && (dt_type == UPLL_DT_CANDIDATE)) {
    if (val_vunknown == NULL) {
      UPLL_LOG_DEBUG("Value structure mandatory for CREATE/UPDATE");
      return UPLL_RC_ERR_BAD_REQUEST;
    } else {
      ret_val = ValidateVunknownValue(val_vunknown, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Syntax check failed for KT_VUNKNOWN value structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    }
  } else if (((operation == UNC_OP_READ) || (operation == UNC_OP_READ_SIBLING)
        || (operation == UNC_OP_READ_SIBLING_BEGIN)
        || (operation == UNC_OP_READ_SIBLING_COUNT))
      && ((dt_type == UPLL_DT_CANDIDATE) || (dt_type == UPLL_DT_RUNNING)
        || (dt_type == UPLL_DT_STARTUP) || (dt_type == UPLL_DT_STATE))) {
    if (option1 == UNC_OPT1_NORMAL) {
      if (option2 == UNC_OPT2_NONE) {
        if (val_vunknown != NULL) {
          ret_val = ValidateVunknownValue(val_vunknown);
          if (ret_val != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Syntax check failed for Vunknown value structure");
            return UPLL_RC_ERR_CFG_SYNTAX;
          }
          return UPLL_RC_SUCCESS;
        } else {
          UPLL_LOG_TRACE("Value structure is optional for READ");
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
  } else if ((operation == UNC_OP_DELETE) && (dt_type == UPLL_DT_CANDIDATE)) {
    UPLL_LOG_TRACE("Value structure is none for this operation :%d", operation);
    return UPLL_RC_SUCCESS;
  } else if (((operation == UNC_OP_READ_NEXT) ||
        (operation == UNC_OP_READ_BULK)) &&
      ((dt_type == UPLL_DT_CANDIDATE) ||
       (dt_type == UPLL_DT_RUNNING) ||
       (dt_type == UPLL_DT_STARTUP))) {
    UPLL_LOG_TRACE("Value structure is none for this operation :%d", operation);
    return UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_DEBUG("Invalid datatype(%d) and operation(%d)", dt_type,
        operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
}

upll_rc_t VunknownMoMgr::ValidateAttribute(ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                        IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  if (req->operation == UNC_OP_UPDATE) {
    ConfigKeyVal *okey = NULL;
    result_code = GetChildConfigKey(okey, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Returing error %d", result_code);
      if (okey) delete okey;
      return result_code;
    }
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone,
      kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
    result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                               dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Record does Not Exists");
      delete okey;
      return result_code;
    }
    GET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    result_code = CtrlrIdAndDomainIdUpdationCheck(ikey, okey);
    DELETE_IF_NOT_NULL(okey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Controller_id/domain_id updation check failed");
      return result_code;
    }
  }
  return result_code;
}

upll_rc_t VunknownMoMgr::CtrlrIdAndDomainIdUpdationCheck(ConfigKeyVal *ikey,
                                                   ConfigKeyVal *okey) {
  UPLL_FUNC_TRACE;
  val_vunknown *vunk_val = reinterpret_cast<val_vunknown *>(GetVal(ikey));
  val_vunknown *vunk_val1 = reinterpret_cast<val_vunknown *>(GetVal(okey));
  if (!vunk_val || !vunk_val1) return UPLL_RC_ERR_GENERIC;
  if (vunk_val->valid[UPLL_IDX_DOMAIN_ID_VUN] == UNC_VF_VALID) {
    if (strncmp(reinterpret_cast<const char *>(vunk_val->domain_id),
                reinterpret_cast<const char *>(vunk_val1->domain_id),
                kMaxLenDomainId+1)) {
      UPLL_LOG_DEBUG("domain id comparision failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunknownMoMgr::ValidateVunknownKey(key_vunknown_t *key_vunknown,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val = UPLL_RC_SUCCESS;
  /** Validate vtn_key structure */
  VtnMoMgr *objvtnmgr =
    reinterpret_cast<VtnMoMgr *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VTN)));
  if (NULL == objvtnmgr) {
    UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }
  ret_val = objvtnmgr->ValidateVtnKey(&(key_vunknown->vtn_key));

  if (UPLL_RC_SUCCESS != ret_val) {
    UPLL_LOG_DEBUG("Vtn_name syntax validation failed.Err Code- %d", ret_val);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    UPLL_LOG_TRACE("KT_VUNKNOWN:vunknown_name (%s)",
                   key_vunknown->vunknown_name);
    ret_val = ValidateKey(reinterpret_cast<char *>(key_vunknown->vunknown_name),
        kMinLenVnodeName, kMaxLenVnodeName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Validation failure for key_vunknown->vunknown_name");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(key_vunknown->vunknown_name);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunknownMoMgr::ValidateVunknownValue(val_vunknown_t *vunk_val,
    uint32_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val =false;

  // Attribute syntax validation
  for (unsigned int valid_index = 0;
       valid_index < sizeof(vunk_val->valid) / sizeof(vunk_val->valid[0]);
       valid_index++) {
    if (vunk_val->valid[valid_index] == UNC_VF_VALID) {
      switch (valid_index) {
        case UPLL_IDX_DOMAIN_ID_VUN:
          ret_val = ValidateDefaultStr(vunk_val->domain_id,
                                       kMinLenDomainId, kMaxLenDomainId);
          break;
        case UPLL_IDX_DESC_VUN:
          ret_val = ValidateDesc(vunk_val->description,
                                 kMinLenDescription, kMaxLenDescription);
          break;
        case UPLL_IDX_TYPE_VUN:
          ret_val = ValidateNumericRange(vunk_val->type,
                                 (uint8_t) VUNKNOWN_TYPE_BRIDGE,
                                 (uint8_t) VUNKNOWN_TYPE_ROUTER, true, true);
          break;
      }
      if (!ret_val) {
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    }
  }

  // Additional checks
  for (unsigned int valid_index = 0;
       valid_index < sizeof(vunk_val->valid) / sizeof(vunk_val->valid[0]);
       valid_index++) {
    uint8_t flag = vunk_val->valid[valid_index];
    switch (operation) {
      case UNC_OP_CREATE:
        {
          switch (valid_index) {
            case UPLL_IDX_DOMAIN_ID_VUN:
              if ((flag == UNC_VF_INVALID || flag == UNC_VF_VALID_NO_VALUE)) {
                UPLL_LOG_DEBUG("domain_id flag is invalid"
                               " or valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break;
            case UPLL_IDX_DESC_VUN:
              break;
            case UPLL_IDX_TYPE_VUN:
              if ((flag == UNC_VF_INVALID) || (flag == UNC_VF_VALID_NO_VALUE)) {
                vunk_val->type = VUNKNOWN_TYPE_BRIDGE;
              }
              break;
            default:
              break;
          }
        }
        break;
      case UNC_OP_UPDATE:
        {
          switch (valid_index) {
            case UPLL_IDX_DOMAIN_ID_VUN:
              if (flag == UNC_VF_VALID_NO_VALUE) {
                UPLL_LOG_DEBUG
                    ("controller_id or domain_id flag is valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break;
            case UPLL_IDX_DESC_VUN:
            case UPLL_IDX_TYPE_VUN:
            default:
              break;
          }
        }
        break;
    }
  }

  // Resets
  for (unsigned int valid_index = 0;
       valid_index < sizeof(vunk_val->valid) / sizeof(vunk_val->valid[0]);
       valid_index++) {
    uint8_t flag = vunk_val->valid[valid_index];
    if (flag != UNC_VF_INVALID && flag != UNC_VF_VALID_NO_VALUE)
      continue;

    switch (valid_index) {
      case UPLL_IDX_DOMAIN_ID_VUN:
        StringReset(vunk_val->domain_id);
        break;
      case UPLL_IDX_DESC_VUN:
        StringReset(vunk_val->description);
        break;
      case UPLL_IDX_TYPE_VUN:
        vunk_val->type = VUNKNOWN_TYPE_BRIDGE;
        break;
      default:
        UPLL_LOG_TRACE("Never here");
        break;
    }
  }
  return UPLL_RC_SUCCESS;
}

bool VunknownMoMgr::CompareValidValue(void *&val1,
                                 void *val2,
                                 bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vunknown_t *val_vunknown1 = reinterpret_cast<val_vunknown_t *>(val1);
  val_vunknown_t *val_vunknown2 = reinterpret_cast<val_vunknown_t *>(val2);
  if (!val_vunknown2) {
      UPLL_LOG_TRACE("Invalid param");
      return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
        loop < sizeof(val_vunknown1->valid) / sizeof(uint8_t); ++loop) {
      if (UNC_VF_INVALID == val_vunknown1->valid[loop]
          && UNC_VF_VALID == val_vunknown2->valid[loop])
        val_vunknown1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  /* Specify the configured ip address for
   * PFC to clear the configured IP address
   */
  if ((UNC_VF_VALID == val_vunknown1->valid[UPLL_IDX_DESC_VUN])
        && (UNC_VF_VALID == val_vunknown2->valid[UPLL_IDX_DESC_VUN]))
    if (!strncmp(reinterpret_cast<char*>(val_vunknown1->description),
                 reinterpret_cast<char*>(val_vunknown2->description),
                 kMaxLenDescription))
        val_vunknown1->valid[UPLL_IDX_DESC_VUN] = UNC_VF_INVALID;
  if (UNC_VF_VALID == val_vunknown1->valid[UPLL_IDX_TYPE_VUN]
      && UNC_VF_VALID == val_vunknown2->valid[UPLL_IDX_TYPE_VUN]) {
    if (val_vunknown1->type == val_vunknown2->type)
      val_vunknown1->valid[UPLL_IDX_TYPE_VUN] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vunknown1->valid[UPLL_IDX_CONTROLLER_ID_VUN]
      && UNC_VF_VALID == val_vunknown2->valid[UPLL_IDX_CONTROLLER_ID_VUN]) {
    if (!strcmp(reinterpret_cast<char*>(val_vunknown1->controller_id),
                reinterpret_cast<char*>(val_vunknown2->controller_id)))
      val_vunknown1->valid[UPLL_IDX_CONTROLLER_ID_VUN] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vunknown1->valid[UPLL_IDX_DOMAIN_ID_VUN]
      && UNC_VF_VALID == val_vunknown2->valid[UPLL_IDX_DOMAIN_ID_VUN]) {
    if (!strcmp(reinterpret_cast<char*>(val_vunknown1->domain_id),
                reinterpret_cast<char*>(val_vunknown2->domain_id)))
      val_vunknown1->valid[UPLL_IDX_DOMAIN_ID_VUN] = UNC_VF_INVALID;
  }
  for (unsigned int loop = 0;
      loop < sizeof(val_vunknown1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vunknown1->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vunknown1->valid[loop])) {
        invalid_attr = false;
        break;
    }
  }
  return invalid_attr;
}

upll_rc_t VunknownMoMgr::CreateVnodeConfigKey(ConfigKeyVal *ikey,
                                              ConfigKeyVal *&okey) {
  if (ikey == NULL) return UPLL_RC_ERR_GENERIC;

  key_vunknown * temp_key_vunknown = reinterpret_cast<key_vunknown *>
                                      (malloc(sizeof(key_vunknown)));
if (!temp_key_vunknown) {
    UPLL_LOG_ERROR("temp_key_vunknown Memory allocation failed");
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(temp_key_vunknown->vtn_key.vtn_name,
         reinterpret_cast<key_vunknown*>(ikey->get_key())->vtn_key.vtn_name,
          (kMaxLenVtnName+1) );
  uuu::upll_strncpy(temp_key_vunknown->vunknown_name,
         reinterpret_cast<key_vunknown*>(ikey->get_key())->vunknown_name,
          (kMaxLenVtnName+1));

  ConfigKeyVal *ck_vunknown = new ConfigKeyVal(UNC_KT_VUNKNOWN,
                                   IpctSt::kIpcStKeyVunknown,
                                   reinterpret_cast<void *>(temp_key_vunknown),
                                   NULL);
  if (ck_vunknown == NULL) return UPLL_RC_ERR_GENERIC;
  okey = ck_vunknown;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunknownMoMgr::IsReferenced(IpcReqRespHeader *req,
                                      ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                                (GetMoManager(UNC_KT_VUNK_IF)));
  if (mgr == NULL) return UPLL_RC_ERR_GENERIC;
  result_code = mgr->IsReferenced(req, ikey, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code (%d)",
           result_code);
    return result_code;
  }
  UPLL_LOG_DEBUG("IsReferenced result code (%d)", result_code);
  return UPLL_RC_SUCCESS;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc

