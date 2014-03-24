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
                                        DalDmlIntf *dmi,
                                        bool restore_flag) {
  UPLL_FUNC_TRACE;
  if (!ikey || !(ikey->get_key()) || !req || !dmi) {
    UPLL_LOG_DEBUG("Cannot perform create operation");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  if (!restore_flag) {
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
  //  Vnode Existence check in CANDIDATE DB
  result_code = VnodeChecks(ikey, req->datatype, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(
        "Another Vnode with the same name already exists. Error code : %d",
        result_code);
    return result_code;
  }

  //  check for key support on controller and max count
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

  result_code = RestoreVnode(ikey, req, dmi, &ctrlr_dom, restore_flag);
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
  uint8_t *controller_id = reinterpret_cast<uint8_t *>(
      const_cast<char *>(ctrlr_id));

  /* check if object is renamed in the corresponding Rename Tbl
   * if "renamed"  create the object by the UNC name.
   * else - create using the controller name.
   */
  result_code = GetRenamedUncKey(ikey, UPLL_DT_RUNNING, dmi, controller_id);
  if (result_code != UPLL_RC_SUCCESS && result_code !=
      UPLL_RC_ERR_NO_SUCH_INSTANCE) {
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
  result_code = CheckVtnExistenceOnController(ikey, &req, &ctrlr_dom, dmi);
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
          reinterpret_cast<char *>(ctrlr_dom.ctrlr),
          req->datatype,
          &ctrlrtype)) {
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
    DbSubOp dbop = {kOpReadSingle,
      kOpMatchNone,
      kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
    result_code = ReadConfigDB(okey,
                               req->datatype,
                               UNC_OP_READ,
                               dbop,
                               dmi,
                               MAINTBL);
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
  // * other semantic validations *//
  if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
    result_code = IsHostAddrAndPrefixLenInUse(ikey, dmi, req);
  }
  // Commented for #403
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
                                   controller_domain_t *ctrlr_dom,
                                   bool restore_flag) {
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
  if (!restore_flag) {
    if (UPLL_DT_CANDIDATE == req->datatype) {
      result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
                                   MAINTBL);
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_DEBUG("Key instance exist");
        if ((ikey)->get_cfg_val()) {
          UPLL_LOG_DEBUG("Read Key with Value struct");
          switch (ikey->get_key_type()) {
            case UNC_KT_VROUTER: {
              val_vrt *vrt_val = reinterpret_cast<val_vrt *>
                  (GetVal(ikey));
              if (vrt_val != NULL) {
                vrt_val->valid[UPLL_IDX_DHCP_RELAY_ADMIN_STATUS_VRT] =
                    UNC_VF_INVALID;
              }
            }
            break;
            case UNC_KT_VLINK: {
              val_vlink_t *vlink_val = reinterpret_cast<val_vlink_t *>
                  (GetVal(ikey));
              if (vlink_val != NULL) {
                vlink_val->valid[UPLL_IDX_ADMIN_STATUS_VLNK] = UNC_VF_INVALID;
              }
            }
            break;
            default:
            UPLL_LOG_DEBUG("Other vnode child keytypes");
          }
          DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
            kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain };
          result_code = ReadConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dbop,
                                     dmi, MAINTBL);
          if (UPLL_RC_SUCCESS != result_code &&
              UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_DEBUG("ReadConfigDB Failed %d",  result_code);
          }
          if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)  {
            return UPLL_RC_ERR_CFG_SEMANTIC;
          }
        } else  {
          result_code = UPLL_RC_SUCCESS;
        }
        if (UPLL_RC_SUCCESS == result_code) {
          result_code = RestoreChildren(ikey,
                                        req->datatype,
                                        UPLL_DT_RUNNING,
                                        dmi,
                                        req);
          UPLL_LOG_DEBUG("Restore Children returns %d", result_code);
          return result_code;
        }
      } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("UpdateConfigDB Failed %d", result_code);
        return result_code;
      }
    } else {
      result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  } else {
    result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }

  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE ||
      UPLL_DT_IMPORT == req->datatype) {
    if (key_type == UNC_KT_VTEP || key_type == UNC_KT_VTUNNEL
        || key_type == UNC_KT_VTEP_GRP) {
      //   create a record in CANDIDATE DB
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
        return result_code;
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
  if (!restore_flag) {
    if (UPLL_DT_CANDIDATE == req->datatype) {
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
    }
  }
  /* set the controller domain in the ikey */
  //  SET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);

#if 0
  if (ikey->get_key_type() == UNC_KT_VLINK) {
    if (!ikey->get_cfg_val()) return UPLL_RC_ERR_GENERIC;
    SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom[1]);
  }
#endif
  //  create a record in CANDIDATE DB
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
      return result_code;
    }
  }
  // TODO(karthi): moved here, because after creating the vnode
  // we need to set the rename flag.

  /* If parent VTN is renamed, set the renamed flag in Vnode
   ** and create an entry in vnode rename table if VTN is renamed
   */
  if (!restore_flag) {
    if (UPLL_DT_IMPORT != req->datatype) {
      result_code = SetVnodeRenameFlag(ikey, req->datatype, ctrlr_dom, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        return result_code;
      }
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
                                    (const_cast<MoManager*>
                                     (GetMoManager(UNC_KT_VTN))));
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
  uint8_t *temp_vtn_name = NULL;
  uint8_t *temp_vnode_name = NULL;
  ConfigKeyVal *ck_rename_vnode = NULL;
  /*Check if parent vtn renamed and get the renamed name */
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                    (const_cast<MoManager*>(
                                            GetMoManager(UNC_KT_VTN))));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetParentConfigKey(parent_ck_vtn, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetParentConfigKey Failed");
    return result_code;
  }
  //  SET_USER_DATA_CTRLR_DOMAIN(parent_ck_vtn, *ctrlr_dom);
  UPLL_LOG_DEBUG("ctrlr dom %p %p %p\n", ctrlr_dom->ctrlr, ctrlr_dom->domain,
                 ikey->get_user_data());
  UPLL_LOG_DEBUG("%p %p %p %p \n", parent_ck_vtn, ikey, ikey->get_user_data(),
                 parent_ck_vtn->get_user_data());
  result_code = mgr->GetRenamedControllerKey(parent_ck_vtn, dt_type, dmi,
                                             ctrlr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(parent_ck_vtn);
    UPLL_LOG_DEBUG("GetRenamedControllerKey failed. Result : %d",
                   result_code);
    return result_code;
  }
  int rename = 0;
  GET_USER_DATA_FLAGS(parent_ck_vtn, rename);
  if (!(rename & RENAME)) {
    DELETE_IF_NOT_NULL(parent_ck_vtn);
    return UPLL_RC_SUCCESS;
  }
  // create entry in Vnode Rename Table-parent_ck_vtn contains the renamed name
  // TODO(karthi): Here removed CreateVnodeRenameEntry and combined
  GetVnodeName(ikey, temp_vtn_name, temp_vnode_name);
  if (temp_vtn_name == NULL || temp_vnode_name == NULL)
    return UPLL_RC_ERR_GENERIC;
  key_vnode* key_rename_vnode = reinterpret_cast<key_vnode*>
      (ConfigKeyVal::Malloc(sizeof(key_vnode)));
  uuu::upll_strncpy(key_rename_vnode->vtn_key.vtn_name,
                    temp_vtn_name, (kMaxLenVtnName+1) );
  uuu::upll_strncpy(key_rename_vnode->vnode_name,
                    temp_vnode_name, (kMaxLenVnodeName+1));

  val_rename_vnode_t* val_rename_vnode = reinterpret_cast<val_rename_vnode_t*>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode_t)));
  uuu::upll_strncpy(val_rename_vnode->ctrlr_vtn_name,
                    reinterpret_cast<key_vtn_t*>
                    (parent_ck_vtn->get_key())->vtn_name,
                    (kMaxLenVtnName+1));
  uuu::upll_strncpy(val_rename_vnode->ctrlr_vnode_name,
                    temp_vnode_name, (kMaxLenVnodeName+1));

  ck_rename_vnode = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcInvalidStNum,
                                     key_rename_vnode,
                                     new ConfigVal(
                                         IpctSt::kIpcInvalidStNum,
                                         val_rename_vnode));

  val_rename_vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
  val_rename_vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

  SET_USER_DATA_CTRLR_DOMAIN(ck_rename_vnode, *ctrlr_dom);
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};
  /* Create the Entry in Vnode rename table for vbr and vrt
   * Vlink rename table for vlink key type */

  result_code = UpdateConfigDB(ck_rename_vnode,
                               dt_type,
                               UNC_OP_CREATE,
                               dmi,
                               &dbop,
                               RENAMETBL);
  //   SET_USER_DATA_CTRLR(ck_rename_vnode, ctrlr_id);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Record Creation failed in %d", result_code);
    DELETE_IF_NOT_NULL(parent_ck_vtn);
    delete ck_rename_vnode;
    return result_code;
  }
  /* Update the Rename Flag for vnode in the main table */
  DELETE_IF_NOT_NULL(ck_rename_vnode);
  rename = 0;
  GET_USER_DATA_FLAGS(ikey, rename);
  rename |= VTN_RENAME;
  SET_USER_DATA_FLAGS(ikey, rename);
  dbop.matchop = kOpMatchNone;
  dbop.inoutop = kOpInOutFlag;
  result_code = UpdateConfigDB(ikey, dt_type, UNC_OP_UPDATE, dmi, &dbop,
                               MAINTBL);
  DELETE_IF_NOT_NULL(parent_ck_vtn);
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
    if (UPLL_RC_SUCCESS == result_code) {
      /* The VTN name is available in the controller
       * but not in the domain, so we have to create the new name
       * for the give vtn */
      std::string domain = reinterpret_cast<const char*>(ctrlr_dom->domain);
      std::string vtn_name = reinterpret_cast<const char*>
          (reinterpret_cast<key_vtn_t*>(
                  ck_domain_vtn->get_key())->vtn_name);
      if (strlen(vtn_name.c_str()) >= 10) {
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
              &(std::ostringstream() << _timeval.tv_usec) )->str();
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
      ctrlr_val->cs_attr[0] = UNC_CS_NOT_SUPPORTED;
    else if (valid_vtn_desc  == UNC_VF_VALID)
      ctrlr_val->cs_attr[0] = UNC_CS_NOT_APPLIED;
    ctrlr_val->cs_row_status  = UNC_CS_NOT_APPLIED;
    /* Inform to the POM keytypes for the new entry in vtn controller table*/
    if (!OVERLAY_KT(ikey->get_key_type())) {
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
  //  ck_vtn_cntrlr->SetCfgVal(new ConfigVal(IpctSt::kIpcInvalidStNum,
  //  ctrlr_val));
  /* Create/Update Entry in Controller Table*/
  //  result_code = mgr->UpdateConfigDB(ck_vtn_cntrlr, req->datatype,
  result_code = mgr->UpdateConfigDB(ck_domain_vtn, req->datatype,
                                    op, dmi, CTRLRTBL);
  DELETE_IF_NOT_NULL(ck_vtn);
  DELETE_IF_NOT_NULL(ck_vtn_cntrlr);
  DELETE_IF_NOT_NULL(ck_domain_vtn);
  return result_code;
}

upll_rc_t VnodeMoMgr::IntimatePOMAboutNewController(
    ConfigKeyVal *ikey,
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
  if (dt_type == UPLL_DT_AUDIT) {
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
    pom_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>(
            GetMoManager(
                pom_keys[count])));
    if (!pom_mgr) {
      UPLL_LOG_DEBUG("error in fetching MoMgr reference");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = pom_mgr->UpdateControllerTableForVtn(vtn_name,
                                                       ctrlr_dom,
                                                       op,
                                                       dt_type,
                                                       dmi,
                                                       flag);
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
  DbSubOp dbop1 = { kOpReadSingle,
    kOpMatchNone,
    kOpInOutCtrlr|kOpInOutDomain };
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
    // DELETE_IF_NOT_NULL(parent_key);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);
  SET_USER_DATA_CTRLR_DOMAIN(parent_key, ctrlr_dom);
  //  delete ck_vbr;
  /* GetReference count from vtn controller table */
  MoMgrImpl *mgr =
      reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(GetMoManager(
                  UNC_KT_VTN)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;
  result_code = mgr->ReadConfigDB(parent_key, req->datatype, UNC_OP_READ, dbop,
                                  dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) return result_code;
  val_vtn_ctrlr *vtn_st_val = reinterpret_cast<val_vtn_ctrlr *>
      (GetVal(parent_key));
  if (!vtn_st_val) {
    UPLL_LOG_DEBUG("Val is empty");
    return UPLL_RC_ERR_GENERIC;
  }

  if (1 == vtn_st_val->ref_count) {
    if (!OVERLAY_KT(ikey->get_key_type())) {
      result_code = IntimatePOMAboutNewController(parent_key,
                                                  &ctrlr_dom,
                                                  dmi,
                                                  UNC_OP_DELETE,
                                                  req->datatype);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("IntimatePOMAboutNewController %d", result_code);
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
      return result_code;
    }
    parent_key->SetCfgVal(NULL);
    result_code = mgr->UpdateConfigDB(parent_key, req->datatype, UNC_OP_DELETE,
                                      dmi, &dbop, RENAMETBL);
    if (UPLL_RC_SUCCESS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_TRACE("UpdateConfigDB Failed %d", result_code);
      return result_code;
    }
    result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?
        UPLL_RC_SUCCESS:result_code;
  } else {
    //  Reduce the ref count if vnode is not last node.
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
      //  Getting the controller vtn, vnode name
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

      if (!uui::IpcUtil::SendReqToDriver(
              (const char *)(ctrlr_dom.ctrlr),
              reinterpret_cast<char *>
              (ctrlr_dom.domain),
              PFCDRIVER_SERVICE_NAME,
              PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
        UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                      ikey->get_key_type(), reinterpret_cast<char *>
                      (ctrlr_dom.ctrlr));
        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
        return UPLL_RC_ERR_GENERIC;
      }

      //  Populate ConfigKeyVal and IpcReqRespHeader with the response
      //  from driver
      ikey->ResetWith(ipc_resp.ckv_data);
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      result_code = (header->result_code = ipc_resp.header.result_code);
    } else {
      UPLL_LOG_DEBUG("Control Operation not allowed for %d data type",
                     header->datatype);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    }
  } else {
    UPLL_LOG_DEBUG("Control Operation not allowed for %d key type",
                   ikey->get_key_type());
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::GetVnodeType(const void *key, bool vnode,
                                   unc_key_type_t &keytype,
                                   ConfigKeyVal *&ck_val, DalDmlIntf *dmi,
                                   upll_keytype_datatype_t dt_type) {
  unc_key_type_t *ktype, if_ktype[] = { UNC_KT_VBR_IF, UNC_KT_VRT_IF,
    UNC_KT_VUNK_IF, UNC_KT_VTEP_IF,
    UNC_KT_VTUNNEL_IF };
  unc_key_type_t vnode_ktype[] = { UNC_KT_VBRIDGE, UNC_KT_VROUTER, UNC_KT_VTEP,
    UNC_KT_VTUNNEL, UNC_KT_VUNKNOWN };
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

upll_rc_t VnodeMoMgr::UpdateParentOperStatus(ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_ERROR("Returning error \n");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ktype = ikey->get_key_type();
  if ((ktype != UNC_KT_VBRIDGE) && (ktype != UNC_KT_VROUTER))
    return UPLL_RC_SUCCESS;
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain};
  result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return result_code;
  }
  val_db_vbr_st *valst = reinterpret_cast<val_db_vbr_st *>(GetStateVal(ikey));
  if (!valst) {
    UPLL_LOG_DEBUG("Returning error\n");
    return UPLL_RC_ERR_GENERIC;
  }
  if (valst->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] == UNC_VF_VALID) {
    if (valst->vbr_val_st.oper_status !=  UPLL_OPER_STATUS_DOWN)
      return UPLL_RC_SUCCESS;
    ConfigKeyVal *ck_vtn = NULL;
    //  decrement the down count in vtn controller table
    result_code = GetParentConfigKey(ck_vtn, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return result_code;
    }
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                                   (GetMoManager(UNC_KT_VTN)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Returning error \n");
      delete ck_vtn;
      return UPLL_RC_ERR_GENERIC;
    }
    DbSubOp dbop = {kOpReadSingle,
      kOpMatchCtrlr | kOpMatchDomain,
      kOpInOutNone};
    result_code = mgr->ReadConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_READ, dbop,
                                    dmi, CTRLRTBL);
    if (result_code == UPLL_RC_SUCCESS) {
      val_vtn_ctrlr *vtn_val_st = reinterpret_cast<val_vtn_ctrlr *>
          (GetVal(ck_vtn));
      vtn_val_st->down_count = (vtn_val_st->down_count > 0)?
          (vtn_val_st->down_count - 1):0;
      if (vtn_val_st->down_count == 0)
        vtn_val_st->oper_status  = UPLL_OPER_STATUS_UP;
      result_code = mgr->UpdateConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_UPDATE,
                                        dmi, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d\n", result_code);
        delete ck_vtn;
        return result_code;
      }
    } else {
      UPLL_LOG_DEBUG("Last vnode in ctrlr table %s\n",
                     (reinterpret_cast<key_vbr *>
                      (ikey->get_key()))->vbridge_name);
    }
    //  initialize parent vtn of main tbl to uninit so oper status
    //  gets computed during dt_state
    ConfigKeyVal *tmp_ckv = ck_vtn;
    ck_vtn = NULL;
    // get ck_vtn with oper status uninit.
    result_code = mgr->GetCkvUninit(ck_vtn, tmp_ckv, dmi);
    DELETE_IF_NOT_NULL(tmp_ckv);
    if (result_code != UPLL_RC_SUCCESS) {
      if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      } else {
        result_code = UPLL_RC_SUCCESS;
      }
      DELETE_IF_NOT_NULL(ck_vtn);
      return result_code;
    }
    result_code = mgr->UpdateConfigDB(ck_vtn, UPLL_DT_STATE, UNC_OP_UPDATE,
                                      dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
        result_code = UPLL_RC_SUCCESS;
    }
    DELETE_IF_NOT_NULL(ck_vtn);
  }
  return result_code;
}

upll_rc_t VnodeMoMgr::UpdateOperStatus(ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi,
                                       state_notification notification,
                                       bool skip,
                                       bool save_to_db) {
  UPLL_FUNC_TRACE;
  bool oper_status_change = false;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_DEBUG("Returning error");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ktype = ikey->get_key_type();
  bool read_db = false;
  string s;
  if (!skip) {
    if (!save_to_db) {
      void *key = ikey->get_key();
      const char *vtn_name = reinterpret_cast<const char *>(
          reinterpret_cast<key_vtn *>(key)->vtn_name);
      s = string(vtn_name) +
          reinterpret_cast<char *>(
              reinterpret_cast<char *>(key) + sizeof(struct key_vtn));
      map<string, ConfigKeyVal *>::const_iterator got
          = vnode_oper_map.find(s);
      if (got == vnode_oper_map.end())
        read_db = true;
    }
    if (read_db || save_to_db) {
      DbSubOp dbop = { kOpReadMultiple, kOpMatchNone,
        kOpInOutCtrlr | kOpInOutDomain };
      result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                                 MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Error in reading: %d", result_code);
        return result_code;
      }
    }
  } else if (!save_to_db) {
    UPLL_LOG_ERROR("Returning error \n");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *tkey, *tkey_next = ikey;
  while (tkey_next != NULL) {
    tkey = tkey_next;
    tkey_next = tkey->get_next_cfg_key_val();
    tkey->set_next_cfg_key_val(NULL);

    switch (ktype) {
      case UNC_KT_VBRIDGE:
        oper_status_change = SetOperStatus<val_vbr_st_t, val_db_vbr_st_t *>(
            tkey,  dmi, notification, true, save_to_db);
        break;
      case UNC_KT_VROUTER:
        oper_status_change = SetOperStatus<val_vrt_st_t, val_db_vrt_st_t *>(
            tkey,  dmi, notification, true, save_to_db);
        break;
      case UNC_KT_VTEP:
        oper_status_change = SetOperStatus
            <val_vtep_st_t, val_db_vtep_st_t *>(
                tkey,  dmi, notification, true, save_to_db);
        break;
      case UNC_KT_VTUNNEL:
        oper_status_change = SetOperStatus
            <val_vtunnel_st_t, val_db_vtunnel_st_t *>(
                tkey,  dmi, notification, true, save_to_db);
        break;
      default:
        UPLL_LOG_DEBUG("Operstatus attribute not supported for this kt %d",
                       ktype);
        break;
    }
    //  vishnu
    if (oper_status_change && (notification != kCtrlrDisconnect)) {
      VtnMoMgr *mgr = reinterpret_cast<VtnMoMgr *>
          (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        return UPLL_RC_ERR_GENERIC;
      }
      ConfigKeyVal *ck_vtn = NULL;
      upll_rc_t result_code = GetParentConfigKey(ck_vtn, tkey);
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
    if (skip)
      break;
  }
#if 0
  if (!save_to_db) {
    map<void *, ConfigKeyVal *>::const_iterator it;
    VtnMoMgr *mgr = reinterpret_cast<VtnMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr");
      return UPLL_RC_ERR_GENERIC;
    }
    for (it = mgr->vtn_oper_map.begin(); it != mgr->vtn_oper_map.end(); ++it) {
      DbSubOp dbop = { kOpNotRead, kOpMatchCtrlr | kOpMatchDomain,
        kOpInOutNone };
      ConfigKeyVal *ck_vtn_main = NULL, *tkey = it->second;
      result_code = mgr->UpdateConfigDB(tkey, UPLL_DT_RUNNING, UNC_OP_UPDATE,
                                        dmi, &dbop, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in update oper status %d", result_code);
        return result_code;
      }
      result_code = mgr->GetChildConfigKey(ck_vtn_main, tkey);
      if (!ck_vtn_main) {
        UPLL_LOG_DEBUG("Invalid param");
        return result_code;
      }
      DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
      result_code = mgr->ReadConfigDB(ck_vtn_main, UPLL_DT_STATE, UNC_OP_READ,
                                      dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in reading: %d", result_code);
        DELETE_IF_NOT_NULL(ck_vtn_main);
        return result_code;
      }
      mgr->SetOperStatus(ck_vtn_main, notification, dmi);
      map<void *, ConfigKeyVal *>::const_iterator local_it = it++;
      mgr->vtn_oper_map.erase(local_it->first);
      if (tckv) delete tckv;
    }
    mgr->vtn_oper_map.clear();
  }
#endif
  return result_code;
}


template<typename T1, typename T2>
bool VnodeMoMgr::SetOperStatus(ConfigKeyVal *ikey,
                               DalDmlIntf *dmi, int notification,
                               bool skip, bool save_to_db) {
  /* update corresponding vnode operstatus */
  UPLL_FUNC_TRACE;
  bool oper_change = false;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!skip) {
    bool read_db = false;;
    if (!save_to_db) {
      void *key = ikey->get_key();
      const char *vtn_name = reinterpret_cast<const char *>(
          reinterpret_cast<key_vtn *>(key)->vtn_name);
      string s = string(vtn_name) +
          reinterpret_cast<char *>(
              reinterpret_cast<char *>(key) + sizeof(struct key_vtn));
      map<string, ConfigKeyVal *>::const_iterator got
          = vnode_oper_map.find(s);
      if (got == vnode_oper_map.end())
        read_db = true;
    }
    if (read_db || save_to_db) {
      DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
      result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                                 MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Error in reading: %d", result_code);
        return oper_change;
      }
    }
  }
  ConfigVal *tmp =
      (ikey->get_cfg_val()) ? ikey->get_cfg_val()->get_next_cfg_val() : NULL;
  unc_key_type_t ktype = ikey->get_key_type();
  T2 vn_valst = (T2)((tmp != NULL) ? tmp->get_val() : NULL);
  if (vn_valst == NULL) {
    UPLL_LOG_DEBUG("Invalid param");
    return oper_change;
  }
  T1 *vn_val = reinterpret_cast<T1 *>(vn_valst);
  /* Update oper status based on notification */
  vn_val->valid[0] = UNC_VF_VALID;
  UPLL_LOG_DEBUG("notification %d down_count %d fault_count %d", notification,
                 vn_valst->down_count, vn_valst->fault_count);
  switch (notification) {
    case kCtrlrReconnect:
      return UPLL_RC_SUCCESS;
    case kCtrlrDisconnect:
    case kPortUnknown:
      oper_change = (vn_val->oper_status != UPLL_OPER_STATUS_UNKNOWN)?
          true:false;
      vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      vn_valst->down_count = 0;
      vn_valst->fault_count = 0;
      break;
    case kCtrlrReconnectIfDown:
      vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      break;
    case kCtrlrReconnectIfUp:
      if (vn_val->oper_status == UPLL_OPER_STATUS_DOWN) {
        return UPLL_RC_SUCCESS;
      }
      vn_val->oper_status = UPLL_OPER_STATUS_UP;
      break;
      //       return false;
    case kAdminStatusDisabled:
      oper_change = (vn_val->oper_status != UPLL_OPER_STATUS_DOWN)?true:false;
      vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      break;
    case kAdminStatusEnabled:
      if ((vn_valst->down_count == 0) && (vn_valst->fault_count == 0)) {
        //  oper_change = (vn_val->oper_status != UPLL_OPER_STATUS_UNINIT)?
        //                 true : false;
        oper_change = ((vn_val->oper_status == UPLL_OPER_STATUS_DOWN)||
                       (vn_val->oper_status == UPLL_OPER_STATUS_UNKNOWN))?
            true:false;
        if (OVERLAY_KT(ktype))
          vn_val->oper_status = UPLL_OPER_STATUS_UP;
        else
          vn_val->oper_status = UPLL_OPER_STATUS_UNINIT;
      } else {
        oper_change = (vn_val->oper_status != UPLL_OPER_STATUS_DOWN)?
            true:false;
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      }
      break;
    case kPathFault:
      vn_valst->fault_count= (vn_valst->fault_count + 1);
      if (vn_valst->fault_count == 1) {
        oper_change = (vn_val->oper_status !=
                       UPLL_OPER_STATUS_DOWN)?true:false;
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
        //  generate alarm
        if (!EnqueOperStatusNotification(ikey, ALARM_OPER_DOWN)) {
          UPLL_LOG_DEBUG("Alarm Notification failed");
          return UPLL_RC_ERR_GENERIC;
        }
      }
      break;
    case kPathFaultReset:
      vn_valst->fault_count = (vn_valst->fault_count > 0) ?
          (vn_valst->fault_count - 1) : 0;
      if (vn_valst->fault_count == 0 && vn_valst->down_count == 0) {
        oper_change = (vn_val->oper_status ==
                       UPLL_OPER_STATUS_DOWN)?true:false;
        if (OVERLAY_KT(ktype))
          vn_val->oper_status = UPLL_OPER_STATUS_UP;
        else
          vn_val->oper_status = UPLL_OPER_STATUS_UNINIT;
        //  reset alarm
        if (vn_valst->fault_count == 0) {
          if (!EnqueOperStatusNotification(ikey, ALARM_OPER_UP)) {
            UPLL_LOG_DEBUG("Alarm Notification failed");
            return UPLL_RC_ERR_GENERIC;
          }
        }
      }
      break;
    case kPortFault:
    case kBoundaryFault:
      vn_valst->down_count = (vn_valst->down_count + 1);
      if (vn_valst->down_count == 1) {
        oper_change = (vn_val->oper_status !=
                       UPLL_OPER_STATUS_DOWN)?true:false;
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      }
      break;
    case kPortFaultReset:
    case kBoundaryFaultReset:
      vn_valst->down_count = (vn_valst->down_count > 0) ?
          (vn_valst->down_count - 1) : 0;
      /* fall through intentional */
      if (vn_valst->down_count == 0 && vn_valst->fault_count == 0) {
        oper_change = ((vn_val->oper_status == UPLL_OPER_STATUS_DOWN) ||
                       (vn_val->oper_status == UPLL_OPER_STATUS_UNKNOWN))?
            true:false;
        if (OVERLAY_KT(ktype))
          vn_val->oper_status = UPLL_OPER_STATUS_UP;
        else
          vn_val->oper_status = UPLL_OPER_STATUS_UNINIT;
        //  generate alarm
      }
      break;
    case kPortFaultResetWithAdminDisabled:
    case kBoundaryFaultResetWithAdminDisabled:
      vn_valst->down_count = (vn_valst->down_count > 0) ?
          (vn_valst->down_count - 1) : 0;
      //  oper status stays down irrespective of down_count
      //  as admin is disabled for one of the child interfaces.
      vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      break;
  }
  if (save_to_db) {
    DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
    result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                                 &dbop, MAINTBL);
    UPLL_LOG_TRACE("Vnode SetOperstatus for VTN after Update is \n %s",
                   ikey->ToStrAll().c_str());
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in update oper status %d", result_code);
    }
  } else {
    void *key = ikey->get_key();
    const char *vtn_name = reinterpret_cast<const char *>(
        reinterpret_cast<key_vtn *>(key)->vtn_name);
    string s = string(vtn_name) +
        reinterpret_cast<char *>(
            reinterpret_cast<char *>(key) + sizeof(struct key_vtn));
    vnode_oper_map[s] = ikey;
    if (vnode_oper_map[s] == ikey)
      UPLL_LOG_DEBUG("Storing %s %p", s.c_str(), ikey);
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
  unc_key_type_t child_keytype;

  /* Create Vnode If key */
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
    default:
      UPLL_LOG_DEBUG("Unsupported operation on keytype %d", ktype);
      return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetUninitOperState(ck_vn, dmi);
  if (UPLL_RC_SUCCESS != result_code || ck_vn == NULL)  {
    return result_code;
  }
  ConfigKeyVal *tkey = ck_vn;
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  while (ck_vn) {
    state_notification notification = kAdminStatusEnabled;
    tkey = ck_vn;
    ck_vn = tkey->get_next_cfg_key_val();
    tkey->set_next_cfg_key_val(NULL);
    /* read the state value */
    val_db_vbr_st * vnode_runst = reinterpret_cast<val_db_vbr_st *>
        (GetStateVal(tkey));
    if (!vnode_runst) {
      UPLL_LOG_DEBUG("Invalid param");
      return UPLL_RC_ERR_GENERIC;
    }
    if ((vnode_runst->down_count == 0) && (vnode_runst->fault_count == 0)) {
      //  get count of vnode ifs down
      ConfigKeyVal *ck_vnif = NULL;

      VnodeChildMoMgr *mgr = reinterpret_cast<VnodeChildMoMgr *>
          (const_cast<MoManager *>(GetMoManager(child_keytype)));
      result_code = mgr->GetCkvUninit(ck_vnif, tkey, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d\n", result_code);
        return result_code;
      }
      val_db_vbr_st *vnif_stval = reinterpret_cast<val_db_vbr_st *>
          (GetStateVal(ck_vnif));
      vnif_stval->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
      vnif_stval->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
#if 0
      uint32_t cur_instance_count = 0;
      result_code = mgr->GetInstanceCount(ck_vnif, NULL, UPLL_DT_STATE,
                                          &cur_instance_count, dmi, MAINTBL);
      if (result_code == UPLL_RC_SUCCESS) {
        if (cur_instance_count == 0) {
          vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_UP;
          notification = kAdminStatusEnabled;
        } else {
          vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
          notification = kAdminStatusDisabled;
        }
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
        notification = kAdminStatusDisabled;
      } else {
      }
#else
        DbSubOp dbop = { kOpReadSingle,
          kOpMatchNone,
          kOpInOutFlag | kOpInOutCtrlr
          | kOpInOutDomain };
      result_code = mgr->ReadConfigDB(ck_vnif, UPLL_DT_STATE, UNC_OP_READ,
                                      dbop, dmi, MAINTBL);
      if (result_code == UPLL_RC_SUCCESS) {
        // operationally down interface present in the interface table
        vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
        result_code = UPLL_RC_SUCCESS;
        notification = kAdminStatusDisabled;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        ck_vnif->SetCfgVal(NULL);
        result_code = mgr->UpdateConfigDB(ck_vnif, UPLL_DT_STATE, UNC_OP_READ,
                                          dmi, &dbop, MAINTBL);
        if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
          // vnode has no interface that is down
          vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_UP;
          result_code = UPLL_RC_SUCCESS;
          notification = kAdminStatusEnabled;
        } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          if (tkey->get_key_type() == UNC_KT_VBRIDGE) {
            VlanMapMoMgr *vlan_mgr = reinterpret_cast<VlanMapMoMgr *>
                (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_VLANMAP)));
            ConfigKeyVal *ckv_vlanmap = NULL;
            result_code = vlan_mgr->GetChildConfigKey(ckv_vlanmap, tkey);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("GetChildConfigKey failed for vlanmap\n");
              return result_code;
            }
            result_code = vlan_mgr->UpdateConfigDB(ckv_vlanmap,
                                                   UPLL_DT_STATE,
                                                   UNC_OP_READ,
                                                   dmi,
                                                   &dbop,
                                                   MAINTBL);
            if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
              // vbridge with vlanmap configured
              vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_UP;
              result_code = UPLL_RC_SUCCESS;
              notification = kAdminStatusEnabled;
            }
            DELETE_IF_NOT_NULL(ckv_vlanmap);
          }
          if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            // standalone vnode
            if (OVERLAY_KT(ktype)) {
              notification = kAdminStatusEnabled;
              vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_UP;
            } else {
              notification = kAdminStatusDisabled;
              vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
            }
            result_code = UPLL_RC_SUCCESS;
          }
        }
      }
#endif
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d\n", result_code);
        delete ck_vnif;
        delete tkey;
        delete ck_vn;
        return result_code;
      }
      DELETE_IF_NOT_NULL(ck_vnif);
      vnode_runst->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
    } else {
      if (vnode_runst->down_count > 0)
        notification =  kPortFault;
      else if (vnode_runst->fault_count > 0)
        notification =  kPathFault;
      vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
      vnode_runst->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
    }
    result_code = UpdateConfigDB(tkey, UPLL_DT_STATE, UNC_OP_UPDATE,
                                 dmi, &dbop1, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("UpdateConfigDB Executed %d", result_code);
      break;
    }
    //  invoked from port status handler
    //  don't propagate as the event is already propagated.
    if ((config_id == 0) && (session_id == 0)) {
      delete tkey;
      continue;
    }
    if (notification != kAdminStatusEnabled) {
      VtnMoMgr *mgr = reinterpret_cast<VtnMoMgr *>
          (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        return UPLL_RC_ERR_GENERIC;
      }
      ConfigKeyVal *ck_vtn = NULL;
      upll_rc_t result_code = GetParentConfigKey(ck_vtn, tkey);
      if (result_code != UPLL_RC_SUCCESS) {
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
    delete tkey;
  }
  if (ck_vn)
    delete ck_vn;
  ck_vn = NULL;
  return result_code;
}

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
    default:
      UPLL_LOG_DEBUG("Invalid KeyType");
  }
  return cfg_instance->SendOperStatusAlarm(reinterpret_cast<char*>(vtn_name),
                                           reinterpret_cast<char*>(vnode_name),
                                           reinterpret_cast<char*>(NULL),
                                           oper_status_change);
}

#if 0
upll_rc_t VnodeMoMgr::InitOperStatus(ConfigKeyVal *tkey) {
  state_notification notification;
  UPLL_FUNC_TRACE;

  if (!tkey) {
    UPLL_LOG_DEBUG("Returning error \n");
    return UPLL_RC_ERR_GENERIC;
  }

  /* read the state value */
  val_db_vbr_st * vnode_runst = reinterpret_cast<val_db_vbr_st *>
      (GetStateVal(tkey));
  if (!vnode_runst) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  if ((vnode_runst->down_count == 0) && (vnode_runst->fault_count == 0)) {
    //  get count of vnode ifs down
    uint32_t cur_instance_count = 0;
    ConfigKeyVal *ck_vnif = NULL;

    VnodeChildMoMgr *mgr = reinterpret_cast<VnodeChildMoMgr *>
        (const_cast<MoManager *>(GetMoManager(child_keytype)));
    result_code = mgr->GetCkvUninit(ck_vnif, tkey, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return result_code;
    }
    val_db_vbr_st *vnif_stval = reinterpret_cast<val_db_vbr_st *>
        (GetStateVal(ck_vnif));
    vnif_stval->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
    vnif_stval->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
    result_code = mgr->GetInstanceCount(ck_vnif, NULL, UPLL_DT_STATE,
                                        &cur_instance_count, dmi, MAINTBL);
    if (result_code == UPLL_RC_SUCCESS) {
      if (cur_instance_count == 0) {
        vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_UP;
        notification = kAdminStatusEnabled;
      } else {
        vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
        notification = kAdminStatusDisabled;
      }
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
      notification = kAdminStatusDisabled;
    } else {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      delete ck_vnif;
      delete tkey;
      delete ck_vn;
      return result_code;
    }
    DELETE_IF_NOT_NULL(ck_vnif);
    vnode_runst->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
  } else {
    if (vnode_runst->down_count > 0)
      notification =  kPortFault;
    else if (vnode_runst->fault_count > 0)
      notification =  kPathFault;
    vnode_runst->vbr_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
    vnode_runst->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
  }
  DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(tkey, UPLL_DT_STATE, UNC_OP_UPDATE,
                               dmi, &dbop1, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateConfigDB Executed %d", result_code);
    return result_code;
  }
  VtnMoMgr *mgr = reinterpret_cast<VtnMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
  if (!mgr) {
    UPLL_LOG_DEBUG("Invalid mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *ck_vtn = NULL;
  upll_rc_t result_code = GetParentConfigKey(ck_vtn, tkey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }
  result_code = mgr->UpdateOperStatus(ck_vtn, dmi, notification, false);
  delete ck_vtn;
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("VTN UpdateOperStatus failed");
  }
  return result_code;
}
#endif

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc

