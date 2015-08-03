/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vbr_if_flowfilter_momgr.hh"
#include "vbr_if_momgr.hh"
#include "vbr_if_flowfilter_entry_momgr.hh"
#include "uncxx/upll_log.hh"


namespace unc {
namespace upll {
namespace kt_momgr {

using unc::upll::ipc_util::IpcUtil;
#define VTN_RENAME_FLAG   0x01
#define VBR_RENAME_FLAG   0x02
#define SET_FLAG_VLINK    0x40
#define SET_FLAG_PORTMAP  0x20
#define SET_FLAG_VLINK_PORTMAP (SET_FLAG_VLINK | SET_FLAG_PORTMAP)
#define SET_FLAG_NO_VLINK_PORTMAP 0x9F

// VbrIfFlowFilter Table(Main Table)
BindInfo VbrIfFlowFilterMoMgr::vbr_if_flowfilter_bind_info[] = {
  { uudst::vbr_if_flowfilter::kDbiVtnName, CFG_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_if_flowfilter::kDbiVbrName, CFG_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_if_flowfilter::kDbiVbrIfName, CFG_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vbr_if_flowfilter::kDbiInputDirection, CFG_KEY,
    offsetof(key_vbr_if_flowfilter_t, direction),
    uud::kDalUint8, 1 },
  { uudst::vbr_if_flowfilter::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vbr_if_flowfilter::kDbiDomainId, CK_VAL,
    offsetof(key_user_data, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vbr_if_flowfilter::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vbr_if_flowfilter::kDbiCsRowStatus, CS_VAL,
    offsetof(val_flowfilter_t, cs_row_status),
    uud::kDalUint8, 1 },
};

BindInfo VbrIfFlowFilterMoMgr::vbr_if_flowfiltermaintbl_bind_info[] = {
  { uudst::vbr_if_flowfilter::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.vbr_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_if_flowfilter::kDbiVbrName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.vbr_key.vbridge_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_if_flowfilter::kDbiVbrIfName, CFG_MATCH_KEY,
    offsetof(key_vbr_if_flowfilter_t, if_key.if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vbr_if_flowfilter::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vbr_if_flowfilter::kDbiVbrName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vnode_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vbr_if_flowfilter::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

unc_key_type_t VbrIfFlowFilterMoMgr::vbr_if_flowfilter_child[] = {
  UNC_KT_VBRIF_FLOWFILTER_ENTRY };

VbrIfFlowFilterMoMgr::VbrIfFlowFilterMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  // setting  table indexed for ctrl and rename table to NULL;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  // For Main Table
  table[MAINTBL] = new Table(uudst::kDbiVbrIfFlowFilterTbl,
      UNC_KT_VBRIF_FLOWFILTER, vbr_if_flowfilter_bind_info,
      IpctSt::kIpcStKeyVbrIfFlowfilter, IpctSt::kIpcStValFlowfilter,
      uudst::vbr_if_flowfilter::kDbiVbrIfFlowFilterNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;

  nchild = sizeof(vbr_if_flowfilter_child) / sizeof(vbr_if_flowfilter_child[0]);
  child = vbr_if_flowfilter_child;
}

upll_rc_t VbrIfFlowFilterMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  if (ikey == NULL || req == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE("ikey= %s", ikey->ToStrAll().c_str());

  if (req->datatype == UPLL_DT_IMPORT) {
    UPLL_LOG_DEBUG("Inside:-%d", req->datatype);
     if (ikey->get_cfg_val() &&
            (ikey->get_cfg_val()->get_st_num() ==
                 IpctSt::kIpcStPfcdrvValVbrifVextif)) {
        UPLL_LOG_DEBUG("val struct num (%d)",
                       ikey->get_cfg_val()->get_st_num());
        ikey->SetCfgVal(NULL);
     }
  }

  if (req->datatype != UPLL_DT_IMPORT) {
    // validate syntax and semantics
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed, Error - %d", result_code);
      return result_code;
    }
  }

  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed, Error - %d", result_code);
    return result_code;
  }

  // create a record in CANDIDATE DB
  VbrIfMoMgr *mgrvbrif =
      reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
                  UNC_KT_VBR_IF)));
  ConfigKeyVal *ckv = NULL;

  InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;

  result_code = mgrvbrif->GetChildConfigKey(ckv, NULL);
  key_vbr_if_flowfilter_t *ff_key = reinterpret_cast
      <key_vbr_if_flowfilter_t *>(ikey->get_key());
  key_vbr_if_t *vbrif_key = reinterpret_cast<key_vbr_if_t *>(ckv->get_key());

  uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
                    ff_key->if_key.vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);

  uuu::upll_strncpy(vbrif_key->vbr_key.vbridge_name,
                    ff_key->if_key.vbr_key.vbridge_name,
                    kMaxLenVtnName + 1);

  uuu::upll_strncpy(vbrif_key->if_name,
                    ff_key->if_key.if_name,
                    kMaxLenInterfaceName + 1);

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  result_code = GetControllerDomainID(ikey, req->datatype, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
    DELETE_IF_NOT_NULL(ckv);
    UPLL_LOG_INFO("Failed to get the controller domain details, err:%d",
                  result_code);
    return result_code;
  }
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);
  // check whether this keytype is not requested under unified vbridge
  if (IsUnifiedVbr(ctrlr_dom.ctrlr)) {
     UPLL_LOG_DEBUG(
         "This KT is not allowed to be created under Unified Vbridge");
     DELETE_IF_NOT_NULL(ckv);
     return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  uint8_t* vexternal = reinterpret_cast<uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenVnodeName + 1));
  uint8_t* vex_if = reinterpret_cast<uint8_t*>
      (ConfigKeyVal::Malloc(kMaxLenInterfaceName + 1));
  result_code = mgrvbrif->GetVexternal(ckv, req->datatype, dmi,
                                       vexternal, vex_if, flags,
                          reinterpret_cast<const char*>(ctrlr_dom.ctrlr));
  FREE_IF_NOT_NULL(vexternal);
  FREE_IF_NOT_NULL(vex_if);

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetVexternal failed %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }

  /* VlanmapOnBoundary: vbr_if_flowfilter is not allowed on
   * vbr interface of boundary vlink which is SW or SD mapped */
  VlinkMoMgr *vlink_mgr =
      reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager*>(GetMoManager(
                  UNC_KT_VLINK)));
  if (!vlink_mgr) {
    UPLL_LOG_DEBUG("vlink_mgr is Null");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ck_vlink = NULL;
  vn_if_type iftype;

  /* Verifies whether the requested vbridge interface is boundary
   * vlinked with logical port id is SW or SD. If it is part of SW or SD
   * boundary linked, cannot configure flo-filter in that vbridge interface*/
  result_code =  vlink_mgr->CheckIfMemberOfVlink(ckv,
                                                 req->datatype, ck_vlink,
                                                 dmi, iftype);
  if (result_code == UPLL_RC_SUCCESS) {
    unc_key_type_t ktype1;
    if (iftype == kVlinkBoundaryNode1) {
      UPLL_LOG_DEBUG("iftype is kVlinkBoundaryNode1");
      ktype1 = vlink_mgr->GetVlinkVnodeIfKeyType(ck_vlink, 0);
      if (ktype1 == UNC_KT_VBR_VLANMAP) {
        UPLL_LOG_DEBUG("iftype is kVlinkBoundaryNode1 And "
                       "ktype is UNC_KT_VBR_VLANMAP");
        DELETE_IF_NOT_NULL(ckv);
        DELETE_IF_NOT_NULL(ck_vlink);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    } else if (iftype == kVlinkBoundaryNode2) {
      UPLL_LOG_DEBUG("iftype is kVlinkBoundaryNode2");
      ktype1 = vlink_mgr->GetVlinkVnodeIfKeyType(ck_vlink, 1);
      if (ktype1 == UNC_KT_VBR_VLANMAP) {
        UPLL_LOG_DEBUG("iftype is kVlinkBoundaryNode2 And "
                       "ktype is UNC_KT_VBR_VLANMAP");
        DELETE_IF_NOT_NULL(ckv);
        DELETE_IF_NOT_NULL(ck_vlink);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      }
    }
  } else if (result_code != UPLL_RC_SUCCESS &&
            result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(ckv);
    UPLL_LOG_DEBUG("CheckIfMemberOfVlink is Fail");
    return result_code;
  }
  DELETE_IF_NOT_NULL(ck_vlink);
  /* If its not member of vlink continue below steps */

  DELETE_IF_NOT_NULL(ckv);
  uint8_t flag_port_map = 0;
  GET_USER_DATA_FLAGS(ikey, flag_port_map);
  if (flags & kVlinkConfigured) {
    flag_port_map = flag_port_map|SET_FLAG_VLINK;
  } else if (flags & kPortMapConfigured) {
    flag_port_map = flag_port_map|SET_FLAG_PORTMAP;
  } else if (flags & kVlinkPortMapConfigured) {
    flag_port_map = flag_port_map|SET_FLAG_VLINK_PORTMAP;
  }

  SET_USER_DATA_FLAGS(ikey, flag_port_map);

  result_code = ValidateCapability(
      req, ikey,
      reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
    return result_code;
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

  DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain |
    kOpInOutFlag};
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                               dmi, &dbop1, config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Failed to update DB, err %d", result_code);
    return result_code;
  }
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  uint8_t flags = 0;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE(" ikey is %s", ikey->ToStrAll().c_str());
  pfcdrv_val_vbrif_vextif_t *pfc_val =
    reinterpret_cast<pfcdrv_val_vbrif_vextif_t *> ((GetVal(ikey)));

  if (pfc_val == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  GET_USER_DATA_FLAGS(ikey, flags);

  if (pfc_val->interface_type == PFCDRV_IF_TYPE_VBRIF) {
    flags |= SET_FLAG_VLINK;
  } else if (pfc_val->interface_type == PFCDRV_IF_TYPE_VEXTIF) {
    flags |= SET_FLAG_PORTMAP;
  }

  ConfigKeyVal *okey = NULL;
  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed :%d",
                   result_code);
    return result_code;
  }

  SET_USER_DATA_FLAGS(okey, flags);

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  result_code = GetControllerDomainID(okey, UPLL_DT_AUDIT, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Failed to Get the Controller Domain details, err:%d",
                   result_code);
  }

  GET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);

  // Create a record in AUDIT DB
  std::string vtn_name = "";
  result_code = UpdateConfigDB(okey, UPLL_DT_AUDIT,
                               UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL,
                               vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateConfigDB Failed err_code %d", result_code);
  }

  delete okey;
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::AuditUpdateController(unc_key_type_t keytype,
                             const char *ctrlr_id,
                             uint32_t session_id,
                             uint32_t config_id,
                             uuc::UpdateCtrlrPhase phase1,
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
  upll_keytype_datatype_t vext_datatype =  UPLL_DT_CANDIDATE;
  uint8_t db_flag = 0;
  uint8_t auditdb_flag = 0;
  uint8_t *ctrlr = reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id));
  string vtn_name = "";

  if (phase1 == uuc::kUpllUcpCreate) {
    return result_code;
  }
  unc_keytype_operation_t op[2] = {UNC_OP_INVALID, UNC_OP_INVALID};
  int nop = 0;
  if (phase1 == uuc::kUpllUcpUpdate) {
    op[0] = UNC_OP_UPDATE;
    op[1] = UNC_OP_CREATE;
    nop = 2;
  } else if (phase1 == uuc::kUpllUcpDelete) {
    op[0] = UNC_OP_DELETE;
    nop = 1;
  }
  for (int i = 0; i < nop; i++) {
    /* retreives the delta of running and audit configuration */
    cursor = NULL;
    unc_keytype_operation_t op1 = op[i];
    uuc::UpdateCtrlrPhase phase = (op[i] == UNC_OP_UPDATE)?uuc::kUpllUcpUpdate:
      ((op[i] == UNC_OP_CREATE)?uuc::kUpllUcpCreate:
       ((op[i] == UNC_OP_DELETE)?uuc::kUpllUcpDelete:uuc::kUpllUcpInvalid));
    UPLL_LOG_DEBUG("Operation is %d", op[i]);
    result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT, op[i],
        ckv_running_db, ckv_audit_db,
        &cursor, dmi, ctrlr, TC_CONFIG_GLOBAL, vtn_name, MAINTBL, true, true);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_SUCCESS;
      UPLL_LOG_DEBUG("No more diff found for operation %d", op[i]);
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      continue;
    }
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      return result_code;
    }
    if (cursor == NULL) {
      UPLL_LOG_DEBUG("cursor is null");
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      return UPLL_RC_ERR_GENERIC;
    }
    while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor)) &&
           ((result_code = ContinueAuditProcess()) == UPLL_RC_SUCCESS)) {
      op1 = op[i];
      if (phase != uuc::kUpllUcpDelete) {
        uint8_t *db_ctrlr = NULL;
        GET_USER_DATA_CTRLR(ckv_running_db, db_ctrlr);
        UPLL_LOG_DEBUG("db ctrl_id and audit ctlr_id are  %s %s",
                        db_ctrlr, ctrlr_id);
        // Skipping the controller ID if the controller id in DB and
        // controller id available for Audit are not the same
        if (db_ctrlr && strncmp(
                reinterpret_cast<const char *>(db_ctrlr),
                reinterpret_cast<const char *>(ctrlr_id),
                strlen(reinterpret_cast<const char *>(ctrlr_id)) + 1)) {
          continue;
        }
      }

      switch (phase) {
        case uuc::kUpllUcpDelete:
        case uuc::kUpllUcpCreate:
          UPLL_LOG_TRACE("Created  record is %s ",
                          ckv_running_db->ToStrAll().c_str());
          result_code = DupConfigKeyVal(ckv_driver_req,
                                        ckv_running_db, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed. err_code & phase %d %d",
                           result_code, phase);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }

          if (phase == uuc::kUpllUcpDelete)
            GET_USER_DATA_FLAGS(ckv_driver_req, auditdb_flag);
         break;
        case uuc::kUpllUcpUpdate:
          ckv_audit_dup_db = NULL;
          ckv_driver_req = NULL;
          UPLL_LOG_TRACE("UpdateRecord  record  is %s ",
                          ckv_running_db->ToStrAll().c_str());
          UPLL_LOG_TRACE("UpdateRecord  record  is %s ",
                          ckv_audit_db->ToStrAll().c_str());
          result_code = DupConfigKeyVal(ckv_driver_req,
                                        ckv_running_db, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for running record."
                           " err_code & phase %d %d", result_code, phase);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          result_code = DupConfigKeyVal(ckv_audit_dup_db, ckv_audit_db,
                                        MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for audit record. "
                           "err_code & phase %d %d", result_code, phase);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          GET_USER_DATA_FLAGS(ckv_audit_dup_db, auditdb_flag);
          break;
        default:
          UPLL_LOG_DEBUG("Invalid operation %d", phase);
          return UPLL_RC_ERR_NO_SUCH_OPERATION;
          break;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ckv_driver_req, ctrlr_dom);
      if ((NULL == ctrlr_dom.ctrlr) || (NULL == ctrlr_dom.domain)) {
        UPLL_LOG_INFO("controller id or domain is NULL");
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_audit_dup_db);
        dmi->CloseCursor(cursor, true);
        return UPLL_RC_ERR_GENERIC;
      }
      UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                     ctrlr_dom.domain);
      db_flag = 0;
      GET_USER_DATA_FLAGS(ckv_driver_req, db_flag);
      uint8_t running_db_flag = 0;
      uint8_t audit_db_flag = 0;
      GET_USER_DATA_FLAGS(ckv_running_db, running_db_flag);
      if (UNC_OP_UPDATE == op1)
        GET_USER_DATA_FLAGS(ckv_audit_db, audit_db_flag);
      // If port-map/vLink flag is not set at running and the operation is
      // UPDATE,check if port-map/vLink flag is set in audit DB,
      // then port-map/vLink is deleted in the UPDATE phase from UNC
      // delete request is to be sent to the driver.
      // If port-map/vLink flag is set in running and the operation is
      // UPDATE, check if port-map/vLink flag is set in audit DB,
      // if not set, then port-map/vLink is created during UPDATE phase.
      // flowfilter create request should be sent to the driver.

      if ((!(SET_FLAG_PORTMAP & running_db_flag)) &&
          (!(SET_FLAG_VLINK & running_db_flag))) {
        if (op1 != UNC_OP_UPDATE) {
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(ckv_audit_dup_db);
          continue;
        } else {
          if ((!(SET_FLAG_PORTMAP & audit_db_flag)) &&
              (!(SET_FLAG_VLINK & audit_db_flag))) {
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ckv_audit_dup_db);
            continue;
          }
          op1 = UNC_OP_DELETE;
        }
      } else if (op1 == UNC_OP_UPDATE) {
        if ((!(SET_FLAG_PORTMAP & audit_db_flag)) &&
            (!(SET_FLAG_VLINK & audit_db_flag))) {
          op1 = UNC_OP_CREATE;
        }
      }

      running_db_flag &= SET_FLAG_VLINK_PORTMAP;
      audit_db_flag &= SET_FLAG_VLINK_PORTMAP;
      if (UNC_OP_UPDATE == op1) {
        void *running_val = NULL;
        bool invalid_attr = false;
        running_val = GetVal(ckv_driver_req);
        invalid_attr = FilterAttributes(running_val,
             GetVal(ckv_audit_dup_db), false, UNC_OP_UPDATE);
        if ((invalid_attr) &&
             (running_db_flag == audit_db_flag)) {
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(ckv_audit_dup_db);
          // Assuming that the diff found only in ConfigStatus
          // Setting the value as OnlyCSDiff in the out parameter ctrlr_affected
          // The value Configdiff should be given more priority than the value
          // onlycs.
          // So If the out parameter ctrlr_affected has already value
          // as configdiff then dont change the value
          if (*ctrlr_affected != uuc::kCtrlrAffectedConfigDiff) {
            UPLL_LOG_INFO("Setting the ctrlr_affected to OnlyCSDiff");
            *ctrlr_affected = uuc::kCtrlrAffectedOnlyCSDiff;
          }
          continue;
        }
      }

      DELETE_IF_NOT_NULL(ckv_audit_dup_db);
      IpcResponse ipc_response;
      memset(&ipc_response, 0, sizeof(IpcResponse));
      if (op1 == UNC_OP_DELETE) {
        vext_datatype = UPLL_DT_AUDIT;
        db_flag = auditdb_flag;
      } else {
        vext_datatype = UPLL_DT_RUNNING;
      }
      // bool ipc_result = true;
      if ((op1 == UNC_OP_UPDATE) &&
          (running_db_flag != audit_db_flag)) {
        ConfigKeyVal *temp_ckv_driver_req = NULL;
        result_code = DupConfigKeyVal(temp_ckv_driver_req, ckv_driver_req,
                                      MAINTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(ckv_running_db);
          DELETE_IF_NOT_NULL(ckv_audit_db);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
        // First delete then recreate
        op1 = UNC_OP_DELETE;
        vext_datatype = UPLL_DT_AUDIT;
        result_code = SendInterfaceAuditRequestToDriver(
                temp_ckv_driver_req, ckv_audit_db, op1, ctrlr_dom,
                vext_datatype, dmi, session_id, config_id,
                audit_db_flag, ipc_response);
        if (UPLL_RC_SUCCESS != result_code) {
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(temp_ckv_driver_req);
          DELETE_IF_NOT_NULL(ckv_audit_db);
          dmi->CloseCursor(cursor, true);
          if (ipc_response.ckv_data) {
            result_code = HandleAuditDriverIpcError(phase,
                ipc_response, dmi, ckv_running_db,
                err_ckv);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("HandleAuditDriverIpcError failed %d",
                              result_code);
              DELETE_IF_NOT_NULL(ipc_response.ckv_data);
              DELETE_IF_NOT_NULL(ckv_running_db);
              return result_code;
            } else {
              DELETE_IF_NOT_NULL(ipc_response.ckv_data);
              return ipc_response.header.result_code;
            }
          }
          DELETE_IF_NOT_NULL(ckv_running_db);
          return result_code;
        }
        DELETE_IF_NOT_NULL(temp_ckv_driver_req);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        // Recreate it now
        op1 = UNC_OP_CREATE;
        vext_datatype = UPLL_DT_RUNNING;
        result_code = SendInterfaceAuditRequestToDriver(
                ckv_driver_req, ckv_audit_db, op1, ctrlr_dom,
                vext_datatype, dmi, session_id, config_id,
                running_db_flag, ipc_response);
      } else {
        result_code = SendInterfaceAuditRequestToDriver(
                ckv_driver_req, ckv_audit_db, op1, ctrlr_dom,
                vext_datatype, dmi, session_id, config_id,
                db_flag, ipc_response);
      }
      if (UPLL_RC_SUCCESS != result_code) {
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        dmi->CloseCursor(cursor, true);
        if (ipc_response.ckv_data) {
          result_code = HandleAuditDriverIpcError(phase,
              ipc_response, dmi, ckv_running_db,
              err_ckv);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("HandleAuditDriverIpcError failed %d",
                            result_code);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running_db);
            return result_code;
          } else {
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            return ipc_response.header.result_code;
          }
        }
        DELETE_IF_NOT_NULL(ckv_running_db);
        return result_code;
      }
      DELETE_IF_NOT_NULL(ipc_response.ckv_data);
      DELETE_IF_NOT_NULL(ckv_driver_req);
      // DELETE_IF_NOT_NULL(ipc_response.ckv_data);
      // *ctrlr_affected = true;
      if (*ctrlr_affected == uuc::kCtrlrAffectedOnlyCSDiff) {
        UPLL_LOG_INFO("Reset ctrlr state from OnlyCSDiff to ConfigDiff");
      }
      UPLL_LOG_DEBUG("Setting the ctrlr_affected to ConfigDiff");
      *ctrlr_affected = uuc::kCtrlrAffectedConfigDiff;
    }
    dmi->CloseCursor(cursor, true);
    DELETE_IF_NOT_NULL(ckv_running_db);
    DELETE_IF_NOT_NULL(ckv_audit_db);
  }
  if (uud::kDalRcSuccess != db_result) {
     UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d", db_result);
     result_code =  DalToUpllResCode(db_result);
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
                ? UPLL_RC_SUCCESS : result_code;
  return result_code;
}


bool VbrIfFlowFilterMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                    BindInfo *&binfo,
                                    int &nattr,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;

  /* Main Table only update */
  if (MAINTBL == tbl) {
    nattr = sizeof(vbr_if_flowfiltermaintbl_bind_info)/
            sizeof(vbr_if_flowfiltermaintbl_bind_info[0]);
    binfo = vbr_if_flowfiltermaintbl_bind_info;
  } else {
    return PFC_FALSE;
  }

  UPLL_LOG_DEBUG("Successful Completion");
  return PFC_TRUE;
}

upll_rc_t VbrIfFlowFilterMoMgr::ValidateCapability(IpcReqRespHeader *req,
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

  if (NULL == ctrlr_name) {
    UPLL_LOG_DEBUG("ctrlr_name is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_TRACE("ctrlr_name : (%s)"
               "operation : (%d)",
               ctrlr_name, req->operation);

  bool result_code = false;
  uint32_t  max_instance_count;
  const uint8_t *attrs = NULL;
  uint32_t max_attrs = 0;

  switch (req->operation) {
    case UNC_OP_CREATE: {
      UPLL_LOG_TRACE("Calling GetCreateCapability Operation %d ",
                     req->operation);

      result_code = GetCreateCapability(ctrlr_name, ikey->get_key_type(),
                                        &max_instance_count,
                                        &max_attrs, &attrs);
      break;
    }
    default: {
      if (req->datatype == UPLL_DT_STATE) {
        UPLL_LOG_TRACE("Calling GetStateCapability Operation %d ",
                       req->operation);
        result_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      } else {
        UPLL_LOG_TRACE("Calling GetReadCapability Operation %d ",
                       req->operation);
        result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      }
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

upll_rc_t VbrIfFlowFilterMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_DEBUG("ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_VBRIF_FLOWFILTER != key->get_key_type()) {
    UPLL_LOG_DEBUG(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (key->get_st_num() != IpctSt::kIpcStKeyVbrIfFlowfilter) {
     UPLL_LOG_DEBUG("Invalid key structure received. received struct num - %d",
                  key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_DEBUG(" Error: option2 is not NONE");
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
  if ((req->option1 != UNC_OPT1_NORMAL)
              &&(req->option1 != UNC_OPT1_DETAIL)) {
    UPLL_LOG_DEBUG(" Error: option1 is not NORMAL");
    return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if ((req->option1 != UNC_OPT1_NORMAL)
      &&(req->operation == UNC_OP_READ_SIBLING_COUNT)) {
    UPLL_LOG_DEBUG(" Error: option1 is not NORMAL for ReadSiblingCount");
    return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if ((req->option1 == UNC_OPT1_DETAIL) &&
      (req->datatype != UPLL_DT_STATE)) {
      UPLL_LOG_DEBUG(" Invalid Datatype(%d)", req->datatype);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  if ((req->datatype == UPLL_DT_IMPORT) && (req->operation == UNC_OP_READ ||
       req->operation == UNC_OP_READ_SIBLING ||
       req->operation == UNC_OP_READ_SIBLING_BEGIN ||
       req->operation == UNC_OP_READ_NEXT ||
       req->operation == UNC_OP_READ_BULK ||
       req->operation == UNC_OP_READ_SIBLING_COUNT)) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }

  /** Read key structure */
  key_vbr_if_flowfilter_t *key_vbr_if_flowfilter =
      reinterpret_cast<key_vbr_if_flowfilter_t *>(key->get_key());

  /** Validate key structure */
  if (NULL == key_vbr_if_flowfilter) {
    UPLL_LOG_DEBUG("KT_VBRIF_FLOWFILTER Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

    rt_code = ValidateVbrIfFlowfilterKey(key_vbr_if_flowfilter,
                                         req->operation);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_DEBUG(" key_vbrif_flowfilter syntax validation failed :"
                     "Err Code - %d",
                     rt_code);
      return rt_code;
    }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::ValidateVbrIfFlowfilterKey(
    key_vbr_if_flowfilter_t* key_vbr_if_flowfilter,
    unc_keytype_operation_t op) {

  UPLL_FUNC_TRACE;

  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  VbrIfMoMgr *mgrvbrif =
      reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
                  UNC_KT_VBR_IF)));

  if (NULL == mgrvbrif) {
    UPLL_LOG_DEBUG("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }

  rt_code = mgrvbrif->ValidateVbrifKey(&(key_vbr_if_flowfilter->if_key));

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_DEBUG(" Vbrif_key syntax validation failed :Err Code - %d",
                   rt_code);

    return rt_code;
  }


  if ((op != UNC_OP_READ_SIBLING_COUNT) &&
      (op != UNC_OP_READ_SIBLING_BEGIN)) {
    /** validate direction */
    if (!ValidateNumericRange(key_vbr_if_flowfilter->direction,
                              (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                              (uint8_t) UPLL_FLOWFILTER_DIR_OUT, true, true)) {
      UPLL_LOG_DEBUG("direction syntax validation failed ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
    // input direction should be not set for
    // sibling begin or count operation
    // as 0 or 1 are valid values setting an invalid value;
    key_vbr_if_flowfilter->direction = 0xFE;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                  DalDmlIntf *dmi,
                                                  IpcReqRespHeader *req) {
  // No operation
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::SetRenameFlag(ConfigKeyVal *ikey,
                                              DalDmlIntf *dmi,
                                              IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *pkey = NULL;
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetParentConfigKey failed %d", result_code);
    return result_code;
  }
  MoMgrImpl *mgr =
    reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
            UNC_KT_VBR_IF)));
  if (!mgr) {
    UPLL_LOG_DEBUG("mgr is NULL");
    DELETE_IF_NOT_NULL(pkey);
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t rename = 0;
  result_code = mgr->IsRenamed(pkey, req->datatype, dmi, rename);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("IsRenamed failed %d", result_code);
    DELETE_IF_NOT_NULL(pkey);
    return result_code;
  }
  UPLL_LOG_DEBUG("Flag from parent : %d", rename);
  SET_USER_DATA_FLAGS(ikey, rename);
  DELETE_IF_NOT_NULL(pkey);
  return UPLL_RC_SUCCESS;
}


upll_rc_t VbrIfFlowFilterMoMgr::IsReferenced(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
  // Check the object existence
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi,
                               &dbop, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::AllocVal(ConfigVal *&ck_val,
                                         upll_keytype_datatype_t dt_type,
                                         MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  void *val = NULL;
  if (ck_val != NULL)
    return UPLL_RC_ERR_GENERIC;
  switch (tbl) {
    case MAINTBL:
        val = reinterpret_cast<void *>
            (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
        ck_val = new ConfigVal(IpctSt::kIpcStValFlowfilter, val);

      break;
    default:
      val = NULL;
      break;
  }

  UPLL_LOG_DEBUG(" AllocVal Successfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                  ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_if_flowfilter_t *vbr_if_ff_key;
  void *pkey = NULL;
  bool cfgval_ctrlr = false;

  if (parent_key == NULL) {
    vbr_if_ff_key = reinterpret_cast<key_vbr_if_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vbr_if_ff_key->direction = 0xFE;
    okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            vbr_if_ff_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) return UPLL_RC_ERR_GENERIC;

  if (okey) {
    if (okey->get_key_type() != UNC_KT_VBRIF_FLOWFILTER)
      return UPLL_RC_ERR_GENERIC;
  }
  if ((okey) && (okey->get_key())) {
    vbr_if_ff_key = reinterpret_cast<key_vbr_if_flowfilter_t *>
        (okey->get_key());
  } else {
    vbr_if_ff_key = reinterpret_cast<key_vbr_if_flowfilter_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_t)));
    // If no direction is specified , 0xFE is filled to bind output direction
    vbr_if_ff_key->direction = 0xFE;
  }

  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_vtn_t *>
          (pkey)->vtn_name,
          kMaxLenVtnName + 1);
      break;
    case UNC_KT_VBRIDGE:
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_vbr_t *>
          (pkey)->vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vbridge_name,
          reinterpret_cast<key_vbr_t *>
          (pkey)->vbridge_name,
          kMaxLenVnodeName + 1);
      break;
    case UNC_KT_VBR_IF:
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_vbr_if_t *>
          (pkey)->vbr_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vbridge_name,
          reinterpret_cast<key_vbr_if_t *>
          (pkey)->vbr_key.vbridge_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vbr_if_ff_key->if_key.if_name,
                        reinterpret_cast<key_vbr_if_t *>
                        (pkey)->if_name,
                        kMaxLenInterfaceName + 1);
      break;
    case UNC_KT_VBRIF_FLOWFILTER:
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vtn_key.vtn_name,
          reinterpret_cast<key_vbr_if_flowfilter_t *>
          (pkey)->if_key.vbr_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vbr_if_ff_key->if_key.vbr_key.vbridge_name,
          reinterpret_cast<key_vbr_if_flowfilter_t *>
          (pkey)->if_key.vbr_key.vbridge_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vbr_if_ff_key->if_key.if_name,
                        reinterpret_cast<key_vbr_if_flowfilter_t *>
                        (pkey)->if_key.if_name,
                        kMaxLenInterfaceName + 1);
      vbr_if_ff_key->direction =
          reinterpret_cast<key_vbr_if_flowfilter_t *>
          (pkey)->direction;
      break;
    /* VlanmapOnBoundary: Added vlink case */
    case UNC_KT_VLINK:
      {
        uint8_t *vnode_name;
        uint8_t *vnode_if_name;
        uint8_t flags = 0;
        val_vlink *vlink_val =
            reinterpret_cast<val_vlink *>(GetVal(parent_key));
        if (!vlink_val) {
          UPLL_LOG_TRACE("ERROR");
          if (!okey || !(okey->get_key()))
            FREE_IF_NOT_NULL(vbr_if_ff_key);
          return UPLL_RC_ERR_GENERIC;
        }

        GET_USER_DATA_FLAGS(parent_key->get_cfg_val(), flags);
        flags &=  VLINK_FLAG_NODE_POS;
        UPLL_LOG_DEBUG("Vlink flag node position %d", flags);

        if (flags == kVlinkVnode2) {
          cfgval_ctrlr = true;
          vnode_name = vlink_val->vnode2_name;
          vnode_if_name = vlink_val->vnode2_ifname;
        } else {
          vnode_name = vlink_val->vnode1_name;
          vnode_if_name = vlink_val->vnode1_ifname;
        }
        uuu::upll_strncpy(vbr_if_ff_key->if_key.vbr_key.vtn_key.vtn_name,
            reinterpret_cast<key_vlink *>(pkey)->vtn_key.vtn_name,
            (kMaxLenVtnName + 1));
        if (vnode_name)
          uuu::upll_strncpy(vbr_if_ff_key->if_key.vbr_key.vbridge_name,
                            vnode_name, (kMaxLenVnodeName + 1));
        if (vnode_if_name)
          uuu::upll_strncpy(vbr_if_ff_key->if_key.if_name, vnode_if_name,
              (kMaxLenInterfaceName + 1));

        break;
    }
    default:
      if (vbr_if_ff_key) free(vbr_if_ff_key);
      return UPLL_RC_ERR_GENERIC;
  }


  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyVbrIfFlowfilter, vbr_if_ff_key);
  }

  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVbrIfFlowfilter,
                            vbr_if_ff_key, NULL);
  }

  /* In case of node2 of vlink, set the user_data
   * from vlink's cfg_val */
  if (cfgval_ctrlr) {
    SET_USER_DATA(okey, parent_key->get_cfg_val());
  } else {
    SET_USER_DATA(okey, parent_key);
  }

  UPLL_LOG_DEBUG("okey filled Succesfully %d", result_code);
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                                ConfigKeyVal *&req,
                                                MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) {
    UPLL_LOG_DEBUG("Request is null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey != NULL) {
    UPLL_LOG_DEBUG("oKey already Contains Data");
    return UPLL_RC_ERR_GENERIC;
  }

  if (req->get_key_type() != UNC_KT_VBRIF_FLOWFILTER) {
    UPLL_LOG_DEBUG(" DupConfigKeyval Failed.");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    if (tbl == MAINTBL) {
      val_flowfilter_t *ival = NULL;
      ival = reinterpret_cast<val_flowfilter_t *> (GetVal(req));
      if (NULL != ival) {
      val_flowfilter_t *vbr_if_flowfilter_val = NULL;
        vbr_if_flowfilter_val = reinterpret_cast<val_flowfilter_t *>
            (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
        memcpy(vbr_if_flowfilter_val, ival, sizeof(val_flowfilter_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vbr_if_flowfilter_val);
      tmp1->set_user_data(tmp->get_user_data());
      }
    }
  }

  void *tkey = req->get_key();
  if (tkey != NULL) {
    key_vbr_if_flowfilter_t *ikey = NULL;
    ikey = reinterpret_cast<key_vbr_if_flowfilter_t *> (tkey);
    key_vbr_if_flowfilter_t *vbr_if_flowfilter =
        reinterpret_cast<key_vbr_if_flowfilter_t*>
          (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_t)));

    memcpy(vbr_if_flowfilter, ikey, sizeof(key_vbr_if_flowfilter_t));
    okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                          IpctSt::kIpcStKeyVbrIfFlowfilter, vbr_if_flowfilter,
                          tmp1);
    SET_USER_DATA(okey, req);

    UPLL_LOG_DEBUG("DupConfigkeyVal Succesfull.");
    return UPLL_RC_SUCCESS;
  }
  // free(vbr_if_flowfilter_val);
  // free(tmp1);
  DELETE_IF_NOT_NULL(tmp1);
  return UPLL_RC_ERR_GENERIC;
}

upll_rc_t VbrIfFlowFilterMoMgr::UpdateMo(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Implementation Not supported for this KT.");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VbrIfFlowFilterMoMgr::RenameMo(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                         const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_DEBUG("Implementation Not supported for this KT.");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VbrIfFlowFilterMoMgr::MergeValidate(unc_key_type_t keytype,
                                             const char *ctrlr_id,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetRenamedUncKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *unc_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrifff start",
                  ikey->ToStrAll().c_str());
  if ((NULL == ikey) || (ctrlr_id == NULL) || (NULL == dmi)) {
    UPLL_LOG_DEBUG("ikey/ctrlr_id dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t rename = 0;
  MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>
             (const_cast<MoManager *> (GetMoManager(UNC_KT_VBRIDGE)));
  if (mgrvbr == NULL) {
    UPLL_LOG_DEBUG("mgrvbr NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  val_rename_vnode *rename_val = reinterpret_cast
          <val_rename_vnode*>(ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
  if (!rename_val) {
    UPLL_LOG_DEBUG("rename_val NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbr_if_flowfilter_t *ctrlr_key =
      reinterpret_cast<key_vbr_if_flowfilter_t *> (ikey->get_key());
  if (!ctrlr_key) {
    UPLL_LOG_DEBUG("ctrlr_key NULL");
    FREE_IF_NOT_NULL(rename_val);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(rename_val->ctrlr_vtn_name,
                    ctrlr_key->if_key.vbr_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  rename_val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;

  uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                    ctrlr_key->if_key.vbr_key.vbridge_name,
                    (kMaxLenVnodeName + 1));
  rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

  result_code = mgrvbr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to get the configkey for Vribdge");
    FREE_IF_NOT_NULL(rename_val);
    mgrvbr = NULL;
    return result_code;
  }
  if (!unc_key) {
    UPLL_LOG_DEBUG("unc_key NULL");
    FREE_IF_NOT_NULL(rename_val);
    mgrvbr = NULL;
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
  result_code = mgrvbr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ,
                                     dbop, dmi, RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    mgrvbr = NULL;
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    key_vbr_if_flowfilter_t *vbr_if_flowfilter_key =
        reinterpret_cast<key_vbr_if_flowfilter_t *> (unc_key->get_key());
    if (strcmp(
            reinterpret_cast<char *>
            (ctrlr_key->if_key.vbr_key.vtn_key.vtn_name),
            reinterpret_cast<const char *>(vbr_if_flowfilter_key->
                                           if_key.vbr_key.vtn_key.vtn_name))) {
      uuu::upll_strncpy(
                     ctrlr_key->if_key.vbr_key.vtn_key.vtn_name,
                     vbr_if_flowfilter_key->if_key.vbr_key.vtn_key.vtn_name,
                     (kMaxLenVtnName + 1));
      rename |= VTN_RENAME_FLAG;
    }
    if (strcmp(
            reinterpret_cast<char *>(ctrlr_key->if_key.vbr_key.vbridge_name),
            reinterpret_cast<const char *>(vbr_if_flowfilter_key->
                                           if_key.vbr_key.vbridge_name))) {
      uuu::upll_strncpy(ctrlr_key->if_key.vbr_key.vbridge_name,
                      vbr_if_flowfilter_key->if_key.vbr_key.vbridge_name,
                      (kMaxLenVnodeName + 1));
      rename |= VBR_RENAME_FLAG;
    }
    SET_USER_DATA(ikey, unc_key);
    SET_USER_DATA_FLAGS(ikey, rename);
  }

  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrifff end",
                  ikey->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  mgrvbr = NULL;
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *okey = NULL;
  UPLL_LOG_TRACE("Start Input ConfigKeyval = %s", ikey->ToStrAll().c_str());

  MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>
    (const_cast<MoManager *> (GetMoManager(UNC_KT_VBRIDGE)));

  if (mgrvbr == NULL) {
    UPLL_LOG_DEBUG("obj null");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgrvbr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey fail");
    return result_code;
  }

  if (ctrlr_dom) {
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  } else {
    UPLL_LOG_DEBUG("ctrlr null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  uuu::upll_strncpy(
      reinterpret_cast<key_vbr *> (okey->get_key())->vtn_key.vtn_name,
      reinterpret_cast<key_vbr_if_flowfilter_t *>
      (ikey->get_key())->if_key.vbr_key.vtn_key.vtn_name,
      (kMaxLenVtnName + 1));

  uuu::upll_strncpy(
      reinterpret_cast<key_vbr *> (okey->get_key())->vbridge_name,
      reinterpret_cast<key_vbr_if_flowfilter_t *>
      (ikey->get_key())->if_key.vbr_key.vbridge_name,
      (kMaxLenVnodeName + 1));

  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag };
  result_code = mgrvbr->ReadConfigDB(okey, dt_type, UNC_OP_READ,
      dbop, dmi, RENAMETBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB no instance");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("Unable to Read from DB");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  val_rename_vnode *rename_val = NULL;
  rename_val = reinterpret_cast<val_rename_vnode *> (GetVal(okey));

  if (!rename_val) {
    UPLL_LOG_DEBUG("Vbr Name is not Valid.");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(reinterpret_cast<key_vbr_if_flowfilter_t *>
      (ikey->get_key())->if_key.vbr_key.vtn_key.vtn_name,
      rename_val->ctrlr_vtn_name,
      (kMaxLenVtnName + 1));

  uuu::upll_strncpy(reinterpret_cast<key_vbr_if_flowfilter_t *>
      (ikey->get_key())->if_key.vbr_key.vbridge_name,
      rename_val->ctrlr_vnode_name,
      (kMaxLenVnodeName + 1));

  DELETE_IF_NOT_NULL(okey);
  UPLL_LOG_TRACE("End Input ConfigKeyVal= %s", ikey->ToStrAll().c_str());
  UPLL_LOG_DEBUG("Renamed Controller key is sucessfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::UpdateAuditConfigStatus(
    unc_keytype_configstatus_t cs_status,
    uuc::UpdateCtrlrPhase phase,
    ConfigKeyVal *&ckv_running,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  val_flowfilter_t *val = NULL;
  val = (ckv_running != NULL)?
     reinterpret_cast<val_flowfilter_t *> (GetVal(ckv_running)):NULL;

  if (NULL == val) {
    UPLL_LOG_DEBUG("val strct is empty");
    return UPLL_RC_ERR_GENERIC;
  }
  if (uuc::kUpllUcpCreate == phase )
    val->cs_row_status = cs_status;
  if ((uuc::kUpllUcpUpdate == phase) &&
           (val->cs_row_status == UNC_CS_INVALID ||
            val->cs_row_status == UNC_CS_NOT_APPLIED))
    val->cs_row_status = cs_status;

  UPLL_LOG_DEBUG("AuditUpdate Config Status Information %d", result_code);
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::ReadMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  uint8_t db_flag = 0;
  ConfigKeyVal *l_key = NULL, *dup_key = NULL;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|
                   kOpInOutDomain |kOpInOutFlag };
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("MoMgrImpl::ReadMo - ValidateMessage %d", result_code);
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  switch (req->datatype) {
    // Retrieving config information
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_STATE:
      if ((req->option1 == UNC_OPT1_NORMAL)&&
          (req->option2 == UNC_OPT2_NONE)) {
        result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
          return result_code;
        }
        // Retrieving state information
      } else if ((req->datatype == UPLL_DT_STATE) &&
                 (req->option1 == UNC_OPT1_DETAIL) &&
                 (req->option2 == UNC_OPT2_NONE)) {
        result_code =  DupConfigKeyVal(dup_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal fail in ReadMo for dup_key");
          return result_code;
        }

        result_code = ReadConfigDB(dup_key, req->datatype,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadConfigDB  fail in ReadMo for dup_key");
          delete dup_key;
          return result_code;
        }
        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(dup_key);
          UPLL_LOG_DEBUG("DupConfigKeyVal fail in ReadMo for l_key");
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
        // Added ValidateCapa
        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
          return result_code;
        }

        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          return result_code;
        }
        // 2.send request to driver
        // ///////////////////////


        GET_USER_DATA_FLAGS(dup_key, db_flag);
        pfcdrv_val_vbrif_vextif *pfc_val =
            reinterpret_cast<pfcdrv_val_vbrif_vextif *>
            (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif)));

        // For DT_STATE additional information will be returned which is not
        // required. So, passing UPLL_DT_RUNNING
        result_code = GetVexternalInformation(dup_key, UPLL_DT_RUNNING,
                                              pfc_val, db_flag,
                      reinterpret_cast<const char *>(ctrlr_dom.ctrlr), dmi);
        if (UPLL_RC_SUCCESS != result_code) {
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          return result_code;
        }

        l_key->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifVextif,
                                       pfc_val));
        // ///////////////////////////
        IpcResponse ipc_resp;
        memset(&ipc_resp, 0, sizeof(IpcResponse));
        IpcRequest ipc_req;
        memset(&ipc_req, 0, sizeof(IpcRequest));
        ipc_req.header.clnt_sess_id = req->clnt_sess_id;
        ipc_req.header.config_id = req->config_id;
        ipc_req.header.operation = req->operation;
        ipc_req.header.option1 = req->option1;
        ipc_req.header.datatype = req->datatype;
        ipc_req.ckv_data = l_key;
        if (!IpcUtil::SendReqToDriver(
                (const char *)ctrlr_dom.ctrlr,
                reinterpret_cast<char *>(ctrlr_dom.domain),
                PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL,
                &ipc_req, true, &ipc_resp)) {
          UPLL_LOG_DEBUG("SendReqToDriver failed for Key %d controller %s",
                         l_key->get_key_type(),
                         reinterpret_cast<char *>(ctrlr_dom.ctrlr));
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          return ipc_resp.header.result_code;
        }
        if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                        l_key->get_key_type(), ctrlr_dom.ctrlr,
                        ipc_resp.header.result_code);
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          return ipc_resp.header.result_code;
        }
        ConfigKeyVal *okey = NULL;
        result_code = ConstructReadDetailResponse(dup_key,
                                                  ipc_resp.ckv_data,
                                                  ctrlr_dom,
                                                  &okey, dmi);
        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
        DELETE_IF_NOT_NULL(dup_key);
        DELETE_IF_NOT_NULL(l_key);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ConstructReadDetailResponse error code (%d)",
                         result_code);
          return result_code;
        } else {
          if (okey != NULL) {
            ikey->ResetWith(okey);
            DELETE_IF_NOT_NULL(okey);
          }
        }
      }
      break;
    default:
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

bool VbrIfFlowFilterMoMgr::IsValidKey(void *key,
                                      uint64_t index, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  key_vbr_if_flowfilter_t *vbr_ff_key =
      reinterpret_cast<key_vbr_if_flowfilter_t *>(key);
  if (vbr_ff_key == NULL) {
    return false;
  }

  switch (index) {
    case uudst::vbr_if_flowfilter::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vbr_ff_key->if_key.vbr_key.vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbr_if_flowfilter::kDbiVbrName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vbr_ff_key->if_key.vbr_key.vbridge_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VBR Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbr_if_flowfilter::kDbiVbrIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vbr_ff_key->if_key.if_name),
                            kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VBR interface name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vbr_if_flowfilter::kDbiInputDirection:
      if (vbr_ff_key->direction == 0xFE) {
        // if operation is read sibling begin or
        // read sibling count return false
        // for output binding
        vbr_ff_key->direction = 0;
        return false;
      } else {
        if (!ValidateNumericRange(vbr_ff_key->direction,
                                  (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                                  (uint8_t) UPLL_FLOWFILTER_DIR_OUT,
                                  true, true)) {
          UPLL_LOG_DEBUG(" input direction syntax validation failed ");
          return false;
        }
      }
      break;
    default:
      UPLL_LOG_DEBUG("Invalid Key Index");
      return false;
  }
  return true;
}


upll_rc_t VbrIfFlowFilterMoMgr::UpdateConfigStatus(ConfigKeyVal *key,
                                                   unc_keytype_operation_t op,
                                                   uint32_t driver_result,
                                                   ConfigKeyVal *upd_key,
                                                   DalDmlIntf *dmi,
                                                   ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_flowfilter_t *vbr_if_flowfilter_val = NULL;
  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  vbr_if_flowfilter_val = reinterpret_cast<val_flowfilter_t *> (GetVal(key));
  if (vbr_if_flowfilter_val == NULL) return UPLL_RC_ERR_GENERIC;

  if (op == UNC_OP_CREATE) {
    if (vbr_if_flowfilter_val->cs_row_status != UNC_CS_NOT_SUPPORTED)
      vbr_if_flowfilter_val->cs_row_status = cs_status;
  } else {
    UPLL_LOG_DEBUG("Operation Not Supported.");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("Update Config Status Successfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }

  key_rename_vnode_info *key_rename = NULL;
  key_rename = reinterpret_cast<key_rename_vnode_info *> (ikey->get_key());
  key_vbr_if_flowfilter_t * key_vbr_if =
      reinterpret_cast<key_vbr_if_flowfilter_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_flowfilter_t)));

  if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vtn_name))) {
    UPLL_LOG_DEBUG("old_unc_vtn_name NULL");
    if (key_vbr_if) free(key_vbr_if);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(key_vbr_if->if_key.vbr_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name,
                    (kMaxLenVtnName + 1));

  if (UNC_KT_VBRIDGE == ikey->get_key_type()) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      UPLL_LOG_DEBUG("old_unc_vnode_name NULL");
      free(key_vbr_if);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbr_if->if_key.vbr_key.vbridge_name,
                      key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      UPLL_LOG_DEBUG("new_unc_vnode_name NULL");
      free(key_vbr_if);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vbr_if->if_key.vbr_key.vbridge_name,
                      key_rename->new_unc_vnode_name, (kMaxLenVnodeName + 1));
  }
  key_vbr_if->direction = 0xFE;
  okey = new ConfigKeyVal(UNC_KT_VBRIF_FLOWFILTER,
                           IpctSt::kIpcStKeyVbrIfFlowfilter,
                           key_vbr_if, NULL);
  if (!okey) {
    free(key_vbr_if);
    return UPLL_RC_ERR_GENERIC;
  }

  return result_code;
}
upll_rc_t VbrIfFlowFilterMoMgr::ConstructReadDetailResponse(
    ConfigKeyVal *ikey,
    ConfigKeyVal *drv_resp_ckv,
    controller_domain ctrlr_dom,
    ConfigKeyVal **okey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *tmp_okey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *drv_resp_val = NULL;
  drv_resp_val =  drv_resp_ckv->get_cfg_val();
  result_code =  GetChildConfigKey(tmp_okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed err code (%d)", result_code);
    return result_code;
  }

  val_flowfilter_t *tmp_val_ff = reinterpret_cast<val_flowfilter_t *>
      (GetVal(ikey));
  if (!tmp_val_ff) {
    UPLL_LOG_DEBUG(" Invalid value read from DB");
    DELETE_IF_NOT_NULL(tmp_okey);
    return UPLL_RC_ERR_GENERIC;
  }
  val_flowfilter_t *val_ff = reinterpret_cast<val_flowfilter_t *>
      (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
  memcpy(val_ff, tmp_val_ff, sizeof(val_flowfilter_t));
  tmp_okey->AppendCfgVal(IpctSt::kIpcStValFlowfilter, val_ff);
  while (drv_resp_val != NULL) {
    val_flowfilter_entry_t *val_entry = NULL;
    if (IpctSt::kIpcStValFlowfilterEntry ==
        drv_resp_val->get_st_num()) {
      UPLL_LOG_TRACE("Get the val struct");
      val_entry = reinterpret_cast< val_flowfilter_entry_t *>
          (drv_resp_val->get_val());
       // SetRedirectNodeAndPortinRead will Set the
       // redirect-direction/vbr_nmae/vbr_if_name
       result_code =  SetRedirectNodeAndPortForRead(ikey ,
                                                   ctrlr_dom,
                                                   val_entry ,
                                                   dmi);
      if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("SetRedirectNodeAndPortForRead Fails - %d",
            result_code);
         DELETE_IF_NOT_NULL(tmp_okey);
         return result_code;
       }
    }
    drv_resp_val = drv_resp_val->get_next_cfg_val();
  }
  tmp_okey->AppendCfgVal(drv_resp_ckv->GetCfgValAndUnlink());
  if (*okey == NULL) {
    *okey = tmp_okey;
  } else {
    (*okey)->AppendCfgKeyVal(tmp_okey);
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
                                              ConfigKeyVal *ikey,
                                              bool begin, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t db_flag = 0;
  controller_domain ctrlr_dom;
  ConfigKeyVal *l_key = NULL, *tmp_key = NULL;
  ConfigKeyVal *okey = NULL, *tctrl_key = NULL;

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("validate Message Failed %d", result_code);
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  switch (req->datatype) {
    // Retrieving config information
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_STATE:
      if (req->option1 == UNC_OPT1_NORMAL) {
        result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" Read request failed result(%d)", result_code);
          return result_code;
        }
        // Retrieving state information
      } else if ((req->datatype == UPLL_DT_STATE) &&
                 (req->option1 == UNC_OPT1_DETAIL) &&
                 (req->option2 == UNC_OPT2_NONE)) {
        result_code =  DupConfigKeyVal(tctrl_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed for l_key%d ", result_code);
          return result_code;
        }

        result_code = ReadInfoFromDB(req, tctrl_key, dmi, &ctrlr_dom);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDb failed for tctrl_key%d ", result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }

        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("DupConfigKeyVal fail in ReadSiblingMo for l_key");
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(tctrl_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("validate Capability Failed %d", result_code);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }

        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(tctrl_key);
          UPLL_LOG_DEBUG("GetRenamedControllerKey Failed %d", result_code);
          return result_code;
        }

        // 2.send request to driver
        // ///////////////////////

        GET_USER_DATA_FLAGS(tctrl_key, db_flag);
        pfcdrv_val_vbrif_vextif *pfc_val =
            reinterpret_cast<pfcdrv_val_vbrif_vextif *>
            (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif)));

        // For DT_STATE additional information will be returned which is not
        // required. So, passing UPLL_DT_RUNNING
        result_code = GetVexternalInformation(tctrl_key, UPLL_DT_RUNNING,
                                              pfc_val, db_flag,
                      reinterpret_cast<const char *>(ctrlr_dom.ctrlr), dmi);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("GetVexternalInformation failed err(%d)", result_code);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }

        l_key->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifVextif,
                                       pfc_val));
        // ///////////////////////////
        IpcResponse ipc_resp;
        memset(&ipc_resp, 0, sizeof(IpcResponse));
        IpcRequest ipc_req;
        memset(&ipc_req, 0, sizeof(IpcRequest));
        ipc_req.header.clnt_sess_id = req->clnt_sess_id;
        ipc_req.header.config_id = req->config_id;
        ipc_req.header.operation = UNC_OP_READ;
        ipc_req.header.option1 = req->option1;
        ipc_req.header.datatype = req->datatype;

        tmp_key = tctrl_key;
        while (tmp_key != NULL) {
          reinterpret_cast<key_vbr_if_flowfilter_t*>
              (l_key->get_key())->direction =
              reinterpret_cast<key_vbr_if_flowfilter_t*>
              (tmp_key->get_key())->direction;
          ipc_req.ckv_data = l_key;
          if (!IpcUtil::SendReqToDriver(
                      (const char *)ctrlr_dom.ctrlr,
                      reinterpret_cast<char *>(ctrlr_dom.domain),
                      PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL,
                      &ipc_req, true, &ipc_resp)) {
            UPLL_LOG_DEBUG("SendReqToDriver failed for Key %d controller %s",
                           l_key->get_key_type(),
                           reinterpret_cast<char *>(ctrlr_dom.ctrlr));
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            return ipc_resp.header.result_code;
          }

          if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Driver response for Key %d controller %s result %d",
                          l_key->get_key_type(), ctrlr_dom.ctrlr,
                          ipc_resp.header.result_code);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            return ipc_resp.header.result_code;
          }

          result_code = ConstructReadDetailResponse(tmp_key,
                                                    ipc_resp.ckv_data,
                                                    ctrlr_dom,
                                                    &okey, dmi);

          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ConstructReadDetailResponse error code (%d)",
                           result_code);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(tctrl_key);
            return result_code;
          }
          tmp_key = tmp_key->get_next_cfg_key_val();
        }
        if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
          ikey->ResetWith(okey);
          DELETE_IF_NOT_NULL(okey);
        }
        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
        DELETE_IF_NOT_NULL(l_key);
        DELETE_IF_NOT_NULL(tctrl_key);
      }
      break;
    default:
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::TxUpdateController(
    unc_key_type_t keytype,
    uint32_t session_id,
    uint32_t config_id,
    uuc::UpdateCtrlrPhase phase,
    set<string> *affected_ctrlr_set,
    DalDmlIntf *dmi,
    ConfigKeyVal **err_ckv,
    TxUpdateUtil *tx_util,
    TcConfigMode config_mode,
    std::string vtn_name)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  uint8_t db_flag = 0;
  uint8_t db_flag_running = 0;
  upll_keytype_datatype_t vext_datatype =  UPLL_DT_CANDIDATE;

  // TxUpdate skipped if config mode is VIRTUAL
  if (config_mode == TC_CONFIG_VIRTUAL) {
    return UPLL_RC_SUCCESS;
  }

  if (affected_ctrlr_set == NULL)
    return UPLL_RC_ERR_GENERIC;
  //  Create phase is skipped and is performed during update phase.
  //  This is done for the following reason,
  //  Vbridge Interface configuration is commited to pfc, then portmap
  //  and flowfilter are created and commited.
  //  The flowfilter should be created inside the vexternal whose information
  //  will be sent to pfc during update of vbr_if but create request
  //  for flowfilter is sent in create phase due to which commit fails.
  if (phase == uuc::kUpllUcpCreate) {
    return result_code;
  }
  unc_keytype_operation_t op_arr[2] = {UNC_OP_INVALID, UNC_OP_INVALID};
  int nop = 0;
  // Create and Update are done is same phase.
  if (phase == uuc::kUpllUcpUpdate) {
    op_arr[0] = UNC_OP_UPDATE;
    op_arr[1] = UNC_OP_CREATE;
    nop = 2;
  } else if (phase == uuc::kUpllUcpDelete) {
    op_arr[0] = UNC_OP_DELETE;
    nop = 1;
  } else {
    UPLL_LOG_INFO("Invalid phase");
    return result_code;
  }
  for (int i = 0; i < nop; i++) {
    /* retreives the delta of running and audit configuration */
    dal_cursor_handle = NULL;
    unc_keytype_operation_t op1 = op_arr[i];
    if (tx_util->GetErrCount() > 0) {
      UPLL_LOG_ERROR("Error Occured From Driver Side..Exiting the loop");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
        op1, req, nreq, &dal_cursor_handle, dmi, config_mode,
        vtn_name, MAINTBL);

    while (result_code == UPLL_RC_SUCCESS) {
      if (tx_util->GetErrCount() > 0) {
        UPLL_LOG_ERROR("TxUpdateUtil says exit the loop.");
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(req);
        return UPLL_RC_ERR_GENERIC;
      }

      ck_main = NULL;
      db_result = dmi->GetNextRecord(dal_cursor_handle);
      result_code = DalToUpllResCode(db_result);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" GetNextRecord failed err code(%d)", result_code);
        break;
      }
      switch (op1)   {
        case UNC_OP_CREATE:
        case UNC_OP_UPDATE:
        case UNC_OP_DELETE:
          op1 = op_arr[i];
          result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            dmi->CloseCursor(dal_cursor_handle, true);
            return result_code;
          }
          break;
        default:
          UPLL_LOG_DEBUG("TxUpdateController Invalid operation");
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          dmi->CloseCursor(dal_cursor_handle, true);
          return UPLL_RC_ERR_GENERIC;
      }

      GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
      if (ctrlr_dom.ctrlr == NULL) {
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        dmi->CloseCursor(dal_cursor_handle, true);
        return UPLL_RC_ERR_GENERIC;
      }
      db_flag = 0;
      GET_USER_DATA_FLAGS(ck_main, db_flag);
      if (!(SET_FLAG_PORTMAP & db_flag) && !(SET_FLAG_VLINK & db_flag)) {
        if (op1 != UNC_OP_UPDATE) {
          DELETE_IF_NOT_NULL(ck_main);
          continue;
        } else {
          GET_USER_DATA_FLAGS(nreq, db_flag_running);
          if (!(SET_FLAG_PORTMAP & db_flag_running) &&
              !(SET_FLAG_VLINK & db_flag_running)) {
            UPLL_LOG_DEBUG("portmap flag is not available at running as well ");
            DELETE_IF_NOT_NULL(ck_main);
            continue;
          }
          op1 = UNC_OP_DELETE;
          vext_datatype = UPLL_DT_RUNNING;
          db_flag = db_flag_running;
        }
      } else if (op1 == UNC_OP_UPDATE) {
        GET_USER_DATA_FLAGS(nreq, db_flag_running);
        if (!(SET_FLAG_PORTMAP & db_flag_running) &&
            !(SET_FLAG_VLINK & db_flag_running)) {
          UPLL_LOG_DEBUG("Portmap flag is not set at running");
          op1 = UNC_OP_CREATE;
          vext_datatype = UPLL_DT_CANDIDATE;
        }
      }

      if (UNC_OP_DELETE == op1) {
        vext_datatype = UPLL_DT_RUNNING;
      }

      ConfigKeyVal *ckv_driver = NULL;
      result_code = DupConfigKeyVal(ckv_driver, ck_main, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(ck_main);
        dmi->CloseCursor(dal_cursor_handle, true);
        return result_code;
      }
      ConfigKeyVal *ckv_unc = NULL;
      result_code = DupConfigKeyVal(ckv_unc, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(ckv_driver);
        DELETE_IF_NOT_NULL(ck_main);
        dmi->CloseCursor(dal_cursor_handle, true);
        return result_code;
      }

      if (UNC_OP_UPDATE != op1) {
        result_code = SendInterfaceRequestToDriver(
            ckv_driver, nreq, op1, ctrlr_dom, vext_datatype, dmi,
            session_id, config_id, db_flag, affected_ctrlr_set,
            ckv_unc, tx_util);
      } else {
      //  For vbr_if_flowfilter update request comes only
      //  when there is a change in db flags.
      //  So Delete request is sent for running DB record and
      //  Create record is sent for Candidate DB record.
        unc_keytype_operation_t temp_op = UNC_OP_DELETE;
        GET_USER_DATA_FLAGS(nreq, db_flag);
        result_code = SendInterfaceRequestToDriver(
            ckv_driver, nreq, temp_op, ctrlr_dom, UPLL_DT_RUNNING, dmi,
            session_id, config_id, db_flag, affected_ctrlr_set,
            ckv_unc, tx_util);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("SendInterfaceRequestToDriver failed %d", result_code);
        } else {
          temp_op = UNC_OP_CREATE;
          ConfigKeyVal *ckv_driver2 = NULL;
          result_code = DupConfigKeyVal(ckv_driver2, ck_main, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
          } else {
            ConfigKeyVal *ckv_unc2 = NULL;
            result_code = DupConfigKeyVal(ckv_unc2, req, MAINTBL);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
              DELETE_IF_NOT_NULL(ckv_driver2);
            }
            GET_USER_DATA_FLAGS(req, db_flag);
            result_code = SendInterfaceRequestToDriver(
                ckv_driver2, nreq, temp_op, ctrlr_dom, UPLL_DT_CANDIDATE, dmi,
                session_id, config_id, db_flag, affected_ctrlr_set,
                ckv_unc2, tx_util);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("SendInterfaceRequestToDrive failed %d",
                                                          result_code);
              DELETE_IF_NOT_NULL(ckv_driver2);
              DELETE_IF_NOT_NULL(ckv_unc2);
            }
          }
        }
      }
      DELETE_IF_NOT_NULL(ck_main);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("IpcSend failed %d", result_code);
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(ckv_driver);
        DELETE_IF_NOT_NULL(ckv_unc);
        return result_code;
      }
    }
    dmi->CloseCursor(dal_cursor_handle, true);
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(nreq);
  }
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
    UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::SetVlinkPortmapConfiguration(
    ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type,
    DalDmlIntf *dmi, InterfacePortMapInfo flags,
    unc_keytype_operation_t oper,
    TcConfigMode config_mode,
    string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == ikey->get_key()) {
    return result_code;
  }
  if (!flags) {
    //  return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *ckv = NULL;
  result_code = GetChildConfigKey(ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  key_vbr_if_flowfilter_t *ff_key = reinterpret_cast<key_vbr_if_flowfilter_t *>
      (ckv->get_key());
  key_vbr_if_t *vbrif_key = reinterpret_cast<key_vbr_if_t *>(ikey->get_key());

  uuu::upll_strncpy(ff_key->if_key.vbr_key.vtn_key.vtn_name,
                    vbrif_key->vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);

  uuu::upll_strncpy(ff_key->if_key.vbr_key.vbridge_name,
                    vbrif_key->vbr_key.vbridge_name,
                    kMaxLenVtnName + 1);

  uuu::upll_strncpy(ff_key->if_key.if_name,
                    vbrif_key->if_name,
                    kMaxLenInterfaceName + 1);
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  // The below statement allows to read the partial key with
  // direction as output only
  ff_key->direction = 0xFE;
  result_code = UpdateConfigDB(ckv, dt_type, UNC_OP_READ,
                                    dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No flowfilter object");
    DELETE_IF_NOT_NULL(ckv);
    return UPLL_RC_SUCCESS;
  } else if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
  }
  ff_key->direction = 0xFE;
  result_code = ReadConfigDB(ckv, dt_type ,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_DEBUG("No Recrods in the Vbr_If_FlowFilter Table");
    DELETE_IF_NOT_NULL(ckv);
    return UPLL_RC_SUCCESS;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Read ConfigDB failure %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }
  uint8_t flag_port_map = 0;
  ConfigKeyVal *temp_ckv = ckv;
  while (temp_ckv) {
    flag_port_map = 0;
    GET_USER_DATA_FLAGS(temp_ckv, flag_port_map);
    if (flags & kVlinkConfigured) {
      if (flag_port_map & SET_FLAG_VLINK) {
        UPLL_LOG_DEBUG("vlink flag is already set in DB");
        DELETE_IF_NOT_NULL(ckv);
        return UPLL_RC_SUCCESS;
      }
      UPLL_LOG_DEBUG("only vlink");
      flag_port_map |= SET_FLAG_VLINK;
    } else if (flags & kPortMapConfigured) {
      if (flag_port_map & SET_FLAG_PORTMAP) {
        UPLL_LOG_DEBUG("Port-Map flag is already set in DB");
        DELETE_IF_NOT_NULL(ckv);
        return UPLL_RC_SUCCESS;
      }
      UPLL_LOG_DEBUG("only portmap");
      flag_port_map |= SET_FLAG_PORTMAP;
    } else if (flags & kVlinkPortMapConfigured) {
      if (flag_port_map & SET_FLAG_VLINK_PORTMAP) {
        UPLL_LOG_DEBUG("Port-Map/vlink flag is already set in DB");
        DELETE_IF_NOT_NULL(ckv);
        return UPLL_RC_SUCCESS;
      }
      UPLL_LOG_DEBUG("portmap with vlink");
      flag_port_map |= SET_FLAG_VLINK_PORTMAP;
    } else {
      UPLL_LOG_DEBUG("No portmap/vlink");
      flag_port_map &= SET_FLAG_NO_VLINK_PORTMAP;
    }
    SET_USER_DATA_FLAGS(temp_ckv, flag_port_map);

    DbSubOp dbop_up = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
    result_code = UpdateConfigDB(temp_ckv, dt_type, UNC_OP_UPDATE,
                                 dmi, &dbop_up, config_mode,
                                 vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("failed to update the portmap configuration");
      DELETE_IF_NOT_NULL(ckv);
      return result_code;
    }

    VbrIfFlowFilterEntryMoMgr *mgr =
        reinterpret_cast<VbrIfFlowFilterEntryMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIF_FLOWFILTER_ENTRY)));
    if (mgr == NULL) {
      DELETE_IF_NOT_NULL(ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->SetVlinkPortmapConfiguration(temp_ckv, dt_type,
                                                    dmi, flags, oper,
                                                    config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("update portmap flag for flowfilterentry failed, err %d",
                     result_code);
      DELETE_IF_NOT_NULL(ckv);
      return result_code;
    }
    temp_ckv = temp_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetVexternalInformation(ConfigKeyVal* ck_main,
                 upll_keytype_datatype_t dt_type,
                 pfcdrv_val_vbrif_vextif *& pfc_val,
                 uint8_t db_flag, const char *ctrlr_id, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *ckv = NULL;
  InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;
  if (db_flag & SET_FLAG_PORTMAP)  {
    UPLL_LOG_DEBUG("portMap is configured!!!");
    VbrIfMoMgr *mgrvbrif = reinterpret_cast<VbrIfMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
    if (mgrvbrif == NULL) {
      UPLL_LOG_DEBUG("Unable to get the instance of VBRIF");
      return result_code;
    }
    result_code = mgrvbrif->GetChildConfigKey(ckv, NULL);
    key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t *>
        (ckv->get_key());
    key_vbr_if_flowfilter_t * temp_key =
        reinterpret_cast<key_vbr_if_flowfilter_t*>(ck_main->get_key());

    uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                      temp_key->if_key.vbr_key.vtn_key.vtn_name,
                      kMaxLenVtnName + 1);

    uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                      temp_key->if_key.vbr_key.vbridge_name,
                      kMaxLenVtnName + 1);

    uuu::upll_strncpy(vbr_if_key->if_name,
                      temp_key->if_key.if_name,
                      kMaxLenInterfaceName + 1);

    uint8_t* vexternal = reinterpret_cast<uint8_t*>
        (ConfigKeyVal::Malloc(kMaxLenVnodeName + 1));
    uint8_t* vex_if = reinterpret_cast<uint8_t*>
        (ConfigKeyVal::Malloc(kMaxLenInterfaceName + 1));

    result_code = mgrvbrif->GetVexternal(ckv, dt_type, dmi,
                                         vexternal, vex_if,
                                         flags, ctrlr_id);
    if (UPLL_RC_SUCCESS != result_code) {
      FREE_IF_NOT_NULL(vexternal);
      FREE_IF_NOT_NULL(vex_if);
      DELETE_IF_NOT_NULL(ckv);
      UPLL_LOG_DEBUG("Failed to get VExternal details, err:%d", result_code);
      return result_code;
    }

    // pfc_val->valid[PFCDRV_IDX_VAL_VBRIF_VEXTIF_FFE] = UNC_VF_VALID;
    unc_keytype_ctrtype_t ctrlrtype = UNC_CT_UNKNOWN;
    uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
    if (!ctrlr_mgr->GetCtrlrType(ctrlr_id, dt_type, &ctrlrtype)) {
      UPLL_LOG_ERROR("Controller name %s not found in datatype %d",
                     ctrlr_id, dt_type);
      FREE_IF_NOT_NULL(vexternal);
      FREE_IF_NOT_NULL(vex_if);
      DELETE_IF_NOT_NULL(ckv);
      return UPLL_RC_ERR_GENERIC;
    }
    if (ctrlrtype == UNC_CT_ODC) {
      pfc_val->valid[PFCDRV_IDX_INTERFACE_TYPE] = UNC_VF_VALID;
      pfc_val->interface_type = PFCDRV_IF_TYPE_VBRIF;
      pfc_val->valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] = UNC_VF_INVALID;
      pfc_val->valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] = UNC_VF_INVALID;
    } else {
      pfc_val->valid[PFCDRV_IDX_INTERFACE_TYPE] = UNC_VF_VALID;
      pfc_val->interface_type = PFCDRV_IF_TYPE_VEXTIF;
      pfc_val->valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] = UNC_VF_VALID;
      uuu::upll_strncpy(pfc_val->vexternal_name, vexternal,
                        kMaxLenVnodeName + 1);
      pfc_val->valid[PFCDRV_IDX_VEXT_IF_NAME_VBRIF] = UNC_VF_VALID;
      uuu::upll_strncpy(pfc_val->vext_if_name, vex_if,
                        kMaxLenInterfaceName + 1);
    }
    // pfc_val->val_vbrif_vextif.valid[PFCDRV_IDX_INTERFACE_TYPE] =
    // UNC_VF_VALID;
    // pfc_val->val_vbrif_vextif.interface_type = PFCDRV_IF_TYPE_VEXTIF;
    free(vexternal);
    free(vex_if);
    DELETE_IF_NOT_NULL(ckv);
  } else if (db_flag & SET_FLAG_VLINK) {
    UPLL_LOG_DEBUG("vlink is configured!");
    pfc_val->valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF] = UNC_VF_INVALID;
    pfc_val->valid[PFCDRV_IDX_INTERFACE_TYPE] = UNC_VF_VALID;

    pfc_val->interface_type = PFCDRV_IF_TYPE_VBRIF;
  } else {
     UPLL_LOG_DEBUG("Portmap/Vlink is not configured");
     return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }
  UPLL_LOG_DEBUG("GetVExternalInformation returned successfully");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetControllerDomainID(ConfigKeyVal *ikey,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  VbrIfMoMgr *mgrvbrif =
      reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager *>(GetMoManager(
                  UNC_KT_VBR_IF)));

  ConfigKeyVal *ckv = NULL;
  result_code = mgrvbrif->GetChildConfigKey(ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Unable to get the ParentConfigKey, resultcode=%d",
                    result_code);
    return result_code;
  }

  key_vbr_if_flowfilter_t *temp_key =
      reinterpret_cast<key_vbr_if_flowfilter_t*>(ikey->get_key());

  key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t*>(ckv->get_key());

  uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                    temp_key->if_key.vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                    temp_key->if_key.vbr_key.vbridge_name,
                    kMaxLenVnodeName + 1);

  uuu::upll_strncpy(vbr_if_key->if_name,
                    temp_key->if_key.if_name,
                    kMaxLenInterfaceName + 1);


  ConfigKeyVal *vbr_key = NULL;
  result_code = mgrvbrif->GetParentConfigKey(vbr_key, ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetParentConfigKey Failed, err %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }

  result_code = mgrvbrif->GetControllerDomainId(vbr_key, dt_type,
                                                &ctrlr_dom, dmi);
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    DELETE_IF_NOT_NULL(ckv);
    DELETE_IF_NOT_NULL(vbr_key);
    UPLL_LOG_DEBUG("GetControllerDomainId error err code(%d)", result_code);
    return result_code;
  }

  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                   ctrlr_dom.ctrlr, ctrlr_dom.domain);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  DELETE_IF_NOT_NULL(ckv);
  DELETE_IF_NOT_NULL(vbr_key);
  return result_code;
}

upll_rc_t VbrIfFlowFilterMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                   ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_DEBUG(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VBRIF_FLOWFILTER) {
    UPLL_LOG_DEBUG(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_if_flowfilter_t *pkey =
      reinterpret_cast<key_vbr_if_flowfilter_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_DEBUG(" Input vbr if flow filter key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vbr_if_t)));

  uuu::upll_strncpy(vbr_if_key->vbr_key.vtn_key.vtn_name,
                    reinterpret_cast<key_vbr_if_flowfilter_t *>
                    (pkey)->if_key.vbr_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vbr_if_key->vbr_key.vbridge_name,
                    reinterpret_cast<key_vbr_if_flowfilter_t *>
                    (pkey)->if_key.vbr_key.vbridge_name,
                    kMaxLenVnodeName + 1);
  uuu::upll_strncpy(vbr_if_key->if_name,
                    reinterpret_cast<key_vbr_if_flowfilter_t *>
                    (pkey)->if_key.if_name,
                    kMaxLenInterfaceName + 1);
  okey = new ConfigKeyVal(UNC_KT_VBR_IF,
                          IpctSt::kIpcStKeyVbrIf,
                          vbr_if_key, NULL);
  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::DeleteChildrenPOM(
          ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
          DalDmlIntf *dmi, TcConfigMode config_mode,
          string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if (NULL == ikey || NULL == dmi) {
  UPLL_LOG_DEBUG("Delete Operation failed:Bad request");
  return result_code;
  }
  // Read the DB get the flowlist value and send the delete request to
  // flowlist momgr if flowlist is configured.

  ConfigKeyVal *tempckv = NULL;
  result_code = GetChildConfigKey(tempckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  result_code = UpdateConfigDB(tempckv, dt_type, UNC_OP_DELETE, dmi,
      config_mode, vtn_name, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    DELETE_IF_NOT_NULL(tempckv);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("UPLL_RC_ERR_NO_SUCH_INSTANCE");
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    return result_code;
  }
  delete tempckv;
  tempckv = NULL;
  return  UPLL_RC_SUCCESS;
}

upll_rc_t VbrIfFlowFilterMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_flowfilter_t *val = reinterpret_cast<val_flowfilter_t *>
    (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
  val->cs_row_status = UNC_CS_APPLIED;
  ikey->AppendCfgVal(IpctSt::kIpcStValFlowfilter, val);
  return UPLL_RC_SUCCESS;
}

bool VbrIfFlowFilterMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return true;
  return false;
}

// This function sends vbr_if_flowfilter commit request  to driver
upll_rc_t VbrIfFlowFilterMoMgr::SendInterfaceRequestToDriver(
    ConfigKeyVal *ckv_driver, ConfigKeyVal *ckv_audit,
    unc_keytype_operation_t op, controller_domain_t ctrlr_dom,
    upll_keytype_datatype_t vext_datatype, DalDmlIntf *dmi,
    uint32_t session_id, uint32_t config_id,
    uint8_t db_flag, set<string> *affected_ctrlr_set,
    ConfigKeyVal *ckv_unc, TxUpdateUtil *tx_util) {
  UPLL_FUNC_TRACE;
  pfcdrv_val_vbrif_vextif *pfc_val =
    reinterpret_cast<pfcdrv_val_vbrif_vextif *>
    (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif)));

//  If operation is DELETE then datatype is RUNNING
  if (UNC_OP_DELETE == op) {
    vext_datatype = UPLL_DT_RUNNING;
  }

// Get VExternal information from vbr_if
  upll_rc_t result_code = GetVexternalInformation(ckv_driver, vext_datatype,
      pfc_val, db_flag, reinterpret_cast<const char*> (ctrlr_dom.ctrlr), dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    FREE_IF_NOT_NULL(pfc_val);
    return result_code;
  }
  UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
      ctrlr_dom.domain);
  upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
    UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;
// Rename the ckv to controller  name
  result_code = GetRenamedControllerKey(ckv_driver, dt_type,
      dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey failed %d", result_code);
    FREE_IF_NOT_NULL(pfc_val);
    return result_code;
  }

  ckv_driver->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifVextif,
        pfc_val));
  UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
      ctrlr_dom.domain);
  // Inserting the controller to Set
  affected_ctrlr_set->insert
    (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));
  result_code = tx_util->EnqueueRequest(session_id, config_id,
                                        UPLL_DT_CANDIDATE, op, dmi,
                                        ckv_driver, ckv_unc, string());
  return result_code;
}

// Handle Audit Driver request
upll_rc_t VbrIfFlowFilterMoMgr::HandleAuditDriverIpcError(
    uuc::UpdateCtrlrPhase phase,
    IpcResponse ipc_response, DalDmlIntf *dmi,
    ConfigKeyVal *ckv_running_db, ConfigKeyVal **err_ckv) {
  UPLL_LOG_DEBUG("driver return failure err_code is %d",
                 ipc_response.header.result_code);
// Set error ckv to ckv_running
  *err_ckv = ckv_running_db;
  if (phase == uuc::kUpllUcpDelete) {
    UPLL_LOG_DEBUG("Phase is Delete");
    return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *resp = NULL;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  result_code = GetChildConfigKey(resp, ipc_response.ckv_data);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("DupConfigKeyVal failed for ipc response ckv err_code %d",
        result_code);
    *err_ckv = NULL;
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCs };
  result_code = ReadConfigDB(resp, UPLL_DT_RUNNING, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB Failed");
    DELETE_IF_NOT_NULL(resp);
    *err_ckv = NULL;
    return result_code;
  }
// Compute the ConfigStatus
  result_code = UpdateAuditConfigStatus(UNC_CS_INVALID, phase, resp, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Update Audit config status failed %d",
        result_code);
    DELETE_IF_NOT_NULL(resp);
    *err_ckv = NULL;
    return result_code;
  }
// Update the DB with the configstatus
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutCs };
  string temp_vtn_name = "";
  result_code = UpdateConfigDB(resp, UPLL_DT_RUNNING, UNC_OP_UPDATE,
      dmi, &dbop1, TC_CONFIG_GLOBAL, temp_vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateConfigDB failed for ipc response ckv err_code %d",
        result_code);
    DELETE_IF_NOT_NULL(resp);
    *err_ckv = NULL;
    return result_code;
  }
  DELETE_IF_NOT_NULL(resp);
  return UPLL_RC_SUCCESS;
}

// Send Audit request to Driver
upll_rc_t VbrIfFlowFilterMoMgr::SendInterfaceAuditRequestToDriver(
    ConfigKeyVal *ckv_running_db, ConfigKeyVal *ckv_audit_db,
    unc_keytype_operation_t op, controller_domain_t ctrlr_dom,
    upll_keytype_datatype_t vext_datatype,  DalDmlIntf *dmi,
    uint32_t session_id, uint32_t config_id,
    uint8_t db_flag, IpcResponse &ipc_response) {
  UPLL_FUNC_TRACE;
  pfcdrv_val_vbrif_vextif *pfc_val =
      reinterpret_cast<pfcdrv_val_vbrif_vextif *>
      (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbrif_vextif)));
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
// Get VExternal information from vbr_if
  result_code = GetVexternalInformation(ckv_running_db, vext_datatype,
      pfc_val, db_flag, reinterpret_cast<const char*>(ctrlr_dom.ctrlr), dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetVexternalInformation failed %d", result_code);
    free(pfc_val);
    return result_code;
  }
  upll_keytype_datatype_t dt_type =
      (op == UNC_OP_DELETE)?UPLL_DT_AUDIT:UPLL_DT_RUNNING;
  result_code = GetRenamedControllerKey(ckv_running_db, dt_type,
                                        dmi, &ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetRenamedControllerKey failed %d", result_code);
    free(pfc_val);
    return result_code;
  }

  ckv_running_db->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrifVextif,
                            pfc_val));
  UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);


  memset(&ipc_response, 0, sizeof(IpcResponse));
  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(IpcRequest));
  ipc_req.header.clnt_sess_id = session_id;
  ipc_req.header.config_id = config_id;
  ipc_req.header.operation = op;
  ipc_req.header.datatype = UPLL_DT_CANDIDATE;
  ipc_req.ckv_data = ckv_running_db;
  if (!IpcUtil::SendReqToDriver((const char *)ctrlr_dom.ctrlr,
                                reinterpret_cast<char *>(ctrlr_dom.domain),
                                PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL,
                                &ipc_req, true, &ipc_response)) {
    UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                  ckv_running_db->get_key_type(),
                  reinterpret_cast<char *>(ctrlr_dom.ctrlr));
  }
  return ipc_response.header.result_code;
}
upll_rc_t VbrIfFlowFilterMoMgr::TxUpdateErrorHandler(ConfigKeyVal *ckv_unc,
    ConfigKeyVal *ckv_driver,
    DalDmlIntf *dmi,
    upll_keytype_datatype_t dt_type,
    ConfigKeyVal **err_ckv,
    IpcResponse *ipc_resp) {
  UPLL_FUNC_TRACE;
  *err_ckv = ckv_unc;
  DELETE_IF_NOT_NULL(ckv_driver);
  DELETE_IF_NOT_NULL(ipc_resp->ckv_data);
  return UPLL_RC_SUCCESS;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
