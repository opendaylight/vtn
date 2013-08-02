/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
//  #include <iostream>
#include <map>
#include <string>
#include <list>
#include "momgr_impl.hh"
#include "vtn_momgr.hh"
#include "vbr_momgr.hh"
#include "upll_log.hh"
#include "vlink_momgr.hh"
#include "vnode_momgr.hh"
#include "vnode_child_momgr.hh"

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
  if (ikey == NULL || ctrlr_id == NULL || strlen(ctrlr_id) == 0) {
    UPLL_LOG_DEBUG("Invalid Param ikey/ctrlr_id");
    return UPLL_RC_ERR_GENERIC;
  }
  SET_USER_DATA_CTRLR(ikey, ctrlr_id);
  DbSubOp dbop = { kOpReadCount, kOpMatchCtrlr, kOpInOutNone };

  DalBindInfo *dal_bind_info = new DalBindInfo(tbl_index);
  result_code = BindAttr(dal_bind_info, ikey, UNC_OP_READ, dt_type, dbop, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("BindAttr returns error %d\n", result_code);
    delete dal_bind_info;
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = DalToUpllResCode(
      dmi->GetRecordCount(dt_type, tbl_index, dal_bind_info, count));
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
  /* rename is set implies user wants the ikey
   * populated with val from db */
  if (!rename) {
    result_code = GetChildConfigKey(okey,ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("Returning error %d\n",result_code);
      return result_code;
    }
  } else
    okey = ikey;
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop, dmi,
                                       MAINTBL);
  if ((result_code != UPLL_RC_SUCCESS) &&
       (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE))  {
    UPLL_LOG_DEBUG("Returning error code %d\n",result_code);
    if (okey != ikey) delete okey;
    return UPLL_RC_ERR_GENERIC;
  }
  GET_USER_DATA_FLAGS(okey, rename);
  SET_USER_DATA(ikey,okey);
  rename &= RENAME;
  if (okey != ikey) delete okey;
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::RenameChildren(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    DalDmlIntf *dmi,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *tkey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool fail = false;

  char rename = 0;
  GET_USER_DATA_FLAGS(ikey, rename);
  rename &= RENAME;
  for (int i = 0; i < nchild; i++) {
    unc_key_type_t ktype = child[i];
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
                                (const_cast<MoManager *>(GetMoManager(ktype)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr\n");
      return UPLL_RC_ERR_GENERIC;
    }
    // cout << *ikey << ktype << " " << mgr << "\n";
    mgr->GetChildConfigKey(tkey, ikey);
    // cout << "Renaming keytype " << ktype << " " << *tkey << "\n";
    DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
    result_code = mgr->ReadConfigDB(tkey, dt_type, UNC_OP_READ, dbop, dmi, tbl);
    ConfigKeyVal *tmp = tkey;
    while (tmp != NULL) {
      uint8_t child_rename=0;
      GET_USER_DATA_FLAGS(tmp, child_rename);
      child_rename &= RENAME;
      if (child_rename == rename) continue;
      rename &= RENAME;
      SET_USER_DATA_FLAGS(tmp, rename);
      result_code = mgr->UpdateConfigDB(tkey, dt_type, UNC_OP_UPDATE, dmi, tbl);
      if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("UpdateConfigDB failed with error code %d\n",
                                                      result_code);
         return result_code;
      }
      tmp = tmp->get_next_cfg_key_val();
    }
    delete tkey;
    tkey = NULL;
    result_code = mgr->RenameChildren(ikey, dt_type, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Renamed failed with error code %d\n", result_code);
      fail = true;
    }
  }
  return ((fail == true) ? UPLL_RC_ERR_GENERIC : UPLL_RC_SUCCESS);
}

upll_rc_t MoMgrImpl::DeleteChildren(ConfigKeyVal *ikey,
                                    upll_keytype_datatype_t dt_type,
                                    DalDmlIntf *dmi,
                                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *tkey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool fail = false;

  for (int i = 0; i < nchild; i++) {
    unc_key_type_t ktype = child[i];
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
                          (const_cast<MoManager*>(GetMoManager(ktype)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr %d\n", ktype);
      continue;
    }
    mgr->GetChildConfigKey(tkey, ikey);
     //    std::cout << "Deleting keytype " << ktype << " " << *tkey << "\n";
    result_code = mgr->DeleteChildren(tkey, dt_type, dmi);
    /* Delete all the tables for this momgr
     * RENAMETBL to be deleted only once */
    for (int j = 0; j < mgr->get_ntable(); j++) {
      if ((mgr == NULL) ||
          (mgr->GetTable((MoMgrTables)j, dt_type) >= uudst::kDalNumTables)) {
        continue;
      }
      result_code = mgr->UpdateConfigDB(tkey, dt_type, UNC_OP_DELETE, dmi,
                                        (MoMgrTables) j);
      result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                     UPLL_RC_SUCCESS : result_code;
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DeleteChild failed with result_code %d\n", result_code);
        fail = true;
      }
    }
    delete tkey;
    tkey = NULL;
  }
  return ((fail == true) ? UPLL_RC_ERR_GENERIC : UPLL_RC_SUCCESS);
}

upll_rc_t MoMgrImpl::RestoreChildren(ConfigKeyVal *&ikey,
                                     upll_keytype_datatype_t dest_cfg,
                                     upll_keytype_datatype_t src_cfg,
                                     DalDmlIntf *dmi,
                                     MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *tkey = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  bool fail = false;

  for (int i = 0; i < nchild; i++) {
    unc_key_type_t ktype = child[i];
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
                             (const_cast<MoManager*>((GetMoManager(ktype))));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr\n");
      return UPLL_RC_ERR_GENERIC;
    }
    const uudst::kDalTableIndex tbl_index = mgr->GetTable(tbl, dest_cfg);
    if (tbl_index >= uudst::kDalNumTables) {
      UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
      return UPLL_RC_ERR_GENERIC;
    }
    DalBindInfo *dal_bind_info = new DalBindInfo(tbl_index);
    mgr->GetChildConfigKey(tkey, ikey);
    DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr | kOpMatchDomain,
                     kOpInOutNone };
    result_code = BindAttr(dal_bind_info, tkey, UNC_OP_READ, dest_cfg, dbop,
                           tbl);
    db_result = dmi->CopyMatchingRecords(dest_cfg, src_cfg, tbl_index,
                                         dal_bind_info);
    result_code = DalToUpllResCode(db_result);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("No matching instance in running config\n");
    } else if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Failed to restore child from running config %d\n",
                     result_code);
      fail = true;
    }
    result_code = mgr->RestoreChildren(tkey, dest_cfg, src_cfg, dmi);
    delete dal_bind_info;
    delete tkey;
    tkey = NULL;
  }
  return ((fail == true) ? UPLL_RC_ERR_GENERIC : UPLL_RC_SUCCESS);
}

upll_rc_t MoMgrImpl::DiffConfigDB(upll_keytype_datatype_t dt_cfg1,
                                  upll_keytype_datatype_t dt_cfg2,
                                  unc_keytype_operation_t op,
                                  ConfigKeyVal *&req,
                                  ConfigKeyVal *&nreq,
                                  DalCursor **cfg1_cursor,
                                  DalDmlIntf *dmi,
                                  MoMgrTables tbl) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code;
  result_code = DiffConfigDB(dt_cfg1, dt_cfg2, op, req, nreq, cfg1_cursor, dmi,
                             NULL, tbl);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
    return UPLL_RC_SUCCESS;

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
                                  MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  const uudst::kDalTableIndex tbl_index = GetTable(tbl, dt_cfg1);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("Table Index %d Table %d Operation op %d\n",
                 tbl_index, tbl, op);
  upll_rc_t result_code;
  DalResultCode db_result = uud::kDalRcSuccess;
  result_code = GetChildConfigKey(req, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Error from GetGetChildConfigKey for table(%d)", tbl_index);
    return result_code;
  }
  DbSubOp dbop = { kOpReadDiff, kOpMatchCtrlr | kOpMatchDomain,
                   kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain };
  if (UNC_OP_DELETE == op)
    dbop.matchop = kOpMatchCtrlr | kOpMatchDomain; 
  uint16_t max_record_count = 0;
  if (ctrlr_id) {
    dbop.inoutop &= ~kOpInOutCtrlr;
    SET_USER_DATA_CTRLR(req, ctrlr_id)
  }
  if (tbl == CTRLRTBL) {
    dbop.inoutop |= kOpInOutCs;
  }
  if (op == UNC_OP_UPDATE) {
    dbop.matchop = kOpMatchFlag;
    dbop.readop |= kOpReadDiffUpd;
  }
  if (dt_cfg2 == UPLL_DT_AUDIT) dbop.matchop |= kOpMatchCs;
  DalBindInfo *binfo_cfg1 = new DalBindInfo(tbl_index);
  result_code = BindAttr(binfo_cfg1, req, UNC_OP_READ, dt_cfg1, dbop, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error from BindAttr for table(%d)", tbl_index);
    delete binfo_cfg1;
    return result_code;
  }

  switch (op) {
    case UNC_OP_DELETE:
      db_result = dmi->GetDeletedRecords(dt_cfg1, dt_cfg2, tbl_index,
                                         max_record_count, binfo_cfg1,
                                         cfg1_cursor);
      break;
    case UNC_OP_CREATE:
      db_result = dmi->GetCreatedRecords(dt_cfg1, dt_cfg2, tbl_index,
                                         max_record_count, binfo_cfg1,
                                         cfg1_cursor);
      break;
    case UNC_OP_UPDATE: {
      DalBindInfo *binfo_cfg2 = new DalBindInfo(tbl_index);
      result_code = GetChildConfigKey(nreq, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Failed in GetChildConfigKey");
        delete binfo_cfg1;
        delete binfo_cfg2;
        return result_code;
      }
      result_code = BindAttr(binfo_cfg2, nreq, UNC_OP_READ, dt_cfg2, dbop, tbl);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error from BindAttr for table(%d)", tbl_index);
        delete binfo_cfg1;
        delete binfo_cfg2;
        return result_code;
      }
      db_result = dmi->GetUpdatedRecords(dt_cfg1, dt_cfg2, tbl_index,
                                         max_record_count, binfo_cfg1,
                                         binfo_cfg2, cfg1_cursor);
      // delete binfo_cfg2; TODO(l): Not a place to delete
      break;
    }
    default:
      break;
  }
  result_code = DalToUpllResCode(db_result);
  // if (binfo_cfg1) delete binfo_cfg1; TODO(l): Not a place to delete
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
  DalCursor *dal_cursor_handle;
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
    UPLL_LOG_INFO("Exiting MoMgrImpl::ReadConfigDB result code %d\n",
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
            reinterpret_cast<uint32_t*>(malloc(sizeof(uint32_t)));
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
      return UPLL_RC_ERR_GENERIC;
  }
  result_code = DalToUpllResCode(db_result);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning %d\n",result_code);
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
      tkey = NULL;
      result_code = DupConfigKeyVal(tkey, ikey, tbl);
      if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Dup failed error %d\n",result_code);
          delete dal_bind_info;
          return result_code;
      }
      if (!end_resp)
         end_resp = tkey;
      else 
         end_resp->AppendCfgKeyVal(tkey);
      if (op != UNC_OP_READ) count++; else nrec_read++;
    }
    if (result_code == UPLL_RC_SUCCESS) {
      if (end_resp)
        ikey->ResetWith(end_resp);
      string s(ikey->ToStrAll());
      UPLL_LOG_DEBUG(" sibling_count %d count %d operation %d response %s\n",
                   sibling_count, count, op, s.c_str());
         
    }
    dmi->CloseCursor(dal_cursor_handle);
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

//  cout << tbl_index << "\n";
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
        }
      }
    } else {
      if (dt_type != UPLL_DT_CANDIDATE)
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
  string s(ikey->ToStrAll());
  switch (op) {
    case UNC_OP_CREATE:
      UPLL_LOG_TRACE("Dbop %s dt_type %d CREATE %d\n", s.c_str(), dt_type,
                    tbl_index);
      result_code = DalToUpllResCode(
          dmi->CreateRecord(dt_type, tbl_index, dal_bind_info));
      break;
    case UNC_OP_DELETE:
      UPLL_LOG_TRACE("Dbop %s dt_type %d DELETE  %d\n", s.c_str(), dt_type,
                    tbl_index);
      result_code = DalToUpllResCode(
          dmi->DeleteRecords(dt_type, tbl_index, dal_bind_info));
      break;
    case UNC_OP_UPDATE:
      UPLL_LOG_TRACE("Dbop %s dt_type %d UPD  %d\n", s.c_str(), dt_type,
                    tbl_index);
      result_code = DalToUpllResCode(
          dmi->UpdateRecords(dt_type, tbl_index, dal_bind_info));
      break;
    case UNC_OP_READ:
      UPLL_LOG_TRACE("Dbop %s dt_type %d EXISTS  %d\n", s.c_str(), dt_type,
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

  UPLL_LOG_TRACE("Valid Falg is true ConfigKeyVal %s", (req->ToStrAll()).c_str());
  if ((req == NULL) || (tkey == NULL)) {
    UPLL_LOG_DEBUG("NULL input parameters");
    return UPLL_RC_ERR_GENERIC;
  }
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
            memcpy(ck_val1->get_val(),ck_val->get_val(),st_def->ist_size);
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
          UPLL_LOG_DEBUG("Invalid Controller\n");
          return UPLL_RC_ERR_GENERIC;
        }
      } else if (dbop.matchop & kOpMatchDomain) {
        uint8_t *dom = NULL;
        GET_USER_DATA_DOMAIN(req, dom);
        if (!dom) {
          UPLL_LOG_DEBUG("Invalid Domain\n");
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
    UPLL_LOG_DEBUG(" the attr_type %x number %d \n", binfo[i].struct_type, i);
    if (attr_type == CFG_KEY) {
      p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
          + binfo[i].offset);
      UPLL_LOG_DEBUG(" key struct %d tkey %p p %p\n", attr_type, tkey, p);
      switch (op) {
        case UNC_OP_CREATE:
          UPLL_LOG_DEBUG(" Bind input Key %"PFC_PFMT_u64" p %p\n", indx, p);
          db_info->BindInput(indx, binfo[i].app_data_type, binfo[i].array_size,
                             p);
          break;
        case UNC_OP_UPDATE:
          if (IsValidKey(tkey, indx)) {
            UPLL_LOG_DEBUG("tkey %p bind match UPD p %p\n", tkey, p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          }
          break;
        case UNC_OP_DELETE:
          if (IsValidKey(tkey, indx)) {
            UPLL_LOG_DEBUG("tkey %p bind match DEL p %p\n", tkey, p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
              binfo[i].array_size, p);
          }
          break;
        case UNC_OP_READ:
        case UNC_OP_READ_SIBLING:
        case UNC_OP_READ_SIBLING_BEGIN:
        case UNC_OP_READ_SIBLING_COUNT:
          if ((dbop.readop & kOpReadSingle) || (dbop.readop & kOpReadExist)
              || (dbop.readop & kOpReadMultiple) || (dbop.readop & kOpReadCount))  {
            if (IsValidKey(tkey, indx)) {
              UPLL_LOG_DEBUG("tkey %p bind match READ p %p\n", tkey, p);
              db_info->BindMatch(indx, binfo[i].app_data_type,
                                 binfo[i].array_size, p);
              if (dbop.readop & kOpReadMultiple) {
                UPLL_LOG_DEBUG("tkey %p bind output READ p %p\n", tkey, p);
                db_info->BindOutput(indx, binfo[i].app_data_type,
                                 binfo[i].array_size, p);
              }
            } else {
              UPLL_LOG_DEBUG("tkey %p bind output READ p %p\n", tkey, p);
              db_info->BindOutput(indx, binfo[i].app_data_type,
                                  binfo[i].array_size, p);
            }
          } else if (dbop.readop & kOpReadDiff) {
            UPLL_LOG_DEBUG("tkey %p DIFF match/output p %p\n", tkey, p);
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
        } else 
          tuser_data = NULL;
      } else {
        tuser_data = reinterpret_cast<key_user_data_t *>(req->get_user_data());
      }
      if (!tuser_data) {
        UPLL_LOG_DEBUG("null tuser_data\n");
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
            UPLL_LOG_DEBUG("CR bind input Cntrlr/Domain/Flag %p\n", p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          }
          break;
        case UNC_OP_UPDATE:
          if ((par_ctrlr && (dbop.matchop & kOpMatchCtrlr)) ||
              (par_dom && (dbop.matchop & kOpMatchDomain))  ||
              (par_flag && (dbop.matchop & kOpMatchFlag))) {
            UPLL_LOG_DEBUG("UPD bind match flag/Cntrlr %p\n ", p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          } else if ((par_ctrlr && (dbop.inoutop & kOpInOutCtrlr))
              || (par_dom && (dbop.inoutop & kOpInOutDomain))
              || (par_flag && (dbop.inoutop & kOpInOutFlag))) {
            UPLL_LOG_DEBUG("UPD bind input flag/Cntrlr/domain %p\n ", p);
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
            UPLL_LOG_DEBUG("RD bind output flag/Cntrlr/domain %p\n", p);
            db_info->BindOutput(indx, binfo[i].app_data_type,
                                binfo[i].array_size, p);
          }
          /* fall through intended */
        case UNC_OP_DELETE:
          if ((par_ctrlr && (dbop.matchop & kOpMatchCtrlr)) ||
              (par_dom && (dbop.matchop & kOpMatchDomain)) ||
              (par_flag && (dbop.matchop & kOpMatchFlag))) {
            UPLL_LOG_DEBUG("RD/DEL bind match flag/Cntrlr/domain %p\n", p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          }
          break;
        default:
          // do nothing
          break;
      }

    } else if (tval && (attr_type != ST_VAL) && (attr_type != ST_META_VAL)) {

      if (op == UNC_OP_DELETE) continue;
      p = reinterpret_cast<void *>(reinterpret_cast<char *>(tval)
          + binfo[i].offset);
      bool valid_is_defined = false;
      if (attr_type == CFG_VAL) {
        result_code = GetValid(tval, indx, valid, dt_type, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_DEBUG("Returning %d\n",result_code);
           return result_code;
        }
        if (!valid) {
          UPLL_LOG_DEBUG(" Invalid for attr %d", i);
           switch (op) {
           case UNC_OP_CREATE:
           case UNC_OP_UPDATE:
             valid_is_defined = true;
              break;
           default:
             valid_is_defined = false;
           }
        } else if (((uint8_t) *valid == UNC_VF_VALID) || 
                   ((uint8_t) *valid == UNC_VF_VALID_NO_VALUE)) {
          valid_is_defined = true; 
        }
      } else if (attr_type == CFG_META_VAL) {
        if ((*(reinterpret_cast<uint8_t *>(p)) == UNC_VF_VALID)
            || (*(reinterpret_cast<uint8_t *>(p)) == UNC_VF_VALID_NO_VALUE))
          valid_is_defined = true;
      }
      switch (op) {
        case UNC_OP_CREATE:
          if ((attr_type == CFG_META_VAL) || valid_is_defined 
              || ((attr_type == CS_VAL) && (dbop.inoutop & kOpInOutCs))) {
            UPLL_LOG_DEBUG("tval/meta CR bind input %p p %p\n", tval, p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size,
                               reinterpret_cast<void *>(p));
          }
          break;
        case UNC_OP_UPDATE:
#if 0
          if ((attr_type == CFG_META_VAL)
              || ((attr_type == CS_VAL) && (dbop.matchop & kOpMatchCs))) {
            UPLL_LOG_DEBUG("tval/meta UP bind match %p p %p\n", tval, p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size,
                               reinterpret_cast<void *>(p));
          }
#endif
          if (valid_is_defined || 
             ((attr_type == CS_VAL) && (dbop.inoutop & kOpInOutCs))) {
            UPLL_LOG_DEBUG("tval/meta UP bind input %p p %p\n", tval, p);
            // store VALID_NO_VALUE flag in candidate as INVALID
            if ((attr_type == CFG_META_VAL) &&  
                (*(reinterpret_cast<uint8_t *>(p)) == UNC_VF_VALID_NO_VALUE)) {
              UPLL_LOG_TRACE("Resetting VALID_NO_VALUE to INVALID %p \n", p);
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
              UPLL_LOG_DEBUG("tval RD bind match %p p %p\n", tval, p);
              db_info->BindMatch(indx, binfo[i].app_data_type,
                                 binfo[i].array_size,
                                 reinterpret_cast<void *>(p));
            } else if ((dbop.readop & (kOpReadExist | kOpReadCount)) == 0) {
              switch (attr_type) {
                case CS_VAL:
                  if (dbop.inoutop & kOpInOutCs) {
                    UPLL_LOG_DEBUG("tvalcs RD bind output %p p %p\n", tval, p);
                    db_info->BindOutput(indx, binfo[i].app_data_type,
                                        binfo[i].array_size,
                                        reinterpret_cast<void *>(p));
                  }
                  break;
                case CFG_VAL:
                case CFG_META_VAL:
                  UPLL_LOG_DEBUG("tval RD bind output %p p %p\n", tval, p);
                  db_info->BindOutput(indx, binfo[i].app_data_type,
                                      binfo[i].array_size,
                                      reinterpret_cast<void *>(p));
                default:
                  break;
              }
            }
          } else if (dbop.readop & kOpReadDiff) {
            if ((attr_type == CFG_META_VAL) || (attr_type == CFG_VAL)
                || ((attr_type == CS_VAL) && (dbop.inoutop & kOpInOutCs))) {
              UPLL_LOG_DEBUG("tval %d RDDiff bind output %p p %p\n", attr_type,
                             tval, p);
              db_info->BindOutput(indx, binfo[i].app_data_type,
                                  binfo[i].array_size,
                                  reinterpret_cast<void *>(p));
            }
            if ((attr_type == CFG_META_VAL) || (attr_type == CFG_VAL) ||
                ((attr_type == CS_VAL) && (dbop.matchop & kOpMatchCs))) {
              if (dbop.readop & kOpReadDiffUpd) {
                UPLL_LOG_DEBUG("tval %d RDDiff bind match %p p %p\n", attr_type,
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
      if (attr_type == ST_VAL) {
        result_code = GetValid(sval, indx, valid_st, dt_type, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning %d\n",result_code);
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
        }
        else if ((*(reinterpret_cast<uint8_t *>(valid_st)) == UNC_VF_VALID) ||
                 (*(reinterpret_cast<uint8_t *>(valid_st)) == UNC_VF_VALID_NO_VALUE))
          valid_is_defined = true;
        UPLL_LOG_TRACE(" The ST_VAL valid flag is %d", valid_is_defined);
      } else if (attr_type == ST_META_VAL) {
        if ((*(reinterpret_cast<uint8_t *>(p)) == UNC_VF_VALID)
            || (*(reinterpret_cast<uint8_t *>(p)) == UNC_VF_VALID_NO_VALUE))
          valid_is_defined = true;
      }
      switch (op) {
        case UNC_OP_CREATE:
          if ((attr_type == ST_META_VAL) || valid_is_defined) {
            UPLL_LOG_DEBUG("sval CR/UPD bind input %p p %p\n", sval, p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size,
                               reinterpret_cast<void *>(p));
          }
          break;
        case UNC_OP_UPDATE:
#if 0
          if (attr_type == ST_META_VAL) {
            UPLL_LOG_DEBUG("sval/meta UP bind match %p p %p\n", sval, p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size,
                               reinterpret_cast<void *>(p));
          }
#endif
          if (valid_is_defined) {
            UPLL_LOG_DEBUG("sval/meta UP bind input %p p %p\n", sval, p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size,
                               reinterpret_cast<void *>(p));
          }
          break;
        case UNC_OP_READ:
        case UNC_OP_READ_SIBLING:
        case UNC_OP_READ_SIBLING_BEGIN:
        case UNC_OP_READ_SIBLING_COUNT:
          if (dbop.readop & ~( kOpReadDiff | kOpReadDiffUpd | kOpReadExist)) {
            if (valid_is_defined) {
              UPLL_LOG_DEBUG("sval RD bind match %p p %p\n", sval, p);
              db_info->BindMatch(indx, binfo[i].app_data_type,
                                 binfo[i].array_size,
                                 reinterpret_cast<void *>(p));
            } else if ((dbop.readop & (kOpReadExist | kOpReadCount)) == 0) {
              UPLL_LOG_DEBUG("sval RD bind output %p p %p\n", sval, p);
              db_info->BindOutput(indx, binfo[i].app_data_type,
                                  binfo[i].array_size,
                                  reinterpret_cast<void *>(p));
            }
          } else if (dbop.readop & kOpReadDiff) {
            if ((attr_type == ST_META_VAL) || (attr_type == ST_VAL)) {
              UPLL_LOG_DEBUG("sval %d RDDiff bind output %p p %p\n", attr_type,
                             sval, p);
              db_info->BindOutput(indx, binfo[i].app_data_type,
                                  binfo[i].array_size,
                                  reinterpret_cast<void *>(p));
              if (dbop.readop & kOpReadDiffUpd) {
                UPLL_LOG_DEBUG("sval %d RDDiff bind match %p p %p\n", attr_type,
                             sval, p);
                db_info->BindMatch(indx, binfo[i].app_data_type,
                                   binfo[i].array_size,
                                   reinterpret_cast<void *>(p));
              }
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
          dmi->CreateRecord(dt_type, tbl_index, dal_bind_info));
      break;
    case UNC_OP_UPDATE:
      result_code = DalToUpllResCode(
          dmi->UpdateRecords(dt_type, tbl_index, dal_bind_info));
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
  if (!GetRenameKeyBindInfo(req->get_key_type(), binfo, nattr, tbl)){
     UPLL_LOG_DEBUG("GetRenameKeyBindInfo Not available for the keytype %d"
                    "For the Table %d", req->get_key_type(), tbl);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("The nAttribute %d", nattr);
  tuser_data = reinterpret_cast<key_user_data_t *>(req->get_user_data());
  for (int i = 0; i < nattr; i++ ) {
    UPLL_LOG_TRACE("The If condition value is %d i=%d", (nattr/2), i);
    if (i == (nattr / 2)) {
      if (req->get_next_cfg_key_val()
          && (req->get_next_cfg_key_val())->get_key())
        tkey = (req->get_next_cfg_key_val())->get_key();
        DumpRenameInfo(req->get_next_cfg_key_val());
    }

    indx = binfo[i].index;
    BindStructTypes attr_type = binfo[i].struct_type;
    UPLL_LOG_DEBUG("the attr_type %d attr number %d\n", binfo[i].struct_type,
                   i);
    p = reinterpret_cast<void *>(reinterpret_cast<char *>(tkey)
        + binfo[i].offset);
    UPLL_LOG_DEBUG("key struct %d tkey %p p %p\n", attr_type, tkey, p);
    if (CFG_INPUT_KEY == attr_type || CFG_MATCH_KEY == attr_type) {
      switch (op) {
        case UNC_OP_CREATE:
#if 0
          if (!IsValidKey(tkey, indx))  { 
            UPLL_LOG_DEBUG("Given Key is Invalid %s", (req->ToStrAll()).c_str());
            return UPLL_RC_ERR_GENERIC;
          }
#endif
          UPLL_LOG_DEBUG(" Bind input Key %"PFC_PFMT_u64" p %p\n", indx,
                       reinterpret_cast<char*>(p));
          db_info->BindInput(indx, binfo[i].app_data_type, binfo[i].array_size,
                           p);
          break;
        case UNC_OP_UPDATE:
          UPLL_LOG_TRACE("Validate the Key in Update");
//          if (IsValidKey(tkey, indx)) {
            switch (attr_type) {
              case CFG_INPUT_KEY:
                UPLL_LOG_DEBUG("tkey %p bindinput %p\n", tkey,
                             reinterpret_cast<char*>(p));
                db_info->BindInput(indx, binfo[i].app_data_type,
                                 binfo[i].array_size, p);
                break;
              case CFG_MATCH_KEY:
                UPLL_LOG_DEBUG("tkey %p bindmatch %p\n", tkey,
                               reinterpret_cast<char*>(p));
                db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
                break;
              default:
                break;
            }
//          } else {
//             UPLL_LOG_DEBUG("Invalid Key for the given key %s", (req->ToStrAll()).c_str());
//          }
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
            UPLL_LOG_DEBUG("CR bind input Cntrlr/Flag %p\n", p);
            db_info->BindInput(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          }
          break;
        case UNC_OP_UPDATE:
          if ((dbop.matchop & (kOpMatchCtrlr | kOpMatchDomain))
              || (dbop.matchop & kOpMatchFlag)) {
            UPLL_LOG_DEBUG("UPD bind match Cntrlr/Flag %p\n", p);
            db_info->BindMatch(indx, binfo[i].app_data_type,
                               binfo[i].array_size, p);
          } else if ((dbop.inoutop & (kOpInOutCtrlr | kOpInOutDomain))
              || (dbop.inoutop & kOpInOutFlag)) {
            UPLL_LOG_DEBUG("UPD bind input Cntrlr/Flag %p\n", p);
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
upll_rc_t MoMgrImpl::BindCandidateDirty(DalBindInfo *db_info,
       upll_keytype_datatype_t dt_type, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  int nattr;
  BindInfo *binfo;
  
  void *dummy = malloc(sizeof(uint16_t)); /* dummy pointer */
  if (dummy == NULL) {
    throw new std::bad_alloc;
  }
  memset(dummy, 0, sizeof(uint16_t));

  if (!GetBindInfo(tbl, dt_type, binfo, nattr))
    return UPLL_RC_ERR_GENERIC;

  for (int i = 0; i < nattr; i++) {
    uint64_t indx = binfo[i].index;
    BindStructTypes attr_type = binfo[i].struct_type;
    if (attr_type != CS_VAL && attr_type != ST_VAL &&
        attr_type != ST_META_VAL && 
       (attr_type == CFG_META_VAL && indx != uudst::kDbiVtnCtrlrTbl)) {
      db_info->BindMatch(indx, binfo[i].app_data_type,
                         binfo[i].array_size, dummy);
    }
  }

  free(dummy);
  return UPLL_RC_SUCCESS;
}


#if 0
template<typename T1, typename T2>
upll_rc_t MoMgrImpl::GetCkvWithOperSt(ConfigKeyVal *&ck_vn,
                                  unc_key_type_t ktype,
                                  DalDmlIntf     *dmi) {
  if (ck_vn != NULL) return UPLL_RC_ERR_GENERIC;
  ConfigVal *cval = NULL;
  MoMgrImpl *mgr = NULL ; 
  upll_rc_t result_code = AllocVal(cval,UPLL_DT_STATE,MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d",result_code);
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
    UPLL_LOG_DEBUG("Unsupported operation on keytype %d\n",ktype);
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
                                  dbop,dmi,MAINTBL); 
  if (UPLL_RC_SUCCESS != result_code) {
     result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                 UPLL_RC_SUCCESS : result_code;
     UPLL_LOG_DEBUG("Returning %d",result_code);
     if (ck_vn) delete ck_vn;
     return result_code;
  }
  return UPLL_RC_SUCCESS;
}

template upll_rc_t 
MoMgrImpl::GetCkvWithOperSt<val_vlink_st_t,val_db_vlink_st_t> (ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi) ;
template upll_rc_t 
MoMgrImpl::GetCkvWithOperSt<val_vbr_if_st_t,val_db_vbr_if_st_t> (ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi) ;
template upll_rc_t 
MoMgrImpl::GetCkvWithOperSt<val_vrt_if_st_t,val_db_vrt_if_st_t> (ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi) ;
template upll_rc_t 
MoMgrImpl::GetCkvWithOperSt<val_vtunnel_if_st_t,val_db_vtunnel_if_st_t> (ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi) ;
template upll_rc_t 
MoMgrImpl::GetCkvWithOperSt<val_vtep_if_st_t,val_db_vtep_if_st_t> (ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi) ;
template upll_rc_t 
MoMgrImpl::GetCkvWithOperSt<val_vtn_st_t,val_db_vtn_st_t> (ConfigKeyVal *&ck_vn, unc_key_type_t ktype, DalDmlIntf   *dmi) ;
#else
template<typename T1,typename T2>
upll_rc_t MoMgrImpl::GetUninitOperState(ConfigKeyVal *&ck_vn,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *cval = NULL;
  /* Allocate Memory for vnode st */
  result_code = AllocVal(cval,UPLL_DT_STATE,MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d",result_code);
    return result_code;
  }
  /* initialize vnode st */
  T2 *vnif_st = reinterpret_cast<T2 *>
                               (cval->get_next_cfg_val()->get_val());
  if (!vnif_st) {
   delete cval;
   UPLL_LOG_DEBUG("Invalid param");
   return UPLL_RC_ERR_GENERIC;
  }
  T1 *vnif = reinterpret_cast<T1 *>(vnif_st);
  vnif->valid[UPLL_IDX_OPER_STATUS_VBRIS] = UNC_VF_VALID;
  vnif->oper_status = UPLL_OPER_STATUS_UNINIT;
  /* Create Vnode If child */
  result_code = GetChildConfigKey (ck_vn, NULL); 
  if (UPLL_RC_SUCCESS != result_code)  {
    free(vnif_st);
    UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", result_code);
    return result_code;
  }
  ck_vn->AppendCfgVal(cval); 

  /* Reading the Vnode Table and Check the Operstatus is unknown 
   * for any one of the vnode if */
  DbSubOp dbop = { kOpReadExist | kOpReadMultiple, kOpMatchNone, kOpInOutFlag |
                           kOpInOutCtrlr | kOpInOutDomain };
  if (PORT_MAPPED_KEYTYPE(ck_vn->get_key_type()))
    dbop.readop = kOpReadMultiple;
  result_code = ReadConfigDB(ck_vn, UPLL_DT_STATE, UNC_OP_READ,
                                  dbop, dmi, MAINTBL); 
  if (UPLL_RC_SUCCESS != result_code) {
     result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                 UPLL_RC_SUCCESS : result_code;
     UPLL_LOG_DEBUG("Returning %d",result_code);
     if (ck_vn) delete ck_vn;
     ck_vn = NULL;
  }
  return result_code;
}
template upll_rc_t 
MoMgrImpl::GetUninitOperState<val_vlink_st_t,val_db_vlink_st_t> 
                        (ConfigKeyVal *&ck_vn, DalDmlIntf   *dmi) ;
template upll_rc_t 
MoMgrImpl::GetUninitOperState<val_vbr_st_t,val_db_vbr_st_t> 
                        (ConfigKeyVal *&ck_vn, DalDmlIntf   *dmi) ;
template upll_rc_t 
MoMgrImpl::GetUninitOperState<val_vbr_if_st_t,val_db_vbr_if_st_t> 
                        (ConfigKeyVal *&ck_vn, DalDmlIntf *dmi) ;
template upll_rc_t 
MoMgrImpl::GetUninitOperState<val_vrt_st_t,val_db_vrt_st_t> 
                        (ConfigKeyVal *&ck_vn, DalDmlIntf *dmi) ;
template upll_rc_t 
MoMgrImpl::GetUninitOperState<val_vtunnel_st_t,val_db_vtunnel_st_t> 
                        (ConfigKeyVal *&ck_vn, DalDmlIntf *dmi);
template upll_rc_t 
MoMgrImpl::GetUninitOperState<val_vtunnel_if_st_t,val_db_vtunnel_if_st_t> 
                        (ConfigKeyVal *&ck_vn, DalDmlIntf *dmi);
template upll_rc_t 
MoMgrImpl::GetUninitOperState<val_vtep_st_t,val_db_vtep_st_t> 
                        (ConfigKeyVal *&ck_vn, DalDmlIntf *dmi);
template upll_rc_t 
MoMgrImpl::GetUninitOperState<val_vtep_if_st_t,val_db_vtep_if_st_t> 
                        (ConfigKeyVal *&ck_vn, DalDmlIntf   *dmi);
template upll_rc_t 
MoMgrImpl::GetUninitOperState<val_vtn_st_t,val_db_vtn_st_t> 
                        (ConfigKeyVal *&ck_vn, DalDmlIntf   *dmi);
#endif


}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
