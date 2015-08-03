/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <map>
#include <string>
#include <list>
#include "momgr_impl.hh"
#include "vtn_momgr.hh"
#include "vbr_momgr.hh"
#include "uncxx/upll_log.hh"
#include "vlink_momgr.hh"
#include "vnode_momgr.hh"
#include "vnode_child_momgr.hh"
#include "flowlist_momgr.hh"
#include "flowlist_entry_momgr.hh"
#include "policingprofile_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "vbr_portmap_momgr.hh"
#include "vtunnel_momgr.hh"
#include "unw_spine_domain_momgr.hh"
#define IMPORT_READ_FAILURE 0xFF

namespace unc {
namespace upll {
namespace kt_momgr {

upll_rc_t MoMgrImpl::GetInstanceCount(ConfigKeyVal *ikey,
                                      char *ctrlr_id,
                                      upll_keytype_datatype_t dt_type,
                                      uint32_t *count,
                                      DalDmlIntf *dmi,
                                      MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code;
  const uudst::kDalTableIndex tbl_index = GetTable(tbl, dt_type);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey == NULL) {
    UPLL_LOG_DEBUG("Invalid Param ikey/ctrlr_id");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadCount, kOpMatchCtrlr, kOpInOutNone };
  if (!ctrlr_id || strlen(ctrlr_id) == 0) {
    dbop.matchop = kOpMatchNone;
  } else {
    SET_USER_DATA_CTRLR(ikey, ctrlr_id);
  }

  DalBindInfo *dal_bind_info = new DalBindInfo(tbl_index);
  result_code = BindAttr(dal_bind_info, ikey, UNC_OP_READ, dt_type, dbop, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("BindAttr returns error %d", result_code);
    delete dal_bind_info;
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = DalToUpllResCode(dmi->GetRecordCount(
          (dt_type == UPLL_DT_STATE)?UPLL_DT_RUNNING:dt_type,
          tbl_index, dal_bind_info, count));
  delete dal_bind_info;
  return result_code;
}

upll_rc_t MoMgrImpl::IsRenamed(ConfigKeyVal *ikey,
                               upll_keytype_datatype_t dt_type,
                               DalDmlIntf *dmi,
                               uint8_t &rename) {
  UPLL_FUNC_TRACE;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag | kOpInOutCtrlr
                       | kOpInOutDomain };
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code;
  MoMgrTables tbl = MAINTBL;
  /* rename is set and dt_type is running/audit implies
   * operaton is delete and ikey has to be populated with
   * val from db.
   */
  if (rename &&
     ((dt_type == UPLL_DT_RUNNING) ||
      (dt_type == UPLL_DT_AUDIT))) {
    okey = ikey;
  } else  {
    result_code = GetChildConfigKey(okey, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_TRACE("Returning error %d", result_code);
       return result_code;
    }
  }

  if (UNC_KT_VTN == table[MAINTBL]->get_key_type()) {
    controller_domain_t ctrlr_dom;
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    /* if controller and domain is present then bind it for
     * match and get the exact information from vtn controller
     * table
     */
     if (ctrlr_dom.ctrlr != NULL && ctrlr_dom.domain != NULL) {
         dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;
         dbop.inoutop = kOpInOutFlag;
     }
     tbl = CTRLRTBL;
  }
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                                       tbl);
  if ((result_code != UPLL_RC_SUCCESS) &&
       (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE))  {
    UPLL_LOG_DEBUG("Returning error code %d", result_code);
    if (okey != ikey) delete okey;
    return result_code;
  }
  if (okey != ikey)
    SET_USER_DATA(ikey, okey);
  GET_USER_DATA_FLAGS(okey, rename);
#if 0
  rename &= RENAME;
#else
  GET_RENAME_FLAG(rename, ikey->get_key_type())
#endif
  if (okey != ikey) delete okey;
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::DeleteChildren(ConfigKeyVal *ikey,
                                    ConfigKeyVal *pkey,
                                    upll_keytype_datatype_t dt_type,
                                    DalDmlIntf *dmi,
                                    TcConfigMode config_mode,
                                    string vtn_name,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t ktype;

  for (int i = nchild; i > 0; i--) {
    ConfigKeyVal *tkey = NULL;
    ktype = child[(i - 1)];
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
                          (const_cast<MoManager*>(GetMoManager(ktype)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr %d", ktype);
      continue;
    }
    result_code = mgr->GetChildConfigKey(tkey, pkey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
      return result_code;
    }
    /* For deleting the vnode rename table for the vtn or vnode
     * need no to match the controller and domain
     */
    memset(tkey->get_user_data(), 0 , sizeof(key_user_data_t));
    result_code = mgr->DeleteChildren(tkey, pkey, dt_type, dmi, config_mode,
                                      vtn_name);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_ERROR("DeleteChildren failed %d", result_code);
    }
    DELETE_IF_NOT_NULL(tkey);
  }
  bool kt_flag = false;
  IS_POM_KT(GetMoMgrKeyType(MAINTBL, dt_type), kt_flag);
  if (kt_flag) {
    result_code = DeleteChildrenPOM(ikey, dt_type, dmi, config_mode, vtn_name);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("DeleteChildrenPOM failed %d", result_code);
      return result_code;
    }
  } else {
    /* Delete all the tables for this momgr
     * RENAMETBL to be deleted only once */
    for (int j = get_ntable(); j > MAINTBL; j--) {
      if ((GetTable((MoMgrTables)(j - 1), dt_type) >= uudst::kDalNumTables)) {
        continue;
      }
      // don't delete the convert table, it is deleted as part of KT_VBR_PORTMAP
      if (((MoMgrTables)(j-1)) == CONVERTTBL) {
        continue;
      }
      if (ikey->get_key_type() == UNC_KT_UNW_SPINE_DOMAIN) {
        // When SPINE_DOMAIN is deleted, deallocate GVTNID allocated for this
        // domain
        ConfigKeyVal *spd_ckv = NULL;
        result_code = GetChildConfigKey(spd_ckv, ikey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Error in retrieving Child ConfigKeyVal");
          return result_code;
        }
        DbSubOp rd_dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
        result_code = ReadConfigDB(spd_ckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                   rd_dbop, dmi, MAINTBL);
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG("No spine domain found");
        } else if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Error in Reading spine domain ckv %d", result_code);
          DELETE_IF_NOT_NULL(spd_ckv);
          return result_code;
        } else {
          VbrPortMapMoMgr *vbrpm_mgr = reinterpret_cast<VbrPortMapMoMgr *>(
              const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_PORTMAP)));
          for (ConfigKeyVal *tmp_spd = spd_ckv; tmp_spd;
               tmp_spd = tmp_spd->get_next_cfg_key_val()) {
            result_code = vbrpm_mgr->DeAllocGvtnId(tmp_spd, dmi,
                                                   config_mode, vtn_name);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_ERROR("Error in DeAllocGvtnId: %d", result_code);
              DELETE_IF_NOT_NULL(spd_ckv);
              return result_code;
            }
          }
        }
        DELETE_IF_NOT_NULL(spd_ckv);
      }
      /* Match Controller and domain is not need for delete children*/
      DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
      result_code = UpdateConfigDB(ikey, dt_type, UNC_OP_DELETE, dmi, &dbop,
                                   config_mode, vtn_name, (MoMgrTables)(j - 1));
      result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                     UPLL_RC_SUCCESS : result_code;
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DeleteChild failed with result_code %d", result_code);
        return result_code;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::DiffConfigDB(upll_keytype_datatype_t dt_cfg1,
                                  upll_keytype_datatype_t dt_cfg2,
                                  unc_keytype_operation_t op,
                                  ConfigKeyVal *&req,
                                  ConfigKeyVal *&nreq,
                                  DalCursor **cfg1_cursor,
                                  DalDmlIntf *dmi,
                                  TcConfigMode config_mode,
                                  std::string vtn_name,
                                  MoMgrTables tbl) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code;
  result_code = DiffConfigDB(dt_cfg1, dt_cfg2, op, req, nreq, cfg1_cursor, dmi,
                             NULL, config_mode, vtn_name, tbl);
  return result_code;
}

upll_rc_t MoMgrImpl::DiffConfigDB(upll_keytype_datatype_t dt_cfg1,
                                  upll_keytype_datatype_t dt_cfg2,
                                  unc_keytype_operation_t op,
                                  ConfigKeyVal *&req,
                                  ConfigKeyVal *&nreq,
                                  DalCursor **cfg1_cursor,
                                  DalDmlIntf *dmi,
                                  uint8_t *ctrlr_id,
                                  TcConfigMode config_mode,
                                  std::string vtn_name,
                                  MoMgrTables tbl,
                                  bool read_withcs,
                                  bool auditdiff_with_flag) {
  UPLL_FUNC_TRACE;
  const uudst::kDalTableIndex tbl_index = GetTable(tbl, dt_cfg1);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("Table Index %d Table %d Operation op %d",
                 tbl_index, tbl, op);
  upll_rc_t result_code;
  DalResultCode db_result = uud::kDalRcSuccess;
  if (tbl == CONVERTTBL) {
    if (tbl_index == uudst::kDbiConvertVbrTbl ||
        tbl_index == uudst::kDbiConvertVbrIfTbl ||
        tbl_index == uudst::kDbiConvertVtunnelTbl ||
        tbl_index == uudst::kDbiConvertVtunnelIfTbl ||
        tbl_index == uudst::kDbiConvertVlinkTbl) {
      result_code = GetChildConvertConfigKey(req, NULL);
    } else {
      result_code = GetChildConfigKey(req, NULL);
    }
  } else {
    result_code = GetChildConfigKey(req, NULL);
  }
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Error from GetGetChildConfigKey for table(%d)", tbl_index);
    return result_code;
  }
  DbSubOp dbop = { kOpReadDiff, kOpMatchCtrlr | kOpMatchDomain,
                   kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain };
  if (UNC_OP_DELETE == op)
    dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;
  uint16_t max_record_count = 0;
  #if 0
  if (ctrlr_id) {
    dbop.inoutop &= ~kOpInOutCtrlr;
    SET_USER_DATA_CTRLR(req, ctrlr_id)
  }
  #endif
  if (tbl == CTRLRTBL) {
    dbop.inoutop |= kOpInOutCs;
  }
  if (op == UNC_OP_UPDATE) {
    if (tbl == CTRLRTBL)
      dbop.matchop |= kOpMatchFlag;
    else
      dbop.matchop = kOpMatchFlag;
    if ((tbl == CONVERTTBL) && (dt_cfg2 == UPLL_DT_AUDIT) &&
        (op == UNC_OP_UPDATE)) {
      unc_key_type_t keytype = GetMoMgrKeyType(tbl, dt_cfg1);
      if (keytype == UNC_KT_VTN)
        dbop.matchop |= kOpMatchCtrlr | kOpMatchDomain;
    }
    dbop.readop |= kOpReadDiffUpd;
    if (read_withcs)
      dbop.inoutop |= kOpInOutCs;
  }
  if (dt_cfg2 == UPLL_DT_AUDIT) {
    dbop.matchop |= kOpMatchCs;
    if (!auditdiff_with_flag) {
      dbop.matchop &= ~kOpMatchFlag;
    } else {
      if (op == UNC_OP_CREATE || op == UNC_OP_DELETE) {
        dbop.matchop |= kOpMatchFlag;
      }
    }
  }

  if (dt_cfg1 == UPLL_DT_IMPORT) {
    if (!auditdiff_with_flag) {
      dbop.matchop &= ~kOpMatchFlag;
    }
  }

  DalBindInfo *binfo_cfg1 = new DalBindInfo(tbl_index);
  result_code = BindAttr(binfo_cfg1, req, UNC_OP_READ, dt_cfg1, dbop, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error from BindAttr for table(%d)", tbl_index);
    delete binfo_cfg1;
    DELETE_IF_NOT_NULL(req);
    req = NULL;
    return result_code;
  }

  if (dt_cfg1 == UPLL_DT_RUNNING && dt_cfg2 == UPLL_DT_AUDIT) {
    result_code = ContinueAuditProcess();
    if (result_code != UPLL_RC_SUCCESS) {
      delete binfo_cfg1;
      DELETE_IF_NOT_NULL(req);
      return result_code;
    }
  }
  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
      const_cast<char *>(vtn_name.c_str()));
  }

  switch (op) {
    case UNC_OP_DELETE:
      db_result = dmi->GetDeletedRecords(dt_cfg1, dt_cfg2, tbl_index,
                                         max_record_count, binfo_cfg1,
                                         cfg1_cursor, config_mode, vtnname);
      break;
    case UNC_OP_CREATE:
      db_result = dmi->GetCreatedRecords(dt_cfg1, dt_cfg2, tbl_index,
                                         max_record_count, binfo_cfg1,
                                         cfg1_cursor, config_mode, vtnname);
      break;
    case UNC_OP_UPDATE: {
      DalBindInfo *binfo_cfg2 = new DalBindInfo(tbl_index);
      if (tbl == CONVERTTBL) {
        if (tbl_index == uudst::kDbiConvertVbrTbl ||
            tbl_index == uudst::kDbiConvertVbrIfTbl ||
            tbl_index == uudst::kDbiConvertVtunnelTbl ||
            tbl_index == uudst::kDbiConvertVtunnelIfTbl ||
            tbl_index == uudst::kDbiConvertVlinkTbl) {
          result_code = GetChildConvertConfigKey(nreq, NULL);
        } else {
          result_code = GetChildConfigKey(nreq, NULL);
        }
      } else {
        result_code = GetChildConfigKey(nreq, NULL);
      }
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Failed in GetChildConfigKey");
        delete binfo_cfg1;
        delete binfo_cfg2;
        delete req;
        req = NULL;
        return result_code;
      }
      result_code = BindAttr(
          binfo_cfg2, nreq, UNC_OP_READ,
          ((dt_cfg2 == UPLL_DT_RUNNING)?UPLL_DT_STATE:dt_cfg2),
          dbop, tbl);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error from BindAttr for table(%d)", tbl_index);
        delete binfo_cfg1;
        delete binfo_cfg2;
        delete req;
        delete nreq;
        req = nreq = NULL;
        return result_code;
      }
      db_result = dmi->GetUpdatedRecords(dt_cfg1, dt_cfg2, tbl_index,
                                         max_record_count, binfo_cfg1,
                                         binfo_cfg2, cfg1_cursor,
                                         config_mode, vtnname);
      // For Success case, binfo_cfg2 will be deleted by the caller.
      if (db_result != uud::kDalRcSuccess) {
        delete binfo_cfg2;
      }
      break;
    }
    default:
      break;
  }
  result_code = DalToUpllResCode(db_result);
  // For Success case, binfo_cfg1 will be deleted by the caller.
  if (result_code != UPLL_RC_SUCCESS) {
    delete binfo_cfg1;
  }
  return result_code;
}

upll_rc_t MoMgrImpl::ReadConfigDB(ConfigKeyVal *ikey,
                                  upll_keytype_datatype_t dt_type,
                                  unc_keytype_operation_t op,
                                  DbSubOp dbop,
                                  DalDmlIntf *dmi,
                                  MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  uint32_t sibling_count = 0;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = ReadConfigDB(ikey, dt_type, op, dbop, sibling_count, dmi, tbl);
  return result_code;
}

upll_rc_t MoMgrImpl::ReadConfigDB(ConfigKeyVal *ikey,
                                  upll_keytype_datatype_t dt_type,
                                  unc_keytype_operation_t op,
                                  DbSubOp dbop,
                                  uint32_t &sibling_count,
                                  DalDmlIntf *dmi,
                                  MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  const uudst::kDalTableIndex tbl_index = GetTable(tbl, dt_type);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  DalCursor *dal_cursor_handle = NULL;
  UPLL_LOG_TRACE("tbl_index is %d", tbl_index);
  if (!READ_OP(op)) {
    UPLL_LOG_INFO("Exiting MoMgrImpl::ReadConfigDB");
    return UPLL_RC_ERR_GENERIC;
  }
  DalBindInfo *dal_bind_info = new DalBindInfo(tbl_index);
  upll_rc_t result_code;
  DalResultCode db_result = uud::kDalRcGeneralError;
#if 0
  uint16_t max_record_count = 1;
#endif
  ConfigKeyVal *tkey = NULL;
  result_code = BindAttr(dal_bind_info, ikey, op, dt_type, dbop, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    if (dal_bind_info) delete dal_bind_info;
    UPLL_LOG_INFO("Exiting MoMgrImpl::ReadConfigDB result code %d",
                  result_code);
    return result_code;
  }
  dt_type = (dt_type == UPLL_DT_STATE) ? UPLL_DT_RUNNING : dt_type;
  switch (op) {
    case UNC_OP_READ_SIBLING_COUNT:
      {
        db_result = dmi->GetRecordCount(dt_type, tbl_index, dal_bind_info,
                                        &sibling_count);
        uint32_t *sib_count =
            reinterpret_cast<uint32_t*>(ConfigKeyVal::Malloc
                (sizeof(uint32_t)));
        *sib_count = sibling_count;
        ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, sib_count));
      }
      break;
    case UNC_OP_READ_SIBLING:
      db_result = dmi->GetSiblingRecords(dt_type, tbl_index,
                                         (uint16_t) sibling_count,
                                         dal_bind_info, &dal_cursor_handle);
      break;
    case UNC_OP_READ_SIBLING_BEGIN:
      db_result = dmi->GetMultipleRecords(dt_type, tbl_index,
                                          (uint16_t) sibling_count,
                                          dal_bind_info,
                                          &dal_cursor_handle);
      break;
    case UNC_OP_READ:
      if (dbop.readop & kOpReadMultiple) {
        db_result = dmi->GetMultipleRecords(dt_type, tbl_index,
         sibling_count, dal_bind_info, &dal_cursor_handle);
      } else {
        db_result = dmi->GetSingleRecord(dt_type, tbl_index, dal_bind_info);
      }
      break;
    default:
      DELETE_IF_NOT_NULL(dal_bind_info);
      return UPLL_RC_ERR_GENERIC;
  }
  result_code = DalToUpllResCode(db_result);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning %d", result_code);
    delete dal_bind_info;
    return result_code;
  }
  if (dbop.readop & kOpReadMultiple) {
    uint32_t count = 0;
    uint32_t nrec_read = 0;
    ConfigKeyVal *end_resp = NULL;
    while ((!sibling_count) || (count < sibling_count)) {
      db_result = dmi->GetNextRecord(dal_cursor_handle);
      result_code = DalToUpllResCode(db_result);
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
           count = (op == UNC_OP_READ)?nrec_read:count;
           result_code = (count) ? UPLL_RC_SUCCESS : result_code;
           sibling_count = count;
        }
        break;
      }
      ConfigKeyVal *prev_key = tkey;
      tkey = NULL;
      result_code = DupConfigKeyVal(tkey, ikey, tbl);
      if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Dup failed error %d", result_code);
          delete dal_bind_info;
          DELETE_IF_NOT_NULL(end_resp);
          return result_code;
      }
      if (!end_resp) {
         end_resp = tkey;
      } else {
         prev_key->AppendCfgKeyVal(tkey);
      }
      if (op != UNC_OP_READ) {
        count++;
      } else {
        nrec_read++;
      }
    }
    if (result_code == UPLL_RC_SUCCESS) {
      if (end_resp)
        ikey->ResetWith(end_resp);
      UPLL_LOG_DEBUG(" sibling_count %d count %d operation %d response %s",
                   sibling_count, count, op, (ikey->ToStrAll()).c_str());
    }
    dmi->CloseCursor(dal_cursor_handle);
    DELETE_IF_NOT_NULL(end_resp);
  }
  if (dal_bind_info) delete dal_bind_info;
  return result_code;
}

upll_rc_t MoMgrImpl::UpdateConfigDB(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    unc_keytype_operation_t op,
                                    DalDmlIntf *dmi,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UpdateConfigDB(ikey, dt_type, op, dmi, NULL, tbl);

  return result_code;
}
#if 0
upll_rc_t MoMgrImpl::UpdateConfigDB(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    unc_keytype_operation_t op,
                                    DalDmlIntf *dmi,
                                    DbSubOp *pdbop,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  const uudst::kDalTableIndex tbl_index = GetTable(tbl, dt_type);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }

  bool exists = false;
  DalBindInfo *dal_bind_info = new DalBindInfo(tbl_index);
  upll_rc_t result_code;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutFlag | kOpInOutCtrlr
                       | kOpInOutDomain };
  if (pdbop == NULL) {
    if (op == UNC_OP_DELETE) dbop.inoutop = kOpInOutNone;
    if (op != UNC_OP_CREATE) {
      if ((tbl == RENAMETBL) || (tbl == CTRLRTBL)) {
        dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;
        dbop.inoutop = kOpInOutFlag | kOpInOutCs;
      }
      if (op == UNC_OP_UPDATE) {
        if  (dt_type == UPLL_DT_CANDIDATE) {
          dbop.inoutop = kOpInOutCs;
        } else if (dt_type == UPLL_DT_RUNNING) {
          dbop.inoutop |= kOpInOutCs;
        } else if (dt_type == UPLL_DT_AUDIT) {
          dbop.inoutop = kOpInOutFlag;
        }
      }
    } else {
      if (dt_type != UPLL_DT_CANDIDATE || tbl == CTRLRTBL)
        if (dt_type != UPLL_DT_AUDIT)
          dbop.inoutop |= kOpInOutCs;
    }
    pdbop = &dbop;
  }
  result_code = BindAttr(dal_bind_info, ikey, op, dt_type, *pdbop, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    if (dal_bind_info) delete dal_bind_info;
    return result_code;
  }
  dt_type = (dt_type == UPLL_DT_STATE) ? UPLL_DT_RUNNING : dt_type;
  switch (op) {
    case UNC_OP_CREATE:
      UPLL_LOG_TRACE("Dbop %s dt_type %d CREATE %d", (ikey->ToStrAll()).c_str(),
                     dt_type, tbl_index);
      result_code = DalToUpllResCode(
          dmi->CreateRecord(dt_type, tbl_index, dal_bind_info));
      break;
    case UNC_OP_DELETE:
      UPLL_LOG_TRACE("Dbop %s dt_type %d DELETE  %d",
                     (ikey->ToStrAll()).c_str(), dt_type,
                     tbl_index);
      result_code = DalToUpllResCode(
          dmi->DeleteRecords(dt_type, tbl_index, dal_bind_info, false));
      break;
    case UNC_OP_UPDATE:
      UPLL_LOG_TRACE("Dbop %s dt_type %d UPD  %d",
                     (ikey->ToStrAll()).c_str(), dt_type,
                     tbl_index);
      result_code = DalToUpllResCode(
          dmi->UpdateRecords(dt_type, tbl_index, dal_bind_info));
      break;
    case UNC_OP_READ:
      UPLL_LOG_TRACE("Dbop %s dt_type %d EXISTS  %d",
                     (ikey->ToStrAll()).c_str(), dt_type,
                      tbl_index);
      result_code = DalToUpllResCode(
          dmi->RecordExists(dt_type, tbl_index, dal_bind_info, &exists));
      if (result_code == UPLL_RC_SUCCESS) {
        if (exists)
          result_code = UPLL_RC_ERR_INSTANCE_EXISTS;
        else
          result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
      }
      break;
    default:
      break;
  }
  delete dal_bind_info;

  return result_code;
}
#endif
upll_rc_t MoMgrImpl::UpdateConfigDB(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    unc_keytype_operation_t op,
                                    DalDmlIntf *dmi,
                                    DbSubOp *pdbop,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  const uudst::kDalTableIndex tbl_index = GetTable(tbl, dt_type);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }

  bool exists = false;
  DalBindInfo *dal_bind_info = new DalBindInfo(tbl_index);
  upll_rc_t result_code;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutFlag | kOpInOutCtrlr
                       | kOpInOutDomain };
  if (op == UNC_OP_CREATE || op == UNC_OP_UPDATE || op == UNC_OP_DELETE) {
    UPLL_LOG_ERROR("Invalid operation - %d", op);
    if (dal_bind_info) delete dal_bind_info;
    return UPLL_RC_ERR_GENERIC;
  }

  if (pdbop == NULL) {
    if ((tbl == RENAMETBL) || (tbl == CTRLRTBL)) {
      dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;
      dbop.inoutop = kOpInOutFlag | kOpInOutCs;
    }
    pdbop = &dbop;
  }
  result_code = BindAttr(dal_bind_info, ikey, op, dt_type, *pdbop, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    if (dal_bind_info) delete dal_bind_info;
    return result_code;
  }
  dt_type = (dt_type == UPLL_DT_STATE) ? UPLL_DT_RUNNING : dt_type;
  UPLL_LOG_TRACE("Dbop %s dt_type %d EXISTS  %d",
                 (ikey->ToStrAll()).c_str(), dt_type,
                  tbl_index);
  result_code = DalToUpllResCode(
      dmi->RecordExists(dt_type, tbl_index, dal_bind_info, &exists));
  if (result_code == UPLL_RC_SUCCESS) {
    if (exists)
      result_code = UPLL_RC_ERR_INSTANCE_EXISTS;
    else
      result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  delete dal_bind_info;

  return result_code;
}

upll_rc_t MoMgrImpl::UpdateConfigDB(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    unc_keytype_operation_t op,
                                    DalDmlIntf *dmi,
                                    TcConfigMode cfg_mode,
                                    string vtn_name,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UpdateConfigDB(ikey, dt_type, op, dmi, NULL,
                                         cfg_mode, vtn_name, tbl);

  return result_code;
}

upll_rc_t MoMgrImpl::UpdateConfigDB(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    unc_keytype_operation_t op,
                                    DalDmlIntf *dmi,
                                    DbSubOp *pdbop,
                                    TcConfigMode cfg_mode,
                                    string vtn_name,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  const uudst::kDalTableIndex tbl_index = GetTable(tbl, dt_type);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }

  DalBindInfo *dal_bind_info = new DalBindInfo(tbl_index);
  upll_rc_t result_code;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutFlag | kOpInOutCtrlr
                       | kOpInOutDomain };
  if (op == UNC_OP_READ) {
    UPLL_LOG_ERROR("Invalid operation - %d", op);
    if (dal_bind_info) delete dal_bind_info;
    return UPLL_RC_ERR_GENERIC;
  }

  if (pdbop == NULL) {
    if (op == UNC_OP_DELETE) dbop.inoutop = kOpInOutNone;
    if (op != UNC_OP_CREATE) {
      if ((tbl == RENAMETBL) || (tbl == CTRLRTBL)) {
        dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;
        dbop.inoutop = kOpInOutFlag | kOpInOutCs;
      }
      if (op == UNC_OP_UPDATE) {
        if  (dt_type == UPLL_DT_CANDIDATE) {
          dbop.inoutop = kOpInOutCs;
        } else if (dt_type == UPLL_DT_RUNNING) {
          dbop.inoutop |= kOpInOutCs;
        } else if (dt_type == UPLL_DT_AUDIT) {
          dbop.inoutop = kOpInOutFlag;
        }
      }
    } else {
      if (dt_type != UPLL_DT_CANDIDATE || tbl == CTRLRTBL)
        if (dt_type != UPLL_DT_AUDIT)
          dbop.inoutop |= kOpInOutCs;
    }
    pdbop = &dbop;
  }
  result_code = BindAttr(dal_bind_info, ikey, op, dt_type, *pdbop, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    if (dal_bind_info) delete dal_bind_info;
    return result_code;
  }
  dt_type = (dt_type == UPLL_DT_STATE) ? UPLL_DT_RUNNING : dt_type;
  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
      const_cast<char *>(vtn_name.c_str()));
  }
  switch (op) {
    case UNC_OP_CREATE:
      UPLL_LOG_TRACE("Dbop %s dt_type %d CREATE %d", (ikey->ToStrAll()).c_str(),
                     dt_type, tbl_index);
      result_code = DalToUpllResCode(
          dmi->CreateRecord(dt_type, tbl_index, dal_bind_info, cfg_mode,
                            vtnname));
      break;
    case UNC_OP_DELETE:
      UPLL_LOG_TRACE("Dbop %s dt_type %d DELETE  %d",
                     (ikey->ToStrAll()).c_str(), dt_type,
                     tbl_index);
      result_code = DalToUpllResCode(
          dmi->DeleteRecords(dt_type, tbl_index, dal_bind_info, false,
                             cfg_mode, vtnname));
      break;
    case UNC_OP_UPDATE:
      UPLL_LOG_TRACE("Dbop %s dt_type %d UPD  %d",
                     (ikey->ToStrAll()).c_str(), dt_type,
                     tbl_index);
      result_code = DalToUpllResCode(
          dmi->UpdateRecords(dt_type, tbl_index, dal_bind_info, cfg_mode,
                             vtnname));
      break;
    default:
      break;
  }
  delete dal_bind_info;

  return result_code;
}

upll_rc_t MoMgrImpl::BindAttr(DalBindInfo *db_info,
                              ConfigKeyVal *&req,
                              unc_keytype_operation_t op,
                              upll_keytype_datatype_t dt_type,
                              DbSubOp dbop,
                              MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *ck_val = (req) ? req->get_cfg_val() : NULL;
  void *tkey = (req) ? req->get_key() : NULL;
  void *tval = NULL, *sval = NULL;
  void *p = NULL;
  BindInfo *binfo;
  int nattr;
  uint8_t *valid = NULL, *valid_st = NULL;
  key_user_data_t *tuser_data = NULL;

  if ((req == NULL) || (tkey == NULL)) {
    UPLL_LOG_DEBUG("NULL input parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("Valid Falg is true ConfigKeyVal %s",
                 (req->ToStr()).c_str());
  if (!GetBindInfo(tbl, dt_type, binfo, nattr)) return UPLL_RC_ERR_GENERIC;
  tuser_data = reinterpret_cast<key_user_data_t *>(req->get_user_data());
  switch (op) {
    case UNC_OP_READ:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING_COUNT:
      if (dbop.readop & ~(kOpReadExist | kOpReadCount)) {
        if (ck_val == NULL) {
          AllocVal(ck_val, dt_type, tbl);
          if (!ck_val) return UPLL_RC_ERR_GENERIC;
          req->AppendCfgVal(ck_val);
        } else if ((dt_type == UPLL_DT_STATE) &&
                   (ck_val->get_next_cfg_val() == NULL) && tbl != RENAMETBL) {
          ConfigVal *ck_val1 = NULL;
          AllocVal(ck_val1, dt_type, tbl);
          const pfc_ipcstdef_t *st_def = IpctSt::GetIpcStdef(
                                           ck_val->get_st_num());
          if (st_def) {
            memcpy(ck_val1->get_val(), ck_val->get_val(), st_def->ist_size);
            req->SetCfgVal(ck_val1);
            ck_val = ck_val1;
          } else {
            delete ck_val1;
          }
        }
      }
      if ((!tuser_data) && (dbop.readop & ~kOpReadCount)
          && ((dbop.inoutop & (kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain))
              || (dbop.readop & kOpReadDiff))) {
        GET_USER_DATA(req);
        tuser_data = reinterpret_cast<key_user_data_t *>(req->get_user_data());
      }
      /* fall through */
    case UNC_OP_UPDATE:
      if (dbop.matchop & kOpMatchCtrlr) {
        uint8_t *ctrlr = NULL;
        GET_USER_DATA_CTRLR(req, ctrlr);
        if (!ctrlr) {
          UPLL_LOG_DEBUG("Invalid Controller");
          return UPLL_RC_ERR_GENERIC;
        }
      } else if (dbop.matchop & kOpMatchDomain) {
        uint8_t *dom = NULL;
        GET_USER_DATA_DOMAIN(req, dom);
        if (!dom) {
          UPLL_LOG_DEBUG("Invalid Domain");
          return UPLL_RC_ERR_GENERIC;
        }
      }
      /* fall through intended */
    case UNC_OP_CREATE:
      if (ck_val) {
        tval = ck_val->get_val();
      }
      if (dt_type == UPLL_DT_STATE) {
        ConfigVal *nval = (ck_val)?ck_val->get_next_cfg_val():NULL;
        sval = (nval) ? nval->get_val() : NULL;
        if (nval && (sval == NULL)) return UPLL_RC_ERR_GENERIC;
      }
      break;
    default:
      break;
  }
  for (int i = 0; i < nattr; i++) {
    uint64_t indx = binfo[i].index;
    BindStructTypes attr_type = binfo[i].struct_type;
    UPLL_LOG_TRACE(" the attr_type %x number %d", binfo[i].struct_type, i);
    if (attr_type == CFG_KEY) {
      p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
          + binfo[i].offset);
      UPLL_LOG_TRACE(" key struct %d tkey %p p %p", attr_type, tkey, p);
      switch (op) {
        case UNC_OP_CREATE:
          UPLL_LOG_TRACE(" Bind input Key %"PFC_PFMT_u64" p %p", indx, p);
          db_info->BindInput(indx, binfo[i].app_data_type, binfo[i].array_size,
                             p);
          break;
        case UNC_OP_UPDATE:
          if (IsValidKey(tkey, indx, tbl)) {
            UPLL_LOG_TRACE("tkey %p bind match UPD p %p", tkey, p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          }
          break;
        case UNC_OP_DELETE:
          if (IsValidKey(tkey, indx, tbl)) {
            UPLL_LOG_TRACE("tkey %p bind match DEL p %p", tkey, p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
              binfo[i].array_size, p);
          }
          break;
        case UNC_OP_READ:
        case UNC_OP_READ_SIBLING:
        case UNC_OP_READ_SIBLING_BEGIN:
        case UNC_OP_READ_SIBLING_COUNT:
          if ((dbop.readop & kOpReadSingle) ||
              (dbop.readop & kOpReadExist) ||
              (dbop.readop & kOpReadMultiple) ||
              (dbop.readop & kOpReadCount)) {
            if (IsValidKey(tkey, indx, tbl)) {
              UPLL_LOG_TRACE("tkey %p bind match READ p %p", tkey, p);
              db_info->BindMatch(indx, binfo[i].app_data_type,
                                 binfo[i].array_size, p);
              if (dbop.readop & kOpReadMultiple) {
                UPLL_LOG_TRACE("tkey %p bind output READ p %p", tkey, p);
                db_info->BindOutput(indx, binfo[i].app_data_type,
                                 binfo[i].array_size, p);
              }
            } else {
              UPLL_LOG_TRACE("tkey %p bind output READ p %p", tkey, p);
              db_info->BindOutput(indx, binfo[i].app_data_type,
                                  binfo[i].array_size, p);
            }
          } else if (dbop.readop & kOpReadDiff) {
            UPLL_LOG_TRACE("tkey %p DIFF match/output p %p", tkey, p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
            db_info->BindOutput(indx, binfo[i].app_data_type,
                                binfo[i].array_size, p);
          }
          break;
        default:
          break;
      }
    } else if (tuser_data
        && ((attr_type == CK_VAL) || (attr_type == CK_VAL2))) {
      if (attr_type == CK_VAL2) {
        if (req->get_cfg_val()) {
          GET_USER_DATA(req->get_cfg_val());
          tuser_data =
                 reinterpret_cast<key_user_data *>
                    (req->get_cfg_val()->get_user_data());
        } else {
          tuser_data = NULL;
        }
      } else {
        tuser_data = reinterpret_cast<key_user_data_t *>(req->get_user_data());
      }
      if (!tuser_data) {
        UPLL_LOG_DEBUG("null tuser_data");
        continue;
      }
      p = reinterpret_cast<void *>(reinterpret_cast<char *>(tuser_data)
          + binfo[i].offset);
      bool par_flag = false, par_ctrlr = false, par_dom = false;
      if (binfo[i].offset == offsetof(key_user_data_t, flags))
        par_flag = true;
      else if (binfo[i].offset == offsetof(key_user_data_t, ctrlr_id))
        par_ctrlr = true;
      else if (binfo[i].offset == offsetof(key_user_data_t, domain_id))
        par_dom = true;
      switch (op) {
        case UNC_OP_CREATE:
          if ((par_ctrlr && (dbop.inoutop & kOpInOutCtrlr)) ||
              (par_dom && (dbop.inoutop & kOpInOutDomain))  ||
              (par_flag && (dbop.inoutop & kOpInOutFlag))) {
            UPLL_LOG_TRACE("CR bind input Cntrlr/Domain/Flag %p", p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          }
          break;
        case UNC_OP_UPDATE:
          if ((par_ctrlr && (dbop.matchop & kOpMatchCtrlr)) ||
              (par_dom && (dbop.matchop & kOpMatchDomain))  ||
              (par_flag && (dbop.matchop & kOpMatchFlag))) {
            UPLL_LOG_TRACE("UPD bind match flag/Cntrlr %p ", p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          } else if ((par_ctrlr && (dbop.inoutop & kOpInOutCtrlr))
              || (par_dom && (dbop.inoutop & kOpInOutDomain))
              || (par_flag && (dbop.inoutop & kOpInOutFlag))) {
            UPLL_LOG_TRACE("UPD bind input flag/Cntrlr/domain %p ", p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          }
          break;
        case UNC_OP_READ:
        case UNC_OP_READ_SIBLING:
        case UNC_OP_READ_SIBLING_BEGIN:
        case UNC_OP_READ_SIBLING_COUNT:
          if ((par_ctrlr && (dbop.inoutop & kOpInOutCtrlr)) ||
              (par_dom && (dbop.inoutop & kOpInOutDomain)) ||
              (par_flag && (dbop.inoutop & kOpInOutFlag))) {
            UPLL_LOG_TRACE("RD bind output flag/Cntrlr/domain %p", p);
            db_info->BindOutput(indx, binfo[i].app_data_type,
                                binfo[i].array_size, p);
          }
          /* fall through intended */
        case UNC_OP_DELETE:
          if ((par_ctrlr && (dbop.matchop & kOpMatchCtrlr)) ||
              (par_dom && (dbop.matchop & kOpMatchDomain)) ||
              (par_flag && (dbop.matchop & kOpMatchFlag))) {
            UPLL_LOG_TRACE("RD/DEL bind match flag/Cntrlr/domain %p", p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          }
          break;
        default:
          // do nothing
          break;
      }

    } else if (tval && (attr_type != ST_VAL) && (attr_type != ST_META_VAL)) {
#if 1
      if (attr_type == CFG_DEF_VAL) {
         attr_type = (dbop.readop & kOpReadDiffUpd)?attr_type:CFG_META_VAL;
         UPLL_LOG_DEBUG("ATTR: attr_type %d readop %d op %d\n",
                        attr_type, dbop.readop, op);
      }
#endif
      if (op == UNC_OP_DELETE) continue;
      if (dt_type == UPLL_DT_STATE) {
#if 0
        attr_type = (attr_type == CFG_ST_VAL)?CFG_VAL:
                    ((attr_type == CFG_ST_META_VAL)?CFG_META_VAL:attr_type);
#else
        // bind down count only for output and not for match
        if (attr_type == CFG_ST_VAL) {
          attr_type = (dbop.readop & kOpReadDiffUpd)?CFG_DEF_VAL:CFG_VAL;
        } else if (attr_type == CFG_ST_META_VAL) {
          attr_type = (dbop.readop & kOpReadDiffUpd)?CFG_DEF_VAL:CFG_META_VAL;
        }
#endif
      } else if ((attr_type == CFG_ST_VAL) || (attr_type == CFG_ST_META_VAL)) {
        continue;
      }
      p = reinterpret_cast<void *>(reinterpret_cast<char *>(tval)
          + binfo[i].offset);
      bool valid_is_defined = false;
      if (attr_type == CFG_VAL) {
        result_code = GetValid(tval, indx, valid, dt_type, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_TRACE("Returning %d", result_code);
           return result_code;
        }
        if (!valid) {
          UPLL_LOG_TRACE(" Invalid for attr %d", i);
           switch (op) {
           case UNC_OP_CREATE:
           case UNC_OP_UPDATE:
             valid_is_defined = true;
              break;
           default:
             valid_is_defined = false;
           }
        } else if ((*valid == UNC_VF_VALID) ||
                   (*valid == UNC_VF_VALID_NO_VALUE)) {
          valid_is_defined = true;
        }
      } else if (attr_type == CFG_META_VAL) {
        if ((*(reinterpret_cast<uint8_t *>(p)) == UNC_VF_VALID)
            || (*(reinterpret_cast<uint8_t *>(p)) == UNC_VF_VALID_NO_VALUE))
          valid_is_defined = true;
      }
      switch (op) {
        case UNC_OP_CREATE:
#if 0
          if ((attr_type == CFG_META_VAL) || valid_is_defined
#else
          if ((attr_type == CFG_META_VAL) || (attr_type == CFG_VAL)
#endif
              || ((attr_type == CS_VAL) && (dbop.inoutop & kOpInOutCs))) {
            UPLL_LOG_TRACE("tval/meta CR bind input %p p %p", tval, p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size,
                               reinterpret_cast<void *>(p));
          }
          break;
        case UNC_OP_UPDATE:
#if 0
          if ((attr_type == CFG_META_VAL)
              || ((attr_type == CS_VAL) && (dbop.matchop & kOpMatchCs))) {
            UPLL_LOG_TRACE("tval/meta UP bind match %p p %p", tval, p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size,
                               reinterpret_cast<void *>(p));
          }
#endif
          if (valid_is_defined ||
             ((attr_type == CS_VAL) && (dbop.inoutop & kOpInOutCs))) {
            UPLL_LOG_TRACE("tval/meta UP bind input %p p %p", tval, p);
            // store VALID_NO_VALUE flag in candidate as INVALID
            if ((attr_type == CFG_META_VAL) &&
                (*(reinterpret_cast<uint8_t *>(p)) == UNC_VF_VALID_NO_VALUE)) {
              UPLL_LOG_TRACE("Resetting VALID_NO_VALUE to INVALID %p", p);
             *(reinterpret_cast<uint8_t *>(p)) = UNC_VF_INVALID;
            }
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size,
                               reinterpret_cast<void *>(p));
          }
          break;
        case UNC_OP_READ:
        case UNC_OP_READ_SIBLING:
        case UNC_OP_READ_SIBLING_BEGIN:
        case UNC_OP_READ_SIBLING_COUNT:
          if (dbop.readop & ~(kOpReadDiff | kOpReadExist | kOpReadDiffUpd)) {
            if (valid_is_defined) {
              UPLL_LOG_TRACE("tval RD bind match %p p %p", tval, p);
              db_info->BindMatch(indx, binfo[i].app_data_type,
                                 binfo[i].array_size,
                                 reinterpret_cast<void *>(p));
            } else if ((dbop.readop & (kOpReadExist | kOpReadCount)) == 0) {
              switch (attr_type) {
                case CS_VAL:
                  if (dbop.inoutop & kOpInOutCs) {
                    UPLL_LOG_TRACE("tvalcs RD bind output %p p %p", tval, p);
                    db_info->BindOutput(indx, binfo[i].app_data_type,
                                        binfo[i].array_size,
                                        reinterpret_cast<void *>(p));
                  }
                  break;
                case CFG_VAL:
                case CFG_META_VAL:
                  UPLL_LOG_TRACE("tval RD bind output %p p %p", tval, p);
                  db_info->BindOutput(indx, binfo[i].app_data_type,
                                      binfo[i].array_size,
                                      reinterpret_cast<void *>(p));
                default:
                  break;
              }
            }
          } else if (dbop.readop & kOpReadDiff) {
            if ((attr_type == CFG_META_VAL) || (attr_type == CFG_VAL) ||
#if 1
                (attr_type == CFG_DEF_VAL) ||
#endif
                ((attr_type == CS_VAL) && (dbop.inoutop & kOpInOutCs))) {
              UPLL_LOG_TRACE("tval %d RDDiff bind output %p p %p", attr_type,
                             tval, p);
              db_info->BindOutput(indx, binfo[i].app_data_type,
                                  binfo[i].array_size,
                                  reinterpret_cast<void *>(p));
            }
#if 1
            if ((attr_type == CFG_META_VAL) || (attr_type == CFG_VAL) ||
#else
            if ((attr_type == CFG_VAL) ||
#endif
                ((attr_type == CS_VAL) && (dbop.matchop & kOpMatchCs))) {
#if 1
              if ((dbop.readop & kOpReadDiffUpd) &&
                  (attr_type != CFG_DEF_VAL)) {
#else
              if (dbop.readop & kOpReadDiffUpd)
#endif
                UPLL_LOG_TRACE("tval %d RDDiff bind match %p p %p", attr_type,
                             tval, p);
                db_info->BindMatch(indx, binfo[i].app_data_type,
                                   binfo[i].array_size,
                                   reinterpret_cast<void *>(p));
              }
            }
          }
        default:
          break;
      }
    } else if (sval) {
      if (op == UNC_OP_DELETE) continue;
      bool valid_is_defined = false;
      p = reinterpret_cast<void *>(reinterpret_cast<char *>(sval)
          + binfo[i].offset);
#if 0
      if (attr_type == CFG_ST_VAL) {
        uint32_t val_p =  *(reinterpret_cast<uint32_t *>(p));
        attr_type = (op == UNC_OP_UPDATE)?
                    ((val_p != INVALID_MATCH_VALUE)?ST_VAL:attr_type):ST_VAL;
      }
#endif
      if (attr_type == ST_VAL) {
        result_code = GetValid(sval, indx, valid_st, dt_type, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("Returning %d", result_code);
          return result_code;
        }
        if (!valid_st) {
          switch (op) {
          case UNC_OP_CREATE:
          case UNC_OP_UPDATE:
            valid_is_defined = true;
            break;
          default:
            valid_is_defined = false;
          }
        } else if (
          (*(reinterpret_cast<uint8_t *>(valid_st)) == UNC_VF_VALID) ||
          (*(reinterpret_cast<uint8_t *>(valid_st)) == UNC_VF_VALID_NO_VALUE)) {
          valid_is_defined = true;
          UPLL_LOG_TRACE(" The ST_VAL valid flag is %d", valid_is_defined);
        }
      } else if (attr_type == ST_META_VAL) {
        if ((*(reinterpret_cast<uint8_t *>(p)) == UNC_VF_VALID)
            || (*(reinterpret_cast<uint8_t *>(p)) == UNC_VF_VALID_NO_VALUE))
          valid_is_defined = true;
      }
      switch (op) {
        case UNC_OP_CREATE:
          if ((attr_type == ST_META_VAL) || valid_is_defined) {
            UPLL_LOG_TRACE("sval CR/UPD bind input %p p %p", sval, p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size,
                               reinterpret_cast<void *>(p));
          }
          break;
        case UNC_OP_UPDATE:
#if 0
          if (attr_type == ST_META_VAL) {
            UPLL_LOG_TRACE("sval/meta UP bind match %p p %p", sval, p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size,
                               reinterpret_cast<void *>(p));
          }
#endif
          if (valid_is_defined) {
            UPLL_LOG_TRACE("sval/meta UP bind input %p p %p", sval, p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size,
                               reinterpret_cast<void *>(p));
          }
          break;
        case UNC_OP_READ:
        case UNC_OP_READ_SIBLING:
        case UNC_OP_READ_SIBLING_BEGIN:
        case UNC_OP_READ_SIBLING_COUNT:
          if (dbop.readop & ~(kOpReadDiff | kOpReadDiffUpd | kOpReadExist)) {
            if (valid_is_defined) {
              UPLL_LOG_TRACE("sval RD bind match %p p %p", sval, p);
              db_info->BindMatch(indx, binfo[i].app_data_type,
                                 binfo[i].array_size,
                                 reinterpret_cast<void *>(p));
            } else if ((dbop.readop & (kOpReadExist | kOpReadCount)) == 0) {
              UPLL_LOG_TRACE("sval RD bind output %p p %p", sval, p);
              db_info->BindOutput(indx, binfo[i].app_data_type,
                                  binfo[i].array_size,
                                  reinterpret_cast<void *>(p));
            }
          } else if (dbop.readop & kOpReadDiff) {
            if ((attr_type == ST_META_VAL) || (attr_type == ST_VAL)) {
              UPLL_LOG_TRACE("sval %d RDDiff bind output %p p %p", attr_type,
                             sval, p);
              db_info->BindOutput(indx, binfo[i].app_data_type,
                                  binfo[i].array_size,
                                  reinterpret_cast<void *>(p));
#if 0
              if (dbop.readop & kOpReadDiffUpd) {
                UPLL_LOG_TRACE("sval %d RDDiff bind match %p p %p", attr_type,
                             sval, p);
                db_info->BindMatch(indx, binfo[i].app_data_type,
                                   binfo[i].array_size,
                                   reinterpret_cast<void *>(p));
              }
#endif
            }
          }
        default:
          break;
      }
    }
  }
  return result_code;
}

upll_rc_t MoMgrImpl::UpdateRenameKey(ConfigKeyVal *&ikey,
                                     upll_keytype_datatype_t dt_type,
                                     unc_keytype_operation_t op,
                                     DalDmlIntf *dmi, DbSubOp *pdbop,
                                     MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  const uudst::kDalTableIndex tbl_index = GetTable(tbl, dt_type);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
//  cout << tbl_index << "\n";
  bool exists = false;
  DalBindInfo *dal_bind_info = new DalBindInfo(tbl_index);
  upll_rc_t result_code;
  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutFlag | kOpInOutCtrlr
                       | kOpInOutDomain };
  if (pdbop == NULL) {
    if (op == UNC_OP_DELETE)
      dbop.inoutop = kOpInOutNone;
    if ((tbl == RENAMETBL) || (tbl == CTRLRTBL)) {
      dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;
      dbop.inoutop = kOpInOutFlag;
    }
    pdbop = &dbop;
  }
    result_code = BindAttrRename(dal_bind_info, ikey, op, dt_type, *pdbop, tbl);
    switch (op) {
      case UNC_OP_CREATE:
        result_code = DalToUpllResCode(
      dmi->CreateRecord(dt_type, tbl_index, dal_bind_info,
                        TC_CONFIG_GLOBAL, NULL));
        break;
      case UNC_OP_UPDATE:
        result_code = DalToUpllResCode(
      dmi->UpdateRecords(dt_type, tbl_index, dal_bind_info,
                         TC_CONFIG_GLOBAL, NULL));
        break;
      case UNC_OP_READ:
        result_code = DalToUpllResCode(
      dmi->RecordExists(dt_type, tbl_index, dal_bind_info, &exists));
        if (result_code == UPLL_RC_SUCCESS) {
    if (exists)
        result_code = UPLL_RC_ERR_INSTANCE_EXISTS;
    else
        result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
        }
        break;
      default:
        break;
    }
    delete dal_bind_info;

    return result_code;
  }

  upll_rc_t MoMgrImpl::BindAttrRename(DalBindInfo *db_info,
                                    ConfigKeyVal *&req,
                                    unc_keytype_operation_t op,
                                    upll_keytype_datatype_t dt_type,
                                    DbSubOp dbop,
                                    MoMgrTables tbl) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  void *tkey = (req) ? req->get_key() : NULL;
  void *p = NULL;
  BindInfo *binfo = NULL;
  int nattr;
  uint64_t indx;
  key_user_data_t *tuser_data;

  if ((req == NULL) || (tkey == NULL)) {
    UPLL_LOG_DEBUG("Input is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  if (!GetRenameKeyBindInfo(req->get_key_type(), binfo, nattr, tbl)) {
     UPLL_LOG_DEBUG("GetRenameKeyBindInfo Not available for the keytype %d"
                    "For the Table %d", req->get_key_type(), tbl);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("The nAttribute %d", nattr);
  tuser_data = reinterpret_cast<key_user_data_t *>(req->get_user_data());
  for (int i = 0; i < nattr; i++) {
    UPLL_LOG_TRACE("The If condition value is %d i=%d", (nattr/2), i);
    if (i == (nattr / 2)) {
      if (req->get_next_cfg_key_val()
          && (req->get_next_cfg_key_val())->get_key()) {
        tkey = (req->get_next_cfg_key_val())->get_key();
        DumpRenameInfo(req->get_next_cfg_key_val());
      }
    }
    indx = binfo[i].index;
    BindStructTypes attr_type = binfo[i].struct_type;
    UPLL_LOG_TRACE("the attr_type %d attr number %d", binfo[i].struct_type,
                   i);
    p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
        + binfo[i].offset);
    UPLL_LOG_TRACE("key struct %d tkey %p p %p", attr_type, tkey, p);
    if (CFG_INPUT_KEY == attr_type || CFG_MATCH_KEY == attr_type) {
      switch (op) {
        case UNC_OP_CREATE:
#if 0
          if (!IsValidKey(tkey, indx))  {
            UPLL_LOG_TRACE("Given Key is Invalid %s",
                           (req->ToStrAll()).c_str());
            return UPLL_RC_ERR_GENERIC;
          }
#endif
          UPLL_LOG_TRACE(" Bind input Key %"PFC_PFMT_u64" p %p", indx,
                       reinterpret_cast<char*>(p));
          db_info->BindInput(indx, binfo[i].app_data_type, binfo[i].array_size,
                           p);
          break;
        case UNC_OP_UPDATE:
          UPLL_LOG_TRACE("Validate the Key in Update");
//          if (IsValidKey(tkey, indx)) {
            switch (attr_type) {
              case CFG_INPUT_KEY:
                UPLL_LOG_TRACE("tkey %p bindinput %p", tkey,
                             reinterpret_cast<char*>(p));
                db_info->BindInput(indx, binfo[i].app_data_type,
                                 binfo[i].array_size, p);
                break;
              case CFG_MATCH_KEY:
                UPLL_LOG_TRACE("tkey %p bindmatch %p", tkey,
                               reinterpret_cast<char*>(p));
                db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
                break;
              default:
                break;
            }
          break;
        default:
          break;
      }
    }
    if (tuser_data && attr_type == CK_VAL) {
      p = reinterpret_cast<void *>(reinterpret_cast<char *>(tuser_data)
          + binfo[i].offset);
      switch (op) {
        case UNC_OP_CREATE:
          if ((dbop.inoutop & (kOpInOutCtrlr | kOpInOutDomain))) {
            UPLL_LOG_TRACE("CR bind input Cntrlr/Flag %p", p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          }
          break;
        case UNC_OP_UPDATE:
          if ((dbop.matchop & (kOpMatchCtrlr | kOpMatchDomain))
              || (dbop.matchop & kOpMatchFlag)) {
            UPLL_LOG_TRACE("UPD bind match Cntrlr/Flag %p", p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          } else if ((dbop.inoutop & (kOpInOutCtrlr | kOpInOutDomain))
              || (dbop.inoutop & kOpInOutFlag)) {
            UPLL_LOG_TRACE("UPD bind input Cntrlr/Flag %p", p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          }
          break;
        default:
          break;
      }
    }
  }
  return result_code;
}

upll_rc_t MoMgrImpl::BindStartup(DalBindInfo *db_info,
       upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  int nattr;
  BindInfo *binfo;
  unc_keytype_configstatus_t cs_val = UNC_CS_NOT_APPLIED;
  if (!GetBindInfo(tbl, dt_type, binfo, nattr))
    return UPLL_RC_ERR_GENERIC;
  for (int i = 0; i < nattr; i++) {
    uint64_t indx = binfo[i].index;
    BindStructTypes attr_type = binfo[i].struct_type;
    if (attr_type == CS_VAL) {
      db_info->BindInput(indx, binfo[i].app_data_type,
                         binfo[i].array_size, &cs_val);
    }
  }
  return UPLL_RC_SUCCESS;
}

// Binding Dummy pointers for matching
// This is currently specific for CheckRecordsIdentical API.
upll_rc_t MoMgrImpl::BindKeyAndVal(DalBindInfo *db_info,
                                   upll_keytype_datatype_t dt_type,
                                   MoMgrTables tbl,
                                   const uudst::kDalTableIndex index) {
  UPLL_FUNC_TRACE;
  int nattr;
  BindInfo *binfo;

  if (!GetBindInfo(tbl, dt_type, binfo, nattr)) {
    return UPLL_RC_ERR_GENERIC;
  }

  for (int i = 0; i < nattr; i++) {
    uint64_t indx = binfo[i].index;
    BindStructTypes attr_type = binfo[i].struct_type;
    size_t size;

    switch (binfo[i].app_data_type) {
      case uud::kDalChar:
        size = sizeof(int8_t);
        break;
      case uud::kDalUint8:
        size = sizeof(uint8_t);
        break;
      case uud::kDalUint16:
        size = sizeof(uint16_t);
        break;
      case uud::kDalUint32:
        size = sizeof(uint32_t);
        break;
      case uud::kDalUint64:
        size = sizeof(uint64_t);
        break;
      default:
        UPLL_LOG_ERROR("Invalid DalCDataType");
        return UPLL_RC_ERR_GENERIC;
    }

    void *dummy = ConfigKeyVal::Malloc(size * binfo[i].array_size);

    if (MAINTBL == tbl && (table[MAINTBL]->get_key_type() == UNC_KT_VBRIDGE ||
        table[MAINTBL]->get_key_type() == UNC_KT_VROUTER ||
        table[MAINTBL]->get_key_type() == UNC_KT_VTERMINAL ||
        table[MAINTBL]->get_key_type() == UNC_KT_VBR_PORTMAP)) {
      if (attr_type != CS_VAL && attr_type != ST_VAL &&
          attr_type != CFG_DEF_VAL &&  // to ignore valid_admin_status
          attr_type != ST_META_VAL &&
          attr_type != CFG_ST_META_VAL &&
          attr_type != CK_VAL &&
          attr_type != CFG_ST_VAL) {
        UPLL_LOG_TRACE("Bind for attr type %d", attr_type);
        db_info->BindMatch(indx, binfo[i].app_data_type,
                           binfo[i].array_size, dummy);
      }
    } else {
      if (attr_type != CS_VAL && attr_type != ST_VAL &&
          attr_type != CFG_DEF_VAL &&  // to ignore valid_admin_status
          attr_type != ST_META_VAL &&
          attr_type != CFG_ST_META_VAL &&
          attr_type != CFG_ST_VAL
          && ((attr_type == CFG_KEY) ||
              (attr_type == CFG_VAL) ||
              (CK_VAL == attr_type)||
              (CK_VAL2 == attr_type) ||
              (CFG_META_VAL == attr_type))) {
        UPLL_LOG_TRACE("Bind for attr type %d", attr_type);
        db_info->BindMatch(indx, binfo[i].app_data_type,
                           binfo[i].array_size, dummy);
      }
    }
    FREE_IF_NOT_NULL(dummy);
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::TxCopyRenameTableFromCandidateToRunning(
                                    unc_key_type_t key_type,
                                    unc_keytype_operation_t op,
                                    DalDmlIntf* dmi,
                                    TcConfigMode config_mode,
                                    std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *can_ckv = NULL, *run_ckv = NULL;
  DalCursor *cfg1_cursor = NULL;
  DalResultCode db_result = uud::kDalRcSuccess;
  unc_keytype_operation_t operation = op;

  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op,
        can_ckv, run_ckv, &cfg1_cursor, dmi, NULL, config_mode,
        vtn_name, RENAMETBL, true);
  while (result_code == UPLL_RC_SUCCESS) {
    db_result = dmi->GetNextRecord(cfg1_cursor);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS)
       break;
    /* VRT and VBR sharing the same table so need not use
     * VRT key type here */
    operation = op;
    switch (key_type) {
    case UNC_KT_VTN: {
      val_rename_vtn *ren_val = static_cast<val_rename_vtn *>(
                                            GetVal(can_ckv));
      ren_val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
      }
      break;
    case UNC_KT_VBRIDGE:
//    case UNC_KT_VTERMINAL:
    case UNC_KT_VLINK: {
      val_rename_vnode *ren_val = static_cast<val_rename_vnode *>(
                                            GetVal(can_ckv));
      ren_val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
      ren_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
      }
      break;
    case UNC_KT_POLICING_PROFILE: {
      val_rename_policingprofile_t *ren_val = static_cast
        <val_rename_policingprofile_t *>(GetVal(can_ckv));
      ren_val->valid[UPLL_IDX_RENAME_PROFILE_RPP] = UNC_VF_VALID;
      }
      break;
    case UNC_KT_FLOWLIST: {
      val_rename_flowlist_t *ren_val = static_cast
        <val_rename_flowlist_t *>(GetVal(can_ckv));
      ren_val->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;
      }
      break;
    default:
      UPLL_LOG_DEBUG("No special operation for %u", key_type);
      break;
    }
    if (UNC_OP_UPDATE == op) {
      ConfigKeyVal *temp_ckv = NULL;
      result_code = GetChildConfigKey(temp_ckv, run_ckv);
      if (result_code != UPLL_RC_SUCCESS) {
        delete can_ckv;
        can_ckv = NULL;
        DELETE_IF_NOT_NULL(run_ckv);
        if (cfg1_cursor)
          dmi->CloseCursor(cfg1_cursor, true);
        UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
        return result_code;
      }
      UPLL_LOG_TRACE("The Running Configuration is %s",
                     temp_ckv->ToStr().c_str());
      DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr,
                       kOpInOutNone};
      if ((key_type != UNC_KT_FLOWLIST) &&
          (key_type != UNC_KT_POLICING_PROFILE)) {
        dbop.matchop |= kOpMatchDomain;
      }
      result_code = UpdateConfigDB(temp_ckv, UPLL_DT_RUNNING,
                                   UNC_OP_DELETE, dmi, &dbop,
                                   config_mode, vtn_name, RENAMETBL);
      delete temp_ckv;
      if (result_code != UPLL_RC_SUCCESS) {
        delete can_ckv;
        can_ckv = NULL;
        DELETE_IF_NOT_NULL(run_ckv);
        if (cfg1_cursor)
          dmi->CloseCursor(cfg1_cursor, true);
        UPLL_LOG_DEBUG("UpdateConfigKey failed %d", result_code);
        return result_code;
      }
      operation = UNC_OP_CREATE;
    }
    // Copy Rename Table Info into Running
    result_code = UpdateConfigDB(can_ckv, UPLL_DT_RUNNING,
                                 operation, dmi,
                                 config_mode, vtn_name, RENAMETBL);
    if (result_code != UPLL_RC_SUCCESS) {
      delete can_ckv;
      can_ckv = NULL;
      DELETE_IF_NOT_NULL(run_ckv);
      if (cfg1_cursor)
        dmi->CloseCursor(cfg1_cursor, true);
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      return result_code;
    }
  }
  if (cfg1_cursor)
    dmi->CloseCursor(cfg1_cursor, true);
  DELETE_IF_NOT_NULL(can_ckv);
  DELETE_IF_NOT_NULL(run_ckv);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
    UPLL_RC_SUCCESS : result_code;
  return result_code;
}

#if 0
template<typename T1, typename T2>
upll_rc_t MoMgrImpl::GetCkvWithOperSt(ConfigKeyVal *&ck_vn,
                                  unc_key_type_t ktype,
                                  DalDmlIntf     *dmi) {
  if (ck_vn != NULL) return UPLL_RC_ERR_GENERIC;
  ConfigVal *cval = NULL;
  MoMgrImpl *mgr = NULL;
  upll_rc_t result_code = AllocVal(cval, UPLL_DT_STATE, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }

  /* initialize vnode st */
  T2 * vnode_st = reinterpret_cast<T2 *>
                               (cval->get_next_cfg_val()->get_val());
  if (!vnode_st) {
    delete cval;
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  T1 *vn_st = reinterpret_cast<T1 *>(vnode_st);
  vn_st->valid[UPLL_IDX_OPER_STATUS_VBRIS] = UNC_VF_VALID;
  vn_st->oper_status = UPLL_OPER_STATUS_UNINIT;

  /* Create Vnode If child */
  switch (ktype) {
  case UNC_KT_VTN:
  case UNC_KT_VLINK:
    mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(ktype)));
    break;
  case UNC_KT_VBRIDGE:
    mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VBR_IF)));
    break;
  case UNC_KT_VROUTER:
    mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VRT_IF)));
    break;
  case UNC_KT_VTEP:
    mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VTEP_IF)));
    break;
  case UNC_KT_VTUNNEL:
    mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                            (GetMoManager(UNC_KT_VTUNNEL_IF)));
    break;
  default:
    UPLL_LOG_DEBUG("Unsupported operation on keytype %d", ktype);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(ck_vn, NULL);
  if (UPLL_RC_SUCCESS != result_code || ck_vn == NULL)  {
    delete cval;
    if (ck_vn) delete ck_vn;
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  ck_vn->AppendCfgVal(cval);

  /* Reading the Vnode Table and Check the Operstatus is unknown
   * for any one of the vnode if */
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag |
                           kOpInOutCtrlr | kOpInOutDomain };
  result_code = mgr->ReadConfigDB(ck_vn, UPLL_DT_STATE, UNC_OP_READ,
                                  dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
     result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                 UPLL_RC_SUCCESS : result_code;
     UPLL_LOG_DEBUG("Returning %d", result_code);
     if (ck_vn) delete ck_vn;
     return result_code;
  }
  return UPLL_RC_SUCCESS;
}

template upll_rc_t
MoMgrImpl::GetCkvWithOperSt<val_vlink_st_t, val_db_vlink_st_t>
(ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi);
template upll_rc_t
MoMgrImpl::GetCkvWithOperSt<val_vbr_if_st_t, val_db_vbr_if_st_t>
(ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi);
template upll_rc_t
MoMgrImpl::GetCkvWithOperSt<val_vrt_if_st_t, val_db_vrt_if_st_t>
(ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi);
template upll_rc_t
MoMgrImpl::GetCkvWithOperSt<val_vtunnel_if_st_t, val_db_vtunnel_if_st_t>
(ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi);
template upll_rc_t
MoMgrImpl::GetCkvWithOperSt<val_vtep_if_st_t, val_db_vtep_if_st_t>
(ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi);
template upll_rc_t
MoMgrImpl::GetCkvWithOperSt<val_vtn_st_t, val_db_vtn_st_t>
(ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi);
#endif

upll_rc_t MoMgrImpl::GetUninitOperState(ConfigKeyVal *&ck_vn,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetCkvUninit(ck_vn, NULL, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag |
                           kOpInOutCtrlr | kOpInOutDomain };
  result_code = ReadConfigDB(ck_vn, UPLL_DT_STATE, UNC_OP_READ,
                                  dbop, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
     result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                 UPLL_RC_SUCCESS : result_code;
     UPLL_LOG_DEBUG("Returning %d", result_code);
     delete ck_vn;
     ck_vn = NULL;
  }
  return result_code;
}

upll_rc_t MoMgrImpl::GetCkvUninit(ConfigKeyVal *&ck_vn,
                                  ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi,
                                  val_oper_status oper_status,
                                  MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (ck_vn == NULL) {
    ConfigVal *cval = NULL;
    /* Create ckv of corresponding keytype */
    if (tbl == CONVERTTBL) {
      result_code = GetChildConvertConfigKey(ck_vn, ikey);
    } else {
      result_code = GetChildConfigKey(ck_vn, ikey);
    }
    if (UPLL_RC_SUCCESS != result_code)  {
      UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
      return result_code;
    }
    /* Allocate Memory for vnode st */
    result_code = AllocVal(cval, UPLL_DT_STATE, tbl);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      DELETE_IF_NOT_NULL(ck_vn);
      return result_code;
    }
    ck_vn->AppendCfgVal(cval);
  }
    /* initialize vnode st */
  void *vnif = GetStateVal(ck_vn);
  if (!vnif) {
    UPLL_LOG_DEBUG("Invalid param\n");
    DELETE_IF_NOT_NULL(ck_vn);
    return UPLL_RC_ERR_GENERIC;
  }
  switch (ck_vn->get_key_type()) {
  case UNC_KT_VTN:
  {
    val_db_vtn_st *vtnst = reinterpret_cast<val_db_vtn_st *>(vnif);
    vtnst->vtn_val_st.valid[UPLL_IDX_OPER_STATUS_VBRIS] = UNC_VF_VALID;
    vtnst->vtn_val_st.oper_status = oper_status;
    vtnst->down_count = 0;
    vtnst->unknown_count = 0;
    break;
  }
  case UNC_KT_VBRIDGE:
  case UNC_KT_VROUTER:
  case UNC_KT_VTERMINAL:
  case UNC_KT_VTUNNEL:
  case UNC_KT_VTEP:
  case UNC_KT_VLINK:
  {
  /* cast generically as vbr as all  vnode st structures
   * are the same and form the first field in the db st structure.
   */
    val_db_vbr_st *vnodest = reinterpret_cast<val_db_vbr_st *>(vnif);
    vnodest->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRIS] = UNC_VF_VALID;
    vnodest->vbr_val_st.oper_status = oper_status;
    vnodest->down_count = 0;
    vnodest->unknown_count = 0;
    break;
  }
  case UNC_KT_VBR_IF:
  case UNC_KT_VRT_IF:
  case UNC_KT_VTERM_IF:
  case UNC_KT_VTEP_IF:
  case UNC_KT_VTUNNEL_IF:
  {
  /* cast generically as vbr_if as all  vnodeif st structures
   * are the same and form the first field in the db st structure.
   */
    val_db_vbr_if_st *vnifst = reinterpret_cast<val_db_vbr_if_st *>(vnif);
    vnifst->vbr_if_val_st.valid[UPLL_IDX_OPER_STATUS_VBRIS] = UNC_VF_VALID;
    vnifst->vbr_if_val_st.oper_status = oper_status;
    vnifst->down_count = 0;
    break;
  }
  default:
    UPLL_LOG_DEBUG("Unsupported keytype\n");
    DELETE_IF_NOT_NULL(ck_vn);
    return UPLL_RC_ERR_GENERIC;
  }

#if 0
  /* Reading the Vnode Table and Check the Operstatus is unknown
   * for any one of the vnode if */
  DbSubOp dbop = { kOpReadExist | kOpReadMultiple, kOpMatchNone, kOpInOutFlag |
                           kOpInOutCtrlr | kOpInOutDomain };
  if (PORT_MAPPED_KEYTYPE(ck_vn->get_key_type()))
    dbop.readop = kOpReadMultiple;
#endif
  return result_code;
}

upll_rc_t MoMgrImpl::BindImportDB(ConfigKeyVal *&ikey,
                                  DalBindInfo *&db_info,
                                  upll_keytype_datatype_t dt_type,
                                  MoMgrTables tbl ) {
  UPLL_FUNC_TRACE;
  int nattr = 0;
  BindInfo *binfo;
  ConfigVal *ck_val = NULL;
  key_user_data *tuser_data  = NULL;
  if (!ikey || !(ikey->get_key())) {
    UPLL_LOG_DEBUG("Input key is Empty");
    return UPLL_RC_ERR_GENERIC;
  }
   /* Allocate memeory for key user data to fetch
    * controller, domain and rename flag */
  AllocVal(ck_val, dt_type, RENAMETBL);
  if (!ck_val) return UPLL_RC_ERR_GENERIC;
  ikey->SetCfgVal(ck_val);
  void *tval = ck_val->get_val();
  if (!tval) return UPLL_RC_ERR_GENERIC;
  GET_USER_DATA(ikey);
  tuser_data = reinterpret_cast<key_user_data_t *>(ikey->get_user_data());
  if (!tuser_data) {
    UPLL_LOG_DEBUG("Memory Allocation Failed");
    return UPLL_RC_ERR_GENERIC;
  }
  void *tkey = ikey->get_key();
  void *p = NULL;

  if (!GetBindInfo(tbl, dt_type, binfo, nattr))
    return UPLL_RC_ERR_GENERIC;

  for (int i = 0; i < nattr; i++) {
    uint64_t indx = binfo[i].index;
    BindStructTypes attr_type = binfo[i].struct_type;

    UPLL_LOG_TRACE("Attribute type is %d", attr_type);
    if (CFG_KEY == attr_type) {
      p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
          + binfo[i].offset);
       UPLL_LOG_TRACE("Attribute type is %d", attr_type);
       if (IsValidKey(tkey, indx, tbl)) {
         UPLL_LOG_TRACE("Key is valid ");
         db_info->BindMatch(indx, binfo[i].app_data_type,
                         binfo[i].array_size, p);
       }
    }
    if (CK_VAL == attr_type) {
      /* For Domain and controller output */
      UPLL_LOG_TRACE("Attribute type is %d", attr_type);
      p = reinterpret_cast<void *>(reinterpret_cast<char *>(tuser_data)
          + binfo[i].offset);
    }
    if (CFG_VAL == attr_type) {
      UPLL_LOG_TRACE("Attribute type is %d", attr_type);
      p = reinterpret_cast<void *>(reinterpret_cast<char *>(tval)
          + binfo[i].offset);
    }
    if (p)
      db_info->BindOutput(indx, binfo[i].app_data_type,
                          binfo[i].array_size, p);
    }
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::Getvalstnum(ConfigKeyVal *&ikey,
                                 uui::IpctSt::IpcStructNum &struct_num) {
  switch (ikey->get_key_type()) {
    case UNC_KT_FLOWLIST:
      struct_num = IpctSt::kIpcStValRenameFlowlist;
      break;
    case UNC_KT_POLICING_PROFILE:
      struct_num = IpctSt::kIpcStValRenamePolicingprofile;
      break;
    case UNC_KT_VTN:
      struct_num = IpctSt::kIpcStValRenameVtn;
      break;
    case UNC_KT_VBRIDGE:
      struct_num = IpctSt::kIpcStValRenameVbr;
      break;
    case UNC_KT_VTERMINAL:
      struct_num = IpctSt::kIpcStValRenameVterminal;
      break;
    case UNC_KT_VROUTER:
      struct_num = IpctSt::kIpcStValRenameVrt;
      break;
    case UNC_KT_VLINK:
      struct_num = IpctSt::kIpcStValRenameVlink;
      break;
    default:
      struct_num  = IpctSt::kIpcInvalidStNum;
      break;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::Swapvaltokey(ConfigKeyVal *&ikey,
                                  uint8_t rename_flag) {
  UPLL_FUNC_TRACE;
  void *rename_val = NULL;
  uint8_t temp_str[33];
  void *rename = NULL;
  const char *renamed_name = NULL;
  uui::IpctSt::IpcStructNum struct_num  = IpctSt::kIpcInvalidStNum;
  if (!ikey || !(ikey->get_key())) {
    return UPLL_RC_ERR_GENERIC;
  }
  Getvalstnum(ikey, struct_num);
  if (rename_flag != IMPORT_READ_FAILURE) {
    switch (ikey->get_key_type()) {
      case UNC_KT_FLOWLIST:
        {
         rename = reinterpret_cast<val_rename_flowlist_t *>(
                   ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
         rename_val = reinterpret_cast<val_rename_flowlist_t *>(GetVal(ikey));
         if (!rename_val) {
          UPLL_LOG_DEBUG("Val is Empty");
          free(rename);
          return UPLL_RC_ERR_GENERIC;
        }
        if (!rename_flag) {
          if (!strcmp((const char *)reinterpret_cast<key_flowlist_t *>
                      (ikey->get_key())->flowlist_name,
                      (const char *)reinterpret_cast<val_rename_vtn_t*>
                      (rename_val)->new_name))
          reinterpret_cast<val_rename_flowlist_t*>(rename)->
              valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_INVALID;
        } else {
          reinterpret_cast<val_rename_flowlist_t*>(rename)->
                                valid[UPLL_IDX_RENAME_TYPE_RFL] = UNC_VF_VALID;
          reinterpret_cast<val_rename_flowlist_t*>(rename)->
                            valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;
          renamed_name = (const char *)reinterpret_cast<key_flowlist_t *>
              (ikey->get_key())->flowlist_name;
          bool auto_rename = false;
          /* For global key type  auto rename stored as PFC as  a key and UNC as
           * a value .
           *  so need to compare with unc name with value.
           */
          for (std::map<std::string, std::string>::iterator
               it = auto_rename_.begin();
               it != auto_rename_.end(); it++) {
          if (strcmp(renamed_name, (const char *)((it->second).c_str())) == 0) {
              auto_rename = true;
              break;
            }
          }
          if (auto_rename) {
            reinterpret_cast<val_rename_flowlist_t*>(rename)->rename_type =
                UPLL_RENAME_TYPE_AUTO;
          } else {
            reinterpret_cast<val_rename_flowlist_t*>(rename)->rename_type =
                UPLL_RENAME_TYPE_MANUAL;
          }
        }
         /* copyt key to temp */
        uuu::upll_strncpy(temp_str, reinterpret_cast<key_flowlist_t*>
                           (ikey->get_key())->flowlist_name,
                           (kMaxLenFlowListName+1));
        /* Copy Controller name to key */
        uuu::upll_strncpy(reinterpret_cast<key_flowlist_t *>
                          (ikey->get_key())->flowlist_name,
                          reinterpret_cast<val_rename_flowlist_t* >
                          (rename_val)->flowlist_newname,
                         (kMaxLenFlowListName+1));
        /* Copy the UNC name to Val */
        uuu::upll_strncpy(reinterpret_cast<val_rename_flowlist_t*>
                           (rename)->flowlist_newname, temp_str,
                           (kMaxLenFlowListName+1));
        }
        break;
      case UNC_KT_POLICING_PROFILE:
        {
         rename = reinterpret_cast<val_rename_policingprofile_t *>(
                           ConfigKeyVal::Malloc(
                               sizeof(val_rename_policingprofile_t)));
         rename_val = reinterpret_cast<val_rename_policingprofile_t *>
             (GetVal(ikey));
         if (!rename_val) {
          UPLL_LOG_DEBUG("Val is Empty");
          free(rename);
          return UPLL_RC_ERR_GENERIC;
         }
        if (!rename_flag) {
          if (!strcmp(
                  (const char *)reinterpret_cast<key_policingprofile *>
                  (ikey->get_key())->policingprofile_name,
                  (const char *)reinterpret_cast<val_rename_vtn_t*>
                  (rename_val)->new_name))
          reinterpret_cast<val_rename_policingprofile_t*>(rename)->
              valid[UPLL_IDX_RENAME_PROFILE_RPP] = UNC_VF_INVALID;
        } else {
          reinterpret_cast<val_rename_policingprofile_t*>
              (rename)->valid[UPLL_IDX_RENAME_PROFILE_RPP] = UNC_VF_VALID;
          reinterpret_cast<val_rename_policingprofile_t*>
              (rename)->valid[UPLL_IDX_RENAME_TYPE_RPP] = UNC_VF_VALID;
          renamed_name = (const char *)reinterpret_cast<key_policingprofile *>
              (ikey->get_key())->policingprofile_name;
          bool auto_rename = false;
          /* For global key type  auto rename stored as PFC as  a key and UNC as
           * a value .
           *  so need to compare with unc name with value.
           */
          for (std::map<std::string, std::string>::iterator
               it = auto_rename_.begin();
               it != auto_rename_.end(); it++) {
            if (strcmp(renamed_name,
                       (const char *)((it->second).c_str())) == 0) {
              auto_rename = true;
              break;
            }
          }
          if (auto_rename) {
            reinterpret_cast<val_rename_policingprofile_t*>
                (rename)->rename_type = UPLL_RENAME_TYPE_AUTO;
          } else {
            reinterpret_cast<val_rename_policingprofile_t*>
                (rename)->rename_type = UPLL_RENAME_TYPE_MANUAL;
          }
        }
        /* copyt key to temp */
        uuu::upll_strncpy(temp_str, reinterpret_cast<key_policingprofile_t*>
                           (ikey->get_key())->policingprofile_name,
                           (kMaxLenPolicingProfileName+1));
        /* Copy Controller name to key */
        uuu::upll_strncpy(reinterpret_cast<key_policingprofile_t *>
                            (ikey->get_key())->policingprofile_name,
             reinterpret_cast<val_rename_policingprofile_t* >
                            (rename_val)->policingprofile_newname,
                            (kMaxLenPolicingProfileName+1));
        /* Copy the UNC name to Val */
        uuu::upll_strncpy(reinterpret_cast<val_rename_policingprofile_t*>
                           (rename)->policingprofile_newname, temp_str,
                           (kMaxLenPolicingProfileName+1));
        }

        break;
      case UNC_KT_VTN:
        {
        rename = reinterpret_cast<val_rename_vtn_t *>(
                           ConfigKeyVal::Malloc(sizeof(val_rename_vtn_t)));
        rename_val = reinterpret_cast<val_rename_vtn_t *>(GetVal(ikey));
        if (!rename_val) {
          UPLL_LOG_DEBUG("Val is Empty");
          free(rename);
          return UPLL_RC_ERR_GENERIC;
        }
        if (!rename_flag) {
          if (!strcmp(
                  (const char *)reinterpret_cast<key_vtn_t *>
                  (ikey->get_key())->vtn_name,
                  (const char *)reinterpret_cast<val_rename_vtn_t*>
                  (rename_val)->new_name))
          reinterpret_cast<val_rename_vtn_t*>(rename)->
              valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_INVALID;
        } else {
          reinterpret_cast<val_rename_vtn_t*>(rename)->
              valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
          reinterpret_cast<val_rename_vtn_t*>(rename)->
              valid[UPLL_IDX_RENAME_TYPE_RVTN] = UNC_VF_VALID;
          renamed_name = (const char *)reinterpret_cast<key_vtn_t *>
              (ikey->get_key())->vtn_name;
          bool auto_rename = false;
          /* For global key type  auto rename stored as PFC as  a key and UNC as
           * a value .
           *  so need to compare with unc name with value.
           */
          for (std::map<std::string, std::string>::iterator
               it = auto_rename_.begin();
               it != auto_rename_.end(); it++) {
            if (strcmp(renamed_name,
                       (const char *)((it->second).c_str())) == 0) {
              auto_rename = true;
              break;
            }
          }
          if (auto_rename) {
            reinterpret_cast<val_rename_vtn_t*>(rename)->rename_type =
                UPLL_RENAME_TYPE_AUTO;
          } else {
            reinterpret_cast<val_rename_vtn_t*>(rename)->rename_type =
                UPLL_RENAME_TYPE_MANUAL;
          }
        }
         /* copyt key to temp */
        uuu::upll_strncpy(temp_str, reinterpret_cast<key_vtn_t*>
                           (ikey->get_key())->vtn_name,
                           (kMaxLenVtnName+1));
        /* Copy Controller name to key */
        uuu::upll_strncpy(reinterpret_cast<key_vtn_t *>
                            (ikey->get_key())->vtn_name,
             reinterpret_cast<val_rename_vtn_t* >
                            (rename_val)->new_name,
                            (kMaxLenVtnName+1));
        /* Copy the UNC name to Val */
        uuu::upll_strncpy(reinterpret_cast<val_rename_vtn_t*>(rename)->new_name,
                          temp_str, (kMaxLenVtnName+1));
        }
        break;
      case UNC_KT_VBRIDGE:
      case UNC_KT_VTERMINAL:
      case UNC_KT_VROUTER:
      case UNC_KT_VLINK:
      {
        rename = reinterpret_cast<val_rename_vtn_t *>(
                           ConfigKeyVal::Malloc(sizeof(val_rename_vtn_t)));
        rename_val = reinterpret_cast<val_rename_vnode_t *>(GetVal(ikey));
        if (!rename_val) {
          UPLL_LOG_DEBUG("Val is Empty");
          free(rename);
          return UPLL_RC_ERR_GENERIC;
        }
        PrintMap();
        if (!strcmp(
                (const char*)reinterpret_cast<key_vbr_t *>
                (ikey->get_key())->vbridge_name,
                (const char *)reinterpret_cast<val_rename_vnode_t*>
                (rename_val)->ctrlr_vnode_name)) {
          reinterpret_cast<val_rename_vtn_t*>(rename)->
              valid[UPLL_IDX_NEW_NAME_RVBR] = UNC_VF_INVALID;
        } else {
          reinterpret_cast<val_rename_vtn_t*>(rename)->
              valid[UPLL_IDX_NEW_NAME_RVBR] = UNC_VF_VALID;
          reinterpret_cast<val_rename_vbr_t*>(rename)->
              valid[UPLL_IDX_RENAME_TYPE_RVBR] = UNC_VF_VALID;
          /*
           * Take the UNC name and checks in the map
           */
          renamed_name = (const char*)reinterpret_cast<key_vbr_t *>
                            (ikey->get_key())->vbridge_name;
          std::map<std::string, std::string>::iterator unc_name =
              auto_rename_.find(renamed_name);
          if (unc_name != auto_rename_.end()) {
            reinterpret_cast<val_rename_vbr_t*>(rename)->rename_type =
                UPLL_RENAME_TYPE_AUTO;
          } else {
            reinterpret_cast<val_rename_vbr_t*>(rename)->rename_type =
                UPLL_RENAME_TYPE_MANUAL;
          }
        }

        /* copyt key to temp */
        uuu::upll_strncpy(temp_str, reinterpret_cast<key_vbr_t*>
                            (ikey->get_key())->vbridge_name,
                            (kMaxLenVnodeName+1));

        /* Copy Controller name to key */
        uuu::upll_strncpy(reinterpret_cast<key_vbr_t *>
                            (ikey->get_key())->vtn_key.vtn_name,
             reinterpret_cast<val_rename_vnode_t* >
                            (rename_val)->ctrlr_vtn_name,
                            (kMaxLenVtnName+1));
        uuu::upll_strncpy(reinterpret_cast<key_vbr_t *>
                            (ikey->get_key())->vbridge_name,
             reinterpret_cast<val_rename_vnode_t* >
                            (rename_val)->ctrlr_vnode_name,
                            (kMaxLenVnodeName+1));
        /* Copy the UNC name to Val */
        uuu::upll_strncpy(reinterpret_cast<val_rename_vtn_t*>(rename)->new_name,
                          temp_str, (kMaxLenVnodeName+1));
      }
        break;
      default:
        break;
    }
  }
  ikey->SetCfgVal(new ConfigVal(struct_num, rename));
  return UPLL_RC_SUCCESS;
}


upll_rc_t MoMgrImpl::SwapKey(ConfigKeyVal *&ikey,
                             uint8_t rename_flag) {
  UPLL_FUNC_TRACE;
  uui::IpctSt::IpcStructNum struct_num  = IpctSt::kIpcInvalidStNum;
  void *rename = NULL;
  UPLL_LOG_TRACE("Before Swap Key %s %d",
                 ikey->ToStrAll().c_str(), rename_flag);
  if (rename_flag) {
    Swapvaltokey(ikey, rename_flag);
  } else {
    Getvalstnum(ikey, struct_num);

    switch (ikey->get_key_type()) {
      case UNC_KT_FLOWLIST:
          rename = reinterpret_cast
               <val_rename_flowlist *>(ConfigKeyVal::Malloc
               (sizeof(val_rename_flowlist)));
          uuu::upll_strncpy(reinterpret_cast<val_rename_flowlist *>(rename)
               ->flowlist_newname,
               reinterpret_cast<key_flowlist_t*>(ikey->get_key())
               ->flowlist_name, (kMaxLenFlowListName+1));
          break;
     case UNC_KT_POLICING_PROFILE:
          rename = reinterpret_cast
                 <val_rename_policingprofile *>(
                 ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile)));
          uuu::upll_strncpy(reinterpret_cast<val_rename_policingprofile *>
                 (rename)->policingprofile_newname,
                 reinterpret_cast<key_policingprofile_t*>
                 (ikey->get_key())->policingprofile_name,
                 (kMaxLenPolicingProfileName+1));
          break;
      case UNC_KT_VTN:
          rename = reinterpret_cast<val_rename_vtn_t *>(
                   ConfigKeyVal::Malloc(sizeof(val_rename_vtn_t)));
          uuu::upll_strncpy(reinterpret_cast<val_rename_vtn_t *>
                   (rename)->new_name,
               reinterpret_cast<key_vtn_t*>(ikey->get_key())->vtn_name,
                          (kMaxLenVtnName+1));
          break;
      case UNC_KT_VBRIDGE:
      case UNC_KT_VTERMINAL:
      case UNC_KT_VROUTER:
      case UNC_KT_VLINK:
          rename = reinterpret_cast<val_rename_vtn_t *>(
                   ConfigKeyVal::Malloc(sizeof(val_rename_vtn_t)));
          uuu::upll_strncpy(reinterpret_cast<val_rename_vtn_t *>
                (rename)->new_name,
          reinterpret_cast<key_vbr_t*>(ikey->get_key())->vbridge_name,
          (kMaxLenVnodeName+1));
          break;
       default:
          return UPLL_RC_ERR_GENERIC;
          break;
    }

    reinterpret_cast<val_rename_vtn_t *>(rename)->valid
            [UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_INVALID;
  ikey->SetCfgVal(new ConfigVal(struct_num, rename));
  }
  UPLL_LOG_TRACE("AfterSwap Key %s", ikey->ToStrAll().c_str());
  return UPLL_RC_SUCCESS;
}


std::string MoMgrImpl::GetReadImportQueryString(unc_keytype_operation_t op,
                                                unc_key_type_t kt) const {
  std::string query_string = "";
  if (op == UNC_OP_READ_SIBLING) {
    switch (kt) {
      case UNC_KT_FLOWLIST:
        query_string = \
          " (select unc_flowlist_name, ctrlr_name, ctrlr_flowlist_name from "
            " (select unc_flowlist_name, ctrlr_name, ctrlr_flowlist_name "
               " from im_flowlist_rename_tbl "
            " union all "
            "select flowlist_name, ctrlr_name, flowlist_name from "
              "im_flowlist_ctrlr_tbl where flags & 1 = 0 ) as temp "
            "where ctrlr_flowlist_name > ?) "
          " order by ctrlr_flowlist_name ";
      break;
      case UNC_KT_POLICING_PROFILE:
        query_string =
          "(select unc_policingprofile_name, ctrlr_name, "
           "ctrlr_policingprofile_name from "
           "(select unc_policingprofile_name, ctrlr_name, "
           "ctrlr_policingprofile_name "
           "from im_policingprofile_rename_tbl "
           "union all select policingprofile_name,"
           "ctrlr_name, policingprofile_name from im_policingprofile_ctrlr_tbl "
           "where flags & 1 = 0 ) as temp where "
           "ctrlr_policingprofile_name > ?) "
          "order by ctrlr_policingprofile_name";
        break;
      case UNC_KT_VTN:
        query_string =
          "(select vtn_name, ctrlr_name, domain_id, ctrlr_vtn_name from "
           "(select vtn_name, ctrlr_name, domain_id, ctrlr_vtn_name "
           "from im_vtn_rename_tbl union all select vtn_name,"
           "ctrlr_name, domain_id, vtn_name from im_vtn_ctrlr_tbl "
           "where flags & 1 = 0 ) as temp where ctrlr_vtn_name > ?) "
          "order by ctrlr_vtn_name";
        break;
      case UNC_KT_VBRIDGE:
        query_string =
          "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
           "ctrlr_vtn_name, ctrlr_vnode_name from "
           "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
            "ctrlr_vtn_name, ctrlr_vnode_name from im_vnode_rename_tbl where "
            "(vtn_name, unc_vnode_name) IN (select vtn_name, vbridge_name "
            "from im_vbr_tbl) "
            "union all select vtn_name, vbridge_name, ctrlr_name, domain_id, "
            "vtn_name, vbridge_name from im_vbr_tbl where flags & 3 = 0) as "
            "temp  where ctrlr_vtn_name = ? and ctrlr_vnode_name > ?) "
            "order by ctrlr_vtn_name, ctrlr_vnode_name ";
        break;
      case UNC_KT_VROUTER:
        query_string =
          "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
           "ctrlr_vtn_name, ctrlr_vnode_name from "
           "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
           "ctrlr_vtn_name, ctrlr_vnode_name from im_vnode_rename_tbl where "
           "(vtn_name, unc_vnode_name) IN (select vtn_name, vrouter_name "
           "from im_vrt_tbl) "
           "union all select vtn_name, vrouter_name, ctrlr_name, domain_id, "
           "vtn_name, vrouter_name from im_vrt_tbl where flags & 3 = 0) as "
           "temp  where ctrlr_vtn_name = ? and ctrlr_vnode_name > ?) "
           "order by ctrlr_vtn_name, ctrlr_vnode_name ";
        break;
      case UNC_KT_VLINK:
        query_string =
          "(select vtn_name, unc_vlink_name, ctrlr_name, domain_id, "
           "ctrlr_vtn_name, ctrlr_vlink_name from "
           "(select vtn_name, unc_vlink_name, ctrlr_name, domain_id, "
           "ctrlr_vtn_name, ctrlr_vlink_name from im_vlink_rename_tbl where "
           "(vtn_name, unc_vlink_name) IN (select vtn_name, vlink_name "
           "from im_vlink_tbl) "
           "union all select vtn_name, vlink_name, controller1_name, "
           "domain1_id, "
           "vtn_name, vlink_name from im_vlink_tbl where key_flags & 3 = 0) "
           "as temp  where "
           "ctrlr_vtn_name = ? and ctrlr_vlink_name > ?) order by "
           "ctrlr_vtn_name, ctrlr_vlink_name ";
        break;
        case UNC_KT_VTERMINAL:
        query_string =
        "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
         "ctrlr_vtn_name, ctrlr_vnode_name from "
         "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
          "ctrlr_vtn_name, ctrlr_vnode_name from im_vnode_rename_tbl where "
          "(vtn_name, unc_vnode_name) IN (select vtn_name, vterminal_name "
          "from im_vterminal_tbl) "
          "union all select vtn_name, vterminal_name, ctrlr_name, domain_id, "
          "vtn_name, vterminal_name from im_vterminal_tbl where flags & 3 = 0) "
          "as temp  where ctrlr_vtn_name = ? and ctrlr_vnode_name > ?) "
          "order by ctrlr_vtn_name, ctrlr_vnode_name ";
        break;

      default:
        break;
    }
  } else if (op == UNC_OP_READ_SIBLING_BEGIN) {
    switch (kt) {
      case UNC_KT_FLOWLIST:
        query_string =
            "(select unc_flowlist_name, ctrlr_name, ctrlr_flowlist_name from "
             "im_flowlist_rename_tbl "
             "union all select flowlist_name, ctrlr_name, flowlist_name from "
             "im_flowlist_ctrlr_tbl "
             "where flags & 1 = 0) order by ctrlr_flowlist_name ";
        break;
      case UNC_KT_POLICING_PROFILE:
        query_string =
            "(select unc_policingprofile_name, ctrlr_name, "
             "ctrlr_policingprofile_name from im_policingprofile_rename_tbl "
             "union all select policingprofile_name, ctrlr_name, "
             "policingprofile_name from im_policingprofile_ctrlr_tbl "
             "where flags & 1 = 0) order by ctrlr_policingprofile_name";
        break;
      case UNC_KT_VTN:
        query_string = "(select vtn_name, ctrlr_name, domain_id, "
                        "ctrlr_vtn_name from im_vtn_rename_tbl  union all "
                        "select vtn_name, ctrlr_name, domain_id, vtn_name "
                        "from im_vtn_ctrlr_tbl  where flags & 1 = 0) "
                        "order by ctrlr_vtn_name ";
        break;
      case UNC_KT_VBRIDGE:
        query_string =
            "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
            "ctrlr_vtn_name, ctrlr_vnode_name from "
            "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
            "ctrlr_vtn_name, ctrlr_vnode_name from im_vnode_rename_tbl where "
            "(vtn_name, unc_vnode_name) IN (select vtn_name, vbridge_name "
            "from im_vbr_tbl) "
            "union all select vtn_name, vbridge_name, ctrlr_name, domain_id, "
            "vtn_name, vbridge_name from im_vbr_tbl where flags & 3 = 0) as "
            "temp  where ctrlr_vtn_name = ?) order by "
            "ctrlr_vtn_name, ctrlr_vnode_name ";
        break;
      case UNC_KT_VROUTER:
        query_string =
            "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
            "ctrlr_vtn_name, ctrlr_vnode_name from "
            "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
            "ctrlr_vtn_name, ctrlr_vnode_name from im_vnode_rename_tbl where "
            "(vtn_name, unc_vnode_name) IN (select vtn_name, vrouter_name "
            "from im_vrt_tbl) "
            "union all select vtn_name, vrouter_name, ctrlr_name, domain_id, "
            "vtn_name, vrouter_name from im_vrt_tbl where flags & 3 = 0) "
            "as temp  where ctrlr_vtn_name = ?) order by ctrlr_vtn_name, "
            "ctrlr_vnode_name ";
        break;
      case UNC_KT_VLINK:
        query_string =
            "(select vtn_name, unc_vlink_name, ctrlr_name, domain_id, "
            "ctrlr_vtn_name, ctrlr_vlink_name from "
            "(select vtn_name, unc_vlink_name, ctrlr_name, domain_id, "
            "ctrlr_vtn_name, ctrlr_vlink_name from im_vlink_rename_tbl where "
            "(vtn_name, unc_vlink_name) IN (select vtn_name, vlink_name "
            "from im_vlink_tbl) "
            "union all select vtn_name, vlink_name, controller1_name, "
            "domain1_id, "
            "vtn_name, vlink_name from im_vlink_tbl where key_flags & 3 = 0) "
            "as temp  where "
            "ctrlr_vtn_name = ? ) order by ctrlr_vtn_name, ctrlr_vlink_name ";
        break;
      case UNC_KT_VTERMINAL:
        query_string =
            "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
            "ctrlr_vtn_name, ctrlr_vnode_name from "
             "(select vtn_name, unc_vnode_name, ctrlr_name, domain_id, "
             "ctrlr_vtn_name, ctrlr_vnode_name from im_vnode_rename_tbl where "
             "(vtn_name, unc_vnode_name) IN (select vtn_name, "
             "vterminal_name from im_vterminal_tbl) "
             "union all select vtn_name, vterminal_name, ctrlr_name, "
             " domain_id, "
             "vtn_name, vterminal_name from im_vterminal_tbl "
             "where flags & 3 = 0) as temp  where "
             "ctrlr_vtn_name = ?) order by ctrlr_vtn_name, ctrlr_vnode_name ";
        break;

      default:
        break;
    }
  }
  return query_string;
}

upll_rc_t MoMgrImpl::ReadImportDB(ConfigKeyVal *&in_key,
                                   IpcReqRespHeader *header,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  uint8_t rename = 0;
  uint32_t count = 0;
  ConfigKeyVal *ikey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetChildConfigKey(ikey, in_key);
  if (UPLL_RC_SUCCESS != result_code)
    return result_code;
  const uudst::kDalTableIndex tbl_index = GetTable(RENAMETBL, header->datatype);
  void *tkey = (ikey)?ikey->get_key():NULL;
  if (!tkey) {
    delete ikey;
    return UPLL_RC_ERR_GENERIC;
  }
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    delete ikey;
    return UPLL_RC_ERR_GENERIC;
  }
  DalCursor *dal_cursor_handle = NULL;
  UPLL_LOG_TRACE("tbl_index is %d", tbl_index);
  unc_keytype_operation_t op = header->operation;

  if (!READ_OP(op)) {
    UPLL_LOG_INFO("Exiting MoMgrImpl::ReadConfigDB");
    delete ikey;
    return UPLL_RC_ERR_GENERIC;
  }
#if 0
  uint16_t max_record_count = 1;
  result_code = GetRenamedUncKey(ikey, header->datatype,
                                            dmi, NULL);
  if (UPLL_RC_SUCCESS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetRenamedUncKey is Failed %d", result_code);
      return result_code;
  }
#endif

  if (op == UNC_OP_READ_SIBLING_BEGIN || op == UNC_OP_READ_SIBLING) {
       DalBindInfo *dal_bind_info = new DalBindInfo(tbl_index);
       result_code = BindImportDB(ikey, dal_bind_info, header->datatype,
                                  RENAMETBL);
       if (result_code != UPLL_RC_SUCCESS) {
         if (dal_bind_info) delete dal_bind_info;
         UPLL_LOG_INFO("Exiting MoMgrImpl::ReadConfigDB result code %d",
                       result_code);
         delete ikey;
         return result_code;
       }
       std::string query_string = GetReadImportQueryString(
           op, ikey->get_key_type());
       if (query_string.empty()) {
         UPLL_LOG_TRACE("Null Query String for Operation(%d) KeyType(%d)",
                        op, ikey->get_key_type());
         if (dal_bind_info) delete dal_bind_info;
         delete ikey;
         return UPLL_RC_ERR_GENERIC;
       }
       UPLL_LOG_TRACE("Header rep count before %d", header->rep_count);
       result_code = DalToUpllResCode(
           dmi->ExecuteAppQueryMultipleRecords(
               query_string, header->rep_count,
               dal_bind_info, &dal_cursor_handle));

       UPLL_LOG_TRACE("Header rep count after %d", header->rep_count);
       ConfigKeyVal *end_resp = NULL;
       bool flag = false;
       ConfigKeyVal *prev_key = NULL;
       while (result_code == UPLL_RC_SUCCESS && ((count < header->rep_count) ||
                                                 (header->rep_count == 0))) {
         result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
         if (UPLL_RC_SUCCESS == result_code) {
           ConfigKeyVal *tkey = NULL;
           val_rename_vtn_t *val = (ikey)?reinterpret_cast<val_rename_vtn_t*>
               (GetVal(ikey)):NULL;
           if (val) {
             val->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
           }
           UPLL_LOG_TRACE("GetNextRecord %s", ikey->ToStrAll().c_str());
           result_code = DupConfigKeyVal(tkey, ikey, RENAMETBL);
           if (result_code != UPLL_RC_SUCCESS) {
             UPLL_LOG_DEBUG("Dup failed error %d", result_code);
             delete ikey;
             delete dal_bind_info;
             dmi->CloseCursor(dal_cursor_handle);
             return result_code;
           }
           flag = true;
           rename = 0;
           result_code = UpdateConfigDB(tkey, header->datatype, UNC_OP_READ,
                                        dmi, RENAMETBL);
           if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS)
                 rename = 1;
               else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
                 UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
                 delete ikey;
                 dmi->CloseCursor(dal_cursor_handle);
                 delete dal_bind_info;
                 DELETE_IF_NOT_NULL(tkey);
                 return result_code;
               }
               result_code = SwapKey(tkey, rename);
               if (UPLL_RC_SUCCESS != result_code) {
                 UPLL_LOG_DEBUG("SwapKey failed %d", result_code);
                 delete ikey;
                 dmi->CloseCursor(dal_cursor_handle);
                 delete dal_bind_info;
                 DELETE_IF_NOT_NULL(tkey);
                 return result_code;
               }
               if (!end_resp)
                 end_resp = tkey;
               else
                 prev_key->AppendCfgKeyVal(tkey);
               prev_key = tkey;
               count++;
               UPLL_LOG_TRACE("end_resp %s", end_resp->ToStrAll().c_str());
         } else {
           UPLL_LOG_TRACE("No Record");
         }
         UPLL_LOG_TRACE("Result code = %d and Count = %d", result_code, count);
       }
       header->rep_count = count;
       result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code && flag)?
           UPLL_RC_SUCCESS:result_code;
       if (dal_cursor_handle)
         dmi->CloseCursor(dal_cursor_handle);
       if (dal_bind_info)
         delete dal_bind_info;
       if (result_code == UPLL_RC_SUCCESS) {
         if (end_resp)
           ikey->ResetWith(end_resp);
         DELETE_IF_NOT_NULL(end_resp);
         in_key->ResetWith(ikey);
         DELETE_IF_NOT_NULL(ikey);
         UPLL_LOG_TRACE("ResetWith is Called");
       } else {
         delete ikey;
         return result_code;
       }
  } else if (op == UNC_OP_READ) {
#if 0  // tbl is set, but not used.
    MoMgrTables tbl = MAINTBL;
    if (UNC_KT_VTN == ikey->get_key_type() ||
        UNC_KT_FLOWLIST == ikey->get_key_type() ||
        UNC_KT_POLICING_PROFILE == ikey->get_key_type())
      tbl = CTRLRTBL;
#endif
    /* We are not allow to read using the UNC Name
    */
#if 1
    DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone};

    ConfigKeyVal *temp = NULL;

    result_code = CopyKeyToVal(ikey, temp);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("CopyKeyToVal failed");
      delete ikey;
      return result_code;
    }
    dbop.readop = kOpReadSingle;
    result_code = UpdateConfigDB(temp, header->datatype, UNC_OP_READ,
                                 dmi, &dbop, RENAMETBL);
    if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(temp);
      delete ikey;
      return result_code;
    }
    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
      UPLL_LOG_DEBUG("Read Not allowed by using UNC Name");
      result_code = SwapKey(ikey, rename);
      UPLL_LOG_TRACE("After No SwapKey %s", ikey->ToStrAll().c_str());
      in_key->ResetWith(ikey);
      delete ikey;
      DELETE_IF_NOT_NULL(temp);
      return result_code;
    }
    DELETE_IF_NOT_NULL(temp);
#endif
    rename  = 0;
    result_code = GetRenamedUncKey(ikey, header->datatype,
                                   dmi, NULL);
    if (UPLL_RC_SUCCESS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetRenamedUncKey is Failed %d", result_code);
      delete ikey;
      return result_code;
    }
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UpdateConfigDB(ikey, header->datatype, UNC_OP_READ,
                                   dmi, MAINTBL);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("VTN doesn't exist in IMPORT DB. Error code : %d",
                       result_code);
        delete ikey;
        return result_code;
      } else {
        result_code = UPLL_RC_SUCCESS;
      }
      ikey->SetCfgVal(NULL);
    } else {
      DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
      result_code = ReadConfigDB(ikey, header->datatype, header->operation,
                                 dbop, dmi, RENAMETBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        delete ikey;
        return result_code;
      }
      rename = 1;
    }
    result_code = SwapKey(ikey, rename);
    UPLL_LOG_TRACE("After No SwapKey %s", ikey->ToStrAll().c_str());
    in_key->ResetWith(ikey);
    delete ikey;
  } else {
    UPLL_LOG_TRACE("Unexpected Operation : %d", op);
    delete ikey;
    return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t MoMgrImpl::DeleteGlobalConfigInCandidate(unc_key_type_t key_type,
                                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  std::string query_string = "";
  switch (key_type) {
    case UNC_KT_VTN:
      query_string = "DELETE FROM ca_vtn_tbl WHERE not exists "
                      "(SELECT vtn_name from ca_vtn_ctrlr_tbl "
                       "where ca_vtn_tbl.vtn_name=ca_vtn_ctrlr_tbl.vtn_name);";
      break;
    case UNC_KT_FLOWLIST:
      query_string = "DELETE FROM ca_flowlist_tbl WHERE not exists "
                      "(SELECT flowlist_name from ca_flowlist_ctrlr_tbl "
                       "where ca_flowlist_tbl.flowlist_name="
                       "ca_flowlist_ctrlr_tbl.flowlist_name);";
      break;
    case UNC_KT_FLOWLIST_ENTRY:
      query_string = "DELETE FROM ca_flowlist_entry_tbl WHERE not exists "
                      "(SELECT flowlist_name, sequence_num "
                       "from ca_flowlist_entry_ctrlr_tbl "
                       "where ca_flowlist_entry_tbl.flowlist_name="
                       "ca_flowlist_entry_ctrlr_tbl.flowlist_name and "
                       "ca_flowlist_entry_tbl.sequence_num ="
                       "ca_flowlist_entry_ctrlr_tbl.sequence_num);";
      break;
    case UNC_KT_POLICING_PROFILE:
      query_string = "DELETE FROM ca_policingprofile_tbl WHERE not exists "
                      "(SELECT policingprofile_name from "
                       "ca_policingprofile_ctrlr_tbl "
                       "where ca_policingprofile_tbl.policingprofile_name="
                       "ca_policingprofile_ctrlr_tbl.policingprofile_name);";
      break;
    case UNC_KT_POLICING_PROFILE_ENTRY:
      query_string =
          "DELETE FROM ca_policingprofile_entry_tbl WHERE not exists "
           "(SELECT policingprofile_name, sequence_num from "
            "ca_policingprofile_entry_ctrlr_tbl "
            "where ca_policingprofile_entry_tbl.policingprofile_name="
            "ca_policingprofile_entry_ctrlr_tbl.policingprofile_name and "
            "ca_policingprofile_entry_tbl.sequence_num ="
            "ca_policingprofile_entry_ctrlr_tbl.sequence_num);";
      break;
    case UNC_KT_VTN_FLOWFILTER:
      query_string = "DELETE FROM ca_vtn_flowfilter_tbl WHERE not exists "
                      "(SELECT vtn_name, direction from "
                       "ca_vtn_flowfilter_ctrlr_tbl "
                       "where ca_vtn_flowfilter_tbl.vtn_name="
                       "ca_vtn_flowfilter_ctrlr_tbl.vtn_name "
                       "and ca_vtn_flowfilter_tbl.direction="
                       "ca_vtn_flowfilter_ctrlr_tbl.direction);";
      break;
    case UNC_KT_VTN_FLOWFILTER_ENTRY:
     query_string = "DELETE FROM ca_vtn_flowfilter_entry_tbl WHERE not exists "
                     "(SELECT vtn_name, direction, sequence_num from "
                      "ca_vtn_flowfilter_entry_ctrlr_tbl "
                      "where ca_vtn_flowfilter_entry_tbl.vtn_name="
                      "ca_vtn_flowfilter_entry_ctrlr_tbl.vtn_name "
                      "and ca_vtn_flowfilter_entry_tbl.direction="
                      "ca_vtn_flowfilter_entry_ctrlr_tbl.direction "
                      "and ca_vtn_flowfilter_entry_tbl.sequence_num="
                      "ca_vtn_flowfilter_entry_ctrlr_tbl.sequence_num);";
      break;
    case UNC_KT_VTN_POLICINGMAP:
      query_string = "DELETE FROM ca_vtn_policingmap_tbl WHERE not exists "
                      "(SELECT vtn_name from "
                       "ca_vtn_policingmap_ctrlr_tbl "
                       "where ca_vtn_policingmap_tbl.vtn_name="
                       "ca_vtn_policingmap_ctrlr_tbl.vtn_name);";
      break;
     default:
      return UPLL_RC_ERR_GENERIC;
  }

  const uudst::kDalTableIndex tbl_index = GetTable(MAINTBL, UPLL_DT_CANDIDATE);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t  result_code = DalToUpllResCode(
      dmi->ExecuteAppQueryModifyRecord(
          UPLL_DT_CANDIDATE,
          tbl_index, query_string,
          NULL, UNC_OP_DELETE, TC_CONFIG_GLOBAL, NULL));
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code ||
      UPLL_RC_ERR_PARENT_DOES_NOT_EXIST == result_code)
    result_code = UPLL_RC_SUCCESS;
  return result_code;
}

upll_rc_t
MoMgrImpl::PI_MergeValidate_for_Vtn_Flowfilter(unc_key_type_t keytype,
                                               const char *ctrlr_id,
                                               ConfigKeyVal *conflict_ckv,
                                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  int nattr = 0;
  BindInfo *binfo = NULL;
  DalCursor *dal_cursor_handle = NULL;

  const uudst::kDalTableIndex tbl_index = GetTable(CTRLRTBL, UPLL_DT_IMPORT);

  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  DalBindInfo dal_bind_info(tbl_index);

  if (!GetBindInfo(CTRLRTBL, UPLL_DT_IMPORT, binfo, nattr)) {
    UPLL_LOG_DEBUG("GetBindInfo failed");
    return UPLL_RC_ERR_GENERIC;
  }
  nattr = 3;
  ConfigKeyVal *vtn_ff_ckv  = NULL;

  result_code = GetChildConfigKey(vtn_ff_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }

  uint32_t count = 0;
  void *tkey = vtn_ff_ckv->get_key();
  for (int i = 0; i < nattr; i++) {
    uint64_t indx = binfo[i].index;
    void *p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
                 + binfo[i].offset);
    if (i != 2) {
      dal_bind_info.BindOutput(indx, binfo[i].app_data_type,
               binfo[i].array_size, p);
    } else {
      dal_bind_info.BindOutput(binfo[i-1].index, binfo[i-1].app_data_type,
               binfo[i-1].array_size, &count);
    }
  }

  string query_string = "select vtn_name, direction, count(*) from "
                         "im_vtn_flowfilter_ctrlr_tbl GROUP BY vtn_name, "
                         "direction ORDER BY vtn_name;";

  result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
                 query_string, 0, &dal_bind_info,
                 &dal_cursor_handle));
  VtnMoMgr *vtnmgr = static_cast<VtnMoMgr*>(const_cast<MoManager*>
                                           (GetMoManager(UNC_KT_VTN)));
  if (!vtnmgr) {
    UPLL_LOG_DEBUG("vtnmgr failed");
    DELETE_IF_NOT_NULL(vtn_ff_ckv);
    if (dal_cursor_handle != NULL)
      dmi->CloseCursor(dal_cursor_handle, false);
    return UPLL_RC_ERR_GENERIC;
  }
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (UPLL_RC_SUCCESS == result_code) {
      key_vtn_flowfilter_t *key_vtn_ff =
          reinterpret_cast<key_vtn_flowfilter_t *>(vtn_ff_ckv->get_key());

      ConfigKeyVal *vtn_ckv = NULL;
      result_code = vtnmgr->GetChildConfigKey(vtn_ckv, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(vtn_ff_ckv);
        if (dal_cursor_handle != NULL)
          dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }

      key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(vtn_ckv->get_key());
      uuu::upll_strncpy(vtn_ikey->vtn_name, key_vtn_ff->vtn_key.vtn_name,
                        kMaxLenVtnName+1);
      uint32_t imp_instance_count = 0;
      result_code = vtnmgr->GetNormalDomainCtrlrTableCount(
          vtn_ckv, UPLL_DT_IMPORT, dmi, &imp_instance_count);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("Get Instance Count failed %d", result_code);
        DELETE_IF_NOT_NULL(vtn_ff_ckv);
        DELETE_IF_NOT_NULL(vtn_ckv);
        if (dal_cursor_handle != NULL)
          dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }

      UPLL_LOG_DEBUG("count, instance count  (%d) (%d)",
                     count, imp_instance_count);

      if (count != imp_instance_count) {
        UPLL_LOG_INFO("Merge Conflict: Multi domain count not matched");
        DELETE_IF_NOT_NULL(vtn_ff_ckv);
        DELETE_IF_NOT_NULL(vtn_ckv);
        if (dal_cursor_handle != NULL)
          dmi->CloseCursor(dal_cursor_handle, false);
       return UPLL_RC_ERR_MERGE_CONFLICT;
      }
      DELETE_IF_NOT_NULL(vtn_ckv);
    }
  }
  DELETE_IF_NOT_NULL(vtn_ff_ckv);
  if (dal_cursor_handle != NULL)
    dmi->CloseCursor(dal_cursor_handle, false);

  return UPLL_RC_SUCCESS;
}

upll_rc_t
MoMgrImpl::PI_MergeValidate_for_Vtn_Flowfilter_Entry(unc_key_type_t keytype,
                                               const char *ctrlr_id,
                                               ConfigKeyVal *conflict_ckv,
                                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  int nattr = 0;
  BindInfo *binfo = NULL;
  DalCursor *dal_cursor_handle = NULL;

  const uudst::kDalTableIndex tbl_index = GetTable(CTRLRTBL, UPLL_DT_IMPORT);

  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  DalBindInfo dal_bind_info(tbl_index);

  if (!GetBindInfo(CTRLRTBL, UPLL_DT_IMPORT, binfo, nattr)) {
    UPLL_LOG_DEBUG("GetBindInfo failed");
    return UPLL_RC_ERR_GENERIC;
  }
  nattr = 4;
  ConfigKeyVal *vtn_ffe_ckv  = NULL;

  result_code = GetChildConfigKey(vtn_ffe_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }

  uint32_t count = 0;
  void *tkey = vtn_ffe_ckv->get_key();
  for (int i = 0; i < nattr; i++) {
    uint64_t indx = binfo[i].index;
    void *p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
                 + binfo[i].offset);
    if (i != 3) {
      dal_bind_info.BindOutput(indx, binfo[i].app_data_type,
               binfo[i].array_size, p);
    } else {
      dal_bind_info.BindOutput(binfo[i-1].index, binfo[i-1].app_data_type,
               binfo[i-1].array_size, &count);
    }
  }

  string query_string = "select vtn_name, direction, sequence_num, count(*) "
                         "from im_vtn_flowfilter_entry_ctrlr_tbl GROUP BY "
                         "vtn_name, direction, sequence_num ORDER BY vtn_name;";

  result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
                 query_string, 0, &dal_bind_info,
                 &dal_cursor_handle));
  VtnMoMgr *vtnmgr = static_cast<VtnMoMgr*>(const_cast<MoManager*>
                                           (GetMoManager(UNC_KT_VTN)));
  if (!vtnmgr) {
    UPLL_LOG_DEBUG("vtnmgr failed");
    DELETE_IF_NOT_NULL(vtn_ffe_ckv);
    if (dal_cursor_handle != NULL)
      dmi->CloseCursor(dal_cursor_handle, false);
    return UPLL_RC_ERR_GENERIC;
  }
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (UPLL_RC_SUCCESS == result_code) {
      key_vtn_flowfilter_entry_t *key_vtn_ff_entry =
          reinterpret_cast<key_vtn_flowfilter_entry_t *>
          (vtn_ffe_ckv->get_key());

      ConfigKeyVal *vtn_ckv = NULL;
      result_code = vtnmgr->GetChildConfigKey(vtn_ckv, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(vtn_ffe_ckv);
        if (dal_cursor_handle != NULL)
          dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }

      key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(vtn_ckv->get_key());
      uuu::upll_strncpy(vtn_ikey->vtn_name,
                        key_vtn_ff_entry->flowfilter_key.vtn_key.vtn_name,
                        kMaxLenVtnName+1);
      uint32_t imp_instance_count = 0;
      result_code = vtnmgr->GetNormalDomainCtrlrTableCount(
          vtn_ckv, UPLL_DT_IMPORT, dmi, &imp_instance_count);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetInstanceCount failed %d", result_code);
        DELETE_IF_NOT_NULL(vtn_ffe_ckv);
        DELETE_IF_NOT_NULL(vtn_ckv);
        if (dal_cursor_handle != NULL)
          dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }

      UPLL_LOG_DEBUG("count %d, instance count %d", count, imp_instance_count);

      if (count != imp_instance_count) {
        UPLL_LOG_INFO("Merge Conflict: Multi domain count not matched");
        DELETE_IF_NOT_NULL(vtn_ffe_ckv);
        DELETE_IF_NOT_NULL(vtn_ckv);
        if (dal_cursor_handle != NULL)
          dmi->CloseCursor(dal_cursor_handle, false);
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
      DELETE_IF_NOT_NULL(vtn_ckv);
    }
  }
  if (dal_cursor_handle != NULL)
    dmi->CloseCursor(dal_cursor_handle, false);

  delete vtn_ffe_ckv;
  return UPLL_RC_SUCCESS;
}

upll_rc_t
MoMgrImpl::PI_MergeValidate_for_Vtn_Policingmap(unc_key_type_t keytype,
                                               const char *ctrlr_id,
                                               ConfigKeyVal *conflict_ckv,
                                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  BindInfo *binfo = NULL;
  DalCursor *dal_cursor_handle = NULL;
  int nattr = 0;

  const uudst::kDalTableIndex tbl_index = GetTable(CTRLRTBL, UPLL_DT_IMPORT);

  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  DalBindInfo dal_bind_info(tbl_index);

  if (!GetBindInfo(CTRLRTBL, UPLL_DT_IMPORT, binfo, nattr)) {
    UPLL_LOG_DEBUG("GetBindInfo failed");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vtn_pm_ckv  = NULL;

  result_code = GetChildConfigKey(vtn_pm_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }

  uint32_t count = 0;
  void *tkey = vtn_pm_ckv->get_key();
  void *p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
               + binfo[0].offset);
  dal_bind_info.BindOutput(binfo[0].index, binfo[0].app_data_type,
                       binfo[0].array_size, p);

  dal_bind_info.BindOutput(binfo[4].index, binfo[4].app_data_type,
                           binfo[4].array_size, &count);

  string query_string = "select vtn_name, count(*) from "
                         "im_vtn_policingmap_ctrlr_tbl GROUP BY vtn_name "
                         "ORDER BY vtn_name;";

  result_code = DalToUpllResCode(dmi->ExecuteAppQueryMultipleRecords(
                 query_string, 0, &dal_bind_info,
                 &dal_cursor_handle));

  VtnMoMgr *vtnmgr = static_cast<VtnMoMgr*>(const_cast<MoManager*>
                                           (GetMoManager(UNC_KT_VTN)));
  if (!vtnmgr) {
    UPLL_LOG_DEBUG("vtnmgr failed");
    DELETE_IF_NOT_NULL(vtn_pm_ckv);
    if (dal_cursor_handle != NULL)
      dmi->CloseCursor(dal_cursor_handle, false);
    return UPLL_RC_ERR_GENERIC;
  }
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (UPLL_RC_SUCCESS == result_code) {
      key_vtn_t *key_vtn =
          reinterpret_cast<key_vtn_t *>(vtn_pm_ckv->get_key());

      ConfigKeyVal *vtn_ckv = NULL;
      result_code = vtnmgr->GetChildConfigKey(vtn_ckv, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(vtn_pm_ckv);
        if (dal_cursor_handle != NULL)
          dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }

      key_vtn_t *vtn_ikey = reinterpret_cast<key_vtn_t *>(vtn_ckv->get_key());
      uuu::upll_strncpy(vtn_ikey->vtn_name, key_vtn->vtn_name,
                        kMaxLenVtnName+1);
      uint32_t imp_instance_count = 0;
      result_code = vtnmgr->GetNormalDomainCtrlrTableCount(
          vtn_ckv, UPLL_DT_IMPORT, dmi, &imp_instance_count);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_INFO("GetInstanceCount failed %d", result_code);
        DELETE_IF_NOT_NULL(vtn_pm_ckv);
        DELETE_IF_NOT_NULL(vtn_ckv);
        if (dal_cursor_handle != NULL)
          dmi->CloseCursor(dal_cursor_handle, false);
        return result_code;
      }
      UPLL_LOG_DEBUG("count, instance count (%d) (%d)",
                     count, imp_instance_count);
      if (count != imp_instance_count) {
        UPLL_LOG_INFO("Merge Conflict: Multi domain count not matched");
        DELETE_IF_NOT_NULL(vtn_pm_ckv);
        DELETE_IF_NOT_NULL(vtn_ckv);
        if (dal_cursor_handle != NULL)
          dmi->CloseCursor(dal_cursor_handle, false);
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
      DELETE_IF_NOT_NULL(vtn_ckv);
    }
  }
  if (dal_cursor_handle != NULL)
    dmi->CloseCursor(dal_cursor_handle, false);
  DELETE_IF_NOT_NULL(vtn_pm_ckv);
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::MergeValidateIpAddress(ConfigKeyVal* ikey,
                                            upll_keytype_datatype_t dt_type,
                                            DalDmlIntf *dmi,
                                            const char *ctrlr_id,
                                            upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t     result_code = UPLL_RC_SUCCESS;
  MoMgrImpl      *vbr_mgr    = NULL;
  MoMgrImpl      *vrtif_mgr  = NULL;
  ConfigKeyVal  *vrtif_ckv  = NULL;
  ConfigKeyVal  *vbr_ckv    = NULL;
  struct in_addr  ip_addr;
  ip_addr.s_addr = 0;

  if (!GetVal(ikey)) {
    UPLL_LOG_DEBUG("check not required");
    return UPLL_RC_SUCCESS;
  }

  bool host_addr_flag = false;
  /* Saves received ikey vtn_name, vnode_name, ip_addr and prefix_len*/
  if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
    val_vbr_t *vbrval = reinterpret_cast<val_vbr_t*>(GetVal(ikey));
    if ((vbrval->valid[UPLL_IDX_HOST_ADDR_VBR] != UNC_VF_VALID) &&
        (vbrval->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("check not required");
      return UPLL_RC_SUCCESS;
    } else if (vbrval->valid[UPLL_IDX_HOST_ADDR_VBR] == UNC_VF_VALID &&
              vbrval->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] == UNC_VF_VALID) {
      host_addr_flag = true;
    }
    ip_addr.s_addr = vbrval->host_addr.s_addr;
  } else if (ikey->get_key_type() == UNC_KT_VRT_IF) {
    val_vrt_if_t *vrtifval = reinterpret_cast<val_vrt_if_t*>(GetVal(ikey));
    if ((vrtifval->valid[UPLL_IDX_IP_ADDR_VI] != UNC_VF_VALID) &&
        (vrtifval->valid[UPLL_IDX_PREFIXLEN_VI] != UNC_VF_VALID) &&
        (vrtifval->valid[UPLL_IDX_MAC_ADDR_VI] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("check not required");
      return UPLL_RC_SUCCESS;
    } else if (vrtifval->valid[UPLL_IDX_IP_ADDR_VI] == UNC_VF_VALID &&
               vrtifval->valid[UPLL_IDX_PREFIXLEN_VI] == UNC_VF_VALID) {
       host_addr_flag = true;
    }
    ip_addr.s_addr   = vrtifval->ip_addr.s_addr;
  }

  if (host_addr_flag) {
    vrtif_mgr = reinterpret_cast<MoMgrImpl *>(
        const_cast<MoManager*>(GetMoManager(UNC_KT_VRT_IF)));
    if (!vrtif_mgr) {
      UPLL_LOG_DEBUG("Invalid Mgr");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = vrtif_mgr->GetChildConfigKey(vrtif_ckv, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Vrouter If GetChildConfigKey Failed");
      return result_code;
    }
    key_vrt_if_t *vrtif_key = reinterpret_cast<key_vrt_if_t *>(
        vrtif_ckv->get_key());
    vrtif_key->vrt_key.vrouter_name[0] = 0;
    vrtif_key->if_name[0] = 0;
    val_vrt_if_t *vrtif_val = reinterpret_cast<val_vrt_if *>
        (ConfigKeyVal::Malloc(sizeof(val_vrt_if)));
    vrtif_val->valid[UPLL_IDX_IP_ADDR_VI]   = UNC_VF_VALID;
    vrtif_val->ip_addr.s_addr               = ip_addr.s_addr;
    vrtif_ckv->AppendCfgVal(IpctSt::kIpcStValVrtIf, vrtif_val);

    /* Verifies whether the received ikey ip_address is configured in another
     * vrouter interface. If it is configured for another interface means
     * Semantic error else Checks in to vbridge table */
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr };
    result_code = vrtif_mgr->ReadConfigDB(vrtif_ckv, dt_type, UNC_OP_READ,
                                          dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      if (result_code == UPLL_RC_SUCCESS) {
        if (import_type == UPLL_IMPORT_TYPE_FULL) {
          UPLL_LOG_INFO("Same Ip Address is already configured for another"
                         " vrouter interface");
          DELETE_IF_NOT_NULL(vrtif_ckv);
          return UPLL_RC_ERR_MERGE_CONFLICT;
        } else {
          uint8_t *ctr = NULL;
          GET_USER_DATA_CTRLR(vrtif_ckv, ctr);
          if (ctr) {
            if (strcmp(reinterpret_cast<char*>(ctr), ctrlr_id)) {
              UPLL_LOG_INFO("Same host address is configured for another"
                             " controller vbridge");
              DELETE_IF_NOT_NULL(vrtif_ckv);
              return UPLL_RC_ERR_MERGE_CONFLICT;
            }
          }
        }
      }
      DELETE_IF_NOT_NULL(vrtif_ckv);
      return result_code;
    }
    DELETE_IF_NOT_NULL(vrtif_ckv);

    vbr_mgr = reinterpret_cast<MoMgrImpl *>(
        const_cast<MoManager*>(GetMoManager(UNC_KT_VBRIDGE)));
    if (!vbr_mgr) {
      UPLL_LOG_DEBUG("Invalid Mgr");
      return UPLL_RC_ERR_GENERIC;
    }

    result_code = vbr_mgr->GetChildConfigKey(vbr_ckv, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("VBridge GetChildConfigKey Failed");
      return result_code;
    }
    key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>(
        vbr_ckv->get_key());
    vbr_key->vbridge_name[0] = 0;

    val_vbr_t *vbr_val = reinterpret_cast<val_vbr_t *>
        (ConfigKeyVal::Malloc(sizeof(val_vbr_t)));
    vbr_val->valid[UPLL_IDX_HOST_ADDR_VBR] = UNC_VF_VALID;
    vbr_val->host_addr.s_addr = ip_addr.s_addr;
    vbr_ckv->AppendCfgVal(IpctSt::kIpcStValVbr, vbr_val);

    /* Verifies whether the received ikey ip_address is configured in another
     * vrouter interface. If it is configured for another
     * interface means Semantic error else Checks in to vbridge table */
    // SET_USER_DATA(vbr_ckv, ikey);
    result_code = vbr_mgr->ReadConfigDB(vbr_ckv, dt_type, UNC_OP_READ,
                                        dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      if (result_code == UPLL_RC_SUCCESS) {
        if (import_type == UPLL_IMPORT_TYPE_FULL) {
          UPLL_LOG_INFO("Same Ip Address is already configured for another"
                        " vbridge");
          DELETE_IF_NOT_NULL(vbr_ckv);
          return UPLL_RC_ERR_MERGE_CONFLICT;
        } else {
          uint8_t *ctr = NULL;
          GET_USER_DATA_CTRLR(vbr_ckv, ctr);
          if (ctr) {
            if (strcmp(reinterpret_cast<char*>(ctr), ctrlr_id)) {
              UPLL_LOG_INFO("Same host address is configured for another"
                            " controller vbridge");
              DELETE_IF_NOT_NULL(vbr_ckv);
              return UPLL_RC_ERR_MERGE_CONFLICT;
            }
          }
        }
      }
      DELETE_IF_NOT_NULL(vbr_ckv);
      return result_code;
    }
    DELETE_IF_NOT_NULL(vbr_ckv);
  }
  if (ikey->get_key_type() == UNC_KT_VRT_IF) {
    val_vrt_if_t *vrtifval = reinterpret_cast<val_vrt_if_t*>(GetVal(ikey));
    if  (vrtifval->valid[UPLL_IDX_MAC_ADDR_VI] == UNC_VF_VALID) {
      result_code = GetChildConfigKey(vrtif_ckv, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Vrouter If GetChildConfigKey Failed");
        return result_code;
      }
      key_vrt_if_t *vrtif_key = reinterpret_cast<key_vrt_if_t *>(
          vrtif_ckv->get_key());
      vrtif_key->vrt_key.vrouter_name[0] = 0;
      vrtif_key->if_name[0] = 0;
      val_vrt_if_t *vrtif_val = reinterpret_cast<val_vrt_if *>
        (ConfigKeyVal::Malloc(sizeof(val_vrt_if)));
      vrtif_val->valid[UPLL_IDX_MAC_ADDR_VI]   = UNC_VF_VALID;
      memcpy(vrtif_val->macaddr, vrtifval->macaddr, sizeof(vrtif_val->macaddr));
      vrtif_ckv->AppendCfgVal(IpctSt::kIpcStValVrtIf, vrtif_val);

      /* Verifies whether the received ikey macaddr is configured in another
       * vrouter interface. If it is configured for another interface means
       * conflict error for full import else if controller is
       * different conflict error in partial import */
      DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr };
      result_code = ReadConfigDB(vrtif_ckv, dt_type, UNC_OP_READ,
                                 dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        if (result_code == UPLL_RC_SUCCESS) {
          if (import_type == UPLL_IMPORT_TYPE_FULL) {
            UPLL_LOG_INFO("Same MAC Address is already configured for another"
                " vrouter interface");
            DELETE_IF_NOT_NULL(vrtif_ckv);
            return UPLL_RC_ERR_MERGE_CONFLICT;
          } else {
            uint8_t *ctr = NULL;
            GET_USER_DATA_CTRLR(vrtif_ckv, ctr);
            if (ctr) {
              if (strcmp(reinterpret_cast<char*>(ctr), ctrlr_id)) {
                UPLL_LOG_INFO("Same MAC address is configured for another"
                    " controller vrouter interface");
                DELETE_IF_NOT_NULL(vrtif_ckv);
                return UPLL_RC_ERR_MERGE_CONFLICT;
              }
            }
          }
        }
        DELETE_IF_NOT_NULL(vrtif_ckv);
        return result_code;
      }
      DELETE_IF_NOT_NULL(vrtif_ckv);
    }
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t MoMgrImpl::BindKeyAndValForMerge(DalBindInfo *db_info,
                                   upll_keytype_datatype_t dt_type,
                                   MoMgrTables tbl,
                                   const uudst::kDalTableIndex index) {
  UPLL_FUNC_TRACE;
  int nattr;
  BindInfo *binfo;
  uint8_t index_flag = 0;

  if (!GetBindInfo(tbl, dt_type, binfo, nattr)) {
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t key_type = table[MAINTBL]->get_key_type();

  for (int i = 0; i < nattr; i++) {
    uint64_t indx = binfo[i].index;
    BindStructTypes attr_type = binfo[i].struct_type;

    if ((MAINTBL == tbl) && (index_flag != 2)) {
      if (UNC_KT_VBRIDGE == key_type) {
        if (indx == uudst::vbridge::kDbiVbrDesc ||
            indx ==  uudst::vbridge::kDbiValidVbrDesc) {
          UPLL_LOG_TRACE(
              "Skipping vbridge description index %"PFC_PFMT_u64"", indx);
          index_flag++;
          continue;
        }
      } else if (UNC_KT_VTN == key_type) {
        if (indx ==  uudst::vtn::kDbiVtnDesc ||
            indx == uudst::vtn::kDbiValidVtnDesc) {
          index_flag++;
          continue;
        }
      } else if (UNC_KT_VBR_IF == key_type) {
        if (indx == uudst::vbridge_interface::kDbiDesc ||
            indx == uudst::vbridge_interface::kDbiValidDesc) {
          index_flag++;
          continue;
        }
      } else if (UNC_KT_VROUTER == key_type) {
        if (indx == uudst::vrouter::kDbiVrtDesc ||
            indx == uudst::vrouter::kDbiValidDesc) {
          index_flag++;
          continue;
        }
      } else if (UNC_KT_VRT_IF == key_type) {
        if (indx == uudst::vrouter_interface::kDbiDesc ||
            indx == uudst::vrouter_interface::kDbiValidDesc) {
          index_flag++;
          continue;
        }
      } else if (UNC_KT_VLINK == key_type) {
        if (indx == uudst::vlink::kDbiDesc ||
            indx == uudst::vlink::kDbiValidDesc) {
          index_flag++;
          continue;
        }
      } else if (UNC_KT_VTERMINAL == key_type) {
        if (indx == uudst::vterminal::kDbiVterminalDesc ||
            indx == uudst::vterminal::kDbiValidVterminalDesc) {
          index_flag++;
          continue;
        }
      } else if (UNC_KT_VTERM_IF == key_type) {
        if (indx == uudst::vterminal_interface::kDbiDesc ||
            indx == uudst::vterminal_interface::kDbiValidDesc) {
          index_flag++;
          continue;
        }
      }
    }
    size_t size;

    switch (binfo[i].app_data_type) {
      case uud::kDalChar:
        size = sizeof(int8_t);
        break;
      case uud::kDalUint8:
        size = sizeof(uint8_t);
        break;
      case uud::kDalUint16:
        size = sizeof(uint16_t);
        break;
      case uud::kDalUint32:
        size = sizeof(uint32_t);
        break;
      case uud::kDalUint64:
        size = sizeof(uint64_t);
        break;
      default:
        UPLL_LOG_ERROR("Invalid DalCDataType");
        return UPLL_RC_ERR_GENERIC;
    }

    void *dummy = ConfigKeyVal::Malloc(size * binfo[i].array_size);

    UPLL_LOG_TRACE("Bind Match for the column");
    if (MAINTBL == tbl && (table[MAINTBL]->get_key_type() == UNC_KT_VBRIDGE ||
        table[MAINTBL]->get_key_type() == UNC_KT_VROUTER ||
        table[MAINTBL]->get_key_type() == UNC_KT_VTERMINAL ||
        table[MAINTBL]->get_key_type() == UNC_KT_VBR_PORTMAP)) {
      if (attr_type != CS_VAL && attr_type != ST_VAL &&
          attr_type != CFG_DEF_VAL &&  // to ignore valid_admin_status
          attr_type != ST_META_VAL &&
          attr_type != CFG_ST_META_VAL &&
          attr_type != CFG_ST_VAL) {
        UPLL_LOG_TRACE("Bind for attr type %d", attr_type);
        if (attr_type == CK_VAL &&
            ((table[MAINTBL]->get_key_type() == UNC_KT_VBRIDGE &&
             (indx == uudst::vbridge::kDbiVbrFlags)) ||
            (table[MAINTBL]->get_key_type() == UNC_KT_VROUTER &&
             indx == uudst::vrouter::kDbiVrtFlags) ||
            (table[MAINTBL]->get_key_type() == UNC_KT_VTERMINAL &&
             indx == uudst::vterminal::kDbiVterminalFlags) ||
            (table[MAINTBL]->get_key_type() == UNC_KT_VBR_PORTMAP &&
             indx == uudst::vbridge_portmap::kDbiVbrPortMapFlags))) {
          // don't do any thing.
        } else if (attr_type == CK_VAL) {  // continue the ctrlr and domain
          FREE_IF_NOT_NULL(dummy);
          continue;
        }
        db_info->BindMatch(indx, binfo[i].app_data_type,
                           binfo[i].array_size, dummy);
      }
    } else {
          if ((attr_type == CFG_KEY) ||
              (attr_type == CFG_VAL) ||
              (CK_VAL == attr_type ||
               CK_VAL2 == attr_type) ||
              (CFG_META_VAL == attr_type) ||
              (attr_type == CFG_DEF_VAL)) {
        UPLL_LOG_TRACE("Bind for attr type %d", attr_type);
        if ((UNC_KT_VBR_IF == key_type || UNC_KT_VLINK == key_type) &&
            CFG_DEF_VAL == attr_type) {
          // do nothing
        } else if (CFG_DEF_VAL == attr_type) {
          FREE_IF_NOT_NULL(dummy);
          continue;
        }
        db_info->BindMatch(indx, binfo[i].app_data_type,
                           binfo[i].array_size, dummy);
      }
    }
  FREE_IF_NOT_NULL(dummy);
  }

  return UPLL_RC_SUCCESS;
}

// PC start
upll_rc_t MoMgrImpl::PerformModeSpecificSemanticCheck(ConfigKeyVal *ikey,
                                           DalDmlIntf *dmi,
                                           uint32_t session_id,
                                           uint32_t config_id,
                                           unc_keytype_operation_t operation,
                                           unc_key_type_t keytype,
                                           TcConfigMode config_mode,
                                           string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  IpcReqRespHeader req;
  memset(&req, 0, sizeof(IpcReqRespHeader));
  req.datatype = UPLL_DT_RUNNING;
  req.operation = operation;
  req.clnt_sess_id = session_id;
  req.config_id = config_id;

  DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutNone};
  switch (operation) {
    case UNC_OP_DELETE:
      if (keytype == UNC_KT_FLOWLIST ||
          keytype == UNC_KT_FLOWLIST_ENTRY ||
          keytype == UNC_KT_POLICING_PROFILE ||
          keytype == UNC_KT_POLICING_PROFILE_ENTRY) {
        // result_code = IsReferenced(&req, ikey, dmi);
      }
    break;
    case UNC_OP_CREATE:
      if (keytype == UNC_KT_FLOWLIST || keytype == UNC_KT_FLOWLIST_ENTRY ||
          keytype == UNC_KT_POLICING_PROFILE ||
          keytype == UNC_KT_POLICING_PROFILE_ENTRY) {
        result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING,
                                     UNC_OP_READ, dmi, &dbop, MAINTBL);
        if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
          UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
          if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              result_code = UPLL_RC_ERR_CFG_SEMANTIC;
          }
          return result_code;
        }
        if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS)
          result_code = UPLL_RC_SUCCESS;
      } else if (keytype == UNC_KT_VBRIDGE || keytype == UNC_KT_VROUTER ||
                 keytype == UNC_KT_VTERMINAL || keytype == UNC_KT_VUNKNOWN ||
                 keytype == UNC_KT_VTEP || keytype == UNC_KT_VTUNNEL) {
        result_code = CtrlrTypeAndDomainCheck(ikey, &req);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("CtrlrTypeAndDomainCheck failed %d", result_code);
          return result_code;
        }
      }
    case UNC_OP_UPDATE:
      if (keytype == UNC_KT_VTN_FLOWFILTER_ENTRY ||
          keytype == UNC_KT_VBR_FLOWFILTER_ENTRY ||
          keytype == UNC_KT_VBRIF_FLOWFILTER_ENTRY ||
          keytype == UNC_KT_VRTIF_FLOWFILTER_ENTRY ||
          keytype == UNC_KT_VTERMIF_FLOWFILTER_ENTRY ||
          keytype == UNC_KT_VBR_VLANMAP || keytype == UNC_KT_VBR_PORTMAP ||
          keytype == UNC_KT_VLINK ||
          keytype == UNC_KT_VBR_IF ||
          keytype == UNC_KT_VTERM_IF || keytype == UNC_KT_FLOWLIST_ENTRY ||
          keytype == UNC_KT_POLICING_PROFILE_ENTRY) {
        result_code = ValidateAttribute(ikey, dmi, &req);
        if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code)
          result_code = UPLL_RC_SUCCESS;
      } else if (keytype == UNC_KT_VTN_POLICINGMAP ||
                 keytype == UNC_KT_VBR_POLICINGMAP  ||
                 keytype == UNC_KT_VBRIF_POLICINGMAP ||
                 keytype == UNC_KT_VTERMIF_POLICINGMAP) {
        result_code = IsPolicyProfileReferenced(ikey, UPLL_DT_RUNNING,
                                                  dmi, UNC_OP_READ);
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          result_code = UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
      break;
     default:
       return UPLL_RC_SUCCESS;
  }
  return result_code;
}

upll_rc_t MoMgrImpl::TxComputeOperStatusandCommit(unc_key_type_t key_type,
                            CtrlrCommitStatusList *ctrlr_commit_status,
                            unc_keytype_operation_t op,
                            DalDmlIntf* dmi, TcConfigMode config_mode,
                            std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ckey = NULL;
  DalCursor *cfg1_cursor = NULL;
  DalResultCode db_result = uud::kDalRcSuccess;
  upll_keytype_datatype_t dt_type = UPLL_DT_STATE;
  if (op == UNC_OP_DELETE)
    dt_type = UPLL_DT_RUNNING;

  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  if (ctrlr_commit_status != NULL) {
  for (ccsListItr = ctrlr_commit_status->begin();
        ccsListItr != ctrlr_commit_status->end(); ++ccsListItr) {
      ccStatusPtr = *ccsListItr;
      ctrlr_result[ccStatusPtr->ctrlr_id] = ccStatusPtr->upll_ctrlr_result;
    }
  }

  result_code= DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op,
        req, nreq, &cfg1_cursor, dmi, NULL, config_mode, vtn_name,
        CONVERTTBL, true);
  while (result_code == UPLL_RC_SUCCESS) {
    db_result = dmi->GetNextRecord(cfg1_cursor);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS)
      break;
    if (op == UNC_OP_DELETE) {
      if (key_type == UNC_KT_VTN)
        result_code = GetChildConfigKey(ckey, req);
      else
        result_code = GetChildConvertConfigKey(ckey, req);
    } else {
      result_code = DupConfigKeyVal(ckey, req, CONVERTTBL);
    }
    if (result_code != UPLL_RC_SUCCESS)
      break;
    if (op != UNC_OP_DELETE) {
      if (key_type == UNC_KT_VTN) {
        uint8_t *ctrlr = NULL;
        GET_USER_DATA_CTRLR(ckey, ctrlr);
        if (ctrlr_result[reinterpret_cast<char*>(ctrlr)] != UPLL_RC_SUCCESS) {
          result_code = UpdateUVbrConfigStatusFromVtnGwPort(ckey, dmi,
                                                      config_mode, vtn_name);
          if (result_code != UPLL_RC_SUCCESS) {
            DELETE_IF_NOT_NULL(ckey);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            UPLL_LOG_DEBUG("Returning error %d", result_code);
            return UPLL_RC_ERR_GENERIC;
          }
        }
      }
      uint32_t driver_result;
      if (ctrlr_result.empty()) {
        UPLL_LOG_TRACE("ctrlr_commit_status is NULL");
        driver_result = UPLL_RC_ERR_CTR_DISCONNECTED;
      } else {
        uint8_t *ctrlr = NULL;
        GET_USER_DATA_CTRLR(ckey, ctrlr);
        driver_result = ctrlr_result[reinterpret_cast<char *>(ctrlr)];
      }
      if (op == UNC_OP_CREATE &&
        key_type == UNC_KT_VTN) {
        result_code = UpdateGWPortStatus(ckey, dmi, driver_result);
      } else {
        result_code = UpdateConfigStatus(ckey, op,
                      driver_result, nreq, dmi);
      }
    } else {
      if (key_type != UNC_KT_VTN) {
        result_code = UpdateParentOperStatus(ckey, dmi, UPLL_RC_SUCCESS);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(ckey);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          UPLL_LOG_DEBUG("Returning error %d\n", result_code);
          return result_code;
        }
      }
    }

    if (!(key_type == UNC_KT_VBR_IF &&
        (op == UNC_OP_CREATE || op == UNC_OP_UPDATE))) {
      // vbr_if update will be handled as part of setinterface operstatus
      DbSubOp dbop_update = {kOpNotRead, kOpMatchNone,
                           kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain};
      if ((key_type == UNC_KT_VTN) &&  (op == UNC_OP_UPDATE)) {
        dbop_update.matchop = kOpMatchCtrlr|kOpMatchDomain;
        dbop_update.inoutop = kOpInOutFlag;
      }
      result_code = UpdateConfigDB(ckey, dt_type, op, dmi, &dbop_update,
                                 config_mode, vtn_name, CONVERTTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(ckey);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        if (cfg1_cursor)
          dmi->CloseCursor(cfg1_cursor, true);
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
    }
    DELETE_IF_NOT_NULL(ckey);
  }
  DELETE_IF_NOT_NULL(req);
  DELETE_IF_NOT_NULL(nreq);
  if (cfg1_cursor) {
    dmi->CloseCursor(cfg1_cursor, true);
    cfg1_cursor = NULL;
  }
  // Copying Rename Table to Running
  UPLL_LOG_TRACE("key_type is %d", key_type);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                 UPLL_RC_SUCCESS: result_code;
  return result_code;
}

// Aborts gvtn_id tbl and convert_vtunnel tbl
upll_rc_t MoMgrImpl::CopyVTunnelFromRunningToCandidate(
                          unc_keytype_operation_t op,
                          DalDmlIntf* dmi,
                          TcConfigMode config_mode,
                          std::string cfg_vtn_name) {
  UPLL_FUNC_TRACE;
  DalResultCode db_result = uud::kDalRcSuccess;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *req = NULL, *nreq = NULL, *ckey = NULL;
  DalCursor *cfg1_cursor = NULL;
  unc_keytype_operation_t tmp_op = UNC_OP_UPDATE;
  if (op == UNC_OP_CREATE)
    tmp_op = UNC_OP_DELETE;
  else if (op == UNC_OP_DELETE)
    tmp_op = UNC_OP_CREATE;
  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, tmp_op,
                             req, nreq, &cfg1_cursor, dmi, NULL,
                             config_mode, cfg_vtn_name, CONVERTTBL, true);
  while (result_code == UPLL_RC_SUCCESS) {
    db_result = dmi->GetNextRecord(cfg1_cursor);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS)
      break;
    result_code = DupConfigKeyVal(ckey, req, CONVERTTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Error in DupConfigKeyVal: %d", result_code);
      break;
    }
    VbrPortMapMoMgr *mgr = (reinterpret_cast<VbrPortMapMoMgr *>
                           (const_cast<MoManager *>(
                            GetMoManager(UNC_KT_VBR_PORTMAP))));
    if (!mgr) {
      UPLL_LOG_ERROR("Invalid mgr");
      result_code = UPLL_RC_ERR_GENERIC;
      break;
    }
    if (op == UNC_OP_CREATE) {
      result_code = mgr->AllocGvtnId(ckey, dmi, UPLL_DT_CANDIDATE,
                                     config_mode, cfg_vtn_name);
    } else if (op == UNC_OP_DELETE) {
      result_code = mgr->DeAllocGvtnId(ckey, dmi, UPLL_DT_CANDIDATE,
                                       config_mode, cfg_vtn_name);
    } else if (op == UNC_OP_UPDATE) {
      result_code = mgr->DeAllocGvtnId(ckey, dmi, UPLL_DT_CANDIDATE,
                                       config_mode, cfg_vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("AllocGvtnId error: %d", result_code);
        break;
      }
      result_code = mgr->AllocGvtnId(nreq, dmi, UPLL_DT_CANDIDATE,
                                     config_mode, cfg_vtn_name);
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Alloc or DeAllocGvtnId error:%d op:%d config_mode:%d",
                    config_mode, result_code, op);
      break;
    }
    DbSubOp dbop_update = {kOpNotRead, kOpMatchNone,
                           kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain};
    if (op == UNC_OP_UPDATE) {
      result_code = UpdateConfigDB(nreq, UPLL_DT_CANDIDATE, op, dmi,
                      &dbop_update, config_mode, cfg_vtn_name, CONVERTTBL);
    } else {
      result_code = UpdateConfigDB(ckey, UPLL_DT_CANDIDATE, op, dmi,
                      &dbop_update, config_mode, cfg_vtn_name, CONVERTTBL);
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("UpdateConfigDB error: %d", result_code);
      break;
    }
    DELETE_IF_NOT_NULL(ckey);
  }
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  } else if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(ckey);
    UPLL_LOG_INFO("DiffConfigDB failed. result:%d", result_code);
  }
  DELETE_IF_NOT_NULL(req);
  DELETE_IF_NOT_NULL(nreq);
  if (cfg1_cursor) {
    dmi->CloseCursor(cfg1_cursor, true);
    cfg1_cursor = NULL;
  }
  return result_code;
}

upll_rc_t MoMgrImpl::TxCopyConvertTblFromCandidateToRunning(
                          unc_keytype_operation_t op,
                          CtrlrCommitStatusList *ctrlr_commit_status,
                          DalDmlIntf* dmi,
                          TcConfigMode config_mode,
                          std::string vtn_name) {
  UPLL_FUNC_TRACE;

  //  convert tbl keytypes
  unc_key_type_t convert_key[]= {
    UNC_KT_VTN, UNC_KT_VBRIDGE, UNC_KT_VBR_IF, UNC_KT_VBR_PORTMAP,
    UNC_KT_VTUNNEL, UNC_KT_VTUNNEL_IF, UNC_KT_VLINK
  };

  unc_key_type_t keytype;
  DalResultCode db_result = uud::kDalRcSuccess;
  upll_rc_t     result_code = UPLL_RC_SUCCESS;

  for (uint8_t i = 0; i < sizeof(convert_key) / sizeof(unc_key_type_t); i++) {
    keytype = convert_key[i];
    MoMgrTables tbl = CONVERTTBL;
    if (keytype == UNC_KT_VBR_PORTMAP) {
      tbl = VBIDTBL;
    }
    MoMgrImpl *mgr = (reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
                GetMoManager(keytype))));
    if (!mgr) {
      UPLL_LOG_ERROR("Invalid mgr");
      return UPLL_RC_ERR_GENERIC;
    }

    ConfigKeyVal *req = NULL, *nreq = NULL, *ckey = NULL;
    DalCursor *cfg1_cursor = NULL;
    if (keytype == UNC_KT_VTUNNEL || keytype == UNC_KT_VTUNNEL_IF ||
        keytype == UNC_KT_VLINK || keytype == UNC_KT_VBR_PORTMAP ||
       (op == UNC_OP_DELETE && keytype == UNC_KT_VTN)) {
      result_code= mgr->DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op,
                                     req, nreq, &cfg1_cursor, dmi, NULL,
                                     config_mode, vtn_name, tbl, true);
      while (result_code == UPLL_RC_SUCCESS) {
        db_result = dmi->GetNextRecord(cfg1_cursor);
        result_code = DalToUpllResCode(db_result);
        if (result_code != UPLL_RC_SUCCESS)
          break;
        if (op == UNC_OP_DELETE) {
          if (keytype == UNC_KT_VTN)
            result_code = mgr->GetChildConfigKey(ckey, req);
          else if (keytype == UNC_KT_VBR_PORTMAP)
            result_code = mgr->GetVbIdChildConfigKey(ckey, req);
          else
            result_code = mgr->GetChildConvertConfigKey(ckey, req);
        } else {
          result_code = mgr->DupConfigKeyVal(ckey, req, tbl);
        }
        if (result_code != UPLL_RC_SUCCESS)
          break;
        DbSubOp dbop_update = {kOpNotRead, kOpMatchNone,
          kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain};
        if (keytype == UNC_KT_VTN) {
          dbop_update.matchop = kOpMatchCtrlr | kOpMatchDomain;
        }

        if (keytype == UNC_KT_VTUNNEL) {
          VbrPortMapMoMgr *vbrpm_mgr = (reinterpret_cast<VbrPortMapMoMgr *>(
                    const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_PORTMAP))));
          if (op == UNC_OP_CREATE) {
            result_code = vbrpm_mgr->AllocGvtnId(req, dmi, UPLL_DT_RUNNING,
                                                 config_mode, vtn_name);
          } else if (op == UNC_OP_DELETE) {
            result_code = vbrpm_mgr->DeAllocGvtnId(req, dmi, UPLL_DT_RUNNING,
                                                   config_mode, vtn_name);
          } else {
            result_code = vbrpm_mgr->DeAllocGvtnId(nreq, dmi, UPLL_DT_RUNNING,
                                                   config_mode, vtn_name);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_INFO("DeAllocGvtnId error: %d", result_code);
              DELETE_IF_NOT_NULL(ckey);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              if (cfg1_cursor)
                dmi->CloseCursor(cfg1_cursor, true);
              return result_code;
            }
            result_code = vbrpm_mgr->AllocGvtnId(req, dmi, UPLL_DT_RUNNING,
                                                 config_mode, vtn_name);
          }
          if (result_code != UPLL_RC_SUCCESS) {
            DELETE_IF_NOT_NULL(ckey);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            if (cfg1_cursor)
              dmi->CloseCursor(cfg1_cursor, true);
            UPLL_LOG_DEBUG("Returning error %d", result_code);
            return result_code;
          }
        }
        result_code = mgr->UpdateConfigDB(ckey, UPLL_DT_RUNNING, op, dmi,
                          &dbop_update, config_mode, vtn_name, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(ckey);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          if (cfg1_cursor)
            dmi->CloseCursor(cfg1_cursor, true);
          UPLL_LOG_DEBUG("Returning error %d", result_code);
          return result_code;
        }
        DELETE_IF_NOT_NULL(ckey);
      }
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      if (cfg1_cursor) {
        dmi->CloseCursor(cfg1_cursor, true);
        cfg1_cursor = NULL;
      }
    } else {
      result_code = mgr->TxComputeOperStatusandCommit(keytype,
                                  ctrlr_commit_status, op, dmi,
                                  config_mode, vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Error in TxComputeOperStatusandCommit:%d", result_code);
        return result_code;
      }
    }
  }  //  end of keytype for loop
  return result_code;
}


bool MoMgrImpl::ThresholdAlarmProcessingRequired(ConfigKeyVal *can_key,
                                                 ConfigKeyVal *run_key,
                                                 unc_keytype_operation_t op,
                                                 TcConfigMode config_mode) {
  UPLL_FUNC_TRACE;
  if (can_key->get_key_type()  == UNC_KT_UNW_LABEL) {
    val_unw_label *ca_val = reinterpret_cast<val_unw_label *>(GetVal(can_key));
    val_unw_label *ru_val = reinterpret_cast<val_unw_label *>(GetVal(run_key));
    // 1. If falling threshold value in candidate and runing is not same
    // 2. if raising threshold value in candidate and running is not the same
    if (op == UNC_OP_CREATE) {
        // If label is created with non default values
        if ((ca_val->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL] == UNC_VF_VALID) ||
            (ca_val->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] == UNC_VF_VALID))
          return true;
    } else {
      // For Update operation if any of the falling or raising threshold is
      // changed
      if ((ca_val->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL] !=
           ru_val->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL]) ||
          ((ca_val->valid[UPLL_IDX_RAISING_THRESHOLD_UNWL] == UNC_VF_VALID) &&
           (ca_val->raising_threshold  != ru_val->raising_threshold))) {
        UPLL_LOG_TRACE("Alarm check is required.raising-threshold is changed");
        return true;
      }
      if ((ca_val->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] !=
          ru_val->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL]) ||
          ((ca_val->valid[UPLL_IDX_FALLING_THRESHOLD_UNWL] == UNC_VF_VALID) &&
           (ca_val->falling_threshold != ru_val->falling_threshold))) {
        UPLL_LOG_TRACE("Alarm check is required.falling-threshold is changed");
        return true;
      }
    }
  } else if (can_key->get_key_type()  == UNC_KT_UNW_SPINE_DOMAIN) {
    val_unw_spdom_ext *ca_val = reinterpret_cast<val_unw_spdom_ext*>
                                                 (GetVal(can_key));

    val_unw_spdom_ext *ru_val = reinterpret_cast<val_unw_spdom_ext*>
                                                   (GetVal(run_key));

      if (op == UNC_OP_CREATE) {
      // if label is assigned
      if (ca_val->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS] ==
          UNC_VF_VALID) {
        return true;
      }
      //  check for used count for global config mode only
       if ((config_mode == TC_CONFIG_GLOBAL) &&
           (ca_val->used_label_count > 0)) {
        return true;
       }
    } else {
      // For Update Operation Need to check for used count
      // based on config mode
       if ((ca_val->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS] !=
            ru_val->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS]) ||
           ((ca_val->val_unw_spine_dom.valid[UPLL_IDX_UNW_LABEL_ID_UNWS] ==
             UNC_VF_VALID) &&
            (ca_val->val_unw_spine_dom.unw_label_id !=
             ru_val->val_unw_spine_dom.unw_label_id))) {
         return true;
       }
       if (config_mode == TC_CONFIG_GLOBAL) {
          if ((ca_val->valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] !=
               ru_val->valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS]) ||
              ((ca_val->valid[UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS] ==
                UNC_VF_VALID) &&
              (ca_val->used_label_count != ru_val->used_label_count))) {
            return true;
          }
       }
    }
  }
  return false;
}

upll_rc_t MoMgrImpl::UpdateGWPortStatus(ConfigKeyVal *ckey, DalDmlIntf *dmi,
                                        uint32_t driver_result) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_db_vtn_st *val_vtnst = reinterpret_cast<val_db_vtn_st *>
        (ConfigKeyVal::Malloc(sizeof(val_db_vtn_st)));
  if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
    val_vtnst->vtn_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
  } else {
    controller_domain_t ctrlr_dom = {NULL, NULL};
    val_oper_status port_oper_status = UPLL_OPER_STATUS_UNINIT;
    GET_USER_DATA_CTRLR_DOMAIN(ckey, ctrlr_dom);
    val_port_map_t pm;
    val_vtn_gateway_port *gw_port_val =
        reinterpret_cast<val_vtn_gateway_port *>(GetVal(ckey));
    pm.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
    uuu::upll_strncpy(pm.logical_port_id, gw_port_val->logical_port_id,
                kMaxLenLogicalPortId+1);
    VbrPortMapMoMgr *mgr = reinterpret_cast<VbrPortMapMoMgr *>
                    (const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_PORTMAP)));
    result_code = mgr->GetPortStatusFromPhysical(&pm, ctrlr_dom,
                                            port_oper_status);

    val_vtnst->vtn_val_st.oper_status = port_oper_status;
  }
  val_vtnst->vtn_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
  ckey->AppendCfgVal(IpctSt::kIpcStValVtnSt, val_vtnst);

  return result_code;
}


upll_rc_t MoMgrImpl::UpdateUVbrConfigStatusFromVtnGwPort(ConfigKeyVal *ikey,
                          DalDmlIntf* dmi, TcConfigMode config_mode,
                          std::string vtn_name) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VbrMoMgr *mgr = static_cast<VbrMoMgr *>(const_cast<MoManager *>(
          GetMoManager(UNC_KT_VBRIDGE)));
  if (!mgr) {
    return result_code;
  }
  ConfigKeyVal *conv_vbr_ckv = NULL;
  result_code = mgr->GetChildConvertConfigKey(conv_vbr_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  key_convert_vbr_t *convert_vbr_key = reinterpret_cast<key_convert_vbr_t*>(
      conv_vbr_ckv->get_key());
  uuu::upll_strncpy(convert_vbr_key->vbr_key.vtn_key.vtn_name,
                  reinterpret_cast<key_vtn_t*>(ikey->get_key())->vtn_name,
                  kMaxLenVtnName + 1);
  DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone};
  controller_domain_t ctrlr_dom ={NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(conv_vbr_ckv, ctrlr_dom);
  result_code = mgr->ReadConfigDB(conv_vbr_ckv, UPLL_DT_CANDIDATE,
                                  UNC_OP_READ, dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(conv_vbr_ckv);
    return result_code;
  }

  ConfigKeyVal *vbr_ckv = NULL;
  result_code = mgr->GetChildConfigKey(vbr_ckv, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(conv_vbr_ckv);
    return result_code;
  }

  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t*>(vbr_ckv->get_key());
  uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                  convert_vbr_key->vbr_key.vtn_key.vtn_name,
                  kMaxLenVtnName + 1);
  uuu::upll_strncpy(vbr_key->vbridge_name,
                  convert_vbr_key->vbr_key.vbridge_name, kMaxLenVnodeName + 1);
  SET_USER_DATA_CTRLR(vbr_ckv, "#");
  SET_USER_DATA_DOMAIN(vbr_ckv, "#");

  DbSubOp dbop1 = {kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain, kOpInOutCs};
  result_code = mgr->ReadConfigDB(vbr_ckv, UPLL_DT_RUNNING,
                             UNC_OP_READ, dbop1, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(conv_vbr_ckv);
    DELETE_IF_NOT_NULL(vbr_ckv);
    return result_code;
  }

  val_vbr_t *vbr_val = reinterpret_cast<val_vbr_t*>(GetVal(vbr_ckv));
  if (vbr_val->cs_row_status == UNC_CS_APPLIED) {
    vbr_val->cs_row_status = UNC_CS_PARTIALLY_APPLIED;
    DbSubOp update_dbop = { kOpNotRead, kOpMatchNone, kOpInOutCs};
    result_code = mgr->UpdateConfigDB(vbr_ckv, UPLL_DT_RUNNING,
                                 UNC_OP_UPDATE, dmi, &update_dbop,
                                 config_mode, vtn_name, MAINTBL);
  }
  DELETE_IF_NOT_NULL(vbr_ckv);
  DELETE_IF_NOT_NULL(conv_vbr_ckv);
  return result_code;
}

// Input: ikey is VBR_IF, convert VBR_IF, VBR_PORTMAP, VTERM_IF, VLINK
// Unique VTNs will be added to vtn_list set provided by user.
upll_rc_t MoMgrImpl::GetUniqueVtns(ConfigKeyVal *ikey,
                                   std::set<std::string> *vtn_list,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrTables tbl = MAINTBL;
  if (ikey->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
    tbl = CONVERTTBL;
  }
  const uudst::kDalTableIndex tbl_index = GetTable(tbl, UPLL_DT_RUNNING);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_ERROR("Invalid Table Index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ck_vn = NULL;
  result_code = DupConfigKeyVal(ck_vn, ikey, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Returning %d", result_code);
    return result_code;
  }

  string query_string;
  switch (ck_vn->get_key_type()) {
    case UNC_KT_VBR_IF: {
      if (tbl == MAINTBL)
        query_string = "SELECT DISTINCT vtn_name from ru_vbr_if_tbl";
      else
        query_string = "SELECT DISTINCT vtn_name from ru_convert_vbr_if_tbl";
    }
    break;
    case UNC_KT_VTERM_IF: {
      query_string = "SELECT DISTINCT vtn_name from ru_vterm_if_tbl";
    }
    break;
    case UNC_KT_VBR_PORTMAP: {
      query_string = "SELECT DISTINCT vtn_name from ru_vbr_portmap_tbl";
    }
    break;
    case UNC_KT_VLINK: {
      query_string = "SELECT DISTINCT vtn_name from ru_vlink_tbl";
    }
    break;
    default:
      UPLL_LOG_ERROR("Unexpected keytype");
      DELETE_IF_NOT_NULL(ck_vn);
      return UPLL_RC_ERR_GENERIC;
    break;
  }

  DalBindInfo dal_bind_info(tbl_index);
  BindInfoForPortEvents(&dal_bind_info, ck_vn);
  DalCursor *dal_cursor_handle = NULL;
  result_code = DalToUpllResCode(
      dmi->ExecuteAppQueryMultipleRecords(query_string, 0, &dal_bind_info,
                                          &dal_cursor_handle));
  while (result_code == UPLL_RC_SUCCESS) {
    result_code = DalToUpllResCode(dmi->GetNextRecord(dal_cursor_handle));
    if (UPLL_RC_SUCCESS == result_code) {
      key_vtn *vtn_key = reinterpret_cast<key_vtn*>(ck_vn->get_key());
      vtn_list->insert(reinterpret_cast<char*>(vtn_key->vtn_name));
    }
  }
  DELETE_IF_NOT_NULL(ck_vn);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  } else {
    UPLL_LOG_ERROR("error result code = %d", result_code);
  }
  if (dal_cursor_handle) {
    dmi->CloseCursor(dal_cursor_handle, false);
  }
  return result_code;
}

// Input: ck_vn is VBR_IF, convert VBR_IF, VBR_PORTMAP, VTERM_IF, VLINK
upll_rc_t MoMgrImpl::BindInfoForPortEvents(DalBindInfo *dal_bind_info,
                                           ConfigKeyVal *ck_vn) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  int nattr = 0;
  BindInfo *binfo = NULL;
  MoMgrTables tbl = MAINTBL;
  if (ck_vn->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
    tbl = CONVERTTBL;
  }
  if (!GetBindInfo(tbl, UPLL_DT_RUNNING, binfo, nattr)) {
    UPLL_LOG_ERROR("GetBindInfo failed");
    return UPLL_RC_ERR_GENERIC;
  }

  // index 0 is VTN for the input keytypes.
  void *tkey = ck_vn->get_key();
  uint64_t indx = binfo[0].index;
  void *p = reinterpret_cast<void *>(
      reinterpret_cast<char *>(tkey) + binfo[0].offset);
  // bind vtn_name for output
  dal_bind_info->BindOutput(indx, binfo[0].app_data_type,
                            binfo[0].array_size, p);

  // TODO(later): need to remove hardcoding of indices
  // don't bind ctrlr_dom for vlink
  if (ck_vn->get_key_type() != UNC_KT_VLINK) {
    // set keytype specific indexes to match ctrlr_dom from user_data
    uint8_t min_user_data_index = 0, max_user_data_index = 0;
    switch (ck_vn->get_key_type()) {
      case UNC_KT_VBR_PORTMAP:
        min_user_data_index = 9;
        max_user_data_index = 11;
      break;
      case UNC_KT_VTERM_IF:
        min_user_data_index = 10;
        max_user_data_index = 12;
      break;
      case UNC_KT_VBR_IF: {
        if (tbl == MAINTBL) {
          min_user_data_index = 13;
          max_user_data_index = 15;
        } else {
          min_user_data_index = 6;
          max_user_data_index = 8;
        }
      }
      break;
      // case UNC_KT_VLINK:
      default:
        UPLL_LOG_ERROR("Unexpected key type received");
        return UPLL_RC_ERR_GENERIC;
      break;
    }
    tkey = ck_vn->get_user_data();
    for (uint8_t user_index = min_user_data_index;
         user_index < max_user_data_index; user_index++) {
      indx = binfo[user_index].index;
      p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
                   + binfo[user_index].offset);
      // bind controller-domain as match
      dal_bind_info->BindMatch(indx, binfo[user_index].app_data_type,
                binfo[user_index].array_size, p);
    }
  }
  // For interface and vbr portmap, bind logical-port-id for match.
  // For vlink, bind boundary name for match.
  if (tbl == MAINTBL) {
    tkey = GetVal(ck_vn);
    switch (ck_vn->get_key_type()) {
      case UNC_KT_VBR_PORTMAP:
      case UNC_KT_VTERM_IF:
      case UNC_KT_VBR_IF:
        indx = binfo[5].index;
        p = reinterpret_cast<void *>(
            reinterpret_cast<char *>(tkey) + binfo[5].offset);
        // bind controller-domain as match and output
        dal_bind_info->BindMatch(indx, binfo[5].app_data_type,
                                 binfo[5].array_size, p);
      break;
      case UNC_KT_VLINK:
        indx = binfo[7].index;
        p = reinterpret_cast<void *>(
            reinterpret_cast<char *>(tkey) + binfo[7].offset);
        // bind controller-domain as match and output
        dal_bind_info->BindMatch(indx, binfo[7].app_data_type,
                                 binfo[7].array_size, p);
        // dal_bind_info->BindMatch(indx, binfo[22].app_data_type,
        //                          binfo[22].array_size, p);
      break;
      default:
        UPLL_LOG_ERROR("Unexpected key type received");
        return UPLL_RC_ERR_GENERIC;
      break;
    }
  }
  return result_code;
}

#if 0
template upll_rc_t
MoMgrImpl::GetUninitOperState<val_vlink_st_t, val_db_vlink_st_t>
                        (ConfigKeyVal *&ck_vn, DalDmlIntf   *dmi);
template upll_rc_t
MoMgrImpl::GetUninitOperState<val_vbr_st_t, val_db_vbr_st_t>
                        (ConfigKeyVal *&ck_vn, DalDmlIntf   *dmi);
template upll_rc_t
MoMgrImpl::GetUninitOperState<val_vbr_if_st_t, val_db_vbr_if_st_t>
                        (ConfigKeyVal *&ck_vn, DalDmlIntf *dmi);
template upll_rc_t
MoMgrImpl::GetUninitOperState<val_vrt_if_st_t, val_db_vrt_if_st_t>
                        (ConfigKeyVal *&ck_vn, DalDmlIntf *dmi);
template upll_rc_t
MoMgrImpl::GetUninitOperState<val_vrt_st_t, val_db_vrt_st_t>
                        (ConfigKeyVal *&ck_vn, DalDmlIntf *dmi);
template upll_rc_t
MoMgrImpl::GetUninitOperState<val_vtunnel_st_t, val_db_vtunnel_st_t>
                        (ConfigKeyVal *&ck_vn, DalDmlIntf *dmi);
template upll_rc_t
MoMgrImpl::GetUninitOperState<val_vtunnel_if_st_t, val_db_vtunnel_if_st_t>
                        (ConfigKeyVal *&ck_vn, DalDmlIntf *dmi);
template upll_rc_t
MoMgrImpl::GetUninitOperState<val_vtep_st_t, val_db_vtep_st_t>
                        (ConfigKeyVal *&ck_vn, DalDmlIntf *dmi);
template upll_rc_t
MoMgrImpl::GetUninitOperState<val_vtep_if_st_t, val_db_vtep_if_st_t>
                        (ConfigKeyVal *&ck_vn, DalDmlIntf   *dmi);
template upll_rc_t
MoMgrImpl::GetUninitOperState<val_vtn_st_t, val_db_vtn_st_t>
                        (ConfigKeyVal *&ck_vn, DalDmlIntf   *dmi);
#endif



}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
