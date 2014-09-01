/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vbr_momgr.hh"
#define NUM_KEY_MAIN_TBL_  5
#define NUM_KEY_RENAME_TBL_ 4


namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VbrMoMgr::vbr_bind_info[] = { { uudst::vbridge::kDbiVtnName, CFG_KEY,
                                         offsetof(key_vbr, vtn_key.vtn_name),
                                         uud::kDalChar, 32 },
                                       { uudst::vbridge::kDbiVbrName, CFG_KEY,
                                         offsetof(key_vbr, vbridge_name),
                                         uud::kDalChar, 32 },
                                       { uudst::vbridge::kDbiCtrlrName, CFG_VAL,
                                         offsetof(val_vbr, controller_id),
                                         uud::kDalChar, 32 },
                                       { uudst::vbridge::kDbiCtrlrName, CK_VAL,
                                         offsetof(key_user_data, ctrlr_id),
                                         uud::kDalChar, 32 },
                                       { uudst::vbridge::kDbiDomainId, CFG_VAL,
                                         offsetof(val_vbr, domain_id),
                                         uud::kDalChar, 32 },
                                       { uudst::vbridge::kDbiDomainId, CK_VAL,
                                         offsetof(key_user_data, domain_id),
                                         uud::kDalChar, 32 },
                                       { uudst::vbridge::kDbiVbrDesc, CFG_VAL,
                                         offsetof(val_vbr, vbr_description),
                                         uud::kDalChar, 128 },
                                       { uudst::vbridge::kDbiHostAddr, CFG_VAL,
                                         offsetof(val_vbr, host_addr),
                                         uud::kDalUint32, 1 },
                                       { uudst::vbridge::kDbiHostAddrMask,
                                         CFG_VAL, offsetof(val_vbr,
                                                           host_addr_prefixlen),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiOperStatus, ST_VAL,
                                         offsetof(val_db_vbr_st,
                                                  vbr_val_st.oper_status),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiDownCount, ST_VAL,
                                         offsetof(val_db_vbr_st, down_count),
                                         uud::kDalUint32, 1 },
                                       { uudst::vbridge::kDbiUnknownCount, ST_VAL,
                                         offsetof(val_db_vbr_st, unknown_count),
                                         uud::kDalUint32, 1 },
                                       { uudst::vbridge::kDbiValidCtrlrName,
                                         CFG_META_VAL, offsetof(val_vbr,
                                                                valid[0]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiValidDomainId,
                                         CFG_META_VAL, offsetof(val_vbr,
                                                                valid[1]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiValidVbrDesc,
                                         CFG_META_VAL, offsetof(val_vbr,
                                                                valid[2]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiValidHostAddr,
                                         CFG_META_VAL, offsetof(val_vbr,
                                                                valid[3]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiValidHostAddrMask,
                                         CFG_META_VAL, offsetof(val_vbr,
                                                                valid[4]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiValidOperStatus,
                                         ST_META_VAL, offsetof(val_vbr_st,
                                                               valid[0]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiCsRowStatus,
                                         CS_VAL, offsetof(val_vbr,
                                                          cs_row_status),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiCsCtrlrName,
                                         CS_VAL, offsetof(val_vbr, cs_attr[0]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiCsDomainId, CS_VAL,
                                         offsetof(val_vbr, cs_attr[1]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiCsVbrDesc, CS_VAL,
                                         offsetof(val_vbr, cs_attr[2]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiCsHostAddr, CS_VAL,
                                         offsetof(val_vbr, cs_attr[3]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiCsHostAddrMask,
                                         CS_VAL, offsetof(val_vbr, cs_attr[4]),
                                         uud::kDalUint8, 1 },
                                       { uudst::vbridge::kDbiVbrFlags, CK_VAL,
                                         offsetof(key_user_data_t, flags),
                                         uud::kDalUint8, 1 } };

BindInfo VbrMoMgr::vbr_rename_bind_info[] = {
    { uudst::vnode_rename::kDbiUncVtnName, CFG_KEY, offsetof(key_vbr,
                                                             vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_KEY, offsetof(key_vbr,
                                                               vbridge_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrName, CK_VAL, offsetof(key_user_data_t,
                                                           ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiDomainId, CK_VAL, offsetof(key_user_data_t,
                                                          domain_id),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrVtnName, CFG_VAL, offsetof(val_rename_vnode,
                                                               ctrlr_vtn_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrVnodeName, CFG_VAL, offsetof(
        val_rename_vnode, ctrlr_vnode_name),
      uud::kDalChar, 32 } };

BindInfo VbrMoMgr::key_vbr_maintbl_bind_info[] = {
    { uudst::vbridge::kDbiVtnName, CFG_MATCH_KEY, offsetof(key_vbr_t,
                                                           vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vbridge::kDbiVbrName, CFG_MATCH_KEY, offsetof(key_vbr_t,
                                                           vbridge_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vbridge::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vbridge::kDbiVbrName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vbridge::kDbiVbrFlags, CK_VAL, offsetof(key_user_data_t,
                                                            flags),
      uud::kDalUint8, 1 } };

BindInfo VbrMoMgr::key_vbr_renametbl_update_bind_info[] = {
    { uudst::vnode_rename::kDbiUncVtnName, CFG_MATCH_KEY, offsetof(
        key_vbr_t, vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_MATCH_KEY, offsetof(
        key_vbr_t, vbridge_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vnode_rename::kDbiUncVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVtnName + 1 }, };

unc_key_type_t VbrMoMgr::vbr_child[] = { UNC_KT_VBR_VLANMAP,
                                         UNC_KT_VBR_NWMONITOR,
                                         UNC_KT_VBR_POLICINGMAP,
                                         UNC_KT_VBR_FLOWFILTER, UNC_KT_VBR_IF };



VbrMoMgr::VbrMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];
  table[MAINTBL] = new Table(uudst::kDbiVbrTbl, UNC_KT_VBRIDGE, vbr_bind_info,
                         IpctSt::kIpcStKeyVbr, IpctSt::kIpcStValVbr,
                         (uudst::vbridge::kDbiVbrNumCols+2));
  table[RENAMETBL] = new Table(uudst::kDbiVNodeRenameTbl, UNC_KT_VBRIDGE,
                  vbr_rename_bind_info, IpctSt::kIpcInvalidStNum,
                  IpctSt::kIpcInvalidStNum,
                  uudst::vnode_rename::kDbiVnodeRenameNumCols);
  table[CTRLRTBL] = NULL;
  nchild = sizeof(vbr_child) / sizeof(*vbr_child);
  child = vbr_child;
}
/*
 * Based on the key type the bind info will pass
 **/
bool VbrMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                                    int &nattr, MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_vbr_maintbl_bind_info;
  } else if (RENAMETBL == tbl) {
    nattr = NUM_KEY_RENAME_TBL_;
    binfo = key_vbr_renametbl_update_bind_info;
  } else {
      UPLL_LOG_TRACE("Invalid table");
      return false;
  }
  return true;
}


bool VbrMoMgr::IsValidKey(void *key,
                          uint64_t index) {
  UPLL_FUNC_TRACE;
  key_vbr *vbr_key = reinterpret_cast<key_vbr *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vbridge::kDbiVtnName:
    case uudst::vnode_rename::kDbiUncVtnName:
        ret_val = ValidateKey(reinterpret_cast<char *>(vbr_key->vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge::kDbiVbrName:
    case uudst::vnode_rename::kDbiUncvnodeName:
      ret_val = ValidateKey(reinterpret_cast<char *>(vbr_key->vbridge_name),
                            kMinLenVnodeName,
                            kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("VBR Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    default:
      UPLL_LOG_TRACE("Invalid Key Index");
      return false;
      break;
  }
  return true;
}

upll_rc_t VbrMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr *vbr_key = NULL;
  if (okey && (okey->get_key())) {
    vbr_key = reinterpret_cast<key_vbr_t *>
                (okey->get_key());
  } else {
    vbr_key = reinterpret_cast<key_vbr *>
      (ConfigKeyVal::Malloc(sizeof(key_vbr)));
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey) 
      okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, vbr_key,
                            NULL);
    else if (okey->get_key() != vbr_key)
      okey->SetKey(IpctSt::kIpcStKeyVbr,vbr_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      free(vbr_key);
    return UPLL_RC_ERR_GENERIC;
  }

  /* presumes MoMgrs receive only supported keytypes */
  switch (parent_key->get_key_type()) {
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(vbr_key->vbridge_name, reinterpret_cast<key_vbr *>
                       (pkey)->vbridge_name, (kMaxLenVnodeName+1));
      uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
             reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name, (kMaxLenVtnName+1));
      break;
    case UNC_KT_VLINK: {
      uint8_t *vnode_name;
      uint8_t flags = 0;
      val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(parent_key));
      if (!vlink_val) {
        free(vbr_key);
        return UPLL_RC_ERR_GENERIC;
      }
      GET_USER_DATA_FLAGS(parent_key->get_cfg_val(), flags);
      flags &=  VLINK_FLAG_NODE_POS;
      UPLL_LOG_DEBUG("Vlink flag node position %d",flags);
      if (flags == kVlinkVnode2) {
        vnode_name = vlink_val->vnode2_name;
      } else {
        vnode_name = vlink_val->vnode1_name;
      }
      uuu::upll_strncpy(vbr_key->vbridge_name, vnode_name,
                          (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(vbr_key->vtn_key.vtn_name, reinterpret_cast<key_vlink *>(
                        parent_key->get_key())->vtn_key.vtn_name , (kMaxLenVtnName+1));
     }
     break;  
    case UNC_KT_VRT_IF:
      uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
             reinterpret_cast<key_vrt_if *>(pkey)->vrt_key.vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTN:
    default:
      uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name, (kMaxLenVtnName+1));
      *(vbr_key->vbridge_name) = *"";
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, vbr_key, NULL);
  else if (okey->get_key() != vbr_key)
    okey->SetKey(IpctSt::kIpcStKeyVbr,vbr_key);
  if (okey == NULL) {
    free(vbr_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

upll_rc_t VbrMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                       ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (ikey == NULL) {
    UPLL_LOG_DEBUG("Null ikey param");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VBRIDGE)
    return UPLL_RC_ERR_GENERIC;
  void *pkey = ikey->get_key();
  if (!pkey)
    return UPLL_RC_ERR_GENERIC;
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>
      (ConfigKeyVal::Malloc(sizeof(key_vtn)));
  uuu::upll_strncpy(vtn_key->vtn_name,
       reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name, (kMaxLenVtnName+1));
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  if (okey == NULL) {
    free(vtn_key);
    return UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey, AdaptType adapt_type) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Config Val is Null");
      return UPLL_RC_ERR_GENERIC;
    }
    while (cval ) {
      if (IpctSt::kIpcStValVbrSt == cval->get_st_num()) {
        val_vbr_st *vbr_val_st = reinterpret_cast<val_vbr_st *>
                         (ConfigKeyVal::Malloc(sizeof(val_vbr_st)));
        val_db_vbr_st *db_vbr_val_st = reinterpret_cast<val_db_vbr_st *>
                                     (cval->get_val());
        memcpy(vbr_val_st,&(db_vbr_val_st->vbr_val_st),
               sizeof(val_vbr_st));
        cval->SetVal(IpctSt::kIpcStValVbrSt, vbr_val_st);
      }
      cval = cval->get_next_cfg_val();
    }
    if (adapt_type == ADAPT_ONE)
      break;
    ikey = ikey->get_next_cfg_key_val();
  } 
  UPLL_LOG_DEBUG("Exiting VbrMoMgr::AdaptValToVtnService");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::AllocVal(ConfigVal *&ck_val,
                             upll_keytype_datatype_t dt_type,
                             MoMgrTables tbl) {
  void *val;  // , *nxt_val;
  // ConfigVal *ck_nxtval;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>(ConfigKeyVal::Malloc(sizeof(val_vbr)));
      ck_val = new ConfigVal(IpctSt::kIpcStValVbr, val);
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_db_vbr_st)));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVbrSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    case RENAMETBL:
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                    ConfigKeyVal *&req,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) {
    UPLL_LOG_DEBUG("Input ConfigKeyVal req is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (okey != NULL) {
    UPLL_LOG_DEBUG("Output ConfigKeyVal okey is not NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (req->get_key_type() != UNC_KT_VBRIDGE) {
    UPLL_LOG_DEBUG("Input ConfigKeyVal keytype mismatch");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
  if (tmp) {
    if (tbl == MAINTBL) {
      val_vbr *ival = reinterpret_cast<val_vbr *>(GetVal(req));
      if (ival == NULL) {
        UPLL_LOG_DEBUG("Null Val structure");
        return UPLL_RC_ERR_GENERIC;
      }
      val_vbr *vbr_val = reinterpret_cast<val_vbr *>
          (ConfigKeyVal::Malloc(sizeof(val_vbr)));
      memcpy(vbr_val, ival, sizeof(val_vbr));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVbr, vbr_val);
    } else if (tbl == RENAMETBL) {
      void *rename_val;
      ConfigVal *ck_v = req->get_cfg_val();
      if (ck_v != NULL && ck_v->get_st_num() == IpctSt::kIpcInvalidStNum) {
        val_rename_vnode *ival = reinterpret_cast<val_rename_vnode *>
                                                          (GetVal(req));
        if (ival == NULL) {
          UPLL_LOG_DEBUG("Null Val structure");
          return UPLL_RC_ERR_GENERIC;
        }
        rename_val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
        memcpy(rename_val, ival, sizeof(val_rename_vnode));
      } else {
        val_rename_vbr *ival = reinterpret_cast<val_rename_vbr *>(GetVal(req));
        if (ival == NULL) {
          UPLL_LOG_DEBUG("Null Val structure");
          return UPLL_RC_ERR_GENERIC;
        }
        rename_val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_rename_vbr)));
        memcpy(rename_val, ival, sizeof(val_rename_vbr));
      }
      tmp1 = new ConfigVal(IpctSt::kIpcStValRenameVbr, rename_val);
    }
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL) {
      val_db_vbr_st *ival = reinterpret_cast<val_db_vbr_st *>(tmp->get_val());
      if (ival == NULL) {
        UPLL_LOG_DEBUG("Null Val structure");
        delete tmp1;
        return UPLL_RC_ERR_GENERIC;
      }
      val_db_vbr_st *val_vbr = reinterpret_cast<val_db_vbr_st *>
          (ConfigKeyVal::Malloc(sizeof(val_db_vbr_st)));
      memcpy(val_vbr, ival, sizeof(val_db_vbr_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVbrSt, val_vbr);
      tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = (req)->get_key();
  if (!tkey) {
    UPLL_LOG_DEBUG("Null tkey");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
//  cout << "VbrMoMgr::DupConfigKeyVal";
  key_vbr *ikey = reinterpret_cast<key_vbr *>(tkey);
  key_vbr *vbr_key = reinterpret_cast<key_vbr *>
      (ConfigKeyVal::Malloc(sizeof(key_vbr)));
  memcpy(vbr_key, ikey, sizeof(key_vbr));
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, vbr_key, tmp1);
  if (!okey) {
    delete tmp1;
    free(vbr_key);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi,
                                     uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  val_rename_vnode *rename_vnode = reinterpret_cast<val_rename_vnode *>(
      ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
  rename_vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_INVALID;
  rename_vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_INVALID;
  key_vbr *ctrlr_key = reinterpret_cast<key_vbr *>(ikey->get_key());
  upll_rc_t ret_val = ValidateKey(reinterpret_cast<char *>(ctrlr_key->vtn_key.vtn_name),
                        kMinLenVtnName, kMaxLenVtnName);
  if (ret_val == UPLL_RC_SUCCESS)  {
    uuu::upll_strncpy(rename_vnode->ctrlr_vtn_name, ctrlr_key->vtn_key.vtn_name,
                   (kMaxLenVtnName+1));
    rename_vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
  }
  ret_val = ValidateKey(reinterpret_cast<char *>(ctrlr_key->vbridge_name),
                        kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val == UPLL_RC_SUCCESS)  {
    uuu::upll_strncpy(rename_vnode->ctrlr_vnode_name, ctrlr_key->vbridge_name,
                   (kMaxLenVnodeName+1));
    rename_vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
  }
  result_code = GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed with result_code %d",
                     result_code);
    FREE_IF_NOT_NULL(rename_vnode);
    return result_code;
  }
  if (ctrlr_id) {
    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  }else
    dbop.matchop = kOpMatchNone;
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vnode);

    uint8_t rename  = 0;
//  UPLL_LOG_TRACE("Before Read from Rename Table %s", (unc_key->ToStrAll()).c_str());
  dbop.inoutop = kOpInOutCtrlr | kOpInOutDomain ;
  result_code = ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                              RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    key_vbr *vbr_key = reinterpret_cast<key_vbr *>(unc_key->get_key());
    if (strcmp(reinterpret_cast<char *>(ctrlr_key->vtn_key.vtn_name),
               reinterpret_cast<char*>(vbr_key->vtn_key.vtn_name))) {
      UPLL_LOG_DEBUG("Not Same Vtn Name");
      uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, vbr_key->vtn_key.vtn_name,
                        (kMaxLenVtnName+1));
      rename |= VTN_RENAME;
    }
    if (strcmp(reinterpret_cast<char *>(ctrlr_key->vbridge_name),
               reinterpret_cast<const char *>(vbr_key->vbridge_name))) {
      UPLL_LOG_DEBUG("Not same Vbridge Name");
      uuu::upll_strncpy(ctrlr_key->vbridge_name, vbr_key->vbridge_name,
                        (kMaxLenVnodeName+1));
      rename |= VN_RENAME;
    }
  SET_USER_DATA(ikey, unc_key);
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {

    upll_rc_t res_code = UPLL_RC_SUCCESS;
    MoMgrImpl *vtn_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                                       (GetMoManager(UNC_KT_VTN)));
    if (!vtn_mgr) {
      UPLL_LOG_DEBUG("mgr is NULL");
      DELETE_IF_NOT_NULL(unc_key);
      return UPLL_RC_ERR_GENERIC;
    }
    DELETE_IF_NOT_NULL(unc_key);
    res_code = vtn_mgr->GetChildConfigKey(unc_key, NULL);
    if (res_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed with res_code %d",
                     res_code);
      return res_code;
    }
    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
    key_vtn *vtn_key = reinterpret_cast<key_vtn *>(unc_key->get_key());
    uuu::upll_strncpy(vtn_key->vtn_name, ctrlr_key->vtn_key.vtn_name,
                      (kMaxLenVtnName+1));
    res_code = vtn_mgr->GetRenamedUncKey(unc_key, dt_type,
                                            dmi, ctrlr_id);
    if (res_code == UPLL_RC_SUCCESS) {
      if (strcmp(reinterpret_cast<char *>(ctrlr_key->vtn_key.vtn_name),
                 reinterpret_cast<char*>(vtn_key->vtn_name))) {
        UPLL_LOG_DEBUG("Not Same Vtn Name");
        uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, vtn_key->vtn_name,
                          (kMaxLenVtnName+1));
        rename |= VTN_RENAME;
      }
    }
  SET_USER_DATA_CTRLR(ikey, ctrlr_id);
  }
  SET_USER_DATA_FLAGS(ikey, rename);
  delete unc_key;
  return result_code;
}


upll_rc_t VbrMoMgr::GetRenamedControllerKey(ConfigKeyVal *ikey,
                                            upll_keytype_datatype_t dt_type,
                                            DalDmlIntf *dmi,
                                            controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  uint8_t rename = 0;
  ConfigKeyVal *okey = NULL;
  if (ikey == NULL || dmi == NULL) {
    UPLL_LOG_DEBUG("Null param ikey/dmi");
    return UPLL_RC_ERR_GENERIC;
  }
#if 0
  ConfigKeyVal *dup_key = NULL;
  val_vbr_t *val = reinterpret_cast<val_vbr_t *>(GetVal(ikey));
  ConfigVal *cval = ikey->get_cfg_val();
  if (cval && cval->get_st_num() == IpctSt::kIpcStValVbr &&
      val && val->valid[UPLL_IDX_CONTROLLER_ID_VBR] == UNC_VF_VALID &&
      val->valid[UPLL_IDX_DOMAIN_ID_VBR] == UNC_VF_VALID) {
    GET_USER_DATA_FLAGS(ikey, rename);
  } else {
    result_code = GetChildConfigKey(dup_key, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed %d",result_code);
      return result_code;
    }
#endif
    result_code = IsRenamed(ikey, dt_type, dmi, rename);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning %d",result_code);
      return result_code;
    }
    if (!ctrlr_dom->ctrlr ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->ctrlr)) ||
      !ctrlr_dom->domain ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->domain))) {
      GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
    }
#if 0
    cval = dup_key->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Val is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = GetControllerDomainId(dup_key, ctrlr_dom);
    SET_USER_DATA(ikey, dup_key);
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
    if (dup_key)
     delete dup_key;
  }
  if (!ctrlr_dom->ctrlr ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->ctrlr)) ||
      !ctrlr_dom->domain ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->domain))) {
    result_code = GetControllerDomainId(ikey, ctrlr_dom);
  }
  if (!ctrlr_dom->ctrlr ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->ctrlr)) ||
      !ctrlr_dom->domain ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->domain))) {
      UPLL_LOG_DEBUG("Invalid Ctrlr/domain");
      return UPLL_RC_ERR_GENERIC;
  }
#endif
  if (rename == 0)
    return UPLL_RC_SUCCESS;
  /* Read controller name from running rename table,
   * since there is no rename table for audit case */
  if (dt_type == UPLL_DT_AUDIT)
    dt_type = UPLL_DT_RUNNING;

  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("Returning error %d",result_code);
     return result_code;
  }
  SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                                  kOpInOutFlag };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi, RENAMETBL);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  key_vbr *ctrlr_key = reinterpret_cast<key_vbr *>(ikey->get_key());
  if (result_code == UPLL_RC_SUCCESS) {
    val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode *>
        (GetVal(okey));
    if (!rename_val) {
      UPLL_LOG_DEBUG("Val is Empty");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (!ctrlr_key) {
      UPLL_LOG_DEBUG("Key is Empty");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_TRACE("Rename flag %d", rename);
    if (rename & VTN_RENAME) { /* vtn renamed */
      uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, rename_val->ctrlr_vtn_name,
                        (kMaxLenVtnName+1));
    }
    if (rename & VN_RENAME) { /* vnode renamed */
      uuu::upll_strncpy(ctrlr_key->vbridge_name, rename_val->ctrlr_vnode_name,
                        (kMaxLenVnodeName+1));
    }
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    MoMgrImpl *vtn_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                                       (GetMoManager(UNC_KT_VTN)));
    if (!vtn_mgr) {
      UPLL_LOG_DEBUG("mgr is NULL");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    DELETE_IF_NOT_NULL(okey);
    result_code = vtn_mgr->GetChildConfigKey(okey, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed with result_code %d",
                     result_code);
      DELETE_IF_NOT_NULL(okey);
      return result_code;
    }
    key_vtn *unc_key = reinterpret_cast<key_vtn *>(okey->get_key());
    uuu::upll_strncpy(unc_key->vtn_name, ctrlr_key->vtn_key.vtn_name,
                      (kMaxLenVtnName+1));
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
    result_code = vtn_mgr->GetRenamedControllerKey(okey, dt_type,
                                                   dmi, ctrlr_dom);
    if (result_code == UPLL_RC_SUCCESS) {
      if (strcmp(reinterpret_cast<char *>(ctrlr_key->vtn_key.vtn_name),
                 reinterpret_cast<char*>(unc_key->vtn_name))) {
        UPLL_LOG_DEBUG("Not Same Vtn Name");
        uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, unc_key->vtn_name,
                          (kMaxLenVtnName+1));
      }
    }
  }
  SET_USER_DATA_FLAGS(ikey, rename);
  DELETE_IF_NOT_NULL(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::UpdateConfigStatus(ConfigKeyVal *vbr_key,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vbr *vbr_val;

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED
                                        : UNC_CS_NOT_APPLIED;
  vbr_val = reinterpret_cast<val_vbr *>(GetVal(vbr_key));
  val_vbr *vbr_val2 = reinterpret_cast<val_vbr *>(GetVal(upd_key));
  if (vbr_val == NULL) return UPLL_RC_ERR_GENERIC;
  UPLL_LOG_TRACE("Key in Candidate %s", (vbr_key->ToStrAll()).c_str());

  if (op == UNC_OP_CREATE) {
    vbr_val->cs_row_status = cs_status;
    val_db_vbr_st *val_vbrst = reinterpret_cast<val_db_vbr_st *>
        (ConfigKeyVal::Malloc(sizeof(val_db_vbr_st)));
    if(driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
      val_vbrst->vbr_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
    } else {
      val_vbrst->vbr_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
    }
    val_vbrst->down_count  = 0;
    val_vbrst->unknown_count  = 0;
    val_vbrst->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
    vbr_key->AppendCfgVal(IpctSt::kIpcStValVbrSt, val_vbrst);
  } else if (op == UNC_OP_UPDATE) {
    void *vbrval = reinterpret_cast<void *>(vbr_val);
    CompareValidValue(vbrval, GetVal(upd_key), true);
    UPLL_LOG_TRACE("Key in Running %s", (upd_key->ToStrAll()).c_str());
    vbr_val->cs_row_status = vbr_val2->cs_row_status;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
    loop < sizeof(vbr_val->valid) / sizeof(vbr_val->valid[0]); ++loop) {
    if ((UNC_VF_VALID == vbr_val->valid[loop])
          || (UNC_VF_VALID_NO_VALUE == vbr_val->valid[loop])) {
      // Description is set to APPLIED
      if (loop == UPLL_IDX_DESC_VBR)
        vbr_val->cs_attr[loop] = UNC_CS_APPLIED;
      else
        vbr_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == vbr_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
        vbr_val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if ((UNC_VF_INVALID == vbr_val->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
        vbr_val->cs_attr[loop] = vbr_val2->cs_attr[loop];
    }
  }
  return result_code;
}

/* Pure Virtual functions from MoMgrImpl */
upll_rc_t VbrMoMgr::GetControllerDomainId(ConfigKeyVal *ikey,
                                          controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (!ikey || !ctrlr_dom) {
    UPLL_LOG_INFO("Illegal parameter");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vbr * temp_vbr = reinterpret_cast<val_vbr *> (GetVal(ikey));
  if (temp_vbr == NULL
      || temp_vbr->valid[UPLL_IDX_CONTROLLER_ID_VBR] != UNC_VF_VALID
      || !strlen(reinterpret_cast<char*>(temp_vbr->controller_id))) {
    ctrlr_dom->ctrlr = NULL;
    UPLL_LOG_DEBUG("Ctrlr null");
  } else {
    SET_USER_DATA_CTRLR(ikey, temp_vbr->controller_id);
  }
  if (temp_vbr == NULL
      || temp_vbr->valid[UPLL_IDX_DOMAIN_ID_VBR] != UNC_VF_VALID
      || !strlen(reinterpret_cast<char*>(
         temp_vbr->domain_id))) {
    ctrlr_dom->domain = NULL;
    UPLL_LOG_DEBUG("Domain null");
  } else {
    SET_USER_DATA_DOMAIN(ikey, temp_vbr->domain_id);
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  }
  UPLL_LOG_DEBUG("ctrlr_dom %s %s",ctrlr_dom->ctrlr, ctrlr_dom->domain);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::GetVnodeName(ConfigKeyVal *ikey,
                                 uint8_t *&vtn_name,
                                 uint8_t *&vnode_name) {
  // UPLL_FUNC_TRACE;
  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>(ikey->get_key());
  if (vbr_key == NULL) return UPLL_RC_ERR_GENERIC;
  vtn_name = vbr_key->vtn_key.vtn_name;
  vnode_name = vbr_key->vbridge_name;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::SwapKeyVal(ConfigKeyVal *ikey,
                               ConfigKeyVal *&okey,
                               DalDmlIntf *dmi,
                               uint8_t *ctrlr,
                               bool &no_rename) {
  // UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  okey = NULL;
  if (!ikey || !(ikey->get_key()))
    return UPLL_RC_ERR_GENERIC;

  if (ikey->get_key_type() != UNC_KT_VBRIDGE) return UPLL_RC_ERR_BAD_REQUEST;

  val_rename_vbr_t *tval = reinterpret_cast<val_rename_vbr_t *> (GetVal(ikey));
  if (!tval) {
    UPLL_LOG_DEBUG("tval is null");
    return UPLL_RC_ERR_GENERIC;
  }
  /* The PFC Name and New Name should not be equal */
  if (!strcmp(reinterpret_cast<char *>(tval->new_name),
              reinterpret_cast<char *>(reinterpret_cast<key_vbr_t *>
              (ikey->get_key())->vbridge_name)))
    return UPLL_RC_ERR_GENERIC;

  key_vbr_t *key_vbr = reinterpret_cast<key_vbr_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_t)));

  if (tval->valid[UPLL_IDX_NEW_NAME_RVBR] == UNC_VF_VALID_NO_VALUE) {
    uuu::upll_strncpy(key_vbr->vbridge_name, (static_cast<key_vbr_t *>
                     (ikey->get_key())->vbridge_name), (kMaxLenVnodeName+1));
    no_rename = true;
  } else {
    if (reinterpret_cast<val_rename_vbr_t *>
       (tval)->valid[UPLL_IDX_NEW_NAME_RVBR] == UNC_VF_VALID) {
      // checking the string is empty or not
      if (!strlen(reinterpret_cast<char *>(static_cast<val_rename_vbr_t *>
         (tval)->new_name))) {
        free(key_vbr);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(key_vbr->vbridge_name, (static_cast<val_rename_vbr_t *>
           ((ikey->get_cfg_val())->get_val()))->new_name, (kMaxLenVnodeName+1));
    } else {
      free(key_vbr);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  // Checking the vbridge parent is renamed get the UNC name
  ConfigKeyVal *pkey = NULL;
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code || pkey == NULL) {
    if (pkey) delete pkey;
    free(key_vbr);
    return result_code;
  }
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VTN)));
  result_code = mgr->GetRenamedUncKey(pkey, UPLL_DT_IMPORT, dmi, ctrlr);
  if (UPLL_RC_SUCCESS != result_code
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    delete pkey;
    free(key_vbr);
    return result_code;
  }
  // use the UNC VTN name if PFC VTN name is renamed;
  if (strlen(reinterpret_cast<char *>(reinterpret_cast<key_vtn_t *>
                                     (pkey->get_key())->vtn_name)))
    uuu::upll_strncpy(key_vbr->vtn_key.vtn_name, reinterpret_cast<key_vtn_t *>
                     (pkey->get_key())->vtn_name, (kMaxLenVtnName+1));
  delete pkey;
  pkey = NULL;
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key_vbr, NULL);
  if (!okey) {
    free(key_vbr);
    return UPLL_RC_ERR_GENERIC;
  }
//  cout << " SetConfigEnd ";
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vbr_t *val;
  val = (ckv_running != NULL) ? reinterpret_cast<val_vbr_t *>
                               (GetVal(ckv_running)) : NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase)
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;
  for (unsigned int loop = 0; loop < sizeof(val->valid) / sizeof(uint8_t);
      ++loop) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) ||
         cs_status == UNC_CS_APPLIED)
       val->cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t VbrMoMgr::MergeValidate(unc_key_type_t keytype,
                                  const char *ctrlr_id,
                                  ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi,
                                  upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *tkey = NULL;

  if (!ikey || !ikey->get_key() || !(strlen(reinterpret_cast<const char *>
     (ctrlr_id)))) {
    UPLL_LOG_DEBUG("Input is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *dup_key = NULL;
  result_code = GetChildConfigKey(dup_key, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed");
    if (dup_key) delete dup_key;
    return result_code;
  }
  /*
   * Here getting FULL Key (VTN & VBR Name )
   */
  result_code = ReadConfigDB(dup_key, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (dup_key) delete dup_key;
     return result_code;
  }

  ConfigKeyVal *travel = dup_key;
  while (travel) {
    /*
     * Checks the Val structure is available or not.If availabl
     * Checks Host address value is available or not in import ckval
     */
    result_code = DupConfigKeyVal(tkey, travel, MAINTBL);

    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG(" DupConfigKeyVal is Failed");
      if (tkey) delete tkey;
      if (dup_key) delete dup_key;
      return result_code;
    }
   /* Same Name should not present in the vnodes in running*/
    if (import_type == UPLL_IMPORT_TYPE_FULL) {
      result_code = VnodeChecks(tkey, UPLL_DT_CANDIDATE, dmi);
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code ||
        UPLL_RC_ERR_CFG_SEMANTIC == result_code) {
        ikey->ResetWith(tkey);
        DELETE_IF_NOT_NULL(tkey);
        if (dup_key) delete dup_key;
        UPLL_LOG_DEBUG("VBridge Name Conflict %s", (ikey->ToStrAll()).c_str());
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
    } else {
      result_code = PartialImport_VnodeChecks(tkey,
                                  UPLL_DT_CANDIDATE, ctrlr_id ,dmi);
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code ||
        UPLL_RC_ERR_CFG_SEMANTIC == result_code) {
        ikey->ResetWith(tkey);
        DELETE_IF_NOT_NULL(tkey);
        if (dup_key) delete dup_key;
        UPLL_LOG_DEBUG("VBridge Name Conflict %s", (ikey->ToStrAll()).c_str());
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
    }
    /* Any other DB error */
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("VnodeChecks Failed %d", result_code);
       if (tkey) delete tkey;
       if (dup_key) delete dup_key;
      return result_code;
    }
    result_code = MergeValidateIpAddress(travel, UPLL_DT_CANDIDATE,
        dmi, ctrlr_id, import_type);
    if (UPLL_RC_SUCCESS != result_code) {
      /* If Merge conflict ikey should reset with that ConfigKeyVal */
      if (result_code == UPLL_RC_ERR_MERGE_CONFLICT) {
        ikey->ResetWith(tkey);
        delete dup_key;
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
      if (dup_key) delete dup_key;
      return result_code;
    }
    DELETE_IF_NOT_NULL(tkey);
    travel = travel->get_next_cfg_key_val();
  }
  if (dup_key)
    delete dup_key;
  return result_code;
}


#if 0
  while (travel) {
    result_code = MergeValidateIpAddress(travel, UPLL_DT_CANDIDATE,
                                         dmi, ctrlr_id, import_type);
    if (result_code != UPLL_RC_SUCCESS) {
      /* If Merge conflict ikey should reset with that ConfigKeyVal */
      if (result_code == UPLL_RC_ERR_MERGE_CONFLICT) {
        ikey->ResetWith(tkey);
        delete dup_key;
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
      if (dup_key) delete dup_key;
      return result_code;
    }
    travel = travel->get_next_cfg_key_val();
  }  //  while end
  if (dup_key)
    delete dup_key;
  return result_code;
}

  while (travel) {
    /*
     * Checks the Val structure is available or not.If availabl
     * Checks Host address value is available or not in import ckval
     */
    result_code = DupConfigKeyVal(tkey, travel, MAINTBL);

    if (UPLL_RC_SUCCESS != result_code || tkey == NULL) {
      UPLL_LOG_DEBUG(" DupConfigKeyVal is Failed");
      if (tkey) delete tkey;
      if (dup_key) delete dup_key;
      return result_code;
    }
   /* Same Name should not present in the vnodes in running*/
    if (import_type == UPLL_IMPORT_TYPE_FULL) {
      result_code = VnodeChecks(tkey, UPLL_DT_CANDIDATE, dmi);
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code ||
        UPLL_RC_ERR_CFG_SEMANTIC == result_code) {
        ikey->ResetWith(tkey);
        if (tkey) delete tkey;
        if (dup_key) delete dup_key;
        UPLL_LOG_DEBUG("VBridge Name Conflict %s", (ikey->ToStrAll()).c_str());
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
    } else {
      result_code = PartialImport_VnodeChecks(tkey,
                                  UPLL_DT_CANDIDATE, ctrlr_id ,dmi);
    }
    /* Any other DB error */
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("VnodeChecks Failed %d", result_code);
       if (tkey) delete tkey;
       if (dup_key) delete dup_key;
      return result_code;
    }

    val_vbr_t *tval = reinterpret_cast<val_vbr_t *>(GetVal(tkey));
    if (!tval) {
       UPLL_LOG_DEBUG(" Value Structure is not Available ");
    }
    if (tval &&
        tval->valid[UPLL_IDX_HOST_ADDR_VBR] == UNC_VF_VALID) {
      /* set the host address field set to valid others are invalid */
      tval->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_VALID;
      tval->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_VALID;
      tval->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_INVALID;
      tval->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
      tval->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;

      /*Checsk the hostaddress should not be present same vtn */
      memset(reinterpret_cast<key_vbr_t *>
          (tkey->get_key())->vbridge_name, 0, (kMaxLenVnodeName+1));
      result_code = ReadConfigDB(tkey, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop1, dmi,
          MAINTBL);

      result_code = MergeValidateIpAddress(travel, UPLL_DT_CANDIDATE,
                                         dmi, ctrlr_id, import_type);
      if (UPLL_RC_SUCCESS == result_code) {
        if (import_type == UPLL_IMPORT_TYPE_FULL) {
          ikey->ResetWith(tkey);
          if (tkey) delete tkey;
          if (dup_key) delete dup_key;
          UPLL_LOG_DEBUG("VBridge Host Address Conflict");
          return UPLL_RC_ERR_MERGE_CONFLICT;
        } else { 
          uint8_t *ctrlr_name = NULL;
          GET_USER_DATA_CTRLR(tkey,ctrlr_name);
          if (strcmp(ctrlr_id,(const char *)ctrlr_name)) {
            UPLL_LOG_INFO("controller names are mismatched");
            return UPLL_RC_ERR_MERGE_CONFLICT;
          }
 
          if (strcmp(reinterpret_cast< const char *>(reinterpret_cast<key_vbr_t *>(
              ikey->get_key())->vtn_key.vtn_name), reinterpret_cast< const char *>
             (reinterpret_cast<key_vbr_t *>(tkey->get_key())->vtn_key.vtn_name))) {
            UPLL_LOG_INFO("vtn names are mismatched");
            return UPLL_RC_ERR_MERGE_CONFLICT;
          }
        }
      }

      /* Any other DB error */
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code);
        if (tkey) delete tkey;
        if (dup_key) delete dup_key;
        return result_code;
      }
 
      if (tkey) {
      delete tkey;
      tkey = NULL; 
      }
    travel = travel->get_next_cfg_key_val();
  }
  if (dup_key)
    delete dup_key;
  return result_code;
}
#endif
upll_rc_t VbrMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                    ConfigKeyVal *ikey) {
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>
                                                    (ikey->get_key());
  key_vbr_t * key_vbr = reinterpret_cast<key_vbr_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_t)));
  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
    free(key_vbr);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(key_vbr->vtn_key.vtn_name, key_rename->old_unc_vtn_name,
                    (kMaxLenVtnName+1));
  if (ikey->get_key_type() == table[MAINTBL]->get_key_type()) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      free(key_vbr);
      return UPLL_RC_ERR_GENERIC;
    }
  uuu::upll_strncpy(key_vbr->vbridge_name, key_rename->old_unc_vnode_name,
                      (kMaxLenVnodeName+1));
  }
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, key_vbr, NULL);
  if (!okey) {
    free(key_vbr);
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA(okey, ikey);
  return result_code;
}

upll_rc_t VbrMoMgr::GetRenameInfo(ConfigKeyVal *ikey,
                                  ConfigKeyVal *okey,
                                  ConfigKeyVal *&rename_info,
                                  DalDmlIntf *dmi,
                                  const char *ctrlr_id,
                                  bool &renamed) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!okey || !ikey || (rename_info != NULL))
    return UPLL_RC_ERR_GENERIC;

  key_vbr_t * vbr_key = NULL;
  key_rename_vnode_info *vbr_rename_info =
                    reinterpret_cast<key_rename_vnode_info *>
                    (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info)));

  vbr_key = reinterpret_cast<key_vbr_t *>(ikey->get_key());
  if (!vbr_key) {
    free(vbr_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  if (renamed) {
    if (! reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey)) ||
        !(strlen(reinterpret_cast<char *>
               (reinterpret_cast<val_rename_vnode_t *>
               (GetVal(ikey))->ctrlr_vnode_name)))) {
      free(vbr_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vbr_rename_info->ctrlr_vnode_name,
        reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey))->ctrlr_vnode_name,
         (kMaxLenVnodeName+1));
    if (! reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey)) ||
        !(strlen(reinterpret_cast<char *>
               (reinterpret_cast<val_rename_vnode_t *>
               (GetVal(ikey))->ctrlr_vtn_name)))) {
      free(vbr_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vbr_rename_info->ctrlr_vtn_name,
          reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey))->ctrlr_vtn_name,
          (kMaxLenVtnName+1));
  } else {
    if (strlen(reinterpret_cast<char *>(vbr_key->vbridge_name)) == 0) {
      free(vbr_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vbr_rename_info->ctrlr_vnode_name, vbr_key->vbridge_name,
                      (kMaxLenVnodeName+1));
    if (strlen(reinterpret_cast<char *>(vbr_key->vtn_key.vtn_name)) == 0) {
      free(vbr_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vbr_rename_info->ctrlr_vtn_name,
         vbr_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
  }

  if (strlen(reinterpret_cast<char *>(vbr_key->vbridge_name)) == 0) {
    free(vbr_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vbr_rename_info->old_unc_vnode_name, vbr_key->vbridge_name,
                    (kMaxLenVnodeName+1));
  if (strlen(reinterpret_cast<char *>(vbr_key->vtn_key.vtn_name)) == 0) {
    free(vbr_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vbr_rename_info->new_unc_vtn_name,
                   vbr_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
  uuu::upll_strncpy(vbr_rename_info->old_unc_vtn_name,
         vbr_key->vtn_key.vtn_name, (kMaxLenVtnName+1));

  vbr_key = reinterpret_cast<key_vbr_t *>(okey->get_key());
  if (!vbr_key) {
    free(vbr_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  if (strlen(reinterpret_cast<char *>(vbr_key->vtn_key.vtn_name)) == 0) {
    free(vbr_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vbr_rename_info->new_unc_vnode_name,
         vbr_key->vbridge_name, (kMaxLenVnodeName+1));

  rename_info = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcInvalidStNum,
                                 vbr_rename_info, NULL);
  if (!rename_info) {
    free(vbr_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("Renamed bool value is %d", renamed);
  if (!renamed) {
#if 0
    result_code = GetControllerDomainId(okey, &ctrlr_dom);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetControllerDomainId is Failed");
      return result_code;
    }
#endif
    val_rename_vnode_t *vnode = reinterpret_cast<val_rename_vnode_t*>
        (ConfigKeyVal::Malloc(sizeof(val_rename_vnode_t)));
    ConfigKeyVal *tmp_key = NULL;
    result_code = GetChildConfigKey(tmp_key, ikey);
    if (UPLL_RC_SUCCESS!=result_code || tmp_key == NULL) {
      free(vnode);
      return result_code;
    }
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};
    result_code = ReadConfigDB(tmp_key, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                                MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("ReadConfigDB Failed %d\n", result_code);
      free(vnode);
      delete tmp_key;
      return result_code;
    }
    controller_domain ctrlr_dom;
    result_code = GetControllerDomainId(tmp_key, &ctrlr_dom);
    if (UPLL_RC_SUCCESS != result_code) {
       UPLL_LOG_INFO("Returning error %d\n",result_code);
       delete tmp_key;
       return result_code;
    }
    SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    uuu::upll_strncpy(vnode->ctrlr_vtn_name, reinterpret_cast<key_vbr_t *>
                     (ikey->get_key())->vtn_key.vtn_name, (kMaxLenVtnName+1));
    uuu::upll_strncpy(vnode->ctrlr_vnode_name, reinterpret_cast<key_vbr_t *>
                     (ikey->get_key())->vbridge_name, (kMaxLenVnodeName+1));
    ConfigVal *rename_val_ = new ConfigVal(IpctSt::kIpcInvalidStNum, vnode);
    okey->SetCfgVal(rename_val_);
    vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
    vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
    UPLL_LOG_TRACE("Before Create entry in rename table %s",
        (okey->ToStrAll()).c_str());
    dbop.readop = kOpNotRead;
    result_code = UpdateConfigDB(okey, UPLL_DT_IMPORT, UNC_OP_CREATE, dmi,
                                 &dbop, RENAMETBL);
    delete tmp_key;
  }
  return result_code;
}

upll_rc_t VbrMoMgr::CtrlrIdAndDomainIdUpdationCheck(ConfigKeyVal *ikey,
                                                   ConfigKeyVal *okey) {
  UPLL_FUNC_TRACE;
  val_vbr *vbr_val = reinterpret_cast<val_vbr *>(GetVal(ikey));
  val_vbr *vbr_val1 = reinterpret_cast<val_vbr *>(GetVal(okey));
  if (vbr_val->valid[UPLL_IDX_CONTROLLER_ID_VBR] == UNC_VF_VALID) {
    if (strncmp(reinterpret_cast<const char *>(vbr_val->controller_id),
                reinterpret_cast<const char *>(vbr_val1->controller_id),
                kMaxLenCtrlrId+1)) {
      UPLL_LOG_DEBUG("controller id comparision failed");
      return UPLL_RC_ERR_CFG_SYNTAX;
    } 
  } 
  if (vbr_val->valid[UPLL_IDX_DOMAIN_ID_VBR] == UNC_VF_VALID) {
    if (strncmp(reinterpret_cast<const char *>(vbr_val->domain_id),
                reinterpret_cast<const char *>(vbr_val1->domain_id),
                kMaxLenDomainId+1)) {
      UPLL_LOG_DEBUG("domain id comparision failed"); 
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::ValidateVbrKey(key_vbr *vbr_key,
                       unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(reinterpret_cast<char *>(vbr_key->vtn_key.vtn_name),
                        kMinLenVtnName,
                        kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("vtn name syntax check failed."
                  "Received vtn_name - %s",
                  vbr_key->vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (operation != UNC_OP_READ_SIBLING_COUNT && 
      operation != UNC_OP_READ_SIBLING_BEGIN) {
    ret_val = ValidateKey(reinterpret_cast<char *> (vbr_key->vbridge_name),
                        kMinLenVnodeName,
                        kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Vbridge name syntax check failed."
                  "Received vbridge_name - %s",
                  vbr_key->vbridge_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(vbr_key->vbridge_name);
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VbrMoMgr::ValidateVbrValue(val_vbr *vbr_val,
                                     uint32_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val = false;

  // Attribute syntax validation
  for (unsigned int valid_index = 0;
       valid_index < sizeof(vbr_val->valid) / sizeof(vbr_val->valid[0]);
       valid_index++) { 
    if (vbr_val->valid[valid_index] == UNC_VF_VALID) {
      switch(valid_index) {    
        case UPLL_IDX_CONTROLLER_ID_VBR:
          ret_val = ValidateString(vbr_val->controller_id,
                                   kMinLenCtrlrId, kMaxLenCtrlrId); 
          break;
        case UPLL_IDX_DOMAIN_ID_VBR:
          ret_val = ValidateDefaultStr(vbr_val->domain_id,
                                       kMinLenDomainId, kMaxLenDomainId);
          break;
        case UPLL_IDX_DESC_VBR:
          ret_val = ValidateDesc(vbr_val->vbr_description,
                                 kMinLenDescription, kMaxLenDescription);
          break; 
        case UPLL_IDX_HOST_ADDR_VBR:
          ret_val = ValidateIpv4Addr(vbr_val->host_addr.s_addr,
              vbr_val->host_addr_prefixlen);  
          break;
        case UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR:
          ret_val = ValidateNumericRange(
              (uint8_t) vbr_val->host_addr_prefixlen,
              kMinIpv4Prefix, kMaxIpv4Prefix, true, true);
          break;
      }
      if (!ret_val) {
        return UPLL_RC_ERR_CFG_SYNTAX; 
      }
    }
  }

  // Additional checks
  for (unsigned int valid_index = 0;
       valid_index < sizeof(vbr_val->valid) / sizeof(vbr_val->valid[0]);
       valid_index++) {
    uint8_t flag = vbr_val->valid[valid_index];
    switch (operation) {
      case UNC_OP_CREATE:
        {
          switch(valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VBR:
            case UPLL_IDX_DOMAIN_ID_VBR:
              if ((flag == UNC_VF_INVALID || flag == UNC_VF_VALID_NO_VALUE)) {
                UPLL_LOG_DEBUG("controller_id or domain_id flag is invalid"
                               " or valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX; 
              }
              break;
            case UPLL_IDX_DESC_VBR:
              break;
            case UPLL_IDX_HOST_ADDR_VBR:
              if (vbr_val->valid[UPLL_IDX_HOST_ADDR_VBR] !=
                  vbr_val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR]) {
                UPLL_LOG_DEBUG("Host address and prefix length"
                    "both do not have same valid flags: %d, %d",
                    vbr_val->valid[UPLL_IDX_HOST_ADDR_VBR],
                    vbr_val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR]);
                return UPLL_RC_ERR_CFG_SYNTAX; 
              }
              break;
            case UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR:
              if ((flag == UNC_VF_INVALID) || (flag == UNC_VF_VALID_NO_VALUE)) {
                vbr_val->host_addr_prefixlen = 0;
              }
              break;
            default:
              break;
          }
        } 
        break;
      case UNC_OP_UPDATE:
        {
          switch(valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VBR:
            case UPLL_IDX_DOMAIN_ID_VBR:
              if (flag == UNC_VF_VALID_NO_VALUE) {
                UPLL_LOG_DEBUG("controller_id or domain_id flag is valid_no_value");
                return UPLL_RC_ERR_CFG_SYNTAX;
              }
              break; 
            case UPLL_IDX_DESC_VBR:
            case UPLL_IDX_HOST_ADDR_VBR:
            case UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR:
            default:
              break;
          }
        } 
        break;
    }
  }

  // Resets
  for (unsigned int valid_index = 0;
       valid_index < sizeof(vbr_val->valid) / sizeof(vbr_val->valid[0]);
       valid_index++) {
    uint8_t flag = vbr_val->valid[valid_index];
    if (flag != UNC_VF_INVALID && flag != UNC_VF_VALID_NO_VALUE)  
      continue;
       
    switch(valid_index) {
      case UPLL_IDX_CONTROLLER_ID_VBR:
        StringReset(vbr_val->controller_id);
        break;
      case UPLL_IDX_DOMAIN_ID_VBR:
        StringReset(vbr_val->domain_id);
        break;
      case UPLL_IDX_DESC_VBR:
        StringReset(vbr_val->vbr_description);
        break;
      case UPLL_IDX_HOST_ADDR_VBR:
        vbr_val->host_addr.s_addr = 0; 
        break;
      case UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR:
        vbr_val->host_addr_prefixlen = 0;
        break;
      default:
        UPLL_LOG_TRACE("Never here");
        break;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::ValidateVbrPingValue(val_ping *ping_val) {
  UPLL_FUNC_TRACE;

  if (ping_val->valid[UPLL_IDX_TARGET_ADDR_PING] == UNC_VF_VALID) {
    if ((!bc_check(ping_val->target_addr))
        || (!mc_check(ping_val->target_addr))) {
      UPLL_LOG_DEBUG("Invalid target address received."
                    "Received Target addr - %d",
                    ping_val->target_addr);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_DEBUG("Target Address is mandatory for ping operation");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (ping_val->valid[UPLL_IDX_SRC_ADDR_PING] == UNC_VF_VALID) {
    if ((!bc_check(ping_val->src_addr)) || (!mc_check(ping_val->src_addr))) {
      UPLL_LOG_DEBUG("Invalid Source address received."
                    "Received Source addr - %d",
                    ping_val->src_addr);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (ping_val->valid[UPLL_IDX_DF_BIT_PING] == UNC_VF_VALID) {
    if (!ValidateNumericRange(ping_val->dfbit, (uint8_t) UPLL_DF_BIT_DISABLE,
                              (uint8_t) UPLL_DF_BIT_ENABLE, true, true)) {
      UPLL_LOG_DEBUG("dfbit syntax check failed."
                    "received dfbit - %d",
                    ping_val->dfbit);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (ping_val->valid[UPLL_IDX_DF_BIT_PING] == UNC_VF_VALID_NO_VALUE) {
    ping_val->dfbit = UPLL_DF_BIT_ENABLE;
  }
  if (ping_val->valid[UPLL_IDX_PACKET_SIZE_PING] == UNC_VF_VALID) {
    if (!ValidateNumericRange(ping_val->packet_size,
                              (uint16_t) kMinPingPacketLen,
                              (uint16_t) kMaxPingPacketLen, true, true)) {
      UPLL_LOG_DEBUG("packet_size syntax check failed."
                    "received packet_size - %d",
                    ping_val->packet_size);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (ping_val->valid[UPLL_IDX_PACKET_SIZE_PING] == UNC_VF_VALID_NO_VALUE) {
    ping_val->packet_size = 0;
  }
  if (ping_val->valid[UPLL_IDX_COUNT_PING] == UNC_VF_VALID) {
    if (!ValidateNumericRange(ping_val->count, (uint32_t) kMinPingCount,
                              (uint32_t) kMaxPingCount, true, true)) {
      UPLL_LOG_DEBUG("count syntax check failed."
                    "received count - %d",
                    ping_val->count);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (ping_val->valid[UPLL_IDX_COUNT_PING] == UNC_VF_VALID_NO_VALUE) {
    ping_val->count = 0;
  }
  if (ping_val->valid[UPLL_IDX_INTERVAL_PING] == UNC_VF_VALID) {
    if (!ValidateNumericRange(ping_val->interval, (uint8_t) kMinPingInterval,
                              (uint8_t) kMaxPingInterval, true, true)) {
      UPLL_LOG_DEBUG("Interval syntax check failed."
                    "received interval - %d",
                    ping_val->interval);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (ping_val->valid[UPLL_IDX_INTERVAL_PING] == UNC_VF_VALID_NO_VALUE) {
    ping_val->interval = 0;
  }
  if (ping_val->valid[UPLL_IDX_TIMEOUT_PING] == UNC_VF_VALID) {
    if (!ValidateNumericRange(ping_val->timeout, (uint8_t) kMinPingTimeout,
                              (uint8_t) kMaxPingTimeout, true, true)) {
      UPLL_LOG_DEBUG("Timeout syntax check failed."
                    "received timeout - %d",
                    ping_val->timeout);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (ping_val->valid[UPLL_IDX_TIMEOUT_PING] == UNC_VF_VALID_NO_VALUE) {
    ping_val->timeout = 0;
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VbrMoMgr::ValidateVbrRenameValue(val_rename_vbr *vbr_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (vbr_rename->valid[UPLL_IDX_NEW_NAME_RVBR] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(vbr_rename->new_name),
                          kMinLenVnodeName,
                          kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Rename syntax check failed."
                    "Received new_name - %s",
                    vbr_rename->new_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (!req || !ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVbr) {
    UPLL_LOG_DEBUG("Invalid key structure received. received struct - %d",
                  (ikey->get_st_num()));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  unc_key_type_t Ktype = ikey->get_key_type();
  if (UNC_KT_VBRIDGE != Ktype) {
    UPLL_LOG_DEBUG("Invalid keytype received. Received keytype - %d", Ktype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vbr *vbr_key = reinterpret_cast<key_vbr *> (ikey->get_key());
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  ret_val = ValidateVbrKey(vbr_key, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("syntax check failed for key_vbr struct");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }

  if ((operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) && 
      (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_IMPORT)) {
      ConfigVal *cfg_val = ikey->get_cfg_val();
      if (cfg_val == NULL) {
        UPLL_LOG_DEBUG("ConfigVal struct is empty");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      if (cfg_val->get_st_num() != IpctSt::kIpcStValVbr) {
        UPLL_LOG_DEBUG(
            "Invalid val structure received.received struct - %d",
            cfg_val->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      val_vbr *vbr_val =
          reinterpret_cast<val_vbr *>(ikey->get_cfg_val()->get_val());
      if (vbr_val == NULL) {
        UPLL_LOG_DEBUG("val struct is mandatory for create and update op");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      ret_val = ValidateVbrValue(vbr_val, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failed for val_vbr structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_RENAME) && (dt_type == UPLL_DT_IMPORT)) {
      if (option1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("Error option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      ConfigVal *cfg_val = ikey->get_cfg_val();
      if (!cfg_val) return UPLL_RC_ERR_CFG_SYNTAX;
      if (cfg_val->get_st_num() != IpctSt::kIpcStValRenameVbr) {
        UPLL_LOG_DEBUG(
            "Invalid val_rename structure received.received struct - %d",
            cfg_val->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      val_rename_vbr *vbr_rename =
          reinterpret_cast<val_rename_vbr *>(cfg_val->get_val());
      if (vbr_rename == NULL) {
        UPLL_LOG_DEBUG(
          "syntax check for val_rename_vbr struct is Mandatory for Rename op");
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      ret_val = ValidateVbrRenameValue(vbr_rename);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failed for val_rename_vbr structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING
        || operation == UNC_OP_READ_SIBLING_BEGIN) && (dt_type == UPLL_DT_IMPORT)) { 
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG("Error option1 is not NORMAL");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG("Error option2 is not NONE");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    ConfigVal *cfg_val = ikey->get_cfg_val();
    if (!cfg_val) return UPLL_RC_SUCCESS;
    if (cfg_val->get_st_num() != IpctSt::kIpcStValRenameVbr) {
      UPLL_LOG_DEBUG(
          "Invalid val_rename structure received.received struct - %d",
          cfg_val->get_st_num());
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    val_rename_vbr *vbr_rename =
          reinterpret_cast<val_rename_vbr *>(cfg_val->get_val());
    if (vbr_rename == NULL) {
      UPLL_LOG_DEBUG("syntax check for val_rename_vbr struct is optional");
      return UPLL_RC_SUCCESS;
    }
    ret_val = ValidateVbrRenameValue(vbr_rename);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("syntax check failed for val_rename_vbr structure");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING
             || operation == UNC_OP_READ_SIBLING_BEGIN) &&
         (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
         dt_type == UPLL_DT_STARTUP)) {
      if (option1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("Error option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      ConfigVal *cfg_val = ikey->get_cfg_val();
      if (!cfg_val) return UPLL_RC_SUCCESS;
      if (cfg_val->get_st_num() != IpctSt::kIpcStValVbr) {
        UPLL_LOG_DEBUG("value structure matching is invalid. st.num - %d",
                      cfg_val->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      val_vbr *vbr_val =
          reinterpret_cast<val_vbr *>(ikey->get_cfg_val()->get_val());
      if (vbr_val == NULL) {
        UPLL_LOG_DEBUG("syntax check for val vbr struct is an optional");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVbrValue(vbr_val, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failure for val vbr structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ_SIBLING_COUNT) && 
            (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
             dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE)) {

      if (option1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("Error option2 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      ConfigVal *cfg_val = ikey->get_cfg_val();
      if (!cfg_val) return UPLL_RC_SUCCESS;
      if (cfg_val->get_st_num() != IpctSt::kIpcStValVbr) {
        UPLL_LOG_DEBUG("value structure matching is invalid. st.num - %d",
                      cfg_val->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      val_vbr *vbr_val =
          reinterpret_cast<val_vbr *>(ikey->get_cfg_val()->get_val());
      if (vbr_val == NULL) {
        UPLL_LOG_DEBUG("syntax check for val vbr struct is an optional");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVbrValue(vbr_val, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failure for val vbr structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING
             || operation == UNC_OP_READ_SIBLING_BEGIN) &&
            (dt_type == UPLL_DT_STATE)) {
      if ((option1 != UNC_OPT1_NORMAL) &&
          (option1 != UNC_OPT1_COUNT)) {
        UPLL_LOG_DEBUG("Error option1 is Invalid");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if ((option2 != UNC_OPT2_MAC_ENTRY) &&
          (option2 != UNC_OPT2_MAC_ENTRY_STATIC) &&
          (option2 != UNC_OPT2_MAC_ENTRY_DYNAMIC) &&
          (option2 != UNC_OPT2_L2DOMAIN) &&
          (option2 != UNC_OPT2_NONE)) {
        UPLL_LOG_DEBUG("Error option2 is Invalid");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      UPLL_LOG_DEBUG("val struct validation is an optional");
      return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_DELETE || operation == UNC_OP_READ_NEXT
      || operation == UNC_OP_READ_BULK) &&
      (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
        || dt_type == UPLL_DT_IMPORT || dt_type == UPLL_DT_STARTUP)) {
      UPLL_LOG_DEBUG("Value structure is none for operation type:%d", operation);
      return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_CONTROL) &&
             (dt_type == UPLL_DT_RUNNING || dt_type == UPLL_DT_STATE)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG("Error option1 is not NORMAL");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 == UNC_OPT2_PING) {
      ConfigVal *cfg_val = ikey->get_cfg_val();
      if (!cfg_val) {
        UPLL_LOG_TRACE("ConfigVal struct is null");
        return UPLL_RC_SUCCESS;
      }
      if (cfg_val->get_st_num() != IpctSt::kIpcStValPing) {
        UPLL_LOG_DEBUG(
            "Invalid val_ping structure received.received struct - %d",
            cfg_val->get_st_num());
        return UPLL_RC_ERR_BAD_REQUEST;
      }
      val_ping *ping_val = reinterpret_cast<val_ping *>(GetVal(ikey));
      if (ping_val == NULL) {
        UPLL_LOG_TRACE("val_ping struct is missing");
        return UPLL_RC_ERR_GENERIC;
      }
      ret_val = ValidateVbrPingValue(ping_val);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("syntax check failure for val_ping structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
#if 0
        // TODO(owner): this option is not enabled in key_type.h
        // remove if 0 once its added
        } else if (option2 == UNC_OPT2_CLEAR_ARPAGENT) {
      UPLL_LOG_DEBUG("Value structure is none for operation type:%d", operation);
        return UPLL_RC_SUCCESS;
#endif
      } else {
        UPLL_LOG_DEBUG("Error option2 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
  }
  UPLL_LOG_DEBUG("Error Unsupported Datatype-(%d) or Operation-(%d)", dt_type,
                 operation);
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
}

upll_rc_t VbrMoMgr::ValVbrAttributeSupportCheck(
                                       val_vbr_t *vbr_val,
                                       const uint8_t *attrs,
                                       unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  if (vbr_val != NULL) {
    if ((vbr_val->valid[UPLL_IDX_DOMAIN_ID_VBR] == UNC_VF_VALID)
        || (vbr_val->valid[UPLL_IDX_DOMAIN_ID_VBR] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr::kCapDomainId] == 0) {
        vbr_val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Domain Id Attribute is not supported by ctrlr");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((vbr_val->valid[UPLL_IDX_DESC_VBR] == UNC_VF_VALID)
        || (vbr_val->valid[UPLL_IDX_DESC_VBR] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr::kCapDesc] == 0) {
        vbr_val->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("Desc. Attribute is not supported by ctrlr");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((vbr_val->valid[UPLL_IDX_HOST_ADDR_VBR] == UNC_VF_VALID)
        || (vbr_val->valid[UPLL_IDX_HOST_ADDR_VBR] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr::kCapHostAddr] == 0) {
        vbr_val->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG("host_addr attribute is not supported by ctrlr");
          UPLL_LOG_DEBUG("vbr_val->valid[UPLL_IDX_HOST_ADDR_VBR] is %d",
                    vbr_val->valid[UPLL_IDX_HOST_ADDR_VBR]);  
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((vbr_val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] == UNC_VF_VALID)
        || (vbr_val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr::kCapHostAddrPrefixlen] == 0) {
        vbr_val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_DEBUG(
              "host_addr_prefixlen attribute is not supported by ctrlr");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        } 
      }
    }
  } else {
    UPLL_LOG_DEBUG("Error val_vbr Struct is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;
  if (!ikey || !req ) {
      UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is Null");
      return ret_val;
  }

  if (!ctrlr_name) {
    ctrlr_name = reinterpret_cast<char*>((reinterpret_cast<key_user_data_t *>
                  (ikey->get_user_data()))->ctrlr_id);
    if (!ctrlr_name || !strlen(ctrlr_name)) {
      UPLL_LOG_DEBUG("Controller Name is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  bool result_code = false;
  uint32_t max_attrs = 0;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs = NULL;

  switch (req->operation) {
    case UNC_OP_CREATE:
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count, &max_attrs,
                                        &attrs);
      break;

    case UNC_OP_UPDATE:
      result_code = GetUpdateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_attrs, &attrs);
      break;
    case UNC_OP_READ:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING_COUNT:
      result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      break;
    default:
      UPLL_LOG_DEBUG("Invalid Operation Code - (%d)", req->operation);
      return UPLL_RC_ERR_GENERIC;
  }
  if (!result_code) {
    UPLL_LOG_DEBUG("key_type - %d is not supported by controller - %s",
                  ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  
  val_vbr *vbr_val = NULL;
  if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num() ==
       IpctSt::kIpcStValVbr)) {
    vbr_val =
          reinterpret_cast<val_vbr *>(ikey->get_cfg_val()->get_val());
  }
  if (vbr_val) {
    if (max_attrs > 0) {
      ret_val = ValVbrAttributeSupportCheck(vbr_val, attrs, req->operation);
      return ret_val;
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d", req->operation);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::CreateVnodeConfigKey(ConfigKeyVal *ikey,
                                         ConfigKeyVal *&okey) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || ikey->get_key() == NULL) {
    UPLL_LOG_DEBUG("Null Input param ikey");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr * temp_key_vbr = reinterpret_cast<key_vbr *>
      (ConfigKeyVal::Malloc(sizeof(key_vbr)));
  uuu::upll_strncpy(temp_key_vbr->vtn_key.vtn_name,
         reinterpret_cast<key_vbr*>(ikey->get_key())->vtn_key.vtn_name,
          (kMaxLenVtnName+1));
  uuu::upll_strncpy(temp_key_vbr->vbridge_name,
         reinterpret_cast<key_vbr*>(ikey->get_key())->vbridge_name,
         (kMaxLenVnodeName+1));

  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr
                                          ,temp_key_vbr, NULL);
  return UPLL_RC_SUCCESS;
}

bool VbrMoMgr::FilterAttributes(void *&val1,
                                void *val2,
                                bool copy_to_running,
                                unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  val_vbr_t *val_vbr1 = reinterpret_cast<val_vbr_t *>(val1);
  val_vbr_t *val_vbr2 = reinterpret_cast<val_vbr_t *>(val2);
  /* No need to configure description in controller. */
  val_vbr1->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  if (op == UNC_OP_UPDATE) {
    val_vbr1->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
    val_vbr1->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_INVALID;
    val_vbr2->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
    val_vbr2->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_INVALID;
  }
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool VbrMoMgr::CompareValidValue(void *&val1,
                                 void *val2,
                                 bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vbr_t *val_vbr1 = reinterpret_cast<val_vbr_t *>(val1);
  val_vbr_t *val_vbr2 = reinterpret_cast<val_vbr_t *>(val2);
  if (!val_vbr2) {
    UPLL_LOG_TRACE("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0;
       loop < sizeof(val_vbr1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vbr1->valid[loop]
        && UNC_VF_VALID == val_vbr2->valid[loop]) 
      val_vbr1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (copy_to_running) {
    if (UNC_VF_INVALID != val_vbr1->valid[UPLL_IDX_DESC_VBR]) {
      if ((UNC_VF_VALID == val_vbr1->valid[UPLL_IDX_DESC_VBR]) &&
          !strcmp(reinterpret_cast<char*>(val_vbr1->vbr_description),
                  reinterpret_cast<char*>(val_vbr2->vbr_description)))
        val_vbr1->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == val_vbr1->valid[UPLL_IDX_CONTROLLER_ID_VBR]
        && UNC_VF_VALID == val_vbr2->valid[UPLL_IDX_CONTROLLER_ID_VBR]) {
      if (!strcmp(reinterpret_cast<char*>(val_vbr1->controller_id),
                  reinterpret_cast<char*>(val_vbr2->controller_id)))
        val_vbr1->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == val_vbr1->valid[UPLL_IDX_DOMAIN_ID_VBR]
        && UNC_VF_VALID == val_vbr2->valid[UPLL_IDX_DOMAIN_ID_VBR]) {
      if (!strcmp(reinterpret_cast<char*>(val_vbr1->domain_id),
                  reinterpret_cast<char*>(val_vbr2->domain_id)))
        val_vbr1->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == val_vbr1->valid[UPLL_IDX_HOST_ADDR_VBR]
        && UNC_VF_VALID == val_vbr2->valid[UPLL_IDX_HOST_ADDR_VBR]) {
      if (!memcmp(&val_vbr1->host_addr, &val_vbr2->host_addr,
                  sizeof(val_vbr2->host_addr)))
        val_vbr1->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_INVALID;
    }
    if (UNC_VF_VALID == val_vbr1->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR]
        && UNC_VF_VALID == val_vbr2->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR]) {
      if (val_vbr1->host_addr_prefixlen == val_vbr2->host_addr_prefixlen)
        val_vbr1->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_INVALID;
    }
  }
  // Description is not send to Controller
  if (!copy_to_running)
    val_vbr1->valid[UPLL_IDX_DESC_VBR] = UNC_VF_INVALID;
  for (unsigned int loop = 0;
       loop < sizeof(val_vbr1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vbr1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vbr1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}

upll_rc_t VbrMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                                (GetMoManager(UNC_KT_VBR_IF)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid MoMgr");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->IsReferenced(ikey, dt_type, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code (%d)",
           result_code);
    return result_code;
  }
  UPLL_LOG_DEBUG("IsReferenced result code (%d)", result_code);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::ValidateVtnRename(ConfigKeyVal *org_vtn_ckv,
                                      ConfigKeyVal *rename_vtn_ckv,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t    result_code = UPLL_RC_SUCCESS;
  DbSubOp      dbop        = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  ConfigKeyVal *vbr_ckv    = NULL;

  if (!org_vtn_ckv || !org_vtn_ckv->get_key() ||
      !rename_vtn_ckv || !rename_vtn_ckv->get_key()) {
    UPLL_LOG_DEBUG("Input is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = GetChildConfigKey(vbr_ckv, org_vtn_ckv);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed");
    return result_code;
  }

  /* Gets All VBRIDGE ConfigKeyVall based on original vtn name */
  result_code = ReadConfigDB(vbr_ckv, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
      result_code = UPLL_RC_SUCCESS;

    DELETE_IF_NOT_NULL(vbr_ckv);
    return result_code;
  }

  uint8_t *vtn_rename = reinterpret_cast<key_vtn_t*>(
      rename_vtn_ckv->get_key())->vtn_name;
  ConfigKeyVal *travel = vbr_ckv;
  while (travel) {
    /* Verifies whether the same hostaddress is exist under the
     * new vtn name */
    uuu::upll_strncpy(reinterpret_cast<key_vbr_t*>(
                   travel->get_key())->vtn_key.vtn_name,
                   vtn_rename, (kMaxLenVtnName + 1));
    result_code = MergeValidateIpAddress(travel, UPLL_DT_IMPORT, dmi, NULL,
                                         UPLL_IMPORT_TYPE_FULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Vbridge host address validation failed "
                     "during VTN stiching");
      DELETE_IF_NOT_NULL(vbr_ckv);
      return result_code;
    }

    travel = travel->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(vbr_ckv);
  return UPLL_RC_SUCCESS;
}
#if 0
upll_rc_t VbrMoMgr::IsHostAddrAndPrefixLenInUse(ConfigKeyVal *ckv, DalDmlIntf *dmi, 
					     IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ckv_vbr = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  upll_keytype_datatype_t dt_type = req->datatype;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };

  result_code = GetChildConfigKey(ckv_vbr, NULL);
  if (!ckv_vbr || result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(ckv_vbr);
    return UPLL_RC_ERR_GENERIC;
  }
  val_vbr_t *vbr_val = static_cast<val_vbr_t *>
      (ConfigKeyVal::Malloc(sizeof(val_vbr_t)));
  key_vbr *vbrkey = reinterpret_cast<key_vbr*>(ckv->get_key());
  if (!strlen(reinterpret_cast<const char *>(vbrkey->vtn_key.vtn_name))) {
    free(vbr_val);
    delete(ckv_vbr);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(reinterpret_cast<key_vbr*>
                   (ckv_vbr->get_key())->vtn_key.vtn_name,
                    vbrkey->vtn_key.vtn_name,
                    kMaxLenVtnName+1);
  val_vbr_t *vbrval = reinterpret_cast<val_vbr *>(GetVal(ckv));

  if ((vbrval->valid[UPLL_IDX_HOST_ADDR_VBR] != UNC_VF_VALID) && 
  (vbrval->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] != UNC_VF_VALID)) {
    UPLL_LOG_DEBUG("Semantic check not required!");
    free(vbr_val);
    delete (ckv_vbr); 

    return UPLL_RC_SUCCESS;
  }

  vbr_val->host_addr = vbrval->host_addr;
  vbr_val->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_VALID;
  vbr_val->host_addr_prefixlen = vbrval->host_addr_prefixlen;
  vbr_val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] = UNC_VF_VALID;
  ckv_vbr->AppendCfgVal(IpctSt::kIpcStValVbr, vbr_val);

  UPLL_LOG_TRACE(" existence check %s",(ckv_vbr->ToStrAll()).c_str());

  result_code = UpdateConfigDB(ckv_vbr, dt_type , UNC_OP_READ,
                                          dmi, &dbop, MAINTBL);
  delete ckv_vbr;
  
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("More than one vbridge configured with the same "
                     "host address and prefix length!");
      return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("UpdateConfigDB Failed %d", result_code);
      return result_code;
  }
  return UPLL_RC_SUCCESS;
}
#endif
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
