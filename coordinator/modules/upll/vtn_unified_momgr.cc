/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vtn_unified_momgr.hh"
#include "upll_validation.hh"
#include "uncxx/upll_log.hh"
#include "unc/uppl_common.h"
#include "unified_nw_momgr.hh"
#include "vbr_portmap_momgr.hh"
using unc::upll::ipc_util::IpcUtil;

#define NUM_KEY_MAIN_TBL_ 3
namespace unc {
namespace upll {
namespace kt_momgr {


// vtn_unified(Main Table)

BindInfo VtnUnifiedMoMgr::vtn_unified_bind_info[] = {
  { uudst::vtn_unified::kDbiVtnName, CFG_KEY, offsetof(
      key_vtn_unified, vtn_key.vtn_name), uud::kDalChar, 32 },
  { uudst::vtn_unified::kDbiUnifiedNwName, CFG_KEY, offsetof(
      key_vtn_unified, unified_nw_id), uud::kDalChar, 32 },
  { uudst::vtn_unified::kDbiUnwSpineDomainName, CFG_VAL, offsetof(
      val_vtn_unified, spine_id), uud::kDalChar, 32 },
  { uudst::vtn_unified::kDbiValidUnwSpineDomainName, CFG_META_VAL, offsetof(
      val_vtn_unified, valid[UPLL_IDX_SPINE_ID_VUNW]), uud::kDalUint8, 1 },
  { uudst::vtn_unified::kDbiCsRowStatus, CS_VAL, offsetof(
      val_vtn_unified, cs_row_status), uud::kDalUint8, 1 },
  { uudst::vtn_unified::kDbiCsUnwSpineDomainName, CS_VAL, offsetof(
      val_vtn_unified, cs_attr[0]), uud::kDalUint8, 1 },
  { uudst::vtn_unified::kDbiFlags, CK_VAL, offsetof(
    key_user_data_t, flags), uud::kDalUint8, 1 }
};

BindInfo VtnUnifiedMoMgr::vtn_unified_maintbl_rename_bind_info[] = {
  { uudst::vtn_unified::kDbiVtnName, CFG_MATCH_KEY, offsetof(
    key_vtn_unified_t, vtn_key.vtn_name), uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vtn_unified::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vtn_unified::kDbiFlags, CK_VAL, offsetof(
    key_user_data_t, flags), uud::kDalUint8, 1 }
};

bool VtnUnifiedMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                           BindInfo *&binfo, int &nattr,
                                           MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = vtn_unified_maintbl_rename_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalide Key");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

VtnUnifiedMoMgr::VtnUnifiedMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVtnUnifiedTbl, UNC_KT_VTN_UNIFIED,
      vtn_unified_bind_info, IpctSt::kIpcStKeyVtnUnified,
      IpctSt::kIpcStValVtnUnified,
      uudst::vtn_unified::kDbiVtnUnifiedNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;
  nchild = 0;
  child = NULL;
}

upll_rc_t VtnUnifiedMoMgr::CheckIfUnifiedNwIdExists(ConfigKeyVal *ckUnw,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req,
                                TcConfigMode configMode) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;

// check the existence of Unw in DB

  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  MoMgrImpl *unwMgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                        (GetMoManager(UNC_KT_UNIFIED_NETWORK)));


  ret_val =unwMgr->UpdateConfigDB(ckUnw, req->datatype,
                     UNC_OP_READ, dmi, &dbop, MAINTBL);
  if (ret_val == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("The unified_nw given is not existing in DB");
    return ret_val;
  } else if ((ret_val != UPLL_RC_SUCCESS) && (ret_val !=
                        UPLL_RC_ERR_INSTANCE_EXISTS) ) {
    UPLL_LOG_DEBUG("Error Accessing the database %d" , ret_val);
    return ret_val;
  }

  if (configMode == TC_CONFIG_VTN) {
    ret_val =unwMgr->UpdateConfigDB(ckUnw, UPLL_DT_RUNNING,
                     UNC_OP_READ, dmi, &dbop, MAINTBL);
    if (ret_val == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG(
          "vtn_mode : The unified_nw given is not existing in Running DB");
      return ret_val;
    } else if ((ret_val != UPLL_RC_SUCCESS) && (ret_val !=
                        UPLL_RC_ERR_INSTANCE_EXISTS) ) {
      UPLL_LOG_DEBUG("Error Accessing the database %d" , ret_val);
      return ret_val;
    }
  }

  return UPLL_RC_SUCCESS;
}
upll_rc_t VtnUnifiedMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;

  key_vtn_unified_t *vtn_unified_key =
    reinterpret_cast<key_vtn_unified_t *>(ikey->get_key());

  if (!vtn_unified_key) {
    UPLL_LOG_DEBUG("Empty Key Structure");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name;
  ret_val = GetConfigModeInfo(req, config_mode, vtn_name);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return ret_val;
  }

// Framing ConfigKey for Unified network

  ConfigKeyVal *ckUnw = NULL;
  key_unified_nw_t *unw_key = reinterpret_cast<key_unified_nw_t *>
        (ConfigKeyVal::Malloc(sizeof(key_unified_nw_t)));
  uuu::upll_strncpy(unw_key->unified_nw_id,
       vtn_unified_key->unified_nw_id, (kMaxLenUnifiedNwName + 1));
  ckUnw = new ConfigKeyVal(UNC_KT_UNIFIED_NETWORK,
           IpctSt::kIpcStKeyUnifiedNw, unw_key, NULL);
  if (!ckUnw) {
    UPLL_LOG_DEBUG("ConfigKeyVal for unified_nw is NULL");
    return ret_val;
  }

  // check whether the given nw id is present in unified_nw_tbl

  ret_val = CheckIfUnifiedNwIdExists(ckUnw, dmi, req, config_mode);
  DELETE_IF_NOT_NULL(ckUnw);
  if (ret_val != UPLL_RC_SUCCESS) {
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  ConfigVal *cfg_val = ikey->get_cfg_val();
  if (!cfg_val) {
    return UPLL_RC_SUCCESS;
  }
  val_vtn_unified_t *vtn_unified_val =
                reinterpret_cast<val_vtn_unified_t*> (cfg_val->get_val());
  if (vtn_unified_val->valid[UPLL_IDX_SPINE_ID_VUNW] != UNC_VF_VALID) {
    return UPLL_RC_SUCCESS;
  }

  // Framing spine domain config key from vtn_unified keyval

  key_unw_spine_domain_t *spine_key =
               reinterpret_cast<key_unw_spine_domain_t *>
               (ConfigKeyVal::Malloc(sizeof(key_unw_spine_domain_t)));
  uuu::upll_strncpy((spine_key->unw_key).unified_nw_id,
                   vtn_unified_key->unified_nw_id, (kMaxLenUnifiedNwName + 1));
  uuu::upll_strncpy(spine_key->unw_spine_id, vtn_unified_val->spine_id,
                                               (kMaxLenSpineName + 1));
  ConfigKeyVal* ckNwSpineDomain = new ConfigKeyVal(UNC_KT_UNW_SPINE_DOMAIN,
           IpctSt::kIpcStKeyUnwSpineDomain, spine_key, NULL);

  // check whether the given spine id is present in unw_spine_domain_tbl

  ret_val = CheckIfSpineDomainExists(&ckNwSpineDomain, dmi, req, config_mode);
  if (ret_val != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(ckNwSpineDomain);
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }

  // Framing vtunnel config val by filling the vtn name

  key_convert_vtunnel_t *co_vtunnel_key = ConfigKeyVal::Malloc
                                   <key_convert_vtunnel_t> ();
  uuu::upll_strncpy((co_vtunnel_key->vtn_key).vtn_name,
         (reinterpret_cast<key_vtn_unified_t *>(ikey->get_key()))
         ->vtn_key.vtn_name, (kMaxLenVtnName + 1));
  ConfigKeyVal *ckVtunnel = new ConfigKeyVal(UNC_KT_VTUNNEL,
           IpctSt::kIpcStKeyConvertVtunnel, co_vtunnel_key, NULL);

/**while update and create operation, if there is a change in 
controller id/domain, vbrportmap iface to be called**/

  if ((req->operation == UNC_OP_UPDATE ) ||
      (req->operation == UNC_OP_CREATE)) {
    ret_val = HandleChangeofSpineDomainWithVtunnel(ckVtunnel,
                                dmi, req, ckNwSpineDomain);
  }
  DELETE_IF_NOT_NULL(ckNwSpineDomain);
  DELETE_IF_NOT_NULL(ckVtunnel);
  return ret_val;
}

bool VtnUnifiedMoMgr::IsValidKey(void *key, uint64_t index,
                                 MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  key_vtn_unified_t *vtn_unified_key =
           reinterpret_cast<key_vtn_unified_t*> (key);
  upll_rc_t ret_val;
  switch (index) {
    case uudst::vtn_unified::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vtn_unified_key->vtn_key.vtn_name),
                            kMinLenVtnName,
                            kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
    break;
    case uudst::vtn_unified::kDbiUnifiedNwName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vtn_unified_key->unified_nw_id),
                            kMinLenUnifiedNwName,
                            kMaxLenUnifiedNwName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Unified Network Name is not valid(%d)", ret_val);
        return false;
      }
    break;
    default:
      UPLL_LOG_DEBUG("Index is wrong");
      return false;
  }
  return true;
}


upll_rc_t VtnUnifiedMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                               ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VTN_UNIFIED) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vtn_unified_t *tkey = reinterpret_cast<key_vtn_unified_t*>
                                              (ikey->get_key());
  if (!tkey) {
    UPLL_LOG_DEBUG(" Input vtn unified key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vtn_t)));

  uuu::upll_strncpy(vtn_key->vtn_name, tkey->vtn_key.vtn_name,
                                       (kMaxLenVtnName + 1));
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnUnifiedMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn_unified_t *vtn_unified_key;
  // case 1 : if parent key is NULL, new configkeyval is created and returned.
  if (parent_key == NULL) {
    vtn_unified_key = reinterpret_cast<key_vtn_unified_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_unified_t)));
    okey = new ConfigKeyVal(UNC_KT_VTN_UNIFIED, IpctSt::kIpcStKeyVtnUnified,
        vtn_unified_key, NULL);
    UPLL_LOG_DEBUG("GetChildConfigKey parent key with NULL");
    return UPLL_RC_SUCCESS;
  }

  // case 2 : if parent key is not NULL, based on the type of parent key,
  // the appropriate values are copied from parent key to okey
  void * pkey = parent_key->get_key();
  if (!pkey) {
    UPLL_LOG_DEBUG("GetChildConfigKey pkey NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VTN_UNIFIED)
      return UPLL_RC_ERR_GENERIC;
  }

  if ((okey) && (okey->get_key())) {
    vtn_unified_key = reinterpret_cast<key_vtn_unified_t *>(okey->get_key());
  } else {
    vtn_unified_key = reinterpret_cast<key_vtn_unified_t *>
                      (ConfigKeyVal::Malloc(sizeof(key_vtn_unified_t)));
  }
  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy((vtn_unified_key->vtn_key).vtn_name,
         (reinterpret_cast<key_vtn_t *>(pkey))->vtn_name,
         (kMaxLenVtnName + 1));
      break;
    case UNC_KT_VTN_UNIFIED:
      uuu::upll_strncpy((vtn_unified_key->vtn_key).vtn_name,
         (reinterpret_cast<key_vtn_unified_t *>(pkey))->vtn_key.vtn_name,
         (kMaxLenVtnName + 1));
       uuu::upll_strncpy(vtn_unified_key->unified_nw_id, reinterpret_cast
       <key_vtn_unified_t *>(pkey)->unified_nw_id, (kMaxLenUnifiedNwName+1));
      break;
    default:
      if (vtn_unified_key) free(vtn_unified_key);
      return UPLL_RC_ERR_GENERIC;
  }

  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey and its key part are not null, "
               "vtn_unified_key is set to its key part");
    okey->SetKey(IpctSt::kIpcStKeyVtnUnified, vtn_unified_key);
  }

  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VTN_UNIFIED,
           IpctSt::kIpcStKeyVtnUnified, vtn_unified_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("GetChildConfigKey Allocation Successful");
  return result_code;
}

upll_rc_t VtnUnifiedMoMgr::AllocVal(ConfigVal *&ckVal,
                                    upll_keytype_datatype_t dtType,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;
  if (ckVal != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_vtn_unified_t)));
      ckVal = new ConfigVal(IpctSt::kIpcStValVtnUnified, val);
      break;
    default:
      val = NULL;
      return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("AllocVal Success");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnUnifiedMoMgr::GetValid(void *val, uint64_t indx, uint8_t *&valid,
                   upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (val == NULL) {
    UPLL_LOG_DEBUG("GetValid val NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (tbl == MAINTBL) {
    UPLL_LOG_DEBUG("GetValid MAINTBL");
    switch (indx) {
      case uudst::vtn_unified::kDbiUnwSpineDomainName:
           valid = &((reinterpret_cast<val_vtn_unified_t *>(val)))
                   ->valid[UPLL_IDX_SPINE_ID_VUNW];
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  } else {
    UPLL_LOG_DEBUG("KT_VTN_UNIFIED does not have other than MAINTBL");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnUnifiedMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                           ConfigKeyVal *&req,
                              MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  val_vtn_unified_t *vtn_unified_val = NULL;
  if (req == NULL) {
    UPLL_LOG_DEBUG("In sufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_DEBUG("oKey already Contains Data");
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->get_key_type() != UNC_KT_VTN_UNIFIED) {
    UPLL_LOG_DEBUG(" Invalid KeyType.");
    return UPLL_RC_ERR_GENERIC;
  }

  if (tbl != MAINTBL) {
    UPLL_LOG_DEBUG(" Invalid Table.");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    val_vtn_unified_t *ival = reinterpret_cast<val_vtn_unified_t*>(GetVal(req));
    if (NULL != ival) {
      vtn_unified_val = reinterpret_cast<val_vtn_unified_t *>
                        (ConfigKeyVal::Malloc(sizeof(val_vtn_unified_t)));
      memcpy(vtn_unified_val, ival, sizeof(val_vtn_unified_t));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVtnUnified, vtn_unified_val);
    }
  }
  if (tmp1) {
    tmp1->set_user_data(tmp->get_user_data());
  }
  void *tkey = (req)->get_key();
  if (!tkey) {
    UPLL_LOG_DEBUG("Key Structure is NULL in the input ConfigKeyVal");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  key_vtn_unified_t *ikey = reinterpret_cast<key_vtn_unified_t *>(tkey);

  key_vtn_unified_t *vtn_unifiedkey =   reinterpret_cast<key_vtn_unified_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vtn_unified_t)));
  memcpy(vtn_unifiedkey, ikey, sizeof(key_vtn_unified_t));
  okey = new ConfigKeyVal(UNC_KT_VTN_UNIFIED, IpctSt::kIpcStKeyVtnUnified,
                          vtn_unifiedkey, tmp1);
  SET_USER_DATA(okey, req);
  UPLL_LOG_DEBUG(" Creation of Duplicate ConfigKeyVal is Successful ");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnUnifiedMoMgr::ValidateVtnUnifiedKey(ConfigKeyVal *kval,
                                unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  key_vtn_unified_t *vtn_unified_key =
                reinterpret_cast<key_vtn_unified_t*> (kval->get_key());

  if (!vtn_unified_key) {
    UPLL_LOG_DEBUG("Empty Key Structure");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (kval->get_st_num() != IpctSt::kIpcStKeyVtnUnified) {
    UPLL_LOG_DEBUG("Invalid key structure received.struct num - %d",
                   kval->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ret_val = ValidateKey(reinterpret_cast<char *>
            (vtn_unified_key->vtn_key.vtn_name),
            kMinLenVtnName,
            kMaxLenVtnName);

  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
    return ret_val;
  }

  if ((operation != UNC_OP_READ_SIBLING_COUNT) &&
      (operation != UNC_OP_READ_SIBLING_BEGIN)) {
    ret_val = ValidateKey(reinterpret_cast<char *>
            (vtn_unified_key->unified_nw_id),
            kMinLenUnifiedNwName,
            kMaxLenUnifiedNwName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Unified Network Name is not valid(%d)", ret_val);
      return ret_val;
    }
  } else {
      UPLL_LOG_TRACE("Operation is %d", operation);
      StringReset(vtn_unified_key->unified_nw_id);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnUnifiedMoMgr::CheckIfSpineDomainExists(
                     ConfigKeyVal **ckNwSpineDomain,
                     DalDmlIntf *dmi,
                     IpcReqRespHeader *req,
                     TcConfigMode configMode ) {
  UPLL_FUNC_TRACE;

  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;

  /**Retrive the record from spine domain table
  which matches unified nw id and spine domain id **/
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  MoMgrImpl *spineMgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                        (GetMoManager(UNC_KT_UNW_SPINE_DOMAIN)));
  ret_val =spineMgr->ReadConfigDB((*ckNwSpineDomain), req->datatype,
                     UNC_OP_READ, dbop, dmi, MAINTBL);
  if (ret_val == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("The spine domain entry is not present in Database");
    return ret_val;
  }
  if ((ret_val != UPLL_RC_SUCCESS) && (ret_val !=
                  UPLL_RC_ERR_INSTANCE_EXISTS)) {
    UPLL_LOG_DEBUG("Error reading DB, Returning error %d", ret_val);
    return ret_val;
  }

  // in vtn mode the spine id existence is checked in running DB also.
  if (configMode == TC_CONFIG_VTN) {
    DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
    ret_val = spineMgr->UpdateConfigDB((*ckNwSpineDomain), UPLL_DT_RUNNING,
                     UNC_OP_READ, dmi, &dbop, MAINTBL);
    if (ret_val == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG(
          "vtn_mode : The spine_id given is not existing in Running DB");
      return ret_val;
    } else if ((ret_val != UPLL_RC_SUCCESS) && (ret_val !=
                            UPLL_RC_ERR_INSTANCE_EXISTS)) {
      UPLL_LOG_DEBUG("Error Accessing the database %d" , ret_val);
      return ret_val;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnUnifiedMoMgr::HandleChangeofSpineDomainWithVtunnel(
                                ConfigKeyVal *ckVtunnel,
                                DalDmlIntf *dmi, IpcReqRespHeader *req,
                                ConfigKeyVal *ckNwSpineDomain) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  // fetch the values of spine domain record
  ConfigVal *cfgVal = ckNwSpineDomain->get_cfg_val();
  if (!cfgVal) {
    UPLL_LOG_DEBUG("ConfVal of spine domain is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  val_unw_spine_domain_t *valSpineDom = NULL;

  if (cfgVal->get_st_num() == IpctSt::kIpctStValUnwSpineDomain_Ext) {
    val_unw_spdom_ext_t* spineExtVal =
        reinterpret_cast<val_unw_spdom_ext_t *>((cfgVal->get_val()));

    if (!spineExtVal) {
     UPLL_LOG_DEBUG("Value of spine domain is NULL");
     return UPLL_RC_ERR_GENERIC;
    }
    valSpineDom = &(spineExtVal->val_unw_spine_dom);
  } else if (cfgVal->get_st_num() == IpctSt::kIpcStValUnwSpineDomain) {
      valSpineDom = reinterpret_cast<val_unw_spine_domain_t *>(
                                          (cfgVal->get_val()));
  } else {
      UPLL_LOG_DEBUG("inappropriate ST num for Spine domain config key val");
      return UPLL_RC_ERR_GENERIC;
  }

  // ##Retrieve the vtunnel entries from vtunnel table
  // which matches vtn name(present in ikey) from DB
  MoMgrImpl *VtunMgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                           (GetMoManager(UNC_KT_VTUNNEL)));
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|
                                               kOpInOutDomain };

  ret_val = VtunMgr->ReadConfigDB(ckVtunnel, req->datatype,
                    UNC_OP_READ, dbop, dmi, CONVERTTBL);
  if (ret_val == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("No vtunnel entries, so no need of comparision");
    return UPLL_RC_SUCCESS;
  }
  if ((ret_val != UPLL_RC_SUCCESS) && (ret_val !=
                UPLL_RC_ERR_INSTANCE_EXISTS) ) {
    UPLL_LOG_DEBUG("Returning error %d", ret_val);
    return ret_val;
  }

  // fetch the values of vtunnel record and compare the
  // controller id and domain with the same of spine domain

  MoMgrImpl *VbrportMapMgr = reinterpret_cast<MoMgrImpl *>(
     const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_PORTMAP)));
  if (ckVtunnel) {
    controller_domain ctrlr_dom = {NULL, NULL};
    GET_USER_DATA_CTRLR_DOMAIN(ckVtunnel, ctrlr_dom);
    if (ctrlr_dom.ctrlr == NULL || ctrlr_dom.domain == NULL) {
      UPLL_LOG_DEBUG("The controller domain values are null.");
      return UPLL_RC_ERR_GENERIC;
    }
    if ((uuu::upll_strncmp(ctrlr_dom.ctrlr,
                      valSpineDom->spine_controller_id,
                      kMaxLenCtrlrId+1))||
                      (uuu::upll_strncmp(ctrlr_dom.domain,
                      valSpineDom->spine_domain_id,
                      kMaxLenDomainId+1))) {
      if ((ret_val = VbrportMapMgr->HandleSpineDomainIdChange(dmi, req,
                             ckNwSpineDomain, ckVtunnel)) != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("HandleSpineDomainIdChange : Returning error %d",
                                                               ret_val);
        return ret_val;
      }
    }
  }
  return  UPLL_RC_SUCCESS;
}

upll_rc_t VtnUnifiedMoMgr::ValidateVtnUnifiedVal(ConfigKeyVal *ikey,
                                          unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  ConfigVal *cfg_val = ikey->get_cfg_val();
  if (!cfg_val) {
    if (operation == UNC_OP_UPDATE) {
      UPLL_LOG_ERROR("Mandatory val structure is null for update");
      return UPLL_RC_ERR_BAD_REQUEST;
    } else {
      return UPLL_RC_SUCCESS;
    }
  }
  val_vtn_unified_t *vtn_unified_val =
                reinterpret_cast<val_vtn_unified_t*> (cfg_val->get_val());
  if (!vtn_unified_val) {
    if (operation == UNC_OP_UPDATE) {
      UPLL_LOG_DEBUG("Empty Val Structure");
      return UPLL_RC_ERR_BAD_REQUEST;
    } else {
      return UPLL_RC_SUCCESS;
    }
  }
  if (cfg_val->get_st_num() != IpctSt::kIpcStValVtnUnified) {
    UPLL_LOG_DEBUG("Invalid val structure received.struct num - %d",
                   cfg_val->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (vtn_unified_val->valid[UPLL_IDX_SPINE_ID_VUNW] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>
            (vtn_unified_val->spine_id),
            kMinLenUnwSpineID,
            kMaxLenUnwSpineID);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Spine Id Name is not valid(%d)", ret_val);
      return ret_val;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnUnifiedMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                              ConfigKeyVal *kval) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == kval)) {
    UPLL_LOG_DEBUG("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_VTN_UNIFIED != kval->get_key_type()) {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", kval->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (req->option1 != UNC_OPT1_NORMAL) {
    UPLL_LOG_DEBUG(" invalid option1(%d)", req->option1);
    return UPLL_RC_ERR_INVALID_OPTION1;
  }

  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" invalid option2(%d)", req->option2);
    return UPLL_RC_ERR_INVALID_OPTION2;
  }

  if (!((req->datatype == UPLL_DT_CANDIDATE) ||
        (req->datatype == UPLL_DT_RUNNING) ||
        (req->datatype == UPLL_DT_STARTUP) ||
        (req->datatype == UPLL_DT_STATE))) {
    UPLL_LOG_DEBUG(" Unsupported dataType(%d)", req->datatype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  switch (req->operation) {
    case UNC_OP_CREATE:
    case UNC_OP_UPDATE:
    case UNC_OP_READ:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_COUNT:
    case UNC_OP_READ_SIBLING_BEGIN:
      ret_val = ValidateVtnUnifiedKey(kval, req->operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        return ret_val;
      }
      return ValidateVtnUnifiedVal(kval, req->operation);
    case UNC_OP_DELETE:
      return ValidateVtnUnifiedKey(kval, req->operation);
    default:
      UPLL_LOG_ERROR("Invalid operation(%d)", req->operation);
      return UPLL_RC_ERR_BAD_REQUEST;
  }
  return UPLL_RC_SUCCESS;
}

// TODO(AUTHOR) : to check whether the below function is needed
upll_rc_t VtnUnifiedMoMgr::GetOperation(uuc::UpdateCtrlrPhase phase,
                                            unc_keytype_operation_t &op) {
  if (uuc::kUpllUcpDelete2 == phase) {
    UPLL_LOG_DEBUG("Delete phase 1");
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  } else if (uuc::kUpllUcpUpdate == phase) {
    op = UNC_OP_UPDATE;
  } else if (uuc::kUpllUcpCreate == phase) {
    op = UNC_OP_CREATE;
  } else if (uuc::kUpllUcpDelete == phase) {
    op = UNC_OP_DELETE;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

bool VtnUnifiedMoMgr::CompareValidValue(void *&val1, void *val2,
                                 bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;

  val_vtn_unified_t *val1_vtn_unified = reinterpret_cast<
      val_vtn_unified_t*>(val1);
  val_vtn_unified_t *val2_vtn_unified = reinterpret_cast<
      val_vtn_unified_t*>(val2);

  if (UNC_VF_INVALID == val1_vtn_unified->valid[UPLL_IDX_SPINE_ID_VUNW] &&
             UNC_VF_VALID == val2_vtn_unified->valid[UPLL_IDX_SPINE_ID_VUNW]) {
    val1_vtn_unified->valid[UPLL_IDX_SPINE_ID_VUNW] = UNC_VF_VALID_NO_VALUE;
  }
  if (UNC_VF_VALID == val1_vtn_unified->valid[UPLL_IDX_SPINE_ID_VUNW]
        && UNC_VF_VALID == val2_vtn_unified->valid[UPLL_IDX_SPINE_ID_VUNW]) {
    if (!strcmp(reinterpret_cast<char*>
                (val1_vtn_unified->spine_id),
                reinterpret_cast<char*>
                (val2_vtn_unified->spine_id)))
      val1_vtn_unified->valid[UPLL_IDX_SPINE_ID_VUNW] = UNC_VF_INVALID;
  }
  if ((UNC_VF_VALID == val1_vtn_unified->valid[UPLL_IDX_SPINE_ID_VUNW])
       ||(UNC_VF_VALID_NO_VALUE == val1_vtn_unified->valid[
          UPLL_IDX_SPINE_ID_VUNW])) {
      invalid_attr = false;
    }
  return invalid_attr;
}

upll_rc_t VtnUnifiedMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                               unc_keytype_operation_t op,
                                               uint32_t driver_result,
                                               ConfigKeyVal *upd_key,
                                               DalDmlIntf *dmi,
                                               ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
     UPLL_LOG_DEBUG("input key is NULL");
     return UPLL_RC_ERR_GENERIC;
  }
  val_vtn_unified *vtn_unified_val =
      reinterpret_cast<val_vtn_unified *>(GetVal(ikey));
  if (vtn_unified_val == NULL) {
    UPLL_LOG_DEBUG("input val is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  vtn_unified_val->cs_row_status = UNC_CS_APPLIED;
  vtn_unified_val->cs_attr[0] = UNC_CS_APPLIED;

  if (op == UNC_OP_UPDATE) {
    void *ival = reinterpret_cast<void *>(vtn_unified_val);
    CompareValidValue(ival, GetVal(upd_key), true);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
VtnUnifiedMoMgr::CopyToConfigKey(ConfigKeyVal * &okey,
                          ConfigKeyVal * ikey)  {
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  key_vtn_unified_t *key_vtn_unw = ConfigKeyVal::Malloc<key_vtn_unified_t>();

  key_rename_vnode_info *key_rename =
          reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());

  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    ConfigKeyVal::Free(key_vtn_unw);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vtn_unw->vtn_key.vtn_name, key_rename->old_unc_vtn_name,
                       (kMaxLenVtnName + 1));

  okey = new ConfigKeyVal(UNC_KT_VTN_UNIFIED, IpctSt::kIpcStKeyVtnUnified,
                          key_vtn_unw, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtnUnifiedMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
                                            upll_keytype_datatype_t dt_type,
                                            DalDmlIntf *dmi,
                                            uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *unc_key    = NULL;
  upll_rc_t    result_code = UPLL_RC_SUCCESS;

  if ((ikey == NULL) || (ctrlr_id == NULL) || (dmi == NULL)) {
    UPLL_LOG_DEBUG("ikey/ctrlr_id dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr,
                   kOpInOutCtrlr | kOpInOutDomain };
  val_rename_vtn *rename_vtn_key = ConfigKeyVal::Malloc<val_rename_vtn>();
  if (!rename_vtn_key) {
    UPLL_LOG_DEBUG("rename_vtn_key NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>(ikey->get_key());
  if (!vtn_key) {
    UPLL_LOG_ERROR("rename_vtn_key NULL");
    free(rename_vtn_key);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(rename_vtn_key->new_name,
                    vtn_key->vtn_name, (kMaxLenVtnName + 1));
  rename_vtn_key->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

  MoMgrImpl *mgr = static_cast<MoMgrImpl*>((const_cast<MoManager*>(GetMoManager(
      UNC_KT_VTN))));
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail");
    free(rename_vtn_key);
    return result_code;
  }

  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vtn_key);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop,
                                  dmi, RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    key_vtn *vtn_key = reinterpret_cast<key_vtn *>(unc_key->get_key());
    uuu::upll_strncpy(vtn_key->vtn_name, vtn_key->vtn_name,
                      (kMaxLenVtnName + 1));
    SET_USER_DATA(ikey, unc_key);
    SET_USER_DATA_FLAGS(ikey, VTN_RENAME);
  }

  DELETE_IF_NOT_NULL(unc_key);
  return UPLL_RC_SUCCESS;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
