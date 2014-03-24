/*
 * Copyright (c) 2012-2014 NEC Corporation
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
#if 0
namespace upll_dal_vbrif unc::upll::dal::schema::table::vbridge_interface;
#endif
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

unc_key_type_t VbrIfMoMgr::vbr_if_child[] = { UNC_KT_VBRIF_FLOWFILTER,
  UNC_KT_VBRIF_POLICINGMAP };


VbrIfMoMgr::VbrIfMoMgr() {
  UPLL_FUNC_TRACE;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable];
  table[MAINTBL] = new Table(uudst::kDbiVbrIfTbl,
                             UNC_KT_VBR_IF,
                             vbr_if_bind_info,
                             IpctSt::kIpcStKeyVbrIf,
                             IpctSt::kIpcStValVbrIf,
                             uudst::vbridge_interface::kDbiVbrIfNumCols);
  table[CTRLRTBL] = NULL;
  table[RENAMETBL] = NULL;
  nchild = sizeof(vbr_if_child) / sizeof(*vbr_if_child);
  child = vbr_if_child;
}
/*
 * Based on the key type the bind info will pass
 **/

bool VbrIfMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                      BindInfo *&binfo,
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
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain };
  result_code = ReadConfigDB(temp_ck, req->datatype, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code);
    delete temp_ck;
    return result_code;
  }
  GET_USER_DATA_CTRLR_DOMAIN(temp_ck, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
#endif
  result_code = ValidateCapability(
      req, ikey, reinterpret_cast<char *>(ctrlr_dom.ctrlr));
  if (UPLL_RC_SUCCESS  != result_code) {
    UPLL_LOG_DEBUG("Validate Capability is Failed. Error_code : %d",
                   result_code);
    delete temp_ck;
    return result_code;
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

  result_code = ValidateAttribute(okey, dmi, req);
  if (UPLL_RC_SUCCESS  != result_code) {
    delete okey;
    UPLL_LOG_ERROR("Validate Attribute is Failed");
    return result_code;
  }

  result_code = UpdateConfigVal(okey, req->datatype, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    delete okey;
    UPLL_LOG_DEBUG("UpdateConfigVal is Failed");
    return result_code;
  }
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  UPLL_LOG_DEBUG("The okey Structue before update  %s",
                 (okey->ToStrAll()).c_str());
  result_code = UpdateConfigDB(okey, req->datatype, UNC_OP_UPDATE,
                               dmi, &dbop1, MAINTBL);
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
  ConfigKeyVal *ck_ifkey = NULL;
  ConfigKeyVal *ck_vlink = NULL;
  result_code = GetChildConfigKey(ck_ifkey, ikey);
  if (result_code != UPLL_RC_SUCCESS || ck_ifkey == NULL) {
    UPLL_LOG_INFO("GetChildConfigKey failed err_code %d", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  vn_if_type iftype;
  VlinkMoMgr *mgr =
      reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager*>(GetMoManager(
                  UNC_KT_VLINK)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->CheckIfMemberOfVlink(ck_ifkey, UPLL_DT_RUNNING,
                                          ck_vlink, dmi, iftype);
  if ((result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ||
      ((iftype == kVlinkInternalNode1) || (iftype == kVlinkInternalNode2))) {
    delete ck_ifkey;
    if (ck_vlink) delete ck_vlink;
    UPLL_LOG_DEBUG("Internal link interface");
    result_code = VnodeChildMoMgr::CreateAuditMoImpl(ikey, dmi, ctrlr_id);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Create Audit Vbrif failed %s",
                    (ikey->ToStrAll()).c_str());
    }
    return result_code;
  } else if (result_code != UPLL_RC_SUCCESS) {
    delete ck_ifkey;
    if (ck_vlink) delete ck_vlink;
    UPLL_LOG_INFO("Error in reading vlink key %d", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ck_vlink));
  if (vlink_val == NULL) return UPLL_RC_ERR_GENERIC;
  uint8_t valid_boundary = vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
  // store VALID_NO_VAL in running only in case of boundary link
  if ((valid_boundary == UNC_VF_VALID_NO_VALUE) ||
      (valid_boundary == UNC_VF_VALID)) {
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutFlag};
    result_code = ReadConfigDB(ck_ifkey, UPLL_DT_RUNNING, UNC_OP_READ,
                               dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      delete ck_ifkey;
      UPLL_LOG_INFO("Retrieving a Record for VbrIf in RUNNING DB failed");
      return result_code;
    }
    void *db_val, *drv_val = GetVal(ikey);
    db_val =  GetVal(ck_ifkey);
    if (!db_val || !drv_val) {
      delete ck_ifkey;
      return UPLL_RC_ERR_GENERIC;
    }

    // validate params of running against those received from driver
    val_port_map vbr_db_portmap = (reinterpret_cast<val_drv_vbr_if*>
                                   (db_val))->vbr_if_val.portmap;
    val_port_map *vbr_drv_portmap = &((reinterpret_cast<val_drv_vbr_if*>
                                       (drv_val))->vbr_if_val.portmap);
    bool portid_equal = (vbr_db_portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
                         vbr_drv_portmap->valid[UPLL_IDX_LOGICAL_PORT_ID_PM]);

    // to be uncommented when driver stops translating from no-vlan-id
    // to 65535 as it
    // cannot distingish between boundary vlinked / regular mapped vbrifs
    if (vbr_drv_portmap->valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_INVALID) {
      vbr_drv_portmap->vlan_id =  0xFFFF;
      vbr_drv_portmap->valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
    }

    if (portid_equal &&
        (vbr_db_portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] == UNC_VF_VALID)) {
      portid_equal = (strcmp(reinterpret_cast<char*>
                             (vbr_db_portmap.logical_port_id),
                             reinterpret_cast<char*>
                             (vbr_drv_portmap->logical_port_id)) == 0);
    }
    bool vlanid_equal = (vbr_db_portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
                         vbr_drv_portmap->valid[UPLL_IDX_VLAN_ID_PM]);
    if (vlanid_equal &&
        (vbr_db_portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID)) {
      vlanid_equal = (vbr_db_portmap.vlan_id == vbr_drv_portmap->vlan_id);
    }
    bool tagged_equal = (vbr_db_portmap.valid[UPLL_IDX_TAGGED_PM] ==
                         vbr_drv_portmap->valid[UPLL_IDX_TAGGED_PM]);
    if (tagged_equal &&
        (vbr_db_portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID)) {
      tagged_equal = (vbr_db_portmap.tagged == vbr_drv_portmap->tagged);
    }

    // create boundary vlink
    if (portid_equal && vlanid_equal && tagged_equal) {
      result_code = mgr->UpdateConfigDB(ck_vlink, UPLL_DT_AUDIT,
                                        UNC_OP_CREATE, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        delete ck_ifkey;
        delete ck_vlink;
        UPLL_LOG_INFO("Retrieving a Record for VbrIf in RUNNING DB failed");
        return result_code;
      }
    } else {
      UPLL_LOG_INFO("Boundary data does not match");
    }
  }
  result_code = VnodeChildMoMgr::CreateAuditMoImpl(ikey, dmi, ctrlr_id);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Create Audit Vbrif failed %s", (ikey->ToStrAll()).c_str());
    return UPLL_RC_ERR_GENERIC;
  }
  delete ck_ifkey;
  delete ck_vlink;
  return UPLL_RC_SUCCESS;
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

  val_drv_vbr_if *vbr_drv_if_val = reinterpret_cast<val_drv_vbr_if *>
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
  result_code = UpdateConfigDB(ck_drv_vbr_if,
                               UPLL_DT_AUDIT,
                               UNC_OP_CREATE,
                               dmi,
                               MAINTBL);
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
  ConfigKeyVal *ckv_rvbrif = NULL;;
  upll_rc_t result_code = GetChildConfigKey(ckv_rvbrif, ck_port_map);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetChilConfigKey Failed");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain, kOpInOutFlag};
  result_code = ReadConfigDB(ckv_rvbrif,
                             UPLL_DT_RUNNING,
                             UNC_OP_READ,
                             dbop,
                             dmi,
                             MAINTBL);
  bool port_map_in_run = false;
  val_drv_vbr_if *drv_vbr_if_val = reinterpret_cast<val_drv_vbr_if *>
      (GetVal(ck_port_map));
  if (result_code == UPLL_RC_SUCCESS) {
    val_drv_vbr_if *drv_rifval = reinterpret_cast<val_drv_vbr_if *>
        (GetVal(ckv_rvbrif));
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
      uuu::upll_strncpy(reinterpret_cast<char *>
                        (drv_vbr_if_val->vex_link_name),
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
  }
  return UPLL_RC_SUCCESS;
}

bool VbrIfMoMgr::IsValidKey(void *key, uint64_t index) {
  UPLL_FUNC_TRACE;
  key_vbr_if *if_key = reinterpret_cast<key_vbr_if *>(key);
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
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
  key_vbr_if *pkey = reinterpret_cast<key_vbr_if *>
      (ikey->get_key());
  if (!pkey) return UPLL_RC_ERR_GENERIC;
  key_vbr *vbr_key = reinterpret_cast<key_vbr *>(ConfigKeyVal::Malloc
                                                 (sizeof(key_vbr)));
  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                    reinterpret_cast<key_vbr_if *>
                    (pkey)->vbr_key.vtn_key.vtn_name,
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy(vbr_key->vbridge_name,
                    reinterpret_cast<key_vbr_if *>(pkey)->vbr_key.vbridge_name,
                    (kMaxLenVnodeName+1));
  if (okey) delete okey;
  okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr, vbr_key, NULL);
  if (okey == NULL) {
    free(vbr_key);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, ikey);
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
      void *oval;
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
    }
    tmp = tmp->get_next_cfg_val();
  }
  if (tmp) {
    if (tbl == MAINTBL) {
      void *ovalst;
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
      tmp1->AppendCfgVal(tmp2);
    }
  }
  void *tkey = (req)->get_key();
  key_vbr_if *ikey = reinterpret_cast<key_vbr_if *>(tkey);
  key_vbr_if *vbr_if_key = reinterpret_cast<key_vbr_if *>(ConfigKeyVal::Malloc(
          sizeof(key_vbr_if)));
  if (!vbr_if_key) {
    UPLL_LOG_DEBUG(" Memory allocation failed");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  memcpy(vbr_if_key, ikey, sizeof(key_vbr_if));
  okey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, vbr_if_key,
                          tmp1);
  if (okey) {
    SET_USER_DATA(okey, req);
  } else {
    DELETE_IF_NOT_NULL(tmp1);
    FREE_IF_NOT_NULL(vbr_if_key);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                         unc_keytype_operation_t op,
                                         uint32_t driver_result,
                                         ConfigKeyVal *upd_key,
                                         DalDmlIntf *dmi,
                                         ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_drv_vbr_if_t *vbrif_val = reinterpret_cast<val_drv_vbr_if_t *>
      (GetVal(ikey));

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  if (vbrif_val == NULL) return UPLL_RC_ERR_GENERIC;
  val_vbr_if_t *vbr_if_val = &vbrif_val->vbr_if_val;
  bool port_map_change = false;
  switch (op) {
    case UNC_OP_UPDATE:
      {
        void *val = reinterpret_cast<void *>(vbrif_val);
        val_drv_vbr_if_t *vbrif_val2 = reinterpret_cast<val_drv_vbr_if_t *>
            (GetVal(upd_key));
#if 0
        if (vbr_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID
            && vbrif_val2->vbr_if_val.valid[UPLL_IDX_PM_VBRI] != UNC_VF_VALID)
          port_map_change = true;
#else
        if (vbr_if_val->valid[UPLL_IDX_PM_VBRI] !=
            vbrif_val2->vbr_if_val.valid[UPLL_IDX_PM_VBRI])
          port_map_change = true;
#endif
        CompareValidValue(val, GetVal(upd_key), true);
        if (vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] != UNC_VF_INVALID)
          port_map_change = true;
        else if (vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
                 != UNC_VF_INVALID)
          port_map_change = true;
        uint8_t cand_flag = 0, run_flag = 0;
        GET_USER_DATA_FLAGS(ikey, cand_flag);
        GET_USER_DATA_FLAGS(upd_key, run_flag);
        if ((cand_flag & VIF_TYPE) != (run_flag & VIF_TYPE))
          port_map_change = true;
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
        val_db_vbr_if_st *vbr_if_valst = reinterpret_cast<val_db_vbr_if_st *>
            (ConfigKeyVal::Malloc(sizeof(val_db_vbr_if_st)));
        ikey->AppendCfgVal(IpctSt::kIpcStValVbrIfSt, vbr_if_valst);
        UPLL_LOG_DEBUG("valid %d  admin %d op %d",
                       vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI],
                       vbr_if_val->admin_status, op);
#if 0
        if (vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] != UNC_VF_INVALID) {
          val_vbr_if_st *vnif_st = &(reinterpret_cast<val_db_vbr_if_st  *>
                                     (GetStateVal(ikey))->vbr_if_val_st);
          vnif_st->oper_status = UPLL_OPER_STATUS_UNINIT;
        }
        upll_rc_t result_code = InitOperStatus<val_vbr_if_st, val_db_vbr_if_st>
            (ikey, vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI],
             vbr_if_val->admin_status,
             vbr_if_val->valid[UPLL_IDX_PM_VBRI],
             &vbr_if_val->portmap);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error settiing oper status");
          return UPLL_RC_ERR_GENERIC;
        }
#else
        val_vbr_if_st *vnif_st = &(reinterpret_cast<val_db_vbr_if_st  *>
                                   (GetStateVal(ikey))->vbr_if_val_st);
        vnif_st->oper_status = UPLL_OPER_STATUS_UNINIT;
#if 0
        (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED)?
            UPLL_OPER_STATUS_UNKNOWN:UPLL_OPER_STATUS_UNINIT;
#endif
        vnif_st->valid[UPLL_IDX_OPER_STATUS_VRTS] = UNC_VF_VALID;
#endif
        if (op == UNC_OP_CREATE) {
          vbr_if_valst->down_count = 0;
        } else {
          val_db_vbr_if_st *run_vbrifst = reinterpret_cast<val_db_vbr_if_st *>
              (GetStateVal(upd_key));
          vbr_if_valst->down_count = (run_vbrifst ? run_vbrifst->down_count:0);
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
  return UPLL_RC_SUCCESS;
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
    vbr_if_val = reinterpret_cast<val_vbr_if *>
        (ikey->get_cfg_val()->get_val());
  } else if ((ikey->get_cfg_val()) &&
             ((ikey->get_cfg_val())->get_st_num() ==
              IpctSt::kIpcStValVtnNeighbor)) {
    vtn_neighbor =  reinterpret_cast<val_vtn_neighbor *>
        (ikey->get_cfg_val()->get_val());
  } else if ((ikey->get_cfg_val()) &&
             ((ikey->get_cfg_val())->get_st_num() ==
              IpctSt::kIpcStPfcdrvValVbrIf)) {
    vbr_if_val = &(reinterpret_cast<val_drv_vbr_if *>(
            ikey->get_cfg_val()->get_val())->vbr_if_val);
  } else if ((ikey->get_cfg_val()) &&
             (((ikey->get_cfg_val())->get_st_num() !=
               IpctSt::kIpcStValVbrIf) ||
              ((ikey->get_cfg_val())->get_st_num() !=
               IpctSt::kIpcStValVtnNeighbor) ||
              ((ikey->get_cfg_val())->get_st_num() !=
               IpctSt::kIpcStPfcdrvValVbrIf))) {
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
      UPLL_LOG_DEBUG("Val struct Validation is Mandatory"
                     "for UPDATE operation");
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
      UPLL_LOG_DEBUG("Val struct Validation is an optional"
                     "for READ operation");
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
        UPLL_LOG_DEBUG("Val struct Validation is an optional"
                       "for READ operation");
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
  } else if ((vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI]
              == UNC_VF_VALID_NO_VALUE)
             && (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
    vbr_if_val->admin_status = UPLL_ADMIN_ENABLE;
  } else if ((vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_INVALID)
             && (operation == UNC_OP_CREATE)) {
    vbr_if_val->admin_status = UPLL_ADMIN_ENABLE;
    vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID_NO_VALUE;
  }
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
      vbr_if_val->portmap.vlan_id = 0;
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
    } else if (((vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM]
                 == UNC_VF_VALID_NO_VALUE) ||
                (vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM]
                 == UNC_VF_INVALID))
               && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
      if (vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
        vbr_if_val->portmap.tagged = UPLL_VLAN_TAGGED;
      } else {
        vbr_if_val->portmap.tagged = UPLL_VLAN_UNTAGGED;
      }
      vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_VALID;
    }
  } else if ((vbr_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE)
             && (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
    memset(&(vbr_if_val->portmap), 0, sizeof(vbr_if_val->portmap));
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
      result_code = GetCreateCapability(ctrlr_name,
                                        ikey->get_key_type(),
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
  }
  result_code = IsLogicalPortAndVlanIdInUse(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Returning error %d\n", result_code);
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
        vbr_if_val->vbr_if_val.valid[UPLL_IDX_DESC_VBRI] =
            UNC_VF_NOT_SUPPORTED;
        UPLL_LOG_INFO("Description attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((vbr_if_val->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] ==
         UNC_VF_VALID)
        ||(vbr_if_val->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] ==
           UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_if::kCapAdminStatus] == 0) {
        vbr_if_val->vbr_if_val.valid[UPLL_IDX_DESC_VBRI] =
            UNC_VF_NOT_SUPPORTED;
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

bool VbrIfMoMgr::FilterAttributes(void *&val1,
                                  void *val2,
                                  bool copy_to_running,
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
  if ((if1->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] == UNC_VF_VALID_NO_VALUE) &&
      (if1->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] == UNC_VF_VALID_NO_VALUE) &&
      (if1->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] == UNC_VF_VALID_NO_VALUE)) {
    if1->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] =
        (copy_to_running)?UNC_VF_VALID_NO_VALUE:UNC_VF_VALID;
    if1->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] =
        (copy_to_running)?UNC_VF_VALID_NO_VALUE:UNC_VF_VALID;
    if1->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] =
        (copy_to_running)?UNC_VF_VALID_NO_VALUE:UNC_VF_VALID;
  } else {
    if ((UNC_VF_VALID == if1->valid[PFCDRV_IDX_VEXT_NAME_VBRIF])
        && (UNC_VF_VALID == if2->valid[PFCDRV_IDX_VEXT_NAME_VBRIF]))
      if (!strcmp (reinterpret_cast<char *>(if1->vex_name),
                   reinterpret_cast<const char*>(if2->vex_name)))
        if1->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] =
            (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
    if ((UNC_VF_VALID == if1->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF])
        && (UNC_VF_VALID == if2->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF]))
      if (!strcmp (reinterpret_cast<char *>(if1->vex_if_name),
                   reinterpret_cast<const char*>(if2->vex_if_name)))
        if1->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] =
            (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
    if ((UNC_VF_VALID == if1->valid[PFCDRV_IDX_VLINK_NAME_VBRIF])
        && (UNC_VF_VALID == if2->valid[PFCDRV_IDX_VLINK_NAME_VBRIF]))
      if (!strcmp (reinterpret_cast<char *>(if1->vex_link_name),
                   reinterpret_cast<const char*>(if2->vex_link_name)))
        if1->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] =
            (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  }
  for (unsigned int loop = 0;
       loop < sizeof(val_vbr_if1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vbr_if1->portmap.valid[loop]
        && UNC_VF_VALID == val_vbr_if2->portmap.valid[loop])
      val_vbr_if1->portmap.valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if (UNC_VF_INVALID != val_vbr_if1->valid[UPLL_IDX_DESC_VBRI]) {
    if (!copy_to_running ||
        ((UNC_VF_VALID == val_vbr_if1->valid[UPLL_IDX_DESC_VBRI]) &&
         (!strcmp(reinterpret_cast<char *>(val_vbr_if1->description),
                   reinterpret_cast<const char*>(val_vbr_if2->description)))))
      val_vbr_if1->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
  }
#if 1
  // admin state val is needed to determine oper status
  if (val_vbr_if1->admin_status == val_vbr_if2->admin_status)
    val_vbr_if1->valid[UPLL_IDX_ADMIN_STATUS_VBRI] =
        (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
  /* Driver cannot shutdown an unmapped vbrif */
  if (!copy_to_running && val_vbr_if1->valid[UPLL_IDX_PM_VBRI] ==
      UNC_VF_INVALID)
    val_vbr_if1->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_INVALID;
#endif
  if (val_vbr_if1->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID
      && val_vbr_if2->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    if (memcmp(&(val_vbr_if1->portmap), &(val_vbr_if2->portmap),
               sizeof(val_port_map_t))) {
      if (val_vbr_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
          UNC_VF_VALID
          && val_vbr_if2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
          == UNC_VF_VALID) {
        if (!strcmp(reinterpret_cast<char *>
                    (val_vbr_if1->portmap.logical_port_id),
                    reinterpret_cast<char *>
                    (val_vbr_if2->portmap.logical_port_id)))
          val_vbr_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
              (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
      }
      if (val_vbr_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] != UNC_VF_INVALID
          && val_vbr_if2->portmap.valid[UPLL_IDX_VLAN_ID_PM] !=
          UNC_VF_INVALID) {
        if (val_vbr_if1->portmap.vlan_id == val_vbr_if2->portmap.vlan_id)
          val_vbr_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] =
              (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
      }
#if 1
      if (val_vbr_if1->portmap.valid[UPLL_IDX_TAGGED_PM] != UNC_VF_INVALID
          && val_vbr_if2->portmap.valid[UPLL_IDX_TAGGED_PM] !=
          UNC_VF_INVALID) {
        if (val_vbr_if1->portmap.tagged == val_vbr_if2->portmap.tagged)
          val_vbr_if1->portmap.valid[UPLL_IDX_TAGGED_PM] =
              (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
      }
#endif
    } else {
      val_vbr_if1->valid[UPLL_IDX_PM_VBRI] =
          (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
      val_vbr_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
          (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
      val_vbr_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] =
          (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
      val_vbr_if1->portmap.valid[UPLL_IDX_TAGGED_PM] =
          (copy_to_running)?UNC_VF_INVALID:UNC_VF_VALUE_NOT_MODIFIED;
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
              (UNC_VF_VALID_NO_VALUE == (uint8_t)
               val_vbr_if1->portmap.valid[i])) {
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


upll_rc_t VbrIfMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t dt_type,
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
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ,
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
    // check with karthik
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
                                      DalDmlIntf *dmi) {
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
  val_drv_vbr_if *val_drv_vbr = reinterpret_cast<val_drv_vbr_if *>
      (GetVal(ikey));
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
      UPLL_LOG_DEBUG("Interface is linked/bounded with Vlink."
                     "Could not update Portmap");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
    result_code = UpdatePortMap(okey, datatype, dmi, ikey);
  }
  DELETE_IF_NOT_NULL(okey);
  return result_code;
}

upll_rc_t VbrIfMoMgr::UpdatePortMap(ConfigKeyVal *okey,
                                    upll_keytype_datatype_t datatype,
                                    DalDmlIntf *dmi,
                                    ConfigKeyVal *ikey) {
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
  val_drv_vbr_if *val_drv_vbr = reinterpret_cast<val_drv_vbr_if *>
      (GetVal(ikey));
  bool port_map_status = (val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] ==
                          UNC_VF_VALID)?
      true:false;
  if (port_map_status) {
    val_drv_vbr_if *vbr_val_db = reinterpret_cast<val_drv_vbr_if *>
        (GetVal(okey));
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
    // TODO(karthi): any think inform to POM.. will call it from here
    /* Info to POM */
  }
  // Notify POM only when PortMap is created or Deleted
  if (PortMapNotificationVal == kPortMapCreated) {
    UPLL_LOG_DEBUG("Portmapstatus-true");
    result_code = mgr->SetVlinkPortmapConfiguration(okey,
                                                    datatype,
                                                    dmi,
                                                    kPortMapConfigured,
                                                    UNC_OP_CREATE);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
      return result_code;
    }
    result_code = pm_mgr->SetVlinkPortmapConfiguration(okey,
                                                       datatype,
                                                       dmi,
                                                       kPortMapConfigured,
                                                       UNC_OP_CREATE);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
      return result_code;
    }
  } else if (PortMapNotificationVal == kPortMapDeleted) {
    UPLL_LOG_DEBUG("Portmapstatus-false");
    result_code = mgr->SetVlinkPortmapConfiguration(okey,
                                                    datatype,
                                                    dmi,
                                                    kVlinkPortMapNotConfigured,
                                                    UNC_OP_DELETE);
    if (UPLL_RC_SUCCESS != result_code
        && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
      return result_code;
    }
    result_code = pm_mgr->SetVlinkPortmapConfiguration(
        okey,
        datatype,
        dmi,
        kVlinkPortMapNotConfigured,
        UNC_OP_DELETE);
    if (UPLL_RC_SUCCESS != result_code
        && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
      return result_code;
    }
  }
  result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE ==
                 result_code)?UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t VbrIfMoMgr::AdaptValToVtnService(ConfigKeyVal *ikey) {
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
        val_vbr_if_st *vbr_if_val_st = reinterpret_cast<val_vbr_if_st *>
            (ConfigKeyVal::Malloc(sizeof(val_vbr_if_st)));
        val_db_vbr_if_st *db_vbr_if_val_st = reinterpret_cast
            <val_db_vbr_if_st *>
            (cval->get_val());
        memcpy(vbr_if_val_st, &(db_vbr_if_val_st->vbr_if_val_st),
               sizeof(val_vbr_if_st));
        cval->SetVal(IpctSt::kIpcStValVbrIfSt, vbr_if_val_st);
      }
      cval = cval->get_next_cfg_val();
    }
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::GetVexternal(ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t dt_type,
                                   DalDmlIntf *dmi,
                                   uint8_t *vexternal, uint8_t *vex_if,
                                   InterfacePortMapInfo &iftype) {
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
      iftype = kPortMapConfigured;
      if (vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]) {
        if (!strlen(reinterpret_cast<const char *>(vbr_if_val->vex_name)))
          return UPLL_RC_ERR_GENERIC;
        uuu::upll_strncpy(vexternal, vbr_if_val->vex_name,
                          (kMaxLenInterfaceName + 1));
        if (!strlen(reinterpret_cast<const char *>(vbr_if_val->vex_if_name)))
          return UPLL_RC_ERR_GENERIC;
        uuu::upll_strncpy(vex_if, vbr_if_val->vex_if_name,
                          (kMaxLenInterfaceName + 1));
      }
    }
  } else {
    iftype = kVlinkPortMapNotConfigured;
    vexternal[0] = '\0';
    vex_if[0] = '\0';
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VbrIfMoMgr::AuditUpdateController(
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
  ConfigKeyVal  *ckv_running_db = NULL;
  ConfigKeyVal  *ckv_audit_db = NULL;
  ConfigKeyVal  *ckv_driver_req = NULL;
  ConfigKeyVal  *ckv_audit_dup_db = NULL;
  DalCursor *cursor = NULL;
  val_drv_vbr_if *drv_vbr_if = NULL;
  uint8_t *ctrlr = reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id));

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone};
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
      ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
       ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
  /* retreives the delta of running and audit configuration */
  UPLL_LOG_DEBUG("Operation is %d", op);
  result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT, op,
                             ckv_running_db, ckv_audit_db,
                             &cursor, dmi, ctrlr, MAINTBL, true);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("Skipping diff for opertaion %d", op);
    DELETE_IF_NOT_NULL(ckv_running_db);
    DELETE_IF_NOT_NULL(ckv_audit_db);
    return UPLL_RC_SUCCESS;;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
    DELETE_IF_NOT_NULL(ckv_running_db);
    DELETE_IF_NOT_NULL(ckv_audit_db);
    return result_code;
  }
  while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor))) {
    UPLL_LOG_DEBUG("Diff Running Record: Operation:  is  %d\n %s\n", op,
                   ckv_running_db->ToStrAll().c_str());
    /* ignore records of another controller for create and update operation */
    if (phase != uuc::kUpllUcpDelete) {
      uint8_t *db_ctrlr = NULL;
      GET_USER_DATA_CTRLR(ckv_running_db, db_ctrlr);
      UPLL_LOG_DEBUG("db ctrl_id and audit ctlr_id are  %s %s",
                     db_ctrlr,
                     ctrlr_id);
      if (db_ctrlr && strncmp(reinterpret_cast<const char *>(db_ctrlr),
                              reinterpret_cast<const char *>(ctrlr_id),
                              strlen(reinterpret_cast<const char *>
                                     (ctrlr_id)) + 1))
        continue;
    }
    switch (phase) {
      case uuc::kUpllUcpDelete:
        result_code = GetChildConfigKey(ckv_driver_req, ckv_running_db);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed. err_code & phase %d %d",
                         result_code, phase);
          return result_code;
        }
        if (ckv_driver_req->get_cfg_val()) {
          UPLL_LOG_DEBUG("Invalid param");
          return UPLL_RC_ERR_GENERIC;
        }
        result_code = ReadConfigDB(ckv_driver_req, UPLL_DT_AUDIT, UNC_OP_READ,
                                   dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d", result_code);
          return UPLL_RC_ERR_GENERIC;
        }
        drv_vbr_if = reinterpret_cast<val_drv_vbr_if_t *>
            (GetVal(ckv_driver_req));
        if (!drv_vbr_if) {
          UPLL_LOG_DEBUG("drv_vbr_if is NULL");
          return UPLL_RC_ERR_GENERIC;
        }
        if (drv_vbr_if->vbr_if_val.valid[UPLL_IDX_PM_VBRI] != UNC_VF_VALID) {
          ckv_driver_req->DeleteCfgVal();
        } else {
          drv_vbr_if->valid[0] = UNC_VF_INVALID;
          for (int i = 0; i < 3 ; i++) {
            if (drv_vbr_if->vbr_if_val.valid[i] != UNC_VF_INVALID) {
              drv_vbr_if->valid[0] = UNC_VF_VALID;
              break;
            }
          }
        }
        break;
      case uuc::kUpllUcpCreate:
        result_code = DupConfigKeyVal(ckv_driver_req, ckv_running_db, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed. err_code & phase %d %d",
                         result_code, phase);
          return result_code;
        }
        break;
      case uuc::kUpllUcpUpdate:
        UPLL_LOG_DEBUG("Diff audit record  is %s ",
                       ckv_audit_db->ToStrAll().c_str());
        result_code = DupConfigKeyVal(ckv_driver_req, ckv_running_db, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed for running record."
                         "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        result_code = DupConfigKeyVal(ckv_audit_dup_db, ckv_audit_db, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed for audit record."
                         "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        if (GetVal(ckv_driver_req) != NULL &&
            GetVal(ckv_audit_dup_db) != NULL) {
          void *val1 = GetVal(ckv_driver_req);
          if (FilterAttributes(val1, GetVal(ckv_audit_dup_db), false,
                               UNC_OP_UPDATE)) {
            delete ckv_driver_req;
            ckv_driver_req = NULL;
            delete ckv_audit_dup_db;
            ckv_audit_dup_db = NULL;
            // Assuming that the diff found only in ConfigStatus
            // Setting the   value as OnlyCSDiff in the out
            // parameter ctrlr_affected
            // The value Configdiff should be given more priority
            // than the value
            // onlycs .
            // So  If the out parameter ctrlr_affected has already value as
            // configdiff then dont change the value
            if (*ctrlr_affected != uuc::kCtrlrAffectedConfigDiff) {
              UPLL_LOG_INFO("Setting the ctrlr_affected to OnlyCSDiff");
              *ctrlr_affected = uuc::kCtrlrAffectedOnlyCSDiff;
            }
            continue;
          }
        }
        break;
      default:
        UPLL_LOG_DEBUG("Invalid operation %d", phase);
        return UPLL_RC_ERR_NO_SUCH_OPERATION;
        break;
    }
    if (phase == uuc::kUpllUcpUpdate || phase == uuc::kUpllUcpCreate) {
      drv_vbr_if = reinterpret_cast<val_drv_vbr_if_t *>
          (GetVal(ckv_driver_req));
      drv_vbr_if->valid[0] = UNC_VF_INVALID;
      for (int i = 0; i < 3 ; i++) {
        if (drv_vbr_if->vbr_if_val.valid[i] != UNC_VF_INVALID) {
          drv_vbr_if->valid[0] = UNC_VF_VALID;
          break;
        }
      }
    }
    GET_USER_DATA_CTRLR_DOMAIN(ckv_driver_req, ctrlr_dom);
    if ((NULL == ctrlr_dom.ctrlr) || (NULL == ctrlr_dom.domain)) {
      UPLL_LOG_INFO("controller id or domain is NULL");
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      DELETE_IF_NOT_NULL(ckv_driver_req);
      dmi->CloseCursor(cursor, true);
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                   ctrlr_dom.domain);
    upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
        UPLL_DT_AUDIT : UPLL_DT_RUNNING;
    result_code = GetRenamedControllerKey(ckv_driver_req, dt_type,
                                          dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
                     result_code);
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      DELETE_IF_NOT_NULL(ckv_driver_req);
      dmi->CloseCursor(cursor, true);
      return result_code;
    }

    IpcResponse ipc_response;
    memset(&ipc_response, 0, sizeof(IpcResponse));
    IpcRequest ipc_req;
    memset(&ipc_req, 0, sizeof(IpcRequest));
    ipc_req.header.clnt_sess_id = session_id;
    ipc_req.header.config_id = config_id;
    ipc_req.header.operation = op;
    ipc_req.header.datatype = UPLL_DT_CANDIDATE;
    ipc_req.ckv_data = ckv_driver_req;
    if (!IpcUtil::SendReqToDriver((const char *)ctrlr_dom.ctrlr,
                                  reinterpret_cast<char *>
                                  (ctrlr_dom.domain),
                                  PFCDRIVER_SERVICE_NAME,
                                  PFCDRIVER_SVID_LOGICAL,
                                  &ipc_req,
                                  true,
                                  &ipc_response)) {
      UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                    ckv_driver_req->get_key_type(), reinterpret_cast<char *>
                    (ctrlr_dom.ctrlr));
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      DELETE_IF_NOT_NULL(ckv_driver_req);
      dmi->CloseCursor(cursor, true);
      return UPLL_RC_ERR_GENERIC;
    }
    if (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("driver return failure err_code is %d",
                     ipc_response.header.result_code);
      result_code = AdaptValToVtnService(ckv_running_db);
      if (result_code != UPLL_RC_SUCCESS &&
          result_code != UPLL_RC_ERR_GENERIC) {
        UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                       result_code);
        DELETE_IF_NOT_NULL(ckv_running_db);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        DELETE_IF_NOT_NULL(ckv_driver_req);
        dmi->CloseCursor(cursor, true);
        delete ipc_response.ckv_data;
        return result_code;
      }
      *err_ckv = ckv_running_db;
      if (phase != uuc::kUpllUcpDelete) {
        ConfigKeyVal *resp = NULL;
        result_code = DupConfigKeyVal(resp, ipc_response.ckv_data);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed for ipc"
                         "response ckv err_code %d",
                         result_code);
          DELETE_IF_NOT_NULL(ckv_running_db);
          DELETE_IF_NOT_NULL(ckv_audit_db);
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(resp);
          *err_ckv = NULL;
          dmi->CloseCursor(cursor, true);
          delete ipc_response.ckv_data;
          return result_code;
        }
        result_code = UpdateAuditConfigStatus(UNC_CS_INVALID,
                                              phase,
                                              resp,
                                              dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Update Audit config status failed %d",
                         result_code);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(ckv_running_db);
          DELETE_IF_NOT_NULL(ckv_audit_db);
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(resp);
          *err_ckv = NULL;
          return result_code;
        }
        result_code = UpdateConfigDB(resp, UPLL_DT_RUNNING, UNC_OP_UPDATE,
                                     dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("UpdateConfigDB failed for ipc response"
                         "ckv err_code %d",
                         result_code);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(ckv_running_db);
          DELETE_IF_NOT_NULL(ckv_audit_db);
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(resp);
          *err_ckv = NULL;
          return result_code;
        }
        if (keytype == UNC_KT_VTN) {
          result_code = SetConsolidatedStatus(resp, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("SetConsolidatedStatus failed for ipc"
                           "response ckv err_code %d",
                           result_code);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(resp);
            *err_ckv = NULL;
            return result_code;
          }
        }
        DELETE_IF_NOT_NULL(resp);
      }
      return ipc_response.header.result_code;
    }
    DELETE_IF_NOT_NULL(ipc_response.ckv_data);
    DELETE_IF_NOT_NULL(ckv_driver_req);
    DELETE_IF_NOT_NULL(ckv_audit_dup_db);

    // *ctrlr_affected = true;
    if (*ctrlr_affected == uuc::kCtrlrAffectedOnlyCSDiff) {
      UPLL_LOG_INFO("Reset ctrlr state from OnlyCSDiff to ConfigDiff");
    }
    UPLL_LOG_DEBUG("Setting the ctrlr_affected to ConfigDiff");
    *ctrlr_affected = uuc::kCtrlrAffectedConfigDiff;
  }
  dmi->CloseCursor(cursor, true);
  if (uud::kDalRcSuccess != db_result) {
    UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d", db_result);
    result_code =  DalToUpllResCode(db_result);
  }
  DELETE_IF_NOT_NULL(ckv_running_db);
  DELETE_IF_NOT_NULL(ckv_audit_db);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      ? UPLL_RC_SUCCESS : result_code;
  return result_code;
}


upll_rc_t VbrIfMoMgr::TxUpdateController(unc_key_type_t keytype,
                                         uint32_t session_id,
                                         uint32_t config_id,
                                         uuc::UpdateCtrlrPhase phase,
                                         set<string> *affected_ctrlr_set,
                                         DalDmlIntf *dmi,
                                         ConfigKeyVal **err_ckv)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS, driver_result = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  IpcResponse ipc_resp;
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
      ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
       UNC_OP_DELETE);

  if (affected_ctrlr_set == NULL)
    return UPLL_RC_ERR_GENERIC;
  driver_result = DiffConfigDB(UPLL_DT_CANDIDATE,
                               UPLL_DT_RUNNING,
                               op,
                               req,
                               nreq,
                               &dal_cursor_handle,
                               dmi,
                               MAINTBL);
  while (driver_result == UPLL_RC_SUCCESS) {
    //  Get Next Record
    db_result = dmi->GetNextRecord(dal_cursor_handle);
    driver_result = DalToUpllResCode(db_result);
    if (driver_result != UPLL_RC_SUCCESS)
      break;
    switch (op)   {
      case UNC_OP_CREATE:
      case UNC_OP_UPDATE:
        /* fall through intended */
        result_code = DupConfigKeyVal(ck_main, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          delete req;
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        break;
      case UNC_OP_DELETE:
        result_code = GetChildConfigKey(ck_main, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("GetChildConfigKey failed %d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          delete req;
          return result_code;
        }
      default:
        break;
    }
    GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
    if (ctrlr_dom.ctrlr == NULL) {
      delete ck_main;
      delete req;
      DELETE_IF_NOT_NULL(nreq);
      return UPLL_RC_ERR_GENERIC;
    }
    uint8_t bound_vlink = 0;
    GET_USER_DATA_FLAGS(ck_main, bound_vlink);
    if (op == UNC_OP_DELETE) {
      if (ck_main->get_cfg_val()) {
        UPLL_LOG_DEBUG("Invalid param");
        delete ck_main;
        delete req;
        return UPLL_RC_ERR_GENERIC;
      }
      DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone};
      result_code = ReadConfigDB(ck_main, UPLL_DT_RUNNING, UNC_OP_READ,
                                 dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        delete ck_main;
        delete req;
        return UPLL_RC_ERR_GENERIC;
      }
      val_drv_vbr_if_t *val_vbr = reinterpret_cast<val_drv_vbr_if_t *>
          (GetVal(ck_main));
      if (val_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] != UNC_VF_VALID) {
        ck_main->DeleteCfgVal();
      }
    }
    if ((op == UNC_OP_CREATE) || (op == UNC_OP_UPDATE)) {
      void *main = GetVal(ck_main);
      void *val_nrec = (nreq) ? GetVal(nreq) : NULL;
      if (FilterAttributes(main, val_nrec, false, op)) {
        delete ck_main;
        ck_main = NULL;
        continue;
      }
      UPLL_LOG_TRACE("%s", ck_main->ToStrAll().c_str());
    }
    val_drv_vbr_if *vbr_ifval = reinterpret_cast<val_drv_vbr_if *>
        (GetVal(ck_main));
    if (vbr_ifval) {
      vbr_ifval->valid[0] = UNC_VF_INVALID;
      for (int i = 0; i < 3 ; i++) {
        if (vbr_ifval->vbr_if_val.valid[i] != UNC_VF_INVALID) {
          vbr_ifval->valid[0] = UNC_VF_VALID;
          break;
        }
      }
      // for (int i=1; i < 4 ; i++)
      //  vbr_ifval->valid[i] = vbr_ifval->vbr_if_val.valid[UPLL_IDX_PM_VBRI];
      switch (vbr_ifval->vbr_if_val.valid[UPLL_IDX_PM_VBRI]) {
        case UNC_VF_VALID_NO_VALUE :
          {
            UPLL_LOG_TRACE("entering valid-no-value");
            val_drv_vbr_if *oval_vbrif = reinterpret_cast<val_drv_vbr_if *>
                (GetVal(nreq));
            if (!oval_vbrif) {
              UPLL_LOG_DEBUG("Invalid param");
              delete req;
              DELETE_IF_NOT_NULL(nreq);
              DELETE_IF_NOT_NULL(ck_main);
              return UPLL_RC_ERR_GENERIC;
            }
            vbr_ifval->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
            vbr_ifval->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID;
            vbr_ifval->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID;
            uuu::upll_strncpy(vbr_ifval->vex_name, oval_vbrif->vex_name,
                              kMaxLenVnodeName+1);
            uuu::upll_strncpy(vbr_ifval->vex_if_name, oval_vbrif->vex_if_name,
                              kMaxLenVnodeName+1);
            uuu::upll_strncpy(vbr_ifval->vex_link_name,
                              oval_vbrif->vex_link_name,
                              kMaxLenVnodeName+1);
            break;
          }
        case UNC_VF_VALID:
          {
            // if it is update operation where only
            // the logicalport/vlanid/tag is getting updated
            // set vex/vexif/vexlink to invalid
            val_drv_vbr_if *db_ifval = NULL;
            db_ifval = (op != UNC_OP_UPDATE) ?
                reinterpret_cast<val_drv_vbr_if *>(GetVal(ck_main)):
                reinterpret_cast<val_drv_vbr_if *>(GetVal(nreq));
            if (!db_ifval) {
              UPLL_LOG_TRACE("Invalid param");
              delete req;
              DELETE_IF_NOT_NULL(nreq);
              DELETE_IF_NOT_NULL(ck_main);
              return UPLL_RC_ERR_GENERIC;
            }
            if (op == UNC_OP_DELETE) {
              if ((db_ifval->vbr_if_val.valid[UPLL_IDX_PM_VBRI] ==
                   UNC_VF_VALID) &&
                  (db_ifval->
                   vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
                   == UNC_VF_VALID)) {
                /* portmap is updated - fill in vex, vexlink -
                   vexif set to invalid for portmap update
                   */
                vbr_ifval->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
                uuu::upll_strncpy(vbr_ifval->vex_name, db_ifval->vex_name,
                                  kMaxLenVnodeName+1);
                vbr_ifval->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] =
                    UNC_VF_INVALID;
                vbr_ifval->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_VALID;
                uuu::upll_strncpy(vbr_ifval->vex_link_name,
                                  db_ifval->vex_link_name,
                                  kMaxLenVnodeName+1);
                if (vbr_ifval->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] ==
                    UNC_VF_VALID) {
                  if (vbr_ifval->
                      vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
                      UNC_VF_INVALID) {
                    vbr_ifval->vbr_if_val.portmap.vlan_id  =
                        db_ifval->vbr_if_val.portmap.vlan_id;
                    vbr_ifval->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] =
                        UNC_VF_VALID;
                  }
                }
              }
            }
            if (op == UNC_OP_UPDATE) {
              if (vbr_ifval->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] ==
                  UNC_VF_INVALID) {
                /* set port admin status to disable */
                if ((db_ifval->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] ==
                     UNC_VF_VALID) &&
                    (db_ifval->vbr_if_val.admin_status ==
                     UPLL_ADMIN_DISABLE)) {
                  vbr_ifval->vbr_if_val.admin_status = UPLL_ADMIN_DISABLE;
                  vbr_ifval->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] =
                      UNC_VF_VALID;
                }
              }
            }
            break;
          }
        default:
          break;
      }
    }
    UPLL_LOG_TRACE("%s", ck_main->ToStrAll().c_str());
    upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
        UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;
    if (!OVERLAY_KT(keytype)) {
      result_code = GetRenamedControllerKey(ck_main, dt_type,
                                            dmi, &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS)
        break;
    }
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                   ctrlr_dom.domain);
    // Inserting the controller to Set
    affected_ctrlr_set->insert
        (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));
    driver_result = SendIpcReq(session_id,
                               config_id,
                               op,
                               UPLL_DT_CANDIDATE,
                               ck_main,
                               &ctrlr_dom,
                               &ipc_resp);
    if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
      driver_result = UPLL_RC_SUCCESS;
      UPLL_LOG_DEBUG("Controller disconnected");
    }
    if (driver_result != UPLL_RC_SUCCESS) {
      ConfigKeyVal *ck_vlink = NULL;
      /* Validating if VbridgeIf is a node for Vlink */
      if ((bound_vlink & VIF_TYPE_BOUNDARY) == 0x0) {
        SET_USER_DATA_CTRLR(ipc_resp.ckv_data, ctrlr_dom.ctrlr);
        *err_ckv = ipc_resp.ckv_data;
      } else {
        VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>
            (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
        vn_if_type iftype;
        if (!mgr) {
          UPLL_LOG_DEBUG("Invalid mgr");
          delete req;
          DELETE_IF_NOT_NULL(nreq);
          return UPLL_RC_ERR_GENERIC;
        }
        ConfigKeyVal *ck_vif = NULL;
        result_code = GetChildConfigKey(ck_vif, req);
        result_code = mgr->CheckIfMemberOfVlink(ck_vif,
                                                UPLL_DT_CANDIDATE,
                                                ck_vlink,
                                                dmi,
                                                iftype);
        DELETE_IF_NOT_NULL(ck_vif);
        if (result_code == UPLL_RC_SUCCESS) {
          delete ipc_resp.ckv_data;
          SET_USER_DATA_CTRLR(ck_vlink, ctrlr_dom.ctrlr);
          *err_ckv = ck_vlink;
          break;
        } else  {
          SET_USER_DATA_CTRLR(ipc_resp.ckv_data, ctrlr_dom.ctrlr);
          *err_ckv = ipc_resp.ckv_data;
          if (ck_vlink) delete ck_vlink;
          UPLL_LOG_DEBUG("Failed to map boundary if to vlink");
        }
      }
      if (*err_ckv) {
        result_code = GetRenamedUncKey(*err_ckv, dt_type, dmi,
                                       ctrlr_dom.ctrlr);
        if (result_code != UPLL_RC_SUCCESS &&
            result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", result_code);
          delete req;
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }

        result_code = AdaptValToVtnService(*err_ckv);
        if (result_code != UPLL_RC_SUCCESS &&
            result_code != UPLL_RC_ERR_GENERIC) {
          // If no val structure, ignore error
          UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                         result_code);
          delete req;
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
      }
      break;
    }
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    if (ck_main)
      delete ck_main;
    ck_main = NULL;
  }
  dmi->CloseCursor(dal_cursor_handle, true);
  if (req)
    delete req;
  if (nreq)
    delete nreq;
  driver_result = (driver_result ==
                   UPLL_RC_ERR_NO_SUCH_INSTANCE)?UPLL_RC_SUCCESS:driver_result;
  return driver_result;
}

upll_rc_t VbrIfMoMgr::PortStatusHandler(const char *ctrlr_name,
                                        const char *domain_name,
                                        const char *portid,
                                        bool oper_status,
                                        DalDmlIntf *dmi  ) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_TRACE("controller_name is : (%s) portid :(%s) domain_id"
                 ":(%s) oper_status :(%d)",
                 ctrlr_name, portid, domain_name, oper_status);
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  /* Allocate Memory for UNC_KT_VBR_IF key and value structure */
  val_drv_vbr_if *vbrif_val = reinterpret_cast<val_drv_vbr_if*>
      (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
  val_port_map *pm = &vbrif_val->vbr_if_val.portmap;
  (vbrif_val->vbr_if_val).valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;

  /* Copy port_id from input to port_id variable in portmap structure */
  uuu::upll_strncpy(pm->logical_port_id, portid,
                    (kMaxLenLogicalPortId + 1));
  /* set valid flag as VALID */
  pm->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;

  ConfigKeyVal *vbrifkey = NULL;
  result_code = GetChildConfigKey(vbrifkey, NULL);
  if (!vbrifkey || (result_code != UPLL_RC_SUCCESS)) {
    free(vbrif_val);
    if (vbrifkey) delete vbrifkey;
    return UPLL_RC_ERR_GENERIC;
  }

  vbrifkey->AppendCfgVal(IpctSt::kIpcStValVbrIf, vbrif_val);
  SET_USER_DATA_CTRLR(vbrifkey, ctrlr_name);
  SET_USER_DATA_DOMAIN(vbrifkey, domain_name);
  state_notification notification =
      (oper_status == UPLL_OPER_STATUS_UP) ? kPortFaultReset : kPortFault;
  // vlinked interfaces should not be handled
  result_code = UpdateOperStatus(vbrifkey,
                                 dmi,
                                 notification,
                                 false,
                                 false,
                                 false);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      result_code = UPLL_RC_SUCCESS;
    else
      UPLL_LOG_DEBUG("Invalid oper status update %d", result_code);
    DELETE_IF_NOT_NULL(vbrifkey);
    return result_code;
  }
  if (notification == kPortFaultReset) {
    VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Returning error\n");
      DELETE_IF_NOT_NULL(vbrifkey);
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->TxUpdateDtState(UNC_KT_VBRIDGE, 0, 0, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("failed to update vnode oper status %d\n", result_code);
      DELETE_IF_NOT_NULL(vbrifkey);
      return result_code;
    }

    VtnMoMgr *vtn_mgr = reinterpret_cast<VtnMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
    if (!vtn_mgr) {
      UPLL_LOG_DEBUG("Returning error\n");
      DELETE_IF_NOT_NULL(vbrifkey);
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = vtn_mgr->TxUpdateDtState(UNC_KT_VTN, 0, 0, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("failed to update vtn oper status %d\n", result_code);
    }
  }
  DELETE_IF_NOT_NULL(vbrifkey);
  return result_code;
}

upll_rc_t VbrIfMoMgr::GetVbrIfFromVExternal(uint8_t *vtn_name,
                                            uint8_t *vext_name,
                                            ConfigKeyVal *&tmpckv,
                                            DalDmlIntf *dmi) {
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
                            new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf,
                                          drv_val_vbrif));
  if (tmpckv == NULL) {
    free(key_vbrif);
    free(drv_val_vbrif);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = ReadConfigDB(tmpckv, UPLL_DT_RUNNING,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("vbrif ReadConfigDB Failed result_code - %d",
                   result_code);
  }
  UPLL_LOG_DEBUG("tmpckv is %s", tmpckv->ToStrAll().c_str());
  return result_code;
}

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
    if (vbrif_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
        UNC_VF_VALID) {
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
          val_vbr_if_t *if_val = &reinterpret_cast<val_drv_vbr_if *>
              (GetVal(tmp))->vbr_if_val;
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

upll_rc_t VbrIfMoMgr::PathFaultHandler(const char *ctrlr_name,
                                       const char *domain_id,
                                       std::vector<std::string> &ingress_ports,
                                       std::vector<std::string> &egress_ports,
                                       bool alarm_asserted,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Alarm_asserted is %d", alarm_asserted);
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  set<key_vnode_type_t, key_vnode_type_compare> sw1_vbridge_set,
      sw2_vbridge_set;
  set<key_vnode_type_t, key_vnode_type_compare>::iterator vbridge1set_itr,
      vbridge2set_itr;
  set<key_vnode_type_t, key_vnode_type_compare> vnode_set;
  set<key_vnode_type_t, key_vnode_type_compare>::iterator vnodeset_itr;
  set<key_vlink_t, vlink_compare> vlink_set;
  set<key_vnode_if_t, key_vnode_if_compare> boundary_if_set;
  state_notification notification = (alarm_asserted == UPLL_OPER_STATUS_UP) ?
      kPathFault : kPathFaultReset;

  std::vector<int>::size_type ingress_size = ingress_ports.size();
  for (uint32_t port_count = 0; port_count < ingress_size; port_count++) {
    UPLL_LOG_DEBUG("ingress port : %s", ingress_ports[port_count].c_str());

    /* Get all the vbridges connected to sw1*/
    result_code = GetMappedVbridges(ctrlr_name, domain_id,
                                    ingress_ports[port_count],
                                    dmi, &sw1_vbridge_set);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in reading mapped vbridges: %d", result_code);
      return result_code;
    }
  }

  std::vector<int>::size_type egress_size = egress_ports.size();
  for (uint32_t port_count = 0; port_count < egress_size; port_count++) {
    UPLL_LOG_DEBUG("egress port : %s", egress_ports[port_count].c_str());

    /* Get all the vbridges connected to sw2*/
    result_code = GetMappedVbridges(ctrlr_name, domain_id,
                                    egress_ports[port_count],
                                    dmi, &sw2_vbridge_set);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in reading mapped vbridges: %d", result_code);
      return result_code;
    }
  }

  /* Iterating through all the vbridges in the vbridge1 set*/
  for (vbridge1set_itr = sw1_vbridge_set.begin();
       vbridge1set_itr != sw1_vbridge_set.end(); ) {
    /* Get the connected topology of each vbridge in sw1_vbridge set */
    VlinkMoMgr *vlink_mgr = reinterpret_cast<VlinkMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));

    key_vnode_type temp_vnode_key = *vbridge1set_itr++;
    key_vnode_type src_vnode_key;
    memcpy(&(src_vnode_key), &temp_vnode_key, sizeof(key_vnode_type_t));
    vnode_set.insert(src_vnode_key);
    vlink_mgr->GetConnected(&src_vnode_key, &vnode_set,
                            &vlink_set, &boundary_if_set, dmi);

    /* Iterating through all the vbridges in the vbridge2 set*/
    for (vbridge2set_itr = sw2_vbridge_set.begin();
         vbridge2set_itr != sw2_vbridge_set.end(); ) {
      /* check if any vbridge in sw2_vbridge set belongs to connected topology
         of a vbridge in sw1_vbridge set */
      key_vnode_type_t vn = *vbridge2set_itr++;

      vnodeset_itr = vnode_set.find(vn);

      /* If the vbridge in sw2_vbridge set belongs to the connected toplogy
         of the vbridge in sw1_vbridge set update the path fault status of
         the corresponding vnodes in the connected topology of the src vbridge
         in sw1_vbridge set */
      if (vnodeset_itr == vnode_set.end())
        continue;
      result_code = vlink_mgr->UpdateVlinkOperStatusUsingVlinkSet(
          &vlink_set, dmi, notification);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Operstatus updation failed");
        return result_code;
      }

      set<key_vnode_type_t, key_vnode_type_compare>::iterator vnode_set_iter;
      set<key_vnode_type_t, key_vnode_type_compare>::iterator vnode_set_end =
          vnode_set.end();
      for (vnode_set_iter = vnode_set.begin();
           vnode_set_iter != vnode_set_end;
           ++vnode_set_iter) {
        VnodeChildMoMgr *mgr = reinterpret_cast<VnodeChildMoMgr *>
            (const_cast<MoManager*>(GetMoManager((*vnode_set_iter).key_type)));
        result_code = mgr->SetLinkedIfOperStatusforPathFault(*vnode_set_iter,
                                                             notification,
                                                             dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Errorin updating Operstatus for Pathfault");
          return result_code;
        }
      }

      result_code = SetBoundaryIfOperStatusforPathFault(
          boundary_if_set, notification, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Errorin updating Operstatus for Pathfault");
        return result_code;
      }

      /*remove the vnodes whose oper status is updated from sw2_vbridge_set and
        from sw1_vbridge_set*/

      for (vnodeset_itr = vnode_set.begin(); vnodeset_itr != vnode_set_end;) {
        key_vnode_type_t vn = *vnodeset_itr++;
        key_vnode_type_t vn1 = *vbridge2set_itr;
        if (memcmp(&vn, &vn1 , sizeof(key_vnode_type_t)) == 0)
          vbridge2set_itr++;
        sw2_vbridge_set.erase(vn);
        vn1 = *vbridge1set_itr;
        if (memcmp(&vn, &vn1, sizeof(key_vnode_type_t)) == 0)
          vbridge1set_itr++;
        sw1_vbridge_set.erase(vn);
      }
    }
    boundary_if_set.clear();
    vnode_set.clear();
    vlink_set.clear();
  }
  result_code = RestoreUnInitOPerStatus(dmi);
  return result_code;
}

/*This function gives the bridges connected to a particular switch log port*/
upll_rc_t VbrIfMoMgr::GetMappedVbridges(
    const char *ctrlr_name,
    const char *domain_id,
    std::string logportid, DalDmlIntf *dmi,
    set<key_vnode_type_t, key_vnode_type_compare> *sw_vbridge_set) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vbr_if = NULL;
  result_code = GetChildConfigKey(ck_vbr_if, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  SET_USER_DATA_CTRLR(ck_vbr_if, ctrlr_name);
  SET_USER_DATA_DOMAIN(ck_vbr_if, domain_id);
  ConfigVal *cv_val = NULL;
  result_code = AllocVal(cv_val, UPLL_DT_RUNNING);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    DELETE_IF_NOT_NULL(ck_vbr_if);
    return result_code;
  }
  ck_vbr_if->SetCfgVal(cv_val);
  val_vbr_if *vbr_if_val = reinterpret_cast<val_vbr_if *>(GetVal(ck_vbr_if));
  if (!vbr_if_val) {
    UPLL_LOG_DEBUG("ConfigVal is NULL");
    DELETE_IF_NOT_NULL(ck_vbr_if);
    return UPLL_RC_ERR_GENERIC;
  }

  /* copy switch id to val_vbr_if strcuture */
  /*Setting portmap and switch_id valid status*/
  vbr_if_val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  uuu::upll_strncpy(vbr_if_val->portmap.logical_port_id, logportid.c_str(),
                    (kMaxLenLogicalPortId + 1));

  /* Get all the vbridges under the VTN */
  DbSubOp dbop = { kOpReadMultiple,
    kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutNone };
  result_code = ReadConfigDB(ck_vbr_if,
                             UPLL_DT_RUNNING,
                             UNC_OP_READ,
                             dbop,
                             dmi,
                             MAINTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(ck_vbr_if);
    return UPLL_RC_SUCCESS;
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in reading: %d", result_code);
    DELETE_IF_NOT_NULL(ck_vbr_if);
    return result_code;
  }
  /* populate sw_vbridge set with vbridges mapped to the specified switch*/
  key_vbr_if_t *tkey = reinterpret_cast<key_vbr_if_t *>
      (ck_vbr_if->get_key());
  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_t)));
  uuu::upll_strncpy(vbr_key->vbridge_name, tkey->vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                    tkey->vbr_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  key_vnode_type_t vnode_type;
  vnode_type.vnode_key = *(reinterpret_cast<key_vnode_t *>(vbr_key));
  vnode_type.key_type = UNC_KT_VBR_IF;
  sw_vbridge_set->insert(vnode_type);
  ConfigKeyVal::Free(vbr_key);
  DELETE_IF_NOT_NULL(ck_vbr_if);
  return result_code;
}

upll_rc_t VbrIfMoMgr::GetBoundaryInterfaces(key_vnode_if_t boundary_if,
                                            DalDmlIntf *dmi,
                                            ConfigKeyVal *&iokey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  GetChildConfigKey(iokey, NULL);
  key_vnode_if *vnode_if_key = reinterpret_cast<key_vnode_if*>
      (iokey->get_key());
  memcpy(vnode_if_key, &boundary_if, sizeof(key_vnode_if));

  /* Get all the vbridges under the VTN */
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(iokey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  return result_code;
}

upll_rc_t VbrIfMoMgr::SetBoundaryIfOperStatusforPathFault(
    const set<key_vnode_if_t, key_vnode_if_compare> &boundary_if_set,
    state_notification notification,
    DalDmlIntf *dmi) {
  set<key_vnode_if_t, key_vnode_if_compare>::iterator boundary_if_set_iter;
  set<key_vnode_if_t, key_vnode_if_compare>::iterator boundary_if_set_end =
      boundary_if_set.end();
  ConfigKeyVal *ck_boundary_if = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  for (boundary_if_set_iter = boundary_if_set.begin();
       boundary_if_set_iter != boundary_if_set_end;
       ++boundary_if_set_iter) {
    result_code = GetBoundaryInterfaces(*boundary_if_set_iter, dmi,
                                        ck_boundary_if);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("Operstatus updation failed");
      DELETE_IF_NOT_NULL(ck_boundary_if);
      return result_code;
    }
    while (ck_boundary_if != NULL) {
      ConfigKeyVal *ckv_tmp = ck_boundary_if;
      ck_boundary_if = ck_boundary_if->get_next_cfg_key_val();
      result_code = UpdateOperStatus(ckv_tmp, dmi, notification,
                                     true, false, false);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Operstatus updation failed");
        DELETE_IF_NOT_NULL(ckv_tmp);
        DELETE_IF_NOT_NULL(ck_boundary_if);
        return result_code;
      }
      delete ckv_tmp;
    }
  }
  return result_code;
}

upll_rc_t VbrIfMoMgr::RestoreUnInitOPerStatus(DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VnodeMoMgr *vnode_mgr = NULL;
  VtnMoMgr *vtn_mgr = NULL;
  unc_key_type_t key_type[] = {UNC_KT_VBRIDGE, UNC_KT_VROUTER, UNC_KT_VTN};
  for (int count = 0;
       count < static_cast<int>(sizeof(key_type)/sizeof(unc_key_type_t));
       count++) {
    if (key_type[count] == UNC_KT_VTN) {
      vtn_mgr = reinterpret_cast<VtnMoMgr *>
          (const_cast<MoManager *>(GetMoManager(key_type[count])));
      result_code = vtn_mgr->TxUpdateDtState(key_type[count], 0, 0, dmi);
    } else {
      vnode_mgr = reinterpret_cast<VnodeMoMgr *>
          (const_cast<MoManager *>(GetMoManager(key_type[count])));
      result_code = vnode_mgr->TxUpdateDtState(key_type[count], 0, 0, dmi);
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("failed to update vnode oper status %d\n", result_code);
      // DELETE_IF_NOT_NULL(ck_vnode_if);
      return result_code;
    }
  }
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
