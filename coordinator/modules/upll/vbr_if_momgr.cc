/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <ctype.h>
#include <sstream>
#include "vbr_if_momgr.hh"
#include "vbr_momgr.hh"
#include "vtn_momgr.hh"
#include "vlink_momgr.hh"
#include "vterm_if_momgr.hh"

#define NUM_KEY_MAIN_TBL_ 6
#define MAX_VNODE_CHILD_STRING_LEN 32
#define VLINK_VNODE1 0x80
#define VLINK_VNODE2 0x40

using unc::upll::ipc_util::IpcUtil;

namespace unc {
namespace upll {
namespace kt_momgr {


#define UPLL_VLAN_UNTAGGED 0
#define UPLL_VLAN_TAGGED 1

BindInfo VbrIfMoMgr::vbr_if_bind_info[] = {
    { uudst::vbridge_interface::kDbiVtnName, CFG_KEY, offsetof(
        key_vbr_if, vbr_key.vtn_key.vtn_name),
      uud::kDalChar, 32 },
    { uudst::vbridge_interface::kDbiVbrName, CFG_KEY, offsetof(
        key_vbr_if, vbr_key.vbridge_name),
      uud::kDalChar, 32 },
    { uudst::vbridge_interface::kDbiIfName, CFG_KEY, offsetof(key_vbr_if,
                                                              if_name),
      uud::kDalChar, 32 },
    { uudst::vbridge_interface::kDbiAdminStatus, CFG_VAL, offsetof(
        val_vbr_if, admin_status),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiDesc, CFG_VAL, offsetof(val_vbr_if,
                                                            description),
      uud::kDalChar, 128 },
    { uudst::vbridge_interface::kDbiLogicalPortId, CFG_VAL, offsetof(
        val_vbr_if, portmap.logical_port_id),
      uud::kDalChar, 320 },
    { uudst::vbridge_interface::kDbiVlanId, CFG_VAL, offsetof(val_vbr_if,
                                                              portmap.vlan_id),
      uud::kDalUint16, 1 },
    { uudst::vbridge_interface::kDbiTagged, CFG_VAL, offsetof(val_vbr_if,
                                                              portmap.tagged),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiVexName, CFG_VAL, offsetof(val_drv_vbr_if,
                                                               vex_name),
      uud::kDalChar, 32 },
    { uudst::vbridge_interface::kDbiVexIfName, CFG_VAL, offsetof(val_drv_vbr_if,
                                                                 vex_if_name),
      uud::kDalChar, 32 },
    { uudst::vbridge_interface::kDbiVexLinkName, CFG_VAL, offsetof(
        val_drv_vbr_if, vex_link_name),
      uud::kDalChar, 32 },
    { uudst::vbridge_interface::kDbiOperStatus, ST_VAL, offsetof(
        val_db_vbr_if_st, vbr_if_val_st.oper_status),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiDownCount, ST_VAL, offsetof(
        val_db_vbr_if_st, down_count),
      uud::kDalUint32, 1 },
    { uudst::vbridge_interface::kDbiCtrlrName, CK_VAL, offsetof(
        key_user_data_t, ctrlr_id), uud::kDalChar, 32 },
    { uudst::vbridge_interface::kDbiDomainId, CK_VAL, offsetof(
        key_user_data_t, domain_id), uud::kDalChar, 32 },
    { uudst::vbridge_interface::kDbiFlags, CK_VAL, offsetof(
        key_user_data_t, flags),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiValidAdminStatus, CFG_DEF_VAL, offsetof(
        val_vbr_if, valid[UPLL_IDX_ADMIN_STATUS_VBRI]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiValidDesc, CFG_META_VAL, offsetof(
        val_vbr_if, valid[UPLL_IDX_DESC_VBRI]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiValidPortMap, CFG_META_VAL, offsetof(
        val_vbr_if, valid[UPLL_IDX_PM_VBRI]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiValidLogicalPortId, CFG_META_VAL, offsetof(
        val_vbr_if, portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiValidVlanid, CFG_META_VAL, offsetof(
        val_vbr_if, portmap.valid[UPLL_IDX_VLAN_ID_PM]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiValidTagged, CFG_META_VAL, offsetof(
        val_vbr_if, portmap.valid[UPLL_IDX_TAGGED_PM]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiValidVexName, CFG_META_VAL, offsetof(
        val_drv_vbr_if, valid[PFCDRV_IDX_VEXT_NAME_VBRIF]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiValidVexIfName, CFG_META_VAL, offsetof(
        val_drv_vbr_if, valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiValidVexLinkName, CFG_META_VAL, offsetof(
        val_drv_vbr_if, valid[PFCDRV_IDX_VLINK_NAME_VBRIF]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiValidOperStatus, ST_META_VAL, offsetof(
        val_db_vbr_if_st, vbr_if_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiCsAdminStatus, CS_VAL, offsetof(
        val_vbr_if, cs_attr[UPLL_IDX_ADMIN_STATUS_VBRI]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiCsDesc, CS_VAL, offsetof(
        val_vbr_if, cs_attr[UPLL_IDX_DESC_VBRI]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiCsPortMap, CS_VAL, offsetof(
        val_vbr_if, cs_attr[UPLL_IDX_PM_VBRI]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiCsLogicalPortId, CS_VAL, offsetof(
        val_vbr_if, portmap.cs_attr[UPLL_IDX_LOGICAL_PORT_ID_PM]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiCsVlanid, CS_VAL, offsetof(
        val_vbr_if, portmap.cs_attr[UPLL_IDX_VLAN_ID_PM]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiCsTagged, CS_VAL, offsetof(
        val_vbr_if, portmap.cs_attr[UPLL_IDX_TAGGED_PM]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiCsRowstatus, CS_VAL, offsetof(
        val_vbr_if, cs_row_status),
      uud::kDalUint8, 1 } };

BindInfo VbrIfMoMgr::key_vbr_if_maintbl_bind_info[] = {
    { uudst::vbridge_interface::kDbiVtnName, CFG_MATCH_KEY, offsetof(
        key_vbr_if_t, vbr_key.vtn_key.vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vbridge_interface::kDbiVbrName, CFG_MATCH_KEY, offsetof(
        key_vbr_if_t, vbr_key.vbridge_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vbridge_interface::kDbiIfName, CFG_MATCH_KEY, offsetof(
        key_vbr_if_t, if_name),
      uud::kDalChar, kMaxLenInterfaceName + 1 },
    { uudst::vbridge_interface::kDbiVtnName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vtn_name),
      uud::kDalChar, kMaxLenVtnName + 1 },
    { uudst::vbridge_interface::kDbiVbrName, CFG_INPUT_KEY, offsetof(
        key_rename_vnode_info_t, new_unc_vnode_name),
      uud::kDalChar, kMaxLenVnodeName + 1 },
    { uudst::vbridge_interface::kDbiFlags, CK_VAL, offsetof(
        key_user_data_t, flags),
      uud::kDalUint8, 1 } };

BindInfo VbrIfMoMgr::convert_vbr_if_bind_info[] = {
         { uudst::convert_vbridge_interface::kDbiVtnName, CFG_KEY,
           offsetof(key_convert_vbr_if,
                    convert_vbr_key.vbr_key.vtn_key.vtn_name),
           uud::kDalChar, kMaxLenVtnName + 1 },
         { uudst::convert_vbridge_interface::kDbiVbrName, CFG_KEY,
           offsetof(key_convert_vbr_if, convert_vbr_key.vbr_key.vbridge_name),
           uud::kDalChar, kMaxLenVnodeName + 1 },
         { uudst::convert_vbridge_interface::kDbiConvertVbrName, CFG_KEY,
           offsetof(key_convert_vbr_if, convert_vbr_key.conv_vbr_name),
           uud::kDalChar, kMaxLenConvertVnodeName + 1 },
         { uudst::convert_vbridge_interface::kDbiConvertIfName, CFG_KEY,
           offsetof(key_convert_vbr_if, convert_if_name),
           uud::kDalChar, kMaxLenInterfaceName + 1 },
         { uudst::convert_vbridge_interface::kDbiOperStatus, ST_VAL,
           offsetof(val_db_vbr_if_st, vbr_if_val_st.oper_status),
           uud::kDalUint8, 1 },
         { uudst::convert_vbridge_interface::kDbiDownCount, ST_VAL,
           offsetof(val_db_vbr_if_st, down_count),
           uud::kDalUint32, 1 },
         { uudst::convert_vbridge_interface::kDbiCtrlrName, CK_VAL,
           offsetof(key_user_data_t, ctrlr_id),
           uud::kDalChar, kMaxLenCtrlrId + 1 },
         { uudst::convert_vbridge_interface::kDbiDomainId, CK_VAL,
           offsetof(key_user_data_t, domain_id), uud::kDalChar,
           kMaxLenDomainId + 1 },
         { uudst::convert_vbridge_interface::kDbiFlags, CK_VAL,
           offsetof(key_user_data_t, flags), uud::kDalUint8, 1 },
         { uudst::convert_vbridge_interface::kDbiValidOperStatus, ST_META_VAL,
           offsetof(val_db_vbr_if_st,
                    vbr_if_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS]),
           uud::kDalUint8, 1 },
         { uudst::convert_vbridge_interface::kDbiCsRowStatus,
           CS_VAL, offsetof(val_convert_vbr_if,
                            cs_row_status),
           uud::kDalUint8, 1 }};

unc_key_type_t VbrIfMoMgr::vbr_if_child[] = { UNC_KT_VBRIF_FLOWFILTER,
                                              UNC_KT_VBRIF_POLICINGMAP };


VbrIfMoMgr::VbrIfMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  table[MAINTBL] = new Table(
      uudst::kDbiVbrIfTbl, UNC_KT_VBR_IF, vbr_if_bind_info,
      IpctSt::kIpcStKeyVbrIf, IpctSt::kIpcStValVbrIf,
      uudst::vbridge_interface::kDbiVbrIfNumCols);
  table[CTRLRTBL] = NULL;
  table[RENAMETBL] = NULL;
  //  Added for vb_if converttbl
  table[CONVERTTBL] = new Table(
      uudst::kDbiConvertVbrIfTbl, UNC_KT_VBR_IF, convert_vbr_if_bind_info,
      IpctSt::kIpcStKeyConvertVbrIf, IpctSt::kIpcStValConvertVbrIf,
      uudst::convert_vbridge_interface::kDbiConvertVbrIfNumCols);
  nchild = sizeof(vbr_if_child) / sizeof(*vbr_if_child);
  child = vbr_if_child;
}
/*
 * Based on the key type the bind info will pass
 **/

bool VbrIfMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                                      int &nattr, MoMgrTables tbl) {
  /* Main Table or rename table only update */
  if (MAINTBL == tbl) {
    nattr = NUM_KEY_MAIN_TBL_;
    binfo = key_vbr_if_maintbl_bind_info;
  } else {
    UPLL_LOG_DEBUG("Invalid Table ");
    return PFC_FALSE;
  }
  return PFC_TRUE;
}


upll_rc_t VbrIfMoMgr::UpdateMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if (NULL == ikey || NULL == req || !(ikey->get_key())) {
     UPLL_LOG_ERROR("Given Input is Empty");
     return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("UpdateMo for %d", ikey->get_key_type());
  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Validation Message is Failed ");
      return result_code;
  }
#if 0
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  ConfigKeyVal *temp_ck = NULL;
  result_code = GetParentConfigKey(temp_ck, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Error in retrieving the Parent ConfigKeyVal");
    delete temp_ck;
    return result_code;
  }
  result_code = GetControllerDomainId(temp_ck, req->datatype, &ctrlr_dom, dmi);
  UPLL_LOG_INFO("GetControllerDomainId result code is: %d", result_code);
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    UPLL_LOG_INFO("Invalid ctrlr/domain");
    return UPLL_RC_ERR_GENERIC;
  }
#else
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  ConfigKeyVal *temp_ck = NULL;
  result_code = GetChildConfigKey(temp_ck, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Error in retrieving the Child ConfigKeyVal");
    delete temp_ck;
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain  };
  result_code = ReadConfigDB(temp_ck, req->datatype, UNC_OP_READ,
                                  dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code);
    delete temp_ck;
    return result_code;
  }

   /* Some of the validations of ValidateVbrIfValue method moved to here
    * because, we require readconfigDB to distingwish whether VLAN_ID is
    * already set or not from DB, which degreates performence.
    */

  val_vbr_if *val_if_ikey = reinterpret_cast<val_vbr_if_t *>(GetVal(ikey));
  val_vbr_if *val_if_temp_ck = reinterpret_cast<val_vbr_if_t *>
                                                      (GetVal(temp_ck));

  if (val_if_ikey->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
      /* When tagged attr is VALID_NO_VALUE , then default value(TAGGED) has to
       * be set based on VLAN_ID flag.
       */
    if (val_if_ikey->portmap.valid[UPLL_IDX_TAGGED_PM] ==
         UNC_VF_VALID_NO_VALUE) {
      if ((val_if_temp_ck->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
          UNC_VF_VALID) && (!(val_if_ikey->portmap.valid[UPLL_IDX_VLAN_ID_PM]
                               ==  UNC_VF_VALID_NO_VALUE))) {
        val_if_ikey->portmap.tagged = UPLL_VLAN_TAGGED;
        val_if_ikey->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      } else if (val_if_ikey->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
                 UNC_VF_VALID) {
        val_if_ikey->portmap.tagged = UPLL_VLAN_TAGGED;
        val_if_ikey->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      }
        /* When tagged attr is VALID, check for VLAN_ID flag.
         * If it is INVALID in both DB and current req then remove Tagged attr,
         * else retain the value
         */
    } else if (val_if_ikey->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID) {
      if ((val_if_temp_ck->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
           UNC_VF_INVALID) &&
          (val_if_ikey->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_INVALID)) {
        val_if_ikey->portmap.tagged = 0;
        val_if_ikey->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
      }
        /* When tagged attr is INVALID , check for VLAN_ID flag
         * If it doesn't exists in DB and VALID in current req, then set tagged
         * to Default value (TAGGED)
         */
    } else if (val_if_ikey->portmap.valid[UPLL_IDX_TAGGED_PM] ==
                                                           UNC_VF_INVALID) {
      if ((val_if_temp_ck->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
           UNC_VF_INVALID) &&
          (val_if_ikey->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID)) {
        val_if_ikey->portmap.tagged = UPLL_VLAN_TAGGED;
        val_if_ikey->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      }
    }
  }

  // To validate all the flags are INVALID during Update
  bool is_invalid = IsAllAttrInvalid(reinterpret_cast<val_vbr_if *>
                                    (GetVal(ikey)));
  if (is_invalid) {
    UPLL_LOG_INFO("No attributes to be updated");
    DELETE_IF_NOT_NULL(temp_ck);
    return UPLL_RC_SUCCESS;
  }
  GET_USER_DATA_CTRLR_DOMAIN(temp_ck, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
#endif
  if (!IsUnifiedVbr(ctrlr_dom.ctrlr)) {
    result_code = ValidateCapability(
        req, ikey, reinterpret_cast<char *>(ctrlr_dom.ctrlr));
    if (UPLL_RC_SUCCESS  != result_code) {
      UPLL_LOG_DEBUG("Validate Capability is Failed. Error_code : %d",
                     result_code);
      delete temp_ck;
      return result_code;
    }
  }
#if 0
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
      UPLL_LOG_ERROR("Record does Not Exists");
      return result_code;
  }
#endif
  delete temp_ck;
  result_code = DupConfigKeyVal(okey, ikey, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" DupConfigKeyVal Failed %d", result_code);
    return result_code;
  }

  val_drv_vbr_if *valif = reinterpret_cast<val_drv_vbr_if *>
      (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
  void *ifval = GetVal(ikey);
  memcpy(&(valif->vbr_if_val), ifval, sizeof(val_vbr_if));
  (okey->get_cfg_val())->SetVal(IpctSt::kIpcStPfcdrvValVbrIf, valif);

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  result_code = ValidateAttribute(okey, dmi, req);
  if (UPLL_RC_SUCCESS  != result_code) {
      delete okey;
      UPLL_LOG_ERROR("Validate Attribute is Failed");
      return result_code;
  }

  if (req->datatype == UPLL_DT_CANDIDATE && config_mode == TC_CONFIG_VTN) {
    req->datatype = UPLL_DT_RUNNING;
    result_code = ValidateAttribute(okey, dmi, req);
    if (UPLL_RC_SUCCESS  != result_code) {
      delete okey;
      UPLL_LOG_ERROR("Validate Attribute is Failed");
      req->datatype = UPLL_DT_CANDIDATE;
      return result_code;
    }
    req->datatype = UPLL_DT_CANDIDATE;
  }

  result_code = UpdateConfigVal(okey, req->datatype, dmi, config_mode,
                                vtn_name);
  if (UPLL_RC_SUCCESS != result_code) {
      delete okey;
      UPLL_LOG_DEBUG("UpdateConfigVal is Failed");
      return result_code;
  }
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  UPLL_LOG_DEBUG("The okey Structue before update  %s",
                 (okey->ToStrAll()).c_str());
  result_code = UpdateConfigDB(okey, req->datatype, UNC_OP_UPDATE,
                               dmi, &dbop1, config_mode,
                               vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    delete okey;
    UPLL_LOG_ERROR("Updation Failure in DB : %d", result_code);
    return result_code;
  }
  delete okey;
  return result_code;
}

upll_rc_t VbrIfMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                        const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = VnodeChildMoMgr::CreateAuditMoImpl(ikey, dmi, ctrlr_id);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Create Audit Vbrif failed %s", (ikey->ToStrAll()).c_str());
  }
  return result_code;
}

upll_rc_t VbrIfMoMgr::GetVbrIfValfromDB(ConfigKeyVal *ikey,
                                        ConfigKeyVal *&ck_drv_vbr_if,
                                        upll_keytype_datatype_t dt_type,
                                        DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetChildConfigKey(ck_drv_vbr_if, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Get Vbrifkey failed");
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(ck_drv_vbr_if, dt_type, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  return result_code;
}

upll_rc_t VbrIfMoMgr::updateVbrIf(IpcReqRespHeader *req,
                                  ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *ck_drv_vbr_if = NULL;
#if 0
  result_code = GetVbrIfValfromDB(ikey, ck_drv_vbr_if, UPLL_DT_RUNNING, dmi);
#endif

  key_vbr_if *temp_vbr_if_key = reinterpret_cast<key_vbr_if *>(ikey->get_key());

  key_vbr_if *vbr_if_key = reinterpret_cast<key_vbr_if *>(ConfigKeyVal::Malloc(
      sizeof(key_vbr_if)));
  uuu::upll_strncpy(vbr_if_key->if_name, temp_vbr_if_key->if_name,
                   (kMaxLenInterfaceName + 1));
  uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                    temp_vbr_if_key->vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
  uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                    temp_vbr_if_key->vbr_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));

  val_drv_vbr_if *vbr_drv_if_val =
      reinterpret_cast<val_drv_vbr_if *>
     (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
  ConfigVal *cv_drv_vbr_if = new ConfigVal(IpctSt::kIpcStValVbrIf,
                                           vbr_drv_if_val);
  ck_drv_vbr_if = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                   vbr_if_key, cv_drv_vbr_if);

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(ck_drv_vbr_if, UPLL_DT_RUNNING, UNC_OP_READ, dbop,
                             dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    delete ck_drv_vbr_if;
    UPLL_LOG_INFO("Retrieving a record for VbrIf in RUNNING DB failed");
    return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  result_code = UpdateConfigDB(ck_drv_vbr_if, UPLL_DT_AUDIT, UNC_OP_CREATE, dmi,
                               config_mode, vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    delete ck_drv_vbr_if;
    UPLL_LOG_INFO("Creating a VbrIf record in AUDIT DB failed");
    return result_code;
  }
  if (ck_drv_vbr_if)
    delete ck_drv_vbr_if;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::ConverttoDriverPortMap(ConfigKeyVal *ck_port_map,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ckv_rvbrif = NULL;
  upll_rc_t result_code = GetChildConfigKey(ckv_rvbrif, ck_port_map);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetChilConfigKey Failed");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag};
  result_code = ReadConfigDB(ckv_rvbrif, UPLL_DT_RUNNING,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  bool port_map_in_run = false;
  val_drv_vbr_if *drv_vbr_if_val = reinterpret_cast<val_drv_vbr_if *>
                                                (GetVal(ck_port_map));
  if (result_code == UPLL_RC_SUCCESS) {
    val_drv_vbr_if *drv_rifval =
        reinterpret_cast<val_drv_vbr_if *>(GetVal(ckv_rvbrif));
    if (drv_rifval == NULL) {
       UPLL_LOG_ERROR("val vbr is NULL");
       DELETE_IF_NOT_NULL(ckv_rvbrif);
       return UPLL_RC_ERR_GENERIC;
    }
    if (drv_rifval->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
      port_map_in_run = true;
      drv_vbr_if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
      drv_vbr_if_val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
      uuu::upll_strncpy(drv_vbr_if_val->vex_name, drv_rifval->vex_name,
                        kMaxLenVnodeName+1);
      drv_vbr_if_val->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID;
      uuu::upll_strncpy(reinterpret_cast<char *>(drv_vbr_if_val->vex_if_name),
                        drv_rifval->vex_if_name, kMaxLenInterfaceName+1);
      drv_vbr_if_val->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID;
      uuu::upll_strncpy(reinterpret_cast<char *>(drv_vbr_if_val->vex_link_name),
              drv_rifval->vex_link_name, kMaxLenVlinkName+1);
    }
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_ERROR("ReadConfigDB failure %d", result_code);
    DELETE_IF_NOT_NULL(ckv_rvbrif);
    return result_code;
  }
  DELETE_IF_NOT_NULL(ckv_rvbrif);
  if (!port_map_in_run || UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    std::string if_name = reinterpret_cast<const char *>(
                reinterpret_cast<key_vbr_if*>(ck_port_map->get_key())->if_name);
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

      // Check if constructed vexternal is unique or not.
      key_vbr *vbr_key = ConfigKeyVal::Malloc<key_vbr>();
      uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                        reinterpret_cast<key_vbr_if *>
                        (ck_port_map->get_key())->vbr_key.vtn_key.vtn_name,
                        (kMaxLenVtnName+1));
      uuu::upll_strncpy(vbr_key->vbridge_name,
                        vex_name.c_str(), (kMaxLenVnodeName+1));

      ConfigKeyVal *vex_ckv = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                               IpctSt::kIpcStKeyVbr,
                                               vbr_key, NULL);
      controller_domain ctrlr_dom;
      memset(&ctrlr_dom, 0, sizeof(controller_domain));
      GET_USER_DATA_CTRLR_DOMAIN(ck_port_map, ctrlr_dom);
      SET_USER_DATA_CTRLR_DOMAIN(vex_ckv, ctrlr_dom);
      VbrMoMgr *vbr_mgr = reinterpret_cast<VbrMoMgr *>
          (const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));
      // Auto generated vexternal name of vBridge interface, should not be same
      // of other vnodes and other vexternal's of the same controller
      result_code = vbr_mgr->VnodeChecks(vex_ckv, UPLL_DT_CANDIDATE, dmi, true);
      DELETE_IF_NOT_NULL(vex_ckv);
      if (result_code == UPLL_RC_ERR_CFG_SEMANTIC ||
          result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("Another Vnode %s already exists.  Error code : %d",
                       vex_name.c_str(), result_code);
        continue;  // retry and generate unique name
      } else if (result_code == UPLL_RC_SUCCESS) {
        drv_vbr_if_val->valid[PFCDRV_IDX_VAL_VBRIF] = UNC_VF_VALID;
        drv_vbr_if_val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
        uuu::upll_strncpy(drv_vbr_if_val->vex_name, vex_name.c_str(),
                          (kMaxLenVnodeName+1));

        drv_vbr_if_val->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID;
        uuu::upll_strncpy(drv_vbr_if_val->vex_if_name,  vex_if_name.c_str(),
                          (kMaxLenVnodeName + 1));

        drv_vbr_if_val->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID;
        uuu::upll_strncpy(drv_vbr_if_val->vex_link_name, vex_link_name.c_str(),
                          (kMaxLenVnodeName + 1));
      } else {
        UPLL_LOG_DEBUG("Vnodchecks failed. %d", result_code);
        return result_code;
      }
      break;
    }
  }
  return UPLL_RC_SUCCESS;
}

bool VbrIfMoMgr::IsValidKey(void *key, uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  //  For converttbl key structure is key_convert_vbr,
  //  other tbls key structure is key_vbr
  if (tbl == CONVERTTBL) {
    key_convert_vbr_if *conv_if_key =
        reinterpret_cast<key_convert_vbr_if *>(key);
    switch (index) {
      case uudst::convert_vbridge_interface::kDbiVtnName:
        //  vtn name
        ret_val = ValidateKey(
            reinterpret_cast<char *>(
                conv_if_key->convert_vbr_key.vbr_key.vtn_key.vtn_name),
            kMinLenVtnName, kMaxLenVtnName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::convert_vbridge_interface::kDbiVbrName:
        //  unified vBridge name
        ret_val = ValidateKey(
            reinterpret_cast<char *>(
                conv_if_key->convert_vbr_key.vbr_key.vbridge_name),
            kMinLenVnodeName, kMaxLenVnodeName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Unified VBR Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::convert_vbridge_interface::kDbiConvertVbrName:
        //  converted vBridge name
        ret_val = ValidateKey(reinterpret_cast<char *>(
                conv_if_key->convert_vbr_key.conv_vbr_name),
            kMinLenConvertVnodeName, kMaxLenConvertVnodeName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Convert VBR Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::convert_vbridge_interface::kDbiConvertIfName:
        //  converted vBridge interface name
        ret_val = ValidateKey(reinterpret_cast<char *>(
                conv_if_key->convert_if_name),
                kMinLenInterfaceName, kMaxLenInterfaceName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VBR IF Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      default:
        UPLL_LOG_INFO("Wrong Index");
        break;
    }
  } else {
    key_vbr_if *if_key = reinterpret_cast<key_vbr_if *>(key);
    switch (index) {
      case uudst::vbridge_interface::kDbiVtnName:
        ret_val = ValidateKey(
            reinterpret_cast<char *>(if_key->vbr_key.vtn_key.vtn_name),
            kMinLenVtnName, kMaxLenVtnName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VTN Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::vbridge_interface::kDbiVbrName:
        ret_val = ValidateKey(
            reinterpret_cast<char *>(if_key->vbr_key.vbridge_name),
            kMinLenVnodeName, kMaxLenVnodeName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VBR Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      case uudst::vbridge_interface::kDbiIfName:
        ret_val = ValidateKey(reinterpret_cast<char *>(if_key->if_name),
                              kMinLenInterfaceName, kMaxLenInterfaceName);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("VBR IF Name is not valid(%d)", ret_val);
          return false;
        }
        break;
      default:
        UPLL_LOG_INFO("Wrong Index");
        break;
    }
  }
  return true;
}

upll_rc_t VbrIfMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  bool cfgval_ctrlr = false;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_if *vbr_key_if;
  void *pkey;
  if (parent_key == NULL) {
    vbr_key_if = reinterpret_cast<key_vbr_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, vbr_key_if,
                            NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VBR_IF)
      return UPLL_RC_ERR_GENERIC;
    vbr_key_if = reinterpret_cast<key_vbr_if *>(okey->get_key());
  } else {
    vbr_key_if = reinterpret_cast<key_vbr_if *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if)));
  }
  unc_key_type_t keytype = parent_key->get_key_type();
  switch (keytype) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(vbr_key_if->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn *>(pkey)->vtn_name,
                        (kMaxLenVtnName + 1));
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(vbr_key_if->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      uuu::upll_strncpy(vbr_key_if->vbr_key.vbridge_name,
                        reinterpret_cast<key_vbr *>(pkey)->vbridge_name,
                        (kMaxLenVnodeName + 1));
      break;
    case UNC_KT_VBR_IF:
      if (parent_key->get_st_num() == IpctSt::kIpcStKeyVbrIf) {
        uuu::upll_strncpy(
          vbr_key_if->vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_vbr_if *>(pkey)->vbr_key.vtn_key.vtn_name,
          (kMaxLenVtnName + 1));
        uuu::upll_strncpy(
          vbr_key_if->vbr_key.vbridge_name,
          reinterpret_cast<key_vbr_if *>(pkey)->vbr_key.vbridge_name,
          (kMaxLenVnodeName + 1));
        uuu::upll_strncpy(vbr_key_if->if_name,
                        reinterpret_cast<key_vbr_if *>(pkey)->if_name,
                        (kMaxLenInterfaceName + 1));
      } else {
        uuu::upll_strncpy(
          vbr_key_if->vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_convert_vbr_if *>(pkey)->
          convert_vbr_key.vbr_key.vtn_key.vtn_name, (kMaxLenVtnName + 1));
        uuu::upll_strncpy(
          vbr_key_if->vbr_key.vbridge_name,
          reinterpret_cast<key_convert_vbr_if*>(pkey)->
          convert_vbr_key.vbr_key.vbridge_name, (kMaxLenVnodeName + 1));
        uuu::upll_strncpy(vbr_key_if->if_name,
                        reinterpret_cast<key_convert_vbr_if *>(pkey)->
          convert_if_name, (kMaxLenInterfaceName + 1));
      }
      break;
    case UNC_KT_VLINK: {
      uint8_t *vnode_name, *if_name;
      uint8_t flags = 0;
      val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(parent_key));
      if (!vlink_val) {
        if (!okey || !(okey->get_key()))
          free(vbr_key_if);
        return UPLL_RC_ERR_GENERIC;
      }
      GET_USER_DATA_FLAGS(parent_key->get_cfg_val(), flags);
      flags &=  VLINK_FLAG_NODE_POS;
      UPLL_LOG_DEBUG("Vlink flag node position %d", flags);
      if (flags == kVlinkVnode2) {
        cfgval_ctrlr = true;
        vnode_name = vlink_val->vnode2_name;
        if_name = vlink_val->vnode2_ifname;
      } else {
        vnode_name = vlink_val->vnode1_name;
        if_name = vlink_val->vnode1_ifname;
      }
      uuu::upll_strncpy(vbr_key_if->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
                        (kMaxLenVtnName + 1));
      if (vnode_name)
        uuu::upll_strncpy(vbr_key_if->vbr_key.vbridge_name, vnode_name,
                          (kMaxLenVnodeName + 1));
      if (if_name)
        uuu::upll_strncpy(vbr_key_if->if_name, if_name,
                          (kMaxLenInterfaceName + 1));
    }
    break;
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, vbr_key_if,
                            NULL);
  else if (okey->get_key() != vbr_key_if)
    okey->SetKey(IpctSt::kIpcStKeyVbrIf, vbr_key_if);

  if (okey == NULL) {
    free(vbr_key_if);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    if (cfgval_ctrlr) {
      SET_USER_DATA(okey, parent_key->get_cfg_val());
    } else {
      SET_USER_DATA(okey, parent_key);
    }
  }
  return result_code;
}

upll_rc_t VbrIfMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                         ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey)
    return UPLL_RC_ERR_GENERIC;
  unc_key_type_t ikey_type = ikey->get_key_type();

  if (ikey_type != UNC_KT_VBR_IF) return UPLL_RC_ERR_GENERIC;
  if (ikey->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
    key_convert_vbr_if *pkey = reinterpret_cast<key_convert_vbr_if*>
      (ikey->get_key());
    if (!pkey) return UPLL_RC_ERR_GENERIC;
    key_convert_vbr_t *conv_vbr_key = reinterpret_cast<key_convert_vbr_t*>
           (ConfigKeyVal::Malloc(sizeof(key_convert_vbr)));
    uuu::upll_strncpy(conv_vbr_key->vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_convert_vbr_if *>
          (pkey)->convert_vbr_key.vbr_key.vtn_key.vtn_name,
          (kMaxLenVtnName+1));
    uuu::upll_strncpy(conv_vbr_key->vbr_key.vbridge_name,
          reinterpret_cast<key_convert_vbr_if *>
          (pkey)->convert_vbr_key.vbr_key.vbridge_name,
          (kMaxLenVnodeName+1));
    uuu::upll_strncpy(conv_vbr_key->conv_vbr_name,
          reinterpret_cast<key_convert_vbr_if *>
          (pkey)->convert_vbr_key.conv_vbr_name,
          (kMaxLenVnodeName+1));
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyConvertVbr,
                            conv_vbr_key, NULL);
    if (okey == NULL) {
      free(conv_vbr_key);
      result_code = UPLL_RC_ERR_GENERIC;
    } else {
      SET_USER_DATA(okey, ikey);
    }
  } else {
    key_vbr_if *pkey = reinterpret_cast<key_vbr_if *>
        (ikey->get_key());
    if (!pkey) return UPLL_RC_ERR_GENERIC;
    key_vbr *vbr_key = reinterpret_cast<key_vbr *>(ConfigKeyVal::Malloc
                   (sizeof(key_vbr)));
    uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
            reinterpret_cast<key_vbr_if *>(pkey)->vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName+1));
    uuu::upll_strncpy(vbr_key->vbridge_name,
            reinterpret_cast<key_vbr_if *>(pkey)->vbr_key.vbridge_name,
            (kMaxLenVnodeName+1));
    if (okey) delete okey;
    okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr,
                            vbr_key, NULL);
    if (okey == NULL) {
      free(vbr_key);
      result_code = UPLL_RC_ERR_GENERIC;
    } else {
      SET_USER_DATA(okey, ikey);
    }
  }
  return result_code;
}

upll_rc_t VbrIfMoMgr::AllocVal(ConfigVal *&ck_val,
                               upll_keytype_datatype_t dt_type,
                               MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val;

  if (ck_val != NULL) return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
      val = reinterpret_cast<void *>(ConfigKeyVal::Malloc(
                    sizeof(val_drv_vbr_if)));
      ck_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>(ConfigKeyVal::Malloc(
               sizeof(val_db_vbr_if_st)));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVbrIfSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    case CONVERTTBL:
      val = reinterpret_cast<void *>(ConfigKeyVal::Malloc(
                    sizeof(val_convert_vbr_if)));
      ck_val = new ConfigVal(IpctSt::kIpcStValConvertVbrIf, val);
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>(ConfigKeyVal::Malloc(
               sizeof(val_db_vbr_if_st)));
        ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVbrIfSt, val);
        ck_val->AppendCfgVal(ck_nxtval);
      }
      break;
    default:
      val = NULL;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                                      MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VBR_IF)
     return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      void *oval = NULL;
      if ((req->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVbrIf) {
        val_vbr_if *ival = reinterpret_cast<val_vbr_if *>(GetVal(req));
        if (!ival) return UPLL_RC_ERR_GENERIC;
        val_vbr_if *vbr_val_if = reinterpret_cast<val_vbr_if *>(
            ConfigKeyVal::Malloc(sizeof(val_vbr_if)));
        memcpy(vbr_val_if, ival, sizeof(val_vbr_if));
        oval = reinterpret_cast<void *>(vbr_val_if);
      } else {
        val_drv_vbr_if *ival = reinterpret_cast<val_drv_vbr_if *>(GetVal(req));
        if (!ival) return UPLL_RC_ERR_GENERIC;
        val_drv_vbr_if *vbr_val_if = reinterpret_cast<val_drv_vbr_if *>(
            ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
        memcpy(vbr_val_if, ival, sizeof(val_drv_vbr_if));
        oval = reinterpret_cast<void *>(vbr_val_if);
      }
      tmp1 = new ConfigVal(req->get_cfg_val()->get_st_num(), oval);
    } else if (tbl == CONVERTTBL) {
      void *oval = NULL;
      val_convert_vbr_if *ival =
          reinterpret_cast<val_convert_vbr_if_t *>(GetVal(req));
      if (!ival) return UPLL_RC_ERR_GENERIC;
      val_convert_vbr_if_t *vbr_val_if =
          reinterpret_cast<val_convert_vbr_if_t *>(ConfigKeyVal::
          Malloc(sizeof(val_convert_vbr_if_t)));
      memcpy(vbr_val_if, ival, sizeof(val_convert_vbr_if_t));
      oval = reinterpret_cast<void *>(vbr_val_if);
      tmp1 = new ConfigVal(req->get_cfg_val()->get_st_num(), oval);
    }
    tmp = tmp->get_next_cfg_val();
  }
  if (tmp) {
    if (tbl == MAINTBL || tbl == CONVERTTBL) {
      void *ovalst = NULL;
      val_db_vbr_if_st *ival =
            reinterpret_cast<val_db_vbr_if_st *>(tmp->get_val());
      if (ival == NULL) {
        DELETE_IF_NOT_NULL(tmp1);
        return UPLL_RC_ERR_GENERIC;
      }
      val_db_vbr_if_st *val_vbr_if =
            reinterpret_cast<val_db_vbr_if_st *>(ConfigKeyVal::Malloc(
                sizeof(val_db_vbr_if_st)));
      memcpy(val_vbr_if, ival, sizeof(val_db_vbr_if_st));
      ovalst = reinterpret_cast<void *>(val_vbr_if);
      ConfigVal *tmp2 = new ConfigVal(
        req->get_cfg_val()->get_next_cfg_val()->get_st_num(), ovalst);
      if (tmp1)
        tmp1->AppendCfgVal(tmp2);
    }
  }
  void *tkey = (req)->get_key();
  if (tbl == CONVERTTBL) {
    key_convert_vbr_if *ikey = reinterpret_cast<key_convert_vbr_if *>(tkey);
    key_convert_vbr_if *vbr_if_key = reinterpret_cast<key_convert_vbr_if_t *>
                        (ConfigKeyVal::Malloc(sizeof(key_convert_vbr_if_t)));
    if (!vbr_if_key) {
      UPLL_LOG_DEBUG(" Memory allocation failed");
      DELETE_IF_NOT_NULL(tmp1);
      return UPLL_RC_ERR_GENERIC;
    }
    memcpy(vbr_if_key, ikey, sizeof(key_convert_vbr_if_t));
    okey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyConvertVbrIf,
                            vbr_if_key, tmp1);
    if (!okey) {
      DELETE_IF_NOT_NULL(tmp1);
      FREE_IF_NOT_NULL(vbr_if_key);
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    key_vbr_if *ikey = reinterpret_cast<key_vbr_if *>(tkey);
    key_vbr_if *vbr_if_key =
      reinterpret_cast<key_vbr_if *>(ConfigKeyVal::Malloc(sizeof(key_vbr_if)));
    if (!vbr_if_key) {
      UPLL_LOG_DEBUG(" Memory allocation failed");
      DELETE_IF_NOT_NULL(tmp1);
      return UPLL_RC_ERR_GENERIC;
    }
    memcpy(vbr_if_key, ikey, sizeof(key_vbr_if));
    okey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, vbr_if_key,
                            tmp1);
    if (!okey) {
      DELETE_IF_NOT_NULL(tmp1);
      FREE_IF_NOT_NULL(vbr_if_key);
      return UPLL_RC_ERR_GENERIC;
    }
  }
  SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}
upll_rc_t VbrIfMoMgr::UpdateConvVbrIfConfigStatus(ConfigKeyVal *ikey,
                                         unc_keytype_operation_t op,
                                         uint32_t driver_result,
                                         ConfigKeyVal *upd_key,
                                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  bool port_map_change = false;
  if (op == UNC_OP_CREATE) {
    port_map_change = true;
  } else if (op == UNC_OP_UPDATE) {
    uint8_t cand_flag = 0, run_flag = 0;
    GET_USER_DATA_FLAGS(ikey, cand_flag);
    GET_USER_DATA_FLAGS(upd_key, run_flag);
    if ((cand_flag & VIF_TYPE) != (run_flag & VIF_TYPE))
      port_map_change = true;
  }
  if (port_map_change) {
    val_db_vbr_if_st *vnif_st =
          reinterpret_cast<val_db_vbr_if_st *>
         (ConfigKeyVal::Malloc(sizeof(val_db_vbr_if_st)));
    vnif_st->vbr_if_val_st.valid[0] = UNC_VF_VALID;
    if (op == UNC_OP_CREATE) {
      if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
        vnif_st->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
        vnif_st->down_count = PORT_UNKNOWN;
      } else {
        vnif_st->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
        vnif_st->down_count = 0;
      }
    } else if (op == UNC_OP_UPDATE) {
      val_db_vbr_if_st *run_vbrifst = reinterpret_cast<val_db_vbr_if_st *>
                               (GetStateVal(upd_key));
      vnif_st->vbr_if_val_st.oper_status =
          run_vbrifst->vbr_if_val_st.oper_status;
      vnif_st->down_count = run_vbrifst->down_count;
    }
    ikey->AppendCfgVal(IpctSt::kIpcStValVbrIfSt, vnif_st);
  }
  return (SetInterfaceOperStatus(ikey, dmi, op, true, driver_result));
}
upll_rc_t VbrIfMoMgr::UpdateUvbrIfConfigStatus(ConfigKeyVal *ikey,
                                         unc_keytype_operation_t op,
                                         uint32_t driver_result,
                                         ConfigKeyVal *upd_key,
                                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  val_drv_vbr_if_t *drv_vbr_if_val =
      reinterpret_cast<val_drv_vbr_if_t *>(GetVal(ikey));
  if (!drv_vbr_if_val) {
    UPLL_LOG_DEBUG("invalid val ");
    return UPLL_RC_ERR_GENERIC;
  }
  val_vbr_if_t *vbr_if_val = &(drv_vbr_if_val->vbr_if_val);

  bool oper_change = false;
  if (op == UNC_OP_CREATE) {
    vbr_if_val->cs_row_status = UNC_CS_APPLIED;
    oper_change = true;
  } else if (op == UNC_OP_UPDATE) {
    if ((UNC_VF_INVALID == vbr_if_val->valid[UPLL_IDX_DESC_VBRI])
        && (UNC_VF_VALID == reinterpret_cast<val_drv_vbr_if_t *>(
                    GetVal(upd_key))->vbr_if_val.valid[UPLL_IDX_DESC_VBRI]))
      vbr_if_val->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_VALID_NO_VALUE;

    vbr_if_val->cs_row_status = reinterpret_cast<val_drv_vbr_if_t *>(
        GetVal(upd_key))->vbr_if_val.cs_row_status;
    uint8_t cand_flag = 0, run_flag = 0;
    GET_USER_DATA_FLAGS(ikey, cand_flag);
    GET_USER_DATA_FLAGS(upd_key, run_flag);
    if ((cand_flag & VIF_TYPE) != (run_flag & VIF_TYPE)) {
      oper_change = true;
    }
  }
  if (oper_change) {
    val_db_vbr_if_st *vnif_st =
          reinterpret_cast<val_db_vbr_if_st *>
         (ConfigKeyVal::Malloc(sizeof(val_db_vbr_if_st)));
    // for CREATE, default value for operstatus is DOWN(unmapped to any vlink)
    if (op == UNC_OP_CREATE) {
      vnif_st->vbr_if_val_st.valid[0] = UNC_VF_VALID;
      vnif_st->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
      vnif_st->down_count = 0;
    } else if (op == UNC_OP_UPDATE) {
        val_db_vbr_if_st *run_vbrifst = reinterpret_cast<val_db_vbr_if_st *>
                                 (GetStateVal(upd_key));
        vnif_st->vbr_if_val_st.oper_status =
            run_vbrifst->vbr_if_val_st.oper_status;
        vnif_st->down_count = run_vbrifst->down_count;
    }
    ikey->AppendCfgVal(IpctSt::kIpcStValVbrIfSt, vnif_st);
  }
  if ((op != UNC_OP_DELETE) &&
      (vbr_if_val->valid[UPLL_IDX_DESC_VBRI] != UNC_VF_INVALID)) {
    vbr_if_val->cs_attr[UPLL_IDX_DESC_VBRI] = UNC_CS_APPLIED;
  }
  return (SetInterfaceOperStatus(ikey, dmi, op, false, driver_result));
}

upll_rc_t VbrIfMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                         unc_keytype_operation_t op,
                                         uint32_t driver_result,
                                         ConfigKeyVal *upd_key,
                                         DalDmlIntf *dmi,
                                         ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;

  uint8_t *ctrlr_name = NULL;
  GET_USER_DATA_CTRLR(ikey, ctrlr_name);
  if (ctrlr_name) {
    if (IsUnifiedVbr(ctrlr_name)) {
      return UpdateUvbrIfConfigStatus(ikey, op, UPLL_RC_SUCCESS, upd_key, dmi);
    }
  }
  if (ikey->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
    return UpdateConvVbrIfConfigStatus(ikey, op, driver_result, upd_key, dmi);
  }

  val_drv_vbr_if_t *vbrif_val =
      reinterpret_cast<val_drv_vbr_if_t *>(GetVal(ikey));

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  if (vbrif_val == NULL) return UPLL_RC_ERR_GENERIC;
  val_vbr_if_t *vbr_if_val = &vbrif_val->vbr_if_val;
  bool port_map_change = false;
  bool propagate = false;
  switch (op) {
  case UNC_OP_UPDATE:
  {
    void *val = reinterpret_cast<void *>(vbrif_val);
    val_drv_vbr_if_t *vbrif_val2 = reinterpret_cast<val_drv_vbr_if_t *>
                                  (GetVal(upd_key));
    if (vbr_if_val->valid[UPLL_IDX_PM_VBRI] !=
        vbrif_val2->vbr_if_val.valid[UPLL_IDX_PM_VBRI]) {
      propagate = true;
      port_map_change = true;
    }
    CompareValidValue(val, GetVal(upd_key), true);
    if (vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] != UNC_VF_INVALID) {
      port_map_change = true;
      propagate = true;
    } else if (vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
           != UNC_VF_INVALID) {
      propagate = true;
      port_map_change = true;
    }
    uint8_t cand_flag = 0, run_flag = 0;
    GET_USER_DATA_FLAGS(ikey, cand_flag);
    GET_USER_DATA_FLAGS(upd_key, run_flag);
    if ((cand_flag & VIF_TYPE) != (run_flag & VIF_TYPE)) {
      port_map_change = true;
      propagate = true;
    }
    UPLL_LOG_DEBUG("ikey flags %d upd_key flags %d %d", cand_flag, run_flag,
                                                    port_map_change);
  }
    /* fall through intended */
  case UNC_OP_CREATE:
    if (op == UNC_OP_CREATE) {
      vbr_if_val->cs_row_status = cs_status;
      port_map_change = true;
    }
    if (port_map_change) {
      val_db_vbr_if_st *vbrif_st =
          reinterpret_cast<val_db_vbr_if_st *>
         (ConfigKeyVal::Malloc(sizeof(val_db_vbr_if_st)));
      ikey->AppendCfgVal(IpctSt::kIpcStValVbrIfSt, vbrif_st);
      val_db_vbr_if_st *vnif_st = reinterpret_cast<val_db_vbr_if_st  *>
               (GetStateVal(ikey));
      UPLL_LOG_DEBUG("valid %d  admin %d op %d",
                        vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI],
                        vbr_if_val->admin_status, op);
      vnif_st->vbr_if_val_st.valid[0] = UNC_VF_VALID;
      if (op == UNC_OP_CREATE) {
        if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
          vnif_st->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
          vnif_st->down_count = PORT_UNKNOWN;
        } else {
          vnif_st->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
          vnif_st->down_count = 0;
        }
      } else {
        val_db_vbr_if_st *run_vbrifst = reinterpret_cast<val_db_vbr_if_st *>
                                 (GetStateVal(upd_key));
        vnif_st->vbr_if_val_st.oper_status =
            run_vbrifst->vbr_if_val_st.oper_status;
        vnif_st->down_count = run_vbrifst->down_count;
      }
    }
    break;
  default:
    UPLL_LOG_DEBUG("Invalid op %d", op);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("%s", (ikey->ToStrAll()).c_str());
  val_vbr_if *vbr_if_val2 = reinterpret_cast<val_vbr_if *>(GetVal(upd_key));
  if (UNC_OP_UPDATE == op) {
    UPLL_LOG_TRACE("%s", (upd_key->ToStrAll()).c_str());
    vbr_if_val->cs_row_status = vbr_if_val2->cs_row_status;
  }
  for (unsigned int loop = 0; loop <
       sizeof(vbr_if_val->valid) / sizeof(vbr_if_val->valid[0]); ++loop) {
    if ((UNC_VF_VALID == (uint8_t) vbr_if_val->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == (uint8_t) vbr_if_val->valid[loop])) {
      // Description is set to APPLIED
      if (loop == UPLL_IDX_DESC_VBRI)
        vbr_if_val->cs_attr[loop] = UNC_CS_APPLIED;
      else
        vbr_if_val->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == vbr_if_val->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
      vbr_if_val->cs_attr[loop] = vbr_if_val2->cs_attr[loop];
    } else if ((UNC_VF_INVALID == vbr_if_val->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
      vbr_if_val->cs_attr[loop] = UNC_CS_APPLIED;
    }
  }
  val_port_map *pm =  &vbr_if_val->portmap;
  for (unsigned int loop = 0; loop < sizeof(pm->valid) / sizeof(pm->valid[0]);
                                                                       ++loop) {
    if ((UNC_VF_VALID == (uint8_t) pm->valid[loop])
        || (UNC_VF_VALID_NO_VALUE == (uint8_t) pm->valid[loop])) {
      pm->cs_attr[loop] = cs_status;
    } else if ((UNC_VF_INVALID == pm->valid[loop]) &&
               (UNC_OP_UPDATE == op)) {
      pm->cs_attr[loop] = vbr_if_val2->portmap.cs_attr[loop];
    } else if ((UNC_VF_INVALID == pm->valid[loop]) &&
               (UNC_OP_CREATE == op)) {
      pm->cs_attr[loop] = UNC_CS_APPLIED;
    }
  }
  return (SetInterfaceOperStatus(ikey, dmi, op, propagate, driver_result));
}

upll_rc_t VbrIfMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vbr_if_t *val;
  val =
      (ckv_running != NULL) ? reinterpret_cast<val_vbr_if_t *>(GetVal(
          ckv_running)) :
          NULL;
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
  for (unsigned int loop = 0;
       loop < sizeof(val->portmap.valid) / sizeof(uint8_t); ++loop) {
    if ((cs_status == UNC_CS_INVALID &&
                      UNC_VF_VALID == val->portmap.valid[loop])
        || cs_status == UNC_CS_APPLIED)
      val->portmap.cs_attr[loop] = cs_status;
  }
  return result_code;
}

upll_rc_t VbrIfMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                      ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !req || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is NUll");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (ikey->get_st_num() != IpctSt::kIpcStKeyVbrIf) {
    UPLL_LOG_DEBUG("Invalid key structure received. received struct - %d",
                  ikey->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  key_vbr_if *vbr_if_key = reinterpret_cast<key_vbr_if *>(ikey->get_key());
  unc_key_type_t ktype = ikey->get_key_type();
  if (UNC_KT_VBR_IF != ktype) {
    UPLL_LOG_DEBUG("Invalid Keytype received. received keytype - %d", ktype);
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  ret_val = ValidateVbrifKey(vbr_if_key, operation);

  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Validation failure for key_vbr_if struct");
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  val_vbr_if *vbr_if_val = NULL;
  val_vtn_neighbor *vtn_neighbor = NULL;

  if ((ikey->get_cfg_val())
     && ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVbrIf)) {
    vbr_if_val = reinterpret_cast<val_vbr_if *>(ikey->get_cfg_val()->get_val());
  } else if ((ikey->get_cfg_val()) &&
       ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVtnNeighbor)) {
    vtn_neighbor =  reinterpret_cast<val_vtn_neighbor *>
                                          (ikey->get_cfg_val()->get_val());
  } else if ((ikey->get_cfg_val()) &&
        ((ikey->get_cfg_val())->get_st_num() == IpctSt::kIpcStPfcdrvValVbrIf)) {
    vbr_if_val = &(reinterpret_cast<val_drv_vbr_if *>(
                   ikey->get_cfg_val()->get_val())->vbr_if_val);
  } else if ((ikey->get_cfg_val()) &&
       (((ikey->get_cfg_val())->get_st_num() != IpctSt::kIpcStValVbrIf) ||
       ((ikey->get_cfg_val())->get_st_num() != IpctSt::kIpcStValVtnNeighbor) ||
       ((ikey->get_cfg_val())->get_st_num() != IpctSt::kIpcStPfcdrvValVbrIf))) {
    UPLL_LOG_DEBUG("Invalid val structure received.received struct - %d",
                    ikey->get_cfg_val()->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if ((operation == UNC_OP_CREATE) && ((dt_type == UPLL_DT_CANDIDATE)
        || (UPLL_DT_IMPORT == dt_type))) {
    if (vbr_if_val == NULL) {
      UPLL_LOG_DEBUG("Val struct Validation is an optional"
                      "for CREATE operation");
      return UPLL_RC_SUCCESS;
    }
    ret_val = ValidateVbrIfValue(vbr_if_val, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" Val struct Validation failure for CREATE operation");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_UPDATE) && (dt_type == UPLL_DT_CANDIDATE)) {
    if (vbr_if_val == NULL) {
      UPLL_LOG_DEBUG("Val struct Validation is Mandatory for UPDATE operation");
      return UPLL_RC_ERR_BAD_REQUEST;
    }
    ret_val = ValidateVbrIfValue(vbr_if_val, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Val struct Validation failure for UPDATE operation");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING ||
              operation == UNC_OP_READ_SIBLING_BEGIN) &&
             (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
              dt_type == UPLL_DT_STARTUP)) {
      if (option1 != UNC_OPT1_NORMAL) {
         UPLL_LOG_DEBUG("Error option1 is not NORMAL");
         return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("Error option1 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (vbr_if_val == NULL) {
        UPLL_LOG_DEBUG(
            "Val struct Validation is an optional for READ operation");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVbrIfValue(vbr_if_val, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Val struct Validation failure for READ operation");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ_SIBLING ||
              operation == UNC_OP_READ_SIBLING_BEGIN) &&
             (dt_type == UPLL_DT_STATE)) {
      if (option1 != UNC_OPT1_NORMAL) {
         UPLL_LOG_DEBUG("Error option1 is not NORMAL");
         return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 != UNC_OPT2_NONE) {
        UPLL_LOG_DEBUG("Error option1 is not NONE");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
      if (vbr_if_val == NULL) {
        UPLL_LOG_DEBUG("Val struct Validation is an optional for"
                       "READ_SIBLING operation");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVbrIfValue(vbr_if_val, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Val struct Validation failure for"
                       "READ_SIBLING operation");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_READ) && (dt_type == UPLL_DT_STATE)) {
    if (option1 != UNC_OPT1_NORMAL) {
       UPLL_LOG_DEBUG("Error option1 is not NORMAL");
       return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 == UNC_OPT2_NONE) {
      if (vbr_if_val == NULL) {
        UPLL_LOG_DEBUG(
            "Val struct Validation is an optional for READ operation");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVbrIfValue(vbr_if_val, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Val struct Validation failure for READ operation");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    } else if (option2 == UNC_OPT2_NEIGHBOR) {
      if (vtn_neighbor == NULL) {
        UPLL_LOG_DEBUG(
            "Val vtn_neighbor struct Validation is an optional"
             "for READ operation");
        return UPLL_RC_SUCCESS;
      }
      ret_val = ValidateVtnNeighborValue(vtn_neighbor, operation);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Val struct Validation failure for"
                       "val_vtn_neighbor structure");
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Error option2 is not matching");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
  } else if ((operation == UNC_OP_READ_SIBLING_COUNT) &&
             (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
             dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE)) {
    if (option1 != UNC_OPT1_NORMAL) {
       UPLL_LOG_DEBUG("Error option1 is not NORMAL");
       return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG("Error option1 is not NONE");
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
    if (vbr_if_val == NULL) {
      UPLL_LOG_DEBUG("Val struct Validation is an optional for"
                     " READ_SIBLING_COUNT operation");
      return UPLL_RC_SUCCESS;
    }
    ret_val = ValidateVbrIfValue(vbr_if_val, operation);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Val struct Validation failure for"
                     "READ_SIBLING_COUNT operation");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
    return UPLL_RC_SUCCESS;
  } else if ((operation == UNC_OP_DELETE || operation == UNC_OP_READ_NEXT
      || operation == UNC_OP_READ_BULK)  &&
      (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING
        || dt_type == UPLL_DT_STARTUP)) {
      UPLL_LOG_DEBUG("value struct is none for this operation - %d", operation);
      return UPLL_RC_SUCCESS;
  }
  UPLL_LOG_DEBUG("Error Unsupported datatype (%d) or operation - (%d)",
                  dt_type, operation);
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
}


upll_rc_t VbrIfMoMgr::ValidateVbrifKey(key_vbr_if *vbr_if_key,
                                      unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  ret_val = ValidateKey(
      reinterpret_cast<char *>(vbr_if_key->vbr_key.vtn_key.vtn_name),
      kMinLenVtnName, kMaxLenVtnName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Vtn Name syntax check failed."
                  "Received VTN Name - %s",
                  vbr_if_key->vbr_key.vtn_key.vtn_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  ret_val = ValidateKey(
      reinterpret_cast<char *>(vbr_if_key->vbr_key.vbridge_name),
      kMinLenVnodeName, kMaxLenVnodeName);
  if (ret_val != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("VBR Name syntax check failed."
                  "Received VBR Name -%s",
                  vbr_if_key->vbr_key.vbridge_name);
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (operation != UNC_OP_READ_SIBLING_COUNT &&
      operation != UNC_OP_READ_SIBLING_BEGIN) {
    ret_val = ValidateKey(
      reinterpret_cast<char *>(vbr_if_key->if_name), kMinLenInterfaceName,
      kMaxLenInterfaceName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Interface Name syntax check failed."
                  "Received if_name - %s",
                  vbr_if_key->if_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    UPLL_LOG_TRACE("Operation is %d", operation);
    StringReset(vbr_if_key->if_name);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::ValidateVbrIfValue(val_vbr_if *vbr_if_val,
                                         unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;

  if (vbr_if_val->valid[UPLL_IDX_DESC_VBRI] == UNC_VF_VALID) {
    if (!ValidateDesc(vbr_if_val->description,
                           kMinLenDescription, kMaxLenDescription)) {
       UPLL_LOG_DEBUG("Description syntax check failed."
                    "Received description - %s",
                    vbr_if_val->description);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vbr_if_val->valid[UPLL_IDX_DESC_VBRI] == UNC_VF_VALID_NO_VALUE
      && (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
    uuu::upll_strncpy(vbr_if_val->description, " ", kMaxLenDescription+1);
  }
  if (vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_VALID) {
    if (!ValidateNumericRange(vbr_if_val->admin_status,
                              (uint8_t) UPLL_ADMIN_ENABLE,
                              (uint8_t) UPLL_ADMIN_DISABLE, true, true)) {
      UPLL_LOG_DEBUG("Admin status range check failed."
                    "Received Admin status - %d",
                    vbr_if_val->admin_status);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  }
#if 0
  else if ((vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI]
      == UNC_VF_VALID_NO_VALUE)
      && (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
    vbr_if_val->admin_status = UPLL_ADMIN_ENABLE;
  } else if ((vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_INVALID)
      && (operation == UNC_OP_CREATE)) {
    vbr_if_val->admin_status = UPLL_ADMIN_ENABLE;
    vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID_NO_VALUE;
  }
#endif
  if (vbr_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    if (vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
        == UNC_VF_VALID) {
      if (!ValidateLogicalPortId(
          reinterpret_cast<char *>(vbr_if_val->portmap.logical_port_id),
          kMinLenLogicalPortId, kMaxLenLogicalPortId)) {
        UPLL_LOG_DEBUG("Logical Port id syntax check failed."
                      "Received Logical Port Id - %s",
                      vbr_if_val->portmap.logical_port_id);
        return UPLL_RC_ERR_CFG_SYNTAX;
     }
     if (toupper(vbr_if_val->portmap.logical_port_id[0]) == 'S'
          && toupper(vbr_if_val->portmap.logical_port_id[1]) == 'W') {
        UPLL_LOG_DEBUG("Invalid logical_port_id - %s",
                       vbr_if_val->portmap.logical_port_id);
        return UPLL_RC_ERR_CFG_SYNTAX;
     }
    } else if (vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
                 UNC_VF_VALID_NO_VALUE
        && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
      uuu::upll_strncpy(vbr_if_val->portmap.logical_port_id, " ",
                            kMaxLenLogicalPortId+1);
      // Delete all dependent attributes.
      vbr_if_val->portmap.vlan_id = 0;
      vbr_if_val->portmap.tagged = 0;
      vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID_NO_VALUE;
      vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
    } else if ((vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
                UNC_VF_INVALID) && (operation == UNC_OP_CREATE)) {
       vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
       vbr_if_val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
    }
    if (vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
      if ((vbr_if_val->portmap.vlan_id != 0xFFFF) &&
          !ValidateNumericRange(vbr_if_val->portmap.vlan_id,
                                (uint16_t) kMinVlanId, (uint16_t) kMaxVlanId,
                                true, true)) {
        UPLL_LOG_DEBUG("Vlan Id Number check failed."
                      "Received vlan_id - %d",
                      vbr_if_val->portmap.vlan_id);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM]
        == UNC_VF_VALID_NO_VALUE
        && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
      /* If VLAN_ID is erased, Tagged attribute also needs to be erased */
      vbr_if_val->portmap.vlan_id = 0;
      vbr_if_val->portmap.tagged = 0;
      vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID_NO_VALUE;
    } else if ((vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
                UNC_VF_INVALID) && (operation == UNC_OP_CREATE)) {
      vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
    }
    if (vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID) {
      // TODO(owner): Enum value for TAg, UNTAG is Missing on Ipct File
      if (!ValidateNumericRange((uint8_t) vbr_if_val->portmap.tagged,
                                (uint8_t) UPLL_VLAN_UNTAGGED,
                                (uint8_t) UPLL_VLAN_TAGGED, true, true)) {
        UPLL_LOG_DEBUG("Tagged Numeric range check failed."
                      "Received Tag - %d",
                      vbr_if_val->portmap.tagged);
        return UPLL_RC_ERR_CFG_SYNTAX;
      }
    } else if (((vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] ==
         UNC_VF_VALID_NO_VALUE) ||
        (vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_INVALID)) &&
        (operation == UNC_OP_CREATE)) {
      if (vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
        vbr_if_val->portmap.tagged = UPLL_VLAN_TAGGED;
        vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
      }
    }
  } else if ((vbr_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE)
      && (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
    memset(&(vbr_if_val->portmap), 0, sizeof(vbr_if_val->portmap));
    for (unsigned int port_valid = 0;
         port_valid < sizeof(vbr_if_val->portmap.valid);
         ++port_valid) {
      vbr_if_val->portmap.valid[port_valid] = UNC_VF_VALID_NO_VALUE;
    }
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VbrIfMoMgr::ValidateVtnNeighborValue(
    val_vtn_neighbor *vtn_neighbor,
    unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  if (vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] == UNC_VF_VALID) {
    ret_val = ValidateKey(
        reinterpret_cast<char *>(vtn_neighbor->connected_vnode_name),
        kMinLenVnodeName, kMaxLenVnodeName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("connected_vnode_name syntax check failed."
                    "Received connected_vnode_name - %s",
                    vtn_neighbor->connected_vnode_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN]
      == UNC_VF_VALID_NO_VALUE
      && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
    uuu::upll_strncpy(vtn_neighbor->connected_vnode_name, " ",
                      kMaxLenVnodeName+1);
  }
  if (vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN] == UNC_VF_VALID) {
    ret_val = ValidateKey(
        reinterpret_cast<char *>(vtn_neighbor->connected_if_name),
        kMinLenInterfaceName, kMinLenInterfaceName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("connected_if_name syntax check failed."
                    "Received connected_if_name - %s",
                    vtn_neighbor->connected_if_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN]
      == UNC_VF_VALID_NO_VALUE
      && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
    uuu::upll_strncpy(vtn_neighbor->connected_if_name, " ",
                      kMaxLenInterfaceName+1);
  }
  if (vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] == UNC_VF_VALID) {
    ret_val = ValidateKey(
        reinterpret_cast<char *>(vtn_neighbor->connected_vlink_name),
        kMinLenVlinkName, kMaxLenVlinkName);
    if (ret_val != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("connected_vlink_name syntax check failed."
                    "Received connected_vlink_name - %s",
                    vtn_neighbor->connected_vlink_name);
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else if (vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN]
      == UNC_VF_VALID_NO_VALUE
      && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
  uuu::upll_strncpy(vtn_neighbor->connected_vlink_name, " ",
                          kMaxLenVlinkName+1);
}
return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey,
                                         const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_ERR_GENERIC;

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

  bool result_code = false;
  uint32_t max_attrs = 0;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs;

  switch (req->operation) {
    case UNC_OP_CREATE:
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count,
                                        &max_attrs, &attrs);
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
      UPLL_LOG_INFO("Invalid operation code - (%d)", req->operation);
      return UPLL_RC_ERR_GENERIC;
  }

  if (!result_code) {
    UPLL_LOG_INFO("key_type - %d is not supported by controller - %s",
                  ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  if (max_attrs > 0) {
    ret_val = ValVbrIfAttributeSupportCheck(attrs, ikey, req->operation,
                                            req->datatype);
    return ret_val;
  } else {
    UPLL_LOG_DEBUG("Attribute list is empty for operation %d", req->operation);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                        DalDmlIntf *dmi,
                                        IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_drv_vbr_if *if_val = reinterpret_cast<val_drv_vbr_if *>(GetVal(ikey));
  if (!if_val) {
    if (req->operation == UNC_OP_CREATE) {
      UPLL_LOG_DEBUG("Val Structure is Null");
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Val structure is must");
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    controller_domain ctrlr_dom;
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    //  unified vBridge interface does not support other than description
    //  attribute, if unified vBridge interface receives valid flag for
    //  portmap and shutdown other than UNC_VF_INVALID
    //  return UPLL_RC_ERR_CFG_SEMANTIC error.
    if (ctrlr_dom.ctrlr && IsUnifiedVbr(ctrlr_dom.ctrlr)) {
      if ((if_val->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] !=
          UNC_VF_INVALID)||
          (if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] != UNC_VF_INVALID)) {
        UPLL_LOG_ERROR("Unified vBridge interface supports only description"
                       "attribute");
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    } else {
      if ((if_val->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI]
           == UNC_VF_VALID_NO_VALUE)
          && (req->operation == UNC_OP_UPDATE ||
              req->operation == UNC_OP_CREATE)) {
        if_val->vbr_if_val.admin_status = UPLL_ADMIN_ENABLE;
      } else if ((if_val->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] ==
                  UNC_VF_INVALID)
                 && (req->operation == UNC_OP_CREATE)) {
        if_val->vbr_if_val.admin_status = UPLL_ADMIN_ENABLE;
        if_val->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] =
            UNC_VF_VALID_NO_VALUE;
      }
    }
  }
  ConfigKeyVal *vbrif_ckv_tmp = NULL;
  result_code = DupConfigKeyVal(vbrif_ckv_tmp, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to create VBR_IF ConfigKeyVal from ikey");
    return result_code;
  }

  /* Verifies whether the same logical-port-id and vlan-id is used in
   * other vbridge_if under the same controller and same domain*/
  result_code = IsLogicalPortAndVlanIdInUse<val_vbr_if_t>
                                (vbrif_ckv_tmp, dmi, req);
  DELETE_IF_NOT_NULL(vbrif_ckv_tmp);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Same logical_portid, vlan_id, vlan_tagged "
                   "is already mapped in other vbridge interface");
    return result_code;
  }

  /* Verifies whether the same logical-port-id and vlan-id is used in
   * other vterminal_if under the same controller and same domain*/
  ConfigKeyVal *vterm_if_ckv = NULL;
  VtermIfMoMgr* vtermif_mgr  =  reinterpret_cast<VtermIfMoMgr *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));

  result_code = vtermif_mgr->GetChildConfigKey(vterm_if_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to create vtem_if duplicate ConfigKeyVal");
    return result_code;
  }
  val_vbr_if_t *vbrif_val = &(if_val->vbr_if_val);

  val_vterm_if_t *vtermif_val = reinterpret_cast<val_vterm_if *>
      (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
  /* Copies only portmap attributes to vterm_if value structure */
  memcpy(&(vtermif_val->portmap), &(vbrif_val->portmap), sizeof(val_port_map));
  vtermif_val->valid[UPLL_IDX_PM_VTERMI] = vbrif_val->valid[UPLL_IDX_PM_VBRI];
  vterm_if_ckv->AppendCfgVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  SET_USER_DATA(vterm_if_ckv, ikey);

  result_code = vtermif_mgr->IsLogicalPortAndVlanIdInUse
                  <val_vterm_if_t>(vterm_if_ckv, dmi, req);
  DELETE_IF_NOT_NULL(vterm_if_ckv);

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Same logical_portid, vlan_id, vlan_tagged "
                   "is already mapped in other vterminal interface");
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::ValVbrIfAttributeSupportCheck(
          const uint8_t *attrs, ConfigKeyVal *ikey,
          unc_keytype_operation_t operation,
          upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  val_vbr_if_t *vbr_if_val = NULL;
  if (dt_type == UPLL_DT_IMPORT) {
    ConfigVal *cfg_val = ikey->get_cfg_val();
    if (!cfg_val)
      return UPLL_RC_SUCCESS;

    vbr_if_val = &(reinterpret_cast<val_drv_vbr_if *>(
                  cfg_val->get_val())->vbr_if_val);
    if (vbr_if_val) {
    }
  } else {
    ConfigVal *cfg_val = ikey->get_cfg_val();
        if (!cfg_val)
              return UPLL_RC_SUCCESS;

    vbr_if_val =
      reinterpret_cast<val_vbr_if_t *>(ikey->get_cfg_val()->get_val());
  }
  if (vbr_if_val != NULL) {
    if ((vbr_if_val->valid[UPLL_IDX_DESC_VBRI] == UNC_VF_VALID) ||
       (vbr_if_val->valid[UPLL_IDX_DESC_VBRI] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_if::kCapDesc] == 0) {
        vbr_if_val->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_INFO("Description attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_VALID) ||
        (vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] ==
         UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_if::kCapAdminStatus] == 0) {
        vbr_if_val->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
        if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
          UPLL_LOG_INFO("Admin status attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    if ((vbr_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) ||
        (vbr_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE)) {
      if ((vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
          == UNC_VF_VALID) ||
         (vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
            == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapLogicalPortId] == 0) {
          vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
              UNC_VF_INVALID;
          if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
            UPLL_LOG_INFO("portmap.logical_port_id attr"
                          " is not supported by ctrlr ");
            return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
          }
        }
      }
      if ((vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) ||
          (vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM]
            == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapVlanId] == 0) {
          vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
          if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
            UPLL_LOG_INFO("portmap.vlanid attr is not supported by ctrlr ");
            return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
          }
        }
      }
      if ((vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID) ||
          (vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM]
            == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapTagged] == 0) {
          vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
          if (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE) {
            UPLL_LOG_INFO("portmap.Tagged attr is not supported by ctrlr ");
            return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
          }
        }
      }
    }
  } else {
    UPLL_LOG_INFO("ERROR:Vbr_if Value structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

#if 0
  else if (UPLL_DT_IMPORT == dt_type) {
    val_drv_vbr_if *vbr_if_val =
      reinterpret_cast<val_drv_vbr_if *>(ikey->get_cfg_val()->get_val());
  if (vbr_if_val != NULL) {
    if ((vbr_if_val->vbr_if_val.valid[UPLL_IDX_DESC_VBRI] == UNC_VF_VALID)
        || (vbr_if_val->vbr_if_val.valid[UPLL_IDX_DESC_VBRI] ==
            UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_if::kCapDesc] == 0) {
        vbr_if_val->vbr_if_val.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_NOT_SUPPORTED;
        UPLL_LOG_INFO("Description attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((vbr_if_val->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] ==
         UNC_VF_VALID)
        ||(vbr_if_val->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] ==
           UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_if::kCapAdminStatus] == 0) {
        vbr_if_val->vbr_if_val.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_NOT_SUPPORTED;
        UPLL_LOG_INFO("Admin status attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((vbr_if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID)
        || (vbr_if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] ==
            UNC_VF_VALID_NO_VALUE)) {
      if ((vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
           UNC_VF_VALID)
          || (vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
              == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapLogicalPortId] == 0) {
          vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
              UNC_VF_NOT_SUPPORTED;
          UPLL_LOG_INFO("portmap.swich_id attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
      if ((vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
           UNC_VF_VALID)
          || (vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM]
              == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapVlanId] == 0) {
          vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] =
              UNC_VF_NOT_SUPPORTED;
          UPLL_LOG_INFO("portmap.vlanid attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
      if ((vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] ==
           UNC_VF_VALID)
          || (vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM]
              == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapTagged] == 0) {
          vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] =
              UNC_VF_NOT_SUPPORTED;
          UPLL_LOG_INFO("portmap.Tagged attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    return UPLL_RC_SUCCESS;
  }
  }
#endif

upll_rc_t VbrIfMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key()))
       return UPLL_RC_ERR_GENERIC;
  key_rename_vnode_info *key_rename =
                     reinterpret_cast<key_rename_vnode_info *>(ikey->get_key());

  if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vtn_name)))
    return UPLL_RC_ERR_GENERIC;

  key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));

  uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name, (kMaxLenVtnName + 1));
  if (UNC_KT_VBRIDGE == ikey->get_key_type()) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      free(key_rename);
      free(vbr_if_key);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                      key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      free(vbr_if_key);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                      key_rename->new_unc_vnode_name, (kMaxLenVnodeName + 1));
  }
  okey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                          vbr_if_key, NULL);
  if (!okey) {
    free(key_rename);
    free(vbr_if_key);
    return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_SUCCESS;
}

bool VbrIfMoMgr::FilterAttributes(void *&val1, void *val2, bool copy_to_running,
                                  unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  val_drv_vbr_if_t *val_vbr_if1 = reinterpret_cast<val_drv_vbr_if_t *>(val1);
  /* No need to configure description in controller. */
  val_vbr_if1->vbr_if_val.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, copy_to_running);
  return false;
}


bool VbrIfMoMgr::CompareValidValue(void *&val1, void *val2,
                                   bool copy_to_running) {
  UPLL_FUNC_TRACE;
  bool invalid_attr = true;
#if 0
  val_vbr_if_t *val_vbr_if1 = reinterpret_cast<val_vbr_if_t *>(val1);
  val_vbr_if_t *val_vbr_if2 = reinterpret_cast<val_vbr_if_t *>(val2);
#else
  val_drv_vbr_if_t *if1 = reinterpret_cast<val_drv_vbr_if_t *>(val1);
  val_vbr_if_t *val_vbr_if1 = &if1->vbr_if_val;
  val_drv_vbr_if_t *if2 = reinterpret_cast<val_drv_vbr_if_t *>(val2);
  val_vbr_if_t *val_vbr_if2 = &if2->vbr_if_val;
  UPLL_LOG_DEBUG("cand valid_admin %d run valid_admin %d",
                 val_vbr_if1->valid[UPLL_IDX_ADMIN_STATUS_VBRI],
                 val_vbr_if2->valid[UPLL_IDX_ADMIN_STATUS_VBRI]);
#endif
  for (unsigned int loop = 0;
       loop < sizeof(val_vbr_if1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vbr_if1->valid[loop]
        && ((UNC_VF_VALID == val_vbr_if2->valid[loop]) ||
            (UNC_VF_VALID_NO_VALUE == val_vbr_if2->valid[loop])))
      val_vbr_if1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  for (unsigned int loop = 1;
       loop < sizeof(if1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == if1->valid[loop]
        && UNC_VF_VALID == if2->valid[loop])
      if1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  for (unsigned int loop = 0;
       loop < sizeof(val_vbr_if1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vbr_if1->portmap.valid[loop]
        && UNC_VF_VALID == val_vbr_if2->portmap.valid[loop])
      val_vbr_if1->portmap.valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  // admin state val is needed to determine oper status
  if (val_vbr_if1->admin_status == val_vbr_if2->admin_status)
    val_vbr_if1->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_INVALID;
  if (copy_to_running) {
    if ((if1->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] == UNC_VF_VALID_NO_VALUE) &&
        (if1->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] == UNC_VF_VALID_NO_VALUE) &&
        (if1->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] == UNC_VF_VALID_NO_VALUE)) {
      if1->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
      if1->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
      if1->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
    } else {
      if ((UNC_VF_VALID == if1->valid[PFCDRV_IDX_VEXT_NAME_VBRIF])
          && (UNC_VF_VALID == if2->valid[PFCDRV_IDX_VEXT_NAME_VBRIF]))
        if (!strcmp(reinterpret_cast<char *>(if1->vex_name),
                     reinterpret_cast<const char*>(if2->vex_name)))
          if1->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_INVALID;
      if ((UNC_VF_VALID == if1->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF])
          && (UNC_VF_VALID == if2->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF]))
        if (!strcmp(reinterpret_cast<char *>(if1->vex_if_name),
                     reinterpret_cast<const char*>(if2->vex_if_name)))
          if1->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_INVALID;
      if ((UNC_VF_VALID == if1->valid[PFCDRV_IDX_VLINK_NAME_VBRIF])
          && (UNC_VF_VALID == if2->valid[PFCDRV_IDX_VLINK_NAME_VBRIF]))
        if (!strcmp(reinterpret_cast<char *>(if1->vex_link_name),
                    reinterpret_cast<const char*>(if2->vex_link_name)))
          if1->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_INVALID;
    }
    if (UNC_VF_INVALID != val_vbr_if1->valid[UPLL_IDX_DESC_VBRI]) {
      if ((UNC_VF_VALID == val_vbr_if1->valid[UPLL_IDX_DESC_VBRI]) &&
          (!strcmp(reinterpret_cast<char *>(val_vbr_if1->description),
                   reinterpret_cast<const char*>(val_vbr_if2->description))))
        val_vbr_if1->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
    }
    if (val_vbr_if1->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID
        && val_vbr_if2->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
      if (memcmp(&(val_vbr_if1->portmap), &(val_vbr_if2->portmap),
                 sizeof(val_port_map_t))) {
        if (val_vbr_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
            UNC_VF_VALID
            && val_vbr_if2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
            == UNC_VF_VALID) {
          if (!strcmp(
                  reinterpret_cast<char *>
                  (val_vbr_if1->portmap.logical_port_id),
                  reinterpret_cast<char *>
                  (val_vbr_if2->portmap.logical_port_id)))
            val_vbr_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
                UNC_VF_INVALID;
        }
        if (val_vbr_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] != UNC_VF_INVALID &&
            val_vbr_if2->portmap.valid[UPLL_IDX_VLAN_ID_PM] != UNC_VF_INVALID) {
          if (val_vbr_if1->portmap.vlan_id == val_vbr_if2->portmap.vlan_id)
            val_vbr_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
        }
#if 1
        if (val_vbr_if1->portmap.valid[UPLL_IDX_TAGGED_PM] != UNC_VF_INVALID &&
            val_vbr_if2->portmap.valid[UPLL_IDX_TAGGED_PM] != UNC_VF_INVALID) {
          if (val_vbr_if1->portmap.tagged == val_vbr_if2->portmap.tagged) {
            if (val_vbr_if1->portmap.valid[UPLL_IDX_TAGGED_PM] ==
                val_vbr_if2->portmap.valid[UPLL_IDX_TAGGED_PM]) {
              val_vbr_if1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
            }
          }
        }
#endif
      } else {
        val_vbr_if1->valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
        val_vbr_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
            UNC_VF_INVALID;
        val_vbr_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
        val_vbr_if1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
      }
    }
  }
  if (!copy_to_running)
    val_vbr_if1->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
  for (unsigned int loop = 0;
       loop < sizeof(val_vbr_if1->valid) / sizeof(val_vbr_if1->valid[0]);
       ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vbr_if1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vbr_if1->valid[loop])) {
      if (loop == UPLL_IDX_PM_VBRI) {
        for (unsigned int i = 0;
             i < sizeof(val_vbr_if1->portmap.valid) / sizeof(uint8_t); ++i) {
          if ((UNC_VF_VALID == (uint8_t) val_vbr_if1->portmap.valid[i]) ||
              (UNC_VF_VALID_NO_VALUE ==
               (uint8_t) val_vbr_if1->portmap.valid[i])) {
            invalid_attr = false;
            break;
          }
        }
      } else {
        invalid_attr = false;
      }
      if (invalid_attr == false) break;
    }
  }
  for (unsigned int loop = 1;
       loop < sizeof(if1->valid) / sizeof(uint8_t); ++loop) {
    if ((UNC_VF_VALID == if1->valid[loop]) ||
        (UNC_VF_VALID_NO_VALUE == if1->valid[loop])) {
      invalid_attr = false;
      break;
    }
  }
  return invalid_attr;
}


upll_rc_t VbrIfMoMgr::IsReferenced(IpcReqRespHeader *req,
                                   ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *okey = NULL;
  if (NULL == ikey)
     return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKeyFailed %d", result_code);
    return result_code;
  }

  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                    UPLL_RC_SUCCESS:result_code;
    delete okey;
    return result_code;
  }
  ConfigKeyVal *temkey = okey;
  while (temkey != NULL) {
    uint8_t vlink_flag = 0;
    GET_USER_DATA_FLAGS(temkey, vlink_flag);
    if (vlink_flag & VIF_TYPE) {
      delete okey;
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    temkey = temkey->get_next_cfg_key_val();
  }
  delete okey;
#if 0
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                                (GetMoManager(UNC_KT_VLINK)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Instance is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  while (okey) {
    result_code = mgr->CheckVnodeInfo(okey, dt_type, dmi);
    if (UPLL_RC_SUCCESS == result_code)
      return UPLL_RC_ERR_CFG_SEMANTIC;
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("CheckVnodeInfor Failed %d", result_code);
      return result_code;
    }
    okey = okey->get_next_cfg_key_val();
  }
#endif
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::UpdateConfigVal(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t datatype,
                                      DalDmlIntf *dmi,
                                      TcConfigMode config_mode,
                                      string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failure %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(okey, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Read failed %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  val_drv_vbr_if *val_drv_vbr =
      reinterpret_cast<val_drv_vbr_if *>(GetVal(ikey));
  if (!val_drv_vbr) {
    UPLL_LOG_DEBUG("Val Vbr is Null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t flag = 0;
  GET_USER_DATA_FLAGS(okey, flag);
  if ((val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) ||
      (val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] ==
       UNC_VF_VALID_NO_VALUE)) {
    if (flag & VIF_TYPE) {
      DELETE_IF_NOT_NULL(okey);
      UPLL_LOG_DEBUG("Interface is linked/bounded with Vlink. "
                     "Could not update Portmap");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    result_code = UpdatePortMap(okey, datatype, dmi, ikey, config_mode,
                                vtn_name);
  }
  DELETE_IF_NOT_NULL(okey);
  return result_code;
}

upll_rc_t VbrIfMoMgr::UpdatePortMap(ConfigKeyVal *okey,
                                    upll_keytype_datatype_t datatype,
                                    DalDmlIntf *dmi,
                                    ConfigKeyVal *ikey,
                                    TcConfigMode config_mode,
                                    string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
                                      GetMoManager(UNC_KT_VBRIF_FLOWFILTER)));
  if (!mgr) {
      UPLL_LOG_DEBUG("Invalid Instance");
      return UPLL_RC_ERR_GENERIC;
  }
  MoMgrImpl *pm_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
                                      GetMoManager(UNC_KT_VBRIF_POLICINGMAP)));
  if (!pm_mgr) {
      UPLL_LOG_DEBUG("Invalid Instance");
      return UPLL_RC_ERR_GENERIC;
  }

  NotifyPOMForPortMapInfo PortMapNotificationVal = kPortMapNoChange;
  val_drv_vbr_if *val_drv_vbr =
      reinterpret_cast<val_drv_vbr_if *>(GetVal(ikey));
  bool port_map_status =
      (val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID)?
      true:false;
  if (port_map_status) {
    val_drv_vbr_if *vbr_val_db =
        reinterpret_cast<val_drv_vbr_if *>(GetVal(okey));
    if (!vbr_val_db) {
      UPLL_LOG_DEBUG("Invalid param");
      return UPLL_RC_ERR_GENERIC;
    }
    if (vbr_val_db->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_INVALID) {
      /* portmap getting created for the first time */
      result_code = ConverttoDriverPortMap(ikey, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ConvertToDriverPortMap Failure %d", result_code);
        return result_code;
      }
      // Set the PortMapNotificationVal Enum status to Portmap Created
      // Which will be used to Notify the POM
      PortMapNotificationVal = kPortMapCreated;
    } else {
      /* portmap already exists - only change in vlan/tagged */
      val_drv_vbr->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_INVALID;
      val_drv_vbr->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_INVALID;
      val_drv_vbr->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_INVALID;
      PortMapNotificationVal = kPortMapUpdated;
    }
  } else {
#if 0
    string s(okey->ToStrAll());
    port_map_valid_status = true;
    GET_USER_DATA_FLAGS(okey, rename);
    if ((rename & VLINK_VNODE1) || (rename & VLINK_VNODE2)) {
        return UPLL_RC_ERR_GENERIC;
    }
#endif
    val_drv_vbr->valid[PFCDRV_IDX_VAL_VBRIF] = UNC_VF_VALID_NO_VALUE;
    val_drv_vbr->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
    val_drv_vbr->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
    val_drv_vbr->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID_NO_VALUE;
    val_drv_vbr->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
                                             UNC_VF_VALID_NO_VALUE;
    uuu::upll_strncpy(val_drv_vbr->vbr_if_val.portmap.logical_port_id,
                                                               "\0", 1);
    val_drv_vbr->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] =
                                             UNC_VF_VALID_NO_VALUE;
    val_drv_vbr->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] =
                                             UNC_VF_VALID_NO_VALUE;
    val_drv_vbr->vbr_if_val.portmap.tagged = UPLL_VLAN_UNTAGGED;
    val_drv_vbr->vbr_if_val.portmap.vlan_id = 0;
    uuu::upll_strncpy(val_drv_vbr->vex_name, " ", (kMaxLenVnodeName+1));
    uuu::upll_strncpy(val_drv_vbr->vex_if_name, " ", (kMaxLenVnodeName+1));
    uuu::upll_strncpy(val_drv_vbr->vex_link_name, " ", (kMaxLenVnodeName+1));

    // Set the PortMapNotificationVal Enum status to Portmap Deleted
    // Which will be used to Notify the POM
    PortMapNotificationVal = kPortMapDeleted;
     // TODO(PI): any think inform to POM.. will call it from here
  /* Info to POM */
  }
  // Notify POM only when PortMap is created or Deleted
  if (PortMapNotificationVal == kPortMapCreated) {
    UPLL_LOG_DEBUG("Portmapstatus-true");
    result_code = mgr->SetVlinkPortmapConfiguration(
        okey, datatype, dmi,  kPortMapConfigured,
        UNC_OP_CREATE, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
       return result_code;
    }
      result_code = pm_mgr->SetVlinkPortmapConfiguration(
          okey, datatype, dmi,  kPortMapConfigured,
          UNC_OP_CREATE, config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
         return result_code;
      }
  } else if (PortMapNotificationVal == kPortMapDeleted) {
    UPLL_LOG_DEBUG("Portmapstatus-false");
    result_code = mgr->SetVlinkPortmapConfiguration(
        okey, datatype, dmi,  kVlinkPortMapNotConfigured,
        UNC_OP_DELETE, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code
        && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
      return result_code;
     }
     result_code = pm_mgr->SetVlinkPortmapConfiguration(
         okey, datatype, dmi,  kVlinkPortMapNotConfigured,
         UNC_OP_DELETE, config_mode, vtn_name);
     if (UPLL_RC_SUCCESS != result_code
          && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
        return result_code;
     }
  }
  result_code =
      (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t VbrIfMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey,
                                           AdaptType adapt_type) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Invalid ikey");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbr_if *vbrif_key = reinterpret_cast<key_vbr_if*>(ikey->get_key());
  while (ikey) {
    ConfigVal *cval = ikey->get_cfg_val();
    if (!cval) {
      UPLL_LOG_DEBUG("Config Val is Null");
      return UPLL_RC_ERR_GENERIC;
    }
    while (cval) {
      if (IpctSt::kIpcStPfcdrvValVbrIf == cval->get_st_num()) {
         val_vbr_if_t *vbr_if_val = reinterpret_cast<val_vbr_if_t *>
                                (ConfigKeyVal::Malloc(sizeof(val_vbr_if_t)));
         val_drv_vbr_if *vbr_drv_if_val = reinterpret_cast<val_drv_vbr_if *>
                                     (cval->get_val());
         memcpy(vbr_if_val, &(vbr_drv_if_val->vbr_if_val),
               sizeof(val_vbr_if_t));
         cval->SetVal(IpctSt::kIpcStValVbrIf, vbr_if_val);
         /* do not display portmap info if not configured (for boundary)*/
         uint8_t vlink_flag = 0;
         GET_USER_DATA_FLAGS(ikey, vlink_flag);
         UPLL_LOG_DEBUG("Interface type %d", vlink_flag);
         // set admin status to valid no value
         if (vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_INVALID)
           vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] =
               UNC_VF_VALID_NO_VALUE;
         if (vlink_flag & VIF_TYPE)
           vbr_if_val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
      }
      if (IpctSt::kIpcStValVbrIfSt == cval->get_st_num()) {
        controller_domain ctrlr_dom = {NULL, NULL};
        GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
        if (ctrlr_dom.ctrlr && !IsUnifiedVbr(ctrlr_dom.ctrlr)) {
          CheckOperStatus<val_vbr_if_st>(vbrif_key->vbr_key.vtn_key.vtn_name,
                                       cval, UNC_KT_VBR_IF, ctrlr_dom);
        }
      }
      cval = cval->get_next_cfg_val();
    }
    if (adapt_type == ADAPT_ONE)
      break;
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::GetVexternal(ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t dt_type,
                                   DalDmlIntf *dmi,
                                   uint8_t *vexternal, uint8_t *vex_if,
                                   InterfacePortMapInfo &iftype,
                                   const char* ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_if_t *vbr_if_key = (ikey)?reinterpret_cast<key_vbr_if_t*>(
      ikey->get_key()) : NULL;
  if (!vbr_if_key) {
    UPLL_LOG_DEBUG("Key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }
  val_drv_vbr_if *vbr_if_val = reinterpret_cast<val_drv_vbr_if *>
                   (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
  // vbr_if_val->valid[UPLL_IDX_VBR_IF_DRV_PM]  = UNC_VF_VALID;
  // vbr_if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  // vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  ikey->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrIf, vbr_if_val);

  /* Get the vbridgeIf instance from db */
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  switch (dt_type) {
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_IMPORT:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STATE:
           result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop,
                                       dmi, MAINTBL);
           break;
    case UPLL_DT_AUDIT:
           result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop,
                                       dmi, MAINTBL);
           break;
    default:
           UPLL_LOG_DEBUG("Invalid Datatype %d", dt_type);
           return UPLL_RC_ERR_GENERIC;
           break;
  }

  if (result_code != UPLL_RC_SUCCESS
      && result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("ReadConfiDB Failed: %d", result_code);
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    uint8_t vbrif_flag = 0;
    iftype = kVlinkPortMapNotConfigured;
    GET_USER_DATA_FLAGS(ikey, vbrif_flag);
    if (vbrif_flag & VIF_TYPE_BOUNDARY) {
      iftype = kVlinkPortMapConfigured;  // boundary
    } else if (vbrif_flag & VIF_TYPE_LINKED) {
      iftype = kVlinkConfigured;  // linked
    }
    vbr_if_val = reinterpret_cast<val_drv_vbr_if_t *>
           (GetVal(ikey));
    if (vbr_if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
      unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
      uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
      if (!ctrlr_mgr->GetCtrlrType(ctrlr_id, dt_type, &ctrlrtype)) {
        UPLL_LOG_ERROR("Controller name %s not found in datatype %d",
                       ctrlr_id, dt_type);
        return UPLL_RC_ERR_GENERIC;
      }
      iftype = kPortMapConfigured;
       if (vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]) {
         // In ODC, there are no vexternals, so get vext and vext_if only for
         // other controllers
         if (ctrlrtype != UNC_CT_ODC) {
           if (!strlen(reinterpret_cast<const char *>(vbr_if_val->vex_name))) {
             UPLL_LOG_ERROR("Vexternal name cannot be NULL");
             return UPLL_RC_ERR_GENERIC;
           }
           uuu::upll_strncpy(vexternal, vbr_if_val->vex_name,
                             (kMaxLenInterfaceName + 1));
           if (!strlen(reinterpret_cast<const char *>
                       (vbr_if_val->vex_if_name))) {
             UPLL_LOG_ERROR("Vexternal interface name cannot be NULL");
             return UPLL_RC_ERR_GENERIC;
           }
           uuu::upll_strncpy(vex_if, vbr_if_val->vex_if_name,
                        (kMaxLenInterfaceName + 1));
         }
      }
    }
  } else {
    iftype = kVlinkPortMapNotConfigured;
    vexternal[0] = '\0';
    vex_if[0] = '\0';
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::GetVbrIfFromVExternal(uint8_t *vtn_name,
                                            uint8_t *vext_name,
                                            ConfigKeyVal *&tmpckv,
                                            DalDmlIntf *dmi,
                                            controller_domain_t ctr_dom,
                                            upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_DEBUG(" vtn_name - %s, vext_name - %s", vtn_name, vext_name);
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  if (tmpckv) {
    UPLL_LOG_INFO("tmpckv is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbr_if_t *key_vbrif = static_cast<key_vbr_if_t *>
               (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
  uuu::upll_strncpy(key_vbrif->vbr_key.vtn_key.vtn_name,
               vtn_name, (kMaxLenVtnName + 1));
  val_drv_vbr_if_t *drv_val_vbrif = static_cast<val_drv_vbr_if_t *>
               (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if_t)));
  drv_val_vbrif->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
  uuu::upll_strncpy(drv_val_vbrif->vex_name,
                vext_name, (kMaxLenVnodeName + 1));
  tmpckv = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, key_vbrif,
              new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, drv_val_vbrif));
  if (tmpckv == NULL) {
    free(key_vbrif);
    free(drv_val_vbrif);
    return UPLL_RC_ERR_GENERIC;
  }

  if (ctr_dom.ctrlr) {
    dbop.matchop |= kOpMatchCtrlr;
    SET_USER_DATA_CTRLR(tmpckv, ctr_dom.ctrlr);
  }
  if (ctr_dom.domain) {
    dbop.matchop |= kOpMatchDomain;
    SET_USER_DATA_DOMAIN(tmpckv, ctr_dom.domain);
  }
  result_code = ReadConfigDB(tmpckv, dt_type,
                          UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("vbrif ReadConfigDB Failed result_code - %d",
                     result_code);
  }
  UPLL_LOG_DEBUG("tmpckv is %s", tmpckv->ToStrAll().c_str());
  return result_code;
}

#if 0
upll_rc_t VbrIfMoMgr::IsLogicalPortAndVlanIdInUse(ConfigKeyVal *ikey,
                                                  DalDmlIntf *dmi,
                                                  IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ckv_vbrif = NULL;
  val_vbr_if *vbrif_val = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !(ikey->get_cfg_val()))
    return UPLL_RC_ERR_GENERIC;

  result_code = GetChildConfigKey(ckv_vbrif, NULL);
  if ((ckv_vbrif == NULL) || (result_code != UPLL_RC_SUCCESS))
    return UPLL_RC_ERR_GENERIC;
  if (UPLL_DT_IMPORT == req->datatype) {
    val_drv_vbr_if * vbr_drv = static_cast<val_drv_vbr_if *>(GetVal(ikey));
    if (!vbr_drv) {
      DELETE_IF_NOT_NULL(ckv_vbrif);
      return UPLL_RC_SUCCESS;
    }
    vbrif_val = &vbr_drv->vbr_if_val;
  } else {
    vbrif_val = &(static_cast<val_drv_vbr_if *>(GetVal(ikey))->vbr_if_val);
  }
  if (!vbrif_val) {
    UPLL_LOG_DEBUG("Returning error\n");
    DELETE_IF_NOT_NULL(ckv_vbrif);
    return UPLL_RC_ERR_GENERIC;
  }
  if (vbrif_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    val_drv_vbr_if *drv_if_val = reinterpret_cast<val_drv_vbr_if *>
      (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
    val_vbr_if_t *if_val = &drv_if_val->vbr_if_val;
    if (vbrif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] == UNC_VF_VALID) {
      uuu::upll_strncpy(if_val->portmap.logical_port_id,
          vbrif_val->portmap.logical_port_id,
          kMaxLenLogicalPortId+1);
      if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
      if_val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
    }
#if 0
    if (vbrif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
      if_val->portmap.vlan_id = vbrif_val->portmap.vlan_id;
      if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] =
                      vbrif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM];
    }
    if ((vbrif_val->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID) ||
        (vbrif_val->portmap.valid[UPLL_IDX_TAGGED_PM] ==
                                              UNC_VF_VALID_NO_VALUE)) {
      if_val->portmap.tagged = vbrif_val->portmap.tagged;
      if_val->portmap.valid[UPLL_IDX_TAGGED_PM] =
                         vbrif_val->portmap.valid[UPLL_IDX_TAGGED_PM];
    }
#endif
    ckv_vbrif->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrIf, drv_if_val);
    SET_USER_DATA(ckv_vbrif, ikey);
    DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr | kOpMatchDomain,
                     kOpInOutFlag};
    result_code = ReadConfigDB(ckv_vbrif, UPLL_DT_CANDIDATE, UNC_OP_READ,
                               dbop, dmi, MAINTBL);
    if (result_code == UPLL_RC_SUCCESS) {
      ConfigKeyVal *tmp = ckv_vbrif;
      while (tmp) {
        if (!memcmp(ikey->get_key(), tmp->get_key(),
              sizeof(key_vbr_if_t))) {
          UPLL_LOG_TRACE("Looking on the Same key");
        } else {
          bool match = false;
          val_vbr_if_t *if_val = &(reinterpret_cast<val_drv_vbr_if *>
                                  (GetVal(tmp))->vbr_if_val);
          if (vbrif_val->portmap.valid[UPLL_IDX_VLAN_ID_PM]
                                                         == UNC_VF_VALID) {
            if (if_val->portmap.vlan_id == vbrif_val->portmap.vlan_id) {
              if (if_val->portmap.tagged == vbrif_val->portmap.tagged)
                match = true;
            }
          } else {
            if (if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
              if (if_val->portmap.tagged == vbrif_val->portmap.tagged)
                match = true;
            } else {
                match = true;
            }
          }
          if (match) {
            UPLL_LOG_DEBUG("More than one vbridge interface is configured "
                           " same logical port id and vlanid!");
            delete ckv_vbrif;
            ckv_vbrif = tmp = NULL;
            return UPLL_RC_ERR_CFG_SEMANTIC;
          }
        }
        tmp = tmp->get_next_cfg_key_val();
      }
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
    }
  }
  delete ckv_vbrif, ckv_vbrif = NULL;
  return result_code;
}
#endif

upll_rc_t VbrIfMoMgr::AdaptValToDriver(ConfigKeyVal *ck_new,
                                       ConfigKeyVal *ck_old,
                                       unc_keytype_operation_t op,
                                       upll_keytype_datatype_t dt_type,
                                       unc_key_type_t keytype,
                                       DalDmlIntf *dmi,
                                       bool &not_send_to_drv,
                                       bool audit_update_phase) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  GET_USER_DATA_CTRLR_DOMAIN(ck_new, ctrlr_dom);
  //  unified vBridge interface not shared to controller
  if (ctrlr_dom.ctrlr) {
    if (IsUnifiedVbr(ctrlr_dom.ctrlr)) {
      not_send_to_drv = true;
      return UPLL_RC_SUCCESS;
    }
  }

  val_drv_vbr_if *vbr_ifval = reinterpret_cast<val_drv_vbr_if *>
      (GetVal(ck_new));
  if (!vbr_ifval) {
    return UPLL_RC_ERR_GENERIC;
  }
  vbr_ifval->valid[0] = UNC_VF_INVALID;
  for (int i = 0; i < 3 ; ++i) {
    if (vbr_ifval->vbr_if_val.valid[i] != UNC_VF_INVALID) {
      vbr_ifval->valid[0] = UNC_VF_VALID;
      break;
    }
  }
  if (!audit_update_phase) {
    if (op == UNC_OP_DELETE || op == UNC_OP_UPDATE) {
      // Perform semantic check for vnode interface deletion
      // check whether the vnode interface is referred in the
      // flowfilter-redirection.
      result_code = CheckVnodeInterfaceForRedirection(ck_new,
                                                      ck_old,
                                                      dmi,
                                                      UPLL_DT_CANDIDATE,
                                                      op);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Flowfilter redirection validation fails for keytype %d"
                      " cannot perform operation  %d errcode %d",
                      keytype, op, result_code);
        return result_code;
      }
    }
  }
  return result_code;
}

upll_rc_t VbrIfMoMgr::PartialMergeValidate(unc_key_type_t key_type,
                                           const char *ctrlr_name,
                                           ConfigKeyVal *err_ckv,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_running = NULL;
  string vtn_name = "";

  if (UNC_KT_VBR_IF != key_type || !err_ckv) {
    UPLL_LOG_TRACE("Invalid Input");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name) {
    UPLL_LOG_DEBUG("Controller Name is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr,
                   kOpInOutFlag|kOpInOutCtrlr };
  /*
   * Read info from Running DB.
   */
  result_code = GetChildConfigKey(ckv_running, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("GetChildConfigKey Failed - %d", result_code);
    return result_code;
  }
  SET_USER_DATA_CTRLR(ckv_running, ctrlr_name);
  result_code = ReadConfigDB(ckv_running, UPLL_DT_RUNNING, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    delete ckv_running;
    return UPLL_RC_SUCCESS;
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("ReadConfigDB Failed - %d", result_code);
    delete ckv_running;
    return result_code;
  }
  ConfigKeyVal *start_ckv = ckv_running;
  while (ckv_running) {
    val_drv_vbr_if_t *ru_vbrif_val = reinterpret_cast<val_drv_vbr_if_t *>
                                       (GetVal(ckv_running));
#if 0
    bool valid_pm_st = (UNC_VF_VALID ==
                       ru_vbrif_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI]);
    bool valid_pm = (UNC_VF_VALID ==
         ru_vbrif_val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]);
#endif
    ConfigKeyVal *ckv_import = NULL;
    uint8_t vbrif_flag = 0;
    GET_USER_DATA_FLAGS(ckv_running, vbrif_flag);
    /*
     * Check vbr_if is boundary interface or not.
     */
    if (vbrif_flag & VIF_TYPE_BOUNDARY) {
#if 0
      if (!(valid_pm_st && valid_pm)) {
        err_ckv->ResetWithoutNextCkv(ckv_running);
        UPLL_LOG_DEBUG("MergeConflict with %s", err_ckv->ToStr().c_str());
        delete start_ckv;
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
#endif
      /*
       * Read info from Import DB.
       *
       */
      result_code = GetChildConfigKey(ckv_import, ckv_running);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("GetChildConfigKey Failed - %d", result_code);
        delete start_ckv;
        return result_code;
      }
      dbop.readop = kOpReadSingle;
      result_code = ReadConfigDB(ckv_import, UPLL_DT_IMPORT, UNC_OP_READ,
                                 dbop, dmi, MAINTBL);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        err_ckv->ResetWithoutNextCkv(ckv_running);
        UPLL_LOG_DEBUG("MergeConflict with %s", err_ckv->ToStr().c_str());
        delete start_ckv;
        delete ckv_import;
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("ReadConfigDB Failed - %d", result_code);
        delete start_ckv;
        delete ckv_import;
        return result_code;
      }
      val_drv_vbr_if_t *im_vbrif_val = reinterpret_cast<val_drv_vbr_if_t *>
                                               (GetVal(ckv_import));
      /*
       * Compare valid flags and values.
       */
      for (unsigned int valid_index = 0;
        valid_index < sizeof(ru_vbrif_val->vbr_if_val.portmap.valid) /
                      sizeof(uint8_t); ++valid_index) {
        uint8_t ru_flag = ru_vbrif_val->vbr_if_val.portmap.valid[valid_index];
        uint8_t im_flag = im_vbrif_val->vbr_if_val.portmap.valid[valid_index];
        switch (im_flag) {
          case UNC_VF_VALID:
            if (ru_flag == UNC_VF_VALID) {
              switch (valid_index) {
                case UPLL_IDX_LOGICAL_PORT_ID_PM:
                  result_code = (strcmp(
                          reinterpret_cast<char*>
                          (ru_vbrif_val->vbr_if_val.portmap.logical_port_id),
                          reinterpret_cast<char*>
                          (im_vbrif_val->vbr_if_val.portmap.logical_port_id))
                      == 0)?UPLL_RC_SUCCESS:UPLL_RC_ERR_MERGE_CONFLICT;
                  break;
                case UPLL_IDX_VLAN_ID_PM:
                  result_code = ((ru_vbrif_val->vbr_if_val.portmap.vlan_id) ==
                             (im_vbrif_val->vbr_if_val.portmap.vlan_id))?
                             UPLL_RC_SUCCESS:UPLL_RC_ERR_MERGE_CONFLICT;
                  break;
                case UPLL_IDX_TAGGED_PM:
                  result_code = ((ru_vbrif_val->vbr_if_val.portmap.tagged) ==
                         (im_vbrif_val->vbr_if_val.portmap.tagged))?
                         UPLL_RC_SUCCESS:UPLL_RC_ERR_MERGE_CONFLICT;
                  break;
                default:
                  break;
              }
              if (UPLL_RC_SUCCESS != result_code) {
                err_ckv->ResetWithoutNextCkv(ckv_running);
                UPLL_LOG_DEBUG("MergeConflict with %s"
                               , err_ckv->ToStr().c_str());
                delete ckv_import;
                delete start_ckv;
                return UPLL_RC_ERR_MERGE_CONFLICT;
              }
            } else {
              err_ckv->ResetWithoutNextCkv(ckv_running);
              UPLL_LOG_DEBUG("MergeConflict with %s", err_ckv->ToStr().c_str());
              delete start_ckv;
              delete ckv_import;
              return UPLL_RC_ERR_MERGE_CONFLICT;
            }
            break;
         case UNC_VF_VALID_NO_VALUE:
         case UNC_VF_INVALID:
            if ((ru_flag == UNC_VF_VALID_NO_VALUE) ||
                (ru_flag == UNC_VF_INVALID)) {
              // Nothing to do
            } else if (ru_flag == UNC_VF_VALID) {
              err_ckv->ResetWithoutNextCkv(ckv_running);
              UPLL_LOG_DEBUG("MergeConflict with %s", err_ckv->ToStr().c_str());
              delete start_ckv;
              delete ckv_import;
              return UPLL_RC_ERR_MERGE_CONFLICT;
            }
            break;
         default:
           break;
        }
      }
      if (result_code == UPLL_RC_SUCCESS) {
        /*
         * set vbr_if boundary flag info from running_db to import_db
         * and Update the same.
         *
         */
        uint8_t rename_flag = 0x00;
        GET_USER_DATA_FLAGS(ckv_import, rename_flag);
        rename_flag |= vbrif_flag;
        SET_USER_DATA_FLAGS(ckv_import, rename_flag);
        DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr,
                       kOpInOutFlag};

        result_code = UpdateConfigDB(ckv_import, UPLL_DT_IMPORT,
                     UNC_OP_UPDATE, dmi, &dbop, TC_CONFIG_GLOBAL,
                     vtn_name, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          delete start_ckv;
          delete ckv_import;
          return result_code;
        }
      }
      delete ckv_import;
    }
    ckv_running = ckv_running->get_next_cfg_key_val();
  }
  delete start_ckv;
  return result_code;
}

upll_rc_t VbrIfMoMgr::TranslateError(ConfigKeyVal *err_ckv,
                                     DalDmlIntf *dmi,
                                     upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vbr_if_temp = NULL;

  if ((err_ckv->get_key_type() != UNC_KT_VLINK) || !(err_ckv->get_key())) {
    return UPLL_RC_ERR_GENERIC;
  }

  key_vlink *vlink_key = reinterpret_cast<key_vlink *>(err_ckv->get_key());

  result_code = GetChildConfigKey(ck_vbr_if_temp, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey failed %d", result_code);
    return result_code;
  }

  key_vbr_if *vbr_if_key = reinterpret_cast<key_vbr_if *>(
      ck_vbr_if_temp->get_key());
  uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                    vlink_key->vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));

  val_drv_vbr_if *vbr_drv_if_val = ConfigKeyVal::Malloc<val_drv_vbr_if>();
  uuu::upll_strncpy(vbr_drv_if_val->vex_link_name, vlink_key->vlink_name,
                    (kMaxLenVnodeName + 1));
  vbr_drv_if_val->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID;
  ck_vbr_if_temp->AppendCfgVal(IpctSt::IpctSt::kIpcStValVbrIf, vbr_drv_if_val);

  // Check whether given vlink_name is present in
  // vbridge interface table as vexternal vlink
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(ck_vbr_if_temp, datatype, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  // convert vlink KT error to vbrif KT error if it is portmap error
  if (result_code == UPLL_RC_SUCCESS) {
    ConfigKeyVal *err_next = err_ckv->get_next_cfg_key_val();
    err_ckv->set_next_cfg_key_val(NULL);
    err_ckv->ResetWith(ck_vbr_if_temp);
    err_ckv->AppendCfgKeyVal(err_next);
  }
  DELETE_IF_NOT_NULL(ck_vbr_if_temp);
  return result_code;
}

upll_rc_t VbrIfMoMgr::TxUpdateErrorHandler(ConfigKeyVal *req,
                                            ConfigKeyVal *ck_main,
                                            DalDmlIntf *dmi,
                                            upll_keytype_datatype_t dt_type,
                                            ConfigKeyVal **err_ckv,
                                            IpcResponse *ipc_resp) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  *err_ckv = NULL;
  // Validating if VbridgeIf is a node for Vlink
  VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>
    (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
  if (!mgr) {
    UPLL_LOG_ERROR("Invalid mgr");
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return UPLL_RC_ERR_GENERIC;
  }
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);

  ConfigKeyVal *ck_vlink = NULL;
  ConfigKeyVal *ck_vif = NULL;
  vn_if_type iftype;
  result_code = GetChildConfigKey(ck_vif, req);

  result_code = mgr->CheckIfMemberOfVlink(ck_vif, UPLL_DT_CANDIDATE,
                                          ck_vlink, dmi, iftype);
  if (result_code == UPLL_RC_SUCCESS) {
    SET_USER_DATA_CTRLR(ck_vlink, ctrlr_dom.ctrlr);
    *err_ckv = ck_vlink;
    DELETE_IF_NOT_NULL(ck_vif);
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return result_code;
  } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_TRACE("Checking in running table");
    result_code = mgr->CheckIfMemberOfVlink(ck_vif, UPLL_DT_RUNNING,
                                            ck_vlink, dmi, iftype);
    DELETE_IF_NOT_NULL(ck_vif);
    if (result_code == UPLL_RC_SUCCESS) {
      SET_USER_DATA_CTRLR(ck_vlink, ctrlr_dom.ctrlr);
      *err_ckv = ck_vlink;
      DELETE_IF_NOT_NULL(ck_main);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
      return result_code;
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      SET_USER_DATA_CTRLR(ipc_resp->ckv_data, ctrlr_dom.ctrlr);
      DELETE_IF_NOT_NULL(ck_vlink);
      UPLL_LOG_DEBUG("Failed to map boundary if to vlink");
      result_code = UPLL_RC_SUCCESS;
    }
  } else {
    DELETE_IF_NOT_NULL(ck_vlink);
    return result_code;
  }
  DELETE_IF_NOT_NULL(ck_vlink);
  // Get the UNC key for the renamed controller key.
  result_code = GetRenamedUncKey(ipc_resp->ckv_data, dt_type, dmi,
                                 ctrlr_dom.ctrlr);
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_ERROR("GetRenamedUncKey failed %d", result_code);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return UPLL_RC_ERR_GENERIC;
  }
  // Convert driver response to VTN service response
  result_code = AdaptValToVtnService(ipc_resp->ckv_data, ADAPT_ONE);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_GENERIC) {
    // If no val structure, ignore error
    UPLL_LOG_ERROR("AdaptValToVtnService failed result_code %d",
                   result_code);
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
    return UPLL_RC_ERR_GENERIC;
  }

  SET_USER_DATA_CTRLR(ipc_resp->ckv_data, ctrlr_dom.ctrlr);
  *err_ckv = ipc_resp->ckv_data;

  DELETE_IF_NOT_NULL(ck_main);
  DELETE_IF_NOT_NULL(req);
  return result_code;
}

upll_rc_t VbrIfMoMgr::GetChildConvertConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_convert_vbr_if *vbr_convert_if_key = NULL;
  if (okey && (okey->get_key())) {
    vbr_convert_if_key = reinterpret_cast<key_convert_vbr_if_t *>
                (okey->get_key());
  } else {
    vbr_convert_if_key = reinterpret_cast<key_convert_vbr_if *>
      (ConfigKeyVal::Malloc(sizeof(key_convert_vbr_if)));
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyConvertVbrIf,
                              vbr_convert_if_key, NULL);
    else if (okey->get_key() != vbr_convert_if_key)
      okey->SetKey(IpctSt::kIpcStKeyConvertVbrIf, vbr_convert_if_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      free(vbr_convert_if_key);
    return UPLL_RC_ERR_GENERIC;
  }

  /* presumes MoMgrs receive only supported keytypes */
  switch (parent_key->get_key_type()) {
    case UNC_KT_VBR_PORTMAP:
      uuu::upll_strncpy(
          vbr_convert_if_key->convert_vbr_key.vbr_key.vtn_key.vtn_name,
         reinterpret_cast<key_vbr_portmap*>(pkey)->vbr_key.vtn_key.vtn_name,
        (kMaxLenVtnName+1));
      uuu::upll_strncpy(
          vbr_convert_if_key->convert_vbr_key.vbr_key.vbridge_name,
         reinterpret_cast<key_vbr_portmap*>(pkey)->vbr_key.vbridge_name,
         (kMaxLenVnodeName+1));
     break;
    case UNC_KT_VBRIDGE:
      if (parent_key->get_st_num() == IpctSt::kIpcStKeyConvertVbr) {
        uuu::upll_strncpy(
          vbr_convert_if_key->convert_vbr_key.conv_vbr_name,
         reinterpret_cast<key_convert_vbr*>(pkey)->conv_vbr_name,
        (kMaxLenConvertVnodeName+1));
        uuu::upll_strncpy(
          vbr_convert_if_key->convert_vbr_key.vbr_key.vtn_key.vtn_name,
         reinterpret_cast<key_convert_vbr*>(pkey)->vbr_key.vtn_key.vtn_name,
        (kMaxLenVtnName+1));
        uuu::upll_strncpy(
          vbr_convert_if_key->convert_vbr_key.vbr_key.vbridge_name,
         reinterpret_cast<key_convert_vbr *>(pkey)->vbr_key.vbridge_name,
        (kMaxLenVnodeName+1));

      } else {
        uuu::upll_strncpy(
          vbr_convert_if_key->convert_vbr_key.vbr_key.vtn_key.vtn_name,
         reinterpret_cast<key_vbr *>(pkey)->vtn_key.vtn_name,
        (kMaxLenVtnName+1));
      uuu::upll_strncpy(
          vbr_convert_if_key->convert_vbr_key.vbr_key.vbridge_name,
         reinterpret_cast<key_vbr *>(pkey)->vbridge_name,
        (kMaxLenVnodeName+1));
      }
     break;
    case UNC_KT_VBR_IF:
      if (parent_key->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
        uuu::upll_strncpy(vbr_convert_if_key->convert_if_name,
           reinterpret_cast<key_convert_vbr_if *>(pkey)->convert_if_name,
           (kMaxLenInterfaceName+1));
        uuu::upll_strncpy(vbr_convert_if_key->convert_vbr_key.conv_vbr_name,
           reinterpret_cast<key_convert_vbr_if *>
           (pkey)->convert_vbr_key.conv_vbr_name,
           (kMaxLenConvertVnodeName+1));
        uuu::upll_strncpy(vbr_convert_if_key->
                          convert_vbr_key.vbr_key.vbridge_name,
           reinterpret_cast<key_convert_vbr_if *>
           (pkey)->convert_vbr_key.vbr_key.vbridge_name,
           (kMaxLenVnodeName+1));
        uuu::upll_strncpy(vbr_convert_if_key->
                          convert_vbr_key.vbr_key.vtn_key.vtn_name,
            reinterpret_cast<key_convert_vbr_if *>
            (pkey)->convert_vbr_key.vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName+1));
      } else {
        uuu::upll_strncpy(vbr_convert_if_key->convert_if_name,
           reinterpret_cast<key_vbr_if *>(pkey)->if_name,
           (kMaxLenInterfaceName+1));
        uuu::upll_strncpy(vbr_convert_if_key->
                          convert_vbr_key.vbr_key.vbridge_name,
           reinterpret_cast<key_vbr_if *>
           (pkey)->vbr_key.vbridge_name,
           (kMaxLenVnodeName+1));
        uuu::upll_strncpy(vbr_convert_if_key->
                          convert_vbr_key.vbr_key.vtn_key.vtn_name,
            reinterpret_cast<key_vbr_if *>
            (pkey)->vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName+1));
      }
      break;
    default:
      break;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyConvertVbrIf,
                            vbr_convert_if_key, NULL);
  else if (okey->get_key() != vbr_convert_if_key)
    okey->SetKey(IpctSt::kIpcStKeyConvertVbrIf, vbr_convert_if_key);
  if (okey == NULL) {
    free(vbr_convert_if_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
  }
  return result_code;
}


upll_rc_t
VbrIfMoMgr::CreateConvertVbridgeInterfaceFromVlink(ConfigKeyVal *vlink_ckv,
                                                   ConfigKeyVal *vbr_pm_ckv,
                                                   IpcReqRespHeader *req,
                                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  ConfigKeyVal *convert_vbrif_ckv   = NULL;
  uint8_t      *vbridge_name = NULL;
  uint8_t      *if_name      = NULL;

  //  validate input param
  key_vlink_t *vlink_key = reinterpret_cast<key_vlink_t*>(
      vlink_ckv->get_key());
  if (!vlink_key) {
    UPLL_LOG_ERROR("vLink key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  // Get vLink controller and domain
  VlinkMoMgr *vlink_mgr = reinterpret_cast<VlinkMoMgr *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
  if (!vlink_mgr) {
    UPLL_LOG_INFO("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }

  controller_domain vlink_ctrlr_dom[2] = { { NULL, NULL }, { NULL, NULL } };
  upll_rc_t result_code = vlink_mgr->GetControllerDomainId(vlink_ckv,
                                                           vlink_ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to get vlink controller and domain");
    return UPLL_RC_SUCCESS;
  }

  // If vLink node is unified vBridge interface corresponding controller
  // and domain is '#' in vLink.
  if ((!strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[0].ctrlr), "#")) &&
      (!strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[0].domain), "#"))) {
    vbridge_name = reinterpret_cast<val_vlink_t*>(GetVal(
                              vlink_ckv))->vnode1_name;
    if_name      = reinterpret_cast<val_vlink_t*>(GetVal(
                              vlink_ckv))->vnode1_ifname;
  } else {
    vbridge_name = reinterpret_cast<val_vlink_t*>(GetVal(
                              vlink_ckv))->vnode2_name;
    if_name      = reinterpret_cast<val_vlink_t*>(GetVal(
                              vlink_ckv))->vnode2_ifname;
  }
  if (!vbridge_name || !if_name) {
    UPLL_LOG_ERROR("Failed to get vbridge_name and if_name from vlink");
    return UPLL_RC_ERR_GENERIC;
  }

  //  Get converted vBridge name based on vtn_name,vbridge_name,
  //  controller_name and domain_name

  //  Populate convert_vbr_ckv for vBridge converttbl ReadConfigDb
  key_convert_vbr *convert_vbr_key = reinterpret_cast<key_convert_vbr *>
    (ConfigKeyVal::Malloc(sizeof(key_convert_vbr)));
  uuu::upll_strncpy(convert_vbr_key->vbr_key.vtn_key.vtn_name,
      vlink_key->vtn_key.vtn_name, (kMaxLenVtnName + 1));
  uuu::upll_strncpy(convert_vbr_key->vbr_key.vbridge_name,
      vbridge_name, (kMaxLenVnodeName + 1));
  ConfigKeyVal *convert_vbr_ckv =  new ConfigKeyVal(UNC_KT_VBRIDGE,
                       IpctSt::kIpcStKeyConvertVbr, convert_vbr_key, NULL);
  controller_domain_t ctrlr_dom = { NULL, NULL };
  GET_USER_DATA_CTRLR_DOMAIN(vbr_pm_ckv, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(convert_vbr_ckv, ctrlr_dom);

  VbrMoMgr *vbr_mgr = reinterpret_cast<VbrMoMgr *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));
  if (!vbr_mgr) {
    UPLL_LOG_INFO("Invalid Mgr");
    DELETE_IF_NOT_NULL(convert_vbr_ckv);
    return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutNone };
  result_code = vbr_mgr->ReadConfigDB(convert_vbr_ckv, UPLL_DT_CANDIDATE,
                                      UNC_OP_READ, dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to get converted vBridge name");
    DELETE_IF_NOT_NULL(convert_vbr_ckv);
    return result_code;
  }

  //  Populate convert vbr_if_ckv to create unified vBridge interface in
  //  corresponding converted vBridge
  key_convert_vbr_if *convert_vbrif_key = reinterpret_cast<key_convert_vbr_if *>
    (ConfigKeyVal::Malloc(sizeof(key_convert_vbr_if)));
  uuu::upll_strncpy(convert_vbrif_key->convert_vbr_key.vbr_key.vtn_key.vtn_name,
      vlink_key->vtn_key.vtn_name, (kMaxLenVtnName + 1));
  uuu::upll_strncpy(convert_vbrif_key->convert_vbr_key.vbr_key.vbridge_name,
      vbridge_name, (kMaxLenVnodeName + 1));
  uuu::upll_strncpy(convert_vbrif_key->convert_vbr_key.conv_vbr_name,
      convert_vbr_key->conv_vbr_name, (kMaxLenConvertVnodeName + 1));
  uuu::upll_strncpy(convert_vbrif_key->convert_if_name, if_name,
                    (kMaxLenInterfaceName+1));

  convert_vbrif_ckv =  new ConfigKeyVal(UNC_KT_VBR_IF,
                       IpctSt::kIpcStKeyConvertVbrIf, convert_vbrif_key, NULL);
  SET_USER_DATA_CTRLR_DOMAIN(convert_vbrif_ckv,  ctrlr_dom);

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string cfg_vtn_name = "";
  //  Get acquired config mode
  result_code = GetConfigModeInfo(req, config_mode, cfg_vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    DELETE_IF_NOT_NULL(convert_vbrif_ckv);
    DELETE_IF_NOT_NULL(convert_vbr_ckv);
    return result_code;
  }

  //  create vBridge interfcae in converttbl
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone,
    kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
  SET_USER_DATA_FLAGS(convert_vbrif_ckv, BOUNDARY_UVBRIF_FLAG);
  result_code = UpdateConfigDB(convert_vbrif_ckv, UPLL_DT_CANDIDATE,
                               UNC_OP_CREATE, dmi, &dbop1, config_mode,
                               cfg_vtn_name, CONVERTTBL);
  DELETE_IF_NOT_NULL(convert_vbrif_ckv);
  DELETE_IF_NOT_NULL(convert_vbr_ckv);

  return result_code;
}

upll_rc_t
VbrIfMoMgr::DeleteConvertVbridgeInterfaceFromVlink(ConfigKeyVal *vlink_ckv,
                                                   ConfigKeyVal *vbr_pm_ckv,
                                                   IpcReqRespHeader *req,
                                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  ConfigKeyVal *convert_vbrif_ckv   = NULL;
  uint8_t      *vbridge_name = NULL;
  uint8_t      *if_name      = NULL;


  key_vlink_t *vlink_key = reinterpret_cast<key_vlink_t*>(vlink_ckv->get_key());
  if (!vlink_key) {
    UPLL_LOG_ERROR("vLink key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  controller_domain vlink_ctrlr_dom[2] = { { NULL, NULL }, { NULL, NULL } };
  VlinkMoMgr *vlink_mgr = reinterpret_cast<VlinkMoMgr *>(
      const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
  upll_rc_t result_code =
      vlink_mgr->GetControllerDomainId(vlink_ckv, vlink_ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to get vlink controller and domain");
    return UPLL_RC_SUCCESS;
  }

  // If vLink node is unified vBridge interface corresponding controller
  // and domain is '#' in vLink.
  if ((!strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[0].ctrlr), "#")) &&
      (!strcmp(reinterpret_cast<char *>(vlink_ctrlr_dom[0].domain), "#"))) {
    vbridge_name = reinterpret_cast<val_vlink_t*>(GetVal(
                              vlink_ckv))->vnode1_name;
    if_name      = reinterpret_cast<val_vlink_t*>(GetVal(
                              vlink_ckv))->vnode1_ifname;
  } else {
    vbridge_name = reinterpret_cast<val_vlink_t*>(GetVal(
                              vlink_ckv))->vnode2_name;
    if_name      = reinterpret_cast<val_vlink_t*>(GetVal(
                              vlink_ckv))->vnode2_ifname;
  }
  if (!vbridge_name || !if_name) {
    UPLL_LOG_ERROR("Failed to get vbridge_name and if_name from vlink");
    return UPLL_RC_ERR_GENERIC;
  }

  // Populate convert vbr_if_ckv to delete unified vBridge
  // interface in converttbl
  key_convert_vbr_if *convert_vbrif_key = reinterpret_cast<key_convert_vbr_if *>
    (ConfigKeyVal::Malloc(sizeof(key_convert_vbr_if)));
  uuu::upll_strncpy(convert_vbrif_key->convert_vbr_key.vbr_key.vtn_key.vtn_name,
      vlink_key->vtn_key.vtn_name, (kMaxLenVtnName + 1));
  uuu::upll_strncpy(convert_vbrif_key->convert_vbr_key.vbr_key.vbridge_name,
      vbridge_name, (kMaxLenVnodeName + 1));
  uuu::upll_strncpy(convert_vbrif_key->convert_if_name, if_name,
                    (kMaxLenInterfaceName+1));

  convert_vbrif_ckv =  new ConfigKeyVal(UNC_KT_VBR_IF,
                       IpctSt::kIpcStKeyConvertVbrIf, convert_vbrif_key, NULL);
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  GET_USER_DATA_CTRLR_DOMAIN(vbr_pm_ckv, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(convert_vbrif_ckv, ctrlr_dom);

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string cfg_vtn_name = "";
  //  Get acquired config mode
  result_code = GetConfigModeInfo(req, config_mode, cfg_vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    DELETE_IF_NOT_NULL(convert_vbrif_ckv);
    return result_code;
  }

  DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone };

  //  delete unified vBridge interface in converttbl
  result_code = UpdateConfigDB(convert_vbrif_ckv, UPLL_DT_CANDIDATE,
                               UNC_OP_DELETE, dmi, &dbop, config_mode,
                               cfg_vtn_name, CONVERTTBL);

  DELETE_IF_NOT_NULL(convert_vbrif_ckv);
  return result_code;
}

upll_rc_t VbrIfMoMgr::ConvertVbrIf(
    DalDmlIntf *dmi, bool match_ctrlr_dom, ConfigKeyVal *ikey,
    TcConfigMode config_mode, string vtn_name, unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // validate the input
  if (!ikey || !dmi || !ikey->get_key()) {
    UPLL_LOG_DEBUG("IpcReqRespHeader or ConfigKeyVal or DalDmlIntf is Null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (UNC_KT_VBR_IF != ikey->get_key_type()) {
    UPLL_LOG_DEBUG("Invalid keytype. Keytype- %d", ikey->get_key_type());
    return UPLL_RC_ERR_GENERIC;
  }

  if (ikey->get_st_num() != IpctSt::kIpcStKeyConvertVbrIf) {
    UPLL_LOG_DEBUG("Invalid struct received.");
    return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain|
                                             kOpInOutFlag};
  // TODO(U17+) if convert_vbr_if needs to applied to controller, then need to
  // revisit the flag settting
  // indicates conversion happend for vbr_portmap creation
  uint8_t flag = 0;
  flag |= USER_VBRPM_IF_FLAG;
  SET_USER_DATA_FLAGS(ikey, flag);

  if (op == UNC_OP_DELETE) {
    dbop.inoutop = kOpInOutNone;
    dbop.matchop = kOpMatchFlag;
    // when convert_vbr_if created for boundary vlink, UNIFIED_VBRIF_FLAG is
    // set, so dont delete that vbr_if in this path
    if (match_ctrlr_dom) {
      dbop.matchop = kOpMatchCtrlr|kOpMatchDomain|kOpMatchFlag;
    }
  }
  // Insert/delete the entry to database
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, op, dmi, &dbop,
                               config_mode, vtn_name, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("UpdateConfigDB failed, %d", result_code);
  }
  return result_code;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
