/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "momgr_impl.hh"
#include "vterm_if_momgr.hh"
#include "vterm_if_flowfilter_momgr.hh"
#include "vterm_if_flowfilter_entry_momgr.hh"
#include "uncxx/upll_log.hh"


namespace unc {
namespace upll {
namespace kt_momgr {

using unc::upll::ipc_util::IpcUtil;
#define VTN_RENAME_FLAG   0x01
#define VTERM_RENAME_FLAG   0x02
#define SET_FLAG_PORTMAP  0x20
#define SET_FLAG_NO_VLINK_PORTMAP 0x9F

// VtermIfFlowFilter Table(Main Table)
BindInfo VtermIfFlowFilterMoMgr::vterm_if_flowfilter_bind_info[] = {
  { uudst::vterm_if_flowfilter::kDbiVtnName, CFG_KEY,
    offsetof(key_vterm_if_flowfilter_t, if_key.vterm_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vterm_if_flowfilter::kDbiVtermName, CFG_KEY,
    offsetof(key_vterm_if_flowfilter_t, if_key.vterm_key.vterminal_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vterm_if_flowfilter::kDbiVtermIfName, CFG_KEY,
    offsetof(key_vterm_if_flowfilter_t, if_key.if_name),
    uud::kDalChar, (kMaxLenInterfaceName + 1) },
  { uudst::vterm_if_flowfilter::kDbiInputDirection, CFG_KEY,
    offsetof(key_vterm_if_flowfilter_t, direction),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter::kDbiCtrlrName, CK_VAL,
    offsetof(key_user_data_t, ctrlr_id),
    uud::kDalChar, (kMaxLenCtrlrId + 1) },
  { uudst::vterm_if_flowfilter::kDbiDomainId, CK_VAL,
    offsetof(key_user_data, domain_id),
    uud::kDalChar, (kMaxLenDomainId + 1) },
  { uudst::vterm_if_flowfilter::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 },
  { uudst::vterm_if_flowfilter::kDbiCsRowStatus, CS_VAL,
    offsetof(val_flowfilter_t, cs_row_status),
    uud::kDalUint8, 1 },
};

BindInfo VtermIfFlowFilterMoMgr::vterm_if_flowfiltermaintbl_bind_info[] = {
  { uudst::vterm_if_flowfilter::kDbiVtnName, CFG_MATCH_KEY,
    offsetof(key_vterm_if_flowfilter_t, if_key.vterm_key.vtn_key.vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vterm_if_flowfilter::kDbiVtermName, CFG_MATCH_KEY,
    offsetof(key_vterm_if_flowfilter_t, if_key.vterm_key.vterminal_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vterm_if_flowfilter::kDbiVtnName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, (kMaxLenVtnName + 1) },
  { uudst::vterm_if_flowfilter::kDbiVtermName, CFG_INPUT_KEY,
    offsetof(key_rename_vnode_info_t, new_unc_vnode_name),
    uud::kDalChar, (kMaxLenVnodeName + 1) },
  { uudst::vterm_if_flowfilter::kDbiFlags, CK_VAL,
    offsetof(key_user_data_t, flags),
    uud::kDalUint8, 1 }
};

unc_key_type_t VtermIfFlowFilterMoMgr::vterm_if_flowfilter_child[] = {
  UNC_KT_VTERMIF_FLOWFILTER_ENTRY };

VtermIfFlowFilterMoMgr::VtermIfFlowFilterMoMgr() : MoMgrImpl() {
  UPLL_FUNC_TRACE;
  // Rename and ctrlr tables not required for this KT
  // setting  table indexed for ctrl and rename table to NULL;
  ntable = MAX_MOMGR_TBLS;
  table = new Table *[ntable]();
  // For Main Table
  table[MAINTBL] = new Table(uudst::kDbiVtermIfFlowFilterTbl,
      UNC_KT_VTERMIF_FLOWFILTER, vterm_if_flowfilter_bind_info,
      IpctSt::kIpcStKeyVtermIfFlowfilter, IpctSt::kIpcStValFlowfilter,
      uudst::vterm_if_flowfilter::kDbiVtermIfFlowFilterNumCols);
  table[RENAMETBL] = NULL;
  table[CTRLRTBL] = NULL;
  table[CONVERTTBL] = NULL;

  nchild =
      sizeof(vterm_if_flowfilter_child) / sizeof(vterm_if_flowfilter_child[0]);
  child = vterm_if_flowfilter_child;
}

/* Method to create an Entry in vTerminal Interface Flowfilter in candidate or
 * import DB
 * Req contains the datatype, operation and options
 * ikey contains the key and value structure*/
upll_rc_t VtermIfFlowFilterMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  if (ikey == NULL || req == NULL || dmi == NULL) {
    UPLL_LOG_ERROR("Invalid input parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE("ikey= %s", ikey->ToStrAll().c_str());

  /* Validate Message is invoked to validate the keytype, operations and options
   * The vtn, vterm, vterm_if and direction are validated
   * to their allowed range*/
  if (req->datatype != UPLL_DT_IMPORT) {
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("ValidateMessage failed, Error - %d", result_code);
      return result_code;
    }
  }

  /* If the incoming record already exists in running DB then the record and
   * its children KTs are copied from running to candidate if present*/
  /* If create sequence is called during three scenarios
   * 1) If the incoming record is not present in running
   * 3) If datatype is Import*/
    // create a record in CANDIDATE DB
  VtermIfMoMgr *mgrvtermif = reinterpret_cast<VtermIfMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));

  InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;
  /* GetPortmapInfo is invoked to check whether portmap is configured
   * in vterm_if. If portmap is set flag is set to kPortMapConfigured*/
  ConfigKeyVal *pkey = NULL;
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetParentConfigKey failed %d", result_code);
    return result_code;
  }
  result_code = mgrvtermif->GetPortmapInfo(pkey, req->datatype, dmi, flags);
  DELETE_IF_NOT_NULL(pkey);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
    UPLL_LOG_ERROR("GetPortmapinfo Failed err_code %d", result_code);
    return result_code;
  }

  uint8_t flag_port_map = 0, iflag = 0;
  if (flags & kPortMapConfigured) {
    flag_port_map = flag_port_map | SET_FLAG_PORTMAP;
  }
  GET_USER_DATA_FLAGS(ikey, iflag);
  iflag |= flag_port_map;
  SET_USER_DATA_FLAGS(ikey, iflag);

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
    /* Controller and Domain ids are retrieved from parent KT ie vterm_if.
     * If the function returns UPLL_RC_ERR_NO_SUCH_INSTANCE then vterm_if doesnt
     * exists so UPLL_RC_ERR_PARENT_DOES_NOT_EXIST is returned*/
    result_code = GetControllerDomainID(ikey, req->datatype, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        result_code = UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
      }
      UPLL_LOG_ERROR("Failed to Get the Controller Domain details, err:%d",
                     result_code);
      return result_code;
    }
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

    UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                   ctrlr_dom.ctrlr, ctrlr_dom.domain);
/* The incoming record is checked whether it can be created in the specified
 * controller and whether its attributes are supported by the controller*/
    result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("validate Capability Failed %d", result_code);
      return result_code;
    }
    TcConfigMode config_mode;
    std::string vtn_name = "";
    result_code = GetConfigModeInfo(req, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetConfigMode failed");
      return result_code;
    }

    DbSubOp dbop1 = {kOpNotRead, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain |
                                               kOpInOutFlag};
/* The record is created in the specified datatype*/
    result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                                 dmi, &dbop1, config_mode,
                                 vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Failed to update DB, err %d", result_code);
      return result_code;
    }
    return result_code;
}

/* Method is invoked to create an entry in audit DB when a controller
 * is audited*/
upll_rc_t VtermIfFlowFilterMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

/* Check whether portmap is configured in vterm_if. If configured set flag
 * in DB to SET_FLAG_PORTMAP*/
  InterfacePortMapInfo flags = kVlinkPortMapNotConfigured;
  VtermIfMoMgr *mgrvtermif = reinterpret_cast<VtermIfMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));
  ConfigKeyVal *pkey = NULL;
  string vtn_name = "";
  result_code = GetParentConfigKey(pkey, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetParentConfigKey failed %d", result_code);
    return result_code;
  }
  result_code = mgrvtermif->GetPortmapInfo(pkey, UPLL_DT_AUDIT, dmi, flags);
  DELETE_IF_NOT_NULL(pkey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetPortmapinfo Failed err_code %d", result_code);
    return result_code;
  }

  uint8_t flag_port_map = 0;
  GET_USER_DATA_FLAGS(ikey, flag_port_map);
  if (flags & kPortMapConfigured) {
    flag_port_map |= SET_FLAG_PORTMAP;
  }
  SET_USER_DATA_FLAGS(ikey, flag_port_map);

/* Get controller and domain id from  vterm_if table*/
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  result_code = GetControllerDomainID(ikey, UPLL_DT_AUDIT, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Failed to Get the Controller Domain details, err:%d",
                   result_code);
    return result_code;
  }

  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                 ctrlr_dom.ctrlr, ctrlr_dom.domain);

/* Create record in vTerm_if_flowfilter tbl in audit DB*/
  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT,
                               UNC_OP_CREATE, dmi, TC_CONFIG_GLOBAL,
                               vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("UpdateConfigDB Failed err_code %d", result_code);
  }
  return result_code;
}

/* This method is inovked to sync audited controller with the unc running DB.
 * Created, Deleted and Updated records are fetched from doing diff on
 * running and audit DB and appropriate requests are sent to driver*/
upll_rc_t VtermIfFlowFilterMoMgr::AuditUpdateController(unc_key_type_t keytype,
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
  uint8_t db_flag = 0;
  string vtn_name = "";
  uint8_t *ctrlr = reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id));

/* Create request is skipped as portmap will be sent to driver only
 * during update phase.*/
  if (phase1 == uuc::kUpllUcpCreate) {
    return result_code;
  }
/* Create and update operations are done in update phase.
 * */
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
/* If kUpllUcpUpdate Update and Create operations are done
 * If kUpllUcpDelete Delete operation is done*/
  for (int i = 0; i < nop; i++) {
    /* retreives the delta of running and audit configuration */
    cursor = NULL;
    unc_keytype_operation_t op1 = op[i];
    uuc::UpdateCtrlrPhase phase = (op[i] == UNC_OP_UPDATE)?uuc::kUpllUcpUpdate:
      ((op[i] == UNC_OP_CREATE)?uuc::kUpllUcpCreate:
       ((op[i] == UNC_OP_DELETE)?uuc::kUpllUcpDelete:uuc::kUpllUcpInvalid));
    UPLL_LOG_DEBUG("Operation is %d", op[i]);
/* The method gives the created, deleted or updated records between two DBs
 * depending on the operation passed*/
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
      UPLL_LOG_ERROR("DiffConfigDB failed - %d", result_code);
      DELETE_IF_NOT_NULL(ckv_running_db);
      DELETE_IF_NOT_NULL(ckv_audit_db);
      if (cursor)
        dmi->CloseCursor(cursor, true);
      return result_code;
    }
    if (cursor == NULL) {
      UPLL_LOG_ERROR("cursor is null");
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
/* Skipping the controller ID if the controller id in DB and
 * controller id available for Audit are not the same*/
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
            UPLL_LOG_ERROR("DupConfigKeyVal failed. err_code & phase %d %d",
                           result_code, phase);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
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
            UPLL_LOG_ERROR("DupConfigKeyVal failed for running record. "
                           "err_code & phase %d %d", result_code, phase);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          result_code = DupConfigKeyVal(ckv_audit_dup_db,
                                        ckv_audit_db, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("DupConfigKeyVal failed for audit record. "
                           "err_code & phase %d %d", result_code, phase);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ckv_audit_dup_db);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          break;
        default:
          UPLL_LOG_ERROR("Invalid operation %d", phase);
          DELETE_IF_NOT_NULL(ckv_running_db);
          DELETE_IF_NOT_NULL(ckv_audit_db);
          dmi->CloseCursor(cursor, true);
          return UPLL_RC_ERR_NO_SUCH_OPERATION;
          break;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ckv_driver_req, ctrlr_dom);
      if ((NULL == ctrlr_dom.ctrlr) || (NULL == ctrlr_dom.domain)) {
        UPLL_LOG_ERROR("controller id or domain is NULL");
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_audit_dup_db);
        DELETE_IF_NOT_NULL(ckv_running_db);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        dmi->CloseCursor(cursor, true);
        return UPLL_RC_ERR_GENERIC;
      }
      UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                     ctrlr_dom.domain);
      db_flag = 0;
      GET_USER_DATA_FLAGS(ckv_driver_req, db_flag);
/* If portmap/vlink flag is not set at running and the operation is
 * update then portmap/vlink is deleted in the update phase from UNC
 * hence flowfilter seq no also should get deleted from controller
 * hence sending the delete request to the controller driver*/
      if (SET_FLAG_PORTMAP & db_flag) {
        // Continue the operations
      } else {
        if (UNC_OP_UPDATE == op1) {
          op1 = UNC_OP_DELETE;
        } else {
          // NO PortMap/Vlink Configured,
          // Configuration is  not sento to driver.
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(ckv_audit_dup_db);
          continue;
        }
      }

      if (UNC_OP_UPDATE == op1) {
        void *running_val = NULL;
        bool invalid_attr = false;
        running_val = GetVal(ckv_driver_req);
        invalid_attr = FilterAttributes(running_val,
             GetVal(ckv_audit_dup_db), false, UNC_OP_UPDATE);
        if (invalid_attr) {
          DELETE_IF_NOT_NULL(ckv_driver_req);
          DELETE_IF_NOT_NULL(ckv_audit_dup_db);
/* Assuming that the diff found only in ConfigStatus
 * Setting the   value as OnlyCSDiff in the out parameter ctrlr_affected
 * The value Configdiff should be given more priority than the value
 * onlycs . So  If the out parameter ctrlr_affected has already value as
 * configdiff then dont change the value*/
          if (*ctrlr_affected != uuc::kCtrlrAffectedConfigDiff) {
            UPLL_LOG_INFO("Setting the ctrlr_affected to OnlyCSDiff");
            *ctrlr_affected = uuc::kCtrlrAffectedOnlyCSDiff;
          }
          continue;
        }
      }

      DELETE_IF_NOT_NULL(ckv_audit_dup_db);
      upll_keytype_datatype_t dt_type = (op1 == UNC_OP_DELETE)?
               UPLL_DT_AUDIT:UPLL_DT_RUNNING;
      result_code = GetRenamedControllerKey(ckv_driver_req, dt_type,
                                            dmi, &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(ckv_driver_req);
        UPLL_LOG_ERROR("GetRenamedControllerKey failed %d", result_code);
        dmi->CloseCursor(cursor, true);
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_running_db);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        return result_code;
      }
      UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                   ctrlr_dom.domain);


      IpcResponse ipc_response;
      memset(&ipc_response, 0, sizeof(IpcResponse));
      IpcRequest ipc_req;
      memset(&ipc_req, 0, sizeof(IpcRequest));
      ipc_req.header.clnt_sess_id = session_id;
      ipc_req.header.config_id = config_id;
      ipc_req.header.operation = op1;
      ipc_req.header.datatype = UPLL_DT_CANDIDATE;
      ipc_req.ckv_data = ckv_driver_req;
      if (!IpcUtil::SendReqToDriver(
              (const char *)ctrlr_dom.ctrlr, reinterpret_cast<char *>
              (ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
              PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_response)) {
        UPLL_LOG_ERROR("Request to driver for Key %d for controller %s failed ",
                      ckv_driver_req->get_key_type(),
                      reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        DELETE_IF_NOT_NULL(ckv_driver_req);
        DELETE_IF_NOT_NULL(ckv_running_db);
        DELETE_IF_NOT_NULL(ckv_audit_db);
        dmi->CloseCursor(cursor, true);
        return ipc_response.header.result_code;
      }
      if (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("driver return failure err_code is %d",
                       ipc_response.header.result_code);
/* If driver sends failure response set the err_ckv to the running DB
 * record to be sent to upper layer*/
        *err_ckv = ckv_running_db;
        if (phase != uuc::kUpllUcpDelete) {
          ConfigKeyVal *resp = NULL;
          result_code = GetChildConfigKey(resp, ipc_response.ckv_data);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR(
                "DupConfigKeyVal failed for ipc response ckv err_code %d",
                result_code);
            delete ipc_response.ckv_data;
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            *err_ckv = NULL;
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCs };
          result_code = ReadConfigDB(resp, dt_type, UNC_OP_READ,
                                     dbop, dmi, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("ReadConfigDB Failed");
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(resp);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            *err_ckv = NULL;
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
/* The config status is set to UNC_CS_INVALID for failed record in running DB*/
          result_code = UpdateAuditConfigStatus(UNC_CS_INVALID, phase,
                                                resp, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("Update Audit config status failed %d",
                     result_code);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(resp);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            *err_ckv = NULL;
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutCs };
          result_code = UpdateConfigDB(resp, dt_type, UNC_OP_UPDATE,
                                       dmi, &dbop1, TC_CONFIG_GLOBAL,
                                       vtn_name, MAINTBL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR(
                "UpdateConfigDB failed for ipc response ckv err_code %d",
                result_code);
            DELETE_IF_NOT_NULL(ckv_driver_req);
            DELETE_IF_NOT_NULL(resp);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running_db);
            DELETE_IF_NOT_NULL(ckv_audit_db);
            *err_ckv = NULL;
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          DELETE_IF_NOT_NULL(resp);
        }
        return ipc_response.header.result_code;
      }
      DELETE_IF_NOT_NULL(ckv_driver_req);
      DELETE_IF_NOT_NULL(ipc_response.ckv_data);
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

bool VtermIfFlowFilterMoMgr::GetRenameKeyBindInfo(unc_key_type_t key_type,
                                    BindInfo *&binfo,
                                    int &nattr,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;

  /* Main Table only update */
  if (MAINTBL == tbl) {
    nattr = sizeof(vterm_if_flowfiltermaintbl_bind_info)/
            sizeof(vterm_if_flowfiltermaintbl_bind_info[0]);
    binfo = vterm_if_flowfiltermaintbl_bind_info;
  } else {
    return PFC_FALSE;
  }

  UPLL_LOG_DEBUG("Successful Completion");
  return PFC_TRUE;
}

/* Method is invoked whether the requested operation for the given record can be
 * carried out in the specific controller*/
upll_rc_t VtermIfFlowFilterMoMgr::ValidateCapability(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 const char *ctrlr_name) {
  UPLL_FUNC_TRACE;

  if ((NULL == req) || (NULL == ikey)) {
    UPLL_LOG_ERROR("IpcReqRespHeader/ConfigKeyval is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if (!ctrlr_name) {
    ctrlr_name = reinterpret_cast<char*>((reinterpret_cast<key_user_data_t *>
                  (ikey->get_user_data()))->ctrlr_id);
  }

  if (NULL == ctrlr_name) {
    UPLL_LOG_ERROR("ctrlr_name is NULL");
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
        UPLL_LOG_TRACE("Calling GetStateCapability Operation  %d ",
                       req->operation);
        result_code = GetStateCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      } else {
        UPLL_LOG_TRACE("Calling GetReadCapability Operation  %d ",
                       req->operation);
        result_code = GetReadCapability(ctrlr_name, ikey->get_key_type(),
                                      &max_attrs, &attrs);
      }
    }
  }

  if (!result_code) {
    UPLL_LOG_ERROR("keytype(%d) is not supported by controller(%s) "
                   "for opeartion(%d)",
                   ikey->get_key_type(), ctrlr_name, req->operation);
    return UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR;
  }
  return UPLL_RC_SUCCESS;
}

/* Method is invoked to validate the key, val and the req*/
upll_rc_t VtermIfFlowFilterMoMgr::ValidateMessage(IpcReqRespHeader *req,
                                                ConfigKeyVal *key) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  if ((NULL == req) || (NULL == key)) {
    UPLL_LOG_ERROR("ConfigKeyval is NULL");
    return UPLL_RC_ERR_BAD_REQUEST;
  }

  if (UNC_KT_VTERMIF_FLOWFILTER != key->get_key_type()) {
    UPLL_LOG_ERROR(" Invalid keytype(%d)", key->get_key_type());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (key->get_st_num() != IpctSt::kIpcStKeyVtermIfFlowfilter) {
     UPLL_LOG_ERROR("Invalid key structure received. received struct num - %d",
                  key->get_st_num());
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  if (req->option2 != UNC_OPT2_NONE) {
    UPLL_LOG_ERROR(" Error: option2 is not NONE");
    return UPLL_RC_ERR_INVALID_OPTION2;
  }
  if ((req->option1 != UNC_OPT1_NORMAL)
              &&(req->option1 != UNC_OPT1_DETAIL)) {
     UPLL_LOG_ERROR(" Error: option1 is not NORMAL or DETAIL");
     return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if ((req->option1 != UNC_OPT1_NORMAL)
              &&(req->operation == UNC_OP_READ_SIBLING_COUNT)) {
     UPLL_LOG_ERROR(" Error: option1 is not NORMAL for ReadSiblingCount");
     return UPLL_RC_ERR_INVALID_OPTION1;
  }
  if ((req->option1 == UNC_OPT1_DETAIL) &&
      (req->datatype != UPLL_DT_STATE)) {
      UPLL_LOG_ERROR(" Invalid Datatype(%d)", req->datatype);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  if ((req->datatype == UPLL_DT_IMPORT) && (req->operation == UNC_OP_READ ||
       req->operation == UNC_OP_READ_SIBLING ||
       req->operation == UNC_OP_READ_SIBLING_BEGIN ||
       req->operation == UNC_OP_READ_NEXT ||
       req->operation == UNC_OP_READ_BULK ||
       req->operation == UNC_OP_READ_SIBLING_COUNT)) {
    UPLL_LOG_ERROR("Operations not allowed for this DT %d", req->operation);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }

  /** Read key structure */
  key_vterm_if_flowfilter_t *key_vterm_if_flowfilter =
      reinterpret_cast<key_vterm_if_flowfilter_t *>(key->get_key());

  /** Validate key structure */
  if (NULL == key_vterm_if_flowfilter) {
    UPLL_LOG_ERROR("KT_VTERMIF_FLOWFILTER Key structure is empty!!");
    return UPLL_RC_ERR_BAD_REQUEST;
  }
/* Validate whether the vtn, vterm, interface, direction present in the
 * key are valid*/
    rt_code = ValidateVtermIfFlowfilterKey(key_vterm_if_flowfilter,
                                         req->operation);

    if (UPLL_RC_SUCCESS != rt_code) {
      UPLL_LOG_ERROR(" key_vtermif_flowfilter syntax validation failed :"
                     "Err Code - %d",
                     rt_code);
      return rt_code;
    }

  return UPLL_RC_SUCCESS;
}

/* Method validates the key parameters of vterm_flowfilter*/
upll_rc_t VtermIfFlowFilterMoMgr::ValidateVtermIfFlowfilterKey(
    key_vterm_if_flowfilter_t* key_vterm_if_flowfilter,
    unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  upll_rc_t rt_code = UPLL_RC_ERR_GENERIC;

  VtermIfMoMgr *mgrvtermif = reinterpret_cast<VtermIfMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));

  if (NULL == mgrvtermif) {
    UPLL_LOG_ERROR("unable to get VtnMoMgr object to validate key_vtn");
    return UPLL_RC_ERR_GENERIC;
  }
/* Validates the vterm_if key present in vterm_if_ff key*/
  rt_code = mgrvtermif->ValidateVtermIfKey(&(key_vterm_if_flowfilter->if_key));

  if (UPLL_RC_SUCCESS != rt_code) {
    UPLL_LOG_ERROR(" Vtermif_key syntax validation failed :Err Code - %d",
                   rt_code);
    return rt_code;
  }

  if ((op != UNC_OP_READ_SIBLING_COUNT) &&
      (op != UNC_OP_READ_SIBLING_BEGIN)) {
/** validate direction */
    if (!ValidateNumericRange(key_vterm_if_flowfilter->direction,
                              (uint8_t) UPLL_FLOWFILTER_DIR_IN,
                              (uint8_t) UPLL_FLOWFILTER_DIR_OUT, true, true)) {
      UPLL_LOG_ERROR("direction syntax validation failed ");
      return UPLL_RC_ERR_CFG_SYNTAX;
    }
  } else {
/* input direction should be not set for sibling begin or count operation
 * as 0 or 1 are valid values setting an invalid value; */
    key_vterm_if_flowfilter->direction = 0xFE;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterMoMgr::ValidateAttribute(ConfigKeyVal *ikey,
                                                  DalDmlIntf *dmi,
                                                  IpcReqRespHeader *req) {
/* No operations done as vterm_if_flowfilter does not have any attributes in
 * val structure*/
  return UPLL_RC_SUCCESS;
}

/* Method is called to check whether the record exists in specified DB*/
upll_rc_t VtermIfFlowFilterMoMgr::IsReferenced(IpcReqRespHeader *req,
                                               ConfigKeyVal *ikey,
                                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone };
/* Check the object existence*/
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi,
                               &dbop, MAINTBL);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_ERROR("UpdateConfigDB failed %d", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}

/* Method allocates val structure depending upon the table specified*/
upll_rc_t VtermIfFlowFilterMoMgr::AllocVal(ConfigVal *&ck_val,
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

/* Method is used to create a configkey of vterm_if_flowfilter_entry
 * from parent configkey or itself*/
upll_rc_t VtermIfFlowFilterMoMgr::GetChildConfigKey(ConfigKeyVal *&okey,
                                                  ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vterm_if_flowfilter_t *vterm_if_ff_key = NULL;
  void *pkey = NULL;
/* If parent configkey is null, create an empty configkey of vterm_if_ffe*/
  if (parent_key == NULL) {
    vterm_if_ff_key = reinterpret_cast<key_vterm_if_flowfilter_t*>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_t)));
/* If no direction is specified , 0xFE is filled to bind output direction*/
    vterm_if_ff_key->direction = 0xFE;
    okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            vterm_if_ff_key, NULL);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    UPLL_LOG_ERROR("Parent key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
/* If the KT of output configkey is not UNC_KT_VTERMIF_FLOWFILTER
 * return error*/
  if (okey) {
    if (okey->get_key_type() != UNC_KT_VTERMIF_FLOWFILTER) {
      UPLL_LOG_ERROR("okey KT is not UNC_KT_VTERMIF_FLOWFILTER");
      return UPLL_RC_ERR_GENERIC;
    }
  }
  if ((okey) && (okey->get_key())) {
    vterm_if_ff_key = reinterpret_cast<key_vterm_if_flowfilter_t *>
        (okey->get_key());
  } else {
    vterm_if_ff_key = reinterpret_cast<key_vterm_if_flowfilter_t *>
        (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_t)));
/* If no direction is specified , 0xFE is filled to bind output direction*/
    vterm_if_ff_key->direction = 0xFE;
  }

  switch (parent_key->get_key_type()) {
    case UNC_KT_VTN:
      uuu::upll_strncpy(
          vterm_if_ff_key->if_key.vterm_key.vtn_key.vtn_name,
          reinterpret_cast<key_vtn_t *>
          (pkey)->vtn_name,
          kMaxLenVtnName + 1);
      break;
    case UNC_KT_VTERMINAL:
      uuu::upll_strncpy(
          vterm_if_ff_key->if_key.vterm_key.vtn_key.vtn_name,
          reinterpret_cast<key_vterm_t *>
          (pkey)->vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vterm_if_ff_key->if_key.vterm_key.vterminal_name,
          reinterpret_cast<key_vterm_t *>
          (pkey)->vterminal_name,
          kMaxLenVnodeName + 1);
      break;
    case UNC_KT_VTERM_IF:
      uuu::upll_strncpy(
          vterm_if_ff_key->if_key.vterm_key.vtn_key.vtn_name,
          reinterpret_cast<key_vterm_if_t *>
          (pkey)->vterm_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vterm_if_ff_key->if_key.vterm_key.vterminal_name,
          reinterpret_cast<key_vterm_if_t *>
          (pkey)->vterm_key.vterminal_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vterm_if_ff_key->if_key.if_name,
                        reinterpret_cast<key_vterm_if_t *>
                        (pkey)->if_name,
                        kMaxLenInterfaceName + 1);
      break;
    case UNC_KT_VTERMIF_FLOWFILTER:
      uuu::upll_strncpy(
          vterm_if_ff_key->if_key.vterm_key.vtn_key.vtn_name,
          reinterpret_cast<key_vterm_if_flowfilter_t *>
          (pkey)->if_key.vterm_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      uuu::upll_strncpy(
          vterm_if_ff_key->if_key.vterm_key.vterminal_name,
          reinterpret_cast<key_vterm_if_flowfilter_t *>
          (pkey)->if_key.vterm_key.vterminal_name,
          kMaxLenVnodeName + 1);
      uuu::upll_strncpy(vterm_if_ff_key->if_key.if_name,
                        reinterpret_cast<key_vterm_if_flowfilter_t *>
                        (pkey)->if_key.if_name,
                        kMaxLenInterfaceName + 1);
      vterm_if_ff_key->direction =
          reinterpret_cast<key_vterm_if_flowfilter_t *>
          (pkey)->direction;
      break;
    default:
      if ((NULL == okey) || (NULL == okey->get_key())) {
        free(vterm_if_ff_key);
      }
      UPLL_LOG_ERROR("Wrong parent KT");
      return UPLL_RC_ERR_GENERIC;
  }


  if ((okey) && !(okey->get_key())) {
    UPLL_LOG_DEBUG("okey not null and flow list name updated");
    okey->SetKey(IpctSt::kIpcStKeyVtermIfFlowfilter, vterm_if_ff_key);
  }

  if (!okey) {
    okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            vterm_if_ff_key, NULL);
  }
  SET_USER_DATA(okey, parent_key);
  UPLL_LOG_DEBUG("okey filled Succesfully %d", result_code);
  return result_code;
}

/* The method is used to create a duplicate of the input configkeyval*/
upll_rc_t VtermIfFlowFilterMoMgr::DupConfigKeyVal(ConfigKeyVal *&okey,
                                                ConfigKeyVal *&req,
                                                MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  if (req == NULL) {
    UPLL_LOG_ERROR("Request is null");
    return UPLL_RC_ERR_GENERIC;
  }

  if (okey != NULL) {
    UPLL_LOG_ERROR("oKey already Contains Data");
    return UPLL_RC_ERR_GENERIC;
  }

  if (req->get_key_type() != UNC_KT_VTERMIF_FLOWFILTER) {
    UPLL_LOG_ERROR(" DupConfigKeyval Failed.");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();
/* If table is maintbl then create a configval for val_flowfilter type*/
  if (tmp) {
    if (tbl == MAINTBL) {
      val_flowfilter_t *ival = NULL;
      ival = reinterpret_cast<val_flowfilter_t *> (GetVal(req));
      if (NULL != ival) {
      val_flowfilter_t *vterm_if_flowfilter_val = NULL;
        vterm_if_flowfilter_val = reinterpret_cast<val_flowfilter_t *>
            (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
        memcpy(vterm_if_flowfilter_val, ival, sizeof(val_flowfilter_t));
        tmp1 = new ConfigVal(IpctSt::kIpcStValFlowfilter,
                             vterm_if_flowfilter_val);
      tmp1->set_user_data(tmp->get_user_data());
      }
    }
  }

  void *tkey = req->get_key();
  if (tkey != NULL) {
    key_vterm_if_flowfilter_t *ikey = NULL;
    ikey = reinterpret_cast<key_vterm_if_flowfilter_t *> (tkey);
    key_vterm_if_flowfilter_t *vterm_if_flowfilter =
        reinterpret_cast<key_vterm_if_flowfilter_t*>
          (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_t)));

    memcpy(vterm_if_flowfilter, ikey, sizeof(key_vterm_if_flowfilter_t));
    okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                            IpctSt::kIpcStKeyVtermIfFlowfilter,
                            vterm_if_flowfilter, tmp1);
    SET_USER_DATA(okey, req);

    UPLL_LOG_DEBUG("DupConfigkeyVal Succesfull.");
    return UPLL_RC_SUCCESS;
  }
  DELETE_IF_NOT_NULL(tmp1);
  return UPLL_RC_ERR_GENERIC;
}

upll_rc_t VtermIfFlowFilterMoMgr::UpdateMo(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_ERROR("Implementation Not supported for this KT.");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VtermIfFlowFilterMoMgr::RenameMo(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                         const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_ERROR("Implementation Not supported for this KT.");
  return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
}

upll_rc_t VtermIfFlowFilterMoMgr::MergeValidate(unc_key_type_t keytype,
                                             const char *ctrlr_id,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             upll_import_type import_type) {
  // Merge Conflict will never happen
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
}

/* This method is called to fetch the unc name of vtn and vterm names
 * from vterm rename table*/
upll_rc_t VtermIfFlowFilterMoMgr::GetRenamedUncKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *unc_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  if ((NULL == ikey) || (ctrlr_id == NULL) || (NULL == dmi)) {
    UPLL_LOG_ERROR("ikey/ctrlr_id dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t rename = 0;
  MoMgrImpl *mgrvterm = reinterpret_cast<MoMgrImpl *>
             (const_cast<MoManager *> (GetMoManager(UNC_KT_VTERMINAL)));
  if (mgrvterm == NULL) {
    UPLL_LOG_ERROR("mgrvterm NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  val_rename_vnode *rename_val = reinterpret_cast
          <val_rename_vnode*>(ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
  if (!rename_val) {
    UPLL_LOG_ERROR("rename_val NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  key_vterm_if_flowfilter_t *ctrlr_key =
      reinterpret_cast<key_vterm_if_flowfilter_t *> (ikey->get_key());
  if (!ctrlr_key) {
    UPLL_LOG_ERROR("ctrlr_key NULL");
    FREE_IF_NOT_NULL(rename_val);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(rename_val->ctrlr_vtn_name,
                    ctrlr_key->if_key.vterm_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  rename_val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;

  uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                    ctrlr_key->if_key.vterm_key.vterminal_name,
                    (kMaxLenVnodeName + 1));
  rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

  result_code = mgrvterm->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Unable to get the configkey for Vribdge");
    FREE_IF_NOT_NULL(rename_val);
    return result_code;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
/* Read the vterm rename table to get the unc names.
 * If UPLL_RC_ERR_NO_SUCH_INSTANCE is returned return SUCCESS.
 * If it returns UPLL_RC_SUCCESS then replace the controller vtn, vterm names
 * with unc names.
 * It is called when a response is received from controller*/
  result_code = mgrvterm->ReadConfigDB(unc_key, dt_type, UNC_OP_READ,
                                     dbop, dmi, RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    key_vterm_if_flowfilter_t *vterm_if_flowfilter_key =
        reinterpret_cast<key_vterm_if_flowfilter_t *> (unc_key->get_key());
    if (strcmp(reinterpret_cast<char *>(ctrlr_key->
               if_key.vterm_key.vtn_key.vtn_name),
               reinterpret_cast<const char *>(vterm_if_flowfilter_key->
               if_key.vterm_key.vtn_key.vtn_name))) {
      uuu::upll_strncpy(
                     ctrlr_key->if_key.vterm_key.vtn_key.vtn_name,
                     vterm_if_flowfilter_key->if_key.vterm_key.vtn_key.vtn_name,
                     (kMaxLenVtnName + 1));
      rename |= VTN_RENAME_FLAG;
    }
    if (strcmp(reinterpret_cast<char *>(ctrlr_key->
               if_key.vterm_key.vterminal_name),
               reinterpret_cast<const char *>(vterm_if_flowfilter_key->
               if_key.vterm_key.vterminal_name))) {
      uuu::upll_strncpy(ctrlr_key->if_key.vterm_key.vterminal_name,
                      vterm_if_flowfilter_key->if_key.vterm_key.vterminal_name,
                      (kMaxLenVnodeName + 1));
      rename |= VTERM_RENAME_FLAG;
    }
    SET_USER_DATA(ikey, unc_key);
    SET_USER_DATA_FLAGS(ikey, rename);
  }

  UPLL_LOG_TRACE("%s GetRenamedUncKey vtermifff end",
                  ikey->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  return UPLL_RC_SUCCESS;
}

/* This method called to fetch the controller name for vtn and vterm
 * from vterm rename table for the given controller. It is invoked when a
 * has to be sent to driver*/
upll_rc_t VtermIfFlowFilterMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  ConfigKeyVal *okey = NULL;

  MoMgrImpl *mgrvterm = reinterpret_cast<MoMgrImpl *>
    (const_cast<MoManager *> (GetMoManager(UNC_KT_VTERMINAL)));

  if (mgrvterm == NULL) {
    UPLL_LOG_ERROR("obj null");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = mgrvterm->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetChildConfigKey fail");
    return result_code;
  }

  if (ctrlr_dom) {
    SET_USER_DATA_CTRLR_DOMAIN(okey, *ctrlr_dom);
  } else {
    UPLL_LOG_ERROR("ctrlr null");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("ctrlr : %s; domain : %s", ctrlr_dom->ctrlr,
      ctrlr_dom->domain);

  uuu::upll_strncpy(
      reinterpret_cast<key_vterm_t *> (okey->get_key())->vtn_key.vtn_name,
      reinterpret_cast<key_vterm_if_flowfilter_t *>
      (ikey->get_key())->if_key.vterm_key.vtn_key.vtn_name,
      (kMaxLenVtnName + 1));

  uuu::upll_strncpy(
      reinterpret_cast<key_vterm *> (okey->get_key())->vterminal_name,
      reinterpret_cast<key_vterm_if_flowfilter_t *>
      (ikey->get_key())->if_key.vterm_key.vterminal_name,
      (kMaxLenVnodeName + 1));
/* vterm rename table is read. If it returns UPLL_RC_ERR_NO_SUCH_INSTANCE
 * return SUCCESS. If UPLL_RC_SUCCESS is returned replace unc names with
 * controller names in configkeyval.
 * It is invoked when a request has to be sent to driver*/
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutFlag };
  result_code = mgrvterm->ReadConfigDB(okey, dt_type, UNC_OP_READ,
      dbop, dmi, RENAMETBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB no instance");
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_ERROR("Unable to Read from DB");
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }

  val_rename_vnode *rename_val = NULL;
  rename_val = reinterpret_cast<val_rename_vnode *> (GetVal(okey));

  if (!rename_val) {
    UPLL_LOG_ERROR("Vbr Name is not Valid.");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(reinterpret_cast<key_vterm_if_flowfilter_t *>
      (ikey->get_key())->if_key.vterm_key.vtn_key.vtn_name,
      rename_val->ctrlr_vtn_name,
      (kMaxLenVtnName + 1));

  uuu::upll_strncpy(reinterpret_cast<key_vterm_if_flowfilter_t *>
      (ikey->get_key())->if_key.vterm_key.vterminal_name,
      rename_val->ctrlr_vnode_name,
      (kMaxLenVnodeName + 1));

  DELETE_IF_NOT_NULL(okey);
  UPLL_LOG_TRACE("End Input ConfigKeyVal= %s", ikey->ToStrAll().c_str());
  UPLL_LOG_DEBUG("Renamed Controller key is sucessfull.");
  return UPLL_RC_SUCCESS;
}

/* Method to update the config status during Audit*/
upll_rc_t VtermIfFlowFilterMoMgr::UpdateAuditConfigStatus(
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
    UPLL_LOG_ERROR("val strct is empty");
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

/* Method to read the record from specified DB. It is used for normal
 * and detail read*/
upll_rc_t VtermIfFlowFilterMoMgr::ReadMo(IpcReqRespHeader *req,
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
    UPLL_LOG_ERROR("MoMgrImpl::ReadMo - ValidateMessage %d", result_code);
    return result_code;
  }

  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  switch (req->datatype) {
/* Retrieving config information*/
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_STATE:
      if ((req->option1 == UNC_OPT1_NORMAL)&&
          (req->option2 == UNC_OPT2_NONE)) {
        result_code = ReadInfoFromDB(req, ikey, dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR(" Read request failed result(%d)", result_code);
          return result_code;
        }
/* Retrieving state information*/
      } else if ((req->datatype == UPLL_DT_STATE) &&
                 (req->option1 == UNC_OPT1_DETAIL) &&
                 (req->option2 == UNC_OPT2_NONE)) {
        result_code =  DupConfigKeyVal(dup_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("DupConfigKeyVal fail in ReadMo for dup_key");
          return result_code;
        }

        result_code = ReadConfigDB(dup_key, req->datatype,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("ReadConfigDB  fail in ReadMo for dup_key");
          delete dup_key;
          return result_code;
        }
        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(dup_key);
          UPLL_LOG_ERROR("DupConfigKeyVal fail in ReadMo for l_key");
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(dup_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
/* Validate Read capability for that controller*/
        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("validate Capability Failed %d", result_code);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(dup_key);
          return result_code;
        }

/* The method is called to get the controller names for
 * corresponding unc names*/
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("GetRenamedControllerKey failed %d", result_code);
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          return result_code;
        }

        GET_USER_DATA_FLAGS(dup_key, db_flag);
        if ((db_flag & SET_FLAG_PORTMAP)) {
          UPLL_LOG_DEBUG("Portmap is configured");
        } else {
          UPLL_LOG_ERROR("Portmap not configured");
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
        }
/* Send request to driver*/
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
          UPLL_LOG_ERROR("SendReqToDriver failed for Key %d controller %s",
                         l_key->get_key_type(),
                         reinterpret_cast<char *>(ctrlr_dom.ctrlr));
          DELETE_IF_NOT_NULL(dup_key);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          return ipc_resp.header.result_code;
        }
        if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Driver response for Key %d controller %s result %d",
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
          UPLL_LOG_ERROR("ConstructReadDetailResponse error code (%d)",
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

bool VtermIfFlowFilterMoMgr::IsValidKey(void *key, uint64_t index,
                                        MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t ret_val = UPLL_RC_SUCCESS;
  key_vterm_if_flowfilter_t *vterm_ff_key =
      reinterpret_cast<key_vterm_if_flowfilter_t *>(key);
  if (vterm_ff_key == NULL) {
    return false;
  }

  switch (index) {
    case uudst::vterm_if_flowfilter::kDbiVtnName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vterm_ff_key->if_key.vterm_key.vtn_key.vtn_name),
                            kMinLenVtnName, kMaxLenVtnName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTN Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vterm_if_flowfilter::kDbiVtermName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vterm_ff_key->if_key.vterm_key.vterminal_name),
                            kMinLenVnodeName, kMaxLenVnodeName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTERM Name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vterm_if_flowfilter::kDbiVtermIfName:
      ret_val = ValidateKey(reinterpret_cast<char *>
                            (vterm_ff_key->if_key.if_name),
                            kMinLenInterfaceName, kMaxLenInterfaceName);
      if (ret_val != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("VTERM interface name is not valid(%d)", ret_val);
        return false;
      }
      break;
    case uudst::vterm_if_flowfilter::kDbiInputDirection:
      if (vterm_ff_key->direction == 0xFE) {
        // if operation is read sibling begin or
        // read sibling count return false
        // for output binding
        vterm_ff_key->direction = 0;
        return false;
      } else {
        if (!ValidateNumericRange(vterm_ff_key->direction,
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


upll_rc_t VtermIfFlowFilterMoMgr::UpdateConfigStatus(ConfigKeyVal *key,
                                                   unc_keytype_operation_t op,
                                                   uint32_t driver_result,
                                                   ConfigKeyVal *upd_key,
                                                   DalDmlIntf *dmi,
                                                   ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  val_flowfilter_t *vterm_if_flowfilter_val = NULL;
  unc_keytype_configstatus_t cs_status =
      (driver_result == 0) ? UNC_CS_APPLIED : UNC_CS_NOT_APPLIED;
  vterm_if_flowfilter_val = reinterpret_cast<val_flowfilter_t *> (GetVal(key));
  if (vterm_if_flowfilter_val == NULL) return UPLL_RC_ERR_GENERIC;

  if (op == UNC_OP_CREATE) {
    if (vterm_if_flowfilter_val->cs_row_status != UNC_CS_NOT_SUPPORTED)
      vterm_if_flowfilter_val->cs_row_status = cs_status;
  } else {
    UPLL_LOG_ERROR("Operation Not Supported.");
    return UPLL_RC_ERR_GENERIC;
  }

  UPLL_LOG_DEBUG("Update Config Status Successfull.");
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterMoMgr::CopyToConfigKey(ConfigKeyVal *&okey,
                                              ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_ERROR("Input Key Not Valid");
    return UPLL_RC_ERR_GENERIC;
  }

  key_rename_vnode_info *key_rename = NULL;
  key_rename = reinterpret_cast<key_rename_vnode_info *> (ikey->get_key());
  key_vterm_if_flowfilter_t * key_vterm_if =
      reinterpret_cast<key_vterm_if_flowfilter_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vterm_if_flowfilter_t)));

  if (!strlen(reinterpret_cast<char *> (key_rename->old_unc_vtn_name))) {
    UPLL_LOG_ERROR("old_unc_vtn_name NULL");
    if (key_vterm_if) free(key_vterm_if);
    return UPLL_RC_ERR_GENERIC;
  }

  uuu::upll_strncpy(key_vterm_if->if_key.vterm_key.vtn_key.vtn_name,
                    key_rename->old_unc_vtn_name,
                    (kMaxLenVtnName + 1));

  if (UNC_KT_VTERMINAL == ikey->get_key_type()) {
    if (!strlen(reinterpret_cast<char *>(key_rename->old_unc_vnode_name))) {
      UPLL_LOG_ERROR("old_unc_vnode_name NULL");
      free(key_vterm_if);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vterm_if->if_key.vterm_key.vterminal_name,
                      key_rename->old_unc_vnode_name, (kMaxLenVnodeName + 1));
  } else {
    if (!strlen(reinterpret_cast<char *>(key_rename->new_unc_vnode_name))) {
      UPLL_LOG_ERROR("new_unc_vnode_name NULL");
      free(key_vterm_if);
      return UPLL_RC_ERR_GENERIC;
    }
    uuu::upll_strncpy(key_vterm_if->if_key.vterm_key.vterminal_name,
                      key_rename->new_unc_vnode_name, (kMaxLenVnodeName + 1));
  }
  key_vterm_if->direction = 0xFE;
  okey = new ConfigKeyVal(UNC_KT_VTERMIF_FLOWFILTER,
                           IpctSt::kIpcStKeyVtermIfFlowfilter,
                           key_vterm_if, NULL);
  if (!okey) {
    UPLL_LOG_ERROR("okey is NULL ");
    free(key_vterm_if);
    return UPLL_RC_ERR_GENERIC;
  }

  return result_code;
}

upll_rc_t VtermIfFlowFilterMoMgr::ConstructReadDetailResponse(
    ConfigKeyVal *ikey,
    ConfigKeyVal *drv_resp_ckv,
    controller_domain ctrlr_dom,
    ConfigKeyVal **okey,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *tmp_okey = NULL;
  ConfigVal *drv_resp_val = NULL;
  drv_resp_val =  drv_resp_ckv->get_cfg_val();
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code =  GetChildConfigKey(tmp_okey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetChildConfigKey failed err code (%d)", result_code);
    return result_code;
  }

  val_flowfilter_t *tmp_val_ff = reinterpret_cast<val_flowfilter_t *>
      (GetVal(ikey));
  if (!tmp_val_ff) {
    UPLL_LOG_ERROR(" Invalid value read from DB");
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
      // redirect-direction in val_entry
      result_code =  SetRedirectNodeAndPortForRead(ikey,
                                                   ctrlr_dom,
                                                   val_entry,
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

upll_rc_t VtermIfFlowFilterMoMgr::ReadSiblingMo(IpcReqRespHeader *req,
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
    UPLL_LOG_ERROR("validate Message Failed %d", result_code);
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
          if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("Read request failed result(%d)", result_code);
          } else {
            UPLL_LOG_ERROR("Read request failed result(%d)", result_code);
          }
          return result_code;
        }
        // Retrieving state information
      } else if ((req->datatype == UPLL_DT_STATE) &&
                 (req->option1 == UNC_OPT1_DETAIL) &&
                 (req->option2 == UNC_OPT2_NONE)) {
        result_code =  DupConfigKeyVal(tctrl_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR(" DupConfigKeyVal failed for l_key%d ", result_code);
          return result_code;
        }

        result_code = ReadInfoFromDB(req, tctrl_key, dmi, &ctrlr_dom);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_ERROR("ReadConfigDb failed for tctrl_key%d ", result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }

        result_code =  DupConfigKeyVal(l_key, ikey, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("DupConfigKeyVal fail in ReadSiblingMo for l_key");
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(tctrl_key, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(l_key, ctrlr_dom);
        result_code = ValidateCapability(req, ikey,
                   reinterpret_cast<const char *>(ctrlr_dom.ctrlr));
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("validate Capability Failed %d", result_code);
          DELETE_IF_NOT_NULL(tctrl_key);
          DELETE_IF_NOT_NULL(l_key);
          return result_code;
        }

        // 1.Getting renamed name if renamed
        result_code = GetRenamedControllerKey(l_key, req->datatype,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("GetRenamedControllerKey failed %d", result_code);
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(tctrl_key);
          return result_code;
        }

        // 2.send request to driver
        // ///////////////////////

        GET_USER_DATA_FLAGS(tctrl_key, db_flag);
        if ((db_flag & SET_FLAG_PORTMAP)) {
          UPLL_LOG_ERROR("Portmap is configured");
        } else {
          UPLL_LOG_ERROR("Portmap not configured");
          DELETE_IF_NOT_NULL(l_key);
          DELETE_IF_NOT_NULL(tctrl_key);
          return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
        }
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
          reinterpret_cast<key_vterm_if_flowfilter_t*>
              (l_key->get_key())->direction =
              reinterpret_cast<key_vterm_if_flowfilter_t*>
              (tmp_key->get_key())->direction;
          ipc_req.ckv_data = l_key;
          if (!IpcUtil::SendReqToDriver(
                      (const char *)ctrlr_dom.ctrlr,
                      reinterpret_cast<char *>(ctrlr_dom.domain),
                      PFCDRIVER_SERVICE_NAME, PFCDRIVER_SVID_LOGICAL,
                      &ipc_req, true, &ipc_resp)) {
            UPLL_LOG_ERROR("SendReqToDriver failed for Key %d controller %s",
                           l_key->get_key_type(),
                           reinterpret_cast<char *>(ctrlr_dom.ctrlr));
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            DELETE_IF_NOT_NULL(okey);
            return ipc_resp.header.result_code;
          }

          if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("Driver response for Key %d controller %s result %d",
                          l_key->get_key_type(), ctrlr_dom.ctrlr,
                          ipc_resp.header.result_code);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            DELETE_IF_NOT_NULL(okey);
            return ipc_resp.header.result_code;
          }

          result_code = ConstructReadDetailResponse(tmp_key,
                                                  ipc_resp.ckv_data,
                                                  ctrlr_dom,
                                                  &okey, dmi);

          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_ERROR("ConstructReadDetailResponse error code (%d)",
                           result_code);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            DELETE_IF_NOT_NULL(l_key);
            DELETE_IF_NOT_NULL(tctrl_key);
            DELETE_IF_NOT_NULL(okey);
            return result_code;
          }
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          tmp_key = tmp_key->get_next_cfg_key_val();
        }
        if ((okey != NULL) && (result_code == UPLL_RC_SUCCESS)) {
          ikey->ResetWith(okey);
          DELETE_IF_NOT_NULL(okey);
        }
        DELETE_IF_NOT_NULL(l_key);
        DELETE_IF_NOT_NULL(tctrl_key);
      }
      break;
    default:
      result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }
  return result_code;
}

upll_rc_t VtermIfFlowFilterMoMgr::TxUpdateController(unc_key_type_t keytype,
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

  // TxUpdate skipped if config mode is VIRTUAL
  if (config_mode == TC_CONFIG_VIRTUAL) {
    return UPLL_RC_SUCCESS;
  }

  if (affected_ctrlr_set == NULL)
      return UPLL_RC_ERR_GENERIC;
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
               ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
               ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));

  unc_keytype_operation_t op1 = op;

  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                     op, req, nreq, &dal_cursor_handle, dmi, config_mode,
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
    switch (op)   {
      case UNC_OP_CREATE:
      case UNC_OP_UPDATE:
      case UNC_OP_DELETE:
        op1 = op;
        result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("DupConfigKeyVal failed %d", result_code);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          dmi->CloseCursor(dal_cursor_handle, true);
          return result_code;
        }
        break;
      default:
        UPLL_LOG_ERROR("TxUpdateController Invalid operation");
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        dmi->CloseCursor(dal_cursor_handle, true);
        return UPLL_RC_ERR_GENERIC;
    }

    GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
    if (ctrlr_dom.ctrlr == NULL) {
        UPLL_LOG_ERROR("Ctrlr id is NULL");
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        dmi->CloseCursor(dal_cursor_handle, true);
        return UPLL_RC_ERR_GENERIC;
    }
    db_flag = 0;
    GET_USER_DATA_FLAGS(ck_main, db_flag);
    if (!(SET_FLAG_PORTMAP & db_flag)) {
      if (op1 != UNC_OP_UPDATE) {
        DELETE_IF_NOT_NULL(ck_main);
        continue;
      } else {
        GET_USER_DATA_FLAGS(nreq, db_flag_running);
        if (!(SET_FLAG_PORTMAP & db_flag_running)) {
          UPLL_LOG_DEBUG("portmap flag is not available at running as well ");
          DELETE_IF_NOT_NULL(ck_main);
          continue;
        }
        op1 = UNC_OP_DELETE;
        db_flag = db_flag_running;
      }
    } else if (op1 == UNC_OP_UPDATE) {
      GET_USER_DATA_FLAGS(nreq, db_flag_running);
      if (!(SET_FLAG_PORTMAP & db_flag_running)) {
         UPLL_LOG_DEBUG("Portmap flag is not set at running");
         op1 = UNC_OP_CREATE;
      }
    }

    if (op1 == UNC_OP_UPDATE) {
      DELETE_IF_NOT_NULL(ck_main);
      continue;
    }

    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
        ctrlr_dom.domain);
    upll_keytype_datatype_t dt_type = (op1 == UNC_OP_DELETE)?
             UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;
    result_code = GetRenamedControllerKey(ck_main, dt_type,
                                          dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("GetRenamedControllerKey failed %d", result_code);
      DELETE_IF_NOT_NULL(ck_main);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      dmi->CloseCursor(dal_cursor_handle, true);
      return result_code;
    }

    // Inserting the controller to Set
    affected_ctrlr_set->insert
      (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));

    ConfigKeyVal *ckv_driver = NULL;
    result_code = DupConfigKeyVal(ckv_driver, ck_main, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("DupConfigKeyVal failed %d", result_code);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(ck_main);
      DELETE_IF_NOT_NULL(nreq);
      dmi->CloseCursor(dal_cursor_handle, true);
      return result_code;
    }
    DELETE_IF_NOT_NULL(ck_main);

    ConfigKeyVal *ckv_unc = NULL;
    result_code = DupConfigKeyVal(ckv_unc, req, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("DupConfigKeyVal failed %d", result_code);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      DELETE_IF_NOT_NULL(ckv_driver);
      dmi->CloseCursor(dal_cursor_handle, true);
      return result_code;
    }

    result_code = tx_util->EnqueueRequest(session_id, config_id,
                                          UPLL_DT_CANDIDATE, op1, dmi,
                                          ckv_driver, ckv_unc, string());
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      DELETE_IF_NOT_NULL(ckv_driver);
      DELETE_IF_NOT_NULL(ckv_unc);
      dmi->CloseCursor(dal_cursor_handle, true);
      return result_code;
    }
  }
  dmi->CloseCursor(dal_cursor_handle, true);
  DELETE_IF_NOT_NULL(req);
  DELETE_IF_NOT_NULL(nreq);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
                 UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t VtermIfFlowFilterMoMgr::SetPortmapConfiguration(
                                 ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 DalDmlIntf *dmi,
                                 InterfacePortMapInfo flags,
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
  key_vterm_if_flowfilter_t *ff_key =
      reinterpret_cast<key_vterm_if_flowfilter_t *>
      (ckv->get_key());
  key_vterm_if_t *vtermif_key =
      reinterpret_cast<key_vterm_if_t *>(ikey->get_key());

  uuu::upll_strncpy(ff_key->if_key.vterm_key.vtn_key.vtn_name,
                    vtermif_key->vterm_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);

  uuu::upll_strncpy(ff_key->if_key.vterm_key.vterminal_name,
                    vtermif_key->vterm_key.vterminal_name,
                    kMaxLenVtnName + 1);

  uuu::upll_strncpy(ff_key->if_key.if_name,
                    vtermif_key->if_name,
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
    UPLL_LOG_ERROR("UpdateConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }
  ff_key->direction = 0xFE;

  result_code = ReadConfigDB(ckv, dt_type ,
                             UNC_OP_READ, dbop, dmi, MAINTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    UPLL_LOG_ERROR("No Recrods in the Vterm_If_FlowFilter Table");
    DELETE_IF_NOT_NULL(ckv);
    return UPLL_RC_SUCCESS;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Read ConfigDB failure %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }
  uint8_t flag_port_map = 0;
  ConfigKeyVal *temp_ckv = ckv;
  while (temp_ckv) {
    flag_port_map = 0;
    GET_USER_DATA_FLAGS(temp_ckv, flag_port_map);
    if (flags & kPortMapConfigured) {
       if (flag_port_map & SET_FLAG_PORTMAP) {
        UPLL_LOG_DEBUG("Port-Map flag is already set in DB");
        DELETE_IF_NOT_NULL(ckv);
        return UPLL_RC_SUCCESS;
      }
      UPLL_LOG_DEBUG("only portmap");
      flag_port_map |= SET_FLAG_PORTMAP;
    } else {
      UPLL_LOG_DEBUG("No portmap");
      flag_port_map &= SET_FLAG_NO_VLINK_PORTMAP;
    }
    SET_USER_DATA_FLAGS(temp_ckv, flag_port_map);

    DbSubOp dbop_up = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
    result_code = UpdateConfigDB(temp_ckv, dt_type, UNC_OP_UPDATE,
                                 dmi, &dbop_up, config_mode,
                                 vtn_name, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("failed to update the portmap configuration");
      DELETE_IF_NOT_NULL(ckv);
      return result_code;
    }

    VtermIfFlowFilterEntryMoMgr *mgr =
        reinterpret_cast<VtermIfFlowFilterEntryMoMgr *>
        (const_cast<MoManager *>
         (GetMoManager(UNC_KT_VTERMIF_FLOWFILTER_ENTRY)));
    if (mgr == NULL) {
      DELETE_IF_NOT_NULL(ckv);
      UPLL_LOG_ERROR("mgr is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->SetPortmapConfiguration(temp_ckv, dt_type, dmi,
                                               flags, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("update portmap flag for flowfilterentry failed, err %d",
                     result_code);
      DELETE_IF_NOT_NULL(ckv);
      return result_code;
    }
    temp_ckv = temp_ckv->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterMoMgr::GetControllerDomainID(ConfigKeyVal *ikey,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  ConfigKeyVal *ckv = NULL;
  result_code = GetParentConfigKey(ckv, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Unable to get the ParentConfigKey, resultcode=%d",
                    result_code);
    return result_code;
  }

  VtermIfMoMgr *mgrvtermif = reinterpret_cast<VtermIfMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));
  if (mgrvtermif == NULL) {
    UPLL_LOG_ERROR("Unable to get the instance of VTERMIF");
    return result_code;
  }
  ConfigKeyVal *vterm_key = NULL;
  result_code = mgrvtermif->GetParentConfigKey(vterm_key, ckv);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("GetParentConfigKey Failed, err %d", result_code);
    DELETE_IF_NOT_NULL(ckv);
    return result_code;
  }

  result_code = mgrvtermif->GetControllerDomainId(vterm_key, dt_type,
                                                &ctrlr_dom, dmi);
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    DELETE_IF_NOT_NULL(ckv);
    DELETE_IF_NOT_NULL(vterm_key);
    UPLL_LOG_ERROR("GetControllerDomainId error err code(%d)", result_code);
    return result_code;
  }

  UPLL_LOG_DEBUG("ctrlrid %s, domainid %s",
                   ctrlr_dom.ctrlr, ctrlr_dom.domain);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);

  DELETE_IF_NOT_NULL(ckv);
  DELETE_IF_NOT_NULL(vterm_key);
  return result_code;
}

upll_rc_t VtermIfFlowFilterMoMgr::GetParentConfigKey(ConfigKeyVal *&okey,
                                                   ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;

  if (!ikey) {
    UPLL_LOG_ERROR(" Input Key is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DELETE_IF_NOT_NULL(okey);
  unc_key_type_t ikey_type = ikey->get_key_type();
  if (ikey_type != UNC_KT_VTERMIF_FLOWFILTER) {
    UPLL_LOG_ERROR(" Invalid key type received. Key type - %d", ikey_type);
    return UPLL_RC_ERR_GENERIC;
  }

  key_vterm_if_flowfilter_t *pkey =
      reinterpret_cast<key_vterm_if_flowfilter_t*>(ikey->get_key());
  if (!pkey) {
    UPLL_LOG_ERROR(" Input vterm if flow filter key is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  key_vterm_if_t *vterm_if_key = reinterpret_cast<key_vterm_if_t*>
      (ConfigKeyVal::Malloc(sizeof(key_vterm_if_t)));

  uuu::upll_strncpy(vterm_if_key->vterm_key.vtn_key.vtn_name,
                    reinterpret_cast<key_vterm_if_flowfilter_t *>
                    (pkey)->if_key.vterm_key.vtn_key.vtn_name,
                    kMaxLenVtnName + 1);
  uuu::upll_strncpy(vterm_if_key->vterm_key.vterminal_name,
                    reinterpret_cast<key_vterm_if_flowfilter_t *>
                    (pkey)->if_key.vterm_key.vterminal_name,
                    kMaxLenVnodeName + 1);
  uuu::upll_strncpy(vterm_if_key->if_name,
                    reinterpret_cast<key_vterm_if_flowfilter_t *>
                    (pkey)->if_key.if_name,
                    kMaxLenInterfaceName + 1);
  okey = new ConfigKeyVal(UNC_KT_VTERM_IF,
                          IpctSt::kIpcStKeyVtermIf,
                          vterm_if_key, NULL);

  SET_USER_DATA(okey, ikey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterMoMgr::DeleteChildrenPOM(
          ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
          DalDmlIntf *dmi,
          TcConfigMode config_mode,
          string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  if (NULL == ikey || NULL == dmi) {
    UPLL_LOG_ERROR("Delete Operation failed:Bad request");
    return result_code;
  }
  // Read the DB get the flowlist value and send the delete request to
  // flowlist momgr if flowlist is configured.

  ConfigKeyVal *tempckv = NULL;
  result_code = GetChildConfigKey(tempckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("GetChildConfigKey failed %d", result_code);
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
    UPLL_LOG_ERROR("DeleteMo record Err in vtnpolicingmaptbl (%d)",
        result_code);
    return result_code;
  }
  delete tempckv;
  tempckv = NULL;
  return  UPLL_RC_SUCCESS;
}

upll_rc_t VtermIfFlowFilterMoMgr::SetValidAudit(ConfigKeyVal *&ikey) {
  UPLL_FUNC_TRACE;
  val_flowfilter_t *val = reinterpret_cast<val_flowfilter_t *>
    (ConfigKeyVal::Malloc(sizeof(val_flowfilter_t)));
  val->cs_row_status = UNC_CS_APPLIED;
  ikey->AppendCfgVal(IpctSt::kIpcStValFlowfilter, val);
  return UPLL_RC_SUCCESS;
}

bool VtermIfFlowFilterMoMgr::FilterAttributes(void *&val1,
                                          void *val2,
                                          bool copy_to_running,
                                          unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  if (op != UNC_OP_CREATE)
    return true;
  return false;
}
upll_rc_t VtermIfFlowFilterMoMgr::TxUpdateErrorHandler(ConfigKeyVal *ckv_unc,
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
