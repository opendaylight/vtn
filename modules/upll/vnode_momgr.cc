/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vnode_momgr.hh"
#include "vnode_child_momgr.hh"
#include "vtn_momgr.hh"
#include "vlink_momgr.hh"
#include "ctrlr_mgr.hh"
#include "vtn_flowfilter_momgr.hh"
#include "vtn_flowfilter_entry_momgr.hh"
#include "vtn_policingmap_momgr.hh"
namespace unc {
namespace upll {
namespace kt_momgr {

upll_rc_t VnodeMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey,
                                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key()) || !req || !dmi) {
    UPLL_LOG_DEBUG("Cannot perform create operation");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage Failed : %d", result_code);
    return result_code;
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateArgument Failed : %d", result_code);
    return result_code;
  }

  // check for key support on controller and max count
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  if (UPLL_DT_CANDIDATE == req->datatype) {
    result_code = GetInstanceCount(ikey,
                                 reinterpret_cast<char *>(ctrlr_dom.ctrlr),
                                 req->datatype, &cur_instance_count, dmi,
                                 MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("GetInstanceCount error %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
  }
#if 1
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateCapability Failed. Error_code : %d", result_code);
    return result_code;
  }
#endif
  UPLL_LOG_DEBUG("ikey keytype %d", ikey->get_key_type());
  // Vnode Existence check in CANDIDATE DB
  result_code = VnodeChecks(ikey, req->datatype, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(
        "Another Vnode with the same name already exists. Error code : %d",
        result_code);
    return result_code;
  }

  // Parent VTN check
  result_code = GetParentConfigKey(parent_ck_vtn, ikey);
  if (result_code == UPLL_RC_ERR_GENERIC) {
    UPLL_LOG_DEBUG(
        "Error in retrieving the parent VTN ConfigKeyVal. Error code : %d",
        result_code);
    return result_code;
  }
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                    (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN))));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->UpdateConfigDB(parent_ck_vtn, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);
  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
    UPLL_LOG_DEBUG("VTN doesn't exist in CANDIDATE DB. Error code : %d",
                   result_code);
    return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  }
  result_code = RestoreVnode(ikey, req, dmi, &ctrlr_dom);

  return result_code;
}

upll_rc_t VnodeMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key()) || !dmi) {
    UPLL_LOG_DEBUG("Cannot perform create operation");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = GetControllerDomainId(ikey, &ctrlr_dom);
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    UPLL_LOG_INFO("Invalid ctrlr domain");
    return result_code;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutDomain
                       | kOpInOutCtrlr };
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE,
		  dmi, &dbop, MAINTBL);
      #if 1
  controller_domain_t print_ctrlr_dom;
  print_ctrlr_dom.ctrlr = NULL;
  print_ctrlr_dom.domain = NULL;
        GET_USER_DATA_CTRLR_DOMAIN(ikey,print_ctrlr_dom);
    if (print_ctrlr_dom.ctrlr != NULL ) {
      UPLL_LOG_DEBUG("print_ctrlr_dom.ctrlr is %s", print_ctrlr_dom.ctrlr);
    }
    if (print_ctrlr_dom.domain != NULL ) {
      UPLL_LOG_DEBUG("print_ctrlr_dom.domain is %s\n",print_ctrlr_dom.domain);
    }
    #endif
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Record Creation failed in CANDIDATE DB");
    return result_code;
  }
  IpcReqRespHeader req;
  memset(&req, 0, sizeof(IpcReqRespHeader));
  req.datatype = UPLL_DT_AUDIT;
  result_code = CheckVtnExistenceOnController(ikey, &req, &ctrlr_dom, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("VTN doesn't exist on controller");
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::CtrlrTypeAndDomainCheck(ConfigKeyVal *ikey,
                      IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  result_code = GetControllerDomainId(ikey, &ctrlr_dom);
  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    UPLL_LOG_INFO("Invalid ctrlr domain");
    return result_code;
  }
  unc_keytype_ctrtype_t ctrlrtype;
  uuc::CtrlrMgr *ctrlr_mgr = uuc::CtrlrMgr::GetInstance();
  if (!ctrlr_mgr->GetCtrlrType(
        reinterpret_cast<char *>(ctrlr_dom.ctrlr), req->datatype, &ctrlrtype)) {
    UPLL_LOG_DEBUG("Specified Controller Doesn't Exist");
    return UPLL_RC_ERR_CFG_SEMANTIC;
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::ValidateAttribute(ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                        IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (req->operation == UNC_OP_UPDATE) {
    ConfigKeyVal *okey = NULL;
    result_code = GetChildConfigKey(okey, ikey);
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag}; 
    result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ, dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Record does Not Exists");
      return result_code;
    }
    result_code = CtrlrTypeAndDomainCheck(okey, req);
    } else {
      result_code = CtrlrTypeAndDomainCheck(ikey, req);
    }
    if (UPLL_RC_SUCCESS  != result_code) {
      UPLL_LOG_ERROR("Controller type and domain check failed");
      return result_code;
    }
  //*other semantic validations*//
  result_code = IsHostAddrAndPrefixLenInUse(ikey, dmi, req);
  if (result_code != UPLL_RC_ERR_CFG_SEMANTIC) {
    return UPLL_RC_SUCCESS;
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::RestoreVnode(ConfigKeyVal *ikey,
                                   IpcReqRespHeader *req,
                                   DalDmlIntf *dmi,
                                   controller_domain_t *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dmi == NULL) {
    UPLL_LOG_DEBUG("Create error due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t key_type = ikey->get_key_type();
  /* check if vnode exists in RUNNING DB
   ** if exists : restore to CANDIDATE DB
   ** else : validate the attributes
   */
  if (UPLL_DT_CANDIDATE  == req->datatype) {
      result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
                                       MAINTBL);
    if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      result_code = RestoreChildren(ikey, req->datatype, UPLL_DT_RUNNING, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Restoring children failed. Error code : %d", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
      return result_code;
    }  
  }
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE ||
           UPLL_DT_IMPORT == req->datatype) {
    if (key_type == UNC_KT_VTEP || key_type == UNC_KT_VTUNNEL
        || key_type == UNC_KT_VTEP_GRP) {
      //  create a record in CANDIDATE DB
      DbSubOp dbop = { kOpReadExist, kOpMatchNone, kOpInOutFlag };
      result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE,
                                   dmi, &dbop, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Record Creation failed in CANDIDATE DB");
        return result_code;
      }
      result_code = CheckVtnExistenceOnController(ikey, req, ctrlr_dom,
                                                  dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("VTN doesn't exist on controller");
      }
      return result_code;
    }
  } else {
    UPLL_LOG_ERROR("Problem in reading RUNNING DB");
    return result_code;
  }

  /* check if any UNC vtn is renamed as this VTN on the given Controller
   ** if so : throw an error
   */
  if (UPLL_DT_CANDIDATE == req->datatype ) {
    if (ikey->get_key_type() != UNC_KT_VLINK) {
      result_code = CheckRenamedVtnName(ikey, req->datatype, ctrlr_dom, dmi);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("Specified VTN is already on controller");
        return UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Error in Reading DB %d", result_code);
        return result_code;
      }
      }
    }
  /* If parent VTN is renamed, set the renamed flag in Vnode
   ** and create an entry in vnode rename table if VTN is renamed
   */
    result_code = SetVnodeRenameFlag(ikey, req->datatype, ctrlr_dom, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      return result_code;
    }
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

  if (key_type == UNC_KT_VBRIDGE || key_type == UNC_KT_VROUTER) {
    /* check if controller is aware of parent VTN
     ** and create an entry in VtnCtrlrTbl on failure
     */
    result_code = CheckVtnExistenceOnController(ikey, req, ctrlr_dom, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("VTN doesn't exist on controller");
    }
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::CheckRenamedVtnName(ConfigKeyVal *ikey,
                                          upll_keytype_datatype_t dt_type,
                                          controller_domain *ctrlr_dom,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dmi == NULL || ctrlr_dom == NULL) {
    UPLL_LOG_INFO("Create error due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vtn_key = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  /* check if any other vtn is renamed as this vtn on this controller */
  result_code = GetParentConfigKey(vtn_key, ikey);
  if (result_code != UPLL_RC_SUCCESS) return result_code;

  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                    (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN))));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    delete vtn_key;
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetRenamedUncKey(vtn_key, dt_type, dmi, ctrlr_dom->ctrlr);
  delete vtn_key;
  return result_code;
}

upll_rc_t VnodeMoMgr::SetVnodeRenameFlag(ConfigKeyVal *&ikey,
                                         upll_keytype_datatype_t dt_type,
                                         controller_domain *ctrlr_dom,
                                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dmi == NULL || ctrlr_dom == NULL) {
    UPLL_LOG_INFO("Create error due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  /*Check if parent vtn renamed and get the renamed name */
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN))));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetRenamedControllerKey(parent_ck_vtn, dt_type, dmi,
                                             ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }
  int rename = 0;
  GET_USER_DATA_FLAGS(parent_ck_vtn, rename);
  if (!(rename & RENAME)) return UPLL_RC_SUCCESS;
  // create entry in Vnode Rename Table-parent_ck_vtn contains the renamed name
  result_code = CreateVnodeRenameEntry(ikey, dt_type, ctrlr_dom, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Update Config result %d", result_code);
    return result_code;
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::CreateVnodeRenameEntry(ConfigKeyVal *ikey,
                                             upll_keytype_datatype_t dt_type,
                                             controller_domain *ctrlr_dom,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dmi == NULL || ctrlr_dom == NULL) {
    UPLL_LOG_DEBUG("Create error due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t *temp_vtn_name = NULL;
  ConfigVal *cv_rename_vnode = NULL;    // comment due to configval
  ConfigKeyVal *ck_rename_vnode = NULL;

  uint8_t *temp_vnode_name = NULL;
  GetVnodeName(ikey, temp_vtn_name, temp_vnode_name);
  if (temp_vtn_name == NULL || temp_vnode_name == NULL)
    return UPLL_RC_ERR_GENERIC;

  // create entry in Vnode Rename Table
  unc_key_type_t key_type = ikey->get_key_type();
  key_vnode* key_rename_vnode = reinterpret_cast<key_vnode*>
                                (ConfigKeyVal::Malloc(sizeof(key_vnode)));
  uuu::upll_strncpy(key_rename_vnode->vtn_key.vtn_name,
         temp_vtn_name, (kMaxLenVtnName+1) );
  uuu::upll_strncpy(key_rename_vnode->vnode_name,
                    temp_vnode_name, (kMaxLenVnodeName+1));

  uint8_t *new_vtn_name =
      reinterpret_cast<key_vtn *>(parent_ck_vtn->get_key())->vtn_name;
  val_rename_vnode_t* val_rename_vnode = reinterpret_cast<val_rename_vnode_t*>
                            (ConfigKeyVal::Malloc(sizeof(val_rename_vnode_t)));
  uuu::upll_strncpy(val_rename_vnode->ctrlr_vtn_name,
                   new_vtn_name, (kMaxLenVtnName+1));
  cv_rename_vnode = new ConfigVal(IpctSt::kIpcInvalidStNum, val_rename_vnode);
  ck_rename_vnode = new ConfigKeyVal(key_type, IpctSt::kIpcInvalidStNum,
                                     key_rename_vnode, cv_rename_vnode);
  SET_USER_DATA_CTRLR_DOMAIN(ck_rename_vnode, *ctrlr_dom);
  result_code = UpdateConfigDB(ck_rename_vnode, dt_type, UNC_OP_CREATE, dmi,
                               RENAMETBL);
//  SET_USER_DATA_CTRLR(ck_rename_vnode, ctrlr_id);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Record Creation failed in %d", result_code);
    delete ck_rename_vnode;
    return result_code;
  }
  delete ck_rename_vnode;
  return result_code;
}

upll_rc_t VnodeMoMgr::CheckVtnExistenceOnController(
                                      ConfigKeyVal *ikey,
                                      IpcReqRespHeader *req,
                                      controller_domain *ctrlr_dom,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dmi == NULL || ctrlr_dom == NULL) {
    UPLL_LOG_ERROR("Create error due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vtn_cntrlr = NULL;
  unc_keytype_operation_t op = UNC_OP_UPDATE;
  result_code = GetParentConfigKey(ck_vtn_cntrlr, ikey);
  if (result_code != UPLL_RC_SUCCESS) return result_code;
  /* set the controller -id */
  SET_USER_DATA_CTRLR_DOMAIN(ck_vtn_cntrlr, *ctrlr_dom);
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VTN)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    delete ck_vtn_cntrlr;
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone};
  result_code = mgr->ReadConfigDB(ck_vtn_cntrlr, req->datatype,
                              UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    ConfigKeyVal *ck_vtn = NULL;
    result_code = GetParentConfigKey(ck_vtn, ikey);
    if (result_code == UPLL_RC_ERR_GENERIC) {
      UPLL_LOG_ERROR("Error in retrieving the parent VTN ConfigKeyVal");
      delete ck_vtn_cntrlr;
      return result_code;
    }
    dbop.matchop = kOpMatchNone;
    result_code = mgr->ReadConfigDB(ck_vtn, req->datatype, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code);
      delete ck_vtn;
      delete ck_vtn_cntrlr;
      return result_code;
    }
    if (req->datatype != UPLL_DT_AUDIT) {
      result_code = mgr->ValidateCapability(
           req, ck_vtn, reinterpret_cast<char *>(ctrlr_dom->ctrlr));
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
      delete ck_vtn;
      delete ck_vtn_cntrlr;
      return result_code;
    }
    result_code = IntimatePOMAboutNewController(ikey, ctrlr_dom,
                                                dmi, UNC_OP_CREATE);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in updating POM Manager"
                    " about the addition of a new Controller");
        delete ck_vtn;
        delete ck_vtn_cntrlr;
        return result_code;
      }
    }
    
    val_vtn_ctrlr *ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>
                                     (GetVal(ck_vtn_cntrlr));
    val_vtn *vtn_val = reinterpret_cast<val_vtn*>
                                     (GetVal(ck_vtn));
    if (!ctrlr_val || !vtn_val) {
      UPLL_LOG_DEBUG("Invalid param");
      delete ck_vtn;
      delete ck_vtn_cntrlr;
      return UPLL_RC_ERR_GENERIC;
    }
    ctrlr_val->ref_count = 1;
    ctrlr_val->down_count = 0;
    ctrlr_val->flags = 0;
    ctrlr_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    ctrlr_val->alarm_status = UPLL_ALARM_CLEAR;
    ctrlr_val->cs_attr[0]  = (vtn_val->valid[UPLL_IDX_DESC_VTN] ==
         UNC_VF_NOT_SOPPORTED)?  UNC_CS_NOT_SUPPORTED : UNC_CS_UNKNOWN;
    ctrlr_val->cs_row_status  = UNC_CS_NOT_APPLIED;
    delete ck_vtn;
    op = UNC_OP_CREATE;
  } else if (result_code == UPLL_RC_SUCCESS) {
    val_vtn_ctrlr *ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>
                                     (GetVal(ck_vtn_cntrlr));
    if (!ctrlr_val) {
      UPLL_LOG_DEBUG("Invalid param");
      return UPLL_RC_ERR_GENERIC;
    }
    ctrlr_val->ref_count++;
  }
  result_code = mgr->UpdateConfigDB(ck_vtn_cntrlr, req->datatype,
                                      op, dmi, CTRLRTBL);
  delete ck_vtn_cntrlr;
  return result_code;
}

upll_rc_t VnodeMoMgr::IntimatePOMAboutNewController(ConfigKeyVal *ikey,
                                        controller_domain *ctrlr_dom,
                                        DalDmlIntf *dmi,
                                        unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t pom_keys[] = { UNC_KT_VTN_FLOWFILTER,
                                UNC_KT_VTN_FLOWFILTER_ENTRY,
                                UNC_KT_VTN_POLICINGMAP};
  MoMgrImpl *pom_mgr = NULL;
  uint8_t *vtn_name = (reinterpret_cast<key_vtn*>(ikey->get_key()))->vtn_name;
  if (!vtn_name) return UPLL_RC_ERR_GENERIC;

  int npom_mgr = sizeof(pom_keys)/sizeof(pom_keys[0]);
  for (uint8_t count = 0; count < npom_mgr; count++) {
    pom_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>(GetMoManager(
              pom_keys[count])));
    if (!pom_mgr) {
      UPLL_LOG_DEBUG("error in fetching MoMgr reference");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = pom_mgr->UpdateControllerTableForVtn(vtn_name,
                              ctrlr_dom, UNC_OP_CREATE, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Failed in Intimating POM module of new controller, %d",
                     result_code);
      return result_code;
    }
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::DeleteMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                               DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code =  ValidateDeleteMoReq(req, ikey, dmi);

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }
  /* Checks the last node for VTN on this controller, If is this last node,
   * then value is one,
   */
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                   kOpInOutNone };
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain };
  ConfigKeyVal *parent_key = NULL;

  result_code = GetParentConfigKey(parent_key, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    if (NULL != parent_key) delete parent_key;
    return result_code;
  }

  /* for VTN delete from Controller Table */
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  ConfigKeyVal *ck_vbr = NULL;
  result_code = GetChildConfigKey(ck_vbr, ikey);
  if (!ck_vbr || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Invalid param %d", result_code);
    return result_code;
  }
  result_code = ReadConfigDB(ck_vbr, req->datatype, UNC_OP_READ, dbop1,
                                  dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS)
    result_code = GetControllerDomainId(ck_vbr, &ctrlr_dom);

  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    UPLL_LOG_INFO("Invalid ctrlr/domain");
    //DELETE_IF_NOT_NULL(parent_key);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);
  SET_USER_DATA_CTRLR_DOMAIN(parent_key, ctrlr_dom);
  delete ck_vbr;
  /* GetReference count from vtn controller table */
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VTN)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->ReadConfigDB(parent_key, req->datatype, UNC_OP_READ, dbop,
                                  dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) return result_code;
  val_vtn_ctrlr *vtn_st_val = reinterpret_cast<val_vtn_ctrlr *>
                                   (GetVal(parent_key));
  // vtn_st_val->ref_count = 1;  //  TODO(l): refcount is not updated.

  /*  Delete the Current Node */
  if (1 == vtn_st_val->ref_count) {
    result_code = mgr->UpdateConfigDB(parent_key, req->datatype, UNC_OP_DELETE,
                                      dmi, CTRLRTBL);

  } else {
    // Reduce the ref count if vnode is not last node.
    vtn_st_val->ref_count--;
    result_code = mgr->UpdateConfigDB(parent_key, req->datatype, UNC_OP_UPDATE,
                                      dmi, CTRLRTBL);
  }
  delete parent_key;
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in updating/deleting ctrlr table %d result %d",
                    vtn_st_val->ref_count, result_code);
    return result_code;
  }
  result_code = DeleteCandidateMo(req, ikey, dmi);
  return result_code;
}

upll_rc_t VnodeMoMgr::ControlMo(IpcReqRespHeader *header, ConfigKeyVal *ikey,
                                DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  if ((ikey->get_key_type() == UNC_KT_VBRIDGE)
      || (ikey->get_key_type() == UNC_KT_VROUTER)) {
    if ((header->datatype == UPLL_DT_RUNNING ||
         header->datatype == UPLL_DT_STATE)) {
      result_code = ValidateMessage(header, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                      result_code);
        return result_code;
      }
      ConfigKeyVal *okey = NULL;
      result_code = GetChildConfigKey(okey, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
        return result_code;
      } 
      result_code = UpdateConfigDB(okey, UPLL_DT_RUNNING, UNC_OP_READ,
                                      dmi, MAINTBL);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("Record doesn't exist in DB. Error code : %d",
                     result_code);
        return result_code;
      }
      // Getting the controller vtn, vnode name
      result_code = GetRenamedControllerKey(ikey, UPLL_DT_RUNNING, dmi,
                                          &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Exiting VnodeMoMgr::ControlMo");
        return result_code;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
      UPLL_LOG_TRACE("After Read  %s",(ikey->ToStrAll()).c_str());

      IpcResponse ipc_resp;
      memset(&(ipc_resp),0,sizeof(IpcResponse));
      IpcRequest ipc_req;
      memset(&ipc_req, 0, sizeof(ipc_req));
      memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
      ipc_req.ckv_data = ikey;

      if (!uui::IpcUtil::SendReqToDriver((const char *)(ctrlr_dom.ctrlr),
          reinterpret_cast<char *>(ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME, 
          PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
        UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                      ikey->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        return UPLL_RC_ERR_GENERIC;
      }
    
      // Populate ConfigKeyVal and IpcReqRespHeader with the response from driver
      ikey->ResetWith(ipc_resp.ckv_data);
    } else {
      UPLL_LOG_DEBUG("Control Operation not allowed for %d data type", header->datatype);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    } 
  } else {
    UPLL_LOG_DEBUG("Control Operation not allowed for %d key type", ikey->get_key_type());
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::GetVnodeType(const void *key, bool vnode,
                                  unc_key_type_t &keytype,
                                  ConfigKeyVal *&ck_val, DalDmlIntf *dmi,
                                  upll_keytype_datatype_t dt_type) {
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
        (reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                  (GetMoManager(keytype))));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid param");
      return UPLL_RC_ERR_GENERIC;
    }
    upll_rc_t result_code = mgr->GetChildConfigKey(ck_val, NULL);
    if (!ck_val || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Invalid param keytype  %d", keytype);
      return UPLL_RC_ERR_GENERIC;
    }
    switch (keytype) {
     case UNC_KT_VBRIDGE:
      case UNC_KT_VROUTER:
      case UNC_KT_VTUNNEL:
      case UNC_KT_VTEP: {
        const pfc_ipcstdef_t *key_stdef = IpctSt::GetIpcStdef(
            ck_val->get_st_num());
        if (sizeof(key_vnode_t) != key_stdef->ist_size)
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
        if (sizeof(*reinterpret_cast<const key_vbr_if_t *>(key))
                         != key_stdef->ist_size)
          return UPLL_RC_ERR_GENERIC;
        memcpy(ck_val->get_key(), key, sizeof(key_vbr_if_t));
        break;
      }
      default:
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
    DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutFlag
                          | kOpInOutDomain };
    result_code = mgr->ReadConfigDB(ck_val, dt_type,
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

bool VnodeMoMgr::UpdateOperStatus(ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi,
                                  state_notification notification) {
  UPLL_FUNC_TRACE;
  bool oper_status_change = false;
  unc_key_type_t ktype = ikey->get_key_type();
  VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>
                (const_cast<MoManager*>(GetMoManager(ktype)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid mgr");
    return false;
  }
  switch (ktype) {
    case UNC_KT_VBRIDGE:
      oper_status_change = mgr->SetOperStatus<val_vbr_st_t, val_db_vbr_st_t *>(
                                        ikey,  dmi, notification);
      break;
    case UNC_KT_VROUTER:
      oper_status_change = mgr->SetOperStatus<val_vrt_st_t, val_db_vrt_st_t *>(
                                        ikey,  dmi, notification);
        break;
    case UNC_KT_VTEP:
      oper_status_change = mgr->SetOperStatus
                                 <val_vtep_st_t, val_db_vtep_st_t *>(
                                        ikey,  dmi, notification);
      break;
    case UNC_KT_VTUNNEL:
      oper_status_change = mgr->SetOperStatus
                                 <val_vtunnel_st_t, val_db_vtunnel_st_t *>(
                                        ikey,  dmi, notification);
      break;
    default:
      UPLL_LOG_DEBUG("Operstatus attribute not supported for this kt %d",
                          ktype);
      break;
  }
  if (oper_status_change) {
     VtnMoMgr *mgr = reinterpret_cast<VtnMoMgr *>
                  (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
     if (!mgr) {
       UPLL_LOG_DEBUG("Invalid mgr");
       return UPLL_RC_ERR_GENERIC;
     }
     ConfigKeyVal *ck_vtn = NULL;
     upll_rc_t result_code = GetParentConfigKey(ck_vtn,ikey);
     if (!ck_vtn || result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_DEBUG("Returning error %d",result_code);
       return false;
     }
     oper_status_change = mgr->UpdateOperStatus(ck_vtn, dmi, 
                                                notification,false);
  }
  return oper_status_change;
}

/*This function updates the operational status of the node*/
upll_rc_t VnodeMoMgr::UpdateVnodeOperStatus(
    key_vnode_t *src_vnode, set<key_vnode_t> *vnode_set,
    set<key_vlink_t, vlink_compare> *vlink_set, DalDmlIntf *dmi,
    state_notification notification) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  for (set<key_vnode_t>::iterator vnode_itr = vnode_set->begin();
       vnode_itr != vnode_set->end(); ++vnode_itr) {
    unc_key_type_t ktype;
    ConfigKeyVal *ck_vn;
    key_vnode_t vn_key = *vnode_itr;
    result_code = GetVnodeType(reinterpret_cast<const void *>(&vn_key),
                               true, ktype, ck_vn, dmi, UPLL_DT_CANDIDATE);
    if (!ck_vn || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d ", result_code);
      return UPLL_RC_ERR_GENERIC;
    }
    UpdateOperStatus(ck_vn, dmi, notification);
#if 0
    if (oper_status_change) {
      VtnMoMgr *mgr = reinterpret_cast<VtnMoMgr *>
                  (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        return UPLL_RC_ERR_GENERIC;
      }
      uint8_t *vtn_name = reinterpret_cast<key_vbr *>
                                 (ck_vn->get_key())->vtn_key.vtn_name;
      oper_status_change = mgr->SetOperStatus(vtn_name, dmi, notification);
    }
#endif
    delete ck_vn;
  }
  VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager*>
                                            (GetMoManager(UNC_KT_VLINK)));
  for (set<key_vlink_t>::iterator vlink_itr = vlink_set->begin();
       vlink_itr != vlink_set->end(); ++vlink_itr) {
    key_vlink_t *vlink_key = reinterpret_cast<key_vlink_t *>
                                      (malloc(sizeof(key_vlink_t)));
    if (!vlink_key) return UPLL_RC_ERR_GENERIC;
    *vlink_key = *vlink_itr;
    ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VLINK, IpctSt::kIpcStKeyVlink,
                                          vlink_key, NULL);
    result_code = mgr->SetOperStatus(ikey, dmi, notification);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Record updation failed in UPLL_DT_STATE %d",
                    result_code);
      return result_code;
    }
    delete ikey;
  }
  return result_code;
}

template<typename T1, typename T2>
bool VnodeMoMgr::SetOperStatus(ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi, int notification,
                                    bool skip) {
  /* update corresponding vnode operstatus */
  bool oper_change = false;
  upll_rc_t result_code;
  if (!skip) {
    DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
    result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                               MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Error in reading: %d", result_code);
      return oper_change;
    }
  }
  ConfigVal *tmp =
      (ikey->get_cfg_val()) ? ikey->get_cfg_val()->get_next_cfg_val() : NULL;
  T2 vn_valst = (T2)((tmp != NULL) ? tmp->get_val() : NULL);
  if (vn_valst == NULL) {
    UPLL_LOG_DEBUG("Invalid param\n");
    return UPLL_RC_ERR_GENERIC;
  }
  T1 *vn_val = reinterpret_cast<T1 *>(vn_valst);
  /* Update oper status based on notification */
  vn_val->valid[0] = UNC_VF_VALID;
  UPLL_LOG_DEBUG("notification %d down_count %d fault_count %d\n",notification,
                  vn_valst->down_count,vn_valst->fault_count);
  switch (notification) {
    case kCtrlrDisconnect:
      vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      break;
    case kCtrlrReconnect:
      return false;
    case kPathFault:
      if (vn_valst->fault_count++ == 1) {
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
        // generate alarm
        oper_change = true;
      }
      break;
    case kPathFaultReset:
      vn_valst->fault_count = (vn_valst->fault_count > 0) ?
          (vn_valst->fault_count - 1) : 0;
      if (vn_valst->fault_count == 0) {
        vn_val->oper_status = UPLL_OPER_STATUS_UP;
        // generate alarm
        oper_change = true;
      }
      break;
    case kPortFault:
    case kBoundaryFault:
      if (++vn_valst->down_count == 1) {
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
        oper_change = true;
      }
      break;
    case kPortFaultReset:
      vn_valst->down_count = (vn_valst->down_count > 0) ?
          (vn_valst->down_count - 1) : 0;
      if (vn_valst->down_count == 0) {
        vn_val->oper_status = UPLL_OPER_STATUS_UP;
        // generate alarm
        oper_change = true;
      }
      break;
    case kBoundaryFaultReset:
      vn_valst->fault_count = (vn_valst->fault_count > 0) ?
          (vn_valst->fault_count - 1) : 0;
      if (vn_valst->fault_count == 0) {
        vn_val->oper_status = UPLL_OPER_STATUS_UP;
        // generate alarm
        oper_change = true;
      }
      break;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                               &dbop, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in update oper status %d", result_code);
  }
  return oper_change;
}


/* This function update the vnode operstatus 
 * while doing commit
 */
upll_rc_t VnodeMoMgr::TxUpdateDtState(unc_key_type_t ktype,
                                      uint32_t session_id,
                                      uint32_t config_id,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vn = NULL;

  /* Create Vnode If key */
  switch (ktype) {
  case UNC_KT_VBRIDGE:
    result_code = GetUninitOperState<val_vbr_st_t, val_db_vbr_st_t>
                                    (ck_vn, dmi);
    break;
  case UNC_KT_VROUTER:
    result_code = GetUninitOperState<val_vrt_st_t,val_db_vrt_st>
                                    (ck_vn, dmi); 
    break;
  case UNC_KT_VTEP:
    result_code = GetUninitOperState<val_vtep_st_t,val_db_vtep_st>
                                    (ck_vn, dmi); 
    break;
  case UNC_KT_VTUNNEL:
    result_code = GetUninitOperState<val_vtunnel_st_t,val_db_vtunnel_st>
                                    (ck_vn, dmi); 
    break;
  default:
    UPLL_LOG_DEBUG("Unsupported operation on keytype %d\n",ktype);
    return UPLL_RC_ERR_GENERIC;
  }
  if (UPLL_RC_SUCCESS != result_code || ck_vn == NULL)  {
    return result_code;
  }
  ConfigKeyVal *tkey = ck_vn;
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  while (tkey) {
    /* read the state value */
    ConfigKeyVal *okey = NULL;
    result_code = GetChildConfigKey(okey,tkey);
    if (!okey || result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning %d\n",result_code);
      return result_code;
    }
    result_code = ReadConfigDB(okey, UPLL_DT_STATE,
                              UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning %d\n",result_code);
      return result_code;
    }
    val_db_vbr_st * vnode_runst = reinterpret_cast<val_db_vbr_st *>
                                                (GetStateVal(okey));
    if (!vnode_runst) {
      UPLL_LOG_DEBUG("Invalid param\n");
      return UPLL_RC_ERR_GENERIC;
    }
    if ((vnode_runst->down_count == 0) && (vnode_runst->fault_count == 0)) {
      if (vnode_runst->vbr_val_st.oper_status != UPLL_OPER_STATUS_UNKNOWN) {
        val_db_vbr_st * vnode_st = reinterpret_cast<val_db_vbr_st *>
                                                (GetStateVal(tkey));
        vnode_st->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
        vnode_st->vbr_val_st.oper_status = UPLL_OPER_STATUS_UP;
        vnode_st->down_count = vnode_runst->down_count;
        vnode_st->fault_count = vnode_runst->fault_count;
        result_code = UpdateConfigDB(tkey, UPLL_DT_STATE, UNC_OP_UPDATE, 
                                      dmi, &dbop1, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_DEBUG("UpdateConfigDB Executed %d", result_code);
           break;
        }
      }
    }
    if (okey) delete okey;
    tkey= tkey->get_next_cfg_key_val();
  }
  if (ck_vn)
    delete ck_vn;
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc

