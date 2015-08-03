/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vbr_momgr.hh"
#include "vlink_momgr.hh"
#include "upll_db_query.hh"
#include "unified_nw_momgr.hh"
#include "vbr_portmap_momgr.hh"
#define NUM_KEY_MAIN_TBL_  5
#define NUM_KEY_RENAME_TBL_ 4
#define NUM_KEY_CONV_VBR_MAIN_TBL 6

using unc::upll::ipc_util::IpcUtil;
using unc::upll::ipc_util::KtUtil;
using unc::upll::dal::DalResultCode;

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
                                       { uudst::vbridge::kDbiUnknownCount,
                                         ST_VAL,
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

BindInfo VbrMoMgr::conv_vbr_bind_info[] = {
         { uudst::convert_vbridge::kDbiVtnName, CFG_KEY,
           offsetof(key_convert_vbr, vbr_key.vtn_key.vtn_name),
           uud::kDalChar, kMaxLenVtnName + 1 },
         { uudst::convert_vbridge::kDbiVbrName, CFG_KEY,
           offsetof(key_convert_vbr, vbr_key.vbridge_name),
           uud::kDalChar, kMaxLenVnodeName + 1 },
         { uudst::convert_vbridge::kDbiConvertVbrName, CFG_KEY,
           offsetof(key_convert_vbr, conv_vbr_name),
           uud::kDalChar, kMaxLenConvertVnodeName + 1 },
         { uudst::convert_vbridge::kDbiLabel, CFG_VAL,
           offsetof(val_convert_vbr, label),
           uud::kDalUint32, 1 },
         { uudst::convert_vbridge::kDbiOperStatus, ST_VAL,
           offsetof(val_db_vbr_st,
                    vbr_val_st.oper_status),
           uud::kDalUint8, 1 },
         { uudst::convert_vbridge::kDbiDownCount, ST_VAL,
           offsetof(val_db_vbr_st, down_count),
           uud::kDalUint32, 1 },
         { uudst::convert_vbridge::kDbiUnknownCount,
           ST_VAL,
           offsetof(val_db_vbr_st, unknown_count),
           uud::kDalUint32, 1 },
         { uudst::convert_vbridge::kDbiCtrlrName, CK_VAL,
           offsetof(key_user_data, ctrlr_id),
           uud::kDalChar, kMaxLenCtrlrId + 1 },
         { uudst::convert_vbridge::kDbiDomainId, CK_VAL,
           offsetof(key_user_data, domain_id),
           uud::kDalChar, kMaxLenDomainId + 1 },
         { uudst::convert_vbridge::kDbiFlags, CK_VAL,
           offsetof(key_user_data_t, flags),
           uud::kDalUint8, 1 },
         { uudst::convert_vbridge::kDbiValidLabel,
           CFG_META_VAL, offsetof(val_convert_vbr,
                                  valid[0]),
           uud::kDalUint8, 1 },
         { uudst::convert_vbridge::kDbiValidOperStatus,
           ST_META_VAL, offsetof(val_vbr_st,
                                 valid[0]),
           uud::kDalUint8, 1 },
         { uudst::convert_vbridge::kDbiCsLabel,
           CS_VAL, offsetof(val_convert_vbr, cs_attr[0]),
           uud::kDalUint8, 1 },
         { uudst::convert_vbridge::kDbiCsRowStatus,
           CS_VAL, offsetof(val_convert_vbr,
                            cs_row_status),
           uud::kDalUint8, 1 }};

BindInfo VbrMoMgr::key_conv_vbr_converttbl_bind_info[] = {
    { uudst::convert_vbridge::kDbiVtnName, CFG_MATCH_KEY, offsetof(
        key_convert_vbr_t, vbr_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::convert_vbridge::kDbiVbrName, CFG_MATCH_KEY, offsetof(
        key_convert_vbr_t, vbr_key.vbridge_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::convert_vbridge::kDbiConvertVbrName, CFG_MATCH_KEY, offsetof(
        key_convert_vbr_t, conv_vbr_name),
      uud::kDalChar, kMaxLenConvertVnodeName + 1 },
    { uudst::convert_vbridge::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::convert_vbridge::kDbiVbrName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::convert_vbridge::kDbiFlags, CK_VAL, offsetof(
        key_user_data_t, flags),
      uud::kDalUint8, 1 } };

unc_key_type_t VbrMoMgr::vbr_child[] = { UNC_KT_VBR_VLANMAP,
                                         UNC_KT_VBR_NWMONITOR,
                                         UNC_KT_VBR_POLICINGMAP,
                                         UNC_KT_VBR_FLOWFILTER,
                                         UNC_KT_VBR_PORTMAP, UNC_KT_VBR_IF };



VbrMoMgr::VbrMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVbrTbl, UNC_KT_VBRIDGE, vbr_bind_info,
                         IpctSt::kIpcStKeyVbr, IpctSt::kIpcStValVbr,
                         (uudst::vbridge::kDbiVbrNumCols+2));
  table[RENAMETBL] = new Table(uudst::kDbiVNodeRenameTbl, UNC_KT_VBRIDGE,
                  vbr_rename_bind_info, IpctSt::kIpcInvalidStNum,
                  IpctSt::kIpcInvalidStNum,
                  uudst::vnode_rename::kDbiVnodeRenameNumCols);
  table[CONVERTTBL] = new Table(uudst::kDbiConvertVbrTbl, UNC_KT_VBRIDGE,
                  conv_vbr_bind_info, IpctSt::kIpcStKeyConvertVbr,
                  IpctSt::kIpcInvalidStNum,
                  (uudst::convert_vbridge::kDbiConvertVbrNumCols+3));
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
  } else if (CONVERTTBL == tbl) {
    nattr = NUM_KEY_CONV_VBR_MAIN_TBL;
    binfo = key_conv_vbr_converttbl_bind_info;
  } else {
      UPLL_LOG_TRACE("Invalid table");
      return false;
  }
  return true;
}


bool VbrMoMgr::IsValidKey(void *key,
                          uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (tbl == CONVERTTBL) {
    key_convert_vbr_t *con_vbr_key = reinterpret_cast<key_convert_vbr_t *>(key);
    switch (index) {
      case uudst::convert_vbridge::kDbiVtnName:
         ret_val = ValidateKey(
              reinterpret_cast<char *>(con_vbr_key->vbr_key.vtn_key.vtn_name),
              kMinLenVtnName, kMaxLenVtnName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::convert_vbridge::kDbiVbrName:
        ret_val = ValidateKey(
                  reinterpret_cast<char *>(con_vbr_key->vbr_key.vbridge_name),
                  kMinLenVnodeName, kMaxLenVnodeName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VBR Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::convert_vbridge::kDbiConvertVbrName:
        ret_val = ValidateKey(
                  reinterpret_cast<char *>(con_vbr_key->conv_vbr_name),
                  kMinLenConvertVnodeName, kMaxLenConvertVnodeName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Convert VBR Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      default:
        UPLL_LOG_TRACE("Invalid Key Index");
        return false;
    }
  } else {
    key_vbr *vbr_key = reinterpret_cast<key_vbr *>(key);
    switch (index) {
      case uudst::vbridge::kDbiVtnName:
      case uudst::vnode_rename::kDbiUncVtnName:
         ret_val = ValidateKey(
              reinterpret_cast<char *>(vbr_key->vtn_key.vtn_name),
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
    }
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
      okey->SetKey(IpctSt::kIpcStKeyVbr, vbr_key);
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
      if (parent_key->get_st_num() == IpctSt::kIpcStKeyConvertVbr) {
        uuu::upll_strncpy(vbr_key->vbridge_name,
           reinterpret_cast<key_convert_vbr *>(pkey)->vbr_key.vbridge_name,
           (kMaxLenVnodeName+1));
        uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
           reinterpret_cast<key_convert_vbr *>(pkey)->vbr_key.vtn_key.vtn_name,
           (kMaxLenVtnName+1));
      } else {
        uuu::upll_strncpy(vbr_key->vbridge_name, reinterpret_cast<key_vbr *>
                       (pkey)->vbridge_name, (kMaxLenVnodeName+1));
        uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
             reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      }
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
      UPLL_LOG_DEBUG("Vlink flag node position %d", flags);
      if (flags == kVlinkVnode2) {
        vnode_name = vlink_val->vnode2_name;
      } else {
        vnode_name = vlink_val->vnode1_name;
      }
      uuu::upll_strncpy(vbr_key->vbridge_name, vnode_name,
                          (kMaxLenVnodeName + 1));
      uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                        reinterpret_cast<key_vlink *>(
                        parent_key->get_key())->vtn_key.vtn_name ,
                        (kMaxLenVtnName+1));
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
    okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                            vbr_key, NULL);
  else if (okey->get_key() != vbr_key)
    okey->SetKey(IpctSt::kIpcStKeyVbr, vbr_key);
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
  if (ikey->get_st_num() == IpctSt::kIpcStKeyConvertVbr)
    uuu::upll_strncpy(vtn_key->vtn_name,
       reinterpret_cast<key_convert_vbr_t *>(pkey)->vbr_key.vtn_key.vtn_name,
       (kMaxLenVtnName+1));
  else
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

upll_rc_t VbrMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                                         AdaptType adapt_type) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbr *vbr_key = reinterpret_cast<key_vbr*>(ikey->get_key());
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    while (cval) {
      controller_domain ctrlr_dom = {NULL, NULL};
      GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
      if (IpctSt::kIpcStValVbrSt == cval->get_st_num()) {
        if (ctrlr_dom.ctrlr && !IsUnifiedVbr(ctrlr_dom.ctrlr)) {
          CheckOperStatus<val_vbr_st>(vbr_key->vtn_key.vtn_name,
                                      cval, UNC_KT_VBRIDGE, ctrlr_dom);
        }
      } else if (IpctSt::kIpcStValVbr == cval->get_st_num()) {
        if (cval->get_val()) {
          val_vbr_t *val = reinterpret_cast<val_vbr *>(cval->get_val());
          if ((val->valid[UPLL_IDX_LABEL_VBR] == UNC_VF_VALID) ||
              (ctrlr_dom.ctrlr && IsUnifiedVbr(ctrlr_dom.ctrlr))) {
            for (unsigned int loop = 0;
                 loop < sizeof(val->valid) / sizeof(val->valid[0]); ++loop) {
              if (loop != UPLL_IDX_DESC_VBR) {
                val->valid[loop] = UNC_VF_INVALID;
              }
            }
            StringReset(val->controller_id);
            StringReset(val->domain_id);
            val->host_addr.s_addr = 0;
            val->host_addr_prefixlen = 0;
            val->label = 0;
          }
        }
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
    case CONVERTTBL:
      val = reinterpret_cast<void *>
          (ConfigKeyVal::Malloc(sizeof(val_convert_vbr)));
      ck_val = new ConfigVal(IpctSt::kIpcStValConvertVbr, val);
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_db_vbr_st)));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVbrSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
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
    } else if (tbl == CONVERTTBL) {
      val_convert_vbr_t *ival =
                      reinterpret_cast<val_convert_vbr_t *>(GetVal(req));
      if (ival == NULL) {
        UPLL_LOG_DEBUG("Null Val structure");
        return UPLL_RC_ERR_GENERIC;
      }
      val_convert_vbr_t *vbr_val = reinterpret_cast<val_convert_vbr_t *>
                             (ConfigKeyVal::Malloc(sizeof(val_convert_vbr)));
      memcpy(vbr_val, ival, sizeof(val_convert_vbr_t));
      tmp1 = new ConfigVal(IpctSt::kIpcStValConvertVbr, vbr_val);
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
    if (tbl == MAINTBL || tbl == CONVERTTBL) {
      if (tmp->get_st_num() == IpctSt::kIpcStValVbrSt) {
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
      } else if (tmp->get_st_num() == IpctSt::kIpcStValVbr) {
        val_vbr_t *ival = reinterpret_cast<val_vbr_t *>(tmp->get_val());
        if (ival == NULL) {
          UPLL_LOG_DEBUG("Null Val structure");
          delete tmp1;
          return UPLL_RC_ERR_GENERIC;
        }
        val_vbr_t *val_vbr = reinterpret_cast<val_vbr_t *>
            (ConfigKeyVal::Malloc(sizeof(val_vbr_t)));
        memcpy(val_vbr, ival, sizeof(val_vbr_t));
        ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVbr, val_vbr);
        tmp1->AppendCfgVal(tmp2);
      }
    }
  };
  void *tkey = (req)->get_key();
  if (!tkey) {
    UPLL_LOG_DEBUG("Null tkey");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
//  cout << "VbrMoMgr::DupConfigKeyVal";
  if (tbl == CONVERTTBL) {
    key_convert_vbr_t *ikey = reinterpret_cast<key_convert_vbr_t *>(tkey);
    key_convert_vbr_t *vbr_key = reinterpret_cast<key_convert_vbr_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_convert_vbr_t)));
    memcpy(vbr_key, ikey, sizeof(key_convert_vbr_t));
    okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyConvertVbr,
                            vbr_key, tmp1);
    if (!okey) {
      delete tmp1;
      free(vbr_key);
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    key_vbr *ikey = reinterpret_cast<key_vbr *>(tkey);
    key_vbr *vbr_key = reinterpret_cast<key_vbr *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr)));
    memcpy(vbr_key, ikey, sizeof(key_vbr));
    okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, vbr_key,
                            tmp1);
    if (!okey) {
      delete tmp1;
      free(vbr_key);
      return UPLL_RC_ERR_GENERIC;
    }
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
  upll_rc_t ret_val = ValidateKey(
      reinterpret_cast<char *>(ctrlr_key->vtn_key.vtn_name),
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
  } else {
    dbop.matchop = kOpMatchNone;
  }
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_vnode);

    uint8_t rename  = 0;
//  UPLL_LOG_TRACE("Before Read from Rename Table %s",
//  (unc_key->ToStrAll()).c_str());
  dbop.inoutop = kOpInOutCtrlr | kOpInOutDomain;
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
    MoMgrImpl *vtn_mgr =
        reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
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
      UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
      return result_code;
    }
#endif
    result_code = IsRenamed(ikey, dt_type, dmi, rename);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning %d", result_code);
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

  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_DEBUG("Returning error %d", result_code);
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
    MoMgrImpl *vtn_mgr =
        reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
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

upll_rc_t VbrMoMgr::UpdateUvbrConfigStatus(ConfigKeyVal *vbr_key,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  val_vbr_t *vbr_val =  reinterpret_cast<val_vbr_t *>(GetVal(vbr_key));
  if (!vbr_val) {
    UPLL_LOG_DEBUG("invalid val ");
    return UPLL_RC_ERR_GENERIC;
  }

  if (op == UNC_OP_CREATE) {
    vbr_val->cs_row_status = UNC_CS_APPLIED;
    val_db_vbr_st *val_vbrst = reinterpret_cast<val_db_vbr_st *>
        (ConfigKeyVal::Malloc(sizeof(val_db_vbr_st)));
    val_vbrst->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
    val_vbrst->down_count  = 0;
    val_vbrst->unknown_count  = 0;
    val_vbrst->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
    vbr_key->AppendCfgVal(IpctSt::kIpcStValVbrSt, val_vbrst);
  } else if (op == UNC_OP_UPDATE) {
    vbr_val->cs_row_status = reinterpret_cast<val_vbr_t *>(
        GetVal(upd_key))->cs_row_status;
    if (UNC_VF_INVALID == vbr_val->valid[UPLL_IDX_DESC_VBR] && UNC_VF_VALID ==
        reinterpret_cast<val_vbr_t*>(GetVal(upd_key))->valid[UPLL_IDX_DESC_VBR])
      vbr_val->valid[UPLL_IDX_DESC_VBR] = UNC_VF_VALID_NO_VALUE;
  }
  if ((op != UNC_OP_DELETE) &&
      (vbr_val->valid[UPLL_IDX_DESC_VBR] != UNC_VF_INVALID)) {
    vbr_val->cs_attr[UPLL_IDX_DESC_VBR] = UNC_CS_APPLIED;
  }
  return UPLL_RC_SUCCESS;
}
upll_rc_t VbrMoMgr::UpdateConvVbrConfigStatus(ConfigKeyVal *vbr_key,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  val_convert_vbr_t *vbr_val =
      reinterpret_cast<val_convert_vbr_t *>(GetVal(vbr_key));
  if (!vbr_val) {
    UPLL_LOG_DEBUG("invalid val ");
    return UPLL_RC_ERR_GENERIC;
  }
  if (op == UNC_OP_CREATE) {
    val_db_vbr_st *val_vbrst = reinterpret_cast<val_db_vbr_st *>
        (ConfigKeyVal::Malloc(sizeof(val_db_vbr_st)));
    val_vbrst->vbr_val_st.oper_status = driver_result == UPLL_RC_SUCCESS?
                            UPLL_OPER_STATUS_UP:
                            UPLL_OPER_STATUS_UNKNOWN;
    if (driver_result != UPLL_RC_SUCCESS) {
      uint8_t *ctrlr = NULL;
      GET_USER_DATA_CTRLR(vbr_key, ctrlr);
      if (ctrlr)
        uuc::CtrlrMgr::GetInstance()->
            AddtoUnknownCtrlrList(reinterpret_cast<char*>(ctrlr));
    }
    val_vbrst->down_count  = 0;
    val_vbrst->unknown_count  = 0;
    val_vbrst->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
    vbr_key->AppendCfgVal(IpctSt::kIpcStValVbrSt, val_vbrst);
  }
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

  if (vbr_key->get_st_num() == IpctSt::kIpcStKeyConvertVbr) {
    return UpdateConvVbrConfigStatus(vbr_key, op, driver_result, upd_key, dmi);
  } else {
    if (vbr_val->valid[UPLL_IDX_CONTROLLER_ID_VBR] == UNC_VF_VALID) {
      if (IsUnifiedVbr(vbr_val->controller_id)) {
        return UpdateUvbrConfigStatus(vbr_key, op, driver_result, upd_key, dmi);
      }
    }
  }

  if (op == UNC_OP_CREATE) {
    vbr_val->cs_row_status = cs_status;
    val_db_vbr_st *val_vbrst = reinterpret_cast<val_db_vbr_st *>
        (ConfigKeyVal::Malloc(sizeof(val_db_vbr_st)));
    if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
      val_vbrst->vbr_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
      uint8_t *ctrlr = NULL;
      GET_USER_DATA_CTRLR(vbr_key, ctrlr);
      if (ctrlr)
        uuc::CtrlrMgr::GetInstance()->
            AddtoUnknownCtrlrList(reinterpret_cast<char*>(ctrlr));
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
  UPLL_LOG_DEBUG("ctrlr_dom %s %s", ctrlr_dom->ctrlr, ctrlr_dom->domain);
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
  if (ckv_running->get_st_num() == IpctSt::kIpcStKeyConvertVbr) {
    return UPLL_RC_SUCCESS;
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
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain };
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
    controller_domain ctrlr_dom;
    ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(travel, ctrlr_dom);
    if (ctrlr_dom.ctrlr && IsUnifiedVbr(ctrlr_dom.ctrlr)) {
      result_code = UnifiedVbrVnodeChecks(tkey, UPLL_DT_CANDIDATE,
                                          ctrlr_id, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(tkey);
        if (dup_key) delete dup_key;
        return result_code;
      }
    } else {
      /* Same Name should not present in the vnodes in running*/
      if (import_type == UPLL_IMPORT_TYPE_FULL) {
        result_code = VnodeChecks(tkey, UPLL_DT_CANDIDATE, dmi, false);
        if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code ||
            UPLL_RC_ERR_CFG_SEMANTIC == result_code) {
          ikey->ResetWith(tkey);
          DELETE_IF_NOT_NULL(tkey);
          if (dup_key) delete dup_key;
          UPLL_LOG_DEBUG("VBridge Name Conflict %s",
                         (ikey->ToStrAll()).c_str());
          return UPLL_RC_ERR_MERGE_CONFLICT;
        }
      } else {
        result_code = PartialImport_VnodeChecks(tkey,
                                                UPLL_DT_CANDIDATE,
                                                ctrlr_id , dmi);
        if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code ||
            UPLL_RC_ERR_CFG_SEMANTIC == result_code) {
          ikey->ResetWith(tkey);
          DELETE_IF_NOT_NULL(tkey);
          if (dup_key) delete dup_key;
          UPLL_LOG_DEBUG("VBridge Name Conflict %s",
                         (ikey->ToStrAll()).c_str());
          return UPLL_RC_ERR_MERGE_CONFLICT;
        }
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
  string vtn_id = "";

  key_rename_vnode_info *vbr_rename_info =
                    reinterpret_cast<key_rename_vnode_info *>
                    (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info)));

  vbr_key = reinterpret_cast<key_vbr_t *>(ikey->get_key());
  if (!vbr_key) {
    free(vbr_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  if (renamed) {
    if (!reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey)) ||
        !(strlen(reinterpret_cast<char *>
               (reinterpret_cast<val_rename_vnode_t *>
               (GetVal(ikey))->ctrlr_vnode_name)))) {
      free(vbr_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vbr_rename_info->ctrlr_vnode_name,
        reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey))->ctrlr_vnode_name,
         (kMaxLenVnodeName+1));
    if (!reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey)) ||
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
    if (UPLL_RC_SUCCESS != result_code || tmp_key == NULL) {
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
       UPLL_LOG_INFO("Returning error %d\n", result_code);
       delete tmp_key;
       return result_code;
    }
    ConfigKeyVal *conv_vbr_ckv = NULL;
    if (ctrlr_dom.ctrlr && IsUnifiedVbr(ctrlr_dom.ctrlr)) {
      result_code = GetChildConvertConfigKey(conv_vbr_ckv, tmp_key);
      if (result_code != UPLL_RC_SUCCESS) {
        delete tmp_key;
        return result_code;
      }

      DbSubOp dbop1 = {kOpReadSingle, kOpMatchNone,
        kOpInOutCtrlr | kOpInOutDomain};
      result_code = ReadConfigDB(conv_vbr_ckv, UPLL_DT_IMPORT, UNC_OP_READ,
                                 dbop1, dmi, CONVERTTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        delete tmp_key;
        DELETE_IF_NOT_NULL(conv_vbr_ckv);
        return result_code;
      }
      GET_USER_DATA_CTRLR_DOMAIN(conv_vbr_ckv, ctrlr_dom);
      SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    } else {
      SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    }
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
                                 &dbop, TC_CONFIG_GLOBAL, vtn_id, RENAMETBL);
    delete tmp_key;
    DELETE_IF_NOT_NULL(conv_vbr_ckv);
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
      switch (valid_index) {
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
  if (operation == UNC_OP_CREATE) {
    if (((vbr_val->valid[UPLL_IDX_CONTROLLER_ID_VBR] == UNC_VF_VALID) &&
        (vbr_val->valid[UPLL_IDX_DOMAIN_ID_VBR] != UNC_VF_VALID)) ||
        ((vbr_val->valid[UPLL_IDX_DOMAIN_ID_VBR] == UNC_VF_VALID) &&
        (vbr_val->valid[UPLL_IDX_CONTROLLER_ID_VBR] != UNC_VF_VALID))) {
      UPLL_LOG_DEBUG("Received INVALID ctrlr and domain id");
      return UPLL_RC_ERR_CFG_SYNTAX;
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
          switch (valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VBR:
              if ((flag == UNC_VF_INVALID || flag == UNC_VF_VALID_NO_VALUE)) {
                UPLL_LOG_DEBUG("ctrlr_id flag is invalid or valid_no_value");
                // Set ctrlr_id to "#" for Unified vBridge.
                vbr_val->valid[valid_index] = UNC_VF_VALID;
                uuu::upll_strncpy(reinterpret_cast<char *>
                  (vbr_val->controller_id), "#", (kMaxLenCtrlrId+1));
              }
              break;
            case UPLL_IDX_DOMAIN_ID_VBR:
              if ((flag == UNC_VF_INVALID || flag == UNC_VF_VALID_NO_VALUE)) {
                UPLL_LOG_DEBUG("domain_id flag is invalid or valid_no_value");
                // Set ctrlr_id to "#" for Unified vBridge.
                vbr_val->valid[valid_index] = UNC_VF_VALID;
                uuu::upll_strncpy(reinterpret_cast<char *>
                  (vbr_val->domain_id), "#", (kMaxLenCtrlrId+1));
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
          switch (valid_index) {
            case UPLL_IDX_CONTROLLER_ID_VBR:
            case UPLL_IDX_DOMAIN_ID_VBR:
              if (flag == UNC_VF_VALID_NO_VALUE) {
                UPLL_LOG_DEBUG(
                    "controller_id or domain_id flag is valid_no_value");
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
              if (vbr_val->valid[UPLL_IDX_HOST_ADDR_VBR] !=
                  vbr_val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR]) {
                UPLL_LOG_DEBUG("Host address and prefix length"
                    "both do not have same valid flags: %d, %d",
                    vbr_val->valid[UPLL_IDX_HOST_ADDR_VBR],
                    vbr_val->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR]);
                return UPLL_RC_ERR_CFG_SYNTAX;
             }
              break;
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

    switch (valid_index) {
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
      case UPLL_IDX_LABEL_VBR:
        vbr_val->label = 0;
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
  } else if (ping_val->valid[UPLL_IDX_PACKET_SIZE_PING] ==
             UNC_VF_VALID_NO_VALUE) {
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
// To validate option1 and option2 for Normal vbridge and Unified vbridge
// during DT_STATE operations
upll_rc_t VbrMoMgr::ValidateVbrRead(IpcReqRespHeader *req,
                                    ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  if (!req || !ikey || !dmi) {
    UPLL_LOG_ERROR("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  if ((dt_type == UPLL_DT_STATE) || (operation == UNC_OP_CONTROL)) {
    ConfigKeyVal *okey = NULL;
    ret_val = GetChildConfigKey(okey, ikey);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", ret_val);
      return ret_val;
    }

    //  Get controller and domain for requested vBridge.
    //  To find received vBridge is unified vBridge or normal vBridge
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
    ret_val = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi, MAINTBL);
    if (ret_val != UPLL_RC_SUCCESS) {
      if (ret_val == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_INFO("Requested vBridge is not exist in running DB");
      } else {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", ret_val);
      }
      DELETE_IF_NOT_NULL(okey);
      return ret_val;
    }

    if (!(okey->get_cfg_val()) || !GetVal(okey)) {
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    val_vbr_t *vbr_val = reinterpret_cast<val_vbr_t *>(GetVal(okey));
    if (vbr_val->valid[UPLL_IDX_CONTROLLER_ID_VBR] == UNC_VF_VALID) {
      if (IsUnifiedVbr(vbr_val->controller_id)) {
        DELETE_IF_NOT_NULL(okey);
        if (operation == UNC_OP_CONTROL) {
          UPLL_LOG_INFO("Control operation is not supported for "
                        "unified vBridge");
          return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
        }
        if ((option1 != UNC_OPT1_NORMAL) && (option1 != UNC_OPT1_DETAIL)) {
          UPLL_LOG_INFO("Invalid option1 for unified vBridge");
          return UPLL_RC_ERR_INVALID_OPTION1;
        }
        if ((option2 != UNC_OPT2_EXPAND) && (option2 != UNC_OPT2_NONE)) {
          UPLL_LOG_INFO("Invalid option2 for unified vBridge");
          return UPLL_RC_ERR_INVALID_OPTION2;
        }
      } else {
        if (option2 == UNC_OPT2_EXPAND) {
          DELETE_IF_NOT_NULL(okey);
          UPLL_LOG_INFO("Expand option is not supported for normal vBridge");
          return UPLL_RC_ERR_INVALID_OPTION2;
        }
      }
    }
    DELETE_IF_NOT_NULL(okey);
  }
  return ret_val;
}

upll_rc_t VbrMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  // To store ctrlr and domain as "#" in case of Unified vBridge
  val_vbr *vbr_val = NULL;
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
      // If val structure is NULL, then declare a val structure
      // and initialise its ctrlr_id and domain_id with "#"
      ConfigVal *cfg_val = ikey->get_cfg_val();
      if ((cfg_val == NULL) || !(GetVal(ikey))) {
        if (dt_type == UPLL_DT_CANDIDATE) {
          vbr_val = reinterpret_cast<val_vbr *>
                                (ConfigKeyVal::Malloc(sizeof(val_vbr)));
          uuu::upll_strncpy(reinterpret_cast<char *>
                (vbr_val->controller_id), "#", (kMaxLenCtrlrId+1));
          vbr_val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
          uuu::upll_strncpy(reinterpret_cast<char *>
                (vbr_val->domain_id), "#", (kMaxLenCtrlrId+1));
          vbr_val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;
          for (unsigned int valid_index = 0;
               valid_index < sizeof(vbr_val->valid) / sizeof(vbr_val->valid[0]);
               valid_index++) {
            if ((valid_index == UPLL_IDX_CONTROLLER_ID_VBR) ||
                (valid_index == UPLL_IDX_DOMAIN_ID_VBR))
              continue;
            vbr_val->valid[valid_index] = UNC_VF_INVALID;
          }
          StringReset(vbr_val->vbr_description);
          vbr_val->host_addr.s_addr = 0;
          vbr_val->host_addr_prefixlen = 0;
          vbr_val->label = 0;
          ikey->AppendCfgVal(IpctSt::kIpcStValVbr, vbr_val);
        } else {
          UPLL_LOG_DEBUG("vBridge val structure is NULL during import");
          return UPLL_RC_ERR_BAD_REQUEST;
        }
      } else {
        if (cfg_val->get_st_num() != IpctSt::kIpcStValVbr) {
          UPLL_LOG_DEBUG(
              "Invalid val structure received.received struct - %d",
              cfg_val->get_st_num());
          return UPLL_RC_ERR_BAD_REQUEST;
        }
        vbr_val =
            reinterpret_cast<val_vbr *>(ikey->get_cfg_val()->get_val());
        ret_val = ValidateVbrValue(vbr_val, operation);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("syntax check failed for val_vbr structure");
          return UPLL_RC_ERR_CFG_SYNTAX;
        }
        if ((dt_type == UPLL_DT_CANDIDATE) &&
            vbr_val->valid[UPLL_IDX_LABEL_VBR] != UNC_VF_INVALID) {
          return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
        }
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
        || operation == UNC_OP_READ_SIBLING_BEGIN) &&
             (dt_type == UPLL_DT_IMPORT)) {
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
          (option2 != UNC_OPT2_L2DOMAIN) &&
          (option2 != UNC_OPT2_EXPAND) &&
          (option2 != UNC_OPT2_NONE)) {
        UPLL_LOG_DEBUG("Error option2 is Invalid");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (option2 == UNC_OPT2_EXPAND) {
        if (operation != UNC_OP_READ) {
          UPLL_LOG_DEBUG("Option2 is Invalid for Expand read");
          return UPLL_RC_ERR_INVALID_OPTION2;
        } else if (option1 != UNC_OPT1_NORMAL) {
          UPLL_LOG_DEBUG("Option1 is Invalid for Expand read");
          return UPLL_RC_ERR_INVALID_OPTION1;
        }
      }
      UPLL_LOG_DEBUG("val struct validation is an optional");
      return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_DELETE || operation == UNC_OP_READ_NEXT
      || operation == UNC_OP_READ_BULK) &&
      (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
        || dt_type == UPLL_DT_IMPORT || dt_type == UPLL_DT_STARTUP)) {
      UPLL_LOG_DEBUG("Value structure is none for operation type:%d",
                     operation);
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
      UPLL_LOG_DEBUG("Value structure is none for operation type:%d",
                     operation);
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
  if (!ikey || !req) {
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
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                     req->operation);
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
                                          , temp_key_vbr, NULL);
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
    if (UNC_VF_VALID == val_vbr1->valid[UPLL_IDX_LABEL_VBR]
        && UNC_VF_VALID == val_vbr2->valid[UPLL_IDX_LABEL_VBR]) {
      if (val_vbr1->label == val_vbr2->label)
        val_vbr1->valid[UPLL_IDX_LABEL_VBR] = UNC_VF_INVALID;
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

upll_rc_t VbrMoMgr::IsReferenced(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                                (GetMoManager(UNC_KT_VBR_IF)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid MoMgr");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->IsReferenced(req, ikey, dmi);
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
// To perform create or delete of a Convertd vBridge
upll_rc_t VbrMoMgr::ConvertVbr(DalDmlIntf *dmi,
                               IpcReqRespHeader *req,
                               ConfigKeyVal *ikey,
                               TcConfigMode config_mode,
                               string vtn_name,
                               unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!req) {
    UPLL_LOG_DEBUG("Input req is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  IpcReqRespHeader req_tmp;
  memcpy(&req_tmp, req, sizeof(IpcReqRespHeader));
  // clnt_sess_id, config_id, rep_count, option1, option2 = 0
  req_tmp.operation = op;
  req_tmp.datatype = UPLL_DT_CANDIDATE;
  result_code = ValidateConvertVbrMessage(&req_tmp, ikey, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ValidateConvertVbrMessage faliled %d", result_code);
    return result_code;
  }
  if (op == UNC_OP_CREATE) {
    result_code = CreateConvertVbrMo(&req_tmp, ikey, config_mode, vtn_name,
                                     dmi);
    if (UPLL_RC_SUCCESS != result_code)
      UPLL_LOG_DEBUG("CreateConvertVbrMo faliled %d", result_code);
    return result_code;
  } else {
    result_code =  DeleteConvertVbrMo(&req_tmp, ikey, config_mode, vtn_name,
                                      dmi);
    if (UPLL_RC_SUCCESS != result_code)
      UPLL_LOG_DEBUG("DeleteConvertVbrMo faliled %d", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::ValidateConvertVbrMessage(IpcReqRespHeader *req,
                                              ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !(ikey->get_key()) ||!req || !dmi) {
    UPLL_LOG_DEBUG("Invalid input parameters to create convert vBridge");
    return UPLL_RC_ERR_GENERIC;
  }
  // check whether operation is create/Update and dt_type should be candidate.
  if (((req->operation != UNC_OP_CREATE) &&
      (req->operation != UNC_OP_DELETE)) ||
      ((req->datatype != UPLL_DT_CANDIDATE) &&
      (req->datatype != UPLL_DT_IMPORT))) {
    UPLL_LOG_DEBUG("Invalid request for convert vBridge %d", req->operation);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  // validate ctrlr and domain during Create
  if (req->operation == UNC_OP_CREATE) {
    controller_domain ctrlr_dom = {NULL, NULL};
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    if (!(ikey->get_cfg_val()) ||!GetVal(ikey) || !(ctrlr_dom.ctrlr) ||
        !strlen(reinterpret_cast<const char *>(ctrlr_dom.ctrlr)) ||
        !(ctrlr_dom.domain) ||
        !strlen(reinterpret_cast<const char *>(ctrlr_dom.domain))) {
      UPLL_LOG_DEBUG("Invalid parameter for convert vBridge create");
      return UPLL_RC_ERR_GENERIC;
    }
  }
  return result_code;
}

upll_rc_t VbrMoMgr::CreateConvertVbrMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       TcConfigMode config_mode,
                                       string vtn_name,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  key_convert_vbr_t *con_vbr_key =
               reinterpret_cast<key_convert_vbr_t *>(ikey->get_key());

  upll_rc_t res_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_run = NULL;
  res_code = GetChildConvertConfigKey(ckv_run, ikey);
  if (res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey failed %d", res_code);
    return res_code;
  }
  // Check if any entry of ru_convert_vbr_tbl in the same controller and domain
  // if exists, then copy that converted_vbr_name and create in candidate DB
  // else, auto generate the name
  DbSubOp dbop1 = {kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                   kOpInOutNone};
  res_code = ReadConfigDB(ckv_run, UPLL_DT_RUNNING, UNC_OP_READ,
                          dbop1, dmi, CONVERTTBL);
  if ((res_code != UPLL_RC_SUCCESS) &&
      (res_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", res_code);
    DELETE_IF_NOT_NULL(ckv_run);
    return res_code;
  } else if (res_code == UPLL_RC_SUCCESS) {
    uuu::upll_strncpy(con_vbr_key->conv_vbr_name,
      reinterpret_cast<key_convert_vbr_t *>(ckv_run->get_key())->conv_vbr_name,
      (kMaxLenConvertVnodeName+1));
  } else {
    while (1) {
      // Generate cinverted vbridge based on timestamp and
      //  if the name is not unique, Regenerate until getting an unique name
      string conv_vbr_name = unc::upll::upll_util::getTime();
      string input = conv_vbr_name;
      std::stringstream ssoutput;
      for (int i = 0; i < 7; i++)
        ssoutput << input[(strlen(input.c_str())) - (7 - i)];
      // Assign auto generated odd numbers for convert vbridge
      int autogenerateno = atoi((ssoutput.str()).c_str());
      if (autogenerateno % 2 != 0) {
        conv_vbr_name =
          string(reinterpret_cast<char *>(con_vbr_key->vbr_key.vbridge_name)) +
          "_" + ssoutput.str();
        UPLL_LOG_DEBUG("Auto generated convert vbridge name is %s",
            conv_vbr_name.c_str());
      } else {
        continue;
      }
      uuu::upll_strncpy(con_vbr_key->conv_vbr_name, conv_vbr_name.c_str(),
          (kMaxLenConvertVnodeName+1));
      DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutNone};
      res_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_READ, dmi,
                               &dbop, CONVERTTBL);
      if ((res_code != UPLL_RC_ERR_INSTANCE_EXISTS) &&
          (res_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
        UPLL_LOG_TRACE("UpdateConfigDB Failed -%d", res_code);
        return res_code;
      } else if (res_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_TRACE("Convert vbr name already exists in convert_vbr_tbl");
        continue;
      }
      break;
    }
  }
  DELETE_IF_NOT_NULL(ckv_run);
  controller_domain ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  if (!ctrlr_dom.ctrlr || !ctrlr_dom.domain) {
    UPLL_LOG_ERROR("ctrlr or domain is NULL in input ckv");
    return UPLL_RC_ERR_GENERIC;
  }
  // Update/Create vtn_ctrlr_tbl for convereted vbridge create
  bool ctrlr_vtn_flag = false;
  bool conv_vbr = true;
  res_code = CheckRenamedVtnName(ikey, req->datatype, &ctrlr_dom, dmi);
  if (res_code != UPLL_RC_ERR_NO_SUCH_INSTANCE &&
      res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in Reading DB %d", res_code);
    return res_code;
  } else if (res_code == UPLL_RC_SUCCESS) {
     ctrlr_vtn_flag = true;
  }
  res_code = CheckVtnExistenceOnController(ikey, req, &ctrlr_dom,
                                           ctrlr_vtn_flag, dmi, conv_vbr);
  if (UPLL_RC_SUCCESS != res_code) {
    UPLL_LOG_INFO("CheckVtnExistenceOnController failed %d", res_code);
    return res_code;
  }

  ConfigKeyVal *uvbr_ckv = NULL;
  res_code = GetUnifiedVbridgeConfigKey(uvbr_ckv, ikey);
  if (res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Record Creation fialed in CANDIDATE DB");
    return res_code;
  }
  bool auto_rename = false;
  if (UPLL_DT_CANDIDATE == req->datatype)
    auto_rename = true;
  res_code = GenerateAutoName(uvbr_ckv, req->datatype, &ctrlr_dom,
                              dmi, &auto_rename, config_mode, vtn_name);
  if (res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Record Creation fialed in CANDIDATE DB");
    DELETE_IF_NOT_NULL(uvbr_ckv);
    return res_code;
  }
  uint8_t vbr_flag = 0;
  GET_USER_DATA_FLAGS(uvbr_ckv, vbr_flag);
  SET_USER_DATA_FLAGS(ikey, vbr_flag);
  DELETE_IF_NOT_NULL(uvbr_ckv);

  // Creating entry in Candidate DB.
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutDomain
    | kOpInOutCtrlr };
  res_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi, &dbop,
                            config_mode, vtn_name, CONVERTTBL);
  return res_code;
}


upll_rc_t VbrMoMgr::DeleteConvertVbrMo(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey,
                                        TcConfigMode config_mode,
                                        string vtn_name,
                                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *parent_key = NULL;

  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  if ((ctrlr_dom.ctrlr == NULL) || (ctrlr_dom.domain == NULL)) {
    UPLL_LOG_INFO("Invalid ctrlr/domain");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = GetParentConfigKey(parent_key, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    if (NULL != parent_key) delete parent_key;
    return result_code;
  }
  // Update Ref count in vtn controller table or
  // Delete vtn controller table if it is a last entry
  bool conv_vbr = true;
  result_code = HandleVtnCtrlrTbl(req, parent_key, UNC_KT_VBRIDGE, &ctrlr_dom,
                                  dmi, conv_vbr);
  DELETE_IF_NOT_NULL(parent_key);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to update vtn controller table %d", result_code);
    return result_code;
  }

  // Delete the vnode_rename_tbl entry if exists.
  ConfigKeyVal *uvbr_ckv = NULL;
  result_code = GetUnifiedVbridgeConfigKey(uvbr_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetUnifiedVbridgeConfigKey fails with %d", result_code);
    return result_code;
  }
  DbSubOp dbop = {kOpNotRead, kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone};
  result_code = UpdateConfigDB(uvbr_ckv, req->datatype, UNC_OP_DELETE, dmi,
                               &dbop, config_mode, vtn_name, RENAMETBL);
  DELETE_IF_NOT_NULL(uvbr_ckv);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                UPLL_RC_SUCCESS:result_code;
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Delete Operation fails with %d", result_code);
    return result_code;
  }

  // Delete the vbr_convert_tbl entry.
  result_code = UpdateConfigDB(ikey, req->datatype, req->operation, dmi,
                               &dbop, config_mode, vtn_name, CONVERTTBL);
  return result_code;
}

// To change the UNC specific ConfigKeyVal to Driver
// speific ConfigKeyVal during commit/Audit
upll_rc_t VbrMoMgr::AdaptValToDriver(ConfigKeyVal *&ck_vbr,
                                     ConfigKeyVal *ck_conv_vbr,
                                     MoMgrTables tbl_indx,
                                     bool &is_unified) {
  UPLL_FUNC_TRACE;
  if (tbl_indx == MAINTBL) {
    // In case of MAINTBL check whether it is Unified vBrisge
    // or normal vBridge
    val_vbr *vbr_val = reinterpret_cast<val_vbr *>(GetVal(ck_conv_vbr));
    if (IsUnifiedVbr(vbr_val->controller_id)) {
      is_unified = true;
      return UPLL_RC_SUCCESS;
    }
    upll_rc_t res_code = DupConfigKeyVal(ck_vbr, ck_conv_vbr);
    if (res_code != UPLL_RC_SUCCESS)
      UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", res_code);
    return res_code;
  } else if (tbl_indx == CONVERTTBL) {
    // Incase of CONVERTTBL change Convert vbridge ConfigKeyVal to
    // Normal vbridge ConfigKeyVal to send it to the Driver.
    key_convert_vbr_t *conv_vbr_key =
              reinterpret_cast<key_convert_vbr_t *>(ck_conv_vbr->get_key());
    ConfigVal *val = ck_conv_vbr->get_cfg_val();
    val_convert_vbr_t *conv_vbr_val = val ?
            reinterpret_cast<val_convert_vbr_t *>(GetVal(ck_conv_vbr)) : NULL;
    key_vbr *vbr_key = NULL;
    val_vbr *vbr_val = NULL;
    if (conv_vbr_key) {
      vbr_key = reinterpret_cast<key_vbr *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr)));
      uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                 conv_vbr_key->vbr_key.vtn_key.vtn_name, (kMaxLenVtnName+1));
      uuu::upll_strncpy(vbr_key->vbridge_name,
                 conv_vbr_key->vbr_key.vbridge_name, (kMaxLenVnodeName+1));
    }

    ck_vbr = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                            vbr_key, NULL);
    if (!ck_vbr) {
      UPLL_LOG_DEBUG("ConfigVal allocation failed");
      FREE_IF_NOT_NULL(vbr_key);
      return UPLL_RC_ERR_GENERIC;
    }
    SET_USER_DATA(ck_vbr, ck_conv_vbr);

    while (conv_vbr_val && ((val->get_st_num() ==
                                IpctSt::kIpcStValConvertVbr))) {
      controller_domain ctrlr_dom;
      memset(&ctrlr_dom, 0, sizeof(controller_domain));
      GET_USER_DATA_CTRLR_DOMAIN(ck_conv_vbr, ctrlr_dom);
      vbr_val = reinterpret_cast<val_vbr *>
        (ConfigKeyVal::Malloc(sizeof(val_vbr)));
      uuu::upll_strncpy(vbr_val->controller_id,
                 ctrlr_dom.ctrlr, (kMaxLenCtrlrId+1));
      vbr_val->valid[UPLL_IDX_CONTROLLER_ID_VBR] = UNC_VF_VALID;
      uuu::upll_strncpy(vbr_val->domain_id,
                 ctrlr_dom.domain, (kMaxLenDomainId+1));
      vbr_val->valid[UPLL_IDX_DOMAIN_ID_VBR] = UNC_VF_VALID;
      vbr_val->label = conv_vbr_val->label;
      vbr_val->valid[UPLL_IDX_LABEL_VBR] = UNC_VF_VALID;

      for (unsigned int loop = 0;
       loop < sizeof(vbr_val->valid) / sizeof(uint8_t); ++loop) {
       if ((loop != UPLL_IDX_CONTROLLER_ID_VBR) && (loop !=
           UPLL_IDX_DOMAIN_ID_VBR) && (loop != UPLL_IDX_LABEL_VBR))
           vbr_val->valid[loop] = UNC_VF_INVALID;
      }
      ConfigVal *val_tmp = new ConfigVal(IpctSt::kIpcStValVbr, vbr_val);
      if (!val_tmp) {
        UPLL_LOG_DEBUG("ConfigVal allocation failed");
        FREE_IF_NOT_NULL(vbr_key);
        FREE_IF_NOT_NULL(vbr_val);
        return UPLL_RC_ERR_GENERIC;
      }
      ck_vbr->AppendCfgVal(val_tmp);
      val = val->get_next_cfg_val();
      conv_vbr_val = val ?
            reinterpret_cast<val_convert_vbr_t *>(val->get_val()) : NULL;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::TxUpdateController(unc_key_type_t keytype,
                                       uint32_t session_id,
                                       uint32_t config_id,
                                       uuc::UpdateCtrlrPhase phase,
                                       set<string> *affected_ctrlr_set,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal **err_ckv,
                                       TxUpdateUtil *tx_util,
                                       TcConfigMode config_mode,
                                       std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // keytype and config_mode check
  if ((config_mode == TC_CONFIG_VIRTUAL) && (!VIRTUAL_MODE_KT(keytype))) {
      UPLL_LOG_DEBUG("Mode is VIRTUAL and Keytype is VTN KT");
      return UPLL_RC_SUCCESS;
  } else if ((config_mode == TC_CONFIG_VIRTUAL) &&
             (phase != uuc::kUpllUcpUpdate)) {
    UPLL_LOG_DEBUG("Mode is VIRTUAL and Keytype is VTN KT");
    return UPLL_RC_SUCCESS;
  } else if ((config_mode == TC_CONFIG_VTN) && (VIRTUAL_MODE_KT(keytype)) &&
            (phase == uuc::kUpllUcpUpdate)) {
    UPLL_LOG_DEBUG("Mode is VTN and Keytype is VIRTUAL and phase is UPDATE");
    return UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_DEBUG("TxUpdateController continue");
  }
  DalResultCode db_result;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL, *ck_old = NULL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  if (affected_ctrlr_set == NULL)
    return UPLL_RC_ERR_GENERIC;
  // Get the corresponding operation (CREATE/UPDATE/DELETE) based on the phase
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
      ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
       ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
  if (op == UNC_OP_INVALID) {
    UPLL_LOG_INFO("Invalid operation received-%d", op);
    // Not a valid operation
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  MoMgrTables tbl[] = {MAINTBL, CONVERTTBL};
  int ntbl =  sizeof(tbl)/ sizeof(tbl[0]);

  for (int tbl_indx = 0; tbl_indx < ntbl; tbl_indx++) {
  // Get CREATE, DELETE and UPDATE object information from the MAINTBL between
  // candidate configuration and running configuration where 'req' parameter
  // contains the candidate information and
  // the 'nreq' parameter contains the running configuration.
    if (tbl[tbl_indx] == CONVERTTBL) {
      result_code = GetChildConvertConfigKey(req, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        return UPLL_RC_ERR_GENERIC;
      }
      if (op == UNC_OP_UPDATE) {
        result_code = GetChildConvertConfigKey(nreq, NULL);
        if (result_code != UPLL_RC_SUCCESS) {
          return UPLL_RC_ERR_GENERIC;
        }
      }
    }
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                               op, req, nreq, &dal_cursor_handle, dmi,
                               config_mode, vtn_name, tbl[tbl_indx]);
    while (result_code == UPLL_RC_SUCCESS) {
      if (tx_util->GetErrCount() > 0) {
        UPLL_LOG_ERROR("TxUpdateUtil says exit the loop.");
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(req);
        return UPLL_RC_ERR_GENERIC;
      }

      // Iterate loop to get next record
      db_result = dmi->GetNextRecord(dal_cursor_handle);
      result_code = DalToUpllResCode(db_result);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" GetNextRecord failed err code(%d)", result_code);
        break;
      }
      // Duplicates the candidate ConfigKeyVal 'req' into 'ck_main'
      bool is_unified = false;
      result_code = AdaptValToDriver(ck_main, req, tbl[tbl_indx], is_unified);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("AdaptValToDriver failed for vBridge ConfigKeyVal-%d",
                       result_code);
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(ck_main);
        return result_code;
      }
      if (is_unified) {
        DELETE_IF_NOT_NULL(ck_main);
        continue;
      }
      string domain_type;
      // If the Table is CONVERTTBL, send domain_id with (PF_LEAF) prefix
      // Else send normal domain_id
      if (tbl[tbl_indx] == CONVERTTBL) {
        domain_type = "(PF_LEAF)";
      }
      if (op == UNC_OP_UPDATE) {
        result_code = AdaptValToDriver(ck_old, nreq, tbl[tbl_indx], is_unified);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("AdaptValToDriver failed for converting candidate"
                        " ConfigKeyVal-%d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_old);
          return result_code;
        }
      }
      // Case1: commit(del) - Check in RUNNING since no info exists in CAND
      // Case2: Commit(Cr/upd) - Check in CANDIDATE always
      upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
          UPLL_DT_RUNNING : UPLL_DT_CANDIDATE;
      if ((op == UNC_OP_CREATE) || (op == UNC_OP_UPDATE)) {
        void *main = GetVal(ck_main);
        void *val_nrec = (ck_old) ? GetVal(ck_old) : NULL;
        // Filter the attribute which is not sent to controller
        if (FilterAttributes(main, val_nrec, false, op)) {
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_old);
          continue;
        }
      }

      // Perform mode specific sematic check for each keytype
      // check the record existence in running database
      if (config_mode != TC_CONFIG_GLOBAL) {
        result_code = PerformModeSpecificSemanticCheck(ck_main, dmi, session_id,
                                                       config_id, op, keytype,
                                                       config_mode, vtn_name);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Mode specific semantic check failed %d\n",
                          result_code);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_old);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          dmi->CloseCursor(dal_cursor_handle, true);
          return result_code;
        }
      }
      // Get controller & domain-id from ConfigKeyVal 'ck_main'
      GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
      // If operation is UPDATE, append the running ConfigVal to the
      // candidate ConfigKeyVal using AppendCfgVal().
      // This provides the old and new value structures to the
      // driver in the case of UPDATE operation.

      // Contains controller specific key
      ConfigKeyVal *ckv_driver = NULL;
      result_code = DupConfigKeyVal(ckv_driver, ck_main);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("DupConfigKeyVal failed for candidate ConfigKeyVal-%d",
                      result_code);
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(ck_old);
        return result_code;
      }
      if (op == UNC_OP_UPDATE) {
        ConfigVal *next_val = (ck_old->get_cfg_val())->DupVal();
        uint8_t rename_flag = 0;
        GET_USER_DATA_FLAGS(ck_old->get_cfg_val(), rename_flag)
        SET_USER_DATA_FLAGS(next_val, rename_flag)
        ckv_driver->AppendCfgVal(next_val);
      }
      DELETE_IF_NOT_NULL(ck_old);
      // Get the controller key for the renamed unc key.
      result_code = GetRenamedControllerKey(ckv_driver, dt_type,
                                            dmi, &ctrlr_dom);

      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
                       result_code);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(ck_main);
        dmi->CloseCursor(dal_cursor_handle, true);
        return result_code;
      }
      UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                     ctrlr_dom.domain);
      // Inserting the controller to Set
      affected_ctrlr_set->insert
          (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));

      DELETE_IF_NOT_NULL(ck_main);
      // Contains unc specific key
      ConfigKeyVal *ckv_unc = NULL;
      result_code = DupConfigKeyVal(ckv_unc, req, tbl[tbl_indx]);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("DupConfigKeyVal failed for candidate ConfigKeyVal-%d",
                      result_code);
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(ckv_driver);
        return result_code;
      }

      result_code = tx_util->EnqueueRequest(session_id, config_id,
                                            UPLL_DT_CANDIDATE, op, dmi,
                                            ckv_driver, ckv_unc, domain_type);
      if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_ERROR("EnqueueRequest request failed");
         dmi->CloseCursor(dal_cursor_handle, true);
         DELETE_IF_NOT_NULL(nreq);
         DELETE_IF_NOT_NULL(req);
         DELETE_IF_NOT_NULL(ckv_driver);
         DELETE_IF_NOT_NULL(ckv_unc);
        return result_code;
      }
    }
    if (dal_cursor_handle) {
      dmi->CloseCursor(dal_cursor_handle, true);
      dal_cursor_handle = NULL;
    }
    DELETE_IF_NOT_NULL(nreq);
    DELETE_IF_NOT_NULL(req);
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
        UPLL_RC_SUCCESS : result_code;
    if (result_code != UPLL_RC_SUCCESS)
      return result_code;
  }
  return result_code;
}

// when Commit/Audit fails during Converted
// Vbridge corresponding vbr_portmap sholud be sent to apper layer as err_ckv.
upll_rc_t VbrMoMgr::TranslateVbrPortmapToVbrErr(ConfigKeyVal *ikey,
                                           ConfigKeyVal **err_ckv,
                                           upll_keytype_datatype_t dt_type,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  pfcdrv_val_vbr_portmap *val_pm =
              reinterpret_cast<pfcdrv_val_vbr_portmap *>(GetVal(ikey));
  res_code = GetChildConvertConfigKey(okey, ikey);
  if (res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", res_code);
    return res_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  if (val_pm->vbrpm.valid[UPLL_IDX_CONTROLLER_ID_VBRPM] == UNC_VF_VALID) {
    SET_USER_DATA_CTRLR(okey, val_pm->vbrpm.controller_id);
    dbop.matchop |= kOpMatchCtrlr;
  } else {
    uint8_t *ctrlr = NULL;
    GET_USER_DATA_CTRLR(okey, ctrlr);
    if (ctrlr && strlen(reinterpret_cast<char *>(ctrlr))) {
      dbop.matchop |= kOpMatchCtrlr;
    }
  }
  if (val_pm->vbrpm.valid[UPLL_IDX_DOMAIN_ID_VBRPM] == UNC_VF_VALID) {
    SET_USER_DATA_DOMAIN(okey, val_pm->vbrpm.domain_id);
    dbop.matchop |= kOpMatchDomain;
  } else {
    uint8_t *domain = NULL;
    GET_USER_DATA_DOMAIN(okey, domain);
    if (domain && strlen(reinterpret_cast<char *>(domain))) {
      dbop.matchop |= kOpMatchDomain;
    }
  }
  res_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                            CONVERTTBL);
  if ((res_code != UPLL_RC_SUCCESS) &&
      (res_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", res_code);
    DELETE_IF_NOT_NULL(okey);
    return res_code;
  }
  std::set<std::string> vbridge_set;
  ConfigKeyVal *travel = okey;
  while (travel) {
    if (!(travel->get_key())) {
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    key_convert_vbr_t *cvbr_key =
              reinterpret_cast<key_convert_vbr_t *>(travel->get_key());
    std::string vbr_name(
             reinterpret_cast<char *>(cvbr_key->vbr_key.vbridge_name));
    std::pair<std::set<std::string>::iterator, bool> ret;
    ret = vbridge_set.insert(vbr_name);
    if (ret.second == false) {
      travel = travel->get_next_cfg_key_val();
      continue;
    }
    ConfigKeyVal *uvbr_ckv = NULL;
    res_code =ConvertVbrToUvbr(travel, uvbr_ckv);
    if (res_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(uvbr_ckv);
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (!(*err_ckv))
      *err_ckv = uvbr_ckv;
    else
      (*err_ckv)->AppendCfgKeyVal(uvbr_ckv);
    travel = travel->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(okey);
  return res_code;
}

upll_rc_t VbrMoMgr::ConvertVbrToUvbr(ConfigKeyVal *cvbr_ckv,
                                     ConfigKeyVal *&uvbr_ckv) {
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  key_convert_vbr_t *cvbr_key =
                reinterpret_cast<key_convert_vbr_t *>(cvbr_ckv->get_key());
  res_code = GetChildConfigKey(uvbr_ckv, NULL);
  if (res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", res_code);
    return res_code;
  }
  key_vbr_t *uvbr_key = reinterpret_cast<key_vbr *>(uvbr_ckv->get_key());
  uuu::upll_strncpy(uvbr_key->vtn_key.vtn_name,
                    cvbr_key->vbr_key.vtn_key.vtn_name, kMaxLenVtnName+1);
  uuu::upll_strncpy(uvbr_key->vbridge_name,
                    cvbr_key->vbr_key.vbridge_name, kMaxLenVtnName+1);
  val_vbr_t *vbr_val = ConfigKeyVal::Malloc<val_vbr_t>();
  memset(vbr_val, 0, sizeof(val_vbr_t));
  ConfigVal  *val = new ConfigVal(IpctSt::kIpcStValVbr, vbr_val);
  uvbr_ckv->SetCfgVal(val);
  return res_code;
}

upll_rc_t VbrMoMgr::AuditUpdateController(
                             unc_key_type_t keytype,
                             const char *ctrlr_id,
                             uint32_t session_id,
                             uint32_t config_id,
                             uuc::UpdateCtrlrPhase phase,
                             DalDmlIntf *dmi,
                             ConfigKeyVal **err_ckv,
                             KTxCtrlrAffectedState *ctrlr_affected) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result = uud::kDalRcSuccess;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal  *ckv_running = NULL;
  ConfigKeyVal  *ckv_audit = NULL;
  ConfigKeyVal  *ckv_drvr = NULL;
  ConfigKeyVal  *resp = NULL;
  DalCursor *cursor = NULL;
  IpcResponse ipc_response;
  bool invalid_attr = false;
  string vtn_name = "";
  // Specifies true for audit update transaction
  // else false default
  const bool audit_update_phase = true;
  uint8_t *in_ctrlr = reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id));
  // Decides whether to retrieve from controller table or main table
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
    ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
     ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
  if (phase == uuc::kUpllUcpDelete2)
    return result_code;
  if (op == UNC_OP_INVALID) {
    UPLL_LOG_INFO("Invalid operation received-%d", op);
    // Not a valid operation
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  MoMgrTables tbl[] = {MAINTBL, CONVERTTBL};
  int ntbl =  sizeof(tbl)/ sizeof(tbl[0]);

  for (int tbl_indx = 0; tbl_indx < ntbl; tbl_indx++) {
    bool auditdiff_with_flag = false;
    if ((op == UNC_OP_CREATE) || (op == UNC_OP_DELETE))
      auditdiff_with_flag = true;
    // Get CREATE, DELETE and UPDATE object information based on the table 'tbl'
    // between running configuration and audit configuration
    // where 'ckv_running' parameter contains the running information and
    // the 'ckv_audit' parameter contains the audit configuration.
    if (tbl[tbl_indx] == CONVERTTBL) {
      result_code = GetChildConvertConfigKey(ckv_running, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        return UPLL_RC_ERR_GENERIC;
      }
      if (op == UNC_OP_UPDATE) {
        result_code = GetChildConvertConfigKey(ckv_audit, NULL);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(ckv_running);
          return UPLL_RC_ERR_GENERIC;
        }
      }
    }
    result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT, op,
                               ckv_running, ckv_audit, &cursor, dmi,
                               in_ctrlr, TC_CONFIG_GLOBAL, vtn_name,
                               tbl[tbl_indx], true, auditdiff_with_flag);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
      DELETE_IF_NOT_NULL(ckv_running);
      DELETE_IF_NOT_NULL(ckv_audit);
      return result_code;
    }
    // Iterate loop to get next record
    while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor)) &&
           ((result_code = ContinueAuditProcess()) == UPLL_RC_SUCCESS)) {
      UPLL_LOG_TRACE("Diff Running Record for Keytype: Operation:  is"
            " %d %d\n %s", keytype, op, ckv_running->ToStrAll().c_str());
       // ignore records of another controller for create and update op
      if (phase != uuc::kUpllUcpDelete) {
        uint8_t *db_ctrlr = NULL;
        GET_USER_DATA_CTRLR(ckv_running, db_ctrlr);
        if ((!db_ctrlr) ||
            (db_ctrlr && strncmp(reinterpret_cast<const char *>(db_ctrlr),
            reinterpret_cast<const char *>(ctrlr_id),
            strlen(reinterpret_cast<const char *>(ctrlr_id)) + 1))) {
          continue;
        }
      }
      if (op == UNC_OP_UPDATE)
        UPLL_LOG_TRACE("Diff Audit Record for Keytype: Operation:  is"
              " %d %d\n %s", keytype, op, ckv_audit->ToStrAll().c_str());
      // To fetch the records from the running which differ
      // with the audit records
      // If audit_update_phase flag is true and operation is UNC_OP_UPDATE
      // append the audit ConfigVal into running ConfigKeyVal
      // 'ckv_drvr' contains both running and audit ConfigVal
      result_code =  GetDiffRecord(ckv_running, ckv_audit, phase, tbl[tbl_indx],
                                   ckv_drvr, dmi, invalid_attr,
                                   audit_update_phase);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("GetDiffRecord failed err code is %d", result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      if (invalid_attr) {
        DELETE_IF_NOT_NULL(ckv_drvr);
        // Assuming that the diff found only in ConfigStatus
        // Setting the value as OnlyCSDiff in the out parameter ctrlr_affected
        // The value Configdiff should be given more priority than the value
        // only cs.
        // So, if the out parameter ctrlr_affected has already value as
        //  configdiff then dont change the value
        if (*ctrlr_affected != uuc::kCtrlrAffectedConfigDiff) {
          UPLL_LOG_INFO("Setting the ctrlr_affected to OnlyCSDiff for KT %u",
                        keytype);
          *ctrlr_affected = uuc::kCtrlrAffectedOnlyCSDiff;
        }
        continue;
      }

      GET_USER_DATA_CTRLR_DOMAIN(ckv_drvr, ctrlr_dom);
      if ((ctrlr_dom.ctrlr == NULL) ||(ctrlr_dom.domain == NULL)) {
        UPLL_LOG_DEBUG("Controller_id or domain_id is NULL");
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return UPLL_RC_ERR_GENERIC;
      }
      UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                   ctrlr_dom.domain);


      // Case1: commit(del) - Check in AUDIT since no info exists in RUNNING
      // Case2: Commit(Cr/upd) - Check in RUNNING always
      upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
          UPLL_DT_AUDIT : UPLL_DT_RUNNING;
      // not_send_to_drv is to decide whether the configuration needs
      // to be sent to controller or not
      bool is_unified = false;
      result_code = AdaptValToDriver(resp, ckv_drvr, tbl[tbl_indx], is_unified);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" DupConfigKeyVal failed err code(%d)",
                       result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(resp);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      if (is_unified) {
        DELETE_IF_NOT_NULL(resp);
        DELETE_IF_NOT_NULL(ckv_drvr);
        continue;
      }
      if (tbl[tbl_indx] == CONVERTTBL) {
        DELETE_IF_NOT_NULL(ckv_drvr);
        result_code = DupConfigKeyVal(ckv_drvr, resp);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed err code(%d)",
                         result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(ckv_drvr, ctrlr_dom);
        if (op == UNC_OP_UPDATE) {
          void *val1 = GetVal(ckv_drvr);
          void *val2 = ((ckv_drvr->get_cfg_val())->get_next_cfg_val()) ?
               (((ckv_drvr->get_cfg_val())->get_next_cfg_val())->get_val()) :
                NULL;
           //  Filter the attribute which is not sent to controller
           if (FilterAttributes(val1, val2, false, op)) {
             DELETE_IF_NOT_NULL(ckv_drvr);
             DELETE_IF_NOT_NULL(resp);
             continue;
          }
        }
      }
      // Get the controller key for the renamed unc key.
      result_code = GetRenamedControllerKey(ckv_drvr, dt_type,
                                            dmi, &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
                       result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(resp);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }

      string domain_type;
      // If the Table is CONVERTTBL, send domain_id with (PF_LEAF) prefix
      if (tbl[tbl_indx] == CONVERTTBL) {
        domain_type = (string("(PF_LEAF)")+
                      reinterpret_cast<char*>(ctrlr_dom.domain));
      } else {
        if (ctrlr_dom.domain != NULL)
          domain_type = reinterpret_cast<char *>(ctrlr_dom.domain);
      }

      memset(&ipc_response, 0, sizeof(IpcResponse));
      IpcRequest ipc_req;
      memset(&ipc_req, 0, sizeof(IpcRequest));
      ipc_req.header.clnt_sess_id = session_id;
      ipc_req.header.config_id = config_id;
      ipc_req.header.operation = op;
      ipc_req.header.datatype = UPLL_DT_CANDIDATE;
      ipc_req.ckv_data = ckv_drvr;
      // To populate IPC request.
      char drv_domain_name[KtUtil::kDrvDomainNameLenWith0];
      uuu::upll_strncpy(drv_domain_name, domain_type.c_str(),
                        KtUtil::kDrvDomainNameLenWith0);
      if (!IpcUtil::SendReqToDriver((const char *)ctrlr_dom.ctrlr,
                                    drv_domain_name, PFCDRIVER_SERVICE_NAME,
                                    PFCDRIVER_SVID_LOGICAL, &ipc_req,
                                    true, &ipc_response)) {
        UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                      ckv_drvr->get_key_type(),
                      reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        DELETE_IF_NOT_NULL(resp);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        dmi->CloseCursor(cursor, true);
        return ipc_response.header.result_code;
      }

      UPLL_LOG_DEBUG("Result code from driver %d",
                   ipc_response.header.result_code);
      if  (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
        // To convert the value structure read from DB to
        // VTNService during READ operations
        result_code = AdaptValToVtnService(resp, ADAPT_ONE);
        if (result_code != UPLL_RC_SUCCESS &&
            result_code != UPLL_RC_ERR_GENERIC) {
          UPLL_LOG_DEBUG("AdaptValToVtnService failed %d\n", result_code);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
        if (phase == uuc::kUpllUcpDelete) {
          *err_ckv = resp;
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          dmi->CloseCursor(cursor, true);
          return ipc_response.header.result_code;
        } else {
          UPLL_LOG_DEBUG("driver return failure err_code is %d",
                         ipc_response.header.result_code);
          *err_ckv = resp;
          ConfigKeyVal *ctrlr_key = NULL;
          // To get duplicate ConfigKeyVal from running ConfigKeyVal,
          // either from CTRLTBL or MAINTBL based on the key type.
          result_code = DupConfigKeyVal(ctrlr_key, ckv_running, tbl[tbl_indx]);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("DupConfigKeyVal failed");
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_drvr);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          // To update the audit configuration status as
          // UNC_CS_INVALID for the error record.
          result_code = UpdateAuditConfigStatus(UNC_CS_INVALID,
                                                phase, ctrlr_key, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("UpdateAuditConfigStatus failed");
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_drvr);
            dmi->CloseCursor(cursor, true);
            DELETE_IF_NOT_NULL(ctrlr_key);
            return result_code;
          }
          // To update configuration status.
          result_code = UpdateConfigDB(ctrlr_key, UPLL_DT_RUNNING,
                                       UNC_OP_UPDATE, dmi, TC_CONFIG_GLOBAL,
                                       "", tbl[tbl_indx]);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          dmi->CloseCursor(cursor, true);
          DELETE_IF_NOT_NULL(ctrlr_key);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG(
                "UpdateConfigDB failed for ipc response ckv err_code %d",
                result_code);
            return result_code;
          }
          return ipc_response.header.result_code;
        }
      }
      DELETE_IF_NOT_NULL(ckv_drvr);
      DELETE_IF_NOT_NULL(ipc_response.ckv_data);
      //  *ctrlr_affected = true;
      if (*ctrlr_affected == uuc::kCtrlrAffectedOnlyCSDiff) {
        UPLL_LOG_INFO("Reset ctrlr state from OnlyCSDiff to ConfigDiff  KT %u",
                      keytype);
      }
      UPLL_LOG_DEBUG("Setting the ctrlr_affected to ConfigDiff, KT %u",
                     keytype);
      *ctrlr_affected = uuc::kCtrlrAffectedConfigDiff;
      DELETE_IF_NOT_NULL(resp);
    }
    if (cursor) {
      dmi->CloseCursor(cursor, true);
      cursor = NULL;
    }
    if (uud::kDalRcSuccess != db_result) {
      UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d", db_result);
      result_code =  DalToUpllResCode(db_result);
    }
    DELETE_IF_NOT_NULL(ckv_running);
    DELETE_IF_NOT_NULL(ckv_audit);
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
        ? UPLL_RC_SUCCESS : result_code;
    if (result_code != UPLL_RC_SUCCESS)
      return result_code;
  }
  return result_code;
}

upll_rc_t VbrMoMgr::ReadExpandMo(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  if (!req || !ikey || !dmi) {
    UPLL_LOG_DEBUG("Input argument id NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *con_vbr = NULL;
  ConfigKeyVal *vbr_pm = NULL;
  ConfigKeyVal *con_vbrif = NULL;
  ConfigKeyVal *con_vlink = NULL;
  // Read all the Converted vbr tables Under the given Unified vBridge.
  // if the given vnode is not a Unified vBridge then this returns
  // NO SUCH INSTANCE
  res_code = GetChildConvertConfigKey(con_vbr, ikey);
  if (res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConvertConfigKey failed %d", res_code);
    return res_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
                   kOpInOutCtrlr | kOpInOutDomain };
  res_code = ReadConfigDB(con_vbr, req->datatype, UNC_OP_READ, dbop, dmi,
                          CONVERTTBL);
  if ((res_code != UPLL_RC_SUCCESS) &&
      (res_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", res_code);
    DELETE_IF_NOT_NULL(con_vbr);
    return res_code;
  } else if (res_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(con_vbr);
    con_vbr = NULL;
    res_code = UPLL_RC_SUCCESS;
  } else {
    // Read all the vbr_portmap records under that Unified vBridge
    MoMgrImpl *mgr_pm = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
                     GetMoManager(UNC_KT_VBR_PORTMAP)));
    if (!mgr_pm) {
      UPLL_LOG_ERROR("Unable to get KT_VBR_PORTMAP instace");
      return UPLL_RC_ERR_GENERIC;
    }
    res_code = mgr_pm->GetChildConfigKey(vbr_pm, ikey);
    if (res_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(con_vbr);
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", res_code);
      return res_code;
    }
    res_code = mgr_pm->ReadConfigDB(vbr_pm, req->datatype, UNC_OP_READ, dbop,
                                    dmi, MAINTBL);
    if ((res_code != UPLL_RC_SUCCESS) &&
        (res_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", res_code);
      DELETE_IF_NOT_NULL(con_vbr);
      DELETE_IF_NOT_NULL(vbr_pm);
      return res_code;
    } else if (res_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(vbr_pm);
      vbr_pm = NULL;
      res_code = UPLL_RC_SUCCESS;
    }
    // Read all Convert vbr_if records under the given Unified vBridge
    MoMgrImpl *mgr_if = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
                        GetMoManager(UNC_KT_VBR_IF)));
    if (!mgr_if) {
      UPLL_LOG_ERROR("Unable to get KT_VBR_IF instace");
      DELETE_IF_NOT_NULL(con_vbr);
      DELETE_IF_NOT_NULL(vbr_pm);
      return UPLL_RC_ERR_GENERIC;
    }
    res_code = mgr_if->GetChildConvertConfigKey(con_vbrif, ikey);
    if (res_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConvertConfigKey failed %d", res_code);
      DELETE_IF_NOT_NULL(con_vbr);
      DELETE_IF_NOT_NULL(vbr_pm);
      DELETE_IF_NOT_NULL(con_vbrif);
      return res_code;
    }
    res_code = mgr_if->ReadConfigDB(con_vbrif, req->datatype, UNC_OP_READ, dbop,
                                    dmi, CONVERTTBL);
    if ((res_code != UPLL_RC_SUCCESS) &&
        (res_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", res_code);
      DELETE_IF_NOT_NULL(con_vbr);
      DELETE_IF_NOT_NULL(vbr_pm);
      DELETE_IF_NOT_NULL(con_vbrif);
      return res_code;
    } else if (res_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(con_vbrif);
      con_vbrif = NULL;
      res_code = UPLL_RC_SUCCESS;
    }
    // Read all convert_vlink records under the given Unified vBridge
    MoMgrImpl *vlnk_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                            (GetMoManager(UNC_KT_VLINK)));
    if (!vlnk_mgr) {
      UPLL_LOG_DEBUG("mgr is NULL");
      DELETE_IF_NOT_NULL(con_vbr);
      DELETE_IF_NOT_NULL(vbr_pm);
      DELETE_IF_NOT_NULL(con_vbrif);
      return UPLL_RC_ERR_GENERIC;
    }
    res_code = vlnk_mgr->GetChildConvertConfigKey(con_vlink, ikey);
    if (res_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConvertConfigKey failed %d", res_code);
      DELETE_IF_NOT_NULL(con_vbr);
      DELETE_IF_NOT_NULL(vbr_pm);
      DELETE_IF_NOT_NULL(con_vbrif);
      return res_code;
    }
    res_code = vlnk_mgr->ReadConfigDB(con_vlink, req->datatype, UNC_OP_READ,
                                      dbop, dmi, CONVERTTBL);
    if ((res_code != UPLL_RC_SUCCESS) &&
        (res_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", res_code);
      DELETE_IF_NOT_NULL(con_vbr);
      DELETE_IF_NOT_NULL(vbr_pm);
      DELETE_IF_NOT_NULL(con_vbrif);
      DELETE_IF_NOT_NULL(con_vlink);
      return res_code;
    } else if (res_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(con_vlink);
      con_vlink = NULL;
      res_code = UPLL_RC_SUCCESS;
    }
  }
  // Invoke ReadConvertVbridge to frame vbr_expand structures for
  // all the convert_vbr ConfigKeyVal's read.
  res_code = ReadConvertVbridge(req, ikey, dmi, con_vbr, vbr_pm, con_vbrif,
                                con_vlink);
  if (res_code != UPLL_RC_SUCCESS)
    UPLL_LOG_DEBUG("ReadConvertVbridge failed %d", res_code);
  DELETE_IF_NOT_NULL(con_vbr);
  DELETE_IF_NOT_NULL(vbr_pm);
  DELETE_IF_NOT_NULL(con_vbrif);
  DELETE_IF_NOT_NULL(con_vlink);
  return res_code;
}

upll_rc_t VbrMoMgr::HandleVbrPortmapRead(ConfigKeyVal *ikey,
                                         ConfigKeyVal *vbr_pm,
                                         ConfigVal *vbr_ex) {
  UPLL_FUNC_TRACE;
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  if (!ikey || !vbr_pm || !vbr_ex) {
    UPLL_LOG_DEBUG("Input argument is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  uint32_t vbr_pm_count = 0;
  ConfigVal *vbrpm_ex = NULL;
  vbrpm_ex = new ConfigVal(IpctSt::kIpcStUint32, NULL);
  if (!vbrpm_ex) {
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *trav_vbrpm = vbr_pm;
  while (trav_vbrpm) {
    if (!(trav_vbrpm->get_key()) || !(trav_vbrpm->get_cfg_val()) ||
        !GetVal(trav_vbrpm)) {
      UPLL_LOG_DEBUG("vbr_portmap ConfigKeyVal has NULL key or val");
      DELETE_IF_NOT_NULL(vbrpm_ex);
      return UPLL_RC_ERR_GENERIC;
    }
    if (!CompareVbrPm(ikey, trav_vbrpm)) {
      trav_vbrpm = trav_vbrpm->get_next_cfg_key_val();
      continue;
    }
    // Frame vbr_portmap expand structure using vbr_pm Configkeyval
    res_code = FrameVbrPmExpand(trav_vbrpm, vbrpm_ex);
    if (res_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("FrameVbrPmExpand failed %d", res_code);
      DELETE_IF_NOT_NULL(vbrpm_ex);
      return res_code;
    }
    vbr_pm_count++;
    trav_vbrpm = trav_vbrpm->get_next_cfg_key_val();
  }
  // Updating the vbr_portmap count in ConfigVal
  uint32_t *count = reinterpret_cast<uint32_t *>(vbrpm_ex->get_val());
  if (!count) {
    count = ConfigKeyVal::Malloc<uint32_t>();
    vbrpm_ex->SetVal(IpctSt::kIpcStUint32, count);
  }
  *count = vbr_pm_count;
  vbr_ex->AppendCfgVal(vbrpm_ex);
  return res_code;
}
// To check whether the vbr_portmap is under the given converted vbridge or not
bool VbrMoMgr::CompareVbrPm(ConfigKeyVal *ckv_cvbr,
                            ConfigKeyVal *ckv_vbrpm) {
  controller_domain_t ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ckv_cvbr, ctrlr_dom);
  if (!ctrlr_dom.ctrlr || !ctrlr_dom.domain) {
    return false;
  }
  pfcdrv_val_vbr_portmap *vbrpm_val =
             reinterpret_cast<pfcdrv_val_vbr_portmap *>(GetVal(ckv_vbrpm));
  // Check whether the controller and domain are same or not
  // for the given convert_vbr key and vbr_pm key
  if (!(strncmp(reinterpret_cast<const char *>(ctrlr_dom.ctrlr),
                reinterpret_cast<const char *>(vbrpm_val->vbrpm.controller_id),
                kMaxLenCtrlrId+1)) &&
      !(strncmp(reinterpret_cast<const char *>(ctrlr_dom.domain),
                reinterpret_cast<const char *>(vbrpm_val->vbrpm.domain_id),
                kMaxLenDomainId+1))) {
    if (vbrpm_val->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] ==
                                                           UNC_VF_VALID) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

upll_rc_t VbrMoMgr::HandleVbrIfRead(ConfigKeyVal *ikey,
                                    IpcReqRespHeader *req,
                                    DalDmlIntf *dmi,
                                    ConfigKeyVal *con_vbrif,
                                    ConfigKeyVal *con_vlink,
                                    ConfigVal *vbr_ex) {
  UPLL_FUNC_TRACE;
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  if (!ikey || !dmi || !req || !vbr_ex) {
    UPLL_LOG_DEBUG("Input argument is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  // variable to maintain vbr_if_count
  uint32_t vbr_if_count = 0;
  // Val structures to append expand structures
  ConfigVal *vbrif_ex = NULL;
  vbrif_ex = new ConfigVal(IpctSt::kIpcStUint32, NULL);
  if (!vbrif_ex) {
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *trav_vbrif = con_vbrif;
  while (trav_vbrif) {
    if (!(trav_vbrif->get_key())) {
      UPLL_LOG_DEBUG("convert_vbr_if ConfigKeyVal has NULL key");
      DELETE_IF_NOT_NULL(vbrif_ex);
      return UPLL_RC_ERR_GENERIC;
    }
    if (!CompareVbrIf(ikey, trav_vbrif)) {
      trav_vbrif = trav_vbrif->get_next_cfg_key_val();
      continue;
    }
    ConfigKeyVal *trav_vlink = con_vlink;
    bool vbr_if_found = false;
    while (trav_vlink) {
      if (!(trav_vlink->get_key()) || !(trav_vlink->get_cfg_val()) ||
          !GetVal(trav_vlink)) {
        UPLL_LOG_DEBUG("convert_vlink ConfigKeyVal has NULL key or val");
        DELETE_IF_NOT_NULL(vbrif_ex);
        return UPLL_RC_ERR_GENERIC;
      }
      if (!CompareVlink(trav_vbrif, trav_vlink)) {
        trav_vlink = trav_vlink->get_next_cfg_key_val();
        continue;
      }
      vbr_if_found = true;
      res_code = FrameVbrIfExpandUsingConvVlink(trav_vlink, vbrif_ex);
      if (res_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("FrameVbrIfExpandUsingConvVlink failed %d", res_code);
        DELETE_IF_NOT_NULL(vbrif_ex);
        return res_code;
      }
      vbr_if_count++;
      break;
    }
    if (vbr_if_found == false) {
      ConfigKeyVal *vlnk_key= NULL;
      bool uvb_vnode1 = false;
      res_code = GetVlinkMaintblFromConvVbrIf(trav_vbrif, vlnk_key, req, dmi,
                                              &uvb_vnode1);
      if ((res_code != UPLL_RC_SUCCESS) &&
          (res_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
        UPLL_LOG_DEBUG("GetVlinkMaintblFromConvVbrIf failed %d", res_code);
        DELETE_IF_NOT_NULL(vbrif_ex);
        DELETE_IF_NOT_NULL(vlnk_key);
        return res_code;
      } else if (res_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        res_code = FrameVbrIfExpandUsingVlink(trav_vbrif, NULL, vbrif_ex,
                                              &uvb_vnode1);
        if (res_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("FrameVbrIfExpandUsingVlink failed %d", res_code);
          DELETE_IF_NOT_NULL(vbrif_ex);
          DELETE_IF_NOT_NULL(vlnk_key);
          return res_code;
        }
        vbr_if_count++;
      } else {
        if (!(vlnk_key->get_cfg_val()) || !(GetVal(vlnk_key))) {
          UPLL_LOG_DEBUG("Vlink_ckv has either key or val structure NULL");
          DELETE_IF_NOT_NULL(vbrif_ex);
          DELETE_IF_NOT_NULL(vlnk_key);
          return UPLL_RC_ERR_GENERIC;
        }
        res_code = FrameVbrIfExpandUsingVlink(trav_vbrif, vlnk_key, vbrif_ex,
                                              &uvb_vnode1);
        if (res_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("FrameVbrIfExpandUsingVlink failed %d", res_code);
          DELETE_IF_NOT_NULL(vbrif_ex);
          DELETE_IF_NOT_NULL(vlnk_key);
          return res_code;
        }
        vbr_if_count++;
      }
      DELETE_IF_NOT_NULL(vlnk_key);
    }
    trav_vbrif = trav_vbrif->get_next_cfg_key_val();
  }
  // Updating the con_vbr_if count
  uint32_t *count = NULL;
  count = reinterpret_cast<uint32_t *>(vbrif_ex->get_val());
  if (!count) {
    count = ConfigKeyVal::Malloc<uint32_t>();
    vbrif_ex->SetVal(IpctSt::kIpcStUint32, count);
  }
  *count = vbr_if_count;
  vbr_ex->AppendCfgVal(vbrif_ex);
  return res_code;
}

// To check whether the convert_vbr_if is under the given convertvbridge or not
bool VbrMoMgr::CompareVbrIf(ConfigKeyVal *ckv_cvbr,
                            ConfigKeyVal *ckv_vbrif) {
  key_convert_vbr_t *convbr_key =
              reinterpret_cast<key_convert_vbr_t *>(ckv_cvbr->get_key());
  key_convert_vbr_if_t *convbrif_key =
              reinterpret_cast<key_convert_vbr_if_t *>(ckv_vbrif->get_key());
  // Check whether the convert_vbr_name is same or not
  // for the given convert_vbr key and vbr_if_key
  if (!(strncmp(reinterpret_cast<const char *>(convbr_key->conv_vbr_name),
                reinterpret_cast<const char *>
                            (convbrif_key->convert_vbr_key.conv_vbr_name),
                kMaxLenConvertVnodeName+1))) {
    return true;
  } else {
    return false;
  }
}

// To check whether the convert_vbr_if exists in convert_vlink ot not
bool VbrMoMgr::CompareVlink(ConfigKeyVal *ckv_vbrif,
                            ConfigKeyVal *ckv_vlink) {
  key_convert_vbr_if_t *convbrif_key =
              reinterpret_cast<key_convert_vbr_if_t *>(ckv_vbrif->get_key());
  val_convert_vlink_t *convlink_val =
              reinterpret_cast<val_convert_vlink_t *>(GetVal(ckv_vlink));
  if ((convlink_val->valid[UPLL_IDX_VNODE1_NAME_CVLINK] == UNC_VF_VALID) &&
      (convlink_val->valid[UPLL_IDX_VNODE1_IF_NAME_CVLINK] == UNC_VF_VALID)) {
    if (!(strncmp(reinterpret_cast<const char *>(convbrif_key->convert_if_name),
                  reinterpret_cast<const char *>(convlink_val->vnode1_ifname),
                  kMaxLenInterfaceName+1)) &&
        !(strncmp(reinterpret_cast<const char *>
                 (convbrif_key->convert_vbr_key.conv_vbr_name),
                  reinterpret_cast<const char *>(convlink_val->vnode1_name),
                  kMaxLenConvertVnodeName+1))) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

// To check whether the convert_vbr_if present in vlink_main_tbl
upll_rc_t VbrMoMgr::GetVlinkMaintblFromConvVbrIf(ConfigKeyVal *vbrif_ckv,
                                                 ConfigKeyVal *&vlnk_ckv,
                                                 IpcReqRespHeader *req,
                                                 DalDmlIntf *dmi,
                                                 bool *uvb_vnode1) {
  UPLL_FUNC_TRACE;
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  key_convert_vbr_if_t *con_key_vbrif =
            reinterpret_cast<key_convert_vbr_if_t *>(vbrif_ckv->get_key());
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                         (GetMoManager(UNC_KT_VLINK)));
  if (!mgr)
    return UPLL_RC_ERR_GENERIC;
  // Construct Vlink configkeyval with the given convert_vbr_if ConfigKeyVal.
  res_code = mgr->GetChildConfigKey(vlnk_ckv, NULL);
  if (res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed %d", res_code);
    return UPLL_RC_ERR_GENERIC;
  }
  key_vlink_t *vlink_key =
              reinterpret_cast<key_vlink_t *>(vlnk_ckv->get_key());
  uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
                    con_key_vbrif->convert_vbr_key.vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName+1);
  val_vlink_t *vlink_val = ConfigKeyVal::Malloc<val_vlink_t>();
  uuu::upll_strncpy(vlink_val->vnode1_name,
                    con_key_vbrif->convert_vbr_key.vbr_key.vbridge_name,
                    kMaxLenVnodeName+1);
  vlink_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_VALID;
  uuu::upll_strncpy(vlink_val->vnode1_ifname, con_key_vbrif->convert_if_name,
                    kMaxLenVnodeName+1);
  vlink_val->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_VALID;
  vlink_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_INVALID;
  vlink_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
  ConfigVal *cfg_vlink = new ConfigVal(IpctSt::kIpcStValVlink, vlink_val);
  vlnk_ckv->SetCfgVal(cfg_vlink);
  // Read to get the vlink ConfigKeyVal in which ithe given
  // convert_vbr_if is used.
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  res_code = mgr->ReadConfigDB(vlnk_ckv, req->datatype, UNC_OP_READ, dbop,
                               dmi, MAINTBL);
  if ((res_code != UPLL_RC_SUCCESS) &&
      (res_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", res_code);
    return res_code;
  } else if (res_code == UPLL_RC_SUCCESS) {
    *uvb_vnode1 = true;
    return res_code;
  }
  val_vlink_t *vlink_val1 = ConfigKeyVal::Malloc<val_vlink_t>();
  uuu::upll_strncpy(vlink_val1->vnode2_name,
                    con_key_vbrif->convert_vbr_key.vbr_key.vbridge_name,
                    kMaxLenVnodeName+1);
  vlink_val1->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_VALID;
  uuu::upll_strncpy(vlink_val1->vnode2_ifname, con_key_vbrif->convert_if_name,
                    kMaxLenVnodeName+1);
  vlink_val1->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_VALID;
  vlink_val1->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
  vlink_val1->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_INVALID;
  ConfigVal *cfg_vlink1 = new ConfigVal(IpctSt::kIpcStValVlink, vlink_val1);
  vlnk_ckv->SetCfgVal(cfg_vlink1);
  res_code = mgr->ReadConfigDB(vlnk_ckv, req->datatype, UNC_OP_READ, dbop,
                               dmi, MAINTBL);
  if ((res_code != UPLL_RC_SUCCESS) &&
      (res_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", res_code);
  }
  return res_code;
}

upll_rc_t VbrMoMgr::HandleVtunnelRead(ConfigKeyVal *vlink_ckv,
                                      ConfigVal *vtun_ex) {
  UPLL_FUNC_TRACE;
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  uint32_t vtun_count= 0;
  if (!vtun_ex)
    return UPLL_RC_ERR_GENERIC;
  std::set<std::string> vtunnel_set;
  ConfigKeyVal *trav_vtun = vlink_ckv;
  while (trav_vtun) {
    if (!(trav_vtun->get_key()) || !(trav_vtun->get_cfg_val()) ||
        !(GetVal(trav_vtun))) {
      return UPLL_RC_ERR_GENERIC;
    }
    val_convert_vlink_t *vlnk_val =
               reinterpret_cast<val_convert_vlink_t *>(GetVal(trav_vtun));
    if (vlnk_val->valid[UPLL_IDX_VNODE2_NAME_CVLINK] != UNC_VF_VALID) {
      return UPLL_RC_ERR_GENERIC;
    }
    std::string vtun_name(reinterpret_cast<char *>(vlnk_val->vnode2_name));
    std::pair<std::set<std::string>::iterator, bool> ret;
    ret = vtunnel_set.insert(vtun_name);
    if (ret.second == false) {
      trav_vtun = trav_vtun->get_next_cfg_key_val();
      continue;
    }
    // Invoke FrameVtunnelExpand to frame the val_vtunnel_expand structure
    res_code = FrameVtunnelExpand(trav_vtun, vtun_ex);
    if (res_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("FrameVtunnelExpand failed %d", res_code);
      return res_code;
    }
    // handle Vtunnel_if under that particular vtunnel
    res_code = HandleVtunnelIfRead(vtun_name, vlink_ckv, vtun_ex);
    if (res_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("HandleVtunnelIfRead failed %d", res_code);
      return res_code;
    }
    vtun_count++;
    trav_vtun = trav_vtun->get_next_cfg_key_val();
  }
  uint32_t *count = reinterpret_cast<uint32_t *>(vtun_ex->get_val());
  if (!count) {
    count = ConfigKeyVal::Malloc<uint32_t>();
    vtun_ex->SetVal(IpctSt::kIpcStUint32, count);
  }
  *count = vtun_count;
  return res_code;
}

upll_rc_t VbrMoMgr::HandleVtunnelIfRead(const std::string &vtun_name,
                                        ConfigKeyVal *vlink_ckv,
                                        ConfigVal *vtun_ex) {
  UPLL_FUNC_TRACE;
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  uint32_t vtun_if_count =0;
  ConfigVal *vtunif_ex = new ConfigVal(IpctSt::kIpcStUint32, NULL);
  if (!vtunif_ex)
    return UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *trav_vlink = vlink_ckv;
  while (trav_vlink) {
    if (!(trav_vlink->get_key()) || !(trav_vlink->get_cfg_val()) ||
        !(GetVal(trav_vlink))) {
      UPLL_LOG_DEBUG("vlink ckv's key or val structure is NULL");
      DELETE_IF_NOT_NULL(vtunif_ex);
      return UPLL_RC_ERR_GENERIC;
    }
    val_convert_vlink_t *convlk_val =
            reinterpret_cast<val_convert_vlink_t *>(GetVal(trav_vlink));
    if (convlk_val->valid[UPLL_IDX_VNODE2_NAME_CVLINK] != UNC_VF_VALID) {
      DELETE_IF_NOT_NULL(vtunif_ex);
      return UPLL_RC_ERR_GENERIC;
    }
    if (!strncmp(reinterpret_cast<const char *>(vtun_name.c_str()),
                reinterpret_cast<const char *>(convlk_val->vnode2_name),
                kMaxLenConvertVnodeName+1)) {
      res_code = FrameVtunIfExpand(trav_vlink, vtunif_ex);
      if (res_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("FrameVtunIfExpand failed %d", res_code);
        DELETE_IF_NOT_NULL(vtunif_ex);
        return res_code;
      }
      vtun_if_count++;
    }
    trav_vlink = trav_vlink->get_next_cfg_key_val();
  }
  // Updating vtun_if_count
  uint32_t *count = reinterpret_cast<uint32_t *>(vtunif_ex->get_val());
  if (!count) {
    count = ConfigKeyVal::Malloc<uint32_t>();
    vtunif_ex->SetVal(IpctSt::kIpcStUint32, count);
  }
  *count = vtun_if_count;
  vtun_ex->AppendCfgVal(vtunif_ex);
  return res_code;
}

upll_rc_t VbrMoMgr::HandleVlinkRead(ConfigKeyVal *vlink_ckv,
                                    ConfigVal *vlink_ex) {
  UPLL_FUNC_TRACE;
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  if (!vlink_ex)
    return UPLL_RC_ERR_GENERIC;
  // variable to maintain the vlink count
  uint32_t vlink_count= 0;
  ConfigKeyVal *trav_vlink = vlink_ckv;
  while (trav_vlink) {
    if (!(trav_vlink->get_key()) || !(trav_vlink->get_cfg_val()) ||
        !(GetVal(trav_vlink))) {
      UPLL_LOG_DEBUG("vlink ckv's key or val structure is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    // Invoke FrameVlinkExpand to frame the val_vlink_expand structure
    res_code = FrameVlinkExpand(trav_vlink, vlink_ex);
    if (res_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("FrameVlinkExpand failed %d", res_code);
      return res_code;
    }
    vlink_count++;
    trav_vlink = trav_vlink->get_next_cfg_key_val();
  }
  // Updating vlink count
  uint32_t *count = reinterpret_cast<uint32_t *>(vlink_ex->get_val());
  if (!count) {
    count = ConfigKeyVal::Malloc<uint32_t>();
    vlink_ex->SetVal(IpctSt::kIpcStUint32, count);
  }
  *count = vlink_count;
  return res_code;
}

upll_rc_t VbrMoMgr::ReadConvertVbridge(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *con_vbr,
                                       ConfigKeyVal *vbr_pm,
                                       ConfigKeyVal *con_vbrif,
                                       ConfigKeyVal *con_vlink) {
  UPLL_FUNC_TRACE;
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  // Variables to maintain the vbr Count.
  uint32_t vbr_count = 0;
  // Val structure to store the vbr count
  ConfigVal *vbr_ex = new ConfigVal(IpctSt::kIpcStUint32, NULL);
  if (!vbr_ex)
    return UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *trav_vbr = con_vbr;
  while (trav_vbr) {
    if (!(trav_vbr->get_key()) || !(trav_vbr->get_cfg_val()) ||
        !(GetVal(trav_vbr))) {
      DELETE_IF_NOT_NULL(vbr_ex);
      return UPLL_RC_ERR_GENERIC;
    }
    // Invoke FrameVbrExpand() to frame vbr_expand structure
    res_code = FrameVbrExpand(trav_vbr, vbr_ex, req, dmi);
    if (res_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("FrameVbrExpand failed %d", res_code);
      DELETE_IF_NOT_NULL(vbr_ex);
      return res_code;
    }
    // Invoke HandleVbrPortmapRead() to frame vbr_pm_expand structure
    // using convert_vbr and vbr_pm ConfigKeyVal's read from DB.
    res_code = HandleVbrPortmapRead(trav_vbr, vbr_pm, vbr_ex);
    if (res_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("HandleVbrPortmapRead failed %d", res_code);
      DELETE_IF_NOT_NULL(vbr_ex);
      return res_code;
    }
    // Invoke HandleVbrIfRead() to frame vbr_if_expand structure,
    // using convert_vbr, convert_vbr_if and convert_vlink ConfigKeyVal's
    // read from DB.
    res_code = HandleVbrIfRead(trav_vbr, req, dmi, con_vbrif, con_vlink,
                               vbr_ex);
    if (res_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("HandleVbrIfRead failed %d", res_code);
      DELETE_IF_NOT_NULL(vbr_ex);
      return res_code;
    }
    vbr_count++;
    trav_vbr = trav_vbr->get_next_cfg_key_val();
  }
  // Updating convert_vbr count
  uint32_t *count = reinterpret_cast<uint32_t *>(vbr_ex->get_val());
  if (!count) {
    count = ConfigKeyVal::Malloc<uint32_t>();
    vbr_ex->SetVal(IpctSt::kIpcStUint32, count);
  }
  *count = vbr_count;
  ikey->SetCfgVal(vbr_ex);

  ConfigVal *vtun_ex = new ConfigVal(IpctSt::kIpcStUint32, NULL);
  if (!vtun_ex) {
    DELETE_IF_NOT_NULL(vbr_ex);
    return UPLL_RC_ERR_GENERIC;
  }
  // Invoke HandleVtunnelRead() to frame val_vtunnel_expand structure
  // using convert_vlink ConfigKeyVal's read from DB.
  res_code = HandleVtunnelRead(con_vlink, vtun_ex);
  if (res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("HandleVtunnelRead failed %d", res_code);
    DELETE_IF_NOT_NULL(vbr_ex);
    DELETE_IF_NOT_NULL(vtun_ex);
    return res_code;
  }
  ikey->AppendCfgVal(vtun_ex);

  ConfigVal *vlink_ex = new ConfigVal(IpctSt::kIpcStUint32, NULL);
  if (!vlink_ex) {
    DELETE_IF_NOT_NULL(vbr_ex);
    DELETE_IF_NOT_NULL(vtun_ex);
    return UPLL_RC_ERR_GENERIC;
  }
  // Invoke HandleVlinkRead() to frame val_vlink_expand structure
  // using convert_vlink ConfigKeyVal's read from DB.
  res_code = HandleVlinkRead(con_vlink, vlink_ex);
  if (res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("HandleVlinkRead failed %d", res_code);
    DELETE_IF_NOT_NULL(vbr_ex);
    DELETE_IF_NOT_NULL(vtun_ex);
    DELETE_IF_NOT_NULL(vlink_ex);
    return res_code;
  }
  ikey->AppendCfgVal(vlink_ex);
  return res_code;
}

upll_rc_t VbrMoMgr::FrameVbrExpand(ConfigKeyVal *ikey,
                                   ConfigVal *vbr_ex,
                                   IpcReqRespHeader *req,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  if (!ikey || !dmi  || !req || !(ikey->get_key()) ||
                      !(ikey->get_cfg_val()) || !GetVal(ikey)) {
    UPLL_LOG_DEBUG("Input argument is NULL")
    return UPLL_RC_ERR_GENERIC;
  }
  key_convert_vbr_t *con_vbr_key =
             reinterpret_cast<key_convert_vbr_t *>(ikey->get_key());
  val_convert_vbr_t *con_val_vbr =
             reinterpret_cast<val_convert_vbr_t *>(GetVal(ikey));
  // Get vtn_key(parent) from ikey and check for its controller name.
  ConfigKeyVal *parent_key= NULL;
  res_code = GetParentConfigKey(parent_key, ikey);
  if (res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetParentConfigKey failed %d", res_code);
    return res_code;
  }
  MoMgrImpl *vtn_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                      (GetMoManager(UNC_KT_VTN)));
  if (!vtn_mgr) {
    UPLL_LOG_DEBUG("mgr is NULL");
    DELETE_IF_NOT_NULL(parent_key);
    return UPLL_RC_ERR_GENERIC;
  }
  controller_domain ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(parent_key, ctrlr_dom);
  res_code = vtn_mgr->GetRenamedControllerKey(parent_key, req->datatype,
                                              dmi, &ctrlr_dom);
  if (res_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey failed %d", res_code);
    DELETE_IF_NOT_NULL(parent_key);
    return UPLL_RC_ERR_GENERIC;
  }
  // Frame UNC_KT_VTN_CONTROLLER ConfigKeyVal and
  // send a Read req to the driver to get the l-vtnid
  key_vtn_controller_t *convtn_key =
               ConfigKeyVal::Malloc<key_vtn_controller_t>();
  uuu::upll_strncpy(convtn_key->vtn_key.vtn_name,
               reinterpret_cast<key_vtn *>(parent_key->get_key())->vtn_name,
               kMaxLenVtnName+1);
  uuu::upll_strncpy(convtn_key->controller_name, ctrlr_dom.ctrlr,
               kMaxLenCtrlrId+1);
  uuu::upll_strncpy(convtn_key->domain_id, ctrlr_dom.domain,
               kMaxLenDomainId+1);
  ConfigKeyVal *lvtn_key = new ConfigKeyVal(UNC_KT_VTN_CONTROLLER,
                           IpctSt::kIpcStKeyVtnController, convtn_key, NULL);
  SET_USER_DATA_CTRLR_DOMAIN(lvtn_key, ctrlr_dom);
  IpcResponse ipc_resp;
  memset(&(ipc_resp), 0, sizeof(IpcResponse));
  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  memcpy(&(ipc_req.header), req, sizeof(IpcReqRespHeader));
  ipc_req.header.option2 = UNC_OPT2_NONE;
  ipc_req.ckv_data = lvtn_key;
  UPLL_LOG_TRACE("Before Sending to Driver %s", (ikey->ToStrAll()).c_str());
  UPLL_LOG_TRACE("Domain Name %s", ctrlr_dom.domain);
  UPLL_LOG_TRACE("Controller Name %s", ctrlr_dom.ctrlr);
  if (!IpcUtil::SendReqToDriver((const char *)(ctrlr_dom.ctrlr),
         reinterpret_cast<char *>(ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
         PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
    UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                  lvtn_key->get_key_type(),
                  reinterpret_cast<char *>(ctrlr_dom.ctrlr));
    DELETE_IF_NOT_NULL(parent_key);
    DELETE_IF_NOT_NULL(lvtn_key);
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    return ipc_resp.header.result_code;
  }

  DELETE_IF_NOT_NULL(ipc_req.ckv_data);
  UPLL_LOG_TRACE("AfterDriver Read from Controller %s",
                 ((ipc_resp.ckv_data)->ToStrAll()).c_str());
  if ((ipc_resp.header.result_code != UPLL_RC_SUCCESS) &&
      (ipc_resp.header.result_code != UPLL_RC_ERR_CTR_DISCONNECTED)) {
    DELETE_IF_NOT_NULL(parent_key);
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    return ipc_resp.header.result_code;
  }
  pfcdrv_val_vtn_controller *convtn_val = NULL;
  lvtn_key = ipc_resp.ckv_data;
  if (ipc_resp.header.result_code == UPLL_RC_SUCCESS) {
    if (!lvtn_key || !(lvtn_key->get_key()) || !(lvtn_key->get_cfg_val()) ||
        !(GetVal(lvtn_key))) {
      UPLL_LOG_DEBUG("Response from driver contains NULL arguments");
      DELETE_IF_NOT_NULL(parent_key);
      DELETE_IF_NOT_NULL(lvtn_key);
      return UPLL_RC_ERR_GENERIC;
    }
    convtn_val =
        reinterpret_cast<pfcdrv_val_vtn_controller *>(GetVal(lvtn_key));
  }
  // Frame the val_vbr_expand structure, with the values read
  // from DB and Controller
  val_vbr_expand *tmp_vbr_ex = reinterpret_cast<val_vbr_expand *>
                    (ConfigKeyVal::Malloc(sizeof(val_vbr_expand)));
  for (unsigned int loop = 0;
    loop < sizeof(tmp_vbr_ex->valid) / sizeof(tmp_vbr_ex->valid[0]); ++loop) {
    switch (loop) {
      case UPLL_IDX_VBRIDGE_NAME_VBRE:
        uuu::upll_strncpy(tmp_vbr_ex->vbridge_name, con_vbr_key->conv_vbr_name,
                         (kMaxLenConvertVnodeName+1));
        tmp_vbr_ex->valid[UPLL_IDX_VBRIDGE_NAME_VBRE] = UNC_VF_VALID;
        break;
      case UPLL_IDX_CONTROLLER_ID_VBRE:
        uuu::upll_strncpy(tmp_vbr_ex->controller_id, ctrlr_dom.ctrlr,
                         (kMaxLenCtrlrId+1));
        tmp_vbr_ex->valid[UPLL_IDX_CONTROLLER_ID_VBRE] = UNC_VF_VALID;
        break;
      case UPLL_IDX_DOMAIN_ID_VBRE:
        uuu::upll_strncpy(tmp_vbr_ex->domain_id, ctrlr_dom.domain,
                         (kMaxLenDomainId+1));
        tmp_vbr_ex->valid[UPLL_IDX_DOMAIN_ID_VBRE] = UNC_VF_VALID;
        break;
      case UPLL_IDX_LABEL_VBRE:
        if (con_val_vbr->valid[UPLL_IDX_LABEL_CONV_VBR] == UNC_VF_VALID) {
          tmp_vbr_ex->label = con_val_vbr->label;
          tmp_vbr_ex->valid[UPLL_IDX_LABEL_VBRE] = UNC_VF_VALID;
        } else {
          tmp_vbr_ex->valid[UPLL_IDX_LABEL_VBRE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_CONTROLLER_VTN_NAME_VBRE:
        uuu::upll_strncpy(tmp_vbr_ex->controller_vtn_name,
                reinterpret_cast<key_vtn *>(parent_key->get_key())->vtn_name,
                (kMaxLenVtnName+1));
        tmp_vbr_ex->valid[UPLL_IDX_CONTROLLER_VTN_NAME_VBRE] = UNC_VF_VALID;
        break;
      case UPLL_IDX_CONTROLLER_VTN_LABEL_VBRE:
        if ((ipc_resp.header.result_code == UPLL_RC_SUCCESS) &&
            (convtn_val->valid[PFCDRV_IDX_VTNCTRL_LABEL] == UNC_VF_VALID)) {
          tmp_vbr_ex->controller_vtn_label = convtn_val->label;
          tmp_vbr_ex->valid[UPLL_IDX_CONTROLLER_VTN_LABEL_VBRE] = UNC_VF_VALID;
        } else {
          tmp_vbr_ex->valid[UPLL_IDX_CONTROLLER_VTN_LABEL_VBRE] =
                                                            UNC_VF_INVALID;
        }
        break;
      default:
        DELETE_IF_NOT_NULL(parent_key);
        DELETE_IF_NOT_NULL(lvtn_key);
        FREE_IF_NOT_NULL(tmp_vbr_ex);
        return UPLL_RC_ERR_GENERIC;
    }
  }
  DELETE_IF_NOT_NULL(parent_key);
  DELETE_IF_NOT_NULL(lvtn_key);
  ConfigVal *val_tmp = new ConfigVal(IpctSt::kIpcStValVbrExpand, tmp_vbr_ex);
  if (!val_tmp) {
    FREE_IF_NOT_NULL(tmp_vbr_ex);
    return UPLL_RC_ERR_GENERIC;
  }
  vbr_ex->AppendCfgVal(val_tmp);
  return res_code;
}

upll_rc_t VbrMoMgr::FrameVbrPmExpand(ConfigKeyVal *ikey,
                                     ConfigVal *vbrpm_ex) {
  key_vbr_portmap_t *vbrpm_key =
           reinterpret_cast<key_vbr_portmap_t *>(ikey->get_key());
  pfcdrv_val_vbr_portmap *valpm_vbr =
           reinterpret_cast<pfcdrv_val_vbr_portmap *>(GetVal(ikey));
  val_vbr_portmap_expand *tmp_pm_ex =
                  ConfigKeyVal::Malloc<val_vbr_portmap_expand>();
  // Frame val_vbr_portmap_expand structure using the record read from DB.
  for (unsigned int loop = 0;
       loop < sizeof(tmp_pm_ex->valid) / sizeof(tmp_pm_ex->valid[0]);
       ++loop) {
    switch (loop) {
      case UPLL_IDX_PORTMAP_ID_VBRPME:
        uuu::upll_strncpy(tmp_pm_ex->portmap_id, vbrpm_key->portmap_id,
                         (kMaxLenVnodeName+1));
        tmp_pm_ex->valid[UPLL_IDX_PORTMAP_ID_VBRPME] = UNC_VF_VALID;
        break;
      case UPLL_IDX_LOGICAL_PORT_ID_VBRPME:
        if (valpm_vbr->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] ==
                                                          UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_pm_ex->logical_port_id,
                   valpm_vbr->vbrpm.logical_port_id, (kMaxLenVnodeName+1));
          tmp_pm_ex->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPME] = UNC_VF_VALID;
        } else {
          tmp_pm_ex->valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPME] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_LABEL_TYPE_VBRPME:
        if (valpm_vbr->vbrpm.valid[UPLL_IDX_LABEL_TYPE_VBRPM] == UNC_VF_VALID) {
          tmp_pm_ex->label_type = valpm_vbr->vbrpm.label_type;
          tmp_pm_ex->valid[UPLL_IDX_LABEL_TYPE_VBRPME] = UNC_VF_VALID;
        } else {
          tmp_pm_ex->valid[UPLL_IDX_LABEL_TYPE_VBRPME] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_LABEL_VBRPME:
        if (valpm_vbr->vbrpm.valid[UPLL_IDX_LABEL_VBRPM] == UNC_VF_VALID) {
          tmp_pm_ex->label = valpm_vbr->vbrpm.label;
          tmp_pm_ex->valid[UPLL_IDX_LABEL_VBRPME] = UNC_VF_VALID;
        } else {
          tmp_pm_ex->valid[UPLL_IDX_LABEL_VBRPME] = UNC_VF_INVALID;
        }
        break;
      default:
        DELETE_IF_NOT_NULL(vbrpm_ex);
        FREE_IF_NOT_NULL(tmp_pm_ex);
        return UPLL_RC_ERR_GENERIC;
    }
  }
  // Append the val_vbr_portmap_expand to vbr_portmap_expand ConfigVal
  ConfigVal *val_tmp = new ConfigVal(IpctSt::kIpcStValVbrPortMapExpand,
                                    tmp_pm_ex);
  if (!val_tmp) {
    FREE_IF_NOT_NULL(tmp_pm_ex);
    return UPLL_RC_ERR_GENERIC;
  }
  vbrpm_ex->AppendCfgVal(val_tmp);
  return UPLL_RC_SUCCESS;
}

// To frame vbr_if_expand structure using convert
upll_rc_t VbrMoMgr::FrameVbrIfExpandUsingConvVlink(ConfigKeyVal *ikey,
                                                   ConfigVal *vbrif_ex) {
  UPLL_FUNC_TRACE;
  key_convert_vlink_t *vlink_key =
               reinterpret_cast<key_convert_vlink_t *>(ikey->get_key());
  val_convert_vlink_t *vlink_val =
               reinterpret_cast<val_convert_vlink_t *>(GetVal(ikey));
  val_vbr_if_expand_t *tmp_vbrifx = ConfigKeyVal::Malloc<val_vbr_if_expand_t>();
  // Frame val_vbr_if_expand structure with the values read from DB.
  for (unsigned int loop = 0;
       loop < sizeof(tmp_vbrifx->valid) / sizeof(tmp_vbrifx->valid[0]);
       ++loop) {
    switch (loop) {
      case UPLL_IDX_IF_NAME_VBRIE:
        if (vlink_val->valid[UPLL_IDX_VNODE1_IF_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vbrifx->if_name, vlink_val->vnode1_ifname,
                           (kMaxLenInterfaceName+1));
          tmp_vbrifx->valid[UPLL_IDX_IF_NAME_VBRIE] = UNC_VF_VALID;
        } else {
          tmp_vbrifx->valid[UPLL_IDX_IF_NAME_VBRIE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_CONN_VNODE_NAME_VBRIE:
        if (vlink_val->valid[UPLL_IDX_VNODE2_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vbrifx->connected_vnode_name,
                       vlink_val->vnode2_name, (kMaxLenConvertVnodeName+1));
          tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_NAME_VBRIE] = UNC_VF_VALID;
        } else {
          tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_NAME_VBRIE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_CONN_VNODE_IF_NAME_VBRIE:
        if (vlink_val->valid[UPLL_IDX_VNODE2_IF_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vbrifx->connected_if_name,
                   vlink_val->vnode2_ifname, (kMaxLenInterfaceName+1));
          tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VBRIE] = UNC_VF_VALID;
        } else {
          tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VBRIE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_CONN_VLINK_NAME_VBRIE:
        uuu::upll_strncpy(tmp_vbrifx->connected_vlink_name,
                  vlink_key->convert_vlink_name, (kMaxLenVlinkName+1));
        tmp_vbrifx->valid[UPLL_IDX_CONN_VLINK_NAME_VBRIE] = UNC_VF_VALID;
        break;
      default :
        UPLL_LOG_DEBUG("Invalid attribute for val_vbr_if_expand");
        FREE_IF_NOT_NULL(tmp_vbrifx)
        return UPLL_RC_ERR_GENERIC;
    }
  }
  // Append the framed vbr_if expand structure to vbr_if ConfigVal.
  vbrif_ex->AppendCfgVal(IpctSt::kIpcStValVbrIfExpand, tmp_vbrifx);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::FrameVbrIfExpandUsingVlink(ConfigKeyVal *vbrif_ckv,
                                               ConfigKeyVal *vlnk_ckv,
                                               ConfigVal *vbrif_ex,
                                               bool *uvb_vnode1) {
  UPLL_FUNC_TRACE;
  key_vlink_t *vlink_key = NULL;
  val_vlink_t *vlink_val = NULL;
  key_convert_vbr_if_t *vbr_if_key = NULL;
  if (vlnk_ckv) {
    vlink_key = reinterpret_cast<key_vlink_t *>(vlnk_ckv->get_key());
    if (vlnk_ckv->get_cfg_val())
      vlink_val = reinterpret_cast<val_vlink_t *>(GetVal(vlnk_ckv));
    if (!vlink_key || !vlink_val)
      return UPLL_RC_ERR_GENERIC;
  }
  vbr_if_key = reinterpret_cast<key_convert_vbr_if_t *>(vbrif_ckv->get_key());
  val_vbr_if_expand_t *tmp_vbrifx = ConfigKeyVal::Malloc<val_vbr_if_expand_t>();
  // Frame val_vbr_if_expand structure with the values read from DB.
  for (unsigned int loop = 0;
       loop < sizeof(tmp_vbrifx->valid) / sizeof(tmp_vbrifx->valid[0]);
       ++loop) {
    switch (loop) {
      case UPLL_IDX_IF_NAME_VBRIE:
          uuu::upll_strncpy(tmp_vbrifx->if_name, vbr_if_key->convert_if_name,
                           (kMaxLenInterfaceName+1));
          tmp_vbrifx->valid[UPLL_IDX_IF_NAME_VBRIE] = UNC_VF_VALID;
        break;
      case UPLL_IDX_CONN_VNODE_NAME_VBRIE:
        if (vlnk_ckv) {
          if ((*uvb_vnode1) &&
           (vlink_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] == UNC_VF_VALID)) {
            uuu::upll_strncpy(tmp_vbrifx->connected_vnode_name,
                         vlink_val->vnode2_name, (kMaxLenConvertVnodeName+1));
            tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_NAME_VBRIE] = UNC_VF_VALID;
          } else if (vlink_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] ==
                                                              UNC_VF_VALID) {
            uuu::upll_strncpy(tmp_vbrifx->connected_vnode_name,
                         vlink_val->vnode1_name, (kMaxLenConvertVnodeName+1));
            tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_NAME_VBRIE] = UNC_VF_VALID;
          } else {
            tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_NAME_VBRIE] = UNC_VF_INVALID;
          }
        } else {
          tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_NAME_VBRIE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_CONN_VNODE_IF_NAME_VBRIE:
        if (vlnk_ckv) {
          if ((*uvb_vnode1) &&
           (vlink_val->valid[UPLL_IDX_VNODE2_IF_NAME_CVLINK] == UNC_VF_VALID)) {
            uuu::upll_strncpy(tmp_vbrifx->connected_if_name,
                     vlink_val->vnode2_ifname, (kMaxLenInterfaceName+1));
            tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VBRIE] = UNC_VF_VALID;
          } else if (vlink_val->valid[UPLL_IDX_VNODE1_IF_NAME_CVLINK] ==
                                                                 UNC_VF_VALID) {
            uuu::upll_strncpy(tmp_vbrifx->connected_if_name,
                     vlink_val->vnode1_ifname, (kMaxLenInterfaceName+1));
            tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VBRIE] = UNC_VF_VALID;
          } else {
            tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VBRIE] =
                                                                 UNC_VF_INVALID;
          }
        } else {
          tmp_vbrifx->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VBRIE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_CONN_VLINK_NAME_VBRIE:
        if (!vlnk_ckv) {
          tmp_vbrifx->valid[UPLL_IDX_CONN_VLINK_NAME_VBRIE] = UNC_VF_INVALID;
        } else {
          uuu::upll_strncpy(tmp_vbrifx->connected_vlink_name,
                    vlink_key->vlink_name, (kMaxLenVlinkName+1));
          tmp_vbrifx->valid[UPLL_IDX_CONN_VLINK_NAME_VBRIE] = UNC_VF_VALID;
        }
        break;
      default :
        UPLL_LOG_DEBUG("Invalid attribute for val_vbr_if_expand");
        FREE_IF_NOT_NULL(tmp_vbrifx)
        return UPLL_RC_ERR_GENERIC;
    }
  }
  // Append the framed vbr_if expand structure to vbr_if ConfigVal.
  vbrif_ex->AppendCfgVal(IpctSt::kIpcStValVbrIfExpand, tmp_vbrifx);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::FrameVtunnelExpand(ConfigKeyVal *ikey,
                                       ConfigVal *vtun_ex) {
  UPLL_FUNC_TRACE;
  val_convert_vlink_t *vlink_val =
               reinterpret_cast<val_convert_vlink_t *>(GetVal(ikey));
  controller_domain_t ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom);
  if (!ctrlr_dom.ctrlr || !ctrlr_dom.domain)
    return UPLL_RC_ERR_GENERIC;
  val_vtunnel_expand *tmp_vtunx = ConfigKeyVal::Malloc<val_vtunnel_expand>();
  // Frame val_vtunnel_expand structure with the values read from DB.
  for (unsigned int loop = 0;
       loop < sizeof(tmp_vtunx->valid) / sizeof(tmp_vtunx->valid[0]); ++loop) {
    switch (loop) {
      case UPLL_IDX_VTUNNEL_NAME_VTNLE:
        if (vlink_val->valid[UPLL_IDX_VNODE2_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vtunx->vtunnel_name, vlink_val->vnode2_name,
                           (kMaxLenConvertVnodeName+1));
          tmp_vtunx->valid[UPLL_IDX_VTUNNEL_NAME_VTNLE] = UNC_VF_VALID;
        } else {
          tmp_vtunx->valid[UPLL_IDX_VTUNNEL_NAME_VTNLE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_CONTROLLER_ID_VTNLE:
        uuu::upll_strncpy(tmp_vtunx->controller_id, ctrlr_dom.ctrlr,
                         (kMaxLenCtrlrId+1));
        tmp_vtunx->valid[UPLL_IDX_CONTROLLER_ID_VTNLE] = UNC_VF_VALID;
        break;
      case UPLL_IDX_DOMAIN_ID_VTNLE:
        uuu::upll_strncpy(tmp_vtunx->domain_id, ctrlr_dom.domain,
                         (kMaxLenDomainId+1));
        tmp_vtunx->valid[UPLL_IDX_DOMAIN_ID_VTNLE] = UNC_VF_VALID;
        break;
      case UPLL_IDX_LABEL_VTNLE:
        if (vlink_val->valid[UPLL_IDX_LABEL_CVLINK] == UNC_VF_VALID) {
          tmp_vtunx->label = vlink_val->label;
          tmp_vtunx->valid[UPLL_IDX_LABEL_VTNLE] = UNC_VF_VALID;
        } else {
          tmp_vtunx->valid[UPLL_IDX_LABEL_VTNLE] = UNC_VF_INVALID;
        }
        break;
      default :
        UPLL_LOG_DEBUG("Invalid argument for val_vtunnel_expand");
        FREE_IF_NOT_NULL(tmp_vtunx);
        return UPLL_RC_ERR_GENERIC;
    }
  }
  // Append the framed vtunnel expand structure to vtunnel ConfigVal.
  vtun_ex->AppendCfgVal(IpctSt::kIpcStValVtunnelExpand, tmp_vtunx);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::FrameVtunIfExpand(ConfigKeyVal *ikey,
                                      ConfigVal *vtunif_ex) {
  UPLL_FUNC_TRACE;
  key_convert_vlink_t *vlink_key =
               reinterpret_cast<key_convert_vlink_t *>(ikey->get_key());
  val_convert_vlink_t *vlink_val =
               reinterpret_cast<val_convert_vlink_t *>(GetVal(ikey));
  val_vtunnel_if_expand_t *tmp_vtunifx =
                     ConfigKeyVal::Malloc<val_vtunnel_if_expand_t>();
  // Frame vtunnel_if_expand structute with the values read from DB.
  for (unsigned int loop = 0;
       loop < sizeof(tmp_vtunifx->valid) / sizeof(tmp_vtunifx->valid[0]);
       ++loop) {
    switch (loop) {
      case UPLL_IDX_IF_NAME_VTNLIE:
        if (vlink_val->valid[UPLL_IDX_VNODE2_IF_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vtunifx->if_name, vlink_val->vnode2_ifname,
                           (kMaxLenInterfaceName+1));
          tmp_vtunifx->valid[UPLL_IDX_IF_NAME_VTNLIE] = UNC_VF_VALID;
        } else {
          tmp_vtunifx->valid[UPLL_IDX_IF_NAME_VTNLIE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_CONN_VNODE_NAME_VBRIE:
        if (vlink_val->valid[UPLL_IDX_VNODE1_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vtunifx->connected_vnode_name,
                       vlink_val->vnode1_name, (kMaxLenConvertVnodeName+1));
          tmp_vtunifx->valid[UPLL_IDX_CONN_VNODE_NAME_VTNLIE] = UNC_VF_VALID;
        } else {
          tmp_vtunifx->valid[UPLL_IDX_CONN_VNODE_NAME_VTNLIE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_CONN_VNODE_IF_NAME_VTNLIE:
        if (vlink_val->valid[UPLL_IDX_VNODE1_IF_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vtunifx->connected_if_name,
                   vlink_val->vnode1_ifname, (kMaxLenInterfaceName+1));
          tmp_vtunifx->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VTNLIE] = UNC_VF_VALID;
        } else {
          tmp_vtunifx->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VTNLIE] =
                                                          UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_CONN_VLINK_NAME_VTNLIE:
        uuu::upll_strncpy(tmp_vtunifx->connected_vlink_name,
                  vlink_key->convert_vlink_name, (kMaxLenVlinkName+1));
        tmp_vtunifx->valid[UPLL_IDX_CONN_VLINK_NAME_VTNLIE] = UNC_VF_VALID;
        break;
      default :
        UPLL_LOG_DEBUG("Invalid attribute for val_vbr_if_expand");
        FREE_IF_NOT_NULL(tmp_vtunifx)
        return UPLL_RC_ERR_GENERIC;
    }
  }
  // Append the framed vtunnel_if expand structure to vtunnel_if ConfigVal.
  vtunif_ex->AppendCfgVal(IpctSt::kIpcStValVtunnelIfExpand, tmp_vtunifx);
  if (!tmp_vtunifx) {
    FREE_IF_NOT_NULL(tmp_vtunifx);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::FrameVlinkExpand(ConfigKeyVal *ikey,
                                     ConfigVal *vlink_ex) {
  UPLL_FUNC_TRACE;
  key_convert_vlink_t *vlink_key =
               reinterpret_cast<key_convert_vlink_t *>(ikey->get_key());
  val_convert_vlink_t *vlink_val =
               reinterpret_cast<val_convert_vlink_t *>(GetVal(ikey));
  val_vlink_expand *tmp_vlinkx = ConfigKeyVal::Malloc<val_vlink_expand>();
  // Frame val_vlink_expand structure with convert_vlink value read from DB.
  for (unsigned int loop = 0;
       loop < sizeof(tmp_vlinkx->valid) / sizeof(tmp_vlinkx->valid[0]);
       ++loop) {
    switch (loop) {
      case UPLL_IDX_VLINK_NAME_VLNKE:
        uuu::upll_strncpy(tmp_vlinkx->vlink_name, vlink_key->convert_vlink_name,
                         (kMaxLenVlinkName+1));
        tmp_vlinkx->valid[UPLL_IDX_VLINK_NAME_VLNKE] = UNC_VF_VALID;
        break;
      case UPLL_IDX_VNODE1_NAME_VLNKE:
        if (vlink_val->valid[UPLL_IDX_VNODE1_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vlinkx->vnode1_name, vlink_val->vnode1_name,
                           (kMaxLenConvertVnodeName+1));
          tmp_vlinkx->valid[UPLL_IDX_VNODE1_NAME_VLNKE] = UNC_VF_VALID;
        } else {
          tmp_vlinkx->valid[UPLL_IDX_VNODE1_NAME_VLNKE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_VNODE1_IF_NAME_VLNKE:
        if (vlink_val->valid[UPLL_IDX_VNODE1_IF_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vlinkx->vnode1_ifname, vlink_val->vnode1_ifname,
                           (kMaxLenInterfaceName+1));
          tmp_vlinkx->valid[UPLL_IDX_VNODE1_IF_NAME_VLNKE] = UNC_VF_VALID;
        } else {
          tmp_vlinkx->valid[UPLL_IDX_VNODE1_IF_NAME_VLNKE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_VNODE2_NAME_VLNKE:
        if (vlink_val->valid[UPLL_IDX_VNODE2_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vlinkx->vnode2_name, vlink_val->vnode2_name,
                           (kMaxLenConvertVnodeName+1));
          tmp_vlinkx->valid[UPLL_IDX_VNODE2_NAME_VLNKE] = UNC_VF_VALID;
        } else {
          tmp_vlinkx->valid[UPLL_IDX_VNODE2_NAME_VLNKE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_VNODE2_IF_NAME_VLNKE:
        if (vlink_val->valid[UPLL_IDX_VNODE2_IF_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vlinkx->vnode2_ifname, vlink_val->vnode2_ifname,
                           (kMaxLenInterfaceName+1));
          tmp_vlinkx->valid[UPLL_IDX_VNODE2_IF_NAME_VLNKE] = UNC_VF_VALID;
        } else {
          tmp_vlinkx->valid[UPLL_IDX_VNODE2_IF_NAME_VLNKE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_BOUNDARY_NAME_VLNKE:
        if (vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_CVLINK] == UNC_VF_VALID) {
          uuu::upll_strncpy(tmp_vlinkx->boundary_name, vlink_val->boundary_name,
                           (kMaxLenBoundaryName+1));
          tmp_vlinkx->valid[UPLL_IDX_BOUNDARY_NAME_VLNKE] = UNC_VF_VALID;
        } else {
          tmp_vlinkx->valid[UPLL_IDX_BOUNDARY_NAME_VLNKE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_LABEL_TYPE_VLNKE:
        if (vlink_val->valid[UPLL_IDX_LABEL_TYPE_CVLINK] == UNC_VF_VALID) {
          tmp_vlinkx->label_type = vlink_val->label_type;
          tmp_vlinkx->valid[UPLL_IDX_LABEL_TYPE_VLNKE] = UNC_VF_VALID;
        } else {
          tmp_vlinkx->valid[UPLL_IDX_LABEL_TYPE_VLNKE] = UNC_VF_INVALID;
        }
        break;
      case UPLL_IDX_LABEL_VLNKE:
        if (vlink_val->valid[UPLL_IDX_LABEL_CVLINK] == UNC_VF_VALID) {
          tmp_vlinkx->label = vlink_val->label;
          tmp_vlinkx->valid[UPLL_IDX_LABEL_VLNKE] = UNC_VF_VALID;
        } else {
          tmp_vlinkx->valid[UPLL_IDX_LABEL_VLNKE] = UNC_VF_INVALID;
        }
        break;
      default :
        UPLL_LOG_DEBUG("Invalid attribute for val_vlink_expand");
        FREE_IF_NOT_NULL(tmp_vlinkx)
        return UPLL_RC_ERR_GENERIC;
    }
  }
  // Append the framed vlink expand structure to vlink ConfigVal.
  vlink_ex->AppendCfgVal(IpctSt::kIpcStValVlinkExpand, tmp_vlinkx);
  return UPLL_RC_SUCCESS;
}
// To get the vbr_key using convert_tbl ConfigKeyVal's.
upll_rc_t VbrMoMgr::GetUnifiedVbridgeConfigKey(ConfigKeyVal *&vbr_ckv,
                                             ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  key_vbr_t *vbr_key = NULL;
  if (!parent_key || !(parent_key->get_key())) {
    UPLL_LOG_DEBUG("Input key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (vbr_ckv && (vbr_ckv->get_key())) {
    vbr_key = reinterpret_cast<key_vbr_t *>(vbr_ckv->get_key());
  } else {
    vbr_key = ConfigKeyVal::Malloc<key_vbr_t>();
  }
  void *pkey = parent_key->get_key();
  // Fill unified vbr_key's vbr_name and vtn_name using
  // the given convert_tbl configkeyval's.
  switch (parent_key->get_key_type()) {
    case UNC_KT_VBRIDGE:
      if (parent_key->get_st_num() == IpctSt::kIpcStKeyConvertVbr) {
        uuu::upll_strncpy(vbr_key->vbridge_name,
           reinterpret_cast<key_convert_vbr *>(pkey)->vbr_key.vbridge_name,
           (kMaxLenVnodeName+1));
        uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
            reinterpret_cast<key_convert_vbr *>(pkey)->vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName+1));
      }
      break;
    case UNC_KT_VBR_IF:
     if (parent_key->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
        uuu::upll_strncpy(vbr_key->vbridge_name,
                         reinterpret_cast<key_convert_vbr_if_t *>
                         (pkey)->convert_vbr_key.vbr_key.vbridge_name,
                         (kMaxLenVnodeName+1));
        uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                         reinterpret_cast<key_convert_vbr_if_t *>
                         (pkey)->convert_vbr_key.vbr_key.vtn_key.vtn_name,
                         (kMaxLenVtnName+1));
      }
      break;
    case UNC_KT_VLINK:
     if (parent_key->get_st_num() == IpctSt::kIpcStKeyConvertVlink) {
        uuu::upll_strncpy(vbr_key->vbridge_name,
                         reinterpret_cast<key_convert_vlink_t *>
                         (pkey)->vbr_key.vbridge_name,
                         (kMaxLenVnodeName+1));
        uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                         reinterpret_cast<key_convert_vlink_t *>
                         (pkey)->vbr_key.vtn_key.vtn_name,
                         (kMaxLenVtnName+1));
      }
      break;
    default:
      break;
  }
  if (!vbr_ckv)
    vbr_ckv = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                               vbr_key, NULL);
  else if (vbr_ckv->get_key() != vbr_key)
    vbr_ckv->SetKey(IpctSt::kIpcStKeyVbr, vbr_key);
  if (vbr_ckv == NULL) {
    free(vbr_key);
    res_code =  UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(vbr_ckv, parent_key);
  }
  return res_code;
}

upll_rc_t VbrMoMgr::IsRenamed(ConfigKeyVal *ikey,
                              upll_keytype_datatype_t dt_type,
                              DalDmlIntf *dmi,
                              uint8_t &rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string ctrlr_name;
  string domain_name;
  controller_domain_t ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  uint8_t tmp_flag = 0;
  GET_USER_DATA_FLAGS(ikey, tmp_flag);
  if (ctrlr_dom.ctrlr != NULL) {
    ctrlr_name = reinterpret_cast<char *>(ctrlr_dom.ctrlr);
  }
  if (ctrlr_dom.domain != NULL) {
    domain_name = reinterpret_cast<char *>(ctrlr_dom.domain);
  }

  result_code = MoMgrImpl::IsRenamed(ikey, dt_type, dmi, rename);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning %d", result_code);
    return result_code;
  }

  controller_domain_t ctr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctr_dom);
  if (ctr_dom.ctrlr && IsUnifiedVbr(ctr_dom.ctrlr)) {
    rename = tmp_flag;
    SET_USER_DATA_CTRLR(ikey, ctrlr_name.c_str());
    SET_USER_DATA_DOMAIN(ikey, domain_name.c_str());
  }
  return result_code;
}
#if 0
upll_rc_t VbrMoMgr::IsHostAddrAndPrefixLenInUse(ConfigKeyVal *ckv,
                                                DalDmlIntf *dmi,
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

  UPLL_LOG_TRACE(" existence check %s", (ckv_vbr->ToStrAll()).c_str());

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
upll_rc_t VbrMoMgr::GetChildConvertConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_convert_vbr *vbr_convert_key = NULL;
  if (okey && (okey->get_key())) {
    vbr_convert_key = reinterpret_cast<key_convert_vbr_t *>
                (okey->get_key());
  } else {
    vbr_convert_key = reinterpret_cast<key_convert_vbr *>
      (ConfigKeyVal::Malloc(sizeof(key_convert_vbr)));
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyConvertVbr,
                              vbr_convert_key, NULL);
    else if (okey->get_key() != vbr_convert_key)
      okey->SetKey(IpctSt::kIpcStKeyConvertVbr, vbr_convert_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      free(vbr_convert_key);
    return UPLL_RC_ERR_GENERIC;
  }

  /* presumes MoMgrs receive only supported keytypes */
  switch (parent_key->get_key_type()) {
    case UNC_KT_VBRIDGE:
      if (parent_key->get_st_num() == IpctSt::kIpcStKeyConvertVbr) {
        uuu::upll_strncpy(vbr_convert_key->conv_vbr_name,
           reinterpret_cast<key_convert_vbr *>(pkey)->conv_vbr_name,
           (kMaxLenConvertVnodeName+1));
        uuu::upll_strncpy(vbr_convert_key->vbr_key.vbridge_name,
           reinterpret_cast<key_convert_vbr *>(pkey)->vbr_key.vbridge_name,
           (kMaxLenVnodeName+1));
        uuu::upll_strncpy(vbr_convert_key->vbr_key.vtn_key.vtn_name,
            reinterpret_cast<key_convert_vbr *>(pkey)->vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName+1));
      } else {
        uuu::upll_strncpy(vbr_convert_key->vbr_key.vbridge_name,
                          reinterpret_cast<key_vbr *>(pkey)->vbridge_name,
                          (kMaxLenVnodeName+1));
        uuu::upll_strncpy(vbr_convert_key->vbr_key.vtn_key.vtn_name,
            reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name,
            (kMaxLenVtnName+1));
      }
      break;
    case UNC_KT_VTN:
      uuu::upll_strncpy(vbr_convert_key->vbr_key.vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name, (kMaxLenVtnName+1));
      break;
    case UNC_KT_VBR_PORTMAP:
      uuu::upll_strncpy(vbr_convert_key->vbr_key.vtn_key.vtn_name,
         reinterpret_cast<key_vbr_portmap*>(pkey)->vbr_key.vtn_key.vtn_name,
        (kMaxLenVtnName+1));
      uuu::upll_strncpy(vbr_convert_key->vbr_key.vbridge_name,
         reinterpret_cast<key_vbr_portmap*>(pkey)->vbr_key.vbridge_name,
         (kMaxLenVnodeName+1));
     break;
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyConvertVbr,
                            vbr_convert_key, NULL);
  else if (okey->get_key() != vbr_convert_key)
    okey->SetKey(IpctSt::kIpcStKeyConvertVbr, vbr_convert_key);
  if (okey == NULL) {
    free(vbr_convert_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

upll_rc_t VbrMoMgr::CreateImportConvertVbridge(ConfigKeyVal* uni_vbr_ckv,
                                               IpcReqRespHeader *req,
                                               DalDmlIntf *dmi,
                                               upll_import_type import_type,
                                               const char *ctrlr_id) {
  UPLL_FUNC_TRACE;

  if (!uni_vbr_ckv || !dmi || !req) {
    UPLL_LOG_ERROR("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }

  //  input ckv is unified vbridge ckv
  val_vbr_t *vbr_val = reinterpret_cast<val_vbr_t*>(GetVal(uni_vbr_ckv));
  if (!vbr_val || vbr_val->valid[UPLL_IDX_LABEL_VBR] != UNC_VF_VALID) {
    UPLL_LOG_DEBUG("VBID is not exist, no need to create entry in"
                   "convert vbridge table");
    return UPLL_RC_SUCCESS;
  }

  if (!(vbr_val->valid[UPLL_IDX_CONTROLLER_ID_VBR] == UNC_VF_VALID &&
        vbr_val->valid[UPLL_IDX_DOMAIN_ID_VBR] == UNC_VF_VALID)) {
    UPLL_LOG_ERROR("Received invalid controller or domain");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }

  upll_rc_t    result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *convert_vbr_ckv = NULL;
  ConfigKeyVal *dup_ckv = NULL;

  result_code = DupConfigKeyVal(dup_ckv, uni_vbr_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(dup_ckv);
    return result_code;
  }

  controller_domain_t ctrlr_dom = {NULL, NULL};
  result_code = GetControllerDomainId(uni_vbr_ckv, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(dup_ckv);
    return result_code;
  }

  //  Validate unified vbridge already exist in db. If its exist
  //  create in vtn_ctrl tbl. if it is not exist create.
  ConfigKeyVal *vbr_ckv = NULL;
  result_code = GetChildConfigKey(vbr_ckv, uni_vbr_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(dup_ckv);
    return result_code;
  }
  SET_USER_DATA_CTRLR(vbr_ckv, "#");
  SET_USER_DATA_DOMAIN(vbr_ckv, "#");

  DbSubOp dbop = {kOpReadExist, kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone};
  result_code = UpdateConfigDB(vbr_ckv, req->datatype, UNC_OP_READ, dmi,
                               &dbop, MAINTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    if (req->datatype == UPLL_DT_AUDIT) {
      result_code = CreateAuditMoImpl(dup_ckv, dmi, ctrlr_id);
    } else {
      result_code = CreateMo(req, dup_ckv, dmi);
    }
  } else {
    result_code = CheckVtnExistenceOnController(dup_ckv, req, &ctrlr_dom,
                                                false, dmi, true);
  }
  DELETE_IF_NOT_NULL(vbr_ckv);
  DELETE_IF_NOT_NULL(dup_ckv);

  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS)
    result_code = UPLL_RC_SUCCESS;

  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_INFO("Unified vBridge creation failed");
    return result_code;
  }

  //  Get vbridge convert ConfigKeyVal from unified vBridge ckv
  result_code = GetChildConvertConfigKey(convert_vbr_ckv, uni_vbr_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to get convert configkey val from unified"
                   "vbridge configkey val");
    return result_code;
  }

  //  In partial import case, received vBridge may be already exist in
  //  running database.If exist convert vBridge name is same as existing name
  if (import_type == UPLL_IMPORT_TYPE_PARTIAL) {
    DbSubOp dbop1 = {kOpReadSingle, kOpMatchCtrlr |
                    kOpMatchDomain, kOpInOutNone };
    result_code = ReadConfigDB(convert_vbr_ckv, UPLL_DT_RUNNING, UNC_OP_READ,
                               dbop1, dmi, CONVERTTBL);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE &&
        result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to read converttbl information from"
                     "running db. result_code = %d", result_code);
      DELETE_IF_NOT_NULL(convert_vbr_ckv);
      return result_code;
    }
  }
  key_convert_vbr_t *con_vbr_key = reinterpret_cast<key_convert_vbr_t*>
      (convert_vbr_ckv->get_key());
  //  Auto generate new name during full import
  //  and in partial import if it is not already exist in running
  if (import_type == UPLL_IMPORT_TYPE_FULL ||
      result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    // Generate convert vBridge based on timestamp and if the name is not unique
    //  in converttbls regenerate until getting an unique name
    while (1) {
      // Generate converted vbridge based on timestamp and
      //  if the name is not unique, Regenerate until getting an unique name
      string conv_vbr_name = unc::upll::upll_util::getTime();
      string input = conv_vbr_name;
      std::stringstream ssoutput;
      for (int i = 0; i < 7; i++)
        ssoutput << input[(strlen(input.c_str())) - (7 - i)];
      // Assign auto generated odd numbers for convert vbridge
      int autogenerateno = atoi((ssoutput.str()).c_str());
      if (autogenerateno % 2 != 0) {
        conv_vbr_name = string(reinterpret_cast<char *>(
                con_vbr_key->vbr_key.vbridge_name)) + "_" + ssoutput.str();
        UPLL_LOG_DEBUG("Auto generated convert vbridge name is %s",
            conv_vbr_name.c_str());
      } else {
        continue;
      }
      uuu::upll_strncpy(con_vbr_key->conv_vbr_name, conv_vbr_name.c_str(),
          (kMaxLenConvertVnodeName+1));
      DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutNone};
      result_code = UpdateConfigDB(convert_vbr_ckv, UPLL_DT_IMPORT, UNC_OP_READ,
                                   dmi, &dbop, CONVERTTBL);

      if ((result_code != UPLL_RC_ERR_INSTANCE_EXISTS) &&
          (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
        UPLL_LOG_TRACE("UpdateConfigDB Failed -%d", result_code);
        DELETE_IF_NOT_NULL(convert_vbr_ckv);
        return result_code;
      } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_TRACE("Convert vbr name already exists in convert_vbr_tbl");
        continue;
      }
      break;
    }
  }

  // Populate convert vbr value
  val_convert_vbr *convert_vbr_val = ConfigKeyVal::Malloc<val_convert_vbr>();
  convert_vbr_val->valid[UPLL_IDX_LABEL_CONV_VBR] = UNC_VF_VALID;
  convert_vbr_val->label   = vbr_val->label;
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValConvertVbr,
                                     convert_vbr_val);
  convert_vbr_ckv->SetCfgVal(cfg_val);

  //  converted vBridge validation is not required during audit
  if (req->datatype == UPLL_DT_IMPORT) {
    // Validate is VBID already exist in another vBridge under same VTN
    DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr |
                    kOpMatchDomain, kOpInOutNone};
    result_code = UpdateConfigDB(convert_vbr_ckv, UPLL_DT_RUNNING,
                                 UNC_OP_READ, dmi, &dbop, CONVERTTBL);
    if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(convert_vbr_ckv);
      return result_code;
    }
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code =  IsVbIdInUse(convert_vbr_ckv, dmi, req->datatype);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("VBID validation failed: result_code : %u", result_code);
        DELETE_IF_NOT_NULL(convert_vbr_ckv);
        return result_code;
      }

      // Validate transparent vBridge
      result_code = CheckUnifiedVbridgeTransparency(convert_vbr_ckv,
                                                    dmi, req->datatype);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("VBID validation failed: result_code : %u", result_code);
        DELETE_IF_NOT_NULL(convert_vbr_ckv);
        return result_code;
      }
    }
  }

  std::string cfg_vtn_name = "";
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutFlag |
                    kOpInOutDomain | kOpInOutCtrlr };
  //  Create entry vBridge converttbl
  if (req->datatype == UPLL_DT_AUDIT) {
    result_code = SetValidAudit(convert_vbr_ckv);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("SetValidAudit failed %d", result_code);
      DELETE_IF_NOT_NULL(convert_vbr_ckv);
      return result_code;
    }
    dbop1.inoutop |= kOpInOutCs;
  }
  result_code = UpdateConfigDB(convert_vbr_ckv, req->datatype, UNC_OP_CREATE,
                               dmi, &dbop1, TC_CONFIG_GLOBAL,
                               cfg_vtn_name, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Convert Vbridge creation failed : %u", result_code);
    DELETE_IF_NOT_NULL(convert_vbr_ckv);
    return result_code;
  }

  if (reinterpret_cast<val_convert_vbr_t*>(
          GetVal(convert_vbr_ckv))->label != ANY_VLAN_ID) {
    // For each convert vBridge entry alloc VBID in candidate
    VbrPortMapMoMgr *mgr = reinterpret_cast<VbrPortMapMoMgr*>(
        const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_PORTMAP)));
    if (!mgr) {
      UPLL_LOG_ERROR("Invalid momgr");
      DELETE_IF_NOT_NULL(convert_vbr_ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->AllocVbid(reinterpret_cast<key_convert_vbr_t*>(
            convert_vbr_ckv->get_key())->vbr_key.vtn_key.vtn_name,
        reinterpret_cast<val_convert_vbr_t*>(GetVal(convert_vbr_ckv))->label,
        dmi, req->datatype, TC_CONFIG_GLOBAL, "");
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
      DELETE_IF_NOT_NULL(convert_vbr_ckv);
      return result_code;
    }
  }
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    result_code = UPLL_RC_SUCCESS;
  }

  DELETE_IF_NOT_NULL(convert_vbr_ckv);
  return result_code;
}


upll_rc_t VbrMoMgr::IsVbIdInUse(ConfigKeyVal *ikey,
                                DalDmlIntf *dmi,
                                upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;

  if (!ikey || !dmi) {
    UPLL_LOG_ERROR("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }

  val_convert_vbr_t    *iconv_vbr_val = reinterpret_cast<val_convert_vbr_t*>(
                                              GetVal(ikey));
  // Semantic check not required if vBID is not VALID
  if (!iconv_vbr_val || iconv_vbr_val->valid[UPLL_IDX_LABEL_CONV_VBR] !=
      UNC_VF_VALID || iconv_vbr_val->label == ANY_VLAN_ID) {
    UPLL_LOG_DEBUG("Semantic check not required");
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t         result_code      = UPLL_RC_SUCCESS;
  ConfigKeyVal      *convert_vbr_ckv = NULL;
  key_convert_vbr_t *convert_vbr_key = NULL;
  val_convert_vbr_t *convert_vbr_val = NULL;

  // Get vbr_key from input ckv
  key_convert_vbr_t  *iconv_vbr_key = reinterpret_cast<key_convert_vbr_t*>
                            (ikey->get_key());
  if (!iconv_vbr_key) {
    return UPLL_RC_ERR_GENERIC;
  }

  // Populate convert key
  convert_vbr_key = ConfigKeyVal::Malloc<key_convert_vbr>();
  uuu::upll_strncpy(convert_vbr_key->vbr_key.vtn_key.vtn_name,
                    iconv_vbr_key->vbr_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  convert_vbr_ckv = new ConfigKeyVal(UNC_KT_VBRIDGE,
            IpctSt::kIpcStKeyConvertVbr, convert_vbr_key, NULL);
  // Populate convert val
  convert_vbr_val = ConfigKeyVal::Malloc<val_convert_vbr>();
  convert_vbr_val->valid[UPLL_IDX_LABEL_CONV_VBR] = UNC_VF_VALID;
  convert_vbr_val->label   = iconv_vbr_val->label;
  convert_vbr_ckv->AppendCfgVal(IpctSt::kIpcStValConvertVbr, convert_vbr_val);

  //  return semantic error if same vbid is already used in VTN
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(convert_vbr_ckv, dt_type , UNC_OP_READ,
                               dmi, &dbop, CONVERTTBL);
  delete convert_vbr_ckv;
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_DEBUG("More than one vbridge configured with the same "
                     "VBID in vtn!");
      return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("UpdateConfigDB Failed %d", result_code);
      return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::CheckUnifiedVbridgeTransparency(ConfigKeyVal *ikey,
                                          DalDmlIntf *dmi,
                                          upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  val_convert_vbr_t *ivbr_val = reinterpret_cast<val_convert_vbr_t*>(
                                  GetVal(ikey));
  key_convert_vbr_t *ivbr_key = reinterpret_cast<key_convert_vbr_t*>(
                                  ikey->get_key());
  // Semantic check not required if vBID is not VALID
  if (ivbr_val && ivbr_val->valid[UPLL_IDX_LABEL_CONV_VBR] == UNC_VF_VALID &&
      ivbr_val->label == ANY_VLAN_ID) {
    //  Received vBridge is transparent vBridge.
    //  Return error if more than one unified vBridge exist on the same VTN

    DalBindInfo *db_info = new DalBindInfo(uudst::kDbiVbrTbl);

    //  Bind input VTN name
    db_info->BindMatch(uudst::vbridge::kDbiVtnName, uud::kDalChar,
                       (kMaxLenVtnName + 1),
                 reinterpret_cast<void *>(ivbr_key->vbr_key.vtn_key.vtn_name));
    //  Bind input vbridge name
    db_info->BindMatch(uudst::vbridge::kDbiVbrName, uud::kDalChar,
                     (kMaxLenVnodeName + 1),
                     reinterpret_cast<void *>(ivbr_key->vbr_key.vbridge_name));

    // Bind input controller name
    db_info->BindMatch(uudst::vbridge::kDbiCtrlrName, uud::kDalChar,
                     (kMaxLenCtrlrId + 1),
                     reinterpret_cast<void *>(const_cast<char*>("#")));

    // Bind input domain name
    db_info->BindMatch(uudst::vbridge::kDbiDomainId, uud::kDalChar,
                     (kMaxLenDomainId + 1),
                     reinterpret_cast<void *>(const_cast<char*>("#")));

    std::string query_string = QUERY_IMPORT_READ_NOTEQUAL_VBRIDGE_NAME;
    upll_rc_t result_code = DalToUpllResCode(
        dmi->ExecuteAppQuerySingleRecord(query_string, db_info));
    DELETE_IF_NOT_NULL(db_info);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      if (result_code == UPLL_RC_SUCCESS) {
        result_code = UPLL_RC_ERR_CFG_SEMANTIC;
        UPLL_LOG_INFO("Received vbridge is transparent vBridge. In "
                      "UNC multiple unified vBridge exists for the same vtn");
      }
      UPLL_LOG_DEBUG("Reading vbr_tbl failed %d", result_code);
      return result_code;
    }
  } else {
    // Received vBridge is not transparent vBridge
    // Validate whether the transparent vBridge already exist on the same VTN
    key_convert_vbr_t *convert_vbr_key =
        ConfigKeyVal::Malloc<key_convert_vbr>();
    uuu::upll_strncpy(convert_vbr_key->vbr_key.vtn_key.vtn_name,
                      ivbr_key->vbr_key.vtn_key.vtn_name, (kMaxLenVtnName + 1));
    ConfigKeyVal *convert_vbr_ckv = new ConfigKeyVal(UNC_KT_VBRIDGE,
               IpctSt::kIpcStKeyConvertVbr, convert_vbr_key, NULL);
    val_convert_vbr_t *convert_vbr_val =
        ConfigKeyVal::Malloc<val_convert_vbr>();
    convert_vbr_val->valid[UPLL_IDX_LABEL_CONV_VBR] = UNC_VF_VALID;
    convert_vbr_val->label   = ANY_VLAN_ID;

    convert_vbr_ckv->AppendCfgVal(IpctSt::kIpcStValConvertVbr, convert_vbr_val);

    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
    result_code = UpdateConfigDB(convert_vbr_ckv, datatype , UNC_OP_READ,
                                 dmi, &dbop, CONVERTTBL);
    DELETE_IF_NOT_NULL(convert_vbr_ckv);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
       UPLL_LOG_INFO("Received vbridge is not transparent. In UNC"
                     " transparent vBridge portmap exists for the same VTN");
       result_code = UPLL_RC_ERR_CFG_SEMANTIC;
      }
    }
  }

  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      result_code = UPLL_RC_SUCCESS;
  return result_code;
}

upll_rc_t VbrMoMgr::MergeValidateVbid(ConfigKeyVal* ikey,
                                       upll_keytype_datatype_t dt_type,
                                       DalDmlIntf *dmi,
                                       const char *ctrlr_id,
                                       upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ikey || !dmi) {
    UPLL_LOG_ERROR("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }

  val_convert_vbr_t *iconv_vbr_val =
      reinterpret_cast<val_convert_vbr_t*>(GetVal(ikey));
  // Semantic check not required if vBID is not VALID
  if (!iconv_vbr_val ||
      iconv_vbr_val->valid[UPLL_IDX_LABEL_CONV_VBR] != UNC_VF_VALID) {
    UPLL_LOG_DEBUG("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }
  if (iconv_vbr_val->label == ANY_VLAN_ID) {
    UPLL_LOG_DEBUG("VBID in use validation not required");
    return UPLL_RC_SUCCESS;
  }

  ConfigKeyVal      *convert_vbr_ckv = NULL;
  key_convert_vbr_t *convert_vbr_key = NULL;
  val_convert_vbr_t *convert_vbr_val = NULL;

  // Get vbr_key from input ckv
  key_convert_vbr_t   *iconv_vbr_key =
      reinterpret_cast<key_convert_vbr_t*>(ikey->get_key());
  if (!iconv_vbr_key) {
    return UPLL_RC_ERR_GENERIC;
  }

  // Populate convert key
  convert_vbr_key = ConfigKeyVal::Malloc<key_convert_vbr>();
  uuu::upll_strncpy(convert_vbr_key->vbr_key.vtn_key.vtn_name,
         iconv_vbr_key->vbr_key.vtn_key.vtn_name, (kMaxLenVtnName + 1));
  convert_vbr_ckv = new ConfigKeyVal(UNC_KT_VBRIDGE,
            IpctSt::kIpcStKeyConvertVbr, convert_vbr_key, NULL);
  // Populate convert val
  convert_vbr_val = ConfigKeyVal::Malloc<val_convert_vbr>();
  convert_vbr_val->valid[UPLL_IDX_LABEL_CONV_VBR] = UNC_VF_VALID;
  convert_vbr_val->label   = iconv_vbr_val->label;
  convert_vbr_ckv->AppendCfgVal(IpctSt::kIpcStValConvertVbr, convert_vbr_val);

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr };
  result_code = ReadConfigDB(convert_vbr_ckv, dt_type, UNC_OP_READ, dbop,
                             dmi, CONVERTTBL);
  if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    if (result_code == UPLL_RC_SUCCESS) {
      if (import_type == UPLL_IMPORT_TYPE_FULL) {
        UPLL_LOG_INFO("Same VBID is already configured for another vbridge");
        DELETE_IF_NOT_NULL(convert_vbr_ckv);
        return UPLL_RC_ERR_MERGE_CONFLICT;
      } else {
        uint8_t *ctr = NULL;
        GET_USER_DATA_CTRLR(convert_vbr_ckv, ctr);
        if (ctr) {
          if (strcmp(reinterpret_cast<char*>(ctr), ctrlr_id)) {
            UPLL_LOG_DEBUG("Same VBID is configured for another "
                           "controller vbridge");
            if (strcmp(reinterpret_cast<char*>(
                        convert_vbr_key->vbr_key.vbridge_name),
                       reinterpret_cast<char*>(
                           iconv_vbr_key->vbr_key.vbridge_name))) {
              DELETE_IF_NOT_NULL(convert_vbr_ckv);
              return UPLL_RC_ERR_MERGE_CONFLICT;
            }
          }
        }
      }
    }
    DELETE_IF_NOT_NULL(convert_vbr_ckv);
    return result_code;
  }
  DELETE_IF_NOT_NULL(convert_vbr_ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::MergeValidateTransparentvBridge(ConfigKeyVal* ikey,
                                       upll_keytype_datatype_t dt_type,
                                       DalDmlIntf *dmi,
                                       const char *ctrlr_id,
                                       upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_convert_vbr_t *ivbr_val = reinterpret_cast<val_convert_vbr_t*>(
                                  GetVal(ikey));
  key_convert_vbr_t *ivbr_key = reinterpret_cast<key_convert_vbr_t*>(
                                  ikey->get_key());
  // Semantic check not required if vBID is not VALID
  if (ivbr_val && ivbr_val->valid[UPLL_IDX_LABEL_CONV_VBR] == UNC_VF_VALID &&
      ivbr_val->label == ANY_VLAN_ID) {
    //  Received vBridge is transparent vBridge.
    //  Return error if more than one unified vBridge exist on the same VTN

    DalBindInfo *db_info = new DalBindInfo(uudst::kDbiVbrTbl);

    //  Bind input VTN name
    db_info->BindMatch(uudst::vbridge::kDbiVtnName,
                     uud::kDalChar,
                     (kMaxLenVtnName + 1),
                reinterpret_cast<void *>(ivbr_key->vbr_key.vtn_key.vtn_name));
    //  Bind input vbridge name
    db_info->BindMatch(uudst::vbridge::kDbiVbrName,
                     uud::kDalChar,
                     (kMaxLenVnodeName + 1),
                     reinterpret_cast<void *>(ivbr_key->vbr_key.vbridge_name));

    // Bind input controller name
    db_info->BindMatch(uudst::vbridge::kDbiCtrlrName,
                     uud::kDalChar, (kMaxLenCtrlrId + 1),
                     reinterpret_cast<void *>(const_cast<char*>("#")));
    // Bind input domain name
    db_info->BindMatch(uudst::vbridge::kDbiDomainId,
                     uud::kDalChar, (kMaxLenDomainId + 1),
                     reinterpret_cast<void *>(const_cast<char*>("#")));

    std::string query_string = "";
    if (dt_type == UPLL_DT_CANDIDATE) {
      query_string = QUERY_CANDIDATE_READ_NOTEQUAL_VBRIDGE_NAME;
    } else {
      query_string = QUERY_IMPORT_READ_NOTEQUAL_VBRIDGE_NAME;
    }

    uint8_t vbr_name[kMaxLenVnodeName + 1];
    db_info->BindOutput(uudst::vbridge::kDbiVbrName,
                        uud::kDalChar, (kMaxLenVnodeName + 1), vbr_name);
    upll_rc_t result_code = DalToUpllResCode(
        dmi->ExecuteAppQuerySingleRecord(query_string, db_info));
    DELETE_IF_NOT_NULL(db_info);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      if (result_code == UPLL_RC_SUCCESS) {
        result_code = UPLL_RC_ERR_MERGE_CONFLICT;
        UPLL_LOG_INFO("Received vbridge is transparent vBridge.In "
                      "UNC multiple unified vBridge exists for the same vtn");
      }
      UPLL_LOG_DEBUG("Reading vbr_tbl failed %d", result_code);
      return result_code;
    }
  } else {
    // Received vBridge is not transparent vBridge
    // Validate whether the transparent vBridge already exist on the same VTN
    key_convert_vbr_t *convert_vbr_key =
        ConfigKeyVal::Malloc<key_convert_vbr>();
    uuu::upll_strncpy(convert_vbr_key->vbr_key.vtn_key.vtn_name,
                      ivbr_key->vbr_key.vtn_key.vtn_name, (kMaxLenVtnName + 1));
    ConfigKeyVal *convert_vbr_ckv = new ConfigKeyVal(UNC_KT_VBRIDGE,
               IpctSt::kIpcStKeyConvertVbr, convert_vbr_key, NULL);
    val_convert_vbr_t *convert_vbr_val =
        ConfigKeyVal::Malloc<val_convert_vbr>();
    convert_vbr_val->valid[UPLL_IDX_LABEL_CONV_VBR] = UNC_VF_VALID;
    convert_vbr_val->label   = ANY_VLAN_ID;
    convert_vbr_ckv->AppendCfgVal(IpctSt::kIpcStValConvertVbr, convert_vbr_val);

    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr };
    result_code = ReadConfigDB(convert_vbr_ckv, dt_type, UNC_OP_READ,
        dbop, dmi, CONVERTTBL);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      if (result_code == UPLL_RC_SUCCESS) {
        if (import_type == UPLL_IMPORT_TYPE_FULL) {
          UPLL_LOG_DEBUG("Received vBridge is not a transparent vBridge,"
              "transparent vBridge is configured for same VTN");
          DELETE_IF_NOT_NULL(convert_vbr_ckv);
          return UPLL_RC_ERR_MERGE_CONFLICT;
        } else {
          uint8_t *ctr = NULL;
          GET_USER_DATA_CTRLR(convert_vbr_ckv, ctr);
          if (ctr) {
            if (strcmp(reinterpret_cast<char*>(ctr), ctrlr_id)) {
              UPLL_LOG_DEBUG(" Received vBridge is not a transparent vBridge,"
                              "transparent vBridge is configured for another "
                              "controller in same VTN");
              return UPLL_RC_ERR_MERGE_CONFLICT;
            }
          }
        }
      }
      DELETE_IF_NOT_NULL(convert_vbr_ckv);
      return result_code;
    }
    DELETE_IF_NOT_NULL(convert_vbr_ckv);
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::PartialMergeValidate(unc_key_type_t keytype,
                                         const char *ctrlr_id,
                                         ConfigKeyVal *err_ckv,
                                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *run_ckv = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ctrlr_id || !err_ckv) {
    UPLL_LOG_DEBUG("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }

  //  Get Convert vbridge ConfigKeyVal
  result_code = GetChildConvertConfigKey(run_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }

  SET_USER_DATA_CTRLR(run_ckv, ctrlr_id);
  DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr, kOpInOutFlag };
  //  Get all running convert vbridge information for the controller
  //  to be merge
  result_code = ReadConfigDB(run_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop,
                             dmi, CONVERTTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code &&
      UPLL_RC_SUCCESS != result_code) {
    delete run_ckv;
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    //  In Running there is no convert vbridge information so return success
    delete run_ckv;
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *start_ckv = run_ckv;
  while (run_ckv) {
    ConfigKeyVal *import_ckv = NULL;
    result_code = GetChildConvertConfigKey(import_ckv, run_ckv);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to get convert vbr ConfigKeyVal");
      delete start_ckv;
      return result_code;
    }
    val_convert_vbr *run_val = reinterpret_cast<val_convert_vbr *>(
                                         GetVal(run_ckv));
    val_convert_vbr *imp_val = ConfigKeyVal::Malloc<val_convert_vbr_t>();
    memcpy(imp_val, run_val, sizeof(val_convert_vbr));
    import_ckv->AppendCfgVal(IpctSt::kIpcStValConvertVbr, imp_val);

    dbop.readop = kOpReadSingle;
    result_code = ReadConfigDB(import_ckv, UPLL_DT_IMPORT, UNC_OP_READ,
                               dbop, dmi, CONVERTTBL);
    DELETE_IF_NOT_NULL(import_ckv);
    if (result_code != UPLL_RC_SUCCESS) {
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         result_code = UPLL_RC_ERR_MERGE_CONFLICT;
         UPLL_LOG_INFO("Converted vbridge configuration is modified"
                       " in imported configuration");
      }
      ConfigKeyVal *unified_vbr_ckv = NULL;
      upll_rc_t lc_result_code = GetChildConfigKey(unified_vbr_ckv, run_ckv);
      if (lc_result_code != UPLL_RC_SUCCESS) {
        delete start_ckv;
        return lc_result_code;
      }
      err_ckv->ResetWith(unified_vbr_ckv);
      delete unified_vbr_ckv;
      delete start_ckv;
      // err_ckv population
      return result_code;
    }
    run_ckv = run_ckv->get_next_cfg_key_val();
  }
  delete start_ckv;
  return UPLL_RC_SUCCESS;
}

//  Below validations performed by using vBridge convert ConfigKeyVal.
//
//  case1: if same vBID is used in different KT_VBRIDGE on the same VTN
//  case2: When the given vbr_portmap is transparent and if there are
//         already existing unified vBridges for that VTN.
//  case3: When the given vbr_portmap is not transparent and if there is
//         already existing vbr_portmap which is transparent in that VTN
//  case4: When the given vbr_portmap is transparent and if there are
//         already existing vbr_portmaps which are not transparent in that VTN
//
//  case2,3,4 is enough to perform for each convert vBridge ConfigKeyVal,
//  because if port-map is transparent and vBID also transparent.To avoid
//  repeated step for each portmap
upll_rc_t VbrMoMgr::MergeValidateConvertVbridge(ConfigKeyVal *ikey,
                                                upll_keytype_datatype_t dt_type,
                                                DalDmlIntf *dmi,
                                                const char *ctrlr_id,
                                                upll_import_type import_type) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;


  //  Validate whether VBID is unique per VTN
  result_code = MergeValidateVbid(ikey, dt_type, dmi,
                                  ctrlr_id, import_type);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("VBID merge validation failed. result_code = %d",
                   result_code);
    return result_code;
  }

  //  Validate vBridge transparent
  result_code = MergeValidateTransparentvBridge(ikey, dt_type, dmi,
                                                ctrlr_id, import_type);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Transparent vBridge merge validation failed."
                   " result_code = %d", result_code);

    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
VbrMoMgr::MergeConvertVbridgeImportToCandidate(const char *ctrlr_name,
                             DalDmlIntf *dmi, upll_import_type import_type) {
  UPLL_FUNC_TRACE;

  ConfigKeyVal *imp_ckv = NULL;
  upll_rc_t result_code = GetChildConvertConfigKey(imp_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to get vBridge convert ConfigKeyVal");
    return result_code;
  }

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
  //  Read all import converted vBridge information
  result_code = ReadConfigDB(imp_ckv, UPLL_DT_IMPORT,
                             UNC_OP_READ, dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("No convert vBridge information in import DB");
      result_code = UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_ERROR("ReadConfigDb failed for impor convert vBridge");
    }
    DELETE_IF_NOT_NULL(imp_ckv);
    return result_code;
  }

  ConfigKeyVal *conv_vbr_start_ckv = imp_ckv;
  while (conv_vbr_start_ckv) {
    // For each convert vBridge entry alloc VBID in candidate
    if (reinterpret_cast<val_convert_vbr_t*>(GetVal
            (conv_vbr_start_ckv))->label != ANY_VLAN_ID) {
      VbrPortMapMoMgr *mgr = reinterpret_cast<VbrPortMapMoMgr*>(
          const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_PORTMAP)));
      if (!mgr) {
        UPLL_LOG_ERROR("Invalid momgr");
        DELETE_IF_NOT_NULL(imp_ckv);
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->AllocVbid(reinterpret_cast<key_convert_vbr_t*>(
        conv_vbr_start_ckv->get_key())->vbr_key.vtn_key.vtn_name,
        reinterpret_cast<val_convert_vbr_t*>(GetVal(conv_vbr_start_ckv))->label,
          dmi, UPLL_DT_CANDIDATE, TC_CONFIG_GLOBAL, "");
      if (result_code != UPLL_RC_SUCCESS &&
          result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
        DELETE_IF_NOT_NULL(imp_ckv);
        return result_code;
      }
    }

    DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutDomain
      | kOpInOutCtrlr };
    string temp_vtn_name = "";
    //  Create same entry in candidate
    result_code = UpdateConfigDB(conv_vbr_start_ckv, UPLL_DT_CANDIDATE,
                                 UNC_OP_CREATE, dmi, &dbop1, TC_CONFIG_GLOBAL,
                                 temp_vtn_name, CONVERTTBL);
    if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS)
      result_code = UPLL_RC_SUCCESS;
    conv_vbr_start_ckv = conv_vbr_start_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(imp_ckv);
  return result_code;
}

upll_rc_t VbrMoMgr::UnifiedVbrVnodeChecks(ConfigKeyVal *ikey,
                                          upll_keytype_datatype_t datatype,
                                          const char *ctrlr_id,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  unc_key_type_t nodes[] = {UNC_KT_VROUTER, UNC_KT_VTERMINAL,
                            UNC_KT_VUNKNOWN, UNC_KT_VTEP, UNC_KT_VTUNNEL};
  int nop = sizeof(nodes)/ sizeof(nodes[0]);
  ConfigKeyVal *ck_vnode = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  for (int indx = 0; indx < nop; indx++) {
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
        const_cast<MoManager *>(GetMoManager(nodes[indx])));
    if (!mgr) {
      UPLL_LOG_TRACE("Invalid mgr");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->CreateVnodeConfigKey(ikey, ck_vnode);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("CreateVnodeConfigKey failed - %d", result_code);
      return result_code;
    }
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr };
    result_code = mgr->ReadConfigDB(ck_vnode, datatype, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      if (result_code == UPLL_RC_SUCCESS) {
        uint8_t *ctrlr_name = NULL;
        GET_USER_DATA_CTRLR(ck_vnode, ctrlr_name);
        if (strcmp(ctrlr_id, reinterpret_cast<char*>(ctrlr_name))) {
          UPLL_LOG_INFO("vnode already exists in another vnode tbl");
          delete ck_vnode;
          return UPLL_RC_ERR_MERGE_CONFLICT;
        }
      } else {
        delete ck_vnode;
        return result_code;
      }
    }
    if (ck_vnode) {
      delete ck_vnode;
      ck_vnode = NULL;
    }
  }

  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t*>(ikey->get_key());
  if (!vbr_key) {
    UPLL_LOG_INFO("Invalid key");
    return result_code;
  }

  DalBindInfo *db_info = new DalBindInfo(uudst::kDbiVbrTbl);
  //  Bind input VTN name
  db_info->BindMatch(uudst::vbridge::kDbiVtnName, uud::kDalChar,
                     (kMaxLenVtnName + 1),
                     reinterpret_cast<void *>(vbr_key->vtn_key.vtn_name));
  //  Bind input vbridge name
  db_info->BindMatch(uudst::vbridge::kDbiVbrName, uud::kDalChar,
                     (kMaxLenVnodeName + 1),
                     reinterpret_cast<void *>(vbr_key->vbridge_name));

  uint8_t vbr_name[kMaxLenVnodeName + 1];
  db_info->BindOutput(uudst::vbridge::kDbiVbrName,
                      uud::kDalChar, (kMaxLenVnodeName + 1), vbr_name);
  // Bind input controller name
  db_info->BindMatch(uudst::vbridge::kDbiCtrlrName, uud::kDalChar,
                     (kMaxLenCtrlrId + 1),
                     reinterpret_cast<void*>(const_cast<char*>("#")));
  uint8_t ctr_name[kMaxLenCtrlrId + 1];
  db_info->BindOutput(uudst::vbridge::kDbiCtrlrName,
                      uud::kDalChar, (kMaxLenCtrlrId + 1), ctr_name);

  std::string query_string =
      "SELECT vbridge_name, ctrlr_name from ca_vbr_tbl WHERE vtn_name = ? AND "
      "vbridge_name = ? AND ctrlr_name != ?";
  result_code = DalToUpllResCode(
      dmi->ExecuteAppQuerySingleRecord(query_string, db_info));
  DELETE_IF_NOT_NULL(db_info);
  if (result_code == UPLL_RC_SUCCESS) {
    if (strcmp(reinterpret_cast<char*>(ctr_name),
               reinterpret_cast<char*>(const_cast<char*>(ctrlr_id)))) {
      UPLL_LOG_INFO("Normal vbridge is exist for the same name");
      return UPLL_RC_ERR_INSTANCE_EXISTS;
    }
  } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    return result_code;
  }

  db_info = new DalBindInfo(uudst::kDbiVbrPortMapTbl);
  //  Bind input VTN name
  db_info->BindMatch(uudst::vbridge_portmap::kDbiVtnName, uud::kDalChar,
                     (kMaxLenVtnName + 1),
                     reinterpret_cast<void *>(vbr_key->vtn_key.vtn_name));
  //  Bind input vbridge name
  db_info->BindMatch(uudst::vbridge_portmap::kDbiVbrName, uud::kDalChar,
                     (kMaxLenVnodeName + 1),
                     reinterpret_cast<void *>(vbr_key->vbridge_name));

  uint8_t pm_name[kMaxLenPortMapName + 1];
  db_info->BindOutput(uudst::vbridge_portmap::kDbiPortMapId,
                      uud::kDalChar, (kMaxLenPortMapName + 1), pm_name);

  // Bind input controller name
  db_info->BindMatch(uudst::vbridge_portmap::kDbiCtrlrName, uud::kDalChar,
                     (kMaxLenCtrlrId + 1),
                     reinterpret_cast<void*>(const_cast<char*>(ctrlr_id)));

  std::string query_string1 =
      "SELECT portmap_id from ca_vbr_portmap_tbl WHERE vtn_name = ? AND "
      "vbridge_name = ? AND ctrlr_name != ?";
  result_code = DalToUpllResCode(
      dmi->ExecuteAppQuerySingleRecord(query_string1, db_info));
  DELETE_IF_NOT_NULL(db_info);
  if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    if (result_code == UPLL_RC_SUCCESS) {
      ConfigKeyVal* vbr_pm_ckv = NULL;
      MoMgrImpl *vbr_pm_mgr = reinterpret_cast<MoMgrImpl *>(
          const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_PORTMAP)));
      result_code = vbr_pm_mgr->GetChildConfigKey(vbr_pm_ckv, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        return result_code;
      }
      key_vbr_portmap_t *vbr_pm_key =
          reinterpret_cast<key_vbr_portmap_t *>(vbr_pm_ckv->get_key());
      uuu::upll_strncpy(vbr_pm_key->vbr_key.vtn_key.vtn_name,
                        vbr_key->vtn_key.vtn_name, kMaxLenVtnName + 1);
      uuu::upll_strncpy(vbr_pm_key->vbr_key.vbridge_name,
                        vbr_key->vbridge_name, kMaxLenVnodeName + 1);
      SET_USER_DATA_CTRLR(vbr_pm_ckv, ctrlr_id);
      DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
      result_code = vbr_pm_mgr->ReadConfigDB(vbr_pm_ckv, UPLL_DT_CANDIDATE,
                                             UNC_OP_READ, dbop, dmi, MAINTBL);
      DELETE_IF_NOT_NULL(vbr_pm_ckv);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_ERR_INSTANCE_EXISTS;
        UPLL_LOG_INFO("For the same unified vBridge already portmap exists for"
                    "different controller");
      }
    } else {
      UPLL_LOG_DEBUG("Reading vbr_portmap_tbl  %d", result_code);
    }
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  if (ikey && (ikey->get_st_num() == IpctSt::kIpcStKeyConvertVbr)) {
    val_convert_vbr_t *val =
                 reinterpret_cast<val_convert_vbr_t *>(GetVal(ikey));
    if (NULL == val) {
      UPLL_LOG_DEBUG("val is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    for (unsigned int loop = 0;
          loop < sizeof(val->valid)/sizeof(val->valid[0]); ++loop ) {
      val->cs_attr[loop] = UNC_CS_NOT_APPLIED;
    }
    val->cs_row_status = UNC_CS_NOT_APPLIED;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
VbrMoMgr::UpdateUVbrConfigStatusAuditVote(ConfigKeyVal *ckv_drv_rslt,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ckv_drv_rslt || !(ckv_drv_rslt->get_key())) {
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *start_ckv = ckv_drv_rslt;
  while (start_ckv) {
    ConfigKeyVal *vbr_ckv = NULL;
    result_code = GetChildConfigKey(vbr_ckv, start_ckv);
    if (result_code != UPLL_RC_SUCCESS) {
      return result_code;
    }
    val_vbr_t *vbr_val = ConfigKeyVal::Malloc<val_vbr>();
    vbr_val->cs_row_status = UNC_CS_INVALID;

    vbr_ckv->AppendCfgVal(IpctSt::kIpcStValVbr, vbr_val);
    DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutCs };
    result_code = UpdateConfigDB(vbr_ckv, UPLL_DT_RUNNING, UNC_OP_UPDATE,
                                 dmi, &dbop, TC_CONFIG_GLOBAL, "");
    DELETE_IF_NOT_NULL(vbr_ckv);
    if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS)
      result_code = UPLL_RC_SUCCESS;
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      return result_code;
    }
    start_ckv = start_ckv->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t
VbrMoMgr::UpdateUvbrConfigStatusAuditUpdate(uuc::UpdateCtrlrPhase phase,
                                            ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi) {
  if (!ikey || !(ikey->get_key())) {
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vbr_ckv = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *conv_vbr_ckv = NULL;
  DbSubOp read_dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone};
  key_vbr_portmap_t *vbr_pm_key =
      reinterpret_cast<key_vbr_portmap_t *>(ikey->get_key());
  key_vbr_t *vbr_key = ConfigKeyVal::Malloc<key_vbr>();
  result_code = ValidateKey(reinterpret_cast<char *>(
          vbr_pm_key->vbr_key.vbridge_name),
           kMinLenVnodeName, kMaxLenVnodeName);
  if (result_code != UPLL_RC_SUCCESS) {
    key_convert_vbr_t *conv_vbr_key = ConfigKeyVal::Malloc<key_convert_vbr_t>();
    uuu::upll_strncpy(conv_vbr_key->vbr_key.vbridge_name,
                    vbr_pm_key->vbr_key.vbridge_name, (kMaxLenVnodeName+1));
    conv_vbr_ckv = new ConfigKeyVal(UNC_KT_VBRIDGE,
                    IpctSt::kIpcStKeyConvertVbr, conv_vbr_key, NULL);
    pfcdrv_val_vbr_portmap_t *drv_pm =
        reinterpret_cast<pfcdrv_val_vbr_portmap_t*>(GetVal(ikey));
    if (drv_pm->vbrpm.valid[UPLL_IDX_CONTROLLER_ID_VBRPM] == UNC_VF_VALID) {
      SET_USER_DATA_CTRLR(conv_vbr_ckv, drv_pm->vbrpm.controller_id);
      read_dbop.matchop |= kOpMatchCtrlr;
    }
    if (drv_pm->vbrpm.valid[UPLL_IDX_DOMAIN_ID_VBRPM] == UNC_VF_VALID) {
      SET_USER_DATA_DOMAIN(conv_vbr_ckv, drv_pm->vbrpm.domain_id);
      read_dbop.matchop |= kOpMatchDomain;
    }

    //  Read all import converted vBridge information
    result_code = ReadConfigDB(conv_vbr_ckv, UPLL_DT_RUNNING,
                               UNC_OP_READ, read_dbop, dmi, CONVERTTBL);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(conv_vbr_ckv);
      FREE_IF_NOT_NULL(vbr_key);
      return result_code;
    }
    if (result_code == UPLL_RC_SUCCESS) {
      uuu::upll_strncpy(vbr_key->vbridge_name,
             conv_vbr_key->vbr_key.vbridge_name, (kMaxLenVnodeName+1));
    }
    DELETE_IF_NOT_NULL(conv_vbr_ckv);
  } else {
    uuu::upll_strncpy(vbr_key->vbridge_name, vbr_pm_key->vbr_key.vbridge_name,
                      (kMaxLenVnodeName+1));
  }
  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                    vbr_pm_key->vbr_key.vtn_key.vtn_name, (kMaxLenVtnName+1));
  vbr_ckv = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                             vbr_key, NULL);
  val_vbr_t *vbr_val = ConfigKeyVal::Malloc<val_vbr>();
  vbr_val->cs_row_status = UNC_CS_INVALID;
  vbr_ckv->AppendCfgVal(IpctSt::kIpcStValVbr, vbr_val);
  SET_USER_DATA_DOMAIN(vbr_ckv, "#");
  SET_USER_DATA_CTRLR(vbr_ckv, "#");
  DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr | kOpMatchDomain, kOpInOutCs };
  result_code = UpdateConfigDB(vbr_ckv, UPLL_DT_RUNNING, UNC_OP_UPDATE,
                               dmi, &dbop, TC_CONFIG_GLOBAL, "");
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS ||
      result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
    result_code = UPLL_RC_SUCCESS;
  DELETE_IF_NOT_NULL(vbr_ckv);
  DELETE_IF_NOT_NULL(conv_vbr_ckv);

  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
