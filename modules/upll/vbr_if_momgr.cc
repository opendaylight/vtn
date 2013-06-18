/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vbr_if_momgr.hh"
#include "vbr_momgr.hh"
#include "vtn_momgr.hh"
#include "vlink_momgr.hh"
#include <ctype.h>
#include <sstream>
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
    { uudst::vbridge_interface::kDbiValidAdminStatus, CFG_META_VAL, offsetof(
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
    { uudst::vbridge_interface::kDbiValidOperStatus, ST_META_VAL, offsetof(
        val_db_vbr_if_st, vbr_if_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiCsAdminStatus, CS_VAL, offsetof(
        val_vbr_if, cs_attr[UPLL_IDX_ADMIN_STATUS_VBRI]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiCsDesc, CS_VAL, offsetof(
        val_vbr_if, cs_attr[UPLL_IDX_DESC_VBRI]),
      uud::kDalUint8, 1 },
    { uudst::vbridge_interface::kDbiCsPortMap, CFG_META_VAL, offsetof(
        val_vbr_if, portmap.cs_attr[UPLL_IDX_PM_VBRI]),
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
  table[MAINTBL] = new Table(uudst::kDbiVbrIfTbl, UNC_KT_VBR_IF, vbr_if_bind_info,
                         IpctSt::kIpcStKeyVbrIf, IpctSt::kIpcStValVbrIf,
                         uudst::vbridge_interface::kDbiVbrIfNumCols);
  table[CTRLRTBL] = NULL;
  table[RENAMETBL] = NULL;
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
  UPLL_LOG_ERROR("UpdateMo for %d", ikey->get_key_type());
  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Validation Message is Failed ");
      return result_code;
  }
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
      UPLL_LOG_ERROR("Record does Not Exists");
      return result_code;
  }
  result_code = DupConfigKeyVal(okey, ikey, MAINTBL);
   if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG(" DupConfigKeyVal Failed %d", result_code);
    return result_code;
  }

  result_code = ValidateAttribute(okey, dmi,req);
  if (UPLL_RC_SUCCESS  != result_code) {
      delete okey;
      UPLL_LOG_ERROR("Validate Attribute is Failed");
      return result_code;
  }
  result_code = UpdateConfigVal(okey, req->datatype, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
      delete okey;
      UPLL_LOG_DEBUG("UpdateConfigVal is Failed");
      return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  UPLL_LOG_DEBUG("The okey Structue before update  %s", (okey->ToStrAll()).c_str());
  result_code = UpdateConfigDB(okey, req->datatype, UNC_OP_UPDATE,
                               dmi, &dbop, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    delete okey;
    UPLL_LOG_ERROR("Updation Failure in DB : %d", result_code);
    return result_code;
  }
  delete okey;
  return result_code;
}

upll_rc_t VbrIfMoMgr::CreateAuditMoImpl(IpcReqRespHeader *header,
                                        ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                        const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_ifkey = NULL;
  ConfigKeyVal *ck_vlink = NULL;
  result_code = VnodeChildMoMgr::CreateAuditMoImpl(ikey,dmi,ctrlr_id);
  if(result_code != UPLL_RC_SUCCESS) {
    string s(ikey->ToStrAll());
    UPLL_LOG_INFO("Create Audit Vbrif failed %s",s.c_str());
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(ck_ifkey, ikey);
  if(result_code != UPLL_RC_SUCCESS || ck_ifkey == NULL) {
    UPLL_LOG_INFO("GetChildConfigKey failed err_code %d",result_code);
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
  result_code = mgr->CheckIfMemberOfVlink(ck_ifkey,UPLL_DT_RUNNING,
                                             ck_vlink,dmi,iftype);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    delete ck_ifkey;
    if (ck_vlink) delete ck_vlink;
    UPLL_LOG_DEBUG("Internal link interface");
    return UPLL_RC_SUCCESS;
  } else if (result_code != UPLL_RC_SUCCESS) {
     delete ck_ifkey;
     if (ck_vlink) delete ck_vlink;
     UPLL_LOG_INFO("Error in reading vlink key %d",result_code);
     return UPLL_RC_ERR_GENERIC;
  } else if ((iftype == kVlinkInternalNode1) ||
             (iftype == kVlinkInternalNode2))  {
     UPLL_LOG_DEBUG("Internal link interface");
     delete ck_ifkey;
     if (ck_vlink) delete ck_vlink;
     return UPLL_RC_SUCCESS;
  }
  val_vlink *vlink_val = reinterpret_cast<val_vlink *>(GetVal(ck_vlink));
  if (vlink_val == NULL) return UPLL_RC_ERR_GENERIC;
  uint8_t valid_boundary = vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK];
  //store VALID_NO_VAL in running only in case of boundary link
  if ((valid_boundary == UNC_VF_VALID_NO_VALUE) ||
      (valid_boundary == UNC_VF_VALID)) {
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutFlag};
    result_code = ReadConfigDB(ck_ifkey, UPLL_DT_RUNNING, UNC_OP_READ,
                  dbop, dmi, MAINTBL);
    if(result_code != UPLL_RC_SUCCESS) {
      delete ck_ifkey;
      UPLL_LOG_INFO("Retrieving a Record for VbrIf in RUNNING DB failed");
      return result_code;
    }
    void *db_val,*drv_val = GetVal(ikey) ;
    db_val =  GetVal(ck_ifkey);
    if (!db_val || !drv_val)
      return UPLL_RC_ERR_GENERIC;
    //validate params of running against those received from driver
    if (memcmp(db_val,drv_val,sizeof(val_drv_vbr_if)) == 0) {
    // create boundary vlink
      result_code = mgr->UpdateConfigDB(ck_vlink, UPLL_DT_AUDIT,
                     UNC_OP_CREATE, dmi, MAINTBL);
      if(result_code != UPLL_RC_SUCCESS) {
        delete ck_ifkey;
        delete ck_vlink;
        UPLL_LOG_INFO("Retrieving a Record for VbrIf in RUNNING DB failed");
        return result_code;
      }
    } else {
      delete ck_ifkey;
      delete ck_vlink;
      UPLL_LOG_INFO("Invalid data");
      return UPLL_RC_ERR_GENERIC;
    }
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

  result_code = GetVbrIfValfromDB(ikey, ck_drv_vbr_if, UPLL_DT_RUNNING, dmi);

  key_vbr_if *temp_vbr_if_key = reinterpret_cast<key_vbr_if *>(ikey->get_key());

  key_vbr_if *vbr_if_key = reinterpret_cast<key_vbr_if *>(malloc(
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
  result_code = UpdateConfigDB(ck_drv_vbr_if, UPLL_DT_AUDIT, UNC_OP_CREATE, dmi,
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

upll_rc_t VbrIfMoMgr::ConverttoDriverPortMap(ConfigKeyVal *ck_port_map) {
  UPLL_FUNC_TRACE;
  std::string if_name = reinterpret_cast<const char *>(
                reinterpret_cast<key_vbr_if*>(ck_port_map->get_key())->if_name);
  if (strlen(if_name.c_str()) >= 18)
    if_name.assign(if_name.c_str(), 18);

  std::string vex_name = "vx_" + if_name + static_cast<std::ostringstream*>(
                                &(std::ostringstream() << time(NULL)) )->str();
  std::string vex_if_name = "vi_" + if_name + static_cast<std::ostringstream*>(
                                &(std::ostringstream() << time(NULL)) )->str();
  std::string vex_link_name = "vl_" + if_name + static_cast<std::ostringstream*>(
                                &(std::ostringstream() << time(NULL)) )->str();
  val_drv_vbr_if *drv_vbr_if_val = reinterpret_cast<val_drv_vbr_if *>
                                                (GetVal(ck_port_map));
  drv_vbr_if_val->valid[UPLL_IDX_VBR_IF_DRV_PM] = UNC_VF_VALID;
  drv_vbr_if_val->valid[UPLL_IDX_VEXT_DRV_PM] = UNC_VF_VALID;
  uuu::upll_strncpy(drv_vbr_if_val->vex_name, vex_name.c_str(),
		   (kMaxLenVnodeName+1));

  drv_vbr_if_val->valid[UPLL_IDX_VEXT_IF_DRV_PM] = UNC_VF_VALID;
  uuu::upll_strncpy(drv_vbr_if_val->vex_if_name,  vex_if_name.c_str(),
                    (kMaxLenVnodeName + 1));

  drv_vbr_if_val->valid[UPLL_IDX_VEXT_LINK_DRV_PM] = UNC_VF_VALID;
  uuu::upll_strncpy(drv_vbr_if_val->vex_link_name, vex_link_name.c_str(),
                    (kMaxLenVnodeName + 1));
  return UPLL_RC_SUCCESS;
}

bool VbrIfMoMgr::IsValidKey(void *key, uint64_t index) {
  UPLL_FUNC_TRACE;
  key_vbr_if *if_key = reinterpret_cast<key_vbr_if *>(key);
  bool ret_val = UPLL_RC_SUCCESS;
  switch (index) {
    case uudst::vbridge_interface::kDbiVtnName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(if_key->vbr_key.vtn_key.vtn_name),
          kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_interface::kDbiVbrName:
      ret_val = ValidateKey(
          reinterpret_cast<char *>(if_key->vbr_key.vbridge_name),
          kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("VBR Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbridge_interface::kDbiIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>(if_key->if_name),
                            kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("VBR IF Name is not valid(%d)", ret_val);
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
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_if *vbr_key_if;
  void *pkey;
  if (parent_key == NULL) {
    vbr_key_if = reinterpret_cast<key_vbr_if_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
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
        free(vbr_key_if);
        return UPLL_RC_ERR_GENERIC;
      }
      GET_USER_DATA_FLAGS(parent_key->get_cfg_val(), flags);
      flags &=  VLINK_FLAG_NODE_POS;
      UPLL_LOG_DEBUG("Vlink flag node position %d",flags);
      if (flags == kVlinkVnode2) {
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
  if (okey == NULL) {
    free(vbr_key_if);
    result_code = UPLL_RC_ERR_GENERIC;
  } else {
    SET_USER_DATA(okey, parent_key);
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
  key_vbr *vbr_key = reinterpret_cast<key_vbr *>(malloc(sizeof(key_vbr)));
  if (!vbr_key) return UPLL_RC_ERR_GENERIC;
  memset(vbr_key, 0, sizeof(key_vbr));
#if 1
  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
          reinterpret_cast<key_vbr_if *>(pkey)->vbr_key.vtn_key.vtn_name,
          (kMaxLenVtnName+1));
  uuu::upll_strncpy(vbr_key->vbridge_name,
          reinterpret_cast<key_vbr_if *>(pkey)->vbr_key.vbridge_name,
          (kMaxLenVnodeName+1));
#endif
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
      val = reinterpret_cast<void *>(malloc(sizeof(val_drv_vbr_if)));
      if (!val) return UPLL_RC_ERR_GENERIC;
      memset(val, 0, sizeof(val_drv_vbr_if));
      ck_val = new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf, val);
      if (dt_type == UPLL_DT_STATE) {
        val = reinterpret_cast<void *>(malloc(sizeof(val_db_vbr_if_st)));
        memset(val, 0, sizeof(val_db_vbr_if_st));
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
        val_vbr_if *vbr_val_if = reinterpret_cast<val_vbr_if *>(malloc(
            sizeof(val_vbr_if)));
        if (!vbr_val_if) return UPLL_RC_ERR_GENERIC;
        memcpy(vbr_val_if, ival, sizeof(val_vbr_if));
        oval = reinterpret_cast<void *>(vbr_val_if);
      } else {
        val_drv_vbr_if *ival = reinterpret_cast<val_drv_vbr_if *>(GetVal(req));
        if (!ival) return UPLL_RC_ERR_GENERIC;
        val_drv_vbr_if *vbr_val_if = reinterpret_cast<val_drv_vbr_if *>(malloc(
            sizeof(val_drv_vbr_if)));
        if (!vbr_val_if) return UPLL_RC_ERR_GENERIC;
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
      if ((req->get_cfg_val())->get_st_num() == IpctSt::kIpcStValVbrIfSt) {
        val_vbr_if_st *ival = reinterpret_cast<val_vbr_if_st *>(tmp->get_val());
        val_vbr_if_st *val_vbr_if = reinterpret_cast<val_vbr_if_st *>(malloc(
            sizeof(val_vbr_if_st)));
        memcpy(val_vbr_if, ival, sizeof(val_vbr_if_st));
        ovalst = reinterpret_cast<void *>(val_vbr_if);
      } else {
        val_db_vbr_if_st *ival =
            reinterpret_cast<val_db_vbr_if_st *>(tmp->get_val());
        val_db_vbr_if_st *val_vbr_if =
            reinterpret_cast<val_db_vbr_if_st *>(malloc(
                sizeof(val_db_vbr_if_st)));
        memcpy(val_vbr_if, ival, sizeof(val_db_vbr_if_st));
        ovalst = reinterpret_cast<void *>(val_vbr_if);
      }
      ConfigVal *tmp2 = new ConfigVal(
        req->get_cfg_val()->get_next_cfg_val()->get_st_num(), ovalst);
      tmp1->AppendCfgVal(tmp2);
    }
  }
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  key_vbr_if *ikey = reinterpret_cast<key_vbr_if *>(tkey);
  key_vbr_if *vbr_if_key = reinterpret_cast<key_vbr_if *>(malloc(
      sizeof(key_vbr_if)));
  if (!vbr_if_key) {
      UPLL_LOG_DEBUG(" Memory allocation failed");
      DELETE_IF_NOT_NULL(tmp1);
      return UPLL_RC_ERR_GENERIC;
  }
  memcpy(vbr_if_key, ikey, sizeof(key_vbr_if));
  okey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, vbr_if_key,
                          tmp1);
  if (okey) SET_USER_DATA(okey, req);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::UpdateConfigStatus(ConfigKeyVal *ikey,
                                         unc_keytype_operation_t op,
                                         uint32_t driver_result,
                                         ConfigKeyVal *upd_key,
                                         DalDmlIntf *dmi,
                                         ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_drv_vbr_if_t *vbrif_val = reinterpret_cast<val_drv_vbr_if_t *>(GetVal(ikey));

  unc_keytype_configstatus_t cs_status =
      (driver_result == UPLL_RC_SUCCESS) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  if (vbrif_val == NULL) return UPLL_RC_ERR_GENERIC;
  val_vbr_if_t *vbr_if_val = &vbrif_val->vbr_if_val;
  bool port_map_change = false;
  switch (op) {
  case UNC_OP_UPDATE: 
  {
    void *val = reinterpret_cast<void *>(vbrif_val);
    CompareValidValue(val, GetVal(upd_key), true);
    if ((vbr_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) && 
        (vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] ==
          UNC_VF_VALID))    
      port_map_change = true;
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
      upll_rc_t result_code = InitOperStatus<val_vbr_if_st,val_db_vbr_if_st>
                            (ikey,vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI],
                             vbr_if_val->admin_status,
                             vbr_if_val->valid[UPLL_IDX_PM_VBRI],
                             &vbr_if_val->portmap);
      vbr_if_valst->down_count = 0;
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error settiing oper status");
        return UPLL_RC_ERR_GENERIC;
      }
    }
    break;
  default:
    UPLL_LOG_DEBUG("Invalid op %d\n",op);
    return UPLL_RC_ERR_GENERIC;
  }
  for (unsigned int loop = 0; loop < 
       sizeof(vbr_if_val->valid) / sizeof(vbr_if_val->valid[0]); ++loop) {
    if (UNC_VF_NOT_SOPPORTED == vbr_if_val->valid[loop]) {
      vbr_if_val->cs_attr[loop] = UNC_CS_NOT_SUPPORTED;
    } else if ((UNC_VF_VALID == (uint8_t) vbr_if_val->valid[loop]) 
        || (UNC_VF_VALID_NO_VALUE == (uint8_t) vbr_if_val->valid[loop])) {
      vbr_if_val->cs_attr[loop] = cs_status;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::UpdateAuditConfigStatus(
                                      unc_keytype_configstatus_t cs_status,
                                      uuc::UpdateCtrlrPhase phase,
                                      ConfigKeyVal *&ckv_running) {
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
  for (unsigned int loop = 0; loop < sizeof(val->valid) / sizeof(uint8_t);
      ++loop) {
    if ((cs_status == UNC_CS_INVALID && UNC_VF_VALID == val->valid[loop]) ||
         cs_status == UNC_CS_APPLIED)
         val->cs_attr[loop] = cs_status;
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
        UPLL_LOG_DEBUG("Val struct Validation is an optional for READ operation");
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
        UPLL_LOG_DEBUG("Val struct Validation is an optional for READ operation");
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
                  dt_type,operation);
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
  upll_rc_t ret_val = UPLL_RC_SUCCESS;

  if (vbr_if_val->valid[UPLL_IDX_DESC_VBRI] == UNC_VF_VALID) {
    ret_val = ValidateDesc(reinterpret_cast<char *>(vbr_if_val->description),
                           kMinLenDescription, kMaxLenDescription);
    if (ret_val != UPLL_RC_SUCCESS) {
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
  }
  if (vbr_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    if (vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
        == UNC_VF_VALID) {
      ret_val = ValidateLogicalPortId(
          reinterpret_cast<char *>(vbr_if_val->portmap.logical_port_id),
          kMinLenLogicalPortId, kMaxLenLogicalPortId); 
      if (ret_val != UPLL_RC_SUCCESS) { 
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
    } else if (vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM]
        == UNC_VF_VALID_NO_VALUE
        && (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE)) {
      vbr_if_val->portmap.tagged = UPLL_VLAN_UNTAGGED;
    }
  } else if ((vbr_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE)
      && (operation == UNC_OP_UPDATE || operation == UNC_OP_CREATE)) {
    memset(&(vbr_if_val->portmap), 0, sizeof(vbr_if_val->portmap));
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t VbrIfMoMgr::ValidateVtnNeighborValue(val_vtn_neighbor *vtn_neighbor,
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
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  if (!ikey || !req ) {
    UPLL_LOG_DEBUG("ConfigKeyVal / IpcReqRespHeader is Null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name) ctrlr_name = reinterpret_cast<char *>(ikey->get_user_data());

  upll_keytype_datatype_t dt_type = req->datatype;
  unc_keytype_operation_t operation = req->operation;
  unc_keytype_option1_t option1 = req->option1;
  unc_keytype_option2_t option2 = req->option2;

  if (operation == UNC_OP_CREATE) {  // C, I
    if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_IMPORT) {
      ret_val = ValVbrIfAttributeSupportCheck(ctrlr_name, ikey, operation, req->datatype);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VBR_IF struct Capa check failure for create operation");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype - (%d)", dt_type);
      return UPLL_RC_ERR_GENERIC;
    }
  } else if (operation == UNC_OP_UPDATE) {  // C
    if (dt_type == UPLL_DT_CANDIDATE) {
      ret_val = ValVbrIfAttributeSupportCheck(ctrlr_name, ikey, operation, req->datatype);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VBR_IF struct Capa check failure for Update operation");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype - (%d)", dt_type);
      return UPLL_RC_ERR_GENERIC;
    }
  } else if (operation == UNC_OP_READ || operation == UNC_OP_READ_SIBLING
             || operation == UNC_OP_READ_SIBLING_BEGIN
             || operation == UNC_OP_READ_SIBLING_COUNT) {
    if (dt_type == UPLL_DT_CANDIDATE || dt_type == UPLL_DT_RUNNING ||
        dt_type == UPLL_DT_STARTUP || dt_type == UPLL_DT_STATE) {
      if (option1 != UNC_OPT1_NORMAL) {
        UPLL_LOG_DEBUG("Error option1 is not NORMAL");
        return UPLL_RC_ERR_INVALID_OPTION1;
      }
      if (option2 == UNC_OPT2_NONE) {
        if (ikey->get_cfg_val()->get_val() == NULL) {
          UPLL_LOG_DEBUG("val_vbr_if struct is an optional");
          return UPLL_RC_SUCCESS;
        }
        ret_val = ValVbrIfAttributeSupportCheck(ctrlr_name, ikey, operation, req->datatype);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("VBR_IF struct capa check failure for read operation");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
        return UPLL_RC_SUCCESS;
      } else if (option2 == UNC_OPT2_NEIGHBOR) {
        if (ikey->get_cfg_val()->get_val() == NULL) {
        }
        ret_val = ValVtnNeighborAttributeSupportCheck(ctrlr_name, ikey);
        if (ret_val != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(
              "val_vtn_neighbor struct capa check failure for read operation");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Error option2 is not matching");
        return UPLL_RC_ERR_INVALID_OPTION2;
      }
    } else {
      UPLL_LOG_DEBUG("Error Unsupported datatype - (%d)", dt_type);
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  }
  UPLL_LOG_DEBUG("Error Unsupported operation - (%d)", operation);
  return UPLL_RC_ERR_NO_SUCH_INSTANCE;
}

upll_rc_t VbrIfMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                        DalDmlIntf *dmi,
                                        IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  // Semantic check for different vbridges with same switch-id and vlan-id
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *nw_vbrif_ck = NULL;
  if (ikey->get_key_type() != UNC_KT_VBR_IF)
    return UPLL_RC_ERR_CFG_SYNTAX;
  /* allocate a new vbrif key and val */
  result_code = GetChildConfigKey(nw_vbrif_ck, NULL);
  if (result_code != UPLL_RC_SUCCESS || nw_vbrif_ck == NULL) {
    UPLL_LOG_DEBUG("Failed GetChildConfigKey. Urc=%d", result_code);
    if (nw_vbrif_ck) delete nw_vbrif_ck;
    return result_code;
  }
  // key_vbr_if *if_key = reinterpret_cast<key_vbr_if *>(ikey->get_key());
  val_vbr_if *if_val = reinterpret_cast<val_vbr_if *>(GetVal(ikey));
  if (!if_val) {
    if (req->operation == UNC_OP_CREATE) {
      UPLL_LOG_DEBUG("Val Structure is Null");
      delete nw_vbrif_ck;
      return UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Val structure is must");
      delete nw_vbrif_ck;
      return UPLL_RC_ERR_GENERIC;
    }
  }
  key_vbr_if *nw_ifkey = reinterpret_cast<key_vbr_if *>(nw_vbrif_ck->get_key());
  if (!nw_ifkey) {
    delete nw_vbrif_ck;
    return UPLL_RC_ERR_GENERIC;
  }
  val_drv_vbr_if *valif = reinterpret_cast<val_drv_vbr_if *>
      (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
  memcpy(&(valif->vbr_if_val), if_val, sizeof(val_vbr_if));

#if 0
  /* init val with portmap parameters from input key */
  if ((if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID)
      && (if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] == UNC_VF_VALID)) {
    valif->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID;
    valif->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
    uuu::upll_strncpy(valif->vbr_if_val.portmap.logical_port_id,
                      if_val->portmap.logical_port_id, (kMaxLenPortName + 1));
    valif->vbr_if_val.portmap.vlan_id = if_val->portmap.vlan_id;
    DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
    result_code = ReadConfigDB(nw_vbrif_ck, UPLL_DT_CANDIDATE, UNC_OP_READ ,
                               dbop, dmi, MAINTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      return UPLL_RC_SUCCESS;
    } else if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Rturning error %d", result_code);
      return result_code;
    } else {
      ConfigKeyVal *tmp = nw_vbrif_ck;
      /* parse read db output */
      while (tmp) {
        if (memcmp(reinterpret_cast<key_vbr_if *>(tmp->get_key()), if_key,
                   sizeof(key_vbr_if)) != 0) {
          UPLL_LOG_INFO("Different vbridges contain same switch-id and vlan-id ");
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
        tmp = tmp->get_next_cfg_key_val();
      }
    }
    delete nw_vbrif_ck;
  }
#endif
  (ikey->get_cfg_val())->SetVal(IpctSt::kIpcStPfcdrvValVbrIf, valif);
  delete nw_vbrif_ck;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::ValVbrIfAttributeSupportCheck(
                                                   const char *ctrlr_name,
                                                   ConfigKeyVal *ikey,
                                                   unc_keytype_operation_t operation,
                                                   upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  bool result_code = false;
  uint32_t max_attrs = 0;
  uint32_t max_instance_count = 0;
  const uint8_t *attrs;

  switch (operation) {
    case UNC_OP_CREATE:
      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count, &max_attrs, &attrs);
      if (result_code && cur_instance_count >= max_instance_count && 
          cur_instance_count !=0 && max_instance_count != 0) {
        UPLL_LOG_INFO("[%s:%d:%s Instance count %d exceeds %d", __FILE__,
                      __LINE__, __FUNCTION__, cur_instance_count,
                      max_instance_count);
        return UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT;
      }

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
      UPLL_LOG_INFO("Invalid operation code - (%d)", operation);
      return UPLL_RC_ERR_GENERIC;
  }

  if (!result_code) {
    UPLL_LOG_INFO("key_type - %d is not supported by controller - %s",
                  ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  if (NULL == ikey->get_cfg_val()) {
    UPLL_LOG_INFO("Empty cfg_val is received");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_cfg_val()->get_st_num() != IpctSt::kIpcStValVbrIf && UPLL_DT_IMPORT != dt_type) {
    UPLL_LOG_INFO("Invalid Value structure received. received struct - %d",
                  (ikey->get_cfg_val()->get_st_num()));
    return UPLL_RC_ERR_CFG_SYNTAX;
  }
  if (UPLL_DT_IMPORT != dt_type) {
  val_vbr_if *vbr_if_val =
      reinterpret_cast<val_vbr_if *>(ikey->get_cfg_val()->get_val());
  if (vbr_if_val != NULL) {
    if ((vbr_if_val->valid[UPLL_IDX_DESC_VBRI] == UNC_VF_VALID)
        || (vbr_if_val->valid[UPLL_IDX_DESC_VBRI] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_if::kCapDesc] == 0) {
        vbr_if_val->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_INFO("Description attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_VALID)
        ||(vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] ==
           UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_if::kCapAdminStatus] == 0) {
        vbr_if_val->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_INFO("Admin status attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((vbr_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID)
        || (vbr_if_val->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE)) {
      if ((vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] == UNC_VF_VALID)
          || (vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
              == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapLogicalPortId] == 0) {
          vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
              UNC_VF_NOT_SOPPORTED;
          UPLL_LOG_INFO("portmap.swich_id attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
      if ((vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID)
          || (vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM]
              == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapVlanId] == 0) {
          vbr_if_val->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_NOT_SOPPORTED;
          UPLL_LOG_INFO("portmap.vlanid attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
      if ((vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID)
          || (vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM]
              == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapTagged] == 0) {
          vbr_if_val->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_NOT_SOPPORTED;
          UPLL_LOG_INFO("portmap.Tagged attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    return UPLL_RC_SUCCESS;
  } 
  } else if (UPLL_DT_IMPORT == dt_type) {
    val_drv_vbr_if *vbr_if_val =
      reinterpret_cast<val_drv_vbr_if *>(ikey->get_cfg_val()->get_val());
  if (vbr_if_val != NULL) {
    if ((vbr_if_val->vbr_if_val.valid[UPLL_IDX_DESC_VBRI] == UNC_VF_VALID)
        || (vbr_if_val->vbr_if_val.valid[UPLL_IDX_DESC_VBRI] == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_if::kCapDesc] == 0) {
        vbr_if_val->vbr_if_val.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_INFO("Description attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((vbr_if_val->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_VALID)
        ||(vbr_if_val->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] ==
           UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vbr_if::kCapAdminStatus] == 0) {
        vbr_if_val->vbr_if_val.valid[UPLL_IDX_DESC_VBRI] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_INFO("Admin status attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((vbr_if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID)
        || (vbr_if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE)) {
      if ((vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] == UNC_VF_VALID)
          || (vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
              == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapLogicalPortId] == 0) {
          vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
              UNC_VF_NOT_SOPPORTED;
          UPLL_LOG_INFO("portmap.swich_id attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
      if ((vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID)
          || (vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM]
              == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapVlanId] == 0) {
          vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_NOT_SOPPORTED;
          UPLL_LOG_INFO("portmap.vlanid attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
      if ((vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID)
          || (vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM]
              == UNC_VF_VALID_NO_VALUE)) {
        if (attrs[unc::capa::vbr_if::kCapTagged] == 0) {
          vbr_if_val->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_NOT_SOPPORTED;
          UPLL_LOG_INFO("portmap.Tagged attr is not supported by ctrlr ");
          return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
        }
      }
    }
    return UPLL_RC_SUCCESS;
  }
  }
  UPLL_LOG_INFO("Error VBR_IF STRUCT is NULL");
  return UPLL_RC_ERR_GENERIC;
}

upll_rc_t VbrIfMoMgr::ValVtnNeighborAttributeSupportCheck(
    const char *ctrlr_name, ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  bool result_code;
  uint32_t max_attrs;
  const uint8_t *attrs;

  result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(), &max_attrs,
                                  &attrs);
  if (!result_code) {
    UPLL_LOG_INFO("key_type - %d is not supported by controller - %s",
                  ikey->get_key_type(), ctrlr_name);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  val_vtn_neighbor *vtn_neighbor =
      reinterpret_cast<val_vtn_neighbor *>(ikey->get_cfg_val()->get_val());
  if (vtn_neighbor != NULL) {
    if ((vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] == UNC_VF_VALID)
        || (vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn_neighbor::kCapConnectedVnodeName] == 0) {
        vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_INFO("Vtn_neighbor structure attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN] == UNC_VF_VALID)
        || (vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn_neighbor::kCapConnectedIfName] == 0) {
        vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN] =
            UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_INFO("Vtn_neighbor structure attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    if ((vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] == UNC_VF_VALID)
        || (vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN]
            == UNC_VF_VALID_NO_VALUE)) {
      if (attrs[unc::capa::vtn_neighbor::kCapConnectedVlinkName] == 0) {
        vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] = UNC_VF_NOT_SOPPORTED;
        UPLL_LOG_INFO("Vtn_neighbor structure attr is not supported by ctrlr ");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
      }
    }
    return UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_DEBUG("Error VAL_VTN_NEIGHBOR STRUCT is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
}

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
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                      key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name)))
      return UPLL_RC_ERR_GENERIC;
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
  val_vbr_if_t *val_vbr_if1 = reinterpret_cast<val_vbr_if_t *>(val1);
  val_vbr_if_t *val_vbr_if2 = reinterpret_cast<val_vbr_if_t *>(val2);
  for (unsigned int loop = 0;
       loop < sizeof(val_vbr_if1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vbr_if1->valid[loop]
        && UNC_VF_VALID == val_vbr_if2->valid[loop])
      val_vbr_if1->valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  for (unsigned int loop = 0;
       loop < sizeof(val_vbr_if1->valid) / sizeof(uint8_t); ++loop) {
    if (UNC_VF_INVALID == val_vbr_if1->portmap.valid[loop]
          && UNC_VF_VALID == val_vbr_if2->portmap.valid[loop])
        val_vbr_if1->portmap.valid[loop] = UNC_VF_VALID_NO_VALUE;
  }
  if ((UNC_VF_VALID == val_vbr_if1->valid[UPLL_IDX_DESC_VBRI])
       && (UNC_VF_VALID == val_vbr_if2->valid[UPLL_IDX_DESC_VBRI]))
    if (!strcmp (reinterpret_cast<char *>(val_vbr_if1->description),
                  reinterpret_cast<const char*>(val_vbr_if2->description)))
      val_vbr_if1->valid[UPLL_IDX_DESC_VBRI] = UNC_VF_INVALID;
#if 0
  // admin state val is needed to determine oper status
  if (val_vbr_if1->valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_VALID
      && val_vbr_if2->valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_VALID) {
    if (val_vbr_if1->admin_status == val_vbr_if2->admin_status)
      val_vbr_if1->valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_INVALID;
  }
#endif
  if (val_vbr_if1->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID
      && val_vbr_if2->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    if (memcmp(&(val_vbr_if1->portmap), &(val_vbr_if2->portmap),
                sizeof(val_port_map_t))) {
      if (val_vbr_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] == UNC_VF_VALID
          && val_vbr_if2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM]
          == UNC_VF_VALID) {
        if (!strcmp(reinterpret_cast<char *>(val_vbr_if1->portmap.logical_port_id),
                    reinterpret_cast<char *>(val_vbr_if2->portmap.logical_port_id)))
          val_vbr_if1->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] =
              UNC_VF_INVALID;
      }
      if (val_vbr_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID
          && val_vbr_if2->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
        if (val_vbr_if1->portmap.vlan_id == val_vbr_if2->portmap.vlan_id)
          val_vbr_if1->portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_INVALID;
      }
#if 0
      if (val_vbr_if1->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID
          && val_vbr_if2->portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID) {
        if (val_vbr_if1->portmap.tagged == val_vbr_if2->portmap.tagged)
          val_vbr_if1->portmap.valid[UPLL_IDX_TAGGED_PM] = UNC_VF_INVALID;
      }
#endif
    } else {
      val_vbr_if1->valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
    }
  }
  for (unsigned int loop = 0;
       loop < sizeof(val_vbr_if1->valid) / sizeof(val_vbr_if1->valid[0]);
       ++loop) {
    if ((UNC_VF_VALID == (uint8_t) val_vbr_if1->valid[loop]) ||
       (UNC_VF_VALID_NO_VALUE == (uint8_t) val_vbr_if1->valid[loop])) {
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
  GetChildConfigKey(okey, ikey);
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  upll_rc_t result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, 
                                       dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                    UPLL_RC_SUCCESS:result_code;
    return result_code;
  }
  uint8_t vlink_flag = 0;
  GET_USER_DATA_FLAGS(okey,vlink_flag);
  if (vlink_flag & VIF_TYPE) 
    return UPLL_RC_ERR_CFG_SEMANTIC;
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
  uint8_t rename=0;
  bool port_map_valid_status = false;
  bool port_map_status = false;
  val_drv_vbr_if *val_drv_vbr = reinterpret_cast<val_drv_vbr_if *>(GetVal(ikey));
  if (!val_drv_vbr) {
    UPLL_LOG_DEBUG("Val Vbr is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
                                      GetMoManager(UNC_KT_VBRIF_FLOWFILTER)));
  if (!mgr) {
      UPLL_LOG_DEBUG("Invalid Instance");
      return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(okey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failure %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(okey, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Read failed %d\n",result_code);
    return result_code;
  } 
  if (val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    port_map_valid_status = true;
    val_drv_vbr_if *vbr_val_db = reinterpret_cast<val_drv_vbr_if *>(GetVal(okey));
    if (!vbr_val_db) {
      UPLL_LOG_DEBUG("Invalid param\n");
      return UPLL_RC_ERR_GENERIC;
    }
    if (vbr_val_db->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_INVALID) {
      /* portmap getting created for the first time */
      result_code = ConverttoDriverPortMap(ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ConvertToDriverPortMap Failure %d", result_code);
        return result_code;
      }
      port_map_status = true;
    } else {
      /* portmap already exists - only change in vlan/tagged */ 
      val_drv_vbr->valid[UPLL_IDX_VEXT_DRV_PM] = UNC_VF_INVALID;
      val_drv_vbr->valid[UPLL_IDX_VEXT_IF_DRV_PM] = UNC_VF_INVALID;
      val_drv_vbr->valid[UPLL_IDX_VEXT_LINK_DRV_PM] = UNC_VF_INVALID;
    }
  } else if (val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID_NO_VALUE) {
    string s(okey->ToStrAll());

    port_map_valid_status = true;
    GET_USER_DATA_FLAGS(okey, rename);
    if ((rename & VLINK_VNODE1) || (rename & VLINK_VNODE2)) {
        return UPLL_RC_ERR_GENERIC;
    }
    val_drv_vbr->valid[UPLL_IDX_VBR_IF_DRV_PM] = UNC_VF_VALID_NO_VALUE;
    val_drv_vbr->valid[UPLL_IDX_VEXT_DRV_PM] = UNC_VF_VALID_NO_VALUE;
    val_drv_vbr->valid[UPLL_IDX_VEXT_IF_DRV_PM] = UNC_VF_VALID_NO_VALUE;
    val_drv_vbr->valid[UPLL_IDX_VEXT_LINK_DRV_PM] = UNC_VF_VALID_NO_VALUE;
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
     // TODO(karthi): any think inform to POM.. will call it from here
  /* Info to POM */
    port_map_status = false;
  }
  if (port_map_valid_status) { 
    if (port_map_status == true ) {
      UPLL_LOG_DEBUG("Portmapstatus-true");
      result_code = mgr->SetVlinkPortmapConfiguration(okey, datatype, dmi,  kPortMapConfigured);
      if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
         delete okey;
         return result_code; 
      }
    }
    else {
      UPLL_LOG_DEBUG("Portmapstatus-flase");
      result_code = mgr->SetVlinkPortmapConfiguration(okey, datatype, dmi,  kVlinkPortMapNotConfigured);
      if (UPLL_RC_SUCCESS != result_code
          && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("SetVlinkPortMapConfiguration Failure %d", result_code);
        delete okey;
        return result_code;
      }
    }
 }
  DELETE_IF_NOT_NULL(okey);
  result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?UPLL_RC_SUCCESS:result_code;
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
    while (cval ) {
      if (IpctSt::kIpcStPfcdrvValVbrIf == cval->get_st_num()) {
         val_vbr_if_t *vbr_if_val = reinterpret_cast<val_vbr_if_t *>
                                (malloc(sizeof(val_vbr_if_t)));
         if (!vbr_if_val) {
            UPLL_LOG_ERROR("Memory Allocation failed");
            return UPLL_RC_ERR_GENERIC;
         }
         val_drv_vbr_if *vbr_drv_if_val = reinterpret_cast<val_drv_vbr_if *>
                                     (cval->get_val());
         memcpy(vbr_if_val,&(vbr_drv_if_val->vbr_if_val),
               sizeof(val_vbr_if_t));
         cval->SetVal(IpctSt::kIpcStValVbrIf, vbr_if_val);
         /* do not display portmap info if not configured (for boundary)*/
         uint8_t vlink_flag = 0;
         GET_USER_DATA_FLAGS(ikey,vlink_flag);
         UPLL_LOG_DEBUG("Interface type %d\n",vlink_flag);
         if (vlink_flag & VIF_TYPE)
           vbr_if_val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
      }
      if (IpctSt::kIpcStValVbrIfSt == cval->get_st_num()) {
        val_vbr_if_st *vbr_if_val_st = reinterpret_cast<val_vbr_if_st *>
                         (malloc(sizeof(val_vbr_if_st)));
        if (!vbr_if_val_st) {
          UPLL_LOG_ERROR("Memory Allocation failed");
          return UPLL_RC_ERR_GENERIC;
        }
        val_db_vbr_if_st *db_vbr_if_val_st = reinterpret_cast<val_db_vbr_if_st *>
                                     (cval->get_val());
        memcpy(vbr_if_val_st,&(db_vbr_if_val_st->vbr_if_val_st),
               sizeof(val_vbr_if_st));
        cval->SetVal(IpctSt::kIpcStValVbrIfSt, vbr_if_val_st);
      }
      cval = cval->get_next_cfg_val();
    }
    ikey = ikey->get_next_cfg_key_val();
  }
  return UPLL_RC_SUCCESS;
}

/*This function gives the interfaces mapped to a particular vbridge */
upll_rc_t VbrIfMoMgr::GetMappedInterfaces(key_vbr_t *vbr_key,
                                          DalDmlIntf *dmi,
                                          ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  /* Allocate memory for Vbridge interface key and value structure */
  key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t *>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
  val_vbr_if_t *vbr_if_val = reinterpret_cast<val_vbr_if_t *>
      (ConfigKeyVal::Malloc(sizeof(val_vbr_if_t)));

  /* initialize the parent vbr key */
  vbr_if_key->vbr_key = *vbr_key;

  /*Getting only mapped interfaces - set portmap to valid status*/
  vbr_if_val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  ConfigVal *tempval = new ConfigVal(IpctSt::kIpcStValVbrIf, vbr_if_val);
  ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf, vbr_if_key,
                          tempval);
  /* Get all the vbridges under the VTN */
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  return result_code;
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
                                              (malloc(sizeof(val_drv_vbr_if)));
  if (!vbr_if_val) {
    UPLL_LOG_DEBUG("Invalid vbrif_val");
    return UPLL_RC_ERR_GENERIC;
  }
  memset(vbr_if_val, 0, sizeof(val_drv_vbr_if));
  //vbr_if_val->valid[UPLL_IDX_VBR_IF_DRV_PM]  = UNC_VF_VALID;
  //vbr_if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  //vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  ikey->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrIf, vbr_if_val);

  /* Get the vbridgeIf instance from db */
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
 switch (dt_type) {
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_IMPORT:
    case UPLL_DT_RUNNING:
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
      iftype = kVlinkConfigured; //linked
    }

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

/*This function gives the bridges connected to a particular switch log port*/
upll_rc_t VbrIfMoMgr::GetMappedVbridges(uint8_t *logportid, DalDmlIntf *dmi,
                                        set<key_vnode_t>* sw_vbridge_set) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>
                       (ConfigKeyVal::Malloc(sizeof(key_vbr_t)));
  key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if *>
                             (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));
  val_vbr_if_t *vbr_if_val = reinterpret_cast<val_vbr_if *>
                             (ConfigKeyVal::Malloc(sizeof(val_vbr_if_t)));

  /* copy switch id to val_vbr_if strcuture */
  uuu::upll_strncpy(vbr_if_val->portmap.logical_port_id, logportid,
                   (kMaxLenPortName + 1));

  /*Setting portmap and switch_id valid status*/
  vbr_if_val->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
  vbr_if_val->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
  ConfigVal *tempval = new ConfigVal(IpctSt::kIpcStValVbrIf, vbr_if_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF, IpctSt::kIpcStKeyVbrIf,
                                        vbr_if_key, tempval);
  /* Get all the vbridges under the VTN */
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Error in reading: %d", result_code);
    free(vbr_key);
    return result_code;
  }
  /* populate sw_vbridge set with vbridges mapped to the specified switch*/
  ConfigKeyVal *iter_key = ikey;
  while (iter_key != NULL) {
    key_vbr_if_t *tkey = reinterpret_cast<key_vbr_if_t *>
                         (iter_key->get_key());
    uuu::upll_strncpy(vbr_key->vbridge_name, tkey->vbr_key.vbridge_name,
                      (kMaxLenVnodeName + 1));
    uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                      tkey->vbr_key.vtn_key.vtn_name,
                      (kMaxLenVtnName + 1));
    sw_vbridge_set->insert(*(reinterpret_cast<key_vnode_t *>(vbr_key)));
    iter_key = iter_key->get_next_cfg_key_val();
  }
  delete ikey;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfMoMgr::AuditUpdateController(unc_key_type_t keytype,
                             const char *ctrlr_id,
                             uint32_t session_id,
                             uint32_t config_id,
                             uuc::UpdateCtrlrPhase phase,
                             bool *ctrlr_affected,
                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result = uud::kDalRcSuccess;
  controller_domain_t print_ctrlr_dom;
  print_ctrlr_dom.ctrlr = NULL;
  print_ctrlr_dom.domain = NULL;
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
  UPLL_LOG_DEBUG("Operation is %d\n", op); 
  result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT, op,
        ckv_running_db, ckv_audit_db,
        &cursor, dmi, ctrlr, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
    return result_code;
  }
  if (cursor == NULL) {
      UPLL_LOG_DEBUG("cursor is null");
      return UPLL_RC_ERR_GENERIC;
  }
  while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor))) {
    #if 1
    UPLL_LOG_DEBUG("Diff Record: Keytype: Operation:  is %d\n %d\n %s\n", keytype, op, ckv_running_db->ToStrAll().c_str());
    GET_USER_DATA_CTRLR_DOMAIN(ckv_running_db,print_ctrlr_dom);
    if (print_ctrlr_dom.ctrlr != NULL ) {
      UPLL_LOG_DEBUG("print_ctrlr_dom.ctrlr is %s", print_ctrlr_dom.ctrlr);
    }
    if (print_ctrlr_dom.domain != NULL ) {
      UPLL_LOG_DEBUG("print_ctrlr_dom.domain is %s\n",print_ctrlr_dom.domain);
    }
    #endif
    switch (phase) {
      case uuc::kUpllUcpDelete:
        UPLL_LOG_DEBUG("Deleted record is %s\n ",ckv_running_db->ToStrAll().c_str());
        result_code = GetChildConfigKey(ckv_driver_req, ckv_running_db);
        UPLL_LOG_DEBUG("ckv_driver_req in delete is %s\n", ckv_driver_req->ToStrAll().c_str());
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed. err_code & phase %d %d\n",
                           result_code, phase);
          return result_code;
        }
        if (ckv_driver_req->get_cfg_val()) {
          UPLL_LOG_DEBUG("Invalid param");
          return UPLL_RC_ERR_GENERIC;
        }
        result_code = ReadConfigDB(ckv_driver_req, UPLL_DT_RUNNING, UNC_OP_READ,
                                             dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d",result_code);
          return UPLL_RC_ERR_GENERIC;
        }
        drv_vbr_if = reinterpret_cast<val_drv_vbr_if_t *>
           (GetVal(ckv_driver_req));
        if (!drv_vbr_if) {
          UPLL_LOG_DEBUG("drv_vbr_if is NULL");
          return UPLL_RC_ERR_GENERIC;
        }
        if (drv_vbr_if->vbr_if_val.valid[UPLL_IDX_PM_DRV_VBRI] != UNC_VF_VALID) {
          ckv_driver_req->DeleteCfgVal();
        } else {
           drv_vbr_if->valid[0] = UNC_VF_INVALID;
           for (int i=0; i < 3 ; i++) {
             if (drv_vbr_if->vbr_if_val.valid[i] != UNC_VF_INVALID) {
               drv_vbr_if->valid[0] = UNC_VF_VALID;
               break;
             }
           }
        }
        break;
      case uuc::kUpllUcpCreate:
          UPLL_LOG_DEBUG("Created  record is %s\n ",ckv_running_db->ToStrAll().c_str());
          result_code = DupConfigKeyVal(ckv_driver_req, ckv_running_db, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed. err_code & phase %d %d\n",
                           result_code, phase);
            return result_code;
          }
         break;
      case uuc::kUpllUcpUpdate:
          UPLL_LOG_DEBUG("UpdateRecord  record  is %s\n ",ckv_running_db->ToStrAll().c_str());
          UPLL_LOG_DEBUG("UpdateRecord  record  is %s\n ",ckv_audit_db->ToStrAll().c_str());
          result_code = DupConfigKeyVal(ckv_driver_req, ckv_running_db, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for running record. \
                            err_code & phase %d %d\n", result_code, phase);
            return result_code;
          }
          result_code = DupConfigKeyVal(ckv_audit_dup_db, ckv_audit_db, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for audit record. \
                           err_code & phase %d %d\n", result_code, phase);
            return result_code;
          }
          if (GetVal(ckv_driver_req) != NULL &&
            GetVal(ckv_audit_dup_db) != NULL) {
            void *val1 = GetVal(ckv_driver_req);
            FilterAttributes(val1, GetVal(ckv_audit_dup_db), false,
                           UNC_OP_UPDATE);
        }
        break;
      default:
        UPLL_LOG_DEBUG("Invalid operation %d\n", phase);
        return UPLL_RC_ERR_NO_SUCH_OPERATION;
        break;
    }
    GET_USER_DATA_CTRLR_DOMAIN(ckv_driver_req, ctrlr_dom);
    if ((NULL == ctrlr_dom.ctrlr) || (NULL == ctrlr_dom.domain)) {
      UPLL_LOG_INFO("controller id or domain is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);
    IpcResponse ipc_response;
    memset(&ipc_response, 0, sizeof(IpcResponse));
    IpcRequest ipc_req;
    memset(&ipc_req, 0, sizeof(IpcRequest));
    ipc_req.header.clnt_sess_id = session_id;
    ipc_req.header.config_id = config_id;
    ipc_req.header.operation = op;
    ipc_req.header.datatype = UPLL_DT_CANDIDATE;
    ipc_req.ckv_data = ckv_driver_req;
    if (!IpcUtil::SendReqToDriver((const char *)ctrlr_dom.ctrlr, reinterpret_cast<char *>
                                  (ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME, 
                                  PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_response)) {
      UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                    ckv_driver_req->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
      return UPLL_RC_ERR_GENERIC;
    }
    if  (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("driver return failure err_code is %d\n", ipc_response.header.result_code);
        ConfigKeyVal *resp = NULL;
        result_code = DupConfigKeyVal(resp, ipc_response.ckv_data);
        if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_DEBUG("DupConfigKeyVal failed for ipc response ckv err_code %d\n",
                           result_code);
           delete ipc_response.ckv_data;
           return result_code;
        }     
        result_code = UpdateAuditConfigStatus(UNC_CS_INVALID, phase, resp);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Update Audit config status failed %d",
                  result_code);
          return result_code;
        }
        result_code = UpdateConfigDB(resp, UPLL_DT_RUNNING, UNC_OP_UPDATE,
                                       dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("UpdateConfigDB failed for ipc response ckv err_code %d\n",
                          result_code);
          return result_code;
        }  
        if (keytype == UNC_KT_VTN) {
          result_code = SetConsolidatedStatus(resp, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("SetConsolidatedStatus failed for ipc response ckv err_code %d\n",
                            result_code); 
            return result_code;
          }
        }
        if (resp)
          delete resp;
    }
    if (ckv_driver_req) {
     delete ckv_driver_req;
     ckv_driver_req = NULL;
    }
    *ctrlr_affected = true;
  }
  dmi->CloseCursor(cursor, true);
  if (uud::kDalRcSuccess != db_result) {
     UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d", db_result);
     result_code =  DalToUpllResCode(db_result);
  }
  if (ckv_running_db)
     delete ckv_running_db;
  if (ckv_audit_db)
      delete ckv_audit_db;
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
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  ConfigKeyVal *req, *nreq = NULL, *ck_main = NULL;
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
  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                     op, req, nreq, &dal_cursor_handle, dmi, MAINTBL);
  while (result_code == UPLL_RC_SUCCESS) {
      //  Get Next Record
    db_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS)
      break;
    switch (op)   {
      case UNC_OP_CREATE:
      case UNC_OP_UPDATE:
     /* fall through intended */
        result_code = DupConfigKeyVal(ck_main, req);
        break;
     case UNC_OP_DELETE:
       result_code = GetChildConfigKey(ck_main, req);
     default:
       break;
    }
    GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
    if (ctrlr_dom.ctrlr == NULL) {
        return UPLL_RC_ERR_GENERIC;
    }
    uint8_t bound_vlink = 0;
    GET_USER_DATA_FLAGS(ck_main,bound_vlink);
    if (op == UNC_OP_DELETE) {
       if (ck_main->get_cfg_val()) {
         UPLL_LOG_DEBUG("Invalid param");
         return UPLL_RC_ERR_GENERIC;
       }
       DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone};
       result_code = ReadConfigDB(ck_main, UPLL_DT_RUNNING, UNC_OP_READ,
                                             dbop, dmi, MAINTBL);
       if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("Returning error %d",result_code);
         return UPLL_RC_ERR_GENERIC;
       }
       val_drv_vbr_if_t *val_vbr = reinterpret_cast<val_drv_vbr_if_t *>
          (GetVal(ck_main));
       if (val_vbr->vbr_if_val.valid[UPLL_IDX_PM_DRV_VBRI] != UNC_VF_VALID) {
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
      UPLL_LOG_TRACE("%s \n",ck_main->ToStrAll().c_str());
    }
    val_drv_vbr_if *vbr_ifval = reinterpret_cast<val_drv_vbr_if *>
      (GetVal(ck_main));
    if (vbr_ifval) {
      vbr_ifval->valid[0] = UNC_VF_INVALID;
      for (int i=0; i < 3 ; i++) {
        if (vbr_ifval->vbr_if_val.valid[i] != UNC_VF_INVALID) {
          vbr_ifval->valid[0] = UNC_VF_VALID;
          break;
        }
      }
      for (int i=1; i < 4 ; i++)
        vbr_ifval->valid[i] = vbr_ifval->vbr_if_val.valid[UPLL_IDX_PM_DRV_VBRI];
      switch (vbr_ifval->vbr_if_val.valid[UPLL_IDX_PM_DRV_VBRI]) { 
      case UNC_VF_VALID_NO_VALUE :
      {
        UPLL_LOG_TRACE("entering valid-no-value\n");
        val_drv_vbr_if *oval_vbrif = reinterpret_cast<val_drv_vbr_if *>
                                   (GetVal(nreq));
        if (!oval_vbrif) {
          UPLL_LOG_DEBUG("Invalid param\n");
          return UPLL_RC_ERR_GENERIC;
        }
        vbr_ifval->valid[UPLL_IDX_VEXT_DRV_PM] = UNC_VF_VALID;
        vbr_ifval->valid[UPLL_IDX_VEXT_IF_DRV_PM] = UNC_VF_VALID;
        vbr_ifval->valid[UPLL_IDX_VEXT_LINK_DRV_PM] = UNC_VF_VALID;
        uuu::upll_strncpy(vbr_ifval->vex_name, oval_vbrif->vex_name, 
                          kMaxLenVnodeName+1);
        uuu::upll_strncpy(vbr_ifval->vex_if_name, oval_vbrif->vex_if_name, 
                          kMaxLenVnodeName+1);
        uuu::upll_strncpy(vbr_ifval->vex_link_name, oval_vbrif->vex_link_name,
                          kMaxLenVnodeName+1);
        break;
      } 
      case UNC_VF_VALID:
        // if it is update operation where only the logicalport/vlanid/tag is getting updated
        // set vex/vexif/vexlink to invalid 
       if (op == UNC_OP_UPDATE) { 
         val_drv_vbr_if *db_ifval = reinterpret_cast<val_drv_vbr_if *>
                                                    (GetVal(nreq));
         if (!db_ifval) {
           UPLL_LOG_TRACE("Invalid param\n");
           return UPLL_RC_ERR_GENERIC;
         }
         if ((db_ifval->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) &&
             (db_ifval->vbr_if_val.portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] 
                   == UNC_VF_VALID)) {
            /* portmap is updated - fill in vex,vexlink - 
               vexif set to invalid for portmap update
             */
            vbr_ifval->valid[UPLL_IDX_VEXT_DRV_PM] = UNC_VF_VALID;
            uuu::upll_strncpy(vbr_ifval->vex_name, db_ifval->vex_name, 
                          kMaxLenVnodeName+1);
            vbr_ifval->valid[UPLL_IDX_VEXT_IF_DRV_PM] = UNC_VF_INVALID;
            vbr_ifval->valid[UPLL_IDX_VEXT_LINK_DRV_PM] = UNC_VF_VALID;
            uuu::upll_strncpy(vbr_ifval->vex_link_name, db_ifval->vex_link_name,
                          kMaxLenVnodeName+1);
            if (vbr_ifval->vbr_if_val.portmap.valid[UPLL_IDX_TAGGED_PM] == UNC_VF_VALID) { 
              if (vbr_ifval->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_INVALID) { 
                 vbr_ifval->vbr_if_val.portmap.vlan_id  =  db_ifval->vbr_if_val.portmap.vlan_id;
                 vbr_ifval->vbr_if_val.portmap.valid[UPLL_IDX_VLAN_ID_PM] = UNC_VF_VALID; 
              }
            }
         } 
         if (vbr_ifval->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_INVALID) {
            /* set port admin status to disable */
           if ((db_ifval->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_VALID) && 
                  (db_ifval->vbr_if_val.admin_status == UPLL_ADMIN_DISABLE)) {
              vbr_ifval->vbr_if_val.admin_status = UPLL_ADMIN_DISABLE; 
              vbr_ifval->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_VALID;
           }
         }
       }
       break;
      default:
        break;
      }
    }
    UPLL_LOG_TRACE("%s \n",ck_main->ToStrAll().c_str());
    if (!OVERLAY_KT(keytype)) {
      upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
             UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;
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
    result_code = SendIpcReq(session_id, config_id, op,
      UPLL_DT_CANDIDATE, ck_main, &ctrlr_dom, &ipc_resp);
    if (result_code == UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
      result_code = UPLL_RC_SUCCESS;
      UPLL_LOG_DEBUG("Controller disconnected");
    }
    if (result_code != UPLL_RC_SUCCESS) {
      ConfigKeyVal *ck_vlink = NULL;
      /* Validating if VbridgeIf is a node for Vlink */
      if ((bound_vlink & VIF_TYPE_BOUNDARY) == 0x0)
        *err_ckv = ipc_resp.ckv_data;
      else {
        VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>
                    (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
        vn_if_type iftype;
        if (!mgr) {
          UPLL_LOG_DEBUG("Invalid mgr");
          return UPLL_RC_ERR_GENERIC;
        }
        result_code = mgr->CheckIfMemberOfVlink(ipc_resp.ckv_data,
                     UPLL_DT_CANDIDATE, ck_vlink,dmi,iftype);
        if (result_code == UPLL_RC_SUCCESS) {
          delete ipc_resp.ckv_data;
          *err_ckv = ck_vlink;
        } else  {
          *err_ckv = ipc_resp.ckv_data;
          if (ck_vlink) delete ck_vlink;
          UPLL_LOG_DEBUG("Failed to map boundary if to vlink");
        }
      }
      break;
    }
    if (ck_main)
      delete ck_main;
    ck_main = NULL;
  }
  dmi->CloseCursor(dal_cursor_handle, true);
  if (req)
    delete req;
  if (nreq)
    delete nreq;
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?UPLL_RC_SUCCESS:result_code;
  return result_code;
}

upll_rc_t VbrIfMoMgr::PortStatusHandler(const char *ctrlr_name,
    const char *domain_name, const char *portid,
    bool oper_status, DalDmlIntf *dmi  ) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_TRACE("portid :(%s) domain_id :(%s) notification:(%d)",
      portid, domain_name, oper_status);
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  /* Allocate Memory for UNC_KT_VBR_IF key and value structure */
  val_drv_vbr_if *vbrif_val = reinterpret_cast<val_drv_vbr_if*>
                           (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
  val_port_map_t *pm = &vbrif_val->vbr_if_val.portmap;
  pm->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;

  /* Copy port_id from input to port_id variable in portmap structure */
  uuu::upll_strncpy(pm->logical_port_id, portid,
                    sizeof(pm->logical_port_id));
  /* set valid flag as VALID */
  pm->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;

  ConfigKeyVal *vbrifkey = NULL;
  result_code = GetChildConfigKey(vbrifkey,NULL);
  if (!vbrifkey || (result_code != UPLL_RC_SUCCESS)) {
    free(vbrif_val);
    if(vbrifkey) delete vbrifkey;
    return UPLL_RC_ERR_GENERIC;
  }

  vbrifkey->AppendCfgVal(IpctSt::kIpcStValVbrIf, vbrif_val);

  state_notification notification =
       (oper_status == UPLL_OPER_STATUS_UP) ? kPortFaultReset : kPortFault;
#if 0
  /* Get all the Vbridgeinterfaces under the same port_name and switch-id */
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  result_code = ReadConfigDB(vbrifkey, UPLL_DT_STATE, UNC_OP_READ, dbop,
      dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Error in reading %d", result_code);
    return result_code;
  }
  /* Iterate vbr_if list one by one */
  ConfigKeyVal *tmp = vbrifkey;
  while (tmp != NULL) {
    unc_key_type_t keytype = tmp->get_key_type();

    SetOperStatus<val_vbr_if, val_db_vbr_if_st>
      (tmp, keytype, IpctSt::kIpcStKeyVbrIf, dmi, notification);
    /* Iterate Next record */
    tmp = tmp->get_next_cfg_key_val();
  }
#endif
  result_code = UpdateOperStatus(vbrifkey,dmi,notification,false);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Invalid oper status update %d",result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  delete vbrifkey;
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
