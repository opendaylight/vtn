/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <set>
#include <sstream>
#include "policingprofile_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "vtn_policingmap_momgr.hh"
#include "vbr_policingmap_momgr.hh"
#include "vbr_if_policingmap_momgr.hh"
#include "vterm_if_policingmap_momgr.hh"
#include "uncxx/upll_log.hh"
#include "upll_db_query.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

#define  NUM_KEY_MAIN_COL 3
#define  NUM_KEY_CTRL_COL 3
#define  NUM_KEY_RENAME_COL 2
#define  POLICINGPROFILE_RENAME 0x01

/* This file declares interfaces for keyType KT_POLICING_PROFILE */
/**
 * @brief PolicingProfileMoMgr class handles all the request
 *        received from service.
 */

// PolicingProfile Table(Main Table)
BindInfo PolicingProfileMoMgr::policingprofile_bind_info[] = {
  { uudst::policingprofile::kDbiPolicingProfileName,
    CFG_KEY,
    offsetof(key_policingprofile_t, policingprofile_name),
    uud::kDalChar,
    kMaxLenPolicingProfileName + 1 },
  { uudst::policingprofile::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1},
  { uudst::policingprofile::kDbiCsRowStatus,
    CS_VAL,
    offsetof(val_policingprofile_t, cs_row_status),
    uud::kDalUint8, 1 }
};

// PolicingProfile Rename Table
BindInfo PolicingProfileMoMgr::policingprofile_rename_bind_info[] = {
  { uudst::policingprofile_rename::kDbiPolicingProfileName,
    CFG_KEY,
    offsetof(key_policingprofile_t, policingprofile_name),
    uud::kDalChar,
    (kMaxLenPolicingProfileName + 1) },
  { uudst::policingprofile_rename::kDbiCtrlrName,
    CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar,
    (kMaxLenCtrlrId + 1) },
  { uudst::policingprofile_rename::kDbiPolicingProfileNameCtrlr,
    CFG_VAL,
    offsetof(val_rename_policingprofile_t, policingprofile_newname),
    uud::kDalChar,
    (kMaxLenPolicingProfileName+1) }
};

// PolicingProfileController Table
BindInfo PolicingProfileMoMgr::policingprofile_controller_bind_info[] = {
  { uudst::policingprofile_ctrlr::kDbiPolicingProfileName,
    CFG_KEY,
    offsetof(key_policingprofile_t, policingprofile_name),
    uud::kDalChar, (kMaxLenPolicingProfileName + 1) },
  { uudst::policingprofile_ctrlr::kDbiCtrlrName,
    CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::policingprofile_ctrlr::kDbiRefCount,
    CFG_VAL,
    offsetof(val_policingprofile_ctrl_t, ref_count),
    uud::kDalUint32, 1 },
  { uudst::policingprofile_ctrlr::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1},
  { uudst::policingprofile_ctrlr::kDbiCsRowStatus,
    CS_VAL,
    offsetof(val_policingprofile_ctrl_t, cs_row_status),
    uud::kDalUint8, 1 },
};

BindInfo PolicingProfileMoMgr::rename_policingprofile_main_tbl[] = {
  { uudst::policingprofile::kDbiPolicingProfileName,
    CFG_MATCH_KEY,
    offsetof(key_policingprofile_t, policingprofile_name),
    uud::kDalChar,
    (kMaxLenPolicingProfileName + 1) },
  { uudst::policingprofile::kDbiPolicingProfileName,
    CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_policingprofile_name),
    uud::kDalChar,
    (kMaxLenPolicingProfileName + 1) },
  { uudst::policingprofile::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

BindInfo PolicingProfileMoMgr::rename_policingprofile_ctrlr_tbl[] = {
  { uudst::policingprofile_ctrlr::kDbiPolicingProfileName,
    CFG_MATCH_KEY,
    offsetof(key_policingprofile_t, policingprofile_name),
    uud::kDalChar,
    (kMaxLenPolicingProfileName + 1) },
  { uudst::policingprofile_ctrlr::kDbiPolicingProfileName,
    CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_policingprofile_name),
    uud::kDalChar,
    (kMaxLenPolicingProfileName + 1) },
  { uudst::policingprofile_ctrlr::kDbiFlags,
    CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

BindInfo PolicingProfileMoMgr::rename_policingprofile_rename_tbl[] = {
  { uudst::policingprofile_rename::kDbiPolicingProfileName,
    CFG_MATCH_KEY,
    offsetof(key_policingprofile_t, policingprofile_name),
    uud::kDalChar,
    (kMaxLenPolicingProfileName + 1) },
  { uudst::policingprofile_rename::kDbiPolicingProfileName,
    CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_policingprofile_name),
    uud::kDalChar,
    (kMaxLenPolicingProfileName + 1) }
};

unc_key_type_t PolicingProfileMoMgr::policingprofile_child[] = {
  UNC_KT_POLICING_PROFILE_ENTRY,
};

bool PolicingProfileMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
    BindInfo *&binfo, int &nattr, MoMgrTables tbl) {
  switch (key_type) {
    case UNC_KT_POLICING_PROFILE:
      if (MAINTBL == tbl) {
        nattr = NUM_KEY_MAIN_COL;
        binfo = rename_policingprofile_main_tbl;
      } else if (CTRLRTBL == tbl) {
        nattr = NUM_KEY_CTRL_COL;
        binfo = rename_policingprofile_ctrlr_tbl;
      } else {
        nattr = NUM_KEY_RENAME_COL;
        binfo = rename_policingprofile_rename_tbl;
      }
      break;
    default:
      return PFC_FALSE;
  }
  return PFC_TRUE;
}

/**
 * @brief  PolicingProfileMoMgr Class Constructor.
 */
PolicingProfileMoMgr::PolicingProfileMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();

  table[MAINTBL] = new Table(uudst::kDbiPolicingProfileTbl,
      UNC_KT_POLICING_PROFILE, policingprofile_bind_info,
      IpctSt::kIpcStKeyPolicingprofile, IpctSt::kIpcStValPolicingprofile,
      uudst::policingprofile::kDbiPolicingProfileNumCols);

  table[RENAMETBL] = new Table(uudst::kDbiPolicingProfileRenameTbl,
      UNC_KT_POLICING_PROFILE, policingprofile_rename_bind_info,
      IpctSt::kIpcStKeyPolicingprofile, IpctSt::kIpcStValRenamePolicingprofile,
      uudst::policingprofile_rename::kDbiPolicingProfileRenameNumCols);

  table[CTRLRTBL] = new Table(uudst::kDbiPolicingProfileCtrlrTbl,
      UNC_KT_POLICING_PROFILE, policingprofile_controller_bind_info,
      IpctSt::kIpcStKeyPolicingprofile, IpctSt::kIpcInvalidStNum,
      uudst::policingprofile_ctrlr::kDbiPolicingProfileCtrlrNumCols);

  table[CONVERTTBL] = NULL;

  nchild = sizeof(policingprofile_child) / sizeof(policingprofile_child[0]);
  child = policingprofile_child;
}

/**
  @brief  PolicingProfileMoMgr Class Destructor.
 */
PolicingProfileMoMgr::~PolicingProfileMoMgr() {
  for (int i = 0 ; i < ntable; i++) {
    if (table[i]) {
      delete table[i];
    }
  }
  delete[] table;
}

upll_rc_t PolicingProfileMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                  DalDmlIntf *dmi,
                                                  IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_key_type() != UNC_KT_POLICING_PROFILE)
    return UPLL_RC_ERR_GENERIC;
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::GetValid(void *val, uint64_t indx,
    uint8_t *&valid,
    upll_keytype_datatype_t dt_type,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::Value structure is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }
  if (tbl == RENAMETBL) {
    val_rename_policingprofile_t *val_rename =
      reinterpret_cast<val_rename_policingprofile_t *> (val);
    valid = &val_rename->valid[UPLL_IDX_RENAME_PROFILE_RPP];
  } else if (tbl == CTRLRTBL) {
    val_policingprofile_ctrl_t *val_ctrl =
      reinterpret_cast<val_policingprofile_ctrl_t *>(val);
    if (uudst::policingprofile_ctrlr::kDbiRefCount == indx) {
      valid = &val_ctrl->valid[0];
    }
  } else {
    valid = NULL;
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr:: Invalid tbl");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE(" PolicingProfileMoMgr:: GetValid is Successful");
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::AllocVal(ConfigVal *&ck_val,
    upll_keytype_datatype_t dt_type,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;  // *ck_nxtval;
  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_policingprofile_t)));
      ck_val = new ConfigVal(IpctSt::kIpcStValPolicingprofile, val);
      UPLL_LOG_DEBUG(" PolicingProfileMoMgr::Value Allocation successful"
                     " for PolicingProfile MainTbl ");
      break;
    case RENAMETBL:
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile_t)));
      ck_val = new ConfigVal(IpctSt::kIpcStValRenamePolicingprofile, val);
      UPLL_LOG_DEBUG(" PolicingProfileMoMgr::Value Allocation successful"
                     " for PolicingProfile RenameTbl ");
      break;
    case CTRLRTBL:
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_policingprofile_ctrl_t)));
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
      UPLL_LOG_DEBUG(" PolicingProfileMoMgr::Value Allocation successful"
                     " for PolicingProfile CtrlrTbl ");
      break;
    default:
      UPLL_LOG_DEBUG(" PolicingProfileMoMgr::Invalid Tbl");
      return UPLL_RC_ERR_GENERIC;
  }
  if (NULL == val) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::Value Allocation Failed");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                  ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  key_policingprofile_t *policingprofile_key = NULL;
  void *pkey = NULL;

  if (parent_key == NULL) {
    policingprofile_key = reinterpret_cast<key_policingprofile_t *>
        (ConfigKeyVal::Malloc(sizeof(key_policingprofile_t)));

    okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
                            IpctSt::kIpcStKeyPolicingprofile,
                            policingprofile_key, NULL);
    UPLL_LOG_TRACE(" PolicingProfileMoMgr::ConfigKeyVal Allocation "
                   " Successful");
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }

  if (NULL == pkey) {
    UPLL_LOG_TRACE("Key structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey) {
    if (okey->get_key_type() != UNC_KT_POLICING_PROFILE)
      return UPLL_RC_ERR_GENERIC;
  }
  if ((okey) && (okey->get_key())) {
    policingprofile_key = reinterpret_cast<key_policingprofile_t *>
          (okey->get_key());
  } else {
    policingprofile_key = reinterpret_cast<key_policingprofile_t *>
        (ConfigKeyVal::Malloc(sizeof(key_policingprofile_t)));
  }

  switch (parent_key->get_key_type()) {
    case UNC_KT_ROOT:
      break;
    case UNC_KT_POLICING_PROFILE:
      uuu::upll_strncpy(policingprofile_key->policingprofile_name,
                        (reinterpret_cast<key_policingprofile_t*>
                         (pkey)->policingprofile_name),
                        (kMaxLenPolicingProfileName + 1));
      break;
    default:
      if (policingprofile_key) free(policingprofile_key);
      return UPLL_RC_ERR_GENERIC;
  }
  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_TRACE("okey not NULL profile name updated");
    okey->SetKey(IpctSt::kIpcStKeyPolicingprofile, policingprofile_key);
  }
  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
                            IpctSt::kIpcStKeyPolicingprofile,
                            policingprofile_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_TRACE("%s GetChildConfigKey fl start",
                  okey->ToStrAll().c_str());
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::GetRenamedUncKey(
    ConfigKeyVal *ctrlr_key, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  UPLL_LOG_TRACE("%s GetRenamedUncKey pp start",
                  ctrlr_key->ToStrAll().c_str());
  if ((NULL == ctrlr_key) || (NULL == dmi)) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::Input ConfigKeyVal is NULL.");
    return result_code;
  }
  ConfigKeyVal *unc_key = NULL;
  key_policingprofile_t *ctrlr_policingprofile_key =
    reinterpret_cast<key_policingprofile_t *>(ctrlr_key->get_key());
  if (NULL == ctrlr_policingprofile_key) {
    UPLL_LOG_DEBUG("PolicingProfileMoMgr::Key struct is NULL");
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };

  val_rename_policingprofile_t *rename_policingprofile =
  reinterpret_cast <val_rename_policingprofile_t *>
  (ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile_t)));
  if (!rename_policingprofile) {
    UPLL_LOG_DEBUG("rename_policingprofile NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(rename_policingprofile->policingprofile_newname,
                    ctrlr_policingprofile_key->policingprofile_name,
                    (kMaxLenPolicingProfileName+1));
  rename_policingprofile->valid[UPLL_IDX_RENAME_PROFILE_RPP] = UNC_VF_VALID;

  result_code = GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed (%d)", result_code);
    free(rename_policingprofile);
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    free(rename_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenamePolicingprofile,
      rename_policingprofile);
  UPLL_LOG_DEBUG("ctrlr_id pp (%s)", ctrlr_id);
  if (ctrlr_id) {
    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  } else {
    dbop.matchop = kOpMatchNone;
  }
  dbop.inoutop = kOpInOutCtrlr;
  result_code = ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_policingprofile_t *policingprofile_key =
      reinterpret_cast<key_policingprofile_t *>(unc_key->get_key());
    uuu::upll_strncpy(ctrlr_policingprofile_key->policingprofile_name,
                      policingprofile_key->policingprofile_name,
                      (kMaxLenPolicingProfileName+1));
    SET_USER_DATA(ctrlr_key, unc_key);
    SET_USER_DATA_FLAGS(ctrlr_key, PP_RENAME);
  }
  UPLL_LOG_TRACE("%s GetRenamedUncKey pp end",
                  ctrlr_key->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || (NULL == dmi)) {
    UPLL_LOG_DEBUG("Input ConfigKeyVal is NULL");
    return result_code;
  }

  ConfigKeyVal *okey = NULL;
  UPLL_LOG_TRACE("%s GetRenamedCtrl pp start", (ikey->ToStrAll()).c_str());

  /* PolicingProfile renamed */
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail (%d)", result_code);
    return result_code;
  }

  if (NULL != ctrlr_dom) {
    SET_USER_DATA_CTRLR(okey, ctrlr_dom->ctrlr);
  } else {
    UPLL_LOG_DEBUG("ctrlr null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL); /* ctrlr_name */
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB no instance");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("ReadConfigDB failed (%d)", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  val_rename_policingprofile_t *rename_val =
    reinterpret_cast<val_rename_policingprofile_t *>(GetVal(okey));
  if (NULL == rename_val) {
    UPLL_LOG_DEBUG("memory Allocation failed for rename val struct");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(
      reinterpret_cast<key_policingprofile_t *>
      (ikey->get_key())->policingprofile_name,
      rename_val->policingprofile_newname,
      (kMaxLenPolicingProfileName+1));
  UPLL_LOG_DEBUG("profile name (%s) (%s)",
      reinterpret_cast<key_policingprofile_t *>
      (ikey->get_key())->policingprofile_name,
      rename_val->policingprofile_newname);
  DELETE_IF_NOT_NULL(okey);
  UPLL_LOG_TRACE("%s GetRenamedCtrl pp end", (ikey->ToStrAll()).c_str());
  UPLL_LOG_TRACE(" PolicingProfileMOMgr::GetRenameUncKey Successful");
  return UPLL_RC_SUCCESS;
}


upll_rc_t PolicingProfileMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
    ConfigKeyVal *&req,
    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::DupConfigKeyVal Failed."
      " Input ConfigKeyVal is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::DupConfigKeyVal Failed."
      " Output ConfigKeyVal is not NULL ");
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->get_key_type() != UNC_KT_POLICING_PROFILE) {
    UPLL_LOG_DEBUG("PolicingProfileMoMgr::Invalid Key type");
    return UPLL_RC_ERR_GENERIC;
  }
  if (NULL == (req->get_key())) {
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_policingprofile_t *ival =
        reinterpret_cast<val_policingprofile_t *>(GetVal(req));
      if (NULL != ival) {
        val_policingprofile_t *policingprofile_val =
           reinterpret_cast<val_policingprofile_t *>
           (ConfigKeyVal::Malloc(sizeof(val_policingprofile_t)));
        memcpy(policingprofile_val, ival,
          sizeof(val_policingprofile_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValPolicingprofile,
            policingprofile_val);
      }
    } else if (tbl == RENAMETBL) {
      val_rename_policingprofile_t *ival =
        reinterpret_cast<val_rename_policingprofile_t *> (GetVal(req));
      if (NULL != ival) {
        val_rename_policingprofile_t *rename_val =
          reinterpret_cast<val_rename_policingprofile_t *>
          (ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile_t)));
        memcpy(rename_val, ival, sizeof(val_rename_policingprofile_t));
        tmp1 =
            new ConfigVal(IpctSt::kIpcStValRenamePolicingprofile, rename_val);
      }
    } else if (tbl == CTRLRTBL) {
      val_policingprofile_ctrl_t *ival =
          reinterpret_cast<val_policingprofile_ctrl_t *>(GetVal(req));
      if (NULL != ival) {
        val_policingprofile_ctrl_t *ctrl_val =
          reinterpret_cast<val_policingprofile_ctrl_t *>
            (ConfigKeyVal::Malloc(sizeof(val_policingprofile_ctrl_t)));
        memcpy(ctrl_val, ival, sizeof(val_policingprofile_ctrl_t));
        tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum,
                             ctrl_val);
      }
    }
    if (NULL == tmp1) {
      UPLL_LOG_DEBUG(" PolicingProfileMoMgr::DupConfigKeyVal Failed.");
      UPLL_LOG_DEBUG(" Value allocation failed");
      return UPLL_RC_ERR_GENERIC;
    }
    tmp1->set_user_data(tmp->get_user_data());
  }
  key_policingprofile_t *tkey = reinterpret_cast<key_policingprofile_t *>
                                (req->get_key());
  key_policingprofile_t *policingprofile_key =
    reinterpret_cast<key_policingprofile_t *>
      (ConfigKeyVal::Malloc(sizeof(key_policingprofile_t)));
  memcpy(policingprofile_key, reinterpret_cast<key_policingprofile_t *>(tkey),
      sizeof(key_policingprofile_t));
  okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
      IpctSt::kIpcStKeyPolicingprofile, policingprofile_key,
      tmp1);
  SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}

pfc_bool_t PolicingProfileMoMgr::CompareKey(ConfigKeyVal *key1,
    ConfigKeyVal *key2) {
  UPLL_FUNC_TRACE;
  bool match = false;
  if (NULL == key1 || NULL == key2) {
    return match;
  }
  if (key1->get_key_type() != UNC_KT_POLICING_PROFILE) {
    UPLL_LOG_DEBUG("PolicingProfileMoMgr::Invalid Key type");
    return match;
  }
  key_policingprofile_t *policingprofile_key1, *policingprofile_key2;
  policingprofile_key1 =
    reinterpret_cast<key_policingprofile_t *>(key1->get_key());
  policingprofile_key2 =
    reinterpret_cast<key_policingprofile_t *>(key2->get_key());

  if (NULL == policingprofile_key1 || NULL == policingprofile_key2) {
    return false;
  }

  if (strcmp(reinterpret_cast<const char *>
        (policingprofile_key1->policingprofile_name),
        reinterpret_cast<const char *>
        (policingprofile_key2->policingprofile_name)) == 0) {
    match = true;
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::CompareKey .Both Keys are same");
  }
  return match;
}

upll_rc_t PolicingProfileMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingprofile_ctrl_t *val = NULL;
  val = (ckv_running != NULL)?reinterpret_cast
       <val_policingprofile_ctrl_t *>
      (GetVal(ckv_running)):NULL;
  if (NULL == val) {
    UPLL_LOG_ERROR("val structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::MergeValidate(unc_key_type_t keytype,
    const char *ctrlr_id, ConfigKeyVal *okey,
    DalDmlIntf *dmi,
    upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *ckval = NULL;
  if (NULL == ctrlr_id) {
    UPLL_LOG_DEBUG("ctrlr_id NULL");
    return result_code;
  }
  result_code = GetChildConfigKey(ckval, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ckval fail");
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(ckval, UPLL_DT_IMPORT,
              UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(ckval);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("ReadConfigDB fail");
      return result_code;
    }
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *tmp_ckval = ckval;
  while (NULL != ckval) {
    // Check the profile is stand alone
    DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
    // ckval has rename val set, so removing  that to read from ctrlr tbl
    ckval->SetCfgVal(NULL);
    result_code = UpdateConfigDB(ckval,
                                 UPLL_DT_IMPORT,
                                 UNC_OP_READ, dmi,
                                 &dbop, CTRLRTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_ERROR("profile name is stand alone");
      result_code = GetChildConfigKey(okey, ckval);
      DELETE_IF_NOT_NULL(tmp_ckval);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetChildConfigKey fail");
        return result_code;
      }
      return UPLL_RC_ERR_MERGE_CONFLICT;
    } else if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("Database Error");
      DELETE_IF_NOT_NULL(tmp_ckval);
      return result_code;
    }
    ckval = ckval->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(tmp_ckval);
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::SwapKeyVal(ConfigKeyVal *ikey,
    ConfigKeyVal *&okey,
    uud::DalDmlIntf *dmi,
    uint8_t *ctrlr, bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !(ikey->get_key()) || !(strlen(reinterpret_cast<const char *>
     (ctrlr)))) {
    UPLL_LOG_DEBUG("Input ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *cfg_val = ikey->get_cfg_val();
  if (NULL == cfg_val) {
    UPLL_LOG_DEBUG("Value struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  val_rename_policingprofile_t *tval =
    reinterpret_cast<val_rename_policingprofile_t *>(cfg_val->get_val());
  if (NULL == tval) {
    UPLL_LOG_DEBUG("Rename val struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  key_policingprofile_t *key_policingprofile =
      reinterpret_cast<key_policingprofile_t *>(ConfigKeyVal::Malloc
      (sizeof(key_policingprofile_t)));

  if (UNC_VF_VALID_NO_VALUE ==
             tval->valid[UPLL_IDX_RENAME_PROFILE_RPP]) {
    no_rename = true;
    uuu::upll_strncpy(key_policingprofile->policingprofile_name,
                      reinterpret_cast<key_policingprofile_t *>
                      (ikey->get_key())->policingprofile_name,
                      (kMaxLenPolicingProfileName+1));
    UPLL_LOG_DEBUG("No Rename Operation %d", no_rename);
  } else {
     if (UNC_VF_VALID == tval->valid[UPLL_IDX_RENAME_PROFILE_RPP]) {
      /* checking the string is empty or not*/
      if (!strlen(reinterpret_cast<char *>(tval->policingprofile_newname))) {
        UPLL_LOG_DEBUG(" PolicingProfile_newname is NULL");
        free(key_policingprofile);
        return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_policingprofile->policingprofile_name,
                      tval->policingprofile_newname,
                      (kMaxLenPolicingProfileName+1));
    //  copy the new UNC name to KeyVtn
    /* The New Name and PFC name should not be same name */
    if (!strcmp(reinterpret_cast<char *>
       ((reinterpret_cast<key_policingprofile_t *>(ikey->get_key()))
         ->policingprofile_name), reinterpret_cast<char *>
        (tval->policingprofile_newname))) {
        UPLL_LOG_DEBUG("ctrl , new name is same");
        free(key_policingprofile);
         return UPLL_RC_ERR_GENERIC;
       }
    } else {
      UPLL_LOG_DEBUG("Invalid Input");
      free(key_policingprofile);
      return UPLL_RC_ERR_GENERIC;
    }
  }

  okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
      IpctSt::kIpcStValPolicingprofile, key_policingprofile,
      NULL);
  if (NULL == okey) {
    UPLL_LOG_DEBUG("okey Memory Allocation for ConfigKeyVal failed");
    free(key_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::TxCopyCandidateToRunning(
    unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
    DalDmlIntf *dmi, TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result = uud::kDalRcGeneralError;
  unc_keytype_operation_t op[] = { UNC_OP_DELETE, UNC_OP_CREATE,
                                   UNC_OP_UPDATE};
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *policingprofile_key = NULL, *req = NULL, *nreq = NULL,
               *pp_ck_run = NULL;
  DalCursor *cfg1_cursor = NULL;
  uint8_t *ctrlr_id = NULL;
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  if (ctrlr_commit_status != NULL) {
    for (ccsListItr = ctrlr_commit_status->begin();
        ccsListItr != ctrlr_commit_status->end(); ++ccsListItr) {
      ccStatusPtr = *ccsListItr;
      ctrlr_id = reinterpret_cast<uint8_t *>
        (const_cast<char*>(ccStatusPtr->ctrlr_id.c_str()));
      ctrlr_result[ccStatusPtr->ctrlr_id] = ccStatusPtr->upll_ctrlr_result;
      if (ccStatusPtr->upll_ctrlr_result != UPLL_RC_SUCCESS) {
        for (ConfigKeyVal *ck_err = ccStatusPtr->err_ckv; ck_err != NULL;
             ck_err = ck_err->get_next_cfg_key_val()) {
          if (ck_err->get_key_type() != keytype) continue;
          result_code = GetRenamedUncKey(ck_err, UPLL_DT_CANDIDATE, dmi,
              ctrlr_id);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG(
                " PolicingProfileMoMgr::TxCopyCandidateToRunning Failed.");
            UPLL_LOG_ERROR(" GetRenamedUncKey Function failed - %d ",
                result_code);
            return result_code;
          }
        }
      }
    }
  }

  if (config_mode != TC_CONFIG_VTN) {
    for (int i = 0; i < nop; i++) {
      cfg1_cursor = NULL;
      // Update the Main table
      if (op[i] != UNC_OP_UPDATE) {
        result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
            req, nreq, &cfg1_cursor, dmi, NULL, config_mode, vtn_name,
            MAINTBL, true);
        while (result_code == UPLL_RC_SUCCESS) {
          db_result = dmi->GetNextRecord(cfg1_cursor);
          result_code = DalToUpllResCode(db_result);
          if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            result_code = UPLL_RC_SUCCESS;
            break;
          }
          result_code = UpdateMainTbl(req, op[i], UPLL_RC_SUCCESS,
                                    nreq, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("Updating Main table Error %d", result_code);
            dmi->CloseCursor(cfg1_cursor, true);
            DELETE_IF_NOT_NULL(req);
            return result_code;
          }
        }
        if (cfg1_cursor) {
          dmi->CloseCursor(cfg1_cursor, true);
          cfg1_cursor = NULL;
        }
        DELETE_IF_NOT_NULL(req);
      }
      UPLL_LOG_DEBUG("Updating main table complete with op %d", op[i]);
    }  // for loop
  }  // if loop

  if (config_mode != TC_CONFIG_VIRTUAL) {
    for (int i = 0; i < nop; i++) {
      cfg1_cursor = NULL;
      // Update the controller table
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
        nreq, &cfg1_cursor, dmi, NULL, config_mode, vtn_name, CTRLRTBL, true);
      ConfigKeyVal *pp_ctrlr_key = NULL;
      while (result_code == UPLL_RC_SUCCESS) {
        db_result = dmi->GetNextRecord(cfg1_cursor);
        result_code = DalToUpllResCode(db_result);
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          result_code = UPLL_RC_SUCCESS;
          break;
        }
        if (op[i] == UNC_OP_CREATE) {
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
              kOpInOutFlag |kOpInOutCs };
          result_code = GetChildConfigKey(policingprofile_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
               result_code);
            DELETE_IF_NOT_NULL(req);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }

          result_code = ReadConfigDB(policingprofile_key,
                                   UPLL_DT_RUNNING /*UPLL_DT_CANDIDATE*/,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
          if ((result_code != UPLL_RC_SUCCESS) &&
             (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
            UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
            DELETE_IF_NOT_NULL(policingprofile_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          /* set consolidated config status to UNKNOWN to init vtn cs_status
           * to the cs_status of first controller
           */
          uint32_t cur_instance_count;
          ConfigKeyVal *temp_val = NULL;
          result_code = DupConfigKeyVal(temp_val, req, CTRLRTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("DupConfigval failed %d", result_code);
            DELETE_IF_NOT_NULL(policingprofile_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            DELETE_IF_NOT_NULL(temp_val);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }

          result_code = GetInstanceCount(temp_val, NULL,
             UPLL_DT_CANDIDATE, &cur_instance_count,
             dmi, CTRLRTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_INFO("GetInstanceCount failed %d", result_code);
            DELETE_IF_NOT_NULL(policingprofile_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            DELETE_IF_NOT_NULL(temp_val);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          if (cur_instance_count == 1) {
              reinterpret_cast<val_policingprofile *>
              (GetVal(policingprofile_key))->cs_row_status =
              UNC_CS_UNKNOWN;
          }
          result_code = DupConfigKeyVal(pp_ctrlr_key, req, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigVal function is failed %d", result_code);
            DELETE_IF_NOT_NULL(policingprofile_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            DELETE_IF_NOT_NULL(temp_val);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }

          DELETE_IF_NOT_NULL(temp_val);
          GET_USER_DATA_CTRLR(pp_ctrlr_key, ctrlr_id);
          string controller(reinterpret_cast<char *>(ctrlr_id));
          if (ctrlr_result.empty()) {
            UPLL_LOG_TRACE("ctrlr_commit_status is NULL.");
            result_code = UpdateConfigStatus(policingprofile_key, op[i],
                UPLL_RC_ERR_CTR_DISCONNECTED, nreq,
                dmi, pp_ctrlr_key);
          } else {
            result_code = UpdateConfigStatus(policingprofile_key, op[i],
              ctrlr_result[controller], nreq,
              dmi, pp_ctrlr_key);
          }
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO(" UpdateConfigStatus function failed - %d ",
                          result_code);
            DELETE_IF_NOT_NULL(policingprofile_key);
            DELETE_IF_NOT_NULL(pp_ctrlr_key);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
        } else if (op[i] == UNC_OP_DELETE) {
          // Reading Main Running DB for delete op
          DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone,
            kOpInOutFlag | kOpInOutCs };
          result_code = GetChildConfigKey(pp_ck_run, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                           result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          result_code = ReadConfigDB(pp_ck_run, UPLL_DT_RUNNING,
               UNC_OP_READ, dbop1, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
            DELETE_IF_NOT_NULL(pp_ck_run);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          if (result_code == UPLL_RC_SUCCESS) {
            GET_USER_DATA_CTRLR(req, ctrlr_id);
            result_code = SetPPConsolidatedStatus(pp_ck_run, ctrlr_id, dmi);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Could not set consolidated status %d",
                             result_code);
              DELETE_IF_NOT_NULL(pp_ck_run);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
          }
          DELETE_IF_NOT_NULL(pp_ck_run);
          result_code = GetChildConfigKey(pp_ctrlr_key, req);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey Failed  %d", result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
        }
        if (UNC_OP_CREATE == op[i]) {
          val_policingprofile_ctrl_t *val_ctrlr_temp = reinterpret_cast
             <val_policingprofile_ctrl_t *>(GetVal(pp_ctrlr_key));
          val_ctrlr_temp->valid[0] = UNC_VF_VALID;
          val_policingprofile_ctrl_t *val_ctrlr_temp1 = reinterpret_cast
             <val_policingprofile_ctrl_t *>(GetVal(req));
          val_ctrlr_temp->ref_count = val_ctrlr_temp1->ref_count;
          UPLL_LOG_DEBUG("Ref count in dupckv%d", val_ctrlr_temp->ref_count);
          UPLL_LOG_DEBUG("Ref count in req%d", val_ctrlr_temp1->ref_count);
        } else if (UNC_OP_UPDATE == op[i]) {
          result_code = DupConfigKeyVal(pp_ctrlr_key, nreq, CTRLRTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigVal function is failed %d", result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
          val_policingprofile_ctrl_t *val_ctrlr_temp = reinterpret_cast
             <val_policingprofile_ctrl_t *>(GetVal(pp_ctrlr_key));
          val_ctrlr_temp->valid[0] = UNC_VF_VALID;
          val_policingprofile_ctrl_t *val_ctrlr_temp1 = reinterpret_cast
             <val_policingprofile_ctrl_t *>(GetVal(req));
          val_ctrlr_temp->ref_count = val_ctrlr_temp1->ref_count;
          UPLL_LOG_DEBUG("Ref count in dupckv%d", val_ctrlr_temp->ref_count);
          UPLL_LOG_DEBUG("Ref count in req%d", val_ctrlr_temp1->ref_count);
        }
        result_code = UpdateConfigDB(pp_ctrlr_key, UPLL_DT_RUNNING, op[i],
                                   dmi, config_mode, vtn_name, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Unable to Update Configuration at DB %d",
                         result_code);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(pp_ctrlr_key);
          dmi->CloseCursor(cfg1_cursor, true);
          return result_code;
        }
        // update the consolidated config status in the Main Table
        if (op[i] == UNC_OP_CREATE) {
          result_code = UpdateConfigDB(policingprofile_key, UPLL_DT_RUNNING,
             UNC_OP_UPDATE, dmi, config_mode, vtn_name, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("Unable to Update Configuration at DB %d",
                          result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            DELETE_IF_NOT_NULL(policingprofile_key);
            dmi->CloseCursor(cfg1_cursor, true);
            return result_code;
          }
        }

        EnqueCfgNotification(op[i], UPLL_DT_RUNNING, pp_ctrlr_key);
        DELETE_IF_NOT_NULL(policingprofile_key);
        DELETE_IF_NOT_NULL(pp_ctrlr_key);
        result_code = DalToUpllResCode(db_result);
      }
      if (cfg1_cursor) {
        dmi->CloseCursor(cfg1_cursor, true);
        cfg1_cursor = NULL;
       }
       DELETE_IF_NOT_NULL(req);
       DELETE_IF_NOT_NULL(nreq);
       result_code = TxCopyRenameTableFromCandidateToRunning(keytype,
                                                          op[i], dmi,
                                                          config_mode,
                                                          vtn_name);
       UPLL_LOG_DEBUG("TxCopyRenameTableFromCandidateToRunning returned %d",
                                                            result_code);
     }
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;

  if ((UPLL_RC_SUCCESS == result_code) &&
      (config_mode != TC_CONFIG_VIRTUAL)) {
    ClearScratchTbl(config_mode, vtn_name, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ClearScratchTbl failed %d", result_code);
    }
  }
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::UpdateMainTbl(ConfigKeyVal *key_pp,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_pp = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingprofile_t *val_pp = NULL;
  string vtn_name = "";

  switch (op) {
    case UNC_OP_CREATE:
      result_code = DupConfigKeyVal(ck_pp, key_pp, MAINTBL);
      if (!ck_pp || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("DupConfigKeyVal() Returning error %d", result_code);
        return result_code;
      }
      val_pp = reinterpret_cast<val_policingprofile_t *>(GetVal(ck_pp));
      if (!val_pp) {
        UPLL_LOG_DEBUG("invalid val");
        return UPLL_RC_ERR_GENERIC;
      }
      val_pp->cs_row_status = UNC_CS_APPLIED;
      break;
    case UNC_OP_DELETE:

      result_code = GetChildConfigKey(ck_pp, key_pp);
      if (!ck_pp || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("GetChildConfigKey() returning error %d", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
      break;
    default:
          UPLL_LOG_DEBUG("Inalid operation");
      return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
  dbop.inoutop = kOpInOutCs | kOpInOutFlag;
  result_code = UpdateConfigDB(ck_pp, UPLL_DT_STATE, op, dmi, &dbop,
                               TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
  EnqueCfgNotification(op, UPLL_DT_RUNNING, key_pp);
  delete ck_pp;
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::GetDiffRecord(ConfigKeyVal *ckv_running,
                                   ConfigKeyVal *ckv_audit,
                                   uuc::UpdateCtrlrPhase phase, MoMgrTables tbl,
                                   ConfigKeyVal *&okey,
                                   DalDmlIntf *dmi,
                                   bool &invalid_attr,
                                   bool check_audit_phase) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_dup = NULL;
  okey = NULL;
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCs};
  switch (phase) {
    case uuc::kUpllUcpDelete:
    case uuc::kUpllUcpCreate:
      if (tbl == CTRLRTBL) {
        UPLL_LOG_DEBUG("Created  record fot ctrlr_tbl is %s ",
                       ckv_running->ToStrAll().c_str());
        result_code = GetChildConfigKey(okey, ckv_running);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed. err_code & phase %d %d",
                           result_code, phase);
          return result_code;
        }
        upll_keytype_datatype_t dt_type = (phase == uuc::kUpllUcpDelete)?
          UPLL_DT_AUDIT : UPLL_DT_RUNNING;
        result_code = ReadConfigDB(okey, dt_type,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("ReadConfigDB failed. err_code & phase %d %d",
                           result_code, phase);
          return result_code;
        }
      } else {
          UPLL_LOG_DEBUG("Created  record is %s ",
                         ckv_running->ToStrAll().c_str());
          result_code = DupConfigKeyVal(okey, ckv_running, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed. err_code & phase %d %d",
                           result_code, phase);
            return result_code;
          }
      }
    break;
    case uuc::kUpllUcpUpdate:
      if (tbl == CTRLRTBL) {
        UPLL_LOG_DEBUG("UpdateRecord  record for run Ctrlr_tbl is %s ",
                       ckv_running->ToStrAll().c_str());
        /*To be removed*/
        val_policingprofile_ctrl_t *val1 =
            reinterpret_cast<val_policingprofile_ctrl_t *>
            (GetVal(ckv_running));
        UPLL_LOG_DEBUG("cs_row_status : %d flags : %d ref_count : %d",
                       val1->cs_row_status, val1->flags, val1->ref_count);
        UPLL_LOG_DEBUG("UpdateRecord  record for audit Ctrlr_tbl is %s ",
                       ckv_audit->ToStrAll().c_str());
        val_policingprofile_ctrl_t *val2 =
            reinterpret_cast<val_policingprofile_ctrl_t *> (GetVal(ckv_audit));
        UPLL_LOG_DEBUG("cs_row_status : %d flags : %d ref_count : %d",
                       val2->cs_row_status, val2->flags, val2->ref_count);
        result_code = GetChildConfigKey(okey, ckv_running);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey for running record failed. "
                         "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        result_code = ReadConfigDB(okey, UPLL_DT_RUNNING,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("ReadConfigDB from running failed. "
                           "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        result_code = GetChildConfigKey(ckv_dup, ckv_audit);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey for audit record failed. "
                            "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        result_code = ReadConfigDB(ckv_dup, UPLL_DT_AUDIT,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("ReadConfigDB from audit failed. "
                           "err_code & phase %d %d", result_code, phase);
          DELETE_IF_NOT_NULL(ckv_dup);
          return result_code;
        }
      } else {
          UPLL_LOG_DEBUG("UpdateRecord  record  is %s ",
                         ckv_running->ToStrAll().c_str());
          UPLL_LOG_DEBUG("UpdateRecord  record  is %s ",
                         ckv_audit->ToStrAll().c_str());
          result_code = DupConfigKeyVal(okey, ckv_running, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for running record. "
                            "err_code & phase %d %d", result_code, phase);
            return result_code;
          }
          result_code = DupConfigKeyVal(ckv_dup, ckv_audit, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for audit record. "
                           "err_code & phase %d %d", result_code, phase);
            return result_code;
          }
      }
      if (GetVal(okey) != NULL &&
          GetVal(ckv_dup) != NULL) {
        void *val1 = GetVal(okey);
        invalid_attr = FilterAttributes(val1, GetVal(ckv_dup), true,
                         UNC_OP_UPDATE);
      }
      if (check_audit_phase) {
        if ((okey != NULL) && (ckv_dup!= NULL)) {
          ConfigVal *next_val = (ckv_dup->get_cfg_val())->DupVal();
          okey->AppendCfgVal(next_val);
        }
      }

      DELETE_IF_NOT_NULL(ckv_dup);
    break;
    default:
      UPLL_LOG_DEBUG("Invalid operation %d", phase);
      return UPLL_RC_ERR_NO_SUCH_OPERATION;
      break;
  }
  return result_code;
}


upll_rc_t PolicingProfileMoMgr::IsReferenced(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;

  unc::upll::kt_momgr::VtnPolicingMapMoMgr *vtnpmmgr = reinterpret_cast
      <unc::upll::kt_momgr::VtnPolicingMapMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN_POLICINGMAP)));
  if (NULL == vtnpmmgr) {
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = vtnpmmgr->IsPolicingProfileConfigured(
    reinterpret_cast<const char *>
    (reinterpret_cast<key_policingprofile *>(ikey->get_key())->
    policingprofile_name), req->datatype, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
      UPLL_LOG_DEBUG("PolicingProfile is referred in VTN");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    UPLL_LOG_DEBUG("VTN IsPolicingProfileConfigured failed. code(%d)",
                   result_code);
    return result_code;
  }

  // If data_type is Candidate then scratch table ref_count is tallied with
  // controller table ref_count.
  if (UPLL_DT_CANDIDATE == req->datatype) {
    TcConfigMode config_mode = TC_CONFIG_INVALID;
    std::string vtn_name = "";
    result_code = GetConfigModeInfo(req, config_mode, vtn_name);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetConfigMode failed");
      return result_code;
    }

    result_code = RefCountSemanticCheck(reinterpret_cast<const char *>
                                       (reinterpret_cast<key_policingprofile *>
                                       (ikey->get_key())->
                                        policingprofile_name),
                                        dmi, config_mode, vtn_name);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("RefCountSemanticCheck failed %d", result_code);
      return result_code;
    }
  } else {
    unc::upll::kt_momgr::VbrPolicingMapMoMgr *vbrpmmgr = reinterpret_cast
        <unc::upll::kt_momgr::VbrPolicingMapMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_POLICINGMAP)));
    if (NULL == vbrpmmgr) {
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = vbrpmmgr->IsPolicingProfileConfigured(
        reinterpret_cast<const char *>
        (reinterpret_cast<key_policingprofile *>(ikey->get_key())->
         policingprofile_name), req->datatype, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
        UPLL_LOG_INFO("PolicingProfile is referred in vBridge");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
      UPLL_LOG_DEBUG("vBridge IsPolicingProfileConfigured failed. code(%d)",
                     result_code);
      return result_code;
    }

    unc::upll::kt_momgr::VbrIfPolicingMapMoMgr *vbrifpmmgr = reinterpret_cast
        <unc::upll::kt_momgr::VbrIfPolicingMapMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIF_POLICINGMAP)));
    if (NULL == vbrifpmmgr) {
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = vbrifpmmgr->IsPolicingProfileConfigured(
        reinterpret_cast<const char *>
        (reinterpret_cast<key_policingprofile *>(ikey->get_key())->
         policingprofile_name), req->datatype, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
        UPLL_LOG_ERROR(" PolicingProfile is referred in vBridge Interface");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
      UPLL_LOG_DEBUG("vBridge If IsPolicingProfileConfigured failed. code(%d)",
                     result_code);
      return result_code;
    }
    unc::upll::kt_momgr::VtermIfPolicingMapMoMgr *vtermifpmmgr =
        reinterpret_cast<unc::upll::kt_momgr::VtermIfPolicingMapMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERMIF_POLICINGMAP)));
    if (NULL == vtermifpmmgr) {
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = vtermifpmmgr->IsPolicingProfileConfigured(
        reinterpret_cast<const char *>
        (reinterpret_cast<key_policingprofile *>(ikey->get_key())->
         policingprofile_name), req->datatype, dmi);
    if (UPLL_RC_SUCCESS != result_code) {
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
        UPLL_LOG_INFO(" PolicingProfile is referred in vTerm Interface");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
      UPLL_LOG_DEBUG("vTerm If IsPolicingProfileConfigured failed. code(%d)",
                     result_code);
      return result_code;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::ReadMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                    result_code);
      return result_code;
  }

  result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
  }
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey, bool begin,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                    result_code);
      return result_code;
  }
  result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
  }
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 const char *ctrlr_name) {
  UPLL_FUNC_TRACE;

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name) {
    ctrlr_name = static_cast<char *>(ikey->get_user_data());
  }

  UPLL_LOG_TRACE("Controller_name:(%s), operation:(%d)",
                 ctrlr_name, req->operation);

  bool result_code = false;
  uint32_t max_instance_count;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  switch (req->operation) {
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
    default: {
      if (req->datatype == UPLL_DT_STATE)
        result_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      else
        result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
    }
  }

  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for opeartion(%d)",
                   ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_POLICING_PROFILE != key->get_key_type()) {
    UPLL_LOG_DEBUG("Received keytype (%d) is not KT_POLICINGPROFILE!!",
                  key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (key->get_st_num() != IpctSt::kIpcStKeyPolicingprofile) {
     UPLL_LOG_DEBUG("Invalid key structure received. received struct num - %d",
                  key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" Error: option2 is not NONE");
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
  if (req->option1 != UNC_OPT1_NORMAL) {
     UPLL_LOG_DEBUG(" Error: option1 is not NORMAL");
     return UPLL_RC_ERR_INVALID_OPTION1;
  }

  /** Read key, val struct from ConfigKeyVal */
  key_policingprofile_t *key_policingprofile =
      reinterpret_cast<key_policingprofile_t *>(key->get_key());

  if (NULL == key_policingprofile) {
    UPLL_LOG_ERROR("KT_POLICINGPROFILE Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if ((req->operation != UNC_OP_READ_SIBLING_COUNT) &&
      (req->operation != UNC_OP_READ_SIBLING_BEGIN)) {
    rt_code = ValidateKey(reinterpret_cast<char*>(
            key_policingprofile->policingprofile_name),
        (unsigned int)kMinLenPolicingProfileName,
        (unsigned int)kMaxLenPolicingProfileName);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_ERROR("key structure syntax validation failed Err code-%d",
                     rt_code);
      return rt_code;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", req->operation);
    StringReset(key_policingprofile->policingprofile_name);
  }

  UPLL_LOG_TRACE(" key struct validation is success");
  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  if (operation == UNC_OP_RENAME) {
    if (dt_type == UPLL_DT_IMPORT) {
      val_rename_policingprofile_t *val_rename_policingprofile = NULL;
      /** Read rename value structure*/
      if (key->get_cfg_val() && (key->get_cfg_val()->get_st_num() ==
         IpctSt::kIpcStValRenamePolicingprofile)) {
        val_rename_policingprofile =
          reinterpret_cast<val_rename_policingprofile_t *>(
            key->get_cfg_val()->get_val());
      }
      if (val_rename_policingprofile) {
        return ValidatePolicingProfileRenameValue(val_rename_policingprofile,
                                                         operation);
      }
      UPLL_LOG_DEBUG("Mandatory val_rename_policingprofile struct is NULL"
                     " for RENAME");
      return UPLL_RC_ERR_BAD_REQUEST;
    } else {
      UPLL_LOG_DEBUG("Invalid datatype(%d) received for RENAME", dt_type);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
  }
  /** Expected value structure in Request is NONE for all operation,
   so no validation required for value structure */
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::ValidatePolicingProfileRenameValue(
    val_rename_policingprofile_t *val_rename_policingprofile,
    uint32_t operation) {
  UPLL_FUNC_TRACE;


  if (val_rename_policingprofile->valid[UPLL_IDX_RENAME_PROFILE_RPP] ==
      UNC_VF_VALID) {
    /** validate flowlist_newname syntax*/
    return ValidateKey(reinterpret_cast<char *>(
          val_rename_policingprofile->policingprofile_newname),
          (unsigned int)kMinLenPolicingProfileName,
          (unsigned int)kMaxLenPolicingProfileName);
  } else if ((operation == UNC_OP_UPDATE) && (
      val_rename_policingprofile->valid[ UPLL_IDX_RENAME_PROFILE_RPP ] ==
      UNC_VF_VALID_NO_VALUE)) {
    /** Reset the field with default value */
    memset(val_rename_policingprofile->policingprofile_newname, 0,
           kMaxLenPolicingProfileName);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::GetRenameInfo(ConfigKeyVal *ikey,
    ConfigKeyVal *okey, ConfigKeyVal *&rename_info, DalDmlIntf *dmi,
    const char *ctrlr_id, bool &renamed) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_id = "";

  if (!ikey || !okey || NULL != rename_info
      || !(ikey->get_key()) || !(okey->get_key())) {
    UPLL_LOG_DEBUG("Input is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  key_policingprofile_t *policingprofile_key = NULL;
  policingprofile_key = reinterpret_cast<key_policingprofile_t *>
    (ikey->get_key());
  if (policingprofile_key == NULL) {
    UPLL_LOG_DEBUG("policingprofile_key NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_rename_vnode_info_t *key_rename_info =
    reinterpret_cast<key_rename_vnode_info_t *>
    (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info_t)));

  /* Checks the profile is already renamed or not */
  if (renamed) {
    /* if already renamed store the controller name */
    uuu::upll_strncpy(key_rename_info->ctrlr_profile_name,
           reinterpret_cast<val_rename_policingprofile *>(GetVal(ikey))
          ->policingprofile_newname, (kMaxLenPolicingProfileName + 1));
    UPLL_LOG_DEBUG("key_rename_info->ctrlr_profile_name ::: (%s)",
                   key_rename_info->ctrlr_profile_name);
  } else {
    /* if not renamed the ikey contains the controller name */
    uuu::upll_strncpy(key_rename_info->ctrlr_profile_name,
        policingprofile_key->policingprofile_name,
        (kMaxLenPolicingProfileName + 1));
    UPLL_LOG_DEBUG("key_rename_info->ctrlr_profile_name ::: (%s)",
                   key_rename_info->ctrlr_profile_name);
  }
  // To check for standalone configuration
  DbSubOp dbop1 = {kOpReadExist, kOpMatchNone,
                   kOpInOutNone};
  // ikey has rename val set, so removing  that to read from ctrlr tbl
  ikey->SetCfgVal(NULL);
  result_code = UpdateConfigDB(ikey, UPLL_DT_IMPORT,
                             UNC_OP_READ, dmi, &dbop1, CTRLRTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    result_code = UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
    UPLL_LOG_INFO("Stand Alone configuration found %d", result_code);
    free(key_rename_info);  // resource leak
    return result_code;
  }  else if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code)  {
    UPLL_LOG_DEBUG("ReadConfigDB failed - %d", result_code);
    free(key_rename_info);  // resource leak
    return result_code;
  }
  /* Store the old UNC VTN  name */
  uuu::upll_strncpy(key_rename_info->old_policingprofile_name,
                    policingprofile_key->policingprofile_name,
                    (kMaxLenPolicingProfileName + 1));
  UPLL_LOG_DEBUG("key_rename_info->old_policingprofile_name ::: (%s)",
                  key_rename_info->old_policingprofile_name);

  policingprofile_key = reinterpret_cast<key_policingprofile_t *>
    (okey->get_key());

  uuu::upll_strncpy(key_rename_info->new_policingprofile_name,
                    policingprofile_key->policingprofile_name,
                    (kMaxLenPolicingProfileName + 1));
  UPLL_LOG_DEBUG("key_rename_info->new_policingprofile_name ::: (%s)",
                  key_rename_info->new_policingprofile_name);

  rename_info = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
      IpctSt::kIpcInvalidStNum, key_rename_info, NULL);
  if (!rename_info) {
    free(key_rename_info);
    UPLL_LOG_DEBUG("Failed to allocate memory for ConfigkeyVal");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(rename_info, ctrlr_id);

  if (!renamed) {
    val_rename_policingprofile_t *val_rename =
      reinterpret_cast<val_rename_policingprofile_t *>
      (ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile_t)));
    uuu::upll_strncpy(val_rename->policingprofile_newname,
                      key_rename_info->ctrlr_profile_name,
                      (kMaxLenPolicingProfileName+1));
    val_rename->valid[UPLL_IDX_RENAME_PROFILE_RPP] = UNC_VF_VALID;
    ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValRenamePolicingprofile,
        val_rename);

    SET_USER_DATA_CTRLR(ikey, ctrlr_id);

    ikey->SetCfgVal(cfg_val);

    DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutCtrlr};
    result_code = UpdateConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_CREATE, dmi,
        &dbop, TC_CONFIG_GLOBAL, vtn_id, RENAMETBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("UpdateConfigDb Failed (%d)", result_code);
    }
  }
  if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code)
    result_code = UPLL_RC_SUCCESS;
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (NULL == ikey ||  NULL == (ikey->get_key()) || NULL != okey) {
    UPLL_LOG_DEBUG("Invalid Input");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info_t *key_rename =
    reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());
  if (!strlen(reinterpret_cast<char *>
        (key_rename->old_policingprofile_name))) {
    UPLL_LOG_DEBUG(" Invalid Policing Profile Name");
    return UPLL_RC_ERR_GENERIC;
  }
  key_policingprofile_t *key_policingprofile =
    reinterpret_cast<key_policingprofile_t *>
    (ConfigKeyVal::Malloc(sizeof(key_policingprofile_t)));
  uuu::upll_strncpy(key_policingprofile->policingprofile_name,
                    key_rename->old_policingprofile_name,
                    kMaxLenPolicingProfileName+1);

  okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
      IpctSt::kIpcStKeyPolicingprofile, key_policingprofile, NULL);
  if (!okey) {
    UPLL_LOG_DEBUG("okey NULL");
    free(key_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, ikey);
  return result_code;
}

bool PolicingProfileMoMgr::IsValidKey(void *ikey, uint64_t index,
                                      MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_policingprofile_t *key =
      reinterpret_cast<key_policingprofile_t *>(ikey);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  ret_val = ValidateKey(reinterpret_cast<char *>(key->policingprofile_name),
                        kMinLenPolicingProfileName,
                        kMaxLenPolicingProfileName);

  if (UPLL_RC_SUCCESS != ret_val) {
    UPLL_LOG_DEBUG("Policingprofile Name is not valid(%d)", ret_val);
    return false;
  }
  return true;
}

upll_rc_t PolicingProfileMoMgr::GetControllerSpan(ConfigKeyVal *ikey,
                                        upll_keytype_datatype_t dt_type,
                                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr};

  result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop, dmi, CTRLRTBL);
  UPLL_LOG_DEBUG("GetControllerSpan successful:- %d", result_code);
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::SetConsolidatedStatus(ConfigKeyVal *ikey,
        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv = NULL;
  string vtn_name = "";

  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutCs};
  result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" SetConsolidatedStatus failed. GetChildConfigKey Failed"
      " Result Code - %d", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" SetConsolidatedStatus failed. ReadConfigDB Failed"
      " Result Code - %d", result_code);
    delete ckv;
    return result_code;
  }
  std::list< unc_keytype_configstatus_t > list_cs_row;
  val_policingprofile_ctrl_t *val;
  ConfigKeyVal *tmp_ckv = ckv;
  for ( ; tmp_ckv != NULL ; tmp_ckv = tmp_ckv->get_next_cfg_key_val()) {
      val = reinterpret_cast<val_policingprofile_ctrl_t *>(GetVal(tmp_ckv));
      list_cs_row.push_back((unc_keytype_configstatus_t)val->cs_row_status);
      UPLL_LOG_DEBUG("Printing Cs_attr %d", val->cs_row_status);
  }
  if (ckv) delete ckv;
  val_policingprofile_t *val_temp =
      reinterpret_cast<val_policingprofile_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                               TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" SetConsolidatedStatus failed. UpdateConfigDB Failed"
      " Result Code - %d", result_code);
    return result_code;
  }
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::GetPolicingProfileCtrlrKeyval(
    ConfigKeyVal *&pp_keyval,
    const char *policingprofile_name,
    const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (NULL == policingprofile_name || NULL == ctrlr_id ||
     NULL != pp_keyval) {
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = GetChildConfigKey(pp_keyval, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  key_policingprofile_t *key_policingprofie =
    reinterpret_cast<key_policingprofile_t *>(pp_keyval->get_key());
  uuu::upll_strncpy(key_policingprofie->policingprofile_name,
                    policingprofile_name,
                    (kMaxLenPolicingProfileName+1));

  UPLL_LOG_DEBUG("PP name in GetPolicingProfileCtrlrKeyval %s",
                 key_policingprofie->policingprofile_name);
  UPLL_LOG_DEBUG("Ctrlrid in GetPolicingProfileCtrlrKeyval %s",
                 ctrlr_id);
  SET_USER_DATA_CTRLR(pp_keyval, ctrlr_id);

  return UPLL_RC_SUCCESS;
}


upll_rc_t PolicingProfileMoMgr::PolicingProfileCtrlrTblOper
    (const char *policingprofile_name, const char *ctrlr_id,
    DalDmlIntf *dmi, unc_keytype_operation_t oper,
    upll_keytype_datatype_t dt_type, uint8_t pp_flag,
    TcConfigMode config_mode, string vtn_name, uint32_t count,
    bool is_commit) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *pp_ckv = NULL;
  result_code = GetPolicingProfileCtrlrKeyval(pp_ckv,
      policingprofile_name, ctrlr_id);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("PolicingProfileCtrlrTblOper Failed. UpdateConfigDB failed"
                   "Result code - %d", result_code);
    return result_code;
  }

  if (UPLL_DT_IMPORT == dt_type)
     SET_USER_DATA_FLAGS(pp_ckv, pp_flag);

  if (UNC_OP_CREATE == oper) {
    result_code = CtrlrTblCreate(pp_ckv, dmi, dt_type, config_mode, vtn_name,
                                 is_commit, count);
    UPLL_LOG_DEBUG("PolicingProfileCtrlrTblOper Failed.CtrlrTblCreate failed"
                   "Result code - %d", result_code);
  } else if (UNC_OP_DELETE == oper) {
    result_code = CtrlrTblDelete(pp_ckv, dmi, dt_type, config_mode, vtn_name,
                                 is_commit, count);
    UPLL_LOG_DEBUG("PolicingProfileCtrlrTblOper Failed.CtrlrTblDelete failed"
                   "Result code - %d", result_code);
  } else {
    result_code = UPLL_RC_ERR_NO_SUCH_OPERATION;
    UPLL_LOG_DEBUG("PolicingProfileCtrlrTblOper Failed. "
                   "Result code - %d", result_code);
  }
  delete pp_ckv;
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::CtrlrTblCreate(ConfigKeyVal *pp_ckv,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
    TcConfigMode config_mode, string vtn_name, bool is_commit,
    uint32_t count) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *rename_key = NULL;
  uint8_t *ctrlr_id = NULL;
  UPLL_LOG_DEBUG("CtrlrTblCreate input count : %d", count);
  if ((UPLL_DT_CANDIDATE == dt_type) &&
      (false == is_commit)) {
    UPLL_LOG_TRACE("CtrlrTblCreate Not commit");
    result_code = UpdateRefCountInScratchTbl(pp_ckv, dmi, dt_type,
                                             UNC_OP_CREATE, config_mode,
                                             vtn_name, count);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = InsertRecInScratchTbl(pp_ckv, dmi, dt_type, UNC_OP_CREATE,
                                          config_mode, vtn_name, count);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("InsertRecInScratchTbl failed %d", result_code);
        return result_code;
      } else {
        UPLL_LOG_DEBUG("UpdateRefCountInScratchTbl returned %d", result_code);
        return result_code;
      }
    }
  } else if ((UPLL_DT_CANDIDATE == dt_type) &&
             (true == is_commit)) {
    result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  } else {
    //  Update Ref count in policingprofile_ctrlr_tbl directly.
    result_code = UpdateRefCountInCtrlrTbl(pp_ckv, dmi, dt_type, config_mode,
                                           vtn_name);
  }
  UPLL_LOG_DEBUG("UpdateRefCountInCtrlrTbl returns %d", result_code);
  //  If data_type is Candidate then and if error code
  //  UPLL_RC_ERR_INSTANCE_EXISTS is returned the update the ref_count in
  //  in scratch_tbl for the specified record.
  //  If any other data_type and result code  is other than
  //  UPLL_RC_ERR_NO_SUCH_INSTANCE, return result_code
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    return result_code;
  }
  //  If UPLL_RC_ERR_NO_SUCH_INSTANCE there is no record in
  //  policingprofile_ctrlr_tbl for ref_count to be updated.
  //  So Create a record in DB
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    // scenario : profile1 is renamed into profile2 and it stored into candidate
    // now user created profile1 in candidate. UNC accept this configuration.
    // But UNC should return error, when this profile (profile1)
    // is mapped with any
    if (dt_type == UPLL_DT_CANDIDATE) {
       uint8_t *ctrlrid = NULL;
       result_code = GetChildConfigKey(rename_key, pp_ckv);
       if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("GetChildConfigKey failed (%d)", result_code);
         return result_code;
       }
       if (!rename_key) {
         UPLL_LOG_DEBUG("rename_key NULL");
         return UPLL_RC_ERR_GENERIC;
       }

       GET_USER_DATA_CTRLR(rename_key, ctrlrid);

       result_code = GetRenamedUncKey(rename_key, UPLL_DT_CANDIDATE,
                                      dmi, ctrlrid);
       if (result_code == UPLL_RC_SUCCESS) {
         UPLL_LOG_ERROR("Profile name already renamed&exists, return semantic");
         DELETE_IF_NOT_NULL(rename_key);
         return UPLL_RC_ERR_CFG_SEMANTIC;
       } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         UPLL_LOG_DEBUG("GetRenamedUncKey Failed err_code %d", result_code);
         DELETE_IF_NOT_NULL(rename_key);
         return result_code;
       } else {  // If NO_SUCH_INSTANCE check in RUNNING
         result_code = GetRenamedUncKey(rename_key, UPLL_DT_RUNNING,
                                        dmi, ctrlrid);
         DELETE_IF_NOT_NULL(rename_key);
         if (result_code == UPLL_RC_SUCCESS) {
           UPLL_LOG_ERROR("Profile name already renamed & exists");
           return UPLL_RC_ERR_CFG_SEMANTIC;
         } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
           UPLL_LOG_DEBUG("GetRenamedUncKey Failed err_code %d", result_code);
           return result_code;
         }
       }

  #if 1
     // scenario: During import/partial import the
     // policingprofile name got renamed.
     // merge, commit, audit done.
     // rename tbl info will be removed, when delete the policingprofile from
     // candidate and create it again and commit.
     // Fix: copy the running renametbl configuration and placed it in
     // candidate configuration

     ConfigKeyVal *ckv_running_rename = NULL, *ckv_main = NULL;
     DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr, kOpInOutNone};

     result_code = GetChildConfigKey(ckv_running_rename, pp_ckv);
     if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("GetChildConfigKey failed (%d)", result_code);
       return result_code;
     }
     if (!ckv_running_rename) {
       UPLL_LOG_DEBUG("rename_key NULL");
       return UPLL_RC_ERR_GENERIC;
     }

     result_code = ReadConfigDB(ckv_running_rename, UPLL_DT_RUNNING,
            UNC_OP_READ, dbop, dmi, RENAMETBL);
     if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code)
          DELETE_IF_NOT_NULL(ckv_running_rename);
          return result_code;
     }

     if (UPLL_RC_SUCCESS == result_code) {
        val_rename_policingprofile_t* rename_policingprofile = reinterpret_cast
           <val_rename_policingprofile_t *>(GetVal(ckv_running_rename));
        rename_policingprofile->valid[UPLL_IDX_RENAME_PROFILE_RPP] =
            UNC_VF_VALID;

        result_code = UpdateConfigDB(ckv_running_rename, UPLL_DT_CANDIDATE,
              UNC_OP_CREATE, dmi, config_mode, vtn_name, RENAMETBL);
        if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code)
            DELETE_IF_NOT_NULL(ckv_running_rename);
            return result_code;
        }

        result_code = GetChildConfigKey(ckv_main, pp_ckv);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed (%d)", result_code);
          DELETE_IF_NOT_NULL(ckv_running_rename);
          return result_code;
        }

        DbSubOp dbop_1 = {kOpNotRead, kOpMatchNone, kOpInOutFlag};

        SET_USER_DATA_FLAGS(ckv_main, 0x01);
        result_code = UpdateConfigDB(ckv_main, UPLL_DT_CANDIDATE,
              UNC_OP_UPDATE, dmi, &dbop_1, config_mode, vtn_name, MAINTBL);
        DELETE_IF_NOT_NULL(ckv_main);
        DELETE_IF_NOT_NULL(ckv_running_rename);
        if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code)
            return result_code;
        }
        SET_USER_DATA_FLAGS(pp_ckv, 0x01);
     }
     DELETE_IF_NOT_NULL(ckv_running_rename);
#endif
     }
     // capability check
    IpcReqRespHeader *temp_req = reinterpret_cast<IpcReqRespHeader *>
      (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));

    temp_req->operation = UNC_OP_CREATE;
    temp_req->datatype = dt_type;
    val_policingprofile_ctrl_t *val_ctrlr = reinterpret_cast
       <val_policingprofile_ctrl_t *>(ConfigKeyVal::Malloc
       (sizeof(val_policingprofile_ctrl_t)));
    pp_ckv->AppendCfgVal(IpctSt::kIpcInvalidStNum, val_ctrlr);
    GET_USER_DATA_CTRLR(pp_ckv, ctrlr_id);
    result_code = ValidateCapability(temp_req, pp_ckv,
                                     reinterpret_cast<char*>(ctrlr_id));

    free(temp_req);

    unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
    uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
    if (result_code != UPLL_RC_SUCCESS) {
      // Policingprofile is not supported for other than PFC Controller
      // so skip adding entry for such sontroller in ctrlr table
      if ((!ctrlr_mgr->GetCtrlrType(reinterpret_cast<char *>(ctrlr_id),
                     dt_type, &ctrlrtype)) || (ctrlrtype != UNC_CT_PFC)) {
          UPLL_LOG_DEBUG("Controller type is  %d", ctrlrtype);
          return UPLL_RC_SUCCESS;
       }
       UPLL_LOG_DEBUG("Key not supported by controller");
       return result_code;
    }
    UPLL_LOG_DEBUG(" No record found. Create new record");
    if (dt_type == UPLL_DT_CANDIDATE) {
      // ref_count will be updated during Tx start phase.
      UPLL_LOG_DEBUG("CtrlrTblCreate in val count : %d", count);
      val_ctrlr->ref_count = count;
    } else {
      val_ctrlr->ref_count = 1;
    }
    val_ctrlr->valid[0] = UNC_VF_VALID;
    if ((UPLL_DT_AUDIT == dt_type)) {
      ConfigKeyVal *temp_pp_ckv = NULL;
      result_code = GetChildConfigKey(temp_pp_ckv, pp_ckv);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        return result_code;
      }
      DbSubOp dbop1 = {kOpReadSingle, kOpMatchNone, kOpInOutCs};
      result_code = ReadConfigDB(temp_pp_ckv, dt_type,
          UNC_OP_READ, dbop1, dmi, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(temp_pp_ckv);
        if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
          result_code = UPLL_RC_SUCCESS;
        return result_code;
      }
      if (UPLL_DT_AUDIT == dt_type) {
      val_policingprofile_t *temp_val = reinterpret_cast
          <val_policingprofile_t *>(GetVal(temp_pp_ckv));
      val_ctrlr->cs_row_status = static_cast<unc_keytype_configstatus_t>
      (temp_val->cs_row_status);
      DELETE_IF_NOT_NULL(temp_pp_ckv);
      }
    }
    val_policingprofile_ctrl_t *tmp = reinterpret_cast
        <val_policingprofile_ctrl_t *>(GetVal(pp_ckv));
    if (dt_type == UPLL_DT_CANDIDATE) {
      tmp->ref_count = count;
      tmp->valid[0] = UNC_VF_VALID;
    }
    upll_rc_t rt_code = UpdateConfigDB(pp_ckv, dt_type,
                                       UNC_OP_CREATE, dmi, config_mode,
                                       vtn_name, CTRLRTBL);
    if ((UPLL_RC_SUCCESS != rt_code) &&
        (UPLL_RC_ERR_INSTANCE_EXISTS != rt_code)) {
      UPLL_LOG_DEBUG(" CtrlrTblCreate Failed. Create record failed."
        " Result_code - %d", result_code);
      return result_code;
    }
    UPLL_LOG_DEBUG(" refcount from ckv - %d %d", tmp->ref_count,
        tmp->valid[0]);
    UPLL_LOG_DEBUG(" refcount - %d ", val_ctrlr->ref_count);
    if (UPLL_RC_ERR_INSTANCE_EXISTS != rt_code) {
      PolicingProfileEntryMoMgr *ppe_mgr =
        reinterpret_cast<PolicingProfileEntryMoMgr *>
        (const_cast<MoManager *>(GetMoManager(
        UNC_KT_POLICING_PROFILE_ENTRY)));
      key_policingprofile_t *key_policingprofile =
        reinterpret_cast<key_policingprofile_t *>(pp_ckv->get_key());
      result_code = ppe_mgr->PolicingProfileEntryCtrlrTblOper(
          reinterpret_cast<char*>(key_policingprofile->policingprofile_name),
          reinterpret_cast<char*>(ctrlr_id), dmi, UNC_OP_CREATE, dt_type,
          config_mode, vtn_name, false);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("Error in PolicingProfileEntryCtrlrTblOper in"
                       "CREATE (%d)", result_code);
        return result_code;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::CtrlrTblDelete(ConfigKeyVal *pp_ckv,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
    TcConfigMode config_mode, string vtn_name, bool is_commit,
    uint32_t count) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t *ctrlr_id = NULL;
  if ((UPLL_DT_CANDIDATE == dt_type) &&
      (false == is_commit)) {
    // Ref_count in scratch tbl is decremented.
    upll_rc_t rt_code = UpdateRefCountInScratchTbl(
                           pp_ckv, dmi, dt_type,
                           UNC_OP_DELETE, config_mode,
                           vtn_name, count);
    if (rt_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = InsertRecInScratchTbl(pp_ckv, dmi, dt_type, UNC_OP_DELETE,
                                          config_mode, vtn_name, count);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("InsertRecInScratchTbl failed %d", result_code);
        return result_code;
      } else {
        UPLL_LOG_DEBUG("UpdateRefCountInScratchTbl returned %d", result_code);
        return result_code;
      }
    }
  } else {
    DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr, kOpInOutNone};
    DbSubOp dbop1 = {kOpNotRead, kOpMatchCtrlr, kOpInOutNone};
    // Check whether entry is present in CTRLRTBL
    val_policingprofile_ctrl_t *val_ctrlr;
    result_code = ReadConfigDB(pp_ckv, dt_type,
                               UNC_OP_READ, dbop, dmi, CTRLRTBL);
    // Doesnt exists return error
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG(" CtrlrTblDelete Failed. No record found."
                     " Result_code - %d", result_code);
      // Success is sent to continue deleting the record
      return UPLL_RC_SUCCESS;
    } else if (UPLL_RC_SUCCESS ==  result_code) {
      val_ctrlr = reinterpret_cast<val_policingprofile_ctrl_t *>
          (GetVal(pp_ckv));
      UPLL_LOG_DEBUG(" Read is success in CtrlrTblDelete");
      GET_USER_DATA_CTRLR(pp_ckv, ctrlr_id);
      if (1 < val_ctrlr->ref_count) {
        val_ctrlr->ref_count = val_ctrlr->ref_count-1;
        UPLL_LOG_DEBUG(" Refcount is  - %d %d", val_ctrlr->ref_count,
                       val_ctrlr->valid[0]);
        val_ctrlr->valid[0] = UNC_VF_VALID;
        result_code = UpdateConfigDB(pp_ckv, dt_type, UNC_OP_UPDATE, dmi,
                                     &dbop1, config_mode, vtn_name, CTRLRTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_INFO("Update record failed. Result_code - %d", result_code);
          return result_code;
        }
      } else if (1 >= val_ctrlr->ref_count) {
        // If ref_count = 0 delete the entry in ctrlrtbl
        UPLL_LOG_DEBUG(" Refcount is less than 1 - %d %d", val_ctrlr->ref_count,
                       val_ctrlr->valid[0]);
        result_code = UpdateConfigDB(pp_ckv, dt_type, UNC_OP_DELETE, dmi,
                                     &dbop1, config_mode, vtn_name, CTRLRTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          return result_code;
        }

        // Renametbl entry should be deleted when no entry
        // in policing profile ctrlr tbl
        result_code = UpdateConfigDB(pp_ckv, dt_type, UNC_OP_DELETE,
                                     dmi, &dbop1, config_mode, vtn_name,
                                     RENAMETBL);
        if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
          UPLL_LOG_INFO("UpdateConfigDB Failed %d", result_code);
          DELETE_IF_NOT_NULL(pp_ckv);
          return result_code;
        }
        result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?
            UPLL_RC_SUCCESS:result_code;

        PolicingProfileEntryMoMgr *ppe_mgr =
            reinterpret_cast<PolicingProfileEntryMoMgr *>
            (const_cast<MoManager *>(GetMoManager(
                        UNC_KT_POLICING_PROFILE_ENTRY)));
        key_policingprofile_t *key_policingprofile =
            reinterpret_cast<key_policingprofile_t *>(pp_ckv->get_key());
        result_code = ppe_mgr->PolicingProfileEntryCtrlrTblOper(
            reinterpret_cast<char*>(key_policingprofile->policingprofile_name),
            reinterpret_cast<char*>(ctrlr_id), dmi, UNC_OP_DELETE, dt_type,
            config_mode, vtn_name, false);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_INFO(
              "Error in PolicingProfileEntryCtrlrTblOper in DELETE (%d)",
              result_code);
          return result_code;
        }
      }
    } else {
      return result_code;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::UpdateConfigStatus(ConfigKeyVal *ckv,
                                                   unc_keytype_operation_t op,
                                                   uint32_t driver_result,
                                                   ConfigKeyVal *nreq,
                                                   DalDmlIntf   *dmi,
                                                   ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_keytype_configstatus_t  ctrlr_status;
  uint8_t cs_status;
  ctrlr_status = (driver_result == UPLL_RC_SUCCESS) ?
                  UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  if ((NULL == ckv) || (NULL == ctrlr_key)) {
    UPLL_LOG_DEBUG("input struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  val_policingprofile_t *val_policingprofile =
                 reinterpret_cast<val_policingprofile_t *>(GetVal(ckv));
  val_policingprofile_ctrl *ctrlr_val_policingprofile =
      reinterpret_cast<val_policingprofile_ctrl *>(GetVal(ctrlr_key));
  if ((val_policingprofile == NULL) || (ctrlr_val_policingprofile == NULL)) {
    UPLL_LOG_ERROR("Value structure is empty!!");
    return UPLL_RC_ERR_GENERIC;
  }
  cs_status = val_policingprofile->cs_row_status;
  UPLL_LOG_TRACE("cs_status %d ctrlr_status %d\n", cs_status, ctrlr_status);
  if (op == UNC_OP_CREATE) {
    ctrlr_val_policingprofile->cs_row_status = ctrlr_status;
    /* update the vtn status in main tbl */
    if (val_policingprofile->cs_row_status == UNC_CS_UNKNOWN) {
        /* first entry in ctrlr table */
      cs_status = ctrlr_status;
    } else if (val_policingprofile->cs_row_status == UNC_CS_APPLIED) {
        if (ctrlr_status == UNC_CS_NOT_APPLIED) {
          cs_status = UNC_CS_PARTIALLY_APPLIED;
        }
    } else if (val_policingprofile->cs_row_status == UNC_CS_NOT_APPLIED) {
        if (ctrlr_status == UNC_CS_APPLIED) {
          cs_status =  UNC_CS_PARTIALLY_APPLIED;
        }
    } else if (val_policingprofile->cs_row_status == UNC_CS_INVALID) {
      cs_status = UNC_CS_INVALID;
    } else {
        cs_status = UNC_CS_PARTIALLY_APPLIED;
    }
    val_policingprofile->cs_row_status = cs_status;
  }
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                   ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_POLICING_PROFILE) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_root_t *root_key = reinterpret_cast<key_root_t*>
      (ConfigKeyVal::Malloc(sizeof(key_root_t)));

  okey = new ConfigKeyVal(UNC_KT_ROOT, IpctSt::kIpcStKeyRoot,
                          root_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_policingprofile_t *val = reinterpret_cast<val_policingprofile_t *>
    (ConfigKeyVal::Malloc(sizeof(val_policingprofile_t)));
  val->cs_row_status = UNC_CS_APPLIED;
  ikey->AppendCfgVal(IpctSt::kIpcStValPolicingprofile, val);
  return UPLL_RC_SUCCESS;
}

bool PolicingProfileMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return true;
  return false;
}

upll_rc_t PolicingProfileMoMgr::SetPPConsolidatedStatus(ConfigKeyVal *ikey,
                                                        uint8_t *ctrlr_id,
                                                        DalDmlIntf *dmi)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ctrlr_ckv = NULL;
  val_policingprofile_ctrl_t *ctrlr_val = NULL;
  uint8_t *pp_exist_on_ctrlr = NULL;
  bool applied = false, not_applied = false, invalid = false;
  unc_keytype_configstatus_t c_status = UNC_CS_NOT_APPLIED;
  string vtn_name = "";

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
                   kOpInOutCtrlr | kOpInOutCs };
  if (!ikey || !dmi) {
    UPLL_LOG_DEBUG("Invalid Input");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(ctrlr_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code %d", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(ctrlr_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB from ctrltbl failed err code %d",
                   result_code);
    DELETE_IF_NOT_NULL(ctrlr_ckv);
    return result_code;
  }

  for (ConfigKeyVal *tmp = ctrlr_ckv; tmp != NULL;
                     tmp = tmp->get_next_cfg_key_val()) {
    ctrlr_val = reinterpret_cast<val_policingprofile_ctrl_t *>(GetVal(tmp));
    if (!ctrlr_val) {
      UPLL_LOG_ERROR("Controller Value is empty");
      tmp = NULL;
      DELETE_IF_NOT_NULL(ctrlr_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    GET_USER_DATA_CTRLR(tmp, pp_exist_on_ctrlr);
    if (!strcmp(reinterpret_cast<char *>(pp_exist_on_ctrlr),
                reinterpret_cast<char *>(ctrlr_id)))
      continue;  // skipping entry of deleted controller

    switch (ctrlr_val->cs_row_status) {
      case UNC_CS_APPLIED:
        applied = true;
      break;
      case UNC_CS_NOT_APPLIED:
        not_applied = true;
      break;
      case UNC_CS_INVALID:
        invalid = true;
      break;
      default:
        UPLL_LOG_DEBUG("Invalid status");
        DELETE_IF_NOT_NULL(ctrlr_ckv);
       //  return UPLL_RC_ERR_GENERIC;
    }
    pp_exist_on_ctrlr = NULL;
  }
  if (invalid) {
    c_status = UNC_CS_INVALID;
  } else if (applied && !not_applied) {
    c_status = UNC_CS_APPLIED;
  } else if (!applied && not_applied) {
    c_status = UNC_CS_NOT_APPLIED;
  } else if (applied && not_applied) {
    c_status = UNC_CS_PARTIALLY_APPLIED;
  } else {
    c_status = UNC_CS_APPLIED;
  }
  // Set cs_status
  val_policingprofile_t *val = static_cast<val_policingprofile_t *>
                                  (GetVal(ikey));
  val->cs_row_status = c_status;
  DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs};
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                               &dbop_update, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  DELETE_IF_NOT_NULL(ctrlr_ckv);
  return result_code;
}


//  Increments the ref_count by 1 for given policingprofile_name and
//  ctrlr_name in the given datatype.
upll_rc_t PolicingProfileMoMgr::UpdateRefCountInCtrlrTbl(ConfigKeyVal *ikey,
     DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
     TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input key is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
                               (ikey->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("UserData is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  void *tkey = ikey->get_key();
  void *p = NULL;
  const uudst::kDalTableIndex tbl_index = GetTable(CTRLRTBL, dt_type);
  DalBindInfo *db_info = new DalBindInfo(tbl_index);
  BindInfo *binfo = policingprofile_controller_bind_info;
  uint8_t array_index = 0;

  //  Bind match policingprofile_name
  uint64_t indx = binfo[array_index].index;
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
      + binfo[array_index].offset);
  db_info->BindMatch(indx, binfo[array_index].app_data_type,
                     binfo[array_index].array_size, p);

  //  Bind match ctrlr_name
  ++array_index;
  indx = binfo[array_index].index;
  p = reinterpret_cast<void *>(reinterpret_cast<char *>(tuser_data)
      + binfo[array_index].offset);
  db_info->BindMatch(indx, binfo[array_index].app_data_type,
                     binfo[array_index].array_size, p);

  //  Incrment the ref_count by 1 for the matched policingprofile_name
  //  and ctrlr_name for the given datatype
  std::string query_string;
  if (dt_type == UPLL_DT_CANDIDATE) {
    query_string = QUERY_PP_CAND_REF_COUNT_UPDATE;
  } else if (dt_type == UPLL_DT_AUDIT) {
    query_string = QUERY_PP_AUD_REF_COUNT_UPDATE;
  } else if (dt_type == UPLL_DT_IMPORT) {
    query_string = QUERY_PP_IMP_REF_COUNT_UPDATE;
  }
  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
      const_cast<char *>(vtn_name.c_str()));
  }
  upll_rc_t result_code = DalToUpllResCode(
         dmi->ExecuteAppQuery(query_string, dt_type, tbl_index,
                              db_info, UNC_OP_UPDATE, config_mode,
                              vtnname));
  DELETE_IF_NOT_NULL(db_info);
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
                                             unc_keytype_operation_t &op) {
  if (uuc::kUpllUcpDelete == phase) {
    UPLL_LOG_DEBUG("Delete phase 1");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpUpdate == phase) {
    UPLL_LOG_DEBUG("Update phase");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpCreate == phase) {
    op = UNC_OP_CREATE;
  } else if (uuc::kUpllUcpDelete2 == phase) {
    op = UNC_OP_DELETE;
  } else if (uuc::kUpllUcpInit == phase) {
    // return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
    // return success;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::CopyKeyToVal(ConfigKeyVal *ikey,
                                 ConfigKeyVal *&okey) {
  if (!ikey)
    return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  val_rename_policingprofile *val =
      reinterpret_cast<val_rename_policingprofile_t *>
      (ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile)));
  // Note: Validate message is take care of validate the key part
  key_policingprofile_t *key =
      reinterpret_cast<key_policingprofile_t *>(ikey->get_key());
  uuu::upll_strncpy(val->policingprofile_newname, key->policingprofile_name,
                    (kMaxLenPolicingProfileName+1));
  val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
  okey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValRenamePolicingprofile, val));
  return UPLL_RC_SUCCESS;
}


// Update the ref_count in scratch table depending on operation.
// If operation is create then increment the ref_count. If delete
// decrement the ref_count.
upll_rc_t PolicingProfileMoMgr::UpdateRefCountInScratchTbl(
     ConfigKeyVal *ikey,
     DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
     unc_keytype_operation_t op,
     TcConfigMode config_mode, string vtn_name,
     uint32_t count) {
  UPLL_FUNC_TRACE;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input key is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
                               (ikey->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("UserData is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  void *tkey = ikey->get_key();
  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiPpScratchTbl);

  //  Bind match policingprofile_name
  db_info->BindMatch(uudst::pp_scratch::kDbiPolicingProfileName,
                     uud::kDalChar,
                     (kMaxLenPolicingProfileName + 1),
                     reinterpret_cast<void *>(reinterpret_cast
                                              <key_policingprofile_t*>
                                              (tkey)->policingprofile_name));

  //  Bind match ctrlr_name
  db_info->BindMatch(uudst::pp_scratch::kDbiCtrlrName,
                     uud::kDalChar,
                     (kMaxLenCtrlrId + 1),
                     &(tuser_data->ctrlr_id));

  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
      const_cast<char *>(vtn_name.c_str()));
  } else {
    UPLL_LOG_DEBUG("Invalid vtn name");
    DELETE_IF_NOT_NULL(db_info);
    return UPLL_RC_ERR_GENERIC;
  }

  //  Bind match vtn_name
  db_info->BindMatch(uudst::pp_scratch::kDbiVtnName,
                     uud::kDalChar,
                     (kMaxLenVtnName + 1),
                     vtnname);

  //  Incrment the ref_count by 1 for the matched policingprofile_name,
  //  ctrlr_name and vtn_name for the given datatype
  std::stringstream ss;
  ss << count;
  std::string query_string;
  UPLL_LOG_DEBUG(" Count : %s %d", (ss.str()).c_str(), count);
  if (op == UNC_OP_CREATE) {
    query_string += "UPDATE ca_pp_scratch_tbl SET ref_count = ref_count + ";
    query_string += (ss.str());
    query_string += " WHERE policingprofile_name = ? AND ctrlr_name = ?"\
                     " AND vtn_name = ?";
  } else if (op == UNC_OP_DELETE) {
    query_string += "UPDATE ca_pp_scratch_tbl SET ref_count = ref_count - ";
    query_string += (ss.str());
    query_string += " WHERE policingprofile_name = ? AND ctrlr_name = ?"\
                     " AND vtn_name = ?";
  } else {
    UPLL_LOG_DEBUG("Invalid operation");
    DELETE_IF_NOT_NULL(db_info);
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = DalToUpllResCode(
         dmi->ExecuteAppQuery(query_string, dt_type, uudst::kDbiPpScratchTbl,
                              db_info, UNC_OP_UPDATE, config_mode,
                              vtnname));
  DELETE_IF_NOT_NULL(db_info);
  return result_code;
}

//  Insert a new record in scratch table with ref_count value depending
//  on operation.
upll_rc_t PolicingProfileMoMgr::InsertRecInScratchTbl(ConfigKeyVal *ikey,
     DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
     unc_keytype_operation_t op,
     TcConfigMode config_mode, string vtn_name,
     uint32_t count) {
  UPLL_FUNC_TRACE;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input key is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
                               (ikey->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("UserData is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  void *tkey = ikey->get_key();
  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiPpScratchTbl);

  //  Bind Input policingprofile_name
  db_info->BindInput(uudst::pp_scratch::kDbiPolicingProfileName,
                     uud::kDalChar,
                     (kMaxLenPolicingProfileName + 1),
                     reinterpret_cast<void *>(reinterpret_cast
                                              <key_policingprofile_t*>
                                              (tkey)->policingprofile_name));

  //  Bind Input ctrlr_name
  db_info->BindInput(uudst::pp_scratch::kDbiCtrlrName,
                     uud::kDalChar,
                     (kMaxLenCtrlrId + 1),
                     &(tuser_data->ctrlr_id));

  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
      const_cast<char *>(vtn_name.c_str()));
  } else {
    UPLL_LOG_DEBUG("Invalid vtn name");
    DELETE_IF_NOT_NULL(db_info);
    return UPLL_RC_ERR_GENERIC;
  }

  //  Bind Input vtn_name
  db_info->BindInput(uudst::pp_scratch::kDbiVtnName,
                     uud::kDalChar,
                     (kMaxLenVtnName + 1),
                     vtnname);

  // Insert a new record in scratch tbl with ref_count depending on
  // operation.
  std::stringstream ss;
  ss << count;
  std::string query_string;
  if (op == UNC_OP_CREATE) {
    query_string += "INSERT INTO ca_pp_scratch_tbl "\
                     "(policingprofile_name, ctrlr_name, vtn_name, ref_count) "\
                     "VALUES (?, ?, ?, ";
    query_string += (ss.str());
    query_string += ")";
  } else {
    query_string += "INSERT INTO ca_pp_scratch_tbl "\
                     "(policingprofile_name, ctrlr_name, vtn_name, ref_count) "\
                     "VALUES (?, ?, ?, -";
    query_string += (ss.str());
    query_string += ")";
  }

  upll_rc_t result_code = DalToUpllResCode(
         dmi->ExecuteAppQuery(query_string, dt_type, uudst::kDbiPpScratchTbl,
                              db_info, UNC_OP_UPDATE, config_mode,
                              vtnname));
  DELETE_IF_NOT_NULL(db_info);
  return result_code;
}

// Add the ref_count in scratch table for the given flowlist and
// ctrlr_name for the given configuration mode.
upll_rc_t PolicingProfileMoMgr::ComputeRefCountInScratchTbl(
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
    TcConfigMode config_mode, string vtn_name,
    int &ref_count) {
  UPLL_FUNC_TRACE;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input key is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
      (ikey->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("UserData is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  void *tkey = ikey->get_key();

  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiPpScratchTbl);
  //  Bind Match policingprofile_name
  db_info->BindMatch(uudst::pp_scratch::kDbiPolicingProfileName,
                     uud::kDalChar,
                     (kMaxLenPolicingProfileName + 1),
                     reinterpret_cast<void *>(reinterpret_cast
                                              <key_policingprofile_t*>
                                              (tkey)->policingprofile_name));

  //  Bind Match ctrlr_name
  db_info->BindMatch(uudst::pp_scratch::kDbiCtrlrName,
                     uud::kDalChar,
                     (kMaxLenCtrlrId + 1),
                     &(tuser_data->ctrlr_id));

  std::string query_string;
  query_string = QUERY_SUM_PP_SCRATCH_REF_COUNT_WITH_CTRLR;
  uint8_t *vtnname = NULL;
  if (config_mode == TC_CONFIG_VTN) {
    if (!vtn_name.empty()) {
      vtnname = reinterpret_cast<uint8_t *>(
          const_cast<char *>(vtn_name.c_str()));
    } else {
      UPLL_LOG_DEBUG("vtn name is NULL");
      DELETE_IF_NOT_NULL(db_info);
      return UPLL_RC_ERR_GENERIC;
    }

    //  Bind Match vtn_name
    db_info->BindMatch(uudst::pp_scratch::kDbiVtnName,
                       uud::kDalChar,
                       (kMaxLenVtnName + 1),
                       vtnname);
    query_string = QUERY_READ_REF_COUNT_PP_SCRATCH_TBL;
  }

  int db_ref_count = 0;
  //  Bind Output ref_count
  db_info->BindOutput(uudst::pp_scratch::kDbiRefCount,
                      uud::kDalUint32,
                      1,
                      &db_ref_count);
  //  Incrment the ref_count by 1 for the matched policingprofile_name
  //  and ctrlr_name for the given datatype

  upll_rc_t result_code = DalToUpllResCode(dmi->
                                 ExecuteAppQuerySingleRecord(query_string,
                                                             db_info));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Reading Single records failed %d", result_code);
    DELETE_IF_NOT_NULL(db_info);
    return result_code;
  }
  ref_count = db_ref_count;
  DELETE_IF_NOT_NULL(db_info);
  return result_code;
}

// Read Multiple records from controller table
upll_rc_t PolicingProfileMoMgr::ReadCtrlrTbl(
    ConfigKeyVal *&okey,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type) {

  UPLL_FUNC_TRACE;

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr };
  upll_rc_t result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
      CTRLRTBL);
  UPLL_LOG_DEBUG("ReadConfigDB returned %d", result_code);
  return result_code;
}

// Clear the records in scratch table depending upon config_mode
upll_rc_t PolicingProfileMoMgr::ClearScratchTbl(
    TcConfigMode config_mode, string vtn_name,
    DalDmlIntf *dmi, bool is_abort) {

  UPLL_FUNC_TRACE;

  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiPpScratchTbl);

  uint8_t *vtnname = NULL;
  std::string query_string = QUERY_DELETE_ALL_PP_SCRATCH_TBL;
  if (config_mode == TC_CONFIG_VTN) {
    if (!vtn_name.empty()) {
      vtnname = reinterpret_cast<uint8_t *>(
          const_cast<char *>(vtn_name.c_str()));
    } else {
      UPLL_LOG_DEBUG("Invalid vtn name");
      DELETE_IF_NOT_NULL(db_info);
      return UPLL_RC_ERR_GENERIC;
    }
    //  Bind Match vtn_name
    db_info->BindMatch(uudst::pp_scratch::kDbiVtnName,
                       uud::kDalChar,
                       (kMaxLenVtnName + 1),
                       vtnname);
    query_string = QUERY_DELETE_VTN_PP_SCRATCH_TBL;
  }
  upll_rc_t result_code = DalToUpllResCode(
      dmi->ExecuteAppQuery(query_string, UPLL_DT_CANDIDATE,
                           uudst::kDbiPpScratchTbl, db_info, UNC_OP_DELETE,
                           config_mode, vtnname));

  DELETE_IF_NOT_NULL(db_info);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    result_code = UPLL_RC_SUCCESS;
  }
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::RefCountSemanticCheck(
    const char* policingprofile_name, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name) {

  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiPolicingProfileCtrlrTbl);

  db_info->BindMatch(uudst::policingprofile_ctrlr::kDbiPolicingProfileName,
                     uud::kDalChar,
                     (kMaxLenPolicingProfileName + 1),
                     policingprofile_name);


  uint32_t ct_ref_count = 0;
  db_info->BindOutput(uudst::policingprofile_ctrlr::kDbiRefCount,
                      uud::kDalUint32,
                      1,
                      &ct_ref_count);

  std::string query_string = QUERY_SUM_PP_CTRLR_REF_COUNT;
  result_code = DalToUpllResCode(
         dmi->ExecuteAppQuerySingleRecord(query_string, db_info));
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ExecuteAppQuery returned %d", result_code);
    DELETE_IF_NOT_NULL(db_info);
    return result_code;
  }

  DalBindInfo *st_db_info = new DalBindInfo(uudst::kDbiPpScratchTbl);
  st_db_info->BindMatch(uudst::pp_scratch::kDbiPolicingProfileName,
                       uud::kDalChar,
                       (kMaxLenPolicingProfileName + 1),
                       policingprofile_name);

  int st_ref_count = 0;
  st_db_info->BindOutput(uudst::pp_scratch::kDbiRefCount,
                        uud::kDalUint32,
                        1,
                        &st_ref_count);

  query_string = QUERY_SUM_PP_SCRATCH_REF_COUNT;
  result_code = DalToUpllResCode(
         dmi->ExecuteAppQuerySingleRecord(query_string, st_db_info));
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ExecuteAppQuery returned %d", result_code);
    DELETE_IF_NOT_NULL(db_info);
    DELETE_IF_NOT_NULL(st_db_info);
    return result_code;
  }

  if (0 != (ct_ref_count + st_ref_count)) {
    UPLL_LOG_DEBUG("Policing profile is matched");
    DELETE_IF_NOT_NULL(st_db_info);
    DELETE_IF_NOT_NULL(db_info);
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }

  DELETE_IF_NOT_NULL(st_db_info);
  DELETE_IF_NOT_NULL(db_info);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

upll_rc_t PolicingProfileMoMgr::InstanceExistsInScratchTbl(
    ConfigKeyVal *ikey, TcConfigMode config_mode, string vtn_name,
    DalDmlIntf *dmi) {

  UPLL_FUNC_TRACE;

  ConfigKeyVal *ckv = NULL;
  upll_rc_t result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
      (ckv->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("UserData is NULL");
    DELETE_IF_NOT_NULL(ckv);
    return UPLL_RC_ERR_GENERIC;
  }

  void *tkey = ckv->get_key();

  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiPpScratchTbl);

  //  Bind Match policingprofile_name
  db_info->BindMatch(uudst::pp_scratch::kDbiPolicingProfileName,
                     uud::kDalChar,
                     (kMaxLenPolicingProfileName + 1),
                     reinterpret_cast<void *>(reinterpret_cast
                                              <key_policingprofile_t*>
                                              (tkey)->policingprofile_name));

  //  Bind Match ctrlr_name
  db_info->BindMatch(uudst::pp_scratch::kDbiCtrlrName,
                     uud::kDalChar,
                     (kMaxLenCtrlrId + 1),
                     &(tuser_data->ctrlr_id));

  std::string query_string;
  query_string = QUERY_READ_NO_VTN_REF_COUNT_PP_SCRATCH_TBL;
  uint8_t *vtnname = NULL;
  if (config_mode == TC_CONFIG_VTN) {
    if (!vtn_name.empty()) {
      vtnname = reinterpret_cast<uint8_t *>(
          const_cast<char *>(vtn_name.c_str()));
    } else {
      UPLL_LOG_DEBUG("Invalid vtn name");
      DELETE_IF_NOT_NULL(db_info);
      DELETE_IF_NOT_NULL(ckv);
    }

    //  Bind Match vtn_name
    db_info->BindMatch(uudst::pp_scratch::kDbiVtnName,
                       uud::kDalChar,
                       (kMaxLenVtnName + 1),
                       vtnname);
    query_string = QUERY_READ_REF_COUNT_PP_SCRATCH_TBL;
  }

  result_code = DalToUpllResCode(
      dmi->ExecuteAppQuerySingleRecord(query_string, db_info));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Reading scratch tbl failed %d", result_code);
    DELETE_IF_NOT_NULL(db_info);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }
  DELETE_IF_NOT_NULL(ckv);
  DELETE_IF_NOT_NULL(db_info);
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::ClearVirtualKtDirtyInGlobal(DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;

  unc_keytype_operation_t op_arr[] = { UNC_OP_CREATE,
                                       UNC_OP_DELETE,
                                       UNC_OP_UPDATE};
  uint32_t nop = 3;

  uudst::kDalTableIndex tbl_idx = GetTable(MAINTBL, UPLL_DT_CANDIDATE);
  for (uint32_t i = 0; i < nop; i++) {
    result_code = DalToUpllResCode(dmi->ClearGlobalDirtyTblCacheAndDB(
                                            tbl_idx, op_arr[i]));
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ClearGlobalDirtyTblCacheAndDB failed %d", result_code);
      return result_code;
    }
  }

  GetTable(CTRLRTBL, UPLL_DT_CANDIDATE);
  result_code = DalToUpllResCode(dmi->ClearGlobalDirtyTblCacheAndDB(
                                          tbl_idx, UNC_OP_UPDATE));
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ClearGlobalDirtyTblCacheAndDB failed %d", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::ComputeCtrlrTblRefCountFromScratchTbl(
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
    TcConfigMode config_mode, string vtn_name) {

  UPLL_FUNC_TRACE;

  uint8_t *vtnname = NULL;
  if (config_mode == TC_CONFIG_VTN) {
    if (!vtn_name.empty()) {
      vtnname = reinterpret_cast<uint8_t *>(
          const_cast<char *>(vtn_name.c_str()));
    } else {
      UPLL_LOG_DEBUG("vtn name is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    vtn_name = "global_temp_vtn";
    vtnname = reinterpret_cast<uint8_t *>(
          const_cast<char *>(vtn_name.c_str()));
  }
  // If scratch tbl is not dirty then skip the scratch tbl computation
  if (!dmi->IsTableDirtyShallow(uudst::kDbiPpScratchTbl,
                                config_mode, vtnname)) {
    UPLL_LOG_DEBUG("No entries in scratch tbl");
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *ckv = NULL;
  upll_rc_t result_code = GetChildConfigKey(ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  GET_USER_DATA(ckv);
// Read records from scratch tbl
  key_user_data *tuser_data  = reinterpret_cast<key_user_data_t *>
      (ckv->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("UserData is NULL");
    DELETE_IF_NOT_NULL(ckv);
    return UPLL_RC_ERR_GENERIC;
  }

  void *tkey = ckv->get_key();
  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiPpScratchTbl);

  //  Bind Match policingprofile_name
  db_info->BindOutput(uudst::pp_scratch::kDbiPolicingProfileName,
                      uud::kDalChar,
                      (kMaxLenPolicingProfileName + 1),
                      reinterpret_cast<void *>(reinterpret_cast
                                               <key_policingprofile_t*>
                                               (tkey)->policingprofile_name));

  //  Bind Match ctrlr_name
  db_info->BindOutput(uudst::pp_scratch::kDbiCtrlrName,
                      uud::kDalChar,
                      (kMaxLenCtrlrId + 1),
                      &(tuser_data->ctrlr_id));

  std::string query_string;
  query_string = QUERY_READ_AND_SUM_REF_COUNT_PP_SCRATCH_TBL;
  if (config_mode == TC_CONFIG_VTN) {
    //  Bind Match vtn_name
    db_info->BindMatch(uudst::pp_scratch::kDbiVtnName,
                       uud::kDalChar,
                       (kMaxLenVtnName + 1),
                       vtnname);
    query_string = QUERY_READ_PP_SCRATCH_TBL_VTN_MODE;
  }
  int st_ref_count = 0;
  db_info->BindOutput(uudst::pp_scratch::kDbiRefCount,
                      uud::kDalUint32,
                      1,
                      &st_ref_count);
  DalCursor *dal_cursor_handle = NULL;
  result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
                 query_string, 0, db_info, &dal_cursor_handle));
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (UPLL_RC_SUCCESS == result_code) {
      if (st_ref_count == 0) {
        continue;
      }
      ConfigKeyVal *ctrlr_ckv = NULL;
      result_code = GetChildConfigKey(ctrlr_ckv, ckv);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetChildConfigkey failed %d", result_code);
        DELETE_IF_NOT_NULL(db_info);
        DELETE_IF_NOT_NULL(ckv);
        dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }
      if (config_mode != TC_CONFIG_VTN) {
        vtn_name = reinterpret_cast<const char *>(vtnname);
      }
      DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
      result_code = ReadConfigDB(ctrlr_ckv, dt_type, UNC_OP_READ, dbop, dmi,
                                 CTRLRTBL);
      if ((UPLL_RC_SUCCESS != result_code) &&
          (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(db_info);
        DELETE_IF_NOT_NULL(ckv);
        DELETE_IF_NOT_NULL(ctrlr_ckv);
        dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }
      if (UPLL_RC_SUCCESS == result_code) {
        val_policingprofile_ctrl_t *val_ctrlr = reinterpret_cast
            <val_policingprofile_ctrl_t *>(GetVal(ctrlr_ckv));
        unc_keytype_operation_t op;
        if (0 == st_ref_count + val_ctrlr->ref_count) {
          op = UNC_OP_DELETE;
        } else {
          val_ctrlr->ref_count += st_ref_count;
          val_ctrlr->valid[0] = UNC_VF_VALID;
          op = UNC_OP_UPDATE;
        }
        DbSubOp dbop1 = {kOpNotRead, kOpMatchCtrlr, kOpInOutNone};
        result_code = UpdateConfigDB(ctrlr_ckv, dt_type, op, dmi, &dbop1,
                                     config_mode, vtn_name, CTRLRTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
          DELETE_IF_NOT_NULL(db_info);
          DELETE_IF_NOT_NULL(ckv);
          DELETE_IF_NOT_NULL(ctrlr_ckv);
          dmi->CloseCursor(dal_cursor_handle, false);
          return result_code;
        }
        if (op == UNC_OP_DELETE) {
          PolicingProfileEntryMoMgr *ppe_mgr =
              reinterpret_cast<PolicingProfileEntryMoMgr *>
              (const_cast<MoManager *>(GetMoManager(
                          UNC_KT_POLICING_PROFILE_ENTRY)));
          key_policingprofile_t *key_policingprofile =
              reinterpret_cast<key_policingprofile_t *>(ctrlr_ckv->get_key());
          result_code = ppe_mgr->PolicingProfileEntryCtrlrTblOper(
            reinterpret_cast<char*>(key_policingprofile->policingprofile_name),
            reinterpret_cast<char*>(tuser_data->ctrlr_id), dmi, UNC_OP_DELETE,
            dt_type, config_mode, vtn_name, false);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("PolicingProfileEntryCtrlrTblOper Delete failed %d",
                            result_code);
            DELETE_IF_NOT_NULL(db_info);
            DELETE_IF_NOT_NULL(ckv);
            DELETE_IF_NOT_NULL(ctrlr_ckv);
            dmi->CloseCursor(dal_cursor_handle, false);
            return result_code;
          }
          result_code = UpdateConfigDB(ctrlr_ckv, dt_type, UNC_OP_DELETE,
                                       dmi, &dbop1, config_mode, vtn_name,
                                       RENAMETBL);
          if (UPLL_RC_SUCCESS != result_code &&
              UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_INFO("UpdateConfigDB Failed %d", result_code);
            DELETE_IF_NOT_NULL(db_info);
            DELETE_IF_NOT_NULL(ckv);
            DELETE_IF_NOT_NULL(ctrlr_ckv);
            dmi->CloseCursor(dal_cursor_handle, false);
            return result_code;
          }
          if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
            result_code = UPLL_RC_SUCCESS;
        }
        DELETE_IF_NOT_NULL(ctrlr_ckv);
      } else {
        result_code = CtrlrTblCreate(ctrlr_ckv, dmi, dt_type, config_mode,
                                     vtn_name, true, st_ref_count);
        if ((UPLL_RC_SUCCESS != result_code) &&
            (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
          UPLL_LOG_DEBUG("CtrlrTblCreate failed %d", result_code);
          DELETE_IF_NOT_NULL(db_info);
          DELETE_IF_NOT_NULL(ckv);
          DELETE_IF_NOT_NULL(ctrlr_ckv);
          dmi->CloseCursor(dal_cursor_handle, false);
          return result_code;
        }
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
          result_code = UPLL_RC_SUCCESS;
        DELETE_IF_NOT_NULL(ctrlr_ckv);
      }
    } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetNextRecord failed");
      dmi->CloseCursor(dal_cursor_handle, false);
      DELETE_IF_NOT_NULL(db_info);
      DELETE_IF_NOT_NULL(ckv);
      return result_code;
    }
  }
  if (dal_cursor_handle) {
    dmi->CloseCursor(dal_cursor_handle, false);
  }
  DELETE_IF_NOT_NULL(db_info);
  DELETE_IF_NOT_NULL(ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::DeleteChildrenPOM(ConfigKeyVal *ikey,
                      upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
                      TcConfigMode config_mode, string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  for (int i = 0; i < ntable; i++) {
    if (GetTable((MoMgrTables)i, UPLL_DT_CANDIDATE) >= uudst::kDalNumTables) {
      continue;
    }
    // skip the deletion for convert table, it is deleted as part of vbr_portmap
    // delete
    DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
    result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE,
             UNC_OP_DELETE, dmi, &dbop, config_mode,
             vtn_name, (MoMgrTables)i);
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                  UPLL_RC_SUCCESS:result_code;
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Delete Operation fails with %d", result_code);
      return result_code;
    }
  }

  void *tkey = ikey->get_key();
  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiPpScratchTbl);
  db_info->BindMatch(uudst::pp_scratch::kDbiPolicingProfileName,
                     uud::kDalChar, (kMaxLenPolicingProfileName + 1),
                     reinterpret_cast<void *>(reinterpret_cast
                     <key_policingprofile_t*>(tkey)->policingprofile_name));

  std::string query_string = QUERY_DELETE_PP_SCRATCH_TBL_MATCH_PP;
  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
        const_cast<char *>(vtn_name.c_str()));
  }
  result_code = DalToUpllResCode(
         dmi->ExecuteAppQuery(query_string, dt_type, uudst::kDbiPpScratchTbl,
                              db_info, UNC_OP_DELETE, config_mode,
                              vtnname));
  DELETE_IF_NOT_NULL(db_info);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}
}  //  namespace kt_momgr
}  //  namespace upll
}  //  namespace unc
