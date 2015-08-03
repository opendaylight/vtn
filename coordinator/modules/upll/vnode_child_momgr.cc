/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "vnode_child_momgr.hh"
#include "vnode_momgr.hh"
#include "uncxx/upll_log.hh"
#include "vbr_if_momgr.hh"
#include "vtn_momgr.hh"
#include "vlink_momgr.hh"
#include "vterm_if_momgr.hh"
#include "config_mgr.hh"
#include "config_yield.hh"
#include "vlanmap_momgr.hh"

#define NO_VLINK_FLAG 0x03

namespace unc {
namespace upll {
namespace kt_momgr {
using unc::upll::config_momgr::UpllConfigMgr;
upll_rc_t VnodeChildMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal *pckv = NULL;
  string vtn_id = "";

  if ((ikey->get_key_type() == UNC_KT_VTEP_GRP) ||
      (ikey->get_key_type() == UNC_KT_VTEP_GRP_MEMBER)) {
    result_code = GetControllerDomainId(ikey, UPLL_DT_AUDIT,
                                       &ctrlr_dom, dmi);
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  } else if ((ikey->get_key_type() == UNC_KT_VBR_PORTMAP)) {
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>(
        GetMoManager(UNC_KT_VBR_PORTMAP)));
    result_code = mgr->GetControllerDomainId(ikey, &ctrlr_dom);
  } else {
    result_code = GetParentConfigKey(pckv, ikey);
    if (result_code != UPLL_RC_SUCCESS && pckv == NULL) {
      UPLL_LOG_DEBUG("GetParentConfigKey failed err_code is %d",
                      result_code);
      return result_code;
    }
    result_code = GetControllerDomainId(pckv, UPLL_DT_AUDIT,
                                       &ctrlr_dom, dmi);
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(pckv, ctrlr_dom);
    SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    DELETE_IF_NOT_NULL(pckv);
  }

  if ((result_code != UPLL_RC_SUCCESS) || (ctrlr_dom.ctrlr == NULL)
      || (ctrlr_dom.domain == NULL)) {
    UPLL_LOG_INFO("GetControllerDomainId failed err_code %d", result_code);
    DELETE_IF_NOT_NULL(pckv);
    return UPLL_RC_ERR_GENERIC;
  }
  if (ikey->get_key_type() == UNC_KT_VBR_IF) {
    val_vbr_if *vbr_if_val = reinterpret_cast<val_vbr_if_t *>
      (GetVal(ikey));
    if (vbr_if_val->valid[UPLL_IDX_ADMIN_STATUS_VBRI] == UNC_VF_INVALID)
      vbr_if_val->admin_status = UPLL_ADMIN_ENABLE;
  } else if (ikey->get_key_type() == UNC_KT_VTERM_IF) {
    val_vterm_if_t *vterm_if_val = reinterpret_cast<val_vterm_if_t *>
      (GetVal(ikey));
    if (vterm_if_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] ==
        UNC_VF_INVALID)
      vterm_if_val->admin_status = UPLL_ADMIN_ENABLE;
  }
  ConfigKeyVal *temp_key = NULL;
  result_code = GetChildConfigKey(temp_key, ikey);
  if (result_code != UPLL_RC_SUCCESS || temp_key == NULL) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    DELETE_IF_NOT_NULL(pckv);
    return result_code;
  }
  uint8_t temp_flag = 0, rename_flag = 0;
  DbSubOp dbop1 = {kOpReadSingle, kOpMatchNone,
    kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain};
  result_code = ReadConfigDB(temp_key, UPLL_DT_RUNNING,
                             UNC_OP_READ, dbop1, dmi, MAINTBL);
  if (result_code == UPLL_RC_SUCCESS) {
    GET_USER_DATA_FLAGS(temp_key, temp_flag);
    temp_flag = (temp_flag & 0xF0);
    GET_USER_DATA_FLAGS(ikey, rename_flag);
    rename_flag |= temp_flag;
    SET_USER_DATA_FLAGS(ikey, rename_flag);
  } else {
    UPLL_LOG_DEBUG("Record not found in RUNNING DB");
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutDomain
                       | kOpInOutCtrlr };
  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE,
                               dmi, &dbop, TC_CONFIG_GLOBAL, vtn_id, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("UpdateConfigDB failed for creating vnode child err_code %d",
                   result_code);
  }
  DELETE_IF_NOT_NULL(pckv);
  DELETE_IF_NOT_NULL(temp_key);
  return result_code;
}

upll_rc_t VnodeChildMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (UPLL_DT_IMPORT != req->datatype) {
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return result_code;
    }
  }

  ConfigKeyVal *parent_ck_vnode = NULL;

  result_code = GetParentConfigKey(parent_ck_vnode, ikey);
  if (result_code != UPLL_RC_SUCCESS || parent_ck_vnode == NULL) {
    return result_code;
  }
  controller_domain cntrl_dom;
  if ((ikey->get_key_type() == UNC_KT_VTEP_GRP) ||
      (ikey->get_key_type() == UNC_KT_VTEP_GRP_MEMBER)) {
    result_code = GetControllerDomainId(ikey, req->datatype,
                                      &cntrl_dom, dmi);
    cntrl_dom.ctrlr = NULL;
    if (ikey->get_key_type() != UNC_KT_VTEP_GRP)
      cntrl_dom.domain = NULL;

    GET_USER_DATA_CTRLR_DOMAIN(ikey, cntrl_dom);
  } else {
    result_code = GetControllerDomainId(parent_ck_vnode, req->datatype,
                                        &cntrl_dom, dmi);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(parent_ck_vnode);
      return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
    } else if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error from GetControllerDomainId");
      DELETE_IF_NOT_NULL(parent_ck_vnode);
      return result_code;
    }
    cntrl_dom.ctrlr = NULL;
    cntrl_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(parent_ck_vnode, cntrl_dom);
    SET_USER_DATA_CTRLR_DOMAIN(ikey, cntrl_dom);
  }
  if ((result_code != UPLL_RC_SUCCESS) || (cntrl_dom.ctrlr == NULL)
      || (cntrl_dom.domain == NULL)) {
    UPLL_LOG_INFO("Illegal params");
    DELETE_IF_NOT_NULL(parent_ck_vnode);
    return result_code;
  }
  /* check whether this keytype is child of vbridge(KT_VBR_VLANMAP, 
     KT_VBR_NWMONITOR, KT_VBR_NWMONITOR_HOST) and 
     not requested under unified vbridge
  
     KT_VBR_IF is skipped , as it can be created under 
     both unified and normal vbr*/
  unc_key_type_t keyType = ikey->get_key_type();
  if ((IS_VBRIDGE_CHILD_KEY_TYPE(keyType)) && (IsUnifiedVbr(cntrl_dom.ctrlr))) {
      UPLL_LOG_DEBUG("This KT is not allowed to be created under Unified"
                     " Vbridge");
      DELETE_IF_NOT_NULL(parent_ck_vnode);
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  ConfigKeyVal *dup_ikey = NULL;
  if (UPLL_DT_IMPORT != req->datatype) {
      void *ifval = GetVal(ikey);
      /* VlanmapOnBoundary: Added vlan-map check */
      if (((ikey->get_key_type() == UNC_KT_VBR_IF) ||
          (ikey->get_key_type() == UNC_KT_VBR_VLANMAP))&& ifval) {
         result_code = GetChildConfigKey(dup_ikey, ikey);
         if (!dup_ikey || result_code != UPLL_RC_SUCCESS) {
           DELETE_IF_NOT_NULL(parent_ck_vnode);
           UPLL_LOG_DEBUG("Returning error %d", result_code);
           return result_code;
         }

         /* VlanmapOnBoundary: Added vlan-map check */
         if (ikey->get_key_type() == UNC_KT_VBR_IF) {
           val_drv_vbr_if *valif = reinterpret_cast<val_drv_vbr_if *>
             (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
           memcpy(&(valif->vbr_if_val), ifval, sizeof(val_vbr_if));
           dup_ikey->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrIf, valif);
         }

         if (ikey->get_key_type() == UNC_KT_VBR_VLANMAP) {
           /* Case1: "vlanmap vlan-id" create request is received,
            * do existence check with value struct containing vlan-id
            * Case2: "vlanmap log_port_id vlan-id"
            * If log_port_id_valid is present, do exist check without
            * vlan-id(no val struct) */
           if ((!(reinterpret_cast<key_vlan_map_t *>
                 (ikey->get_key())->logical_port_id_valid == PFC_TRUE))) {
             pfcdrv_val_vlan_map_t *val_vlanmap =
               reinterpret_cast<pfcdrv_val_vlan_map_t *>(
                   ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vlan_map_t)));
             if (ikey->get_cfg_val()->get_st_num() ==
                 IpctSt::kIpcStValVlanMap) {
               /* Copy the value strcuture to do existence check */
               memcpy(&(val_vlanmap->vm), ifval, sizeof(val_vlan_map_t));
             } else if (ikey->get_cfg_val()->get_st_num() ==
                        IpctSt::kIpcStPfcdrvValVlanMap) {
               /* This case specific to restore of vlanmap */
               memcpy(val_vlanmap, ifval, sizeof(pfcdrv_val_vlan_map_t));
             }
             dup_ikey->AppendCfgVal(IpctSt::kIpcStPfcdrvValVlanMap,
                                    val_vlanmap);
           }
         }
      } else {
      dup_ikey = ikey;
      }
    } else {
      dup_ikey = ikey;
    }
    UPLL_LOG_DEBUG("%s \n", dup_ikey->ToStrAll().c_str());

  /* VlanmapOnBoundary: Added vlan-map check */
  uint8_t flags = 0;
  if (dup_ikey->get_key_type() == UNC_KT_VBR_VLANMAP) {
    /* When the request is received from boundary,
     * boundary bit is always set in user_data */
    GET_USER_DATA_FLAGS(dup_ikey, flags);

    /* Set the user-configured bit in vlan-map flag
     * case1 : All cases of import
     * case2 : All user configurations (with & without log_port_id) */
    if (!(flags & BOUNDARY_VLANMAP_FLAG)) {
     // uint8_t vlanmap_flag = 0;
      flags |= USER_VLANMAP_FLAG;
      SET_USER_DATA_FLAGS(dup_ikey, flags);
    }
  }

  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    return result_code;
  }

    /* VlanmapOnBoundary: Added vlan-map check
     * If logical-port-id is specified in the user-request */
    if ((dup_ikey->get_key_type() == UNC_KT_VBR_VLANMAP) &&
       (reinterpret_cast<key_vlan_map_t *>
        (ikey->get_key())->logical_port_id_valid == PFC_TRUE)) {
      /* Checks whether the requested vlan-map entry present in DB or not.
       * If it is not present creates a new entry.If entry exists updates
       * user configured vlan map flag.*/
      DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
      result_code = ReadConfigDB(dup_ikey, req->datatype, UNC_OP_READ, dbop,
                                 dmi, MAINTBL);
      if ((result_code != UPLL_RC_SUCCESS) &&
          (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
        UPLL_LOG_DEBUG("ReadconfigDB failed result code = %u", result_code);
        DELETE_IF_NOT_NULL(parent_ck_vnode);
        DELETE_IF_NOT_NULL(dup_ikey);
        return result_code;
      }
      pfcdrv_val_vlan_map_t *val_vlanmap =
            reinterpret_cast<pfcdrv_val_vlan_map_t *> (GetVal(dup_ikey));
      /* It is always expected to get value structure from DB */
      if (!val_vlanmap) {
        UPLL_LOG_ERROR("Val strcut from DB is NULL");
        delete dup_ikey;
        DELETE_IF_NOT_NULL(parent_ck_vnode);
        return UPLL_RC_ERR_GENERIC;
      }

      val_vlan_map_t *ival = NULL;
      if ((ikey->get_cfg_val())->get_st_num() ==
          IpctSt::kIpcStPfcdrvValVlanMap) {
        pfcdrv_val_vlan_map_t* drv_ival =
          reinterpret_cast<pfcdrv_val_vlan_map_t *>(GetVal(ikey));
        if (drv_ival->valid[PFCDRV_IDX_BDRY_REF_COUNT] == UNC_VF_VALID) {
          val_vlanmap->bdry_ref_count = drv_ival->bdry_ref_count;
          val_vlanmap->valid[PFCDRV_IDX_BDRY_REF_COUNT] = UNC_VF_VALID;
        }
        ival = &(reinterpret_cast<pfcdrv_val_vlan_map_t *>(GetVal(ikey))->vm);
      } else {
        ival = reinterpret_cast<val_vlan_map_t *>(GetVal(ikey));
      }

      /* result_code is SUCCESS case1: if vlan-map is already
       * configured via boundary, update the flag
       * case2: if user-configured vlan-map already exists */
      if (result_code == UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("%s \n", dup_ikey->ToStrAll().c_str());
        UPLL_LOG_TRACE("Vlanmap already configured")

        uint8_t vlanmap_flag = 0;
        GET_USER_DATA_FLAGS(dup_ikey, vlanmap_flag)

        /* vlanmap second create request from user when its
         * already created by user */
        if (vlanmap_flag & USER_VLANMAP_FLAG) {
          UPLL_LOG_ERROR("Vlanmap already with the same key exists");
          delete dup_ikey;
          DELETE_IF_NOT_NULL(parent_ck_vnode);
          return UPLL_RC_ERR_INSTANCE_EXISTS;
        }

        /* Vlan-id should be same as existing since its
         * referred by boundary */
        if ((ival->vlan_id != val_vlanmap->vm.vlan_id) &&
            (vlanmap_flag & BOUNDARY_VLANMAP_FLAG)) {
          UPLL_LOG_ERROR("Vlan-id cannot be modified since boundary "
            "vlanmap with different vlan-id is configured");
          delete dup_ikey;
          DELETE_IF_NOT_NULL(parent_ck_vnode);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }

        val_vlanmap->vm.vlan_id  = ival->vlan_id;
        val_vlanmap->vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;

        /* Update the flag if vlanmap is user configured*/
        vlanmap_flag |= flags;
        SET_USER_DATA_FLAGS(dup_ikey, vlanmap_flag);

        DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
        result_code = UpdateConfigDB(dup_ikey, req->datatype, UNC_OP_UPDATE,
                                     dmi, &dbop1, config_mode, vtn_name,
                                     MAINTBL);
        DELETE_IF_NOT_NULL(parent_ck_vnode);
        DELETE_IF_NOT_NULL(dup_ikey);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d", result_code);
          return result_code;
        }
        return UPLL_RC_SUCCESS;
      }
      val_vlanmap->vm.vlan_id  = ival->vlan_id;
      val_vlanmap->vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;

      UPLL_LOG_DEBUG("%s \n", dup_ikey->ToStrAll().c_str());
    } else if (ikey->get_key_type() == UNC_KT_VTERM_IF) {
      // This code better belong in ValidateAttribute
      VtermIfMoMgr *vtermif_mgr = reinterpret_cast<VtermIfMoMgr *>(
             const_cast<MoManager*>(GetMoManager(ikey->get_key_type())));
      // only one interface can be create under vterminal
      // Verifies whether an interface is already configured under
      // requested vterminal.
      result_code = vtermif_mgr->IsVtermIfAlreadyExists(dup_ikey,
                                                        dmi, req);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Failed to create vTerminal Interfcae");
        DELETE_IF_NOT_NULL(parent_ck_vnode);
        return result_code;
      }
    }

    // Skip validate capability, If it is a Unified vBridge.
    if (!IsUnifiedVbr(cntrl_dom.ctrlr)) {
      result_code = ValidateCapability(req, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("ValidateCapability failed");
        DELETE_IF_NOT_NULL(parent_ck_vnode);
        if (UPLL_DT_IMPORT != req->datatype) {
          void *ifval = GetVal(ikey);
          if (((ikey->get_key_type() == UNC_KT_VBR_IF) ||
               (ikey->get_key_type() == UNC_KT_VBR_VLANMAP)) && ifval) {
            DELETE_IF_NOT_NULL(dup_ikey);
          }
        }
        return result_code;
      }
    }
  SET_USER_DATA_CTRLR_DOMAIN(dup_ikey, cntrl_dom);
  result_code = ValidateAttribute(dup_ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ValidateAttribute semantic check returns error %d",
                   result_code);
    DELETE_IF_NOT_NULL(parent_ck_vnode);
    if (UPLL_DT_IMPORT != req->datatype) {
        void *ifval = GetVal(ikey);
      if (((ikey->get_key_type() == UNC_KT_VBR_IF) ||
         (ikey->get_key_type() == UNC_KT_VBR_VLANMAP)) && ifval) {
        DELETE_IF_NOT_NULL(dup_ikey);
      }
    }
    return result_code;
  }

  // PC change
  if ((ikey->get_key_type() == UNC_KT_VBR_IF) ||
      (ikey->get_key_type() == UNC_KT_VBR_VLANMAP) ||
      (ikey->get_key_type() == UNC_KT_VTERM_IF)) {
    if (req->datatype == UPLL_DT_CANDIDATE && config_mode == TC_CONFIG_VTN) {
      req->datatype = UPLL_DT_RUNNING;
      result_code = ValidateAttribute(dup_ikey, dmi, req);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("ValidateAttribute semantic check returns error %d",
                    result_code);
        DELETE_IF_NOT_NULL(parent_ck_vnode);
        if (UPLL_DT_IMPORT != req->datatype) {
          void *ifval = GetVal(ikey);
          if (((ikey->get_key_type() == UNC_KT_VBR_IF) ||
               (ikey->get_key_type() == UNC_KT_VBR_VLANMAP)) && ifval) {
            DELETE_IF_NOT_NULL(dup_ikey);
          }
        }
      req->datatype = UPLL_DT_CANDIDATE;
      return result_code;
      }
      req->datatype = UPLL_DT_CANDIDATE;
    }
  }
  result_code = RestoreVnode(dup_ikey, parent_ck_vnode, req, dmi);
  DELETE_IF_NOT_NULL(parent_ck_vnode);

  if (UPLL_DT_IMPORT != req->datatype) {
      void *ifval = GetVal(ikey);
    /* VlanmapOnBoundary: Added vlan-map check */
    if (((ikey->get_key_type() == UNC_KT_VBR_IF) ||
       (ikey->get_key_type() == UNC_KT_VBR_VLANMAP)) && ifval) {
      DELETE_IF_NOT_NULL(dup_ikey);
    }
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::RestoreVnode(ConfigKeyVal *ikey,
                                        ConfigKeyVal *parent_ck_vnode,
                                        IpcReqRespHeader *req,
                                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *dup_ikey = NULL;
  upll_keytype_datatype_t dt_type = req->datatype;

  result_code = DupConfigKeyVal(dup_ikey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning %d", result_code);
    return result_code;
  }

  uint8_t flag = 0;
  unc_key_type_t ktype = parent_ck_vnode->get_key_type();
  if (UPLL_DT_CANDIDATE == req->datatype) {
    // TODO(author) - Check condition
    if (ktype == UNC_KT_VBRIDGE ||
        ktype == UNC_KT_VROUTER || UNC_KT_VTERMINAL) {
      GET_USER_DATA_FLAGS(dup_ikey, flag);
      result_code = SetRenameField(dup_ikey, parent_ck_vnode);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Problem in setting rename field");
        delete dup_ikey;
        return result_code;
      }
    }

    if (dup_ikey->get_key_type() == UNC_KT_VBR_IF) {
      val_drv_vbr_if_t *if_val = reinterpret_cast<val_drv_vbr_if *>
          (GetVal(dup_ikey));
      if (if_val != NULL &&
          if_val->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
        result_code = ConverttoDriverPortMap(dup_ikey, dmi);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("ConverttoDriverPortMap failed %d", result_code);
          delete dup_ikey;
          return result_code;
        }
      }
    }

    /* VlanmapOnBoundary: Added vlan-map check */
    if ((dup_ikey->get_key_type() == UNC_KT_VBR_VLANMAP) ||
        (dup_ikey->get_key_type() == UNC_KT_VBR_PORTMAP)) {
      uint8_t dup_flag = 0;
      GET_USER_DATA_FLAGS(dup_ikey, dup_flag);
      flag |= dup_flag;
      SET_USER_DATA_FLAGS(dup_ikey, flag);
      UPLL_LOG_TRACE("FLAG RESTORE = %u", flag);
    }
  }
  TcConfigMode config_mode = TC_CONFIG_INVALID;
  std::string vtn_name = "";
  result_code = GetConfigModeInfo(req, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetConfigMode failed");
    if (dup_ikey) delete dup_ikey;
    return result_code;
  }

  result_code = UpdateConfigDB(dup_ikey, dt_type, UNC_OP_CREATE, dmi,
                               config_mode, vtn_name, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to create an entry in Candidate DB");
  }
  if (dup_ikey) delete dup_ikey;
  return result_code;
}


upll_rc_t VnodeChildMoMgr::IsReferenced(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey,
                                        DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *okey = NULL;
  if (NULL == ikey) return UPLL_RC_ERR_GENERIC;

  switch (ikey->get_key_type()) {
  case UNC_KT_VBR_IF:
  case UNC_KT_VRT_IF:
  case UNC_KT_VTEP_IF:
  case UNC_KT_VTUNNEL_IF:
  case UNC_KT_VUNK_IF:
  case UNC_KT_VTERM_IF:
    break;
  default:
    return UPLL_RC_SUCCESS;
  }
  result_code = GetChildConfigKey(okey, ikey);
  if (result_code != UPLL_RC_SUCCESS || okey == NULL) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutFlag };
  result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ, dbop,
                dmi, MAINTBL);
  while (okey) {
    uint8_t if_flag = 0;
    GET_USER_DATA_FLAGS(okey, if_flag);
    if_flag &= VIF_TYPE;
    if (if_flag) {
       UPLL_LOG_DEBUG("Part of a vlink %d", if_flag);
       result_code = UPLL_RC_ERR_CFG_SEMANTIC;
       break;
    }
    okey = okey->get_next_cfg_key_val();
  }
  delete okey;
  return result_code;
}

upll_rc_t VnodeChildMoMgr::SetRenameField(ConfigKeyVal *&ikey,
                                          ConfigKeyVal *parent_ck_vnode) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t rename = 0;
  GET_USER_DATA_FLAGS(parent_ck_vnode, rename);
  rename &= RENAME;
  SET_USER_DATA_FLAGS(ikey, rename);

  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetRenamedUncKey(ConfigKeyVal *ikey,
                                            upll_keytype_datatype_t dt_type,
                                            DalDmlIntf *dmi,
                                            uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t ktype = ikey->get_key_type();
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_id;
  ctrlr_dom.domain = NULL;
  if (!OVERLAY_KT(ktype))
       result_code = GetRenamedKey(ikey, dt_type, dmi,
                                   &ctrlr_dom,
                                   UNC_RENAME_KEY);
  else
     result_code = UPLL_RC_SUCCESS;

  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetRenamedControllerKey(
    ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
    controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t ktype = ikey->get_key_type();
  if (!OVERLAY_KT(ktype)) {
    result_code = GetRenamedKey(ikey, dt_type, dmi,
                                ctrlr_dom, CTRLR_RENAME_KEY);
    UPLL_LOG_DEBUG("Controller : %s; Domain : %s", ctrlr_dom->ctrlr,
                 ctrlr_dom->domain);
  } else {
    result_code = UPLL_RC_SUCCESS;
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetRenamedKey(ConfigKeyVal *ikey,
                                         upll_keytype_datatype_t dt_type,
                                         DalDmlIntf *dmi,
                                         controller_domain *ctrlr_dom,
                                         rename_key flag) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_parent = NULL;
  uint8_t parent_renamed = 0, ikey_flag = 0;
  if (!ikey) return UPLL_RC_ERR_GENERIC;
  upll_rc_t result_code = GetParentConfigKey(ck_parent, ikey);
  if (!ck_parent || (result_code != UPLL_RC_SUCCESS)) return result_code;
  unc_key_type_t ktype = ck_parent->get_key_type();
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                                  (GetMoManager(ktype)));
  if (flag == UNC_RENAME_KEY) {
    result_code = mgr->GetRenamedUncKey(ck_parent, dt_type, dmi,
                                         ctrlr_dom->ctrlr);
    if (result_code != UPLL_RC_SUCCESS &&
         result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("GetRenamedUncKey failed. Result : %d", result_code);
      DELETE_IF_NOT_NULL(ck_parent);
      return result_code;
    }
    GET_USER_DATA_FLAGS(ck_parent, parent_renamed);
  } else {
       result_code = mgr->GetRenamedControllerKey(ck_parent, dt_type, dmi,
                                                  ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed. Result: %d", result_code);
      DELETE_IF_NOT_NULL(ck_parent);
      return result_code;
    }
  }
  if (result_code == UPLL_RC_SUCCESS || (parent_renamed & RENAME_BITS)) {
    result_code  = GetChildConfigKey(ikey, ck_parent);
    if (UPLL_RC_SUCCESS != result_code) {
      DELETE_IF_NOT_NULL(ck_parent);
      UPLL_LOG_DEBUG("GetChildConfigkey Failed");
      return result_code;
    }
    GET_USER_DATA_FLAGS(ikey, ikey_flag);
    SET_USER_DATA(ikey, ck_parent)
    ikey_flag |= (parent_renamed & RENAME_BITS);
    SET_USER_DATA_FLAGS(ikey, ikey_flag);
    GET_USER_DATA_CTRLR_DOMAIN(ikey, *ctrlr_dom);
  }
  delete ck_parent;
  return result_code;
}

upll_rc_t VnodeChildMoMgr::ReadMo(IpcReqRespHeader *header, ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t ktype = ikey->get_key_type();
  switch (ktype) {
  case UNC_KT_VBR_IF:
  case UNC_KT_VRT_IF:
  case UNC_KT_VUNK_IF:
  case UNC_KT_VTEP_IF:
  case UNC_KT_VTUNNEL_IF:
  case UNC_KT_VTERM_IF:
    UPLL_LOG_DEBUG("read request for %d, option 1 %d option2 %d", ktype,
                     header->option1, header->option2);
    if (header->option2 == UNC_OPT2_NEIGHBOR) {
      result_code = ValidateMessage(header, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ValidateMessage failed result_code %d", result_code);
        return result_code;
      }
      result_code = PopulateValVtnNeighbor(ikey, dmi);
      break;
    }
    /* fall through intended */
  default:
    result_code = MoMgrImpl::ReadMo(header, ikey, dmi);
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetControllerDomainId(
    ConfigKeyVal *pckv, upll_keytype_datatype_t dt_type,
    controller_domain *ctrlr_dom, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag | kOpInOutCtrlr
                       | kOpInOutDomain };
  if (!pckv) return UPLL_RC_ERR_GENERIC;
  unc_key_type_t pktype = pckv->get_key_type();
  VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager*>
                                                  (GetMoManager(pktype)));
  result_code = mgr->ReadConfigDB(pckv, dt_type, UNC_OP_READ, dbop, dmi,
                                  MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ReadConfigDB failed for pckv err_code %d", result_code);
    return result_code;
  }
  return (mgr->GetControllerDomainId(pckv, ctrlr_dom));
}


upll_rc_t VnodeChildMoMgr::PopulateValVtnNeighbor(ConfigKeyVal *&in_ckv,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  // Read on the left side of the link
  bool got_left_side = false;
  ConfigKeyVal *vlink_ckv = NULL;
  VlinkMoMgr *vlink_momgr = reinterpret_cast<VlinkMoMgr *>
                           (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
  if (!vlink_momgr) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }
  vn_if_type iftype;
  upll_rc_t result_code = vlink_momgr->CheckIfMemberOfVlink(in_ckv,
                               UPLL_DT_RUNNING, vlink_ckv, dmi, iftype);
  if (!vlink_ckv || result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_TRACE("Not found in vlink table");
  } else if (result_code == UPLL_RC_SUCCESS) {
    key_vlink_t *vlink_key = reinterpret_cast<key_vlink_t *>
                                             (vlink_ckv->get_key());
    if (!vlink_key) {
      UPLL_LOG_DEBUG("Invalid param");
      if (vlink_ckv) delete vlink_ckv;
      return UPLL_RC_ERR_GENERIC;
    }
    if ((iftype == kVlinkBoundaryNode1) || (iftype == kVlinkInternalNode1))
     got_left_side = true;
    val_vlink_t *vlink_val = reinterpret_cast<val_vlink *>
                       (GetVal(vlink_ckv));
    val_vtn_neighbor_t *val_vtn_neighbor =
     reinterpret_cast<val_vtn_neighbor_t *>
      (ConfigKeyVal::Malloc(sizeof(val_vtn_neighbor_t)));
    memset(val_vtn_neighbor, 0, sizeof(val_vtn_neighbor_t));
    val_vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_NAME_VN] = UNC_VF_VALID;
    val_vtn_neighbor->valid[UPLL_IDX_CONN_VNODE_IF_NAME_VN] = UNC_VF_VALID;
    val_vtn_neighbor->valid[UPLL_IDX_CONN_VLINK_NAME_VN] = UNC_VF_VALID;
    uuu::upll_strncpy(val_vtn_neighbor->connected_vnode_name,
                      ((got_left_side) ? vlink_val->vnode2_name :
                                         vlink_val->vnode1_name),
                      (kMaxLenVnodeName + 1));
    uuu::upll_strncpy(val_vtn_neighbor->connected_if_name,
                      ((got_left_side) ? vlink_val->vnode2_ifname :
                                         vlink_val->vnode1_ifname),
                      (kMaxLenInterfaceName + 1));
    uuu::upll_strncpy(val_vtn_neighbor->connected_vlink_name,
                      vlink_key->vlink_name, (kMaxLenVnodeName + 1));
    in_ckv->SetCfgVal(new ConfigVal(IpctSt::kIpcStValVtnNeighbor,
                                    val_vtn_neighbor));
  } else {
    UPLL_LOG_DEBUG("ReadConfigDB failed result_code - %d", result_code);
  }

  if (vlink_ckv) delete vlink_ckv;
  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetInterfaceType(ConfigKeyVal *ck_vnif,
                                            uint8_t valid_pm,
                                            if_type &vnif_type ) {
  UPLL_FUNC_TRACE;
  if (!ck_vnif) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  if (ck_vnif->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
    // converted interface will be boundary by default
    vnif_type = kBoundaryInterface;
    return UPLL_RC_SUCCESS;
  }
  uint8_t flags = 0;
  GET_USER_DATA_FLAGS(ck_vnif, flags);
  flags = flags & VIF_TYPE;
  if (flags & VIF_TYPE_BOUNDARY) {
    vnif_type = kBoundaryInterface;
  } else if (flags & VIF_TYPE_LINKED) {
    vnif_type = kLinkedInterface;
  } else if (valid_pm == UNC_VF_VALID) {
    vnif_type = kMappedInterface;
  } else {
    vnif_type = kUnboundInterface;
  }
  // vishnu tmp fix
  if (ck_vnif->get_key_type() == UNC_KT_VBR_PORTMAP) {
    if (valid_pm == UNC_VF_VALID) {
      vnif_type = kMappedInterface;
    } else {
      vnif_type = kUnboundInterface;
    }
  }
  UPLL_LOG_DEBUG("flags %d vnif_type %d", flags, vnif_type);
  return UPLL_RC_SUCCESS;
}

template<typename T3>
upll_rc_t VnodeChildMoMgr::IsLogicalPortAndVlanIdInUse(ConfigKeyVal *ikey,
                                                       DalDmlIntf *dmi,
                                                       IpcReqRespHeader *req) {
  UPLL_FUNC_TRACE;

  ConfigKeyVal *ckv        = NULL;
  T3           *if_val1    = NULL;
  T3           *if_val2    = NULL;
  upll_rc_t    result_code = UPLL_RC_SUCCESS;

  if (!ikey || !(ikey->get_cfg_val()))
    return UPLL_RC_ERR_GENERIC;

  result_code = GetChildConfigKey(ckv, NULL);
  if ((ckv == NULL) || (result_code != UPLL_RC_SUCCESS))
    return UPLL_RC_ERR_GENERIC;

  /* For VBR_IF keytype value structure is different.
   * VTERM_IF, VTEP_IF, VTUNNEL_IF have same kind of value
   * structure */
  if (ikey->get_key_type() == UNC_KT_VBR_IF) {
    val_drv_vbr_if *drv_ifval = static_cast<val_drv_vbr_if *>(GetVal(ikey));
    /* If Operation CREATE value structure is optional
     * For UPDATE operation the value structure validation is done
     * at ValidateMessage() */
    if (!drv_ifval) {
      DELETE_IF_NOT_NULL(ckv);
      return UPLL_RC_SUCCESS;
    }
    val_vbr_if_t *vbrif_val = &(static_cast<val_drv_vbr_if *>(
            GetVal(ikey))->vbr_if_val);
    if_val1 = reinterpret_cast<T3 *>(vbrif_val);
    if (!if_val1) {
      UPLL_LOG_DEBUG("Returning error\n");
      DELETE_IF_NOT_NULL(ckv);
      return UPLL_RC_ERR_GENERIC;
    }
  } else {
    if_val1 = reinterpret_cast<T3 *>GetVal(ikey);
    /* If Operation CREATE value structure is optional
     * For UPDATE operation the value structure validation is done
     * at ValidateMessage() */
    if (!if_val1) {
      DELETE_IF_NOT_NULL(ckv);
      return UPLL_RC_SUCCESS;
    }
  }

  if (if_val1->valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
    val_drv_vbr_if *drv_if_val2 = NULL;
    if (ikey->get_key_type() == UNC_KT_VBR_IF) {
      drv_if_val2 = reinterpret_cast<val_drv_vbr_if *>
          (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
      if_val2 = reinterpret_cast<T3 *>(&(drv_if_val2->vbr_if_val));
    } else {
      if_val2 = reinterpret_cast<T3 *>
                    (ConfigKeyVal::Malloc(sizeof(T3)));
    }

    val_port_map_t *portmap = &(if_val1->portmap);
    if (portmap->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] == UNC_VF_VALID) {
      uuu::upll_strncpy(if_val2->portmap.logical_port_id,
          if_val1->portmap.logical_port_id,
          kMaxLenLogicalPortId+1);
      if_val2->portmap.valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
      if_val2->valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;
    }

    if (ikey->get_key_type() == UNC_KT_VBR_IF) {
      ckv->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrIf, drv_if_val2);
    } else {
      ckv->AppendCfgVal((ikey->get_cfg_val())->get_st_num(), if_val2);
    }
    SET_USER_DATA(ckv, ikey);

    UPLL_LOG_DEBUG("Before read config DB Ckv%s \n", ckv->ToStrAll().c_str());
    /* Read Multiple matched entrie's based on logical_id
     * in the same controller */
    DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr,
                     kOpInOutFlag};
    result_code = ReadConfigDB(ckv, req->datatype, UNC_OP_READ,
                               dbop, dmi, MAINTBL);
    UPLL_LOG_TRACE("Read config Db result %u", result_code);
    if (result_code == UPLL_RC_SUCCESS) {
      ConfigKeyVal *tmp = ckv;
      while (tmp) {
        // Ignore the record if given input key and database key are same.
        if (IsEqual(*(reinterpret_cast<key_vnode_if_t *>(ikey->get_key())),
                    *(reinterpret_cast<key_vnode_if_t *>(tmp->get_key())))) {
          UPLL_LOG_TRACE("Looking on the Same key");
        } else {
          T3   *if_val_read = NULL;
          bool match        = false;
          if (ikey->get_key_type() == UNC_KT_VBR_IF) {
            if_val_read =
                reinterpret_cast<T3 *>(&(reinterpret_cast<val_drv_vbr_if *>
                (GetVal(tmp))->vbr_if_val));
          } else {
            if_val_read = reinterpret_cast<T3 *>(GetVal(tmp));
          }

          /* Validates whether requested vlan-id attributes and read config DB
           * result attributes are same*/
          if (if_val1->portmap.valid[UPLL_IDX_VLAN_ID_PM]
                                                         == UNC_VF_VALID) {
            if (if_val1->portmap.vlan_id == if_val_read->portmap.vlan_id) {
              if (if_val1->portmap.tagged == if_val_read->portmap.tagged)
                match = true;
            }
          } else {
            if (if_val_read->portmap.valid[UPLL_IDX_VLAN_ID_PM] ==
                UNC_VF_VALID) {
              if (if_val1->portmap.tagged == if_val_read->portmap.tagged)
                match = true;
            } else {
              match = true;
            }
          }
          if (match) {
            UPLL_LOG_DEBUG("More than one vnode interface is configured "
                           " same logical port id and vlanid!");
            delete ckv;
            ckv = tmp = NULL;
            return UPLL_RC_ERR_CFG_SEMANTIC;
          }
        }
        tmp = tmp->get_next_cfg_key_val();
      }
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
    }
  }
  DELETE_IF_NOT_NULL(ckv);
  return result_code;
}

template upll_rc_t VnodeChildMoMgr::IsLogicalPortAndVlanIdInUse<val_vbr_if_t>
                                          (ConfigKeyVal *ikey,
                                          DalDmlIntf *dmi,
                                          IpcReqRespHeader *req);

template upll_rc_t VnodeChildMoMgr::IsLogicalPortAndVlanIdInUse<val_vterm_if_t>
                                          (ConfigKeyVal *ikey,
                                          DalDmlIntf *dmi,
                                          IpcReqRespHeader *req);
template upll_rc_t VnodeChildMoMgr::IsLogicalPortAndVlanIdInUse<val_vtep_if>
                                          (ConfigKeyVal *ikey,
                                          DalDmlIntf *dmi,
                                          IpcReqRespHeader *req);
template upll_rc_t VnodeChildMoMgr::IsLogicalPortAndVlanIdInUse<val_vtunnel_if>
                                          (ConfigKeyVal *ikey,
                                          DalDmlIntf *dmi,
                                          IpcReqRespHeader *req);


upll_rc_t VnodeChildMoMgr::UpdateParentOperStatus(ConfigKeyVal *ikey,
                                                  DalDmlIntf *dmi,
                                                  uint32_t driver_result) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_ERROR("Returning error \n");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ktype = ikey->get_key_type();
  if ((ktype != UNC_KT_VBR_IF) && (ktype != UNC_KT_VRT_IF) &&
      (ktype != UNC_KT_VTERM_IF) && (ktype != UNC_KT_VBR_PORTMAP))
    return UPLL_RC_SUCCESS;
  if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED &&
      ktype != UNC_KT_VBR_PORTMAP) {
    return UPLL_RC_SUCCESS;
  }
  MoMgrTables tbl = MAINTBL;
  if (ikey->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
    tbl = CONVERTTBL;
  }
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  // value not bound during delete - read to get down count / oper status
  result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ,
                                  dbop, dmi, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return result_code;
  }
  val_db_vbr_if_st *valst =
      reinterpret_cast<val_db_vbr_if_st *>(GetStateVal(ikey));
  if (!valst) {
    UPLL_LOG_DEBUG("Returning error\n");
    return UPLL_RC_ERR_GENERIC;
  }
  if (valst->vbr_if_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] != UNC_VF_VALID) {
    return result_code;
  }
  if (valst->vbr_if_val_st.oper_status == UPLL_OPER_STATUS_UP &&
     ktype == UNC_KT_VBR_PORTMAP)
    return result_code;
  bool last_if = false;
  ConfigKeyVal *ck_vn = NULL;
  result_code = GetParentConfigKey(ck_vn, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return result_code;
  }
  /* check if it is last interface */
  uint32_t count = 0;
#if 0
  uint8_t tmp_ifname[kMaxLenInterfaceName + 1];
  key_vbr_if *vbrif_key= reinterpret_cast<key_vbr_if *>(ikey->get_key());
  uuu::upll_strncpy(tmp_ifname, vbrif_key->if_name, kMaxLenInterfaceName + 1);
  vbrif_key->if_name[0] = '\0';
  result_code = GetInstanceCount(ikey, NULL, UPLL_DT_RUNNING, &count, dmi,
       MAINTBL);
  uuu::upll_strncpy(vbrif_key->if_name, tmp_ifname, kMaxLenInterfaceName + 1);
#else
  ConfigKeyVal *dup_ikey = NULL;
  if (tbl == CONVERTTBL)
    result_code = GetChildConvertConfigKey(dup_ikey, ikey);
  else
    result_code = GetChildConfigKey(dup_ikey, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    DELETE_IF_NOT_NULL(ck_vn);
    return UPLL_RC_ERR_GENERIC;
  }
  if (tbl == CONVERTTBL) {
    key_convert_vbr_if *vbrif_key=
                  reinterpret_cast<key_convert_vbr_if*>(dup_ikey->get_key());
    vbrif_key->convert_if_name[0] = '\0';
  } else {
    key_vbr_if *vbrif_key= reinterpret_cast<key_vbr_if *>(dup_ikey->get_key());
    vbrif_key->if_name[0] = '\0';
  }
  result_code = GetInstanceCount(dup_ikey, NULL, UPLL_DT_RUNNING, &count, dmi,
       tbl);
  DELETE_IF_NOT_NULL(dup_ikey);
#endif
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    DELETE_IF_NOT_NULL(ck_vn);
    return UPLL_RC_ERR_GENERIC;
  }
  if (count == 1) {
    last_if = true;
  }
  VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
                          (GetMoManager(ck_vn->get_key_type())));
  if (!mgr) {
    UPLL_LOG_DEBUG("Returning error \n");
    delete ck_vn;
    return UPLL_RC_ERR_GENERIC;
  }
  if (!last_if && (valst->down_count == PORT_UP ||
                  (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED &&
                   ktype != UNC_KT_VBR_PORTMAP))) {
    /* if interface is not the last if and it is UP, return*/
    DELETE_IF_NOT_NULL(ck_vn);
    return UPLL_RC_SUCCESS;
  }
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone,
                   kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain };
  result_code = mgr->ReadConfigDB(ck_vn, UPLL_DT_STATE,
                      UNC_OP_READ, dbop1, dmi, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("'Returning error %d\n", result_code);
    } else {
      result_code = UPLL_RC_SUCCESS;
    }
    DELETE_IF_NOT_NULL(ck_vn);
    return result_code;
  }
  if (!last_if) {
    state_notification notfn = kCommit;
    if (valst->down_count == PORT_UP) {
     /* If last interface was UP, then vNode continues to be UP
      * if interface was UNKNOWN due to controller disconnect,
      * vNode continues to be UNKNOWN*/
        return UPLL_RC_SUCCESS;
    } else if (valst->down_count & PORT_UNKNOWN) {
      notfn = kPortFaultResetFromUnknown;
    } else {
      notfn = kPortFaultReset;
    }
    result_code = mgr->UpdateOperStatus(ck_vn, dmi, notfn, true);
  } else {
    result_code = mgr->UpdateLastInterfaceDelete(ikey, ck_vn, dmi);
  }
  DELETE_IF_NOT_NULL(ck_vn);
  return result_code;
}

upll_rc_t VnodeChildMoMgr::UpdateOperStatus(ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi,
                                       state_notification notification,
                                       unc_keytype_operation_t op,
                                       bool skip,
                                       bool propagate) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrTables tbl = MAINTBL;
  if (ikey->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf)
    tbl = CONVERTTBL;
  if (!skip) {
    DbSubOp dbop =
    { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
    kOpInOutCs | kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag };
    result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                               tbl);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in reading: %d", result_code);
      return result_code;
    }
  }
  ConfigKeyVal *tkey = ikey;
  unc_key_type_t ktype  = ikey->get_key_type();
  while (tkey != NULL) {
    bool oper_status_change = false;
    switch (ktype) {
    case UNC_KT_VBR_IF:
      result_code = SetOperStatus
         <val_vbr_if_st, val_db_vbr_if_st>(tkey, notification, op, dmi,
                                           oper_status_change);
      break;
    case UNC_KT_VRT_IF:
      result_code = SetOperStatus
         <val_vrt_if_st, val_db_vrt_if_st>(tkey, notification, op, dmi,
                                           oper_status_change);
      break;
    case UNC_KT_VTEP_IF:
      result_code = SetOperStatus
         <val_vtep_if_st, val_db_vtep_if_st>(tkey, notification, op, dmi,
                                             oper_status_change);
      break;
    case UNC_KT_VTUNNEL_IF:
      result_code = SetOperStatus
         <val_vtunnel_if_st, val_db_vtunnel_if_st>(tkey, notification, op, dmi,
                                                   oper_status_change);
      break;
    case UNC_KT_VTERM_IF:
      result_code = SetOperStatus
         <val_vterm_if_st, val_db_vterm_if_st>(tkey, notification, op, dmi,
                                               oper_status_change);
      break;
    case UNC_KT_VBR_PORTMAP:
      result_code = SetOperStatus
         <val_vbr_portmap_st, val_db_vbr_portmap_st>(tkey, notification, op,
                                                     dmi, oper_status_change);
      break;
    default:
      UPLL_LOG_DEBUG("oper status attribute not supported");
      return UPLL_RC_SUCCESS;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in SetOperStatus");
      return result_code;
    }
//    alarm_status oper = ALARM_OPER_DOWN;
//    if (!EnqueOperStatusNotification(tkey, oper)) {
//      UPLL_LOG_DEBUG("Alarm Notification failed");
//      return UPLL_RC_ERR_GENERIC;
//     }
     if (propagate == true && oper_status_change == true) {
      ConfigKeyVal *ck_parent = NULL;
      result_code = GetParentConfigKey(ck_parent, tkey);
      if (!ck_parent || result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
      VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>
           (const_cast<MoManager *>(GetMoManager(ck_parent->get_key_type())));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        DELETE_IF_NOT_NULL(ck_parent);
        return UPLL_RC_ERR_GENERIC;
      }
      DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                       kOpInOutCtrlr | kOpInOutDomain };
      result_code = mgr->ReadConfigDB(ck_parent, UPLL_DT_STATE,
                                      UNC_OP_READ, dbop, dmi,
                                      tbl);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in ReadConfigDB : %d", result_code);
        DELETE_IF_NOT_NULL(ck_parent);
        return result_code;
      }
      ConfigVal *tmp =
          (ck_parent->get_cfg_val()) ?
          ck_parent->get_cfg_val()->get_next_cfg_val() : NULL;
      val_db_vbr_st* vn_valst = reinterpret_cast<val_db_vbr_st*>((tmp != NULL) ?
                                                 tmp->get_val() : NULL);
      if (vn_valst == NULL) {
        UPLL_LOG_DEBUG("Invalid param");
        DELETE_IF_NOT_NULL(ck_parent);
        return UPLL_RC_ERR_GENERIC;
      }
      if (vn_valst->vbr_val_st.oper_status == UPLL_OPER_STATUS_UNINIT) {
        /* OperStatus update will be handled as part of vNode TxUpdateDtState */
        DELETE_IF_NOT_NULL(ck_parent);
        tkey = tkey->get_next_cfg_key_val();
        continue;
      } else {
        result_code = mgr->UpdateOperStatus(ck_parent, dmi, notification, true);
        DELETE_IF_NOT_NULL(ck_parent);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("vNodeChildMoMgr UpdateOperStatus failed : %d",
                         result_code);
          return result_code;
        }
      }
    }
    tkey = tkey->get_next_cfg_key_val();
  }
  return result_code;
}

template<typename T1, typename T2>
upll_rc_t VnodeChildMoMgr::SetOperStatus(ConfigKeyVal *ikey,
                                  state_notification &notification,
                                  unc_keytype_operation_t op,
                                  DalDmlIntf *dmi, bool &oper_change) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_name = "";

  MoMgrTables tbl = MAINTBL;
  if (ikey->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf)
    tbl = CONVERTTBL;

  /* update corresponding interface operstatus */
  ConfigVal *tmp = (ikey->get_cfg_val()) ?
                    ikey->get_cfg_val()->get_next_cfg_val() : NULL;
  T2 *vn_valst = reinterpret_cast<T2 *>((tmp != NULL) ? tmp->get_val() : NULL);
  if (vn_valst == NULL) return UPLL_RC_ERR_GENERIC;
  T1 *vn_val = reinterpret_cast<T1 *>(vn_valst);

  UPLL_LOG_DEBUG("notification %d down_count %d ",
                 notification, vn_valst->down_count);

  /* Update oper status based on notification */
  vn_val->valid[0] = UNC_VF_VALID;
  uint32_t orig_status = 0;
  switch (notification) {
  case kCommit:
    if (vn_valst->down_count & PORT_UNKNOWN) {
      /* interface is UNKNOWN */
      /* if interface is UNKNOWN, then there is change only in admin flag */
      if (vn_val->oper_status != UPLL_OPER_STATUS_UNKNOWN)
        oper_change = true;
      vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      notification = kPortUnknown;
    } else {
      if (vn_valst->down_count == PORT_UP) {
        if (vn_val->oper_status == UPLL_OPER_STATUS_UNINIT ||
            vn_val->oper_status == UPLL_OPER_STATUS_UNKNOWN) {
          oper_change = true;
          vn_val->oper_status = UPLL_OPER_STATUS_UP;
          notification = kPortUp;
        } else if (vn_val->oper_status == UPLL_OPER_STATUS_DOWN) {
          oper_change = true;
          vn_val->oper_status = UPLL_OPER_STATUS_UP;
          notification = kPortFaultReset;
        }
      } else {
        /* if change is only in admin status flags, then oper_status
         * will already be DOWN */
        if (vn_val->oper_status != UPLL_OPER_STATUS_DOWN)
          oper_change = true;
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
        notification = kPortFault;
      }
    }
    break;
  case kPortUnknown:
    /* if Port becomes UNKNOWN from DOWN
       then change notification to kPortUnknownFromDown */
    if (vn_valst->down_count != PORT_UP) {
      notification = kPortUnknownFromDown;
    }

    /* Clear Port fault flag. Set Port Unknown flag.
     * Set OperStatus to UNKNOWN and propagate to its Parent */

    vn_valst->down_count &= ~PORT_FAULT;
    vn_valst->down_count |= PORT_UNKNOWN;
    vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    oper_change = true;
    break;
  case kPortFault:
    orig_status = vn_valst->down_count;
    if (orig_status & PORT_UNKNOWN) {
      notification = kPortFaultFromUnknown;
      vn_valst->down_count &= ~PORT_UNKNOWN;
    }
    /* If OperStatus is UP or UNKNOWN, then set OperStatus to DOWN and
     * propagate to it's Parent*/
    if ((orig_status == PORT_UP) || (orig_status & PORT_UNKNOWN)) {
      oper_change = true;
    }
    vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    vn_valst->down_count |= PORT_FAULT;
    break;
  case kPortFaultReset:
    orig_status = vn_valst->down_count;
    if (orig_status & PORT_UNKNOWN) {
      /* if original flag is UNKNOWN, then clear unknown flag,
       * change notification to kPortFaultResetFromUnknown. Propagate. */
      oper_change = true;
      vn_valst->down_count &= ~PORT_UNKNOWN;
      /* if flag becomes UP, then set operstatus to UP, else DOWN */
      if (vn_valst->down_count == PORT_UP) {
        notification = kPortFaultResetFromUnknown;
        vn_val->oper_status = UPLL_OPER_STATUS_UP;
      } else {
        notification = kPortFaultFromUnknown;
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      }
    } else if (orig_status & ~PORT_FAULT) {
      /* operstatus is already DOWN due some other reason than PORT_FAULT.
       * Clear PORT_FAULT flag and break */
      vn_valst->down_count &= ~PORT_FAULT;
      vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    } else {
      /* If operstatus is DOWN only due to PORT_FAULT. Clear PORT_FAULT flag.
       * Set OperStatus to UP and propagate */
      oper_change = true;
      vn_valst->down_count &= ~PORT_FAULT;
      vn_val->oper_status = UPLL_OPER_STATUS_UP;
    }
    break;
  case kPathFault:
    orig_status = vn_valst->down_count;
    /* If OperStatus is UP, set OperStatus to DOWN and
     * propagate to it's Parent. */
    if (orig_status == PORT_UP) {
      oper_change = true;
      notification = kPortFault;
    }
    /* Set the Down count's Pathfualt flag */
    vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    vn_valst->down_count |= PATH_FAULT;
    break;
  case kPathFaultReset:
    /* Clear the Path fault flag. If OperStatus is UP , then set OperStatus
     * to UP and  propagate to it's Parent*/
    vn_valst->down_count &= ~PATH_FAULT;
    if (vn_valst->down_count == PORT_UP) {
      oper_change = true;
      vn_val->oper_status = UPLL_OPER_STATUS_UP;
      notification = kPortFaultReset;
    }
    break;
  default:
      UPLL_LOG_DEBUG("unsupported notification for operstatus update");
      return UPLL_RC_ERR_GENERIC;
    break;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  dbop.inoutop =  kOpInOutCs | kOpInOutFlag |
                    kOpInOutCtrlr | kOpInOutDomain;
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, op, dmi, &dbop,
                               TC_CONFIG_GLOBAL, vtn_name, tbl);
  UPLL_LOG_TRACE("VnodeChild SetOperstatus for VTN after Update is \n %s",
                    ikey->ToStr().c_str());
  return result_code;
}

upll_rc_t VnodeChildMoMgr::SetInterfaceOperStatus(ConfigKeyVal *&ck_vnif,
                                                  DalDmlIntf *dmi,
                                                  unc_keytype_operation_t op,
                                                  bool propagate,
                                                  uint32_t driver_result) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_name = "";
  if (!ck_vnif) {
    UPLL_LOG_DEBUG("empty ck_vnif");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigVal *tmp = (ck_vnif->get_cfg_val()) ?
                    ck_vnif->get_cfg_val()->get_next_cfg_val() : NULL;
  MoMgrTables tbl = MAINTBL;
  if (ck_vnif->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
    tbl = CONVERTTBL;
  }
  if (tmp == NULL) {
    /* Update on interface is not OperStatus Update */
    DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutCs};
    return (UpdateConfigDB(ck_vnif, UPLL_DT_RUNNING, op, dmi, &dbop,
                           TC_CONFIG_GLOBAL, vtn_name, tbl));
  }
  val_db_vbr_if_st *vnif_st =
      reinterpret_cast<val_db_vbr_if_st*>(tmp->get_val());
  /* orig_flag hold present interface down_count
   * new_flag is computed from scratch */
  uint32_t orig_flag = vnif_st->down_count, new_flag = 0;
  uint8_t valid_pm = 0, valid_admin = 0, admin_status = 0;
  val_port_map *pm = NULL, vbr_pm;
  memset(&vbr_pm, 0, sizeof(val_port_map));
  if (ck_vnif->get_key_type() == UNC_KT_VBR_PORTMAP) {
    pm = &vbr_pm;
  }
  result_code = GetPortMap(ck_vnif, valid_pm, pm, valid_admin, admin_status);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return result_code;
  }
  UPLL_LOG_DEBUG("valid_pm %d valid_admin %d admin_status %d",
                  valid_pm, valid_admin, admin_status);
  /* Get admin status of interface */
  if (admin_status == UPLL_ADMIN_DISABLE) {
     vnif_st->down_count |= ADMIN_DISABLE;
     new_flag = new_flag | ADMIN_DISABLE;
  } else {
     vnif_st->down_count &= ~ADMIN_DISABLE;
  }
  if_type vnif_type = kUnboundInterface;
  result_code = GetInterfaceType(ck_vnif, valid_pm, vnif_type);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetInterfaceType error %d\n", result_code);
    return result_code;
  }
  UPLL_LOG_DEBUG("original down_count is %d", vnif_st->down_count);
  switch (vnif_type) {
  case kBoundaryInterface:
  case kLinkedInterface: {
    /* if there is a change in vlink interface admin status,
     * candidate-running diff is obtained only for interface.
     * So updation of vlink and remote interface should be handled here */
    if (vnif_st->down_count != orig_flag) {
      VlinkMoMgr *vlink_mgr =
               reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager *>
               (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK))));
      ConfigKeyVal *ck_vlink = NULL;
      ConfigKeyVal *ck_if[2] = {NULL, NULL};
      result_code = vlink_mgr->GetRemoteIf(ck_vnif, ck_if[1], ck_vlink,
                                           dmi, UPLL_DT_STATE);
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
         // todo:operstatus team remove this check.
         // AdminStatus shold be done only on vlinked interface
        DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutCs |
                     kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain};
        result_code = UpdateConfigDB(ck_vnif, UPLL_DT_STATE, op, dmi,
                                     &dbop, TC_CONFIG_GLOBAL, vtn_name,
                                     tbl);
        DELETE_IF_NOT_NULL(ck_vlink);
        DELETE_IF_NOT_NULL(ck_if[1]);
        result_code = UPLL_RC_SUCCESS;
        }
        return result_code;
      }
      /* if admin status is changed on an interface whose remote interface
       * is disconnected or if remote interface is bypass
       * then treat this interface as normal interface */
      if (vnif_st->vbr_if_val_st.oper_status == UPLL_OPER_STATUS_UNKNOWN) {
        new_flag |= PORT_UNKNOWN;
        DELETE_IF_NOT_NULL(ck_vlink);
        DELETE_IF_NOT_NULL(ck_if[1]);
        break;
      }
      if (ck_if[1]->get_key_type() != UNC_KT_VUNK_IF) {
        val_db_vbr_if_st *rem_if_st =
            reinterpret_cast<val_db_vbr_if_st*>(GetStateVal(ck_if[1]));
        if (rem_if_st->vbr_if_val_st.oper_status != UPLL_OPER_STATUS_UNKNOWN) {
          ck_if[0] = ck_vnif;
          vnif_st->down_count = orig_flag;
          result_code = vlink_mgr->UpdateVlinkAndIfOperStatus(ck_vlink, ck_if,
                                                              0, 0, dmi);
          DELETE_IF_NOT_NULL(ck_vlink);
          DELETE_IF_NOT_NULL(ck_if[1]);
          return result_code;
        }
      }
      DELETE_IF_NOT_NULL(ck_vlink);
      DELETE_IF_NOT_NULL(ck_if[1]);
    } else {
     /* if there is no change in admin status of interface
      * then update the DB with present value
      * Later it will be handled as part of vLink TxUpdateDtState */
      if (vnif_type == kLinkedInterface) {
          if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
            new_flag |= PORT_UNKNOWN;
          }
          break;
      } else {
        uint8_t *ctrlr_name = NULL;
        GET_USER_DATA_CTRLR(ck_vnif, ctrlr_name);
        uint8_t oper_status = UPLL_OPER_STATUS_UNINIT;
        if (ck_vnif->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
          // set gateway port staus to converted if.spine domain interface
          // (vtun_if) doesnt contribute to Unified vBr operstatus
          uint8_t user_flag = 0;
          GET_USER_DATA_FLAGS(ck_vnif, user_flag);
          if (user_flag & BOUNDARY_UVBRIF_FLAG) {
            // since conv if is part of unified vbr if.
            // it doesnt contribute to operstatus
            oper_status = UPLL_OPER_STATUS_UP;
            propagate = false;
          } else {
            if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
              new_flag |= PORT_UNKNOWN;
              break;
            }
            result_code = GetGatewayPortStatus(ck_vnif, &oper_status, dmi);
            switch (oper_status) {
            case  UPLL_OPER_STATUS_DOWN:
              new_flag |= PORT_FAULT;
            break;
            case  UPLL_OPER_STATUS_UNKNOWN:
              new_flag |= PORT_UNKNOWN;
            break;
            case  UPLL_OPER_STATUS_UP:
              new_flag |= PORT_UP;
            break;
            default:
            break;
            }
          }
        } else if (ctrlr_name && IsUnifiedVbr(ctrlr_name)) {
          // need not propagate to parent. Set the operstatus
          // for end user display. Propagation happens as part of converted if
          propagate = false;
        } else {
          DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutCs |
                   kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain};
          result_code = UpdateConfigDB(ck_vnif, UPLL_DT_STATE, op, dmi,
                                 &dbop, TC_CONFIG_GLOBAL,
                                 vtn_name, tbl);
          return result_code;
        }
      }
    }
  }
  break;
  case kMappedInterface: {
    if (pm && valid_pm == UNC_VF_VALID) {
      if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
        new_flag |= PORT_UNKNOWN;
        break;
      }
      controller_domain_t ctrlr_dom = {NULL, NULL};
      val_oper_status port_oper_status = UPLL_OPER_STATUS_UP;
      GET_USER_DATA_CTRLR_DOMAIN(ck_vnif, ctrlr_dom);
      result_code = GetPortStatusFromPhysical(pm, ctrlr_dom, port_oper_status);
      /* During import Physical don't have the Port status
       * in the running db so its return no such instance
       * so here comment this check */
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error retrieving port status from physical %d %d",
                     result_code, port_oper_status);
      // return result_code;
      }
      switch (port_oper_status) {
      case  UPLL_OPER_STATUS_DOWN:
        new_flag |= PORT_FAULT;
        break;
      case  UPLL_OPER_STATUS_UNKNOWN:
        new_flag |= PORT_UNKNOWN;
        break;
      case  UPLL_OPER_STATUS_UP:
        new_flag |= PORT_UP;
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
      }
    }
  break;
  }
  case kUnboundInterface: {
    new_flag |= PORT_FAULT;
    if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED &&
      vnif_st->down_count & PORT_UNKNOWN) {
      new_flag |= PORT_UNKNOWN;
    }
  }
  break;
  }

  state_notification notification = kCommit;
  if (orig_flag == PORT_UP) {
    /* if original status of interface is UP,
    * update the newly computed flag value to interface down_count */
    notification = kCommit;
    vnif_st->down_count = new_flag;
  } else if (orig_flag & PORT_UNKNOWN) {
    /* original status of interface is UNKNOWN */
    if (new_flag & PORT_UNKNOWN) {
      notification = kCommit;
    } else if (new_flag == PORT_UP) {
      /* new status of interface is still UP */
      notification = kPortFaultReset;
    } else if (new_flag & ~PORT_UNKNOWN) {
      /* new status of interface is DOWN */
      notification = kPortFault;
    }
  } else {
    /* original status of interface is DOWN */
    if (new_flag & ~PORT_UNKNOWN) {
      notification = kCommit;
    } else if (new_flag & PORT_UNKNOWN) {
      /* new status of interface is UNKNOWN */
      notification = kPortUnknown;
    } else if (new_flag == PORT_UP) {
      /* new status of interface is UP */
      notification = kPortFaultReset;
    }
  }
  if (tbl == MAINTBL && propagate == false &&
      driver_result != UPLL_RC_ERR_CTR_DISCONNECTED) {
    /* propagate flag is used to determine if Interface OperStatus update
     * should be propagated to Parent vNode and further.
     * If vNode is UNINIT,
     *    propagate is set to false,
     *    vNode update will be handled later as part of TxUpdateDtState
     * Else,
     *    propagate is set to true. */
    unc_key_type_t pktype = UNC_KT_ROOT, ktype = ck_vnif->get_key_type();
    if (ktype == UNC_KT_VBR_IF) {
      pktype = UNC_KT_VBRIDGE;
    } else if (ktype == UNC_KT_VRT_IF) {
      pktype = UNC_KT_VROUTER;
    } else if (ktype == UNC_KT_VTERM_IF) {
      pktype = UNC_KT_VTERMINAL;
    }

    /* Create respective parent vNode ConfigKeyVal
     * with OperStatus field set to UPLL_OPER_STATUS_UNINIT */
    ConfigKeyVal *ck_vn = NULL;
    result_code = GetParentConfigKey(ck_vn, ck_vnif);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return result_code;
    }
    VnodeMoMgr *vn_mgr = reinterpret_cast<VnodeMoMgr *>
                             (const_cast<MoManager *>(GetMoManager(pktype)));
    ConfigVal *cval = NULL;
    result_code = vn_mgr->AllocVal(cval, UPLL_DT_STATE, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d", result_code);
      DELETE_IF_NOT_NULL(ck_vn);
      return result_code;
    }
    ck_vn->AppendCfgVal(cval);
    val_db_vbr_st *st_val =
        reinterpret_cast<val_db_vbr_st*>(GetStateVal(ck_vn));
    st_val->vbr_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
    st_val->vbr_val_st.valid[0]= UNC_VF_VALID;

    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
    result_code = vn_mgr->UpdateConfigDB(ck_vn, UPLL_DT_STATE, UNC_OP_READ,
                                        dmi, &dbop, tbl);
    DELETE_IF_NOT_NULL(ck_vn);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      /* flag set to true as vNode OPerStatus is not UPLL_OPER_STATUS_UNINIT */
      propagate = true;
    } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      /* flag set to false as vNode OPerStatus is UPLL_OPER_STATUS_UNINIT */
      propagate = false;
    } else {
      UPLL_LOG_DEBUG("Error in reading DB");
      return result_code;
    }
  }
  result_code = UpdateOperStatus(ck_vnif, dmi, notification,
                                 op, true, propagate);
  UPLL_LOG_DEBUG("final down_count is %d", vnif_st->down_count);
  return result_code;
}

upll_rc_t VnodeChildMoMgr::PortStatusHandler(ConfigKeyVal *ikey,
                                             const char *ctrlr_name,
                                             const char *domain_name,
                                             const char *logical_port_id,
                                             uint8_t oper_status,
                                             uuc::UpllDbConnMgr* db_con,
                                             uuc::ConfigLock* cfg_lock) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_TRACE("controller:%s portid:%s domain:%s oper_status:%d",
                 ctrlr_name, logical_port_id, domain_name, oper_status);
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  std::set<std::string> vtn_list;
  {
  uuc::ScopedConfigLock lock(*cfg_lock, uuc::kNormalTaskPriority,
                             UPLL_DT_RUNNING, uuc::ConfigLock::CFG_READ_LOCK);
  uuc::DalOdbcMgr *dmi = db_con->GetAlarmRwConn();
  if (dmi == NULL) { return UPLL_RC_ERR_GENERIC; }
  SET_USER_DATA_CTRLR(ikey, ctrlr_name);
  SET_USER_DATA_DOMAIN(ikey, domain_name);

  result_code = PortStatusHandler(ikey, logical_port_id,
                                  oper_status, &vtn_list, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Error %d", result_code);
    db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
    db_con->ReleaseRwConn(dmi);
    return result_code;
  }
  db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
  db_con->ReleaseRwConn(dmi);
  }
  uuc::NormalTaskYield oper_yield(
      uuc::TaskYieldIntf::YIELD_OP_OPER_STATUS_CTR_EVENT, cfg_lock);
  std::set<std::string>::iterator it = vtn_list.begin(),
                                  it_end = vtn_list.end();
  for (; it != it_end; ++it) {
    uuc::ScopedYield scfg_yield(&oper_yield);
    uuc::DalOdbcMgr *dmi = db_con->GetAlarmRwConn();
    if (dmi == NULL) {
       vtn_list.clear();
       return UPLL_RC_ERR_GENERIC;
    }
    // ikey for various applicable KTs contains key_vtn at offset 0.
    key_vtn *vtn_key = reinterpret_cast<key_vtn*>(ikey->get_key());
    uuu::upll_strncpy(vtn_key->vtn_name, (*it).c_str(), kMaxLenVtnName + 1);
    result_code = PortStatusHandler(ikey, oper_status, logical_port_id, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Error %d", result_code);
      db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
      db_con->ReleaseRwConn(dmi);
      vtn_list.clear();
      return result_code;
    }
    db_con->DalTxClose(dmi, (result_code == UPLL_RC_SUCCESS));
    db_con->ReleaseRwConn(dmi);
  }
  vtn_list.clear();
  return result_code;
}

upll_rc_t VnodeChildMoMgr::PortStatusHandler(ConfigKeyVal *ckv_vnif,
                           uint8_t oper_status, const char *logical_port_id,
                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrTables tbl = MAINTBL;
  if (ckv_vnif->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf) {
    tbl = CONVERTTBL;
    ConfigKeyVal *ckv_gw_vtn = NULL;
    VtnMoMgr *vtn_mgr = reinterpret_cast<VtnMoMgr *>(
        const_cast<MoManager*>(GetMoManager(UNC_KT_VTN)));
    result_code = vtn_mgr->GetChildConfigKey(ckv_gw_vtn, ckv_vnif);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Error in GetChildConfigKey");
      return result_code;
    }
    result_code = vtn_mgr->ResetGWPortStatus(ckv_gw_vtn, dmi, oper_status,
                                             logical_port_id, false);
    DELETE_IF_NOT_NULL(ckv_gw_vtn);
    if (result_code != UPLL_RC_SUCCESS) {
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Error in ResetGWPortStatus");
      }
      return result_code;
    }
  }
  ConfigKeyVal *ikey = NULL;
  result_code = DupConfigKeyVal(ikey, ckv_vnif, tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Error in DupConfigKeyVal %d", result_code);
    return result_code;
  }
  DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr | kOpMatchDomain,
                   kOpInOutCs | kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag };
  result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                             tbl);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code = UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    }
    DELETE_IF_NOT_NULL(ikey);
    return result_code;
  }
  while (ikey != NULL) {
    ConfigKeyVal *ck_tmp = ikey->get_next_cfg_key_val();
    ikey->set_next_cfg_key_val(NULL);
    uint8_t user_flag = 0;
    GET_USER_DATA_FLAGS(ikey, user_flag);
    if ((tbl == CONVERTTBL) && (user_flag & BOUNDARY_UVBRIF_FLAG)) {
      // will be handled as part of corresponding UvBrIf
      DELETE_IF_NOT_NULL(ikey);
      ikey = ck_tmp;
      continue;
    }
    uint8_t valid_pm = 0, valid_admin = 0, admin_status = 0;

    val_port_map *pm = NULL, vbr_pm;
    memset(&vbr_pm, 0, sizeof(val_port_map));
    if (ikey->get_key_type() == UNC_KT_VBR_PORTMAP) {
      pm = &vbr_pm;
    }
    result_code = GetPortMap(ikey, valid_pm, pm, valid_admin, admin_status);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      DELETE_IF_NOT_NULL(ikey);
      DELETE_IF_NOT_NULL(ck_tmp);
      return result_code;
    }
    UPLL_LOG_DEBUG("valid_pm %d valid_admin %d admin_status %d",
                  valid_pm, valid_admin, admin_status);
    if (admin_status == UPLL_ADMIN_DISABLE) {
      val_db_vbr_if_st *vnif_st =
          reinterpret_cast<val_db_vbr_if_st*>(GetStateVal(ikey));
      vnif_st->down_count |= ADMIN_DISABLE;
    }
    state_notification notification = kCommit;  // no op
    switch (oper_status) {
      case UPPL_LOGICAL_PORT_OPER_UP :
        notification = kPortFaultReset;
      break;
      case UPPL_LOGICAL_PORT_OPER_DOWN:
      notification = kPortFault;
      break;
      case UPPL_LOGICAL_PORT_OPER_UNKNOWN:
        notification = kPortUnknown;
      break;
      default:
        UPLL_LOG_DEBUG("Unexpected OPerStatus");
        return UPLL_RC_ERR_GENERIC;
    }
    if_type vnif_type = kUnboundInterface;
    result_code = GetInterfaceType(ikey, valid_pm, vnif_type);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetInterfaceType error %d\n", result_code);
      DELETE_IF_NOT_NULL(ikey);
      DELETE_IF_NOT_NULL(ck_tmp);
      return result_code;
    }
    if (vnif_type == kBoundaryInterface && tbl != CONVERTTBL) {
      /* vlinked interfaces which become UNKNOWN should  be handled here */
      ConfigKeyVal *ck_vlink = NULL;
      ConfigKeyVal *ck_rem_if = NULL;
      VlinkMoMgr *vlink_mgr =
            reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager *>
            (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK))));
      result_code = vlink_mgr->GetRemoteIf(ikey, ck_rem_if, ck_vlink,
                                           dmi, UPLL_DT_STATE);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in getting remote interface");
        DELETE_IF_NOT_NULL(ikey);
        DELETE_IF_NOT_NULL(ck_tmp);
        return result_code;
      }
      if (ck_rem_if->get_key_type() != UNC_KT_VUNK_IF) {
        val_db_vbr_if_st *rem_if_st =
            reinterpret_cast<val_db_vbr_if_st*>(GetStateVal(ck_rem_if));
        if (notification == kPortUnknown) {
          if (rem_if_st->vbr_if_val_st.oper_status !=
              UPLL_OPER_STATUS_UNKNOWN) {
            result_code = RecomputeVlinkAndIfoperStatus(ck_vlink, ck_rem_if,
                                                        dmi, notification);
            if (result_code != UPLL_RC_SUCCESS) {
              DELETE_IF_NOT_NULL(ck_vlink);
              DELETE_IF_NOT_NULL(ck_rem_if);
              DELETE_IF_NOT_NULL(ikey);
              DELETE_IF_NOT_NULL(ck_tmp);
              UPLL_LOG_DEBUG("Error in RecomputeVlinkAndIfoperStatus");
              return result_code;
            }
          }
        } else {
          if (rem_if_st->vbr_if_val_st.oper_status !=
              UPLL_OPER_STATUS_UNKNOWN) {
            DELETE_IF_NOT_NULL(ikey);
            DELETE_IF_NOT_NULL(ck_vlink);
            DELETE_IF_NOT_NULL(ck_rem_if);
            ikey = ck_tmp;
            continue;
          }
        }
      } else {
        if (notification != kPortUnknown) {
          DELETE_IF_NOT_NULL(ck_vlink);
          DELETE_IF_NOT_NULL(ck_rem_if);
          /* for boundary across bypass, boundary notification
           * would be received */
          DELETE_IF_NOT_NULL(ikey);
          ikey = ck_tmp;
          continue;
        }
      }
      DELETE_IF_NOT_NULL(ck_vlink);
      DELETE_IF_NOT_NULL(ck_rem_if);
    }
    result_code = UpdateOperStatus(ikey, dmi, notification,
                                   UNC_OP_UPDATE, true, true);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Invalid oper status update %d", result_code);
      return result_code;
    }
    DELETE_IF_NOT_NULL(ikey);
    ikey = ck_tmp;
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::PortStatusHandler(ConfigKeyVal *ikey,
                                             const char *logical_port_id,
                                             uint8_t oper_status,
                                             std::set<std::string> *vtn_list,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t keytype = ikey->get_key_type();
  MoMgrTables tbl = MAINTBL;
  if (ikey->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf)
    tbl = CONVERTTBL;
  if (keytype == UNC_KT_VBR_IF) {
    if (ikey->get_st_num() != IpctSt::kIpcStKeyConvertVbrIf) {
      /* Allocate Memory for UNC_KT_VBR_IF key and value structure */
      val_drv_vbr_if *vbrif_val = reinterpret_cast<val_drv_vbr_if*>
        (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));
      val_port_map *pm = &vbrif_val->vbr_if_val.portmap;
      (vbrif_val->vbr_if_val).valid[UPLL_IDX_PM_VBRI] = UNC_VF_VALID;

      /* Copy port_id from input to port_id variable in
       * portmap structure */
      uuu::upll_strncpy(pm->logical_port_id, logical_port_id,
                (kMaxLenLogicalPortId + 1));
      /* set valid flag as VALID */
      pm->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
      ikey->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrIf, vbrif_val);
    }
  } else if (keytype == UNC_KT_VTERM_IF) {
    /* Allocate Memory for UNC_KT_VTERM_IF key and value structure */
    val_vterm_if *vtermif_val = reinterpret_cast<val_vterm_if*>
        (ConfigKeyVal::Malloc(sizeof(val_vterm_if)));
    val_port_map_t *pm = &vtermif_val->portmap;
    vtermif_val->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_VALID;

    /* Copy port_id from input to port_id variable in
     * portmap structure */
    uuu::upll_strncpy(pm->logical_port_id, logical_port_id,
                      (kMaxLenLogicalPortId + 1));
    /* set valid flag as VALID */
    pm->valid[UPLL_IDX_LOGICAL_PORT_ID_PM] = UNC_VF_VALID;
    ikey->AppendCfgVal(IpctSt::kIpcStValVtermIf, vtermif_val);
  } else if (keytype == UNC_KT_VBR_PORTMAP) {
    /* Allocate Memory for UNC_KT_VBR_PORTMAP key and value structure */
    pfcdrv_val_vbr_portmap *vbrpm_val =
        reinterpret_cast<pfcdrv_val_vbr_portmap*>
        (ConfigKeyVal::Malloc(sizeof(pfcdrv_val_vbr_portmap)));
    val_vbr_portmap *pm = &vbrpm_val->vbrpm;
    vbrpm_val->valid[0] = UNC_VF_VALID;

    /* Copy port_id from input to port_id variable in
     * portmap structure */
    uuu::upll_strncpy(pm->logical_port_id, logical_port_id,
                      (kMaxLenLogicalPortId + 1));
    /* set valid flag as VALID */
    pm->valid[2] = UNC_VF_VALID;
    ikey->AppendCfgVal(IpctSt::kIpcStPfcdrvValVbrPortMap, vbrpm_val);
  } else  {
    UPLL_LOG_DEBUG("Unsupported keytype");
    return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  // if (tbl == CONVERTTBL)
    // match ctrlr_dom bcos logicalportid is not mapped to interface
    // in case of convert_vbr_if
    dbop.matchop = kOpMatchCtrlr|kOpMatchDomain;
  /* Existance check in vbrif table */
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ,
                               dmi, &dbop, tbl);
  /* Update VBR_If OperStatus */
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  } else if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
    if (tbl == CONVERTTBL) {
      val_db_vbr_if_st *val = reinterpret_cast<val_db_vbr_if_st *>
                      (ConfigKeyVal::Malloc(sizeof(val_db_vbr_if_st)));
      ConfigVal *ck_nxtval = new ConfigVal(IpctSt::kIpcStValVbrIfSt, val);
      ConfigVal *cfg_val = ikey->get_cfg_val();
      cfg_val->AppendCfgVal(ck_nxtval);
    }
    result_code = GetUniqueVtns(ikey, vtn_list, dmi);
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetCtrlrStatusFromPhysical(uint8_t *ctrlr_name,
                            val_oper_status &ctrlr_operstatus) {
  UPLL_FUNC_TRACE;
  IpcResponse ipc_resp;
  ConfigKeyVal *ck_ctrlr = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_ctr_t *ctrlr_key = static_cast<key_ctr_t *>
                        (ConfigKeyVal::Malloc(sizeof(key_ctr_t)));
  uuu::upll_strncpy(ctrlr_key->controller_name,
                    reinterpret_cast<const char *>(ctrlr_name),
                    (kMaxLenCtrlrId+1));
  ck_ctrlr = new ConfigKeyVal(UNC_KT_CONTROLLER,
                                 IpctSt::kIpcStKeyCtr,
                                 ctrlr_key, NULL);
  result_code = SendIpcReq(USESS_ID_UPLL, 0, UNC_OP_READ,
                   UPLL_DT_STATE, ck_ctrlr, NULL, &ipc_resp);
  if ((result_code != UPLL_RC_SUCCESS) || (!ipc_resp.ckv_data)) {
    DELETE_IF_NOT_NULL(ck_ctrlr);
    ctrlr_operstatus = UPLL_OPER_STATUS_UNKNOWN;
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      ctrlr_operstatus = UPLL_OPER_STATUS_UNKNOWN;
      UPLL_LOG_DEBUG("Invalid controller id");
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("Error in retrieving controller data from Physical");
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    return result_code;
  }
  ck_ctrlr->ResetWith(ipc_resp.ckv_data);
  DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
  val_ctr_st *phy_ctrlr_st = static_cast<val_ctr_st *>
                                         (GetVal(ck_ctrlr));
  if (phy_ctrlr_st) {
    switch (phy_ctrlr_st->oper_status) {
      case UPPL_CONTROLLER_OPER_DOWN:
      case UPPL_CONTROLLER_OPER_WAITING_AUDIT:
      case UPPL_CONTROLLER_OPER_AUDITING:
        ctrlr_operstatus = UPLL_OPER_STATUS_UNKNOWN;
        break;
      case UPPL_CONTROLLER_OPER_UP:
        ctrlr_operstatus = UPLL_OPER_STATUS_UP;
        break;
      default:
        break;
    }
  }
  DELETE_IF_NOT_NULL(ck_ctrlr)
  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetPortStatusFromPhysical(val_port_map *pm,
                            controller_domain_t ctr_domain,
                            val_oper_status &logical_port_operStatus) {
  UPLL_FUNC_TRACE;
  if (UpllConfigMgr::GetUpllConfigMgr()->get_map_phy_resource_status()
      == false) {
    logical_port_operStatus = UPLL_OPER_STATUS_UNKNOWN;
    return UPLL_RC_SUCCESS;
  }
  IpcResponse ipc_resp;
  ConfigKeyVal *log_Port_CK = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  uint32_t config_id  = 0;
  key_logical_port *phy_logPort_Key = static_cast<key_logical_port *>
                        (ConfigKeyVal::Malloc(sizeof(key_logical_port)));
  uuu::upll_strncpy(phy_logPort_Key->domain_key.ctr_key.controller_name,
                    reinterpret_cast<const char *>(ctr_domain.ctrlr),
                    (kMaxLenCtrlrId+1));
  uuu::upll_strncpy(phy_logPort_Key->domain_key.domain_name,
                    reinterpret_cast<const char *>(ctr_domain.domain),
                    (kMaxLenDomainId+1));
  uuu::upll_strncpy(phy_logPort_Key->port_id,
                    reinterpret_cast<const char *>(pm->logical_port_id),
                    (kMaxLenLogicalPortId+1));
  log_Port_CK = new ConfigKeyVal(UNC_KT_LOGICAL_PORT,
                                 IpctSt::kIpcStKeyLogicalPort,
                                 phy_logPort_Key, NULL);
  result_code = SendIpcReq(USESS_ID_UPLL, config_id, UNC_OP_READ,
                   UPLL_DT_STATE, log_Port_CK, NULL, &ipc_resp);
  if ((result_code != UPLL_RC_SUCCESS) || (!ipc_resp.ckv_data)) {
    delete log_Port_CK;
    log_Port_CK = NULL;
    logical_port_operStatus = UPLL_OPER_STATUS_DOWN;
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      logical_port_operStatus = UPLL_OPER_STATUS_UNKNOWN;
      UPLL_LOG_DEBUG("Invalid Logical Port Id");
      return UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("Error in retrieving LogicalPortId data from Physical");
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    return UPLL_RC_ERR_GENERIC;
  }
  log_Port_CK->ResetWith(ipc_resp.ckv_data);
  DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
  val_logical_port_st *phy_logPort_St = static_cast<val_logical_port_st *>
                                         (GetVal(log_Port_CK));
  if (phy_logPort_St)
    switch (phy_logPort_St->oper_status) {
      case UPPL_CONTROLLER_OPER_DOWN:
        logical_port_operStatus = UPLL_OPER_STATUS_DOWN;
        break;
      case UPPL_CONTROLLER_OPER_UP:
        logical_port_operStatus = UPLL_OPER_STATUS_UP;
        break;
      case UPPL_CONTROLLER_OPER_WAITING_AUDIT:
      case UPPL_CONTROLLER_OPER_AUDITING:
        logical_port_operStatus = UPLL_OPER_STATUS_UNKNOWN;
        break;
      default:
        break;
  }
  if (log_Port_CK)
    delete log_Port_CK;
  return result_code;
}

bool VnodeChildMoMgr::EnqueOperStatusNotification(ConfigKeyVal *ikey,
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
  uint8_t *vif_name = NULL;

  switch (ktype) {
    case UNC_KT_VBR_IF:
      vtn_name = reinterpret_cast<key_vbr_if*>
                   (ikey->get_key())->vbr_key.vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vbr_if*>
                     (ikey->get_key())->vbr_key.vbridge_name;
      vif_name = (reinterpret_cast<key_vbr_if*>(ikey->get_key()))->if_name;
    break;
    case UNC_KT_VRT_IF:
      vtn_name = reinterpret_cast<key_vrt_if*>
                     (ikey->get_key())->vrt_key.vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vrt_if*>
                     (ikey->get_key())->vrt_key.vrouter_name;
      vif_name = (reinterpret_cast<key_vrt_if*>(ikey->get_key()))->if_name;
    break;
    case UNC_KT_VTUNNEL_IF:
      vtn_name = reinterpret_cast<key_vtunnel_if*>
                     (ikey->get_key())->vtunnel_key.vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vtunnel_if*>
                     (ikey->get_key())->vtunnel_key.vtunnel_name;
      vif_name = reinterpret_cast<key_vtunnel_if*>(ikey->get_key())->if_name;
    break;
    case UNC_KT_VTEP_IF:
      vtn_name = reinterpret_cast<key_vtep_if*>
                     (ikey->get_key())->vtep_key.vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vtep_if*>
                     (ikey->get_key())->vtep_key.vtep_name;
      vif_name = (reinterpret_cast<key_vtep_if*>(ikey->get_key()))->if_name;
    break;
    case UNC_KT_VTERM_IF:
      vtn_name = reinterpret_cast<key_vterm_if*>
                     (ikey->get_key())->vterm_key.vtn_key.vtn_name;
      vnode_name = reinterpret_cast<key_vterm_if*>
                     (ikey->get_key())->vterm_key.vterminal_name;
      vif_name = (reinterpret_cast<key_vrt_if*>(ikey->get_key()))->if_name;
    break;
    default:
       UPLL_LOG_DEBUG("Invalid KeyType");
  }
  return cfg_instance->SendOperStatusAlarm(reinterpret_cast<char*>(vtn_name),
                                           reinterpret_cast<char*>(vnode_name),
                                           reinterpret_cast<char*>(vif_name),
                                           oper_status_change);
}

upll_rc_t VnodeChildMoMgr::UpdateVnodeIfOperStatus(ConfigKeyVal *ck_vnif,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  string vtn_name = "";

  MoMgrTables tbl = MAINTBL;
  if (ck_vnif->get_st_num() == IpctSt::kIpcStKeyConvertVbrIf)
    tbl = CONVERTTBL;
  ConfigVal *tmp_ck_if = (ck_vnif->get_cfg_val()) ?
                    ck_vnif->get_cfg_val()->get_next_cfg_val() : NULL;
  val_db_vbr_if_st *vnif_st =
      reinterpret_cast<val_db_vbr_if_st*>
      ((tmp_ck_if != NULL) ? tmp_ck_if->get_val() : NULL);
  if (vnif_st == NULL) return UPLL_RC_ERR_GENERIC;
  uint32_t orig_flag  = vnif_st->down_count;
  /* get portmap and admin status details */
  uint8_t valid_pm = 0, valid_admin = 0, admin_status = 0;
  val_port_map *pm = NULL;
  result_code = GetPortMap(ck_vnif, valid_pm, pm, valid_admin, admin_status);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n", result_code);
    return result_code;
  }
  UPLL_LOG_DEBUG("valid_pm %d valid_admin %d admin_status %d",
                  valid_pm, valid_admin, admin_status);

  if (admin_status == UPLL_ADMIN_DISABLE) {
    /* set ADMIN_DISABLE flag */
    vnif_st->down_count |= ADMIN_DISABLE;
  }
  /* get the type of interface */
  if_type vnif_type;
  result_code = GetInterfaceType(ck_vnif, valid_pm, vnif_type);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetInterfaceType error %d\n", result_code);
    return result_code;
  }
  switch (vnif_type) {
  case kLinkedInterface:
  case kBoundaryInterface: {
    if (ck_vnif->get_key_type() == UNC_KT_VRT_IF) {
      return UPLL_RC_SUCCESS;
    }
    if (tbl == CONVERTTBL) {
      uint8_t user_flags = 0;
      GET_USER_DATA_FLAGS(ck_vnif, user_flags);
      if (!(user_flags & BOUNDARY_UVBRIF_FLAG)) {
        uint8_t state(UPLL_OPER_STATUS_UNKNOWN);
        result_code = GetGatewayPortStatus(ck_vnif, &state, dmi);
        vnif_st->down_count &= ~PORT_UNKNOWN;
        switch (state) {
          case  UPLL_OPER_STATUS_DOWN:
            vnif_st->down_count |= PORT_FAULT;
          break;
          case  UPLL_OPER_STATUS_UNKNOWN:
            return UPLL_RC_SUCCESS;
          break;
          default:
          break;
        }
        break;
      }
    }
    /* if interface is vlink, then updation of interface OperStatus
     * should be done based on vLink and remote interface */
    VlinkMoMgr *vlink_mgr =
             reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager *>
             (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK))));
    ConfigKeyVal *ck_vlink = NULL;
    ConfigKeyVal *ck_if[2] = {NULL, NULL};
    bool is_uninfied = false;
    ConfigKeyVal *orig_conv_if = ck_vnif;
    ConfigKeyVal *ck_vnif_tmp = NULL;
    if (tbl == CONVERTTBL) {
      result_code = GetUninfiedvBrIf(ck_vnif_tmp, ck_vnif, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Error in GetUninfiedvBrIf %d", result_code);
        DELETE_IF_NOT_NULL(ck_vnif_tmp);
        return result_code;
      }
      is_uninfied = true;
      ck_vnif = ck_vnif_tmp;
    }
    ck_if[0] = ck_vnif;
    result_code = vlink_mgr->GetRemoteIf(ck_vnif, ck_if[1],
                                         ck_vlink, dmi, UPLL_DT_STATE);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in getting remote interface");
      return result_code;
    }
    result_code = vlink_mgr->SetVlinkAndIfsOnReconnect(ck_vlink, ck_if,
                                                       dmi, is_uninfied);
    DELETE_IF_NOT_NULL(ck_if[1]);
    DELETE_IF_NOT_NULL(ck_vlink);
    if (is_uninfied) {
      ck_vnif = orig_conv_if;
      DELETE_IF_NOT_NULL(ck_vnif_tmp);
    }
    return result_code;
  break;
  }
  case kMappedInterface: {
    /* if notifn is kCommit, check the port against logical_port_map
     * and Set down_count accordingly */
    unc_key_type_t ktype = ck_vnif->get_key_type();
    if (ktype != UNC_KT_VTEP_IF && ktype != UNC_KT_VTUNNEL_IF) {
      controller_domain ctrlr_dom = {NULL, NULL};
      GET_USER_DATA_CTRLR_DOMAIN(ck_vnif, ctrlr_dom)
      uuc::CtrlrMgr* ctr_mgr = uuc::CtrlrMgr::GetInstance();
      bool result(false);
      uint8_t state(UPPL_LOGICAL_PORT_OPER_UNKNOWN);
      result = ctr_mgr->GetLogicalPortSt(
          reinterpret_cast<const char*>(ctrlr_dom.ctrlr),
          reinterpret_cast<const char*>(pm->logical_port_id),
          state);
      if (result == false) {
      /* port is already set to UNKNOWN. So return */
        return UPLL_RC_SUCCESS;
      }
      /* clear PORT_UNKNOWN flag */
      vnif_st->down_count &= ~PORT_UNKNOWN;
      if (state == UPPL_LOGICAL_PORT_OPER_DOWN) {
        /* Set PORT_FAULT flag */
        vnif_st->down_count |= PORT_FAULT;
      }
    } else {
      vnif_st->down_count &= ~PORT_UNKNOWN;
    }
  }
  break;
  case kUnboundInterface: {
    unc_key_type_t ktype = ck_vnif->get_key_type();
    if (ktype != UNC_KT_VTEP_IF && ktype != UNC_KT_VTUNNEL_IF) {
      if (vnif_st->down_count & PORT_FAULT) {
        return UPLL_RC_SUCCESS;
      }
      /* For kCommit, clear PORT_UNKNOWN flag and set PORT_FAULT flag */
      vnif_st->down_count &= ~PORT_UNKNOWN;
      vnif_st->down_count |= PORT_FAULT;
    } else {
       vnif_st->down_count &= ~PORT_UNKNOWN;
    }
  }
  break;
  default:
    return UPLL_RC_ERR_GENERIC;
  }
  if (vnif_st->down_count == orig_flag) {
    UPLL_LOG_DEBUG("No change in status after reconnect");
    return UPLL_RC_SUCCESS;
  }
  vnif_st->vbr_if_val_st.valid[0] = UNC_VF_VALID;
  if (vnif_st->down_count == PORT_UP) {
    /* if down_count is PORT_UP, set operStatus to UP */
    vnif_st->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_UP;
  } else if (vnif_st->down_count & PORT_UNKNOWN) {
    /* if down_count is UNKNOWN, set operStatus to UNKNOWN */
    vnif_st->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
  } else {
    /* if down_count is DWON, set operStatus to DOWN */
    vnif_st->vbr_if_val_st.oper_status = UPLL_OPER_STATUS_DOWN;
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  return (UpdateConfigDB(ck_vnif, UPLL_DT_STATE, UNC_OP_UPDATE,
                         dmi, &dbop, TC_CONFIG_GLOBAL,
                         vtn_name, tbl));
}

upll_rc_t VnodeChildMoMgr::RecomputeVlinkAndIfoperStatus(ConfigKeyVal *ck_vlink,
                             ConfigKeyVal *ck_rem_if, DalDmlIntf *dmi,
                             state_notification notfn) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (ck_rem_if->get_key_type() != UNC_KT_VUNK_IF) {
    val_db_vbr_if_st *vnif_st =
        reinterpret_cast<val_db_vbr_if_st*>(GetStateVal(ck_rem_if));
    uint32_t orig_status = vnif_st->down_count;
    if (notfn == kPortUnknown || notfn == kCtrlrDisconnect) {
      if (vnif_st->down_count & REMOTE_DOWN) {
        vnif_st->down_count &= ~REMOTE_DOWN;
      }
      if (vnif_st->down_count & PORT_FAULT) {
        uint8_t valid_pm = 0, valid_admin = 0, admin_status = 0;
        val_port_map_t *pm = NULL;
        result_code = GetPortMap(ck_rem_if, valid_pm, pm,
                                 valid_admin, admin_status);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d\n", result_code);
          return result_code;
        }
        if (pm != NULL) {
          controller_domain_t ctrlr_dom = {NULL, NULL};
          val_oper_status port_oper_status = UPLL_OPER_STATUS_DOWN;
          GET_USER_DATA_CTRLR_DOMAIN(ck_rem_if, ctrlr_dom);
          result_code = GetPortStatusFromPhysical(pm, ctrlr_dom,
                                                  port_oper_status);
          if (port_oper_status == UPLL_OPER_STATUS_UP) {
            vnif_st->down_count &= ~PORT_FAULT;
          }
        } else {
          val_vlink *vlink_val = reinterpret_cast<val_vlink *>
                                 (GetVal(ck_vlink));
          if (vlink_val == NULL) return UPLL_RC_ERR_GENERIC;
          if (vlink_val->valid[UPLL_IDX_BOUNDARY_NAME_VLNK] == UNC_VF_VALID)
            vnif_st->down_count &= ~PORT_FAULT;
        }
      }
      vnif_st->down_count &= ~REMOTE_PATH_FAULT;
    } else if (notfn == kPathFault) {
      vnif_st->down_count |= REMOTE_PATH_FAULT;
    } else if (notfn == kPathFaultReset) {
      vnif_st->down_count &= ~REMOTE_PATH_FAULT;
    } else if (notfn == kVtnExhaustion) {
      vnif_st->down_count |= REMOTE_VTN_EXHAUSTION;
    } else {
      vnif_st->down_count &= ~REMOTE_VTN_EXHAUSTION;
    }
    if (notfn == kPortUnknown) {
      val_db_vlink_st *vl_st = reinterpret_cast<val_db_vlink_st*>
                            (GetStateVal(ck_vlink));
      vl_st->down_count &= ~PORT_FAULT;
      vl_st->down_count |= PORT_UNKNOWN;
      vl_st->vlink_val_st.oper_status = UPLL_OPER_STATUS_UNKNOWN;
      DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
      VlinkMoMgr *vlink_momgr = reinterpret_cast<VlinkMoMgr *>(
          const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
      result_code = vlink_momgr->UpdateConfigDB(ck_vlink, UPLL_DT_STATE,
                                                UNC_OP_UPDATE, dmi, &dbop,
                                                TC_CONFIG_GLOBAL, "", MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Error in UpdateConfigDB : %d", result_code);
        return result_code;
      }
    }
    if (vnif_st->down_count != orig_status) {
      result_code = UpdateOperStatus(ck_rem_if, dmi, kCommit,
                                     UNC_OP_UPDATE, true, true);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d\n", result_code);
        return result_code;
      }
    }
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetGatewayPortStatus(ConfigKeyVal *ck_vnif,
                    uint8_t *oper_status, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vtn *gateway_key = ConfigKeyVal::Malloc<key_vtn>();
  ConfigKeyVal *gw_ckv = new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
      gateway_key, NULL);

  // set controller and domain
  controller_domain_t ctrlr_dom  = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(ck_vnif, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(gw_ckv, ctrlr_dom);

  VtnMoMgr *vtn_mgr = reinterpret_cast<VtnMoMgr *>(const_cast<MoManager*>
                                                  (GetMoManager(UNC_KT_VTN)));
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                  kOpInOutNone };
  result_code = vtn_mgr->ReadConfigDB(gw_ckv, UPLL_DT_STATE, UNC_OP_READ,
                                      dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      *oper_status = UPLL_OPER_STATUS_DOWN;
      result_code = UPLL_RC_SUCCESS;
    }
    UPLL_LOG_DEBUG("Error in reading gateway operstatus: %d", result_code);
    DELETE_IF_NOT_NULL(gw_ckv);
    return result_code;
  }
  val_db_vtn_st_t *valst =
      reinterpret_cast<val_db_vtn_st *>(GetStateVal(gw_ckv));
  if (!valst) {
    UPLL_LOG_DEBUG("Returning error\n");
    DELETE_IF_NOT_NULL(gw_ckv);
    return UPLL_RC_ERR_GENERIC;
  }
  *oper_status = valst->vtn_val_st.oper_status;
  DELETE_IF_NOT_NULL(gw_ckv);
  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetConvertedIfStatus(ConfigKeyVal *ck_vnif,
                    uint8_t *oper_status, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_conv_vbrif = NULL;
  result_code = GetChildConvertConfigKey(ckv_conv_vbrif, NULL);
  key_convert_vbr_if *conv_vbr_if_key = reinterpret_cast<key_convert_vbr_if*>
                                   (ckv_conv_vbrif->get_key());
  key_vbr_if *vbr_if_key = reinterpret_cast<key_vbr_if*>
                                   (ck_vnif->get_key());
  uuu::upll_strncpy(conv_vbr_if_key->convert_if_name,
                    vbr_if_key->if_name, (kMaxLenInterfaceName + 1));
  uuu::upll_strncpy(conv_vbr_if_key->convert_vbr_key.vbr_key.vbridge_name,
                    vbr_if_key->vbr_key.vbridge_name, (kMaxLenVnodeName + 1));
  uuu::upll_strncpy(conv_vbr_if_key->convert_vbr_key.vbr_key.vtn_key.vtn_name,
                    vbr_if_key->vbr_key.vtn_key.vtn_name, (kMaxLenVtnName + 1));

  VbrIfMoMgr *if_mgr = reinterpret_cast<VbrIfMoMgr *>(const_cast<MoManager*>
                                                (GetMoManager(UNC_KT_VBR_IF)));
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = if_mgr->ReadConfigDB(ckv_conv_vbrif, UPLL_DT_STATE, UNC_OP_READ,
                                     dbop, dmi, CONVERTTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in reading gateway operstatus: %d", result_code);
    DELETE_IF_NOT_NULL(ckv_conv_vbrif);
    return result_code;
  }
  val_db_vbr_if_st_t *valst =
      reinterpret_cast<val_db_vbr_if_st_t*>(GetStateVal(ckv_conv_vbrif));
  if (!valst) {
    UPLL_LOG_DEBUG("Returning error\n");
    DELETE_IF_NOT_NULL(ckv_conv_vbrif);
    return UPLL_RC_ERR_GENERIC;
  }
  *oper_status = valst->vbr_if_val_st.oper_status;
  DELETE_IF_NOT_NULL(ckv_conv_vbrif);
  return result_code;
}

upll_rc_t VnodeChildMoMgr::GetUninfiedvBrIf(ConfigKeyVal *&ck_vnif_tmp,
                                     ConfigKeyVal *ck_vnif, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetChildConfigKey(ck_vnif_tmp, ck_vnif);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Error in GetChildConfigKey");
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ck_vnif_tmp, UPLL_DT_STATE, UNC_OP_READ,
                                      dbop, dmi, MAINTBL);
  return result_code;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
