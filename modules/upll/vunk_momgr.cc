/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vunk_momgr.hh"
#include "upll_log.hh"
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
  table = new Table *[ntable];
  table[MAINTBL] = new Table(uudst::kDbiVunknownTbl, UNC_KT_VUNKNOWN,
                         vunk_bind_info, IpctSt::kIpcStKeyVunknown,
                         IpctSt::kIpcStValVunknown,
                         uudst::vunknown::kDbiVunknownNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
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

bool VunknownMoMgr::IsValidKey(void *key, uint64_t index) {
  UPLL_FUNC_TRACE;
  key_vunknown *vunk_key = reinterpret_cast<key_vunknown *>(key);
  bool ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vunknown::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vunk_key->vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        pfc_log_info("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vunknown::kDbiVunknownName:
      ret_val = ValidateKey(reinterpret_cast<char *>(vunk_key->vunknown_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        pfc_log_info("Vunknown name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      pfc_log_info("Invalid Key Index");
      break;
  }
  pfc_log_info("Leaving IsValidKey");
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

  if (tmp) {
    if (tbl == MAINTBL) {
      val_vunknown *ival = reinterpret_cast<val_vunknown *>(GetVal(req));
      if (!ival) {
        UPLL_LOG_DEBUG("Invalid vunknown value\n");
        return UPLL_RC_ERR_GENERIC;
      }
      val_vunknown *vunk_val = reinterpret_cast<val_vunknown *>
                                          (malloc(sizeof(val_vunknown)));
      if (!vunk_val) return UPLL_RC_ERR_GENERIC;
      memcpy(vunk_val, ival, sizeof(val_vunknown));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVunknown, vunk_val);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vunknown *ikey = reinterpret_cast<key_vunknown *>(tkey);
  key_vunknown *vunk_key = reinterpret_cast<key_vunknown *>
                                      (malloc(sizeof(key_vunknown)));
  if (!vunk_key) { delete tmp1; return UPLL_RC_ERR_GENERIC;}
  memcpy(vunk_key, ikey, sizeof(key_vunknown));
  okey = new ConfigKeyVal(UNC_KT_VUNKNOWN, IpctSt::kIpcStKeyVunknown, vunk_key,
                          tmp1);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunknownMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status, uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running) {
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
  unc_keytype_configstatus_t cs_status = UNC_CS_NOT_APPLIED;
  vunk_val = reinterpret_cast<val_vunknown *>(GetVal(vunk_key));
  if (vunk_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    vunk_val->cs_row_status = cs_status;
  } else if (op != UNC_OP_UPDATE) {
     return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
       loop < sizeof(vunk_val->valid) / sizeof(vunk_val->valid[0]); ++loop) {
    if ((UNC_VF_VALID == vunk_val->valid[loop])
          || (UNC_VF_VALID_NO_VALUE == vunk_val->valid[loop]))
      vunk_val->cs_attr[loop] =  vunk_val->cs_row_status;
    else if (vunk_val->valid[loop] != UNC_VF_NOT_SOPPORTED)
      vunk_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VunknownMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                         ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>
                                                              (ikey->get_key());
  key_vunknown_t *key_vunk = reinterpret_cast<key_vunknown_t *>
                                               (malloc(sizeof(key_vunknown_t)));
  if (!key_vunk) return UPLL_RC_ERR_GENERIC;
  memset(key_vunk, 0, sizeof(key_vunknown_t));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    free(key_vunk);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vunk->vtn_key.vtn_name,
         key_rename->old_unc_vtn_name, (kMaxLenVtnName+1));
  okey = new ConfigKeyVal(UNC_KT_VUNKNOWN, IpctSt::kIpcStKeyVunknown, key_vunk,
                          NULL);
  if (!okey) {
    free(key_vunk);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}
upll_rc_t VunknownMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (NULL == req) {
    UPLL_LOG_DEBUG("IpcReqRespHeader is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVunknown) {
    pfc_log_debug(
        "Invalid structure received.Expected struct-kIpcStKeyVunknown"
        "received struct -%s ",
      reinterpret_cast<const char *>(IpctSt::GetIpcStdef(ikey->get_st_num())));
    return UPLL_RC_ERR_GENERIC;
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
    pfc_log_debug("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_GENERIC;
  }
  if (key_vunknown == NULL) {
    pfc_log_debug("KT_VUNKNOWN Key structure is empty!!");
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }

  ret_val = ValidateVunknownKey(key_vunknown, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    pfc_log_debug("Syntax check failed for KT_VUNKNOWN key structure");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE))
      && (dt_type == UPLL_DT_CANDIDATE)) {
    if (val_vunknown == NULL) {
      pfc_log_debug("Value structure mandatory for CREATE/UPDATE");
      return UPLL_RC_ERR_CFG_SYNTAX;
    } else {
      ret_val = ValidateVunknownValue(val_vunknown, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        pfc_log_debug("Syntax check failed for KT_VUNKNOWN value structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      pfc_log_trace("Syntax check success for KT_VUNKNOWN value structure");
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
            pfc_log_debug("Syntax check failed for Vunknown value structure");
            return UPLL_RC_ERR_CFG_SYNTAX;
          }
          pfc_log_trace("Validation success for KT_VUNKNOWN value structure");
          return UPLL_RC_SUCCESS;
        } else {
          pfc_log_trace("Value structure is optional for READ");
          return UPLL_RC_SUCCESS;
        }
      } else {
        pfc_log_debug("option2 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
    } else {
      pfc_log_debug("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
  } else if ((operation == UNC_OP_DELETE) || (operation == UNC_OP_READ_NEXT)
             || (operation == UNC_OP_READ_BULK)) {
    pfc_log_trace("Value structure is none for operation type:%d", operation);
    return UPLL_RC_SUCCESS;
  } else {
    pfc_log_debug("Invalid datatype(%d) and operation(%d)", dt_type,
                  operation);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
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
    pfc_log_debug("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }
  ret_val = objvtnmgr->ValidateVtnKey(&(key_vunknown->vtn_key));

  if (UPLL_RC_SUCCESS != ret_val) {
    pfc_log_debug("Vtn_name syntax validation failed.Err Code- %d", ret_val);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    pfc_log_trace("KT_VUNKNOWN:vunknown_name (%s)", key_vunknown->vunknown_name);
    ret_val = ValidateKey(reinterpret_cast<char *>(key_vunknown->vunknown_name),
        kMinLenVnodeName, kMaxLenVnodeName);

    if (ret_val != UPLL_RC_SUCCESS) {
      pfc_log_debug("Validation failure for key_vunknown->vunknown_name");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(key_vunknown->vunknown_name);
  }
  pfc_log_trace("key structure validation successful for vunknown keytype");
  return UPLL_RC_SUCCESS;
}
upll_rc_t VunknownMoMgr::ValidateVunknownValue(val_vunknown_t *val_vunknown,
                                               unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val = UPLL_RC_SUCCESS;

  if (val_vunknown->valid[UPLL_IDX_CONTROLLER_ID_VUN] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(val_vunknown->controller_id),
                           kMinLenCtrlrId,
                           kMaxLenCtrlrId);
    if (ret_val != UPLL_RC_SUCCESS) {
      pfc_log_debug("controller_id syntax check failed."
                    "Received controller_id - %s",
                    val_vunknown->controller_id);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (val_vunknown->valid[UPLL_IDX_DOMAIN_ID_VUN] == UNC_VF_VALID) {
    ret_val = ValidateDefaultStr(reinterpret_cast<char *>(
                          val_vunknown->domain_id),
                          kMinLenDomainId,
                          kMaxLenDomainId);
    if (ret_val != UPLL_RC_SUCCESS) {
      pfc_log_debug("Domain_id syntax check failed."
                    "Received Domain_id - %s",
                    val_vunknown->domain_id);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (val_vunknown->valid[UPLL_IDX_DESC_VUN] == UNC_VF_VALID) {
    ret_val = ValidateDesc(reinterpret_cast<char *>(val_vunknown->description),
                           kMinLenDescription, kMaxLenDescription);
    if (ret_val != UPLL_RC_SUCCESS) {
      pfc_log_debug("Syntax check failed.desc - %s", val_vunknown->description);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vunknown->valid[UPLL_IDX_DESC_VUN] == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vunknown->description[0] = '\0';
  }
  if (val_vunknown->valid[UPLL_IDX_TYPE_VUN] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_vunknown->type,
                                   (uint8_t) VUNKNOWN_TYPE_BRIDGE,
                                  (uint8_t) VUNKNOWN_TYPE_ROUTER, true, true)) {
      pfc_log_debug("Syntax check failed.Type- %d", val_vunknown->type);
      return UPLL_RC_ERR_CFG_SYNTAX;
    } 
  } else if ((val_vunknown->valid[UPLL_IDX_TYPE_VUN] == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vunknown->type = VUNKNOWN_TYPE_BRIDGE;
  }
  pfc_log_trace("value structure validation successful for vunknown keytype");
  return UPLL_RC_SUCCESS;
}
upll_rc_t VunknownMoMgr::ValVunknownAttributeSupportCheck(
    const char * crtlr_name, ConfigKeyVal *ikey, uint32_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val = false;
  uint32_t instance_count = 0;
  uint32_t num_attrs = 0;
  const uint8_t *attrs = 0;
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vunknown_t *val_vunknown = NULL;
  if ((ikey->get_cfg_val())
      && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVunknown)) {
    val_vunknown = reinterpret_cast<val_vunknown_t *>(GetVal(ikey));
  }
  if (val_vunknown != NULL) {
    switch (operation) {
      case UNC_OP_CREATE:
        ret_val = GetCreateCapability(crtlr_name, ikey->get_key_type(),
                                      &instance_count, &num_attrs, &attrs);
        break;

      case UNC_OP_UPDATE:
        ret_val = GetUpdateCapability(crtlr_name, ikey->get_key_type(),
                                      &num_attrs, &attrs);
        break;

      case UNC_OP_READ:
      case UNC_OP_READ_SIBLING:
      case UNC_OP_READ_SIBLING_BEGIN:
      case UNC_OP_READ_SIBLING_COUNT:
        ret_val = GetReadCapability(crtlr_name, ikey->get_key_type(),
                                    &num_attrs, &attrs);
        break;

      default:
        pfc_log_debug("Invalid operation");
        break;
    }

    if (!ret_val) {
      pfc_log_debug("key_type - %d is not supported by controller - %s",
                    ikey->get_key_type(), crtlr_name);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
    }
    if ((val_vunknown->valid[UPLL_IDX_DESC_VUN] == UNC_VF_VALID)
        || (val_vunknown->valid[UPLL_IDX_DESC_VUN] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vunknown::kCapDesc] == 0) {
        val_vunknown->valid[UPLL_IDX_DESC_VUN] = UNC_VF_NOT_SOPPORTED;
        pfc_log_trace("UPLL_IDX_DESC_VUN not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((val_vunknown->valid[UPLL_IDX_TYPE_VUN] == UNC_VF_VALID)
        || (val_vunknown->valid[UPLL_IDX_TYPE_VUN] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vunknown::kCapType] == 0) {
        val_vunknown->valid[UPLL_IDX_TYPE_VUN] = UNC_VF_NOT_SOPPORTED;
        pfc_log_debug("UPLL_IDX_TYPE_VUN  not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  } else {
    pfc_log_debug("value structure is empty");
    return UPLL_RC_ERR_GENERIC;
  }
  pfc_log_info("Exiting KT_VUNKOWN: ValVunknownAttributeSupportCheck()");
  return UPLL_RC_SUCCESS;
}
upll_rc_t VunknownMoMgr::ValidateCapability(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, const char* ctrlr_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (NULL == req) {
    UPLL_LOG_DEBUG("IpcReqRespHeader is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  /* controller name */
  if (!ctrlr_name) {
    ctrlr_name = reinterpret_cast<char *>(ikey->get_user_data());
    pfc_log_trace("controller Name :(%s)", ctrlr_name);
  }
  pfc_log_trace("controller Name :(%s)", ctrlr_name);

  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;
  pfc_log_trace("dt_type   : (%d)"
                "operation : (%d)"
                "option1   : (%d)"
                "option2   : (%d)",
                dt_type, operation, option1, option2);
  if (((operation == UNC_OP_CREATE) || (operation == UNC_OP_UPDATE))
      && (dt_type == UPLL_DT_CANDIDATE)) {
    if (GetVal(ikey) != NULL) {
      ret_val = ValVunknownAttributeSupportCheck(ctrlr_name, ikey, operation);
      if (ret_val == UPLL_RC_SUCCESS) {
        pfc_log_trace("Attribute check success for KT_VUNKOWN value structure");
        return UPLL_RC_SUCCESS;
      } else {
        pfc_log_debug("Attribute check failed for KT_VUNKOWN value structure");
        return UPLL_RC_ERR_GENERIC;
      }
    } else {
      pfc_log_debug("Value structure mandatory for the UPDATE/CREATE");
      return UPLL_RC_ERR_GENERIC;
    }
  } else if (((operation == UNC_OP_READ) || (operation == UNC_OP_READ_SIBLING)
      || (operation == UNC_OP_READ_SIBLING_BEGIN)
      || (operation == UNC_OP_READ_SIBLING_COUNT))
      && ((dt_type == UPLL_DT_CANDIDATE) || (dt_type == UPLL_DT_RUNNING)
          || (dt_type == UPLL_DT_STARTUP) || (dt_type == UPLL_DT_STATE))) {
    if (option1 == UNC_OPT1_NORMAL) {
      if (option2 == UNC_OPT2_NONE) {
        if (GetVal(ikey) != NULL) {
          ret_val = ValVunknownAttributeSupportCheck(ctrlr_name, ikey,
                                                     operation);
          if (ret_val == UPLL_RC_SUCCESS) {
            pfc_log_trace("Attribute check success for VUNKOWN value struct");
            return UPLL_RC_SUCCESS;
          } else {
            pfc_log_debug("Attribute check failed for KT_VUNKOWN value struct");
            return UPLL_RC_ERR_GENERIC;
          }
        } else {
          pfc_log_trace("Value structure is optional for READ");
          return UPLL_RC_SUCCESS;
        }
      } else {
        pfc_log_debug("option2 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
    } else {
      pfc_log_debug("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
  } else if ((operation == UNC_OP_DELETE) || (operation == UNC_OP_READ_NEXT)
      || (operation == UNC_OP_READ_BULK)) {
    pfc_log_trace("Value structure is none for operation type:%d", operation);
    return UPLL_RC_SUCCESS;
  } else {
    pfc_log_debug("Invalid datatype(%d) and operation(%d)", dt_type, operation);
    return UPLL_RC_ERR_GENERIC;
  }
}

bool VunknownMoMgr::CompareValidValue(void *&val1, void *val2, bool audit) {
  return false;
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

upll_rc_t VunknownMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi) {
  return UPLL_RC_SUCCESS;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc

