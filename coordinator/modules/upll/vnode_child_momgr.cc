/*
 * Copyright (c) 2012-2014 NEC Corporation
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
#include "vlanmap_momgr.hh"

#define NO_VLINK_FLAG 0x03 

namespace unc {
namespace upll {
namespace kt_momgr {

upll_rc_t VnodeChildMoMgr::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal *pckv = NULL;
  uint8_t *controller_id = reinterpret_cast<uint8_t *>(
                                 const_cast<char *>(ctrlr_id));

  /* check if object is renamed in the corresponding Rename Tbl
   * if "renamed"  create the object by the UNC name.
   * else - create using the controller name.
   */
  result_code = GetRenamedUncKey(ikey, UPLL_DT_RUNNING, dmi, controller_id);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("GetRenamedUncKey Failed err_code %d", result_code);
    return result_code;
  }
  if ((ikey->get_key_type() == UNC_KT_VTEP_GRP) ||
      (ikey->get_key_type() == UNC_KT_VTEP_GRP_MEMBER)) {
    result_code = GetControllerDomainId(ikey, UPLL_DT_AUDIT,
                                       &ctrlr_dom, dmi);
    ctrlr_dom.ctrlr = NULL;
    ctrlr_dom.domain = NULL;
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
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
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutFlag | kOpInOutDomain
                       | kOpInOutCtrlr };
  // SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE,
                               dmi, &dbop, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("UpdateConfigDB failed for creating vnode child err_code %d",
                   result_code);
  }
  DELETE_IF_NOT_NULL(pckv);
  return result_code;
}

upll_rc_t VnodeChildMoMgr::CreateCandidateMo(IpcReqRespHeader *req,
                                             ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             bool restore_flag) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!restore_flag) {
    result_code = ValidateMessage(req, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n", result_code);
      return result_code;
    }
  }

  if (parent_ck_vnode) {
    UPLL_LOG_DEBUG("parent_ck_vnode has value \n");
    delete parent_ck_vnode;
    parent_ck_vnode = NULL;
  }

  result_code = GetParentConfigKey(parent_ck_vnode, ikey);
  if (result_code != UPLL_RC_SUCCESS || parent_ck_vnode == NULL) {
    return result_code;
  }

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
  ConfigKeyVal *dup_ikey = NULL;
  if (UPLL_DT_IMPORT != req->datatype && !restore_flag) {
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
                 (ikey->get_key())->logical_port_id_valid == PFC_TRUE)) ||
                  restore_flag) {
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
             dup_ikey->AppendCfgVal(IpctSt::kIpcStPfcdrvValVlanMap, val_vlanmap);
           }
         }
      } else {
      dup_ikey = ikey;
      }
    } else {
      dup_ikey = ikey;
    }
    UPLL_LOG_DEBUG("%s \n",dup_ikey->ToStrAll().c_str());

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

  if (!restore_flag) {
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

      /* result_code is SUCCESS if vlan-map is already
       * configured via boundary updates flag */
      if (result_code == UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("%s \n",dup_ikey->ToStrAll().c_str());
        UPLL_LOG_TRACE("Vlanmap already configured")

        uint8_t vlanmap_flag = 0;
        GET_USER_DATA_FLAGS(dup_ikey, vlanmap_flag)

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

        /* vlanmap create request from user when its
         * already created by user*/
        if ((ival->vlan_id == val_vlanmap->vm.vlan_id) &&
            (vlanmap_flag & USER_VLANMAP_FLAG)) {
          UPLL_LOG_ERROR("Vlanmap already with the same key exists");
          delete dup_ikey;
          DELETE_IF_NOT_NULL(parent_ck_vnode);
          return UPLL_RC_ERR_INSTANCE_EXISTS;
        }

        val_vlanmap->vm.vlan_id  = ival->vlan_id;
        val_vlanmap->vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;

        /* Update the flag if vlanmap is user configured*/
        vlanmap_flag |= flags;
        SET_USER_DATA_FLAGS(dup_ikey, vlanmap_flag);

        DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutFlag };
        result_code = UpdateConfigDB(dup_ikey, req->datatype, UNC_OP_UPDATE,
                                     dmi, &dbop1, MAINTBL);
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

      UPLL_LOG_DEBUG("%s \n",dup_ikey->ToStrAll().c_str());
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
  }
  result_code = ValidateCapability(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("ValidateCapability failed");
    DELETE_IF_NOT_NULL(parent_ck_vnode);
    if (UPLL_DT_IMPORT != req->datatype && !restore_flag) {
        void *ifval = GetVal(ikey);
      if (((ikey->get_key_type() == UNC_KT_VBR_IF) ||
         (ikey->get_key_type() == UNC_KT_VBR_VLANMAP)) && ifval) {
        DELETE_IF_NOT_NULL(dup_ikey);
      }
    }
    return result_code;
  }
  SET_USER_DATA_CTRLR_DOMAIN(dup_ikey, cntrl_dom);
  result_code = ValidateAttribute(dup_ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("ValidateAttribute semantic check returns error %d",
                   result_code);
    DELETE_IF_NOT_NULL(parent_ck_vnode);
    if (UPLL_DT_IMPORT != req->datatype && !restore_flag) {
        void *ifval = GetVal(ikey);
      if (((ikey->get_key_type() == UNC_KT_VBR_IF) ||
         (ikey->get_key_type() == UNC_KT_VBR_VLANMAP)) && ifval) {
        DELETE_IF_NOT_NULL(dup_ikey);
      }
    }
    return result_code;
  }
  if (restore_flag) {
    /* Reset the vlink related bits in rename flag */
    unc_key_type_t kt_type = dup_ikey->get_key_type();
    if(IS_INTERFACE_KEY(kt_type)) {
      uint8_t rename = 0;
      GET_USER_DATA_FLAGS(dup_ikey, rename);
      /* In case of interfaces (having portmaps) and is part of
       * boundary vlink, then do not restore port_map info.
       * It will be restored during vlink restore */
      if (kt_type != UNC_KT_VRT_IF && kt_type != UNC_KT_VUNK_IF) {
      /* VRT_IF and VUNK_IF do not have portmaps */
        if_type vnif_type = kUnboundInterface;
        upll_rc_t rcode = UPLL_RC_SUCCESS;
        /* Get the interface type */
        rcode = GetInterfaceType(dup_ikey, UNC_VF_INVALID, vnif_type);
        if (rcode != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetInterfaceType error %d\n",rcode);
          DELETE_IF_NOT_NULL(parent_ck_vnode);
          if (UPLL_DT_IMPORT != req->datatype) {
            void *ifval = GetVal(ikey);
            /* VlanmapOnBoundary: Added vlan-map check */
            if (((ikey->get_key_type() == UNC_KT_VBR_IF) ||
              (ikey->get_key_type() == UNC_KT_VBR_VLANMAP)) && ifval) {
              DELETE_IF_NOT_NULL(dup_ikey);
            }
          }
          return rcode;
        }
        /* If boundarymapped, then keep valid_portmap as INVALID */
        if (kBoundaryInterface == vnif_type) {
          if (kt_type == UNC_KT_VBR_IF) {
            /* In bdry vbr_if, set all portmap flags to INVALID */
            val_drv_vbr_if *val_vbrif = reinterpret_cast<val_drv_vbr_if*>
              (GetVal(dup_ikey));
            val_vbrif->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_INVALID;
            val_vbrif->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_INVALID;
            val_vbrif->valid[PFCDRV_IDX_VLINK_NAME_VBRIF] = UNC_VF_INVALID;
            val_vbrif->vbr_if_val.valid[UPLL_IDX_PM_VBRI] = UNC_VF_INVALID;
          } else if (kt_type == UNC_KT_VTEP_IF) {
            /* Handle vtep if */
            val_vtep_if *val_vtepif = reinterpret_cast<val_vtep_if*>
              (GetVal(dup_ikey));
            val_vtepif->valid[UPLL_IDX_PORT_MAP_VTEPI] = UNC_VF_INVALID;
          } else if (kt_type == UNC_KT_VTUNNEL_IF) {
            /* Handle vtunnel if */
            val_vtunnel_if *val_vtun_if = reinterpret_cast<val_vtunnel_if*>
              (GetVal(dup_ikey));
            val_vtun_if->valid[UPLL_IDX_PORT_MAP_VTNL_IF] = UNC_VF_INVALID;
          } else if (kt_type ==UNC_KT_VTERM_IF) {
            val_vterm_if *valvterm_if = reinterpret_cast<val_vterm_if*>
                  (GetVal(dup_ikey));
            valvterm_if->valid[UPLL_IDX_PM_VTERMI] = UNC_VF_INVALID;
          } else {
             UPLL_LOG_DEBUG("Invalid vnode child keytype");
             DELETE_IF_NOT_NULL(parent_ck_vnode);
             if (UPLL_DT_IMPORT != req->datatype) {
               void *ifval = GetVal(ikey);
               /* VlanmapOnBoundary: Added vlan-map check */
               if (((ikey->get_key_type() == UNC_KT_VBR_IF) ||
                 (ikey->get_key_type() == UNC_KT_VBR_VLANMAP)) && ifval) {
                 DELETE_IF_NOT_NULL(dup_ikey);
               }
             }
             return UPLL_RC_ERR_GENERIC;
          }
          UPLL_LOG_DEBUG("Restoring interface which is part of boundary"
            " \n %s \n",dup_ikey->ToStrAll().c_str());
        } // kBoundaryInterface
      }
      if (rename)
        rename&=NO_VLINK_FLAG;
      SET_USER_DATA_FLAGS(dup_ikey, rename);
    }
    result_code = UpdateConfigDB(dup_ikey, req->datatype, UNC_OP_CREATE, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
       UPLL_LOG_INFO("Failed to create an entry in Candidate DB %d", result_code);
    }
    DELETE_IF_NOT_NULL(parent_ck_vnode);
    /*
     * Restore case ikey and dup_ikey are same
     */
#if 0
    if (UPLL_DT_IMPORT != req->datatype) {
       void *ifval = GetVal(ikey);
      /* VlanmapOnBoundary: Added vlan-map check */
      if (((ikey->get_key_type() == UNC_KT_VBR_IF) ||
         (ikey->get_key_type() == UNC_KT_VBR_VLANMAP)) && ifval) {
        DELETE_IF_NOT_NULL(dup_ikey);
      }
    }
#endif
   return result_code;
  }

  result_code = RestoreVnode(dup_ikey, req, dmi, restore_flag);
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
                                        IpcReqRespHeader *req,
                                        DalDmlIntf *dmi,
                                        bool restore_flag) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *dup_ikey = NULL;
  upll_keytype_datatype_t dt_type = req->datatype;

  if (!restore_flag) {
    if (UPLL_DT_CANDIDATE == req->datatype) {
      result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ, dmi,
				   MAINTBL);
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
	UPLL_LOG_DEBUG("Key instance exist");
	if ((ikey)->get_cfg_val()) {
	  UPLL_LOG_DEBUG("Read Key with Value struct");
          switch (ikey->get_key_type()) { 
          case UNC_KT_VRT_IF: {
	    val_vrt_if *vrt_if_val = reinterpret_cast<val_vrt_if *>
                                                 (GetVal(ikey));
	    if (vrt_if_val != NULL) {
              vrt_if_val->valid[UPLL_IDX_ADMIN_ST_VI] = UNC_VF_INVALID;
	    }
          }
            break;
          case UNC_KT_VBR_IF: {
	    val_drv_vbr_if *vbr_if_val = reinterpret_cast<val_drv_vbr_if *>
                                                 (GetVal(ikey));
	    if (vbr_if_val != NULL) {
              vbr_if_val->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] = UNC_VF_INVALID;
	    }
          }
            break;
          case UNC_KT_VTERM_IF: {
            val_vterm_if *vtermif_val = reinterpret_cast<val_vterm_if *>
                (GetVal(ikey));
            if (vtermif_val != NULL) {
              vtermif_val->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
            }
          }
            break;
          case UNC_KT_VBR_NWMONITOR: {
            val_nwm_t *nwm_val = reinterpret_cast<val_nwm_t *>
                                             (GetVal(ikey));
            if (nwm_val != NULL) {
              nwm_val->valid[UPLL_IDX_ADMIN_STATUS_NWM] = UNC_VF_INVALID;
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
	  result_code = RestoreChildren(ikey, dt_type, UPLL_DT_RUNNING, dmi, req);
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

  if (!restore_flag) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE
	      || UPLL_DT_IMPORT == req->datatype) {
      UPLL_LOG_TRACE("Given Create Record doesn't exists in Running");
      result_code = DupConfigKeyVal(dup_ikey, ikey);
      if (result_code != UPLL_RC_SUCCESS || dup_ikey == NULL) {
	UPLL_LOG_DEBUG("Returning %d", result_code);
	return result_code;
      }
    } else {
  //    std::cout << "Problem in reading RUNNING DB";
      return result_code;
    }
    
    uint8_t flag = 0;
    unc_key_type_t ktype = parent_ck_vnode->get_key_type();
    if (UPLL_DT_CANDIDATE == req->datatype) {
      if (ktype == UNC_KT_VBRIDGE || ktype == UNC_KT_VROUTER || UNC_KT_VTERMINAL) {
        GET_USER_DATA_FLAGS(dup_ikey, flag);
	result_code = SetRenameField(dup_ikey);
	if (result_code != UPLL_RC_SUCCESS) {
	  UPLL_LOG_INFO("Problem in setting rename field");
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
      return result_code;
    }
  }
      }

      /* VlanmapOnBoundary: Added vlan-map check */
      if (dup_ikey->get_key_type() == UNC_KT_VBR_VLANMAP) {
        uint8_t dup_flag = 0;
        GET_USER_DATA_FLAGS(dup_ikey, dup_flag);
        flag |= dup_flag;
        SET_USER_DATA_FLAGS(dup_ikey, flag);
        UPLL_LOG_TRACE("FLAG RESTORE = %u", flag);
      }
    }
  }
  result_code = UpdateConfigDB(dup_ikey, dt_type, UNC_OP_CREATE, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to create an entry in Candidate DB");
  }
  if (dup_ikey) delete dup_ikey;
  return result_code;
}


upll_rc_t VnodeChildMoMgr::IsReferenced(ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t dt_type,
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
  result_code = ReadConfigDB(okey, dt_type, UNC_OP_READ, dbop,
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

upll_rc_t VnodeChildMoMgr::SetRenameField(ConfigKeyVal *&ikey) {
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
  } else {
       result_code = mgr->GetRenamedControllerKey(ck_parent, dt_type, dmi,
                                                  ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed. Result: %d", result_code);
      DELETE_IF_NOT_NULL(ck_parent);
      return result_code;
    }
  }
  if (result_code == UPLL_RC_SUCCESS) {
    result_code  = GetChildConfigKey(ikey, ck_parent);
    if (UPLL_RC_SUCCESS != result_code) {
       DELETE_IF_NOT_NULL (ck_parent);
       UPLL_LOG_DEBUG("GetChildConfigkey Failed");
       return result_code;
    }
    SET_USER_DATA(ikey, ck_parent)
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

template<typename T1, typename T2>
upll_rc_t VnodeChildMoMgr::InitOperStatus(ConfigKeyVal *ikey,
                                          uint8_t valid_admin,
                                          uint8_t admin_status,
                                          uint8_t valid_pm,
                                          val_port_map_t *pm) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  T2 *vnif_db_st = reinterpret_cast<T2 *>(GetStateVal(ikey));
  if (!vnif_db_st) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
  }
  T1 *vnif_st = reinterpret_cast<T1 *>(vnif_db_st);
  // vnif_db_st->down_count = 0;
#if 0
  if ((valid_admin == UNC_VF_VALID) ||
     (valid_admin == UNC_VF_VALID_NO_VALUE)) 
#endif
    if (vnif_st->oper_status != UPLL_OPER_STATUS_UNINIT) {
      if (admin_status == UPLL_ADMIN_DISABLE) {
      vnif_st->oper_status = UPLL_OPER_STATUS_DOWN;
      } else if (admin_status == UPLL_ADMIN_ENABLE) {
        if_type vnif_type = kUnboundInterface;
        result_code = GetInterfaceType(ikey, valid_pm, vnif_type);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetInterfaceType error %d\n",result_code);
          return result_code;
        }

        UPLL_LOG_DEBUG("ktype %d vnif_type %d valid_pm %d\n", 
                    ikey->get_key_type(),vnif_type, valid_pm);
        switch (vnif_type) {
         case kBoundaryInterface:
         case kMappedInterface:
           if (valid_pm != UNC_VF_VALID)
              vnif_st->oper_status = UPLL_OPER_STATUS_DOWN;
           else
              vnif_st->oper_status = UPLL_OPER_STATUS_UNINIT;
           break;
         case kLinkedInterface:
           vnif_st->oper_status = UPLL_OPER_STATUS_UP;
           break;
         case kUnboundInterface:
           vnif_st->oper_status = UPLL_OPER_STATUS_DOWN;
           break;
        }
      }  else {
        vnif_st->oper_status = UPLL_OPER_STATUS_UNKNOWN;
      }
  }
  vnif_st->valid[UPLL_IDX_OPER_STATUS_VRTS] = UNC_VF_VALID;
  return UPLL_RC_SUCCESS;
}

template upll_rc_t VnodeChildMoMgr::InitOperStatus<val_vrt_if_st,
                                          val_db_vrt_if_st>(
                                          ConfigKeyVal *ikey,
                                          uint8_t valid_admin,
                                          uint8_t admin_status,
                                          uint8_t valid_pm,
                                          val_port_map_t *pm);
template upll_rc_t VnodeChildMoMgr::InitOperStatus<val_vtep_if_st,
                                          val_db_vtep_if_st>(
                                          ConfigKeyVal *ikey,
                                          uint8_t valid_admin,
                                          uint8_t admin_status,
                                          uint8_t valid_pm,
                                          val_port_map_t *pm);
template upll_rc_t VnodeChildMoMgr::InitOperStatus<val_vtunnel_if_st,
                                          val_db_vtunnel_if_st>(
                                          ConfigKeyVal *ikey,
                                          uint8_t valid_admin,
                                          uint8_t admin_status,
                                          uint8_t valid_pm,
                                          val_port_map_t *pm);
template upll_rc_t VnodeChildMoMgr::InitOperStatus<val_vbr_if_st,
                                          val_db_vbr_if_st>(
                                          ConfigKeyVal *ikey,
                                          uint8_t valid_admin,
                                          uint8_t admin_status,
                                          uint8_t valid_pm,
                                          val_port_map_t *pm);

upll_rc_t VnodeChildMoMgr::GetInterfaceType(ConfigKeyVal *ck_vnif,
                                            uint8_t valid_pm,
                                            if_type &vnif_type ) {
  UPLL_FUNC_TRACE;
  if (!ck_vnif) {
    UPLL_LOG_DEBUG("Invalid param");
    return UPLL_RC_ERR_GENERIC;
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
  UPLL_LOG_DEBUG("flags %d vnif_type %d", flags, vnif_type);
  return UPLL_RC_SUCCESS;
}

upll_rc_t VnodeChildMoMgr::UpdateParentOperStatus(ConfigKeyVal *ikey, 
                                                  DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_ERROR("Returning error \n");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_key_type_t ktype = ikey->get_key_type();
  if ((ktype != UNC_KT_VBR_IF) && (ktype != UNC_KT_VRT_IF) &&
      (ktype != UNC_KT_VTERM_IF))
    return UPLL_RC_SUCCESS;
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
  // value not bound during delete - read to get down count / oper status
  result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ,
                                  dbop, dmi, MAINTBL); 
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d\n",result_code);
    return result_code;
  }
  val_db_vbr_if_st *valst = reinterpret_cast<val_db_vbr_if_st *>(GetStateVal(ikey)); 
  if (!valst) {
    UPLL_LOG_DEBUG("Returning error\n");
    return UPLL_RC_ERR_GENERIC;
  }
  if (valst->vbr_if_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] == UNC_VF_VALID) {
    bool last_if = false;
    state_notification notification = kAdminStatusEnabled;
    ConfigKeyVal *ck_vn = NULL;
    result_code = GetParentConfigKey(ck_vn,ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      return result_code;
    }
    if (valst->vbr_if_val_st.oper_status !=  UPLL_OPER_STATUS_DOWN) {
      /* check if it is last interface */
      uint32_t count = 0;    
      uint8_t tmp_ifname[kMaxLenInterfaceName + 1] ;
      key_vbr_if *vbrif_key= reinterpret_cast<key_vbr_if *>(ikey->get_key());
      uuu::upll_strncpy(tmp_ifname,vbrif_key->if_name, kMaxLenInterfaceName + 1);
      vbrif_key->if_name[0] = '\0';
      result_code = GetInstanceCount(ikey, NULL, UPLL_DT_RUNNING, &count, dmi,
				   MAINTBL);
      uuu::upll_strncpy(vbrif_key->if_name,tmp_ifname, kMaxLenInterfaceName + 1); 
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d\n", result_code);
        DELETE_IF_NOT_NULL(ck_vn);
        return UPLL_RC_ERR_GENERIC;
      }
      if (count == 1) {
        last_if = true; 
        if (ikey->get_key_type() == UNC_KT_VBR_IF) {
          VlanMapMoMgr *vlan_mgr =
          reinterpret_cast<VlanMapMoMgr *>(const_cast<MoManager *>
          (const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_VLANMAP))));
          result_code = vlan_mgr->CheckIfVnodeisVlanmapped(ck_vn, dmi);
          if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
            notification = kAdminStatusEnabled;
          } else 
            notification = kAdminStatusDisabled;
        }
      }
    }
    VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
                            (GetMoManager(ck_vn->get_key_type())));
    if (!mgr) {
      UPLL_LOG_DEBUG("Returning error \n");
      delete ck_vn;
      return UPLL_RC_ERR_GENERIC;
    }
#if 0
    result_code = mgr->ReadConfigDB(ck_vn, UPLL_DT_STATE, UNC_OP_READ,
                                    dbop, dmi, MAINTBL); 
    if (result_code != UPLL_RC_SUCCESS) {
      if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      } else
        result_code = UPLL_RC_SUCCESS;
      DELETE_IF_NOT_NULL(ck_vn);
      return result_code;
    }
    val_db_vbr_if_st *vn_valst = reinterpret_cast<val_db_vbr_if_st *>
                             GetStateVal(ck_vn);
    if (valst->down_count > 0) { 
      vn_valst->down_count--;
    }
    result_code = mgr->GetCkvUninit(ck_vn,NULL,dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      DELETE_IF_NOT_NULL(ck_vn);
      return result_code;
    }
    result_code = mgr->UpdateConfigDB(ck_vn, UPLL_DT_STATE, UNC_OP_UPDATE,
                                 dmi, MAINTBL);
#else
    switch (valst->vbr_if_val_st.oper_status) {
    case UPLL_OPER_STATUS_DOWN:
      notification = (valst->down_count > 0)?kPortFaultReset:kAdminStatusEnabled;
      break;
    case UPLL_OPER_STATUS_UP:
      if (!last_if) {
        DELETE_IF_NOT_NULL(ck_vn);
        return UPLL_RC_SUCCESS;
      }
      break;
    case UPLL_OPER_STATUS_UNKNOWN:
    default:
      if (!last_if) {
        DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                         kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain };
        result_code = mgr->ReadConfigDB(ck_vn, UPLL_DT_STATE,
                            UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("'Returning error %d\n",result_code);
          } else {
            result_code = UPLL_RC_SUCCESS;
          }
          DELETE_IF_NOT_NULL(ck_vn);
          return result_code;
        }
        val_db_vbr_st *vn_stval = reinterpret_cast<val_db_vbr_st *>
                                  (GetStateVal(ck_vn));
        vn_stval->vbr_val_st.oper_status = UPLL_OPER_STATUS_UNINIT;
        vn_stval->vbr_val_st.valid[UPLL_IDX_OPER_STATUS_VBRS] = UNC_VF_VALID;
        result_code = mgr->UpdateConfigDB(ck_vn, UPLL_DT_STATE,
                                          UNC_OP_UPDATE, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d\n", result_code);
        }
        DELETE_IF_NOT_NULL(ck_vn);
        return result_code;
      }
    } 
    result_code = mgr->UpdateOperStatus(ck_vn, dmi, notification, false, true);
#endif
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                     UPLL_RC_SUCCESS:result_code;
    }
    DELETE_IF_NOT_NULL(ck_vn);
  }
  return result_code; 
}

upll_rc_t VnodeChildMoMgr::UpdateOperStatus(ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi,
                                       state_notification notification,
                                       bool skip, bool upd_if, bool upd_remif,
                                       bool save_to_db) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!skip) {
    DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr | kOpMatchDomain,
                     kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag };
    if (notification == kCtrlrDisconnect || notification == kCtrlrReconnect) {
      dbop.matchop = kOpMatchCtrlr;
      dbop.inoutop = kOpInOutFlag;
    }
    result_code = ReadConfigDB(ikey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                               MAINTBL);

    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in reading: %d", result_code);
      return result_code;
    }
  }
  ConfigKeyVal *tkey = ikey;
  unc_key_type_t ktype  = ikey->get_key_type(),pktype = UNC_KT_ROOT;
  while (tkey != NULL) {
   /* upd_if is set to true ->if operstatus
    * is updated as part of vlink processing.
    */
    if (!upd_if) {
      if_type vnif_type ; 
      result_code = GetInterfaceType(tkey,UNC_VF_INVALID,vnif_type);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d\n",result_code);
        return result_code;
      }
      if ((vnif_type == kBoundaryInterface) ||
          (vnif_type == kLinkedInterface)) {
        tkey = tkey->get_next_cfg_key_val();
        continue;
      } 
    }
    if (notification == kPortFaultReset) {
      val_db_vbr_if_st *vbr_if_st = reinterpret_cast<val_db_vbr_if_st *>
                                (GetStateVal(tkey));
      if (!vbr_if_st) {
        UPLL_LOG_TRACE("Returning NULL\n");
        return UPLL_RC_ERR_GENERIC;
      }
      /* if down count == 0, change notification to admin enabled
       * so that downcount does not get decremented. */
      if (vbr_if_st->down_count == 0)
        notification = kAdminStatusEnabled;

      uint8_t valid_pm,valid_admin,admin_status;
      val_port_map_t *pm = NULL;
      result_code = GetPortMap(ikey, valid_pm, pm, valid_admin, admin_status);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in GetPortMap %d", result_code);
        return result_code;
      }
      if (admin_status == UPLL_ADMIN_DISABLE)
        notification = (notification == kAdminStatusEnabled)?
                        kAdminStatusDisabled:
                        kPortFaultResetWithAdminDisabled; 
    }
    alarm_status oper_status_change = ALARM_NOT_SET;
    switch (ktype) {
    case UNC_KT_VBR_IF:
      pktype = UNC_KT_VBRIDGE;
      result_code = SetOperStatus
         <val_vbr_if_st, val_db_vbr_if_st>(tkey, notification, dmi,
                                           oper_status_change);
      if (tkey) {
        val_drv_vbr_if *vbr_if =
           reinterpret_cast<val_drv_vbr_if *>(GetVal(tkey));
        if (vbr_if)
         vbr_if->vbr_if_val.valid[UPLL_IDX_ADMIN_STATUS_VBRI] =
           UNC_VF_INVALID;
      }
      break;
    case UNC_KT_VRT_IF:
      pktype = UNC_KT_VROUTER;
      result_code = SetOperStatus
         <val_vrt_if_st, val_db_vrt_if_st>(tkey, notification, dmi,
                                           oper_status_change);
      if (tkey) {
        val_vrt_if *vrt_if =
           reinterpret_cast<val_vrt_if *>(GetVal(tkey));
        if (vrt_if)
         vrt_if->valid[UPLL_IDX_ADMIN_ST_VI] = UNC_VF_INVALID;
      }
      break;
    case UNC_KT_VTEP_IF:
      pktype = UNC_KT_VTEP;
      result_code = SetOperStatus
         <val_vtep_if_st, val_db_vtep_if_st>(tkey, notification, dmi,
                                             oper_status_change);
      if (tkey) {
        val_vtep_if *vtep_if =
           reinterpret_cast<val_vtep_if *>(GetVal(tkey));
       if (vtep_if)
         vtep_if->valid[UPLL_IDX_ADMIN_ST_VTEPI] = UNC_VF_INVALID;
      }
      break;
    case UNC_KT_VTUNNEL_IF:
      pktype = UNC_KT_VTUNNEL;
      result_code = SetOperStatus
         <val_vtunnel_if_st, val_db_vtunnel_if_st>(tkey, notification, dmi,
                                                   oper_status_change);
      if (tkey) {
        val_vtunnel_if *vtunnel_if =
           reinterpret_cast<val_vtunnel_if *>(GetVal(tkey));
        if (vtunnel_if)
          vtunnel_if->valid[UPLL_IDX_ADMIN_ST_VTNL_IF] = UNC_VF_INVALID;
      }
      break;
    case UNC_KT_VTERM_IF:
      pktype = UNC_KT_VTERMINAL;
      result_code = SetOperStatus
         <val_vterm_if_st, val_db_vterm_if_st>(tkey, notification, dmi,
                                               oper_status_change);
      if (tkey) {
        val_vterm_if *vterm_if = reinterpret_cast<val_vterm_if *>
                                     (GetVal(tkey));
        if (vterm_if)
          vterm_if->valid[UPLL_IDX_ADMIN_STATUS_VTERMI] = UNC_VF_INVALID;
      }
      break;
    default:
      UPLL_LOG_DEBUG("oper status attribute not supported");
      return UPLL_RC_SUCCESS;
    }
    if (oper_status_change != ALARM_NOT_SET) {
      if ((notification != kAdminStatusEnabled) &&
          (notification != kAdminStatusDisabled) &&
          (notification != kPortUnknown)) { 
        if (!EnqueOperStatusNotification(tkey, oper_status_change)) {
          UPLL_LOG_DEBUG("Alarm Notification failed");
          return UPLL_RC_ERR_GENERIC;
        }
      }
    } else  if (notification != kCtrlrReconnectIfUp &&
                notification != kCtrlrReconnectIfDown &&
                notification != kCtrlrDisconnect) {
      tkey = tkey->get_next_cfg_key_val();
      continue;
    }
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
      delete ck_parent;
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->UpdateOperStatus(ck_parent, dmi, notification, false, save_to_db);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("UpdateOperStatus failed for %d key_type",
                      ck_parent->get_key_type());
      delete ck_parent;
      return result_code;
    }
    if (save_to_db)
      delete ck_parent;
    if (upd_if && upd_remif &&
        notification != kCtrlrReconnectIfUp && 
        notification != kCtrlrReconnectIfDown && 
        notification != kBoundaryFault && 
        notification != kBoundaryFaultReset) { 
      uint8_t vlink_flag = 0;
      uint8_t remote_if = 0;
      ConfigKeyVal *ck_vlink = NULL;
      /* interface is a member of a vlink */
      GET_USER_DATA_FLAGS(ikey, vlink_flag);
      switch (vlink_flag & VIF_TYPE) {
      case kVlinkBoundaryNode1:
      case kVlinkInternalNode1:
        SET_USER_DATA_FLAGS(ikey, kVlinkVnode1);
        remote_if = 1;
        break;
      case kVlinkBoundaryNode2:
      case kVlinkInternalNode2:
        SET_USER_DATA_FLAGS(ikey, kVlinkVnode2);
        remote_if = 0;
        break;
      default:
        if (skip) {
          UPLL_LOG_DEBUG("Not member of vlink %s",(ikey->ToStr()).c_str());
          goto flush_db ;
        }
        tkey = tkey->get_next_cfg_key_val();
        continue;
      }
      VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>
                    (const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->GetVlinkKeyVal(ikey, UPLL_DT_STATE, ck_vlink, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        return result_code;
      }
      SET_USER_DATA_FLAGS(ikey, vlink_flag);
      result_code = mgr->SetOperStatus(ck_vlink, dmi, notification, true);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("SetOperStatus failed %d", result_code);
        return result_code;
      }
      /* set the status of the remote interface */
      ConfigKeyVal *vnif = NULL;
      result_code = mgr->GetVnodeIfFromVlink(ck_vlink,&vnif,dmi,remote_if);
      if (result_code != UPLL_RC_SUCCESS &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("get remote interface failed %d", result_code);
        return result_code;
      }
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
        return UPLL_RC_SUCCESS;

      VnodeChildMoMgr *rem_mgr = reinterpret_cast<VnodeChildMoMgr *>
                  (const_cast<MoManager *>(GetMoManager(vnif->get_key_type())));
      if (!rem_mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = rem_mgr->UpdateOperStatus(vnif, dmi, notification, true,
                                          true,false);
      if (vnif) delete vnif;
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error updating rem interface oper status %d", 
                        result_code);
        return result_code;
      }
      if (ck_vlink) delete ck_vlink;
    }
    if (skip) break;
    tkey = tkey->get_next_cfg_key_val();
  }
  if (pktype == UNC_KT_ROOT)
   return UPLL_RC_ERR_NO_SUCH_INSTANCE;
flush_db:
  if (!save_to_db) {
    map<string ,ConfigKeyVal *>::iterator it;
    VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>
             (const_cast<MoManager *>(GetMoManager(pktype)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid mgr");
      return UPLL_RC_ERR_GENERIC;
    }
    UPLL_LOG_DEBUG("Flushing vnode to db %d",ktype);
    for (it = mgr->vnode_oper_map.begin(); it != mgr->vnode_oper_map.end(); it++) {
      DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
      ConfigKeyVal *tckv = it->second;
      UPLL_LOG_TRACE("%s %p",(it->first).c_str(),it->second);
      UPLL_LOG_DEBUG("nextvnode %p",tckv);
      result_code = mgr->UpdateConfigDB(tckv, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                                   &dbop, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in update oper status %d", result_code);
      }
       if (tckv) delete tckv;
    }
    mgr->vnode_oper_map.clear();
  }
  return result_code;
}

template<typename T1, typename T2>
upll_rc_t VnodeChildMoMgr::SetOperStatus(ConfigKeyVal *ikey,
                                  state_notification &notification,
                                  DalDmlIntf *dmi, alarm_status &oper_change) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  /* update corresponding interface operstatus */
  ConfigVal *tmp = (ikey->get_cfg_val()) ?
                    ikey->get_cfg_val()->get_next_cfg_val() : NULL;
  T2 *vn_valst = reinterpret_cast<T2 *>((tmp != NULL) ? tmp->get_val() : NULL);
  if (vn_valst == NULL) return UPLL_RC_ERR_GENERIC;
  T1 *vn_val = reinterpret_cast<T1 *>(vn_valst);
  
  UPLL_LOG_DEBUG("notification %d down_count %d ",notification, vn_valst->down_count);

    /* Update oper status based on notification */
  vn_val->valid[0] = UNC_VF_VALID;
  switch (notification) {
  case kCtrlrReconnect: {
      uint8_t valid_pm,valid_admin,admin_status;
      val_port_map_t *pm = NULL;
      result_code = GetPortMap(ikey, valid_pm, pm, valid_admin, admin_status);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("GetPortMap failed : %d", result_code);
        return result_code;
      }
      if_type vnif_type;
      result_code = GetInterfaceType(ikey, valid_pm, vnif_type);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        return result_code;
      }
      UPLL_LOG_DEBUG("vnif_type is %d",vnif_type);
      switch (vnif_type) {
      case kLinkedInterface:
      case kBoundaryInterface: {
        if (vnif_type == kLinkedInterface) {
          if (ikey->get_key_type() == UNC_KT_VBR_IF) {
            val_vbr_if *vbr_if_val = reinterpret_cast<val_vbr_if*>(GetVal(ikey));
            if (!vbr_if_val) {
              UPLL_LOG_DEBUG("val vlink is NULL");
              return UPLL_RC_ERR_GENERIC;
            }
            admin_status = vbr_if_val->admin_status;
          } else {
          /* vrouter interface will be handled as part of vbrif vlink
          * vterm interface doesn't support internal vlink */
            return UPLL_RC_SUCCESS;
          }
          notification = (admin_status == UPLL_ADMIN_DISABLE) ?
                          kCtrlrReconnectIfDown:
                          kCtrlrReconnectIfUp;
        }
        VlinkMoMgr *vlink_mgr =
           reinterpret_cast<VlinkMoMgr *>(const_cast<MoManager *>
          (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK))));
        ConfigKeyVal *ck_vlink = NULL;
        ConfigKeyVal *ck_remif = NULL;
        result_code = vlink_mgr->GetRemoteIf(ikey,ck_remif, ck_vlink, dmi, UPLL_DT_STATE);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Returning error %d\n",result_code);
          DELETE_IF_NOT_NULL(ck_remif);
          DELETE_IF_NOT_NULL(ck_vlink);
          return result_code;
        }
        bool is_boundary_mapped = false;
        result_code = vlink_mgr->RestoreVlinkOperStatus(ck_vlink, dmi, notification,
                                                        is_boundary_mapped);
        DELETE_IF_NOT_NULL(ck_vlink);
        if (result_code != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(ck_remif);
          UPLL_LOG_DEBUG("restoring vlink operstatus failed");
          return result_code;
        }
        if (is_boundary_mapped == true) {
          /* Boundary vlinks will be handled as part of Boundary event*/
          DELETE_IF_NOT_NULL(ck_remif);
          return UPLL_RC_SUCCESS;
        }
        vn_val->oper_status = (notification == kCtrlrReconnectIfDown)?
                               UPLL_OPER_STATUS_DOWN:
                               UPLL_OPER_STATUS_UP;
        result_code = UpdateRemoteVlinkIf(ck_remif, notification, dmi);
        DELETE_IF_NOT_NULL(ck_remif);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in update oper status %d", result_code);
          return result_code;
        }
      }
      break;
      case kUnboundInterface:
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
        notification = kCtrlrReconnectIfDown;
      break;
      default:
        /* mapped interfaces and boundary vlinks will be handled as part of respective
         * physical events */
        return UPLL_RC_SUCCESS;
    } 
  }
  break;
  case kAdminStatusDisabled:
    if (vn_val->oper_status != UPLL_OPER_STATUS_DOWN) {
      vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
      oper_change = ALARM_OPER_DOWN;
    }
    break;
  case kRemoteBoundaryFault:
    vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    vn_valst->down_count = 1;
    oper_change = ALARM_OPER_DOWN;
    break;
  case kRemoteBoundaryFaultReset:
    vn_val->oper_status = UPLL_OPER_STATUS_UP;
    vn_valst->down_count = 0;
    oper_change = ALARM_OPER_UP;
    break;
  case kRemoteBoundaryResetWithAdminDisabled:
    vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    vn_valst->down_count = 0;
    oper_change = ALARM_OPER_UP;
    break;
  case kAdminStatusEnabled:
    if (vn_valst->down_count == 0) {
      if_type vnif_type;
      uint8_t valid_pm,valid_admin,admin_status;
      val_port_map_t *pm = NULL;
      result_code = GetPortMap(ikey, valid_pm, pm, valid_admin, admin_status);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in GetPortMap %d", result_code);
        return result_code;
      }
      result_code = GetInterfaceType(ikey,valid_pm,vnif_type);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d",result_code);
        return result_code;
      }
      if ((vnif_type == kLinkedInterface) || (vnif_type == kMappedInterface) ||
          (vnif_type == kBoundaryInterface)) {
        vn_val->oper_status = UPLL_OPER_STATUS_UP;
        oper_change = ALARM_OPER_UP;
      } else if (vnif_type == kUnboundInterface)
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    } else
      vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    break;
  case kPortUnknown:
    if (vn_val->oper_status != UPLL_OPER_STATUS_UNKNOWN) {
     oper_change = ALARM_OPER_DOWN;
    }
    vn_valst->down_count = 0;
    vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    break;
  case kCtrlrDisconnect:
    if (vn_valst->down_count > 0)
      vn_valst->down_count = REMOTE_IF_DISCONNECT;
    oper_change = ALARM_OPER_DOWN;
    vn_val->oper_status = UPLL_OPER_STATUS_UNKNOWN;
    break;
  case kPortFault:
    if (vn_valst->down_count == 1)
      return UPLL_RC_SUCCESS;
  case kPathFault:
  case kBoundaryFault:
    if (vn_valst->down_count < 1) {
      vn_valst->down_count = (vn_valst->down_count + 1);
      if (vn_valst->down_count == 1) {
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
        // generate alarm
        oper_change = ALARM_OPER_DOWN;
      }
    }
    break;
  case kPortFaultReset:
  case kPathFaultReset:
  case kBoundaryFaultReset:
    if (vn_valst->down_count > 0) {
      --vn_valst->down_count;
    }
    if (vn_valst->down_count == 0) {
#if 0
        uint8_t valid_pm,valid_admin,admin_status;
        val_port_map_t *pm = NULL;
        GetPortMap(ikey, valid_pm, pm, valid_admin, admin_status);
        if (notification != kBoundaryFaultResetWithAdminDisabled) {
          if (admin_status == UPLL_ADMIN_ENABLE) {
            vn_val->oper_status = UPLL_OPER_STATUS_UP;
          } else
            vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
        } else
          vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
#else
        vn_val->oper_status = UPLL_OPER_STATUS_UP;
#endif
        // reset alarm
        oper_change = ALARM_OPER_UP;
    } else
        vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    break;
  case kPortFaultResetWithAdminDisabled:
  case kBoundaryFaultResetWithAdminDisabled:
    if (vn_valst->down_count > 0) {
      --vn_valst->down_count;
    }
    /* oper status stays down though down_count == 0
       as admin is down */
    vn_val->oper_status = UPLL_OPER_STATUS_DOWN;
    oper_change = ALARM_OPER_UP;
    break;
  default:
      UPLL_LOG_DEBUG("unsupported notification for operstatus update");
      return UPLL_RC_ERR_GENERIC;
    break;  
  }
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ikey, UPLL_DT_STATE, UNC_OP_UPDATE,
                           dmi, &dbop, MAINTBL);
  UPLL_LOG_TRACE("VnodeChild SetOperstatus for VTN after Update is \n %s",
                    ikey->ToStrAll().c_str());
  return result_code;
}


upll_rc_t VnodeChildMoMgr::TxUpdateDtState(unc_key_type_t ktype,
                                           uint32_t session_id,
                                           uint32_t config_id,
                                           DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *ck_vn = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetUninitOperState(ck_vn, dmi);
  if (!ck_vn || result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("UpdateDtState failed %d", result_code);
    return result_code;
  }
  /* Read port status from physical */
  ConfigKeyVal *tkey;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  while (ck_vn) {
    tkey = ck_vn;
    ck_vn = ck_vn->get_next_cfg_key_val();
    tkey->set_next_cfg_key_val(NULL);
    state_notification notification = kAdminStatusEnabled; //noop 
    uint8_t valid_pm,valid_admin,admin_status;
    val_port_map_t *pm = NULL;
    result_code = GetPortMap(tkey, valid_pm, pm,valid_admin,admin_status);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Returning error %d\n",result_code);
      delete tkey; 
      DELETE_IF_NOT_NULL(ck_vn); 
      return result_code;
    }
    UPLL_LOG_DEBUG("valid_pm %d valid_admin %d admin_status %d",
                    valid_pm, valid_admin,admin_status);
   if (admin_status == UPLL_ADMIN_DISABLE) {
        notification = kAdminStatusDisabled;
   } else if (valid_pm == UNC_VF_VALID) {
        val_oper_status port_oper_status;
        GET_USER_DATA_CTRLR_DOMAIN(tkey, ctrlr_dom);
        result_code = GetPortStatusFromPhysical(pm, ctrlr_dom, port_oper_status,
                                              session_id, config_id);
          /* During import Physical don't have the Port status
           * in the running db so its return no such instance
           * so here comment this check */
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error retrieving port status from physical %d %d",
                        result_code, port_oper_status);
        // return result_code;
        }
        val_db_vbr_if_st *vbrif_valst = reinterpret_cast<val_db_vbr_if_st *>
                                         (GetStateVal(tkey));
        switch (port_oper_status) {
        case  UPLL_OPER_STATUS_DOWN:
          if (vbrif_valst->down_count == 0)
            notification = kPortFault;
          else
            notification = kAdminStatusDisabled;
          break;
        case  UPLL_OPER_STATUS_UNKNOWN:
          notification = kPortUnknown;
          break;
        case  UPLL_OPER_STATUS_UP:
        {
          if (vbrif_valst->down_count > 0)
            notification = kPortFaultReset;
          else
            notification = kAdminStatusEnabled;
          break;
        }
        default:
          if (vbrif_valst->down_count == 0)
            notification = kPortFault;
          else
            notification = kAdminStatusEnabled;
          break;
        }
    }
    if ((notification == kAdminStatusEnabled) || 
        (notification != kPortUnknown && 
         admin_status == UPLL_ADMIN_ENABLE)) {
      if_type vnif_type;
      result_code = GetInterfaceType(tkey,valid_pm,vnif_type);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d",result_code);
        delete tkey; 
        DELETE_IF_NOT_NULL(ck_vn); 
        return result_code;
      }
      switch (vnif_type) {
      case kBoundaryInterface:
      case kLinkedInterface: {
        ConfigKeyVal *ck_remif = NULL;
        VlinkMoMgr *vlink_mgr = reinterpret_cast<VlinkMoMgr *>
                      (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
        if (!vlink_mgr) {
          UPLL_LOG_ERROR("Invalid mgr\n");
          delete tkey; 
          DELETE_IF_NOT_NULL(ck_vn); 
          return UPLL_RC_ERR_GENERIC;
        }
        ConfigKeyVal *ck_vlink = NULL;
        result_code = vlink_mgr->GetRemoteIf(tkey,ck_remif, ck_vlink, dmi, UPLL_DT_STATE);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Returning error %d\n",result_code);
          DELETE_IF_NOT_NULL(ck_remif);
          DELETE_IF_NOT_NULL(ck_vlink);
          delete tkey; 
          DELETE_IF_NOT_NULL(ck_vn); 
          return result_code;
        }
        bool is_boundary_mapped = false;
        result_code = vlink_mgr->RestoreVlinkOperStatus(ck_vlink, dmi, notification, is_boundary_mapped);
        DELETE_IF_NOT_NULL(ck_vlink);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Returning error %d\n",result_code);
          DELETE_IF_NOT_NULL(ck_remif);
          delete tkey; 
          DELETE_IF_NOT_NULL(ck_vn); 
          return result_code;
        }
        if (notification == kAdminStatusEnabled) {
          VnodeChildMoMgr *mgr = reinterpret_cast<VnodeChildMoMgr *>
                                   (const_cast<MoManager*>
                                   (GetMoManager(ck_remif->get_key_type())));
          result_code = mgr->GetPortMap(ck_remif, valid_pm, pm,
                                    valid_admin,admin_status);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Returning error %d\n",result_code);
            DELETE_IF_NOT_NULL(ck_remif);
            delete tkey; 
            DELETE_IF_NOT_NULL(ck_vn); 
            return result_code;
          }
          notification = (valid_admin == UNC_VF_VALID &&
                          admin_status == UPLL_ADMIN_DISABLE)?
                         kAdminStatusDisabled:
                         kAdminStatusEnabled;
          DELETE_IF_NOT_NULL(ck_remif);
        }
      }
      break;
      case kMappedInterface:
        break;
      case kUnboundInterface:
      default:
        notification = kAdminStatusDisabled;
        break;
      }
    }
    // vlinked interfaces should be handled as part of vlink dt state update
    result_code = UpdateOperStatus(tkey, dmi, notification, true, true, true);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error updating oper status %d", result_code);
      delete tkey; 
      DELETE_IF_NOT_NULL(ck_vn); 
      return result_code;
    }
#if 0
      DbSubOp dbop1 = { kOpNotRead, kOpMatchNone, kOpInOutNone };
      result_code = UpdateConfigDB(tkey, UPLL_DT_STATE, UNC_OP_UPDATE,
                                      dmi, &dbop1, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
           UPLL_LOG_DEBUG("UpdateConfigDB Executed %d", result_code);
           break;
      }
#endif
     delete tkey;
  }
  return result_code;
}


upll_rc_t VnodeChildMoMgr::GetPortStatusFromPhysical(val_port_map_t *pm,
                            controller_domain_t ctr_domain,
                            val_oper_status &logical_port_operStatus,
                            uint32_t session_id,
                            uint32_t config_id) {
  UPLL_FUNC_TRACE;
  IpcResponse ipc_resp;
  ConfigKeyVal *log_Port_CK = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

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
  result_code = SendIpcReq(session_id, config_id, UNC_OP_READ,
                   UPLL_DT_STATE, log_Port_CK, NULL, &ipc_resp);
  if ((result_code != UPLL_RC_SUCCESS) || (!ipc_resp.ckv_data)) {
    delete log_Port_CK;
    log_Port_CK = NULL;
    logical_port_operStatus = UPLL_OPER_STATUS_DOWN;
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      logical_port_operStatus = UPLL_OPER_STATUS_UNKNOWN;
      UPLL_LOG_DEBUG("Invalid Logical Port Id");
//      return UPLL_RC_ERR_CFG_SEMANTIC;
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

upll_rc_t VnodeChildMoMgr::SetIfOperStatusforPathFault(
                        const key_vnode_type_t &vnode_key,
                        state_notification notification,
                        DalDmlIntf *dmi) {
  ConfigKeyVal *ck_vnode_if = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetMappedInterfaces(vnode_key, dmi, ck_vnode_if);
  if (result_code != UPLL_RC_SUCCESS &&
      result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Operstatus updation failed");
    return result_code;
  }
  ConfigKeyVal *tmp_vnode_if = ck_vnode_if;
  while (tmp_vnode_if != NULL) {
    result_code = UpdateOperStatus(tmp_vnode_if, dmi, notification, true, false, false);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Operstatus updation failed");
      return result_code;
    }
    tmp_vnode_if = tmp_vnode_if->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ck_vnode_if); 
  return result_code;
}

/*This function gives the interfaces mapped to a particular vbridge */
upll_rc_t VnodeChildMoMgr::GetMappedInterfaces(const key_vnode_type_t &vnode_key_type,
                                          DalDmlIntf *dmi,
                                          ConfigKeyVal *&iokey) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  result_code = GetChildConfigKey(iokey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
     UPLL_LOG_DEBUG("GetChildConfigkey Failed");
     return result_code;
  }

  key_vnode *vnode_key = reinterpret_cast<key_vnode*>(iokey->get_key());
  memcpy(vnode_key, &(vnode_key_type.vnode_key), sizeof(key_vnode));
  
  /* Get all the vbridges under the VTN */
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(iokey, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  return result_code;
}

upll_rc_t VnodeChildMoMgr::UpdateVnodeIfOperStatus(ConfigKeyVal *ck_vnode,
                           DalDmlIntf *dmi,
                           state_notification notification) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_if = NULL;
  result_code = GetChildConfigKey(ck_if, ck_vnode);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
                  kOpInOutFlag};
  result_code = ReadConfigDB(ck_if, UPLL_DT_STATE, UNC_OP_READ, dbop, dmi,
                           MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB failed with result_code %d",
                    result_code);
    DELETE_IF_NOT_NULL(ck_if);
    return result_code;
  }
  ConfigKeyVal *tmp = ck_if;
  while (tmp != NULL) {
    ConfigKeyVal *tmp1 = tmp->get_next_cfg_key_val();
    tmp->set_next_cfg_key_val(NULL);
    result_code = UpdateOperStatus(tmp, dmi, notification, true, true, 
                                   true, true);
    DELETE_IF_NOT_NULL(tmp);
    if (result_code != UPLL_RC_SUCCESS) {
       DELETE_IF_NOT_NULL(tmp1);
       UPLL_LOG_DEBUG("Error updating oper status %d", result_code);
       return result_code;
    }
    tmp = tmp1;
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::UpdateRemoteVlinkIf(ConfigKeyVal *ck_remif,
                                        state_notification notification,
                                        DalDmlIntf *dmi) {
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigVal *tmp = (ck_remif->get_cfg_val()) ?
                    ck_remif->get_cfg_val()->get_next_cfg_val() : NULL;
  val_db_vbr_if_st *vn_valst = 
                    reinterpret_cast<val_db_vbr_if_st *>
                    ((tmp != NULL) ? tmp->get_val() : NULL);
  if (vn_valst == NULL) return UPLL_RC_ERR_GENERIC;
  val_vbr_if_st *vn_val = reinterpret_cast<val_vbr_if_st *>(vn_valst);
  vn_val->oper_status = (notification == kCtrlrReconnectIfUp)?
                         UPLL_OPER_STATUS_UP:
                         UPLL_OPER_STATUS_DOWN;
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
 
  unc_key_type_t ktype = ck_remif->get_key_type();
  VnodeChildMoMgr *vnode_child_mgr = reinterpret_cast<VnodeChildMoMgr *>
                (const_cast<MoManager*>(GetMoManager(ktype)));
  if (!vnode_child_mgr) {
    UPLL_LOG_ERROR("Invalid mgr\n");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = vnode_child_mgr->UpdateConfigDB(ck_remif, UPLL_DT_STATE,
                                                UNC_OP_UPDATE, dmi,
                                                &dbop, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Errorin Updating OPerStatus");
    return result_code;
  }
  ConfigKeyVal *parent_node = NULL;
  result_code = vnode_child_mgr->GetParentConfigKey(parent_node, ck_remif);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in retrieving Parent Node Key");
    return result_code;
  }
  unc_key_type_t parent_key_type = parent_node->get_key_type();
  VnodeMoMgr *vnode_mgr = reinterpret_cast<VnodeMoMgr *>
                (const_cast<MoManager*>(GetMoManager(parent_key_type)));
  if (!vnode_mgr) {
    DELETE_IF_NOT_NULL(parent_node);
    UPLL_LOG_ERROR("Invalid mgr\n");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigVal *cfg_val = NULL;
  result_code = vnode_mgr->AllocVal(cfg_val, UPLL_DT_STATE, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    DELETE_IF_NOT_NULL(parent_node);
    UPLL_LOG_DEBUG("Error in allocating Parent Node cfg val");
    return result_code;
  }
  parent_node->SetCfgVal(cfg_val);
  ConfigVal *cv_tmp = (parent_node->get_cfg_val()) ?
                    parent_node->get_cfg_val()->get_next_cfg_val() : NULL;
  val_db_vbr_st *vnode_st = reinterpret_cast<val_db_vbr_st *>
                    ((cv_tmp != NULL) ? cv_tmp->get_val() : NULL);
  if (vnode_st == NULL) {
    DELETE_IF_NOT_NULL(parent_node);
    UPLL_LOG_DEBUG("Error in retrieving state val");
    return UPLL_RC_ERR_GENERIC;
  }
  vnode_st->vbr_val_st.oper_status = (notification == kCtrlrReconnectIfUp)?
                                      UPLL_OPER_STATUS_UP:
                                      UPLL_OPER_STATUS_DOWN;
  vnode_st->vbr_val_st.valid[0] = UNC_VF_VALID;
  result_code = vnode_mgr->UpdateConfigDB(parent_node, UPLL_DT_STATE, UNC_OP_UPDATE, dmi,
                               &dbop, MAINTBL);

  DELETE_IF_NOT_NULL(parent_node);
  return result_code; 
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
    } else
      ckv->AppendCfgVal((ikey->get_cfg_val())->get_st_num(), if_val2);
       
    SET_USER_DATA(ckv, ikey);

    UPLL_LOG_DEBUG("Before read config DB Ckv%s \n",ckv->ToStrAll().c_str());
    /* Read Multiple matched entrie's based on logical_id
     * in the same controller */
    DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr,
                     kOpInOutFlag};
    result_code = ReadConfigDB(ckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
                               dbop, dmi, MAINTBL);
    UPLL_LOG_TRACE("Read config Db result %u", result_code);
    if (result_code == UPLL_RC_SUCCESS) {
      ConfigKeyVal *tmp = ckv;
      while (tmp) {
        if (!memcmp(ikey->get_key(), tmp->get_key(),
              sizeof(key_vbr_if_t))) {
          UPLL_LOG_TRACE("Looking on the Same key");
        } else {
          T3   *if_val_read = NULL;
          bool match        = false;
          if (ikey->get_key_type() == UNC_KT_VBR_IF) {
            if_val_read = reinterpret_cast<T3 *>(&(reinterpret_cast<val_drv_vbr_if *>
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
            if (if_val_read->portmap.valid[UPLL_IDX_VLAN_ID_PM] == UNC_VF_VALID) {
              if (if_val1->portmap.tagged == if_val_read->portmap.tagged)
                match = true;
            } else
                match = true;
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

upll_rc_t VnodeChildMoMgr::PortStatusHandler(ConfigKeyVal *ikey,
                           bool oper_status, DalDmlIntf *dmi  ) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  state_notification notification =
       (oper_status == UPLL_OPER_STATUS_UP) ? kPortFaultReset : kPortFault;
  // vlinked interfaces should not be handled
  result_code = UpdateOperStatus(ikey, dmi, notification, false, false, false);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      result_code = UPLL_RC_SUCCESS;
    else
      UPLL_LOG_DEBUG("Invalid oper status update %d", result_code);
    return result_code;
  }
  if (notification == kPortFaultReset) {
    unc_key_type_t keytype = UNC_KT_ROOT; 
    if (ikey->get_key_type() == UNC_KT_VTERM_IF)
      keytype = UNC_KT_VTERMINAL;
    else if (ikey->get_key_type() == UNC_KT_VBR_IF)
      keytype = UNC_KT_VBRIDGE;
    VnodeMoMgr *mgr = reinterpret_cast<VnodeMoMgr *>(const_cast<MoManager *>
                             (GetMoManager(keytype)));
    if (!mgr) {
      UPLL_LOG_DEBUG("Returning error\n");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = mgr->TxUpdateDtState(keytype, 0, 0, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("failed to update vnode oper status %d\n", result_code);
      return result_code;
    }

    VtnMoMgr *vtn_mgr = reinterpret_cast<VtnMoMgr *>
                    (const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
    if (!vtn_mgr) {
      UPLL_LOG_DEBUG("Returning error\n");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = vtn_mgr->TxUpdateDtState(UNC_KT_VTN, 0, 0, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("failed to update vtn oper status %d\n", result_code);
    }
  }
  return result_code;
}

upll_rc_t VnodeChildMoMgr::PortStatusHandler(ConfigKeyVal *ikey,
                                             const char *ctrlr_name,
                                             const char *domain_name,
                                             const char *logical_port_id,
                                             bool oper_status,
                                             DalDmlIntf *dmi  ) {
  UPLL_FUNC_TRACE;

  UPLL_LOG_TRACE("controller_name is : (%s) portid :(%s) domain_id :(%s)"
        "oper_status :(%d)", ctrlr_name, logical_port_id, domain_name, oper_status);
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t keytype = ikey->get_key_type();

  if (keytype == UNC_KT_VBR_IF) {
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
  } else {
    UPLL_LOG_DEBUG("Unsupported keytype");
    return UPLL_RC_ERR_GENERIC;
  }

  SET_USER_DATA_CTRLR(ikey, ctrlr_name);
  SET_USER_DATA_DOMAIN(ikey, domain_name);

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutFlag };
  /* Existance check in vbrif table */
  result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING, UNC_OP_READ,
                               dmi, &dbop, MAINTBL);
  /* Update VBR_If OperStatus */
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      result_code = UPLL_RC_SUCCESS;
  else if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code)
    result_code = PortStatusHandler(ikey, oper_status, dmi);

  return result_code;
}
/*This function gives the vNodes connected to a particular switch log port*/
upll_rc_t VnodeChildMoMgr::GetMappedVnodes(
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
  result_code = ReadConfigDB(ck_vbr_if, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    DELETE_IF_NOT_NULL(ck_vbr_if);
    return UPLL_RC_SUCCESS;
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in reading: %d", result_code);
    DELETE_IF_NOT_NULL(ck_vbr_if);
    return result_code;
  }
  ConfigKeyVal *tmp = ck_vbr_if;
  while (tmp) {
    /* populate sw_vbridge set with vbridges mapped to the specified switch*/
    key_vbr_if_t *tkey = reinterpret_cast<key_vbr_if_t *>
                         (tmp->get_key());
    key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>
                         (ConfigKeyVal::Malloc(sizeof(key_vbr_t)));
    uuu::upll_strncpy(vbr_key->vbridge_name, tkey->vbr_key.vbridge_name,
                      (kMaxLenVnodeName + 1));
    uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
                      tkey->vbr_key.vtn_key.vtn_name,
                      (kMaxLenVtnName + 1));
    key_vnode_type_t vnode_type;
    vnode_type.vnode_key = *(reinterpret_cast<key_vnode_t *>(vbr_key));
    vnode_type.key_type = tmp->get_key_type();
    sw_vbridge_set->insert(vnode_type);
    ConfigKeyVal::Free(vbr_key);
    tmp = tmp->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(ck_vbr_if);
  return result_code;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
