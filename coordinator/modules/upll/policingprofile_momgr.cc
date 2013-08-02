/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <set>
#include "policingprofile_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "vtn_policingmap_momgr.hh"
#include "upll_log.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

#define  NUM_KEY_MAIN_COL 3
#define  NUM_KEY_CTRL_COL 4
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
    offsetof(val_policingprofile_ctrl_t, flags),
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
    CFG_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

BindInfo PolicingProfileMoMgr::rename_policingprofile_ctrlr_tbl[] = {
  { uudst::policingprofile_ctrlr::kDbiPolicingProfileName,
    CFG_MATCH_KEY,
    offsetof(key_policingprofile_t, policingprofile_name),
    uud::kDalChar,
    (kMaxLenPolicingProfileName + 1) },
  { uudst::policingprofile_ctrlr::kDbiCtrlrName,
    CFG_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar,
    (kMaxLenCtrlrId + 1) },
  { uudst::policingprofile_ctrlr::kDbiPolicingProfileName,
    CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_policingprofile_name),
    uud::kDalChar,
    (kMaxLenPolicingProfileName + 1) },
  { uudst::policingprofile_ctrlr::kDbiFlags,
    CFG_VAL,
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
  table = new Table *[ntable];

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
      UPLL_LOG_DEBUG(" valid in GetValid %d ", val_ctrl->valid[0]);
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
//    if (policingprofile_key) free(policingprofile_key);
    UPLL_LOG_TRACE(" Key structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey) {
    if (okey->get_key_type() != UNC_KT_POLICING_PROFILE)
      return UPLL_RC_ERR_GENERIC;
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

  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
                            IpctSt::kIpcStKeyPolicingprofile,
                            policingprofile_key, NULL);

  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_TRACE(" PolicingProfileMoMgr::ConfigKeyVal Allocation Successful ");
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::GetRenamedUncKey(
    ConfigKeyVal *ctrlr_key, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ctrlr_key) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::Input ConfigKeyVal is NULL.");
    return result_code;
  }
  ConfigKeyVal *unc_key = NULL;
  key_policingprofile_t *ctrlr_policingprofile_key =
    reinterpret_cast<key_policingprofile_t *>(ctrlr_key->get_key());
  if (NULL == ctrlr_policingprofile_key) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::Key struct is NULL");
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };

  val_rename_policingprofile_t *rename_policingprofile =
  reinterpret_cast <val_rename_policingprofile_t *>
  (ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile_t)));

  uuu::upll_strncpy(rename_policingprofile->policingprofile_newname,
                    ctrlr_policingprofile_key->policingprofile_name,
                    (kMaxLenPolicingProfileName+1));
  rename_policingprofile->valid[UPLL_IDX_RENAME_PROFILE_RPP] = UNC_VF_VALID;

  result_code = GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr:: PolicingProfile Creating"
      " ConfigKeyval failed.");
    free(rename_policingprofile);
    return result_code;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenamePolicingprofile,
      rename_policingprofile);

  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);

  result_code = ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);

  if (result_code == UPLL_RC_SUCCESS) {
    key_policingprofile_t *policingprofile_key =
      reinterpret_cast<key_policingprofile_t *>(unc_key->get_key());
    uuu::upll_strncpy(ctrlr_policingprofile_key->policingprofile_name,
                      policingprofile_key->policingprofile_name,
                      (kMaxLenPolicingProfileName+1));
  }
  delete unc_key;
  UPLL_LOG_TRACE(" PolicingProfileMoMgr::GetRenamedUncKey Successful");
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *&ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::GetRenamedControllerKey Failed."
      " Input ConfigKeyVal is NULL");
    return result_code;
  }
  if (NULL == dmi) {
    return result_code;
  }
  ConfigKeyVal *okey = NULL;
  uint8_t rename = 0;
  result_code = IsRenamed(ikey, dt_type, dmi, rename);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::GetRenamedControllerKey Failed."
      " IsRenamed function failed - %d", result_code);
    return result_code;
  }
  if (!rename) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::GetRenamedControllerKey Success."
      " The Key is not renamed");
    return UPLL_RC_SUCCESS;
  }
  /* PolicingProfile renamed */
  key_policingprofile_t *ctrlr_key = reinterpret_cast<key_policingprofile_t *>
    (ConfigKeyVal::Malloc(sizeof(key_policingprofile_t)));
  if (rename & POLICINGPROFILE_RENAME) {
    result_code = GetChildConfigKey(okey, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" PolicingProfileMoMgr::GetRenamedControllerKey Failed."
      " Memory allocation for ConfigKeyVal failed - %d", result_code);
      free(ctrlr_key);
      return result_code;
    }
    if (NULL == ctrlr_dom) {
      UPLL_LOG_DEBUG(" Ctrlr_id is NULL");
      free(ctrlr_key);
      return UPLL_RC_ERR_GENERIC;
    }
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);

    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag };
    result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
        RENAMETBL); /* ctrlr_name */
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" PolicingProfileMoMgr::GetRenamedControllerKey Failed."
      " ReadConfigDB function failed - %d", result_code);
      free(ctrlr_key);
      return result_code;
    }
    val_rename_policingprofile_t *rename_val =
      reinterpret_cast<val_rename_policingprofile_t *>(GetVal(okey));
    if (NULL == rename_val) {
      UPLL_LOG_DEBUG(" PolicingProfileMoMgr::GetRenamedControllerKey Failed."
        " Memory Allocation failed for rename val struct");
      free(ctrlr_key);
      return UPLL_RC_ERR_GENERIC;
    }
    if (rename_val->valid[UPLL_IDX_RENAME_PROFILE_RPP] != UNC_VF_VALID) {
      UPLL_LOG_DEBUG(" PolicingProfileMoMgr::GetRenamedControllerKey Failed."
        " Rename is INVALID");
      free(ctrlr_key);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(ctrlr_key->policingprofile_name,
                      rename_val->policingprofile_newname,
                      (kMaxLenPolicingProfileName+1));
    delete okey;
  }
  free(ikey->get_key());
  ikey->SetKey(IpctSt::kIpcStValPolicingprofile,
      reinterpret_cast<void *> (ctrlr_key));
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
      val_rename_policingprofile_t *rename_val =
        reinterpret_cast<val_rename_policingprofile_t *>
          (ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile_t)));
      memcpy(rename_val, ival, sizeof(val_rename_policingprofile_t));
      tmp1 = new ConfigVal(IpctSt::kIpcStValRenamePolicingprofile, rename_val);
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
  if (NULL == okey) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::DupConfigKeyVal Failed.");
    UPLL_LOG_DEBUG(" ConfigKeyVal allocation failed");
    free(policingprofile_key);
    return UPLL_RC_ERR_GENERIC;
  }
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
        ConfigKeyVal *&ckv_running) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingprofile_t *val = NULL;
  val = (ckv_running != NULL)?reinterpret_cast<val_policingprofile_t *>
      (GetVal(ckv_running)):NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
     val->cs_row_status = cs_status;

  return result_code;
}

upll_rc_t PolicingProfileMoMgr::MergeValidate(unc_key_type_t keytype,
    const char *ctrlr_id, ConfigKeyVal *okey,
    DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *ckval = NULL;
  if (NULL == ctrlr_id) {
    return result_code;
  }
  result_code = GetChildConfigKey(ckval, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(ckval, UPLL_DT_IMPORT,
              UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  okey = ckval;
  while (NULL != okey) {
    result_code = UpdateConfigDB(okey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
                                 MAINTBL);
    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
      return UPLL_RC_ERR_MERGE_CONFLICT;
    } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      okey = ckval->get_next_cfg_key_val();
    } else {
      return result_code;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::SwapKeyVal(ConfigKeyVal *ikey,
    ConfigKeyVal *&okey,
    uud::DalDmlIntf *dmi,
    uint8_t *ctrlr, bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) {
    UPLL_LOG_DEBUG(
        " PolicingProfileMoMgr::SwapKeyVal Failed. Input ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (NULL != okey) {
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *cfg_val = ikey->get_cfg_val();
  if (NULL == cfg_val) {
    UPLL_LOG_DEBUG(
        " PolicingProfileMoMgr::SwapKeyVal Failed. Val struct is NULL");
    // return UPLL_RC_ERR_BAD_REQUEST;
    return UPLL_RC_ERR_GENERIC;
  }
  val_rename_policingprofile_t *tval =
    reinterpret_cast<val_rename_policingprofile_t *>(cfg_val->get_val());
  if (NULL == tval) {
    UPLL_LOG_DEBUG(
        " PolicingProfileMoMgr::SwapKeyVal Failed. Val struct is NULL");
    // return UPLL_RC_ERR_BAD_REQUEST;
    return UPLL_RC_ERR_GENERIC;
  }
  if (NULL == ikey->get_key()) {
    return UPLL_RC_ERR_GENERIC;
  }
  key_policingprofile_t *pp_key =
      reinterpret_cast<key_policingprofile_t *>(ikey->get_key());
  if (!strlen(reinterpret_cast<char *>(pp_key->policingprofile_name))) {
    return UPLL_RC_ERR_GENERIC;
  }
  /* The New Name and PFC name should not be same name */
  if (strcmp(
        reinterpret_cast<char *>
        (pp_key->policingprofile_name),
        reinterpret_cast<char *>(tval->policingprofile_newname)) == 0) {
    UPLL_LOG_DEBUG(
        " PolicingProfileMoMgr::SwapKeyVal Failed.");
    UPLL_LOG_DEBUG(" Policingprofile names are same");
    return UPLL_RC_ERR_GENERIC;
  }
  key_policingprofile_t *key_policingprofile =
    reinterpret_cast<key_policingprofile_t *>
    (ConfigKeyVal::Malloc(sizeof(key_policingprofile_t)));
  if (UNC_VF_VALID == tval->valid[0]) {
    /* checking the string is empty or not*/
    if (!strlen(reinterpret_cast<char *>(tval->policingprofile_newname))) {
      UPLL_LOG_DEBUG(
          " PolicingProfileMoMgr::SwapKeyVal Failed.");
      UPLL_LOG_DEBUG(" PolicingProfile_newname is NULL");
      free(key_policingprofile);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_policingprofile->policingprofile_name,
                      tval->policingprofile_newname,
                      (kMaxLenPolicingProfileName+1));
  } else if (UNC_VF_VALID_NO_VALUE == tval->valid[0]) {
    no_rename = true;
    uuu::upll_strncpy(key_policingprofile->policingprofile_name,
                      reinterpret_cast<key_policingprofile_t *>
                      (ikey->get_key())->policingprofile_name,
                      (kMaxLenPolicingProfileName+1));
  }
  okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
      IpctSt::kIpcStValPolicingprofile, key_policingprofile,
      NULL);
  if (NULL == okey) {
    UPLL_LOG_DEBUG(
        " PolicingProfileMoMgr::SwapKeyVal Failed.");
    UPLL_LOG_DEBUG(" Memory Allocation for ConfigKeyVal failed");
    free(key_policingprofile);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::TxCopyCandidateToRunning(
    unc_key_type_t keytype, CtrlrCommitStatusList *ctrlr_commit_status,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result = uud::kDalRcGeneralError;
  unc_keytype_operation_t op[] = { UNC_OP_CREATE, UNC_OP_DELETE };
  int nop = sizeof(op) / sizeof(op[0]);
  ConfigKeyVal *policingprofile_key = NULL, *req = NULL, *nreq = NULL;
  DalCursor *cfg1_cursor = NULL;
  uint8_t *ctrlr_id = NULL;
#if 0
  IpcReqRespHeader *req_header = reinterpret_cast<IpcReqRespHeader *>
      (ConfigKeyVal::Malloc(sizeof(IpcReqRespHeader)));
#endif
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  if (ctrlr_commit_status == NULL || (NULL == dmi)) {
    UPLL_LOG_DEBUG(
        " PolicingProfileMoMgr::TxCopyCandidateToRunning Failed.");
    UPLL_LOG_DEBUG(" Incoming CtrlrCommitStatusList is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  for (ccsListItr = ctrlr_commit_status->begin();
      ccsListItr != ctrlr_commit_status->end(); ++ccsListItr) {
    ccStatusPtr = *ccsListItr;
    ctrlr_id = reinterpret_cast<uint8_t *>
        (const_cast<char*>(ccStatusPtr->ctrlr_id.c_str()));
    ctrlr_result[ccStatusPtr->ctrlr_id] = ccStatusPtr->upll_ctrlr_result;
    if (ccStatusPtr->upll_ctrlr_result != UPLL_RC_SUCCESS) {
      for (ConfigKeyVal *ck_err = ccStatusPtr->err_ckv; ck_err != NULL; ck_err =
          ck_err->get_next_cfg_key_val()) {
        if (ck_err->get_key_type() != keytype) continue;
        result_code = GetRenamedUncKey(ck_err, UPLL_DT_CANDIDATE, dmi,
            ctrlr_id);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(
              " PolicingProfileMoMgr::TxCopyCandidateToRunning Failed.");
          UPLL_LOG_DEBUG(" GetRenamedUncKey Function failed - %d ",
              result_code);
          return result_code;
        }
      }
    }
  }
  for (int i = 0; i < nop; i++) {
    // Update the Main table
    if (op[i] != UNC_OP_UPDATE) {
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
                                 req, nreq, &cfg1_cursor, dmi, MAINTBL);
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
          UPLL_LOG_DEBUG("Updating Main table Error %d", result_code);
          return result_code;
        }
      }
      if (cfg1_cursor) {
        dmi->CloseCursor(cfg1_cursor, true);
        cfg1_cursor = NULL;
      }
      if (req)
        delete req;
      req = NULL;
    }
    UPLL_LOG_DEBUG("Updating main table complete with op %d", op[i]);
  }
  for (int i = 0; i < nop; i++) {
    // Update the controller table
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i], req,
                               nreq, &cfg1_cursor, dmi, CTRLRTBL);
    ConfigKeyVal *pp_ctrlr_key = NULL;
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
        break;
      }
      if (op[i] == UNC_OP_CREATE) {
        DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
        result_code = GetChildConfigKey(policingprofile_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey is failed resultcode=%d",
                         result_code);
          return result_code;
        }
        result_code = ReadConfigDB(policingprofile_key, UPLL_DT_CANDIDATE,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Unable to read configuration from CandidateDb");
          return result_code;
        }

        result_code = DupConfigKeyVal(pp_ctrlr_key, req, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigVal function is failed %d", result_code);
          return result_code;
        }
        GET_USER_DATA_CTRLR(pp_ctrlr_key, ctrlr_id);
        string controller(reinterpret_cast<char *>(ctrlr_id));
        result_code = UpdateConfigStatus(policingprofile_key, op[i],
                                         ctrlr_result[controller], NULL,
                                         dmi, pp_ctrlr_key);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" UpdateConfigStatus Function Failed - %d ",
                         result_code);
          delete policingprofile_key;
          return result_code;
        }
      } else if (op[i] == UNC_OP_DELETE) {
        result_code = GetChildConfigKey(pp_ctrlr_key, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey Failed  %d", result_code);
          return result_code;
        }
      }
      result_code = UpdateConfigDB(pp_ctrlr_key, UPLL_DT_RUNNING, op[i],
                                   dmi, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Unable to Update Configuration at DB %d", result_code);
        delete pp_ctrlr_key;
        return result_code;
      }

      // update the consolidated config status in the Main Table
      if (op[i] != UNC_OP_DELETE) {
            result_code = UpdateConfigDB(policingprofile_key, UPLL_DT_RUNNING,
                UNC_OP_UPDATE, dmi, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS)
              return result_code;
          }

      EnqueCfgNotification(op[i], UPLL_DT_RUNNING, pp_ctrlr_key);
      if (pp_ctrlr_key) delete pp_ctrlr_key;
      policingprofile_key = pp_ctrlr_key = NULL;
      result_code = DalToUpllResCode(db_result);
    }
    if (cfg1_cursor) {
      dmi->CloseCursor(cfg1_cursor, true);
      cfg1_cursor = NULL;
    }
    if (req) delete req;
    if (nreq) delete nreq;
    nreq = req = NULL;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::UpdateMainTbl(ConfigKeyVal *key_pp,
      unc_keytype_operation_t op, uint32_t driver_result,
      ConfigKeyVal *nreq, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_pp = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_policingprofile_t *val_pp = NULL;

  switch (op) {
    case UNC_OP_CREATE:
      result_code = DupConfigKeyVal(ck_pp, key_pp, MAINTBL);
      if (!ck_pp || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("DupConfigKeyVal() Returning error %d\n", result_code);
        return result_code;
      }
      val_pp = reinterpret_cast<val_policingprofile_t *>(GetVal(ck_pp));
      if (!val_pp) {
        UPLL_LOG_DEBUG("invalid val \n");
        return UPLL_RC_ERR_GENERIC;
      }
      val_pp->cs_row_status = UNC_CS_NOT_APPLIED;
      break;
    case UNC_OP_DELETE:

      result_code = GetChildConfigKey(ck_pp, key_pp);
      if (!ck_pp || (result_code != UPLL_RC_SUCCESS)) {
        UPLL_LOG_DEBUG("GetChildConfigKey() returning error %d", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
      break;
    default:
          UPLL_LOG_DEBUG("Inalid operation\n");
      return UPLL_RC_ERR_GENERIC;
  }

  result_code = UpdateConfigDB(ck_pp, UPLL_DT_STATE, op, dmi, MAINTBL);
  EnqueCfgNotification(op, UPLL_DT_RUNNING, key_pp);
  delete ck_pp;
  return result_code;
}


upll_rc_t PolicingProfileMoMgr::TxUpdateController(unc_key_type_t keytype,
    uint32_t session_id, uint32_t config_id,
    uuc::UpdateCtrlrPhase phase,
    set<string> *affected_ctrlr_set, DalDmlIntf *dmi,
    ConfigKeyVal **err_ckv) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode dal_result = uud::kDalRcSuccess;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle;
  IpcResponse resp;
  if (uuc::kUpllUcpDelete == phase) {
    UPLL_LOG_TRACE("Delete phase 1");
    return UPLL_RC_SUCCESS;
  }
  if (uuc::kUpllUcpDelete2 == phase) UPLL_LOG_DEBUG("Delete phase 2");
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
          ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
          ((phase == uuc::kUpllUcpDelete2)?UNC_OP_DELETE:UNC_OP_INVALID));
  switch (op) {
    case UNC_OP_CREATE:
    case UNC_OP_DELETE:
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
          op, req, nreq, &dal_cursor_handle, dmi, CTRLRTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG(" PolicingProfileMoMgr::TxUpdateController failed."
            " DiffConfigDB failed to get diff");
        return result_code;
      }
      break;
    case UNC_OP_UPDATE:
      // not supported by keytype
      // return success
      UPLL_LOG_TRACE(" Not supported operation \n");
      return UPLL_RC_SUCCESS;
    default:
      UPLL_LOG_TRACE(" Invalid ooperation \n");
      return UPLL_RC_ERR_GENERIC;
  }
  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;

  while (result_code == UPLL_RC_SUCCESS) {
    // Get Next Record
    dal_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(dal_result);
    if (result_code != UPLL_RC_SUCCESS) {
      break;
    }
    ck_main = NULL;
    if ((op == UNC_OP_CREATE) || (op == UNC_OP_DELETE)) {
      result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DupConfigKeyVal failed during TxUpdate.");
        return result_code;
      }

      GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
      UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom.ctrlr,
                     ctrlr_dom.domain);
      if (NULL == ctrlr_dom.ctrlr || NULL == ctrlr_dom.domain) {
        UPLL_LOG_INFO("Invalid controller/domain");
        result_code = UPLL_RC_ERR_GENERIC;
        if (ck_main) delete ck_main;
        break;
      }
      result_code = TxUpdateProcess(ck_main, &resp, op,
          dmi, &ctrlr_dom);
      if (result_code == UPLL_RC_SUCCESS) {
        affected_ctrlr_set->insert((const char *)ctrlr_dom.ctrlr);
      } else {
        UPLL_LOG_DEBUG("TxUpdateProcess error %d", result_code);
        *err_ckv = resp.ckv_data;
        if (ck_main) delete ck_main;
        break;
      }
    }
    if (ck_main) {
      delete ck_main;
      ck_main = NULL;
    }
  }
  if (nreq)
    delete nreq;
  if (req)
    delete req;
  if (dal_cursor_handle) {
    dmi->CloseCursor(dal_cursor_handle, true);
    dal_cursor_handle = NULL;
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::TxUpdateProcess(ConfigKeyVal *ck_main,
     IpcResponse *ipc_resp, unc_keytype_operation_t op,
    DalDmlIntf *dmi, controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  /* read from main table */
  ConfigKeyVal *dup_ckmain = ck_main;
  if (op == UNC_OP_CREATE)  {
    dup_ckmain = NULL;
    result_code = GetChildConfigKey(dup_ckmain, ck_main);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      if (dup_ckmain) delete dup_ckmain;
      return result_code;
    }
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCs};
    result_code = ReadConfigDB(dup_ckmain, UPLL_DT_CANDIDATE,
                               UNC_OP_READ, dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      string s(dup_ckmain->ToStrAll());
      UPLL_LOG_DEBUG("%s policingprofile read failed from candidatedb (%d)",
                     s.c_str(), result_code);
      return result_code;
    }
  }
  /* Get renamed key if key is renamed */
  result_code =  GetRenamedControllerKey(dup_ckmain, UPLL_DT_CANDIDATE,
      dmi, ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to get the Renamed ControllerKey");
    return result_code;
  }
  result_code = SendIpcReq(ipc_resp->header.clnt_sess_id,
                           ipc_resp->header.config_id, op,
                           UPLL_DT_CANDIDATE,
                           dup_ckmain, ctrlr_dom, ipc_resp);
  if (result_code == UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
    result_code = UPLL_RC_SUCCESS;
    UPLL_LOG_DEBUG("controller disconnected error proceed with commit");
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("IpcSend failed %d\n", result_code);
  }
  if ((op == UNC_OP_CREATE) && dup_ckmain) {
    delete dup_ckmain;
    dup_ckmain = NULL;
  }
  return result_code;
}


upll_rc_t PolicingProfileMoMgr::IsReferenced(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  result_code = UpdateConfigDB(ikey, dt_type, UNC_OP_READ, dmi, CTRLRTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG(" PolicingProfileMoMgr::IsReferenced. Result Code - %d",
        result_code);
    return result_code;
  }
  unc::upll::kt_momgr::VtnPolicingMapMoMgr *pmmgr = reinterpret_cast
      <unc::upll::kt_momgr::VtnPolicingMapMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN_POLICINGMAP)));
  if (NULL == pmmgr) {
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = pmmgr->IsPolicingProfileConfigured(
    reinterpret_cast<const char *>
    (reinterpret_cast<key_policingprofile *>(ikey->get_key())->
    policingprofile_name), dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    UPLL_LOG_DEBUG(" IsPolicingProfileConfigured failed");
    return result_code;
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
  uint32_t dt_type = req->datatype;
  uint32_t operation = req->operation;
  uint32_t option1 = req->option1;
  uint32_t option2 = req->option2;

  UPLL_LOG_DEBUG("dt_type   : (%d)"
               "operation : (%d)"
               "option1   : (%d)"
               "option2   : (%d)",
               dt_type, operation, option1, option2);

  bool result_code = false;
  uint32_t instance_count;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  switch (operation) {
    case UNC_OP_CREATE: {
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &instance_count, &max_attrs, &attrs);
      break;
    }
    case UNC_OP_UPDATE: {
      result_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    }
    default: {
      result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
    }
  }

  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
                   "for opeartion(%d)",
                   ikey->get_key_type(), ctrlr_name, operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
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
  if(req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" Error: option2 is not NONE");
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
  if(req->option1 != UNC_OPT1_NORMAL) {
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
  if (!ikey || !okey || NULL != rename_info ||
      NULL == ctrlr_id || NULL == ikey->get_key())  {  // COVERITY FORWARD NULL
    return UPLL_RC_ERR_GENERIC;
  }
  key_rename_vnode_info_t *key_rename_info =
    reinterpret_cast<key_rename_vnode_info_t *>
    (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info_t)));

  key_policingprofile_t *policingprofile_key = NULL;
  if (!(ikey->get_key())) {
    free(key_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  policingprofile_key = reinterpret_cast<key_policingprofile_t *>
    (ikey->get_key());
  if (!strlen(reinterpret_cast<char *>
        (policingprofile_key->policingprofile_name))) {
    free(key_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_rename_info->old_policingprofile_name,
                    policingprofile_key->policingprofile_name,
                    strlen(reinterpret_cast<const char *>
                    (policingprofile_key->policingprofile_name)));

  if (!(okey->get_key())) {
    free(key_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  policingprofile_key = reinterpret_cast<key_policingprofile_t *>
    (okey->get_key());

  if (!strlen(reinterpret_cast<char *>
        (policingprofile_key->policingprofile_name))) {
    free(key_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_rename_info->new_policingprofile_name,
                    policingprofile_key->policingprofile_name,
                    strlen(reinterpret_cast<const char *>
                    (policingprofile_key->policingprofile_name)));

  rename_info = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
      IpctSt::kIpcInvalidStNum, key_rename_info, NULL);
  SET_USER_DATA_CTRLR(rename_info, ctrlr_id);
  if (!renamed) {
    val_rename_policingprofile_t *val_rename =
      reinterpret_cast<val_rename_policingprofile_t *>
      (ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile_t)));
    uuu::upll_strncpy(val_rename->policingprofile_newname,
                      key_rename_info->old_policingprofile_name,
                      (kMaxLenPolicingProfileName+1));
    ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValRenamePolicingprofile,
        val_rename);
    okey->AppendCfgVal(cfg_val);
    SET_USER_DATA_CTRLR(okey, ctrlr_id);
    DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutCtrlr};
    result_code = UpdateConfigDB(okey, UPLL_DT_IMPORT, UNC_OP_CREATE, dmi,
        &dbop, RENAMETBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" GetRenameInfo Failed. UpdateConfigDb Failed"
        " Result code - %d", result_code);
      free(key_rename_info);
      free(val_rename);
    }
  }
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
    ConfigKeyVal *ikey) {

  UPLL_FUNC_TRACE;
  if (NULL == ikey ||  NULL == (ikey->get_key()) || NULL != okey) {
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (UNC_KT_POLICING_PROFILE == ikey->get_key_type()) {
    key_rename_vnode_info_t *key_rename =
      reinterpret_cast<key_rename_vnode_info_t *>(ikey->get_key());
    key_policingprofile_t *key_policingprofile =
      reinterpret_cast<key_policingprofile_t *>
      (ConfigKeyVal::Malloc(sizeof(key_policingprofile_t)));
    if (!strlen(reinterpret_cast<char *>
          (key_rename->old_policingprofile_name))) {
      UPLL_LOG_DEBUG(" Invalid Policing Profile Name");
      free(key_policingprofile);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_policingprofile->policingprofile_name,
                      key_rename->old_policingprofile_name,
                      kMaxLenPolicingProfileName+1);

    okey = new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
        IpctSt::kIpcStKeyPolicingprofile, key_policingprofile, NULL);
    if (!okey) {
      free(key_policingprofile);
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

bool PolicingProfileMoMgr::IsValidKey(void *ikey, uint64_t index) {
  key_policingprofile_t *key =
    reinterpret_cast<key_policingprofile_t *>(ikey);
  UPLL_LOG_TRACE("Entering IsValidKey");
  bool ret_val = UPLL_RC_SUCCESS;

  ret_val = ValidateKey(reinterpret_cast<char *>
                    (key->policingprofile_name),
                    kMinLenPolicingProfileName, kMaxLenPolicingProfileName);

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
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
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
  val_policingprofile_t *val;
  ConfigKeyVal *tmp_ckv = ckv;
  for ( ; tmp_ckv != NULL ; tmp_ckv = ckv->get_next_cfg_key_val()) {
      val = reinterpret_cast<val_policingprofile_t *>(GetVal(tmp_ckv));
      list_cs_row.push_back((unc_keytype_configstatus_t)val->cs_row_status);
  }
  if (ckv) delete ckv;
  val_policingprofile_t *val_temp =
      reinterpret_cast<val_policingprofile_t *>(GetVal(ikey));
  val_temp->cs_row_status = GetConsolidatedCsStatus(list_cs_row);
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_UPDATE, dmi,
                               MAINTBL);
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
    upll_keytype_datatype_t dt_type) {
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

  if (UNC_OP_CREATE == oper) {
    result_code = CtrlrTblCreate(pp_ckv, dmi, dt_type);
    UPLL_LOG_DEBUG("PolicingProfileCtrlrTblOper Failed.CtrlrTblCreate failed"
                   "Result code - %d", result_code);
  } else if (UNC_OP_DELETE == oper) {
    result_code = CtrlrTblDelete(pp_ckv, dmi, dt_type);
    UPLL_LOG_DEBUG("PolicingProfileCtrlrTblOper Failed.CtrlrTblDelete failed"
                   "Result code - %d", result_code);
  } else {
    result_code = UPLL_RC_ERR_NO_SUCH_OPERATION;
    UPLL_LOG_DEBUG("PolicingProfileCtrlrTblOper Failed. "
                   "Result code - %d", result_code);
  }
  delete pp_ckv;
  PolicingProfileEntryMoMgr *ppe_mgr =
    reinterpret_cast<PolicingProfileEntryMoMgr *>
    (const_cast<MoManager *>(GetMoManager(
                                          UNC_KT_POLICING_PROFILE_ENTRY)));
  result_code = ppe_mgr->PolicingProfileEntryCtrlrTblOper(
        policingprofile_name, ctrlr_id, dmi, oper, dt_type);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  return result_code;
}

upll_rc_t PolicingProfileMoMgr::CtrlrTblCreate(ConfigKeyVal *pp_ckv,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr, kOpInOutNone};
  /*Check whether present in CTRLRTBL*/
  val_policingprofile_ctrl_t *val_ctrlr;
  result_code = ReadConfigDB(pp_ckv, dt_type,
      UNC_OP_READ, dbop, dmi, CTRLRTBL);
  UPLL_LOG_DEBUG(" Read result in CtrlrTblCreate %d ", result_code);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG(" No record found. Create new record");
    val_ctrlr = reinterpret_cast<val_policingprofile_ctrl_t *>
      (GetVal(pp_ckv));
    val_ctrlr->ref_count = 1;
    val_ctrlr->valid[0] = UNC_VF_VALID;
    result_code = UpdateConfigDB(pp_ckv, dt_type,
        UNC_OP_CREATE, dmi, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" CtrlrTblCreate Failed. Create record failed."
        " Result_code - %d", result_code);
      return result_code;
    }
    val_policingprofile_ctrl_t *tmp = reinterpret_cast
        <val_policingprofile_ctrl_t *>(GetVal(pp_ckv));
    UPLL_LOG_DEBUG(" refcount from ckv - %d %d", tmp->ref_count,
        tmp->valid[0]);
    UPLL_LOG_DEBUG(" refcount - %d ", val_ctrlr->ref_count);
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" CtrlrTblCreate Failed. ReadConfigDb Failed."
      " Result_code - %d", result_code);
    return result_code;
  } else {
    UPLL_LOG_DEBUG(" Record exixts . Update");
    val_ctrlr = reinterpret_cast<val_policingprofile_ctrl_t *>
      (GetVal(pp_ckv));
    /*Check max ref_count*/
    val_ctrlr->ref_count = val_ctrlr->ref_count+1;
    val_ctrlr->valid[0] = UNC_VF_VALID;
    UPLL_LOG_DEBUG(" refcount - %d ", val_ctrlr->ref_count);
    result_code = UpdateConfigDB(pp_ckv, dt_type, UNC_OP_UPDATE, dmi,
        &dbop, CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" CtrlrTblCreate Failed. Update record failed."
        " Result_code - %d", result_code);
      return result_code;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t PolicingProfileMoMgr::CtrlrTblDelete(ConfigKeyVal *pp_ckv,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
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
    val_ctrlr = reinterpret_cast<val_policingprofile_ctrl_t *>(GetVal(pp_ckv));
    UPLL_LOG_DEBUG(" Read is success in CtrlrTblDelete");
    if (1 < val_ctrlr->ref_count) {
     val_ctrlr->ref_count = val_ctrlr->ref_count-1;
     UPLL_LOG_DEBUG(" Refcount is  - %d %d", val_ctrlr->ref_count,
                   val_ctrlr->valid[0]);
      val_ctrlr->valid[0] = UNC_VF_VALID;
      result_code = UpdateConfigDB(pp_ckv, dt_type, UNC_OP_UPDATE, dmi,
          &dbop1, CTRLRTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG(" CtrlrTblDelete Failed. Update record failed."
                       " Result_code - %d", result_code);
        return result_code;
      }
    } else if (1 >= val_ctrlr->ref_count) {
    // If ref_count = 0 delete the entry in ctrlrtbl
      UPLL_LOG_DEBUG(" Refcount is less than 1 - %d %d", val_ctrlr->ref_count,
                       val_ctrlr->valid[0]);
      result_code = UpdateConfigDB(pp_ckv, dt_type, UNC_OP_DELETE, dmi,
          &dbop1, CTRLRTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        return result_code;
      }
    }
  } else {
    return result_code;
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
  unc_keytype_configstatus_t status = UNC_CS_UNKNOWN;
  unc_keytype_configstatus_t  cs_status = UNC_CS_UNKNOWN;
  cs_status = (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;

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
  if (op == UNC_OP_CREATE) {
     switch (val_policingprofile->cs_row_status) {
      case UNC_CS_UNKNOWN:
        status = cs_status;
        break;
      case UNC_CS_PARTAILLY_APPLIED:
        if (ctrlr_val_policingprofile->cs_row_status == UNC_CS_NOT_APPLIED) {
        }
        break;
      case UNC_CS_APPLIED:
      case UNC_CS_NOT_APPLIED:
      case UNC_CS_INVALID:
      default:
        status =
            (cs_status == UNC_CS_APPLIED) ? UNC_CS_PARTAILLY_APPLIED : status;
        break;
    }
    val_policingprofile->cs_row_status = status;
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
}  //  namespace kt_momgr
}  //  namespace upll
}  //  namespace unc
