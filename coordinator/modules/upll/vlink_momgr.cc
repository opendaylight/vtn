/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vbr_if_momgr.hh"
#include "vlink_momgr.hh"
#include "ctrlr_capa_defines.hh"
#include "capa_intf.hh"
#include "upll_validation.hh"
#include "uncxx/upll_log.hh"
#include "vtn_momgr.hh"
#include "vtunnel_if_momgr.hh"
#include "vtep_if_momgr.hh"
#include "unc/uppl_common.h"
#include "config_mgr.hh"
#include "vlanmap_momgr.hh"
#include "vbr_if_flowfilter_momgr.hh"
#include "vbr_if_policingmap_momgr.hh"
#include "config_yield.hh"
#include "convert_vnode.hh"

#define NUM_KEY_RENAME_TBL_ 4
#define NUM_KEY_MAIN_TBL_ 5

#define VN1_RENAME 0x04
#define VN2_RENAME 0x08
#define IS_BOUDNARY 0x88


namespace dbl = unc::upll::dal;
namespace unc {
namespace upll {
namespace kt_momgr {

BindInfo VlinkMoMgr::vlink_bind_info[] = {
    { uudst::vlink::kDbiVtnName, CFG_KEY, offsetof(key_vlink, vtn_key),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiVlinkName, CFG_KEY, offsetof(key_vlink, vlink_name),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiAdminStatus, CFG_VAL, offsetof(val_vlink, admin_status),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiVnode1Name, CFG_VAL, offsetof(val_vlink, vnode1_name),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiVnode1Ifname, CFG_VAL, offsetof(val_vlink,
                                                        vnode1_ifname),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiVnode2Name, CFG_VAL, offsetof(val_vlink, vnode2_name),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiVnode2Ifname, CFG_VAL, offsetof(val_vlink,
                                                        vnode2_ifname),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiBoundaryName, CFG_VAL, offsetof(val_vlink,
                                                        boundary_name),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiLabelType, CFG_VAL, offsetof(val_vlink, label_type),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiLabel, CFG_VAL, offsetof(val_vlink, label),
      uud::kDalUint32, 1 },
    { uudst::vlink::kDbiDesc, CFG_VAL, offsetof(val_vlink, description),
      uud::kDalChar, 128 },
    { uudst::vlink::kDbiOperStatus, ST_VAL,
                                          offsetof(val_vlink_st, oper_status),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiDownCount, ST_VAL,
                                        offsetof(val_db_vlink_st, down_count),
      uud::kDalUint32, 1 },
    { uudst::vlink::kDbiCtrlr1Name, CK_VAL, offsetof(key_user_data, ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiDomain1Id, CK_VAL, offsetof(key_user_data, domain_id),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiCtrlr2Name, CK_VAL2, offsetof(key_user_data, ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiDomain2Id, CK_VAL2, offsetof(key_user_data, domain_id),
      uud::kDalChar, 32 },
    { uudst::vlink::kDbiValidAdminStatus, CFG_DEF_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_ADMIN_STATUS_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidVnode1Name, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_VNODE1_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidVnode1Ifname, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_VNODE1_IF_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidVnode2Name, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_VNODE2_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidVnode2Ifname, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_VNODE2_IF_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidBoundaryName, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_BOUNDARY_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidLabelType, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_LABEL_TYPE_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidLabel, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_LABEL_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidDesc, CFG_META_VAL, offsetof(
        val_vlink, valid[UPLL_IDX_DESCRIPTION_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValidOperStatus, ST_META_VAL, offsetof(
        val_vlink_st, valid[UPLL_IDX_OPER_STATUS_VLNKS]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsAdminStatus, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_ADMIN_STATUS_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsVnode1Name, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_VNODE1_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsVnode1Ifname, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_VNODE1_IF_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsVnode2Name, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_VNODE2_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsVnode2Ifname, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_VNODE2_IF_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsBoundaryName, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_BOUNDARY_NAME_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsLabelType, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_LABEL_TYPE_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsLabel, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_LABEL_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsDesc, CS_VAL, offsetof(
        val_vlink, cs_attr[UPLL_IDX_DESCRIPTION_VLNK]),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiCsRowstatus, CS_VAL, offsetof(val_vlink, cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiKeyFlags, CK_VAL, offsetof(key_user_data, flags),
      uud::kDalUint8, 1 },
    { uudst::vlink::kDbiValFlags, CK_VAL2, offsetof(key_user_data, flags),
      uud::kDalUint8, 1 }};

BindInfo VlinkMoMgr::vlink_rename_bind_info[] = {
    { uudst::vnode_rename::kDbiUncVtnName, CFG_KEY, offsetof(key_vlink_t,
                                                          vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_KEY, offsetof(key_vlink_t,
                                                            vlink_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrName, CK_VAL, offsetof(key_user_data,
                                                           ctrlr_id),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiDomainId, CK_VAL, offsetof(key_user_data,
                                                          domain_id),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrVtnName, CFG_VAL, offsetof(
        val_rename_vnode, ctrlr_vtn_name),
      uud::kDalChar, 32 },
    { uudst::vnode_rename::kDbiCtrlrVnodeName, CFG_VAL, offsetof(
        val_rename_vnode, ctrlr_vnode_name),
      uud::kDalChar, 32 } };

BindInfo VlinkMoMgr::key_vlink_maintbl_bind_info[] = {
    { uudst::vlink::kDbiVtnName, CFG_MATCH_KEY, offsetof(key_vlink_t,
                                                         vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vlink::kDbiVlinkName, CFG_MATCH_KEY, offsetof(key_vlink,
                                                           vlink_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vlink::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vlink::kDbiVlinkName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vlink::kDbiKeyFlags, CK_VAL, offsetof(key_user_data_t, flags),
      uud::kDalUint8, 1 } };

BindInfo VlinkMoMgr::key_vlink_renametbl_update_bind_info[] = {
    { uudst::vnode_rename::kDbiUncVtnName, CFG_MATCH_KEY, offsetof(
        key_vlink_t, vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_MATCH_KEY, offsetof(
        key_vlink_t, vlink_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vnode_rename::kDbiUncVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vnode_rename::kDbiUncvnodeName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 } };

BindInfo VlinkMoMgr::convert_vlink_bind_info[] = {
    { uudst::convert_vlink::kDbiVtnName, CFG_KEY,
      offsetof(key_convert_vlink, vbr_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::convert_vlink::kDbiVbrName, CFG_KEY,
      offsetof(key_convert_vlink, vbr_key.vbridge_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::convert_vlink::kDbiVlinkName, CFG_KEY,
      offsetof(key_convert_vlink, convert_vlink_name),
      uud::kDalChar, kMaxLenVlinkName + 1 },
    { uudst::convert_vlink::kDbiVnode1Name, CFG_VAL,
      offsetof(val_convert_vlink, vnode1_name),
      uud::kDalChar, kMaxLenConvertVnodeName + 1 },
    { uudst::convert_vlink::kDbiVnode1Ifname, CFG_VAL,
      offsetof(val_convert_vlink, vnode1_ifname),
      uud::kDalChar, kMaxLenInterfaceName + 1},
    { uudst::convert_vlink::kDbiVnode2Name, CFG_VAL,
      offsetof(val_convert_vlink, vnode2_name),
      uud::kDalChar, kMaxLenConvertVnodeName + 1 },
    { uudst::convert_vlink::kDbiVnode2Ifname, CFG_VAL,
      offsetof(val_convert_vlink, vnode2_ifname),
      uud::kDalChar, kMaxLenInterfaceName + 1 },
    { uudst::convert_vlink::kDbiBoundaryName, CFG_VAL,
      offsetof(val_convert_vlink, boundary_name),
      uud::kDalChar, kMaxLenBoundaryName + 1 },
    { uudst::convert_vlink::kDbiLabelType, CFG_VAL,
      offsetof(val_convert_vlink, label_type), uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiLabel, CFG_VAL,
      offsetof(val_convert_vlink, label), uud::kDalUint32, 1 },
    { uudst::convert_vlink::kDbiCtrlr1Name, CK_VAL,
      offsetof(key_user_data, ctrlr_id), uud::kDalChar, kMaxLenCtrlrId + 1 },
    { uudst::convert_vlink::kDbiDomain1Id, CK_VAL,
      offsetof(key_user_data, domain_id), uud::kDalChar, kMaxLenDomainId + 1 },
    { uudst::convert_vlink::kDbiCtrlr2Name, CK_VAL2,
      offsetof(key_user_data, ctrlr_id), uud::kDalChar, kMaxLenCtrlrId + 1 },
    { uudst::convert_vlink::kDbiDomain2Id, CK_VAL2,
      offsetof(key_user_data, domain_id), uud::kDalChar, kMaxLenDomainId + 1 },
    { uudst::convert_vlink::kDbiOperStatus, ST_VAL,
      offsetof(val_vlink_st, oper_status), uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiDownCount, ST_VAL,
      offsetof(val_db_vlink_st, down_count), uud::kDalUint32, 1 },
    { uudst::convert_vlink::kDbiValidVnode1Name, CFG_META_VAL,
      offsetof(val_convert_vlink, valid[UPLL_IDX_VNODE1_NAME_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiValidVnode1Ifname, CFG_META_VAL,
      offsetof(val_convert_vlink, valid[UPLL_IDX_VNODE1_IF_NAME_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiValidVnode2Name, CFG_META_VAL,
      offsetof(val_convert_vlink, valid[UPLL_IDX_VNODE2_NAME_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiValidVnode2Ifname, CFG_META_VAL,
      offsetof(val_convert_vlink, valid[UPLL_IDX_VNODE2_IF_NAME_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiValidBoundaryName, CFG_META_VAL,
      offsetof(val_convert_vlink, valid[UPLL_IDX_BOUNDARY_NAME_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiValidLabelType, CFG_META_VAL,
      offsetof(val_convert_vlink, valid[UPLL_IDX_LABEL_TYPE_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiValidLabel, CFG_META_VAL,
      offsetof(val_convert_vlink, valid[UPLL_IDX_LABEL_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiValidOperStatus, ST_META_VAL,
      offsetof(val_vlink_st, valid[UPLL_IDX_OPER_STATUS_VLNKS]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiCsVnode1Name, CS_VAL,
      offsetof(val_convert_vlink, cs_attr[UPLL_IDX_VNODE1_NAME_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiCsVnode1Ifname, CS_VAL,
      offsetof(val_convert_vlink, cs_attr[UPLL_IDX_VNODE1_IF_NAME_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiCsVnode2Name, CS_VAL,
      offsetof(val_convert_vlink, cs_attr[UPLL_IDX_VNODE2_NAME_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiCsVnode2Ifname, CS_VAL,
      offsetof(val_convert_vlink, cs_attr[UPLL_IDX_VNODE2_IF_NAME_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiCsBoundaryName, CS_VAL, offsetof(
        val_convert_vlink, cs_attr[UPLL_IDX_BOUNDARY_NAME_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiCsLabelType, CS_VAL, offsetof(
        val_convert_vlink, cs_attr[UPLL_IDX_LABEL_TYPE_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiCsLabel, CS_VAL, offsetof(
        val_convert_vlink, cs_attr[UPLL_IDX_LABEL_CVLINK]),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiCsRowstatus, CS_VAL,
      offsetof(val_vlink, cs_row_status),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiKeyFlags, CK_VAL,
      offsetof(key_user_data, flags),
      uud::kDalUint8, 1 },
    { uudst::convert_vlink::kDbiValFlags, CK_VAL2,
      offsetof(key_user_data, flags),
      uud::kDalUint8, 1 }};

VlinkMoMgr::VlinkMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(uudst::kDbiVlinkTbl, UNC_KT_VLINK,
                             vlink_bind_info, IpctSt::kIpcStKeyVlink,
                             IpctSt::kIpcStValVlink,
                             (uudst::vlink::kDbiVlinkNumCols));
  table[RENAMETBL] = new Table(uudst::kDbiVlinkRenameTbl, UNC_KT_VLINK,
                  vlink_rename_bind_info, IpctSt::kIpcInvalidStNum,
                  IpctSt::kIpcInvalidStNum,
                  uudst::vlink_rename::kDbiVlinkRenameNumCols);
  table[CONVERTTBL] = new Table(uudst::kDbiConvertVlinkTbl, UNC_KT_VLINK,
                  convert_vlink_bind_info, IpctSt::kIpcStKeyConvertVlink,
                  IpctSt::kIpcStValConvertVlink,
                  uudst::convert_vlink::kDbiConvertVlinkNumCols);

  table[CTRLRTBL] = NULL;
  nchild = 0;
  child = NULL;
//    SetMoManager(UNC_KT_VLINK, (MoMgr *)this);
}

upll_rc_t VlinkMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !(ikey->get_key()) || !dmi) {
    UPLL_LOG_DEBUG("Cannot perform create operation");
    return UPLL_RC_ERR_GENERIC;
  }

  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  uint8_t rename_flag = 0;
  string vtn_name = "";

  GET_USER_DATA_FLAGS(ikey, rename_flag);
  GET_RENAME_FLAG(rename_flag, UNC_KT_VLINK);
  if (rename_flag) {
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom);
  } else {
    ConfigKeyVal *ckv_rename = NULL;
    DbSubOp op = {kOpReadSingle, kOpMatchCtrlr, kOpInOutCtrlr | kOpInOutDomain};
    key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vtn)));
    memcpy(vtn_key, &(reinterpret_cast<key_vlink *>(ikey->get_key())->vtn_key),
           sizeof(key_vtn_t));
    ckv_rename = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                  vtn_key, NULL);
    SET_USER_DATA_CTRLR(ckv_rename, ctrlr_id);
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                                   (GetMoManager(UNC_KT_VTN)));
    result_code = mgr->ReadConfigDB(ckv_rename, UPLL_DT_AUDIT,
                                    UNC_OP_READ, op, dmi, CTRLRTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("ReadConfigDB from Ctrlr tbl also failed err code %d",
                    result_code);
      delete ckv_rename;
      return result_code;
    }
    GET_USER_DATA_CTRLR_DOMAIN(ckv_rename, ctrlr_dom);
    SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom);
    delete ckv_rename;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutDomain
                       | kOpInOutCtrlr };
  ConfigKeyVal *temp_key = NULL;
  result_code = GetChildConfigKey(temp_key, ikey);
  if (UPLL_RC_SUCCESS != result_code || temp_key == NULL) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed");
    return result_code;
  }
  DbSubOp dbop1 = {kOpReadSingle, kOpMatchNone,
    kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain};
  result_code = ReadConfigDB(temp_key, UPLL_DT_RUNNING,
      UNC_OP_READ, dbop1, dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    uint8_t tmp_flag = 0, flag = 0;
    GET_USER_DATA_FLAGS(temp_key, tmp_flag);
    GET_USER_DATA_FLAGS(ikey, flag);
    flag |= tmp_flag;
    SET_USER_DATA_FLAGS(ikey, flag);
    flag = 0;
    tmp_flag = 0;
    GET_USER_DATA_FLAGS(temp_key->get_cfg_val(), tmp_flag);
    GET_USER_DATA_FLAGS(ikey->get_cfg_val(), flag);
    flag |= tmp_flag;
    SET_USER_DATA_FLAGS(ikey->get_cfg_val(), flag);
  } else {
    UPLL_LOG_DEBUG("Record not found in RUNNING DB");
  }

  bool auto_rename = false;
  result_code = GenerateAutoName(ikey, UPLL_DT_AUDIT, &ctrlr_dom,
                                 dmi, &auto_rename, TC_CONFIG_GLOBAL, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Record Creation fialed in CANDIDATE DB");
    return result_code;
  }

  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE,
                               dmi, &dbop, TC_CONFIG_GLOBAL, vtn_name,
                               MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Record Creation failed in CANDIDATE DB");
  }
  DELETE_IF_NOT_NULL(temp_key);
  return result_code;
}

upll_rc_t VlinkMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || req == NULL || dmi == NULL) {
    UPLL_LOG_INFO("Cannot perform create operation"
                   "due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom[2] = { { NULL, NULL }, { NULL, NULL } };
  UPLL_LOG_TRACE("VLINK CREATE INPUT %s", (ikey->ToStrAll()).c_str());
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ValidateMessage Failed : %d", result_code);
    return result_code;
  }

  ConfigKeyVal* uppl_bdry = NULL;
  result_code = ValidateAttribute(ikey, uppl_bdry, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
     UPLL_LOG_INFO("ValidateAttribute returning %d", result_code);
     return result_code;
  }
  // vbrIf checks are done and respective Vnodes ContollerIds are filled
  if (UPLL_DT_IMPORT != req->datatype) {
    result_code = UpdateVlinkIf(req, ikey, uppl_bdry, dmi, ctrlr_dom);
    DELETE_IF_NOT_NULL(uppl_bdry);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Error in checking for Vlink Interfaces. result code %d",
                    result_code);
      return result_code;
    }
  } else {
    uint8_t rename = 0x00;
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom[0]);
    GET_USER_DATA_FLAGS(ikey, rename);
    GET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
    UPLL_LOG_TRACE("The Rename Value is %d", rename);
  }
  DELETE_IF_NOT_NULL(uppl_bdry);
  ConfigKeyVal *temp_key = NULL;
  bool is_vunk_interface = true;
  if (UPLL_DT_CANDIDATE == req->datatype ||
      UPLL_DT_IMPORT == req->datatype) {
    result_code = GetChildConfigKey(temp_key, NULL);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(temp_key);
      UPLL_LOG_DEBUG("GetChildConfigKey Failed");
      return result_code;
    }
    SET_USER_DATA_CTRLR_DOMAIN(temp_key, ctrlr_dom[0]);
    if (UNC_KT_VUNK_IF != GetVlinkVnodeIfKeyType(ikey, 0)) {
      if (ctrlr_dom[0].ctrlr == NULL) {
        DELETE_IF_NOT_NULL(temp_key);
        UPLL_LOG_TRACE(" The Node 1 interface controller name is NULL");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
      UPLL_LOG_TRACE(" The Node 1 interface controller name is %s",
                       ctrlr_dom[0].ctrlr);
      is_vunk_interface = false;
    }
  }
  if (UNC_KT_VUNK_IF != GetVlinkVnodeIfKeyType(ikey, 0) &&
      (!IsUnifiedVbr(ctrlr_dom[0].ctrlr))) {
    result_code = ValidateCapability(req, ikey, reinterpret_cast<const char *>
                                     (ctrlr_dom[0].ctrlr));
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(temp_key);
      UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
      return result_code;
    }
  }
  if (UNC_KT_VUNK_IF != GetVlinkVnodeIfKeyType(ikey, 1) &&
      (!IsUnifiedVbr(ctrlr_dom[1].ctrlr))) {
    if (ctrlr_dom[1].ctrlr == NULL) {
      DELETE_IF_NOT_NULL(temp_key);
      UPLL_LOG_TRACE(" The Node 2 interface controller name is NULL");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    UPLL_LOG_TRACE(" The Node 2 interface controller name is %s",
                     ctrlr_dom[1].ctrlr);
    if (is_vunk_interface || strncmp((const char *)ctrlr_dom[0].ctrlr,
                                     (const char *)ctrlr_dom[1].ctrlr,
                                      kMaxLenCtrlrId)) {
      result_code = ValidateCapability(req, ikey,
                       reinterpret_cast<const char *>(ctrlr_dom[1].ctrlr));
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(temp_key);
        UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
        return result_code;
      }
    }
  }
  DELETE_IF_NOT_NULL(temp_key);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  result_code = RestoreVnode(ikey, req, dmi, ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS)
    UPLL_LOG_DEBUG("Returning %d", result_code);
  return result_code;
}

upll_rc_t VlinkMoMgr::DeleteMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = MoMgrImpl::DeleteMo(req, ikey, dmi);
  return result_code;
}

upll_rc_t VlinkMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                        ConfigKeyVal *&uppl_bdry,
                                        DalDmlIntf *dmi,
                                        IpcReqRespHeader *req) {
  upll_rc_t result_code;
  if (!ikey || !ikey->get_cfg_val()) {
    UPLL_LOG_DEBUG("Invalid parameter");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vlink_t *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ikey));
  if (vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) {
    result_code = ValidateBoundary(vlink_val->boundary_name, req, uppl_bdry);
    if (result_code != UPLL_RC_SUCCESS) return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  return UPLL_RC_SUCCESS;
}

#if 0
upll_rc_t VlinkMoMgr::RestoreVnode(ConfigKeyVal *ikey,
                                   IpcReqRespHeader *req,
                                   DalDmlIntf *dmi,
                                   controller_domain_t *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dmi == NULL) {
    UPLL_LOG_DEBUG("Create error due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  /* check if vnode exists in RUNNING DB
   ** if exists : restore to CANDIDATE DB
   ** else : validate the attributes
   */
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
                               MAINTBL);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
    result_code = RestoreChildren(ikey, req->datatype, UPLL_DT_RUNNING, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Restoring children failed. Error code : %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_ERROR("Problem in reading RUNNING DB");
    return result_code;
  }

  /* If parent VTN is renamed, set the renamed flag in Vnode
   ** and create an entry in vnode rename table if VTN is renamed
   */
  result_code = SetVnodeRenameFlag(ikey, req->datatype, ctrlr_dom, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
#if 0
  /* set the controller domain in the ikey */
  SET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  if (ikey->get_key_type() == UNC_KT_VLINK) {
    if (!ikey->get_cfg_val()) return UPLL_RC_ERR_GENERIC;
    SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
  }
#endif
  // create a record in CANDIDATE DB
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutDomain
                       | kOpInOutCtrlr };
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi, &dbop,
                               MAINTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Record Creation fialed in CANDIDATE DB");
    return result_code;
  }
  return result_code;
}
#endif



upll_rc_t VlinkMoMgr::GetNodeType(void *key, bool vnode,
                                  unc_key_type_t &keytype,
                                  ConfigKeyVal *&ck_val, DalDmlIntf *dmi) {
  unc_key_type_t *ktype, if_ktype[] = { UNC_KT_VBR_IF, UNC_KT_VRT_IF,
                                        UNC_KT_VTEP_IF, UNC_KT_VTUNNEL_IF };
  unc_key_type_t vnode_ktype[] = { UNC_KT_VBRIDGE, UNC_KT_VROUTER, UNC_KT_VTEP,
                                   UNC_KT_VTUNNEL };
  int numnodes;

  if (vnode) {
    ktype = vnode_ktype;
    numnodes = sizeof(vnode_ktype) / sizeof(unc_key_type_t);
  } else {
    ktype = if_ktype;
    numnodes = sizeof(if_ktype) / sizeof(unc_key_type_t);
  }
  for (int i = 0; i < numnodes; i++) {
    keytype = ktype[i];
    MoMgrImpl *mgr =
        (reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            keytype))));
    upll_rc_t result_code = mgr->GetChildConfigKey(ck_val, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed result_code %d", result_code);
      return result_code;
    }

    if (ck_val == NULL) {
      UPLL_LOG_ERROR("Invalid param");
      return UPLL_RC_ERR_GENERIC;
    }
    switch (keytype) {
      case UNC_KT_VBRIDGE:
      case UNC_KT_VROUTER:
      case UNC_KT_VTUNNEL:
      case UNC_KT_VTEP: {
        const pfc_ipcstdef_t *key_stdef = IpctSt::GetIpcStdef(
            ck_val->get_st_num());
        if (!key_stdef) {
          UPLL_LOG_DEBUG("Invalid param");
          return UPLL_RC_ERR_GENERIC;
        }
        if (sizeof(reinterpret_cast<key_vnode_t *>(key)) != key_stdef->ist_size)
          return UPLL_RC_ERR_GENERIC;
        memcpy(ck_val->get_key(), key, sizeof(key_vnode_t));
        break;
      }
      case UNC_KT_VBR_IF:
      case UNC_KT_VRT_IF:
      case UNC_KT_VTUNNEL_IF:
      case UNC_KT_VTEP_IF: {
        const pfc_ipcstdef_t *key_stdef = IpctSt::GetIpcStdef(
            ck_val->get_st_num());
        if (!key_stdef) {
          UPLL_LOG_DEBUG("Invalid param");
          return UPLL_RC_ERR_GENERIC;
        }
        if (sizeof(reinterpret_cast<key_vbr_if_t *>(key))
                         != key_stdef->ist_size)
          return UPLL_RC_ERR_GENERIC;
        memcpy(ck_val->get_key(), key, sizeof(key_vbr_if_t));
        break;
      }
      default:
        UPLL_LOG_TRACE("Invalid Keytype");
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
    }
    DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutFlag
                          | kOpInOutDomain };
    result_code = mgr->ReadConfigDB(ck_val, UPLL_DT_STATE,
                                              UNC_OP_READ, dbop1, dmi, MAINTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      continue;
    } else if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Error in reading %d", result_code);
      return result_code;
    } else {
      return UPLL_RC_SUCCESS;
    }
  }
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

upll_rc_t VlinkMoMgr::SwapVnodes(ConfigKeyVal *&ck_vlink) {
  val_vlink * vlink_val = reinterpret_cast<val_vlink *>(GetVal(ck_vlink));
  uint8_t temp_swap[32];
  uuu::upll_strncpy(temp_swap, vlink_val->vnode1_name, (kMaxLenVnodeName+1));
  uuu::upll_strncpy(vlink_val->vnode1_name, vlink_val->vnode2_name,
                   (kMaxLenVnodeName+1));
  uuu::upll_strncpy(vlink_val->vnode2_name,  temp_swap, (kMaxLenVnodeName+1));
  uuu::upll_strncpy(temp_swap, vlink_val->vnode1_ifname,
                   (kMaxLenInterfaceName+1));
  uuu::upll_strncpy(vlink_val->vnode1_ifname, vlink_val->vnode2_ifname,
                    (kMaxLenInterfaceName+1));
  uuu::upll_strncpy(vlink_val->vnode2_ifname, temp_swap,
                   (kMaxLenInterfaceName+1));
#if 0
  int rename_flag = 0;
  int temp_flag = 0;
  GET_USER_DATA_FLAGS(ck_vlink, rename_flag);
#if 0
  if ((rename_flag & 0x30) == 0x30) temp_flag = 0x30;
  else if ((rename_flag & 0xc0) == 0x20) temp_flag = 0x20;
  else if ((rename_flag & 0xc0) == 0x10) temp_flag = 0x10;
  rename_flag |= temp_flag;
#else
  /* Swap the rename fields if vnodes renamed */
  if (rename_flag & VNODE1_RENAME) temp_flag = VNODE1_RENAME;
  if (rename_flag & VNODE2_RENAME) temp_flag |= VNODE2_RENAME;
  rename_flag &= VNODE_RENAME;
  rename_flag |= temp_flag;
#endif
  SET_USER_DATA_FLAGS(ck_vlink, rename_flag);
#endif
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::NotifyPOMForPortMapVlinkFlag(
                                              upll_keytype_datatype_t dt_type,
                                              ConfigKeyVal *ckv_if,
                                              DalDmlIntf *dmi,
                                              unc_keytype_operation_t op,
                                              TcConfigMode config_mode,
                                              string vtn_name) {
  UPLL_FUNC_TRACE;
  MoMgrImpl *pom_mgr =  NULL;
  MoMgrImpl *vnode_mgr = NULL;
  ConfigKeyVal *okey = NULL;
  if (ckv_if->get_key_type() == UNC_KT_VBR_IF) {
    pom_mgr = reinterpret_cast<MoMgrImpl *>
                                     (const_cast<MoManager *>(
                                     GetMoManager(UNC_KT_VBRIF_FLOWFILTER)));
    vnode_mgr = reinterpret_cast<MoMgrImpl *>
                                     (const_cast<MoManager *>(
                                     GetMoManager(UNC_KT_VBR_IF)));
  } else if (ckv_if->get_key_type() == UNC_KT_VRT_IF) {
    pom_mgr = reinterpret_cast<MoMgrImpl *>
                                     (const_cast<MoManager *>(
                                     GetMoManager(UNC_KT_VRTIF_FLOWFILTER)));
    vnode_mgr = reinterpret_cast<MoMgrImpl *>
                                     (const_cast<MoManager *>(
                                     GetMoManager(UNC_KT_VRT_IF)));
  } else {
    UPLL_LOG_DEBUG("Do not notify POM since kt is %d",
        ckv_if->get_key_type());
    return UPLL_RC_SUCCESS;
  }

  if (!pom_mgr || !vnode_mgr) {
    UPLL_LOG_DEBUG("Instance is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  uint8_t user_data_flags = 0;
  InterfacePortMapInfo if_info;
  GET_USER_DATA_FLAGS(ckv_if, user_data_flags);
  if (user_data_flags & VIF_TYPE_BOUNDARY) {
      UPLL_LOG_DEBUG("VLINK TYPE --BOUNDRY");
      if_info = kPortMapConfigured;
  } else if (user_data_flags & VIF_TYPE_LINKED) {
    UPLL_LOG_DEBUG("VLINK TYPE --INTERNAL VLINK");
     if_info = kVlinkConfigured;
  } else {
     UPLL_LOG_DEBUG("VLINK TYPE --NO VLINK");
     if_info = kVlinkPortMapNotConfigured;
  }
  upll_rc_t result_code = vnode_mgr->GetChildConfigKey(okey, ckv_if);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChilConfigKey Failed");
    return UPLL_RC_ERR_GENERIC;
  }
  if (UNC_OP_DELETE == op) {
    // Notify the POM Component to clear the vlink flag
     result_code = pom_mgr->SetVlinkPortmapConfiguration(okey, dt_type,
                                  dmi, kVlinkPortMapNotConfigured, op,
                                  config_mode, vtn_name);
     if (okey) {
       delete okey;
       okey = NULL;
     }
     if (UPLL_RC_SUCCESS != result_code &&
       UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
       UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d",
                    result_code);
       return result_code;
      }
  } else {
    // Notify the POM Component to update the vlink or Portmap flag
     result_code = pom_mgr->SetVlinkPortmapConfiguration(okey, dt_type,
                                            dmi, if_info, op,
                                            config_mode, vtn_name);

     if (okey) {
       delete okey;
       okey = NULL;
     }
     if (UPLL_RC_SUCCESS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d",
                    result_code);
        return result_code;
     }
     if ((if_info == kPortMapConfigured) &&
        (ckv_if->get_key_type() == UNC_KT_VBR_IF)) {
        pom_mgr = reinterpret_cast<MoMgrImpl *>
                                       (const_cast<MoManager *>(
                                       GetMoManager(UNC_KT_VBRIF_POLICINGMAP)));
        if (!pom_mgr) {
           UPLL_LOG_DEBUG("Instance is NULL");
           return UPLL_RC_ERR_GENERIC;
        }
        result_code = vnode_mgr->GetChildConfigKey(okey, ckv_if);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("GetChilConfigKey Failed");
          return UPLL_RC_ERR_GENERIC;
        }

       // Notify the VBR IF PolicingMap Component to update the Portmap flag
       result_code = pom_mgr->SetVlinkPortmapConfiguration(okey, dt_type,
                                                         dmi, if_info, op,
                                                         config_mode, vtn_name);
       if (okey) {
         delete okey;
         okey = NULL;
       }
       if (UPLL_RC_SUCCESS != result_code &&
         UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
         UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d",
                     result_code);
         return result_code;
       }
     }
  }

  return UPLL_RC_SUCCESS;
}


upll_rc_t VlinkMoMgr::UpdateVlinkMemIfFlag(upll_keytype_datatype_t dt_type,
                                           ConfigKeyVal *ckv_if,
                                           DalDmlIntf *dmi,
                                           vnode_if_type &vnif_type,
                                           MoMgrImpl *mgr,
                                           unc_keytype_operation_t op,
                                           TcConfigMode config_mode,
                                           string vtn_name) {
  UPLL_FUNC_TRACE;
  if (!ckv_if || !mgr) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  // ConfigKeyVal *okey = NULL;
  ConfigKeyVal *dup_ckvif = NULL;
  upll_rc_t result_code = mgr->DupConfigKeyVal(dup_ckvif, ckv_if, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Dupkey failed");
    return result_code;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
  void *if_val = GetVal(dup_ckvif);
  if (!if_val) {
     UPLL_LOG_DEBUG("Invalid param");
     if (dup_ckvif) delete dup_ckvif;
     return UPLL_RC_ERR_GENERIC;
  }
  switch (ckv_if->get_key_type()) {
  case UNC_KT_VBR_IF: {
     ResetValid(val_vbr_if, &(reinterpret_cast<val_drv_vbr_if *>
                              (if_val)->vbr_if_val))
     vnif_type = kVbrIf;
     break;
     }
  case UNC_KT_VRT_IF:
     {
      ResetValid(val_vrt_if, if_val);
      vnif_type = kVrtIf;
      break;
    }
  case UNC_KT_VTEP_IF:
     ResetValid(val_vtep_if, if_val);
     vnif_type = kVtepIf;
     break;
  case UNC_KT_VTUNNEL_IF:
     ResetValid(val_vtunnel_if, if_val);
     vnif_type = kVtunnelIf;
     break;
  case UNC_KT_VUNK_IF:
     ResetValid(val_vunk_if, if_val);
     vnif_type = kVunkIf;
     break;
  /* VlanmapOnBoundary: vlanmap case added */
  case UNC_KT_VBR_VLANMAP:
     ResetValid(val_vlan_map, &(reinterpret_cast<pfcdrv_val_vlan_map_t *>
               (if_val)->vm));
     vnif_type = kVlanMap;
     break;
  case UNC_KT_VBR_PORTMAP:
     ResetValid(val_vlan_map, &(reinterpret_cast<pfcdrv_val_vbr_portmap_t *>
               (if_val)->vbrpm));
     vnif_type = kVbrPortMap;
     break;

  default:
     if (dup_ckvif) delete dup_ckvif;
     UPLL_LOG_DEBUG("Unsupported keytype %d", ckv_if->get_key_type());
     return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("Updating the Flag value in Interface Table %d",
                           ckv_if->get_key_type());
  result_code = mgr->UpdateConfigDB(dup_ckvif, dt_type,
                      UNC_OP_UPDATE, dmi, &dbop, config_mode,
                      vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS)
    UPLL_LOG_DEBUG("Returning error %d", result_code);
  if (dup_ckvif) delete dup_ckvif;
  return result_code;
}

upll_rc_t VlinkMoMgr::UpdateVlinkIf(IpcReqRespHeader *req,
                                    ConfigKeyVal *ikey,
                                    ConfigKeyVal *uppl_bdry,
                                    DalDmlIntf *dmi,
                                    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool bound_vlink = false;
  unc_key_type_t key_type;
  if (!req || !ikey || (ikey->get_key_type() != UNC_KT_VLINK)) {
    UPLL_LOG_INFO("Invalid ConfigKeyVal parameter");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  uint8_t valid_boundary =
      reinterpret_cast<val_vlink *>(GetVal(ikey))->
                            valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
  uint8_t rename_flag = 0;
  uint8_t vnode_rename_flag = 0;
  VlinkNodePosition vnodeif_number = kVlinkVnode1;
  unc_key_type_t if1_type = UNC_KT_ROOT, if2_type = UNC_KT_ROOT;
  ConfigKeyVal *ck_if[2] = { NULL, NULL };
  ConfigKeyVal *ck_drv_vbr_if = NULL;
  ConfigVal *cv_link;
  int i = 0;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  VbrIfMoMgr *mgr =
      static_cast<VbrIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VBR_IF)));
  // val is mandatory in vlink structure
  cv_link = ikey->get_cfg_val();
  if (!cv_link) return UPLL_RC_ERR_GENERIC;
  do {
    GET_USER_DATA_FLAGS(cv_link, rename_flag);
    rename_flag |= (uint8_t)(vnodeif_number & VIF_TYPE);
    SET_USER_DATA_FLAGS(cv_link, rename_flag);
    result_code = mgr->GetChildConfigKey(ck_drv_vbr_if, ikey);
    rename_flag &= ~VIF_TYPE;
    SET_USER_DATA_FLAGS(cv_link, rename_flag);
    if (!ck_drv_vbr_if || (result_code != UPLL_RC_SUCCESS))
      return UPLL_RC_ERR_GENERIC;
    void *if_key = ck_drv_vbr_if->get_key();
    if (!if_key) {
      delete ck_drv_vbr_if;  // COV RESOURCE LEAK
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = GetVnodeType(if_key, false, key_type, ck_if[i],
                               dmi, dt_type);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Invalid Vlink if");
      for (int i =0; i <2; i++)
        DELETE_IF_NOT_NULL(ck_if[i]);
      delete ck_drv_vbr_if;  // COV RESOURCE LEAK
      return (result_code);
    }
    if (vnodeif_number == kVlinkVnode1) if1_type = key_type;
    else if (vnodeif_number == kVlinkVnode2) if2_type = key_type;
    GET_USER_DATA_CTRLR_DOMAIN(ck_if[i], ctrlr_dom[i]);
    UPLL_LOG_TRACE(" The Controller and Domain Name is %s & %s",
                     ctrlr_dom[i].ctrlr, ctrlr_dom[i].domain);
    //  During audit, same vlink is exist in both
    //  UNC and PFC. vnode interface vlink flag is set during vnode interface
    //  CreateAuditMoImpl from running to audit
    if (req->datatype != UPLL_DT_AUDIT) {
      GET_USER_DATA_FLAGS(ck_if[i], rename_flag);
      if (rename_flag & VIF_TYPE) {
        UPLL_LOG_DEBUG("Interface is already part of another vlink");
        delete ck_drv_vbr_if;  // COV RESOURCE LEAK
        for (int i =0; i <2; i++)
          DELETE_IF_NOT_NULL(ck_if[i]);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
      GET_USER_DATA_FLAGS(ck_if[i], vnode_rename_flag);
      if (vnode_rename_flag & 0x02) {
        GET_USER_DATA_FLAGS(cv_link, rename_flag);
        if (vnodeif_number == kVlinkVnode2)
          vnode_rename_flag = 0x08;
        else
          vnode_rename_flag = 0x04;
        rename_flag |= vnode_rename_flag;
        SET_USER_DATA_FLAGS(cv_link, rename_flag);
      }
    }
    delete ck_drv_vbr_if;
    ck_drv_vbr_if = NULL;
    vnodeif_number = (vnodeif_number == kVlinkVnode1) ?
                       kVlinkVnode2 : kVlinkVnode1;
    i++;
  } while (vnodeif_number != kVlinkVnode1);
  if ((if1_type == UNC_KT_VUNK_IF) || (if2_type == UNC_KT_VUNK_IF)) {
    bound_vlink = true;
  } else {
    if (!ctrlr_dom[0].ctrlr || !ctrlr_dom[1].ctrlr) {
      UPLL_LOG_DEBUG("Invalid ctrlr");
      for (int i =0; i <2; i++)
        DELETE_IF_NOT_NULL(ck_if[i]);
      return UPLL_RC_ERR_GENERIC;
    }
    if ((strncmp((const char *)ctrlr_dom[0].ctrlr,
                 (const char *)ctrlr_dom[1].ctrlr, kMaxLenCtrlrId)) ||
        (strncmp((const char *)ctrlr_dom[0].domain,
                 (const char *)ctrlr_dom[1].domain, kMaxLenDomainId))) {
      bound_vlink = true;
    }
  }
  if (bound_vlink) {
    UPLL_LOG_DEBUG("Boundary vlink");
    val_vlink_t *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ikey));
    if (vlink_val && vlink_val->admin_status == UPLL_ADMIN_DISABLE) {
      UPLL_LOG_ERROR("Boundary vlink cannot be shut\n");
      for (int i =0; i <2; i++)
        DELETE_IF_NOT_NULL(ck_if[i]);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    if ((if1_type == UNC_KT_VRT_IF) || (if2_type == UNC_KT_VRT_IF)) {
      UPLL_LOG_DEBUG("vrt link is not supported on a boundary");
      for (int i =0; i <2; i++)
        DELETE_IF_NOT_NULL(ck_if[i]);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  } else {
    if ((if1_type == UNC_KT_VBR_IF) && (if2_type == UNC_KT_VBR_IF)) {
      UPLL_LOG_DEBUG("Internal link between 2 vbridges not allowed");
      for (int i =0; i <2; i++)
        DELETE_IF_NOT_NULL(ck_if[i]);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  }
  // Validate BoundaryCombination
  result_code = ValidateIfType(ck_if);
  if (result_code != UPLL_RC_SUCCESS) {
    for (int i =0; i <2; i++)
      DELETE_IF_NOT_NULL(ck_if[i]);
    return result_code;
  }
  if ((if1_type == UNC_KT_VRT_IF) && (if2_type == UNC_KT_VBR_IF)) {
    result_code = SwapVnodes(ikey);
    ConfigKeyVal *tmp = ck_if[0];
    ck_if[0] = ck_if[1];
    ck_if[1] = tmp;
    GET_USER_DATA_CTRLR_DOMAIN(ck_if[0], ctrlr_dom[0]);
    GET_USER_DATA_CTRLR_DOMAIN(ck_if[1], ctrlr_dom[1]);
  }
  vnode_if_type vnif_type[2];
  for (int i = 0; i < 2; i++) {
  // Set the flag - 2 MSB for boundary and next 2 for internal
    GET_USER_DATA_FLAGS(ck_if[i], rename_flag);
    if (bound_vlink) {
      rename_flag |= (i == 0) ?
        kVlinkBoundaryNode1 : kVlinkBoundaryNode2;
      SET_USER_DATA_FLAGS(ck_if[i], rename_flag);
    } else {
      rename_flag |= (i == 0) ? kVlinkInternalNode1 :
              kVlinkInternalNode2;
      // Notify the POM Component to update the Internal Vlink Flag

      SET_USER_DATA_FLAGS(ck_if[i], rename_flag);
      result_code = NotifyPOMForPortMapVlinkFlag(dt_type,
                                            ck_if[i],
                                            dmi,
                                            UNC_OP_CREATE,
                                            config_mode,
                                            vtn_name);

      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        for (int i =0; i <2; i++)
        DELETE_IF_NOT_NULL(ck_if[i]);
        return result_code;
      }
    }
    // SET_USER_DATA_FLAGS(ck_if[i], rename_flag);
    key_type = ck_if[i]->get_key_type();
    MoMgrImpl *if_mgr = reinterpret_cast<MoMgrImpl *>
                 (const_cast<MoManager *>(GetMoManager(key_type)));

    result_code = UpdateVlinkMemIfFlag(dt_type, ck_if[i],
                               dmi, vnif_type[i], if_mgr, UNC_OP_CREATE,
                               config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      for (int i =0; i <2; i++)
        DELETE_IF_NOT_NULL(ck_if[i]);
      return result_code;
    }
  }
  /* Reset the pointers to ikey as the current stored pointers
   * point to ck_if which is going to get deleted.
   */
  if (if1_type != UNC_KT_VUNK_IF) {
    SET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  } else {
    /* For vbypass or unknown, only domain info needs to be copied */
    SET_USER_DATA_DOMAIN(ikey, ctrlr_dom[0].domain);
    GET_USER_DATA_DOMAIN(ikey, ctrlr_dom[0].domain);
  }
  if (if2_type != UNC_KT_VUNK_IF) {
    SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
    GET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
  } else {
    /* For vbypass or unknown, only domain info needs to be copied */
    SET_USER_DATA_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1].domain);
    GET_USER_DATA_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1].domain);
  }
  /* Set the vnode if types in first 6 bits of flag */
  uint8_t flag = 0;
  GET_USER_DATA_FLAGS(ikey, flag);
  /* clear the iftype bits and preserve the rename bits */
  flag &= ~VLINK_FLAG_NODE_TYPE;
  flag |= (vnif_type[0] << kVlinkVnodeIf1Type);
  flag |= (vnif_type[1] << kVlinkVnodeIf2Type);
  SET_USER_DATA_FLAGS(ikey, flag);
  UPLL_LOG_DEBUG("Flags for vnode1if type %d vnode2if type %d value %d",
                  vnif_type[0], vnif_type[1], flag);
  if (valid_boundary) {
    UPLL_LOG_TRACE("Valid boundary");

    /* VlanmapOnBoundary: In case of boundary vlan-map,
     * update vnode_if type as kVlanmap in vlink flag.
     * In case of boundary vbr-portmap update vnode if type
     * as KVbrPortMap in vlink flag */
    result_code = UpdateVlinkFlagFromBoundary(ikey, uppl_bdry);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("UpdateVlinkMemIfFlag returns error:%d", result_code);
      for (int i =0; i <2; i++)
        DELETE_IF_NOT_NULL(ck_if[i]);
      return result_code;
    }

    ConfigKeyVal *dup_vlink = NULL;
    result_code = DupConfigKeyVal(dup_vlink, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("DupConfigKeyVal returns error:%d", result_code);
      for (int i =0; i <2; i++)
        DELETE_IF_NOT_NULL(ck_if[i]);
      return result_code;
    }

    VlinkNodePosition vnode_pos_number = kVlinkVnode1;
    for (int i = 0; i < 2; i++) {
      /* Get the key type of vnode_if */
      unc_key_type_t ktype;
      ktype = GetVlinkVnodeIfKeyType(dup_vlink, i);
      if (ktype == UNC_KT_VBR_VLANMAP || ktype == UNC_KT_VBR_PORTMAP) {
        MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
            (GetMoManager(ktype)));
        if (!mgr) {
          UPLL_LOG_DEBUG("Invalid MoMgr:%d", ktype);
          for (int i =0; i <2; i++)
            DELETE_IF_NOT_NULL(ck_if[i]);
          delete dup_vlink;
          return UPLL_RC_ERR_GENERIC;
        }

        ConfigKeyVal *vnode_child_ckv = NULL;
        /* Set the flag of cfg_val with vnode_number which will be used in
         * vbr vlan-map or vbr_portmap GetChildConfigKey to populate
         * vbr_vlanamp or vbr_portmap ckv respectively from vLink ckv */
        SET_USER_DATA_FLAGS(dup_vlink->get_cfg_val(), vnode_pos_number);

        result_code = mgr->GetChildConfigKey(vnode_child_ckv, dup_vlink);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning %d", result_code);
          delete dup_vlink;
          for (int i =0; i <2; i++)
            DELETE_IF_NOT_NULL(ck_if[i]);
          return result_code;
        }

        val_vlink_t *dup_val =
            reinterpret_cast<val_vlink_t *>(GetVal(dup_vlink));

        // valid flag of db_vlink boundary_name is INVALID,
        // Boundary vlanmap CREATE req
        dup_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] = UNC_VF_INVALID;
        dup_val->valid[UPLL_IDX_LABEL_VLNK]       = UNC_VF_INVALID;

        /* Create request for BoundaryMap */
        result_code = mgr->BoundaryMapReq(req, ikey, dup_vlink,
                                          vnode_child_ckv, uppl_bdry, dmi);
        DELETE_IF_NOT_NULL(vnode_child_ckv);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("BoundaryMapReq returns error: %d", result_code);
          delete dup_vlink;
          for (int i =0; i <2; i++)
            DELETE_IF_NOT_NULL(ck_if[i]);
          return result_code;
        }
      }
      vnode_pos_number = kVlinkVnode2;
    }
    DELETE_IF_NOT_NULL(dup_vlink);
    /* boundary vbr_portmap/vbr_vlanmap end */

    result_code = UpdateVnodeIf(dt_type, ikey, ck_if, dmi, req->operation,
                                uppl_bdry, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
    }
  }
  for (int i = 0; i <2; i++)
    DELETE_IF_NOT_NULL(ck_if[i]);
  return result_code;
}
// Validating Boundary Combination
upll_rc_t VlinkMoMgr::ValidateIfType(ConfigKeyVal **vnodeIf) {
  UPLL_FUNC_TRACE;
  if (!(GetVal(vnodeIf[0])) || !(GetVal(vnodeIf[1])))
    return UPLL_RC_ERR_GENERIC;

  unc_key_type_t node1_ktype = vnodeIf[0]->get_key_type();
  unc_key_type_t node2_ktype = vnodeIf[1]->get_key_type();
  controller_domain_t ctr_dom;
  ctr_dom.ctrlr = ctr_dom.domain = NULL;

  //  Unified vBridge interfcae cannot be linked to VRT_IF/VUNK_IF/
  //  VTEP_IF/VTUNNEL_IF
  if ((node1_ktype == UNC_KT_VUNK_IF) || (node1_ktype == UNC_KT_VRT_IF) ||
      (node1_ktype == UNC_KT_VTUNNEL_IF) ||
      (node1_ktype == UNC_KT_VTEP_IF)) {
    if (node2_ktype == UNC_KT_VBR_IF) {
      GET_USER_DATA_CTRLR_DOMAIN(vnodeIf[1], ctr_dom);
      if (ctr_dom.ctrlr) {
        if (IsUnifiedVbr(ctr_dom.ctrlr)) {
          UPLL_LOG_DEBUG("vLink cannot be configured between unified vBridge"
                         "interfcae and %d", node2_ktype);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    }
  }
  switch (node1_ktype) {
    case UNC_KT_VUNK_IF:
      if (node2_ktype == UNC_KT_VRT_IF) {
        UPLL_LOG_DEBUG("Invalid combination for vlink");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    break;
    case UNC_KT_VRT_IF:
      if ((node2_ktype == UNC_KT_VUNK_IF) || (node2_ktype == UNC_KT_VRT_IF) ||
          (node2_ktype == UNC_KT_VTUNNEL_IF) ||
          (node2_ktype == UNC_KT_VTEP_IF)) {
        UPLL_LOG_DEBUG("Invalid combination for vlink");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    break;
    case UNC_KT_VTUNNEL_IF:
    case UNC_KT_VTEP_IF:
      if ((node2_ktype == UNC_KT_VRT_IF) || (node2_ktype == UNC_KT_VTEP_IF) ||
          (node2_ktype == UNC_KT_VTUNNEL_IF)) {
        UPLL_LOG_DEBUG("Invalid combination for vlink");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    break;
    case UNC_KT_VBR_IF:
      GET_USER_DATA_CTRLR_DOMAIN(vnodeIf[0], ctr_dom);
      if (ctr_dom.ctrlr && IsUnifiedVbr(ctr_dom.ctrlr)) {
        //  node1 is unified vBridge interface, return UPLL_RC_ERR_CFG_SEMANTIC
        //  error if node2 is other than unified vBridge interface
        if ((node2_ktype == UNC_KT_VUNK_IF) || (node2_ktype == UNC_KT_VRT_IF) ||
            (node2_ktype == UNC_KT_VTUNNEL_IF) ||
            (node2_ktype == UNC_KT_VTEP_IF)) {
          UPLL_LOG_DEBUG("vLink cannot be configured between unified vBridge"
                         "interfcae and %d", node2_ktype);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        } else if (node2_ktype == UNC_KT_VBR_IF) {
          //  If node1 is unified vBridge node2 also unified vBridge
          GET_USER_DATA_CTRLR_DOMAIN(vnodeIf[1], ctr_dom);
          if (ctr_dom.ctrlr && IsUnifiedVbr(ctr_dom.ctrlr)) {
            UPLL_LOG_DEBUG("vLink cannot be configured between unified vBridge"
                           "interfcae and normal vBridge interface");
            return UPLL_RC_ERR_CFG_SEMANTIC;
          }
        }
      }
    break;
    default:
      UPLL_LOG_DEBUG("Valid Combination");
  }
  // Mapped Interface Valdiattion
  UPLL_LOG_TRACE("Mapped Interface Validation");
  val_vtunnel_if *vtunnelif_val = NULL;
  if (node1_ktype == UNC_KT_VTUNNEL_IF)
    vtunnelif_val = reinterpret_cast<val_vtunnel_if *>(GetVal(vnodeIf[0]));
  else if (node2_ktype == UNC_KT_VTUNNEL_IF)
    vtunnelif_val = reinterpret_cast<val_vtunnel_if *>(GetVal(vnodeIf[1]));
  if (vtunnelif_val)
    if (vtunnelif_val->valid[UPLL_IDX_PORT_MAP_VTNL_IF] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("Tunnel interface is already mapped");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  val_vtep_if *vtepif_val = NULL;
  if (node1_ktype == UNC_KT_VTEP_IF)
    vtepif_val = reinterpret_cast<val_vtep_if *>(GetVal(vnodeIf[0]));
  else if (node2_ktype == UNC_KT_VTEP_IF)
    vtepif_val = reinterpret_cast<val_vtep_if *>(GetVal(vnodeIf[1]));
  if (vtepif_val)
    if (vtepif_val->valid[UPLL_IDX_PORT_MAP_VTEPI] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("Tep interface is already mapped");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  val_vbr_if *vbrif_val = NULL;
  if (node1_ktype == UNC_KT_VBR_IF)
    vbrif_val = reinterpret_cast<val_vbr_if *>(GetVal(vnodeIf[0]));
  else if (node2_ktype == UNC_KT_VBR_IF)
    vbrif_val = reinterpret_cast<val_vbr_if *>(GetVal(vnodeIf[1]));
  if (vbrif_val)
    if (vbrif_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("Vbridge interface is already mapped");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  return UPLL_RC_SUCCESS;
}

/* Pure Virtual functions from MoMgrImpl */
upll_rc_t VlinkMoMgr::GetControllerDomainId(ConfigKeyVal *ikey,
                                          controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (!ikey || !ctrlr_dom) {
    UPLL_LOG_INFO("Illegal parameter");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_user_data()) {
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
    if ((ctrlr_dom->ctrlr &&
         !strlen(reinterpret_cast<const char *>(ctrlr_dom->ctrlr))) ||
        (ctrlr_dom->domain &&
        !strlen(reinterpret_cast<const char *>(ctrlr_dom->domain)))) {
      UPLL_LOG_DEBUG("Ctrlr domain null");
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_DEBUG("ctrlr_dom %s %s", ctrlr_dom->ctrlr, ctrlr_dom->domain);
  }
  if (ikey->get_cfg_val() && ikey->get_cfg_val()->get_user_data()) {
    GET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
    if ((ctrlr_dom[1].ctrlr &&
         !strlen(reinterpret_cast<const char *>(ctrlr_dom[1].ctrlr))) ||
        (ctrlr_dom[1].domain &&
        !strlen(reinterpret_cast<const char *>(ctrlr_dom[1].domain)))) {
      UPLL_LOG_DEBUG("Ctrlr domain null");
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_DEBUG("boundary 2nd ctrlr_dom %s %s",
                    ctrlr_dom[1].ctrlr, ctrlr_dom[1].domain);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::UpdateVnodeIf(upll_keytype_datatype_t dt_type,
                      ConfigKeyVal *ikey, ConfigKeyVal **vnif,
                      DalDmlIntf *dmi,
                      unc_keytype_operation_t op,
                      ConfigKeyVal *uppl_bdry,
                      TcConfigMode config_mode,
                      string vtn_name) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dt_type == UPLL_DT_INVALID || dmi == NULL)
    return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t valid_boundary =
      reinterpret_cast<val_vlink *>(GetVal(ikey))->
                                valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
  if ((valid_boundary == UNC_VF_VALID) ||
    (valid_boundary == UNC_VF_VALID_NO_VALUE)) {
    for (int i = 0; i < 2; i++) {
      /* VlanmapOnBoundary: for vlanmap continue */
      if ((GetVlinkVnodeIfKeyType(ikey, i) == UNC_KT_VBR_VLANMAP) ||
          (GetVlinkVnodeIfKeyType(ikey, i) == UNC_KT_VBR_PORTMAP))
        continue;

      UPLL_LOG_TRACE("Before UpdatePortmap %s", (vnif[i]->ToStrAll()).c_str());
      result_code = UpdateVnodeIfPortmap(ikey, vnif[i], dmi, dt_type,
                                         op, valid_boundary, uppl_bdry,
                                         config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Updation of VnIf Portmap failed %d", result_code);
        return result_code;
      }
    }
  } else {
      UPLL_LOG_DEBUG("Internal vlink");
  }
#if 0
  if (ck_boundary)
    delete ck_boundary;
  ck_boundary= NULL;
#endif
  return result_code;
}

#if 0
upll_rc_t VlinkMoMgr::CheckPortmapValidandUpdateVbrIf(
    ConfigKeyVal *ikey, ConfigKeyVal *ck_drv_vbr_if, DalDmlIntf *dmi,
                                     upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || ck_boundary == NULL || dmi == NULL
      || ck_drv_vbr_if == NULL || dt_type == UPLL_DT_INVALID)
    return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VbrIfMoMgr *mgr = reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager*>
                                              (GetMoManager(UNC_KT_VBR_IF)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t valid_port = reinterpret_cast<val_drv_vbr_if *>
               (GetVal(ck_drv_vbr_if))->vbr_if_val.valid[UPLL_IDX_PM_DRV_VBRI];
  if (valid_port == UNC_VF_INVALID) {
    result_code = ConverttoDriverPortMap(ck_drv_vbr_if, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      return result_code;
    }
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
    result_code = mgr->UpdateConfigDB(ck_drv_vbr_if, dt_type, UNC_OP_UPDATE,
                         dmi, &dbop, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Updation failed %d", result_code);
        return result_code;
    }
  } else if (valid_port == UNC_VF_VALID) {
#if 0
    if (CompareVbrIfWithPhysicalData(ck_drv_vbr_if, ck_boundary, ikey)) {
        UPLL_LOG_DEBUG("VbrIf is already configured");
        return UPLL_RC_ERR_GENERIC;
    }
#endif
  }
  return result_code;
}
#endif

upll_rc_t VlinkMoMgr::UpdateVnodeIfPortmap(
    ConfigKeyVal *ikey, ConfigKeyVal *ck_vn_if, DalDmlIntf *dmi,
                                     upll_keytype_datatype_t dt_type,
                                     unc_keytype_operation_t op,
                                     uint8_t valid_boundary,
                                     ConfigKeyVal *uppl_bdry,
                                     TcConfigMode config_mode,
                                     string vtn_name) {
  UPLL_FUNC_TRACE;
  // ck_boundary is not used
  if (ikey == NULL || dmi == NULL || ck_vn_if == NULL ||
      dt_type == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t ktype = ck_vn_if->get_key_type();
  // val_port_map *port_map_val;
  val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ikey));
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                              (GetMoManager(ktype)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("ktype %d", ktype);
  uint8_t valid_port = 0;
  if (valid_boundary == UNC_VF_VALID) {
    void *val_if = GetVal(ck_vn_if);
    switch (ktype) {
    case UNC_KT_VTEP_IF:
    {
      val_vtep_if *vtep_if = reinterpret_cast<val_vtep_if *>(val_if);
      vtep_if->valid[UPLL_IDX_ADMIN_ST_VTEPI] = UNC_VF_INVALID;
      valid_port = vtep_if->valid[UPLL_IDX_PORT_MAP_VTEPI];
      // port_map_val = &vtep_if->portmap;
      break;
    }
    case UNC_KT_VTUNNEL_IF:
    {
      val_vtunnel_if *vtunnel_if = reinterpret_cast<val_vtunnel_if *>(val_if);
      vtunnel_if->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] = UNC_VF_INVALID;
      valid_port = vtunnel_if->valid[UPLL_IDX_PORT_MAP_VTNL_IF];
      //  port_map_val = &vtunnel_if->portmap;
      break;
    }
    case UNC_KT_VBR_IF:
    {
      val_drv_vbr_if *vbr_if = reinterpret_cast<val_drv_vbr_if *>(val_if);
      vbr_if->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_INVALID;
      valid_port = vbr_if->vbr_if_val.valid[UPLL_IDX_PM_VBRI];
      result_code = NotifyPOMForPortMapVlinkFlag(dt_type,
                                              ck_vn_if,
                                              dmi,
                                              op,
                                              config_mode,
                                              vtn_name);

      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        return result_code;
      }
      //   port_map_val = &vbr_if->vbr_if_val.portmap;
      break;
    }
    case UNC_KT_VUNK_IF:
      return UPLL_RC_SUCCESS;
    /* VlanmapOnBoundary: During update: input received is vlanmap */
    case UNC_KT_VBR_VLANMAP:
      return UPLL_RC_SUCCESS;
    case UNC_KT_VBR_PORTMAP:
      return UPLL_RC_SUCCESS;
    default:
      UPLL_LOG_DEBUG("Unsupported keytype %d", ktype);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    UPLL_LOG_DEBUG("valid_port %d valid vlanid %d", valid_port,
                      vlink_val->valid[UPLL_IDX_LABEL_VLNK]);
  } else if (valid_boundary == UNC_VF_VALID_NO_VALUE) {
    val_port_map *portmap = NULL;
    switch (ktype) {
    case UNC_KT_VBR_IF:
    {
      val_drv_vbr_if *drv_ifval = reinterpret_cast<val_drv_vbr_if *>
                              GetVal(ck_vn_if);
      drv_ifval->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] =
            UNC_VF_INVALID;
      drv_ifval->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID_NO_VALUE;
      result_code = reinterpret_cast<VbrIfMoMgr *>(mgr)->
                       UpdatePortMap(ck_vn_if, dt_type, dmi, ck_vn_if,
                                     config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Clear Portmap failed %d", result_code);
        return result_code;
      }
      break;
    }
    case UNC_KT_VTEP_IF:
    {
      val_vtep_if *drv_ifval = reinterpret_cast<val_vtep_if *>
                              GetVal(ck_vn_if);
      drv_ifval->valid[UPLL_IDX_ADMIN_ST_VTEPI] = UNC_VF_INVALID;
      drv_ifval->valid[UPLL_IDX_PORT_MAP_VTEPI] = UNC_VF_VALID_NO_VALUE;
      portmap = &drv_ifval->portmap;
    }
     /* fall through intended */
    case UNC_KT_VTUNNEL_IF:
      if (ktype == UNC_KT_VTUNNEL_IF) {
        val_vtunnel_if *drv_ifval = reinterpret_cast<val_vtunnel_if *>
                                   GetVal(ck_vn_if);
        portmap = &drv_ifval->portmap;
        drv_ifval->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] = UNC_VF_INVALID;
        drv_ifval->valid[UPLL_IDX_PORT_MAP_VTNL_IF] = UNC_VF_VALID_NO_VALUE;
      }
      if (portmap) {
        portmap->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
            UNC_VF_VALID_NO_VALUE;
        uuu::upll_strncpy(portmap->logical_port_id, "\0", 1);
        portmap->valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
        portmap->valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
        portmap->tagged = UPLL_VLAN_UNTAGGED;
        portmap->vlan_id = 0;
      }
      break;
    case UNC_KT_VUNK_IF:
      return UPLL_RC_SUCCESS;
    /* VlanmapOnBoundary: vlanmap case continue */
    case UNC_KT_VBR_VLANMAP:
      return UPLL_RC_SUCCESS;
    case UNC_KT_VBR_PORTMAP:
      return UPLL_RC_SUCCESS;
    default:
      UPLL_LOG_DEBUG("Unsupported keytype %d", ktype);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  if (valid_boundary == UNC_VF_VALID) {
    if ((valid_port == UNC_VF_INVALID) ||
        ((valid_port == UNC_VF_VALID) && (op != UNC_OP_CREATE))) {
      result_code = ConverttoDriverPortMap(ck_vn_if, ikey, uppl_bdry, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        return result_code;
      }
    } else if (op == UNC_OP_CREATE) {
      UPLL_LOG_DEBUG("Vlink of a mapped interface is not allowed");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
#if 0
  else if (valid_port == UNC_VF_VALID) {
    if (op == UNC_OP_CREATE) {
      UPLL_LOG_DEBUG("Vlink of a mapped interface is not allowed");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    port_map_val->tagged = UPLL_VLAN_TAGGED;
    port_map_val->valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
    switch (vlink_val->valid[UPLL_IDX_VLAN_ID_VLNK]) {
      case UNC_VF_VALID:
        port_map_val->vlan_id = vlink_val->vlan_id;
        break;
      case UNC_VF_VALID_NO_VALUE:
        port_map_val->vlan_id = 0;
        port_map_val->tagged = UPLL_VLAN_UNTAGGED;
        break;
      default:
        break;
    }
    port_map_val->valid[UPLL_IDX_VLAN_ID_PM] =
      vlink_val->valid[UPLL_IDX_VLAN_ID_VLNK];
#if 0
    if (CompareVbrIfWithPhysicalData(ck_drv_vbr_if, ck_boundary, ikey)) {
      UPLL_LOG_DEBUG("VbrIf is already configured");
      return UPLL_RC_ERR_GENERIC;
    }
#endif
  }
#endif
  IpcReqRespHeader req;
  memset(&req, 0, sizeof(IpcReqRespHeader));
  req.datatype = dt_type;
  req.operation = op;

  switch (ktype) {
    case UNC_KT_VBR_IF: {
      result_code = mgr->ValidateAttribute(ck_vn_if, dmi, &req);
     VtepIfMoMgr *vtunnelif_mgr = reinterpret_cast<VtepIfMoMgr *>(mgr);
      result_code = vtunnelif_mgr->IsLogicalPortAndVlanIdInUse<val_vbr_if>
          (ck_vn_if, dmi, &req);
        }
        break;
      case UNC_KT_VTEP_IF: {
        VtepIfMoMgr *vtepif_mgr = reinterpret_cast<VtepIfMoMgr *>(
            const_cast<MoManager *>(GetMoManager(UNC_KT_VTEP_IF)));
        result_code = vtepif_mgr->IsLogicalPortAndVlanIdInUse<val_vtep_if>
            (ck_vn_if, dmi, &req);
        break;
      }
      case UNC_KT_VTUNNEL_IF: {
        VtunnelIfMoMgr *vtunnelif_mgr = reinterpret_cast<VtunnelIfMoMgr *>(mgr);
        result_code = vtunnelif_mgr->IsLogicalPortAndVlanIdInUse<val_vtunnel_if>
            (ck_vn_if, dmi, &req);
        break;
      }
      default:
        break;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("LogicalPort VlanId validation failed");
      return result_code;
    }
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = mgr->UpdateConfigDB(ck_vn_if, dt_type, UNC_OP_UPDATE,
      dmi, &dbop, config_mode, vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Updation failed %d", result_code);
    return result_code;
  }
  return result_code;
}

bool VlinkMoMgr::CompareVbrIfWithPhysicalData(ConfigKeyVal *ck_drv_vbr_if,
                                              ConfigKeyVal *ck_boundary_data,
                                              ConfigKeyVal *ikey) {
  if (ikey == NULL || ck_drv_vbr_if == NULL || ck_boundary_data == NULL)
    return UPLL_RC_ERR_GENERIC;
  val_boundary *boundary_val_data = reinterpret_cast<val_boundary *>
                                            ((GetVal(ck_boundary_data)));
  if (boundary_val_data == NULL) {
    UPLL_LOG_DEBUG("Boundary invalid");
    return UPLL_RC_ERR_GENERIC;
  }
#if 0
  val_drv_port_map vbr_port_data = reinterpret_cast<val_drv_vbr_if *>(
                      (GetVal(ck_drv_vbr_if)))->portmap;
  if (*(reinterpret_cast<int*>
         (ikey->get_cfg_val()->get_user_data())) & kVnode1) {
    if (boundary_val_data->valid[kIdxValBoundarySwitchid1] != UNC_VF_VALID &&
     vbr_port_data.valid[UPLL_IDX_SWITCH_ID_DRV_PM] != UNC_VF_VALID)
    return false;
    else if (!strcmp(reinterpret_cast<char*>(&boundary_val_data->switch_id_1),
              reinterpret_cast<char*>&vbr_port_data.switch_id))
    return false;
    if (boundary_val_data->valid[kIdxValBoundaryPortname1] != UNC_VF_VALID &&
        vbr_port_data.valid[UPLL_IDX_PORT_NAME_DRV_PM] != UNC_VF_VALID)
    return false;
    else if (!strcmp(reinterpret_cast<char*>(&boundary_val_data->port_name_1),
           reinterpret_cast<char*>)(&vbr_port_data.port_name)))
    return false;

  } else if (*(reinterpret_cast<int*>(ikey->get_cfg_val()->get_user_data()))
            & kVnode2) {
    if (boundary_val_data->valid[kIdxValBoundarySwitchid2] != UNC_VF_VALID
        && vbr_port_data.valid[UPLL_IDX_SWITCH_ID_PM] != UNC_VF_VALID)
    return false;
    else if (!strcmp(reinterpret_cast<char*>(boundary_val_data->switch_id_2),
          reinterpret_cast<char*>(&vbr_port_data.switch_id)))
    return false;
    if (boundary_val_data->valid[kIdxValBoundaryPortname2] != UNC_VF_VALID
       && vbr_port_data.valid[UPLL_IDX_PORT_NAME_DRV_PM] != UNC_VF_VALID)
    return false;
    else if (!strcmp(reinterpret_cast<char*>(&boundary_val_data->port_name_2),
      reinterpret_cast<char*>(&vbr_port_data.port_name)))
    return false;
  }
  if (vlan_id != NULL && vbr_port_data.valid[UPLL_IDX_VLAN_ID_DRV_PM]
         == UNC_VF_VALID )
  if (vlan_id != vbr_port_data.vlan_id)
    return false;
#endif
  return true;
}

upll_rc_t VlinkMoMgr::ConverttoDriverPortMap(ConfigKeyVal *iokey,
                                         ConfigKeyVal *ikey,
                                         ConfigKeyVal *ck_boundary,
                                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (ikey == NULL || iokey == NULL || ikey->get_cfg_val() == NULL ||
      iokey->get_cfg_val() == NULL || ck_boundary == NULL) {
    UPLL_LOG_DEBUG("Invalid Input");
    return UPLL_RC_ERR_GENERIC;
  }
  val_boundary *boundary_val_data = reinterpret_cast<val_boundary *>
                                              (GetVal(ck_boundary));
  if (boundary_val_data == NULL) {
    UPLL_LOG_DEBUG("Invalid parameter");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ktype = iokey->get_key_type();
  val_port_map *port_map_val = NULL;
  UPLL_LOG_DEBUG("ktype %d " , ktype);
  switch (ktype) {
  case UNC_KT_VBR_IF:
  {
    val_drv_vbr_if *drv_ifval = reinterpret_cast<val_drv_vbr_if *>
                               (GetVal(iokey));
    if (!drv_ifval) {
      UPLL_LOG_DEBUG("Invalid param");
      return UPLL_RC_ERR_GENERIC;
    }
    ConfigKeyVal *okey = NULL;
    if (drv_ifval->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_INVALID) {
      MoMgrImpl *vbrif_mgr = reinterpret_cast<MoMgrImpl *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
      if (!vbrif_mgr) {
        UPLL_LOG_ERROR("Instance is NULL");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = vbrif_mgr->GetChildConfigKey(okey, iokey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_ERROR("GetChilConfigKey Failed");
        return UPLL_RC_ERR_GENERIC;
      }
      DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
        kOpInOutFlag};
      result_code = vbrif_mgr->ReadConfigDB(okey, UPLL_DT_RUNNING,
          UNC_OP_READ, dbop, dmi, MAINTBL);
      if ((UPLL_RC_SUCCESS != result_code) &&
          (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
        UPLL_LOG_ERROR("ReadConfigDB failure %d", result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }

      bool port_map_in_run = false;
      if (result_code == UPLL_RC_SUCCESS) {
        val_drv_vbr_if *drv_ifval1 = reinterpret_cast<val_drv_vbr_if *>
          (GetVal(okey));
        if (drv_ifval1 == NULL) {
          UPLL_LOG_ERROR("val vbr is NULL");
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_ERR_GENERIC;
        }
        if (drv_ifval1->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
          port_map_in_run = true;
          drv_ifval->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
          drv_ifval->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
          uuu::upll_strncpy(drv_ifval->vex_name, drv_ifval1->vex_name,
              kMaxLenVnodeName+1);
          drv_ifval->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID;
          uuu::upll_strncpy(reinterpret_cast<char *>(drv_ifval->vex_if_name),
              drv_ifval1->vex_if_name, kMaxLenInterfaceName+1);
          drv_ifval->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID;
          uuu::upll_strncpy(reinterpret_cast<char *>(drv_ifval->vex_link_name),
              drv_ifval1->vex_link_name, kMaxLenVlinkName+1);
        }
      }
      if (!port_map_in_run) {
        UPLL_LOG_TRACE("Generate vexternal and vexternal if name");
        std::string if_name = reinterpret_cast<const char *>
          (reinterpret_cast<key_vbr_if*>(iokey->get_key())->if_name);
        if (strlen(if_name.c_str()) >= 10) {
          if_name.erase(10);
        }
        // Autogenerate a vexternal name, vext-if name, vlink-name.
        // Vexternal name needs to be unique with in controller for that VTN.
        while (1) {
          struct timeval _timeval;
          struct timezone _timezone;
          gettimeofday(&_timeval, &_timezone);

          std::stringstream ss;
          ss << if_name << _timeval.tv_sec << _timeval.tv_usec;
          std::string unique_id = ss.str();
          std::string vex_name("vx_");
          vex_name += unique_id;
          std::string vex_if_name("vi_");
          vex_if_name += unique_id;
          std::string vex_link_name("vl_");
          vex_link_name += unique_id;

          key_vbr *vbr_key = reinterpret_cast<key_vbr *>(ConfigKeyVal::Malloc
                                                         (sizeof(key_vbr)));
          uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                            reinterpret_cast<key_vlink *>
                            (iokey->get_key())->vtn_key.vtn_name,
                            (kMaxLenVtnName+1));
          uuu::upll_strncpy(vbr_key->vbridge_name, vex_name.c_str(),
                            (kMaxLenVnodeName+1));

          ConfigKeyVal *vex_ckv = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                                   IpctSt::kIpcStKeyVbr,
                                                   vbr_key, NULL);

          controller_domain ctrlr_dom;
          ctrlr_dom.ctrlr = NULL;
          ctrlr_dom.domain = NULL;
          GET_USER_DATA_CTRLR_DOMAIN(iokey, ctrlr_dom);
          SET_USER_DATA_CTRLR_DOMAIN(vex_ckv, ctrlr_dom);
          // Auto generated vexternal name of vBridge interface, should not be
          // same of other vnodes and other vexternal's of the same controller
          result_code = VnodeChecks(vex_ckv, UPLL_DT_CANDIDATE, dmi, true);
          DELETE_IF_NOT_NULL(vex_ckv);
          if (result_code == UPLL_RC_ERR_CFG_SEMANTIC ||
              result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
            UPLL_LOG_DEBUG("Another Vnode %s already exists.  Error code : %d",
                           vex_name.c_str(), result_code);
            continue;  // retry and generate unique name
          } else if (result_code == UPLL_RC_SUCCESS) {
            drv_ifval->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
            drv_ifval->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
            uuu::upll_strncpy(drv_ifval->vex_name, vex_name.c_str(),
                              kMaxLenVnodeName+1);

            drv_ifval->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID;
            uuu::upll_strncpy(reinterpret_cast<char *>(drv_ifval->vex_if_name),
                              vex_if_name.c_str(), kMaxLenInterfaceName+1);

            drv_ifval->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID;
            uuu::upll_strncpy(reinterpret_cast<char *>
                              (drv_ifval->vex_link_name),
                              vex_link_name.c_str(), kMaxLenVlinkName+1);
          } else {
            UPLL_LOG_DEBUG("Vnodchecks failed. %d", result_code);
            DELETE_IF_NOT_NULL(okey);
            return result_code;
          }
          break;
        }
      }
    }
    port_map_val = &drv_ifval->vbr_if_val.portmap;
    DELETE_IF_NOT_NULL(okey);
    break;
  }
  case UNC_KT_VTEP_IF:
  {
    val_vtep_if *drv_ifval = reinterpret_cast<val_vtep_if *>
                               (GetVal(iokey));
    if (!drv_ifval) {
      UPLL_LOG_DEBUG("Invalid param");
      return UPLL_RC_ERR_GENERIC;
    }
    drv_ifval->valid[UPLL_IDX_PORT_MAP_VTEPI] = UNC_VF_VALID;
    port_map_val = &drv_ifval->portmap;
    break;
  }
  case UNC_KT_VTUNNEL_IF:
  {
    val_vtunnel_if *drv_ifval = reinterpret_cast<val_vtunnel_if *>
                               (GetVal(iokey));
    if (!drv_ifval) {
      UPLL_LOG_DEBUG("Invalid param");
      return UPLL_RC_ERR_GENERIC;
    }
    drv_ifval->valid[UPLL_IDX_PORT_MAP_VTNL_IF] = UNC_VF_VALID;
    port_map_val = &drv_ifval->portmap;
    break;
  }
  case UNC_KT_VUNK_IF:
    break;
  default:
    UPLL_LOG_DEBUG("Unsupported keytype %d", ktype);
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t rename = 0;
  controller_domain ctrlr_dom[2] = { { NULL, NULL }, { NULL, NULL } };
  uint8_t *node_ctrlr = NULL;
  uint8_t *node_dom = NULL;
  char *port_id = NULL;
  uppl_val_boundary_index index = kIdxBoundaryLogicalPortId1;
  GET_USER_DATA_FLAGS(iokey, rename);

  result_code = GetControllerDomainId(ikey, &ctrlr_dom[0]);
  if (UPLL_RC_SUCCESS != result_code)
    return result_code;

  if (rename & kVlinkBoundaryNode1) {
    node_ctrlr = ctrlr_dom[0].ctrlr;
    node_dom = ctrlr_dom[0].domain;
  } else {
    node_ctrlr = ctrlr_dom[1].ctrlr;
    node_dom = ctrlr_dom[1].domain;
  }
  if ((!node_ctrlr && (ktype != UNC_KT_VUNK_IF)) || !node_dom) {
    UPLL_LOG_DEBUG("Returning error\n");
    return UPLL_RC_ERR_GENERIC;
  }
  if (!node_ctrlr && node_dom && (ktype == UNC_KT_VUNK_IF)) {
    if (!(strcmp(reinterpret_cast<const char *>(node_dom),
          reinterpret_cast<const char *>(boundary_val_data->domain_name2)))
         || (!(strcmp(reinterpret_cast<const char *>(node_dom),
          reinterpret_cast<const char *>(boundary_val_data->domain_name1))))) {
        UPLL_LOG_DEBUG("Controller / Domain Match");
    } else {
        UPLL_LOG_DEBUG("Wrong Controller / domain \n");
        return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  } else if (!(strcmp(reinterpret_cast<const char *>(node_ctrlr),
      reinterpret_cast<const char *>(boundary_val_data->controller_name1))) &&
      !(strcmp(reinterpret_cast<const char *>(node_dom),
      reinterpret_cast<const char *>(boundary_val_data->domain_name1)))) {
      index = kIdxBoundaryLogicalPortId1;
      port_id = reinterpret_cast<char *>(boundary_val_data->logical_port_id1);
  } else if (!(strcmp(reinterpret_cast<const char *>(node_ctrlr),
      reinterpret_cast<const char *>(boundary_val_data->controller_name2))) &&
             !(strcmp(reinterpret_cast<const char *>(node_dom),
      reinterpret_cast<const char *>(boundary_val_data->domain_name2)))) {
      index = kIdxBoundaryLogicalPortId2;
      port_id = reinterpret_cast<char *>(boundary_val_data->logical_port_id2);
  } else {
      UPLL_LOG_DEBUG("Wrong Controller / domain \n");
      return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  // Return as Vunknown Interface doesnt have portmap
  if (ktype == UNC_KT_VUNK_IF)
    return UPLL_RC_SUCCESS;

  if (boundary_val_data->valid[index] == UNC_VF_VALID) {
      port_map_val->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
      uuu::upll_strncpy(port_map_val->logical_port_id,
              port_id, kMaxLenLogicalPortId+1);
  } else if (boundary_val_data->valid[index] == UNC_VF_VALID_NO_VALUE) {
      port_map_val->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID_NO_VALUE;
      uuu::upll_strncpy(port_map_val->logical_port_id, "\0", 1);
  } else {
      port_map_val->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_INVALID;
  }
  val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ikey));
  port_map_val->valid[UPLL_IDX_VLAN_ID_PM] =
                                 vlink_val->valid[UPLL_IDX_LABEL_VLNK];
  switch (vlink_val->valid[UPLL_IDX_LABEL_VLNK]) {
  case UNC_VF_VALID:
    port_map_val->vlan_id = vlink_val->label;
    if (port_map_val->vlan_id == NO_VLAN_ID)
      port_map_val->tagged = UPLL_VLAN_UNTAGGED;
    else
      port_map_val->tagged = UPLL_VLAN_TAGGED;
    port_map_val->valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
    break;
  case UNC_VF_VALID_NO_VALUE:
    port_map_val->vlan_id = 0;
    port_map_val->tagged = UPLL_VLAN_UNTAGGED;
    port_map_val->valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
    break;
  default:
    break;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::CheckIfMemberOfVlink(ConfigKeyVal *ck_vnif,
                                           upll_keytype_datatype_t dt_type,
                                           ConfigKeyVal *&ck_vlink,
                                           DalDmlIntf *dmi,
                                           vn_if_type &interface_type) {
  UPLL_FUNC_TRACE;
  uint8_t if_flag = 0;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ck_vnif || !(ck_vnif->get_key())) {
    UPLL_LOG_DEBUG("Invalid ck_vnif");
    return UPLL_RC_ERR_GENERIC;
  }
  /* Retrive the interface info from vbr_if_tbl(with VTN and vExternal info)
   * which is required to search in vlink_tbl */
  if (ck_vnif->get_key_type() == UNC_KT_VBR_IF) {
    MoMgrImpl *vbr_if_mgr = reinterpret_cast<MoMgrImpl *>
                                       (const_cast<MoManager *>(
                                       GetMoManager(UNC_KT_VBR_IF)));
    key_vbr_if *err_key = reinterpret_cast<key_vbr_if *>(ck_vnif->get_key());
    val_drv_vbr_if *drv_val = reinterpret_cast<val_drv_vbr_if *>GetVal(ck_vnif);
    if (vbr_if_mgr->IsValidKey(ck_vnif->get_key(),
                               uudst::vbridge_interface::kDbiVbrName) &&
        vbr_if_mgr->IsValidKey(ck_vnif->get_key(),
                               uudst::vbridge_interface::kDbiIfName)) {
      UPLL_LOG_DEBUG("Key is complete");
    } else {
      if (!drv_val) return UPLL_RC_ERR_NO_SUCH_INSTANCE;
      if (drv_val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] == UNC_VF_VALID) {
      ConfigKeyVal *ckv_drv_vbr_if = NULL;
      result_code = vbr_if_mgr->GetChildConfigKey(ckv_drv_vbr_if, ck_vnif);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        return result_code;
      }
      val_drv_vbr_if *valif = reinterpret_cast<val_drv_vbr_if *>
                              (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
      uuu::upll_strncpy(valif->vex_name, drv_val->vex_name,
                                         (kMaxLenVnodeName+1));
      valif->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;

      ConfigVal *ck_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, valif);
      ckv_drv_vbr_if->SetCfgVal(ck_val);
      DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutFlag};
      result_code = vbr_if_mgr->ReadConfigDB(ckv_drv_vbr_if,
                                          dt_type, UNC_OP_READ,
                                          dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Interface not found");
        delete ckv_drv_vbr_if;
        return (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
                UPLL_RC_ERR_GENERIC : result_code;
      }
      key_vbr_if *drv_if_key = reinterpret_cast<key_vbr_if *>
                              (ckv_drv_vbr_if->get_key());
      uuu::upll_strncpy(err_key->vbr_key.vbridge_name,
                                    drv_if_key->vbr_key.vbridge_name,
                                    kMaxLenVnodeName+1);
      uuu::upll_strncpy(err_key->if_name,
                                    drv_if_key->if_name,
                                    kMaxLenInterfaceName+1);
      delete ckv_drv_vbr_if;
      }
    }
  }
  interface_type = kUnlinkedInterface;
  GET_USER_DATA_FLAGS(ck_vnif, if_flag);
  SET_USER_DATA_FLAGS(ck_vnif, kVlinkVnode1);
  result_code = GetVlinkKeyVal(ck_vnif, dt_type, ck_vlink, dmi);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    // check if the vBrIf is the second VnodeIf in VLINK TBL
    SET_USER_DATA_FLAGS(ck_vnif, kVlinkVnode2);
    if (ck_vlink) {
      delete ck_vlink;
      ck_vlink = NULL;
    }
    result_code = GetVlinkKeyVal(ck_vnif, dt_type, ck_vlink, dmi);
  } else {
    interface_type = kVlinkInternalNode1;
  }
  /* the interface is not part of a boundary vlink */
  if (result_code != UPLL_RC_SUCCESS) {
      if (ck_vlink) delete ck_vlink;
      ck_vlink = NULL;
      interface_type = kUnlinkedInterface;
  } else {
    if (!ck_vlink) {
       UPLL_LOG_DEBUG("Invalid param");
       return UPLL_RC_ERR_GENERIC;
    }
    uint8_t valid_boundary = reinterpret_cast<val_vlink *>(GetVal(ck_vlink))->
                            valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
    if (interface_type == kUnlinkedInterface) {
      interface_type = (valid_boundary)?
                       kVlinkBoundaryNode2:kVlinkInternalNode2;
    } else if (interface_type == kVlinkInternalNode1) {
      interface_type = (valid_boundary)? kVlinkBoundaryNode1: interface_type;
    } else {
      UPLL_LOG_DEBUG("Invalid value of interface_type %d", interface_type);
    }
  }
  SET_USER_DATA_FLAGS(ck_vnif, if_flag);
  return result_code;
}


upll_rc_t VlinkMoMgr::GetVlinkKeyVal(ConfigKeyVal *keyVal,
                          upll_keytype_datatype_t dt_type,
                                    ConfigKeyVal *&ck_vlink,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  uint8_t mem_vlink = 0;
  if (!keyVal) {
    UPLL_LOG_DEBUG("Invalid keyVal");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vlink *link_val = reinterpret_cast<val_vlink *>
                                  (ConfigKeyVal::Malloc(sizeof(val_vlink)));
  upll_rc_t res_code = UPLL_RC_SUCCESS;
  unc_key_type_t keytype = keyVal->get_key_type();

  /* VlanmapOnBoundary: Added vlanmap check */
  if ((keytype != UNC_KT_VBR_VLANMAP) && (keytype != UNC_KT_VBR_PORTMAP)) {
  key_vbr_if *vbr_key_if = reinterpret_cast<key_vbr_if *>(keyVal->get_key());
  if (!vbr_key_if) {
    UPLL_LOG_ERROR("Invalid key");
    free(link_val);
    return UPLL_RC_ERR_GENERIC;
  }
  GET_USER_DATA_FLAGS(keyVal, mem_vlink);
  mem_vlink &= VIF_TYPE;
  if (mem_vlink & 0x50) {
    uuu::upll_strncpy(link_val->vnode2_name,
                     vbr_key_if->vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
    uuu::upll_strncpy(link_val->vnode2_ifname,
                     vbr_key_if->if_name, (kMaxLenVnodeName + 1));
    link_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_VALID;
    link_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_VALID;
    link_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
    link_val->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_INVALID;
  } else  {
    uuu::upll_strncpy(link_val->vnode1_name,
                      vbr_key_if->vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
    uuu::upll_strncpy(link_val->vnode1_ifname,
                      vbr_key_if->if_name, (kMaxLenVnodeName + 1));
    link_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_VALID;
    link_val->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_VALID;
    link_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_INVALID;
    link_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
    }
    /* VlanmapOnBoundary: changes*/
  } else if (keytype == UNC_KT_VBR_VLANMAP) {
    key_vlan_map *vlanmap_key =
        reinterpret_cast<key_vlan_map *>(keyVal->get_key());
    if (!vlanmap_key) {
      UPLL_LOG_ERROR("Invalid key");
      free(link_val);
      return UPLL_RC_ERR_GENERIC;
    }
    GET_USER_DATA_FLAGS(keyVal, mem_vlink);
    mem_vlink &= VIF_TYPE;
    if (mem_vlink & kVlinkVnode2) {
      uuu::upll_strncpy(link_val->vnode2_name,
                        vlanmap_key->vbr_key.vbridge_name,
                       (kMaxLenVnodeName + 1));
      link_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_VALID;
      link_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
      link_val->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_INVALID;
      link_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
    } else  {
      uuu::upll_strncpy(link_val->vnode1_name,
                        vlanmap_key->vbr_key.vbridge_name,
                       (kMaxLenVnodeName + 1));
      link_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_VALID;
      link_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_INVALID;
      link_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
      link_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
    }
  } else {
    key_vbr_portmap *vbr_pmkey =
        reinterpret_cast<key_vbr_portmap *>(keyVal->get_key());
    if (!vbr_pmkey) {
      UPLL_LOG_ERROR("Invalid key");
      free(link_val);
      return UPLL_RC_ERR_GENERIC;
    }
    GET_USER_DATA_FLAGS(keyVal, mem_vlink);
    mem_vlink &= VIF_TYPE;
    if (mem_vlink & kVlinkVnode2) {
      uuu::upll_strncpy(link_val->vnode2_name,
                        vbr_pmkey->vbr_key.vbridge_name,
                       (kMaxLenVnodeName + 1));
      link_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_VALID;
      link_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
      link_val->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_INVALID;
      link_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
    } else  {
      uuu::upll_strncpy(link_val->vnode1_name,
                        vbr_pmkey->vbr_key.vbridge_name,
                       (kMaxLenVnodeName + 1));
      link_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_VALID;
      link_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_INVALID;
      link_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
      link_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
    }
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                   kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain };

  /* get vlink key from if key */
  upll_rc_t result_code = GetChildConfigKey(ck_vlink, keyVal);
  if (!ck_vlink || result_code != UPLL_RC_SUCCESS) {
    free(link_val);
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }
  ck_vlink->AppendCfgVal(IpctSt::kIpcStValVlink, link_val);
  res_code = ReadConfigDB(ck_vlink, dt_type, UNC_OP_READ,
                                       dbop, dmi, MAINTBL);
  UPLL_LOG_TRACE(" Vlink ReadConfigDB After %d", res_code);
  return res_code;
}

/*
 *  Based on the key type the bind info will pass
 */

bool VlinkMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                                      int &nattr, MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_vlink_maintbl_bind_info;
  } else if (RENAMETBL == tbl) {
    nattr = NUM_KEY_RENAME_TBL_;
    binfo = key_vlink_renametbl_update_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalid Table ");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}

upll_rc_t VlinkMoMgr::GetValid(void *val,
                               uint64_t indx,
                               uint8_t *&valid,
                               upll_keytype_datatype_t dt_type,
                               MoMgrTables tbl) {
  if (val == NULL) return UPLL_RC_ERR_GENERIC;
  if (tbl == MAINTBL) {
    switch (indx) {
      case uudst::vlink::kDbiOperStatus:
        valid = &(reinterpret_cast<val_vlink *>(val))->
                            valid[UPLL_IDX_OPER_STATUS_VLNKS];
          break;
      case uudst::vlink::kDbiDownCount:
        valid = NULL;
          break;
      case uudst::vlink::kDbiAdminStatus:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_ADMIN_STATUS_VLNK];
        break;
      case uudst::vlink::kDbiVnode1Name:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE1_NAME_VLNK];
        break;
      case uudst::vlink::kDbiVnode1Ifname:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE1_IF_NAME_VLNK];
        break;
      case uudst::vlink::kDbiVnode2Name:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE2_NAME_VLNK];
        break;
      case uudst::vlink::kDbiVnode2Ifname:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE2_IF_NAME_VLNK];
        break;
      case uudst::vlink::kDbiBoundaryName:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
        break;
      case uudst::vlink::kDbiLabelType:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
            valid[UPLL_IDX_LABEL_TYPE_VLNK];
        break;
      case uudst::vlink::kDbiLabel:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
            valid[UPLL_IDX_LABEL_VLNK];
        break;
      case uudst::vlink::kDbiDesc:
        valid = &(reinterpret_cast<val_vlink_t *>(val))->
                            valid[UPLL_IDX_DESCRIPTION_VLNK];
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  } else if (tbl == RENAMETBL) {
    switch (indx) {
       case uudst::vlink_rename::kDbiCtrlrVtnName:
         valid = &(reinterpret_cast<val_rename_vnode *>(val))->
                            valid[UPLL_CTRLR_VTN_NAME_VALID];
         break;
       case uudst::vlink_rename::kDbiCtrlrVlinkName:
         valid = &(reinterpret_cast<val_rename_vnode *>(val))->
                            valid[UPLL_CTRLR_VNODE_NAME_VALID];

         break;
       default:
         break;
     }
  } else if (tbl == CONVERTTBL) {
    switch (indx) {
      case uudst::convert_vlink::kDbiOperStatus:
        valid = &(reinterpret_cast<val_vlink *>(val))->
                            valid[UPLL_IDX_OPER_STATUS_VLNKS];
          break;
      case uudst::convert_vlink::kDbiDownCount:
        valid = NULL;
          break;
      case uudst::convert_vlink::kDbiVnode1Name:
        valid = &(reinterpret_cast<val_convert_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE1_NAME_CVLINK];
        break;
      case uudst::convert_vlink::kDbiVnode1Ifname:
        valid = &(reinterpret_cast<val_convert_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE1_IF_NAME_CVLINK];
        break;
      case uudst::convert_vlink::kDbiVnode2Name:
        valid = &(reinterpret_cast<val_convert_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE2_NAME_CVLINK];
        break;
      case uudst::convert_vlink::kDbiVnode2Ifname:
        valid = &(reinterpret_cast<val_convert_vlink_t *>(val))->
                            valid[UPLL_IDX_VNODE2_IF_NAME_CVLINK];
        break;
      case uudst::convert_vlink::kDbiBoundaryName:
        valid = &(reinterpret_cast<val_convert_vlink_t *>(val))->
                            valid[UPLL_IDX_BOUNDARY_NAME_CVLINK];
        break;
      case uudst::convert_vlink::kDbiLabelType:
        valid = &(reinterpret_cast<val_convert_vlink_t *>(val))->
            valid[UPLL_IDX_LABEL_TYPE_CVLINK];
        break;
      case uudst::convert_vlink::kDbiLabel:
        valid = &(reinterpret_cast<val_convert_vlink_t *>(val))->
            valid[UPLL_IDX_LABEL_CVLINK];
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
  }
  return UPLL_RC_SUCCESS;
}

bool VlinkMoMgr::IsValidKey(void *key, uint64_t index,
                            MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (tbl == CONVERTTBL) {
    key_convert_vlink *conv_vlink_key =
        reinterpret_cast<key_convert_vlink *>(key);
    switch (index) {
      case uudst::convert_vlink::kDbiVtnName:
        ret_val = ValidateKey(
            reinterpret_cast<char *>(conv_vlink_key->vbr_key.vtn_key.vtn_name),
            kMinLenVtnName, kMaxLenVtnName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::convert_vlink::kDbiVbrName:
        ret_val = ValidateKey(reinterpret_cast<char *>(
                conv_vlink_key->vbr_key.vbridge_name),
                kMinLenVnodeName, kMaxLenVnodeName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Unified vBridge Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::convert_vlink::kDbiVlinkName:
        ret_val = ValidateKey(reinterpret_cast<char *>(
                conv_vlink_key->convert_vlink_name),
                kMinLenVlinkName, kMaxLenVlinkName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("convert VLink Name is not valid(%d)", ret_val);
          return false;
        }
        break;

      default:
        UPLL_LOG_TRACE("Invalid Key Index");
        return false;
        break;
    }
  } else {
    key_vlink *vlink_key = reinterpret_cast<key_vlink *>(key);
    switch (index) {
      case uudst::vlink::kDbiVtnName:
      case uudst::vnode_rename::kDbiUncVtnName:
        ret_val = ValidateKey(
            reinterpret_cast<char *>(vlink_key->vtn_key.vtn_name),
            kMinLenVtnName, kMaxLenVtnName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::vlink::kDbiVlinkName:
      case uudst::vnode_rename::kDbiUncvnodeName:
        ret_val = ValidateKey(reinterpret_cast<char *>(vlink_key->vlink_name),
                              kMinLenVlinkName, kMaxLenVlinkName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VLink Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      default:
        UPLL_LOG_TRACE("Invalid Key Index");
        return false;
        break;
    }
  }
  return true;
}

upll_rc_t VlinkMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vlink_t * vlink_key = NULL;
  if (okey && okey->get_key()) {
     vlink_key = reinterpret_cast<key_vlink_t *>
                 (okey->get_key());
  } else {
     vlink_key = reinterpret_cast<key_vlink *>
                          (ConfigKeyVal::Malloc(sizeof(key_vlink)));
  }
  unc_key_type_t ktype;

  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink, vlink_key,
                            NULL);
    else if (okey->get_key() != vlink_key)
      okey->SetKey(IpctSt::kIpcStKeyVlink, vlink_key);
    return (okey)?UPLL_RC_SUCCESS:UPLL_RC_ERR_GENERIC;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      free(vlink_key);
    return UPLL_RC_ERR_GENERIC;
  }
  ktype = parent_key->get_key_type();
  switch (ktype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name,
             (kMaxLenVtnName+1) );
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VBR_IF:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vbr_if *>(pkey)->vbr_key.vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VROUTER:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vrt *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VRT_IF:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vrt_if *>(pkey)->vrt_key.vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VLINK:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      uuu::upll_strncpy(vlink_key->vlink_name,
             reinterpret_cast<key_vlink *>(pkey)->vlink_name,
             (kMaxLenVlinkName+1));
      break;
    case UNC_KT_VTEP:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtep *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTEP_IF:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtep_if *>(pkey)->vtep_key.vtn_key.vtn_name,
              (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTUNNEL:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtunnel *>(pkey)->vtn_key.vtn_name,
              (kMaxLenVtnName+1));
      break;
    case UNC_KT_VTUNNEL_IF:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vtunnel_if *>
              (pkey)->vtunnel_key.vtn_key.vtn_name,
              (kMaxLenVtnName+1));
      break;
    case UNC_KT_VUNKNOWN:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vunknown *>(pkey)->vtn_key.vtn_name,
              (kMaxLenVtnName+1));
      break;
    case UNC_KT_VUNK_IF:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vunk_if *>(pkey)->vunk_key.vtn_key.vtn_name,
              (kMaxLenVtnName+1));
      break;
    /* VlanmapOnBoundary: Added vlanmap case */
    case UNC_KT_VBR_VLANMAP:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vlan_map_t *>(pkey)->vbr_key.vtn_key.vtn_name,
              (kMaxLenVtnName+1));
      break;
    case UNC_KT_VBR_PORTMAP:
      uuu::upll_strncpy(vlink_key->vtn_key.vtn_name,
             reinterpret_cast<key_vbr_portmap_t *>(
                 pkey)->vbr_key.vtn_key.vtn_name, (kMaxLenVtnName+1));
      break;

    default:
      if (!okey || !(okey->get_key()))
        free(vlink_key);
      return UPLL_RC_ERR_GENERIC; /* OPERATION_NOT_SUPPORTED ? */
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink, vlink_key,
                          NULL);
  else if (okey->get_key() != vlink_key)
    okey->SetKey(IpctSt::kIpcStKeyVlink, vlink_key);
  if (okey == NULL) {
    free(vlink_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
     SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

/* Pure Virtual functions from MoMgrImpl */
upll_rc_t VlinkMoMgr::GetVnodeName(ConfigKeyVal *ikey, uint8_t *&vtn_name,
                                   uint8_t *&vnode_name) {
  UPLL_FUNC_TRACE;
  key_vlink *vlink_key = reinterpret_cast<key_vlink *>(ikey->get_key());
  if (vlink_key == NULL) return UPLL_RC_ERR_GENERIC;
  vtn_name = vlink_key->vtn_key.vtn_name;
  vnode_name = vlink_key->vlink_name;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                         ConfigKeyVal *ikey) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;  // COV REVERSE INULL
  if (ikey == NULL)
    return UPLL_RC_ERR_GENERIC;
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_VLINK) return UPLL_RC_ERR_GENERIC;
  void *pkey = ikey->get_key();
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vtn *vtn_key = reinterpret_cast<key_vtn *>(
                 ConfigKeyVal::Malloc(sizeof(key_vtn)));
  uuu::upll_strncpy(vtn_key->vtn_name,
         reinterpret_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
          (kMaxLenVtnName+1));
  okey = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn, vtn_key, NULL);
  if (okey == NULL) {
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
  }
  return result_code;
}

upll_rc_t VlinkMoMgr::AllocVal(ConfigVal *&ck_val,
                               upll_keytype_datatype_t dt_type,
                               MoMgrTables tbl) {
  void *val;
  // ConfigVal *ck_nxtval;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>(ConfigKeyVal::Malloc(sizeof(val_vlink)));
      ck_val = new ConfigVal(IpctSt::kIpcStValVlink, val);
      if (!ck_val) {
        UPLL_LOG_ERROR("Invalid memory allocation");
        return UPLL_RC_ERR_GENERIC;
      }
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>(
              ConfigKeyVal::Malloc(sizeof(val_db_vlink_st)));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVlinkSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    case RENAMETBL:
      val = reinterpret_cast<void *>(
           ConfigKeyVal::Malloc(sizeof(val_db_rename_vlink)));
      ck_val = new ConfigVal(IpctSt::kIpcInvalidStNum, val);
      break;
    case CONVERTTBL:
      val = reinterpret_cast<void *>(
           ConfigKeyVal::Malloc(sizeof(val_convert_vlink)));
      ck_val = new ConfigVal(IpctSt::kIpcStValConvertVlink, val);
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>(
              ConfigKeyVal::Malloc(sizeof(val_db_vlink_st)));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVlinkSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                                      MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if ((req == NULL) || (okey != NULL)) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VLINK) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_vlink *ival = reinterpret_cast<val_vlink *>(GetVal(req));
      if (!ival) {
        UPLL_LOG_TRACE("Empty Val structure");
        return UPLL_RC_ERR_GENERIC;
      }
      val_vlink *vlink_val = reinterpret_cast<val_vlink *>
                                (ConfigKeyVal::Malloc(sizeof(val_vlink)));
      memcpy(vlink_val, ival, sizeof(val_vlink));
      tmp1 = new ConfigVal(IpctSt::kIpcStValVlink, vlink_val);
    } else if (tbl == RENAMETBL) {
      void *rename_val;
      ConfigVal *ck_v = req->get_cfg_val();
      if (ck_v->get_st_num() == IpctSt::kIpcInvalidStNum) {
        val_db_rename_vlink *ival = reinterpret_cast<val_db_rename_vlink *>
                                                             (GetVal(req));
        rename_val = reinterpret_cast<void *>
                   (ConfigKeyVal::Malloc(sizeof(val_db_rename_vlink)));
        memcpy(rename_val, ival, sizeof(val_db_rename_vlink));
        tmp1 = new ConfigVal(IpctSt::kIpcInvalidStNum, rename_val);
      } else {
        val_rename_vlink *ival = reinterpret_cast<val_rename_vlink *>
                                                     (GetVal(req));
        rename_val = reinterpret_cast<void *>(
                 ConfigKeyVal::Malloc(sizeof(val_rename_vlink)));
        memcpy(rename_val, ival, sizeof(val_rename_vlink));
        tmp1 = new ConfigVal(IpctSt::kIpcStValRenameVlink, rename_val);
      }
    } else if (tbl == CONVERTTBL) {
      val_convert_vlink_t *ival =
                    reinterpret_cast<val_convert_vlink_t *>(GetVal(req));
      if (!ival) {
        UPLL_LOG_TRACE("Empty Val structure");
        return UPLL_RC_ERR_GENERIC;
      }
      val_convert_vlink_t *vlink_val = reinterpret_cast<val_convert_vlink_t *>
          (ConfigKeyVal::Malloc(sizeof(val_convert_vlink_t)));
      memcpy(vlink_val, ival, sizeof(val_convert_vlink_t));
      tmp1 = new ConfigVal(IpctSt::kIpcStValConvertVlink, vlink_val);
    }
    tmp = tmp->get_next_cfg_val();
  };
  if (tmp) {
    if (tbl == MAINTBL) {
      val_db_vlink_st *ival = reinterpret_cast<val_db_vlink_st *>
                                              (tmp->get_val());
      val_db_vlink_st *val_db_vlink = reinterpret_cast<val_db_vlink_st *>
                       (ConfigKeyVal::Malloc(sizeof(val_db_vlink_st)));
      memcpy(val_db_vlink, ival, sizeof(val_db_vlink_st));
      ConfigVal *tmp2 = new ConfigVal(IpctSt::kIpcStValVlinkSt, val_db_vlink);
      tmp1->AppendCfgVal(tmp2);
    }
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  if (tbl == CONVERTTBL) {
    key_convert_vlink_t *ikey = reinterpret_cast<key_convert_vlink_t *>(tkey);
    key_convert_vlink_t *vlink_key = reinterpret_cast<key_convert_vlink_t *>
        (ConfigKeyVal::Malloc(sizeof(key_convert_vlink_t)));
    memcpy(vlink_key, ikey, sizeof(key_convert_vlink_t));
    okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyConvertVlink,
                            vlink_key, tmp1);
    if (!okey) {
      DELETE_IF_NOT_NULL(tmp1);
      FREE_IF_NOT_NULL(vlink_key);
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    key_vlink *ikey = reinterpret_cast<key_vlink *>(tkey);
    key_vlink *vlink_key = reinterpret_cast<key_vlink *>
        (ConfigKeyVal::Malloc(sizeof(key_vlink)));
    memcpy(vlink_key, ikey, sizeof(key_vlink));
    okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink, vlink_key,
                            tmp1);
    if (!okey) {
      DELETE_IF_NOT_NULL(tmp1);
      FREE_IF_NOT_NULL(vlink_key);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  SET_USER_DATA(okey, req);
  if (RENAMETBL != tbl)
    SET_USER_DATA(okey->get_cfg_val(), req->get_cfg_val());
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
                                       upll_keytype_datatype_t dt_type,
                                       DalDmlIntf *dmi, uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *unc_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  key_vlink *ctrlr_key = reinterpret_cast<key_vlink *>(ikey->get_key());

  /*
   * Vnodes are renamed in value structure. so need to convert into unc name
   * in case of partial import for audit not required
   */
  if (UPLL_DT_IMPORT == dt_type || UPLL_DT_RUNNING == dt_type ||
      UPLL_DT_AUDIT == dt_type) {
    val_vlink_t *val_vlink = reinterpret_cast<val_vlink_t*>(GetVal(ikey));
   /* For rename value is not available so no need to convret into unc
    * name
    */
    if (val_vlink) {
      MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                              (GetMoManager(UNC_KT_VBRIDGE)));
      if (!mgr) {
        return UPLL_RC_ERR_GENERIC;
      }
      ConfigKeyVal *vnode_ckv = NULL;
      uint8_t rename = 0x00;
      controller_domain ctrlr_dom;
      ctrlr_dom.ctrlr = NULL;
      ctrlr_dom.domain = NULL;
      GET_USER_DATA_FLAGS(ikey, rename);
      result_code = mgr->GetChildConfigKey(vnode_ckv, NULL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        return result_code;
      }
      key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>(vnode_ckv->get_key());
      uuu::upll_strncpy(vbr_key->vtn_key.vtn_name, ctrlr_key->vtn_key.vtn_name,
                        (kMaxLenVtnName+1));
      for (int i = 0; i < 2;  i++) {
        uint8_t valid = (0 == i)?val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK]:
            val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK];
        uint8_t *vnode_name = (0 == i)?val_vlink->vnode1_name:
            val_vlink->vnode2_name;
        if (valid) {
          uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                            ctrlr_key->vtn_key.vtn_name,
                            (kMaxLenVtnName+1));
          uuu::upll_strncpy(vbr_key->vbridge_name, vnode_name,
                            (kMaxLenVnodeName+1));
          result_code  = mgr->GetRenamedUncKey(vnode_ckv, dt_type,
                                               dmi, ctrlr_id);
          if (UPLL_RC_SUCCESS != result_code &&
              UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_DEBUG("vnode's GetRenamedUncKey failed %d", result_code);
            delete vnode_ckv;
            return result_code;
          }
          GET_USER_DATA_CTRLR_DOMAIN(vnode_ckv, ctrlr_dom);
          if (UPLL_RC_SUCCESS == result_code) {
            uuu::upll_strncpy(vnode_name, vbr_key->vbridge_name,
                              (kMaxLenVnodeName+1));
            GET_USER_DATA_FLAGS(ikey->get_cfg_val(), rename);
            if (1 == i) {
              rename = rename | VN1_RENAME;
              SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom);
              SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
            } else {
              rename = rename | VN2_RENAME;
              SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
              SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom);
            }
            SET_USER_DATA_FLAGS(ikey->get_cfg_val(), rename);
          }
        }
        result_code = UPLL_RC_SUCCESS;
      }
      delete vnode_ckv;
    }
  }
  val_db_rename_vlink *rename_vlink = reinterpret_cast<val_db_rename_vlink *>
                   (ConfigKeyVal::Malloc(sizeof(val_db_rename_vlink)));
  rename_vlink->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_INVALID;
  rename_vlink->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_INVALID;
  upll_rc_t ret_val = ValidateKey(
                        reinterpret_cast<char *>(ctrlr_key->vtn_key.vtn_name),
                        kMinLenVtnName, kMaxLenVtnName);
  if (ret_val == UPLL_RC_SUCCESS)  {
    uuu::upll_strncpy(rename_vlink->ctrlr_vtn_name,
                      ctrlr_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
    rename_vlink->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
  }
  ret_val = ValidateKey(reinterpret_cast<char *>(ctrlr_key->vlink_name),
                        kMinLenVlinkName, kMaxLenVlinkName);
  if (ret_val == UPLL_RC_SUCCESS)  {
    uuu::upll_strncpy(rename_vlink->ctrlr_vlink_name,
                      ctrlr_key->vlink_name, (kMaxLenVlinkName+1));
    rename_vlink->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
  }
  result_code = GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed result_code %d", result_code);
    return result_code;
  }
  if (ctrlr_id) {
    SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  } else {
    dbop.matchop = kOpMatchNone;
  }

  uint8_t rename = 0;
  unc_key->AppendCfgVal(IpctSt::kIpcInvalidStNum, rename_vlink);

  dbop.inoutop = kOpInOutCtrlr | kOpInOutDomain;
  result_code = ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
                                       RENAMETBL);
  if (result_code == UPLL_RC_SUCCESS) {
    GET_USER_DATA_FLAGS(ikey, rename);
    key_vlink *vlink_key = reinterpret_cast<key_vlink *>(unc_key->get_key());
    if (strcmp(reinterpret_cast<const char *>(ctrlr_key->vtn_key.vtn_name),
               reinterpret_cast<const char *>(vlink_key->vtn_key.vtn_name))) {
      uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name,
                        vlink_key->vtn_key.vtn_name, (kMaxLenVtnName+1));
      rename |= VTN_RENAME;
    }
    if (strcmp(reinterpret_cast<const char *>(ctrlr_key->vlink_name),
               reinterpret_cast<const char *>(vlink_key->vlink_name))) {
      uuu::upll_strncpy(reinterpret_cast<char *>(ctrlr_key->vlink_name),
                        reinterpret_cast<const char *>(vlink_key->vlink_name),
                        (kMaxLenVtnName+1));
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
      UPLL_LOG_DEBUG("GetChildConfigKey Failed with result_code %d",
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
        SET_USER_DATA_CTRLR(ikey, ctrlr_id);
      }
    }
  }
  SET_USER_DATA_FLAGS(ikey, rename);
  DELETE_IF_NOT_NULL(unc_key);
  return result_code;
}

upll_rc_t VlinkMoMgr::GetRenamedControllerKey(ConfigKeyVal *ikey,
                                              upll_keytype_datatype_t dt_type,
                                              DalDmlIntf *dmi,
                                              controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t val_rename = 0;
  UPLL_LOG_DEBUG("Test : Controller %s Domain %s",
                 ctrlr_dom->ctrlr, ctrlr_dom->domain);
  uint8_t rename = 1;
  for (int i = 0; i < 2; i++) {
    unc_key_type_t ktype = GetVlinkVnodeIfKeyType(ikey, i);
    if (ktype == UNC_KT_VUNK_IF)
      return UPLL_RC_SUCCESS;
  }
  result_code = IsRenamed(ikey, dt_type, dmi, rename);
  if ((result_code != UPLL_RC_SUCCESS) || !ikey->get_cfg_val()) {
    UPLL_LOG_DEBUG("Invalid param %d", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  key_vlink *ctrlr_key = reinterpret_cast<key_vlink *>(ikey->get_key());
  if (!ctrlr_dom->ctrlr ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->ctrlr)) ||
      !ctrlr_dom->domain ||
      !strlen(reinterpret_cast<const char *>(ctrlr_dom->domain))) {
    GET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), *ctrlr_dom);
  }

  val_vlink_t *val = reinterpret_cast<val_vlink_t *>(GetVal(ikey));
  if (!val) {
    UPLL_LOG_DEBUG("Val Structure is Error");
    return UPLL_RC_ERR_GENERIC;
  }
  // To Get the rename flags for old/new value vlink structures,
  // since it will be modified in the later part
  GET_USER_DATA_FLAGS(ikey->get_cfg_val(), val_rename);
  ConfigKeyVal *okey = NULL;
  // Get controller specific vnode1 and vnode2 names from
  // the vlink old/new value structure during
  // UPDATE - check renamed vnodes in both old/new value structures
  // CREATE/DELETE - check renamed vnodes in new value structure.
  result_code = GetRenamedVnodeName(ikey, okey, dt_type, dmi, ctrlr_dom);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  if (rename == 0) {
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_SUCCESS;
  }
  DELETE_IF_NOT_NULL(okey);
  if ((ctrlr_dom->ctrlr == NULL) || (ctrlr_dom->domain == NULL)) {
    UPLL_LOG_INFO("Invalid ctrlr/domain");
    return UPLL_RC_ERR_GENERIC;
  }
  okey = NULL;
  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;
  UPLL_LOG_DEBUG("Test : Controller %s Domain %s",
                 ctrlr_dom->ctrlr, ctrlr_dom->domain);
  SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain, kOpInOutFlag };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi, RENAMETBL);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (result_code == UPLL_RC_SUCCESS) {
    val_db_rename_vlink *rename_val = reinterpret_cast<val_db_rename_vlink *>
        (GetVal(okey));
    if (!rename_val) {
      DELETE_IF_NOT_NULL(okey);
      return result_code;
      return UPLL_RC_ERR_GENERIC;
    }
    if (!ctrlr_key) {
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_ERR_GENERIC;
    }
    if (rename & 0x01) { /* vtn renamed */
      uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, rename_val->ctrlr_vtn_name,
                        (kMaxLenVtnName+1));
    }
    if (rename & 0x02) { /* vnode renamed */
      uuu::upll_strncpy(ctrlr_key->vlink_name, rename_val->ctrlr_vlink_name,
                        (kMaxLenVlinkName+1));
    }
    SET_USER_DATA_FLAGS(ikey, rename);
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    MoMgrImpl *vtn_mgr
        = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
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
        uint8_t rename = 0;
        GET_USER_DATA_FLAGS(ikey, rename);
        rename |= VTN_RENAME;
        SET_USER_DATA_FLAGS(ikey, rename);
      }
    }
  }
  // To set back the original rename flags for old/new value vlink structures
  SET_USER_DATA_FLAGS(ikey->get_cfg_val(), val_rename);
  DELETE_IF_NOT_NULL(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::UpdateConfigStatus(ConfigKeyVal *vlink_key,
                                       unc_keytype_operation_t op,
                                       uint32_t driver_result,
                                       ConfigKeyVal *upd_key,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vlink *vlink_val;
  val_vlink *vlink_val2 = NULL;

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  vlink_val = reinterpret_cast<val_vlink *>(GetVal(vlink_key));
  if (vlink_val == NULL) return UPLL_RC_ERR_GENERIC;
  if (op == UNC_OP_CREATE) {
    vlink_val->cs_row_status = cs_status;
  } else if (op == UNC_OP_UPDATE) {
    void *inpval = GetVal(vlink_key);
    CompareValidValue(inpval, GetVal(upd_key), true);
    vlink_val2 = reinterpret_cast<val_vlink *>(GetVal(upd_key));
    if (vlink_val2)
      vlink_val->cs_row_status = vlink_val2->cs_row_status;
  } else {
    return UPLL_RC_ERR_GENERIC;
  }
  val_db_vlink_st *val_vlinkst = reinterpret_cast<val_db_vlink_st *>
                           (ConfigKeyVal::Malloc(sizeof(val_db_vlink_st)));
  vlink_key->AppendCfgVal(IpctSt::kIpcStValVlinkSt, val_vlinkst);

  val_vlinkst->vlink_val_st.valid[UPLL_IDX_OPER_STATUS_VLNKS]
                                            = UNC_VF_VALID;
  val_vlinkst->vlink_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
  controller_domain_t vlink_ctrlr_dom[2] = {{NULL, NULL}, {NULL, NULL}};
  bool bound_vlink = false;
  // check the given vlink is BoundaryVlink or not
  result_code = BoundaryVlink(vlink_key, vlink_ctrlr_dom, bound_vlink);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in BoundaryVlink");
    return result_code;
  }
  if (bound_vlink == false) {
    if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
      val_vlinkst->vlink_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
    } else if (vlink_val->admin_status == UPLL_ADMIN_ENABLE) {
      if (op == UNC_OP_CREATE)
        val_vlinkst->vlink_val_st.oper_status = UPLL_OPER_STATUS_UP;
    }
  }
  UPLL_LOG_TRACE("%s", (vlink_key->ToStr()).c_str());
  for ( unsigned int loop = 0;
        loop < sizeof(vlink_val->valid)/sizeof(vlink_val->valid[0]);
                                                         ++loop ) {
    if ( (UNC_VF_VALID == vlink_val->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == vlink_val->valid[loop])) {
      // Description is set to APPLIED
      if (loop == UPLL_IDX_DESCRIPTION_VLNK)
        vlink_val->cs_attr[loop] = UNC_CS_APPLIED;
      else
        vlink_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == vlink_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
        vlink_val->cs_attr[loop] = UNC_CS_APPLIED;
    } else if ((UNC_VF_INVALID == vlink_val->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
      if (vlink_val2)
        vlink_val->cs_attr[loop] = vlink_val2->cs_attr[loop];
    }
  }
  SetConsolidatedStatus(vlink_key, op, cs_status, dmi);
  return result_code;
}

#if 0
upll_rc_t VlinkMoMgr::GetControllerKey(ConfigKeyVal *ikey,
    ConfigKeyVal *&okey, unc_keytype_datatype_t dt_type, char *ctrlr_name ) {
  okey = NULL;
  char rename = (uint64_t)(ikey->get_user_data());
  key_vlink_t *key = reinterpret_cast<key_vlink_t *>(ikey->get_key());
  if (!rename) return UPLL_RC_SUCCESS;
  key_vlink_t *ctrlr_key = new key_vlink_t;

  /* vtn renamed */
  if (rename & 0x01) {
    key_vtn_t *vtn_key = new key_vtn_t;
    uuu::upll_strncpy(vtn_key->vtn_name, (key->vtn_key.vtn_name));
    ConfigKeyVal *ck_vtn = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                                            vtn_key, NULL);
    //   MoMgrImpl *mgr = (MoMgrImpl *)GetMoManager(UNC_KT_VTN);
//    mgr->ReadConfigDB(ck_vtn, dt_type, UNC_OP_READ, RENAMETBL);
    val_rename_vtn* rename_val = reinterpret_cast<val_rename_vtn *>
                                                  (GetVal(ck_vtn));
    uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, rename_val->new_name);
    delete ck_vtn;
  }
  /* vnode renamed */
  if (rename & 0x10) {
    GetChildConfigKey(okey, ikey);
    //  mgr->ReadConfigDB(okey, dt_type, UNC_OP_READ, RENAMETBL);
    val_rename_vbr *rename_val = reinterpret_cast<val_rename_vbr *>
                                                          (GetVal(okey));
    uuu::upll_strncpy(ctrlr_key->vtn_key.vtn_name, rename_val->new_name);
    delete okey;
  }
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVlink,
                          ctrlr_key, NULL);
  return UPLL_RC_SUCCESS;
}
#endif

upll_rc_t VlinkMoMgr::SwapKeyVal(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                                 DalDmlIntf *dmi, uint8_t *ctrlr,
                                 bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  okey = NULL;
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  if (ikey->get_key_type() != UNC_KT_VLINK) return UPLL_RC_ERR_BAD_REQUEST;
  val_rename_vlink_t *tval = reinterpret_cast<val_rename_vlink_t *>
                             (GetVal(ikey));
  if (!tval)
    return UPLL_RC_ERR_GENERIC;

  /* The PFC Name and New Name should not be equal */
  if (!strcmp(reinterpret_cast<char *>(tval->new_name),
         reinterpret_cast<char *>(reinterpret_cast<key_vlink_t *>
         (ikey->get_key())->vlink_name)))
    return UPLL_RC_ERR_GENERIC;
  key_vlink_t * key_vlink = reinterpret_cast<key_vlink_t *>
                            (ConfigKeyVal::Malloc(sizeof(key_vlink_t)));

  if (tval->valid[UPLL_IDX_NEW_NAME_RVLNK] == UNC_VF_VALID_NO_VALUE) {
    uuu::upll_strncpy(key_vlink->vlink_name,
            static_cast<key_vlink_t *>(ikey->get_key())->vlink_name,
            (kMaxLenVlinkName+1));
    no_rename = true;
  } else {
    if ((reinterpret_cast<val_rename_vlink_t *>(tval))->
            valid[UPLL_IDX_NEW_NAME_RVLNK] == UNC_VF_VALID) {
      /* checking the string is empty or not */
      if (!strlen(reinterpret_cast<char *>
         (static_cast<val_rename_vlink_t *>(tval)->new_name))) {
        free(key_vlink);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(key_vlink->vlink_name,
         static_cast<val_rename_vlink_t *>(GetVal(ikey))->new_name,
         (kMaxLenVlinkName+1));
    } else {
      free(key_vlink);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  /* Checking the vlink parent is renamed get the UNC name */
  ConfigKeyVal *pkey = NULL;
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    free(key_vlink);
    return result_code;
  }
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                              (GetMoManager(UNC_KT_VTN)));
  result_code = mgr->GetRenamedUncKey(pkey, UPLL_DT_IMPORT, dmi, ctrlr);
  if (UPLL_RC_SUCCESS != result_code
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    delete pkey;
    pkey = NULL;
    free(key_vlink);
    return result_code;
  }
  /* use the UNC VTN name if PFC VTN name is renamed; */
  if (strlen(reinterpret_cast<char *>(reinterpret_cast<key_vtn_t *>
                                              (pkey->get_key())->vtn_name)))
    uuu::upll_strncpy(key_vlink->vtn_key.vtn_name,
            reinterpret_cast<key_vtn_t *>(pkey->get_key())->vtn_name,
            (kMaxLenVtnName+1));
    delete pkey;
    pkey = NULL;
  okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink, key_vlink,
                          NULL);
  if (NULL == okey) {
    free(key_vlink);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key())) return UPLL_RC_ERR_GENERIC;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_rename_vnode_info *key_rename = reinterpret_cast<key_rename_vnode_info *>
                                                      (ikey->get_key());
  key_vlink_t * key_vlink = reinterpret_cast<key_vlink_t *>(
                              ConfigKeyVal::Malloc(sizeof(key_vlink_t)));
    UPLL_LOG_TRACE("Table is MainTable ");
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name))) {
      FREE_IF_NOT_NULL(key_vlink);
      return UPLL_RC_ERR_GENERIC;
     }

    uuu::upll_strncpy(key_vlink->vtn_key.vtn_name,
                key_rename->old_unc_vtn_name, (kMaxLenVtnName+1));

     if (ikey->get_key_type() == table[MAINTBL]->get_key_type()) {
       UPLL_LOG_TRACE("Current Rename Key type is %d", ikey->get_key_type());
       if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
         FREE_IF_NOT_NULL(key_vlink);
         return UPLL_RC_ERR_GENERIC;
       }
       uuu::upll_strncpy(key_vlink->vlink_name,
          key_rename->old_unc_vnode_name, (kMaxLenVlinkName+1));
     }
  okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink, key_vlink,
                          NULL);
  if (!okey) {
    free(key_vlink);
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t VlinkMoMgr::GetRenameInfo(ConfigKeyVal *ikey,
                                    ConfigKeyVal *okey,
                                    ConfigKeyVal *&rename_info,
                                    DalDmlIntf *dmi,
                                    const char *ctrlr_id,
                                    bool &renamed) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!okey || !ikey) {
    UPLL_LOG_DEBUG("Empty Input");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vlink_t * vlink_key_ = NULL;
  string vtn_id = "";

  vlink_key_ = reinterpret_cast<key_vlink_t *>(ikey->get_key());
  if (!vlink_key_) {
    UPLL_LOG_DEBUG("Input Key Is Empty");
    return UPLL_RC_ERR_GENERIC;
  }
  key_rename_vnode_info_t *vlink_rename_info =
                        reinterpret_cast<key_rename_vnode_info_t *>
                        (ConfigKeyVal::Malloc(sizeof(key_rename_vnode_info_t)));
  if (renamed) {
    if (!strlen(reinterpret_cast<char *>(reinterpret_cast<val_rename_vnode *>
                                       (GetVal(ikey))->ctrlr_vtn_name))) {
      free(vlink_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vlink_rename_info->ctrlr_vtn_name,
          reinterpret_cast<val_rename_vnode *>(GetVal(ikey))->ctrlr_vtn_name,
          (kMaxLenVtnName+1));
    if (!strlen(reinterpret_cast<char *>(reinterpret_cast<val_rename_vnode *>
                                         (GetVal(ikey))->ctrlr_vnode_name))) {
      free(vlink_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vlink_rename_info->ctrlr_vnode_name,
          reinterpret_cast<val_rename_vnode *>(GetVal(ikey))->ctrlr_vnode_name,
          (kMaxLenVlinkName+1));
  } else {
    if (!strlen(reinterpret_cast<char *>(vlink_key_->vlink_name))) {
       free(vlink_rename_info);
       return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vlink_rename_info->ctrlr_vnode_name,
           vlink_key_->vlink_name, (kMaxLenVlinkName+1));
    if (!strlen(reinterpret_cast<char *>(vlink_key_->vtn_key.vtn_name))) {
      free(vlink_rename_info);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vlink_rename_info->ctrlr_vtn_name,
           vlink_key_->vtn_key.vtn_name, (kMaxLenVtnName+1));
  }
  if (!strlen(reinterpret_cast<char *>(vlink_key_->vlink_name))) {
       free(vlink_rename_info);
       return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vlink_rename_info->old_unc_vnode_name,
         vlink_key_->vlink_name, (kMaxLenVlinkName+1));
  if (!strlen(reinterpret_cast<char *>(vlink_key_->vtn_key.vtn_name))) {
    free(vlink_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vlink_rename_info->new_unc_vtn_name,
         vlink_key_->vtn_key.vtn_name, (kMaxLenVtnName+1));
  uuu::upll_strncpy(vlink_rename_info->old_unc_vtn_name,
         vlink_key_->vtn_key.vtn_name, (kMaxLenVtnName+1));

  if (!(okey->get_key())) {
    free(vlink_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vlink_t *>
                                      (okey->get_key())->vlink_name))) {
    free(vlink_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(vlink_rename_info->new_unc_vnode_name,
       reinterpret_cast<key_vlink_t *>(okey->get_key())->vlink_name,
       (kMaxLenVlinkName+1));

  rename_info = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcInvalidStNum,
                                 vlink_rename_info, NULL);
  if (!rename_info) {
    free(vlink_rename_info);
    return UPLL_RC_ERR_GENERIC;
  }

  if (!renamed) {
    val_rename_vnode_t *vnode = reinterpret_cast<val_rename_vnode_t*>
        (ConfigKeyVal::Malloc(sizeof(val_rename_vnode_t)));
    ConfigKeyVal *tmp_key = NULL;
    result_code = GetChildConfigKey(tmp_key, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
       UPLL_LOG_DEBUG("GetChildConfigKey Failed ");
       free(vnode);  // COV RESOURCE LEAK
       return result_code;
    }
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};
    result_code = ReadConfigDB(tmp_key, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                                                 MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB Failed");
      free(vnode);  // COV RESOURCE LEAK
      delete tmp_key;
      return result_code;
    }
    controller_domain ctrlr_dom;
    result_code = GetControllerDomainId(tmp_key, &ctrlr_dom);
    if (UPLL_RC_SUCCESS != result_code) {
       delete tmp_key;
       return result_code;
    }
    SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);

    vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
    vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

    uuu::upll_strncpy(vnode->ctrlr_vtn_name,
           reinterpret_cast<key_vlink_t *>(ikey->get_key())->vtn_key.vtn_name,
            (kMaxLenVtnName+1));
    uuu::upll_strncpy(vnode->ctrlr_vnode_name,
           reinterpret_cast<key_vlink_t *>(ikey->get_key())->vlink_name,
          (kMaxLenVlinkName+1));
    ConfigVal *rename_val_ = new ConfigVal(IpctSt::kIpcInvalidStNum, vnode);
    okey->SetCfgVal(rename_val_);
    dbop.readop = kOpNotRead;
    result_code = UpdateConfigDB(okey, UPLL_DT_IMPORT, UNC_OP_CREATE, dmi,
                                 TC_CONFIG_GLOBAL, vtn_id, RENAMETBL);
    if (tmp_key)
      delete tmp_key;
  }
  return result_code;
}

upll_rc_t VlinkMoMgr::UpdateVnodeVal(ConfigKeyVal *rename_info,
                                     DalDmlIntf *dmi,
                                     upll_keytype_datatype_t data_type,
                                     bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_name = "";

  if (!rename_info) return UPLL_RC_ERR_GENERIC;
  key_rename_vnode_info *rename_info_ =
                          reinterpret_cast<key_rename_vnode_info *>(
                          rename_info->get_key());
  if (!rename_info_) {
    UPLL_LOG_DEBUG("Rename Info is Empty");
    return UPLL_RC_ERR_GENERIC;
  }

  uint8_t rename = 0;
  key_vlink_t *key_vlink_ =  NULL;
  val_vlink_t *val_vlink_ =  reinterpret_cast<val_vlink_t *>
      (ConfigKeyVal::Malloc(sizeof(val_vlink_t)));
  ConfigKeyVal *okey = NULL;
  UPLL_LOG_TRACE("Rename NoRename Falg = %d", no_rename);
  GetChildConfigKey(okey, NULL);

  if (!okey || !(okey->get_key())) {
    UPLL_LOG_TRACE("GetChildConfigKey Failed ");
    free(val_vlink_);
    if (okey) delete okey;
    return UPLL_RC_ERR_GENERIC;
  }
  okey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValVlink, val_vlink_));

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
                   kOpInOutFlag|kOpInOutCtrlr|kOpInOutDomain };

  key_vlink_ = reinterpret_cast<key_vlink_t *>(okey->get_key());

  uuu::upll_strncpy(key_vlink_->vtn_key.vtn_name,
         rename_info_->new_unc_vtn_name, (kMaxLenVtnName+1));

  uuu::upll_strncpy(val_vlink_->vnode1_name,
         rename_info_->old_unc_vnode_name, (kMaxLenVnodeName+1));

  val_vlink_->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_VALID;

  result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    ConfigKeyVal *tmp = okey;
    while (okey) {
      val_vlink_ =  reinterpret_cast<val_vlink_t *>(GetVal(okey));
      UPLL_LOG_TRACE("After Read ConfigDB in Vlink %s",
                                        (okey->ToStrAll()).c_str());

      GET_USER_DATA_FLAGS(okey->get_cfg_val(), rename);
      UPLL_LOG_TRACE("Rename No Rename %d", no_rename);

      UPLL_LOG_TRACE("Before Update Rename flag =%d", rename);
      if (!no_rename)
        rename = rename | VN1_RENAME;
      else
        rename = rename & NO_VN1_RENAME;

      UPLL_LOG_TRACE("After Update Rename flag =%d", rename);
      SET_USER_DATA_FLAGS(okey->get_cfg_val(), rename);

      uuu::upll_strncpy(val_vlink_->vnode1_name,
             rename_info_->new_unc_vnode_name, (kMaxLenVnodeName+1));

      UPLL_LOG_TRACE("The New vnode name %s", val_vlink_->vnode1_name);
      UPLL_LOG_TRACE("Before Update ConfigDB in Vlink %s",
                                        (okey->ToStrAll()).c_str());
      dbop.readop = kOpNotRead;
      dbop.inoutop =  kOpInOutFlag;

      result_code = UpdateConfigDB(okey, data_type, UNC_OP_UPDATE, dmi,
                                   &dbop, TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) return result_code;
      okey = okey->get_next_cfg_key_val();
    }
    if (tmp) {
      delete tmp;
    }
    return result_code;
  }

  val_vlink_ =  reinterpret_cast<val_vlink_t *>(GetVal(okey));
  val_vlink_->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
  uuu::upll_strncpy(val_vlink_->vnode2_name,
         rename_info_->old_unc_vnode_name, (kMaxLenVnodeName+1));
  val_vlink_->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_VALID;

  result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS == result_code) {
    ConfigKeyVal *temp = okey;
    while (okey) {
      val_vlink_ =  reinterpret_cast<val_vlink_t *>(GetVal(okey));

      GET_USER_DATA_FLAGS(okey->get_cfg_val(), rename);
      UPLL_LOG_TRACE("Rename No Rename %d", no_rename);
      UPLL_LOG_TRACE("Before Update Rename flag =%d", rename);

      if (!no_rename)
          rename = rename | VN2_RENAME;
      else
          rename = rename & NO_VN2_RENAME;

      UPLL_LOG_TRACE("After Update Rename flag =%d", rename);

      SET_USER_DATA_FLAGS(okey->get_cfg_val(), rename);

      uuu::upll_strncpy(val_vlink_->vnode2_name,
              rename_info_->new_unc_vnode_name, (kMaxLenVnodeName+1));
      dbop.readop = kOpNotRead;
      dbop.inoutop =  kOpInOutFlag;
      result_code = UpdateConfigDB(okey, data_type, UNC_OP_UPDATE, dmi,
                                   &dbop, TC_CONFIG_GLOBAL, vtn_name, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) return result_code;
      okey = okey->get_next_cfg_key_val();
    }
    if (temp) {
      delete temp;
    }
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  }
  if (okey)
    delete okey;
  return result_code;
}


upll_rc_t VlinkMoMgr::ValidateBoundary(uint8_t *boundary_name,
                                       IpcReqRespHeader *req,
                                       ConfigKeyVal *&ck_boundary) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  IpcResponse ipc_resp;
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";

  key_boundary *bndrykey = static_cast<key_boundary *>
         (ConfigKeyVal::Malloc(sizeof(key_boundary)));  // COV NULL RETURN

  uuu::upll_strncpy(bndrykey->boundary_id, boundary_name,
                   (kMaxLenBoundaryName+1));
  DELETE_IF_NOT_NULL(ck_boundary);
  ck_boundary = new ConfigKeyVal(UNC_KT_BOUNDARY, IpctSt::kIpcStKeyBoundary,
                                 bndrykey, NULL);

  result_code = SendIpcReq(USESS_ID_UPLL, 0, UNC_OP_READ,
                   req->datatype, ck_boundary, NULL, &ipc_resp);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in retrieving boundary data %d", result_code);
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    DELETE_IF_NOT_NULL(ck_boundary);
    return result_code;
  }

  //  Get acquired configuration mode from tc based on session_id and config_id
  if (req->datatype == UPLL_DT_CANDIDATE && req->operation != UNC_OP_DELETE) {
    result_code = GetConfigModeInfo(req, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetConfigMode failed");
      return result_code;
    }
    //  Acquired configuration mode is VTN specific mode,
    //  and so verify boundary existence in running db.
    // TODO(Tamil) remove the unwanted if check
    if (config_mode == TC_CONFIG_VTN) {
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      result_code = SendIpcReq(USESS_ID_UPLL, 0, UNC_OP_READ,
                             UPLL_DT_RUNNING, ck_boundary, NULL, &ipc_resp);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in retrieving boundary data %d", result_code);
        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
        DELETE_IF_NOT_NULL(ck_boundary);
        return result_code;
      }
    }
  }
  /* store the boundary data returned by physical in temp. class variable*/
  if (ipc_resp.ckv_data)
    ck_boundary->ResetWith(ipc_resp.ckv_data);
  else
    UPLL_LOG_DEBUG("Boundary information not obtained from physical");
  DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
  return result_code;
}

upll_rc_t VlinkMoMgr::IsKeyInUse(upll_keytype_datatype_t dt_type,
                                 const ConfigKeyVal *ckv, bool *in_use,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_link = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  key_vlink_t *vlink_key = reinterpret_cast<key_vlink_t *>
                                    (ConfigKeyVal::Malloc(sizeof(key_vlink)));
  val_vlink_t *linkval = static_cast<val_vlink_t *>
                         (ConfigKeyVal::Malloc(sizeof(val_vlink_t)));
  key_boundary *bndrykey = reinterpret_cast<key_boundary *>(ckv->get_key());
  if (!bndrykey ||
      !strlen(reinterpret_cast<const char *>(bndrykey->boundary_id))) {
    free(linkval);
    free(vlink_key);
    return UPLL_RC_ERR_GENERIC;
  }

  //  Populate vLink ConfigKeyVal
  uuu::upll_strncpy(linkval->boundary_name, bndrykey->boundary_id,
                    (kMaxLenBoundaryName+1));
  linkval->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] = UNC_VF_VALID;
  ck_link = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink, vlink_key,
                             new ConfigVal(IpctSt::kIpcStValVlink, linkval));
  //  Validate whether received bdry is used in vLink maintbl.
  //  If it is used set in_use as true
  result_code = ReadConfigDB(ck_link, dt_type, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  delete ck_link;
  if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    *in_use = (UPLL_RC_SUCCESS == result_code) ? true : false;
    return result_code;
  }
  //  Received boundary not used in vLink maintbl. Perform same check in vLink
  //  converttbl. set in_use as true, if boundary is used in vLink convertbl.

  //  Populate convert vLink ConfigKeyVal
  key_convert_vlink_t *conv_vlink_key =
      reinterpret_cast<key_convert_vlink_t *>
      (ConfigKeyVal::Malloc(sizeof(key_convert_vlink)));
  val_convert_vlink_t *link_conv_val = static_cast<val_convert_vlink_t *>
      (ConfigKeyVal::Malloc(sizeof(val_convert_vlink_t)));
  uuu::upll_strncpy(link_conv_val->boundary_name, bndrykey->boundary_id,
                    (kMaxLenBoundaryName+1));
  link_conv_val->valid[UPLL_IDX_BOUNDARY_NAME_CVLINK] = UNC_VF_VALID;
  ck_link = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyConvertVlink,
                             conv_vlink_key,
                             new ConfigVal(IpctSt::kIpcStValConvertVlink,
                                           link_conv_val));

  //  verify boundary is used in vLink convert tbl
  result_code = ReadConfigDB(ck_link, dt_type, UNC_OP_READ, dbop, dmi,
                             CONVERTTBL);
  delete ck_link;
  if (UPLL_RC_SUCCESS != result_code
      && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failure %d", result_code);
    return result_code;
  }
  *in_use = (UPLL_RC_SUCCESS == result_code) ? true : false;
  UPLL_LOG_TRACE("IsKeyInUse : %d", *in_use);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vlink_t *val;
  val = (ckv_running != NULL) ? reinterpret_cast<val_vlink_t *>
                                     (GetVal(ckv_running)) : NULL;
  if (NULL == val) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase) val->cs_row_status = cs_status;
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
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
    ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
     ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));

  result_code = SetConsolidatedStatus(ckv_running, op, cs_status, dmi);

  return result_code;
}

bool VlinkMoMgr::FilterAttributes(void *&val1, void *val2, bool copy_to_running,
                                  unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  val_vlink_t *val_vlink1 = reinterpret_cast<val_vlink_t *>(val1);
  val_vlink1->valid[UPLL_IDX_DESCRIPTION_VLNK] = UNC_VF_INVALID;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}

bool VlinkMoMgr::CompareValidValue(void *&val1, void *val2,
                                   bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
  val_vlink_t *val_vlink1 = reinterpret_cast<val_vlink_t *>(val1);
  val_vlink_t *val_vlink2 = reinterpret_cast<val_vlink_t *>(val2);
  for (unsigned int loop = 0;
      loop < sizeof(val_vlink1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vlink1->valid[loop]
        && UNC_VF_VALID == val_vlink2->valid[loop])
      val_vlink1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (UNC_VF_INVALID != val_vlink1->valid[UPLL_IDX_DESCRIPTION_VLNK]) {
    if ((!copy_to_running) ||
        ((UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_DESCRIPTION_VLNK]) &&
         (!strcmp(reinterpret_cast<char*>(val_vlink1->description),
                reinterpret_cast<char*>(val_vlink2->description)))))
      val_vlink1->valid[UPLL_IDX_DESCRIPTION_VLNK] = UNC_VF_INVALID;
  }
  if ((val_vlink2->valid[UPLL_IDX_ADMIN_STATUS_VLNK] ==
       val_vlink1->valid[UPLL_IDX_ADMIN_STATUS_VLNK])
      && UNC_VF_INVALID != val_vlink2->valid[UPLL_IDX_ADMIN_STATUS_VLNK]) {
    if (val_vlink1->admin_status == val_vlink2->admin_status)
      val_vlink1->valid[UPLL_IDX_ADMIN_STATUS_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_VNODE1_NAME_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_VNODE1_NAME_VLNK]) {
    if (!strcmp(reinterpret_cast<char*>(val_vlink1->vnode1_name),
                reinterpret_cast<char*>(val_vlink2->vnode1_name)))
      val_vlink1->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK]) {
    if (!strcmp(reinterpret_cast<char*>(val_vlink1->vnode1_ifname),
                reinterpret_cast<char*>(val_vlink2->vnode1_ifname)))
      val_vlink1->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_VNODE2_NAME_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_VNODE2_NAME_VLNK]) {
    if (!strcmp(reinterpret_cast<char*>(val_vlink1->vnode2_name),
                reinterpret_cast<char*>(val_vlink2->vnode2_name)))
      val_vlink1->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK]) {
    if (!strcmp(reinterpret_cast<char*>(val_vlink1->vnode2_ifname),
                reinterpret_cast<char*>(val_vlink2->vnode2_ifname)))
      val_vlink1->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_BOUNDARY_NAME_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_BOUNDARY_NAME_VLNK]) {
    if (!strcmp(reinterpret_cast<char*>(val_vlink1->boundary_name),
                reinterpret_cast<char*>(val_vlink2->boundary_name)))
      val_vlink1->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_LABEL_TYPE_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_LABEL_TYPE_VLNK]) {
    if (val_vlink1->label_type == val_vlink2->label_type)
      val_vlink1->valid[UPLL_IDX_LABEL_TYPE_VLNK] = UNC_VF_INVALID;
  }
  if (UNC_VF_VALID == val_vlink1->valid[UPLL_IDX_LABEL_VLNK]
      && UNC_VF_VALID == val_vlink2->valid[UPLL_IDX_LABEL_VLNK]) {
    if (val_vlink1->label == val_vlink2->label) {
      val_vlink1->valid[UPLL_IDX_LABEL_VLNK] = UNC_VF_INVALID;
    }
  }
  for (unsigned int loop = 0;
        loop < sizeof(val_vlink1->valid)/sizeof(val_vlink1->valid[0]);
                                                         ++loop ) {
    if ((UNC_VF_VALID == val_vlink1->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == val_vlink1->valid[loop]))
        invalid_attr = false;
  }
  return invalid_attr;
}

upll_rc_t VlinkMoMgr::ValidateMessage(IpcReqRespHeader *req,
    ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal is Null");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  unc_key_type_t keytype = ikey->get_key_type();
  if (UNC_KT_VLINK != keytype) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", keytype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVlink) {
    UPLL_LOG_DEBUG(
        "Invalid structure received.Expected struct-kIpcStKeyVlink, "
        "received struct -%d ",
        ((ikey->get_st_num())));
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vlink_t *key_vlink = reinterpret_cast<key_vlink_t *>(ikey->get_key());
  if (key_vlink == NULL) {
    UPLL_LOG_DEBUG("key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ret_val = ValidateVlinkKey(key_vlink, operation);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Key struct Validation failed KT_VLINK");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  val_vlink_t *val_vlink = NULL;
  val_rename_vlink_t *val_rename_vlink = NULL;
  if ((ikey->get_cfg_val())
      && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVlink)) {
    val_vlink =
      reinterpret_cast<val_vlink_t *>(ikey->get_cfg_val()->get_val());
  } else if ((ikey->get_cfg_val()) &&
      ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValRenameVlink)) {
    val_rename_vlink =
      reinterpret_cast<val_rename_vlink_t *>(ikey->get_cfg_val()->get_val());
  }
  if ((operation == UNC_OP_CREATE) &&
      (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_IMPORT)) {
    if (val_vlink == NULL) {
      UPLL_LOG_DEBUG("Value structure is mandatory for CREATE op");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    UPLL_LOG_TRACE("The Input Key is %s", (ikey->ToStrAll()).c_str());
    ret_val = ValidateVlinkValue(val_vlink, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("val struct Validation failed for CREATE op ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_UPDATE) &&
      (dt_type == UPLL_DT_CANDIDATE)) {
    if (val_vlink == NULL) {
      UPLL_LOG_DEBUG("Value structure is mandatory for UPDATE op");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    ret_val = ValidateVlinkValue(val_vlink, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("val struct validation failed for UPDATE op");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_RENAME) && (dt_type == UPLL_DT_IMPORT)) {
    if (val_rename_vlink == NULL) {
      UPLL_LOG_DEBUG("Value rename struct is mandatory for RENAME op");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    ret_val = ValidateRenameVlinkValue(val_rename_vlink);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("val rename struct validation failed for Rename op");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING ||
        operation == UNC_OP_READ_SIBLING_BEGIN) &&
      (dt_type == UPLL_DT_IMPORT)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG("option2 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    if (val_rename_vlink == NULL) {
      UPLL_LOG_DEBUG("val rename structure is an optional for READ op");
      return UPLL_RC_SUCCESS;
    }
    ret_val = ValidateRenameVlinkValue(val_rename_vlink);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("val rename struct validation failed for READ op");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING ||
        operation == UNC_OP_READ_SIBLING_BEGIN) &&
      (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
       dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE)) {
    if (option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG("option1 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG("option2 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    if (val_vlink == NULL) {
      UPLL_LOG_DEBUG("Value struct is an optional for READ op");
      return UPLL_RC_SUCCESS;
    }
    ret_val = ValidateVlinkValue(val_vlink, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("val struct validation failed for READ op");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_DELETE) ||
             (operation == UNC_OP_READ_SIBLING_COUNT) ||
             (operation == UNC_OP_READ_NEXT) ||
             (operation == UNC_OP_READ_BULK)) {
    UPLL_LOG_DEBUG("Value structure validation is none for this operation :%d"
               , operation);
    return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_DEBUG("Invalid datatype(%d) or operation(%d)", dt_type,
                  operation);
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
}

upll_rc_t VlinkMoMgr::ValidateVlinkValue(val_vlink_t *val_vlink,
                                         unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  bool ret_val = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE("Operation type :(%d)", operation);

  if ((operation == UNC_OP_CREATE) &&
     ((val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] == UNC_VF_INVALID) ||
      (val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] == UNC_VF_VALID_NO_VALUE)||
      (val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] == UNC_VF_INVALID) ||
      (val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] ==
                                                     UNC_VF_VALID_NO_VALUE) ||
      (val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] == UNC_VF_INVALID) ||
      (val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] == UNC_VF_VALID_NO_VALUE) ||
      (val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] == UNC_VF_INVALID) ||
      (val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] ==
                                                     UNC_VF_VALID_NO_VALUE))) {
    UPLL_LOG_DEBUG("Vlink node name/If name is mandatory "
                                               " for Create operation");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation == UNC_OP_UPDATE) &&
     ((val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] != UNC_VF_INVALID) ||
     (val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] != UNC_VF_INVALID) ||
     (val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] != UNC_VF_INVALID) ||
     (val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK]  != UNC_VF_INVALID))) {
    UPLL_LOG_DEBUG("Vlink node name/If name must be invalid "
                                               " for Update operation");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK] == UNC_VF_VALID) {
    if (!ValidateNumericRange(val_vlink->admin_status,
                              (uint8_t)(UPLL_ADMIN_ENABLE),
                              (uint8_t)(UPLL_ADMIN_DISABLE), true, true)) {
      UPLL_LOG_DEBUG("Syntax check failed. admin_status - %d",
                     val_vlink->admin_status);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK]
      == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vlink->admin_status = UPLL_ADMIN_ENABLE;
  } else if ((val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK] == UNC_VF_INVALID)
      && (operation == UNC_OP_CREATE)) {
    val_vlink->admin_status = UPLL_ADMIN_ENABLE;
    val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK] = UNC_VF_VALID_NO_VALUE;
  }
  if ((val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] == UNC_VF_VALID) &&
      (!READ_OP(operation))) {
  if ((val_vlink->vnode1_name[0] == '\0')
      || (val_vlink->vnode1_ifname[0] == '\0')
      || (val_vlink->vnode2_name[0] == '\0')
      || (val_vlink->vnode2_ifname[0] == '\0')) {
    UPLL_LOG_DEBUG("Vlink node name/If name is empty!!");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  }
  if (val_vlink->valid[UPLL_IDX_DESCRIPTION_VLNK] == UNC_VF_VALID) {
    if (!ValidateDesc(val_vlink->description,
                         kMinLenDescription, kMaxLenDescription)) {
      UPLL_LOG_DEBUG("Syntax check failed. description-  (%s)",
                    val_vlink->description);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vlink->valid[UPLL_IDX_DESCRIPTION_VLNK] ==
              UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vlink->description[0] = ' ';
    val_vlink->description[1] = '\0';
  }
  if (val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(val_vlink->vnode1_name),
                          kMinLenVnodeName, kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. vnode1_name- (%s)",
                    val_vlink->vnode1_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(val_vlink->vnode1_ifname),
                          kMinLenInterfaceName, kMaxLenInterfaceName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. vnode1_ifname- %s",
                    val_vlink->vnode1_ifname);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  if (val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(val_vlink->vnode2_name),
                          kMinLenVnodeName, kMaxLenVnodeName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. vnode2_name- (%s)",
                    val_vlink->vnode2_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if (val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char*>(val_vlink->vnode2_ifname),
                          kMinLenInterfaceName, kMaxLenInterfaceName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed."
                    "vnode2_ifname- (%s)",
                    val_vlink->vnode2_ifname);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }

  if (val_vlink->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char *>(val_vlink->boundary_name),
                          kMinLenBoundaryName, kMaxLenBoundaryName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. "
                    "boundary_name- (%s)",
                    val_vlink->boundary_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }

    if (val_vlink->valid[UPLL_IDX_LABEL_TYPE_VLNK] != UNC_VF_VALID ||
        val_vlink->label_type != UPLL_LABEL_TYPE_VLAN ||
        val_vlink->valid[UPLL_IDX_LABEL_VLNK] != UNC_VF_VALID) {
      UPLL_LOG_DEBUG("label_id is mandatory if boundary is specified");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
#if 0
    // Boundary validation
    ret_val = ValidateBoundary((val_vlink->boundary_name));
    if (ret_val != UPLL_RC_SUCCESS) return UPLL_RC_ERR_CFG_SEMANTIC;
#endif
  } else if ((val_vlink->valid[UPLL_IDX_BOUNDARY_NAME_VLNK]
      == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
    val_vlink->boundary_name[0] = ' ';
    val_vlink->boundary_name[1] = '\0';
    // In U17, VLAN_ID attribute is going to change as label_type and label.
    // If Boundary name is erased then vlan_id details should be cleared.
    val_vlink->label = 0;
    val_vlink->label_type = 0;
    val_vlink->valid[UPLL_IDX_LABEL_VLNK] = UNC_VF_VALID_NO_VALUE;
    val_vlink->valid[UPLL_IDX_LABEL_TYPE_VLNK] = UNC_VF_VALID_NO_VALUE;
  }

  if (val_vlink->valid[UPLL_IDX_LABEL_TYPE_VLNK] == UNC_VF_VALID) {
    if (val_vlink->label_type != UPLL_LABEL_TYPE_VLAN ||
        val_vlink->valid[UPLL_IDX_LABEL_VLNK] != UNC_VF_VALID) {
      UPLL_LOG_DEBUG("Label and Label type valid flag invalid combination");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vlink->valid[UPLL_IDX_LABEL_TYPE_VLNK] ==
              UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
     val_vlink->label_type = 0;
  }

  if (val_vlink->valid[UPLL_IDX_LABEL_VLNK] == UNC_VF_VALID) {
    if (val_vlink->valid[UPLL_IDX_LABEL_TYPE_VLNK] != UNC_VF_VALID ||
        (val_vlink->valid[UPLL_IDX_LABEL_TYPE_VLNK] == UNC_VF_VALID &&
         val_vlink->label_type != UPLL_LABEL_TYPE_VLAN)) {
    }
    //  no-vlan-id , vlanid(1 to 4096)
    if ((val_vlink->label != NO_VLAN_ID) &&
        !(ValidateNumericRange(val_vlink->label,
         (uint32_t)(kMinVlanId), (uint32_t)
         (kMaxVlanId), true, true))) {
      UPLL_LOG_DEBUG("Syntax check failed. vlan_id- (%d)", val_vlink->label);
     return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if ((val_vlink->valid[UPLL_IDX_LABEL_VLNK] == UNC_VF_VALID_NO_VALUE)
      && ((operation == UNC_OP_UPDATE) || (operation == UNC_OP_CREATE))) {
     val_vlink->label = 0;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::ValidateRenameVlinkValue(
    val_rename_vlink_t *val_rename_vlink) {
  UPLL_FUNC_TRACE;
  bool ret_val = UPLL_RC_SUCCESS;
  if (val_rename_vlink->valid[UPLL_IDX_NEW_NAME_RVLNK] == UNC_VF_VALID) {
    ret_val = ValidateKey(reinterpret_cast<char*>(val_rename_vlink->new_name),
                          kMinLenVlinkName, kMaxLenVlinkName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed."
                    "new_name- (%s)",
                    val_rename_vlink->new_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::ValidateVlinkKey(key_vlink_t *key_vlink,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  int ret_val = UPLL_RC_SUCCESS;
  VtnMoMgr *objvtnmgr =
      reinterpret_cast<VtnMoMgr*>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VTN)));
  if (NULL == objvtnmgr) {
    UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }
  ret_val = objvtnmgr->ValidateVtnKey(&(key_vlink->vtn_key));

  if (UPLL_RC_SUCCESS != ret_val) {
    UPLL_LOG_DEBUG("Vtn_name syntax validation failed.Err Code- %d", ret_val);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if ((operation != UNC_OP_READ_SIBLING_BEGIN) &&
      (operation != UNC_OP_READ_SIBLING_COUNT)) {
    ret_val = ValidateKey(reinterpret_cast<char *>(key_vlink->vlink_name),
        kMinLenVlinkName, kMaxLenVlinkName);

    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Syntax check failed. vlink_name (%s)",
          key_vlink->vlink_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
      StringReset(key_vlink->vlink_name);
  }
  UPLL_LOG_TRACE("key structure validation successful for VLINK keytype");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::ValVlinkAttributeSupportCheck(val_vlink_t *val_vlink,
    const uint8_t* attrs, unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  if ((val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK] == UNC_VF_VALID)
      || (val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vlink::kCapAdminStatus] == 0) {
      val_vlink->valid[UPLL_IDX_ADMIN_STATUS_VLNK] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG(
            "UPLL_IDX_ADMIN_STATUS_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }

  if ((val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] == UNC_VF_VALID)
      || (val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] ==
        UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vlink::kCapVnode1Name] == 0) {
      val_vlink->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG(
            "UPLL_IDX_VNODE1_NAME_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }

  if ((val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] == UNC_VF_VALID)
      || (val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vlink::kCapVnode1IfName] == 0) {
      val_vlink->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG(
            "UPLL_IDX_VNODE1_IF_NAME_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }

  if ((val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] == UNC_VF_VALID)
      || (val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] ==
        UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vlink::kCapVnode2Name] == 0) {
      val_vlink->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG(
            "UPLL_IDX_VNODE2_NAME_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }

  if ((val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] == UNC_VF_VALID)
      || (val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vlink::kCapVnode2IfName] == 0) {
      val_vlink->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG(
            "UPLL_IDX_VNODE2_IF_NAME_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }

  if ((val_vlink->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID)
      || (val_vlink->valid[UPLL_IDX_BOUNDARY_NAME_VLNK]
        == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vlink::kCapBoundaryName] == 0) {
      val_vlink->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG(
            "UPLL_IDX_BOUNDARY_NAME_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }

  if ((val_vlink->valid[UPLL_IDX_LABEL_TYPE_VLNK] == UNC_VF_VALID)
      || (val_vlink->valid[UPLL_IDX_LABEL_TYPE_VLNK] ==
          UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vlink::kCapLabelType] == 0) {
      val_vlink->valid[UPLL_IDX_LABEL_TYPE_VLNK] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("UPLL_IDX_LABEL_TYPE_VLINK not supported"
                       " in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }

  if ((val_vlink->valid[UPLL_IDX_LABEL_VLNK] == UNC_VF_VALID)
      || (val_vlink->valid[UPLL_IDX_LABEL_VLNK] == UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vlink::kCapLabel] == 0) {
      val_vlink->valid[UPLL_IDX_LABEL_VLNK] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("UPLL_IDX_LABEL_VLNK not supported in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }
  if ((val_vlink->valid[UPLL_IDX_DESCRIPTION_VLNK] == UNC_VF_VALID)
      || (val_vlink->valid[UPLL_IDX_DESCRIPTION_VLNK] ==
                                                UNC_VF_VALID_NO_VALUE)) {
    if (attrs[unc::capa::vlink::kCapDesc] == 0) {
      val_vlink->valid[UPLL_IDX_DESCRIPTION_VLNK] = UNC_VF_INVALID;
      if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
        UPLL_LOG_DEBUG("UPLL_IDX_DESCRIPTION_VLNK not supported "
                       "in pfc controller");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::ValidateCapability(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  if (!ikey || !req) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name) {
    ctrlr_name = reinterpret_cast<char*>((reinterpret_cast<key_user_data_t *>
                  (ikey->get_user_data()))->ctrlr_id);
    if (!ctrlr_name || !strlen(ctrlr_name)) {
      UPLL_LOG_DEBUG("Controller Name is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  UPLL_LOG_TRACE("ctrlr_name(%s), operation : (%d)",
      ctrlr_name, req->operation);

  bool result_code = false;
  uint32_t max_instance_count = 0;
  uint32_t max_attrs = 0;
  const uint8_t *attrs = NULL;

  switch (req->operation) {
    case UNC_OP_CREATE:
      UPLL_LOG_TRACE(" The Controller Name is %s", ctrlr_name);
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
          &max_instance_count, &max_attrs, &attrs);
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
      UPLL_LOG_DEBUG("Invalid operation");
      break;
  }
  if (!result_code) {
    UPLL_LOG_DEBUG("keytype(%d) is not supported by controller(%s) "
        "for operation(%d)",
        ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  val_vlink_t *val_vlink = NULL;
  if ((ikey->get_cfg_val())
      && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVlink)) {
    val_vlink = reinterpret_cast<val_vlink_t *>(ikey->get_cfg_val()->get_val());
  }

  if (val_vlink) {
    if (max_attrs > 0) {
      return ValVlinkAttributeSupportCheck(val_vlink, attrs, req->operation);
    } else {
      UPLL_LOG_DEBUG("Attribute list is empty for operation %d",
                                                 req->operation);
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::CheckVnodeInfo(ConfigKeyVal *ikey,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  ConfigVal *val = NULL;
  if (!ikey) return UPLL_RC_ERR_GENERIC;

  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed result code %d", result_code);
    return result_code;
  }

  val_vlink_t *vlink_val = reinterpret_cast<val_vlink_t *>
                               (ConfigKeyVal::Malloc(sizeof(val_vlink_t)));
  vlink_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_VALID;
  vlink_val->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_VALID;
  vlink_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_INVALID;
  vlink_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_INVALID;

  switch (ikey->get_key_type()) {
    case UNC_KT_VBRIDGE:
    case UNC_KT_VBR_IF:
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vbr_if_t*>
                                  (ikey->get_key())->vbr_key.vbridge_name))) {
        free(vlink_val);  // COV RESOURCE LEAK
        delete okey;
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(vlink_val->vnode1_name,
             reinterpret_cast<key_vbr_if_t*>(ikey->get_key())->
             vbr_key.vbridge_name, (kMaxLenVnodeName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vbr_if_t*>
                                       (ikey->get_key())->if_name))) {
        free(vlink_val);  // COV RESOURCE LEAK
        delete okey;
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(vlink_val->vnode1_ifname,
             reinterpret_cast<key_vbr_if_t*>(ikey->get_key())->if_name,
              (kMaxLenInterfaceName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vbr_if_t*>
                                  (ikey->get_key())->vbr_key.vbridge_name))) {
        free(vlink_val);  // COV RESOURCE LEAK
        delete okey;
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(vlink_val->vnode2_name, reinterpret_cast<key_vbr_if_t*>
                                      (ikey->get_key())->vbr_key.vbridge_name,
                   (kMaxLenVnodeName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vbr_if_t*>
                                      (ikey->get_key())->if_name))) {
        free(vlink_val);  // COV RESOURCE LEAK
        delete okey;
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(vlink_val->vnode2_ifname,
             reinterpret_cast<key_vbr_if_t*>(ikey->get_key())->if_name,
              (kMaxLenInterfaceName+1));

      break;
    case UNC_KT_VROUTER:
    case UNC_KT_VRT_IF:
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vrt_if_t*>
                                  (ikey->get_key())->vrt_key.vrouter_name))) {
        free(vlink_val);  // COV RESOURCE LEAK
        delete okey;
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(vlink_val->vnode1_name,
        reinterpret_cast<key_vrt_if_t*>(ikey->get_key())->vrt_key.vrouter_name,
         (kMaxLenVnodeName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vrt_if_t*>
                                          (ikey->get_key())->if_name))) {
        free(vlink_val);  // COV RESOURCE LEAK
        delete okey;
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(vlink_val->vnode1_ifname,
             reinterpret_cast<key_vrt_if_t*>(ikey->get_key())->if_name,
             (kMaxLenInterfaceName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vrt_if_t*>
                                  (ikey->get_key())->vrt_key.vrouter_name))) {
        free(vlink_val);  // COV RESOURCE LEAK
        delete okey;
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(vlink_val->vnode2_name,
        reinterpret_cast<key_vrt_if_t*>(ikey->get_key())->vrt_key.vrouter_name,
        (kMaxLenVnodeName+1));
      if (!strlen(reinterpret_cast<char *>(reinterpret_cast<key_vrt_if_t*>
                                          (ikey->get_key())->if_name))) {
        free(vlink_val);  // COV RESOURCE LEAK
        delete okey;
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(vlink_val->vnode2_ifname,
             reinterpret_cast<key_vrt_if_t*>(ikey->get_key())->if_name,
             (kMaxLenInterfaceName+1));
      break;
    default:
      break;
  }
  val = new ConfigVal(IpctSt::kIpcStValVlink, vlink_val);
  if (!val) {
    free(vlink_val);
    delete okey;  // COV RESOURCE LEAK
    return UPLL_RC_ERR_GENERIC;
  }
  okey->SetCfgVal(val);
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS == result_code) {
    delete okey;
    return result_code;
  }
  vlink_val->valid[UPLL_IDX_VNODE1_NAME_VLNK] = UNC_VF_INVALID;
  vlink_val->valid[UPLL_IDX_VNODE1_IF_NAME_VLNK] = UNC_VF_INVALID;
  vlink_val->valid[UPLL_IDX_VNODE2_NAME_VLNK] = UNC_VF_VALID;
  vlink_val->valid[UPLL_IDX_VNODE2_IF_NAME_VLNK] = UNC_VF_VALID;

  DbSubOp dbop1 = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop1, dmi, MAINTBL);
  delete okey;
  return result_code;
}

upll_rc_t VlinkMoMgr::IsReferenced(IpcReqRespHeader *req,
                                   ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (!ikey) return UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *ck_vlink = NULL;
  upll_rc_t result_code = GetChildConfigKey(ck_vlink, ikey);
  if (!ck_vlink || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Invalid param %d", result_code);
    return result_code;
  }
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone,
    kOpInOutFlag | kOpInOutDomain | kOpInOutCtrlr};
  result_code = ReadConfigDB(ck_vlink, req->datatype, UNC_OP_READ, dbop1,
                                  dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Invalid param %d", result_code);
    delete ck_vlink;
    return result_code;
  }
  // uint8_t iftype, if1_type, if2_type;
  uint8_t if_flag = 0;
  /* reset the bits of the interfaces constituiing the vlink */
  SET_USER_DATA_FLAGS(ck_vlink->get_cfg_val(), kVlinkVnode1);

  /* In bdry vlanmap case/bdry vbrportmap case: During vlink delete, get
   * logical_port_id from UPPL. It is required for updating flag or delete
   * correct entry in vlan-map/vbr-portmap table */
  ConfigKeyVal *ck_boundary = NULL;
  if ((GetVlinkVnodeIfKeyType(ck_vlink, 0) == UNC_KT_VBR_VLANMAP) ||
      (GetVlinkVnodeIfKeyType(ck_vlink, 1) == UNC_KT_VBR_VLANMAP) ||
      (GetVlinkVnodeIfKeyType(ck_vlink, 0) == UNC_KT_VBR_PORTMAP) ||
      (GetVlinkVnodeIfKeyType(ck_vlink, 1) == UNC_KT_VBR_PORTMAP)) {
    req->operation = UNC_OP_DELETE;

    val_vlink_t *vlink_val = reinterpret_cast<val_vlink_t *>(GetVal(ck_vlink));
    /* Get logical_port_id details from physical*/
    upll_rc_t rc = ValidateBoundary(vlink_val->boundary_name, req,
                    ck_boundary);
    if (rc != UPLL_RC_SUCCESS) {
      delete ck_vlink;
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  }
  for (int i = 0 ; i < 2; i++) {
    bool executed = false;
    unc_key_type_t ktype;
    ktype = GetVlinkVnodeIfKeyType(ck_vlink, i);
    UPLL_LOG_DEBUG("Vlink Interface %d type : if1 %d", i, ktype);
    if (ktype == UNC_KT_ROOT) {
      UPLL_LOG_DEBUG("Invalid param");
      delete ck_vlink;
      DELETE_IF_NOT_NULL(ck_boundary);
      return UPLL_RC_ERR_GENERIC;
    }
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                        (GetMoManager(ktype)));
    ConfigKeyVal *ck_vnif = NULL;
    result_code = mgr->GetChildConfigKey(ck_vnif, ck_vlink);
    if (!ck_vnif || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      delete ck_vlink;
      DELETE_IF_NOT_NULL(ck_boundary);
      return result_code;
    }
    /* VlanmapOnBoundary: Boundary-vlanmap case: vlink deletion-
     * Update the flag & ref_count in vlanmap_tbl if ref(user) exists else
     * delete the entry in vlanmap_tbl */
    if (ktype == UNC_KT_VBR_VLANMAP) {
      SET_USER_DATA_FLAGS(ck_vnif, 0);

      uint8_t *logical_port_id = NULL;
      uint8_t flags = 0;

      GET_USER_DATA_FLAGS(ck_vlink->get_cfg_val(), flags);
      flags &= VLINK_FLAG_NODE_POS;

      val_boundary_t *boundary_val = reinterpret_cast<val_boundary_t *>(
                                           GetVal(ck_boundary));
#if 0
      if (flags == kVlinkVnode2)
        logical_port_id = boundary_val->logical_port_id2;
      else
        logical_port_id = boundary_val->logical_port_id1;
#else
      controller_domain ctrlr_dom;
      ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;

      /* Verifies controller_name and domain name and gets
       * vlanmap logical_port_id */
      GET_USER_DATA_CTRLR_DOMAIN(ck_vnif, ctrlr_dom);
      UPLL_LOG_TRACE("ctrlr_name = %s domain_name = %s", ctrlr_dom.ctrlr,
          ctrlr_dom.domain);

      /* If controller or domain is NULL, return error */
      if (!ctrlr_dom.ctrlr || !ctrlr_dom.domain) {
        UPLL_LOG_ERROR("Controller_name or domain_name is NULL");
        DELETE_IF_NOT_NULL(ck_boundary);
        return UPLL_RC_ERR_GENERIC;
      }
      if ((!strcmp(reinterpret_cast<char *>(ctrlr_dom.ctrlr),
         reinterpret_cast<char *>(boundary_val->controller_name1))) &&
         (!strcmp(reinterpret_cast<char *>(ctrlr_dom.domain),
                   reinterpret_cast<char *>(boundary_val->domain_name1)))) {
        logical_port_id = boundary_val->logical_port_id1;
      } else if ((!strcmp(reinterpret_cast<char *>(ctrlr_dom.ctrlr),
              reinterpret_cast<char *>(boundary_val->controller_name2))) &&
          (!strcmp(reinterpret_cast<char *>(ctrlr_dom.domain),
                   reinterpret_cast<char *>(boundary_val->domain_name2)))) {
        logical_port_id = boundary_val->logical_port_id2;
      } else {
        UPLL_LOG_ERROR(" Controller and domain match not found");
        DELETE_IF_NOT_NULL(ck_boundary);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
#endif

      key_vlan_map_t *vlanmap_key  = reinterpret_cast<key_vlan_map_t *>(
                                         ck_vnif->get_key());

      /* Populate the logcial port id before updating vlanmap tbl */
      if (logical_port_id) {
        uuu::upll_strncpy(vlanmap_key->logical_port_id, logical_port_id,
            (kMaxLenLogicalPortId + 1));
        vlanmap_key->logical_port_id_valid = PFC_TRUE;
      }
    }
    if (ktype != UNC_KT_VBR_PORTMAP) {
      result_code = mgr->ReadConfigDB(ck_vnif, req->datatype, UNC_OP_READ,
          dbop1, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returing error %d", result_code);
        delete ck_vnif;
        delete ck_vlink;
        DELETE_IF_NOT_NULL(ck_boundary);
        return result_code;
      }
    }

    // Get the config_id and vtn_name from tclib
    TcConfigMode config_mode = TC_CONFIG_INVALID;
    std::string vtn_name = "";
    result_code = GetConfigModeInfo(req, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetConfigMode failed");
      delete ck_vnif;
      delete ck_vlink;
      DELETE_IF_NOT_NULL(ck_boundary);
      return result_code;
    }

    /* VlanmapOnBoundary: If kt is vlanmap (bdry vlanmap case):
     * NotifyPOMForPortMapVlinkFlag is not reqd
     * UpdateVlinkMemIfFlag will be done inside switch case */
    if (ktype != UNC_KT_VBR_VLANMAP &&
        ktype != UNC_KT_VBR_PORTMAP) {
      GET_USER_DATA_FLAGS(ck_vnif, if_flag);
      if_flag &= ~VIF_TYPE;
      SET_USER_DATA_FLAGS(ck_vnif, if_flag);
      vnode_if_type vnif_type;
      result_code = NotifyPOMForPortMapVlinkFlag(req->datatype,
          ck_vnif,
          dmi,
          UNC_OP_DELETE, config_mode, vtn_name);

      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("NotifyPOMForPortMapVlinkFlag returning error %d",
            result_code);
        delete ck_vnif;
        delete ck_vlink;
        DELETE_IF_NOT_NULL(ck_boundary);
        return result_code;
      }
      result_code = UpdateVlinkMemIfFlag(req->datatype, ck_vnif, dmi,
          vnif_type, mgr, UNC_OP_DELETE,
          config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        delete ck_vnif;
        delete ck_vlink;
        DELETE_IF_NOT_NULL(ck_boundary);
        return result_code;
      }
      UPLL_LOG_DEBUG("Reset bit for iftype %d", vnif_type);
    }
    switch (ktype) {
      case UNC_KT_VBR_IF: {
        val_drv_vbr_if *val_drv_vbr = reinterpret_cast<val_drv_vbr_if *>
                                  (GetVal(ck_vnif));
        if (val_drv_vbr == NULL) {
          UPLL_LOG_DEBUG("Invalid param");
          delete ck_vnif;
          delete ck_vlink;
          DELETE_IF_NOT_NULL(ck_boundary);
          return UPLL_RC_ERR_GENERIC;
        }
        if (val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
          executed = true;
          val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] =
                                        UNC_VF_VALID_NO_VALUE;
          val_drv_vbr->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] =
                                                           UNC_VF_INVALID;
          result_code = reinterpret_cast<VbrIfMoMgr *>(mgr)->
                           UpdateConfigVal(ck_vnif, req->datatype, dmi,
                                           config_mode, vtn_name);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Returning %d", result_code);
            delete ck_vnif;
            delete ck_vlink;
            DELETE_IF_NOT_NULL(ck_boundary);
            return result_code;
          }
        }
        break;
      }
      case UNC_KT_VTEP_IF: {
        val_vtep_if *vtepif_val = static_cast<val_vtep_if *>(GetVal(ck_vnif));
        if (vtepif_val == NULL) {
          UPLL_LOG_DEBUG("Invalid param");
          delete ck_vnif;
          delete ck_vlink;
          DELETE_IF_NOT_NULL(ck_boundary);
          return UPLL_RC_ERR_GENERIC;
        }
        if (vtepif_val->valid[UPLL_IDX_PORT_MAP_VTEPI] == UNC_VF_VALID) {
          executed = true;
          vtepif_val->valid[UPLL_IDX_ADMIN_ST_VTEPI] = UNC_VF_INVALID;
          vtepif_val->valid[UPLL_IDX_PORT_MAP_VTEPI] = UNC_VF_VALID_NO_VALUE;
          result_code = reinterpret_cast<VtepIfMoMgr *>(mgr)->
                         UpdateConfigVal(ck_vnif, req->datatype, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("UpdateConfigVal returned %d", result_code);
            delete ck_vnif;
            delete ck_vlink;
            DELETE_IF_NOT_NULL(ck_boundary);
            return result_code;
          }
        }
        break;
      }
      case UNC_KT_VTUNNEL_IF: {
        val_vtunnel_if *vtunnelif_val = static_cast<val_vtunnel_if *>
                                        (GetVal(ck_vnif));
        if (vtunnelif_val == NULL) {
          UPLL_LOG_DEBUG("Invalid param");
          delete ck_vlink;
          DELETE_IF_NOT_NULL(ck_boundary);
          return UPLL_RC_ERR_GENERIC;
        }
        if (vtunnelif_val->valid[UPLL_IDX_PORT_MAP_VTNL_IF] == UNC_VF_VALID) {
          executed = true;
          vtunnelif_val->valid[UPLL_IDX_PORT_MAP_VTNL_IF] =
                               UNC_VF_VALID_NO_VALUE;
          vtunnelif_val->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] = UNC_VF_INVALID;
          result_code = reinterpret_cast<VtunnelIfMoMgr *>(mgr)->
                         UpdateConfigVal(ck_vnif, req->datatype, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            delete ck_vnif;
            delete ck_vlink;
            DELETE_IF_NOT_NULL(ck_boundary);
            UPLL_LOG_DEBUG("UpdateConfigVal returned %d", result_code);
            return result_code;
          }
        }
        break;
      }
      /* VlanmapOnBoundary: Added vlanmap case */
      case UNC_KT_VBR_VLANMAP:
      case UNC_KT_VBR_PORTMAP: {
        if (ktype == UNC_KT_VBR_VLANMAP) {
        pfcdrv_val_vlan_map_t *vlanmap_val = static_cast
            <pfcdrv_val_vlan_map_t *> (GetVal(ck_vnif));
        if (vlanmap_val == NULL) {
          UPLL_LOG_DEBUG("vlanmap_val is NULL");
          delete ck_vlink;
          DELETE_IF_NOT_NULL(ck_boundary);
          return UPLL_RC_ERR_GENERIC;
        }
        uint8_t flags = 0;
        GET_USER_DATA_FLAGS(ck_vnif, flags);

        if (vlanmap_val->vm.valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID) {
          /* Update only bdry_ref_count or flag if it refered with
           * other vlink or vlan_map*/
          if (((flags & USER_VLANMAP_FLAG) && vlanmap_val->bdry_ref_count > 0)||
              (flags & USER_VLANMAP_FLAG) || vlanmap_val->bdry_ref_count > 1) {
            UPLL_LOG_TRACE("Both user & boundary configured; bdry_ref_count:%u",
                vlanmap_val->bdry_ref_count);
            if (vlanmap_val->bdry_ref_count == 1) {
              UPLL_LOG_TRACE("Vlanmap Flag before reset = %u", flags);
              flags &= ~BOUNDARY_VLANMAP_FLAG;
              SET_USER_DATA_FLAGS(ck_vnif, flags);
              vlanmap_val->valid[PFCDRV_IDX_BDRY_REF_COUNT] =
                  UNC_VF_VALID_NO_VALUE;
            }

            vlanmap_val->bdry_ref_count--;
            DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
            result_code = mgr->UpdateConfigDB(ck_vnif, req->datatype,
                                              UNC_OP_UPDATE, dmi, &dbop1,
                                              config_mode, vtn_name, MAINTBL);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_ERROR("bdry_ref_count update failed:%d", result_code);
              delete ck_vnif;
              delete ck_vlink;
              DELETE_IF_NOT_NULL(ck_boundary);
              return result_code;
            }
          } else {
            /* Deletes vlanmap entry*/
            DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
            result_code = mgr->UpdateConfigDB(ck_vnif, req->datatype,
                                              UNC_OP_DELETE, dmi, &dbop,
                                              config_mode, vtn_name, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("UpdateConfigDB returns error = %d", result_code);
              delete ck_vnif;
              delete ck_vlink;
              DELETE_IF_NOT_NULL(ck_boundary);
              return result_code;
            }
          }
        }
      }
      if (ktype == UNC_KT_VBR_PORTMAP) {
        result_code = mgr->BoundaryMapReq(req, ikey, ck_vlink,
                                              ck_vnif, ck_boundary, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("UpdateConfigDB returns error = %d", result_code);
          delete ck_vnif;
          delete ck_vlink;
          DELETE_IF_NOT_NULL(ck_boundary);
          return result_code;
        }
       }
       delete ck_vnif;
        // reset vlink represent bit in vbr_if flag
        ConfigKeyVal *vbrif_ckv = NULL;
        MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                        (GetMoManager(UNC_KT_VBR_IF)));
        result_code = mgr->GetChildConfigKey(vbrif_ckv, ck_vlink);
        if (!vbrif_ckv || result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey error = %d", result_code);
          delete ck_vlink;
          DELETE_IF_NOT_NULL(ck_boundary);
          return result_code;
        }
        /* Reads vbr_if info from DB */
        result_code = mgr->ReadConfigDB(vbrif_ckv, req->datatype, UNC_OP_READ,
                                        dbop1, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadConfigDB error = %d", result_code);
          delete vbrif_ckv;
          delete ck_vlink;
          DELETE_IF_NOT_NULL(ck_boundary);
          return result_code;
        }

        /* During no vlink resets vnode interface flag */
        GET_USER_DATA_FLAGS(vbrif_ckv, if_flag);
        if_flag &= ~VIF_TYPE;
        SET_USER_DATA_FLAGS(vbrif_ckv, if_flag);
        vnode_if_type vnif_type;
        UPLL_LOG_TRACE("Vlanmap delete vbr_if flag = %u", if_flag);

        result_code = UpdateVlinkMemIfFlag(req->datatype, vbrif_ckv, dmi,
                                           vnif_type, mgr, UNC_OP_DELETE,
                                           config_mode, vtn_name);
        DELETE_IF_NOT_NULL(vbrif_ckv);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("UpdateVlinkVlanmapFlag error = %d", result_code);
          delete ck_vlink;
          DELETE_IF_NOT_NULL(ck_boundary);
          return result_code;
        }

        SET_USER_DATA_FLAGS(ck_vlink->get_cfg_val(), kVlinkVnode2);
        continue;
      }
      break;
      default:
        UPLL_LOG_TRACE("No Portmap");
    }
    if (executed) {
      DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
      result_code = mgr->UpdateConfigDB(ck_vnif, req->datatype, UNC_OP_UPDATE,
                                        dmi, &dbop, config_mode, vtn_name,
                                        MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning %d", result_code);
        delete ck_vnif;
        delete ck_vlink;
        DELETE_IF_NOT_NULL(ck_boundary);
        return result_code;
      }
    }
    SET_USER_DATA_FLAGS(ck_vlink->get_cfg_val(), kVlinkVnode2);
    delete ck_vnif;
  }
  DELETE_IF_NOT_NULL(ck_boundary);
  delete ck_vlink;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::SetConsolidatedStatus(ConfigKeyVal *vlink,
                                      unc_keytype_operation_t op,
                            unc_keytype_configstatus_t cs_status,
                                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(vlink));
  bool bound_vlink = false;
  int node_vunk_if = 0;
  if (!vlink_val) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vnif[2] = {NULL, NULL};
  unc_keytype_configstatus_t if_cstatus[2],
                    vlanid_cstatus[2] = {UNC_CS_UNKNOWN, UNC_CS_UNKNOWN};
  VlinkNodePosition vnode_number = kVlinkVnode1;
  uint8_t rename_flag = 0;
  GET_USER_DATA_FLAGS(vlink->get_cfg_val(), rename_flag);
  for (int i = 0; i < 2; i++) {
    unc_key_type_t ktype = GetVlinkVnodeIfKeyType(vlink, i);
    VnodeChildMoMgr *mgr = reinterpret_cast<VnodeChildMoMgr *>
                                       (const_cast<MoManager*>
                                        (GetMoManager(ktype)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr");
      return UPLL_RC_ERR_GENERIC;
    }
    SET_USER_DATA_FLAGS(vlink->get_cfg_val(), vnode_number);
    result_code = mgr->GetChildConfigKey(vnif[i], vlink);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed result code %d", result_code);
      return result_code;
    }

    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCs };
    result_code = mgr->ReadConfigDB(vnif[i], UPLL_DT_STATE, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      DELETE_IF_NOT_NULL(vnif[i]);
      return result_code;
    }
    vnode_number = kVlinkVnode2;
  }
  SET_USER_DATA_FLAGS(vlink->get_cfg_val(), rename_flag);
  if (!GetVal(vnif[0]) ||!GetVal(vnif[1])) {
    DELETE_IF_NOT_NULL(vnif[0]);
    DELETE_IF_NOT_NULL(vnif[1]);
    UPLL_LOG_DEBUG("Returning error");
    return UPLL_RC_ERR_GENERIC;
  }
  if ((vnif[0]->get_key_type() == UNC_KT_VUNK_IF) ||
      (vnif[1]->get_key_type() == UNC_KT_VUNK_IF)) {
    bound_vlink = true;
    if (vnif[0]->get_key_type() == UNC_KT_VUNK_IF)
      node_vunk_if = 1;
    if (vnif[1]->get_key_type() == UNC_KT_VUNK_IF)
      node_vunk_if = 2;
  } else {
    uint8_t *ctrlr1 = NULL, *ctrlr2 = NULL, *dom1 = NULL, *dom2 = NULL;
    GET_USER_DATA_CTRLR(vlink, ctrlr1);
    GET_USER_DATA_DOMAIN(vlink, dom1);
    GET_USER_DATA_CTRLR(vlink->get_cfg_val(), ctrlr2);
    GET_USER_DATA_DOMAIN(vlink->get_cfg_val(), dom2);
    if (!ctrlr1 || !ctrlr2 || !dom1 || !dom2) {
      UPLL_LOG_DEBUG("Invalid param");
      DELETE_IF_NOT_NULL(vnif[0]);
      DELETE_IF_NOT_NULL(vnif[1]);
      return UPLL_RC_ERR_GENERIC;
    }
    if (strncmp(reinterpret_cast<char *>(ctrlr1),
                reinterpret_cast<char *>(ctrlr2), kMaxLenCtrlrId+1) ||
        strncmp(reinterpret_cast<char *>(dom1),
                reinterpret_cast<char *>(dom2), kMaxLenDomainId+1))
      bound_vlink = true;
  }
  if ((op == UNC_OP_CREATE) || (op == UNC_OP_UPDATE)) {
    if (bound_vlink) {
      unc_key_type_t if_ktype = vnif[0]->get_key_type();
      unc_keytype_configstatus_t c_status = UNC_CS_UNKNOWN;
      val_port_map *pm, *pm1, *pm2;
      pm = pm1 = pm2 = NULL;

      val_vlan_map *vlanmap = NULL, *vlanmap1 = NULL, *vlanmap2 = NULL;
      //  vbr_portmap case
      val_vbr_portmap_t *vbr_pm = NULL, *vbr_pm1 = NULL, *vbr_pm2 = NULL;
      for (int if_type = 0; if_type < 2; if_type++) {
        switch (if_ktype) {
        case UNC_KT_VBR_IF: {
          val_vbr_if *vbr_ifval = &(reinterpret_cast<val_drv_vbr_if *>
                             (GetVal(vnif[if_type]))->vbr_if_val);
          pm = &(vbr_ifval->portmap);
          c_status = (unc_keytype_configstatus_t)
                     (vbr_ifval->cs_row_status); }
        break;
        /* VlanmapOnBoundary: Added vlanmap case */
        case UNC_KT_VBR_VLANMAP: {
          vlanmap = &(reinterpret_cast<pfcdrv_val_vlan_map *>
                             (GetVal(vnif[if_type]))->vm);
          c_status = (unc_keytype_configstatus_t)
                     (reinterpret_cast<pfcdrv_val_vlan_map*>
                     (GetVal(vnif[if_type]))->vm.cs_row_status); }
        break;
        case UNC_KT_VTEP_IF: {
          pm = &(reinterpret_cast<val_vtep_if *>
                             (GetVal(vnif[if_type]))->portmap);
          c_status = (unc_keytype_configstatus_t)
                     (reinterpret_cast<val_vtep_if *>
                     (GetVal(vnif[if_type]))->cs_row_status); }
        break;
        case UNC_KT_VTUNNEL_IF: {
          pm = &(reinterpret_cast<val_vtunnel_if *>
                             (GetVal(vnif[if_type]))->portmap);
          c_status = (unc_keytype_configstatus_t)
                     (reinterpret_cast<val_vtunnel_if *>
                     (GetVal(vnif[if_type]))->cs_row_status); }
        break;
        case UNC_KT_VUNK_IF: {
          c_status = (unc_keytype_configstatus_t)
                     (reinterpret_cast<val_vunk_if *>
                     (GetVal(vnif[if_type]))->cs_row_status); }
        break;
        case UNC_KT_VBR_PORTMAP: {
           vbr_pm = &(reinterpret_cast<pfcdrv_val_vbr_portmap *>
                             (GetVal(vnif[if_type]))->vbrpm);
          c_status = (unc_keytype_configstatus_t)
                     (reinterpret_cast<pfcdrv_val_vbr_portmap*>
                     (GetVal(vnif[if_type]))->vbrpm.cs_row_status); }

        break;
        default:
        break;
        }
        if (if_type == 0) {
          /* VlanmapOnBoundary: changes */
          if (if_ktype == UNC_KT_VBR_VLANMAP) {
            vlanmap1 = vlanmap;
          } else if (if_ktype == UNC_KT_VBR_PORTMAP) {
            //  vbr_portmap
            vbr_pm1 = vbr_pm;
          } else {
            pm1 = static_cast<val_port_map *>(pm);
          }
        } else {
          /* VlanmapOnBoundary: changes */
          if (if_ktype == UNC_KT_VBR_VLANMAP) {
            vlanmap2 = vlanmap;
          } else if (if_ktype == UNC_KT_VBR_PORTMAP) {
            vbr_pm2 = vbr_pm;
          } else {
            pm2 = static_cast<val_port_map *>(pm);
          }
        }
        // Assign Configstatus
        if_cstatus[if_type] = c_status;
        if_ktype = vnif[1]->get_key_type();
      }
      /* VlanmapOnBoundary: changes */
      if (vlanmap1 &&
          (vlanmap1->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID ||
           vlanmap1->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID_NO_VALUE)) {
        vlanid_cstatus[0] = (unc_keytype_configstatus_t)
          (vlanmap1->cs_attr[UPLL_IDX_VLAN_ID_VM]);
      }
      /* VlanmapOnBoundary: changes */
      if (vlanmap2 &&
          (vlanmap2->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID ||
           vlanmap2->valid[UPLL_IDX_VLAN_ID_VM] == UNC_VF_VALID_NO_VALUE)) {
        vlanid_cstatus[1] = (unc_keytype_configstatus_t)
          (vlanmap2->cs_attr[UPLL_IDX_VLAN_ID_VM]);
      }
      //  Vbr portmap node1
      if (vbr_pm1 &&
          (vbr_pm1->valid[UPLL_IDX_LABEL_VBRPM] == UNC_VF_VALID ||
           vbr_pm1->valid[UPLL_IDX_LABEL_VBRPM] == UNC_VF_VALID_NO_VALUE)) {
        vlanid_cstatus[0] = (unc_keytype_configstatus_t)
          (vbr_pm1->cs_attr[UPLL_IDX_LABEL_VBRPM]);
      }
      // Vbrportmap node2
      if (vbr_pm2 &&
          (vbr_pm2->valid[UPLL_IDX_LABEL_VBRPM] == UNC_VF_VALID ||
           vbr_pm2->valid[UPLL_IDX_LABEL_VBRPM] == UNC_VF_VALID_NO_VALUE)) {
        vlanid_cstatus[1] = (unc_keytype_configstatus_t)
          (vbr_pm2->cs_attr[UPLL_IDX_LABEL_VBRPM]);
      }
      if (pm1 && (pm1->valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID ||
          pm1->valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID_NO_VALUE))
        vlanid_cstatus[0] = (unc_keytype_configstatus_t)
                       (pm1->cs_attr[UPLL_IDX_VLAN_ID_PM]);
      if (pm2 && (pm2->valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID ||
          pm2->valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID_NO_VALUE))
        vlanid_cstatus[1] = (unc_keytype_configstatus_t)
                        (pm2->cs_attr[UPLL_IDX_VLAN_ID_PM]);
      if ((vlanid_cstatus[0] == UNC_CS_APPLIED) &&
          (vlanid_cstatus[1] == UNC_CS_APPLIED)) {
        vlink_val->cs_attr[UPLL_IDX_LABEL_VLNK] = UNC_CS_APPLIED;
        vlink_val->cs_attr[UPLL_IDX_LABEL_TYPE_VLNK] = UNC_CS_APPLIED;
      } else if ((vlanid_cstatus[0] == UNC_CS_INVALID) ||
               (vlanid_cstatus[1] == UNC_CS_INVALID)) {
        vlink_val->cs_attr[UPLL_IDX_LABEL_VLNK] = UNC_CS_INVALID;
        vlink_val->cs_attr[UPLL_IDX_LABEL_TYPE_VLNK] = UNC_CS_INVALID;
      } else if ((vlanid_cstatus[0] == UNC_CS_NOT_APPLIED) &&
               (vlanid_cstatus[1] == UNC_CS_NOT_APPLIED)) {
        vlink_val->cs_attr[UPLL_IDX_LABEL_VLNK]  = UNC_CS_NOT_APPLIED;
        vlink_val->cs_attr[UPLL_IDX_LABEL_TYPE_VLNK] = UNC_CS_NOT_APPLIED;
      } else {
        if (node_vunk_if == 1) {  //  If first node is KT_VUNKOWN_IF
          vlink_val->cs_attr[UPLL_IDX_LABEL_VLNK] = vlanid_cstatus[1];
          vlink_val->cs_attr[UPLL_IDX_LABEL_TYPE_VLNK] = vlanid_cstatus[1];
        } else if (node_vunk_if == 2) {  //  If second node is KT_VUNKOWN_IF
          vlink_val->cs_attr[UPLL_IDX_LABEL_VLNK] = vlanid_cstatus[0];
          vlink_val->cs_attr[UPLL_IDX_LABEL_TYPE_VLNK] = vlanid_cstatus[0];
        } else {  //  Set Partially Applied for other cases
          vlink_val->cs_attr[UPLL_IDX_LABEL_VLNK] =
                                          UNC_CS_PARTIALLY_APPLIED;
          vlink_val->cs_attr[UPLL_IDX_LABEL_TYPE_VLNK] =
                                          UNC_CS_PARTIALLY_APPLIED;
        }
      }
    /* update consolidated config status if boundary vlink */
      if (op == UNC_OP_CREATE)  {
      vlink_val->cs_attr[UPLL_IDX_VNODE1_NAME_VLNK] = if_cstatus[0];
      vlink_val->cs_attr[UPLL_IDX_VNODE2_NAME_VLNK] = if_cstatus[1];
      vlink_val->cs_attr[UPLL_IDX_VNODE1_IF_NAME_VLNK] = if_cstatus[0];
      vlink_val->cs_attr[UPLL_IDX_VNODE2_IF_NAME_VLNK] = if_cstatus[1];

      if ((if_cstatus[0] == UNC_CS_APPLIED) &&
        (if_cstatus[1] == UNC_CS_APPLIED))
        vlink_val->cs_row_status = UNC_CS_APPLIED;
      else if ((if_cstatus[0] == UNC_CS_INVALID) ||
           (if_cstatus[1] == UNC_CS_INVALID))
        vlink_val->cs_row_status = UNC_CS_INVALID;
      else if ((if_cstatus[0] == UNC_CS_NOT_APPLIED) &&
           (if_cstatus[1] == UNC_CS_NOT_APPLIED))
        vlink_val->cs_row_status = UNC_CS_NOT_APPLIED;
      else
        vlink_val->cs_row_status = UNC_CS_PARTIALLY_APPLIED;
      }
    }
  }
  if (vnif[0]) delete vnif[0];
  if (vnif[1]) delete vnif[1];
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::MergeValidate(unc_key_type_t keytype,
                                    const char *ctrlr_id, ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi,
                                    upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  if (!ikey || !(ikey->get_key() || !ctrlr_id)) {
    UPLL_LOG_DEBUG(" Input is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *dup_key = NULL;
  result_code = GetChildConfigKey(dup_key, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    if (dup_key) delete dup_key;
    UPLL_LOG_DEBUG("GetChildConfigKey Failed");
    return result_code;
  }
  /*
   * Here getting FULL Key (VTN & Vlink Name )
   */
  result_code = ReadConfigDB(dup_key, UPLL_DT_IMPORT, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    if (dup_key) delete dup_key;
    return result_code;
  }
  /* checks the vnode name present in the running vnode under the
   * same vtn
   */
  ConfigKeyVal * travel = dup_key;
  while (travel) {
    dbop.inoutop = kOpInOutCtrlr;
    ConfigKeyVal *ca_temp = NULL;
    result_code = GetChildConfigKey(ca_temp, travel);
    if (UPLL_RC_SUCCESS != result_code) {
       UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
       delete dup_key;
       return result_code;
    }
    dbop.inoutop = kOpInOutCtrlr | kOpInOutDomain;
    result_code = ReadConfigDB(ca_temp, UPLL_DT_CANDIDATE, UNC_OP_READ,
                               dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG(" ReadConfigDB failed %d", result_code);
      if (dup_key) delete dup_key;
      delete ca_temp;
      return result_code;
    }
    /* Same vlink name should not present in the other controllers */
    if (result_code == UPLL_RC_SUCCESS) {
      if (import_type == UPLL_IMPORT_TYPE_PARTIAL) {
        uint8_t *ctrlr1 = NULL;
        uint8_t *ctrlr2 = NULL;
        GET_USER_DATA_CTRLR(ca_temp, ctrlr1);
        GET_USER_DATA_CTRLR(ca_temp->get_cfg_val(), ctrlr2);
        if (ctrlr1 && ctrlr2) {
          if (strcmp(ctrlr_id, (const char *)ctrlr1) ||
              strcmp(ctrlr_id, (const char *)ctrlr2)) {
            ikey->ResetWith(travel);
            delete dup_key;
            delete ca_temp;
            UPLL_LOG_DEBUG("Vlink Name Conflict %d", result_code);
            return UPLL_RC_ERR_MERGE_CONFLICT;
          }
        } else {
          UPLL_LOG_DEBUG("Ctrlr id is null");
          delete dup_key;
          delete ca_temp;
          return UPLL_RC_ERR_GENERIC;
        }
      } else {
        ikey->ResetWith(travel);
        delete dup_key;
        delete ca_temp;
        UPLL_LOG_DEBUG("Vlink Name Conflict %d", result_code);
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
    }
    /* Any other DB error */
    result_code = UPLL_RC_SUCCESS;
    travel = travel->get_next_cfg_key_val();
    DELETE_IF_NOT_NULL(ca_temp);
  }
  if (dup_key) delete dup_key;
  return result_code;
}

upll_rc_t VlinkMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                                           AdaptType adapt_type) {
  UPLL_FUNC_TRACE;
  // To adapt the value structure with VTNService
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
    while (cval) {
      if (IpctSt::kIpcStValVlink == cval->get_st_num()) {
         // set admin status to valid no value
         val_vlink *vlink_val = reinterpret_cast<val_vlink *>GetVal(ikey);
         if (vlink_val->valid[UPLL_IDX_ADMIN_STATUS_VLNK] == UNC_VF_INVALID)
           vlink_val->valid[UPLL_IDX_ADMIN_STATUS_VLNK] = UNC_VF_VALID_NO_VALUE;
      }
      if (IpctSt::kIpcStValVlinkSt == cval->get_st_num()) {
        val_vlink_st *vlink_val_st = reinterpret_cast<val_vlink_st *>
                         (ConfigKeyVal::Malloc(sizeof(val_vlink_st)));
        val_db_vlink_st *db_vlink_val_st = reinterpret_cast<val_db_vlink_st *>
                                     (cval->get_val());
        uuc::CtrlrMgr* ctr_mgr = uuc::CtrlrMgr::GetInstance();
        controller_domain ctrlr_dom[2] = {{NULL, NULL}, {NULL, NULL}};
        GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom[0]);
        GET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
        val_vlink *vlink_val = reinterpret_cast<val_vlink *>GetVal(ikey);
        if (vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) {
          upll_rc_t rs_code = ResetCtrlrDomForUnifiedInterface(
                          vlink_val->boundary_name, ctrlr_dom);
          if (rs_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("Error in ResetCtrlrDomForUnifiedInterface : %d",
                            rs_code);
            return rs_code;
          }
        }
        if (db_vlink_val_st->vlink_val_st.oper_status !=
            UPLL_OPER_STATUS_UNKNOWN) {
          if ((ctrlr_dom[0].ctrlr && ctr_mgr->IsCtrlrInUnknownList(
                       reinterpret_cast<char*>(ctrlr_dom[0].ctrlr))) ||
              (ctrlr_dom[1].ctrlr && ctr_mgr->IsCtrlrInUnknownList(
                            reinterpret_cast<char*>(ctrlr_dom[1].ctrlr)))) {
            db_vlink_val_st->vlink_val_st.oper_status =
                                          UPLL_OPER_STATUS_UNKNOWN;
          }
        }
        if (db_vlink_val_st->vlink_val_st.oper_status ==
                   UPLL_OPER_STATUS_UP) {
          char *ctrlr1 = reinterpret_cast<char*>(ctrlr_dom[0].ctrlr);
          char *ctrlr2 = reinterpret_cast<char*>(ctrlr_dom[1].ctrlr);
          char *dom1 = reinterpret_cast<char*>(ctrlr_dom[0].domain);
          char *dom2 = reinterpret_cast<char*>(ctrlr_dom[1].domain);
          char *vtn_name = reinterpret_cast<char*>(
                    (reinterpret_cast<key_vtn*>(ikey->get_key()))->vtn_name);
          if ((ctrlr1 && ctr_mgr->IsPathFaultOccured(ctrlr1, dom1)) ||
              (ctrlr2 && ctr_mgr->IsPathFaultOccured(ctrlr2, dom2)) ||
              (ctrlr1 && ctr_mgr->HasVtnExhaustionOccured(
                                  vtn_name, ctrlr1, dom1)) ||
              (ctrlr2 && ctr_mgr->HasVtnExhaustionOccured(
                                  vtn_name, ctrlr2, dom2))) {
            db_vlink_val_st->vlink_val_st.oper_status =
                                          UPLL_OPER_STATUS_DOWN;
          }
        }
        memcpy(vlink_val_st, &(db_vlink_val_st->vlink_val_st),
               sizeof(val_vlink_st));
        cval->SetVal(IpctSt::kIpcStValVlinkSt, vlink_val_st);
      } else if (IpctSt::kIpcStValRenameVlink == cval->get_st_num()) {
        void *key = NULL;
        key = ikey->get_key();
        ConfigVal *val = ikey->get_cfg_val();
        val_rename_vlink_t *val_rename_vlink = reinterpret_cast
           <val_rename_vlink_t *> (ConfigKeyVal::Malloc(
           sizeof(val_rename_vlink_t)));  // COV NULL RETURN
         // Getting valid
         val_rename_vlink->valid[UPLL_IDX_NEW_NAME_RVLNK] = UNC_VF_VALID;
      // Getting new_name
         uuu::upll_strncpy(val_rename_vlink->new_name,
             (reinterpret_cast<key_vlink_t*>(key))->vlink_name,
             (kMaxLenVlinkName+1));
         val->SetVal(IpctSt::kIpcStValRenameVlink, val_rename_vlink);
         ikey->SetCfgVal(val);
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


upll_rc_t VlinkMoMgr::UpdateMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutFlag
                          | kOpInOutDomain };
  if (NULL == ikey || NULL == req || !(ikey->get_key())) {
     UPLL_LOG_ERROR("Given Input is Empty");
     return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("UpdateMo for %d", ikey->get_key_type());
  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Validation Message is Failed ");
      return result_code;
  }
  ConfigKeyVal *dup_ckvlink = NULL;
  result_code = GetChildConfigKey(dup_ckvlink, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(dup_ckvlink, req->datatype, UNC_OP_READ, dbop1,
                             dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
      if (dup_ckvlink) delete dup_ckvlink;
      UPLL_LOG_ERROR("Record does Not Exists");
      return result_code;
  }
  // To validate all the flags are INVALID during Update
  val_vlink_t *vlinkval = reinterpret_cast<val_vlink_t *>(
                                             GetVal(ikey));
  bool is_invalid = true;
  for (unsigned int loop = 0;
    loop < sizeof(vlinkval->valid)/sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID != vlinkval->valid[loop]) {
      is_invalid = false;
      break;
    }
  }
  if (is_invalid) {
    UPLL_LOG_INFO("No attributes to be updated");
    DELETE_IF_NOT_NULL(dup_ckvlink);
    return UPLL_RC_SUCCESS;
  }
  if (dup_ckvlink->get_cfg_val()->get_val()) {
    val_vlink_t *vlink_val = reinterpret_cast<val_vlink_t *>(
                                             GetVal(dup_ckvlink));
    val_vlink_t *vlink_val1 = reinterpret_cast<val_vlink_t *>(
                                             GetVal(ikey));

    if (vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID &&
        vlink_val1->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) {
      if (strncmp(reinterpret_cast<const char *>(vlink_val->boundary_name),
                  reinterpret_cast<const char *>(vlink_val1->boundary_name),
                kMaxLenBoundaryName+1)) {
        UPLL_LOG_DEBUG("Vlink boundary name updation not possible");
        DELETE_IF_NOT_NULL(dup_ckvlink);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    }
    // Check DB value of boundary name, whether it is set or not, if the
    // Boundary name is not set then erase VLAN_ID attribute value.
    if (vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_INVALID &&
        vlink_val1->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_INVALID) {
      vlink_val1->label = 0;
      vlink_val1->valid[UPLL_IDX_LABEL_VLNK] = UNC_VF_VALID_NO_VALUE;
      vlink_val1->label_type = 0;
      vlink_val1->valid[UPLL_IDX_LABEL_TYPE_VLNK] = UNC_VF_VALID_NO_VALUE;
    }
  }

  controller_domain ctrlr_dom[2] = { { NULL, NULL }, { NULL, NULL } };

  result_code = GetControllerDomainId(dup_ckvlink, &ctrlr_dom[0]);
  if (UPLL_RC_SUCCESS != result_code) {
    delete dup_ckvlink;
    return result_code;
  }

  SET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  SET_USER_DATA(ikey->get_cfg_val(), dup_ckvlink->get_cfg_val());
  /* VlanmapOnBoundary: changes */
  SET_USER_DATA(ikey, dup_ckvlink);

  if ((ctrlr_dom[0].ctrlr != NULL) && (!IsUnifiedVbr(ctrlr_dom[0].ctrlr))) {
    result_code = ValidateCapability(
             req, ikey, reinterpret_cast<const char *>
                                   (ctrlr_dom[0].ctrlr));
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
        delete dup_ckvlink;
        return result_code;
      }
  }
  bool bound_vlink = false;
  if (ctrlr_dom[1].ctrlr != NULL && (!IsUnifiedVbr(ctrlr_dom[1].ctrlr))) {
    if (ctrlr_dom[0].ctrlr == NULL ||
        (strcmp(reinterpret_cast<const char *>(ctrlr_dom[0].ctrlr),
               reinterpret_cast<const char *>(ctrlr_dom[1].ctrlr))) ||
        (strcmp(reinterpret_cast<char *>(ctrlr_dom[0].domain),
                   reinterpret_cast<char *>(ctrlr_dom[1].domain)))) {
      bound_vlink = true;
      result_code = ValidateCapability(req, ikey,
                        reinterpret_cast<const char *>(ctrlr_dom[1].ctrlr));
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
        delete dup_ckvlink;
        return result_code;
      }
    }
  } else {
      bound_vlink = true;
  }
  if (bound_vlink) {
    /* boundary vlink */
    val_vlink_t *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ikey));
    if (vlink_val && vlink_val->admin_status == UPLL_ADMIN_DISABLE) {
      UPLL_LOG_ERROR("Boundary vlink cannot be shut\n");
      delete dup_ckvlink;
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  }
  val_vlink_t *tmp_val = reinterpret_cast<val_vlink *>(GetVal(ikey));
  if (!tmp_val) {
    UPLL_LOG_DEBUG("Invalid val");
    delete dup_ckvlink;
    return UPLL_RC_ERR_GENERIC;
  }

  uint8_t valid_boundary = tmp_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
  uint8_t valid_vlanid = tmp_val->valid[UPLL_IDX_LABEL_VLNK];
  /* Below flag is used to reset the valid flag of boundary_name
   * in ikey at the end for only vlan-id update case */
  bool check_boundary_valid = false;

  /* Only vlan-id update from GUI, below code should be executed */
  if (valid_boundary == UNC_VF_INVALID &&
      valid_vlanid != UNC_VF_INVALID) {
    val_vlink_t *val = reinterpret_cast<val_vlink *>(GetVal(dup_ckvlink));
    if (!val) {
      UPLL_LOG_DEBUG("Invalid val");
      delete dup_ckvlink;
      return UPLL_RC_ERR_GENERIC;
    }
    /* Reset valid_boundary to db value */
    valid_boundary = val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
    if (valid_boundary == UNC_VF_VALID) {
      uuu::upll_strncpy(reinterpret_cast<char *>(tmp_val->boundary_name),
          reinterpret_cast<char *>(val->boundary_name),
          kMaxLenBoundaryName);
      tmp_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] =
        val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
      check_boundary_valid = true;
    }
  }

  ConfigKeyVal *ck_boundary = NULL;
  /* Send ipc to UPPL only if boundary_name is VALID */
  result_code = ValidateAttribute(ikey, ck_boundary, dmi, req);
  if (UPLL_RC_SUCCESS  != result_code) {
    delete dup_ckvlink;
    UPLL_LOG_ERROR("Validate Attribute is Failed");
    return result_code;
  }

  /* VlanmapOnBoundary: changes */
  val_vlink_t *vlink_dbval = reinterpret_cast<val_vlink_t *>(
                                             GetVal(dup_ckvlink));
  val_vlink_t *vlink_ikeyval = reinterpret_cast<val_vlink_t *>(
                                             GetVal(ikey));
  /* Case1: New boundary-map request (all combinations)
   * Case2: Boundary-map request after no boundary-map:w
   * Below code should be executed to update the vlink flag */
  if (((vlink_ikeyval->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) &&
       (vlink_dbval->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] ==
        UNC_VF_VALID_NO_VALUE)) ||
       ((vlink_ikeyval->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID) &&
       (vlink_dbval->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_INVALID))) {
    result_code = UpdateVlinkFlagFromBoundary(ikey, ck_boundary);
    if (UPLL_RC_SUCCESS  != result_code) {
      UPLL_LOG_ERROR("Failed to update vlink ikey flag");
      delete dup_ckvlink;
      return result_code;
    }
    uint8_t flags = 0;
    GET_USER_DATA_FLAGS(ikey, flags);
    SET_USER_DATA_FLAGS(dup_ckvlink, flags);
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  /* Except description update request,
   * below code should be executed */
  if (valid_boundary != UNC_VF_INVALID) {
    uint8_t rename_flag = 0;
    VlinkNodePosition vnode_number = kVlinkVnode1;
    ConfigKeyVal *ck_vnif[2] = {NULL, NULL};
    GET_USER_DATA_FLAGS(dup_ckvlink->get_cfg_val(), rename_flag);
    for (int i = 0; i < 2; i++) {
      unc_key_type_t ktype = GetVlinkVnodeIfKeyType(dup_ckvlink, i);
      if (ktype == UNC_KT_VRT_IF) {
        UPLL_LOG_DEBUG("Boundary operation on non-boundary interface");
        if (ck_vnif[0]) delete ck_vnif[0];
        delete dup_ckvlink;
        DELETE_IF_NOT_NULL(ck_boundary);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
      MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                 (GetMoManager(ktype)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid Mgr %d", ktype);
        delete dup_ckvlink;
        DELETE_IF_NOT_NULL(ck_vnif[0]);
        DELETE_IF_NOT_NULL(ck_vnif[1]);
        DELETE_IF_NOT_NULL(ck_boundary);
        return UPLL_RC_ERR_GENERIC;
      }
      SET_USER_DATA_FLAGS(dup_ckvlink->get_cfg_val(), vnode_number);
      result_code = mgr->GetChildConfigKey(ck_vnif[i], dup_ckvlink);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning %d", result_code);
        delete dup_ckvlink;
        DELETE_IF_NOT_NULL(ck_vnif[0]);
        DELETE_IF_NOT_NULL(ck_vnif[1]);
        DELETE_IF_NOT_NULL(ck_boundary);
        return result_code;
      }
      /* VlanmapOnBoundary: Boundary vlink update operation for SW or SD */
      if ((ktype == UNC_KT_VBR_VLANMAP) || (ktype == UNC_KT_VBR_PORTMAP)) {
        UPLL_LOG_DEBUG("Vlink update operation SW or SD boundary");

        /* validateBoundary should be called only once */
        if (!ck_boundary) {
          if ((vlink_ikeyval->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] ==
              UNC_VF_VALID_NO_VALUE) &&
             (vlink_dbval->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] ==
              UNC_VF_VALID)) {
            /* no boundary of logical port id SW or SD. case first gets
             * the boundary info from uppl and updates the vlink ikey flag */
            result_code = ValidateBoundary(vlink_dbval->boundary_name, req,
                                           ck_boundary);
            if (result_code != UPLL_RC_SUCCESS) {
              delete dup_ckvlink;
              DELETE_IF_NOT_NULL(ck_vnif[0]);
              DELETE_IF_NOT_NULL(ck_vnif[1]);
              DELETE_IF_NOT_NULL(ck_boundary);
              return UPLL_RC_ERR_CFG_SEMANTIC;
            }
            result_code = UpdateVlinkFlagFromBoundary(ikey,
                                                ck_boundary);
            if (result_code != UPLL_RC_SUCCESS) {
              delete dup_ckvlink;
              DELETE_IF_NOT_NULL(ck_vnif[0]);
              DELETE_IF_NOT_NULL(ck_vnif[1]);
              DELETE_IF_NOT_NULL(ck_boundary);
              return UPLL_RC_ERR_CFG_SEMANTIC;
            }
          }
        }

        SET_USER_DATA_FLAGS(ck_vnif[i], 0);
        /* Handles boundary vlanmap request */
        result_code = mgr->BoundaryMapReq(req, ikey, dup_ckvlink,
                                              ck_vnif[i], ck_boundary, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Failed to update vlink for SD or SW boundary");
          /* ck_boundary should be deleted */
          DELETE_IF_NOT_NULL(ck_boundary);
          DELETE_IF_NOT_NULL(ck_vnif[0]);
          DELETE_IF_NOT_NULL(ck_vnif[1]);
          delete dup_ckvlink;
          return result_code;
        }
        vnode_number = kVlinkVnode2;
        continue;
      }

      result_code = mgr->ReadConfigDB(ck_vnif[i], req->datatype,
                                              UNC_OP_READ, dbop1, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in reading %d", result_code);
        delete dup_ckvlink;
        if (ck_vnif[0]) delete ck_vnif[0];
        if (ck_vnif[1]) delete ck_vnif[1];
        ck_vnif[0] = ck_vnif[1] = NULL;
        DELETE_IF_NOT_NULL(ck_boundary);
        return result_code;
      }
      vnode_number = kVlinkVnode2;
    }
    SET_USER_DATA_FLAGS(dup_ckvlink->get_cfg_val(), rename_flag);
    if (!ck_vnif[0] || !ck_vnif[1]) {
       UPLL_LOG_DEBUG("Invalid param");
       delete dup_ckvlink;
       DELETE_IF_NOT_NULL(ck_vnif[0]);
       DELETE_IF_NOT_NULL(ck_vnif[1]);
       DELETE_IF_NOT_NULL(ck_boundary);
       return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    UPLL_LOG_DEBUG("Before UpdateVnodeIf %s", (ikey->ToStrAll()).c_str());
    result_code = UpdateVnodeIf(req->datatype, ikey, ck_vnif, dmi,
                                req->operation, ck_boundary,
                                config_mode, vtn_name);
    for (int i = 0; i < 2 ; i++)
      if (ck_vnif[i]) delete ck_vnif[i];
    if (result_code != UPLL_RC_SUCCESS) {
      delete dup_ckvlink;
      DELETE_IF_NOT_NULL(ck_boundary);
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      return result_code;
    }
  }
  DELETE_IF_NOT_NULL(ck_boundary);
  UPLL_LOG_DEBUG("The ikey Structue before update  %s",
                (ikey->ToStrAll()).c_str());

  /* VlanmapOnBoundary: Added vlanmap check */
  if ((GetVlinkVnodeIfKeyType(dup_ckvlink, 0) == UNC_KT_VBR_VLANMAP) ||
     (GetVlinkVnodeIfKeyType(dup_ckvlink, 1) == UNC_KT_VBR_VLANMAP) ||
     (GetVlinkVnodeIfKeyType(dup_ckvlink, 0) == UNC_KT_VBR_PORTMAP) ||
     (GetVlinkVnodeIfKeyType(dup_ckvlink, 1) == UNC_KT_VBR_PORTMAP)) {
    UPLL_LOG_TRACE("ikey flags update with boundary vlanmap vlink flag");
    DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
    result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE,
                               dmi, &dbop, config_mode, vtn_name,
                               MAINTBL);
  } else {
    DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
    result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE,
                                 dmi, &dbop, config_mode, vtn_name,
                                 MAINTBL);
  }

  if (UPLL_RC_SUCCESS != result_code) {
    delete dup_ckvlink;
    UPLL_LOG_ERROR("Updation Failure in DB : %d", result_code);
    return result_code;
  }
  /* Resetting valid[BOUNDARY_NAME] in the request(description
   * update case) */
  if (check_boundary_valid) {
    tmp_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] = UNC_VF_INVALID;
  }
  if (dup_ckvlink) delete dup_ckvlink;
  return result_code;
}

upll_rc_t VlinkMoMgr::GetVnodeIfFromVlink(ConfigKeyVal *vlink,
                                          ConfigKeyVal **vnif,
                                          DalDmlIntf   *dmi, uint8_t pos) {
  UPLL_FUNC_TRACE;
  if (!vlink) {
    UPLL_LOG_TRACE("Input vlink key null");
    return UPLL_RC_ERR_GENERIC;
  }
  int num = (pos > 1)?2:1;
  VlinkNodePosition vnode_number =
                (pos == 1) ?kVlinkVnode2:kVlinkVnode1;
  uint8_t rename_flag = 0;
  GET_USER_DATA_FLAGS(vlink->get_cfg_val(), rename_flag);
  for (int i = 0; i < num ; i++) {
    int j = (pos > 1)?i:pos;
    unc_key_type_t ktype = GetVlinkVnodeIfKeyType(vlink, j);

    /* VlanmapOnBoundary: In boundary vlan-map case:
     * Get the vBrIf of vlan-mapped interface */
    if ((ktype == UNC_KT_VBR_VLANMAP) || (ktype == UNC_KT_VBR_PORTMAP))
      ktype = UNC_KT_VBR_IF;

    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                      (GetMoManager(ktype)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr for keytype %d", ktype);
      return UPLL_RC_ERR_GENERIC;
    }
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    SET_USER_DATA_FLAGS(vlink->get_cfg_val(), vnode_number);
    result_code = mgr->GetChildConfigKey(vnif[i], vlink);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed result_code %d", result_code);
      return result_code;
    }

    DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
      kOpInOutCs | kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
    result_code = mgr->ReadConfigDB(vnif[i], UPLL_DT_STATE,
                              UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(vnif[i]);
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      return result_code;
    }
    if (!GetVal(vnif[i])) {
      UPLL_LOG_DEBUG("Returning error");
      DELETE_IF_NOT_NULL(vnif[i]);
      return UPLL_RC_ERR_GENERIC;
    }
    vnode_number = (vnode_number == kVlinkVnode1)?kVlinkVnode2:kVlinkVnode1;
  }
  SET_USER_DATA_FLAGS(vlink->get_cfg_val(), rename_flag);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::GetRemoteIf(ConfigKeyVal *ck_vnif,
                      ConfigKeyVal *&ck_remif,
                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_vlink = NULL;
  upll_rc_t result_code = GetRemoteIf(ck_vnif, ck_remif, ck_vlink, dmi);
  DELETE_IF_NOT_NULL(ck_vlink);
  return result_code;
}

upll_rc_t VlinkMoMgr::GetRemoteIf(ConfigKeyVal *ck_vnif,
                      ConfigKeyVal *&ck_remif,
                      ConfigKeyVal *&ck_vlink,
                      DalDmlIntf *dmi,
                      upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t vif_flag = 0;
  uint8_t rem_if;

  if (!ck_vnif) {
    UPLL_LOG_DEBUG("Returning error\n");
    return UPLL_RC_ERR_GENERIC;
  }
  GET_USER_DATA_FLAGS(ck_vnif, vif_flag);
  if (vif_flag & 0xA0)
    rem_if = 1;
  else if (vif_flag & 0x50)
    rem_if = 0;
  else
    return UPLL_RC_ERR_GENERIC;
  result_code = GetVlinkKeyVal(ck_vnif, datatype, ck_vlink, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Returning error %d\n", result_code);
    DELETE_IF_NOT_NULL(ck_vlink);
    return result_code;
  }
  result_code = GetVnodeIfFromVlink(ck_vlink, &ck_remif, dmi, rem_if);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("get remote interface failed %d", result_code);
    DELETE_IF_NOT_NULL(ck_vlink);
    return result_code;
  }
  return result_code;
}

/* VlanmapOnBoundary changes */
upll_rc_t VlinkMoMgr::UpdateVlinkFlagFromBoundary(ConfigKeyVal *&ikey,
                                             ConfigKeyVal *ck_boundary) {
  UPLL_FUNC_TRACE;

  uint8_t node1_flag   = 0;
  uint8_t node2_flag   = 0;
  uint8_t tmp_flag     = 0;

  val_vlink_t *ival = reinterpret_cast<val_vlink_t *>(GetVal(ikey));
  if (!ival) {
    UPLL_LOG_DEBUG("ival:%p is NULL", ival);
    return UPLL_RC_ERR_GENERIC;
  }

  /* In case of vLink create req without boundary name */
  if (ival->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_INVALID) {
    UPLL_LOG_DEBUG("Boundary name is not present");
    return UPLL_RC_SUCCESS;
  }

  /* If bdry info from uppl is present */
  if (!ck_boundary || !ck_boundary->get_cfg_val()) {
    UPLL_LOG_DEBUG("ck_boundary is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  controller_domain_t vlink_ctrlr_dom[2];
  vlink_ctrlr_dom[0].ctrlr = NULL;
  vlink_ctrlr_dom[0].domain = NULL;
  vlink_ctrlr_dom[1].ctrlr = NULL;
  vlink_ctrlr_dom[1].domain = NULL;
  val_boundary_t  *boundary_val =
      reinterpret_cast<val_boundary_t *>(GetVal(ck_boundary));
  //  Get vLink controller domain
  upll_rc_t result_code = GetControllerDomainId(ikey, vlink_ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to get vlink controller and domain");
    return UPLL_RC_SUCCESS;
  }

  for (int i = 0; i < 2; i++) {
    uint8_t *bdry_ctr_name = NULL;
    uint8_t *bdry_domain   = NULL;
    uint8_t *logical_port_id = NULL;

    if (i == 0) {
      logical_port_id = boundary_val->logical_port_id1;
      bdry_ctr_name = boundary_val->controller_name1;
      bdry_domain   = boundary_val->domain_name1;
    } else {
      logical_port_id = boundary_val->logical_port_id2;
      bdry_ctr_name = boundary_val->controller_name2;
      bdry_domain   = boundary_val->domain_name2;
    }

    for (int index = 0; index <2; index++) {
      if (!vlink_ctrlr_dom[index].ctrlr) {
        UPLL_LOG_TRACE(
            "Controller name is empty. controller type is unknown");
        continue;
      }

      uint8_t boundarymap_flag = 0;
      if (!strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[index].ctrlr),
                  "#") &&
          (!strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[index].domain),
                   "#"))) {
        // unified vBridge
        boundarymap_flag = kVbrPortMap;
      } else if ((!strcmp(
                  reinterpret_cast<char *>(vlink_ctrlr_dom[index].ctrlr),
                  reinterpret_cast<char *>(bdry_ctr_name))) &&
                 (!strcmp(
                   reinterpret_cast<char *>(vlink_ctrlr_dom[index].domain),
                   reinterpret_cast<char *>(bdry_domain)))) {
        if ((toupper(logical_port_id[0]) == 'S' &&
             toupper(logical_port_id[1]) == 'W') ||
            (toupper(logical_port_id[0]) == 'S' &&
             toupper(logical_port_id[1]) == 'D')) {
          boundarymap_flag = kVlanMap;
        }
      } else {
        //  vlink controller domain is not matched with boundary controller
        //  and domain
        continue;
      }

      if (boundarymap_flag) {
        if (index == 0) {
          node1_flag = (uint8_t)
              (VLINK_FLAG_NODE1_TYPE&(boundarymap_flag << kVlinkVnodeIf1Type));
          //  For VALID_NO_VALUE case vLink flag is reset to vBrIf
          if (ival->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] ==
              UNC_VF_VALID_NO_VALUE) {
            node1_flag = (uint8_t)
                (VLINK_FLAG_NODE1_TYPE&(kVbrIf << kVlinkVnodeIf1Type));
          }
          /*Set ikey node1 flag */
          GET_USER_DATA_FLAGS(ikey, tmp_flag);
          tmp_flag &= ~VLINK_FLAG_NODE1_TYPE;
          tmp_flag |= node1_flag;
          SET_USER_DATA_FLAGS(ikey, tmp_flag);
        } else {
          node2_flag = (uint8_t)
              (VLINK_FLAG_NODE2_TYPE&(boundarymap_flag << kVlinkVnodeIf2Type));
          //  For VALID_NO_VALUE case vLink flag is reset to vBrIf
          if (ival->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] ==
              UNC_VF_VALID_NO_VALUE) {
            node2_flag = (uint8_t)
                (VLINK_FLAG_NODE2_TYPE&(kVbrIf << kVlinkVnodeIf2Type));
          }
          /*Set ikey node1 flag */
          GET_USER_DATA_FLAGS(ikey, tmp_flag);
          tmp_flag &= ~VLINK_FLAG_NODE2_TYPE;
          tmp_flag |= node2_flag;
          SET_USER_DATA_FLAGS(ikey, tmp_flag);
        }
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::GetVlinkKeyValFromVlanMap(
    ConfigKeyVal *vlanmap_ckv,
    ConfigKeyVal *&vlink_ckv,
    DalDmlIntf *dmi,
    upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint32_t  session_id  = USESS_ID_UPLL;
  uint32_t  config_id   = 0;

  key_vlan_map *vlanmap_key = (vlanmap_ckv->get_key()) ?
      (reinterpret_cast<key_vlan_map*>(vlanmap_ckv->get_key())):NULL;
  if (!vlanmap_key) {
    UPLL_LOG_DEBUG("Error in retrieving key vlan map");
    return UPLL_RC_ERR_GENERIC;
  }

  uint8_t *logical_port_id = NULL;
  if (vlanmap_key->logical_port_id_valid == 1) {
    logical_port_id = vlanmap_key->logical_port_id;
    if (logical_port_id == NULL) {
      UPLL_LOG_DEBUG("Recieved empty logical port id");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    UPLL_LOG_ERROR("Logical port id is not present");
    return UPLL_RC_ERR_GENERIC;
  }
  IpcResponse ipc_resp;
  memset(&ipc_resp, 0, sizeof(IpcResponse));

  /* ConfigKeyVal for Boundary
   * */

  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  GET_USER_DATA_CTRLR_DOMAIN(vlanmap_ckv, ctrlr_dom);

  key_boundary *bndrykey = static_cast<key_boundary *>
         (ConfigKeyVal::Malloc(sizeof(key_boundary)));  // COV NULL RETURN
  val_boundary_t *boundary_val = reinterpret_cast<val_boundary *>(
      ConfigKeyVal::Malloc(sizeof(val_boundary)));
  ConfigVal *cv_boundary = new ConfigVal(IpctSt::kIpcStValBoundary,
                                         boundary_val);
  uuu::upll_strncpy(boundary_val->logical_port_id2, logical_port_id,
                    (kMaxLenLogicalPortId + 1));
  boundary_val->valid[kIdxBoundaryLogicalPortId2] = UNC_VF_VALID;
  uuu::upll_strncpy(boundary_val->controller_name2, ctrlr_dom.ctrlr,
                    kMaxLenCtrlrId + 1);
  boundary_val->valid[kIdxBoundaryControllerName2] = UNC_VF_VALID;
  ConfigKeyVal *ck_boundary = new ConfigKeyVal(UNC_KT_BOUNDARY,
                                               IpctSt::kIpcStKeyBoundary,
                                               bndrykey, cv_boundary);
  SET_USER_DATA_CTRLR_DOMAIN(ck_boundary, ctrlr_dom);

  /* Case1: Audit - Always check boundary in RUNNING
   * Case2: Commit(del) -  check CANDIDATE
   * Case3: Commit(Cr/upd) - check RUNNING */
  upll_keytype_datatype_t dt_type = (UPLL_DT_AUDIT == datatype)?
                                    UPLL_DT_RUNNING:datatype;

  result_code = SendIpcReq(session_id, config_id, UNC_OP_READ_SIBLING,
                           dt_type, ck_boundary, NULL, &ipc_resp);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    boundary_val = reinterpret_cast<val_boundary_t *>(GetVal(ck_boundary));
    memset(boundary_val, 0, sizeof(val_boundary_t));
    uuu::upll_strncpy(boundary_val->logical_port_id1, logical_port_id,
        (kMaxLenLogicalPortId + 1));
    boundary_val->valid[kIdxBoundaryLogicalPortId1] = UNC_VF_VALID;
    uuu::upll_strncpy(boundary_val->controller_name1, ctrlr_dom.ctrlr,
        kMaxLenCtrlrId + 1);
    boundary_val->valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    result_code = SendIpcReq(session_id, config_id, UNC_OP_READ_SIBLING,
                           dt_type, ck_boundary, NULL, &ipc_resp);
  }

  if ((result_code != UPLL_RC_SUCCESS) || (!ipc_resp.ckv_data)) {
    UPLL_LOG_DEBUG("Error in retrieving boundary info from UPPL");
    DELETE_IF_NOT_NULL(ck_boundary);
    return result_code;
  }
  ck_boundary->ResetWith(ipc_resp.ckv_data);
  delete ipc_resp.ckv_data;

  ConfigKeyVal *ck_boundary_tmp = ck_boundary;
  while (ck_boundary_tmp) {
    uint8_t *boundary_id = ck_boundary_tmp->get_key()?
      (reinterpret_cast<key_boundary *>
       (ck_boundary_tmp->get_key()))->boundary_id : NULL;
    if (boundary_id == NULL) {
      UPLL_LOG_DEBUG("Boundary key is empty");
      DELETE_IF_NOT_NULL(ck_boundary);
      return UPLL_RC_ERR_GENERIC;
    }

    ConfigKeyVal *ckv_vlink = NULL;
    result_code = GetChildConfigKey(ckv_vlink, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed for vlink");
      DELETE_IF_NOT_NULL(ck_boundary);
      return result_code;
    }
    ConfigVal *cv_vlink = NULL;
    result_code = AllocVal(cv_vlink, datatype, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("AllocVal failed for vlink");
      DELETE_IF_NOT_NULL(ck_boundary);
      return result_code;
    }
    ckv_vlink->AppendCfgVal(cv_vlink);
    val_vlink *vlink_val = reinterpret_cast<val_vlink*>GetVal(ckv_vlink);
    if (vlink_val == NULL) {
      UPLL_LOG_DEBUG("val_vlink is NULL");
      DELETE_IF_NOT_NULL(ckv_vlink);
      DELETE_IF_NOT_NULL(ck_boundary);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vlink_val->boundary_name, boundary_id,
                      kMaxLenBoundaryName+1);
    vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] = UNC_VF_VALID;

    DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag};
    result_code = ReadConfigDB(ckv_vlink, datatype,
                               UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      ck_boundary_tmp = ck_boundary_tmp->get_next_cfg_key_val();
      DELETE_IF_NOT_NULL(ckv_vlink);
      continue;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Failed to read vlink info based on boundary id");
      DELETE_IF_NOT_NULL(ckv_vlink);
      DELETE_IF_NOT_NULL(ck_boundary);
      return result_code;
    }

    if (vlink_ckv == NULL) {
      vlink_ckv = ckv_vlink;
    } else {
      vlink_ckv->AppendCfgKeyVal(ckv_vlink);
    }

    UPLL_LOG_TRACE("%s", vlink_ckv->ToStrAll().c_str());
    ck_boundary_tmp = ck_boundary_tmp->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ck_boundary);

  /* If vlink_ckv is NULL, do not send SUCCESS */
  if (!vlink_ckv) {
    UPLL_LOG_INFO("No vlink found in datatype:%d", datatype);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::AdaptValToDriver(ConfigKeyVal *ck_new,
    ConfigKeyVal *ck_old,
    unc_keytype_operation_t op,
    upll_keytype_datatype_t dt_type,
    unc_key_type_t keytype,
    DalDmlIntf *dmi,
    bool &not_send_to_drv,
    bool audit_update_phase) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain_t vlink_ctrlr_dom[2];
  vlink_ctrlr_dom[0].ctrlr = NULL;
  vlink_ctrlr_dom[0].domain = NULL;
  vlink_ctrlr_dom[1].ctrlr = NULL;
  vlink_ctrlr_dom[1].domain = NULL;
  // check the given vlink is BoundaryVlink or not
  result_code = BoundaryVlink(ck_new, vlink_ctrlr_dom, not_send_to_drv);
  return result_code;
}

/*
 * This function delete the local vlink configuration
 * for the imported controller
 * while deleting checks the local boundary or not.
 * if local boundary skip the delete otherwise delete
 */
upll_rc_t
VlinkMoMgr::PurgeCandidate(unc_key_type_t key_type,
                           const char  *ctrlr_id,
                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (!ctrlr_id || !dmi) {
    UPLL_LOG_DEBUG("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *temp_ckv = NULL;
  ConfigKeyVal *ca_temp_ckv = NULL;
  DalCursor *dal_cursor_handle;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_id = "";

  for (int tbl = MAINTBL; tbl < ntable; tbl++) {
    if (tbl == CONVERTTBL) {
      continue;
    }
    temp_ckv = NULL;
    ca_temp_ckv = NULL;
    dal_cursor_handle = NULL;

    uudst::kDalTableIndex tbl_index;
    tbl_index = GetTable((MoMgrTables)tbl, UPLL_DT_IMPORT);
    if (tbl_index >= uudst::kDalNumTables)
      continue;

    result_code = DiffConfigDB(UPLL_DT_IMPORT, UPLL_DT_CANDIDATE,
                               UNC_OP_DELETE, temp_ckv, ca_temp_ckv,
                               &dal_cursor_handle, dmi, TC_CONFIG_GLOBAL,
                               vtn_id, (MoMgrTables)(tbl));
    while (result_code == UPLL_RC_SUCCESS) {
      // Iterate loop to get next record
      result_code =DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" GetNextRecord failed err code(%d)",
                       result_code);
        break;
      }
      uint8_t *ca_ctrlr_id = NULL;
      GET_USER_DATA_CTRLR(temp_ckv, ca_ctrlr_id);
      if (ca_ctrlr_id) {
        if (!strcmp((const char*)ca_ctrlr_id, (const char*)ctrlr_id)) {
          // Delete the Partial import controller configuration from candidate
          // which is not present in controller configuration.
          bool boundary = false;
          if (MAINTBL == tbl) {
            boundary = true;
            for (int i = 0; i < 2; i++) {
              unc_key_type_t key_type1 = GetVlinkVnodeIfKeyType(temp_ckv, i);
              /*
               * Taking vlink within controller so other
               * overlay keytype check is
               * not required
               */
              if (UNC_KT_VBR_VLANMAP != key_type1 &&
                  UNC_KT_VBR_IF != key_type1 &&
                  UNC_KT_VBR_PORTMAP != key_type1) {
                boundary = false;
                break;
              }
            }
          }
          if (!boundary) {
            UPLL_LOG_DEBUG("The Deleting configuraiton is %s",
                           temp_ckv->ToStr().c_str());
            result_code = UpdateConfigDB(temp_ckv, UPLL_DT_CANDIDATE,
                                         UNC_OP_DELETE, dmi,
                                         TC_CONFIG_GLOBAL,
                                         vtn_id, (MoMgrTables)(tbl));
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("UpdateConfigDB failed  %d", result_code);
              delete temp_ckv;
              delete ca_temp_ckv;
              dmi->CloseCursor(dal_cursor_handle, true);
              return result_code;
            }
          }
        } else {
          // In candidate other controller configuration leave it
        }
      } else {
        // controller id is null  so not todo purge operation
        return UPLL_RC_ERR_GENERIC;
      }
    }

    DELETE_IF_NOT_NULL(temp_ckv);
    delete ca_temp_ckv;
    if (dal_cursor_handle) {
      dmi->CloseCursor(dal_cursor_handle, true);
    }
  }
  result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?
      UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t VlinkMoMgr::CreateImportMo(IpcReqRespHeader *req,
                                    ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi,
                                    const char *ctrlr_id,
                                    const char *domain_id,
                                    upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || req == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  if (!ctrlr_id) {
    UPLL_LOG_DEBUG("Empty controller name");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *temp_ckv = NULL;
  UPLL_LOG_TRACE("TODO Ctrlr Input %s", ikey->ToStr().c_str());
  if (req->datatype == UPLL_DT_IMPORT) {
    result_code = ValidateMessage(req, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("Validate Messgae Failed");
      return result_code;
    }
  }
  SET_USER_DATA_FLAGS(ikey, NO_RENAME);
  bool is_rename = false;
  controller_domain ctrlr_dom[2] = { { NULL, NULL }, { NULL, NULL } };
  if (UPLL_IMPORT_TYPE_PARTIAL == import_type) {
    SET_USER_DATA_CTRLR(ikey, ctrlr_id);
    result_code = DupConfigKeyVal(temp_ckv, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      return result_code;
    }
    result_code = GetUncKey(ikey, req->datatype, dmi,
                            ctrlr_id);
    if (UPLL_RC_SUCCESS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetUncKey failed %d", result_code);
      delete temp_ckv;
      return result_code;
    }
    if (UPLL_RC_SUCCESS == result_code) {
      is_rename = true;
    }
  }
  ConfigKeyVal *uppl_bdry = NULL;
  result_code = UpdateVlinkIf(req, ikey, uppl_bdry, dmi, ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Error in checking for Vlink Interfaces. result code %d",
                  result_code);
    DELETE_IF_NOT_NULL(temp_ckv);
    return result_code;
  }
  if (UPLL_IMPORT_TYPE_PARTIAL == import_type) {
    if (!is_rename) {
      result_code = AutoRename(ikey, req->datatype, dmi,
                               &is_rename);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("AutoRename failed %d", result_code);
        delete temp_ckv;
        return result_code;
      }
    }
  }
  uint8_t rename = 0x00;
  GET_USER_DATA_FLAGS(ikey, rename);
  UPLL_LOG_TRACE("The Rename flga is %d", rename);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom[0]);
  SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
  UPLL_LOG_TRACE("The result code of GetUncKey %d", result_code);
  result_code = CreateMo(req, ikey, dmi);
  if (UPLL_IMPORT_TYPE_PARTIAL == import_type) {
    /*
     * Convert UNC name into Ctrlr name if
     * renamed
     */
    ikey->ResetWith(temp_ckv);
    delete temp_ckv;
  }
  return result_code;
}

upll_rc_t VlinkMoMgr::SetOperStatus(ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                    state_notification notification ) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("ikey is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("notification %d ikey %s\n", notification,
                    ikey->ToStrAll().c_str());
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_name = "";

  uint8_t orig_status = PORT_UP;
  ConfigVal *tmp =
      (ikey->get_cfg_val()) ? ikey->get_cfg_val()->get_next_cfg_val() : NULL;
  val_db_vlink_st *vlink_db_valst = (tmp != NULL) ?
                    reinterpret_cast<val_db_vlink_st_t *>
                    (tmp->get_val()) : NULL;
  if (vlink_db_valst == NULL) {
    UPLL_LOG_ERROR("Returning error \n");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ikey));
  if (vlink_val == NULL) {
    UPLL_LOG_ERROR("Returning error \n");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vlink_st_t *vlink_valst = reinterpret_cast<val_vlink_st_t *>
                                               (vlink_db_valst);

  /* Update oper status based on notification */
  vlink_valst->valid[UPLL_IDX_OPER_STATUS_VLNKS] = UNC_VF_VALID;
  switch (notification) {
  case kCtrlrDisconnect:
    vlink_valst->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    vlink_db_valst->down_count = PORT_UNKNOWN;
    break;
  case kCommit:
    if (vlink_db_valst->down_count & PORT_UNKNOWN) {
      vlink_valst->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    } else if (vlink_db_valst->down_count == PORT_UP) {
      vlink_valst->oper_status = UPLL_OPER_STATUS_UP;
    } else {
      vlink_valst->oper_status = UPLL_OPER_STATUS_DOWN;
    }
    break;
  case kPortUnknown:
    /* Clear Port fault flag. Set Port Unknown flag.
     * Set OperStatus to UNKNOWN */
    vlink_db_valst->down_count &= ~PORT_FAULT;
    vlink_db_valst->down_count |= PORT_UNKNOWN;
    vlink_valst->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    break;
  case kPortFault:
    orig_status = vlink_db_valst->down_count;
    if (orig_status & PORT_UNKNOWN) {
      vlink_db_valst->down_count &= ~PORT_UNKNOWN;
    }
    /* If OperStatus is UP, then set OperStatus to DOWN */
    if ((orig_status == PORT_UP) || (orig_status == PORT_UNKNOWN)) {
      vlink_valst->oper_status = UPLL_OPER_STATUS_DOWN;
    }
    vlink_db_valst->down_count |= PORT_FAULT;
    break;
  case kPortFaultReset:
    orig_status = vlink_db_valst->down_count;
    if (orig_status & PORT_UNKNOWN) {
      /* if original flag is UNKNOWN, then clear unknown flag */
      vlink_db_valst->down_count &= ~PORT_UNKNOWN;
      /* if flag becomes UP, then set operstatus to UP, else DOWN */
      if (vlink_db_valst->down_count == PORT_UP) {
        vlink_valst->oper_status = UPLL_OPER_STATUS_UP;
      } else {
        vlink_valst->oper_status = UPLL_OPER_STATUS_DOWN;
      }
    } else if (orig_status & ~PORT_FAULT) {
      /* operstatus is already DOWN due some other reason than PORT_FAULT.
       * Clear PORT_FAULT flag and break */
      vlink_db_valst->down_count &= ~PORT_FAULT;
      vlink_valst->oper_status = UPLL_OPER_STATUS_DOWN;
    } else {
      /* If operstatus is DOWN only due to PORT_FAULT. Clear PORT_FAULT flag.
       * Set OperStatus to UP */
      vlink_db_valst->down_count &= ~PORT_FAULT;
      vlink_valst->oper_status = UPLL_OPER_STATUS_UP;
    }
    break;
  case kPathFault:
    orig_status = vlink_db_valst->down_count;
    /* If OperStatus is UP, set OperStatus to DOWN */
    if (orig_status == PORT_UP) {
      vlink_valst->oper_status = UPLL_OPER_STATUS_DOWN;
    }
    /* Set the Down count's Pathfualt flag */
    vlink_db_valst->down_count |= PATH_FAULT;
    break;
  case kPathFaultReset:
    /* Clear the Path fault flag.
     * If OperStatus is UP, then set OperStatus to UP */
    vlink_db_valst->down_count &= ~PATH_FAULT;
    if (vlink_db_valst->down_count == PORT_UP) {
      vlink_valst->oper_status = UPLL_OPER_STATUS_UP;
    }
    break;
  default:
      UPLL_LOG_DEBUG("unsupported notification for operstatus update");
      return UPLL_RC_ERR_GENERIC;
    break;
  }

  vlink_val->valid[UPLL_IDX_ADMIN_STATUS_VLNK] = UNC_VF_INVALID;
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                               &dbop, TC_CONFIG_GLOBAL,
                               vtn_name, MAINTBL);
  UPLL_LOG_DEBUG("Vlink SetOperstatus for VTN after Update is \n %s",
                    ikey->ToStr().c_str());
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Error in update oper status");
    return result_code;
  }
  return result_code;
}

upll_rc_t VlinkMoMgr::SetVlinkAndIfsOnReconnect(
    ConfigKeyVal *ck_vlink,
    ConfigKeyVal **ck_vnif,
    DalDmlIntf *dmi, bool is_unified) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_id = "";
  val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ck_vlink));

  uint32_t new_flag[3] = {0, 0, 0};
  if (vlink_val->admin_status == UPLL_ADMIN_DISABLE) {
    new_flag[2] = ADMIN_DISABLE;
    for (int i = 0; i < 2; i++) {
      new_flag[i] = REMOTE_DOWN;
    }
  }
  bool bound_vlink = false;
  controller_domain_t vlink_ctrlr_dom[2] = {{NULL, NULL}, {NULL, NULL}};
  // check the given vlink is BoundaryVlink or not
  result_code = BoundaryVlink(ck_vlink, vlink_ctrlr_dom, bound_vlink);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return result_code;
  }

  if ((vlink_ctrlr_dom[0].ctrlr && IsUnifiedVbr(vlink_ctrlr_dom[0].ctrlr)) ||
      (vlink_ctrlr_dom[1].ctrlr && IsUnifiedVbr(vlink_ctrlr_dom[1].ctrlr))) {
    result_code = ResetCtrlrDomForUnifiedInterface(vlink_val->boundary_name,
                                vlink_ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Error in ResetCtrlrDomForUnifiedInterface : %d",
                          result_code);
      return result_code;
    }
    if (!is_unified && vlink_ctrlr_dom[0].ctrlr && vlink_ctrlr_dom[1].ctrlr &&
        !strcmp(reinterpret_cast<char*>(vlink_ctrlr_dom[0].ctrlr),
                reinterpret_cast<char*>(vlink_ctrlr_dom[1].ctrlr))) {
      // vbr if operstatus for vbrif b/w vBr and UvBr will be handled
      // as part of conv if computation
      return UPLL_RC_SUCCESS;
    }
  }
  bool is_ctrlr_disc = false;
  bool internal_boundary = false;
  uuc::CtrlrMgr* ctr_mgr = uuc::CtrlrMgr::GetInstance();
  if (ck_vnif[1]->get_key_type() != UNC_KT_VUNK_IF) {
    uint8_t* remote_ctrlr = NULL;
    GET_USER_DATA_CTRLR(ck_vnif[1], remote_ctrlr);
    char *ctrlr1 = reinterpret_cast<char*>(vlink_ctrlr_dom[0].ctrlr);
    char *ctrlr2 = reinterpret_cast<char*>(vlink_ctrlr_dom[1].ctrlr);
    if (ctrlr1 && ctrlr2 && !strcmp(ctrlr1, ctrlr2))
      internal_boundary = true;
    is_ctrlr_disc =
       (ctr_mgr->IsCtrlrInUnknownList(reinterpret_cast<char*>(remote_ctrlr))) &&
       (internal_boundary == false);
  }
  if ((bound_vlink == true) &&
      (vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_INVALID)) {
    for (int i = 0; i < 3; i++) {
      if (is_ctrlr_disc && (i != 0)) {
        continue;
      }
      new_flag[i] |= PORT_FAULT;
    }
  }
  VnodeChildMoMgr *if_mgr[2] = {NULL, NULL};
  for (int i= 0; i < 2; i++) {
    if (ck_vnif[i]->get_key_type() != UNC_KT_VUNK_IF) {
      uint8_t valid_pm, valid_admin, admin_status;
      val_port_map_t *pm = NULL;
      if_mgr[i] = reinterpret_cast<VnodeChildMoMgr *>
                               (const_cast<MoManager*>
                               (GetMoManager(ck_vnif[i]->get_key_type())));
      result_code = if_mgr[i]->GetPortMap(ck_vnif[i], valid_pm, pm,
                                 valid_admin, admin_status);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in GetPortMap");
        return result_code;
      }
      if (pm == NULL) {
        continue;
      }
      if (admin_status == UPLL_ADMIN_DISABLE) {
        uint8_t npos = (i == 0)? 1 : 0;
        new_flag[i] |= ADMIN_DISABLE;
        if (!is_ctrlr_disc) {
          new_flag[2] |= REMOTE_DOWN;
          new_flag[npos] |= REMOTE_DOWN;
        }
      }
      if (valid_pm == UNC_VF_VALID) {
        if (i == 1 && !internal_boundary)
          continue;
        unc_key_type_t ktype = ck_vnif[i]->get_key_type();
        if (ktype != UNC_KT_VTEP_IF && ktype != UNC_KT_VTUNNEL_IF) {
          //  get logical port state
          controller_domain ctrlr_dom = {NULL, NULL};
          GET_USER_DATA_CTRLR_DOMAIN(ck_vnif[i], ctrlr_dom)
          bool result(false);
          uint8_t state(UPPL_LOGICAL_PORT_OPER_UNKNOWN);
          result = ctr_mgr->GetLogicalPortSt(
              reinterpret_cast<const char*>(ctrlr_dom.ctrlr),
              reinterpret_cast<const char*>(pm->logical_port_id),
              state);
          if (result == false) {
            val_db_vbr_if_st *vnif_st = reinterpret_cast<val_db_vbr_if_st *>
                                        (GetStateVal(ck_vnif[i]));
            if (vnif_st->down_count & PORT_UNKNOWN) {
              new_flag[i] |= PORT_UNKNOWN;
              new_flag[2] |= PORT_UNKNOWN;
            }
            continue;
          }
          if (state == UPPL_LOGICAL_PORT_OPER_DOWN) {
            /* Set PORT_FAULT flag */
            for (int i= 0; i< 3; i++) {
              if (is_ctrlr_disc && (i != 0))
                continue;
              new_flag[i] |= PORT_FAULT;
            }
          }
        }
      }
    }
  }
  val_db_vlink_st *vlink_st = reinterpret_cast<val_db_vlink_st *>
                             (GetStateVal(ck_vlink));
  if (vlink_st == NULL) {
    UPLL_LOG_DEBUG("Error in retrieving vlink val st");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  if (ck_vnif[0]->get_key_type() != UNC_KT_VUNK_IF) {
    val_db_vbr_if_st *vnif_st = reinterpret_cast<val_db_vbr_if_st *>
                               (GetStateVal(ck_vnif[0]));
    uint8_t npos = 1;
    val_db_vbr_if_st *rem_vnif_st = NULL;
    uint8_t rem_orig_flag = 0;
    if (ck_vnif[npos]->get_key_type() != UNC_KT_VUNK_IF) {
      rem_vnif_st = reinterpret_cast<val_db_vbr_if_st *>
                                 (GetStateVal(ck_vnif[npos]));
      if (rem_vnif_st == NULL) {
        UPLL_LOG_DEBUG("Error in getting state val");
        return UPLL_RC_ERR_GENERIC;
      }
      rem_orig_flag = rem_vnif_st->down_count;
      if (rem_vnif_st->down_count & PORT_FAULT) {
        for (int i= 0; i< 3; i++) {
          if (is_ctrlr_disc && (i != 1))
            continue;
          new_flag[i] |= PORT_FAULT;
        }
      }
      if (rem_vnif_st->down_count & REMOTE_VTN_EXHAUSTION)
        new_flag[1] |= REMOTE_VTN_EXHAUSTION;
      if (rem_vnif_st->down_count & REMOTE_PATH_FAULT)
        new_flag[1] |= REMOTE_PATH_FAULT;
    }
    state_notification notification = kCommit;
    if (vnif_st->down_count != new_flag[0]) {
      UPLL_LOG_DEBUG("orig flag[%d] is set to %d", 0, vnif_st->down_count);
      // vnif_st->down_count = new_flag[0];
      if (is_unified) {
        controller_domain ctrlr_dom = {
          reinterpret_cast<uint8_t*>(const_cast<char*>("#")),
          reinterpret_cast<uint8_t*>(const_cast<char*>("#"))};
        SET_USER_DATA_CTRLR_DOMAIN(ck_vnif[0], ctrlr_dom);
        notification = kPortFaultReset;
        result_code =  if_mgr[0]->UpdateOperStatus(ck_vnif[0], dmi,
                                        notification, UNC_OP_UPDATE,
                                        true, true);
      } else {
        vnif_st->down_count = new_flag[0];
        vnif_st->vbr_if_val_st.valid[0] = UNC_VF_VALID;
        if (vnif_st->down_count == PORT_UP) {
          /* if down_count is PORT_UP, set operStatus to UP */
          vnif_st->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_UP;
        } else if (vnif_st->down_count & PORT_UNKNOWN) {
          /* if down_count is UNKNOWN, set operStatus to UNKNOWN */
          vnif_st->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
        } else {
          /* if down_count is DOWN, set operStatus to DOWN */
          vnif_st->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
        }
        result_code = if_mgr[0]->UpdateConfigDB(ck_vnif[0], UPLL_DT_STATE,
                                              UNC_OP_UPDATE,
                                              dmi, &dbop, TC_CONFIG_GLOBAL,
                                              vtn_id, MAINTBL);
      }
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in UpdateConfigDB : %d", result_code);
        return result_code;
      }
    }
    if ((ck_vnif[1]->get_key_type()) != UNC_KT_VUNK_IF) {
      notification = kCommit;
      if (rem_orig_flag != new_flag[1]) {
        bool propagate = false;
        if (bound_vlink)
          propagate = true;
        if (bound_vlink == false || !(rem_orig_flag & PORT_UNKNOWN) ||
            is_unified) {
          if (is_unified && (rem_orig_flag & PORT_UNKNOWN))
            notification = kPortFaultReset;
          else
            rem_vnif_st->down_count = new_flag[1];
          result_code =  if_mgr[1]->UpdateOperStatus(ck_vnif[1], dmi,
                                        notification, UNC_OP_UPDATE,
                                        true, propagate);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Error in UpdateConfigDB : %d", result_code);
            return result_code;
          }
        }
      }
    }
  }
  if (!(vlink_st->down_count == PORT_UP &&
      vlink_st->vlink_val_st.oper_status == UPLL_OPER_STATUS_UNKNOWN)) {
    if (vlink_st->down_count == new_flag[2])
      return UPLL_RC_SUCCESS;
  }
  vlink_st->down_count = new_flag[2];
  vlink_st->vlink_val_st.valid[0] = UNC_VF_VALID;
  if (vlink_st->down_count == PORT_UP) {
    /* if down_count is PORT_UP, set operStatus to UP */
    vlink_st->vlink_val_st.oper_status = UPLL_OPER_STATUS_UP;
  } else if (vlink_st->down_count & ~PORT_UNKNOWN) {
    /* if down_count is DWON, set operStatus to DOWN */
    vlink_st->vlink_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
  } else {
    /* if down_count is UNKNOWN, OperStatus is alreay UNKNOWN. Return */
    return UPLL_RC_SUCCESS;
  }
  result_code = UpdateConfigDB(ck_vlink, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                               &dbop, TC_CONFIG_GLOBAL,
                               vtn_id, MAINTBL);
  return result_code;
}

upll_rc_t VlinkMoMgr::BoundaryStatusHandler(uint8_t *boundary_name,
                                            bool oper_status,
                                            uuc::UpllDbConnMgr* db_con,
                                            uuc::ConfigLock* cfg_lock) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  std::set<std::string> vtn_list;
  ConfigKeyVal *ck_vlink = NULL;
  result_code = GetChildConfigKey(ck_vlink, NULL);
  if (!ck_vlink || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  ConfigVal *cfg_val = NULL;
  result_code = AllocVal(cfg_val, UPLL_DT_RUNNING, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in allocating cfg_val");
    DELETE_IF_NOT_NULL(ck_vlink);
    return result_code;
  }
  ck_vlink->AppendCfgVal(cfg_val);
  val_vlink *vlink_val = reinterpret_cast<val_vlink*>(GetVal(ck_vlink));
  /* copy fault boundaryname to vlink value structure */
  uuu::upll_strncpy(reinterpret_cast<char *>(vlink_val->boundary_name),
                    reinterpret_cast<char *>(boundary_name),
                    (kMaxLenBoundaryName + 1));

  /* set Boundary flag as valid */
  vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] = UNC_VF_VALID;

  { // scope lock
    uuc::ScopedConfigLock lock(*cfg_lock, uuc::kNormalTaskPriority,
                               UPLL_DT_RUNNING, uuc::ConfigLock::CFG_READ_LOCK);
    uuc::DalOdbcMgr *dmi = db_con->GetAlarmRwConn();
    if (dmi == NULL) {
      DELETE_IF_NOT_NULL(ck_vlink);
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = GetUniqueVtns(ck_vlink, &vtn_list, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in GetUniqueVtns");
      DELETE_IF_NOT_NULL(ck_vlink);
      db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
      db_con->ReleaseRwConn(dmi);
      return result_code;
    }
    db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
    db_con->ReleaseRwConn(dmi);
    if (vtn_list.empty()) {
      DELETE_IF_NOT_NULL(ck_vlink);
      return UPLL_RC_SUCCESS;
    }
  }  // scope lock
  uuc::NormalTaskYield oper_yield(
      uuc::TaskYieldIntf::YIELD_OP_OPER_STATUS_CTR_EVENT, cfg_lock);
  std::set<std::string>::iterator it = vtn_list.begin(),
                                  it_end = vtn_list.end();
  for (; it != it_end; ++it) {
    uuc::ScopedYield scfg_yield(&oper_yield);
    uuc::DalOdbcMgr *dmi = db_con->GetAlarmRwConn();
    if (dmi == NULL) {
      vtn_list.clear();
      DELETE_IF_NOT_NULL(ck_vlink);
      return UPLL_RC_ERR_GENERIC;
    }
    key_vtn *vtn_key = reinterpret_cast<key_vtn*>(ck_vlink->get_key());
    uuu::upll_strncpy(vtn_key->vtn_name, (*it).c_str(), kMaxLenVtnName + 1);
    result_code = BoundaryStatusHandler(ck_vlink, oper_status, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Error in BoundaryStatusHandler : %d", result_code);
      DELETE_IF_NOT_NULL(ck_vlink);
      vtn_list.clear();
      db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
      db_con->ReleaseRwConn(dmi);
      return result_code;
    }
    db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
    db_con->ReleaseRwConn(dmi);
  }
  DELETE_IF_NOT_NULL(ck_vlink);
  return result_code;
}

upll_rc_t VlinkMoMgr::BoundaryStatusHandler(ConfigKeyVal *ikey,
                                            bool oper_status, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vlink = NULL;
  result_code = DupConfigKeyVal(ck_vlink, ikey, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Error in DupConfigKeyVal %d", result_code);
    return result_code;
  }

  state_notification notification =
       (oper_status == UPLL_OPER_STATUS_UP) ? kPortFaultReset :
                                              kPortFault;

  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
                    kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain};

  /* Getting list of Vlinks that have same boundaryname */
  result_code = ReadConfigDB(ck_vlink, UPLL_DT_STATE, UNC_OP_READ, dbop,
      dmi, MAINTBL);

  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      result_code = UPLL_RC_SUCCESS;
    else
      UPLL_LOG_DEBUG("Error in reading %d", result_code);
    DELETE_IF_NOT_NULL(ck_vlink);
    return result_code;
  }
  while (ck_vlink != NULL) {
    ConfigKeyVal *tmp_ck_vlink = ck_vlink->get_next_cfg_key_val();
    ck_vlink->set_next_cfg_key_val(NULL);
    /* update consolidated oper status */
    /* get the constituent interfaces */
    ConfigKeyVal *vnif[2] = {NULL, NULL};
    VnodeChildMoMgr *ktype_mgr[2] = {NULL, NULL};
    val_db_vbr_if_st *vnif_st[2] = {NULL, NULL};
    val_db_vlink_st *vlink_st =
        reinterpret_cast<val_db_vlink_st*>(GetStateVal(ck_vlink));
    for (int i = 0; i < 2; i++) {
      result_code = GetVnodeIfFromVlink(ck_vlink, &vnif[i], dmi, i);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("get %d constituent interface of vlink failed %d",
                       i, result_code);
        DELETE_IF_NOT_NULL(ck_vlink);
        DELETE_IF_NOT_NULL(tmp_ck_vlink);
        return result_code;
      }
      unc_key_type_t ktype = vnif[i]->get_key_type();
      if (vnif[i]->get_key_type() != UNC_KT_VUNK_IF)
        vnif_st[i] = reinterpret_cast<val_db_vbr_if_st *>
                                 (GetStateVal(vnif[i]));
      ktype_mgr[i] = reinterpret_cast<VnodeChildMoMgr *>
                      (const_cast<MoManager*>(GetMoManager(ktype)));
    }
    for (int i = 0; i < 2; i++) {
      if (vnif[i]->get_key_type() != UNC_KT_VUNK_IF) {
        uint8_t valid_pm = 0, valid_admin = 0, admin_status = 0;
        val_port_map_t *pm = NULL;
        vlink_st->down_count &= ~PORT_UNKNOWN;
        result_code = ktype_mgr[i]->GetPortMap(vnif[i], valid_pm, pm,
                                   valid_admin, admin_status);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in GetPortMap");
          DELETE_IF_NOT_NULL(ck_vlink);
          DELETE_IF_NOT_NULL(tmp_ck_vlink);
          DELETE_IF_NOT_NULL(vnif[0]);
          DELETE_IF_NOT_NULL(vnif[1]);
          return result_code;
        }
        if (admin_status == UPLL_ADMIN_DISABLE) {
          uint8_t npos = (i == 0)? 1 : 0;
          vlink_st->down_count |= REMOTE_DOWN;
          vnif_st[i]->down_count |= ADMIN_DISABLE;
          if (vnif_st[npos] != NULL) {
            vnif_st[npos]->down_count |= REMOTE_DOWN;
          }
        }
      }
    }
    /* setting vlink operstatus */
    result_code = SetOperStatus(ck_vlink, dmi, notification);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error ins vlink setoperstatus: %d", result_code);
      DELETE_IF_NOT_NULL(ck_vlink);
      DELETE_IF_NOT_NULL(tmp_ck_vlink);
      DELETE_IF_NOT_NULL(vnif[0]);
      DELETE_IF_NOT_NULL(vnif[1]);
      return result_code;
    }
    for (int i = 0; i < 2; i++) {
      if (vnif[i]->get_key_type() != UNC_KT_VUNK_IF) {
        result_code = ktype_mgr[i]->UpdateOperStatus(vnif[i], dmi,
                                  notification, UNC_OP_UPDATE, true);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Updating operstatus of vnode if failed %d\n",
                        result_code);
          DELETE_IF_NOT_NULL(ck_vlink);
          DELETE_IF_NOT_NULL(tmp_ck_vlink);
          DELETE_IF_NOT_NULL(vnif[0]);
          DELETE_IF_NOT_NULL(vnif[1]);
          return result_code;
        }
      }
    }
    for (int i = 0; i < 2; i++) {
      DELETE_IF_NOT_NULL(vnif[i]);
    }
    DELETE_IF_NOT_NULL(ck_vlink);
    ck_vlink = tmp_ck_vlink;
  }
//  DELETE_IF_NOT_NULL(ck_vlink);
  return result_code;
}

upll_rc_t VlinkMoMgr::RestoreVlinkOperStatus(uint8_t *ctrlr_name,
                         uint8_t *domain_name, uint8_t *vtn_name,
                         uuc::UpllDbConnMgr* db_con,
                         uuc::ConfigLock* cfg_lock, state_notification notfn) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *tmp_ck_vlink = NULL;
  string ctrlr_id = reinterpret_cast<const char*>(ctrlr_name);
  {
  uuc::ScopedConfigLock lock(*cfg_lock, uuc::kNormalTaskPriority,
                             UPLL_DT_RUNNING, uuc::ConfigLock::CFG_READ_LOCK);
  int nattr = 0;
  BindInfo *binfo = NULL;
  DalCursor *dal_cursor_handle = NULL;

  const uudst::kDalTableIndex tbl_index = GetTable(MAINTBL, UPLL_DT_RUNNING);

  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  DalBindInfo dal_bind_info(tbl_index);

  if (!GetBindInfo(MAINTBL, UPLL_DT_RUNNING, binfo, nattr)) {
    UPLL_LOG_DEBUG("GetBindInfo failed");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigKeyVal *ck_vlink = NULL;
  result_code = GetChildConfigKey(ck_vlink, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Invalid param");
    return result_code;
  }
  ConfigVal* cv_vlink = NULL;
  result_code = AllocVal(cv_vlink, UPLL_DT_RUNNING, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in AllocVal");
    return result_code;
  }
  ck_vlink->SetCfgVal(cv_vlink);
  GET_USER_DATA(ck_vlink);
  GET_USER_DATA(ck_vlink->get_cfg_val());

  void *tkey = NULL;
  for (int i = 0; i < nattr; i++) {
    switch (binfo[i].struct_type) {
    case CFG_KEY:
      tkey = ck_vlink->get_key();
    break;
    case CFG_VAL:
    case CFG_META_VAL:
      tkey = ck_vlink->get_cfg_val()->get_val();
    break;
    case CK_VAL:
      tkey = ck_vlink->get_user_data();
    break;
    case CK_VAL2:
      tkey = ck_vlink->get_cfg_val()->get_user_data();
    break;
    default:
      continue;
    }
    uint64_t indx = binfo[i].index;
    void *p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
                + binfo[i].offset);
    dal_bind_info.BindOutput(indx, binfo[i].app_data_type,
               binfo[i].array_size, p);
  }
  string query_string = "SELECT vtn_name, vlink_name, admin_status,"
     "vnode1_name, vnode1_ifname, vnode2_name, vnode2_ifname, boundary_name,"
     "label_type, label, description, controller1_name, domain1_id,"
     "controller2_name, domain2_id, valid_vnode1_name, valid_vnode1_ifname,"
     "valid_vnode2_name, valid_vnode2_ifname, valid_boundary_name,"
     "valid_label_type, valid_label, valid_description, key_flags, val_flags "
     "from ru_vlink_tbl where ";
  if (notfn == kCtrlrDisconnect) {
    query_string = query_string+ "((controller1_name='" + ctrlr_id +
    "' AND controller2_name!='" + ctrlr_id+ "') OR (controller1_name!='" +
    ctrlr_id +
    "' AND controller2_name='"+ ctrlr_id+"') "
    " OR controller1_name='#' OR controller2_name='#')";
  } else {
    string dom_id = reinterpret_cast<const char*>(domain_name);
    query_string = query_string + "(((controller1_name='" +
    ctrlr_id + "' AND domain1_id='" + dom_id + "') OR (controller2_name='"
    + ctrlr_id +
    "' AND domain2_id='"+ dom_id;
    if (notfn == kVtnExhaustion || notfn == kVtnExhaustionReset) {
      string vtn_id = reinterpret_cast<const char*>(vtn_name);
      query_string = query_string +
      "') OR controller1_name='#' OR controller2_name='#')" +
      " AND (vtn_name='" + vtn_id + "')";
    } else {
      query_string = query_string +"'))";
    }
    query_string = query_string + ")";
  }
  uuc::DalOdbcMgr *dmi = db_con->GetAlarmRwConn();
  if (dmi == NULL) {
    UPLL_LOG_ERROR("Error in GetAlarmRwConn");
    DELETE_IF_NOT_NULL(ck_vlink);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
               query_string, 0, &dal_bind_info,
               &dal_cursor_handle));
  ConfigKeyVal *last_tmp_ck_vlink = NULL;
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (result_code == UPLL_RC_SUCCESS) {
      ConfigKeyVal *ck_tmp = NULL;
      result_code = DupConfigKeyVal(ck_tmp, ck_vlink, MAINTBL);
      if (tmp_ck_vlink == NULL) {
        tmp_ck_vlink = ck_tmp;
      } else {
        last_tmp_ck_vlink->AppendCfgKeyVal(ck_tmp);
      }
      last_tmp_ck_vlink = ck_tmp;
    }
  }
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  }
  DELETE_IF_NOT_NULL(ck_vlink);
  if (dal_cursor_handle != NULL)
    dmi->CloseCursor(dal_cursor_handle, false);
  db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
  db_con->ReleaseRwConn(dmi);
  }
  static int yield_count = 0;
  ConfigKeyVal* ck_tmp = tmp_ck_vlink;

  // Below operstatus update needs to be yielded for every iter
  uuc::NormalTaskYield oper_yield(uuc::TaskYieldIntf::
                                  YIELD_OP_OPER_STATUS_CTR_EVENT,
                                  cfg_lock);
  while (tmp_ck_vlink) {
  {
    uuc::ScopedYield scfg_yield(&oper_yield);
    uuc::DalOdbcMgr *dmi = db_con->GetAlarmRwConn();
    if (dmi == NULL) {
      UPLL_LOG_ERROR("Error in GetAlarmRwConn");
      DELETE_IF_NOT_NULL(ck_tmp);
      return UPLL_RC_ERR_GENERIC;
    }
    // Yield after every 50 vlink operstatus processing
    // 50 can be modified based on the performance
    while (yield_count < 50 && tmp_ck_vlink) {
      ++yield_count;
      controller_domain_t vlink_ctrlr_dom[2] = {{NULL, NULL}, {NULL, NULL}};
      result_code = GetControllerDomainId(tmp_ck_vlink, vlink_ctrlr_dom);
      if (UPLL_RC_SUCCESS != result_code) {
        unc_key_type_t ktype[2] = {UNC_KT_ROOT, UNC_KT_ROOT};
        for (int vnode_count = 0; vnode_count < 2; vnode_count++) {
          ktype[vnode_count] =
                GetVlinkVnodeIfKeyType(tmp_ck_vlink, vnode_count);
        }
        if (ktype[0] != UNC_KT_VUNK_IF && ktype[1] != UNC_KT_VUNK_IF) {
          UPLL_LOG_DEBUG("Empty Controller name recieved.")
          UPLL_LOG_DEBUG("Controller is empty only for UNKNOWN controllers");
          db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
          db_con->ReleaseRwConn(dmi);
          DELETE_IF_NOT_NULL(ck_tmp);
          return result_code;
        }
      }
      val_vlink *tmp_val = reinterpret_cast<val_vlink*>(GetVal(tmp_ck_vlink));
      if (!tmp_val) {
        db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
        db_con->ReleaseRwConn(dmi);
        DELETE_IF_NOT_NULL(ck_tmp);
        return UPLL_RC_ERR_GENERIC;
      }
      if (tmp_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] != UNC_VF_VALID) {
        tmp_ck_vlink = tmp_ck_vlink->get_next_cfg_key_val();
        continue;
      }
      result_code = ResetCtrlrDomForUnifiedInterface(
                              tmp_val->boundary_name, vlink_ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Error in ResetCtrlrDomForUnifiedInterface : %d",
                                result_code);
        db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
        db_con->ReleaseRwConn(dmi);
        DELETE_IF_NOT_NULL(ck_tmp);
        return result_code;
      }

      char *ctrlr1 = reinterpret_cast<char*>(vlink_ctrlr_dom[0].ctrlr);
      char *ctrlr2 = reinterpret_cast<char*>(vlink_ctrlr_dom[1].ctrlr);
      char *dom1 = reinterpret_cast<char*>(vlink_ctrlr_dom[0].domain);
      char *dom2 = reinterpret_cast<char*>(vlink_ctrlr_dom[1].domain);
      string dom_id = reinterpret_cast<char*>(domain_name);
      uint8_t vnode_pos = 0;
      if (!ctrlr1 || !ctrlr2 || (strcmp(ctrlr1, ctrlr_id.c_str()) &&
                                 strcmp(ctrlr2, ctrlr_id.c_str()))) {
        // skip by pass interfaces
        tmp_ck_vlink = tmp_ck_vlink->get_next_cfg_key_val();
        continue;
      }
      if (ctrlr1 && strcmp(ctrlr1, ctrlr_id.c_str())) {
        vnode_pos = 0;
      } else if (ctrlr2 && strcmp(ctrlr2, ctrlr_id.c_str())) {
        vnode_pos = 1;
      } else if (dom1 && strcmp(dom1, dom_id.c_str())) {
        vnode_pos = 0;
      } else if (dom2 && strcmp(dom2, dom_id.c_str())) {
        vnode_pos = 1;
      }
      ConfigKeyVal *ck_remif = NULL;
      result_code =
             GetVnodeIfFromVlink(tmp_ck_vlink, &ck_remif, dmi, vnode_pos);
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          result_code = UPLL_RC_SUCCESS;
          DELETE_IF_NOT_NULL(ck_remif);
          tmp_ck_vlink = tmp_ck_vlink->get_next_cfg_key_val();
          continue;
        }
        UPLL_LOG_ERROR("get remote interface failed %d", result_code);
        DELETE_IF_NOT_NULL(ck_remif);
        DELETE_IF_NOT_NULL(ck_tmp);
        db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
        db_con->ReleaseRwConn(dmi);
        return result_code;
      }
      VnodeChildMoMgr *mgr = reinterpret_cast<VnodeChildMoMgr*>(
                                      const_cast<MoManager*>
                                      (GetMoManager(ck_remif->get_key_type())));
      result_code = mgr->RecomputeVlinkAndIfoperStatus(tmp_ck_vlink, ck_remif,
                                                       dmi, notfn);
      DELETE_IF_NOT_NULL(ck_remif);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("get remote interface failed %d", result_code);
        DELETE_IF_NOT_NULL(ck_tmp);
        db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
        db_con->ReleaseRwConn(dmi);
        return result_code;
      }
      yield_count++;
      tmp_ck_vlink = tmp_ck_vlink->get_next_cfg_key_val();
    }
    yield_count = 0;
    db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
    db_con->ReleaseRwConn(dmi);
  }
  }
  yield_count = 0;
  DELETE_IF_NOT_NULL(ck_tmp);
  return result_code;
}

upll_rc_t VlinkMoMgr::GetBoundaryStatusFromPhysical(uint8_t *boundary,
                            controller_domain_t *ctr_domain,
                            val_oper_status &bound_operStatus,
                            uint32_t session_id,
                            uint32_t config_id) {
  UPLL_FUNC_TRACE;
  if (uuc::UpllConfigMgr::GetUpllConfigMgr()->get_map_phy_resource_status()
      == false) {
    bound_operStatus = UPLL_OPER_STATUS_UNKNOWN;
    return UPLL_RC_SUCCESS;
  }
  IpcResponse ipc_resp;
  ConfigKeyVal *ck_bound = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (boundary == NULL) {
    UPLL_LOG_DEBUG("Invalid boundary\n");
    return UPLL_RC_ERR_GENERIC;
  }
  key_boundary *bound_cfg  = static_cast<key_boundary *>
                        (ConfigKeyVal::Malloc(sizeof(key_boundary)));
  uuu::upll_strncpy(bound_cfg->boundary_id,
                    reinterpret_cast<const char *>(boundary),
                    (kMaxLenBoundaryName+1));
  ck_bound = new ConfigKeyVal(UNC_KT_BOUNDARY,
                              IpctSt::kIpcStKeyBoundary, bound_cfg, NULL);
  result_code = SendIpcReq(USESS_ID_UPLL, 0, UNC_OP_READ,
                   UPLL_DT_STATE, ck_bound, NULL, &ipc_resp);
  if ((result_code != UPLL_RC_SUCCESS) || (!ipc_resp.ckv_data)) {
    delete ck_bound;
    ck_bound = NULL;
    bound_operStatus = UPLL_OPER_STATUS_UNKNOWN;
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    UPLL_LOG_ERROR("Invalid Boundary %s %d\n", boundary, result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  ck_bound->ResetWith(ipc_resp.ckv_data);
  DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
  val_boundary_st *bound_st = static_cast<val_boundary_st *>
                                         (GetVal(ck_bound));
  if (!bound_st || (bound_st->valid[kIdxBoundaryStOperStatus] !=
                    UNC_VF_VALID)) {
    UPLL_LOG_DEBUG("Returning error \n");
    return UPLL_RC_ERR_GENERIC;
  }
  switch (bound_st->oper_status) {
    case UPPL_BOUNDARY_OPER_UP:
      bound_operStatus = UPLL_OPER_STATUS_UP;
      break;
    case UPPL_BOUNDARY_OPER_UNKNOWN:
      bound_operStatus = UPLL_OPER_STATUS_UNKNOWN;
      break;
    case UPPL_BOUNDARY_OPER_DOWN:
    default:
      bound_operStatus = UPLL_OPER_STATUS_DOWN;
      break;
  }
  if (ck_bound)
    delete ck_bound;
  return result_code;
}

upll_rc_t VlinkMoMgr::TxUpdateDtState(unc_key_type_t keytype,
                                      uint32_t session_id,
                                      uint32_t config_id,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vlink = NULL;

  result_code = GetUninitOperState(ck_vlink, dmi);
  if (!ck_vlink || UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return result_code;
  }
  ConfigKeyVal *vnif[2] = {NULL, NULL};
  while (ck_vlink) {
    ConfigKeyVal *tmp_ck_vlink = ck_vlink->get_next_cfg_key_val();
    ck_vlink->set_next_cfg_key_val(NULL);
    result_code = UpdateVlinkAndIfOperStatus(ck_vlink, vnif,
                                             session_id, config_id, dmi);
    DELETE_IF_NOT_NULL(ck_vlink);
    DELETE_IF_NOT_NULL(vnif[0]);
    DELETE_IF_NOT_NULL(vnif[1]);
    ck_vlink = tmp_ck_vlink;
  }
  return result_code;
}

upll_rc_t VlinkMoMgr::UpdateVlinkAndIfOperStatus(ConfigKeyVal *ck_vlink,
                          ConfigKeyVal **ck_vnif, uint32_t session_id,
                          uint32_t config_id, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // Get consitutent interface status - ignore if unknown
    VnodeChildMoMgr *ktype_mgr[2];
    bool bound_vlink = false;
    controller_domain_t vlink_ctrlr_dom[] = {{NULL, NULL}, {NULL, NULL}};
    result_code = BoundaryVlink(ck_vlink, vlink_ctrlr_dom, bound_vlink);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Returning error %d\n", result_code);
      return result_code;
    }
//    ConfigKeyVal *vnif[2] = {ck_vnif[0], ck_vnif[1]};
    /* orig_flag holds present down_count. new_flag is computed from scratch
     * index 0 is used for ck_vnif[0], 1 for ck_vnif[1] and 2 for vlink */
    uint32_t orig_flag[3] = {PORT_UP, PORT_UP, PORT_UP};
    uint32_t new_flag[3] = {PORT_UP, PORT_UP, PORT_UP};
    uint8_t admin_status[2] = {UPLL_ADMIN_ENABLE, UPLL_ADMIN_ENABLE};

    /* vnif_st holds state value of interface.
     * It will continue to be NULL for bypass controllers */
    val_db_vbr_if_st *vnif_st[2] = {NULL, NULL};

    /* get the interfaces from vlink and set orig_flag */
    for (int i = 0; i < 2; i++) {
      if (ck_vnif[i] == NULL) {
        result_code = GetVnodeIfFromVlink(ck_vlink, &ck_vnif[i], dmi, i);
        if (result_code != UPLL_RC_SUCCESS) {
          if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
            result_code = UPLL_RC_SUCCESS;
          UPLL_LOG_ERROR("get %d constituent interface of vlink failed %d",
                         i, result_code);
          return result_code;
        }
      }
      unc_key_type_t ktype = ck_vnif[i]->get_key_type();
      ktype_mgr[i] = reinterpret_cast<VnodeChildMoMgr *>
                      (const_cast<MoManager*>(GetMoManager(ktype)));
      uint8_t valid_pm, valid_admin;
      val_port_map_t *pm = NULL;
      result_code = ktype_mgr[i]->GetPortMap(ck_vnif[i], valid_pm, pm,
                                 valid_admin, admin_status[i]);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetPortMap failed %d %d\n", i, result_code);
        return result_code;
      }
      if (ck_vnif[i]->get_key_type() != UNC_KT_VUNK_IF) {
        vnif_st[i] = reinterpret_cast<val_db_vbr_if_st *>
                                 (GetStateVal(ck_vnif[i]));
        UPLL_LOG_DEBUG("orig flag[%d] is set to %d", i, vnif_st[i]->down_count);
        if (vnif_st[i] == NULL) {
          return UPLL_RC_ERR_GENERIC;
        }
        orig_flag[i] = vnif_st[i]->down_count;
      }
    }

    /* set orig_flag of vlink */
    val_db_vlink_st *vlink_st = reinterpret_cast<val_db_vlink_st *>
                               (GetStateVal(ck_vlink));
    if (vlink_st == NULL) {
      return UPLL_RC_ERR_GENERIC;
    }
    orig_flag[2] = vlink_st->down_count;
    UPLL_LOG_DEBUG("orig flag[2] of vlink is set to %d", vlink_st->down_count);

    /* check for the type of vlink */
    val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ck_vlink));
    if (vlink_val == NULL) {
      return UPLL_RC_ERR_GENERIC;
    }
    uuc::CtrlrMgr* ctr_mgr = uuc::CtrlrMgr::GetInstance();
    bool is_path_fault = false;
    if (ck_vnif[0]->get_key_type() != UNC_KT_VUNK_IF) {
      is_path_fault = ctr_mgr->IsPathFaultOccured(
                         reinterpret_cast<char*>(vlink_ctrlr_dom[0].ctrlr),
                         reinterpret_cast<char*>(vlink_ctrlr_dom[0].domain));
    }
    if (!bound_vlink) {
    /* vlink is internal vlink. vLink supports admin status */
      if (vlink_val->admin_status == UPLL_ADMIN_DISABLE) {
        /* if admin is disabled, set vlink ADMIN_DISABLE flag and
         * REMOTE_DOWN flag for interfaces */
        new_flag[2] = ADMIN_DISABLE;
        for (int i = 0; i< 2; i++) {
          new_flag[i] = REMOTE_DOWN;
        }
      } else {
        /* clearing admin_status flag is done here.
         * Since it is straight forward,
         * it is not handled as a seperate switch case in SetOperStatus */
        vlink_st->down_count &= ~ADMIN_DISABLE;
        if (vnif_st[0] != NULL)
          vnif_st[0]->down_count &= ~REMOTE_DOWN;
        if (vnif_st[1] != NULL)
          vnif_st[1]->down_count &= ~REMOTE_DOWN;
      }
    } else {
      if (vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_INVALID) {
        /* vLink is Boundary vlink, but is not boundary mapped */
        for (int i = 0; i < 3; i++) {
          new_flag[i] = PORT_FAULT;
        }
        for (int i = 0; i < 2; i++) {
          if ((vnif_st[i] != NULL) &&
              (vnif_st[i]->down_count & PORT_UNKNOWN)) {
            new_flag[i] = PORT_UNKNOWN;
            new_flag[2] = PORT_UNKNOWN;
          }
        }
      } else {
        result_code = ResetCtrlrDomForUnifiedInterface(vlink_val->boundary_name,
                                         vlink_ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Error in ResetCtrlrDomForUnifiedInterface : %d",
                          result_code);
          return result_code;
        }
        if (session_id != 0 && config_id != 0) {
          /* vLink is Boundary vlink and boundary mapped */
          val_oper_status bound_oper_status = UPLL_OPER_STATUS_DOWN;

          result_code = GetBoundaryStatusFromPhysical(vlink_val->boundary_name,
                        vlink_ctrlr_dom, bound_oper_status,
                        session_id, config_id);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Returning error %d\n", result_code);
            if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
              result_code = UPLL_RC_SUCCESS;
          }
          switch (bound_oper_status) {
          case  UPLL_OPER_STATUS_DOWN:
            for (int i = 0; i< 3; i++) {
              new_flag[i] |= PORT_FAULT;
            }
            break;
          case  UPLL_OPER_STATUS_UNKNOWN:
            new_flag[2] |= PORT_UNKNOWN;
            for (int i = 0; i< 2; i++) {
              if (ck_vnif[i]->get_key_type() != UNC_KT_VUNK_IF) {
                if (vnif_st[i]->down_count == PORT_UNKNOWN) {
                  new_flag[i] |= PORT_UNKNOWN;
                  continue;
                }
                uint8_t valid_pm = 0, valid_admin = 0, admin_status = 0;
                val_port_map_t *pm = NULL;
                result_code =  ktype_mgr[i]->GetPortMap(
                    ck_vnif[i], valid_pm,
                    pm, valid_admin, admin_status);
                if (result_code != UPLL_RC_SUCCESS) {
                  UPLL_LOG_DEBUG("Returning error %d\n", result_code);
                  return result_code;
                }
                if (pm != NULL) {
                  controller_domain_t ctrlr_dom = {NULL, NULL};
                  val_oper_status port_oper_status = UPLL_OPER_STATUS_UP;
                  GET_USER_DATA_CTRLR_DOMAIN(ck_vnif[i], ctrlr_dom);
                  result_code =  ktype_mgr[i]->GetPortStatusFromPhysical(
                      pm, ctrlr_dom, port_oper_status);
                  if (result_code != UPLL_RC_SUCCESS) {
                    UPLL_LOG_DEBUG(
                        "Error retrieving port status from physical %d %d",
                        result_code, port_oper_status);
                  }
                  switch (port_oper_status) {
                  case  UPLL_OPER_STATUS_DOWN:
                    new_flag[i] |= PORT_FAULT;
                    break;
                  case  UPLL_OPER_STATUS_UNKNOWN:
                    new_flag[i] |= PORT_UNKNOWN;
                    break;
                  default:
                  // for OPER_UP do nothing
                    break;
                  }
                } else {
                  val_oper_status ctrlr_operstatus = UPLL_OPER_STATUS_UNKNOWN;
                  result_code = ktype_mgr[i]->GetCtrlrStatusFromPhysical(
                      vlink_ctrlr_dom[i].ctrlr, ctrlr_operstatus);
                  if (result_code != UPLL_RC_SUCCESS) {
                    UPLL_LOG_ERROR("GetCtrlrStatusFromPhysical failed : %d",
                                   result_code);
                  }
                  if (ctrlr_operstatus == UPLL_OPER_STATUS_UNKNOWN) {
                    new_flag[i] |= PORT_UNKNOWN;
                  }
                }
              }
            }
            break;
          default:
            // for OPER_UP do nothing
            break;
          }
        }
      }
      if (is_path_fault) {
        new_flag[1] |= REMOTE_PATH_FAULT;
        is_path_fault = false;
      }
      if (ck_vnif[1]->get_key_type() != UNC_KT_VUNK_IF) {
        is_path_fault = ctr_mgr->IsPathFaultOccured(
                         reinterpret_cast<char*>(vlink_ctrlr_dom[1].ctrlr),
                         reinterpret_cast<char*>(vlink_ctrlr_dom[1].domain));
        if (is_path_fault) {
          new_flag[0] |= REMOTE_PATH_FAULT;
          is_path_fault = false;
        }
      }
    }

    /* get admin status of interfaces and
     * update the respective ADMIN_DISABLE or ADMIN_DISABLE flags */
    for (int i = 0; i < 2; i++) {
      if (ck_vnif[i]->get_key_type() != UNC_KT_VUNK_IF) {
        uint8_t valid_pm, valid_admin;
        val_port_map_t *pm = NULL;
        result_code = ktype_mgr[i]->GetPortMap(ck_vnif[i], valid_pm, pm,
                                   valid_admin, admin_status[i]);
        if (admin_status[i] == UPLL_ADMIN_DISABLE) {
          uint8_t npos = (i == 0)? 1 : 0;
          new_flag[2] |= REMOTE_DOWN;
          vlink_st->down_count |= REMOTE_DOWN;
          new_flag[i] |= ADMIN_DISABLE;
          vnif_st[i]->down_count |= ADMIN_DISABLE;
          new_flag[npos] |= REMOTE_DOWN;
          if (vnif_st[npos] != NULL) {
            vnif_st[npos]->down_count |= REMOTE_DOWN;
          }
        } else {
          uint8_t npos = (i == 0)? 1 : 0;
          vlink_st->down_count &= ~REMOTE_DOWN;
          vnif_st[i]->down_count &= ~ADMIN_DISABLE;
          if (vnif_st[npos] != NULL) {
            vnif_st[npos]->down_count &= ~REMOTE_DOWN;
          }
        }
      }
    }

    /* set the constituent interface OperStatus based on
     * original and new flag values */
    for (int i = 0; i < 2; i++) {
      if (ck_vnif[i]->get_key_type() != UNC_KT_VUNK_IF) {
       if (orig_flag[i] == PORT_UNKNOWN && new_flag[i] == PORT_UNKNOWN)
         continue;
        state_notification notification = kCommit;
        if (orig_flag[i] == PORT_UP) {
          /* original flag status is UP.
           * Set the new computed flag to down_count */
          vnif_st[i]->down_count = new_flag[i];
          notification = kCommit;
        } else  if (orig_flag[i] & PORT_UNKNOWN) {
          /* original flag is UNKNOWN */
          if (new_flag[i] & PORT_UNKNOWN) {
            /* port still continues to be UNKNOWN,
             * some other attribute has changed */
            vnif_st[i]->down_count = new_flag[i];
            notification = kCommit;
          } else if (new_flag[i] == PORT_UP) {
            /* new flag is UP */
            notification = kPortFaultReset;
          } else if (new_flag[i] & ~PORT_UNKNOWN) {
            /* new flag is DOWN */
            notification = kPortFault;
          }
        } else {
          /* original flag is DOWN */
          if (new_flag[i] == PORT_UP) {
            /* new flag is UP */
            notification = kPortFaultReset;
          } else if (new_flag[i] & PORT_UNKNOWN) {
            /* new status of interface is UNKNOWN */
            notification = kPortUnknown;
          } else {
            vnif_st[i]->down_count = new_flag[i];
            notification = kCommit;
            /* new status of interface is UP/DOWN */
          }
        }
        result_code = ktype_mgr[i]->UpdateOperStatus(ck_vnif[i], dmi,
                                      notification, UNC_OP_UPDATE, true);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d\n", result_code);
          return result_code;
        }
      }
    }
    /* set vlink down_count to new_flag and update the OperStatus */
    vlink_st->down_count = new_flag[2];
    result_code = SetOperStatus(ck_vlink, dmi, kCommit);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("SetOperStatus failed %d", result_code);
      return result_code;
    }
  return result_code;
}
upll_rc_t VlinkMoMgr::GetRenamedVnodeName(ConfigKeyVal *ikey,
                                          ConfigKeyVal *okey,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi,
                                          controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t val_rename = 0, val_rename1 = 0, vnode_rename = 0;
  bool same_vnode1 = false, same_vnode2 = false;
  val_vlink_t *val_vlink_next = NULL;
  // During UPDATE operation check both old/new vlink value structures
  // contains same vnode names before renaming to controller specific name
  if (ikey->get_cfg_val()->get_next_cfg_val() &&
      ((ikey->get_cfg_val()->get_next_cfg_val())->get_st_num() ==
       IpctSt::kIpcStValVlink)) {
    GET_USER_DATA_FLAGS(ikey->get_cfg_val()->get_next_cfg_val(), val_rename1);
    val_vlink_t *val_vlink = reinterpret_cast<val_vlink_t *>(GetVal(ikey));
    ConfigVal *configval_vlink_next = ikey->get_cfg_val()->get_next_cfg_val();
    val_vlink_next = reinterpret_cast<val_vlink_t *>
        (configval_vlink_next->get_val());
    if (!val_vlink || !val_vlink_next) {
      UPLL_LOG_DEBUG("Received NULL Val Structure");
      return UPLL_RC_ERR_GENERIC;
    }
    // If vnode1_name of old/new value structures
    // are same set same_vnode1 flag to true
    if (!strncmp(reinterpret_cast<char*>(val_vlink->vnode1_name),
                 reinterpret_cast<char*>(val_vlink_next->vnode1_name),
                 (kMaxLenVnodeName+1)))
      same_vnode1 = true;
    // If vnode2_name of old/new value structures
    // are same set same_vnode2 flag to true
    if (!strncmp(reinterpret_cast<char*>(val_vlink->vnode2_name),
                 reinterpret_cast<char*>(val_vlink_next->vnode2_name),
                 (kMaxLenVnodeName+1)))
      same_vnode2 = true;
  }

  GET_USER_DATA_FLAGS(ikey->get_cfg_val(), val_rename);
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                    (GetMoManager(UNC_KT_VBRIDGE)));
  if (!mgr) {
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *cfg_val = ikey->get_cfg_val();
  bool vnodes_copy[2] = {same_vnode1, same_vnode2};
  uint8_t val_count = 0;
  // check if any one of the vnode is renamed
  // in both old/new vlink value structures
  if ((val_rename & 0x04) || (val_rename & 0x08) ||
      (val_rename1 & 0x04) || (val_rename1 & 0x08)) {
    while (cfg_val && val_count <= 1) {
      val_vlink_t *val_vlink = reinterpret_cast<val_vlink_t *>
          (cfg_val->get_val());
      bool vnode_flag[2] = {false, false};
      if (val_count == 0) {
        vnode_flag[0] = (val_rename & 0x04)? true:false;
        vnode_flag[1] = (val_rename & 0x08)? true:false;
      } else if (val_count == 1) {
        vnode_flag[0] = (val_rename1 & 0x04)? true:false;
        vnode_flag[1] = (val_rename1 & 0x08)? true:false;
      }
      for (int i = 0; i < 2; i++) {
        bool copy_vnode = (i == 0)?same_vnode1:same_vnode2;
        uint8_t *vnode_name = (0 == i)?val_vlink->vnode1_name:
            val_vlink->vnode2_name;
        vnode_rename = (i == 0)? 0x80:0x40;
        // If the vnode names are same in both the value structures,
        // it is not required to rename old value structure,
        // since it is copied in the first iteration itself
        if (vnode_flag[i]) {
          if (val_count == 1 && vnodes_copy[i]) {
            continue;
          }
          if (val_count == 0) {
            SET_USER_DATA_FLAGS(ikey->get_cfg_val(), vnode_rename);
            result_code =  mgr->GetChildConfigKey(okey, ikey);
            if (UPLL_RC_SUCCESS != result_code) {
              DELETE_IF_NOT_NULL(okey);
              return result_code;
            }
          } else if (val_count == 1) {
            dt_type = (dt_type == UPLL_DT_CANDIDATE)?UPLL_DT_RUNNING:dt_type;
            key_vbr *vbr_key = reinterpret_cast<key_vbr *>
                (ConfigKeyVal::Malloc(sizeof(key_vbr)));
            okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                                    vbr_key, NULL);
            uint8_t *vnode_name;
            if (i == 0)
              vnode_name = val_vlink->vnode1_name;
            else
              vnode_name = val_vlink->vnode2_name;
            uuu::upll_strncpy(vbr_key->vbridge_name, vnode_name,
                              (kMaxLenVnodeName + 1));
            uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                              reinterpret_cast<key_vlink *>(ikey->get_key())->
                              vtn_key.vtn_name, (kMaxLenVtnName+1));
          }
          SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
          DbSubOp op = {kOpReadSingle, kOpMatchCtrlr,
            kOpInOutCtrlr | kOpInOutDomain};
          result_code =  mgr->ReadConfigDB(okey, dt_type,
                                           UNC_OP_READ, op, dmi, RENAMETBL);
          if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
            result_code = UPLL_RC_SUCCESS;
            DELETE_IF_NOT_NULL(okey);
            continue;
          }
          if (UPLL_RC_SUCCESS == result_code) {
            val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode *>
                (GetVal(okey));
            if (!rename_val) {
              UPLL_LOG_DEBUG("Val is Empty");
              DELETE_IF_NOT_NULL(okey);
              return UPLL_RC_ERR_GENERIC;
            }
            UPLL_LOG_TRACE("The controller vnode name is %s",
                           rename_val->ctrlr_vnode_name);
            uuu::upll_strncpy(vnode_name, rename_val->ctrlr_vnode_name,
                              (kMaxLenVnodeName+1));
            // If vnode names are same in the both
            // value structure and if vnode of new
            // val structure is renamed, then copy the
            // same to old value structure
            if ((copy_vnode) && (i == 0)) {
              uuu::upll_strncpy(val_vlink_next->vnode1_name,
                                rename_val->ctrlr_vnode_name,
                                (kMaxLenVnodeName+1));
            } else if ((copy_vnode) && (i == 1)) {
              uuu::upll_strncpy(val_vlink_next->vnode2_name,
                                rename_val->ctrlr_vnode_name,
                                (kMaxLenVnodeName+1));
            }
            DELETE_IF_NOT_NULL(okey);
          } else {
            UPLL_LOG_DEBUG("ReadconfigDB failed. Result : %d",
                           result_code);
            DELETE_IF_NOT_NULL(okey);
            return result_code;
          }
        }
      }
      cfg_val = ikey->get_cfg_val()->get_next_cfg_val();
      val_count++;
    }
  }
  DELETE_IF_NOT_NULL(okey);
  return result_code;
}

upll_rc_t VlinkMoMgr::GetChildConvertConfigKey(ConfigKeyVal *&okey,
                                             ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_convert_vlink_t * conv_vlink_key = NULL;

  unc_key_type_t ktype;

  void *pkey;
  if (parent_key == NULL) {
    if (!okey) {
      conv_vlink_key = reinterpret_cast<key_convert_vlink *>
          (ConfigKeyVal::Malloc(sizeof(key_convert_vlink)));

      okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyConvertVlink,
                              conv_vlink_key, NULL);
    }
    return (okey)?UPLL_RC_SUCCESS:UPLL_RC_ERR_GENERIC;
  } else {
    pkey = parent_key->get_key();
  }

  if (!pkey) {
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey) {
    if (okey->get_key_type() != UNC_KT_VLINK)
      return UPLL_RC_ERR_GENERIC;
  }

  if ((okey) && (okey->get_key())) {
    conv_vlink_key = reinterpret_cast<key_convert_vlink_t *>(okey->get_key());
  } else {
    conv_vlink_key = reinterpret_cast<key_convert_vlink_t *>
        (ConfigKeyVal::Malloc(sizeof(key_convert_vlink_t)));
  }

  ktype = parent_key->get_key_type();
  switch (ktype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(conv_vlink_key->vbr_key.vtn_key.vtn_name,
             reinterpret_cast<key_vtn *>(pkey)->vtn_name,
             (kMaxLenVtnName+1) );
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(conv_vlink_key->vbr_key.vtn_key.vtn_name,
             reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name,
             (kMaxLenVtnName+1) );
      uuu::upll_strncpy(conv_vlink_key->vbr_key.vbridge_name,
           reinterpret_cast<key_vbr *>(pkey)->vbridge_name,
           (kMaxLenVnodeName+1));
      break;
    case UNC_KT_VBR_PORTMAP:
      uuu::upll_strncpy(conv_vlink_key->vbr_key.vtn_key.vtn_name,
         reinterpret_cast<key_vbr_portmap*>(pkey)->vbr_key.vtn_key.vtn_name,
         (kMaxLenVtnName+1) );
      uuu::upll_strncpy(conv_vlink_key->vbr_key.vbridge_name,
        reinterpret_cast<key_vbr_portmap*>(pkey)->vbr_key.vbridge_name,
        (kMaxLenVnodeName+1));
      break;
    case UNC_KT_VBR_IF:
      uuu::upll_strncpy(conv_vlink_key->vbr_key.vtn_key.vtn_name,
         reinterpret_cast<key_convert_vbr_if_t*>
               (pkey)->convert_vbr_key.vbr_key.vtn_key.vtn_name,
         (kMaxLenVtnName+1) );
      uuu::upll_strncpy(conv_vlink_key->vbr_key.vbridge_name,
        reinterpret_cast<key_convert_vbr_if_t*>
                               (pkey)->convert_vbr_key.vbr_key.vbridge_name,
        (kMaxLenVnodeName+1));
      break;
    case UNC_KT_VLINK:
      if (parent_key->get_st_num() == IpctSt::kIpcStKeyConvertVlink) {
        uuu::upll_strncpy(conv_vlink_key->vbr_key.vtn_key.vtn_name,
            reinterpret_cast<key_convert_vlink *>(
                pkey)->vbr_key.vtn_key.vtn_name, (kMaxLenVtnName+1));
        uuu::upll_strncpy(conv_vlink_key->vbr_key.vbridge_name,
            reinterpret_cast<key_convert_vlink *>(pkey)->vbr_key.vbridge_name,
           (kMaxLenVnodeName+1));
        uuu::upll_strncpy(conv_vlink_key->convert_vlink_name,
            reinterpret_cast<key_convert_vlink *>(pkey)->convert_vlink_name,
           (kMaxLenVlinkName+1));
      }
      break;
    default:
      if (!okey || !(okey->get_key()))
        free(conv_vlink_key);
      return UPLL_RC_ERR_GENERIC;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyConvertVlink,
                            conv_vlink_key, NULL);
  else if (okey->get_key() != conv_vlink_key)
    okey->SetKey(IpctSt::kIpcStKeyConvertVlink, conv_vlink_key);
  if (okey == NULL) {
    free(conv_vlink_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}

upll_rc_t VlinkMoMgr::ConvertVlink(ConfigKeyVal *ikey,
                                   unc_keytype_operation_t operation,
                                   DalDmlIntf *dmi, TcConfigMode config_mode,
                                   string vtn_name) {
  UPLL_FUNC_TRACE;

  //  validate input
  if (!ikey || !ikey->get_key() || !dmi || (operation != UNC_OP_CREATE)) {
    UPLL_LOG_ERROR("Invalid inputs received");
    return UPLL_RC_ERR_GENERIC;
  }

  //  For create of vlink all value structure attributes
  //  are mandatory.
  val_convert_vlink *convert_vlink_val = reinterpret_cast<val_convert_vlink*>
      (GetVal(ikey));
  if (!convert_vlink_val ||
      convert_vlink_val->valid[UPLL_IDX_LABEL_CVLINK] != UNC_VF_VALID ||
      convert_vlink_val->valid[UPLL_IDX_VNODE1_NAME_CVLINK] != UNC_VF_VALID ||
      convert_vlink_val->valid[UPLL_IDX_VNODE1_IF_NAME_CVLINK] !=
      UNC_VF_VALID ||
      convert_vlink_val->valid[UPLL_IDX_VNODE2_NAME_CVLINK] != UNC_VF_VALID ||
      convert_vlink_val->valid[UPLL_IDX_VNODE2_IF_NAME_CVLINK] !=
      UNC_VF_VALID) {
    UPLL_LOG_ERROR("Received invalid inputs");
    return UPLL_RC_ERR_GENERIC;
  }

  //  During create, creates entry in ca_convert_vlink_tbl
  //  and vtn_gateway_tbl.
  return CreateConvertVlink(ikey, dmi, config_mode, vtn_name);
}

upll_rc_t
VlinkMoMgr::GetBoundaryFromVlinkControllerDomain(ConfigKeyVal **uppl_bdry,
                    controller_domain_t *ctrlr_dom, TcConfigMode config_mode) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  //  Get boundary information from UPPL based on leaf node controller domain
  //  and spine node controller domai
  uint32_t  session_id  = USESS_ID_UPLL;
  uint32_t  config_id   = 0;
  IpcResponse ipc_resp;
  memset(&ipc_resp, 0, sizeof(IpcResponse));

  key_boundary *bndry_key = reinterpret_cast<key_boundary *>
         (ConfigKeyVal::Malloc(sizeof(key_boundary)));
  val_boundary_t *boundary_val = reinterpret_cast<val_boundary *>(
      ConfigKeyVal::Malloc(sizeof(val_boundary)));
  ConfigVal *cv_boundary = new ConfigVal(IpctSt::kIpcStValBoundary,
                                         boundary_val);
  uuu::upll_strncpy(boundary_val->controller_name1, ctrlr_dom[0].ctrlr,
                    kMaxLenCtrlrId+1);
  boundary_val->valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
  uuu::upll_strncpy(boundary_val->controller_name2, ctrlr_dom[1].ctrlr,
                    kMaxLenCtrlrId+1);
  boundary_val->valid[kIdxBoundaryControllerName2] = UNC_VF_VALID;
  uuu::upll_strncpy(boundary_val->domain_name1, ctrlr_dom[0].domain,
                    kMaxLenDomainId+1);
  boundary_val->valid[kIdxBoundaryDomainName1] = UNC_VF_VALID;
  uuu::upll_strncpy(boundary_val->domain_name2, ctrlr_dom[1].domain,
                    kMaxLenDomainId+1);
  boundary_val->valid[kIdxBoundaryDomainName2] = UNC_VF_VALID;
  *uppl_bdry = new ConfigKeyVal(UNC_KT_BOUNDARY, IpctSt::kIpcStKeyBoundary,
                               bndry_key, cv_boundary);
  result_code = SendIpcReq(session_id, config_id, UNC_OP_READ_SIBLING,
                           UPLL_DT_CANDIDATE, *uppl_bdry, NULL, &ipc_resp);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    //  If unable to find the boundary information from UPPL
    //  Swap the controller_name1 and controller_name2,
    //  Swap the domain_name1 and domain_name2 and send Read
    //  request to UPPL with new ConfigKeyVal
    memset(boundary_val, 0, sizeof(val_boundary_t));
    uuu::upll_strncpy(boundary_val->controller_name1, ctrlr_dom[1].ctrlr,
        kMaxLenCtrlrId+1);
    boundary_val->valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
    uuu::upll_strncpy(boundary_val->controller_name2, ctrlr_dom[0].ctrlr,
        kMaxLenCtrlrId+1);
    boundary_val->valid[kIdxBoundaryControllerName2] = UNC_VF_VALID;
    uuu::upll_strncpy(boundary_val->domain_name1, ctrlr_dom[1].domain,
        kMaxLenDomainId+1);
    boundary_val->valid[kIdxBoundaryDomainName1] = UNC_VF_VALID;
    uuu::upll_strncpy(boundary_val->domain_name2, ctrlr_dom[0].domain,
        kMaxLenDomainId+1);
    boundary_val->valid[kIdxBoundaryDomainName2] = UNC_VF_VALID;

    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    result_code = SendIpcReq(session_id, config_id, UNC_OP_READ_SIBLING,
        UPLL_DT_CANDIDATE, *uppl_bdry, NULL, &ipc_resp);
    if (result_code == UPLL_RC_SUCCESS) {
      // Boundary information is avaiable in UPPL and
      // if acuired configuration mode is VTN mode, validate
      // whether the same boundary exist in RUNNING database
      if (config_mode == TC_CONFIG_VTN) {
        (*uppl_bdry)->ResetWith(ipc_resp.ckv_data);
        delete ipc_resp.ckv_data;
        result_code = SendIpcReq(session_id, config_id, UNC_OP_READ,
                                UPLL_DT_RUNNING, *uppl_bdry, NULL, &ipc_resp);
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_ERROR("Acquired configuration mode is VTN mode"
                         "boundary information not available in running");
          result_code = UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    }
  } else if (result_code == UPLL_RC_SUCCESS) {
    // Boundary information is avaiable in UPPL and
    // if acuired configuration mode is VTN mode, validate
    // whether the same boundary exist in RUNNING database
    if (config_mode == TC_CONFIG_VTN) {
      (*uppl_bdry)->ResetWith(ipc_resp.ckv_data);
      delete ipc_resp.ckv_data;
      result_code = SendIpcReq(session_id, config_id, UNC_OP_READ,
                               UPLL_DT_RUNNING, *uppl_bdry, NULL, &ipc_resp);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_ERROR("Acquired configuration mode is VTN mode"
                       "boundary information not available in running");
        result_code = UPLL_RC_ERR_CFG_SEMANTIC;
      }
    }
  }


  //  If unable to found boundary information reset result_code
  //  to UPLL_RC_ERR_CFG_SEMANTIC
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
     result_code = UPLL_RC_ERR_CFG_SEMANTIC;

  if ((result_code != UPLL_RC_SUCCESS) || (!ipc_resp.ckv_data)) {
    UPLL_LOG_DEBUG("Error in retrieving boundary info from UPPL");
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    DELETE_IF_NOT_NULL(*uppl_bdry);
    return result_code;
  }
  (*uppl_bdry)->ResetWith(ipc_resp.ckv_data);
  delete ipc_resp.ckv_data;

  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::CreateConvertVlink(ConfigKeyVal *ikey,
                                         DalDmlIntf *dmi,
                                         TcConfigMode config_mode,
                                         string vtn_name ) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_ERROR("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }

  key_convert_vlink *convert_vlink_key =
    reinterpret_cast<key_convert_vlink*>(ikey->get_key());
  val_convert_vlink_t *vlink_convert_val =
          reinterpret_cast<val_convert_vlink_t*>(GetVal(ikey));
  //  validate input
  if (!convert_vlink_key || !vlink_convert_val) {
    UPLL_LOG_ERROR("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("Received vLink ConfigKeyVal %s", (ikey->ToStrAll()).c_str());

  upll_rc_t         result_code  = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom[2] = { { NULL, NULL }, { NULL, NULL } };

  //  Get convert vLink controller domain
  result_code = GetControllerDomainId(ikey, ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to get vLink controller domain information");
    return result_code;
  }

  //  validate vLink controller domain
  if (!ctrlr_dom[0].ctrlr || !ctrlr_dom[0].domain ||
      !ctrlr_dom[1].ctrlr || !ctrlr_dom[1].domain) {
    UPLL_LOG_ERROR("Invalid controller and domain received");
    return UPLL_RC_ERR_GENERIC;
  }

  //  Get boundary_id from UPPL based on leaf node controller domain
  //  and spine node controller domain.
  //  If acquired config_mode is VTN mode, validate same boundary exist in
  //  runnning database
  ConfigKeyVal *bdry_ckv = NULL;
  result_code = GetBoundaryFromVlinkControllerDomain(&bdry_ckv,
                                           ctrlr_dom, config_mode);
  if (result_code != UPLL_RC_SUCCESS || !bdry_ckv) {
    if (result_code == UPLL_RC_ERR_CFG_SEMANTIC) {
      result_code = UPLL_RC_ERR_EXPAND;
    }
    UPLL_LOG_ERROR("Failed to get boundary information from UPLL."
                   "result_code = %u", result_code);
    DELETE_IF_NOT_NULL(bdry_ckv);
    return result_code;
  }
  UPLL_LOG_TRACE("boundary ConfigKeyVal %s", (bdry_ckv->ToStrAll()).c_str());

  //  Get boundary_id from uppl bdry_ckv key
  uint8_t *boundary_id = bdry_ckv->get_key()?
    (reinterpret_cast<key_boundary *>
     (bdry_ckv->get_key()))->boundary_id : NULL;
  if (boundary_id == NULL) {
    UPLL_LOG_DEBUG("Boundary key is empty");
    DELETE_IF_NOT_NULL(bdry_ckv);
    return UPLL_RC_ERR_GENERIC;
  }

  //  copy bdry_id to vlink_convert_val
  uuu::upll_strncpy(vlink_convert_val->boundary_name,
                    boundary_id,
                    (kMaxLenBoundaryName+1));
  vlink_convert_val->valid[UPLL_IDX_BOUNDARY_NAME_CVLINK] = UNC_VF_VALID;

  //  Validate vLink name is valid or not if not valid auto generate new name
  result_code = ValidateKey(reinterpret_cast<char *>(
          convert_vlink_key->convert_vlink_name),
      kMinLenVlinkName, kMaxLenVlinkName);
  if (result_code != UPLL_RC_SUCCESS) {
    while (1) {
      //  Auto generate vLink name
      struct timeval _timeval;
      struct timezone _timezone;
      gettimeofday(&_timeval, &_timezone);
      std::stringstream ss;
      ss << _timeval.tv_sec << _timeval.tv_usec;
      std::string unique_id = ss.str();
      std::string vlink_name("c_vl_");
      vlink_name += unique_id;

      // populate vlink_name in vlink_ckv
      uuu::upll_strncpy(convert_vlink_key->convert_vlink_name,
                        vlink_name.c_str(), (kMaxLenVlinkName + 1));

      ConfigKeyVal *conv_vlink_ckv = NULL;
      result_code = GetChildConvertConfigKey(conv_vlink_ckv, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to get convert ConfigKeyVal");
        DELETE_IF_NOT_NULL(bdry_ckv);
        return result_code;
      }

      DbSubOp dbop1 = { kOpReadExist, kOpMatchNone, kOpInOutNone };
      result_code = UpdateConfigDB(conv_vlink_ckv, UPLL_DT_CANDIDATE,
                                   UNC_OP_READ, dmi, &dbop1, CONVERTTBL);
      DELETE_IF_NOT_NULL(conv_vlink_ckv);
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        continue;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        break;
      } else {
        DELETE_IF_NOT_NULL(bdry_ckv);
        return result_code;
      }
    }
  }

  UPLL_LOG_TRACE("convert vLink ConfigKeyVal %s", (ikey->ToStrAll()).c_str());
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutDomain
    | kOpInOutCtrlr };

  //  create vLink in converttbl
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_CREATE,
                               dmi, &dbop, config_mode, vtn_name, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to create converted vLink");
    DELETE_IF_NOT_NULL(bdry_ckv);
    return result_code;
  }

  //  Create VTN gateway port
  VtnMoMgr *vtn_mgr =
      reinterpret_cast<VtnMoMgr *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VTN)));
  if (!vtn_mgr) {
    DELETE_IF_NOT_NULL(bdry_ckv);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = vtn_mgr->ConvertGatewayPort(ikey, bdry_ckv, dmi, UNC_OP_CREATE,
                                            config_mode, vtn_name);
  DELETE_IF_NOT_NULL(bdry_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to create converted vLink");
    return result_code;
  }

  return result_code;
}

/** This function delets the convert_vlink entry based on vtn and unified vbr
 * name and also matches ctrlr/domain if match_ctrlr_dom is set.
 * match_ctrlr_dom is false when second port-map with logical-port-id
 * gets deleted(this time need to remove 2 vlinks for given vtn/unified vbridge.
 * Also this function invokes ConvertGatewayPort to delete the vtn_gatewy_port
 * entry.
 */
upll_rc_t VlinkMoMgr::DeleteConvertVlink(
    ConfigKeyVal *ikey, bool match_ctrlr_dom, DalDmlIntf *dmi,
    TcConfigMode config_mode, string vtn_name ) {
  UPLL_FUNC_TRACE;

  upll_rc_t    result_code        = UPLL_RC_SUCCESS;
  ConfigKeyVal *convert_vlink_ckv = NULL;
  DbSubOp      dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};

  //  Get convert vLink ConfigKeyVal
  result_code = GetChildConvertConfigKey(convert_vlink_ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to get vlink convert ConfigKeyVal");
    return result_code;
  }

  // Delete corresponding entry from gateway port tbl
  VtnMoMgr *vtn_mgr =
      reinterpret_cast<VtnMoMgr *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VTN)));
  if (!vtn_mgr) {
    UPLL_LOG_ERROR("Invalid mgr");
    DELETE_IF_NOT_NULL(convert_vlink_ckv);
    return UPLL_RC_ERR_GENERIC;
  }

  if (!match_ctrlr_dom) {
    // Two boundary vlinks going to be deleted, need to send
    // two request to vtn gateway port delete
    DbSubOp read_dbop = { kOpReadMultiple, kOpMatchNone,
                          kOpInOutCtrlr|kOpInOutDomain};
    result_code = ReadConfigDB(convert_vlink_ckv, UPLL_DT_CANDIDATE,
                               UNC_OP_READ, read_dbop, dmi, CONVERTTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("ReadConfigDB failed:%d", result_code);
      DELETE_IF_NOT_NULL(convert_vlink_ckv);
      return result_code;
    }
    for (ConfigKeyVal *tmp = convert_vlink_ckv; tmp;
         tmp = tmp->get_next_cfg_key_val()) {
      result_code = vtn_mgr->ConvertGatewayPort(tmp, NULL, dmi, UNC_OP_DELETE,
                                                config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("ConvertGatewayPort Delete failed:%d", result_code);
        DELETE_IF_NOT_NULL(convert_vlink_ckv);
        return result_code;
      }
    }
    convert_vlink_ckv->DeleteCfgVal();
    StringReset(static_cast<key_convert_vlink_t*>(
            convert_vlink_ckv->get_key())->convert_vlink_name);
  } else {
    result_code = vtn_mgr->ConvertGatewayPort(ikey, NULL, dmi, UNC_OP_DELETE,
                                              config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("ConvertGatewayPort Delete failed:%d", result_code);
      DELETE_IF_NOT_NULL(convert_vlink_ckv);
      return result_code;
    }
    dbop.matchop = kOpMatchCtrlr|kOpMatchDomain;
  }

  //  Delete entry from ca_convert_vlink
  result_code = UpdateConfigDB(convert_vlink_ckv, UPLL_DT_CANDIDATE,
      UNC_OP_DELETE, dmi, &dbop, config_mode, vtn_name, CONVERTTBL);
  DELETE_IF_NOT_NULL(convert_vlink_ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to delete Convert Vlink, %d", result_code);
    return result_code;
  }

  return result_code;
}

upll_rc_t VlinkMoMgr::GetVlinkKeyValFromVbrPortMap(ConfigKeyVal *vbr_pm_ckv,
                                           ConfigKeyVal *&vlink_ckv,
                                           DalDmlIntf *dmi,
                                           upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint32_t  session_id  = USESS_ID_UPLL;
  uint32_t  config_id   = 0;

  pfcdrv_val_vbr_portmap_t *vbr_pm_val =
      reinterpret_cast<pfcdrv_val_vbr_portmap_t *>(GetVal(vbr_pm_ckv));
  if (!vbr_pm_val ||
      vbr_pm_val->vbrpm.valid[UPLL_IDX_LOGICAL_PORT_ID_VBRPM] != UNC_VF_VALID) {
    UPLL_LOG_ERROR("Logical port id is not present");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t *logical_port_id = vbr_pm_val->vbrpm.logical_port_id;

  IpcResponse ipc_resp;
  memset(&ipc_resp, 0, sizeof(IpcResponse));

  // ConfigKeyVal for Boundary
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  GET_USER_DATA_CTRLR_DOMAIN(vbr_pm_ckv, ctrlr_dom);

  key_boundary *bndrykey = static_cast<key_boundary *>
         (ConfigKeyVal::Malloc(sizeof(key_boundary)));  // COV NULL RETURN
  val_boundary_t *boundary_val = reinterpret_cast<val_boundary *>(
      ConfigKeyVal::Malloc(sizeof(val_boundary)));
  ConfigVal *cv_boundary = new ConfigVal(IpctSt::kIpcStValBoundary,
                                         boundary_val);
  uuu::upll_strncpy(boundary_val->logical_port_id2, logical_port_id,
                    kMaxLenLogicalPortId);
  boundary_val->valid[kIdxBoundaryLogicalPortId2] = UNC_VF_VALID;
  uuu::upll_strncpy(boundary_val->controller_name2, ctrlr_dom.ctrlr,
                    kMaxLenCtrlrId);
  boundary_val->valid[kIdxBoundaryControllerName2] = UNC_VF_VALID;
  ConfigKeyVal *ck_boundary = new ConfigKeyVal(UNC_KT_BOUNDARY,
                                               IpctSt::kIpcStKeyBoundary,
                                               bndrykey, cv_boundary);
//  SET_USER_DATA_CTRLR_DOMAIN(ck_boundary, ctrlr_dom);

  /* Case1: Audit - Always check boundary in RUNNING
   * Case2: Commit(del) -  check CANDIDATE
   * Case3: Commit(Cr/upd) - check RUNNING */
  upll_keytype_datatype_t dt_type = (UPLL_DT_AUDIT == datatype)?
                                    UPLL_DT_RUNNING:datatype;

  result_code = SendIpcReq(session_id, config_id, UNC_OP_READ_SIBLING,
                           dt_type, ck_boundary, NULL, &ipc_resp);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    //  If unable to find the boundary, copy the logical_port_id and
    //  controller_name in to boundary 1st node
    boundary_val = reinterpret_cast<val_boundary_t *>(GetVal(ck_boundary));
    memset(boundary_val, 0, sizeof(val_boundary_t));
    uuu::upll_strncpy(boundary_val->logical_port_id1, logical_port_id,
        kMaxLenLogicalPortId);
    boundary_val->valid[kIdxBoundaryLogicalPortId1] = UNC_VF_VALID;
    uuu::upll_strncpy(boundary_val->controller_name1, ctrlr_dom.ctrlr,
        kMaxLenCtrlrId);
    boundary_val->valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    result_code = SendIpcReq(session_id, config_id, UNC_OP_READ_SIBLING,
                           dt_type, ck_boundary, NULL, &ipc_resp);
  }

  if ((result_code != UPLL_RC_SUCCESS) || (!ipc_resp.ckv_data)) {
    UPLL_LOG_DEBUG("Error in retrieving boundary info from UPPL");
    DELETE_IF_NOT_NULL(ck_boundary);
    return result_code;
  }
  ck_boundary->ResetWith(ipc_resp.ckv_data);
  delete ipc_resp.ckv_data;

  ConfigKeyVal *ck_boundary_tmp = ck_boundary;
  //  It may be possible multiple boundaries for the same logical_port_id
  //  and controller_name.Same boundary can also be used in multiple vLink's
  //  based on each boundary get corresponding vLink's
  while (ck_boundary_tmp) {
    uint8_t *boundary_id = ck_boundary_tmp->get_key()?
      (reinterpret_cast<key_boundary *>
       (ck_boundary_tmp->get_key()))->boundary_id : NULL;
    if (boundary_id == NULL) {
      UPLL_LOG_DEBUG("Boundary key is empty");
      DELETE_IF_NOT_NULL(ck_boundary);
      return UPLL_RC_ERR_GENERIC;
    }

    //  Populate vLink ConfigKeyVal
    ConfigKeyVal *ckv_vlink = NULL;
    result_code = GetChildConfigKey(ckv_vlink, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed for vlink");
      DELETE_IF_NOT_NULL(ck_boundary);
      return result_code;
    }
    ConfigVal *cv_vlink = NULL;
    result_code = AllocVal(cv_vlink, datatype, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("AllocVal failed for vlink");
      DELETE_IF_NOT_NULL(ck_boundary);
      return result_code;
    }
    ckv_vlink->AppendCfgVal(cv_vlink);
    val_vlink *vlink_val = reinterpret_cast<val_vlink*>GetVal(ckv_vlink);
    if (vlink_val == NULL) {
      UPLL_LOG_DEBUG("val_vlink is NULL");
      DELETE_IF_NOT_NULL(ckv_vlink);
      DELETE_IF_NOT_NULL(ck_boundary);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vlink_val->boundary_name, boundary_id,
                      kMaxLenBoundaryName+1);
    vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] = UNC_VF_VALID;

    DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag};
    result_code = ReadConfigDB(ckv_vlink, datatype,
                               UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      ck_boundary_tmp = ck_boundary_tmp->get_next_cfg_key_val();
      DELETE_IF_NOT_NULL(ckv_vlink);
      continue;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Failed to read vlink info based on boundary id");
      DELETE_IF_NOT_NULL(ckv_vlink);
      DELETE_IF_NOT_NULL(ck_boundary);
      return result_code;
    }

    if (vlink_ckv == NULL) {
      vlink_ckv = ckv_vlink;
    } else {
      vlink_ckv->AppendCfgKeyVal(ckv_vlink);
    }

    UPLL_LOG_TRACE("%s", vlink_ckv->ToStrAll().c_str());
    ck_boundary_tmp = ck_boundary_tmp->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ck_boundary);

  // If vlink_ckv is NULL, do not send SUCCESS
  if (!vlink_ckv) {
    UPLL_LOG_INFO("No vlink found in datatype:%d", datatype);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VlinkMoMgr::ValidateConvertVlink(uuc::UpdateCtrlrPhase phase,
    DalDmlIntf *dmi, ConfigKeyVal **err_ckv, TxUpdateUtil *tx_util,
    TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *req = NULL, *nreq = NULL;
  DalCursor *dal_cursor_handle = NULL;

  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
      ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
       ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));

  if (op == UNC_OP_INVALID) {
    UPLL_LOG_INFO("Invalid operation received-%d", op);
    // Not a valid operation
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  // Other than create operation and other than VTN mode
  // bdry validation is not required
  if (UNC_OP_CREATE != op || config_mode != TC_CONFIG_VTN) {
    return UPLL_RC_SUCCESS;
  }

  result_code = GetChildConvertConfigKey(req, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to get convert vLink config key val");
    return result_code;
  }

  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                             op, req, nreq, &dal_cursor_handle, dmi,
                             config_mode, vtn_name, CONVERTTBL);
  while (result_code == UPLL_RC_SUCCESS) {
    if (tx_util->GetErrCount() > 0) {
      UPLL_LOG_ERROR("TxUpdateUtil says exit the loop.");
      dmi->CloseCursor(dal_cursor_handle, true);
      DELETE_IF_NOT_NULL(nreq);
      DELETE_IF_NOT_NULL(req);
      return UPLL_RC_ERR_GENERIC;
    }

    DalResultCode db_result = uud::kDalRcSuccess;
    // Iterate loop to get next record
    db_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetNextRecord failed err code(%d)", result_code);
      break;
    }

    //  If boundary is valid
    if (reinterpret_cast<val_convert_vlink_t*>(
        GetVal(req))->valid[UPLL_IDX_BOUNDARY_NAME_CVLINK] == UNC_VF_VALID) {
      ConfigKeyVal *ck_boundary = NULL;

      IpcReqRespHeader ipc_req;
      memset(&ipc_req, 0, sizeof(IpcReqRespHeader));
      ipc_req.datatype = UPLL_DT_RUNNING;
      ipc_req.operation = op;
      result_code = ValidateBoundary(reinterpret_cast<val_convert_vlink_t*>(
              GetVal(req))->boundary_name, &ipc_req, ck_boundary);
      if (UPLL_RC_SUCCESS  != result_code || !ck_boundary) {
        DELETE_IF_NOT_NULL(ck_boundary);
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_ERROR("Boundary information is not available in running DB");
          // TODO(U17): err_ckv fill
          result_code = UPLL_RC_ERR_CFG_SEMANTIC;
        }
        return result_code;
      }
      DELETE_IF_NOT_NULL(ck_boundary);
    }
  }
  if (dal_cursor_handle)
    dmi->CloseCursor(dal_cursor_handle, true);
  DELETE_IF_NOT_NULL(nreq);
  DELETE_IF_NOT_NULL(req);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
      UPLL_RC_SUCCESS : result_code;
  return result_code;
}
upll_rc_t VlinkMoMgr::ResetCtrlrDomForUnifiedInterface(uint8_t *boundary_name,
                      controller_domain *vlink_ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if ((vlink_ctrlr_dom[0].ctrlr &&
      !strcmp(reinterpret_cast<char*>(vlink_ctrlr_dom[0].ctrlr), "#")) ||
      (vlink_ctrlr_dom[1].ctrlr &&
      !strcmp(reinterpret_cast<char*>(vlink_ctrlr_dom[1].ctrlr), "#"))) {
    ConfigKeyVal *ck_boundary = NULL;
    IpcResponse ipc_resp;

    memset(&ipc_resp, 0, sizeof(IpcResponse));
    key_boundary *bndrykey = static_cast<key_boundary *>
         (ConfigKeyVal::Malloc(sizeof(key_boundary)));  // COV NULL RETURN

    uuu::upll_strncpy(bndrykey->boundary_id, boundary_name,
                   (kMaxLenBoundaryName+1));
    ck_boundary = new ConfigKeyVal(UNC_KT_BOUNDARY, IpctSt::kIpcStKeyBoundary,
                                 bndrykey, NULL);

    result_code = SendIpcReq(USESS_ID_UPLL, 0, UNC_OP_READ,
                   UPLL_DT_CANDIDATE, ck_boundary, NULL, &ipc_resp);
    if (UPLL_RC_SUCCESS  != result_code) {
      DELETE_IF_NOT_NULL(ck_boundary);
      UPLL_LOG_ERROR("Boundary information is not available in running DB");
      return result_code;
    }
    if (ipc_resp.ckv_data) {
      ck_boundary->ResetWith(ipc_resp.ckv_data);
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    }
    val_boundary_t *boundary_val =
        reinterpret_cast<val_boundary_t*>(GetVal(ck_boundary));
    if (!boundary_val)
      return UPLL_RC_ERR_GENERIC;
    if (vlink_ctrlr_dom[0].ctrlr &&
        !strcmp(reinterpret_cast<char*>(vlink_ctrlr_dom[0].ctrlr), "#")) {
      if (boundary_val->logical_port_id1[0] == 'M' &&
          boundary_val->logical_port_id1[1] == 'G') {
        uuu::upll_strncpy(vlink_ctrlr_dom[0].ctrlr,
                          boundary_val->controller_name1,
                          (kMaxLenCtrlrId+1));
        uuu::upll_strncpy(vlink_ctrlr_dom[0].domain, boundary_val->domain_name1,
                      (kMaxLenDomainId+1));
      } else {
        uuu::upll_strncpy(vlink_ctrlr_dom[0].ctrlr,
                          boundary_val->controller_name2,
                          (kMaxLenCtrlrId+1));
        uuu::upll_strncpy(vlink_ctrlr_dom[0].domain, boundary_val->domain_name2,
                      (kMaxLenDomainId+1));
      }
    }
    if (vlink_ctrlr_dom[1].ctrlr &&
        !strcmp(reinterpret_cast<char*>(vlink_ctrlr_dom[1].ctrlr), "#")) {
      if (boundary_val->logical_port_id2[0] == 'M' &&
          boundary_val->logical_port_id2[1] == 'G') {
        uuu::upll_strncpy(vlink_ctrlr_dom[1].ctrlr,
                          boundary_val->controller_name2,
                          (kMaxLenCtrlrId+1));
        uuu::upll_strncpy(vlink_ctrlr_dom[1].domain, boundary_val->domain_name2,
                      (kMaxLenDomainId+1));
      } else {
        uuu::upll_strncpy(vlink_ctrlr_dom[1].ctrlr,
                          boundary_val->controller_name1,
                          (kMaxLenCtrlrId+1));
        uuu::upll_strncpy(vlink_ctrlr_dom[1].domain, boundary_val->domain_name1,
                      (kMaxLenDomainId+1));
      }
    }
    DELETE_IF_NOT_NULL(ck_boundary);
  }
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
