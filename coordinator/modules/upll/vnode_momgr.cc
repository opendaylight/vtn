/*
 * Copyright (c) 2012-2014 NEC Corporation
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
#include "config_mgr.hh"
#include "vlanmap_momgr.hh"

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
  if (UPLL_DT_IMPORT != req->datatype) {
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage Failed : %d", result_code);
      return result_code;
    }
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateArgument Failed : %d", result_code);
    return result_code;
  }

  UPLL_LOG_DEBUG("ikey keytype %d", ikey->get_key_type());
  // Vnode Existence check in CANDIDATE DB
  result_code = VnodeChecks(ikey, req->datatype, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(
         "Another Vnode with the same name already exists. Error code : %d",
         result_code);
    return result_code;
  }

  // check for key support on controller and max count
  result_code = GetControllerDomainId(ikey, &ctrlr_dom);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetControllerDomainId Failed");
    return result_code;
  }

  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateCapability Failed. Error_code : %d", result_code);
    return result_code;
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
  bool ctrlr_vtn_flag = false;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  uint8_t *controller_id = reinterpret_cast<uint8_t *>(
                                 const_cast<char *>(ctrlr_id));

  /* check if object is renamed in the corresponding Rename Tbl
   * if "renamed"  create the object by the UNC name.
   * else - create using the controller name.
   */
  result_code = GetRenamedUncKey(ikey, UPLL_DT_RUNNING, dmi, controller_id);
  if (result_code != UPLL_RC_SUCCESS && result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("GetRenamedUncKey Failed err_code %d", result_code);
    return result_code;
  }

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
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Record Creation failed in CANDIDATE DB");
    return result_code;
  }
  IpcReqRespHeader req;
  memset(&req, 0, sizeof(IpcReqRespHeader));
  req.datatype = UPLL_DT_AUDIT;
  result_code = SetValidAudit(ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  result_code = CheckVtnExistenceOnController(ikey, &req, &ctrlr_dom,
                                              ctrlr_vtn_flag, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("VTN doesn't exist on controller");
    return result_code;
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::CtrlrTypeAndDomainCheck(ConfigKeyVal *ikey,
                      IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
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
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  if (req->operation == UNC_OP_UPDATE) {
    ConfigKeyVal *okey = NULL;
    result_code = GetChildConfigKey(okey, ikey);
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
    result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ, dbop, dmi, MAINTBL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Record does Not Exists");
      delete okey;
      return result_code;
    }
    GET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
    result_code = CtrlrIdAndDomainIdUpdationCheck(ikey, okey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Controller_id/domain_id updation check failed");
      delete okey;
      return result_code;
    }
    result_code = CtrlrTypeAndDomainCheck(okey, req);
    delete okey;
    } else {
      result_code = CtrlrTypeAndDomainCheck(ikey, req);
    }
    if (UPLL_RC_SUCCESS  != result_code) {
      UPLL_LOG_ERROR("Controller type and domain check failed");
      return result_code;
    }
  //* other semantic validations *//
  if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
    result_code = ValidateIpAddress(ikey, req->datatype, dmi);
  }
  //Commented for #403
  /*if (ikey->get_key_type() == UNC_KT_VROUTER) {
    result_code = EnableAdminStatus(ikey, dmi, req);
    if (result_code != UPLL_RC_ERR_CFG_SEMANTIC) {
      return UPLL_RC_SUCCESS;
    }
  }*/
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
  bool ctrlr_vtn_flag = false;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t key_type = ikey->get_key_type();
  /* check if vnode exists in RUNNING DB
   ** if exists : restore to CANDIDATE DB
   ** else : validate the attributes
   */
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
                                                ctrlr_vtn_flag, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("VTN doesn't exist on controller");
      return result_code;
    }
    return result_code;
  }

/* set the controller domain in the ikey */
// SET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);

#if 0
if (ikey->get_key_type() == UNC_KT_VLINK) {
  if (!ikey->get_cfg_val()) return UPLL_RC_ERR_GENERIC;
  SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
}
#endif
// create a record in CANDIDATE DB


  if (key_type == UNC_KT_VBRIDGE || key_type == UNC_KT_VROUTER || key_type == UNC_KT_VTERMINAL
      || UNC_KT_VLINK == key_type) {
  /* check if controller is aware of parent VTN
   ** and create an entry in VtnCtrlrTbl on failure
   */
  /* check if any UNC vtn is renamed as this VTN on the given Controller
   ** if so : throw an error
   */

  if (UPLL_DT_CANDIDATE == req->datatype && UNC_KT_VLINK != key_type) {
    result_code = CheckRenamedVtnName(ikey, req->datatype, ctrlr_dom, dmi);
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      if (UPLL_RC_SUCCESS == result_code) {
        ctrlr_vtn_flag = true;
        result_code = CheckVtnExistenceOnController(ikey, req, ctrlr_dom,
                                                    ctrlr_vtn_flag, dmi);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_INFO("CheckVtnExistenceOnController Failed");
          return result_code;
        }
      } else {
        UPLL_LOG_DEBUG("Error in Reading DB %d", result_code);
        return result_code;
      }
    }
  }
  if (!ctrlr_vtn_flag && UNC_KT_VLINK != key_type) {
    result_code = CheckVtnExistenceOnController(ikey, req, ctrlr_dom,
                                                ctrlr_vtn_flag, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("VTN doesn't exist on controller");
      return result_code;
    }
  }
  bool auto_rename = false;
  if (UPLL_DT_CANDIDATE == req->datatype)
    auto_rename = true;
  result_code = GenerateAutoName(ikey, req->datatype, ctrlr_dom, dmi, &auto_rename);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Record Creation fialed in CANDIDATE DB");
    return result_code;
  }

  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutDomain
    | kOpInOutCtrlr };
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi, &dbop,
                               MAINTBL);
  return result_code;
  
}
// TODO (karthi) moved here, because after creating the vnode
// we need to set the rename flag.

/* If parent VTN is renamed, set the renamed flag in Vnode
 ** and create an entry in vnode rename table if VTN is renamed
 */
//  result_code = SetVnodeRenameFlag(ikey, req->datatype, ctrlr_dom, dmi, false);
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
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {  
     UPLL_LOG_INFO("GetRenamedUncKeyFailed %d", result_code);
     delete vtn_key;
     return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    result_code = mgr->GetRenamedUncKey(vtn_key, UPLL_DT_RUNNING, dmi, ctrlr_dom->ctrlr);
    if (UPLL_RC_SUCCESS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {  
       UPLL_LOG_INFO("GetRenamedUncKeyFailed %d", result_code);
       delete vtn_key;
       return result_code;
    }
  }
  delete vtn_key;
  return result_code;
}

upll_rc_t VnodeMoMgr::CheckVtnExistenceOnController(
                                      ConfigKeyVal *ikey,
                                      IpcReqRespHeader *req,
                                      controller_domain *ctrlr_dom,
                                      bool ctrlr_vtn_flag,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dmi == NULL || ctrlr_dom == NULL) {
    UPLL_LOG_ERROR("Create error due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;;
  ConfigKeyVal *ck_vtn_cntrlr = NULL;
  ConfigKeyVal *ck_domain_vtn = NULL;
  ConfigKeyVal *ck_vtn = NULL;
  val_vtn_ctrlr *ctrlr_val = NULL;

  unc_keytype_operation_t op = UNC_OP_UPDATE;
  result_code = GetParentConfigKey(ck_vtn_cntrlr, ikey);
  if (result_code != UPLL_RC_SUCCESS) return result_code;

  result_code = GetParentConfigKey(ck_domain_vtn, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
     DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
     return result_code;
  }

  /* set the controller -id */
  SET_USER_DATA_CTRLR_DOMAIN(ck_vtn_cntrlr, *ctrlr_dom);
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VTN)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
    DELETE_IF_NOT_NULL(ck_domain_vtn);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = GetParentConfigKey(ck_vtn, ikey);
  if (result_code == UPLL_RC_ERR_GENERIC) {
    UPLL_LOG_ERROR("Error in retrieving the parent VTN ConfigKeyVal");
    DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
    DELETE_IF_NOT_NULL(ck_domain_vtn);
    return result_code;
  }
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};

  result_code = mgr->ReadConfigDB(ck_vtn, req->datatype, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code);
    DELETE_IF_NOT_NULL(ck_vtn);
    DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
    DELETE_IF_NOT_NULL(ck_domain_vtn);
    return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code);
    DELETE_IF_NOT_NULL(ck_vtn);
    DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
    DELETE_IF_NOT_NULL(ck_domain_vtn);
    return result_code;
  }
  val_vtn *vtn_val = reinterpret_cast<val_vtn*>
                                   (GetVal(ck_vtn));
  if (!vtn_val) {
      UPLL_LOG_DEBUG("Invalid param");
      DELETE_IF_NOT_NULL(ck_vtn);
      DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
      DELETE_IF_NOT_NULL(ck_domain_vtn);
      return UPLL_RC_ERR_GENERIC;
  }
  uint8_t valid_vtn_desc = vtn_val->valid[UPLL_IDX_DESC_VTN];

  if (req->datatype != UPLL_DT_AUDIT) {
    dt_type = req->datatype;
    result_code = mgr->ValidateCapability(
    req, ck_vtn, reinterpret_cast<char *>(ctrlr_dom->ctrlr));
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
      DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
      DELETE_IF_NOT_NULL(ck_domain_vtn);
      DELETE_IF_NOT_NULL(ck_vtn);
      return result_code;
    }
  } else {
      dt_type = UPLL_DT_AUDIT;
  }

  dbop.matchop = kOpMatchCtrlr|kOpMatchDomain;
  /*Checks VTN exists on the controller and doamin*/
  result_code = mgr->ReadConfigDB(ck_domain_vtn, req->datatype,
                              UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code &&
      UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code)
      DELETE_IF_NOT_NULL(ck_vtn);
      DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
      DELETE_IF_NOT_NULL(ck_domain_vtn);
      return result_code;
  }

  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      bool auto_rename = true;
      if (UPLL_DT_CANDIDATE == req->datatype) {
        ck_vtn_cntrlr->SetCfgVal(NULL);
        SET_USER_DATA_FLAGS(ck_vtn_cntrlr, NO_RENAME);
        dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;
        result_code = mgr->ReadConfigDB(ck_vtn_cntrlr, UPLL_DT_RUNNING,
            UNC_OP_READ, dbop, dmi, RENAMETBL);
        if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
          UPLL_LOG_DEBUG("GetRenamedControllerKey failed %d", result_code)
            DELETE_IF_NOT_NULL(ck_vtn);
          DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
          DELETE_IF_NOT_NULL(ck_domain_vtn);
          return result_code;
        }
        if (UPLL_RC_SUCCESS == result_code) {
          val_rename_vtn_t* rename_vtn = reinterpret_cast<val_rename_vtn_t *>(
              GetVal(ck_vtn_cntrlr));
          rename_vtn->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
          result_code = mgr->UpdateConfigDB(ck_vtn_cntrlr, req->datatype,
              UNC_OP_CREATE, dmi, RENAMETBL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code)
              DELETE_IF_NOT_NULL(ck_vtn);
            DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
            DELETE_IF_NOT_NULL(ck_domain_vtn);
            return result_code;
          }
          SET_USER_DATA_FLAGS(ck_vtn_cntrlr, VTN_RENAME);
          auto_rename = false;
        } else {
          dbop.inoutop = kOpInOutFlag;
          result_code = mgr->ReadConfigDB(ck_vtn_cntrlr, UPLL_DT_RUNNING,
              UNC_OP_READ, dbop, dmi, CTRLRTBL);
          if (UPLL_RC_SUCCESS != result_code &&
              UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_DEBUG("ReadConfigDb failed %d", result_code)
              DELETE_IF_NOT_NULL(ck_vtn);
            DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
            DELETE_IF_NOT_NULL(ck_domain_vtn);
            return result_code;
          } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_TRACE("No Entry in Controller table");
            result_code = UPLL_RC_SUCCESS;
          } else if (result_code == UPLL_RC_SUCCESS && !ctrlr_vtn_flag) {
            UPLL_LOG_TRACE("Entry in Controller table");
            auto_rename = false;
          }
        }
        uint8_t rename = 0x00;
        GET_USER_DATA_FLAGS(ck_vtn_cntrlr, rename);
        SET_USER_DATA_FLAGS(ck_domain_vtn, rename);
        ck_vtn_cntrlr->SetCfgVal(NULL);
      } else {
          auto_rename = false;
      }
      if (auto_rename) {
        dbop.matchop = kOpMatchCtrlr;
        /* If the vtn exists in controller and not in domain
         * then we have to rename the vtn name */
        result_code = mgr->ReadConfigDB(ck_vtn_cntrlr, req->datatype,
            UNC_OP_READ, dbop, dmi, CTRLRTBL);
        if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
          UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code);
          DELETE_IF_NOT_NULL(ck_vtn);
          DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
          DELETE_IF_NOT_NULL(ck_domain_vtn);
          return result_code;
        }
        if ((UPLL_RC_SUCCESS == result_code) || ctrlr_vtn_flag) {
          /* The VTN name is available in the controller
           * but not in the domain, so we have to create the new name
           * for the give vtn */
          std::string domain = reinterpret_cast<const char*>(ctrlr_dom->domain);
          std::string vtn_name = reinterpret_cast<const char*>
            (reinterpret_cast<key_vtn_t*>(
                                          ck_domain_vtn->get_key())->vtn_name);
          if (strlen(vtn_name.c_str())>=10) {
            vtn_name.assign(vtn_name.c_str(), 10);
          }
          struct timeval _timeval;
          struct timezone _timezone;
          gettimeofday(&_timeval, &_timezone);
          /* Renaming the VTN name based on the Time and Micro seconds */
          std::string vtn_domain_name = vtn_name+"_"+
            static_cast<std::ostringstream*>(
                &(std::ostringstream() << _timeval.tv_sec))->str() +
            static_cast<std::ostringstream*>(
                &(std::ostringstream() << _timeval.tv_usec))->str();
          val_rename_vtn_t* rename_vtn = reinterpret_cast<val_rename_vtn_t*>(
              ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
          uuu::upll_strncpy(rename_vtn->new_name, vtn_domain_name.c_str(),
              (kMaxLenVtnName+1));
          rename_vtn->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;
          ck_vtn->SetCfgVal(new ConfigVal(IpctSt::kIpcStValRenameVtn, rename_vtn));
          SET_USER_DATA(ck_vtn, ck_domain_vtn);
          dbop.matchop = kOpMatchNone;
          dbop.inoutop = kOpInOutCtrlr|kOpInOutDomain;
          /* Create Entry in Rename Table */
          result_code = mgr->UpdateConfigDB(ck_vtn, req->datatype,
              UNC_OP_CREATE, dmi, &dbop, RENAMETBL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("UpdateconfigDB failed");
            DELETE_IF_NOT_NULL(ck_vtn);
            DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
            DELETE_IF_NOT_NULL(ck_domain_vtn);
            return result_code;
          }
          SET_USER_DATA_FLAGS(ck_domain_vtn, VTN_RENAME);
          SET_USER_DATA_FLAGS(ck_vtn, VTN_RENAME);
          ck_vtn->SetCfgVal(NULL);
          /* UpdateRename Flag In VTN Main Table*/
          dbop.inoutop = kOpInOutFlag;
          result_code = mgr->UpdateConfigDB(ck_vtn, req->datatype,
              UNC_OP_UPDATE, dmi, &dbop, MAINTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("UpdateconfigDB failed");
            DELETE_IF_NOT_NULL(ck_vtn);
            DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
            DELETE_IF_NOT_NULL(ck_domain_vtn);
            return result_code;
          }
          SET_USER_DATA_FLAGS(ikey, VTN_RENAME);
        }
      }
      ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>
        (GetVal(ck_domain_vtn));
      if (!ctrlr_val) {
        UPLL_LOG_DEBUG("Val is empty");
        DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
        DELETE_IF_NOT_NULL(ck_domain_vtn);
        DELETE_IF_NOT_NULL(ck_vtn);
        return UPLL_RC_ERR_GENERIC;
      }
      ctrlr_val->ref_count = 1;
      ctrlr_val->down_count = 0;
      ctrlr_val->flags = 0;
      ctrlr_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      ctrlr_val->alarm_status = UPLL_ALARM_CLEAR;
      if (valid_vtn_desc  == UNC_VF_NOT_SUPPORTED)
        ctrlr_val->cs_attr[0] = UNC_CS_NOT_SUPPORTED ;
      else if (valid_vtn_desc  == UNC_VF_VALID)
        ctrlr_val->cs_attr[0] = UNC_CS_NOT_APPLIED ;
      ctrlr_val->cs_row_status  = UNC_CS_NOT_APPLIED;
      /* Inform to the POM keytypes for the new entry in vtn controller table*/
      if (! OVERLAY_KT(ikey->get_key_type())) {
        result_code = IntimatePOMAboutNewController(ikey, ctrlr_dom,
                                                    dmi, UNC_OP_CREATE, dt_type);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in updating POM Manager"
                         " about the addition of a new Controller");
          DELETE_IF_NOT_NULL(ck_vtn);
          DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
          DELETE_IF_NOT_NULL(ck_domain_vtn);
          return result_code;
        }
      }
      op = UNC_OP_CREATE;
  } else if (UPLL_RC_SUCCESS == result_code) {
    ctrlr_val = reinterpret_cast<val_vtn_ctrlr *>
        (GetVal(ck_domain_vtn));
    if (!ctrlr_val) {
      UPLL_LOG_DEBUG("Val is empty");
      DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
      DELETE_IF_NOT_NULL(ck_domain_vtn);
      DELETE_IF_NOT_NULL(ck_vtn);
      return UPLL_RC_ERR_GENERIC;
    }

    ctrlr_val->ref_count++;
    op = UNC_OP_UPDATE;
  }
  uint8_t vtn_rename = 0x00;
  GET_USER_DATA_FLAGS(ck_domain_vtn, vtn_rename);
  vtn_rename &= VTN_RENAME;
  SET_USER_DATA_FLAGS(ck_domain_vtn, vtn_rename);
  // ck_vtn_cntrlr->SetCfgVal(new ConfigVal(IpctSt::kIpcInvalidStNum, ctrlr_val));
  /* Create/Update Entry in Controller Table*/
  // result_code = mgr->UpdateConfigDB(ck_vtn_cntrlr, req->datatype,
  result_code = mgr->UpdateConfigDB(ck_domain_vtn, req->datatype,
                                    op, dmi, CTRLRTBL);
  DELETE_IF_NOT_NULL(ck_vtn);
  DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
  DELETE_IF_NOT_NULL(ck_domain_vtn);
  return result_code;
}

upll_rc_t VnodeMoMgr::IntimatePOMAboutNewController(ConfigKeyVal *ikey,
                                        controller_domain *ctrlr_dom,
                                        DalDmlIntf *dmi,
                                        unc_keytype_operation_t op,
                                        upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  /* For Audit case, intimate to POM key type not required 
   * because POM key type ctrlr table are populated before
   * vnodes creation in audit table
   */
  if ((dt_type == UPLL_DT_AUDIT) || (dt_type == UPLL_DT_IMPORT)) {
    return result_code;
  }
  unc_key_type_t pom_keys[] = { UNC_KT_VTN_FLOWFILTER,
                                UNC_KT_VTN_FLOWFILTER_ENTRY,
                                UNC_KT_VTN_POLICINGMAP};
  MoMgrImpl *pom_mgr = NULL;
  uint8_t *vtn_name = (reinterpret_cast<key_vtn*>(ikey->get_key()))->vtn_name;
  if (!vtn_name) return UPLL_RC_ERR_GENERIC;
  UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom->ctrlr,
                 ctrlr_dom->domain);

  int npom_mgr = sizeof(pom_keys)/sizeof(pom_keys[0]);
  uint8_t flag = 0;
  GET_USER_DATA_FLAGS(ikey, flag);
  for (uint8_t count = 0; count < npom_mgr; count++) {
    pom_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>(GetMoManager(
              pom_keys[count])));
    if (!pom_mgr) {
      UPLL_LOG_DEBUG("error in fetching MoMgr reference");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = pom_mgr->UpdateControllerTableForVtn(vtn_name,
                              ctrlr_dom, op, dt_type, dmi, flag);
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
    DELETE_IF_NOT_NULL(parent_key);
    return result_code;
  }
  result_code = ReadConfigDB(ck_vbr, req->datatype, UNC_OP_READ, dbop1,
                                  dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS)
    result_code = GetControllerDomainId(ck_vbr, &ctrlr_dom);

  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    UPLL_LOG_INFO("Invalid ctrlr/domain");
    DELETE_IF_NOT_NULL(parent_key);
    return result_code;
  }
  UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);
  SET_USER_DATA_CTRLR_DOMAIN(parent_key, ctrlr_dom);
  // delete ck_vbr;
  /* GetReference count from vtn controller table */
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
          UNC_KT_VTN)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    DELETE_IF_NOT_NULL(parent_key);
    DELETE_IF_NOT_NULL(ck_vbr);
    return UPLL_RC_ERR_GENERIC;
  }
  dbop.matchop = kOpMatchCtrlr | kOpMatchDomain ;
  result_code = mgr->ReadConfigDB(parent_key, req->datatype, UNC_OP_READ, dbop,
                                  dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) return result_code;
  val_vtn_ctrlr *vtn_st_val = reinterpret_cast<val_vtn_ctrlr *>
                                   (GetVal(parent_key));
  if (!vtn_st_val) {
    UPLL_LOG_DEBUG("Val is empty");
    DELETE_IF_NOT_NULL(parent_key);
    DELETE_IF_NOT_NULL(ck_vbr);
    return UPLL_RC_ERR_GENERIC;
  }

  if (1 == vtn_st_val->ref_count) {
    if (! OVERLAY_KT(ikey->get_key_type())) {
      result_code = IntimatePOMAboutNewController(parent_key, &ctrlr_dom,
                                                dmi, UNC_OP_DELETE, req->datatype);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("IntimatePOMAboutNewController %d", result_code);
        DELETE_IF_NOT_NULL(parent_key);
        DELETE_IF_NOT_NULL(ck_vbr);
        return result_code;
      }
    }
  }
  delete ck_vbr;

  /*  Delete the Current Node */
  if (1 == vtn_st_val->ref_count) {
    result_code = mgr->UpdateConfigDB(parent_key, req->datatype, UNC_OP_DELETE,
                                      dmi, &dbop,  CTRLRTBL);
    if (UPLL_RC_SUCCESS != result_code) {
       UPLL_LOG_DEBUG("UpdateconfigDB Failed %d", result_code);
       DELETE_IF_NOT_NULL(parent_key);
       return result_code;
    }
    parent_key->SetCfgVal(NULL);
    result_code = mgr->UpdateConfigDB(parent_key, req->datatype, UNC_OP_DELETE,
                                      dmi, &dbop, RENAMETBL);
    if (UPLL_RC_SUCCESS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_TRACE("UpdateConfigDB Failed %d", result_code);
      DELETE_IF_NOT_NULL(parent_key);
      return result_code;
    }
    result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?
                   UPLL_RC_SUCCESS:result_code;
  } else {
    // Reduce the ref count if vnode is not last node.
    vtn_st_val->ref_count--;
    result_code = mgr->UpdateConfigDB(parent_key, req->datatype, UNC_OP_UPDATE,
                                      dmi, &dbop,  CTRLRTBL);
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
  if (!header || !ikey || !dmi) {
    UPLL_LOG_DEBUG("Invalid input parameters");
    return UPLL_RC_ERR_GENERIC;
  }
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
      DELETE_IF_NOT_NULL(okey);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("Record doesn't exist in DB. Error code : %d",
                     result_code);
        return result_code;
      }
      // Getting the controller vtn, vnode name
      result_code = GetRenamedControllerKey(ikey, UPLL_DT_RUNNING, dmi,
                                          &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetRenamedControllerKey failed. Result : %d",
                          result_code);
        return result_code;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
      UPLL_LOG_TRACE("After Read  %s", (ikey->ToStrAll()).c_str());

      IpcResponse ipc_resp;
      memset(&(ipc_resp), 0, sizeof(IpcResponse));
      IpcRequest ipc_req;
      memset(&ipc_req, 0, sizeof(ipc_req));
      memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
      ipc_req.ckv_data = ikey;

      if (!uui::IpcUtil::SendReqToDriver((const char *)(ctrlr_dom.ctrlr),
          reinterpret_cast<char *>(ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
          PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
        UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                      ikey->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
        return ipc_resp.header.result_code;
      }

      // Populate ConfigKeyVal and IpcReqRespHeader with the response from driver
      ikey->ResetWith(ipc_resp.ckv_data);
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      result_code = (header->result_code = ipc_resp.header.result_code);
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
  UPLL_FUNC_TRACE;
  unc_key_type_t *ktype, if_ktype[] = { UNC_KT_VBR_IF, UNC_KT_VRT_IF,
                                        UNC_KT_VUNK_IF, UNC_KT_VTEP_IF,
                                        UNC_KT_VTUNNEL_IF };
  unc_key_type_t vnode_ktype[] = { UNC_KT_VBRIDGE, UNC_KT_VROUTER, UNC_KT_VTEP,
                                   UNC_KT_VTUNNEL, UNC_KT_VUNKNOWN, UNC_KT_VTERMINAL};
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
     case UNC_KT_VTERMINAL:
      case UNC_KT_VROUTER:
      case UNC_KT_VUNKNOWN:
      case UNC_KT_VTUNNEL:
      case UNC_KT_VTEP: {
        const pfc_ipcstdef_t *key_stdef = IpctSt::GetIpcStdef(
            ck_val->get_st_num());
        if (sizeof(key_vnode_t) != key_stdef->ist_size) {
          DELETE_IF_NOT_NULL(ck_val);
          return UPLL_RC_ERR_GENERIC;
        }
        memcpy(ck_val->get_key(), key, sizeof(key_vnode_t));
        break;
      }
      case UNC_KT_VBR_IF:
      case UNC_KT_VRT_IF:
      case UNC_KT_VUNK_IF:
      case UNC_KT_VTUNNEL_IF:
      case UNC_KT_VTEP_IF: {
        const pfc_ipcstdef_t *key_stdef = IpctSt::GetIpcStdef(
            ck_val->get_st_num());
        if (sizeof(*reinterpret_cast<const key_vbr_if_t *>(key))
                         != key_stdef->ist_size) {
          DELETE_IF_NOT_NULL(ck_val);
          return UPLL_RC_ERR_GENERIC;
        }
        memcpy(ck_val->get_key(), key, sizeof(key_vbr_if_t));
        break;
      }
      default:
        DELETE_IF_NOT_NULL(ck_val);
        return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
    }
    DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone,
                      kOpInOutFlag | kOpMatchCtrlr | kOpInOutDomain };
    result_code = mgr->ReadConfigDB(ck_val, dt_type,
                                              UNC_OP_READ, dbop1, dmi, MAINTBL);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(ck_val);
      continue;
    } else if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Error in reading %d", result_code);
      DELETE_IF_NOT_NULL(ck_val);
      return result_code;
    } else {
      return UPLL_RC_SUCCESS;
    }
  }
  DELETE_IF_NOT_NULL(ck_val);
  return UPLL_RC_ERR_CFG_SEMANTIC;
}

upll_rc_t VnodeMoMgr::CopyKeyToVal(ConfigKeyVal *ikey,
                                 ConfigKeyVal *&okey) {
  if (!ikey)
    return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = GetChildConfigKey(okey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  val_rename_vnode *val = reinterpret_cast<val_rename_vnode_t *>(
                          ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
  // Note: Validate message is take care of validate the key part
  key_vbr_t *key = reinterpret_cast<key_vbr_t *>(ikey->get_key());
  uuu::upll_strncpy(val->ctrlr_vtn_name, key->vtn_key.vtn_name,
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy(val->ctrlr_vnode_name,
                    key->vbridge_name,(kMaxLenVnodeName+1));
  val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
  val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
  okey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValRenameVbr, val));
  return UPLL_RC_SUCCESS;
}

upll_rc_t VnodeMoMgr::UpdateParentOperStatus(ConfigKeyVal *ikey, 
                                             DalDmlIntf *dmi, uint32_t driver_result) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_ERROR("Returning error \n");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ktype = ikey->get_key_type();
  if ((ktype != UNC_KT_VBRIDGE) && (ktype != UNC_KT_VROUTER) &&
      ktype != UNC_KT_VTERMINAL)
    return UPLL_RC_SUCCESS;
  VtnMoMgr *mgr = reinterpret_cast<VtnMoMgr *>(const_cast<MoManager *>
                               (GetMoManager(UNC_KT_VTN)));
  uint32_t count = 0;

  ConfigKeyVal *dup_ikey = NULL;
  result_code = GetParentConfigKey(dup_ikey,ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetInstanceCount(dup_ikey, NULL, UPLL_DT_RUNNING, &count, dmi,
       CTRLRTBL);
  DELETE_IF_NOT_NULL(dup_ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  if (count == 0) {
    ConfigKeyVal *ck_vtn = NULL;
    result_code = GetParentConfigKey(ck_vtn,ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      return result_code;
    }
    DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
    result_code = mgr->ReadConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_READ, dbop,
                               dmi, MAINTBL);
    if (result_code == UPLL_RC_SUCCESS) {
      val_db_vtn_st *vtn_st = reinterpret_cast<val_db_vtn_st*>GetStateVal(ck_vtn);
      vtn_st->vtn_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
      vtn_st->vtn_val_st.valid[0]= UNC_VF_VALID;
      vtn_st->down_count = 0;
      vtn_st->unknown_count = 0;
      result_code = mgr->UpdateConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_UPDATE, dmi, &dbop, MAINTBL);
      UPLL_LOG_DEBUG("Last vnode in ctrlr table %s\n",
               (reinterpret_cast<key_vbr *>(ikey->get_key()))->vbridge_name);
      //clear path fault alarm if last vnode delete
      controller_domain ctrlr_dom;
      ctrlr_dom.ctrlr = NULL;
      ctrlr_dom.domain = NULL;
      GET_USER_DATA_CTRLR_DOMAIN(ck_vtn, ctrlr_dom);
      uuc::UpllConfigMgr *cfg_instance =
          uuc::UpllConfigMgr::GetUpllConfigMgr();
      const char *ctr_name = reinterpret_cast<const char*>(ctrlr_dom.ctrlr);
      const char *dom_name = reinterpret_cast<const char*>(ctrlr_dom.domain);
      if(uuc::CtrlrMgr::GetInstance()->IsPathFaultOccured(ctr_name, dom_name)) {

        key_vtn *vtn = static_cast<key_vtn *>(ck_vtn->get_key());
        const char *vtn_name = reinterpret_cast<const char*>(vtn->vtn_name);
        UPLL_LOG_DEBUG("clear Path Fault alarm");
        bool bResult(false);
        bResult = cfg_instance->SendPathFaultAlarm( ctr_name, dom_name,
                                                    vtn_name,uuc::UPLL_CLEAR_WITH_TRAP);
        if(!bResult) {
          UPLL_LOG_DEBUG("Pathfault Alarm clear is failed");
        }
      }
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Error in reading VTN TBL");
    }
    DELETE_IF_NOT_NULL(ck_vtn);
    return result_code;
  }
  if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
    return UPLL_RC_SUCCESS;
  }

  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain};
  result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ,
                                  dbop, dmi, MAINTBL); 
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n",result_code);
    return result_code;
  }
  val_db_vbr_st *valst = reinterpret_cast<val_db_vbr_st *>(GetStateVal(ikey)); 
  if (!valst) {
    UPLL_LOG_DEBUG("Returning error\n");
    return UPLL_RC_ERR_GENERIC;
  }
  if (valst->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] == UNC_VF_VALID) {
    if ((valst->down_count == 0) && (valst->unknown_count == 0)) {
      /* return, if OperStatus is UP.
       * return if OperStatus is UNKNWON due to controller disconnect */
      return UPLL_RC_SUCCESS; 
    }
    ConfigKeyVal *ck_vtn = NULL;
    // decrement the down count in vtn controller table
    result_code = GetParentConfigKey(ck_vtn,ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      return result_code;
    }
    DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain, kOpInOutNone};
    result_code = mgr->ReadConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_READ, dbop,
                                 dmi, CTRLRTBL);
    if (result_code == UPLL_RC_SUCCESS) {
      state_notification notfn = kCommit; // do nothing
      if (valst->unknown_count > 0) {
        notfn = kPortFaultResetFromUnknown;
      } else if (valst->down_count > 0) {
        notfn = kPortFaultReset;
      } else {
        DELETE_IF_NOT_NULL(ck_vtn);
        return UPLL_RC_SUCCESS; 
      } 
      result_code = mgr->UpdateOperStatus(ck_vtn, dmi, notfn, true);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d\n",result_code);
        DELETE_IF_NOT_NULL(ck_vtn);
        return result_code;
      }
    } else {
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Error in ReadConfigDB");
        DELETE_IF_NOT_NULL(ck_vtn);
        return result_code;
      }
      UPLL_LOG_DEBUG("Last vnode of ctr-dom in ctrlr table %s\n",
               (reinterpret_cast<key_vbr *>(ikey->get_key()))->vbridge_name);
      //clear path fault alarm if last vnode delete
      controller_domain ctrlr_dom = {NULL, NULL};
      GET_USER_DATA_CTRLR_DOMAIN(ck_vtn, ctrlr_dom);
      uuc::UpllConfigMgr *cfg_instance =
          uuc::UpllConfigMgr::GetUpllConfigMgr();
      const char *ctr_name = reinterpret_cast<const char*>(ctrlr_dom.ctrlr);
      const char *dom_name = reinterpret_cast<const char*>(ctrlr_dom.domain);
      uuc::CtrlrMgr* ctr_mgr = uuc::CtrlrMgr::GetInstance();
      bool is_path_fault = ctr_mgr->IsPathFaultOccured(ctr_name, dom_name);
      if(is_path_fault) {
        key_vtn *vtn = static_cast<key_vtn *>(ck_vtn->get_key());
        const char *vtn_name = reinterpret_cast<const char*>(vtn->vtn_name);
        UPLL_LOG_DEBUG("clear Path Fault alarm");
        bool bResult(false);
        bResult = cfg_instance->SendPathFaultAlarm( ctr_name, dom_name,
                                                    vtn_name,uuc::UPLL_CLEAR_WITH_TRAP);
        if(!bResult) {
          UPLL_LOG_DEBUG("Pathfault Alarm clear is failed");
        }
      }
      if (is_path_fault || valst->vbr_val_st.oper_status != UPLL_OPER_STATUS_UP) {
        ConfigKeyVal *main_vtn = NULL;
        result_code = GetParentConfigKey(main_vtn, ikey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in GetParentConfigKey");
          DELETE_IF_NOT_NULL(ck_vtn);
          return result_code;
        }
        DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
        result_code = mgr->ReadConfigDB(main_vtn, UPLL_DT_STATE, UNC_OP_READ, dbop,
                                         dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in ReadConfigDB");
          DELETE_IF_NOT_NULL(ck_vtn);
          DELETE_IF_NOT_NULL(main_vtn);
          return result_code;
        }
        state_notification notfn = kCommit; // no op;
        if (valst->unknown_count > 0)
          notfn = kPortFaultResetFromUnknown;
        else if (valst->down_count > 0)
          notfn = kPortFaultReset;
        result_code = mgr->SetOperStatus(main_vtn, notfn, dmi); 
        DELETE_IF_NOT_NULL(main_vtn);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in SetOperStatus");
          DELETE_IF_NOT_NULL(ck_vtn);
          return result_code;
        }
      }
    }
    DELETE_IF_NOT_NULL(ck_vtn);
  }
  return result_code; 
}

upll_rc_t VnodeMoMgr::UpdateOperStatus(ConfigKeyVal *ikey,
                              DalDmlIntf *dmi,
                              state_notification notification, bool skip) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_DEBUG("Returning error");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ktype = ikey->get_key_type();
  if (!skip) {
      DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
                       kOpInOutCtrlr | kOpInOutDomain };
      result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Error in reading: %d", result_code);
        return result_code;
      }
  } 
  ConfigKeyVal *tkey, *tkey_next = ikey;
  while (tkey_next != NULL) {
    tkey = tkey_next;
    tkey_next = tkey->get_next_cfg_key_val();
    tkey->set_next_cfg_key_val(NULL);
    bool oper_change = false;

    switch (ktype) {
      case UNC_KT_VBRIDGE:
        result_code = SetOperStatus<val_vbr_st_t, val_db_vbr_st_t *>(
            tkey,  dmi, notification, oper_change);
        break;
      case UNC_KT_VROUTER:
        result_code = SetOperStatus<val_vrt_st_t, val_db_vrt_st_t *>(
            tkey,  dmi, notification, oper_change);
        break;
      case UNC_KT_VTEP:
        result_code = SetOperStatus<val_vtep_st_t, val_db_vtep_st_t *>(
            tkey,  dmi, notification, oper_change);
        break;
      case UNC_KT_VTUNNEL:
        result_code = SetOperStatus<val_vtunnel_st_t, val_db_vtunnel_st_t *>(
             tkey,  dmi, notification, oper_change);
        break;
      case UNC_KT_VTERMINAL:
        result_code = SetOperStatus<val_vterm_st_t, val_db_vterm_st_t *>(
             tkey,  dmi, notification, oper_change);
        break;

      default:
        result_code = UPLL_RC_ERR_GENERIC;
        UPLL_LOG_ERROR("Operstatus attribute not supported for this kt %d",
                       ktype);
        break;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Error in vNode SetOperStatus : %d", result_code);
      return result_code;
    }
    if (oper_change) {
       VtnMoMgr *mgr = reinterpret_cast<VtnMoMgr *>
                    (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
       if (!mgr) {
         UPLL_LOG_ERROR("Invalid mgr");
         return UPLL_RC_ERR_GENERIC;
       }
       ConfigKeyVal *ck_vtn = NULL;
       result_code = GetParentConfigKey(ck_vtn, tkey);
       if (!ck_vtn || result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("Returning error %d", result_code);
         return result_code;
       }
       result_code = mgr->UpdateOperStatus(ck_vtn, dmi, notification, false);
       delete ck_vtn;
       if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("VTN UpdateOperStatus failed");
         return result_code;
       }
    }
  }
  return result_code;
}


template<typename T1, typename T2>
upll_rc_t VnodeMoMgr::SetOperStatus(ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi,
                                    state_notification &notification,
                                    bool &oper_change) {
  /* update corresponding vnode operstatus */
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *tmp =
      (ikey->get_cfg_val()) ? ikey->get_cfg_val()->get_next_cfg_val() : NULL;
  T2 vn_valst = (T2)((tmp != NULL) ? tmp->get_val() : NULL);
  if (vn_valst == NULL) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  T1 *vn_val = reinterpret_cast<T1 *>(vn_valst);
  /* Update oper status based on notification */
  vn_val->valid[0] = UNC_VF_VALID;
  UPLL_LOG_DEBUG("notification %d down_count %d unknown_count %d", notification,
                  vn_valst->down_count, vn_valst->unknown_count);
  switch (notification) {
    case kPortUp:
      if (vn_valst->down_count == STAND_ALONE_VNODE) {
        oper_change = true;
        vn_valst->down_count = 0;
        vn_val->oper_status = UPLL_OPER_STATUS_UP;
        notification = kPortFaultReset;
      } else {
        // vNode is already in UP/DOWN/UNKNWOWN state.
        // An Up interface should not affect the present vNode OperStatus
        return UPLL_RC_SUCCESS;
      }
    case kReConnect:
      oper_change = true;
      if (vn_valst->unknown_count > 0) {
        vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
        notification = kPortUnknown;
      } else if (vn_valst->down_count > 0) {
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
        notification = kPortFault;
      }
      break;
    case kCommit:
      oper_change = true;
      if (vn_valst->unknown_count > 0) {
        if (vn_val->oper_status == UPLL_OPER_STATUS_UNKNOWN)
         oper_change = false;
        else if (vn_val->oper_status == UPLL_OPER_STATUS_DOWN)
          notification = kPortUnknownFromDown;
        else
          notification = kPortUnknown;
        vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      } else if (vn_valst->down_count > 0) {
       if (vn_val->oper_status == UPLL_OPER_STATUS_DOWN)
          oper_change = false;
        else if (vn_val->oper_status == UPLL_OPER_STATUS_UNKNOWN)
          notification = kPortFaultFromUnknown;
        else
          notification = kPortFault;
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      } else if (vn_val->oper_status == UPLL_OPER_STATUS_UNKNOWN) {
        /* controller was disconnected initially */
        vn_val->oper_status = UPLL_OPER_STATUS_UP;
      } else {
        oper_change = false;
        if (vn_val->oper_status == UPLL_OPER_STATUS_UP) {
          // A Port UP interface is added to a vNode which is already UP. So return
          return UPLL_RC_SUCCESS;
        } else if (vn_val->oper_status == UPLL_OPER_STATUS_DOWN) {
          // vNode is DOWN due to initial PathFault
          vn_val->oper_status = UPLL_OPER_STATUS_UP;
          oper_change = true;
          notification = kPortFaultReset;
        } else if (vn_val->oper_status == UPLL_OPER_STATUS_UNINIT) {
          vn_val->oper_status = UPLL_OPER_STATUS_UP;
        }
      }
      break;
    case kPortUnknown:
      if (vn_valst->down_count == STAND_ALONE_VNODE) {
        notification = kPortUnknownFromDown;
        vn_valst->down_count = 0;
      }
      vn_valst->unknown_count++;
      if (vn_valst->unknown_count == 1) {
        oper_change = true;
        if (vn_valst->down_count > 0) {
          notification = kPortUnknownFromDown;
        }
        vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      }
      break;
    case kPortUnknownFromDown:
      vn_valst->unknown_count++;
      if (vn_valst->down_count)
        vn_valst->down_count--;
      if (vn_valst->unknown_count == 1) {
        vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
        oper_change = true;
      } 
      break;
    case kPortFault:
      if (vn_valst->down_count == STAND_ALONE_VNODE) {
        vn_valst->down_count = 1;
      } else {
        vn_valst->down_count++;
        if ((vn_valst->down_count == 1) &&
            (vn_valst->unknown_count == 0)) {
          oper_change = true;
          vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
        }
      }
      break;
    case kPortFaultFromUnknown:
      if (vn_valst->unknown_count)
        vn_valst->unknown_count--;
      vn_valst->down_count++;
      if(vn_valst->unknown_count == 0) {
        oper_change = true;
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      }
      break;
    case kPortFaultReset:
      if (vn_valst->down_count)
        vn_valst->down_count--;
      if ((vn_valst->down_count == 0) &&
          (vn_valst->unknown_count == 0)) {
        oper_change = true;
        vn_val->oper_status = UPLL_OPER_STATUS_UP;
      }
      break;
    case kPortFaultResetFromUnknown:
      if (vn_valst->unknown_count)
        vn_valst->unknown_count--;
      if (vn_valst->unknown_count == 0) {
        oper_change = true;
        if (vn_valst->down_count == 0) {
          vn_val->oper_status = UPLL_OPER_STATUS_UP;
        } else {
          vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
          notification = kPortFaultFromUnknown;
        }
      } 
      break;
      default:
        UPLL_LOG_DEBUG("unsupported notification for operstatus update");
        return UPLL_RC_ERR_GENERIC;
      break;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                               &dbop, MAINTBL);
  UPLL_LOG_TRACE("Vnode SetOperstatus for VTN after Update is \n %s",
                    ikey->ToStr().c_str());
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in update oper status %d", result_code);
  }
  return result_code;
}

/* This function update the vnode operstatus
 * while doing commit
 */

bool VnodeMoMgr::EnqueOperStatusNotification(ConfigKeyVal *ikey,
                                             bool oper_status_change) {
  UPLL_FUNC_TRACE;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_DEBUG("Inputs are Invalid");
    return false;
  }
  uuc::UpllConfigMgr *cfg_instance = uuc::UpllConfigMgr::GetUpllConfigMgr();
  unc_key_type_t ktype  = ikey->get_key_type();
  uint8_t *vtn_name = NULL;
  uint8_t *vnode_name = NULL;

  switch (ktype) {
    case UNC_KT_VBRIDGE:
      vtn_name = reinterpret_cast<key_vbr*>
                   (ikey->get_key())->vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vbr*>
                     (ikey->get_key())->vbridge_name;
    break;
    case UNC_KT_VROUTER:
      vtn_name = reinterpret_cast<key_vrt*>
                     (ikey->get_key())->vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vrt*>
                     (ikey->get_key())->vrouter_name;
    break;
    case UNC_KT_VTUNNEL:
      vtn_name = reinterpret_cast<key_vtunnel*>
                     (ikey->get_key())->vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vtunnel*>
                     (ikey->get_key())->vtunnel_name;
    break;
    case UNC_KT_VTEP:
      vtn_name = reinterpret_cast<key_vtep*>
                   (ikey->get_key())->vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vtep*>
                     (ikey->get_key())->vtep_name;
    break;
    case UNC_KT_VTERMINAL:
      vtn_name = reinterpret_cast<key_vterm*>
                   (ikey->get_key())->vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vterm*>
                     (ikey->get_key())->vterminal_name;
    break;
    default:
       UPLL_LOG_DEBUG("Invalid KeyType");
  }
  return cfg_instance->SendOperStatusAlarm(reinterpret_cast<char*>(vtn_name),
                                           reinterpret_cast<char*>(vnode_name),
                                           reinterpret_cast<char*>(NULL),
                                           oper_status_change);
}

upll_rc_t VnodeMoMgr::TxUpdateDtState(unc_key_type_t ktype,
                                      uint32_t session_id,
                                      uint32_t config_id,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vn = NULL;
  result_code = GetUninitOperState(ck_vn, dmi); 
  if (UPLL_RC_SUCCESS != result_code || ck_vn == NULL)  {
    return result_code;
  }
  ConfigKeyVal *tkey = NULL;
  while (ck_vn) {
    tkey = ck_vn;
    ck_vn = tkey->get_next_cfg_key_val();
    tkey->set_next_cfg_key_val(NULL);
    uint32_t unknown_count = 0, down_count = 0;
    result_code = SetVnodeAndParentOperStatus (tkey, dmi, false, false,
                                              unknown_count, down_count);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(tkey);
      DELETE_IF_NOT_NULL(ck_vn);
      return result_code;
    }
    DELETE_IF_NOT_NULL(tkey);
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::SetVnodeAndParentOperStatus(
                ConfigKeyVal *ck_vnode, DalDmlIntf *dmi, bool is_stand_alone,
                bool recon, uint32_t unknown_count, uint32_t down_count) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t ktype = ck_vnode->get_key_type();
  val_db_vbr_st * vnode_runst = reinterpret_cast<val_db_vbr_st *>
                                              (GetStateVal(ck_vnode));
  if (!vnode_runst) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  state_notification notfn = kCommit;
  uint32_t orig_unknown_count = vnode_runst->unknown_count;
  uint32_t orig_down_count = vnode_runst->down_count;
  if (is_stand_alone == false) {
    /* get child key type */
    unc_key_type_t child_keytype;
    switch (ktype) {
    case UNC_KT_VBRIDGE:
      child_keytype = UNC_KT_VBR_IF;
      break;
    case UNC_KT_VROUTER:
      child_keytype = UNC_KT_VRT_IF;
      break;
    case UNC_KT_VTEP:
      child_keytype = UNC_KT_VTEP_IF;
      break;
    case UNC_KT_VTUNNEL:
      child_keytype = UNC_KT_VTUNNEL_IF;
      break;
    case UNC_KT_VTERMINAL:
      child_keytype = UNC_KT_VTERM_IF;
      break;
    default:
      UPLL_LOG_DEBUG("Unsupported operation on keytype %d", ktype);
      return UPLL_RC_ERR_GENERIC;
    }
    // create interface ConfigKeyVal with OperStatus set to DOWN */
    VnodeChildMoMgr *mgr = reinterpret_cast<VnodeChildMoMgr *>
                           (const_cast<MoManager *>(GetMoManager(child_keytype)));
    if (recon == false) {
      ConfigKeyVal *ck_vnif = NULL;
      result_code = mgr->GetCkvUninit(ck_vnif, ck_vnode, dmi, UPLL_OPER_STATUS_DOWN);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(ck_vnif);
        UPLL_LOG_DEBUG("Returning error %d\n",result_code);
        return result_code;
      }
      /* get the count of interfaces that are DOWN under a given vNode */
      result_code = mgr->GetInstanceCount(ck_vnif, NULL, UPLL_DT_STATE, &down_count, dmi,
                                          MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetInstanceCount is Failed %d", result_code);
        DELETE_IF_NOT_NULL(ck_vnif);
        return result_code;
      }
      if (ck_vnode->get_key_type() == UNC_KT_VBRIDGE ||
          ck_vnode->get_key_type() == UNC_KT_VTERMINAL) {
        DELETE_IF_NOT_NULL(ck_vnif);
        /* get the count of interfaces that are UNKNOWN under a given vNode */
        result_code = mgr->GetCkvUninit(ck_vnif, ck_vnode, dmi, UPLL_OPER_STATUS_UNKNOWN);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d\n",result_code);
          DELETE_IF_NOT_NULL(ck_vnif);
          return result_code;
        }
        result_code = mgr->GetInstanceCount(ck_vnif, NULL, UPLL_DT_STATE, &unknown_count, dmi,
                                            MAINTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("GetInstanceCount is Failed %d", result_code);
          DELETE_IF_NOT_NULL(ck_vnif);
          return result_code;
        } 
      }
      if ((down_count == 0) && (unknown_count == 0)) {
      /* check if there are interfaces which are UP under a given vNode */
        ck_vnif->SetCfgVal(NULL);
  
        DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
        result_code = mgr->UpdateConfigDB(ck_vnif, UPLL_DT_STATE, UNC_OP_READ,
                                          dmi, &dbop, MAINTBL);
      }
      DELETE_IF_NOT_NULL(ck_vnif);
    }
  } 
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE || is_stand_alone == true) {
    /* is_vnode_down is used to check if vNode is stand alone */
    bool is_vnode_down = false;
    if (ktype == UNC_KT_VBRIDGE) {
      controller_domain ctrlr_dom = {NULL, NULL};
      GET_USER_DATA_CTRLR_DOMAIN(ck_vnode, ctrlr_dom)
      uuc::CtrlrMgr* ctr_mgr = uuc::CtrlrMgr::GetInstance();
      if (ctr_mgr->IsPathFaultOccured(reinterpret_cast<char*>(ctrlr_dom.ctrlr),
                                 reinterpret_cast<char*>(ctrlr_dom.domain)))  {
        /* the added stand alone vNode is in Path fault controller domain */
        is_vnode_down = true;
      } else {
        /* Check if stand alone vNode is vlan-mapped.
         * If so, set OperStatus to UP */
        VlanMapMoMgr *vlan_mgr =
          reinterpret_cast<VlanMapMoMgr *>(const_cast<MoManager *>
          (const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_VLANMAP))));
        result_code = vlan_mgr->CheckIfVnodeisVlanmapped(ck_vnode, dmi);
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          /*if not vlan-mapped, OperStatus is DOWN */
          is_vnode_down = true;
        } else if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS){
          UPLL_LOG_DEBUG("Error in reading vlanmap entry");
          return result_code;
        }
      }
    } else {
      /* Stand alone vNode is not vBridge. So OperStatus is DOWN */
      is_vnode_down = true;
    }
    if (is_vnode_down == true) {
      if (vnode_runst->down_count == STAND_ALONE_VNODE) {
        // vNode is already stand-alone on a pthfault reset
        return UPLL_RC_SUCCESS;
      }
      notfn = kPortFault;
      return (SetStandAloneOperStatus(ck_vnode, notfn, dmi));
    }
  }
  if (unknown_count > 0) {
    if (orig_unknown_count == 0) {
      notfn = kReConnect;
    }
  } else if (down_count > 0) {
    if (orig_down_count == 0) {
      notfn = kReConnect;
    }
  }
  if (!(vnode_runst->vbr_val_st.oper_status == UPLL_OPER_STATUS_UNKNOWN &&
      vnode_runst->unknown_count == 0)) {
    if ((recon == true) && (orig_unknown_count == unknown_count &&
                            orig_down_count == down_count)) {
      return UPLL_RC_SUCCESS;
    }
  } 
  vnode_runst->unknown_count = unknown_count;
  vnode_runst->down_count = down_count;
  result_code = UpdateOperStatus(ck_vnode, dmi, notfn, true);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in update oper status %d", result_code);
    return result_code;
  }
  UPLL_LOG_TRACE("Vnode after Update is \n %s",
                  ck_vnode->ToStr().c_str());
  return result_code;
}

upll_rc_t VnodeMoMgr::UpdateLastInterfaceDelete(ConfigKeyVal *ikey, ConfigKeyVal *ck_vnode,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  /* is_vnode_down is used to check if vNode is stand alone */
  bool is_vnode_down = false;
  if (ck_vnode->get_key_type()  == UNC_KT_VBRIDGE) {
    controller_domain ctrlr_dom = {NULL, NULL};
    GET_USER_DATA_CTRLR_DOMAIN(ck_vnode, ctrlr_dom)
      uuc::CtrlrMgr* ctr_mgr = uuc::CtrlrMgr::GetInstance();
    if (ctr_mgr->IsPathFaultOccured(reinterpret_cast<char*>(ctrlr_dom.ctrlr),
                               reinterpret_cast<char*>(ctrlr_dom.domain)))  {
      /* the added stand alone vNode is in Path fault controller domain */
      is_vnode_down = true;
    } else {
      /* Check if stand alone vNode is vlan-mapped.
       * If so, set OperStatus to UP */
      VlanMapMoMgr *vlan_mgr =
        reinterpret_cast<VlanMapMoMgr *>(const_cast<MoManager *>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_VLANMAP))));
      result_code = vlan_mgr->CheckIfVnodeisVlanmapped(ck_vnode, dmi);
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        is_vnode_down = false;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        /*if not vlan-mapped, OperStatus is DOWN */
        is_vnode_down = true;
      } else {
        UPLL_LOG_DEBUG("Error in reading vlanmap entry");
        return result_code;
      }
    }
  } else {
    /* Stand alone vNode is not vBridge. So OperStatus is DOWN */
    is_vnode_down = true;
  }
  ConfigVal *tmp = (ikey->get_cfg_val()) ?
                    ikey->get_cfg_val()->get_next_cfg_val() : NULL;
  val_db_vbr_if_st *if_valst = reinterpret_cast<val_db_vbr_if_st *>((tmp != NULL) ? tmp->get_val() : NULL);
  if (if_valst == NULL) {
    UPLL_LOG_DEBUG("Error in getting state val");
    return UPLL_RC_ERR_GENERIC;
  }
  state_notification notfn = kCommit; // no op
  if (is_vnode_down == true) {
    if (if_valst->down_count == PORT_UP) {
      notfn = kPortFault;
    } else if (if_valst->down_count & PORT_UNKNOWN) {
      notfn = kPortFaultFromUnknown;
    }
    result_code = SetStandAloneOperStatus(ck_vnode, notfn, dmi);
  } else {
    if (if_valst->down_count == PORT_UP) {
      /* If last interface was UP, then vNode continues to be UP
       * if interface was UNKNOWN due to controller disconnect,
       * vNode continues to be UNKNOWN*/
      return UPLL_RC_SUCCESS;
    } else if (if_valst->down_count & PORT_UNKNOWN) {
      notfn = kPortFaultResetFromUnknown;
    } else {
      notfn = kPortFaultReset;
    }
    result_code = UpdateOperStatus(ck_vnode, dmi, notfn, true);
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::SetStandAloneOperStatus(ConfigKeyVal *ck_vnode,
                      state_notification notfn, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_db_vbr_st *vbr_st = reinterpret_cast<val_db_vbr_st*>(GetStateVal(ck_vnode));
  if (vbr_st == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  vbr_st->down_count = STAND_ALONE_VNODE;
  vbr_st->unknown_count = 0;
  vbr_st->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
  vbr_st->vbr_val_st.valid[0] = UNC_VF_VALID;
  VnodeMoMgr *mgr =
  reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
                (const_cast<MoManager*>(GetMoManager(ck_vnode->get_key_type()))));
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = mgr->UpdateConfigDB(ck_vnode, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                           &dbop, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in UpdateConfigDB");
    return result_code;
  }
  if (notfn == kCommit) {
    // vNode was already DOWN due to a DOWN interface
    return UPLL_RC_SUCCESS;
  }
  VtnMoMgr *vtn_mgr = reinterpret_cast<VtnMoMgr *>
                (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
  if (!vtn_mgr) {
    UPLL_LOG_ERROR("Invalid mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ck_vtn = NULL;
  result_code = GetParentConfigKey(ck_vtn, ck_vnode);
  if (!ck_vtn || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }
  result_code = vtn_mgr->UpdateOperStatus(ck_vtn, dmi, notfn, false);
  DELETE_IF_NOT_NULL(ck_vtn);
  return result_code;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc

